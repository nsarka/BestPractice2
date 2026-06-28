//---------------------------------------------------------------------------


#pragma hdrstop

#include "TKaraokeProcessor.h"


//---------------------------------------------------------------------------

#pragma package(smart_init)

//DONE: fix samplefreq
TKaraokeProcessor::TKaraokeProcessor()
{
  HighFilter.SetSampleFreq(44100);
  HighFilter.SetOrder(8);


  BassFilter.SetSampleFreq(44100);
  BassFilter.SetOrder(8);
  HighOn = false;
  BassOn = false;
  VocalPosition = 128;
}
//---------------------------------------------------------------------------

void TKaraokeProcessor::SetBassPassThrough(int cutoff)
{
  if (cutoff) //cutoff > 0
  {
    BassFilter.SetCutOff(cutoff);
    BassOn = true;
  }
  else BassOn = false;
}
//---------------------------------------------------------------------------

void TKaraokeProcessor::SetHighPassThrough(int cutoff)
{
  if (cutoff && cutoff < 22050) //cutoff > 0
  {
    HighFilter.SetCutOff(cutoff);
    HighOn = true;
  }
  else HighOn = false;
}
//---------------------------------------------------------------------------

void TKaraokeProcessor::ProcessAudio(SampleData* sd, int nSamples)
{
  SampleData* Filtered, *FilteredHi;

  if (BassOn)
  {
   Filtered = new SampleData[nSamples];

   BassFilter.Filter((char*)sd,(char*)Filtered,nSamples);
  }

  if(HighOn)
  {
   FilteredHi = new SampleData[nSamples];

   HighFilter.Filter((char*)sd,(char*)FilteredHi,nSamples);
  }

  if(BassOn && HighOn)
  {
    for(int i = 0;i < nSamples;++i)
    {
      int kardata = (int)sd[i].Channel1*(256-VocalPosition)
        - (int)sd[i].Channel2*VocalPosition;

      kardata /= 512;

      sd[i].Channel1 = kardata + Filtered[i].Channel1/2+FilteredHi[i].Channel1/2;

      sd[i].Channel2 = kardata + Filtered[i].Channel2/2+FilteredHi[i].Channel2/2;

    }
    delete[] Filtered;
    delete[] FilteredHi;

  }
  else
    if(BassOn)
    {
      for(int i = 0;i < nSamples;++i)
      {
        int kardata = (int)sd[i].Channel1*(256-VocalPosition)
          - (int)sd[i].Channel2*VocalPosition;

        kardata /= 512;

        sd[i].Channel1 = kardata + Filtered[i].Channel1/2;

        sd[i].Channel2 = kardata + Filtered[i].Channel2/2;

      }
      delete[] Filtered;
    }
    else
      if(HighOn)
      {
        for(int i = 0;i < nSamples;++i)
        {
          int kardata = (int)sd[i].Channel1*(256-VocalPosition)
            - (int)sd[i].Channel2*VocalPosition;

          kardata /= 512;

          sd[i].Channel1 = kardata + FilteredHi[i].Channel1/2;

          sd[i].Channel2 = kardata + FilteredHi[i].Channel2/2;

        }
        delete[] FilteredHi;
      }
      else
        for(int i = 0;i < nSamples;++i)
        {
          int kardata = (int)sd[i].Channel1*(256-VocalPosition)
            - (int)sd[i].Channel2*VocalPosition;

          kardata /= 512;

          sd[i].Channel1 = kardata;

          sd[i].Channel2 = kardata;
        }
      
}
//---------------------------------------------------------------------------

void TKaraokeProcessor::SetSampleFreq(int sf)
{
  BassFilter.SetSampleFreq(sf);
  HighFilter.SetSampleFreq(sf);
};
