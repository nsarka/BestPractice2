#include "qt/AudioEngine.h"

#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QUrl>

#include <cstdio>
#include <functional>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  if (argc != 3) {
    std::fprintf(stderr, "usage: audio_engine_smoke input.wav output.wav\n");
    return 2;
  }

  const QString outputPath = QFileInfo(QString::fromLocal8Bit(argv[2])).absoluteFilePath();
  const QFileInfo outputInfo(outputPath);
  const QString mp3OutputPath = outputInfo.dir().filePath(
    outputInfo.completeBaseName() + ".mp3");
  const QString canceledPath = outputInfo.dir().filePath(
    outputInfo.completeBaseName() + "-canceled.wav");
  const QString canceledMp3Path = outputInfo.dir().filePath(
    outputInfo.completeBaseName() + "-canceled.mp3");
  QFile::remove(outputPath);
  QFile::remove(mp3OutputPath);
  QFile::remove(canceledPath);
  QFile::remove(canceledMp3Path);

  int result = 1;
  bool readyBeforeDecodeCompleted = false;
  bool playbackOkay = false;
  bool durationOkay = false;
  bool wavExportOkay = false;
  bool mp3ExportOkay = false;
  qint64 mp3DecodedMicroseconds = 0;
  int cancellationStage = 0;
  AudioEngine engine;

  std::function<void()> beginExports = [&] {
    QString error;
    if (!engine.startExport(canceledPath, 0, engine.duration(), &error)) {
      std::fprintf(stderr, "could not start cancellation test: %s\n",
                   error.toLocal8Bit().constData());
      app.quit();
      return;
    }
    engine.cancelExport();
  };

  QObject::connect(&engine, &AudioEngine::ready, &app, [&](const QString&) {
    readyBeforeDecodeCompleted = !engine.isDecodeComplete();
    QTimer::singleShot(300, &app, [&] {
      const qint64 firstPosition = engine.position();
      engine.setSpeed(800);
      engine.setPitch(3, 25);
      engine.setQuality(2);
      engine.setAntiAliasEnabled(true);
      engine.setKaraokeSettings(128, 300, 12000);
      engine.setKaraokeEnabled(true);

      QTimer::singleShot(500, &app, [&, firstPosition] {
        playbackOkay = firstPosition > 0 && engine.position() > firstPosition;
        durationOkay = engine.duration() >= 1900 && engine.duration() <= 2100;
        if (!playbackOkay) {
          std::fprintf(stderr, "streaming playback did not advance: %lld -> %lld ms\n",
                       static_cast<long long>(firstPosition),
                       static_cast<long long>(engine.position()));
        }
        if (!durationOkay) {
          std::fprintf(stderr, "unexpected source duration: %lld ms\n",
                       static_cast<long long>(engine.duration()));
        }
        if (engine.isDecodeComplete()) {
          beginExports();
        } else {
          QObject::connect(&engine, &AudioEngine::decodeCompleted, &app, beginExports);
        }
      });
    });
  });
  QObject::connect(&engine, &AudioEngine::exportCanceled, &app, [&] {
    const QString canceledOutput = cancellationStage == 0
      ? canceledPath : canceledMp3Path;
    if (QFileInfo::exists(canceledOutput)) {
      std::fprintf(stderr, "canceled export left a partial file: %s\n",
                   canceledOutput.toLocal8Bit().constData());
      app.quit();
      return;
    }

    if (cancellationStage == 0) {
      cancellationStage = 1;
      QString error;
      if (!engine.startExport(canceledMp3Path, 0, engine.duration(), &error)) {
        std::fprintf(stderr, "could not start MP3 cancellation test: %s\n",
                     error.toLocal8Bit().constData());
        app.quit();
        return;
      }
      engine.cancelExport();
      return;
    }

    cancellationStage = 2;
    QString error;
    if (!engine.startExport(outputPath, 500, 1500, &error)) {
      std::fprintf(stderr, "bounded export failed to start: %s\n",
                   error.toLocal8Bit().constData());
      app.quit();
    }
  });
  QObject::connect(&engine, &AudioEngine::exportFinished, &app, [&](const QString& path) {
    if (path == outputPath) {
      const qint64 outputSize = QFileInfo(outputPath).size();
      wavExportOkay = outputSize >= 190000 && outputSize <= 240000;
      if (!wavExportOkay) {
        std::fprintf(stderr, "unexpected bounded WAV export size: %lld bytes\n",
                     static_cast<long long>(outputSize));
        app.quit();
        return;
      }

      QString error;
      if (!engine.startExport(mp3OutputPath, 500, 1500, &error, 192000)) {
        std::fprintf(stderr, "MP3 export failed to start: %s\n",
                     error.toLocal8Bit().constData());
        app.quit();
      }
      return;
    }

    const qint64 outputSize = QFileInfo(mp3OutputPath).size();
    mp3ExportOkay = path == mp3OutputPath && outputSize > 1000;
    if (!mp3ExportOkay) {
      std::fprintf(stderr, "unexpected bounded MP3 export size: %lld bytes\n",
                   static_cast<long long>(outputSize));
      app.quit();
      return;
    }

    if (!readyBeforeDecodeCompleted) {
      std::fprintf(stderr, "playback was not ready before decoding completed\n");
    }

    auto* decoder = new QAudioDecoder(&app);
    QObject::connect(decoder, &QAudioDecoder::bufferReady, &app, [&, decoder] {
      const QAudioBuffer buffer = decoder->read();
      if (buffer.isValid()) {
        mp3DecodedMicroseconds += buffer.duration();
      }
    });
    QObject::connect(decoder, &QAudioDecoder::finished, &app, [&] {
      const bool mp3DurationOkay = mp3DecodedMicroseconds >= 900000
        && mp3DecodedMicroseconds <= 1600000;
      if (!mp3DurationOkay) {
        std::fprintf(stderr, "unexpected decoded MP3 duration: %lld us\n",
                     static_cast<long long>(mp3DecodedMicroseconds));
      }
      result = readyBeforeDecodeCompleted && playbackOkay && durationOkay
        && wavExportOkay && mp3ExportOkay && mp3DurationOkay ? 0 : 1;
      app.quit();
    });
    QObject::connect(decoder, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error),
                     &app, [&, decoder](QAudioDecoder::Error) {
      std::fprintf(stderr, "exported MP3 decode failed: %s\n",
                   decoder->errorString().toLocal8Bit().constData());
      app.quit();
    });
    decoder->setSource(QUrl::fromLocalFile(mp3OutputPath));
    decoder->start();
  });
  QObject::connect(&engine, &AudioEngine::exportFailed, &app, [&](const QString& error) {
    std::fprintf(stderr, "export failed: %s\n", error.toLocal8Bit().constData());
    app.quit();
  });
  QObject::connect(&engine, &AudioEngine::errorOccurred, &app, [&](const QString& error) {
    std::fprintf(stderr, "%s\n", error.toLocal8Bit().constData());
    app.quit();
  });
  QTimer::singleShot(20000, &app, [&] {
    std::fprintf(stderr, "DSP smoke test timed out\n");
    app.quit();
  });

  engine.loadFile(QString::fromLocal8Bit(argv[1]));
  app.exec();
  return result;
}
