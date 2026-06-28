//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#include <mmsystem.h>
#include <Dialogs.hpp>
#include <AppEvnts.hpp>
#include <ComCtrls.hpp>
#include <ImgList.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include "ThemeMgr.hpp"
#include <Buttons.hpp>


#include <vector>
#include "buffer_.h"
#include <algorithm>

#include "akrip32.h"
#include "TReader.h"

//---------------------------------------------------------------------------
enum WindowRamp{wRampUp,wRampDown};

enum TPlayState{psEnded, psPaused, psPlaying, psQuit};

namespace AR = AudioReader;

class TBPMain : public TForm
{
__published:	// IDE-managed Components
        TOpenDialog *OpenDialog;
        TButton *RefreshCDBtn;
        TCheckBox *CheckCDDB;
        TButton *AboutBtn;
        TListView *TrackList;
        TLabel *Label5;
        TLabel *Label3;
        TComboBox *DriveList;
        TLabel *Label6;
        TBevel *Bevel1;
        TLabel *Label4;
        TTrackBar *CueBar;
        TButton *HelpBtn;
        TApplicationEvents *ApplicationEvents1;
        TSaveDialog *SaveDialog;
        TPageControl *PageControl1;
        TTabSheet *TabSheet1;
        TLabel *Label2;
        TLabel *Label1;
        TLabel *Label7;
        TLabel *Label8;
        TLabel *PitchLbl;
        TLabel *Label9;
        TLabel *FineLbl;
        TScrollBar *SpeedCtrl;
        TComboBox *AlgoBox;
        TScrollBar *FineCtrl;
        TScrollBar *PitchCtrl;
        TCheckBox *FilterBox;
        TTabSheet *TabSheet2;
        TCheckBox *KaraokeBox;
        TScrollBar *BassBar;
        TThemeManager *ThemeManager1;
        TLabel *BassLabel;
        TScrollBar *PositionBar;
        TLabel *Label12;
        TLabel *Label13;
        TScrollBar *HighBar;
        TLabel *HighLabel;
        TLabel *Label14;
        TLabel *Label16;
        TLabel *Label17;
        TImageList *ImList;
        TPopupMenu *TrackListMenu;
        TMenuItem *Selplaytrack;
        TListView *Messages;
        TMenuItem *RefreshCD1;
        TLabel *TimeLabel;
        TShape *Shape1;
        TButton *OpenFileBtn;
        TGroupBox *GrpLoopControls;
        TButton *LoopStartBtn;
        TButton *LoopEndBtn;
        TBevel *Bevel2;
        TEdit *EdtLoopStartMin;
        TEdit *EdtLoopStartFrame;
        TLabel *Label15;
        TLabel *Label18;
        TEdit *EdtLoopStartSec;
        TUpDown *UDLoopStartFrame;
        TUpDown *UDLoopStartMin;
        TUpDown *UDLoopStartSec;
        TEdit *EdtLoopEndMin;
        TUpDown *UDLoopEndMin;
        TLabel *Label19;
        TEdit *EdtLoopEndSec;
        TUpDown *UDLoopEndSec;
        TLabel *Label20;
        TEdit *EdtLoopEndFrame;
        TUpDown *UDLoopEndFrame;
        TCheckBox *LoopCheck;
        TLabel *Start;
        TLabel *End;
        TButton *SaveBtn;
        TScrollBar *VolumeBar;
        TLabel *Label11;
        TLabel *Label21;
        TLabel *Label22;
        TLabel *Label23;
        TButton *LangBtn;
        TBitBtn *BitBtn1;
        TBitBtn *BitBtn2;
        TBitBtn *BitBtn3;
        TBitBtn *PrevBtn;
        TBitBtn *NextBtn;
        TBitBtn *SelectTrackBtn;
        TBitBtn *PauseBtn;
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall SpeedCtrlChange(TObject *Sender);
        void __fastcall RefreshCDBtnClick(TObject *Sender);
        void __fastcall AboutBtnClick(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall HelpBtnClick(TObject *Sender);
        void __fastcall PitchCtrlChange(TObject *Sender);
        void __fastcall FineCtrlChange(TObject *Sender);
        void __fastcall OpenFileClick(TObject *Sender);
        void __fastcall LoopStartBtnClick(TObject *Sender);
        void __fastcall LoopEndBtnClick(TObject *Sender);
        void __fastcall CancelLoopBtnClick(TObject *Sender);
        bool __fastcall ApplicationEvents1Help(WORD Command, int Data,
          bool &CallHelp);
        void __fastcall SaveBtnClick(TObject *Sender);
        void __fastcall BassBarChange(TObject *Sender);
        void __fastcall PositionBarChange(TObject *Sender);
        void __fastcall HighBarChange(TObject *Sender);
        void __fastcall TrackListDblClick(TObject *Sender);
        void __fastcall SelplaytrackClick(TObject *Sender);
        void __fastcall TrackListMenuPopup(TObject *Sender);
        void __fastcall RefreshCD1Click(TObject *Sender);
        void __fastcall VolumeBarChange(TObject *Sender);
        void __fastcall UDLoopStartFrameChangingEx(TObject *Sender,
          bool &AllowChange, short NewValue, TUpDownDirection Direction);
        void __fastcall UDLoopStartSecChangingEx(TObject *Sender,
          bool &AllowChange, short NewValue, TUpDownDirection Direction);
        void __fastcall UDLoopStartMinChangingEx(TObject *Sender,
          bool &AllowChange, short NewValue, TUpDownDirection Direction);
        void __fastcall LoopCheckClick(TObject *Sender);
        void __fastcall UDLoopEndFrameChangingEx(TObject *Sender,
          bool &AllowChange, short NewValue, TUpDownDirection Direction);
        void __fastcall UDLoopEndSecChangingEx(TObject *Sender,
          bool &AllowChange, short NewValue, TUpDownDirection Direction);
        void __fastcall UDLoopEndMinChangingEx(TObject *Sender,
          bool &AllowChange, short NewValue, TUpDownDirection Direction);
        void __fastcall EdtLoopStartMinExit(TObject *Sender);
        void __fastcall LangBtnClick(TObject *Sender);
        void __fastcall BitBtn1Click(TObject *Sender);
        void __fastcall BitBtn2Click(TObject *Sender);
        void __fastcall BitBtn3Click(TObject *Sender);
        void __fastcall FormKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall PrevBtnClick(TObject *Sender);
        void __fastcall NextBtnClick(TObject *Sender);
        void __fastcall PauseBtnClick(TObject *Sender);
        void __fastcall SelectTrackBtnClick(TObject *Sender);
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall FormShortCut(TWMKey &Msg, bool &Handled);

private:	// User declarations

        //If dragging the thumb, DraggingCue = true;
        bool DraggingCue, restarting;
        //This keeps track of the position in the song
        unsigned int SamplesPlayed;
        
        THintWindow* CueBarHint; //For seeking time
        int cbBuffer;
        char* buffer; //Output buffer

        WAVEHDR whdrs[4];

        int SamplesUsed; //Needed for progress bar and time display
                         //Set in WaveOutNewBlock, used in OnWOMDone

        __int64 LoopA, LoopB;

        TPlayState PlayState;

        AR::TReader* DataReader;

        //CD read stuff
        HCDROM hCD;
        TOC toc;
        CDLIST cdlist;
        DWORD track;
        void SetLoopStartClock(__int64 SamplePos);
        void SetLoopEndClock(__int64 SamplePos);
        void PlaySong(__int64 StartSample);
        void ReleaseSongResources();

        
public:		// User declarations
        __fastcall TBPMain(TComponent* Owner);
        void AddMessage(String Msg);
        void AddMessage(const char* Msg);
        void UpdateTime(int nSamplesUsed);
        //void StartCDPlay(int track, int StartSample);
        void StartNewSong(int StartSample);
        void WaveOutNewBlock(WAVEHDR* pwhdr);
protected:

        BEGIN_MESSAGE_MAP
                VCL_MESSAGE_HANDLER(MM_WOM_DONE,TMessage, OnWomDone)
                VCL_MESSAGE_HANDLER(WM_HSCROLL,TWMHScroll,OnWMHScroll)
                VCL_MESSAGE_HANDLER(WM_DROPFILES,TWMDropFiles,OnWMDropFiles)
        END_MESSAGE_MAP(TForm)
        void __fastcall OnWomDone(TMessage& Msg);
        void __fastcall OnWMHScroll(TWMHScroll& Msg);
        void __fastcall OnWMDropFiles(TWMDropFiles& Msg);
        HWAVEOUT    hWaveOut;
        
        WAVEFORMATEX wf;
        unsigned int WindowLength;
        bool LoopOn, IsPaused;

};
//---------------------------------------------------------------------------
extern PACKAGE TBPMain *BPMain;



#endif
