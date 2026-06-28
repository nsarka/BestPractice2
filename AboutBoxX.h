//---------------------------------------------------------------------------

#ifndef AboutBoxXH
#define AboutBoxXH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//#include <ExtCtrls.hpp>

//---------------------------------------------------------------------------
class TAboutX : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label1;
        TLabel *Label2;
        TShape *Shape1;
        TLabel *Label3;
        TLabel *TitleLbl;
        TLabel *AKRipLbl;
        TLabel *Label4;
        TLabel *Label5;
        void __fastcall Label1Click(TObject *Sender);
        void __fastcall FormClick(TObject *Sender);
        void __fastcall Label3Click(TObject *Sender);
        void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall FormPaint(TObject *Sender);
private:	// User declarations
  BITMAPFILEHEADER* bfh;
        

public:		// User declarations
        __fastcall TAboutX(TComponent* Owner);
  //      void SetPalette(Graphics::TBitmap * bmp);
        //WMTimer(TMsg& Msg);
};
//---------------------------------------------------------------------------
extern PACKAGE TAboutX *AboutX;
//---------------------------------------------------------------------------
#endif
