//---------------------------------------------------------------------------

#ifndef LangFormH
#define LangFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TILangForm : public TForm
{
__published:	// IDE-managed Components
        TListView *LVLangs;
        TButton *OKBtn;
        TButton *Button2;
        void __fastcall OKBtnClick(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall LVLangsDblClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TILangForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TILangForm *ILangForm;
//---------------------------------------------------------------------------
#endif
