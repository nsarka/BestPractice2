; bp.nsi based on example2.nsi
;
; 
;
!include "MUI.nsh"

SetCompressor lzma

XPStyle on

;!ifdef HAVE_UPX
!packhdr tmp.dat "upx -9 tmp.dat"
;!endif
 
;Modern UI Configuration
	;!define MUI_ICON "main.ico"
	;!define MUI_HEADERBITMAP "logo.bmp"
	;!define MUI_UNICON "del.ico"
	!define MUI_UNINSTALLER
  	

	!define MUI_COMPONENTSPAGE_NODESC 

  	!insertmacro MUI_PAGE_WELCOME
	

	!define MUI_ABORTWARNING
  
  

  	!insertmacro MUI_PAGE_LICENSE license.txt
  	!insertmacro MUI_PAGE_COMPONENTS

 
  	!insertmacro MUI_PAGE_DIRECTORY
 	!insertmacro MUI_PAGE_INSTFILES
  
  	  ;!define MUI_FINISHPAGE_RUN "$INSTDIR\bp.exe"
  	  ;!define MUI_FINISHPAGE_RUN_NOTCHECKED
 	  ;!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
  	  ;!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
  	!insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
 
  
  

  
Name "BestPractice 1.03"




;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Japanese"



; The file to write
OutFile "bpsetup.exe"

;Turn on CRC checking
CRCCheck on

;------------- Show License ------------------
LicenseData license.txt

; The default installation directory
InstallDir "$PROGRAMFILES\BestPractice"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BestPractice" "UninstallString"


;Specify icon
;Icon main.ico


;Compiler flags
SetOverwrite ifnewer
SetCompress force
SetDatablockOptimize on


Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd


; The stuff to install

Section "Main program"

  SectionIn RO
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
 
  ;Put files there
  File "bp.exe"
  File "akrip32 - COPYING-2.1.txt"
  File "akrip32.dll"
  
  File "bppopup.hlp"
  File "bphelp.chm"

  SetOutPath $INSTDIR\lang\da
  File "lang\da\default.mo"

  SetOutPath $INSTDIR\lang\de
  File "lang\de\default.mo"

  SetOutPath $INSTDIR\lang\es
  File "lang\es\default.mo"

  SetOutPath $INSTDIR\lang\fr
  File "lang\fr\default.mo"

  SetOutPath $INSTDIR\lang\hu
  File "lang\hu\default.mo"

  SetOutPath $INSTDIR\lang\it
  File "lang\it\default.mo"

  SetOutPath $INSTDIR\lang\ja
  File "lang\ja\default.mo"  

  SetOutPath $INSTDIR\lang\nl
  File "lang\nl\default.mo"
  
  SetOutPath $INSTDIR\lang\pt_BR
  File "lang\pt_BR\default.mo"

  SetOutPath $INSTDIR\lang\ru
  File "lang\ru\default.mo"

  SetOutPath $INSTDIR\lang\sv
  File "lang\sv\default.mo"

  SetOutPath $INSTDIR\lang\zh_TW
  File "lang\zh_TW\default.mo"


; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BestPractice" "DisplayName" "BestPractice (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BestPractice" "UninstallString" '"$INSTDIR\uninstall.exe"'

  WriteUninstaller "$INSTDIR\uninstall.exe"
  

SectionEnd



; optional section program shortcuts
Section "Create Program Shortcuts in Start Menu" 

  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\BestPractice"
  CreateShortCut "$SMPROGRAMS\BestPractice\BestPractice.lnk" "$INSTDIR\bp.exe" "" "$INSTDIR\bp.exe" 0
  CreateShortCut "$SMPROGRAMS\BestPractice\BestPractice Help.lnk" "$INSTDIR\bphelp.chm"
  CreateShortCut "$SMPROGRAMS\BestPractice\Uninstall.lnk" "$INSTDIR\uninstall.exe"  


SectionEnd


;Display the Finish header
;Insert this macro after the sections if you are not using a finish page
;!insertmacro MUI_PAGE_FINISH

; uninstall stuff

; special uninstall section.
Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\BestPractice"

  
  ; MUST REMOVE UNINSTALLER, too
  ; remove shortcuts, if any.
  RMDir /r "$SMPROGRAMS\BestPractice"
  ; remove directories used.

  RMDir /r "$INSTDIR"

SectionEnd

; eof
