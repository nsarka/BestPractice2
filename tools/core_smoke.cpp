#include "CWSOLA.h"
#include "IIRButter.h"
#include "TKaraokeProcessor.h"
#include "TReader.h"
#include "TResampler.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace
{
class SilenceReader final : public AudioReader::TReader
{
public:
  __int64 Seek(__int64 sampleposition) override
  {
    position_ = std::max<__int64>(0, sampleposition);
    return position_;
  }

  int FillBuffer(Buffer& buffertofill, int numsamplesrequested) override
  {
    const int bytes_requested = numsamplesrequested * 4;
    buffertofill.data.insert(buffertofill.data.end(), bytes_requested, 0);
    position_ += numsamplesrequested;
    return numsamplesrequested;
  }

  __int64 GetLength() override
  {
    return sample_rate_ * 2;
  }

  TReader* MakeCopy() override
  {
    return new SilenceReader(*this);
  }

  int GetSampleFrequency() override
  {
    return sample_rate_;
  }

private:
  static constexpr int sample_rate_ = 44100;
  __int64 position_ = 0;
};
}

int main()
{
  constexpr unsigned int bytes_per_block = 4096;
  std::vector<char> input(bytes_per_block, 0);
  std::vector<char> output(bytes_per_block * 2, 0);

  SilenceReader reader;
  CWOLA stretcher(1024, bytes_per_block, 256, &reader);
  stretcher.ProcessBlock(1000.0f, output.data());

  TResampler resampler;
  resampler.Resample(output.data(), input.data(), 1.0f, bytes_per_block);

  CIIRButterLow lowpass(44100, 4);
  lowpass.SetCutOff(8000);
  lowpass.Filter(input.data(), output.data(), static_cast<int>(bytes_per_block / 4));

  TKaraokeProcessor karaoke;
  karaoke.SetSampleFreq(44100);
  karaoke.ProcessAudio(reinterpret_cast<SampleData*>(output.data()),
                       static_cast<int>(bytes_per_block / 4));

  return 0;
}
