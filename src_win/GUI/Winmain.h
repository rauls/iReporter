/*
Windows App main.h
*/

#ifndef	__WINMAIN_H
#define	__WINMAIN_H

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


enum {
	IDM_PROCESSLOGNOW		=	2450,
	IDM_PROCESSLOGWIZARD,
	IDM_REGISTERNOW,
	IDM_APPCENTRAL,
	IDM_REGISTERSERIAL,
	IDM_WEBMANUAL,
	IDM_PROCESSANDSAVE_DB,
	IDM_REMOTECONNECT,
	IDM_REMOTEDISCONNECT,
	IDM_WIZARD,
	IDM_NEWSKIN,				// 2460
	IDM_WEBADMIN,
	IDM_PROCESSDATABASE,
	IDM_PROCESSDATABASEFROMLIST,
	IDM_CONVERT2W3C,

	IDM_LIVERADIO1,
	IDM_LIVERADIO2,
	IDM_LIVERADIO3,
	IDM_LIVERADIO4,
	IDM_LIVERADIO5,
	IDM_LIVERADIO6
};



extern int			gNoGUI;
extern char			*szAppName;

extern short		gProcessLog;		
extern int			gStopAll;
extern int			gProcessing;
extern int			gProcessingSchedule;
extern int			gNoGUI;
extern int			gNoThread;

extern int			gRemoteControlled;
extern int			gConvertLogToW3C_Flag;

extern HWND    		ghMainDlg;
extern HWND    		hwndParent;
extern HWND    		ghStatusWnd;
extern HWND 		ghProgressWnd;    // handle of progress bar 
extern int			gSaved;
extern HINSTANCE	hInst;

extern	HDC         gHDCGlobal;     /* The Screen DC                               */
extern	LONG		iNumColors;    /* Number of colors supported by device        */
extern	LONG        iDepth;
extern	INT         iRasterCaps;   /* Raster capabilities                         */
extern	HFONT		hCtrlFont;	// Font for 'fine print' in dialog



// from engine.cpp

extern double Time2JD( void *datePtr, double *aDbl );
extern void JDTimetoString( double jd, char *outStr );
extern void StopAll( long );




// Foward declarations of functions included in this code module:

ATOM MyRegisterClass(CONST WNDCLASS*);
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LONG APIENTRY ButtonBarProc(HWND, UINT, DWORD, LONG);
LRESULT CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SplashDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK FtpOpenDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChangePasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SupportZoneDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

long GetLastErrorTxt( char *p );
long SpawnShowShellFile( char *url );

void DoProcessLogQ( HWND hWnd, long viewreport );


void CustomUpdateProgressBar( HWND hWnd, long , long );
HWND CreateAToolBar( HWND hParent );
BOOL CenterWindow (HWND, HWND);
HWND CreateXStatusWindow( int nParts, HWND parentWnd );
HWND CreateOneStatusWindow( HWND parentWnd );
void GetRectofStatusBarItem( long iPart, RECT *lprc );
void RemapPalette( HDC x, HWND hWnd );
HWND InitLogListView (HWND , long );
LRESULT LogListNotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void RefreshListView( void );
void ClearLogHistory( void );
void DeleteSelectedLogHistory( HWND hWnd );
void CompressSelectedLogHistory( HWND hWnd );
void CompressBzip2SelectedLogHistory( HWND hWnd );
void ShowLogDuration();
long FindItemLogListView( char *txt );
long CopySelectedHistory_ToLOGQ( void );
void ReadLogListAsText( char * );
void SaveLogListAsText( char * );
void RefreshStatusBar( void );
void RepaintListView( void );

void StatusSet( char *txt );
int StatusSetf( char *msg, ... );
int StatusSetID( int id, ... );
int MsgBox_Error( int id, ... );
int MsgBox_yesnocancel( long id, char *txt );
int MsgBox_yesno( long id, ... );


long LoadSkin( HWND hWnd, char *filename );

void LoadMRUList( HWND hWnd );
void AddToMRUList( HWND hWnd );

void ToolBarWrite( char *txt, long x, long y );

void ButtonBarPaint(HWND hWnd );

void ShowHTMLURL( HWND hWnd, char *doc, char *param );
void ShowHTMLShortcut( HWND hWnd, char *doc, char *param );

long InitApp(void);
void ExitApp(void);

long GetPixelXScale( long );
long GetPixelYScale( long );
long XDUtoPixel( long dialogunitX );
long YDUtoPixel( long dialogunitY );

void DeSelectLogHistory( void );

int RealizeMyPalette( HWND hwnd, HPALETTE hPal );
HPALETTE CreateMyPalette( HWND hWnd, HBITMAP hBMP, BITMAP *bitmap );

int CenterWindowOnScreen (HWND hwnd );
void ChangeMainWindowTitle( void );
void ProgressChangeMainWindowTitle( long percent, char *etaStr );
void StatusWindowSetText( char *txt );
void StatusWindowXSetText( char *txt,  HWND statWnd );
void StatusWindowSetProgress( long percent, char *text );
int CreateProgressBar( long totalVal) ;
int UpdateProgressBar( long percent );
int DeleteProgressBar( void ); 

void DrawGradBackground( HDC hdc, RECT *rc, long color, long percent );
HBRUSH GradFromPos( HWND win, HWND ctl );
void GradientBK( HWND hWnd, HDC hdc );
LONG APIENTRY ForceTransparentClient( HWND hWnd, UINT msg, DWORD wParam, LONG lParam );
LONG APIENTRY ForceBackgroundGradient( HWND hWnd, UINT msg, DWORD wParam, LONG lParam );

void ShowProgressDetail( long level, long hits, char *msg, double timesofar );

void ButtonIntoPROC( HWND hWnd );
void ButtonIntoSTOP( HWND hWnd );
void RefreshProcButton( HWND hWnd );

char *GetLogListViewItem( void );
long FindLogInHistoryAndSelect( char *txt );
long AddLogToGUIHistory( char *, long tot );
long AddLogToHistory( char *log );

BOOL AudioPlayResource(LPSTR lpName);
void AudioStopPlay( void);
int CheckWarning( void );

void ProcessLogNow( void );

#define LIGHTGRAY               RGB(192, 192, 192)
#define DARKGRAY                RGB(128, 128, 128)

// important GUI defines to save on complex calls, thus can be refined for diff OS
#define	IsChecked( id )			(SendMessage( GetDlgItem( hDlg, id),  BM_GETCHECK, 0, 0)==BST_CHECKED)
#define	CheckIt( id, check )	SendMessage( GetDlgItem( hDlg, id),  BM_SETCHECK, check, 0)
#define	CheckON( id )			SendMessage( GetDlgItem( hDlg, id),  BM_SETCHECK, 1, 0)
#define	CheckOFF( id )			SendMessage( GetDlgItem( hDlg, id),  BM_SETCHECK, 0, 0)

#define	Enable( id )			EnableWindow( GetDlgItem( hDlg, id),  TRUE )
#define	Disable( id )			EnableWindow( GetDlgItem( hDlg, id),  FALSE )

#define	Hide( id )				Disable(id);ShowWindow ( GetDlgItem( hDlg, id), SW_HIDE)
#define	Show( id )				Enable( id );ShowWindow ( GetDlgItem( hDlg, id), SW_SHOW)
 
#define	SetText( id, text )		SetDlgItemText( hDlg, id, text )
#define	SetTextId( id, strId )	SetDlgItemText( hDlg, id, MAKEINTRESOURCE(strId) )
#define	GetText( id,t,n)		GetDlgItemText( hDlg, id, t , n );
#define SetPopupNum( id, val )	SendMessage( GetDlgItem(hDlg, id), CB_SETCURSEL , val , 0 )
#define GetPopupNum( id )		SendMessage( GetDlgItem(hDlg, id), CB_GETCURSEL , 0, 0 )
#define GetPopupTot( id )		SendMessage( GetDlgItem(hDlg, id), CB_GETCOUNT , 0, 0 )
#define GetPopupText( id, n, ptr )		SendMessage( GetDlgItem(hDlg, id), CB_GETLBTEXT , n, (LPARAM)ptr )

#define	AddPopupItem( id, string ) SendMessage( GetDlgItem(hDlg, id), CB_ADDSTRING, 0, (LPARAM) string );

#define	SetSliderPos( id,pos )	SendMessage( GetDlgItem(hDlg, id ), TBM_SETPOS, TRUE, pos ); \
								UpdateWindow( GetDlgItem(hDlg, id ) )
#define GetSliderPos( id )		SendMessage( GetDlgItem(hDlg, id ), TBM_GETPOS, 0, 0 )
#define	SetSliderRange( id, x ) SendMessage( GetDlgItem(hDlg, id ),TBM_SETRANGE , 1 , MAKELONG( 0, x ) )

#define	SetFont( id, f )		SendMessage( GetDlgItem(hDlg, id), WM_SETFONT, (UINT)(f), TRUE)



#define	ListBox_AddString( filterID, string )	\
	SendMessage( GetDlgItem(hDlg, filterID), LB_ADDSTRING, 0, (LPARAM)string)

#define	ListBox_DelString( filterID, index )	\
	SendMessage( GetDlgItem(hDlg, filterID), LB_DELETESTRING, (WPARAM) index, 0)

#define	ListBox_ChangeString( filterID, index, string )	\
	SendMessage( GetDlgItem(hDlg, filterID), LB_DELETESTRING, (WPARAM) index, 0); \
	SendMessage( GetDlgItem(hDlg, filterID), LB_INSERTSTRING, (WPARAM) index, (LPARAM)string)

#define	ListBox_DelFirst( filterID )	\
	SendMessage( GetDlgItem(hDlg, filterID), LB_DELETESTRING, (WPARAM) 0, 0)

#define	ListBox_GetSelected( filterID )	\
	SendMessage( GetDlgItem(hDlg, filterID), LB_GETCURSEL, 0, 0)

#define	ListBox_GetTotal( filterID )	\
	SendMessage( GetDlgItem(hDlg, filterID), LB_GETCOUNT, 0, 0)

#define	ListBox_GetText( filterID, index, string )	\
	SendMessage( GetDlgItem(hDlg, filterID), LB_GETTEXT, (WPARAM)index, (LPARAM)string)



#define ID_LOG_LISTVIEW     1002

#define	MAKETRANSPARENT \
		case WM_ERASEBKGND: \
		case WM_CTLCOLORSTATIC: \
			return ForceTransparentClient( hDlg, msg, wParam, lParam );


#define ShowProgress( percent, forceShow, msg )		ShowProgressDetail( percent*10,  0, msg, 0 )


#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif


#endif

