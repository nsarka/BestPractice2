//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "AboutBoxX.h"
#include "logo.h"
#include "version.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAboutX *AboutX;

#define TEXT_AKRIP "akrip32 library (c) Jay A. Key"
#define LINK_AKRIP "http://akrip.sourceforge.net"
#define TEXT_BP    "BestPractice " BP_VERSION
#define LINK_BP    "http://www.xs4all.nl/~mp2004/bp"
//---------------------------------------------------------------------------
__fastcall TAboutX::TAboutX(TComponent* Owner)
        : TForm(Owner)
{
 HRSRC rs = FindResource(GetModuleHandle(NULL),MAKEINTRESOURCE(ID_LOGO),"LOGO");
 HGLOBAL bmp = (rs) ? LoadResource(NULL,rs) : NULL;
 bfh =  (bmp) ? (BITMAPFILEHEADER*)LockResource(bmp) : NULL;

 TitleLbl->Caption = TEXT_BP;
 AKRipLbl->Caption = TEXT_AKRIP;

}
//---------------------------------------------------------------------------
void __fastcall TAboutX::Label1Click(TObject *Sender)
{
  TLabel* lbl = dynamic_cast<TLabel*>(Sender);

  lbl->Font->Color = clRed;
  Application->ProcessMessages();
  ShellExecute(NULL,"open",lbl->Caption.c_str(),NULL,NULL,SW_SHOWNORMAL);

  
}

void __fastcall TAboutX::FormClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TAboutX::Label3Click(TObject *Sender)
{
  Close();        
}
//---------------------------------------------------------------------------


void __fastcall TAboutX::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  Close();        
}
//---------------------------------------------------------------------------

void __fastcall TAboutX::FormPaint(TObject *Sender)
{
  if (!bfh) return;

  BITMAPINFO* bmi  = (BITMAPINFO*)(bfh+1);



  Canvas->Brush->Style = bsSolid;
  Canvas->Pen->Color = clPurple;
  Canvas->Pen->Style = psSolid;
  Canvas->Rectangle(0,0,Width,Height);
  //Canvas->Draw(9,16,bmp);

  if(bfh)
  SetDIBitsToDevice(Canvas->Handle,3,3,148,148, 0,0,0,148,(BYTE*)bfh+bfh->bfOffBits,
                    bmi, DIB_RGB_COLORS);
        
}
//---------------------------------------------------------------------------



