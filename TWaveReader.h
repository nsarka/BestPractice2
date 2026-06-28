//---------------------------------------------------------------------------

#ifndef TWaveReaderH
#define TWaveReaderH

#include <windows.h>
#include "TReader.h"

namespace AudioReader
{

class TWaveReader : public TReader
{
private:
  HMMIO hfile;
  WAVEFORMATEX wf;
  bool RIFFGetWaveFormat( WAVEFORMATEX& wf);
  DWORD RIFFGetWaveDataSize(); //BYTE size
  bool RIFFSetStartPoint(int ByteOffset);

  char* my_filename;

public:
  __int64 Seek(__int64 SamplePosition);
  __int64 GetLength(){ return RIFFGetWaveDataSize()>>2;}
  int FillBuffer(Buffer& buffertofill, int numsamplesrequested);
  const char* GetFileName(){return my_filename;};
  int GetSampleFrequency(){return wf.nSamplesPerSec;};
  explicit TWaveReader(const char* filename) ;
  ~TWaveReader();

  virtual TWaveReader* MakeCopy();



};



//---------------------------------------------------------------------------

}//namespace
#endif
