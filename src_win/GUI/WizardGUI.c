/****************************************************************************
*
*
*    PROGRAM: Scheduleing GUI
*
*    PURPOSE: 
*
****************************************************************************/



#include <windows.h>    // includes basic windows functionality
#include <commctrl.h>   // includes the common control header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Local Header Files
#include "Winmain.h"
#include "Winutil.h"
#include "Myansi.h"
#include "config.h"

#include "WinSerialReg.h"

#include "resource.h"
#include "ResDefs.h"	// for ReturnString() etc

/****************************************************************************
* 
*    Global and External variables
*
****************************************************************************/

static	long	wizardPage = 0;
static	HWND	gWizardWnd[5], gWizardMainWnd = 0;
static	char	outputLoc[256];
static	char	logfiletodo[256];

long	currentPage = 0;

void ShowWizard( long x )
{
	long i = 0;
	if( x<0) x = 0;
	if( x>4) x = 4;

	for(i=0;i<5;i++)
		ShowWindow( gWizardWnd[i], SW_HIDE );
	ShowWindow( gWizardWnd[x], SW_SHOW );

	currentPage = x;
}



LRESULT CALLBACK WizardMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hDlg = hWnd;
	long	i=0;

	switch (message) {
		case WM_PAINT:{
				HDC hdc;
                PAINTSTRUCT ps;
                RECT rc;

                GetClientRect(hDlg, &rc);
				hdc = BeginPaint(hDlg, &ps);
				//FillRect( hdc, &rc, gbgBrush );

                EndPaint(hDlg, &ps);

				UpdateWindow( GetDlgItem(hWnd, IDC_VIEWSCROLL) );
            }
			break; 

		
		case WM_INITDIALOG:
			break;

		case WM_CLOSE:
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;
 
		case WM_DESTROY:
			break;

		case WM_NOTIFY:
			break;


		case WM_SIZING:
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam) ) {
				case IDC_FRAME4_SEL_BUTT:
					if ( GetSaveFolderName( outputLoc ) ){
						SetText( IDC_WIZARD4_OUTPUT_LOC, outputLoc );
					}
					SetForegroundWindow( gWizardMainWnd );
					break;

				case IDC_FRAME2_SEL_BUTT:
					{
						if ( OpenLog( logfiletodo,256 ) ){
							SetText( IDC_FRAME2_FILE_NAME, logfiletodo );
							hDlg = gWizardMainWnd;
							Enable( IDC_WIZARD_NEXT );
						}
					}
					SetForegroundWindow( gWizardMainWnd );
					break;
			}
			break;
		//default:
		//	return (DefWindowProc(hWnd, message, wParam, lParam));
	}

    return FALSE;
}







LRESULT CALLBACK WizardProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hDlg = hWnd;
	long	i=0;

	switch (message) {
		case WM_PAINT:{
				HDC hdc;
                PAINTSTRUCT ps;
                RECT rc;

                GetClientRect(hDlg, &rc);
				hdc = BeginPaint(hDlg, &ps);
                EndPaint(hDlg, &ps);
				UpdateWindow( GetDlgItem(hWnd, IDC_VIEWSCROLL) );
            }
			break; 

		
		case WM_INITDIALOG:
			if( !gWizardWnd[0] )	gWizardWnd[0] = CreateDialog(hInst, "Wizard1", hDlg, (DLGPROC) WizardMainProc);
			if( !gWizardWnd[1] )	gWizardWnd[1] = CreateDialog(hInst, "Wizard2", hDlg, (DLGPROC) WizardMainProc);
			if( !gWizardWnd[2] )	gWizardWnd[2] = CreateDialog(hInst, "Wizard3", hDlg, (DLGPROC) WizardMainProc);
			if( !gWizardWnd[3] )	gWizardWnd[3] = CreateDialog(hInst, "Wizard4", hDlg, (DLGPROC) WizardMainProc);
			if( !gWizardWnd[4] )	gWizardWnd[4] = CreateDialog(hInst, "Wizard5", hDlg, (DLGPROC) WizardMainProc);
			gWizardMainWnd = hWnd;
			CenterWindowOnScreen( hWnd );
			ShowWizard(0);
			break;

		case WM_CLOSE:
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;
 
		case WM_DESTROY:
			if( gWizardWnd[0] ) { DestroyWindow( gWizardWnd[0] ); gWizardWnd[0] = NULL; }
			if( gWizardWnd[1] ) { DestroyWindow( gWizardWnd[1] ); gWizardWnd[1] = NULL; }
			if( gWizardWnd[2] ) { DestroyWindow( gWizardWnd[2] ); gWizardWnd[2] = NULL; }
			if( gWizardWnd[3] ) { DestroyWindow( gWizardWnd[3] ); gWizardWnd[3] = NULL; }
			if( gWizardWnd[4] ) { DestroyWindow( gWizardWnd[4] ); gWizardWnd[4] = NULL; }
			gWizardMainWnd = NULL;
			break;

		case WM_NOTIFY:
			break;


		case WM_SIZING:
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam) ) {
				case IDC_WIZARD_NEXT:
					if ( currentPage < 4 ){
						if ( currentPage + 1 == 2 ){
							hDlg = gWizardWnd[1];
							GetText( IDC_FRAME2_FILE_NAME, logfiletodo, 256 );
							if ( strlen(logfiletodo) < 2 ){
								ErrorMsg( ReturnString(IDS_WIZARDGUIMSG));
								break;
							}
						}
						ShowWizard( currentPage + 1 );
						if ( currentPage + 1 == 5 )
							SetText( IDC_WIZARD_NEXT, ReturnString(IDS_DONE) );
						else
							SetText( IDC_WIZARD_NEXT, ReturnString(IDS_NEXT) );
					} else {
						hDlg = gWizardWnd[2];
						i = 1;
						if( IsChecked( IDC_FRAME3_RADIO1 ) ) i = 1;
						if( IsChecked( IDC_FRAME3_RADIO2 ) ) i = 2;
						if( IsChecked( IDC_FRAME3_RADIO3 ) ) i = 3;

						WizardDefaults( &MyPrefStruct, i );

						if( strlen(outputLoc) )
							strcpy( MyPrefStruct.outfile , outputLoc);
						AddFileToLogQ( logfiletodo, 0 );
						SetReg_Prefs( "WizardStart", "yes" );
						SendMessage( hwndParent, WM_COMMAND, IDM_PROCESSLOGWIZARD, lParam);
						EndDialog(hWnd, TRUE);
					}
					break;

				case IDC_WIZARD_PREV:
					SetText( IDC_WIZARD_NEXT, ReturnString(IDS_NEXT) );
					ShowWizard( currentPage - 1 );
					break;

				case IDC_WIZARD_CANCEL:
					SetReg_Prefs( "WizardStart", "no" );
					EndDialog(hWnd, TRUE);
					break;
			}
			break;
		//default:
		//	return (DefWindowProc(hWnd, message, wParam, lParam));
	}

    return FALSE;
}



/* --------------------------------------------------

	Proc dialog stuff here

  ----------------------------------------------------*/


void InitWizardDialog( void )
{
	if ( !gWizardMainWnd )
		DialogBox( hInst, (LPCSTR)"Wizard", 0, (DLGPROC)WizardProc );
}


