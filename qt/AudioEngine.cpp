#include "AudioEngine.h"

#include "CWSOLA.h"
#include "IIRButter.h"
#include "SampleData.h"
#include "TKaraokeProcessor.h"
#include "TReader.h"
#include "TResampler.h"

#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QAudioSink>
#include <QDataStream>
#include <QFile>
#include <QIODevice>
#include <QMediaDevices>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

namespace
{
constexpr int kBytesPerFrame = 4;
constexpr qsizetype kSinkBufferFrames = 4096;

class MemoryReader final : public AudioReader::TReader
{
public:
  MemoryReader(QByteArray pcm, int sampleRate, qint64 startFrame = 0)
    : pcm_(std::move(pcm)), sampleRate_(sampleRate)
  {
    Seek(startFrame);
  }

  __int64 Seek(__int64 samplePosition) override
  {
    position_ = std::clamp<qint64>(samplePosition, 0, GetLength());
    IsEof = position_ >= GetLength();
    return position_;
  }

  int FillBuffer(Buffer& buffer, int samplesRequested) override
  {
    const qint64 bytesRequested = static_cast<qint64>(samplesRequested) * kBytesPerFrame;
    const qint64 bytePosition = position_ * kBytesPerFrame;
    const qint64 available = std::max<qint64>(0, pcm_.size() - bytePosition);
    const qint64 copied = std::min(bytesRequested, available);

    if (copied > 0) {
      const char* begin = pcm_.constData() + bytePosition;
      buffer.data.insert(buffer.data.end(), begin, begin + copied);
    }
    buffer.data.insert(buffer.data.end(), static_cast<size_t>(bytesRequested - copied), 0);

    position_ += copied / kBytesPerFrame;
    IsEof = position_ >= GetLength();
    return static_cast<int>(copied / kBytesPerFrame);
  }

  __int64 GetLength() override
  {
    return pcm_.size() / kBytesPerFrame;
  }

  TReader* MakeCopy() override
  {
    return new MemoryReader(pcm_, sampleRate_, position_);
  }

  int GetSampleFrequency() override
  {
    return sampleRate_;
  }

private:
  QByteArray pcm_;
  int sampleRate_;
  qint64 position_ = 0;
};

void writeWavHeader(QDataStream& stream, quint32 dataSize, int sampleRate)
{
  stream.setByteOrder(QDataStream::LittleEndian);
  stream.writeRawData("RIFF", 4);
  stream << quint32(36 + dataSize);
  stream.writeRawData("WAVEfmt ", 8);
  stream << quint32(16) << quint16(1) << quint16(2);
  stream << quint32(sampleRate) << quint32(sampleRate * kBytesPerFrame);
  stream << quint16(kBytesPerFrame) << quint16(16);
  stream.writeRawData("data", 4);
  stream << dataSize;
}
}

struct AudioEngineParameters
{
  int speed = 1000;
  int semitones = 0;
  int cents = 0;
  int quality = 2;
  int vocalPosition = 128;
  int bassCutoff = 0;
  int trebleRange = 0;
  bool antiAlias = true;
  bool karaoke = false;

  bool operator==(const AudioEngineParameters& other) const
  {
    return speed == other.speed && semitones == other.semitones
      && cents == other.cents && quality == other.quality
      && vocalPosition == other.vocalPosition
      && bassCutoff == other.bassCutoff && trebleRange == other.trebleRange
      && antiAlias == other.antiAlias && karaoke == other.karaoke;
  }
};

namespace
{
struct ProcessedBlock
{
  QByteArray audio;
  qint64 sourceStart = 0;
  qint64 sourceEnd = 0;
};

class DspBlockProcessor
{
public:
  DspBlockProcessor(const QByteArray& source, int rate,
                    const AudioEngineParameters& settings, qint64 startFrame)
    : reader_(source, rate, startFrame),
      windowLength_(std::max(64, rate / 10)),
      blockBytes_(2 * windowLength_ * kBytesPerFrame),
      stretcher_(windowLength_, blockBytes_, windowLength_ / 4, &reader_),
      parameters_(settings),
      sourceFrames_(source.size() / kBytesPerFrame),
      nextSourceFrame_(std::clamp<qint64>(startFrame, 0, sourceFrames_)),
      pitchScale_(static_cast<float>(std::pow(2.0,
        -(settings.semitones + settings.cents / 100.0) / 12.0))),
      lowPass_(rate, 8)
  {
    if (parameters_.quality == 0) {
      stretcher_.SetMode(tsmWOLA);
    } else {
      stretcher_.SetMode(tsmWSOLA);
      stretcher_.SetMode(parameters_.quality == 1 ? wxmFast : wxmGood);
    }

    karaoke_.SetSampleFreq(rate);
    karaoke_.SetBassPassThrough(parameters_.bassCutoff);
    karaoke_.SetHighPassThrough(parameters_.trebleRange > 0
      ? rate / 2 - parameters_.trebleRange
      : 0);
    karaoke_.SetVocalPosition(parameters_.vocalPosition);

    if (pitchScale_ < 1.0f && parameters_.antiAlias) {
      lowPass_.SetCutOff(rate * pitchScale_ / 4.0);
    }
  }

  bool atEnd() const
  {
    return nextSourceFrame_ >= sourceFrames_;
  }

  ProcessedBlock processNext()
  {
    ProcessedBlock block;
    block.sourceStart = nextSourceFrame_;
    if (atEnd()) {
      block.sourceEnd = nextSourceFrame_;
      return block;
    }

    std::vector<char> stretched(static_cast<size_t>(blockBytes_), 0);
    const int bytesUsed = stretcher_.ProcessBlock(
      parameters_.speed * pitchScale_, stretched.data());
    const qint64 consumed = std::max(1, bytesUsed / kBytesPerFrame);
    nextSourceFrame_ = std::min(sourceFrames_, nextSourceFrame_ + consumed);
    block.sourceEnd = nextSourceFrame_;

    if (parameters_.karaoke) {
      karaoke_.ProcessAudio(reinterpret_cast<SampleData*>(stretched.data()),
                            blockBytes_ / kBytesPerFrame);
    }

    char* finalData = stretched.data();
    std::vector<char> filtered;
    if (pitchScale_ < 1.0f && parameters_.antiAlias) {
      filtered.resize(static_cast<size_t>(blockBytes_));
      lowPass_.Filter(stretched.data(), filtered.data(), blockBytes_ / kBytesPerFrame);
      finalData = filtered.data();
    }

    if (std::abs(pitchScale_ - 1.0f) > 0.00001f) {
      const size_t capacity = static_cast<size_t>(
        std::ceil(blockBytes_ * std::max(1.0f, pitchScale_) + 16));
      std::vector<char> resampled(capacity, 0);
      const int frames = resampler_.Resample(
        resampled.data(), finalData, pitchScale_, blockBytes_);
      block.audio.append(resampled.data(), frames * kBytesPerFrame);
    } else {
      block.audio.append(finalData, blockBytes_);
    }

    const qint64 desiredFrames = (block.sourceEnd - block.sourceStart) * 1000
      / std::max(1, parameters_.speed);
    block.audio.truncate(static_cast<qsizetype>(std::min<qint64>(
      block.audio.size(), desiredFrames * kBytesPerFrame)));
    return block;
  }

private:
  MemoryReader reader_;
  int windowLength_;
  int blockBytes_;
  CWOLA stretcher_;
  AudioEngineParameters parameters_;
  qint64 sourceFrames_;
  qint64 nextSourceFrame_;
  float pitchScale_;
  TResampler resampler_;
  TKaraokeProcessor karaoke_;
  CIIRButterLow lowPass_;
};
}

class StreamingAudioDevice final : public QIODevice
{
public:
  explicit StreamingAudioDevice(QObject* parent = nullptr)
    : QIODevice(parent)
  {
    open(QIODevice::ReadOnly);
  }

  void setSource(const QByteArray& source, int sampleRate,
                 const AudioEngineParameters& parameters)
  {
    QMutexLocker lock(&mutex_);
    source_ = source;
    sampleRate_ = sampleRate;
    desiredParameters_ = parameters;
    activeParameters_ = parameters;
    parameterRevision_ = activeRevision_ = 0;
    resetProcessor(0);
  }

  void setParameters(const AudioEngineParameters& parameters)
  {
    QMutexLocker lock(&mutex_);
    if (!(desiredParameters_ == parameters)) {
      desiredParameters_ = parameters;
      ++parameterRevision_;
    }
  }

  void seekToFrame(qint64 frame)
  {
    QMutexLocker lock(&mutex_);
    activeParameters_ = desiredParameters_;
    activeRevision_ = parameterRevision_;
    resetProcessor(frame);
  }

  qint64 currentFrame() const
  {
    QMutexLocker lock(&mutex_);
    return currentFrame_;
  }

  bool isSequential() const override
  {
    return true;
  }

  bool atEnd() const override
  {
    QMutexLocker lock(&mutex_);
    return processor_ && processor_->atEnd() && pendingOffset_ >= pending_.size();
  }

  qint64 bytesAvailable() const override
  {
    QMutexLocker lock(&mutex_);
    const qint64 generatable = processor_ && !processor_->atEnd()
      ? kSinkBufferFrames * kBytesPerFrame
      : 0;
    return pending_.size() - pendingOffset_ + generatable
      + QIODevice::bytesAvailable();
  }

protected:
  qint64 readData(char* data, qint64 maxSize) override
  {
    QMutexLocker lock(&mutex_);
    if (!processor_ || maxSize <= 0) {
      return 0;
    }

    if (activeRevision_ != parameterRevision_) {
      activeParameters_ = desiredParameters_;
      activeRevision_ = parameterRevision_;
      resetProcessor(currentFrame_);
    }

    qint64 written = 0;
    while (written < maxSize) {
      if (pendingOffset_ >= pending_.size()) {
        const ProcessedBlock block = processor_->processNext();
        pending_ = block.audio;
        pendingOffset_ = 0;
        pendingSourceStart_ = block.sourceStart;
        pendingSourceEnd_ = block.sourceEnd;
        currentFrame_ = block.sourceStart;
        if (pending_.isEmpty()) {
          break;
        }
      }

      const qint64 available = pending_.size() - pendingOffset_;
      const qint64 copied = std::min(maxSize - written, available);
      std::memcpy(data + written, pending_.constData() + pendingOffset_,
                  static_cast<size_t>(copied));
      written += copied;
      pendingOffset_ += copied;

      const qint64 sourceSpan = pendingSourceEnd_ - pendingSourceStart_;
      currentFrame_ = pendingSourceStart_ + sourceSpan * pendingOffset_
        / std::max<qint64>(1, pending_.size());
    }
    return written;
  }

  qint64 writeData(const char*, qint64) override
  {
    return -1;
  }

private:
  void resetProcessor(qint64 frame)
  {
    const qint64 sourceFrames = source_.size() / kBytesPerFrame;
    currentFrame_ = std::clamp<qint64>(frame, 0, sourceFrames);
    pending_.clear();
    pendingOffset_ = 0;
    pendingSourceStart_ = pendingSourceEnd_ = currentFrame_;
    processor_ = std::make_unique<DspBlockProcessor>(
      source_, sampleRate_, activeParameters_, currentFrame_);
  }

  mutable QMutex mutex_;
  QByteArray source_;
  int sampleRate_ = 44100;
  AudioEngineParameters desiredParameters_;
  AudioEngineParameters activeParameters_;
  quint64 parameterRevision_ = 0;
  quint64 activeRevision_ = 0;
  std::unique_ptr<DspBlockProcessor> processor_;
  QByteArray pending_;
  qint64 pendingOffset_ = 0;
  qint64 pendingSourceStart_ = 0;
  qint64 pendingSourceEnd_ = 0;
  qint64 currentFrame_ = 0;
};

AudioEngine::AudioEngine(QObject* parent)
  : QObject(parent), parameters_(std::make_unique<AudioEngineParameters>())
{
  QAudioFormat format;
  format.setSampleRate(sampleRate_);
  format.setChannelCount(2);
  format.setSampleFormat(QAudioFormat::Int16);

  decoder_ = new QAudioDecoder(this);
  decoder_->setAudioFormat(format);

  sink_ = new QAudioSink(QMediaDevices::defaultAudioOutput(), format, this);
  sink_->setBufferFrameCount(kSinkBufferFrames);
  streamDevice_ = new StreamingAudioDevice(this);

  positionTimer_ = new QTimer(this);
  positionTimer_->setInterval(30);
  connect(positionTimer_, &QTimer::timeout, this, &AudioEngine::updatePosition);

  connect(decoder_, &QAudioDecoder::bufferReady, this, [this] {
    const QAudioBuffer buffer = decoder_->read();
    if (buffer.isValid()) {
      decodedAudio_.append(buffer.constData<char>(), buffer.byteCount());
    }
  });
  connect(decoder_, &QAudioDecoder::finished, this, [this] {
    if (decodedAudio_.isEmpty()) {
      emit processingChanged(false);
      emit errorOccurred("The audio decoder returned no samples.");
      return;
    }

    durationMs_ = decodedAudio_.size() / kBytesPerFrame * 1000LL / sampleRate_;
    streamDevice_->setSource(decodedAudio_, sampleRate_, *parameters_);
    emit processingChanged(false);
    emit durationChanged(durationMs_);
    emit positionChanged(0);
    emit message(QString("Streaming DSP ready: %1 seconds of stereo PCM.")
      .arg(durationMs_ / 1000.0, 0, 'f', 1));
    emit ready(sourcePath_);
    sink_->start(streamDevice_);
  });
  connect(decoder_, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error),
          this, [this](QAudioDecoder::Error) {
    emit processingChanged(false);
    emit errorOccurred("Decode error: " + decoder_->errorString());
  });
  connect(sink_, &QAudioSink::stateChanged, this, [this](QtAudio::State state) {
    const bool playing = state == QtAudio::ActiveState;
    emit playbackStateChanged(playing);
    if (playing) {
      positionTimer_->start();
    } else if (state != QtAudio::SuspendedState) {
      positionTimer_->stop();
      updatePosition();
    }
  });
}

AudioEngine::~AudioEngine() = default;

void AudioEngine::loadFile(const QString& path)
{
  beginDecode(path);
}

void AudioEngine::beginDecode(const QString& path)
{
  decoder_->stop();
  sink_->stop();
  decodedAudio_.clear();
  sourcePath_ = path;
  durationMs_ = 0;
  emit durationChanged(0);
  emit positionChanged(0);
  emit processingChanged(true);
  emit message("Decoding " + path);
  decoder_->setSource(QUrl::fromLocalFile(path));
  decoder_->start();
}

void AudioEngine::togglePlayback()
{
  if (!hasAudio()) {
    return;
  }

  switch (sink_->state()) {
  case QtAudio::ActiveState:
    sink_->suspend();
    break;
  case QtAudio::SuspendedState:
    sink_->resume();
    break;
  default:
    if (streamDevice_->atEnd()) {
      streamDevice_->seekToFrame(0);
    }
    sink_->start(streamDevice_);
    break;
  }
}

void AudioEngine::seek(qint64 milliseconds)
{
  if (!hasAudio()) {
    return;
  }
  const bool resume = isPlaying();
  sink_->stop();
  const qint64 frame = std::clamp<qint64>(milliseconds, 0, durationMs_)
    * sampleRate_ / 1000;
  streamDevice_->seekToFrame(frame);
  emit positionChanged(position());
  if (resume) {
    sink_->start(streamDevice_);
  }
}

void AudioEngine::setVolume(int value)
{
  sink_->setVolume(std::clamp(value / 255.0, 0.0, 1.0));
}

void AudioEngine::setSpeed(int value)
{
  if (parameters_->speed != value) {
    parameters_->speed = value;
    applyParameters();
  }
}

void AudioEngine::setPitch(int semitones, int cents)
{
  if (parameters_->semitones != semitones || parameters_->cents != cents) {
    parameters_->semitones = semitones;
    parameters_->cents = cents;
    applyParameters();
  }
}

void AudioEngine::setAntiAliasEnabled(bool enabled)
{
  if (parameters_->antiAlias != enabled) {
    parameters_->antiAlias = enabled;
    applyParameters();
  }
}

void AudioEngine::setQuality(int quality)
{
  if (parameters_->quality != quality) {
    parameters_->quality = quality;
    applyParameters();
  }
}

void AudioEngine::setKaraokeEnabled(bool enabled)
{
  if (parameters_->karaoke != enabled) {
    parameters_->karaoke = enabled;
    applyParameters();
  }
}

void AudioEngine::setKaraokeSettings(int vocalPosition, int bassCutoff, int trebleRange)
{
  if (parameters_->vocalPosition != vocalPosition
      || parameters_->bassCutoff != bassCutoff
      || parameters_->trebleRange != trebleRange) {
    parameters_->vocalPosition = vocalPosition;
    parameters_->bassCutoff = bassCutoff;
    parameters_->trebleRange = trebleRange;
    applyParameters();
  }
}

void AudioEngine::applyParameters()
{
  if (hasAudio()) {
    streamDevice_->setParameters(*parameters_);
  }
}

void AudioEngine::updatePosition()
{
  emit positionChanged(position());
}

bool AudioEngine::saveProcessedWav(const QString& path, QString* errorMessage) const
{
  if (!hasAudio()) {
    if (errorMessage) {
      *errorMessage = "There is no processed audio to save.";
    }
    return false;
  }

  QByteArray output;
  const qint64 expectedBytes = decodedAudio_.size() * 1000LL
    / std::max(1, parameters_->speed);
  output.reserve(static_cast<qsizetype>(std::min<qint64>(
    expectedBytes, std::numeric_limits<int>::max())));
  DspBlockProcessor processor(decodedAudio_, sampleRate_, *parameters_, 0);
  while (!processor.atEnd()) {
    output.append(processor.processNext().audio);
  }

  if (output.size() > std::numeric_limits<quint32>::max()) {
    if (errorMessage) {
      *errorMessage = "The processed audio is too large for a standard WAV file.";
    }
    return false;
  }

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    if (errorMessage) {
      *errorMessage = file.errorString();
    }
    return false;
  }

  QDataStream stream(&file);
  writeWavHeader(stream, static_cast<quint32>(output.size()), sampleRate_);
  if (stream.writeRawData(output.constData(), output.size()) != output.size()) {
    if (errorMessage) {
      *errorMessage = file.errorString();
    }
    return false;
  }
  return true;
}

bool AudioEngine::isPlaying() const
{
  return sink_->state() == QtAudio::ActiveState;
}

bool AudioEngine::hasAudio() const
{
  return !decodedAudio_.isEmpty();
}

qint64 AudioEngine::position() const
{
  if (!hasAudio()) {
    return 0;
  }
  return streamDevice_->currentFrame() * 1000LL / sampleRate_;
}

qint64 AudioEngine::duration() const
{
  return durationMs_;
}
