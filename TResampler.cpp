//---------------------------------------------------------------------------



#include "TResampler.h"
#include <math.h>
//---------------------------------------------------------------------------


TResampler::TResampler()
{
    LastData.Channel1 = LastData.Channel2 = 0;
    startpos = 0;
    for (int i = 0;i<2;++i)
      ldata[i].Channel1 = ldata[i].Channel2 = 0;

    mode = rmLinear;
};


int TResampler::Resample(char* DestBuffer, const char* readbuffer,
                         float pitchscale , unsigned int cbBuffer)
{

      return ResampleLinear(DestBuffer, readbuffer,
                         pitchscale , cbBuffer);

       //return ResampleHermite(DestBuffer, readbuffer,
//                         pitchscale , cbBuffer);
}
//---------------------------------------------------------------------------

int TResampler::ResampleLinear(char* DestBuffer, const char* readbuffer,
                         float pitchscale , unsigned int cbBuffer)
{
       //Assumes length(readbuffer) == cbBuffer!

       SampleData* sampledata = (SampleData*)readbuffer;
       SampleData * destdata  = (SampleData *) DestBuffer;

       float step = 1/pitchscale,pos = startpos + step;


       destdata[0].Channel1 =
             ((float)sampledata[0].Channel1+ startpos*((float)sampledata[0].Channel1
                - (float)LastData.Channel1));

           destdata[0].Channel2 =
             ((float)sampledata[0].Channel2+ startpos*((float)sampledata[0].Channel2
                - (float)LastData.Channel2));

       int destbufidx = 1;
       int prev = (int)pos;
       while (prev < (cbBuffer/4 - 1))
       {


         float dx = pos - prev;

           destdata[destbufidx].Channel1 =
             ((float)sampledata[prev].Channel1+ dx*((float)sampledata[prev+1].Channel1
                - (float)sampledata[prev].Channel1));

           destdata[destbufidx].Channel2 =
             ((float)sampledata[prev].Channel2+ dx*((float)sampledata[prev+1].Channel2
                - (float)sampledata[prev].Channel2));


         destbufidx++;
         pos += step;
         prev = (int)pos;
       }

       startpos = pos - floor(pos);
       LastData = sampledata[cbBuffer/4 - 1];

       return destbufidx;
}//---------------------------------------------------------------------------

int TResampler::ResampleHermite(char* DestBuffer, const char* readbuffer,
                         float pitchscale , unsigned int cbBuffer)
{
  SampleData* sampledata = (SampleData*)readbuffer;
       SampleData * destdata  = (SampleData *) DestBuffer;

       float step = 1/pitchscale,pos = startpos;


       int destbufidx = 0;

       while(pos < 1 && pos < cbBuffer<<2)
       {
         destdata[destbufidx].Channel1 = hermite2(startpos,ldata[0].Channel1,ldata[1].Channel1,
                                       sampledata[0].Channel1,sampledata[1].Channel1);

         destdata[destbufidx].Channel2 = hermite2(startpos,ldata[0].Channel2,ldata[1].Channel2,
                                       sampledata[0].Channel2,sampledata[1].Channel2);
         ++destbufidx;
         pos += step;
       }

       while(pos < 2 && pos < cbBuffer<<2)
       {
         destdata[destbufidx].Channel1 = hermite2(startpos,ldata[1].Channel1, sampledata[0].Channel1,
                                       sampledata[1].Channel1,sampledata[2].Channel1);

         destdata[destbufidx].Channel2 = hermite2(startpos,ldata[1].Channel2, sampledata[0].Channel2,
                                       sampledata[1].Channel2,sampledata[2].Channel2);
         ++destbufidx;
         pos += step;
       }

       int prev = (int)pos; //At this integer position in the buffer
       while (prev < (cbBuffer/4 - 2))
       {
         float dx = pos - prev;

         destdata[destbufidx].Channel1 = hermite2(dx,
                                        sampledata[prev - 2].Channel1,
                                        sampledata[prev - 1].Channel1,
                                        sampledata[prev].Channel1,sampledata[prev + 1].Channel1);

         destdata[destbufidx].Channel2 = hermite2(dx,
                                        sampledata[prev - 2].Channel2,
                                        sampledata[prev - 1].Channel2,
                                        sampledata[prev].Channel2,sampledata[prev + 1].Channel2);
         ++destbufidx;
         pos += step;
         prev = (int)pos;



       }

       startpos = pos - floor(pos);
       ldata[0] = sampledata[cbBuffer/4 - 1];
       ldata[1] = sampledata[cbBuffer/4 - 2];

       return destbufidx;
}

// original
inline float hermite1(float x, float y0, float y1, float y2, float y3)
{
    // 4-point, 3rd-order Hermite (x-form)
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
    float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);

    return ((c3 * x + c2) * x + c1) * x + c0;
}

// james mccartney
inline float hermite3(float x, float y0, float y1, float y2, float y3)
{
    // 4-point, 3rd-order Hermite (x-form)
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);
    float c2 = y0 - y1 + c1 - c3;

    return ((c3 * x + c2) * x + c1) * x + c0;
}

// james mccartney
inline float TResampler::hermite2(float x, float y0, float y1, float y2, float y3)
{
        //float  p = sin(M_PI*(1-x))/(M_PI*(1-x))*y2;
//         p += sin(M_PI*(x+1))/(M_PI*(x+1))*y0;
//         p += sin(M_PI*(2-x))/(M_PI*(2-x))*y3;
//
//          if(x>1e-5) p += sin(M_PI*x)/(M_PI*x)*y1;
//          else p += y1;
//
//
//         return p*.9;

        // 4-point, 3rd-order Lagrange (x-form)
        y0 /= 32767;
        y1 /= 32767;
        y2 /= 32767;
        y3 /= 32767;
// 4-point, 3rd-order Hermite (x-form)
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);
    float c2 = y0 - y1 + c1 - c3;

    return 30000*(((c3 * x + c2) * x + c1) * x + c0);
//float c0 = y1;
//float c1 = y2 - 1/3.0*y0 - 1/2.0*y1 - 1/6.0*y3;
//float c2 = 1/2.0*(y0+y2) - y1;
//float c3 = 1/6.0*(y3-y0) + 1/2.0*(y1-y2);
//return 32767*((c3*x+c2)*x+c1)*x+c0;
}

// laurent de soras
inline float hermite4(float frac_pos, float xm1, float x0, float x1, float x2)
{
   const float    c     = (x1 - xm1) * 0.5f;
   const float    v     = x0 - x1;
   const float    w     = c + v;
   const float    a     = w + v + (x2 - x0) * 0.5f;
   const float    b_neg = w + a;

   return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
}
//---------------------------------------------------------------------------
