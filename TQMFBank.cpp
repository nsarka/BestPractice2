//---------------------------------------------------------------------------


#pragma hdrstop

#include "TQMFBank.h"
#include <cstring>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace QMFBank
{
#define fir j32d

float j32d[]={ 0.002346222138340,
       -0.003748483183120 ,
       -0.002060296355165 ,
        0.008053852109207 ,
        0.000877952219988 ,
       -0.014118331864855 ,
        0.002014489071969 ,
        0.022662590572985 ,
       -0.007923790259711 ,
       -0.034934726067175 ,
        0.019393569512341 ,
        0.054853630448975 ,
       -0.044441414672869 ,
       -0.099381318377587 ,
        0.132820798102042,
        0.463783133920886,
        0.463783133920886,
        0.132820798102042,
       -0.099381318377587 ,
       -0.044441414672869 ,
        0.054853630448975 ,
        0.019393569512341 ,
       -0.034934726067175 ,
       -0.007923790259711 ,
        0.022662590572985 ,
        0.002014489071969 ,
       -0.014118331864855 ,
        0.000877952219988 ,
        0.008053852109207 ,
       -0.002060296355165 ,
       -0.003748483183120 ,
        0.002346222138340 };

float j32dnormalizer[] ={  .71572692180904277033520472905276e-7,
 -.11434937409659085341351666834445e-6,
 -.62850381649708247149837430679808e-7,
  .24568682924910059423790676703947e-6,
  .26782376214044857131876088219072e-7,
 -.43068684936459570247480619156755e-6,
  .61453007323443276403032524933534e-7,
  .69133377978000519938214189080705e-6,
 -.24171922679307628148651239906319e-6,
 -.10657014757346009412063755211753e-5,
  .59161064006404175949694517269717e-6,
  .16733377215114451296392632587140e-5,
 -.13557078166160561415973543827884e-5,
 -.30316773473990251621822036159903e-5,
  .40517655777067294241245917763017e-5,
  .14147938910123939983349075633079e-4,
  .14147938910123939983349075633079e-4,
  .40517655777067294241245917763017e-5,
 -.30316773473990251621822036159903e-5,
 -.13557078166160561415973543827884e-5,
  .16733377215114451296392632587140e-5,
  .59161064006404175949694517269717e-6,
 -.10657014757346009412063755211753e-5,
 -.24171922679307628148651239906319e-6,
  .69133377978000519938214189080705e-6,
  .61453007323443276403032524933534e-7,
 -.43068684936459570247480619156755e-6,
  .26782376214044857131876088219072e-7,
  .24568682924910059423790676703947e-6,
 -.62850381649708247149837430679808e-7,
 -.11434937409659085341351666834445e-6,
  .71572692180904277033520472905276e-7};



//---------------------------------------------------------------------------

TQMFBank::TQMFBank()
{
  memset((void*)&sd,0,sizeof(SampleData)*FILTER_LENGTH);
  memset((void*)&sdsynlo,0,sizeof(SampleData)*FILTER_LENGTH);
  memset((void*)&sdsynhi,0,sizeof(SampleData)*FILTER_LENGTH);
}
//---------------------------------------------------------------------------
int TQMFBank::FirFilt(SampleData* inp, unsigned int length, SampleData* out)
{
  float l = 0,r = 0;
  unsigned int outidx = 0, index = 0;
  //TODO: Bit less blunt method here??

  SampleData* input = new SampleData[length+FILTER_LENGTH - 1];
  memcpy((void*)input,(void*)sd,(FILTER_LENGTH - 1)*sizeof(SampleData));
  memcpy((void*)(input+FILTER_LENGTH - 1),(void*)inp,length*sizeof(SampleData));
#if FILTER_LENGTH < 4
#error Hey!
#endif
  while(index < length)// + FILTER_LENGTH - 1)
  {
    register int idx = index;
    for(register int i = 0;i < FILTER_LENGTH;i+=4,idx+=4)
    {
      l += fir[i]*(float)input[idx].Channel1;
      l += fir[++i]*(float)input[++idx].Channel1;
      l += fir[++i]*(float)input[++idx].Channel1;
      l += fir[++i]*(float)input[++idx].Channel1;

      r += fir[i]*(float)input[idx].Channel2;
      r += fir[++i]*(float)input[++idx].Channel2;
      r += fir[++i]*(float)input[++idx].Channel2;
      r += fir[++i]*(float)input[++idx].Channel2;

    }

    out[outidx].Channel1 = (short)(l*.95);
    out[outidx].Channel2 = (short)(r*.95);
    l=r=0;
    outidx++;
    index++;

  }
  //DONE: save last piece of data for next round
  memcpy((void*)&sd,(void*)(inp+length-FILTER_LENGTH - 2),(FILTER_LENGTH - 1)*sizeof(SampleData));
  delete[] input;

  return outidx;
}
//---------------------------------------------------------------------------

int TQMFBank::Analyze(SampleData* inp, unsigned int length, SampleData* out_lo,
         SampleData* out_hi)
{
  float p1l = 0,p2l = 0,p1r = 0, p2r = 0;
  unsigned int outidx = 0, index = 0;
  //TODO: Bit less blunt method here??

  SampleData* input = new SampleData[length+FILTER_LENGTH - 1];
  memcpy((void*)input,(void*)sd,(FILTER_LENGTH - 1)*sizeof(SampleData));
  memcpy((void*)(input+FILTER_LENGTH - 1),(void*)inp,length*sizeof(SampleData));


  while(index < length )//- FILTER_LENGTH)
  {
    int idx = index;
    for(int i = 0;i < FILTER_LENGTH;i+=2,idx+=2)
    {
      p1l += fir[i]*input[idx].Channel1;
      p2l += fir[i+1]*input[idx + 1].Channel1;

      p1r += fir[i]*input[idx].Channel2;
      p2r += fir[i+1]*input[idx + 1].Channel2;
    }

    out_lo[outidx].Channel1 = (p1l + p2l)*.90;
    out_hi[outidx].Channel1 = (p1l - p2l)*.90;

    out_lo[outidx].Channel2 = (p1r + p2r)*.90;
    out_hi[outidx].Channel2 = (p1r - p2r)*.90;

    outidx++;
    index += 2;
    p1l = 0;p2l = 0;p1r = 0; p2r = 0;

  }
  //DONE: save last piece of data for next round
  memcpy((void*)&sd,(void*)(inp+length-FILTER_LENGTH - 2),(FILTER_LENGTH - 1)*sizeof(SampleData));
  delete[] input;

  return outidx;
}
//---------------------------------------------------------------------------
int TQMFBank::Analyze(std::vector<SampleData>& inp, std::vector<SampleDataFl>& out_lo,
         std::vector<SampleDataFl>& out_hi)
{
  float p1l = 0,p2l = 0,p1r = 0, p2r = 0;
  unsigned int outidx = 0, index = 0;
  unsigned int length = inp.size();
  SampleDataFl data;

  out_lo.clear();
  out_hi.clear();
  out_lo.reserve(length>>1+2);
  out_hi.reserve(length>>1+2);



  //TODO: Bit less blunt method here??
  SampleData* input = new SampleData[length+FILTER_LENGTH - 1];
  memcpy((void*)input,(void*)sd,(FILTER_LENGTH - 1)*sizeof(SampleData));
  memcpy((void*)(input+FILTER_LENGTH - 1), inp.data(), length*sizeof(SampleData));


  while(index < length )//- FILTER_LENGTH)
  {
    int idx = index;
    for(int i = 0;i < FILTER_LENGTH;i+=2,idx+=2)
    {
      p1l += fir[i]*input[idx].Channel1;
      p2l += fir[i+1]*input[idx + 1].Channel1;

      p1r += fir[i]*input[idx].Channel2;
      p2r += fir[i+1]*input[idx + 1].Channel2;
    }

    data.Channel1 = (p1l + p2l);
    data.Channel2 = (p1r + p2r);

    out_lo.push_back(data);

    data.Channel1 = (p1l - p2l);
    data.Channel2 = (p1r - p2r);

    out_hi.push_back(data);


    outidx++;
    index += 2;
    p1l = 0;p2l = 0;p1r = 0; p2r = 0;

  }
  //DONE: save last piece of data for next round
  memcpy((void*)&sd, inp.data()+length-FILTER_LENGTH - 2, (FILTER_LENGTH - 1)*sizeof(SampleData));
  delete[] input;

  return outidx;
}
//---------------------------------------------------------------------------

int TQMFBank::Synthesize(SampleDataFl* in_low, SampleDataFl* in_high, unsigned int length,
         SampleData* out)
{
  float p1l = 0,p2l = 0,p1r = 0, p2r = 0;
  unsigned int outidx = 0, index = 0;
  //DONE: process last piece of previous data first
  SampleDataFl* in_lo = new SampleDataFl[length+FILTER_LENGTH-1];
  SampleDataFl* in_hi = new SampleDataFl[length+FILTER_LENGTH-1];

  memcpy((void*)in_lo,(void*)sdsynlo,(FILTER_LENGTH - 1)*sizeof(SampleDataFl));
  memcpy((void*)(in_lo+FILTER_LENGTH-1),(void*)in_low,length*sizeof(SampleDataFl));

  memcpy((void*)in_hi,(void*)sdsynhi,(FILTER_LENGTH - 1)*sizeof(SampleDataFl));
  memcpy((void*)(in_hi+FILTER_LENGTH-1),(void*)in_high,length*sizeof(SampleDataFl));


  while(index < length)// - FILTER_LENGTH)
  {
    int idx = index;
    for(int i = 0;i < FILTER_LENGTH;i += 2,idx++) 
    {
      p1l += fir[i]*(in_lo[idx].Channel1-in_hi[idx].Channel1);
      p2l += fir[i+1]*(in_lo[idx+1].Channel1+in_hi[idx+1].Channel1);

      p1r += fir[i]*(in_lo[idx].Channel2-in_hi[idx].Channel2);
      p2r += fir[i+1]*(in_lo[idx+1].Channel2+in_hi[idx+1].Channel2);
    }
    out[outidx].Channel1 = 1.8*(p1l);
    out[outidx].Channel2 = 1.8*(p1r);

    outidx++;

    out[outidx].Channel1 = 1.8*(p2l);
    out[outidx].Channel2 = 1.8*(p2r);

    outidx++;

    index ++;
     p1l = 0;p2l = 0;p1r = 0; p2r = 0;

  }


  //DONE: save last piece of data for next round
  memcpy((void*)&sdsynlo,(void*)(in_low+length-FILTER_LENGTH-2),(FILTER_LENGTH - 1)*sizeof(SampleDataFl));
  memcpy((void*)&sdsynhi,(void*)(in_hi+length-FILTER_LENGTH-2),(FILTER_LENGTH - 1)*sizeof(SampleDataFl));

  return outidx;
}
//---------------------------------------------------------------------------



} //namespace
