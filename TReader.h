//---------------------------------------------------------------------------

#ifndef TReaderH
#define TReaderH
#include "buffer_.h"
//---------------------------------------------------------------------------

namespace AudioReader
{

class TReader
{
private:

protected:
  bool IsEof;
public:
  TReader(){IsEof = false;};
  virtual ~TReader(){};

  //Seeks to a desired sample pos
  virtual __int64 Seek(__int64 sampleposition) = 0;

  //fills the buffer with the number of requested samples
  //returns number of samples read.
  //Padds buffer with zeros to always return enough data
  virtual int FillBuffer(Buffer& buffertofill, int numsamplesrequested) = 0;

  //returns the length in samples of the file/cd track/etc.
  virtual __int64 GetLength() = 0;

  bool Eof() {return IsEof;}

  virtual TReader* MakeCopy() =0;

  virtual int GetSampleFrequency() = 0;

};

enum ReaderErrors {ErrCannotOpen,ErrBadFile,ErrFormatNotSupported,ErrNoCodec};
} //namespace AudioReader
#endif
