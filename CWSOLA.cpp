//---------------------------------------------------------------------------


#pragma hdrstop

#include "CWSOLA.h"
#include <algorithm>
#include <cmath>
#include <assert.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace
{
constexpr double kPi = 3.141592653589793238462643383279502884;
}

int CWOLA::ProcessBlock(float Speed, char* outbuff)
{
  if(tsmode == tsmWOLA)
    return WOLAProcessBlock(Speed, outbuff);

  return WSOLAProcessBlock(Speed, outbuff);
}
//---------------------------------------------------------------------------

int CWOLA::WOLAProcessBlock(float Speed, char* outbuff)
{
  int bytecounter=0;

  float fstep = Speed*WindowLength/1000.0;
  int step = (int)fstep;
  step_error += fstep - step;

  if (step_error > 1)
  {
    ++step;
    --step_error;
  }
  step *= BLOCK_ALIGN;

  char* tempbuff = new char[copysize];
  //size in bytes to hold WindowLength worth of samples

  for (int i = 0;i < cbBuffer;i += copysize)
  {
    //First do the ramp down part
    int bytes_left = WorkBuffer.data_left();

    // Add new data if either:
    //  1) There is not enough data for the ramp down part
    //  2) There is not enough data to perform the time step
    if (bytes_left < copysize || step > bytes_left)
    {
        int bufferindex = WorkBuffer.pos - WorkBuffer.data.begin();
        Reader->FillBuffer(WorkBuffer, step>>2);
        WorkBuffer.pos = WorkBuffer.data.begin() + bufferindex;

    }

    //Copy to the output buffer
    std::copy(WorkBuffer.pos,WorkBuffer.pos + copysize,outbuff);

    //Place the inbuff pointer on the ideal spot (time-wise)
    //Now guaranteed to have enough data

    WorkBuffer.pos += step;

    std::copy(WorkBuffer.pos - copysize,WorkBuffer.pos,tempbuff);

    ApplyWindow(outbuff,wRampDown);
    ApplyWindow(tempbuff,wRampUp);

    MixBuffers(outbuff,tempbuff);

    outbuff += copysize;

    bytecounter += step;
  }



  std::vector<char> temp;

  int distance = WorkBuffer.data.end() - WorkBuffer.pos;
  
  temp.assign(WorkBuffer.pos - 3*copysize,WorkBuffer.data.end());
  WorkBuffer.data.swap(temp);
  WorkBuffer.pos =  WorkBuffer.data.end() - distance;

  delete[] tempbuff;

  return bytecounter;


}
//---------------------------------------------------------------------------

void CWOLA::ApplyWindow(char* buffer, WindowRamp rampdir)
{
   short int* ibuffer = (short int*) buffer;

   switch(rampdir)
   {
     case wRampDown:


                              ibuffer += XCorrStart*2;
                              for(unsigned int i = XCorrStart; i < WindowLength;i++)
                              {

                                *ibuffer *= Window[i];
                                ibuffer++;
                                *ibuffer *= Window[i];
                                ibuffer++;
                              }
                              break;


     case wRampUp:


                              for(unsigned int i = 0; i < XCorrEnd;i++)
                              {
                                *ibuffer *= Window[WindowLength - 1 - i];
                                ibuffer++;
                                *ibuffer *= Window[WindowLength - 1 - i];
                                ibuffer++;
                              }
                              break;
       
   }
}
//---------------------------------------------------------------------------

void CWOLA::MixBuffers(char* dest, char* mix_in)
{
   short int* idest = (short int*) dest;
   short int* imix_in = (short int*) mix_in;
   


                for(int i = 0;i < copysize/2;++i)
                  idest[i] += imix_in[i];


     
   
}
//---------------------------------------------------------------------------

void CWOLA::SetUpWindow()
{
       Window.assign(WindowLength, 0.0f);
       if(XCorrWin)
        delete[] XCorrWin;

       XCorrWin = new float[WindowLength];

        XCorrLength = WindowLength / 5; //Is 20 ms overlap
        XCorrStart = (WindowLength - XCorrLength)/2;
        XCorrEnd = XCorrStart + XCorrLength;

       for(unsigned int i = 0;i < XCorrStart; ++i)
         Window[i] = 1;

       for(unsigned int i = XCorrStart;i < XCorrEnd; ++i)
         Window[i] = static_cast<float>((0.5+0.5*cos(kPi* (i - XCorrStart)/(XCorrLength-1))));

       for(unsigned int i = XCorrEnd;i < WindowLength; ++i)
         Window[i] = 0;

       for(unsigned int i = 0;i < WindowLength; ++i)
         XCorrWin[i] = Window[i]*Window[WindowLength - 1 - i];
}
//---------------------------------------------------------------------------

int CWOLA::WSOLAProcessBlock(float Speed, char* outbuff)
{
  double fstep = Speed*WindowLength/1000.0;
  int inbuff_stepsize = (int)fstep;
  step_error += fstep - inbuff_stepsize;

  if (step_error > 1)
  {
    ++inbuff_stepsize;
    --step_error;
  }
  int xcorrbufflength = copysize+SearchWin*BLOCK_ALIGN;

  char* xcorrbuff = new char[xcorrbufflength];
  int bytecounter = 0;
  inbuff_stepsize *= BLOCK_ALIGN;

  char* tempbuff = new char[copysize];
  //size in bytes to hold WindowLength worth of samples

  for (int i = 0;i < cbBuffer;i += copysize)
  {
    //First do the ramp down part
    int bytes_left = WorkBuffer.data_left();
    int step = inbuff_stepsize + (SearchWin*BLOCK_ALIGN)/2 - delta_target;

    // Add new data if either:
    //  1) There is not enough data for the ramp down part
    //  2) There is not enough data to perform the time step
    if (bytes_left < copysize || step > bytes_left)
    {
        int bufferindex = WorkBuffer.pos - WorkBuffer.data.begin();
        Reader->FillBuffer(WorkBuffer,step>>2);
        WorkBuffer.pos = WorkBuffer.data.begin() + bufferindex;
    }

    //Copy to the output buffer
    std::copy(WorkBuffer.pos,WorkBuffer.pos + copysize,outbuff);

    //Place the inbuff pointer on the ideal spot (time-wise)
    //but include max search window/2
    //Now guaranteed to have enough data

    WorkBuffer.pos += step;

    //bytes_left = WorkBuffer.pos - WorkBuffer.data.begin();

    //Now do XCORR

    //Copy from inbuff what we need
    std::copy(WorkBuffer.pos - xcorrbufflength, WorkBuffer.pos,xcorrbuff);

    //correct the inbuff position
    int shift;
    if(wxmode == wxmFast)
    {
     shift =  SearchBestXCorrFast(outbuff,xcorrbuff);  //Corrects for block alignment
                                                   //The real shift, if there,
    }
    else                                               //is to the left when shift<0
    {
     shift =  SearchBestXCorr(outbuff,xcorrbuff);
    }

    delta_target = shift+(SearchWin*BLOCK_ALIGN)/2;  //This makes up for the
                                                      //inbuff position in the
                                                      //ramp down code
    //Label3->Caption = delta_target;
    WorkBuffer.pos += shift;   //Put on best XCORR spot (maximally in phase)


    //Now the ramp up from this window position
    //Copy from inbuff what we need

    //assert(WorkBuffer.pos - copysize + 2 > WorkBuffer.data.begin());
    std::copy(WorkBuffer.pos - copysize,WorkBuffer.pos,tempbuff);



    ApplyWindow(outbuff,wRampDown);
    ApplyWindow(tempbuff,wRampUp);

    MixBuffers(outbuff,tempbuff);

    outbuff += copysize;

    bytecounter += (step+shift);
  }



  std::vector<char> temp;


  int distance = WorkBuffer.data.end() - WorkBuffer.pos;
  temp.assign(WorkBuffer.pos - 3*copysize,WorkBuffer.data.end());
  WorkBuffer.data.swap(temp);
  WorkBuffer.pos =  WorkBuffer.data.end() - distance;

  delete[] tempbuff;
  delete[] xcorrbuff;

  return bytecounter;

}
//---------------------------------------------------------------------------

int CWOLA::SearchBestXCorr(char* to_be_matched, char* searchbuffer)
{
     short int* ba = (short int*) to_be_matched;
     short int* bb = (short int*) searchbuffer;

     signed int error=0, min, minpos = 0;

     for(unsigned int k=XCorrStart*2;k < XCorrEnd*2;++k)
     {
       error += abs((int)ba[k] - bb[k]) ;//* XCorrWin[k]; //Left channel
     }
                      
     min = error;

     for (int i = 2,stop = SearchWin*2;i < stop;i+=2)
     {
       error = 0;
       for(unsigned int k=XCorrStart*2;k < XCorrEnd*2;++k)
       {
         error += abs((int)ba[k] - bb[k+i]);// * XCorrWin[k]; //Left channel
       }

       if(error<min)
       {
         min = error;
         minpos = i;
       }
     }


    return ((minpos>>1) - SearchWin + 1)*BLOCK_ALIGN;


}

//---------------------------------------------------------------------------
int CWOLA::SearchBestXCorrFast(char* to_be_matched, char* searchbuffer)
{
     short int* ba = (short int*) to_be_matched;
     short int* bb = (short int*) searchbuffer;
     short int step = WindowLength/20;

     int error, min, minpos = 0,i = 0, delta_win = SearchWin*2, stop = delta_win;
     int loops = 4;

     while (loops)
     {

       error = 0;

       for(unsigned int k=XCorrStart*2;k < XCorrEnd*2;k+=step)
       {
         error += abs((int)ba[k] - (int)bb[k+i]) ;//* XCorrWin[k]; //Left channel
       }

       min = error;
       i+=2;

       for (;i < stop;i+=2)
       {
         error = 0;
         for(unsigned int k=XCorrStart*2;k < XCorrEnd*2;k+=step)
         {
           error += abs((int)ba[k] - (int)bb[k+i]);// * XCorrWin[k]; //Left channel
         }

         if(error<min)
         {
           min = error;
           minpos = i;
         }
       }

       delta_win >>=1;
       i = (minpos - delta_win);
       stop  = minpos + delta_win;
       step >>= 1;
       if(step <= 0) step = 1;
       if (i < 0) i=0;
       if (stop > SearchWin*2) stop = SearchWin*2;

       --loops;
     }

    return ((minpos>>1) - SearchWin + 1)*BLOCK_ALIGN;


}
//---------------------------------------------------------------------------
