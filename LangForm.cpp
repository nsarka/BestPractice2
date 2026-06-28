//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "LangForm.h"
#include "gnugettext.hpp"
#include "languagecodes.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TILangForm *ILangForm;
//---------------------------------------------------------------------------
__fastcall TILangForm::TILangForm(TComponent* Owner)
        : TForm(Owner)
{
  TranslateComponent(this);
  TStringList* sl = new TStringList;
  DefaultInstance->GetListOfLanguages ("default",sl);

  TListItem* li = LVLangs->Items->Add();
    li->Caption = "(System default)";
    li->SubItems->Add("");

    li = LVLangs->Items->Add();
    li->Caption = "English (U.S.)";
    li->SubItems->Add("en_US");

  for(int k=0;k < sl->Count;++k)
  {
    TListItem* li = LVLangs->Items->Add();
    li->Caption = getlanguagename(sl->Strings[k]);
    li->SubItems->Add(sl->Strings[k]);
  }

}
//---------------------------------------------------------------------------
void __fastcall TILangForm::OKBtnClick(TObject *Sender)
{
  int idx = LVLangs->ItemIndex;

  if(idx == -1) return;

    TListItem* li = LVLangs->Items->Item[idx];
    UseLanguage(li->SubItems->Strings[0]);


  
}
//---------------------------------------------------------------------------
void __fastcall TILangForm::Button2Click(TObject *Sender)
{
  Close();        
}
//---------------------------------------------------------------------------
void __fastcall TILangForm::LVLangsDblClick(TObject *Sender)
{
  OKBtnClick(Sender);
  ModalResult = mrOk;
}
//---------------------------------------------------------------------------
