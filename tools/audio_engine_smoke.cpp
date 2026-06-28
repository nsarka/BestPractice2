#include "qt/AudioEngine.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QTimer>

#include <cstdio>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  if (argc != 3) {
    std::fprintf(stderr, "usage: audio_engine_smoke input.wav output.wav\n");
    return 2;
  }

  int result = 1;
  AudioEngine engine;
  QObject::connect(&engine, &AudioEngine::ready, &app, [&](const QString&) {
    QTimer::singleShot(300, &app, [&] {
      const qint64 firstPosition = engine.position();
      engine.setSpeed(800);
      engine.setPitch(3, 25);
      engine.setQuality(2);
      engine.setAntiAliasEnabled(true);
      engine.setKaraokeSettings(128, 300, 12000);
      engine.setKaraokeEnabled(true);

      QTimer::singleShot(500, &app, [&, firstPosition] {
        QString error;
        const bool playbackAdvanced = firstPosition > 0 && engine.position() > firstPosition;
        const bool durationOkay = engine.duration() >= 1900 && engine.duration() <= 2100;
        const bool saved = engine.saveProcessedWav(QString::fromLocal8Bit(argv[2]), &error);
        const qint64 outputSize = QFileInfo(QString::fromLocal8Bit(argv[2])).size();
        const bool outputOkay = outputSize >= 420000 && outputSize <= 460000;
        if (!playbackAdvanced) {
          std::fprintf(stderr, "streaming playback did not advance: %lld -> %lld ms\n",
                       static_cast<long long>(firstPosition),
                       static_cast<long long>(engine.position()));
        }
        if (!durationOkay) {
          std::fprintf(stderr, "unexpected source duration: %lld ms\n",
                       static_cast<long long>(engine.duration()));
        }
        if (!saved) {
          std::fprintf(stderr, "save failed: %s\n", error.toLocal8Bit().constData());
        }
        if (!outputOkay) {
          std::fprintf(stderr, "unexpected export size: %lld bytes\n",
                       static_cast<long long>(outputSize));
        }
        result = playbackAdvanced && durationOkay && saved && outputOkay ? 0 : 1;
        app.quit();
      });
    });
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
