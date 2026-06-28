//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("AboutBoxX.cpp", AboutX);
USEFORM("Main.cpp", BPMain);
USEFORM("ProgressForm.cpp", ProgressFrm);
USEFORM("LangForm.cpp", ILangForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "BestPractice";
                 Application->CreateForm(__classid(TBPMain), &BPMain);
                 Application->CreateForm(__classid(TAboutX), &AboutX);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
        return 0;
}
//---------------------------------------------------------------------------
