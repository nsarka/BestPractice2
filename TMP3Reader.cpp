//---------------------------------------------------------------------------

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)


#pragma hdrstop

#include "TMP3Reader.h"
#include <assert.h>
#define MP3_BLOCK_SIZE 522

//(MPEG1 448kbps 48 kHz)
#define MAX_FRAME_SIZE 1344
namespace{
const int MPA_HEADER_SIZE = 4;

//MPEG 1 layer 3 sample rates
unsigned int const samplerate_table[4][3] =
    {{11025, 12000, 8000},
     {0, 0, 0},                   //illegal :)
     {22050, 24000, 16000},
     {44100, 48000, 32000} };
/*
bits  	MPEG1  	        MPEG2  	        MPEG2.5
00 	44100 Hz 	22050 Hz 	11025 Hz
01 	48000 Hz 	24000 Hz 	12000 Hz
10 	32000 Hz 	16000 Hz 	8000 Hz
11 	reserv. 	reserv. 	reserv. */

// XING Header offset: 1. index = lsf, 2. index = mono
DWORD XINGOffsetStereo = 32 + MPA_HEADER_SIZE;
DWORD XINGOffsetMono = 17 + MPA_HEADER_SIZE;
DWORD LAMEOffset = 112 + MPA_HEADER_SIZE;

unsigned long const bitrate_table[5][15] = {
  /* MPEG-1 */
  { 0,  32000,  64000,  96000, 128000, 160000, 192000, 224000,  /* Layer I   */
       256000, 288000, 320000, 352000, 384000, 416000, 448000 },
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer II  */
       128000, 160000, 192000, 224000, 256000, 320000, 384000 },
  { 0,  32000,  40000,  48000,  56000,  64000,  80000,  96000,  /* Layer III */
       112000, 128000, 160000, 192000, 224000, 256000, 320000 },

  /* MPEG-2 LSF & MPEG-2.5 */
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer I   */
       128000, 144000, 160000, 176000, 192000, 224000, 256000 },
  { 0,   8000,  16000,  24000,  32000,  40000,  48000,  56000,  /* Layers    */
        64000,  80000,  96000, 112000, 128000, 144000, 160000 } /* II & III  */
};

}




//---------------------------------------------------------------------------

#pragma package(smart_init)
namespace{
int g_mp3Drivers = 0;

BOOL CALLBACK acmDriverEnumCallback( HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport ){
  if( fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CODEC ) {
    MMRESULT mmr;

    ACMDRIVERDETAILS details;
    details.cbStruct = sizeof(ACMDRIVERDETAILS);
    mmr = acmDriverDetails( hadid, &details, 0 );

    HACMDRIVER driver;
    mmr = acmDriverOpen( &driver, hadid, 0 );

    int i;
    for(i = 0; i < details.cFormatTags; i++ ){
      ACMFORMATTAGDETAILS fmtDetails;
      ZeroMemory( &fmtDetails, sizeof(fmtDetails) );
      fmtDetails.cbStruct = sizeof(ACMFORMATTAGDETAILS);
      fmtDetails.dwFormatTagIndex = i;
      mmr = acmFormatTagDetails( driver, &fmtDetails, ACM_FORMATTAGDETAILSF_INDEX );
      if( fmtDetails.dwFormatTag == WAVE_FORMAT_MPEGLAYER3 ){
        //OutputDebugString( "Found an MP3-capable ACM codec: " );
//        OutputDebugString( details.szLongName );
//        OutputDebugString( "\n" );
        g_mp3Drivers++;
      }
    }
    mmr = acmDriverClose( driver, 0 );
  }
  return true;
}

}//anon namespace
//---------------------------------------------------------------------------


namespace AudioReader
{

TMP3Reader::TMP3Reader(const char* file)
{
  MMRESULT mmr;
  wav2wavstream = NULL;
  SamplesWrong = 0;
  ID3_skip = 0;
  FindMP3Codec();

  my_file = new char[lstrlen(file)+1];
  lstrcpy(my_file,file);

  hfile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (hfile == INVALID_HANDLE_VALUE)
    throw (ErrCannotOpen);

  if(!GetMP3Info())
  {
    CloseHandle(hfile);
    throw(ErrBadFile);
  }


  if(GetConversionStream() != MMSYSERR_NOERROR)
  {
    CloseHandle(hfile);
    acmStreamClose( g_mp3stream, 0 );

     throw(ErrNoCodec);

  }

  //Parse file for frame info
  SetupFramePointers();

}
//---------------------------------------------------------------------------

TMP3Reader::TMP3Reader(const TMP3Reader* tocopy)
{
  MMRESULT mmr;
  FindMP3Codec();
  wav2wavstream = NULL;

  hfile = CreateFile(tocopy->my_file, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (hfile == INVALID_HANDLE_VALUE)
    throw (ErrCannotOpen); //Should never fire

  if(!GetMP3Info()) //Should never fire
  {
    CloseHandle(hfile);
    throw(ErrBadFile);
  }


  if(GetConversionStream() != MMSYSERR_NOERROR) //Should never fire
  {
    CloseHandle(hfile);
    acmStreamClose( g_mp3stream, 0 );

     throw(ErrNoCodec);

  }
  totalframes = tocopy->totalframes;
  FramePointers = tocopy->FramePointers; //No need to parse the file again!
}
//---------------------------------------------------------------------------

int TMP3Reader::FillBuffer(Buffer& buffer, int numsamplesrequested)
{

  //Convert from samples to 16 bit stereo audio (decoded mp3 as PCM)
  unsigned int numbytes = numsamplesrequested<<2;;

  // find out how big the decompressed buffer will be
  unsigned long rawbufsize = 0, rawbufsize_st = 0;
  MMRESULT mmr = acmStreamSize( g_mp3stream, MP3_BLOCK_SIZE, &rawbufsize, ACM_STREAMSIZEF_SOURCE );
  assert( mmr == 0 );

  if(IsMono)
  {
    mmr = acmStreamSize( wav2wavstream, rawbufsize, &rawbufsize_st, ACM_STREAMSIZEF_SOURCE );
    assert( mmr == 0 );
  }

  assert( rawbufsize > 0 );

  // allocate our I/O buffers
  LPBYTE mp3buf = new BYTE[MP3_BLOCK_SIZE];
  LPBYTE rawbuf = new BYTE[rawbufsize];
  LPBYTE w2wbuf = new BYTE[rawbufsize_st];
  
  // prepare the decoder
  ACMSTREAMHEADER mp3streamHead,w2wstreamHead;
  ZeroMemory( &mp3streamHead, sizeof(ACMSTREAMHEADER ) );
  mp3streamHead.cbStruct = sizeof(ACMSTREAMHEADER );
  mp3streamHead.pbSrc = mp3buf;
  mp3streamHead.cbSrcLength = MP3_BLOCK_SIZE;
  mp3streamHead.pbDst = rawbuf;
  mp3streamHead.cbDstLength = rawbufsize;
  mmr = acmStreamPrepareHeader( g_mp3stream, &mp3streamHead, 0 );
  assert( mmr == 0 );

  if(IsMono)
  {
    // prepare the mono to stereo format conversion
    ZeroMemory( &w2wstreamHead, sizeof(ACMSTREAMHEADER ) );
    w2wstreamHead.cbStruct = sizeof(ACMSTREAMHEADER );
    w2wstreamHead.pbSrc = rawbuf;
    w2wstreamHead.cbSrcLength = rawbufsize;
    w2wstreamHead.pbDst = w2wbuf;
    w2wstreamHead.cbDstLength = rawbufsize_st;
    mmr = acmStreamPrepareHeader( wav2wavstream, &w2wstreamHead, 0 );
    assert( mmr == 0 );
  }

  unsigned int bytes_decoded = 0;
  buffer.data.reserve(buffer.data.size() + numbytes);
  while(bytes_decoded < numbytes) {
    // suck in some MP3 data
    int count;
      ReadFile(hfile, mp3buf, MP3_BLOCK_SIZE, (ULONG*)&count, NULL );
    if( count != MP3_BLOCK_SIZE )
    {
      IsEof = true;
      break;
    }

    // convert the data
    mmr = acmStreamConvert( g_mp3stream, &mp3streamHead, ACM_STREAMCONVERTF_BLOCKALIGN );
    //assert( mmr == 0 );
    
    if(IsMono)
    {
      w2wstreamHead.cbSrcLength = mp3streamHead.cbDstLengthUsed;
      mmr = acmStreamConvert( wav2wavstream, &w2wstreamHead,0 );
      assert( mmr == 0 );
      // insert the decoded PCM into buffer
      LPBYTE raw_buff_start = w2wbuf;
      int bytes_skipped=0;
      if(SamplesWrong)
      {
        bytes_skipped = (SamplesWrong*4 < w2wstreamHead.cbDstLengthUsed)?
                              SamplesWrong*4:w2wstreamHead.cbDstLengthUsed;
        raw_buff_start += bytes_skipped;
        SamplesWrong -= bytes_skipped/4;
      }
      bytes_decoded += w2wstreamHead.cbDstLengthUsed-bytes_skipped;
      buffer.data.insert(buffer.data.end(),raw_buff_start,raw_buff_start+w2wstreamHead.cbDstLengthUsed);
    }
    else
    {
      LPBYTE raw_buff_start = rawbuf;
      int bytes_skipped=0;
      if(SamplesWrong)
      {
        bytes_skipped = (SamplesWrong*4 < mp3streamHead.cbDstLengthUsed)?
                              SamplesWrong*4:mp3streamHead.cbDstLengthUsed;
        raw_buff_start += bytes_skipped;
        SamplesWrong -= bytes_skipped/4;
      }


      bytes_decoded += mp3streamHead.cbDstLengthUsed-bytes_skipped;
      buffer.data.insert(buffer.data.end(),raw_buff_start,raw_buff_start+mp3streamHead.cbDstLengthUsed-bytes_skipped);
    }



  };
  delete mp3buf;
  delete rawbuf;
  delete w2wbuf;
  
  if(bytes_decoded < numbytes) //Possible on EOF
    buffer.data.insert(buffer.data.end(),numbytes-bytes_decoded, 0);

  w2wstreamHead.cbSrcLength = rawbufsize;
  if(IsMono) mmr = acmStreamUnprepareHeader( wav2wavstream, &w2wstreamHead, 0 );  
  assert( mmr == 0 );
  
  mmr = acmStreamUnprepareHeader( g_mp3stream, &mp3streamHead, 0 );

  assert( mmr == 0 );
  return bytes_decoded>>2;
}
//---------------------------------------------------------------------------

TMP3Reader::~TMP3Reader()
{
  // clean up
  MMRESULT mmr;

  CloseHandle(hfile);


  mmr = acmStreamClose( g_mp3stream, 0 );
  assert( mmr == 0 );

  if(wav2wavstream)
  {
    mmr = acmStreamClose( wav2wavstream, 0);
    assert(mmr == 0);
  }
}
//---------------------------------------------------------------------------

#pragma option push -r-
bool TMP3Reader::SetupFramePointers()
{
  //Here we know what the header should contain (MPEG version, Sample frequency
  //Mono or stereo)

  const int buffsize = 16*1024;
  const int max_header_piece = LAMEOffset + MPA_HEADER_SIZE + 4; //4=sizeof tag

  char readbuffer[buffsize]; //16 kB
  char * pbuff = readbuffer;
  //Sets a pointer to every 40th frame in the file.
  SetFilePointer(hfile,ID3_skip,0,FILE_BEGIN);

  ULONG cbRead;

    ReadFile(hfile,(LPVOID)readbuffer,buffsize,&cbRead,NULL);

  //const char syncword[2] = {'\xFF','\xFA'};
  totalframes = 0;
  __int64 totalread = ID3_skip;

  std::map<__int64,__int64>::iterator itr = FramePointers.begin();
  BYTE frames = 0;
  bool done = false;
  while (!done)
  {
    totalread += cbRead;
    for(pbuff = readbuffer;pbuff<readbuffer+cbRead-max_header_piece;++pbuff)
    {
      if(*pbuff == '\xFF') //Search for sync word
        if( ((*(pbuff+1) & 0xE0) == 0xE0) && ( ((*(pbuff+1))>>3 & 3) == MPEGVersion) &&
           ( ((*(pbuff+1))>>1 & 3) == 1) )


        {
          //Add pointer to frame if 40th frame
          //skip VBR header frames
          DWORD* pXingSt = (DWORD*)(pbuff+XINGOffsetStereo);
          DWORD* pXingMo = (DWORD*)(pbuff+XINGOffsetMono);
          DWORD* pLame = (DWORD*)(pbuff+LAMEOffset);

          if( (!IsMono && (*pXingSt == MAKEFOURCC('X','i','n','g')
              || *pXingSt == MAKEFOURCC('I','n','f','o')) ) //!IsMono

              || (IsMono && (*pXingMo == MAKEFOURCC('X','i','n','g')
              || *pXingMo == MAKEFOURCC('I','n','f','o')) ) //IsMono
              || *pXingSt == MAKEFOURCC('V','B','R','I')
              || *pXingSt == MAKEFOURCC('L','A','M','E')
              || *pLame == MAKEFOURCC('L','A','M','E') )
          {
             //OutputDebugStringA("Skipped VBR frame");
              continue;
          }

         //Check if header found is valid
          unsigned int bitrate_index = (*(pbuff+2)>>4)& 0xF;

          if(bitrate_index >14 || bitrate_index ==0)
          {
            continue; //not a real frame
          }
          if(samplerate_table[MPEGVersion][(*(pbuff+2)>>2 & 3)] != SampleFreq)

          {
            //OutputDebugStringA("Skipped not real frame");
            continue;
          }
          int bitrate_table_mpeg = (MPEGVersion == MPEG1)? 2 : 4;

          //update Bitrate for jump to next frame
          BitRate = bitrate_table[bitrate_table_mpeg][bitrate_index];



          int Padding = ((*(pbuff+2)) & 2)>>1;

          ++frames; totalframes++;

          if(frames > 40 || totalframes == 1)
            {
              frames = 0;
              itr = FramePointers.insert(itr,std::pair<__int64,__int64>
                                 (totalframes, totalread - cbRead + (pbuff - readbuffer) )
                                 );
            }
          pbuff += int(((144 * BitRate) / SampleFreq ) + Padding) - 4;
        }
    }

    SetFilePointer(hfile, -max_header_piece, NULL, FILE_CURRENT);
    ReadFile(hfile,(LPVOID)readbuffer,buffsize,&cbRead,NULL);
    done = (cbRead == max_header_piece);
    totalread -= max_header_piece;
  }

  LARGE_INTEGER pos={0};

  itr = FramePointers.find(1);
  assert(itr != FramePointers.end());

  pos.QuadPart =  itr->second;

  SetFilePointer(hfile, pos.LowPart, &pos.HighPart,FILE_BEGIN);
        //FrameLen = int((144 * BitRate / SampleRate ) + Padding);
  return true;
}
#pragma pop //option
//---------------------------------------------------------------------------

void TMP3Reader::FindMP3Codec()
{
  g_mp3stream = NULL;


  // try to find an MP3 codec
  g_mp3Drivers = 0;
  acmDriverEnum( acmDriverEnumCallback, 0, 0 );
  if(g_mp3Drivers == 0)
  {
    //OutputDebugString( "No MP3 decoders found!\n" );
    throw(ErrNoCodec);
  }

}
//---------------------------------------------------------------------------

__int64 TMP3Reader::Seek(__int64 SamplePosition)
{
  const int buffsize = 4*1024;
  char readbuffer[buffsize]; //4 kB
  char * pbuff = readbuffer;
  __int64 frame = SamplePosition/1152 + 1; //MP3 has 1152 samples/frame
                                           //sample 0 is in 1st frame
  SamplesWrong = SamplePosition % 1152;
  
  if(SamplePosition > GetLength())
  {
    IsEof = true;
    return Seek(0);
  }
  else
  {
    IsEof = false;
  }

  std::map<__int64,__int64>::iterator itr = FramePointers.lower_bound(frame);

  LARGE_INTEGER Pos;

  if(itr->first > frame ) --itr; //Should still be OK for frame == 1;

  __int64 diff = frame - itr->first; //Number of frames we're wrong

  Pos.QuadPart = itr->second;

  SetFilePointer(hfile,Pos.LowPart,&Pos.HighPart,FILE_BEGIN);

  ULONG cbRead;
  ReadFile(hfile,(LPVOID)readbuffer,buffsize,&cbRead,NULL);

  __int64 totalread = 0;

  bool done = false;
  while (!done && diff > 0)
  {
    totalread += cbRead;
    for(pbuff = readbuffer;pbuff<readbuffer+cbRead-1 && diff > 0;++pbuff)
    {
      if(*pbuff == '\xFF') //Search for sync word: 0xFFFB or 0xFFFA
        if( ((*(pbuff+1)) & '\xE0') && ( ((*(pbuff+1))>>3 & 3) == MPEGVersion) &&
           ( ((*(pbuff+1))>>1 & 3) == 3) )
        {

          if(!IsFrameValid(pbuff)) continue; //also updates
                                        //BitRate if the frame is valid
          //We found a valid frame - decrease the number of frames we're wrong
          --diff;

          if(diff) //Only need to jump further if diff <> 0
          {
            int Padding = ((*(pbuff+2)) & 2)>>1;
            pbuff += int(((144 * BitRate) / SampleFreq ) + Padding) - 4;
          }
          else
          {
            totalread +=  ((int)(pbuff - (int)readbuffer) -cbRead );
            done = true;
          }
        }
    }
    if(!done)
    {
      SetFilePointer(hfile, -1, NULL, FILE_CURRENT);
      ReadFile(hfile,(LPVOID)readbuffer,buffsize,&cbRead,NULL);
      done  = (cbRead == 1);
      --totalread;
    }
  }

  //Pos.QuadPart += totalread - cbRead +(pbuff - readbuffer);
  SetFilePointer(hfile,Pos.LowPart,&Pos.HighPart,FILE_BEGIN);


  return (frame - 1)*1152;
}
//---------------------------------------------------------------------------

bool TMP3Reader::GetMP3Info()
{
  const int buffsize = 16*1024;
  char readbuffer[buffsize]; //16 kB
  unsigned char * pbuff = readbuffer;

  MpegVersion MPEGVersionLocal[2];
  unsigned int BitRateLocal[2], SampleFreqLocal[2], frame_idx;
  bool IsMonoLocal[2];

  SetFilePointer(hfile,0,0,FILE_BEGIN);
  DWORD cbRead;
  ReadFile(hfile,(LPVOID)readbuffer,buffsize,&cbRead,NULL);

  pbuff = readbuffer;
  frame_idx = 0;

  //First check for ID3V2 tags and skip them:
  ID3_skip = 0;

  while(memcmp(pbuff,"ID3",3) == 0 ) //Skip all ID3V2 tags
  {
     unsigned char * ver =pbuff+3;
     unsigned char * size = pbuff+6;
     unsigned int ID_tag_size = 0;
     if(ver [0] < 0xFF   &&       ver [1] < 0xFF   &&
        ( size[0] < 0x80   &&       size[1] < 0x80   &&
          size[2] < 0x80   &&       size[3] < 0x80))
          {
            //It's a ID3V2 header
            //Find out its size and skip this amount of bytes.
            ID_tag_size += (int)size[0];
            ID_tag_size <<= 7;
            ID_tag_size += (int)size[1];
            ID_tag_size <<= 7;
            ID_tag_size += (int)size[2];
            ID_tag_size <<= 7;
            ID_tag_size += (int)size[3];

            ID3_skip += ID_tag_size + 10; //ID3 header tag size
            SetFilePointer(hfile,ID3_skip,0,FILE_BEGIN);
            ReadFile(hfile,(LPVOID)readbuffer,buffsize,&cbRead,NULL);

          }
  }


  while(cbRead == buffsize)
  {
    while ((pbuff<readbuffer+cbRead-3))
    {
      if(*pbuff == 0xFF) //Search for sync word
        if((*(pbuff+1) & 0xE0) == 0xE0)
        {
          //Got 11 sync bits.
          //Try to get samplerate etc.
          MPEGVersionLocal[frame_idx] = (*(pbuff+1))>>3 & 3;

          //Check for Layer 3
          if( ((*(pbuff+1))>>1 & 3 )!= 1)
          {
            pbuff++;
            continue;
          }

          SampleFreqLocal[frame_idx] =
              samplerate_table[MPEGVersionLocal[frame_idx]][*(pbuff+2)>>2 & 2];

          if (SampleFreqLocal[frame_idx] == 0)
          {
            pbuff++;
            continue;
          }

          IsMonoLocal[frame_idx] = (unsigned char)(*(pbuff+3)>>6 & 3)== 3;
          unsigned int bitrate_index = (*(pbuff+2)>>4)& 0xF;

          if(bitrate_index >14)
          {
            pbuff++;
            continue;
          }
          int bitrate_table_mpeg = (MPEGVersion == MPEG1)? 2 : 4;
          BitRate = bitrate_table[bitrate_table_mpeg][bitrate_index];

          if(frame_idx == 0)
          {
            frame_idx = 1;
            pbuff++;
            continue;
          }
          else
          {
            if(MPEGVersionLocal[0] == MPEGVersionLocal[1]
                                   && SampleFreqLocal[0] == SampleFreqLocal[1]
                                   && IsMonoLocal[0] == IsMonoLocal[1])
            { //Found two identical MP3 headers
              MPEGVersion = MPEGVersionLocal[0];
              SampleFreq = SampleFreqLocal[0];
              IsMono = IsMonoLocal[0];
              SetFilePointer(hfile, 0, 0, FILE_BEGIN);

              //MPEG version is reserved
              if(MPEGVersion == MPEGR) return false;

              return true;
            }
            else //Not identical headers, move last header info
                 //to frame_idx 0 pos and keep looking for 2 identical headers
            {
              MPEGVersionLocal[0] = MPEGVersionLocal[1];
              SampleFreqLocal[0] = SampleFreqLocal[1];
              IsMonoLocal[0] = IsMonoLocal[1];
              pbuff++;
              continue;
            }
          }


        }
      ++pbuff;
    }
    SetFilePointer(hfile, -3, 0, FILE_CURRENT);
    ReadFile(hfile,(LPVOID)readbuffer,buffsize,&cbRead,NULL);
    pbuff = readbuffer;
  }

  SetFilePointer(hfile, 0, 0, FILE_BEGIN);
  return(false);
        //FrameLen = int((144 * BitRate / SampleRate ) + Padding);
}
//---------------------------------------------------------------------------

inline int TMP3Reader::GetSampleFrequency()
{
  return SampleFreq;
}
//---------------------------------------------------------------------------

MMRESULT TMP3Reader::GetConversionStream()
{
  MMRESULT mmr;
  // find the biggest format size
  DWORD maxFormatSize = 0;
  mmr = acmMetrics( NULL, ACM_METRIC_MAX_SIZE_FORMAT, &maxFormatSize );

  // define desired output format
  LPWAVEFORMATEX waveFormat = (LPWAVEFORMATEX) LocalAlloc( LPTR, maxFormatSize );
  LPWAVEFORMATEX waveFormatChoose = (LPWAVEFORMATEX) LocalAlloc( LPTR, maxFormatSize );

  waveFormat->wFormatTag = WAVE_FORMAT_PCM;
  waveFormat->nChannels = 2; // stereo
  waveFormat->nSamplesPerSec = SampleFreq;
  waveFormat->wBitsPerSample = 16; // 16 bits
  waveFormat->nBlockAlign = 4; // 4 bytes of data at a time are useful (1 sample)
  waveFormat->nAvgBytesPerSec = 4 * SampleFreq; // byte-rate
  waveFormat->cbSize = 0; // no more data to follow

  waveFormatChoose->wFormatTag = WAVE_FORMAT_PCM;
  waveFormatChoose->nSamplesPerSec = SampleFreq;
  waveFormatChoose->wBitsPerSample = 16; // 16 bits
  waveFormatChoose->nBlockAlign = 4; // 4 bytes of data at a time are useful (1 sample)
  waveFormatChoose->nAvgBytesPerSec = 4 * SampleFreq; // byte-rate
  waveFormatChoose->cbSize = 0; // no more data to follow

  // define MP3 input format
  LPMPEGLAYER3WAVEFORMAT mp3format = (LPMPEGLAYER3WAVEFORMAT) LocalAlloc( LPTR, maxFormatSize );
  mp3format->wfx.cbSize = MPEGLAYER3_WFX_EXTRA_BYTES;
  mp3format->wfx.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
  mp3format->wfx.nChannels = (IsMono)?1:2;
  mp3format->wfx.nAvgBytesPerSec = BitRate/ 8;  // not really used but must be one of 64, 96, 112, 128, 160kbps
  mp3format->wfx.wBitsPerSample = 0;                  // MUST BE ZERO
  mp3format->wfx.nBlockAlign = 1;                     // MUST BE ONE
  mp3format->wfx.nSamplesPerSec = SampleFreq;
  mp3format->fdwFlags = MPEGLAYER3_FLAG_PADDING_OFF;
  mp3format->nBlockSize = MP3_BLOCK_SIZE;             // voodoo value #1
  mp3format->nFramesPerBlock = 1;                     // MUST BE ONE
  mp3format->nCodecDelay = 1393;                      // voodoo value #2
  mp3format->wID = MPEGLAYER3_ID_MPEG;

  g_mp3stream = NULL;
  mmr = acmStreamOpen( &g_mp3stream,               // open an ACM conversion stream
                     NULL,                       // querying all ACM drivers
                     (LPWAVEFORMATEX) mp3format, // converting from MP3
                     waveFormat,                 // to WAV
                     NULL,                       // with no filter
                     0,                          // or async callbacks
                     0,                          // (and no data for the callback)
                     0 // and no flags
                     );

  if(mmr != MMSYSERR_NOERROR) //try 2 step decode
  {
    wav2wavstream = NULL;

    if(acmFormatSuggest(NULL, //Check all drivers
                     (LPWAVEFORMATEX) mp3format,    //From mp3 (mono)
                     (LPWAVEFORMATEX) waveFormatChoose, //To PCM, but can be mono
                     maxFormatSize,                     //Max size of WFX struct
                     ACM_FORMATSUGGESTF_NSAMPLESPERSEC|
                     ACM_FORMATSUGGESTF_WBITSPERSAMPLE|
                     ACM_FORMATSUGGESTF_WFORMATTAG) == 0)
    {
       mmr = acmStreamOpen( &g_mp3stream,               // open an ACM conversion stream
                     NULL,                       // querying all ACM drivers
                     (LPWAVEFORMATEX) mp3format, // converting from MP3
                     waveFormatChoose,                 // to WAV
                     NULL,                       // with no filter
                     0,                          // or async callbacks
                     0,                          // (and no data for the callback)
                     0                           // and no flags
                     );
       //Now this should be OK, since the ACM suggested it itself!
       if(mmr == MMSYSERR_NOERROR)
       {
         mmr = acmStreamOpen( &wav2wavstream,
                               NULL,
                               waveFormatChoose, //From the suggested format
                               waveFormat,  //To our native processing format
                               NULL, 0,0,0);
       }
    }
  }

  LocalFree( mp3format );
  LocalFree( waveFormat );
  LocalFree( waveFormatChoose );

  return mmr;
}

bool inline TMP3Reader::IsFrameValid(const char* pbuff) 
{

}

} //namespace AudioReader

