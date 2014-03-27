#!include MUI.nsh
!define APPNAME "SpikeRecorder"
!define COMPANYNAME "Backyard Brains"

RequestExecutionLevel admin

Name "${APPNAME}"
OutFile "SpikeRecorderSetup.exe"
SetCompressor lzma

InstallDir "$PROGRAMFILES\${COMPANYNAME}\${APPNAME}"

Page directory
Page instfiles

#!insertmacro MUI_PAGE_WELCOME
#!insertmacro MUI_PAGE_Directory
#!insertmacro MUI_PAGE_INSTFILES
#!insertmacro MUI_PAGE_FINISH

!macro VerifyUserIsAdmin
!macroend

function .onInit
	setShellVarContext all
functionEnd

Section "install"
	SetOutPath $INSTDIR
	File "SpikeRecorder.exe"
	File /r "data"
	File "bass.dll"

	writeUninstaller "$INSTDIR\uninstall.exe"


	createDirectory "$SMPROGRAMS\${COMPANYNAME}"
	createShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk" "$INSTDIR\SpikeRecorder.exe" "" ""

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "Publisher" "${COMPANYNAME}"
	# There is no option for modifying or repairing the install
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "NoRepair" 1
SectionEnd

function un.onInit
	SetShellVarContext all

	MessageBox MB_OKCANCEL "Permanantly remove ${APPNAME}?" IDOK next
		Abort
	next:
functionEnd

section "uninstall"
 	delete "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk"
	rmDir "$SMPROGRAMS\${COMPANYNAME}"

	# Remove files
	delete $INSTDIR\SpikeRecorder.exe
	delete $INSTDIR\data
	delete $INSTDIR\bass.dll

	# Always delete uninstaller as the last action
	delete $INSTDIR\uninstall.exe

	# Try to remove the install directory - this will only happen if it is empty
	rmDir $INSTDIR

	# Remove uninstaller information from the registry
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"
sectionEnd
