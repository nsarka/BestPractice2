//---------------------------------------------------------------------------

#ifndef ProgressFormH
#define ProgressFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TProgressFrm : public TForm
{
__published:	// IDE-managed Components
        TButton *Button1;
        TProgressBar *PB;
        TLabel *PercentLabel;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
public:		// User declarations
        __fastcall TProgressFrm(TComponent* Owner);
        bool Stop;
};
//---------------------------------------------------------------------------
extern PACKAGE TProgressFrm *ProgressFrm;
//---------------------------------------------------------------------------
#endif
