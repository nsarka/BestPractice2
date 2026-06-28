//---------------------------------------------------------------------------

#ifndef TCDTrackReaderH
#define TCDTrackReaderH
//---------------------------------------------------------------------------
#include "TReader.h"
#include "akrip32.h"

namespace AudioReader
{


class TCDTrackReader : public TReader
{
private:
  HCDROM hCD; //The Device

  HANDLE cdthread;
  DWORD dwStart, dwEnd, dwCurrent,cdthreadid;

  CRITICAL_SECTION csTrack;       //To share dwStart, dwEnd and dwCurrent in threads
  static DWORD WINAPI ReadCDToBuffer(LPVOID CallingClass); //Thread function, gives pointer to calling instance
  std::vector<char> threadbuf;
  Buffer CDBuffer;
  int sizebuf;
  bool ReadAhead; //Enables or disables the multithreaded caching of data
  void CheckCDBuffer() ;
  int RipAudio(DWORD dwLen);
  LPTRACKBUF TCDTrackReader::NewTrackBuf( DWORD numFrames );

public:
   TCDTrackReader(HCDROM DeviceHandle,DWORD startframe,DWORD endframe);

   ~TCDTrackReader();

   __int64 Seek(__int64 SamplePosition);

   __int64 GetLength();
   int GetSampleFrequency(){return 44100;};

   int FillBuffer(Buffer& buffertofill, int numsamplesrequested);
   void EnableReadAhead(bool enabled) {ReadAhead = enabled;};
   virtual TCDTrackReader* MakeCopy();

};


}//namespace AudioReader
#endif
