#include "qt/AudioEngine.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QTimer>

#include <cstdio>
#include <functional>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  if (argc != 3) {
    std::fprintf(stderr, "usage: audio_engine_smoke input.wav output.wav\n");
    return 2;
  }

  const QString outputPath = QString::fromLocal8Bit(argv[2]);
  const QString canceledPath = outputPath + ".partial";
  QFile::remove(outputPath);
  QFile::remove(canceledPath);

  int result = 1;
  bool readyBeforeDecodeCompleted = false;
  bool playbackOkay = false;
  bool durationOkay = false;
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
    if (QFileInfo::exists(canceledPath)) {
      std::fprintf(stderr, "canceled export left a partial file\n");
      app.quit();
      return;
    }
    QString error;
    if (!engine.startExport(outputPath, 500, 1500, &error)) {
      std::fprintf(stderr, "bounded export failed to start: %s\n",
                   error.toLocal8Bit().constData());
      app.quit();
    }
  });
  QObject::connect(&engine, &AudioEngine::exportFinished, &app, [&](const QString&) {
    const qint64 outputSize = QFileInfo(outputPath).size();
    const bool boundedOutputOkay = outputSize >= 190000 && outputSize <= 240000;
    if (!readyBeforeDecodeCompleted) {
      std::fprintf(stderr, "playback was not ready before decoding completed\n");
    }
    if (!boundedOutputOkay) {
      std::fprintf(stderr, "unexpected bounded export size: %lld bytes\n",
                   static_cast<long long>(outputSize));
    }
    result = readyBeforeDecodeCompleted && playbackOkay && durationOkay
      && boundedOutputOkay ? 0 : 1;
    app.quit();
  });
  QObject::connect(&engine, &AudioEngine::exportFailed, &app, [&](const QString& error) {
    std::fprintf(stderr, "export failed: %s\n", error.toLocal8Bit().constData());
    app.quit();
  });
  QObject::connect(&engine, &AudioEngine::errorOccurred, &app, [&](const QString& error) {
    std::fprintf(stderr, "%s\n", error.toLocal8Bit().constData());
    app.quit();
  });
  QTimer::singleShot(15000, &app, [&] {
    std::fprintf(stderr, "DSP smoke test timed out\n");
    app.quit();
  });

  engine.loadFile(QString::fromLocal8Bit(argv[1]));
  app.exec();
  return result;
}
