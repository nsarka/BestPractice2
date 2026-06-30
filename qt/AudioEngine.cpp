#include "AudioEngine.h"

#include "CWSOLA.h"
#include "IIRButter.h"
#include "SampleData.h"
#include "TKaraokeProcessor.h"
#include "TReader.h"
#include "TResampler.h"

#include <QAudioBuffer>
#include <QAudioBufferInput>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QAudioSink>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QMediaRecorder>
#include <QMutex>
#include <QMutexLocker>
#include <QSaveFile>
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
constexpr int kProgressiveBufferSeconds = 2;

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

quint32 readBigEndian32(const QByteArray& data, qsizetype offset)
{
  return (quint32(static_cast<uchar>(data[offset])) << 24)
    | (quint32(static_cast<uchar>(data[offset + 1])) << 16)
    | (quint32(static_cast<uchar>(data[offset + 2])) << 8)
    | quint32(static_cast<uchar>(data[offset + 3]));
}

void writeBigEndian32(QByteArray& data, qsizetype offset, quint32 value)
{
  data[offset] = static_cast<char>(value >> 24);
  data[offset + 1] = static_cast<char>(value >> 16);
  data[offset + 2] = static_cast<char>(value >> 8);
  data[offset + 3] = static_cast<char>(value);
}

bool parseMp3FrameHeader(const QByteArray& data, qsizetype offset,
                         int* frameLength, int* sideInfoLength, int* crcLength)
{
  if (offset < 0 || offset + 4 > data.size()) {
    return false;
  }
  const quint32 header = readBigEndian32(data, offset);
  if ((header & 0xffe00000U) != 0xffe00000U) {
    return false;
  }

  const int version = (header >> 19) & 0x3;
  const int layer = (header >> 17) & 0x3;
  const int bitRateIndex = (header >> 12) & 0xf;
  const int sampleRateIndex = (header >> 10) & 0x3;
  if (version == 1 || layer != 1 || bitRateIndex == 0 || bitRateIndex == 15
      || sampleRateIndex == 3) {
    return false;
  }

  static constexpr int mpeg1BitRates[] = {
    0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
  };
  static constexpr int mpeg2BitRates[] = {
    0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160
  };
  static constexpr int sampleRates[] = {44100, 48000, 32000};
  const int bitRate = (version == 3 ? mpeg1BitRates : mpeg2BitRates)[bitRateIndex]
    * 1000;
  int sampleRate = sampleRates[sampleRateIndex];
  sampleRate /= version == 3 ? 1 : (version == 2 ? 2 : 4);
  const int padding = (header >> 9) & 0x1;
  *frameLength = (version == 3 ? 144 : 72) * bitRate / sampleRate + padding;

  const bool mono = ((header >> 6) & 0x3) == 3;
  *sideInfoLength = version == 3 ? (mono ? 17 : 32) : (mono ? 9 : 17);
  *crcLength = ((header >> 16) & 0x1) == 0 ? 2 : 0;
  return *frameLength > 4 && offset + *frameLength <= data.size();
}

// The Windows MP3 MFT can report submitted PCM buffers as the Info frame count.
// Recount MPEG frames so duration and seeking remain correct in other players.
bool normalizeMp3SeekHeader(const QString& path, QString* error)
{
  QFile input(path);
  if (!input.open(QIODevice::ReadOnly)) {
    *error = input.errorString();
    return false;
  }
  QByteArray data = input.readAll();
  input.close();

  qsizetype searchStart = 0;
  if (data.size() >= 10 && data.startsWith("ID3")) {
    const qsizetype tagSize =
      (qsizetype(static_cast<uchar>(data[6]) & 0x7f) << 21)
      | (qsizetype(static_cast<uchar>(data[7]) & 0x7f) << 14)
      | (qsizetype(static_cast<uchar>(data[8]) & 0x7f) << 7)
      | qsizetype(static_cast<uchar>(data[9]) & 0x7f);
    searchStart = 10 + tagSize;
  }

  qsizetype firstFrame = -1;
  int firstLength = 0;
  int firstSideInfo = 0;
  int firstCrc = 0;
  const qsizetype searchEnd = std::min(data.size(), searchStart + 4096);
  for (qsizetype offset = searchStart; offset + 4 <= searchEnd; ++offset) {
    if (parseMp3FrameHeader(data, offset, &firstLength, &firstSideInfo, &firstCrc)) {
      firstFrame = offset;
      break;
    }
  }
  if (firstFrame < 0) {
    *error = "The MP3 encoder produced an invalid MPEG audio stream.";
    return false;
  }

  quint32 frameCount = 0;
  qsizetype frameOffset = firstFrame;
  while (frameOffset + 4 <= data.size()) {
    int frameLength = 0;
    int sideInfo = 0;
    int crc = 0;
    if (!parseMp3FrameHeader(data, frameOffset, &frameLength, &sideInfo, &crc)) {
      break;
    }
    ++frameCount;
    frameOffset += frameLength;
  }

  const qsizetype infoOffset = firstFrame + 4 + firstCrc + firstSideInfo;
  if (infoOffset + 8 > data.size()
      || (data.mid(infoOffset, 4) != "Info" && data.mid(infoOffset, 4) != "Xing")) {
    return true;
  }
  const quint32 flags = readBigEndian32(data, infoOffset + 4);
  qsizetype fieldOffset = infoOffset + 8;
  if (flags & 0x1) {
    if (fieldOffset + 4 > data.size()) {
      *error = "The MP3 encoder produced an invalid seek header.";
      return false;
    }
    writeBigEndian32(data, fieldOffset, frameCount);
    fieldOffset += 4;
  }
  if (flags & 0x2) {
    if (fieldOffset + 4 > data.size()) {
      *error = "The MP3 encoder produced an invalid seek header.";
      return false;
    }
    writeBigEndian32(data, fieldOffset,
                     static_cast<quint32>(data.size() - firstFrame));
    fieldOffset += 4;
  }
  if ((flags & 0x4) && fieldOffset + 100 <= data.size()) {
    for (int index = 0; index < 100; ++index) {
      data[fieldOffset + index] = static_cast<char>(std::min(255, index * 256 / 100));
    }
  }

  QSaveFile output(path);
  if (!output.open(QIODevice::WriteOnly) || output.write(data) != data.size()
      || !output.commit()) {
    *error = output.errorString();
    return false;
  }
  return true;
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

struct AudioExportSession
{
  enum class Format
  {
    Wav,
    Mp3
  };

  AudioExportSession(const QString& outputPath, const QByteArray& source, int sampleRate,
                     const AudioEngineParameters& parameters, qint64 start, qint64 end,
                     Format outputFormat)
    : path(outputPath), format(outputFormat), file(outputPath),
      processor(std::make_unique<DspBlockProcessor>(source, sampleRate, parameters, start)),
      startFrame(start), endFrame(end)
  {
  }

  QString path;
  Format format;
  QFile file;
  std::unique_ptr<DspBlockProcessor> processor;
  std::unique_ptr<QMediaRecorder> recorder;
  std::unique_ptr<QAudioBufferInput> audioInput;
  std::unique_ptr<QMediaCaptureSession> captureSession;
  QAudioFormat audioFormat;
  QByteArray pendingAudio;
  qint64 startFrame = 0;
  qint64 endFrame = 0;
  qint64 pendingSourceStart = 0;
  qint64 pendingSourceEnd = 0;
  qint64 outputBytes = 0;
  qint64 outputFrames = 0;
  bool endSubmitted = false;
  bool canceled = false;
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
      const qint64 decodedFrames = decodedAudio_.size() / kBytesPerFrame;
      if (!playbackStarted_
          && decodedFrames >= sampleRate_ * kProgressiveBufferSeconds) {
        startProgressivePlayback();
      } else if (playbackStarted_ && !decodeComplete_
                 && streamingSourceFrames_ - streamDevice_->currentFrame() <= sampleRate_
                 && decodedFrames > streamingSourceFrames_) {
        const qint64 resumeFrame = streamDevice_->currentFrame();
        sink_->stop();
        streamDevice_->setSource(decodedAudio_, sampleRate_, *parameters_);
        streamDevice_->seekToFrame(resumeFrame);
        streamingSourceFrames_ = decodedFrames;
        durationMs_ = decodedFrames * 1000LL / sampleRate_;
        emit durationChanged(durationMs_);
        if (!userPaused_) {
          sink_->start(streamDevice_);
        }
      }
    }
  });
  connect(decoder_, &QAudioDecoder::finished, this, [this] {
    if (decodedAudio_.isEmpty()) {
      emit processingChanged(false);
      emit errorOccurred("The audio decoder returned no samples.");
      return;
    }

    decodeComplete_ = true;
    durationMs_ = decodedAudio_.size() / kBytesPerFrame * 1000LL / sampleRate_;
    if (!playbackStarted_) {
      startProgressivePlayback();
    } else {
      const qint64 resumeFrame = streamDevice_->currentFrame();
      sink_->stop();
      streamDevice_->setSource(decodedAudio_, sampleRate_, *parameters_);
      streamDevice_->seekToFrame(resumeFrame);
      streamingSourceFrames_ = decodedAudio_.size() / kBytesPerFrame;
      if (!userPaused_) {
        sink_->start(streamDevice_);
      }
    }
    emit durationChanged(durationMs_);
    emit decodeCompleted();
    emit message("Audio decoding complete.");
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
  cancelExport();
  decoder_->stop();
  sink_->stop();
  decodedAudio_.clear();
  sourcePath_ = path;
  durationMs_ = 0;
  streamingSourceFrames_ = 0;
  playbackStarted_ = false;
  decodeComplete_ = false;
  userPaused_ = false;
  emit durationChanged(0);
  emit positionChanged(0);
  emit processingChanged(true);
  emit message("Decoding " + path);
  decoder_->setSource(QUrl::fromLocalFile(path));
  decoder_->start();
}

void AudioEngine::togglePlayback()
{
  if (!playbackStarted_) {
    return;
  }

  switch (sink_->state()) {
  case QtAudio::ActiveState:
    sink_->suspend();
    userPaused_ = true;
    break;
  case QtAudio::SuspendedState:
    sink_->resume();
    userPaused_ = false;
    break;
  default:
    if (streamDevice_->atEnd()) {
      streamDevice_->seekToFrame(0);
    }
    userPaused_ = false;
    sink_->start(streamDevice_);
    break;
  }
}

void AudioEngine::seek(qint64 milliseconds)
{
  if (!playbackStarted_) {
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

void AudioEngine::startProgressivePlayback()
{
  if (playbackStarted_ || decodedAudio_.isEmpty()) {
    return;
  }

  playbackStarted_ = true;
  streamDevice_->setSource(decodedAudio_, sampleRate_, *parameters_);
  streamingSourceFrames_ = decodedAudio_.size() / kBytesPerFrame;
  if (durationMs_ <= 0) {
    durationMs_ = decodedAudio_.size() / kBytesPerFrame * 1000LL / sampleRate_;
  }
  emit processingChanged(false);
  emit durationChanged(durationMs_);
  emit positionChanged(0);
  emit message(QString("Playback started after buffering %1 seconds.")
    .arg(decodedAudio_.size() / double(sampleRate_ * kBytesPerFrame), 0, 'f', 1));
  emit ready(sourcePath_);
  sink_->start(streamDevice_);
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

bool AudioEngine::startExport(const QString& path, qint64 startMilliseconds,
                              qint64 endMilliseconds, QString* errorMessage,
                              int mp3BitRate)
{
  if (!decodeComplete_) {
    if (errorMessage) {
      *errorMessage = "Please wait for audio decoding to complete before exporting.";
    }
    return false;
  }
  if (exportSession_) {
    if (errorMessage) {
      *errorMessage = "An export is already in progress.";
    }
    return false;
  }

  const qint64 totalFrames = decodedAudio_.size() / kBytesPerFrame;
  const qint64 startFrame = std::clamp<qint64>(
    startMilliseconds * sampleRate_ / 1000, 0, totalFrames);
  const qint64 requestedEnd = endMilliseconds > 0
    ? endMilliseconds * sampleRate_ / 1000
    : totalFrames;
  const qint64 endFrame = std::clamp<qint64>(requestedEnd, 0, totalFrames);
  if (startFrame >= endFrame) {
    if (errorMessage) {
      *errorMessage = "The export end must be after its start.";
    }
    return false;
  }

  const QString suffix = QFileInfo(path).suffix().toLower();
  if (suffix != "wav" && suffix != "mp3") {
    if (errorMessage) {
      *errorMessage = "Export files must use the .wav or .mp3 extension.";
    }
    return false;
  }
  const bool mp3 = suffix == "mp3";
  const AudioExportSession::Format outputFormat = mp3
    ? AudioExportSession::Format::Mp3
    : AudioExportSession::Format::Wav;
  exportSession_ = std::make_unique<AudioExportSession>(
    path, decodedAudio_, sampleRate_, *parameters_, startFrame, endFrame, outputFormat);

  if (!mp3) {
    if (!exportSession_->file.open(QIODevice::WriteOnly)) {
      if (errorMessage) {
        *errorMessage = exportSession_->file.errorString();
      }
      exportSession_.reset();
      return false;
    }

    QDataStream stream(&exportSession_->file);
    writeWavHeader(stream, 0, sampleRate_);
    emit exportProgress(0);
    QTimer::singleShot(0, this, &AudioEngine::processExportBlock);
    return true;
  }

  QMediaFormat mediaFormat(QMediaFormat::MP3);
  mediaFormat.setAudioCodec(QMediaFormat::AudioCodec::MP3);
  if (!mediaFormat.isSupported(QMediaFormat::Encode)) {
    if (errorMessage) {
      *errorMessage = "MP3 encoding is not supported by the active multimedia backend.";
    }
    exportSession_.reset();
    return false;
  }

  exportSession_->audioFormat.setSampleRate(sampleRate_);
  exportSession_->audioFormat.setChannelCount(2);
  exportSession_->audioFormat.setSampleFormat(QAudioFormat::Int16);
  exportSession_->recorder = std::make_unique<QMediaRecorder>();
  exportSession_->audioInput = std::make_unique<QAudioBufferInput>(
    exportSession_->audioFormat);
  exportSession_->captureSession = std::make_unique<QMediaCaptureSession>();
  exportSession_->captureSession->setAudioBufferInput(exportSession_->audioInput.get());
  exportSession_->captureSession->setRecorder(exportSession_->recorder.get());
  exportSession_->recorder->setMediaFormat(mediaFormat);
  exportSession_->recorder->setEncodingMode(QMediaRecorder::ConstantBitRateEncoding);
  exportSession_->recorder->setAudioBitRate(
    std::clamp(mp3BitRate, 128000, 320000));
  exportSession_->recorder->setAudioSampleRate(sampleRate_);
  exportSession_->recorder->setAudioChannelCount(2);
  exportSession_->recorder->setAutoStop(true);
  exportSession_->recorder->setOutputLocation(QUrl::fromLocalFile(path));

  auto* recorder = exportSession_->recorder.get();
  connect(exportSession_->audioInput.get(), &QAudioBufferInput::readyToSendAudioBuffer,
          this, &AudioEngine::processExportBlock);
  connect(recorder, &QMediaRecorder::errorOccurred, this,
          [this, recorder](QMediaRecorder::Error, const QString& error) {
    QTimer::singleShot(0, this, [this, recorder, error] {
      if (exportSession_ && exportSession_->recorder.get() == recorder) {
        failExport(error.isEmpty() ? "MP3 encoding failed." : error);
      }
    });
  });
  connect(recorder, &QMediaRecorder::recorderStateChanged, this,
          [this, recorder](QMediaRecorder::RecorderState state) {
    if (state == QMediaRecorder::StoppedState) {
      QTimer::singleShot(0, this, [this, recorder] {
        if (exportSession_ && exportSession_->recorder.get() == recorder
            && (exportSession_->endSubmitted || exportSession_->canceled)) {
          completeMp3Export();
        } else if (exportSession_ && exportSession_->recorder.get() == recorder) {
          failExport("The MP3 encoder stopped before export completed.");
        }
      });
    }
  });

  QFile::remove(path);
  emit exportProgress(0);
  recorder->record();
  if (recorder->error() != QMediaRecorder::NoError) {
    if (errorMessage) {
      *errorMessage = recorder->errorString();
    }
    exportSession_.reset();
    QFile::remove(path);
    return false;
  }
  return true;
}

void AudioEngine::cancelExport()
{
  if (exportSession_) {
    exportSession_->canceled = true;
    QTimer::singleShot(0, this, &AudioEngine::processExportBlock);
  }
}

void AudioEngine::processExportBlock()
{
  if (!exportSession_) {
    return;
  }
  if (exportSession_->canceled) {
    if (exportSession_->format == AudioExportSession::Format::Mp3) {
      if (exportSession_->recorder->recorderState() != QMediaRecorder::StoppedState) {
        exportSession_->recorder->stop();
      } else {
        completeMp3Export();
      }
      return;
    }
    const QString path = exportSession_->path;
    exportSession_->file.close();
    exportSession_.reset();
    QFile::remove(path);
    emit exportCanceled();
    return;
  }

  ProcessedBlock block;
  if (!exportSession_->pendingAudio.isEmpty()) {
    block.audio = exportSession_->pendingAudio;
    block.sourceStart = exportSession_->pendingSourceStart;
    block.sourceEnd = exportSession_->pendingSourceEnd;
  } else {
    block = exportSession_->processor->processNext();
  }
  if (block.sourceStart >= exportSession_->endFrame || block.audio.isEmpty()) {
    if (exportSession_->format == AudioExportSession::Format::Mp3) {
      if (exportSession_->audioInput->sendAudioBuffer(QAudioBuffer())) {
        exportSession_->endSubmitted = true;
      }
      return;
    }
    finishExport();
    return;
  }
  if (block.sourceEnd > exportSession_->endFrame) {
    const qint64 sourceSpan = block.sourceEnd - block.sourceStart;
    const qint64 includedSpan = exportSession_->endFrame - block.sourceStart;
    block.audio.truncate(static_cast<qsizetype>(
      block.audio.size() * includedSpan / std::max<qint64>(1, sourceSpan)));
    block.sourceEnd = exportSession_->endFrame;
  }

  if (exportSession_->format == AudioExportSession::Format::Mp3) {
    const qint64 startTime = exportSession_->outputFrames * 1000000LL / sampleRate_;
    if (!exportSession_->audioInput->sendAudioBuffer(
          QAudioBuffer(block.audio, exportSession_->audioFormat, startTime))) {
      exportSession_->pendingAudio = block.audio;
      exportSession_->pendingSourceStart = block.sourceStart;
      exportSession_->pendingSourceEnd = block.sourceEnd;
      return;
    }
    exportSession_->pendingAudio.clear();
    exportSession_->outputFrames += block.audio.size() / kBytesPerFrame;
  } else if (exportSession_->file.write(block.audio) != block.audio.size()) {
    const QString error = exportSession_->file.errorString();
    failExport(error);
    return;
  }
  exportSession_->outputBytes += block.audio.size();
  if (exportSession_->format == AudioExportSession::Format::Wav
      && exportSession_->outputBytes > std::numeric_limits<quint32>::max()) {
    failExport("The processed audio is too large for a standard WAV file.");
    return;
  }

  const qint64 completed = block.sourceEnd - exportSession_->startFrame;
  const qint64 total = exportSession_->endFrame - exportSession_->startFrame;
  emit exportProgress(static_cast<int>(completed * 100 / std::max<qint64>(1, total)));
  if (block.sourceEnd >= exportSession_->endFrame) {
    if (exportSession_->format == AudioExportSession::Format::Mp3) {
      if (exportSession_->audioInput->sendAudioBuffer(QAudioBuffer())) {
        exportSession_->endSubmitted = true;
      }
    } else {
      finishExport();
    }
  } else {
    QTimer::singleShot(0, this, &AudioEngine::processExportBlock);
  }
}

void AudioEngine::finishExport()
{
  if (!exportSession_) {
    return;
  }
  const QString path = exportSession_->path;
  exportSession_->file.seek(0);
  QDataStream stream(&exportSession_->file);
  writeWavHeader(stream, static_cast<quint32>(exportSession_->outputBytes), sampleRate_);
  exportSession_->file.close();
  exportSession_.reset();
  emit exportProgress(100);
  emit exportFinished(path);
}

void AudioEngine::completeMp3Export()
{
  if (!exportSession_ || exportSession_->format != AudioExportSession::Format::Mp3) {
    return;
  }
  const QString path = exportSession_->path;
  const bool canceled = exportSession_->canceled;
  exportSession_.reset();
  if (canceled) {
    QFile::remove(path);
    emit exportCanceled();
    return;
  }
  if (!QFileInfo::exists(path) || QFileInfo(path).size() == 0) {
    QFile::remove(path);
    emit exportFailed("The MP3 encoder did not produce an output file.");
    return;
  }
  QString normalizationError;
  if (!normalizeMp3SeekHeader(path, &normalizationError)) {
    QFile::remove(path);
    emit exportFailed(normalizationError);
    return;
  }
  emit exportProgress(100);
  emit exportFinished(path);
}

void AudioEngine::failExport(const QString& error)
{
  if (!exportSession_) {
    return;
  }
  const QString path = exportSession_->path;
  exportSession_->file.close();
  if (exportSession_->recorder
      && exportSession_->recorder->recorderState() != QMediaRecorder::StoppedState) {
    exportSession_->recorder->stop();
  }
  exportSession_.reset();
  QFile::remove(path);
  emit exportFailed(error);
}

bool AudioEngine::isPlaying() const
{
  return sink_->state() == QtAudio::ActiveState;
}

bool AudioEngine::hasAudio() const
{
  return !decodedAudio_.isEmpty();
}

bool AudioEngine::isDecodeComplete() const
{
  return decodeComplete_;
}

bool AudioEngine::isExporting() const
{
  return exportSession_ != nullptr;
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
