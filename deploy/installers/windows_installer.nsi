!define PRODUCT_NAME "CourierMan"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "Muhammad Fiaz"

Name "${PRODUCT_NAME}"
OutFile "CourierMan-${PRODUCT_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES64\CourierMan"
RequestExecutionLevel admin
LicenseData "..\..\LICENSE"
LicenseForceSelection checkbox "I accept the GNU GPL v3 license"

Page license
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Install"
  SetOutPath "$INSTDIR"
  File /r "..\..\build\windows\x64\release\*.*"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateShortcut "$DESKTOP\CourierMan.lnk" "$INSTDIR\CourierMan.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CourierMan" \
    "DisplayName" "CourierMan"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CourierMan" \
    "UninstallString" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
  MessageBox MB_YESNO|MB_ICONQUESTION \
    "Delete all user data, settings, and logs?" \
    IDNO keepUserData
  RMDir /r "$APPDATA\CourierMan"
keepUserData:
  Delete "$DESKTOP\CourierMan.lnk"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CourierMan"
SectionEnd
