//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ProgressForm.h"
#include "Main.h"
#include "gnugettext.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProgressFrm *ProgressFrm;
//---------------------------------------------------------------------------
__fastcall TProgressFrm::TProgressFrm(TComponent* Owner)
        : TForm(Owner)
{
  TranslateComponent(this);
}
//---------------------------------------------------------------------------

void __fastcall TProgressFrm::Button1Click(TObject *Sender)
{
  Stop = true;
  Close();        
}
//---------------------------------------------------------------------------
void __fastcall TProgressFrm::FormShow(TObject *Sender)
{
  BPMain->Enabled = false;        
}
//---------------------------------------------------------------------------

void __fastcall TProgressFrm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  BPMain->Enabled = true;        
}
//---------------------------------------------------------------------------

