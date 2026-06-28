Unicode true

!include "MUI2.nsh"

!define PRODUCT_NAME "BestPractice"
!define PRODUCT_VERSION "1.03"
!define PRODUCT_PUBLISHER "BestPractice"
!define PRODUCT_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\BestPractice"

!ifndef STAGE_DIR
  !define STAGE_DIR "build-qt\package"
!endif

!ifndef OUTPUT_FILE
  !define OUTPUT_FILE "build-qt\BestPractice-${PRODUCT_VERSION}-Setup.exe"
!endif

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${OUTPUT_FILE}"
InstallDir "$LOCALAPPDATA\Programs\${PRODUCT_NAME}"
InstallDirRegKey HKCU "${PRODUCT_REG_KEY}" "InstallLocation"
RequestExecutionLevel user
SetCompressor /SOLID lzma
SetDatablockOptimize on
SetOverwrite on
CRCCheck on

!define MUI_ABORTWARNING
!define MUI_ICON "BestPractice.ico"
!define MUI_UNICON "BestPractice.ico"
!define MUI_FINISHPAGE_RUN "$INSTDIR\BestPractice.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Start BestPractice"
!define MUI_FINISHPAGE_LINK "BestPractice project on GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/nsarka/BestPractice2"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; English is intentionally the only installer language.
!insertmacro MUI_LANGUAGE "English"

Icon "BestPractice.ico"
UninstallIcon "BestPractice.ico"

Section "Install" SecMain
  SectionIn RO
  SetOutPath "$INSTDIR"
  File /r "${STAGE_DIR}\*.*"

  WriteUninstaller "$INSTDIR\Uninstall.exe"

  WriteRegStr HKCU "${PRODUCT_REG_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKCU "${PRODUCT_REG_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKCU "${PRODUCT_REG_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKCU "${PRODUCT_REG_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "${PRODUCT_REG_KEY}" "DisplayIcon" "$INSTDIR\BestPractice.exe"
  WriteRegStr HKCU "${PRODUCT_REG_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "${PRODUCT_REG_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegDWORD HKCU "${PRODUCT_REG_KEY}" "NoModify" 1
  WriteRegDWORD HKCU "${PRODUCT_REG_KEY}" "NoRepair" 1

  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\BestPractice.exe"
  CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\Help.lnk" "$INSTDIR\help.html"
  CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
  DeleteRegKey HKCU "${PRODUCT_REG_KEY}"
  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
  RMDir /r "$INSTDIR"
SectionEnd
