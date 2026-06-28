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

  bool saveProcessedWav(const QString& path, QString* errorMessage = nullptr) const;

  bool isPlaying() const;
  bool hasAudio() const;
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

private:
  void beginDecode(const QString& path);
  void applyParameters();
  void updatePosition();

  QAudioDecoder* decoder_ = nullptr;
  QAudioSink* sink_ = nullptr;
  StreamingAudioDevice* streamDevice_ = nullptr;
  QTimer* positionTimer_ = nullptr;

  QByteArray decodedAudio_;
  QString sourcePath_;
  std::unique_ptr<AudioEngineParameters> parameters_;
  int sampleRate_ = 44100;
  qint64 durationMs_ = 0;
};

#endif
