//---------------------------------------------------------------------------
#pragma hdrstop

#include "TWaveReader.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
namespace AudioReader
{

TWaveReader::TWaveReader(const char* filename)
{
  my_filename = new char[lstrlen(filename)+1];
  lstrcpy(my_filename,filename);
  
  hfile = mmioOpen(const_cast<char *>(filename),NULL,MMIO_READ);
  if(!hfile)
    throw(ErrCannotOpen);


  if(RIFFGetWaveFormat(wf))
  {
    if (wf.wFormatTag != WAVE_FORMAT_PCM || wf.nChannels != 2
                || wf.wBitsPerSample != 16)
      throw(ErrFormatNotSupported);
  }
  else
    throw(ErrBadFile);


}
//---------------------------------------------------------------------------

TWaveReader* TWaveReader::MakeCopy()
{
  return new TWaveReader(my_filename);
}
//---------------------------------------------------------------------------

TWaveReader::~TWaveReader()
{
 if(hfile) mmioClose(hfile,0);
}
//---------------------------------------------------------------------------

__int64 TWaveReader::Seek(__int64 SamplePos)
{
  //SamplePos <<= 2; //Bytes per channel*channels per sample
  IsEof = SamplePos > GetLength() || RIFFSetStartPoint((DWORD) SamplePos*4);
  return SamplePos; 
}
//---------------------------------------------------------------------------


bool TWaveReader::RIFFGetWaveFormat(WAVEFORMATEX& wf)
{
  MMCKINFO mmckinfoParent;
  MMCKINFO mmckinfoSubchunk;

  mmioSeek(hfile,0,SEEK_SET);

  // Locate a "RIFF" chunk with a "WAVE" form type to make
  // sure the file is a waveform-audio file.
  mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

  if (mmioDescend(hfile, &mmckinfoParent, NULL, MMIO_FINDRIFF) == MMSYSERR_NOERROR)
  {
    // Find the format chunk (form type "FMT"); it should be
    // a subchunk of the "RIFF" parent chunk.
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');

    if (mmioDescend(hfile, &mmckinfoSubchunk, &mmckinfoParent,
        MMIO_FINDCHUNK) == MMSYSERR_NOERROR)
    {
       if (mmioRead(hfile, (HPSTR) &wf, sizeof(WAVEFORMATEX)) !=
           sizeof(WAVEFORMATEX))
         return false;

       //mmioAscend(hfile,NULL,0);
       return true;

    }
    else
        return false;
  }
  else
      return false;

}
//---------------------------------------------------------------------------

DWORD TWaveReader::RIFFGetWaveDataSize() //also positions file pointer to beginning
                                        //of data stream
{
  MMCKINFO mmckinfoParent;
  MMCKINFO mmckinfoSubchunk;

  mmioSeek(hfile,0,SEEK_SET);

  // Locate a "RIFF" chunk with a "WAVE" form type to make
  // sure the file is a waveform-audio file.
  mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

  if (mmioDescend(hfile, &mmckinfoParent, NULL, MMIO_FINDRIFF) == MMSYSERR_NOERROR)
  {
    // Find the format chunk (form type "FMT"); it should be
    // a subchunk of the "RIFF" parent chunk.
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');

    if (mmioDescend(hfile, &mmckinfoSubchunk, &mmckinfoParent,
        MMIO_FINDCHUNK) == MMSYSERR_NOERROR)
    {
        return mmckinfoSubchunk.cksize;
    }
    else
        return 0;
  }
  else
      return 0;

}
//---------------------------------------------------------------------------

bool TWaveReader::RIFFSetStartPoint(int ByteOffset)
{

  MMCKINFO mmckinfoParent;
  MMCKINFO mmckinfoSubchunk;

  mmioSeek(hfile,0,SEEK_SET);

  // Locate a "RIFF" chunk with a "WAVE" form type to make
  // sure the file is a waveform-audio file.
  mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

  if (mmioDescend(hfile, &mmckinfoParent, NULL, MMIO_FINDRIFF) == MMSYSERR_NOERROR)
  {
    // Find the data chunk (form type "data"); it should be
    // a subchunk of the "RIFF" parent chunk.
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');

    if (mmioDescend(hfile, &mmckinfoSubchunk, &mmckinfoParent,
        MMIO_FINDCHUNK) == MMSYSERR_NOERROR)
    {

       return mmioSeek(hfile, ByteOffset, SEEK_CUR ) == -1;

    }
    else
        return 0;
  }
  else
      return 0;
}
//---------------------------------------------------------------------------


int TWaveReader::FillBuffer(Buffer& buffer, int numsamplesrequested)
{
        //int distance =  buffer.data.end() - buffer.data.begin();
        
        //Convert from samples to 16 bit stereo audio
        unsigned int numbytes = numsamplesrequested<<2;

        char* readbuffer = new char[numbytes];

        unsigned int cbRead = mmioRead(hfile,readbuffer,numbytes);

        //buffer.data.resize(buffer.data.size() + numbytes,0);
        buffer.data.insert(buffer.data.end(),readbuffer,readbuffer+numbytes);

        delete[] readbuffer;

        if(cbRead<numbytes) IsEof = true;
        
        return cbRead>>2;
}


//---------------------------------------------------------------------------


}//namespace AudioReader
