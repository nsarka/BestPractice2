//---------------------------------------------------------------------------


#pragma hdrstop

#include "IIRButter.h"

#include <cmath>
#pragma package(smart_init)
//---------------------------------------------------------------------------

namespace
{
constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kPiOver2 = kPi / 2.0;
}

void CIIRButter::GetAnalogCoeffs(double& A, double& b, double& c, int section_no)
{
  double p_r = 2*kPi*fc*cos(kPiOver2 + kPiOver2/order*(1+2*(section_no-1)));
  //double p_i = 2*kPi*fc*sin(kPiOver2 + kPiOver2/order*(1+2*(section_no-1)));


//          A                  A                     A
//    -------------   =  -------------- = ------------------------
//    s^2 + bs + c       (s+p) (s+p*)      s^2 + 2 re(p) s + |p|^2

//c = p x p* = p_r^2 + p_i^2  = A
//b = 2 x re(p) = 2*p_r

  c = 2*kPi*fc*2*kPi*fc;
  A = c;

  b = -2*p_r;
}

//---------------------------------------------------------------------------

void CIIRButter::SetSampleFreq(double fsample)
{
  if (fabs(fsample - fs) > .1)
  {
    fs = fsample;

    CreateFilter();
  }
}
//---------------------------------------------------------------------------

void CIIRButter::SetCutOff(double c_off)
{
  if(fabs(c_off - cutoff) > .1 && c_off < fs/2)
  {
    cutoff = c_off;
    CreateFilter();
  }
}
//---------------------------------------------------------------------------

void CIIRButter::SetOrder(int new_order)
{
  if(new_order != order)
  {
    order = new_order;
    CreateFilter();
  }
}
//---------------------------------------------------------------------------

void CIIRButter::CreateFilter()
{
   //


}
//---------------------------------------------------------------------------

void CIIRButter::Filter(char* in, char* out, int numsamples)
{

  int maxsection = order>>1;


  if(maxsection > 1)  //Higher order sections available
  {
    float* buffer1 = new float[numsamples*2], *buffer2 = new float[numsamples*2];


    sos[0].Filter(in, buffer1, numsamples);

    for (int section = 1; section < maxsection - 2;++section)
    {
      sos[section].Filter(buffer1,buffer2,numsamples);
      //Output is now in buffer2
      //swap again

      float *temp = buffer1;
      buffer1 = buffer2;
      buffer2 = temp;

    }

    //Now do last section

    sos[maxsection-1].Filter(buffer1,out,numsamples);

    delete[] buffer1;
    delete[] buffer2;
  }
  else
  {
    sos[0].Filter(in,out,numsamples);
  }
}
//---------------------------------------------------------------------------

void CIIRButterLow::CreateFilter()
{
  if(order < 2) order = 2;
   if(order& 0x01) order++;

   if(order > 20) order = 20;

   int sections = order>>1;

   //w_a = 2/T tan(w_d T/2)
   //Find cutoff frequency for the analog template filter by prewarping
   fc = fs*tan(kPi*cutoff/fs)/kPi;

   //fc = cutoff_a / pow(10000,1.0/order);


   for(int section = 1; section <= sections;++section)
   {
     double A, b, c;
     GetAnalogCoeffs(A, b, c, section);
     A *= .99;
     sos_coeffs cf;

     cf.b0 = A;
     cf.b1 = 2*A;
     cf.b2 = A;
     cf.a0 = (4*fs*fs + 2*fs*b + c);
     cf.a1 = (2*c - 8*fs*fs);
     cf.a2 = (4*fs*fs-2*fs*b+c);

     sos[section-1].SetCoefficients(cf);

   }
}
//---------------------------------------------------------------------------

void CIIRButterHi::CreateFilter()
{
  if(order < 2) order = 2;
   if(order& 0x01) order++;

   if(order > 20) order = 20;

   int sections = order>>1;
   if(cutoff == 0) cutoff = fs/4;
   //w_a = 2/T tan(w_d T/2)
   double cutoff_a = fs*tan(kPi*(fs/2 - cutoff)/fs)/kPi;

   fc = cutoff_a;// / pow(1000,1.0/order);


   for(int section = 1; section <= sections;++section)
   {
     double A, b, c;
     GetAnalogCoeffs(A, b, c, section);
     A *= .99;
     sos_coeffs cf;

     cf.b0 = A;
     cf.b1 = -2*A;
     cf.b2 = A;
     cf.a0 = (4*fs*fs + 2*fs*b + c);
     cf.a1 = -(2*c - 8*fs*fs);
     cf.a2 = (4*fs*fs-2*fs*b+c);

     sos[section-1].SetCoefficients(cf);

   }
}
