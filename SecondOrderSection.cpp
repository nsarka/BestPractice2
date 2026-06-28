//---------------------------------------------------------------------------



#include "SecondOrderSection.h"
//---------------------------------------------------------------------------


#define N_1 0
#define N_2 1

//short ints in, short ints out.
//One sample is two channel x 16 bit audio == 32 bits
void CSOS::Filter(char* in, char* out, int numsamples)
{
  short int* indata = (short int*)in, *outdata = (short int*)out;
  float temp;

  temp = cf.b0*indata[0] + cf.b1*xnl[N_1] + cf.b2*xnl[N_2]
                        - cf.a1*ynl[N_1] - cf.a2*ynl[N_2];

  outdata[0] = temp;

  if(temp > 32767)
  {
    outdata[0] = 32767;
    //OutputDebugString("DFG?");
  }
  if(temp < -32768)
    outdata[0] = -32768;

  temp = cf.b0*indata[1] + cf.b1*xnr[N_1] + cf.b2*xnr[N_2]
                        - cf.a1*ynr[N_1] - cf.a2*ynr[N_2];

  outdata[1] = temp;

  if(temp > 32767) outdata[1] = 32767;
  if(temp < -32768) outdata[1] = -32768;

  temp = cf.b0*indata[2] + cf.b1*indata[0] + cf.b2*xnl[N_1]
                        - cf.a1*outdata[0] - cf.a2*ynl[N_1];

  outdata[2] = temp;

  if(temp > 32767) outdata[2] = 32767;
  if(temp < -32768) outdata[2] = -32768;

  temp = cf.b0*indata[3] + cf.b1*indata[1] + cf.b2*xnr[N_1]
                        - cf.a1*outdata[1] - cf.a2*ynr[N_1];

  outdata[3] = temp;

  if(temp > 32767) outdata[3] = 32767;
  if(temp < -32768) outdata[3] = -32768;

  for(int i = 4; i< numsamples*2; ++i)
  {
     //left channel
     temp = cf.b0*indata[i] + cf.b1*indata[i-2] + cf.b2*indata[i-4]
                        - cf.a1*outdata[i-2] - cf.a2*outdata[i-4];

      outdata[i] = temp;

      if(temp > 32767)
      {
        outdata[i] = 32767;
        //OutputDebugString("DFG?");
      }
      if(temp < -32768)
        outdata[i] = -32768;

     //right channel
     //outdata[i+1] = cf.b0*indata[i] + cf.b1*indata[i-1] + cf.b2*indata[i-2]
//                        - cf.a1*outdata[i-1] - cf.a2*outdata[i-2];
//
//     if(outdata[i] > 32767) outdata[i] = 32767;
//     if(outdata[i] < -32768) outdata[i] = -32768;
  }

  xnr[N_1] = indata[numsamples*2 - 1];
  xnr[N_2] = indata[numsamples*2 - 3];

  xnl[N_1] = indata[numsamples*2 - 2];
  xnl[N_2] = indata[numsamples*2 - 4];


  ynr[N_1] = outdata[numsamples*2 - 1];
  ynr[N_2] = outdata[numsamples*2 - 3];

  ynl[N_1] = outdata[numsamples*2 - 2];
  ynl[N_2] = outdata[numsamples*2 - 4];

}
//---------------------------------------------------------------------------

//Multiple sections, keep data as float.
void CSOS::Filter(float* indata, float* outdata, int numsamples)
{



  outdata[0] = cf.b0*indata[0] + cf.b1*xnl[N_1] + cf.b2*xnl[N_2]
                        - cf.a1*ynl[N_1] - cf.a2*ynl[N_2];


  outdata[1] = cf.b0*indata[1] + cf.b1*xnr[N_1] + cf.b2*xnr[N_2]
                        - cf.a1*ynr[N_1] - cf.a2*ynr[N_2];


  outdata[2] = cf.b0*indata[2] + cf.b1*indata[0] + cf.b2*xnl[N_1]
                        - cf.a1*outdata[0] - cf.a2*ynl[N_1];


  outdata[3] = cf.b0*indata[3] + cf.b1*indata[1] + cf.b2*xnr[N_1]
                        - cf.a1*outdata[1] - cf.a2*ynr[N_1];



  for(int i = 4; i< numsamples*2; ++i)
  {
     //left channel
     outdata[i] = cf.b0*indata[i] + cf.b1*indata[i-2] + cf.b2*indata[i-4]
                        - cf.a1*outdata[i-2] - cf.a2*outdata[i-4];

      
     //right channel
     //outdata[i+1] = cf.b0*indata[i] + cf.b1*indata[i-1] + cf.b2*indata[i-2]
//                        - cf.a1*outdata[i-1] - cf.a2*outdata[i-2];
//
//     if(outdata[i] > 32767) outdata[i] = 32767;
//     if(outdata[i] < -32768) outdata[i] = -32768;
  }

  xnr[N_1] = indata[numsamples*2 - 1];
  xnr[N_2] = indata[numsamples*2 - 3];

  xnl[N_1] = indata[numsamples*2 - 2];
  xnl[N_2] = indata[numsamples*2 - 4];


  ynr[N_1] = outdata[numsamples*2 - 1];
  ynr[N_2] = outdata[numsamples*2 - 3];

  ynl[N_1] = outdata[numsamples*2 - 2];
  ynl[N_2] = outdata[numsamples*2 - 4];
}
//---------------------------------------------------------------------------


void CSOS::Filter(float* indata, char* out, int numsamples)
{
  short int *outdata = (short int*)out;
  float temp;

  temp = cf.b0*indata[0] + cf.b1*xnl[N_1] + cf.b2*xnl[N_2]
                        - cf.a1*ynl[N_1] - cf.a2*ynl[N_2];

  outdata[0] = temp;

  if(temp > 32767)
    outdata[0] = 32767;
  if(temp < -32768)
    outdata[0] = -32768;

  temp = cf.b0*indata[1] + cf.b1*xnr[N_1] + cf.b2*xnr[N_2]
                        - cf.a1*ynr[N_1] - cf.a2*ynr[N_2];

  outdata[1] = temp;

  if(temp > 32767) outdata[1] = 32767;
  if(temp < -32768) outdata[1] = -32768;

  temp = cf.b0*indata[2] + cf.b1*indata[0] + cf.b2*xnl[N_1]
                        - cf.a1*outdata[0] - cf.a2*ynl[N_1];

  outdata[2] = temp;

  if(temp > 32767) outdata[2] = 32767;
  if(temp < -32768) outdata[2] = -32768;

  temp = cf.b0*indata[3] + cf.b1*indata[1] + cf.b2*xnr[N_1]
                        - cf.a1*outdata[1] - cf.a2*ynr[N_1];

  outdata[3] = temp;

  if(temp > 32767) outdata[3] = 32767;
  if(temp < -32768) outdata[3] = -32768;

  for(int i = 4; i< numsamples*2; ++i)
  {
     //left channel
     temp = cf.b0*indata[i] + cf.b1*indata[i-2] + cf.b2*indata[i-4]
                        - cf.a1*outdata[i-2] - cf.a2*outdata[i-4];

      outdata[i] = temp;

      if(temp > 32767)
        outdata[i] = 32767;
      if(temp < -32768)
        outdata[i] = -32768;

     //right channel
     //outdata[i+1] = cf.b0*indata[i] + cf.b1*indata[i-1] + cf.b2*indata[i-2]
//                        - cf.a1*outdata[i-1] - cf.a2*outdata[i-2];
//
//     if(outdata[i] > 32767) outdata[i] = 32767;
//     if(outdata[i] < -32768) outdata[i] = -32768;
  }

  xnr[N_1] = indata[numsamples*2 - 1];
  xnr[N_2] = indata[numsamples*2 - 3];

  xnl[N_1] = indata[numsamples*2 - 2];
  xnl[N_2] = indata[numsamples*2 - 4];


  ynr[N_1] = outdata[numsamples*2 - 1];
  ynr[N_2] = outdata[numsamples*2 - 3];

  ynl[N_1] = outdata[numsamples*2 - 2];
  ynl[N_2] = outdata[numsamples*2 - 4];
}
//---------------------------------------------------------------------------

void CSOS::Filter(char* in, float* outdata, int numsamples)
{
  short int* indata = (short int*)in;


  outdata[0] = cf.b0*indata[0] + cf.b1*xnl[N_1] + cf.b2*xnl[N_2]
                        - cf.a1*ynl[N_1] - cf.a2*ynl[N_2];


  outdata[1] = cf.b0*indata[1] + cf.b1*xnr[N_1] + cf.b2*xnr[N_2]
                        - cf.a1*ynr[N_1] - cf.a2*ynr[N_2];


  outdata[2] = cf.b0*indata[2] + cf.b1*indata[0] + cf.b2*xnl[N_1]
                        - cf.a1*outdata[0] - cf.a2*ynl[N_1];


  outdata[3] = cf.b0*indata[3] + cf.b1*indata[1] + cf.b2*xnr[N_1]
                        - cf.a1*outdata[1] - cf.a2*ynr[N_1];



  for(int i = 4; i< numsamples*2; ++i)
  {
     //left channel
     outdata[i] = cf.b0*indata[i] + cf.b1*indata[i-2] + cf.b2*indata[i-4]
                        - cf.a1*outdata[i-2] - cf.a2*outdata[i-4];

      
     //right channel
     //outdata[i+1] = cf.b0*indata[i] + cf.b1*indata[i-1] + cf.b2*indata[i-2]
//                        - cf.a1*outdata[i-1] - cf.a2*outdata[i-2];
//
//     if(outdata[i] > 32767) outdata[i] = 32767;
//     if(outdata[i] < -32768) outdata[i] = -32768;
  }

  xnr[N_1] = indata[numsamples*2 - 1];
  xnr[N_2] = indata[numsamples*2 - 3];

  xnl[N_1] = indata[numsamples*2 - 2];
  xnl[N_2] = indata[numsamples*2 - 4];


  ynr[N_1] = outdata[numsamples*2 - 1];
  ynr[N_2] = outdata[numsamples*2 - 3];

  ynl[N_1] = outdata[numsamples*2 - 2];
  ynl[N_2] = outdata[numsamples*2 - 4];

}
