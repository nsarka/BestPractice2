//---------------------------------------------------------------------------

#ifndef TMP3ReaderH
#define TMP3ReaderH
//---------------------------------------------------------------------------
#include <windows.h>
#include "TReader.h"
#include <mmreg.h>
#include <msacm.h>
//#include <stdio.h>
#include <map>

namespace AudioReader
{
enum MpegVersion {MPEG1=3,MPEG2=2,MPEG25=0,MPEGR=1} ;

class TMP3Reader : public TReader
{
  HACMSTREAM g_mp3stream, wav2wavstream;
  HANDLE hfile;
  __int64 totalframes;
  unsigned int BitRate, SampleFreq, SamplesWrong;
  bool IsMono;
  unsigned int ID3_skip;

  AudioReader::MpegVersion MPEGVersion;

  //maps frames to file position
  std::map<__int64,__int64> FramePointers;

  char* my_file;

public:
  TMP3Reader(const char* file);
  virtual ~TMP3Reader();

  //Seeks to a desired sample pos
  virtual __int64 Seek(__int64 sampleposition);

  //fills the buffer with the number of requested samples
  //returns number of samples read.
  //Padds buffer with zeros to always return enough data
  virtual int FillBuffer(Buffer& buffertofill, int numsamplesrequested);

  //returns the length in samples of the file/cd track/etc.
  virtual __int64 GetLength(){return totalframes*1152;}

  virtual int GetSampleFrequency();

  TMP3Reader* MakeCopy(){return new TMP3Reader(this);};
private:
        bool SetupFramePointers();
        TMP3Reader(const TMP3Reader* tocopy);
        void FindMP3Codec();
        bool GetMP3Info();
        MMRESULT GetConversionStream();
        bool inline IsFrameValid(const char*);
};

} //namespace
#endif
