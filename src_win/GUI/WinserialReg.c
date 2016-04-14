#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "resource.h"

#include "WinMain.h"
#include "WinSerialReg.h"
#include "Registry.h"
#include "Windnr.h"
#include "ice.h"
#include "myansi.h"
#include "weburls.h"

#include "ResDefs.h"	// for ReturnString() etc

#define	DATA_ENCRYPTED		1	// 1 = recovering encrypted data, 0 = not encrypted


extern 	SerialInfo		SerialData;
extern	SerialInfoPtr	SerialDataP;
extern	SerialInfoH 	SerialDataH;

extern long	idcode, serialID, customer_id;
extern	HINSTANCE 		hInst; // current instance
extern	HWND			hwndParent;

static	int		reg_returnstat;
static	HWND	hwndSerialWnd;



LONG APIENTRY SerialDlgProc(HWND hWnd, UINT msg, DWORD dwParam, LONG lParam);

SerialInfoPtr ghuserInfo;


///////////////////////////////        READ SERIAL OF FILE


/*void SetRegVariable( char *name, long value )
{
	SetRegKeyData( APPLICATION_IRREG, name, &value, sizeof(long) );
}*/


long GetRegVariable( char *name )
{
	long value,len=sizeof(long);

	// First look in the new registry settings
	if ( !GetRegKey ( APPLICATION_IRREG, name, &value, &len ) ){
		return value;
	}
	// Secondly, look in the old registry settings
	else if ( !GetRegKey ( APPLICATION_REG, name, &value, &len ) ){
		return value;
	}
	return 0;
}


/*********************** GetUserInfo **********************************/
/*
	SerialDataP = &SerialData;
	SerialDataH = &SerialDataP;
*/

SerialInfoH	GetSerialInfoData( SerialInfoH Data, char *regStr )
{
//static	SerialInfo		readData;
//static	SerialInfoPtr	readDataP= &readData;
	int			ok;
	int			err = 0;
 	HANDLE			hFile;
	unsigned long	len=SERIALINFO_SIZE;

	if ( GetRegKey ( regStr, "Licence_Data", *Data, &len ) ){
		if ((hFile = CreateFile( "core.ser", GENERIC_READ, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_READONLY , NULL)) == (HANDLE)-1) {
			//ErrorMsg( "cant open serial file 'core.ser'");
			return NULL;
		}

		ok = ReadFile(  hFile, *Data, SERIALINFO_SIZE,  &len, NULL );
		CloseHandle(hFile);
	} else
		ok = TRUE;

	if ( ok ){
		// DECRYPT DATA JUST AFTER LOADING!!!
		#if DATA_ENCRYPTED
			DecryptInfo( *Data );
		#endif
		return Data;
	} else
		return NULL;
}



int	SaveSerialData(SerialInfoH userInfo, char *name, char *regStr)
{
	int		ok;
	int		err = 0;
 	HANDLE          	hFile;
	unsigned long		written;

	if ( userInfo ){
		long len; char *txt;

		txt = (*userInfo)->userName+1; len = strlen(txt);
		SetRegKeyString( regStr, "Name", txt, len );

		txt = (*userInfo)->organization+1; len = strlen(txt);
		SetRegKeyString( regStr, "Company", txt, len );

	} else {
		char txt[256]; long len=255;

		GetUserName( txt, &len );
		SetRegKeyString( regStr, "Name", txt, mystrlen(txt) );

		GetComputerName( txt, &len );
		SetRegKeyString( regStr, "Company", txt, mystrlen(txt) );
	}

	// ENCRYPT DATA BEFORE SAVING!!!!!!
	#if DATA_ENCRYPTED
	if (userInfo)
		EncryptInfo(*userInfo);
	#endif

	if (!userInfo)
		return 0;

	if ( SetRegKeyData( regStr, "Licence_Data", *userInfo, SERIALINFO_SIZE ) ){
		if ((hFile = CreateFile( "core.ser", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL , NULL)) == (HANDLE)-1) {
			ErrorMsg( "cant write serial"); return -1;
		}
		ok = WriteFile(  hFile, *userInfo, SERIALINFO_SIZE,  &written, NULL );
		CloseHandle(hFile);
	} else {
		SetRegKeyLocalData( regStr, "Licence_Data", *userInfo, SERIALINFO_SIZE );
		ok = TRUE;
	}

	return 0;
}


HFONT			hRegFont, hFinePrint;


void WriteXYb( HWND hDlg, HFONT font, char *text, long x, long y, long color )
{
static  HDC		hdcSrc, hdc;
    RECT textrc;
    PAINTSTRUCT ps;
	HGDIOBJ of;

	if ( hDlg && text ){
		hdc = BeginPaint(hDlg, &ps);
		hdc = GetDC (hDlg);
		of = SelectObject (hdc, font );

		textrc.left = x; textrc.top = y;
		textrc.right = x+290; textrc.bottom = y+120;

		SetTextColor( hdc, color );
		SetBkColor( hdc, 0x642C04 );

		DrawText( hdc, text, strlen(text), &textrc, DT_LEFT );

		if( of ) SelectObject (hdc, of);

		ReleaseDC( hDlg, hdc );
		EndPaint(hDlg, &ps);
	}
}


// plot text at xy in transparent mode
void WriteXY( HWND hDlg, HFONT font, char *text, long x, long y, long color )
{
static  HDC		hdcSrc, hdc;
    RECT textrc;
    PAINTSTRUCT ps;
	HGDIOBJ of;

	if ( hDlg && text ){
		hdc = BeginPaint(hDlg, &ps);
		hdc = GetDC (hDlg);
		of = SelectObject (hdc, font );
		SetBkMode( hdc, TRANSPARENT );

		textrc.left = x; textrc.top = y;
		textrc.right = x+290; textrc.bottom = y+120;

		if ( color ){
			// Draw Background shadow text
			SetTextColor( hdc, 0x0 );

			textrc.left = x+1; textrc.top = y+1;
			DrawText( hdc, text, strlen(text), &textrc, DT_LEFT );
		}

		textrc.left = x; textrc.top = y;
		SetTextColor( hdc, color );
		DrawText( hdc, text, strlen(text), &textrc, DT_LEFT );

		if( of ) SelectObject (hdc, of);

		ReleaseDC( hDlg, hdc );
		EndPaint(hDlg, &ps);
	}
}

#include "Winutil.h"
void DrawStaticText( HWND hDlg, long id )
{
	RECT rc;
	char szText[256];

	Hide(id);
	GetControlRect( hDlg, GetDlgItem( hDlg, id ), &rc );
	GetText( id, szText, 255 );
	WriteXY( hDlg, hFinePrint, szText, rc.left, rc.top, 0xffffff );
	WriteXY( hDlg, hFinePrint, szText, 1,1, 0xffffff  );
}



void UpdateRegNumber( HWND hDlg, char *txt )
{
	SetText( IDC_REGSERIAL, txt );
	UpdateWindow( hDlg );
	//WriteXYb( hDlg, hRegFont, txt, 155,280, 0xffffff  );
}



//##########
// SerialDlgProc - handle serial dialog inputs
//
//
LONG APIENTRY SerialDlgProc(HWND hWnd, UINT msg, DWORD dwParam, LONG lParam)
{
    char clen, inputString[255];
static	HBITMAP		hbmReg;
static	BITMAP		bmReg;
static	HPALETTE	hPal=NULL, hPalOld;
	long	temp;
	HWND	hDlg = hWnd;

    switch (msg) {
         case WM_PAINT:
            {
				HBITMAP hbmOld;
				RECT rc;
                PAINTSTRUCT ps;
                HDC hdc, hdcSrc;


                GetClientRect(hDlg, &rc);
                hdc = BeginPaint(hDlg, &ps);
				if ( hbmReg ){
					hdc = GetDC (hDlg);
					hdcSrc = CreateCompatibleDC (hdc);

					hbmOld = SelectObject (hdcSrc, hbmReg );
					SetStretchBltMode( hdc, HALFTONE );
					StretchBlt( hdc,0,0, (rc.right-rc.left), (rc.bottom-rc.top),
								hdcSrc, 0, 0, bmReg.bmWidth, bmReg.bmHeight,SRCCOPY );
					SelectObject (hdcSrc, hbmOld);
					DeleteDC (hdcSrc);

					{
						char			txt[255];
						unsigned long	txtSz, x=52, y=160, step = 23;

						txtSz = 255;
						GetComputerName( txt, &txtSz );
						SetDlgItemText(hWnd, IDC_REGORG, txt );

						txtSz = 255;
						GetUserName( txt, &txtSz );
						SetDlgItemText( hWnd, IDC_REGNAME, txt );

						WriteXY( hDlg, hFinePrint, "Please enter your registration details below.", x, y, 0xffffff );

						y = 182;
						WriteXY( hDlg, hFinePrint, "Name:", x, y+=step, 0xffffff );
						WriteXY( hDlg, hFinePrint, "Organization:", x, y+=step, 0xffffff );
						WriteXY( hDlg, hFinePrint, "Registration Number:", x, y+=step, 0xffffff );
					}


					ReleaseDC( hDlg, hdc );

				}
                EndPaint(hDlg, &ps);
            }
            break;

        case WM_DESTROY:
            DeleteObject (hbmReg);
            DeleteObject (hPal);
			DeleteObject (hRegFont);
			hwndSerialWnd = NULL;
            break;

        case WM_INITDIALOG:
			{
				char			txt[255];
				unsigned long	txtSz;

#ifdef DEF_FULLVERSION
				hbmReg = LoadBitmap( hInst, "IDB_ABOUTENT" );
#else
				hbmReg = LoadBitmap( hInst, "IDB_ABOUTSTANDARD" );
#endif
				hwndSerialWnd = hWnd;
				GetObject (hbmReg,sizeof(BITMAP), &bmReg);

				txtSz = 255;
				GetComputerName( txt, &txtSz );
				SetDlgItemText(hWnd, IDC_REGORG, txt );

				txtSz = 255;
				GetUserName( txt, &txtSz );
				SetDlgItemText( hWnd, IDC_REGNAME, txt );

				hRegFont = CreateFont(10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH | FF_SWISS, "MS Sans Serif");
				hFinePrint = CreateFont(8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH | FF_SWISS, "MS Sans Serif");

				Hide( IDC_REGTXT1 );
				Hide( IDC_REGTXT2 );
				Hide( IDC_REGTXT3 );
				Hide( IDC_REGTXT4 );
				Hide( IDC_REGTXT5 );

				SendMessage( GetDlgItem(hDlg, IDC_REGSERIAL), WM_SETFONT, (UINT)(hRegFont), TRUE);
				SendMessage( GetDlgItem(hDlg, IDC_REGTXT5), WM_SETFONT, (UINT)(hRegFont), TRUE);

				//Hide(IDC_REGDEMO);

				if( IsDemoReg() ){
					Disable( IDC_REGDEMO );
				}
			}
			break;

        case WM_COMMAND:
           switch (dwParam) {
				case IDC_REGDEMO:
					if( !IsDemoReg() ){
						mystrcpy( &SerialData.userName[1], "Demonstration (15 days)" );
						SerialData.userName[0] = (char)mystrlen(SerialData.userName);
						mystrcpy( &SerialData.organization[1], "Evaluation Version" );
						SerialData.organization[0] = (char)mystrlen(SerialData.organization);
						temp = clock() & 0xfffff;
						sprintf( &SerialData.serialNumber[1], "DEMO" );		//7530864129
						SerialData.serialNumber[0] = 20;
						reg_returnstat = TRUE;
						customer_id = 0;
						EndDialog(hWnd, TRUE);
						return TRUE;
					}
					break;

				case IDC_REGOK:
					GetDlgItemText(hWnd, IDC_REGNAME,  inputString,	254 );
					clen = (char)mystrlen( inputString );
					SerialData.userName[0] = clen;
					mystrcpy( &SerialData.userName[1], inputString );

					GetDlgItemText(hWnd, IDC_REGORG, 	inputString ,254 );
					clen = (char)mystrlen( inputString );
					SerialData.organization[0] = clen;
					mystrcpy( &SerialData.organization[1], inputString );

					GetDlgItemText(hWnd, IDC_REGSERIAL,inputString ,30 );
					clen = (char)mystrlen( inputString );
					SerialData.serialNumber[0] = clen;
					mystrcpy( &SerialData.serialNumber[1], inputString );


					{
						int i = 0;
						//char txt[256];
						// Removed because Bill doesnt like it even though it has a cool factor :(
						//for(i=0;i<100;i++)
						//{
						//	sprintf( txt, "FWP-%04d-%04d-%02d             ", rand()%9999, rand()%9999, rand()%99 );
						//	UpdateRegNumber( hWnd, txt );
						//	Sleep(10);
						//}

						if ( VerifySerialRelease(inputString) )
						{
							UpdateRegNumber( hWnd, "Serial is verified ok.     " );
							reg_returnstat = TRUE;
						} else
						if ( VerifySerialDemo(inputString) )
						{		//EXT-0574-6816-29
							UpdateRegNumber( hWnd, "Demo is ok.                " );
							reg_returnstat = TRUE;
						} else
						{
							UpdateRegNumber( hWnd, "Serial number is incorrect." );
							reg_returnstat = FALSE;
						}

						SysBeep(1);
						Sleep( 1500 );

						if ( reg_returnstat )
							EndDialog(hWnd, TRUE);
					}
					return TRUE;
					break;

				case IDC_REGCANCEL:
					EndDialog(hWnd, TRUE);
					reg_returnstat = -1;
					return TRUE;
					break;

				case IDC_REGORG:
					break;

				case IDC_REGSERIAL:
					break;

				case IDC_REGNAME:
					break;
            }
 			break;
        default:
            return FALSE;
    }

    return FALSE;
}


void OpenSerialDlg( SerialInfoPtr userInfo )
{
	static	int	running = 0;

	ghuserInfo = userInfo;
	if ( !running )
	{
		running = TRUE;
		DialogBox( hInst, "SERIALREG", hwndParent, (DLGPROC)SerialDlgProc);
		running = FALSE;
	} else
		SetFocus( hwndSerialWnd );
}



void ShowRegMessage( void )
{
	long result;
	char title[128];
	char msg[1024];
	mystrcpy( title, ReturnString( IDS_REGISTONLINE ) );
	mystrcpy( msg, ReturnString( IDS_REGISTMSG1 ) );
	mystrcat( msg, ReturnString( IDS_REGISTMSG2 ) );
	result = MessageBox( NULL, msg, title, MB_YESNO );
	if ( result == IDYES ){
		ShowHTMLShortcut( GetFocus(), URL_REGISTER, NULL );
	}
}




void SetReg_Prefs( char *id, char *ptr )
{
	long len = strlen(ptr);
	SetRegKeyString( APPLICATION_IRPREFSREG, id, ptr, len );
}

void SetReg_PrefsNumber( char *id, long num )
{
	SetRegKeyNumber( APPLICATION_IRPREFSREG, id, num, sizeof(long) );
}

long GetReg_Prefs( char *id, char *ptr )
{
	long	len=255;
	char	data[255];

	if ( ptr ){
		*ptr = 0;
		// First look in the new QUEST registry settings
		if ( !GetRegKey( APPLICATION_IRPREFSREG, id, data, &len ) ){
			mystrncpy( ptr, data, len );
			return len;
		}
		else
		// Secondly, look in the old ACTIVE CONCEPTS registry settings
		if ( !GetRegKey( APPLICATION_PREFSREG, id, data, &len ) ){
			mystrncpy( ptr, data, len );
			// Before we leave here... copy the setting to QUEST registry
			SetRegKeyData( APPLICATION_IRPREFSREG, id, data, len );
			return len;
		}
	}
	return 0;
}

long GetReg_PrefsNumber( char *id, long *ptr )
{
	long	len=sizeof(long);
	long	data;

	if ( ptr ){
		*ptr = 0;
		// First look in the new registry settings
		if ( !GetRegKey ( APPLICATION_IRPREFSREG, id, &data, &len ) ){
			*ptr = data;
			return len;
		}
		else
		// Secondly, look in the old registry settings
		if ( !GetRegKey ( APPLICATION_PREFSREG, id, &data, &len ) ){
			*ptr = data;
			SetReg_PrefsNumber( id, data );
			return len;
		}
	}
	return 0;
}

void SetReg_PrefsFile( char *ptr )
{
	SetReg_Prefs( "Prefsfile", ptr );
}


long GetReg_PrefsFile( char *ptr )
{
	return GetReg_Prefs( "Prefsfile", ptr );
}


/************************** Fetch_SerialInfo ***********************************/
short	Fetch_SerialInfo( SerialInfoPtr userInfo )
{
	OpenSerialDlg( userInfo );
	return reg_returnstat;
}

// 15 day trial expires
void ShowMsg_TrialOver( void )
{
#ifdef DEF_FULLVERSION
	const char *szText = "Sorry, your 15 day trial period has expired.\n\nWould you like to purchase iReporter Enterprise Edition online?";
#else
	const char *szText = "Sorry, your 15 day trial period has expired.\n\nWould you like to purchase iReporter Standard Edition online?";
#endif
	if( gNoGUI ) return;

	// It asks you if you want to go to the website because NO ONE will try to re-type the URL
	if ( MessageBox( GetFocus(), szText, "iReporter Expired", MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL ) == IDYES )
		ShowHTMLShortcut( GetFocus(), URL_APPLICATION, NULL );
}

// Free version OVER
void ShowMsg_FreeVersionExpired( void )
{
	const char *szText = "Sorry, your copy of iReporter Freeware has expired.\n\nWould you like to purchase iReporter Enterprise Edition online?";
	if( gNoGUI ) return;

	// asks i fyou want to go to the website too, coz A) you cant cut/paste the URL, B) no one will re-type it in , its just stupid and unprofessional, this is quicker to BUY IT
	if ( MessageBox( GetFocus(), szText, "iReporter Expired", MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL ) == IDYES )
		ShowHTMLShortcut( GetFocus(), URL_STORE, NULL );
}


void ShowMsg_ReEnterSerial( char *badSerialStr )
{
	char szText[256];
	if( gNoGUI ) return;
	sprintf( szText, "The serial number you entered <%s> is incorrect\n", badSerialStr );
	MessageBox( GetFocus(), szText, "iReporter Registration", MB_OK|MB_ICONINFORMATION|MB_APPLMODAL );
}

