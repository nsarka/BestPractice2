#ifndef BESTPRACTICE_QT_AUDIOENGINE_H
#define BESTPRACTICE_QT_AUDIOENGINE_H

#include <QByteArray>
#include <QObject>
#include <QString>

#include <memory>

class QAudioDecoder;
class QAudioSink;
class QTimer;
class StreamingAudioDevice;
struct AudioEngineParameters;
struct AudioExportSession;

class AudioEngine : public QObject
{
  Q_OBJECT

public:
  explicit AudioEngine(QObject* parent = nullptr);
  ~AudioEngine() override;

  void loadFile(const QString& path);
  void togglePlayback();
  void seek(qint64 milliseconds);
  void setVolume(int value);

  void setSpeed(int value);
  void setPitch(int semitones, int cents);
  void setAntiAliasEnabled(bool enabled);
  void setQuality(int quality);
  void setKaraokeEnabled(bool enabled);
  void setKaraokeSettings(int vocalPosition, int bassCutoff, int trebleRange);

  bool startExport(const QString& path, qint64 startMilliseconds,
                   qint64 endMilliseconds, QString* errorMessage = nullptr,
                   int mp3BitRate = 192000);
  void cancelExport();

  bool isPlaying() const;
  bool hasAudio() const;
  bool isDecodeComplete() const;
  bool isExporting() const;
  qint64 position() const;
  qint64 duration() const;

signals:
  void positionChanged(qint64 milliseconds);
  void durationChanged(qint64 milliseconds);
  void playbackStateChanged(bool playing);
  void processingChanged(bool processing);
  void ready(const QString& path);
  void message(const QString& text);
  void errorOccurred(const QString& text);
  void decodeCompleted();
  void exportProgress(int percent);
  void exportFinished(const QString& path);
  void exportCanceled();
  void exportFailed(const QString& text);

private:
  void beginDecode(const QString& path);
  void applyParameters();
  void updatePosition();
  void startProgressivePlayback();
  void processExportBlock();
  void finishExport();
  void completeMp3Export();
  void failExport(const QString& error);

  QAudioDecoder* decoder_ = nullptr;
  QAudioSink* sink_ = nullptr;
  StreamingAudioDevice* streamDevice_ = nullptr;
  QTimer* positionTimer_ = nullptr;

  QByteArray decodedAudio_;
  QString sourcePath_;
  std::unique_ptr<AudioEngineParameters> parameters_;
  std::unique_ptr<AudioExportSession> exportSession_;
  int sampleRate_ = 44100;
  qint64 durationMs_ = 0;
  qint64 streamingSourceFrames_ = 0;
  bool playbackStarted_ = false;
  bool decodeComplete_ = false;
  bool userPaused_ = false;
};

#endif
