//---------------------------------------------------------------------------


#pragma hdrstop

#include "TCDTrackReader.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
namespace AudioReader
{

void MSB2DWORD( DWORD *d, BYTE *b )
{
  DWORD retVal;

  retVal = (DWORD)b[0];
  retVal = (retVal<<8) + (DWORD)b[1];
  retVal = (retVal<<8) + (DWORD)b[2];
  retVal = (retVal<<8) + (DWORD)b[3];

  *d = retVal;
}


TCDTrackReader::TCDTrackReader(HCDROM DeviceHandle,DWORD startframe,DWORD endframe)
{
  hCD = DeviceHandle;
  dwStart = dwCurrent = startframe;
  dwEnd = endframe;
  cdthread = NULL;
  cdthreadid = 0;
  ReadAhead = true;
  InitializeCriticalSection(&csTrack);

  Seek(0);
}
//---------------------------------------------------------------------------

TCDTrackReader* TCDTrackReader::MakeCopy()
{
  TCDTrackReader* cpy = new TCDTrackReader(hCD,dwStart,dwEnd);
  cpy->EnableReadAhead(false);
  return cpy;
}
//---------------------------------------------------------------------------

__int64 TCDTrackReader::Seek(__int64 SamplePosition)
{
  CDBuffer.data.clear();
  CDBuffer.data.resize(0,0);
  CDBuffer.pos = CDBuffer.data.end();

//  if(tOverlap) tOverlap->len = tOverlap->startFrame = tOverlap->numFrames = 0;

  dwCurrent = dwStart + SamplePosition/(588);



  if(dwCurrent > dwEnd)
  {
    dwCurrent = dwEnd;
    IsEof = true;
    return dwEnd*2352/4;
  }

  IsEof = false;
  //Immediately start reading the new data
  ReadCDToBuffer((LPVOID) this);

  //Add the new data to the CDBuffer
  CheckCDBuffer();

  __int64 SamplePositionDiff = (SamplePosition % 588);

  CDBuffer.pos += SamplePositionDiff*4; 
  return SamplePosition;

}

//---------------------------------------------------------------------------

int TCDTrackReader::FillBuffer(Buffer& buffer,int numsamples)
{
  int numframes;
    int numbytes = ((numsamples<<2)/2352 + 1)*2352;

    //  Make room for new elements
    //buffer.resize(buffer.size() + numbytes,0);

    int bytes_available = CDBuffer.data_left();

    if(bytes_available < numbytes) //not enough bytes are available
    {
      ReadCDToBuffer((LPVOID) this);  //Fill the buffer (if you can)
      CheckCDBuffer();
      bytes_available = CDBuffer.data_left();
    }
    
    if(bytes_available) //When this helped or when there was no problem
    {
      int bytes_to_copy = (numbytes > bytes_available)? bytes_available:numbytes;

      buffer.data.insert(buffer.data.end(),CDBuffer.pos,CDBuffer.pos + bytes_to_copy );

      CDBuffer.pos += bytes_to_copy;

      numbytes -= bytes_to_copy;
    }
    else{
     IsEof = true;
     dwEnd = dwCurrent;
    }
    buffer.data.insert(buffer.data.end(),numbytes,0);

    CheckCDBuffer();
    return bytes_available;
}
//---------------------------------------------------------------------------

void TCDTrackReader::CheckCDBuffer()
{
      if((CDBuffer.data.end() - CDBuffer.pos) < 375*2352) //5 sec audio
      {
        if (sizebuf) //threadbuf has audio, other thread is dead
        {
             //Create room for new elements

            std::vector<char> temp;

            temp.reserve(sizebuf+CDBuffer.data.end()-CDBuffer.pos);


            temp.insert(temp.begin(),CDBuffer.pos,CDBuffer.data.end());

            temp.insert(temp.end(),threadbuf.begin(),threadbuf.end());

            CDBuffer.data.swap(temp);

            CDBuffer.pos = CDBuffer.data.begin();

            sizebuf = 0;

            cdthread = NULL;

            //should not be necessary: is destructed when out of scope.
            temp.clear();


        }
        else
        {
         //Find out why there's no audio (end of track?):

         //Are we already running the cd read thread?
         if(cdthread) return;

         //No other thread running, we don't need this
         //EnterCriticalSection(&csTrack);
         if(dwCurrent < dwEnd && ReadAhead) //There's still audio on the track
         {
           //Start thread
           cdthread = CreateThread(NULL,0,ReadCDToBuffer,
             static_cast<LPVOID>(this),0, &cdthreadid);
           //ReadCDToBuffer(NULL);
         }
         
        }
      }
}
//---------------------------------------------------------------------------

DWORD WINAPI TCDTrackReader::ReadCDToBuffer(LPVOID ClassInstance)
{
  TCDTrackReader* cdr = static_cast<TCDTrackReader*>(ClassInstance);

  int numframes = 750;
  EnterCriticalSection(&(cdr->csTrack));

      if(cdr->dwCurrent + numframes > cdr->dwEnd)
      {
        numframes = cdr->dwEnd - cdr->dwCurrent;
      }


      cdr->threadbuf.clear();
      //DON'T! RipAudio _inserts_ the frames!!
      //threadbuf.resize(numframes*2352);
      if(numframes > 0)
      {
        //dwCurrent is updated inside RipAudio
        numframes = cdr->RipAudio(numframes);
      }
      else
        numframes = 0;



  LeaveCriticalSection(&(cdr->csTrack));

  InterlockedExchange((long*)&(cdr->sizebuf) ,numframes*2352);
  return 0;
}

//---------------------------------------------------------------------------

__int64 TCDTrackReader::GetLength()
{
  return (( dwEnd - dwStart ) * (2352/4));
}
//---------------------------------------------------------------------------

TCDTrackReader::~TCDTrackReader()
{
  {if(cdthread) WaitForSingleObject(cdthread,10000);}
  DeleteCriticalSection(&csTrack);
}
//---------------------------------------------------------------------------

//Return number of frames read - uses hCD and threadbuf from class instance
int TCDTrackReader::RipAudio(DWORD dwLen)
{
  DWORD dwStatus;
  DWORD num2rip = 26;
  int retries;
  LPTRACKBUF t;
  DWORD numWritten = 0;

  t = NewTrackBuf( 26 );
  if ( !t )
    return 0;

  while( dwLen )
    {

      if ( dwLen < num2rip )
	num2rip = dwLen;

      retries = 3;
      dwStatus = SS_ERR;
      while ( retries-- && (dwStatus != SS_COMP) )
	{
	  t->numFrames = num2rip;
	  t->startOffset = 0;
	  t->len = 0;
	  t->startFrame = dwCurrent;
          dwStatus = ReadCDAudioLBA( hCD, t);
	}
      if ( dwStatus == SS_COMP )
	{
	  threadbuf.insert(threadbuf.end(),t->buf + t->startOffset,t->buf + t->startOffset+t->len);
	  numWritten += t->numFrames;
	}
      else
      {
        
        GlobalFree( (HGLOBAL)t );
        return numWritten;

      }

      dwCurrent += num2rip;
      dwLen -= num2rip;
    }

  GlobalFree( (HGLOBAL)t );

  return numWritten;


}
//---------------------------------------------------------------------------

LPTRACKBUF TCDTrackReader::NewTrackBuf( DWORD numFrames )
{
  LPTRACKBUF t;
  int numAlloc;

  numAlloc = (((int)numFrames)*2352) + TRACKBUFEXTRA;

  t = (LPTRACKBUF)GlobalAlloc( GPTR, numAlloc );

  if ( !t )
    return NULL;

  t->startFrame = 0;
  t->numFrames = 0;
  t->maxLen = numFrames * 2352;
  t->len = 0;
  t->status = 0;
  t->startOffset = 0;

  return t;
}



}//namespace AudioReader
