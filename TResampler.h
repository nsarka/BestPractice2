//---------------------------------------------------------------------------

#ifndef TResamplerH
#define TResamplerH

#include "SampleData.h"

enum TResampleMode {rmLinear, rmHermite};

class TResampler
{
  SampleData LastData;
  SampleData ldata[2];
  float startpos;
  TResampleMode mode;

  int ResampleLinear(char* DestBuffer, const char* readbuffer,
                float pitchscale, unsigned int cbBuffer);
  int ResampleHermite(char* DestBuffer, const char* readbuffer,
                float pitchscale, unsigned int cbBuffer);
  inline float hermite2(float x, float y0, float y1, float y2, float y3);                

public:
  TResampler();
  void SetResampleMode( TResampleMode amode){mode = amode;};

  int Resample(char* DestBuffer, const char* readbuffer,
                float pitchscale, unsigned int cbBuffer);

};

#endif
