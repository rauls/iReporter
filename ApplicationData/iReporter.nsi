; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply 
; prompts the user asking them where to install, and drops a copy of example1.nsi
; there. 

;--------------------------------

; The name of the installer
Name "iReporter"

; The file to write
OutFile "..\iReporter_Setup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\iReporter

;--------------------------------

; Pages

Page directory

Page custom StartMenuGroupSelect "" ": Start Menu Folder"
Function StartMenuGroupSelect
	Push $R1

	StartMenu::Select /checknoshortcuts "Don't create a start menu folder" /autoadd /lastused $R0 "iReporter"
	Pop $R1

	StrCmp $R1 "success" success
	StrCmp $R1 "cancel" done
		; error
		MessageBox MB_OK $R1
		Return
	success:
	Pop $R0

	done:
	Pop $R1
FunctionEnd




Page instfiles

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put file there
  File /r /x Report /x AnalyzerDebug.exe /x iReporter.nsi *.*

	# this part is only necessary if you used /checknoshortcuts
	StrCpy $R1 $R0 1
	StrCmp $R1 ">" skip

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut $SMPROGRAMS\$R0\iReporter.lnk $INSTDIR\iReporter.exe

		SetShellVarContext All
		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut "$SMPROGRAMS\$R0\All users iReporter.lnk" $INSTDIR\iReporter.exe

	skip:
  
SectionEnd ; end the section


