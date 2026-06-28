//---------------------------------------------------------------------------

#ifndef CWSOLAH
#define CWSOLAH
//---------------------------------------------------------------------------

#include "TReader.h"


#define BLOCK_ALIGN 4

enum TimeStretchMode {tsmWOLA, tsmWSOLA};
enum WSOLAXMode {wxmFast, wxmGood};

//typedef void (__stdcall  *FillFunc)(unsigned long bytes, Buffer& buffer_to_fill);
using AudioReader::TReader;
class CWOLA {

  enum WindowRamp{wRampUp,wRampDown};

  Buffer WorkBuffer;
  AudioReader::TReader* Reader;

  unsigned int copysize, WindowLength;
  int cbBuffer; //Output buffer size
  int SearchWin;
  int delta_target;
  double step_error; //Corrects for small errors in step size
  //For use in Xcorr and general speedup
  unsigned int XCorrLength,XCorrStart,XCorrEnd;

  std::vector<float> Window;
  float* XCorrWin;

  
  TimeStretchMode tsmode;
  WSOLAXMode wxmode;

  void SetUpWindow();
  void ApplyWindow(char* buffer, WindowRamp rampdir);
  void ApplyWindow(float* buffer, WindowRamp rampdir);
  void MixBuffers(char* dest, char* mix_in);
  void MixBuffers(float* dest, float* mix_in);
  int WSOLAProcessBlock(float Speed, char* outbuff);
  int WSOLAProcessBlock(float Speed, float* outbuff);
  int WOLAProcessBlock(float Speed, char* outbuff);
  int WOLAProcessBlock(float Speed, float* outbuff);
  int SearchBestXCorr(char* to_be_matched, char* searchbuffer);
  int SearchBestXCorr(float* to_be_matched, float* searchbuffer);
  int SearchBestXCorrFast(char* to_be_matched, char* searchbuffer);
  int SearchBestXCorrFast(float* to_be_matched, float* searchbuffer);

public:
  CWOLA(int wl, int cbbuf, int srch_win, AudioReader::TReader* areader) :
        Reader(areader), copysize(BLOCK_ALIGN*wl), WindowLength(wl),
        cbBuffer(cbbuf), SearchWin(srch_win), XCorrWin(NULL),
        tsmode(tsmWOLA), wxmode(wxmFast)
          {SetUpWindow();
           WorkBuffer.data.resize(3*wl*BLOCK_ALIGN,0);
           WorkBuffer.pos = WorkBuffer.data.begin() + 3*wl*BLOCK_ALIGN;
           delta_target = 0;
           step_error = 0;};

  ~CWOLA() { delete[] XCorrWin; }

  CWOLA(const CWOLA&) = delete;
  CWOLA& operator=(const CWOLA&) = delete;

  int ProcessBlock(float Speed, char* outbuffer); //returns bytes used from source

  void SetMode(TimeStretchMode new_mode){tsmode = new_mode;};
  void SetMode(WSOLAXMode new_mode) {wxmode = new_mode;}

};
#endif
