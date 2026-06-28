//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <assert.h>
#include <math.h>
#include <iterator>
#include <typeinfo>
#include <stdio.h>

#include "Main.h"
#include "AboutBoxX.h"
#include "CWSOLA.h"
#include "SampleData.h"
#include "IIRButter.h"
#include "ProgressForm.h"
#include "TCDTrackReader.h"
#include "TWaveReader.h"
#include "TKaraokeProcessor.h"
#include "TMP3Reader.h"
#include "TResampler.h"
#include <IniFiles.hpp>
#include "gnugettext.hpp"
#include "LangForm.h"
#include "hh.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "ThemeMgr"
#pragma resource "*.dfm"

#include "version.h"

#define INIFILE "bpsettings.ini"

#define SECTION_SETTINGS "Settings"
#define SETTING_DRIVE "Drive"
#define SETTING_CDDB "CheckCDDB"
#define SETTING_LANG "Language"


void MSB2DWORD( DWORD *d, BYTE *b )
{
  DWORD retVal;

  retVal = (DWORD)b[0];
  retVal = (retVal<<8) + (DWORD)b[1];
  retVal = (retVal<<8) + (DWORD)b[2];
  retVal = (retVal<<8) + (DWORD)b[3];

  *d = retVal;
}


TBPMain *BPMain;

CWOLA* wola=NULL;
CIIRButterLow filter(44100,8);
TKaraokeProcessor Karaoke;
TResampler Resampler;




//---------------------------------------------------------------------------
__fastcall TBPMain::TBPMain(TComponent* Owner)
        : TForm(Owner)
{
   hWaveOut = NULL;
   DataReader = NULL;
   buffer = NULL;
   DraggingCue = false;

   
   Caption = Caption + " " BP_VERSION;

   CueBarHint = new THintWindow(this);
   CueBarHint->Color = clInfoBk;

   //Set edit boxes for looping to ES_NUMBER
//  SetWindowLong(EdtLoopStartMin->Handle,GWL_STYLE,
//    GetWindowLong(EdtLoopStartMin->Handle,GWL_STYLE)|ES_NUMBER);
//  SetWindowLong(EdtLoopStartSec->Handle,GWL_STYLE,
//    GetWindowLong(EdtLoopStartSec->Handle,GWL_STYLE)|ES_NUMBER);
//  SetWindowLong(EdtLoopStartFrame->Handle,GWL_STYLE,
//    GetWindowLong(EdtLoopStartFrame->Handle,GWL_STYLE)|ES_NUMBER);
//
//  SetWindowLong(EdtLoopEndMin->Handle,GWL_STYLE,
//    GetWindowLong(EdtLoopEndMin->Handle,GWL_STYLE)|ES_NUMBER);
//  SetWindowLong(EdtLoopEndSec->Handle,GWL_STYLE,
//    GetWindowLong(EdtLoopEndSec->Handle,GWL_STYLE)|ES_NUMBER);
//  SetWindowLong(EdtLoopEndFrame->Handle,GWL_STYLE,
//    GetWindowLong(EdtLoopEndFrame->Handle,GWL_STYLE)|ES_NUMBER);

  UDACCEL accel[4]={{0,1},{2,5},{5,10},{10,50}};

  SendMessage(UDLoopStartMin->Handle,UDM_SETACCEL, 3, (LPARAM) accel  );
  SendMessage(UDLoopStartSec->Handle,UDM_SETACCEL, 3, (LPARAM) accel  );
  SendMessage(UDLoopStartFrame->Handle,UDM_SETACCEL, 4, (LPARAM) accel  );

  SendMessage(UDLoopEndMin->Handle,UDM_SETACCEL, 3, (LPARAM) accel  );
  SendMessage(UDLoopEndSec->Handle,UDM_SETACCEL, 3, (LPARAM) accel  );
  SendMessage(UDLoopEndFrame->Handle,UDM_SETACCEL, 4, (LPARAM) accel  );

  PlayState = psEnded;

}
//---------------------------------------------------------------------------

void __fastcall TBPMain::OnWMDropFiles(TWMDropFiles& Msg)
{
  char filename[MAX_PATH];
  DragQueryFile((void*)Msg.Drop,0,filename,MAX_PATH);
  DragFinish((void*)Msg.Drop);
  Msg.Result = 0;

  ReleaseSongResources();
    Application->ProcessMessages();
    try
    {
      if(ExtractFileExt(filename).UpperCase() == ".WAV")
        DataReader = new AR::TWaveReader(filename);
      else
        DataReader = new AR::TMP3Reader(filename);


    }
    catch(AR::ReaderErrors ErrCode)
    {
      PlaySound("SystemAsterisk", NULL, SND_ALIAS);

      if(ErrCode == AR::ErrCannotOpen)
        AddMessage(_("Cannot open the file requested! Is it in use?"));

      if(ErrCode == AR::ErrBadFile)
        AddMessage(_("The file does not seem to be a valid file!"));

      if(ErrCode == AR::ErrFormatNotSupported)
        AddMessage(_("Sorry, this file format is not supported."));

      if(ErrCode == AR::ErrNoCodec)
        AddMessage(_("Sorry, there is no codec available to decode MP3 files."));

      delete DataReader;
      DataReader = NULL;
      return;
    }
    AddMessage(_("Playing file: ") + ExtractFileName(filename));
    Karaoke.SetSampleFreq(DataReader->GetSampleFrequency());
    HighBar->Max = (DataReader->GetSampleFrequency()/2 < 20000)?
                  DataReader->GetSampleFrequency()/21:2000;
    //Erase selected track, if any
    for(int i = 0;i<TrackList->Items->Count;++i)
      TrackList->Items->Item[i]->StateIndex = -1;

    StartNewSong(0);
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::OnWomDone(TMessage& Msg)
{
        
        if(PlayState != psQuit)
          UpdateTime(SamplesUsed);

        WAVEHDR* pwhdr = (WAVEHDR*) Msg.LParam;

        waveOutUnprepareHeader(hWaveOut, pwhdr, sizeof(WAVEHDR));

        delete pwhdr->lpData;

        if(!hWaveOut || PlayState == psQuit)
         return;

        //Four possibilities:
        //a) Just play a new block of audio
        //b) End of the track and we are not looping: stop and display 'Replay'
        //c) End of the track and we are looping: replay from LoopA
        //d) The wavout device is reset - do nothing

        //ad b)
        if(PlayState == psEnded && !LoopOn)
        {
          PlayState = psQuit;
          waveOutReset(hWaveOut);
          Application->ProcessMessages();
          PlaySong(0);
          return;
        }
        //ad c)
        if(LoopOn) //Check whether we should restart again at the loop start
        {
          if(SamplesPlayed >= LoopB || PlayState == psEnded)
          {
            PlayState = psQuit;
            waveOutPause(hWaveOut);
            waveOutReset(hWaveOut);
            Application->ProcessMessages();  //Wait for the buffers to be freed
                                       //in OnWOMDone Msg handler

            SamplesPlayed = DataReader->Seek(LoopA);
            PlayState = psPlaying;
                                       
                                         for(int i=0;i<4;++i)
                                              {
                                                     WaveOutNewBlock(whdrs+i);
                                                          }


            waveOutRestart(hWaveOut);
            return;


          }
        }
        //if(PlayState == psPlaying)//ad a+c)
          WaveOutNewBlock(pwhdr);
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::FormClose(TObject *Sender, TCloseAction &Action)
{
  ReleaseSongResources();
  String IniPath = ExtractFilePath(Application->ExeName) + INIFILE;

  TIniFile* inifile = new TIniFile(IniPath);

  //Drive
  inifile->WriteInteger(SECTION_SETTINGS,SETTING_DRIVE,DriveList->ItemIndex);

  //CDDB setting
  inifile->WriteBool(SECTION_SETTINGS,SETTING_CDDB,CheckCDDB->Checked);

  //Audio Source
  /*inifile->WriteInteger(SECTION_SETTINGS, SETTING_SOURCE,
                       AudioSourceRadio->ItemIndex);  */

  inifile->WriteString(SECTION_SETTINGS,SETTING_LANG, GetCurrentLanguage());                       
  delete inifile;
}
//---------------------------------------------------------------------------

/*-------------------------------------------
Releases acquired resources
- audio device
- data reader
- CWOLA instance
-------------------------------------------*/
void TBPMain::ReleaseSongResources()
{
  PlayState = psQuit; //Do not restart as with psEnded
  waveOutReset(hWaveOut);
  Application->ProcessMessages(); //Process the WOM messages to
                                  //unprepare the whdrs
                                  //hWaveOut MUST BE VALID TO DO SO!

  //close audio device
  waveOutClose(hWaveOut);
  hWaveOut = NULL;

  //free datareader resources
  if(DataReader) delete DataReader;
  DataReader = NULL;

  //free the CWOLA instance
  delete wola;
  wola = NULL;

  LoopOn = false;
  LoopA = 0;
  LoopB = 0;
  
}
//---------------------------------------------------------------------------


void __fastcall TBPMain::SpeedCtrlChange(TObject *Sender)
{
  Label1->Caption = FormatFloat("#.0 %",SpeedCtrl->Position/10.0);
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::RefreshCDBtnClick(TObject *Sender)
{

  TrackList->Items->Clear();

  GETCDHAND cdh={0};


    ZeroMemory( &toc, sizeof(TOC) );
    //Open CD
    if(hCD) CloseCDHandle(hCD);

    int DriveIndex = DriveList->ItemIndex;

    cdh.size     = sizeof(GETCDHAND);
    cdh.ver      = 1;
    cdh.ha       = cdlist.cd[DriveIndex].ha;
    cdh.tgt      = cdlist.cd[DriveIndex].tgt;
    cdh.lun      = cdlist.cd[DriveIndex].lun;
    cdh.readType  = CDR_ANY;      // set for autodetect

    hCD =  GetCDHandle( &cdh );

    if ( !hCD )
      {
        AddMessage("CD-ROM player not found");
        return;
      }
    //ModifyCDParms( hCD, CDP_MSF, FALSE );

    if ( ReadTOC( hCD, &toc, FALSE ) != SS_COMP )
      {
        AddMessage(_("Unable to read Table of Contents"));
        return ;
      }

    if(CheckCDDB->Checked)
    {
      CDDBQUERYITEM qi;
      CDDBQUERY query;

      ZeroMemory( &qi, sizeof(qi) );
      query.num = 1;  // set to the length of qi
      query.q   = &qi;
      CDDBSetOption( CDDB_OPT_SERVER, "www.freedb.org", 0 );
      CDDBSetOption( CDDB_OPT_CGI, "/~cddb/cddb.cgi", 0 );
      CDDBSetOption( CDDB_OPT_USER, "user@akrip.sourceforge.net", 0 );
      CDDBSetOption( CDDB_OPT_AGENT, "BestPractice " BP_VERSION, 0 );

      AddMessage(_("Querying wwww.freedb.org ..."));

      CDDBQuery( hCD, &query );

      if ( query.num > 0 )
      {
        const int DiskInfoSize = 4096;
        char DiskInfo[DiskInfoSize];

        if(CDDBGetDiskInfo(&qi,DiskInfo,DiskInfoSize) == SS_COMP)
        {
          TStringList * CDData = new TStringList;
          CDData->Text =  String(DiskInfo);

          for(int i=toc.firstTrack;i <= toc.lastTrack && !(toc.tracks[i-toc.firstTrack].ADR&0x4);++i)
          {
            TListItem* li = TrackList->Items->Add();
            li->Caption = IntToStr(i);
            li->SubItems->Add(CDData->Values[String("TTITLE"+IntToStr(i-toc.firstTrack))]);
            li->ImageIndex = 0;

          }

          Label4->Caption = CDData->Values["DTITLE"];
          delete CDData;
        }
        else
        {
          AddMessage(_("Something went wrong, no disc data available"));
          for(int i=toc.firstTrack;i <= toc.lastTrack && !(toc.tracks[i-toc.firstTrack].ADR&0x4);++i)
          {
            TListItem* li = TrackList->Items->Add();
            li->Caption = IntToStr(i);
            li->SubItems->Add(_("N/A"));
          }
          Label4->Caption = _("N/A");
        }

      }
      else
      {
        AddMessage(_("No disc data available")) ;
        for(int i=toc.firstTrack;i <= toc.lastTrack && !(toc.tracks[i-toc.firstTrack].ADR&0x4);++i)
        {
          TListItem* li = TrackList->Items->Add();
          li->Caption = IntToStr(i);
          li->SubItems->Add(_("N/A"));
        }
        Label4->Caption = _("N/A");
      }
    }
    else
    {

      for(int i=toc.firstTrack;i <= toc.lastTrack && !(toc.tracks[i-toc.firstTrack].ADR&0x4);++i)
      {
        TListItem* li = TrackList->Items->Add();
        li->Caption = IntToStr(i);
        li->SubItems->Add(_("N/A"));
      }
      Label4->Caption = _("N/A");

    }
    SelectTrackBtn->Enabled = true;

}
//---------------------------------------------------------------------------

void __fastcall TBPMain::AboutBtnClick(TObject *Sender)
{
  AboutX->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::FormCreate(TObject *Sender)
{
  TP_GlobalIgnoreClass(__classid(TFont));
  String IniPath = ExtractFilePath(Application->ExeName) + INIFILE;

  TIniFile* inifile = new TIniFile(IniPath);

  //This MUST be before other settings, as the comboboxes are changed!!
  UseLanguage(inifile->ReadString(SECTION_SETTINGS,SETTING_LANG,""));
  TranslateComponent(this);

  DragAcceptFiles(Handle,TRUE);
  AlgoBox->ItemIndex = 2;
  PageControl1->ActivePageIndex = 0;
  ZeroMemory(&cdlist,sizeof(cdlist));
  GetCDList( &cdlist );



  if ( cdlist.num == 0 )
  {
    RefreshCDBtn->Enabled = false;


    AddMessage(_("No CD-ROM drives were found! Cd reading is disabled."));


  }
  else
  {
    //Only CD related stuff from here on!!
    for (int index = 0;index < cdlist.num; ++index)
      DriveList->Items->Add(cdlist.cd[index].id );

    // ---- Get Settings ----
    //Get the last used drive, but check that it exists
    int drive  = inifile->ReadInteger(SECTION_SETTINGS,SETTING_DRIVE,0);
    if (drive >= DriveList->Items->Count || drive < 0) drive = 0;

    DriveList->ItemIndex = drive;

    //Get CDDB setting
    CheckCDDB->Checked = inifile->ReadBool(SECTION_SETTINGS,SETTING_CDDB,true);
  }

  delete inifile;

  PitchLbl->Caption = "0" + _(" semitone(s)");
  FineLbl->Caption = "0" + _(" cents");
  ImList->GetBitmap(1,PauseBtn->Glyph);
}
//---------------------------------------------------------------------------

void TBPMain::AddMessage(String Msg)
{
  TListItem* li = Messages->Items->Insert(0);
  li->Caption = TimeToStr(Time());
  li->SubItems->Add(Msg);
}
//---------------------------------------------------------------------------

inline void TBPMain::AddMessage(const char* Msg)
{
    AddMessage(String(Msg));
}
//---------------------------------------------------------------------------

inline void TBPMain::UpdateTime(int nSamplesUsed)
{


  SamplesPlayed += nSamplesUsed; //Update our counter

  
  if(!DraggingCue) //No need to update the position if the user is dragging
                   //the thumb
  {
    CueBar->Position = SamplesPlayed/DataReader->GetSampleFrequency();
  }

  //Update the timer
  unsigned int seconds = SamplesPlayed/DataReader->GetSampleFrequency();
  unsigned int minute = seconds/60;
  seconds = seconds - minute*60;
  unsigned int hour = minute/60;
  minute = minute-hour*60;

  TimeLabel->Caption = TimeLabel->Caption.sprintf("%1d:%02d:%02d",hour,minute,seconds);

}
//---------------------------------------------------------------------------


void __fastcall TBPMain::OnWMHScroll(TWMHScroll& Msg)
{
  static unsigned int seconds, minute, hour;

  if(Msg.ScrollBar != CueBar->Handle)
  {
   TForm::Dispatch(&Msg);
   return;
  }

  if(Msg.ScrollCode == TB_THUMBTRACK)
  {
    //Do not allow moving when not playing
    if(PlayState == psEnded) SendMessage(CueBar->Handle,TBM_SETPOS,TRUE,0);

    DraggingCue = true;

    //Update the Seek timer. Get the indicated number of seconds
    seconds = SendMessage(CueBar->Handle,TBM_GETPOS,0,0);

    minute = seconds/60;
    seconds = seconds - minute*60;
    hour = minute/60;
    minute = minute-hour*60;

    String HintString;
    TPoint p(TimeLabel->Left,TimeLabel->Top);
    p = ClientToScreen(p);
        //GetCursorPos(&p1);



    HintString = HintString.sprintf("%1d:%02d:%02d",hour,minute,seconds);
    RECT r = CueBarHint->CalcHintRect(1000,HintString,NULL);
    OffsetRect(&r,TimeLabel->Width+16+p.x,p.y);

    CueBarHint->ActivateHint(r,HintString);
    return;
  }

  if(Msg.ScrollCode == TB_THUMBPOSITION)
  {
    DraggingCue = false;   //Makes sure that the position bar is updated again
                           //See OnWomDone
    CueBarHint->ReleaseHandle();

    int SampleStart = SendMessage(CueBar->Handle,TBM_GETPOS,0,0);

    //SampleStart  = (hour*60+minute)*60 + seconds;
    SampleStart *= DataReader->GetSampleFrequency();
    if(PlayState != psEnded)
    {


      //Set the total of 'played' samples for looping and progress display
      SamplesPlayed = DataReader->Seek(SampleStart);


      //TPlayState pstate_old = PlayState;
      PlayState = psQuit; //Force OnWomDone to do nothing 
      waveOutPause(hWaveOut);
      waveOutReset(hWaveOut);
      Application->ProcessMessages();  //Wait for the buffers to be freed
                                       //in OnWOMDone Msg handler
      waveOutPause(hWaveOut);
      for(int i=0;i<4;++i)
         {
           WaveOutNewBlock(whdrs+i);
         }

      if(!IsPaused)
      {
        waveOutRestart(hWaveOut);
        PlayState = psPlaying;
      }
      else
       PlayState = psPaused;

      //Do not move this line upwards, will call this function again
      //and cause havoc with pstate_old
      CueBar->Position = SamplesPlayed/DataReader->GetSampleFrequency();
      
    }
    else
      //Prevent moving of thumb when not playing
      SendMessage(CueBar->Handle,TBM_SETPOS,TRUE,0);

    return;
  }


}
//---------------------------------------------------------------------------

/*-------------------------------------------
Set things up for this song:
 - set hWaveOut
 - CWOLA* wola is initialized
 - player controls are enabled
 - set cue bar maximum position (in seconds)
-------------------------------------------*/
void TBPMain::StartNewSong(int StartSample)
{
       //Prepare this for the waveOut device
       WAVEFORMATEX wf;
       wf.nSamplesPerSec = DataReader->GetSampleFrequency();
       wf.nChannels = 2;
       wf.nBlockAlign = 4;
       wf.wFormatTag = WAVE_FORMAT_PCM;
       wf.cbSize = sizeof(wf);
       wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nBlockAlign;
       wf.wBitsPerSample = 16;

       //Close existing device, if any, and try to open device
       if(hWaveOut)
        {
          waveOutReset(hWaveOut);
          waveOutClose(hWaveOut);
          hWaveOut = NULL;
        }

       if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wf,(DWORD)Handle, 0L,
                       CALLBACK_WINDOW ) != MMSYSERR_NOERROR)
       {
            AddMessage("Failed to open waveform output device!");
            delete DataReader;
            DataReader = NULL;

            return;
       }

       //datasize == number of samples
       DWORD datasize = DataReader->GetLength();

       if (datasize)
       {
         //Max = max number samples
         CueBar->Max = datasize/DataReader->GetSampleFrequency();

         int WindowLength = wf.nSamplesPerSec/10; //Equals 100 ms of sound
                                              //is half the total window size

         //Set up size of buffers to use
         cbBuffer = 2*WindowLength*4; //Output buffer size, 200 ms


         //Only called once per song
         if(wola) delete wola;

         wola = new CWOLA(WindowLength,cbBuffer,WindowLength/4,DataReader);


         //Enable the loop controls
         GrpLoopControls->Enabled = true;
         LoopStartBtn->Enabled = true;
         LoopEndBtn->Enabled = true;

         //Enable karaoke controls
         PositionBar->Enabled = true;
         BassBar->Enabled = true;
         HighBar->Enabled = true;

         //Set the clocks to 0
         SetLoopStartClock(0);
         SetLoopEndClock(0);

         LoopOn = false;
         LoopCheck->Checked = false;
         LoopA = LoopB = 0;
         SaveBtn->Enabled = true;
         PauseBtn->Enabled = true;

         //Set volume bar
         DWORD dwVolume;
         waveOutGetVolume(hWaveOut,&dwVolume);
         VolumeBar->Position = dwVolume&0xFF;

         if (PlayState == psPaused)
         {
           waveOutPause(hWaveOut);
         }
         else
         {
           PlayState = psPlaying;
           CueBar->Enabled = true;
           PauseBtn->Caption = "";
           ImList->GetBitmap(1,PauseBtn->Glyph);
         }
         PlaySong(StartSample);
       }
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::HelpBtnClick(TObject *Sender)
{
  if(HHCtrlHandle)
  {
    String Path = ExtractFilePath(Application->ExeName) + "bphelp.chm";
    HtmlHelp(Handle,Path.c_str(),HH_DISPLAY_TOPIC,0);
  }
  else
  {
    PlaySound("SystemAsterisk", NULL, SND_ALIAS);
    AddMessage(_("HTML Help control not found. Please check that it is installed"));
  }
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::PitchCtrlChange(TObject *Sender)
{
  PitchLbl->Caption = (PitchCtrl->Position > 0)? String("+")+
                                IntToStr(PitchCtrl->Position)+ _(" semitone(s)")
                                        :IntToStr(PitchCtrl->Position)+_(" semitone(s)");
}
//---------------------------------------------------------------------------


void __fastcall TBPMain::FineCtrlChange(TObject *Sender)
{
  if( FineCtrl->Position > 0) FineLbl->Caption = "+" +
                IntToStr(FineCtrl->Position) + _(" cents");
  else
    FineLbl->Caption =  IntToStr(FineCtrl->Position) + _(" cents");
}
//---------------------------------------------------------------------------

void TBPMain::WaveOutNewBlock(WAVEHDR* pwhdr)
{
       char* outbuffer = new char[cbBuffer];
       float totaldetune = (float)PitchCtrl->Position + (float) FineCtrl->Position/100.0;
        float scale = pow(2,-totaldetune/12.0);

        if(AlgoBox->ItemIndex)
        {
          wola->SetMode(tsmWSOLA);
          if( AlgoBox->ItemIndex == 1)
            wola->SetMode(wxmFast);
          else
            wola->SetMode(wxmGood);
        }
        else
        {
          wola->SetMode(tsmWOLA);
        }

        assert(!DataReader->Eof());
        
        SamplesUsed = wola->ProcessBlock(SpeedCtrl->Position*scale,
                                outbuffer)/4;
        

        if (DataReader->Eof())
        {
          PlayState = psEnded;
        }

        SampleData* sd = reinterpret_cast<SampleData*>(outbuffer);

        if(KaraokeBox->Checked)
        {
          //Karaoke.SetBassPassThrough(BassBar->Position*10);
          Karaoke.ProcessAudio(sd,cbBuffer/4);
          
        }


        char* ResampleBuffer;

        if(fabs(scale-1) > 1e-5) //Scaling active
        {
          ResampleBuffer = new char[1.1*scale*cbBuffer + 4];

          //if(FilterBox->Checked )
            //filter.SetOrder(FilterBar->Position*4);


          if(scale < 1) //pitch shift up - check for anti-alias filter
          {
            if(FilterBox->Checked ) //Only filter if filter is on
            {
              //First we calculate the maximum allowable frequency

              float maxfreq = DataReader->GetSampleFrequency()/2*scale;

              //Since it is an antialias filter, we want to have
              //12 dB suppresion of the alias instead of
              //Recalculate the -3dB point
              //1.4 gives 10 x suppresion
              double cutoff = maxfreq / 2;

              filter.SetCutOff(cutoff);
              char* outextra = new char[cbBuffer];



              //outbuffer contains WSOLA processed audio
              //outextra will contain the filtered WSOLA audio
              filter.Filter(outbuffer,outextra,cbBuffer>>2);

              //original data is not necessary anymore
              delete outbuffer;

              //outextra goes out of scope --> assign to outbuffer
              outbuffer = outextra;
            }
          }

          //Resample. Audio is either prefiltered (scale < 1) or needs to be
          //post-processed
          pwhdr->dwBufferLength =
                        4*Resampler.Resample(ResampleBuffer,outbuffer,scale,cbBuffer);

          //Only resampled audio is needed
          delete outbuffer;

          pwhdr->lpData = ResampleBuffer;

        }
        else //No pitch scaling
        {
          pwhdr->lpData = outbuffer;
          pwhdr->dwBufferLength = cbBuffer;
        }

        //Make this audio block shorter if we are at the end of the loop
        if(SamplesUsed + SamplesPlayed > LoopB && LoopOn)
        {
          int ToPlay = LoopB - SamplesPlayed;

          pwhdr->dwBufferLength *= ToPlay*4;
          pwhdr->dwBufferLength /= SamplesUsed*4;
          SamplesUsed = SamplesPlayed;
        }

        if(waveOutPrepareHeader(hWaveOut, pwhdr, sizeof(WAVEHDR)) !=MMSYSERR_NOERROR)
          AddMessage("Prepare failed");


        // Now the data block can be sent to the output device. The
        // waveOutWrite function returns immediately and waveform
        // data is sent to the output device in the background.
        int wResult = waveOutWrite(hWaveOut, pwhdr, sizeof(WAVEHDR));
        if (wResult != 0)
        {
            waveOutUnprepareHeader(hWaveOut, pwhdr, sizeof(WAVEHDR));

        }

}
//---------------------------------------------------------------------------



void __fastcall TBPMain::OpenFileClick(TObject *Sender)
{

    if(OpenDialog->Execute())
    {
          ReleaseSongResources();
          Application->ProcessMessages();
          try
          {
            if(ExtractFileExt(OpenDialog->FileName).UpperCase() == ".WAV")
              DataReader = new AR::TWaveReader(OpenDialog->FileName.c_str());
            else
              DataReader = new AR::TMP3Reader(OpenDialog->FileName.c_str());


          }
          catch(AR::ReaderErrors ErrCode)
          {
            PlaySound("SystemAsterisk", NULL, SND_ALIAS);

            if(ErrCode == AR::ErrCannotOpen)
              AddMessage(_("Cannot open the file requested! Is it in use?"));

            if(ErrCode == AR::ErrBadFile)
              AddMessage(_("The file does not seem to be a valid file!"));

            if(ErrCode == AR::ErrFormatNotSupported)
              AddMessage(_("Sorry, this file format is not supported."));

            if(ErrCode == AR::ErrNoCodec)
              AddMessage(_("Sorry, there is no codec available to decode MP3 files."));

            delete DataReader;
            DataReader = NULL;
            return;
          }
          AddMessage(_("Playing file: ") + ExtractFileName(OpenDialog->FileName));
          Karaoke.SetSampleFreq(DataReader->GetSampleFrequency());
          HighBar->Max = (DataReader->GetSampleFrequency()/2 < 20000)?
                        DataReader->GetSampleFrequency()/21:2000;
          //Erase selected track, if any
          for(int i = 0;i<TrackList->Items->Count;++i)
            TrackList->Items->Item[i]->StateIndex = -1;
          NextBtn->Enabled = false;
          PrevBtn->Enabled = false;
          StartNewSong(0);
    }
    

}
//---------------------------------------------------------------------------

void __fastcall TBPMain::LoopStartBtnClick(TObject *Sender)
{
 LoopA = SamplesPlayed;

 SetLoopStartClock(LoopA);


}
//---------------------------------------------------------------------------

void __fastcall TBPMain::LoopEndBtnClick(TObject *Sender)
{


  if(SamplesPlayed < LoopA)
  {
    AddMessage(_("Loop end must be after loop start."));
    return;
  }


 LoopB = SamplesPlayed;
 SetLoopEndClock(LoopB);
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::CancelLoopBtnClick(TObject *Sender)
{
  //LoopAIsDefined = false;
  LoopOn = false;

  LoopStartBtn->Enabled = true;
  //LoopEndBtn->Enabled = true;
 //CancelLoopBtn->Enabled = false;
  AddMessage("Loop Cancelled");
}
//---------------------------------------------------------------------------

bool __fastcall TBPMain::ApplicationEvents1Help(WORD Command, int Data,
      bool &CallHelp)
{
  CallHelp = false;
  if(Command == HELP_CONTEXTPOPUP)
  {
    String Path = ExtractFilePath(Application->ExeName) + "bppopup.hlp";
    WinHelp( Application->Handle, Path.c_str(), HELP_CONTEXTPOPUP, Data);
  }
  return true;
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::SaveBtnClick(TObject *Sender)
{
  if(SaveDialog->Execute())
  {

  HMMIO outfile = mmioOpen(SaveDialog->FileName.c_str(),NULL,MMIO_CREATE|MMIO_WRITE|MMIO_EXCLUSIVE);
  if(!outfile)
  {
    AddMessage(_("Could not open the output file!"));
    return;
  }
        WAVEFORMATEX wf;
       wf.nSamplesPerSec = DataReader->GetSampleFrequency();
       wf.nChannels = 2;
       wf.nBlockAlign = 4;
       wf.wFormatTag = WAVE_FORMAT_PCM;
       wf.cbSize = sizeof(wf);
       wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nBlockAlign;
       wf.wBitsPerSample = 16;


       int WindowLength = wf.nSamplesPerSec/10; //Equals 100 ms of sound
                                              //is half the total window size
  MMCKINFO ck1,ck2;
  ck1.fccType = mmioFOURCC('W','A','V','E');
  ck2.ckid = mmioFOURCC('f','m','t',' ');

  mmioCreateChunk(outfile, &ck1,MMIO_CREATERIFF);
  mmioCreateChunk(outfile, &ck2,0);
  mmioWrite(outfile,(const char*)&wf,sizeof(wf));
  mmioAscend(outfile,&ck2,0);

  ck2.ckid = mmioFOURCC('d','a','t','a');
  mmioCreateChunk(outfile,&ck2,0);


  int loopstart  = LoopA, loopend = LoopB;
  AR::TReader* areader;
  bool karaoke = KaraokeBox->Checked;
  bool aliasfilter = FilterBox->Checked;
  TKaraokeProcessor kr;

  kr.SetBassPassThrough(BassBar->Position*10);
  kr.SetHighPassThrough(DataReader->GetSampleFrequency()/2 - HighBar->Position*10);
  kr.SetVocalPosition(PositionBar->Position);

  if(hWaveOut) waveOutPause(hWaveOut);


  areader = DataReader->MakeCopy();

  if (!LoopOn){
   loopstart = 0;
   loopend = areader->GetLength();
  }

  areader->Seek(loopstart);

  char* outbuffer = new char[cbBuffer];
  float totaldetune = (float)PitchCtrl->Position + (float) FineCtrl->Position/100.0;
  float scale = pow(2,-totaldetune/12.0);
  int speed = SpeedCtrl->Position;
  CWOLA* wola = new CWOLA(WindowLength,cbBuffer,WindowLength/4,areader);
        if(AlgoBox->ItemIndex)
        {
          wola->SetMode(tsmWSOLA);
          if( AlgoBox->ItemIndex == 1)
            wola->SetMode(wxmFast);
          else
            wola->SetMode(wxmGood);
        }
        else
        {
          wola->SetMode(tsmWOLA);
        }
  ProgressFrm = new TProgressFrm(this);
  ProgressFrm->PB->Min = 0;
  ProgressFrm->PB->Max = loopend; //areader->GetLength();
  ProgressFrm->PB->Position = 0;
  ProgressFrm->PercentLabel->Caption = "0 %";
  ProgressFrm->Stop = false;

  ProgressFrm->Show();

  char* ResampleBuffer = new char[1.1*scale*cbBuffer + 4];
  char* outextra = new char[cbBuffer];
  SampleData* sd = reinterpret_cast<SampleData*>(outbuffer);

  while(!areader->Eof() && !ProgressFrm->Stop && ProgressFrm->PB->Position < loopend)
  {

      char* finalbuffer = outbuffer;
      int bufflen;

      ProgressFrm->PB->Position = ProgressFrm->PB->Position +
                                  wola->ProcessBlock(speed*scale,
                                  outbuffer)/4;
      ProgressFrm->PercentLabel->Caption =
        IntToStr(ProgressFrm->PB->Position*100/ProgressFrm->PB->Max) + " %";
      Application->ProcessMessages();

      if(karaoke)
        {
          Karaoke.ProcessAudio(sd,cbBuffer/4);
        }




        if(fabs(scale-1) > 1e-5) //Scaling active
        {

          //finalbuffer = outbuffer; //if no filtering needed

          if(scale < 1 && aliasfilter) //pitch shift up - check for anti-alias filter
                                       //outextra will contain the filtered data
          {                            //finalbuffer will point to outextra
              //First we calculate the maximum allowable frequency

              float maxfreq = areader->GetSampleFrequency()/2*scale;

              //Since it is an antialias filter, we want to have
              //12 dB suppresion of the alias instead of
              //Recalculate the -3dB point
              //1.4 gives 10 x suppresion
              double cutoff = maxfreq / 2;

              filter.SetCutOff(cutoff);

              //outbuffer contains WSOLA processed audio
              //outextra will contain the filtered WSOLA audio
              filter.Filter(outbuffer,outextra,cbBuffer>>2);

              //assign to finalbuffer for resampling
              finalbuffer = outextra;
          }

          //Resample. Audio is either prefiltered (scale < 1) or needs to be
          //post-processed
          bufflen = 4*Resampler.Resample(ResampleBuffer,finalbuffer,scale,cbBuffer);

          finalbuffer = ResampleBuffer;

        }
        else //No pitch scaling
        {
          finalbuffer = outbuffer;
          bufflen = cbBuffer;
        }
        
       // Now the data block can be sent to the output file.
       mmioWrite(outfile,finalbuffer,bufflen);
  }

  mmioAscend(outfile,&ck2,0);
  mmioAscend(outfile,&ck1,0);

  mmioClose(outfile,0);
  delete outextra;
  delete outbuffer;
  delete ResampleBuffer;
  delete areader;

  ProgressFrm->Close();
  delete ProgressFrm;
  if(PlayState == psPlaying) waveOutRestart(hWaveOut);

  } //SaveDialog
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::BassBarChange(TObject *Sender)
{
   BassLabel->Caption = IntToStr(BassBar->Position*10) + " Hz";
   Karaoke.SetBassPassThrough(BassBar->Position*10);
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::PositionBarChange(TObject *Sender)
{
  Karaoke.SetVocalPosition(PositionBar->Position);
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::HighBarChange(TObject *Sender)
{
   HighLabel->Caption = IntToStr(HighBar->Position*10) + " Hz";
   Karaoke.SetHighPassThrough(DataReader->GetSampleFrequency()/2 - HighBar->Position*10);
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::TrackListDblClick(TObject *Sender)
{
  /*AudioSourceRadio->ItemIndex = 1;  */
      SelectTrackBtnClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::SelplaytrackClick(TObject *Sender)
{
  if(TrackList->ItemIndex > -1)
    TrackListDblClick(Sender);
}
//---------------------------------------------------------------------------


void __fastcall TBPMain::TrackListMenuPopup(TObject *Sender)
{
   if(TrackList->ItemIndex == -1)
     Selplaytrack->Visible = false;
   else
     Selplaytrack->Visible = true;


}
//---------------------------------------------------------------------------

void __fastcall TBPMain::RefreshCD1Click(TObject *Sender)
{
  RefreshCDBtnClick(Sender);        
}
//---------------------------------------------------------------------------



void __fastcall TBPMain::VolumeBarChange(TObject *Sender)
{
   if(hWaveOut)
   {
     DWORD volume = VolumeBar->Position;
     volume = (volume<<8)|volume;
     volume = (volume<<16)|volume;
     waveOutSetVolume(hWaveOut,volume);
   }
}
//---------------------------------------------------------------------------






void __fastcall TBPMain::UDLoopStartFrameChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
  __int64 Samples = DataReader->GetLength();

  LoopA = ((UDLoopStartMin->Position*60+UDLoopStartSec->Position)
                      * 100 + (__int64)NewValue)* DataReader->GetSampleFrequency();

  LoopA /= 100;
  if(LoopA > Samples)
  {
    LoopA = Samples;
    AllowChange = false;
  }
  else if (LoopA <0)
  {
    LoopA = 0;
    AllowChange = false;
  }

  SetLoopStartClock(LoopA);
  if(LoopB < LoopA)
  {
    LoopB = LoopA;
    SetLoopEndClock(LoopB);
  }

}
//---------------------------------------------------------------------------

void TBPMain::SetLoopStartClock(__int64 SamplePos)
{
 int sec = SamplePos/DataReader->GetSampleFrequency();
 int ss =  SamplePos % DataReader->GetSampleFrequency();

 int min = sec/60;
 ss = (ss*100)/DataReader->GetSampleFrequency();
 sec -= min*60;

 EdtLoopStartMin->Text = IntToStr(min);
 EdtLoopStartSec->Text = IntToStr(sec);
 EdtLoopStartFrame->Text = IntToStr(ss);
}
//---------------------------------------------------------------------------

void TBPMain::SetLoopEndClock(__int64 SamplePos)
{
 int sec = SamplePos/DataReader->GetSampleFrequency();
 int ss =  SamplePos % DataReader->GetSampleFrequency();

 int min = sec/60;
 ss = (ss*100)/DataReader->GetSampleFrequency();
 sec -= min*60;

 EdtLoopEndMin->Text = IntToStr(min);
 EdtLoopEndSec->Text = IntToStr(sec);
 EdtLoopEndFrame->Text = IntToStr(ss);
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::UDLoopStartSecChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
 __int64 Samples = DataReader->GetLength();

  LoopA = ((UDLoopStartMin->Position*60+(__int64)NewValue)
                      * 100 + UDLoopStartFrame->Position)* DataReader->GetSampleFrequency();

  LoopA /= 100;
  if(LoopA > Samples)
  {
    LoopA = Samples;
    AllowChange = false;
  }
  else if (LoopA <0)
  {
    LoopA = 0;
    AllowChange = false;
  }

  SetLoopStartClock(LoopA);
  if(LoopB < LoopA)
  {
    LoopB = LoopA;
    SetLoopEndClock(LoopB);
  }

}
//---------------------------------------------------------------------------

void __fastcall TBPMain::UDLoopStartMinChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
   __int64 Samples = DataReader->GetLength();

  LoopA = (((__int64)NewValue*60+UDLoopStartSec->Position)
                      * 100 + UDLoopStartFrame->Position)* DataReader->GetSampleFrequency();

  LoopA /= 100;
  if(LoopA > Samples)
  {
    LoopA = Samples;
    AllowChange = false;
  }
  else if (LoopA <0)
  {
    LoopA = 0;
    AllowChange = false;
  }

  SetLoopStartClock(LoopA);
  if(LoopB < LoopA)
  {
    LoopB = LoopA;
    SetLoopEndClock(LoopB);
  }
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::LoopCheckClick(TObject *Sender)
{
  if(LoopCheck->Checked)
  {
    LoopA =  ((UDLoopStartMin->Position*60+UDLoopStartSec->Position)
                        * 100 + UDLoopStartFrame->Position)* DataReader->GetSampleFrequency();
    LoopA /= 100;

    LoopB =  ((UDLoopEndMin->Position*60+UDLoopEndSec->Position)
                        * 100 + UDLoopEndFrame->Position)* DataReader->GetSampleFrequency();
    LoopB /= 100;

    if (LoopA >= LoopB)
    {
      LoopCheck->Checked = false;
      LoopOn = false;
      AddMessage(_("Loop start has to be before loop end!"));
      return;
    }

    LoopOn  = true;
    return;
  }

  LoopOn = false;
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::UDLoopEndFrameChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
  __int64 Samples = DataReader->GetLength();

  LoopB = ((UDLoopEndMin->Position*60+UDLoopEndSec->Position)
                      * 100 + (__int64)NewValue)* DataReader->GetSampleFrequency();

  LoopB /= 100;
  if(LoopB > Samples )
  {
   LoopB = Samples;
   AllowChange = false;
  }
  else if(LoopB < 0)
  {
    LoopB = 0;
    AllowChange = false;
  }
  
  SetLoopEndClock(LoopB);
  if(LoopB < LoopA)
  {
    LoopA = LoopB;
    SetLoopStartClock(LoopA);
  }
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::UDLoopEndSecChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
  __int64 Samples = DataReader->GetLength();

  LoopB = ((UDLoopEndMin->Position*60+(__int64)NewValue)
                      * 100 + UDLoopEndFrame->Position)* DataReader->GetSampleFrequency();

  LoopB /= 100;

  if(LoopB > Samples )
  {
   LoopB = Samples;
   AllowChange = false;
  }
  else if(LoopB < 0)
  {
    LoopB = 0;
    AllowChange = false;
  }

  SetLoopEndClock(LoopB);
  if(LoopB < LoopA)
  {
    LoopA = LoopB;
    SetLoopStartClock(LoopB);
  }

}
//---------------------------------------------------------------------------

void __fastcall TBPMain::UDLoopEndMinChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
  __int64 Samples = DataReader->GetLength();

  LoopB = (((__int64)NewValue*60+UDLoopEndSec->Position)
                      * 100 + UDLoopEndFrame->Position)* DataReader->GetSampleFrequency();

  LoopB /= 100;
  if(LoopB > Samples )
  {
   LoopB = Samples;
   AllowChange = false;
  }
  else if(LoopB < 0)
  {
    LoopB = 0;
    AllowChange = false;
  }

  SetLoopEndClock(LoopB);
  if(LoopB < LoopA)
  {
    LoopA = LoopB;
    SetLoopStartClock(LoopA);
  }
}
//---------------------------------------------------------------------------

/*-----------------------------------------
Plays the song from point StartSample

hardware must have been initialized by
StartNewSong. DataReader must be valid
Cuebar maximum is unchanged, only
position is reset. Starts the message loop
!!All whdrs must be free!! Do not use
during the message loop - reset first
------------------------------------------*/
void TBPMain::PlaySong(__int64 StartSample)
{
        //Seek to desired location (probably 0)
         DataReader->Seek(StartSample);
         PlayState = psPlaying;
         IsPaused = false;
         for(int i=0;i<4;++i)
         {
           WaveOutNewBlock(whdrs+i);
         }
         //for cuebar and looping
         SamplesPlayed = StartSample;
         CueBar->Enabled = true;
         CueBar->Position = StartSample/DataReader->GetSampleFrequency();

}
void __fastcall TBPMain::EdtLoopStartMinExit(TObject *Sender)
{
  EdtLoopStartMin->Text = UDLoopStartMin->Position;
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::LangBtnClick(TObject *Sender)
{

  //Save this because retranslation changes it
  int idx = AlgoBox->ItemIndex;

  TILangForm* lf = new TILangForm(this);
  if(lf->ShowModal() == mrOk)
  {
    RetranslateComponent(this);
    AlgoBox->ItemIndex = idx;
  }

  delete lf;

  //Make up for the reset of the textlabels
  PitchCtrlChange(Sender);
  FineCtrlChange(Sender);
  SpeedCtrlChange(Sender);
}
//---------------------------------------------------------------------------


void __fastcall TBPMain::BitBtn1Click(TObject *Sender)
{
  PitchCtrl->Position = 0;        
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::BitBtn2Click(TObject *Sender)
{
  FineCtrl->Position = 0;        
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::BitBtn3Click(TObject *Sender)
{
  SpeedCtrl->Position = 1000;        
}
//---------------------------------------------------------------------------







void __fastcall TBPMain::FormKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
   switch(Key)
   {
     case VK_NUMPAD0:  if(PauseBtn->Enabled)
                       {
                         PauseBtn->SetFocus();
                         PauseBtn->Click();
                       }
                       break;
     case VK_BACK: if(PauseBtn->Enabled)
                   {

                    if(PlayState != psEnded)
                    {
                      waveOutPause(hWaveOut);

                      SamplesPlayed = DataReader->Seek(0);
                      CueBar->Position = SamplesPlayed/DataReader->GetSampleFrequency();
                      waveOutReset(hWaveOut);

                      if(PlayState == psPaused)
                        waveOutPause(hWaveOut);
                      else
                        waveOutRestart(hWaveOut);

                    }
                   }

                   break;

   }
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::PrevBtnClick(TObject *Sender)
{
  int track;
  for(int i = 0;i<TrackList->Items->Count;++i)
     if (TrackList->Items->Item[i]->StateIndex == 0) track = i;

  if(track > 0)
  {
    TrackList->Items->Item[track-1]->Selected = true;
    SelectTrackBtn->Click();
  }

}
//---------------------------------------------------------------------------

void __fastcall TBPMain::NextBtnClick(TObject *Sender)
{
  int track;
  for(int i = 0;i<TrackList->Items->Count;++i)
     if (TrackList->Items->Item[i]->StateIndex == 0) track = i;

  if(track < TrackList->Items->Count-1)
  {
    TrackList->Items->Item[track+1]->Selected = true;
    SelectTrackBtn->Click();
  }

}
//---------------------------------------------------------------------------



void __fastcall TBPMain::PauseBtnClick(TObject *Sender)
{
  switch(PlayState)
  {
    case psPlaying:
                     waveOutPause(hWaveOut);
                     PlayState = psPaused;
                     ImList->GetBitmap(2,PauseBtn->Glyph);
                     PauseBtn->Invalidate();
                     IsPaused = true;
                     break;
    case psPaused:
                     waveOutRestart(hWaveOut);
                     PlayState = psPlaying;
                     ImList->GetBitmap(1,PauseBtn->Glyph);
                     PauseBtn->Invalidate();
                     IsPaused = false;
                     break;
    


  }
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::SelectTrackBtnClick(TObject *Sender)
{
  ReleaseSongResources();
    Application->ProcessMessages();
    int trk = TrackList->ItemIndex;

    if(TrackList->Items->Count > 0 && trk == -1) trk = 0;

    if(trk >= 0)
    {
        NextBtn->Enabled = true;
        PrevBtn->Enabled = true;
        DWORD dwStart, dwEnd;

        String TrackTitle = TrackList->Items->Item[trk]->Caption;
        if(TrackList->Items->Item[trk]->SubItems->Strings[0] != "N/A")
           TrackTitle += " - " + TrackList->Items->Item[trk]->SubItems->Strings[0];
        AddMessage(_("Playing CD Track: ") + TrackTitle);

          track = trk + 1; //For array
          MSB2DWORD( &dwStart, toc.tracks[track-1].addr );
          MSB2DWORD( &dwEnd, toc.tracks[track].addr );

          if (DataReader) delete DataReader;
          DataReader = new AR::TCDTrackReader(hCD,dwStart,dwEnd);
          Karaoke.SetSampleFreq(DataReader->GetSampleFrequency());
          HighBar->Max = (DataReader->GetSampleFrequency()/2 < 20000)?
                        DataReader->GetSampleFrequency()/21:2000;
          for(int i = 0;i<TrackList->Items->Count;++i)
            TrackList->Items->Item[i]->StateIndex = -1;
          TrackList->Items->Item[trk]->StateIndex = 0;
          StartNewSong(0);
    }        
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::Timer1Timer(TObject *Sender)
{
  FlashWindow(PauseBtn->Handle,true);        
}
//---------------------------------------------------------------------------

void __fastcall TBPMain::FormShortCut(TWMKey &Msg, bool &Handled)
{
  short Key = Msg.CharCode;
  TShiftState Shift;

  //Do not process any ALT shortcuts - these can be system commands
  if(GetAsyncKeyState(VK_MENU )& 0x8000){
   Handled = false;
   return;
  }

  if(GetAsyncKeyState(VK_SHIFT)) Shift<<ssShift;

  Handled = true; //if not handled, the handled = false below
   switch(Key)
  {
    case VK_F5: if (Shift.Contains(ssShift)) SpeedCtrl->Position = SpeedCtrl->Position - 100;
                else SpeedCtrl->Position = SpeedCtrl->Position + 100;
                break;

    case VK_F6: if (Shift.Contains(ssShift)) SpeedCtrl->Position = SpeedCtrl->Position - 10;
                else SpeedCtrl->Position = SpeedCtrl->Position + 10;
                break;

    case VK_F7: if (Shift.Contains(ssShift)) SpeedCtrl->Position = SpeedCtrl->Position - 1;
                else SpeedCtrl->Position = SpeedCtrl->Position + 1;
                break;

    case VK_F8: SpeedCtrl->Position = 1000;
                break;

    case VK_F2: if (Shift.Contains(ssShift)) PitchCtrl->Position = PitchCtrl->Position - 1;
                else PitchCtrl->Position = PitchCtrl->Position + 1;
                break;

    case VK_F3: if (Shift.Contains(ssShift)) FineCtrl->Position = FineCtrl->Position - 1;
                else FineCtrl->Position = FineCtrl->Position + 1;
                break;

    case VK_F4: FineCtrl->Position = 0;
                 PitchCtrl->Position = 0;
                 break;

    case VK_ADD: VolumeBar->Position = VolumeBar->Position + 4;
                 break;

    case VK_SUBTRACT: VolumeBar->Position = VolumeBar->Position - 4;
                 break;


     case 'W': if(NextBtn->Enabled)
               {
                 NextBtn->SetFocus();
                 NextBtn->Click();
               }
               break;
     case 'Q': if(PrevBtn->Enabled)
               {
                 PrevBtn->SetFocus();
                 PrevBtn->Click();
                 break;
               }
     case 'P': if(SelectTrackBtn->Enabled)
               {
                 SelectTrackBtn->SetFocus();
                 SelectTrackBtn->Click();
                 break;
               }

     case 'R': RefreshCDBtn->Click(); Key= 0;break;
     case 'M': KaraokeBox->Checked = !KaraokeBox->Checked; break;
     case 'A': PositionBar->Position = PositionBar->Position - 4; break;
     case 'S': PositionBar->Position = PositionBar->Position + 4; break;
     case 'F': BassBar->Position = BassBar->Position - 5; break;
     case 'G': BassBar->Position = BassBar->Position + 5; break;
     case 'J': HighBar->Position = HighBar->Position - 50; break;
     case 'K': HighBar->Position = HighBar->Position + 50; break;
     default: Handled = false;

   }

}
//---------------------------------------------------------------------------



