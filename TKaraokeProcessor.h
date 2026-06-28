//---------------------------------------------------------------------------

#ifndef TKaraokeProcessorH
#define TKaraokeProcessorH
//---------------------------------------------------------------------------
#include "IIRButter.h"
#include "SampleData.h"


class TKaraokeProcessor
{
private:

  CIIRButterLow BassFilter;
  CIIRButterHi  HighFilter;
  bool BassOn, HighOn;
  int VocalPosition;

public:
  TKaraokeProcessor() ;
  void SetBassPassThrough(int cutoff);
  void SetHighPassThrough(int cutoff);
  void SetVocalPosition(int position){VocalPosition = position;};
  void SetSampleFreq(int sf);


  void ProcessAudio(SampleData* data, int nSamples);
};


#endif
