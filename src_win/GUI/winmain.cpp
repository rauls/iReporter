/*
	File:		Winmain.c

	Contains:	Main event loop and basic keyboard/mouse processing

	Written by:	Raul Sobon

dwResult = ExpandEnvironmentStrings( 
               "PATH=%PATH%", 
               lpszSystemInfo, 
               BUFFER); 
 


Changes
* All ftp downloads are now absolute paths, so if its unix, you must include /usr/home/ or what ever your system is.
* Ftp downloads support wildcards, so ftp://user:pass@host.com/usr/home/logs/99*.log will work.
* Ftp uploads via post processing or Schedule upload is user directory relative, /usr/home need not be entered.
* Schedules now can do yearly schedules and are now handled better.
* Schedules include an upload directive, so you can supply a local path or ftp upload path just like post processing.
* Schedule history events now have more detailed information including failures.
* running with a -debug argument causes it to send debug strings to the system , these can be viewed
  with debugview.exe from www.sysinternals.com
* Schedule fixed problem with usa date format, MM/DD/YY
* Schedule now displays all dates in YYYY format.
* Control Panel Services option in the menu works in Windows2000
* You can specify the amount of concurrent DNS lookups , 2 to 100000, the more you use the more ram is used (500 per meg)
* Accept CGI parameters in URLs option added
* Added New 3d time history graphs for Error URLs, Download Files, Top Level Directory
* Added ability to get page titles via the internet instead of URL names.
* Added post processing option to delete local files after a transfer/upload
* Fixed post processing so that it work in uploading multiple directories from a virtual host report.
* Added post processing option to email upon completing of processing to a specifed address with custom message.
  NOTE: This uses the systems Outlook mail clients config for SMTP server, or netscapes default user configs mail options.
* Virtalhost processing now uses less memory when very large hosts are used.
  hostnames are virified from currupt/bad log files so that no wierd names are used taking up ram.
* now remembers your last Log file directory in the dialog.
* remembers the previous saved settings file, so that next time it is ran, it will load that one up by default.
* Improved speed of processing by 5% or so there abouts.
  Also with large Virtual host files, it may be faster because of significant less ram use.
* Removed white border on buttons in the GUI.
* Removed possible wierd GUI glitches/bad redraws.
* Search Engine report now includes strings per search engine.
* Search strings results are now more complete and include " or + symbols people used.
* Added Visitors to columns of more reports.
* Added links to the country report page that links back to CIAs website about each country.
* Demo version now has a Button to start as a demo instead of asking for a demo serial.



*/






#define _WIN32_WINNT	0x0500		// needed to pick up latest platform SDK stuff ie WS_EX_LAYERED etc

#include "FWA.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>

// Windows Header Files:
#include <conio.h>
#include <windows.h>
#include <windef.h>
#include <winuser.h>
#include <wingdi.h>
#include <commctrl.h>   // includes the common control header
#include <mmsystem.h>


#include "resource.h"

// Local Header Files
#include "myansi.h"
#include "winmain.h"
#include "winutil.h"
#include "windnr.h"
#include "winserialreg.h"
#include "winnet_io.h"

#include "httpinterface.h"
#include "prefsGUI.h"
#include "scheduleGUI.h"
#include "schedule.h"
#include "datetime.h"
#include "log_io.h"
#include "config.h"
#include "postproc.h"
#include "version.h"
#include "weburls.h"
#include "LangBuilderGUI.h"
#include "engine_proc.h"
#include "engineregion.h"

// JMC
#include "fileviewgui.h"
#include "engine_dbio.h"

#include "resdefs.h"	// for GetString()
#include "SettingsAppWide.h"
#include "NTServApp.h"
#include "Report.h"
#include "LogFileHistory.h"
#include "FileTypes.h"
#include "GlobalPaths.h"
#include "DateFixFileName.h"


static long lastTime = 0;

//
//
//
//------------------  Windows init/window code follows...
//
//
//

// Makes it easier to determine appropriate code paths:
#define IS_NT      _WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_NT5     _WIN32 && (BOOL)(LOBYTE(LOWORD(GetVersion()))>=5)
#define IS_WIN32S  _WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && _WIN32


// Global Variables:


int			gProcessing = FALSE;
int			gProcessingSchedule = FALSE;
int			gHttpInterface = FALSE;			// true if controlling via http:8000
int			gCloneInstance = FALSE;
int			gStartusingWizard = FALSE;
int			gStartMinimized = FALSE;
short		gHttpPort = 800;
int			gSaved = TRUE;
int			gNoGUI = FALSE;
int			gNoThread = FALSE;
int			gVer2 = FALSE;
int			gNtservice = 0;
int			gSpeed = 45;
int			gCheckUpdate = 0;
int			gConvertLogToW3C_Flag = FALSE;
char		*gApplicationExeName = 0;
long		gUserGraduatedBackgrounds = TRUE;



HINSTANCE 		hInst = NULL; // current instance
HWND			hwndParent = NULL;
HWND    		ghMainDlg = NULL;
HWND    		ghStatusWnd = NULL;
HWND 			ghProgressWnd = NULL;    // handle of progress bar 
HWND			hWndLogListView;
static short	logListView = 0;
static HMENU	hPopupMenu;

HDC             gHDCGlobal;     /* The Screen DC                               */
LONG            iNumColors;    /* Number of colors supported by device        */
LONG            iDepth;    /* Number of colors supported by device        */
INT             iRasterCaps;   /* Raster capabilities                         */
HFONT			hSmall8Font;	// Font for 'fine print' in dialog
HFONT			hSmallFont;	// Font for 'fine print' in dialog
HFONT			hLargeFont;
HFONT			hSmallBoldFont;	// Font for 'fine print' in dialog
HFONT			hButtonFont;	// Font for 'fine print' in dialog
long			about_Special = FALSE;

DWORD			dwVersion;


HBITMAP			hbmBackGround = NULL,
				hbmBackGroundRoll = NULL,
				hbmBackGroundHit = NULL,
				hbmProgress1 = NULL,
				hbmProgress1B = NULL,
				hbmProgress1C = NULL,
				hbmProgress2 = NULL;
BITMAP			bmBackGround,
				bmBackGroundRoll,
				bmBackGroundHit,
				bmProgress1, bmProgress2;

HPALETTE		hMainPal = NULL, 
				hMainPalOld = NULL;

long	buttons_coord[8][4];

long	toolbar_width = 838;
long	toolbar_mainheight = 168;
long	toolbar_listviewtop = 95;
long	toolbar_infopos = 12;
long	toolbar_service_xpos = 620;
long	toolbar_statusxpos = 620;		// debug status output
char	toolbar_service_str[64];

long	toolbar_newskin = 0;

long	toolbar_infocolor = 0xffffff;
long	toolbar_statuscolor = 0xffffff;//0x00ffff;

#define	TOOLBAR_WIDTH		toolbar_width
#define	TOOLBAR_MAINHEIGHT	toolbar_mainheight
#define	LISTVIEW_TOP		toolbar_listviewtop

void StopProcessing( void );
void ButtonBarUpdateInfoLine( HWND hWnd );

char szTitle[32]	= "iReporter"; // The title bar text
char *szDemoTitle   = "iReporter Demo"; // The title bar text
#define		BACKGROUND_COLOR		0x010010


char *szAppName;

int		bWinNT = FALSE;

#define	ABOUTLOGO		"IDB_ABOUTENT"
#define	SPLASHLOGO		"IDB_SPLASHENT"

static const char s_DefaultSettingsFileName[]="Untitled.conf";
static bool s_PrefsFileIsNew(true);


typedef BOOL (__stdcall *MYSWLA)(HWND,COLORREF,DWORD,DWORD); 

HINSTANCE huser32Lib; 
MYSWLA  User32_SetLayeredWindowAttributes = NULL;

void LoadWin2000Calls( void )
{
	huser32Lib = LoadLibrary("user32");
	User32_SetLayeredWindowAttributes = (MYSWLA) GetProcAddress(huser32Lib, "SetLayeredWindowAttributes"); 
}


long GetLastErrorTxt( char *p )
{
	long	er = GetLastError();
    DWORD dwRet =	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
		0,
		er,
		LANG_NEUTRAL,
		(LPTSTR)p,
		255,
		0
	);
	return er;
}

size_t GetString( long id, char *dest, size_t size )
{
	return (size_t)LoadString( hInst, id, dest, size );
}

char *ReturnString( long id )
{
	static char tstring[256];
	LoadString( hInst, id, tstring, 255 );
	return tstring;
}


void StatusSet( char *txt )
{
	if ( ghStatusWnd && gNoGUI==FALSE )
	{
		SendMessage( ghStatusWnd, SB_SETTEXT, (WPARAM) 0, (LPARAM) txt ); 
		UpdateWindow( ghStatusWnd );
		//RefreshStatusBar();
	}
	if ( txt ) OutDebug( txt );
}

int StatusSetf( char *msg, ... )
{
	va_list		args;
	char lineout[4000];

	if ( msg ){
		va_start( args, msg );
		vsprintf( lineout, msg, args );
		va_end( args );
		StatusSet( lineout );
	}
	return 1;
}

// if ID is 0 , then put a blank status in
int StatusSetID( int id, ... )
{
	va_list		args;
	char lineout[4000], txt[512];

	if ( id ){
		if ( id )
			GetString( id, txt, 512 );
		else
			txt[0] = 0;

		if ( txt ){
			va_start( args, id );
			vsprintf( lineout, txt, args );
			va_end( args );
			StatusSet( lineout );
		}
	}
	return 1;
}


int MsgBox_Error( int id, ... )
{
	char lineout[4000], txt[512];

	if ( id ){
		va_list		args;
		GetString( id, txt, 512 );
		va_start( args, id );
		vsprintf( lineout, txt, args );
		ErrorMsg( lineout );
		va_end( args );
	}
	return 1;
}

int MsgBox_yesnocancel( long id, char *txt )
{
	char	msg[2048], string[512], *pt;
	long	ret;

	GetString( id, string, 512 );
	pt = mystrchr( string, ':' );

	*pt = 0;
	if ( txt )
		sprintf( msg, pt+1, txt );
	else
		sprintf( msg, pt+1 );

	ret = MessageBox( GetFocus(), msg, string, MB_YESNOCANCEL|MB_ICONQUESTION );

	return ret;
}


int MsgBox_yesno( long id, ... )
{
	char	msg[2048], txt[512], *pt;
	long	ret;
	va_list		args;

	GetString( id, txt, 512 );
	pt = mystrchr( txt, ':' );

	*pt = 0;
	pt++;

	va_start( args, id );
	vsprintf( msg, pt, args );

	ret = MessageBox( GetFocus(), msg, txt, MB_YESNO|MB_ICONQUESTION );
	va_end( args );

	return ret;
}


//-------------------- Begin Settings File Stuff---------------------------------
static bool DoReadPrefs( Prefs* p )
{
	bool ok( DoReadPrefsFile( gPrefsFilename, p )!=0 );		// non-zero return val means read worked

	if( ok )
	{
		gSaved=TRUE;
		s_PrefsFileIsNew=false;
	}

	return ok;
}


static void ReadDefaultPrefs()
{
	GetReg_PrefsFile( gPrefsFilename );

	if( gPrefsFilename[0] && DoReadPrefs( &MyPrefStruct) )
	{
		return;
	}

	strcpy( gPrefsFilename, "default.conf" );

	if( !DoReadPrefs( &MyPrefStruct) )
	{
		strcpy( gPrefsFilename, s_DefaultSettingsFileName );
		DoReadPrefs( &MyPrefStruct);
	}
}


static long ReadCurrentPrefs()
{
	if( DoReadPrefs( &MyPrefStruct) )
	{
		SetReg_PrefsFile( gPrefsFilename );
		ChangeMainWindowTitle();
		AddToMRUList( hwndParent );
		return 1;
	}
	return 0;
}


static bool SaveCurrentPrefs( bool doSaveAs )
{
	// if we're doing a save as and user hit cancel from file name dialogue
	if( doSaveAs && !GetSavePrefsName( gPrefsFilename ) )
	{
		return false;
	}

	// if couldn't save for some reason (ret val 0 means save worked)
	if( DoSavePrefsFile( gPrefsFilename, &MyPrefStruct )!=0 )
	{
		char errStr[256];

		if ( GetLastErrorTxt( errStr ) )
		{
			ErrorMsg( "Saving settings failed because\n%s", errStr );
		}

		return false;
	}

	gSaved=TRUE;
	s_PrefsFileIsNew=false;
	SetReg_PrefsFile( gPrefsFilename );
	ChangeMainWindowTitle();

	return true;
}


// if current settings are not saved, ask user if to save them or not or cancel.
static long AskSaveCurrentPrefs()
{
	if( !gSaved && !gNoGUI )
	{
		long ret( MsgBox_yesnocancel( IDS_MSG_SAVESETTINGS, gPrefsFilename ) );

		if( ret==IDCANCEL )
		{
			return 0;
		}

		if( ret==IDYES )
		{
			return SaveCurrentPrefs( s_PrefsFileIsNew );
		}
	}
	return 1;
}
//-------------------- End Settings File Stuff---------------------------------


long CLIInitApp(void)
{	
	long success=FALSE, demo = FALSE;

	// Show Stopper bug fixed, who ever forgot this before, "grrrrrrrrrrrrrrrrrrrr"
	// We will make/move this into a CommonInit() later in v5.
	std::string fileName( gPath );
	fileName += CQSettings::SETTINGS_APP_WIDE_DEFAULT;
	CQSettings::TheSettings().OpenAppWideSettings( fileName.c_str() );    

	initLookupGrid();

	demo = IsDemoReg();
	if ( !demo )
		success = DoRegistrationProcedure();

	/*-------------        Check registration ID here */
	if ( success || demo ){
		/* if preferences are NOT valid then create default ones over it. */
		if ( strncmp( "APP", MyPrefStruct.prefID, 4 ) ){
			StatusSetID( IDS_MAKEDEFAULT );
			CreateDefaults(&MyPrefStruct,1);
			if (!DoReadAsciiPrefs( gPrefsFilename, &MyPrefStruct )) {
				StatusSetID( IDS_MAKEDEFAULT );
				CreateDefaults(&MyPrefStruct,1);
			}
		}
		StatusSetID( IDS_OK );
	} else
		OutDebug( "not registered" );
	return success;
}



// --------------------------------- CLI MODE -------------------------------------
int DoCLIMain( int argc, char **argv )
{
	int			count=1, success;

	CreateDefaults(&MyPrefStruct,1);
	
	OutDebug( "Process args..." );
	count=0;
	glogFilenamesNum = 0;

	OutDebug( "CLI Init..." );
	success = CLIInitApp();

	/* Handle the init for App */
	if ( success )
	{
		OutDebug( "Process line..." );
		while ( count < argc ){
			ProcessPrefsLine( argc, argv, 0, count, &MyPrefStruct );
			count++;
		}

		gNoThread = TRUE;
		// Process the LOG NOW DUDE.
		if ( glogFilenamesNum )
			ProcessLog( FALSE );
	
	} else {
		OutDebug( "Init Failed..." );
		//printf( "Init failed\n");
	}
	return 0;
}
// --------------------------------- CLI MODE -------------------------------------


int DoNTService(int argc, char* argv[])
{
	int err = 0;
	char newPath[256];

	GetEXEPath( newPath );
	
	PathFromFullPath( newPath,0 );
	SetCurrentDirectory( newPath );
	mystrcpy( gPath, newPath );
	GetApplicationDataPath( gDataPath );
	GetPersonalDataPath( gPersonalPath );


	initLookupGrid();
	stripxchar( gPath, '\\' );
	CreateDefaults(&MyPrefStruct,1);

	InitReadSchedule( 1 );
	OutDebug( "NT init" );

	/* Handle the init for App */
	if ( CLIInitApp() ){
		OutDebug( "InitNTService" );
		err = InitNTService( argc, argv, TRUE, newPath );

		if ( err )
			UserMsg( newPath );
	} else
		OutDebug( "CLIInit failed" );
	return err;
}

//
//  FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
//
//  PURPOSE: Entry point for the application.
//
//  COMMENTS:
//
//	This function initializes the application and processes the
//	message loop.
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG		msg;
	HACCEL	hAccelTable;
	STARTUPINFO StartupInfo;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	//int		hConHandle;
	long	i;
	int argc = __argc;
	char ** argv = __argv, *p;
	char name[255];

	OutDebug( "Start..." );

	if( !hInstance ) {
		OutDebug( "Getting instance" );
		hInstance = GetModuleHandle (NULL);
	}

	szAppName = szTitle;
	gApplicationExeName = argv[0];

	GetStartupInfo( &StartupInfo );
	if (GetVersion() < 0x80000000)
		bWinNT = 1;
	
	OutDebug( "Process opts" );
	PathFromFullPath( argv[0], gPath );
	GetApplicationDataPath( gDataPath );
	GetPersonalDataPath( gPersonalPath );

	// default http interface is on.
	gHttpInterface = TRUE;

	if ( p = strstr( lpCmdLine, "-nohttp" ) ){
		gHttpInterface = FALSE;
		*p = ' ';
	}
	if ( p = strstr( lpCmdLine, "-port " ) ){
		gHttpInterface = TRUE;
		gHttpPort = atoi( p+6 );
		*p = ' ';
	}

	name[0] = 0;
	GetReg_Prefs( "debug", name );
	p = strstri( lpCmdLine, "-debug" );

	if ( p || name[0] == 'y' ){
		gDebugPrint = TRUE;
	    OutDebug("App is in debug mode.");
	}

	if ( p = strstr( lpCmdLine, "-clone" ) ){
		gCloneInstance = TRUE;
		gHttpInterface = FALSE;
		OutDebug( "Clone mode" );
	}

	if ( p = strstr( lpCmdLine, "-minimize" ) ){
		gCloneInstance = TRUE;
		gStartMinimized = TRUE;
		nCmdShow = SW_MINIMIZE;
	}

#ifdef DEF_DEBUG
	gHttpInterface = TRUE;
	gDebugPrint = TRUE;
#endif


	if ( strstr( lpCmdLine, "-nogui" ) || strstr( lpCmdLine, " -" ) ){
		gNoGUI = TRUE;
	} else
	if ( strstr( lpCmdLine, "-cli" ) ){
		gNoGUI = TRUE;
		gNoThread = TRUE;
	}

	if ( !strcmpd( "-i", lpCmdLine  ) ||
		 !strcmpd( "-u", lpCmdLine  ) ||
		 !strcmpd( "-nt", lpCmdLine  ) ||
		 !strcmpd( "-bootinstall", lpCmdLine  ) ||
		 !strcmpd( "-ntservice", lpCmdLine  )
		 ){
		gNoGUI = TRUE;
		gNtservice = TRUE;
		OutDebug( "NT Service cmd" );
		return DoNTService( argc, argv );
	}

	if ( !gNoGUI )
	{
		HWND w; 
		if( w = GetForegroundWindow() ){
			GetWindowText( w , name, 255 );
			if ( !strcmpd( "Command Prompt", name ) || !strcmpd( "MS-DOS Prompt", name )  ){
				gNoGUI = TRUE;
			}
		}
		if ( gHttpInterface ){
			sprintf( name, "%d", gHttpPort );
			SetReg_Prefs( "webqa", name );
		}
	}
	
	if ( gNoGUI ){
		HANDLE sout=GetStdHandle( STD_OUTPUT_HANDLE );		//STD_ERROR_HANDLE,STD_OUTPUT_HANDLE
		if ( sout )
		    GetConsoleScreenBufferInfo( sout, &csbi );

		OutDebug( "Do CLI..." );

/* LPWSTR * CommandLineToArgvW(  LPCWSTR lpCmdLine,  // pointer to a command-line string//,  int *pNumArgs       // receives the argument count// ); */
		DoCLIMain( argc, argv );

		OutDebug( "Exiting" );
		exit(0);
	}

	LoadWin2000Calls();

	InitCommonControls();

	if (!hPrevInstance) {
		// Perform instance initialization:
		if (!InitApplication(hInstance)) {
			OutDebug( "Exiting, InitApplication error" );
			return (FALSE);
		}
	}

	OutDebug( "Init APP" );

	
	// Handle the init for App
	if ( InitApp() ){
		char skinname[256];

		OutDebug( "Init APP Done" );

		if ( GetReg_Prefs( "skin", skinname ) ){
			if ( mystrlen(skinname) ){
				OutDebugs( "Trying to load skin <%s>", skinname );
				LoadSkin( NULL, skinname );
				OutDebug( "Skin loaded" );
			}
		}

		OutDebug( "Init Instance..." );
		// Perform application initialization:
		if (!InitInstance(hInstance, nCmdShow)) {
			OutDebug( "Exiting, InitInstance error" );
			return (FALSE);
		}

		OutDebug( "Init Instance Done" );

		if ( IsDemoReg() )
			mystrcpy( szTitle, szDemoTitle );

		OutDebug( "Load Accel..." );

		hAccelTable = LoadAccelerators (hInstance, "APP" );
	
		// handle dropped filename on EXE
		if ( i = strlen( lpCmdLine ) && lpCmdLine[0] != '-' ){
			char *p = lpCmdLine;
			OutDebugs( "Handle Drag/Drop file <%s>", lpCmdLine );
			if ( *p == '\"' )
				p++;

			strcpy( gPrefsFilename, p );

			if ( p = strrchr( gPrefsFilename, '\"' ) )
				*p = 0;

			OutDebugs( "Read prefs file <%s>", gPrefsFilename );
			ReadCurrentPrefs();
		} else {
			OutDebugs( "Read default prefs" );
			ReadDefaultPrefs();
			LoadMRUList( hwndParent );
			ChangeMainWindowTitle();
		}

		OutDebug( "Read reg vars" );

		{	char dateString[16]; long ct;

			if( GetReg_Prefs( "updatecheck", dateString ) ){
				i = myatoi( dateString );
				ct = (long)GetCurrentCTime();
				if ( (ct - i) > 5 ){
					GetReg_Prefs( "updatecheckflag", dateString );
					if( dateString[0] == 'y' )	gCheckUpdate = 1;
					if( gCheckUpdate )
						CheckforUpdate();
					sprintf( dateString, "%d", (long)(ct+(ONEDAY*7)) );
					SetReg_Prefs( "updatecheck", dateString );
				}
			}

			GetReg_Prefs( "WizardStart", dateString );
			if( dateString[0] = 'y' || i==0 )
				gStartusingWizard = TRUE;

			if ( GetReg_Prefs( "dnr_ttl", dateString ) )
				MyPrefStruct.dnr_ttl = (short)myatoi( dateString );
		
		}

		ChangeMainWindowTitle();

		// Test only
		//ErrorMsg( "Err test, port = %d", gHttpPort );
		//CautionMsg( "Caution test, port = %d", gHttpPort );
		//NotifyMsg( "Notify test, port = %d", gHttpPort );
		
		OutDebug( "Start Msg loop" );
		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0)) {
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}



	
	} else {
		gSaved = TRUE;
#ifdef	DEF_APP_DEMO
		if ( 1 )
#else
		if ( IsDemoReg() )
#endif
			ShowHTMLShortcut( NULL, "https://www.store.com/", NULL );
		else
			MsgBox_Error( IDS_ERR_REGFAILED );
	}
	ExitApp();
	return (msg.wParam);

	//lpCmdLine; // This will prevent 'unused formal parameter' warnings
}

//
//  FUNCTION: MyRegisterClass(CONST WNDCLASS*)
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
// 	This function and its usage is only necessary if you want this code
// 	to be compatible with Win32 systems prior to the 'RegisterClassEx'
//	function that was added to Windows 95. It is important to call this function
// 	so that the application will get 'well formed' small icons associated
// 	with it.
//
ATOM MyRegisterClass(CONST WNDCLASS *lpwc)
{
	typedef ATOM (CALLBACK* LPREGCLASSFN)(CONST WNDCLASSEX*);

	HMODULE hMod;
	LPREGCLASSFN proc;
	WNDCLASSEX wcex;

	hMod = GetModuleHandle ("USER32");
	if (hMod != NULL) {

#if defined (UNICODE)
		proc = (LPREGCLASSFN)GetProcAddress (hMod, "RegisterClassExW");
#else
		proc = (LPREGCLASSFN)GetProcAddress (hMod, "RegisterClassExA");
#endif

		if (proc != NULL) {
			wcex.style         = lpwc->style;
			wcex.lpfnWndProc   = lpwc->lpfnWndProc;
			wcex.cbClsExtra    = lpwc->cbClsExtra;
			wcex.cbWndExtra    = lpwc->cbWndExtra;
			wcex.hInstance     = lpwc->hInstance;
			wcex.hIcon         = lpwc->hIcon;
			wcex.hCursor       = lpwc->hCursor;
			wcex.hbrBackground = lpwc->hbrBackground;
            wcex.lpszMenuName  = lpwc->lpszMenuName;
			wcex.lpszClassName = lpwc->lpszClassName;

			// Added elements for Windows 95:
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.hIconSm = LoadIcon(wcex.hInstance, "SMALLICON");
			
			return (*proc)(&wcex);//return RegisterClassEx(&wcex);
		}
	}
	return (RegisterClass(lpwc));
}



long GetPixelXScale( long x )
{
static	int pixel_xscale=0;
	if ( pixel_xscale == 0 ){
		HWND hWnd;
		HDC hDC;
		hWnd = GetDesktopWindow();
		hDC = GetDC( hWnd );
		pixel_xscale = GetDeviceCaps( hDC, LOGPIXELSY );
		ReleaseDC( hWnd, hDC );
	}
	return (long)((x*pixel_xscale)/96.0);
}

long GetPixelYScale( long y )
{
static	int pixel_yscale=0;
	if ( pixel_yscale == 0 ){
		HWND hWnd;
		HDC hDC;
		hWnd = GetDesktopWindow();
		hDC = GetDC( hWnd );
		pixel_yscale = GetDeviceCaps( hDC, LOGPIXELSY );
		ReleaseDC( hWnd, hDC );
	}
	return (long)((y*pixel_yscale)/96.0);
}

long	du, baseunitX, baseunitY;

long	XDUtoPixel( long dialogunitX )
{
	return (dialogunitX * baseunitX) / 4 ;
}
long	YDUtoPixel( long dialogunitY )
{
	return (dialogunitY * baseunitY) / 8 ;
}
long	XPixeltoDU( long pixelX )
{
	return (pixelX * 4) / baseunitX  ;
}
long	YPixeltoDU( long pixelY )
{
	return (pixelY * 8) / baseunitY ;
}
	
/*pixelX = (dialogunitX * baseunitX) / 4 
pixelY = (dialogunitY * baseunitY) / 8 
 
Similarly, to convert from pixels to dialog units, an application applies the following formulas: 

dialogunitX = (pixelX * 4) / baseunitX 
dialogunitY = (pixelY * 8) / baseunitY 
*/


//
//  FUNCTION: InitApplication(HANDLE)
//
//  PURPOSE: Initializes window data and registers window class 
//
//  COMMENTS:
//
//       In this function, we initialize a window class by filling out a data
//       structure of type WNDCLASS and calling either RegisterClass or 
//       the internal MyRegisterClass.
//
BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS  wc;
    HWND      hwnd = 0;

	du = GetDialogBaseUnits();
	baseunitY = du>>16;
	baseunitX = du&0xffff;

    // Win32 will always set hPrevInstance to NULL, so lets check
    // things a little closer. This is because we only want a single
    // version of this app to run at a time
	if ( !gCloneInstance )
	    hwnd = FindWindow (szAppName, NULL);


    if (hwnd) {
		hwndParent = hwnd;
        // We found another version of ourself. Lets defer to it:
        if (IsIconic(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }
        SetForegroundWindow (hwnd);

        // If this app actually had any functionality, we would
        // also want to communicate any action that our 'twin'
        // should now perform based on how the user tried to
        // execute us.
        return FALSE;
	}

    // Fill in window class structure with parameters that describe
    // the main window.
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC)WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon (hInstance, "IDI_AAA_APPICON" );
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = 0;		//CreateSolidBrush( BACKGROUND_COLOR );		//GetStockObject(BLACK_BRUSH);
    // Since Windows95 has a slightly different recommended
    // format for the 'Help' menu, lets put this in the alternate menu like this:
    wc.lpszClassName = szAppName;
	if ( bWinNT )
		wc.lpszMenuName  = "IREPORTERNT";
	else
		wc.lpszMenuName  = "IREPORTER";

    // Register the window class and return success/failure code.
    if (IS_WIN95) {
		MyRegisterClass(&wc);
    } else {
		RegisterClass(&wc);
    }

     gHDCGlobal  = GetDC (NULL);
     iRasterCaps = GetDeviceCaps(gHDCGlobal, RASTERCAPS);
     iRasterCaps = (iRasterCaps & RC_PALETTE) ? TRUE : FALSE;
     if (iRasterCaps)
         iNumColors = GetDeviceCaps( gHDCGlobal, SIZEPALETTE);
     else
         iNumColors = GetDeviceCaps( gHDCGlobal, NUMCOLORS);

	 iDepth = GetDeviceCaps( gHDCGlobal, BITSPIXEL);
     ReleaseDC (NULL,gHDCGlobal);
	 return TRUE;
}


void InitFader( HWND hDlg )
{
	if ( IS_NT5 ){
		SetWindowLong( hDlg, GWL_EXSTYLE, GetWindowLong(hDlg,GWL_EXSTYLE) | WS_EX_LAYERED );
		User32_SetLayeredWindowAttributes( hDlg, 0, 255, LWA_ALPHA );
	}
}

void FadeOut( HWND hDlg )
{
	long i;
	if ( IS_NT5 ){
		i = 255;
		while( i>0 ){
			(User32_SetLayeredWindowAttributes) ( hDlg, 0, i, LWA_ALPHA );
			Sleep( 30 );
			i -= gSpeed;
		}
	}
}


/*
The problem can be avoided on Windows 95 and Windows NT by using a static window
with SS_BITMAP and SS_CENTERIMAGE flags instead of using a dialog box window to 
hold the bitmap.
*/

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window 
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	long	w,h;
	
	hInst = hInstance; // Store instance handle in our global variable

	//DialogBox( hInst, "MAINWINDOW", 0, (DLGPROC)WndProc);

	w = TOOLBAR_WIDTH;
	h = TOOLBAR_MAINHEIGHT;

	//OutDebugs( "Createwindow... : %s,%s,%d,%d,%08lx", szAppName, szTitle, w, h,hInstance );

	if ( (hWnd = CreateWindowEx( 0, szAppName, szTitle, WS_TILEDWINDOW ,
//	if ( (hWnd = CreateWindow( szAppName, szTitle, WS_TILEDWINDOW ,
		CW_USEDEFAULT, 0, w, h, NULL, NULL, hInstance, NULL)) == NULL ){		//WS_EX_LAYERED

		OutDebug( "cant create main window" );
		return (FALSE);
	}
	hwndParent = hWnd;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	if ( !(ghStatusWnd = CreateXStatusWindow( 2,hwndParent )) )
		UserMsg( "cant open status window" );

	CreateProgressBar( 100 );

	return (TRUE);
}



//
//   FUNCTION: CenterWindow(HWND, HWND)
//
//   PURPOSE: Centers one window over another. 
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
// 		This functionwill center one window over another ensuring that 
//		the placement of the window is within the 'working area', meaning 
//		that it is both within the display limits of the screen, and not 
//		obscured by the tray or other framing elements of the desktop.
BOOL CenterWindow (HWND hwndChild, HWND lhwndParent)
{
	RECT    rChild, rParent, rWorkArea;
	int     wChild, hChild, wParent, hParent;
	int     xNew, yNew;
	BOOL 	bResult;

	if ( hwndChild && lhwndParent ){
		// Get the Height and Width of the child window
		GetWindowRect (hwndChild, &rChild);
		wChild = rChild.right - rChild.left;
		hChild = rChild.bottom - rChild.top;

		// Get the Height and Width of the parent window
		GetWindowRect (lhwndParent, &rParent);
		wParent = rParent.right - rParent.left;
		hParent = rParent.bottom - rParent.top;

		// Get the limits of the 'workarea'
		bResult = SystemParametersInfo(
			SPI_GETWORKAREA,	// system parameter to query or set
			sizeof(RECT),
			&rWorkArea,
			0);
		if (!bResult) {
			rWorkArea.left = rWorkArea.top = 0;
			rWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
			rWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
		}

		// Calculate new X position, then adjust for workarea
		xNew = rParent.left + ((wParent - wChild) /2);
		if (xNew < rWorkArea.left) {
			xNew = rWorkArea.left;
		} else if ((xNew+wChild) > rWorkArea.right) {
			xNew = rWorkArea.right - wChild;
		}

		// Calculate new Y position, then adjust for workarea
		yNew = rParent.top  + ((hParent - hChild) /2);
		if (yNew < rWorkArea.top) {
			yNew = rWorkArea.top;
		} else if ((yNew+hChild) > rWorkArea.bottom) {
			yNew = rWorkArea.bottom - hChild;
		}

		// Set it, and return
		return SetWindowPos (hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	return FALSE;
}

int CenterWindowOnScreen (HWND hwnd )
{
	RECT    rChild;
	int     wChild, hChild, screenW, screenH;
	int     xNew, yNew;

	if ( hwnd ){
		// Get the Height and Width of the child window
		GetWindowRect (hwnd, &rChild);
		wChild = rChild.right - rChild.left;
		hChild = rChild.bottom - rChild.top;

		screenW = GetSystemMetrics(SM_CXFULLSCREEN);
		screenH = GetSystemMetrics(SM_CYFULLSCREEN);

		xNew = screenW/2 - (wChild/2);
		yNew = screenH/2 - (hChild/2);

		// Set it, and return
		return SetWindowPos (hwnd, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	return FALSE;
}


void ChangeMainWindowTitle( void )
{
	char	txt[MAX_PATH], fname[64];
	int		ret;
	
	if ( hwndParent ) {
		FileFromPath( gPrefsFilename, fname );

		if ( gSaved )
			wsprintf( txt, "%s - %s", szTitle, fname );
		else
			wsprintf( txt, "%s - *%s", szTitle, fname );
			
		ret = SetWindowText( hwndParent, txt );
		
		if ( ret == 0)
			UserMsg( "Error SetWindowText()" );
		UpdateWindow( hwndParent );
	}
}


void ProgressChangeMainWindowTitle( long percent, char *etaStr )
{
	char	txt[MAX_PATH], fname[64];
	int		ret;
	
	if ( hwndParent  && gNoGUI==FALSE ) {
		FileFromPath( gPrefsFilename, fname );

		if ( gSaved )
			wsprintf( txt, "%d%% %s - %s %s", percent, szTitle, fname, etaStr );
		else
			wsprintf( txt, "%d%% %s - *%s %s", percent, szTitle, fname, etaStr );
			
		ret = SetWindowText( hwndParent, txt );
		//UpdateWindow( hwndParent );
	}
}


// ---------------------------

void ShowHTMLURL( HWND hWnd, char *doc, char *param )
{
	char *oper = "open";
	char file[2048];
	char *dir = "c:\\";
	
	if( !strchr( doc, ':' ) )
	{
		GetCurrentDirectory( 256, file );
		strcat( file, "\\" );
		strcat( file, doc );
		DateFixFilename( file, NULL );
	} else
		DateFixFilename( doc, file );


	OutDebug( file );

	ShellExecute( hWnd, oper,file,param,dir, SW_SHOWNORMAL  );
}


DWORD WINAPI ShowUrlThread( LPVOID lpParam )
{
	if ( lpParam ){
		char *text = (char*)lpParam;
		ShowHTMLURL( hwndParent, text, 0 );
		free( text );
	}
	return 1;
}

long SpawnShowShellFile( char *url )
{
	DWORD dwThreadId, dwThrdParam = 1;
	HANDLE 		ghThread = NULL;
	char *param;

	param = (char*)malloc( 1024 );

	mystrcpy( param, url );

	ghThread = CreateThread( 
		NULL,                        // no security attributes 
		0,                           // use default stack size  
		ShowUrlThread,     // thread function 
		param,                // argument to thread function 
		0,                           // use default creation flags 
		&dwThreadId);                // returns the thread identifier  
    
	if (ghThread) {
		SetThreadPriority( ghThread, THREAD_PRIORITY_BELOW_NORMAL );
	}
	return(1);
}



void EditLogFile( HWND hWnd, char *doc, char *param )
{
	char *oper = "edit ";
	char file[2048];
	char *dir = "c:\\";
	long ret;
	
	DateFixFilename( doc, file );

	OutDebug( file );

	ret = (long)ShellExecute( hWnd, oper,file,param,dir, SW_SHOWNORMAL  );
	if( ret < 32 )
		ret = (long)ShellExecute( hWnd, "open", "notepad", file, dir, SW_SHOWNORMAL );
}

#include <intshcut.h>



void ShowHTMLShortcut( HWND hWnd, char *doc, char *param )
{
	char *oper = "open";
	char *dir = "c:\\";
	char newparam[2048];

	DateFixFilename( param, newparam );

	OutDebugs( "Showing %s %", doc, param );

    SHELLEXECUTEINFO sei={0};
	sei.cbSize=sizeof(SHELLEXECUTEINFO);
	if(!hWnd)
	{
		sei.fMask=SEE_MASK_FLAG_NO_UI;
	}
	sei.hwnd=hWnd;
	sei.lpVerb=oper;
	sei.lpFile=doc;
	sei.lpParameters=newparam;
	sei.lpDirectory=dir;
	sei.nShow=SW_SHOW;
	ShellExecuteEx(&sei);
}



void ExplorerView( char *doc )
{
	ShowHTMLShortcut( hwndParent, "iexplore", doc );
}
					
int PDFView( char *doc )
{
	char *oper = "open";
	char *dir = "c:\\";
	char newparam[2048];
	int result;

	DateFixFilename( doc, newparam );
	OutDebug( newparam );

	result = (long)ShellExecute( hwndParent, oper, newparam, "", dir, SW_SHOWNORMAL );
	return result;
}

#define CHATURL "http://www.mircx.com/cgi-bin/jirc.cgi?NickName=%s&AllowSound=false&AllowURL=false&BorderHsp=0&BorderVsp=0&Channel1=iReporter&DisplayAbout=false&DisplayConfigChannel=false&DisplayConfigChannelPass=false&DisplayConfigMisc=false&DisplayConfigNickPass=false&DisplayConfigPort=false&DisplayConfigRealName=false&DisplayConfigServer=false&DisplayConfigServerPass=false&DisplaySoundControl=false&IgnoreChannelChangeMsg=true&IgnoreModeChange=true&IgnoreMOTD=true&IgnoreServerMsg=true&NoConfig=true&NOS=false&ServerName1=jirc.mircx.com&ServerPort=6667&TextScreenColor=white&TitleBackgroundColor=lightGray"

void ConnectToChatServer( HWND hWnd )
{
	char url[2048];
	char *username = NULL;
	char name[128]; 

	if ( !username ){
		unsigned long len(127);
		GetUserName( name, &len );
		if ( len )
			username = name;
		else
			username = GetRegisteredUsername();
	}

	sprintf( url, CHATURL, username );

	ShowHTMLURL( hWnd, url, 0 );
}


short ExplorerHelp( void )
{
	char	fullname[MAX_PATH];
	
	sprintf( fullname, "%s\\%s", gPath, "Manual\\index.htm" );
	if ( GetFileLength( fullname ) ){
		ExplorerView( fullname );
		return 1;
	}
	GetCurrentDirectory( 256, fullname );
	strcat( fullname,  "\\Manual\\guide.pdf" );
	if ( GetFileLength( fullname ) ){
		int res = PDFView( fullname );
		return 1;
	}

	GetCurrentDirectory( 256, fullname );
	strcat( fullname,  "\\Manual\\manual.pdf" );
	if ( GetFileLength( fullname ) ){
		int res = PDFView( fullname );
		return 1;
	}

	sprintf( fullname, "%s\\%s", gPath, "Manual\\guide.pdf" );
	if ( GetFileLength( fullname ) ){
		int res = PDFView( fullname );
		return 1;
	}

	return 0;
}
  

					

void PerformMainHelp( HWND hWnd, long helptype )
{
	char	fullname[MAX_PATH];
	BOOL	bGotHelp;
	
	sprintf( fullname, "%s\\%s", gPath, "MAIN.HLP" );
	bGotHelp = WinHelp (hWnd, fullname, helptype,(DWORD)0);
	if (!bGotHelp) {
		UserMsg( "Unable to activate help" );
	}
}
  







void WriteXY8( HWND hDlg, char *text, long x, long y )
{
	HDC		hdc;
    RECT	textrc;
    PAINTSTRUCT ps;
	HGDIOBJ of;

	if ( hDlg && text ){
		hdc = BeginPaint(hDlg, &ps);
		hdc = GetDC (hDlg);

		SetTextColor( hdc, 0x00ffffff );
		SetBkColor( hdc, 0x00 );

		of = SelectObject (hdc, hSmall8Font);

		textrc.left = x; textrc.top = y;
		textrc.right = x+200; textrc.bottom = y+30;

		DrawText( hdc, text, strlen(text), &textrc, DT_LEFT );

		if( of ) SelectObject (hdc, of);

		ReleaseDC( hDlg, hdc );
		EndPaint(hDlg, &ps);
	}
}



// plot text at xy in transparent mode
void WriteXY( HWND hDlg, HFONT font, char *text, long x, long y, unsigned long color )
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
		textrc.right = x+400; textrc.bottom = y+120;

		if ( color > 0x800000 ){
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

void ToolBarWriteLines( char *src, long x, long y )
{
	char *txt;
	char line[256];
	long cy, lines=0;

	txt = line;
	cy = y;
	while( *src ){
		*txt = *src;
		if ( *src == '\n' ){
			*txt++ = 0;
			if ( *line != '\n' ){
				lines++;
				WriteXY8( ghMainDlg, line, toolbar_statusxpos+x, cy );
				cy+=10;
				if ( lines == 7 ){
					cy = y;
					x+=210;
				}
			}
			txt = line;
		} else
			txt++;
		src++;
	}
}



static	int gwSize=0;
static	int gLastSizeHeight=0;
static	int gLastSizeWidth=0;

void SetWindowSize( HWND hWnd, long wSize )
{
	RECT        rc;
	HWND		hDlg = hWnd;
	long		wH = 0, wW = 0, listH;

	if ( (wSize != gwSize) || wSize == -1){
		if ( wSize<0 ) wSize = 1;
		gwSize = wSize;

		if ( hWndLogListView == NULL && (wSize & 0x1) ){
			hWndLogListView = InitLogListView( hWnd, 1 );
			if ( hWndLogListView ){
				ReadLogListAsText( NULL );
			} else
				UserMsg ("Log Listview not created!" );
		}
		GetObjectRect( hWnd, GetDlgItem( hWnd, IDC_LOGLIST ), &rc );
		listH = (rc.bottom-rc.top)+3;

		if ( ghMainDlg ){
			GetWindowRect( ghMainDlg, &rc );
			wH = TOOLBAR_MAINHEIGHT;			///
			wH += (wSize*(listH));
			MoveWindow( ghMainDlg,  rc.left, rc.top, rc.right-rc.left, wH+800, TRUE);
		}
		if ( hwndParent ){
			GetWindowRect( hwndParent, &rc );
			wH = TOOLBAR_MAINHEIGHT;			///
			wH += (wSize*(listH));
			MoveWindow( hwndParent,  rc.left, rc.top, rc.right-rc.left, wH, TRUE);
		}
		wW = rc.right-rc.left;
		gLastSizeHeight = wH;
		gLastSizeWidth = wW;
	}
}



void CycleWindowSize( HWND hWnd )
{
	if( gwSize )
	{
		StatusSetID( IDS_HIDELIST );
		SetReg_Prefs( "List Open", "no" );
	} else {
		StatusSetID( IDS_HIDELIST );
		SetReg_Prefs( "List Open", "yes" );
	}

	SetWindowSize( hWnd, gwSize ^ 0x01 );

   	ButtonBarPaint( ghMainDlg );

	StatusSetID( IDS_OK );
}

void GrowWindowSize( HWND hWnd )
{
	if ( !gwSize )
		CycleWindowSize( hWnd );
}

void ShrinkWindowSize( HWND hWnd )
{
	if ( gwSize )
		CycleWindowSize( hWnd );
}


void CycleWindowInfoPanel( HWND hWnd )
{
	RECT        rc;
	long		wH = 0, wW = 0;

	if ( ghMainDlg ){
		GetWindowRect( ghMainDlg, &rc );
		wW = rc.right-rc.left;
		if ( wW > TOOLBAR_WIDTH ) wW = TOOLBAR_WIDTH; else
			wW = TOOLBAR_WIDTH+200;
		MoveWindow( ghMainDlg,  rc.left, rc.top, wW, rc.bottom-rc.top, TRUE);
	}
	if ( hwndParent ){
		GetWindowRect( hwndParent, &rc );
		wW = rc.right-rc.left;
		if ( wW > TOOLBAR_WIDTH ) wW = TOOLBAR_WIDTH; else
			wW = TOOLBAR_WIDTH+200;
		MoveWindow( hwndParent,  rc.left, rc.top, wW, rc.bottom-rc.top, TRUE);
	}

   	ButtonBarPaint( ghMainDlg );
	StatusSetID( IDS_OK );
}


long LoadSkin( HWND hWnd, char *filename )
{
	FILE *fp;
	char szText[512], *p,
		files[3][256];
	long  b,i, loaded=0;

	if ( fp = fopen( filename, "r" ) ){
		while( !feof( fp ) ){
			szText[0]=0;
			fgets( szText, 2048, fp );
			trimLine( szText );
			loaded = 1;
			if ( szText[0] != '#' ){
				if ( !strcmpd( "toolbarimagenorm" ,szText) ){
					p = mystrchr( szText, '=' );
					if( p ) {
						p++; while( *p == ' ' ) p++;
						CopyFilenameUsingPath( &files[0][0], filename, p );
					}
				} else
				if ( !strcmpd( "toolbarimageroll",szText ) ){
					p = mystrchr( szText, '=' );
					if( p ) {
						p++; while( *p == ' ' ) p++;
						CopyFilenameUsingPath( &files[1][0], filename, p );
					}
				} else
				if ( !strcmpd( "toolbarimagehit",szText ) ){
					p = mystrchr( szText, '=' );
					if( p ) {
						p++; while( *p == ' ' ) p++;
						CopyFilenameUsingPath( &files[2][0], filename, p );
					}
				} else
				if ( !strcmpd( "button_coord",szText ) ){
					long index = 0;
					p = mystrchr( szText, '=' );
					if( p ){
						p++; while( *p == ' ' ) p++;
						b = myatoi( p );
						while( p ){
							p = mystrchr( p, ',' );
							if ( p ){
								p++; while( *p == ' ' ) p++;
								if ( isdigit( *p ) ){
									i = myatoi( p );
									buttons_coord[b][index++] = i;
								}
							}
						}
					}
				} else
				if ( !strcmpd( "toolbar_infopos",szText ) ){
					if( p = mystrchr( szText, '=' ) ){
						p++; while( *p == ' ' ) p++;
						toolbar_infopos = myatoi( p );
					}
				} else
				if ( !strcmpd( "toolbar_service_xpos",szText ) ){
					if( p = mystrchr( szText, '=' ) ){
						p++; while( *p == ' ' ) p++;
						toolbar_service_xpos = myatoi( p );
					}
				} else
				if ( !strcmpd( "toolbar_infocolor",szText ) ){
					if( p = mystrchr( szText, '=' ) ){
						p++; while( *p == ' ' ) p++;
						toolbar_infocolor = (long)HexStr2Ptr( p );
					}
				} else
				if ( !strcmpd( "toolbar_statuscolor",szText ) ){
					if( p = mystrchr( szText, '=' ) ){
						p++; while( *p == ' ' ) p++;
						toolbar_statuscolor = (long)HexStr2Ptr( p );
					}
				} else
				if ( !strcmpd( "toolbar_width",szText ) ){
					if( p = mystrchr( szText, '=' ) ){
						p++; while( *p == ' ' ) p++;
						toolbar_width = myatoi( p );
					}
				} else
				if ( !strcmpd( "toolbar_mainheight",szText ) ){
					if( p = mystrchr( szText, '=' ) ){
						p++; while( *p == ' ' ) p++;
						toolbar_mainheight = myatoi( p );
					}
				} else
				if ( !strcmpd( "toolbar_listviewtop",szText ) ){
					if( p = mystrchr( szText, '=' ) ){
						p++; while( *p == ' ' ) p++;
						toolbar_listviewtop = myatoi( p );
					}
				}
			}
		}
		fclose ( fp );
	}
	if( loaded )
	{
		long	IDs[] = { IDC_PREFS,IDC_SCHED,IDC_PROCESS,IDC_VIEW,IDC_APPHELP,IDC_SHOWLIST, 0 };
		long	i=0, oserr;

		hbmBackGround = (HBITMAP)LoadImage( hInst, &files[0][0], IMAGE_BITMAP,0,0, LR_LOADFROMFILE );
		hbmBackGroundRoll = (HBITMAP)LoadImage( hInst, &files[1][0], IMAGE_BITMAP,0,0, LR_CREATEDIBSECTION|LR_LOADFROMFILE );oserr=GetLastError();
		hbmBackGroundHit = (HBITMAP)LoadImage( hInst, &files[2][0], IMAGE_BITMAP,0,0, LR_CREATEDIBSECTION|LR_LOADFROMFILE );oserr=GetLastError();

		toolbar_newskin = TRUE;
	}
	return loaded;
}




void AddMenuItem( HWND hWnd, long menuNum, long item, char *text, long id )
{
	MENUITEMINFO	mi; HMENU menu,smenu;

	mi.cbSize  = sizeof( MENUITEMINFO );
	if (*text=='-')
		mi.fType = MFT_SEPARATOR;
	else
		mi.fType = MFT_STRING;
	mi.fState = MFS_ENABLED;
	mi.wID = id;
	mi.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
	mi.dwTypeData  = text;  mi.cch = mystrlen(text);
	mi.hbmpChecked =  
	mi.hbmpUnchecked = NULL;

	menu = GetMenu( hWnd );
	smenu = GetSubMenu( menu, menuNum );
	InsertMenuItem( smenu, item, TRUE, &mi );
}


void RemoveSubMenuItem( HWND hWnd, long menuNum, long item )
{
	HMENU menu,smenu, smenu2;
 
	menu = GetMenu( hWnd );
	smenu = GetSubMenu( menu, menuNum );
	smenu2 = GetSubMenu( smenu, item );
	
	RemoveMenu( smenu2, 1, TRUE );
}


void AddSubMenuItem( HWND hWnd, long menuNum, long item, char *text, long pos )
{
	MENUITEMINFO	mi;
	HMENU menu,smenu, smenu2;

	mi.cbSize  = sizeof( MENUITEMINFO );
	mi.fType = MFT_STRING;
	mi.fState = MFS_ENABLED ;
	mi.wID = 2300+pos;
	mi.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE ;
	mi.dwTypeData  = text;  mi.cch = mystrlen(text);
	mi.hbmpChecked =  
	mi.hbmpUnchecked = NULL;
  
	menu = GetMenu( hWnd );
	smenu = GetSubMenu( menu, menuNum );
	smenu2 = GetSubMenu( smenu, item );
	
	InsertMenuItem( smenu2, pos, TRUE, &mi );
}

void RenameSubMenuItem( HWND hWnd, long menuNum, long item, long pos, char *text )
{
	HMENU menu,smenu, smenu2;
  
	menu = GetMenu( hWnd );
	smenu = GetSubMenu( menu, menuNum );
	smenu2 = GetSubMenu( smenu, item );
	
	ModifyMenu( smenu2, pos, MF_BYPOSITION, 2300+pos, text );
}

void CheckedMenuItem( HWND hWnd, long menuNum, long item, long check )
{
	MENUITEMINFO	mi; HMENU menu,smenu;

	mi.cbSize  = sizeof( MENUITEMINFO );
	mi.fMask =  MIIM_STATE;

	menu = GetMenu( hWnd );
	smenu = GetSubMenu( menu, menuNum );
	if ( check )
		mi.fState = MFS_ENABLED | MFS_CHECKED;
	else
		mi.fState = MFS_ENABLED | MFS_UNCHECKED;

	SetMenuItemInfo ( smenu, item, TRUE, &mi );
	DrawMenuBar( hWnd );
}


#define		MRU_MENU_ITEM		4

char	recentPrefsFiles[10][256];
long	recentPrefsnum = 0;

void LoadFromMRU( long num )
{
	char oldprefs[512];

	if( num < 10 )
	{
		mystrcpy( oldprefs, gPrefsFilename );		// back up current name
		mystrcpy( gPrefsFilename, recentPrefsFiles[num] );

		if( ReadCurrentPrefs() == 0 ){
			if ( GetFileLength(gPrefsFilename) == 0 ){
				MsgBox_Error( IDS_ERR_NOSETTINGS, gPrefsFilename );
			} else {
				MsgBox_Error( IDS_ERR_SETTINGSFAILED, gPrefsFilename );
			}
			// bring back the orig name
			mystrcpy( gPrefsFilename, oldprefs );
		}
	}
}

void LoadMRUList( HWND hWnd )
{
	char	temp[255], temp2[255];
	long	i, done = 0;

	for(i=0; i<10; i++){
		sprintf( temp, "recent_%d", i );
		if ( GetReg_Prefs( temp, temp2 ) ){
			mystrcpy( recentPrefsFiles[i], temp2 );
			sprintf( temp, "&%d %s", i+1, temp2 );
			if ( recentPrefsnum == 0 )
				RenameSubMenuItem( hWnd, 0, MRU_MENU_ITEM, i, temp );
			else
				AddSubMenuItem( hWnd, 0, MRU_MENU_ITEM, temp, i );
			recentPrefsnum++;
		}
	}
	if ( !recentPrefsnum ){
		mystrcpy( recentPrefsFiles[0], gPrefsFilename );
		sprintf( temp, "&1 %s", gPrefsFilename );
		RenameSubMenuItem( hWnd, 0, MRU_MENU_ITEM, 0, temp );
		recentPrefsnum++;
	}
}

void AddToMRUList( HWND hWnd )
{
	char	temp[255];
	long	i, index=1;

	// if it exists inthe list, dont add it.
	for( i=0; i<10; i++ ){
		if ( !strcmpd( gPrefsFilename, recentPrefsFiles[i] ) )
			return;
	}

	// rotate list
	for( i=9; i>0; i-- ){
		mystrcpy( recentPrefsFiles[i], recentPrefsFiles[i-1] );
		//RemoveSubMenuItem( hWnd, 0, 0 );
	}
	mystrcpy( recentPrefsFiles[0], gPrefsFilename );

	// save and redraw list
	for(i=0; i<10; i++){
		if ( mystrlen(recentPrefsFiles[i]) >1 ){
			sprintf( temp, "recent_%d", i );
			SetReg_Prefs( temp, recentPrefsFiles[i] );
			sprintf( temp, "&%d %s", index, recentPrefsFiles[i] );
			if ( GetFileLength( recentPrefsFiles[i] ) ){
				index++;
				if ( i < recentPrefsnum )
					RenameSubMenuItem( hWnd, 0, MRU_MENU_ITEM, i, temp );
				else
					AddSubMenuItem( hWnd, 0, MRU_MENU_ITEM, temp, i );
			}
		}
	}

	if ( recentPrefsnum < 10 )
			recentPrefsnum++;
}


void InstallManualService( HWND hDlg )
{
	char *argv[] = { "X", "-i", 0 }; long err; char msg[128];
	StatusSetID( IDS_INSTALLNTS );
	err = InitNTService( 2, argv, TRUE, msg );
	if ( err )	UserMsg( msg );
	StatusSetID( IDS_DONE );
	ButtonBarPaint( ghMainDlg );
}

void InstallAutoService( HWND hDlg )
{
	char *argv[] = { "X", "-bootinstall", 0 }; long err; char msg[128];
	StatusSetID( IDS_INSTALLNTSBOOT );
	err = InitNTService( 2, argv, TRUE, msg );
	if ( err )	UserMsg( msg );
	StatusSetID( IDS_DONE );
	ButtonBarPaint( ghMainDlg );
}

void GoStartService( HWND hDlg )
{
	char *argv[] = { "X", "-ntstart", 0 }; long err; char msg[128];
	StatusSetID( IDS_STARTUPSERVICE );
	//gDebugPrint = TRUE;
	err = InitNTService( 2, argv, TRUE, msg );
	if ( err )	UserMsg( msg );
	StatusSetID( IDS_DONE );
	ButtonBarPaint( ghMainDlg );
}



void RemoveService( HWND hDlg )
{
	char *argv[] = { "X", "-u", 0 }; long err; char msg[128];
	StatusSetID( IDS_REMOVENTS );
	err = InitNTService( 2, argv, TRUE, msg );
	if ( err )	UserMsg( msg );
	StatusSetID( IDS_DONE );
	ButtonBarPaint( ghMainDlg );
}


void StopService( HWND hDlg )
{
	char *argv[] = { "X", "-ntend", 0 }; long err; char msg[128];
	StatusSetID( IDS_ENDSERVICE );
	//gDebugPrint = TRUE;
	err = InitNTService( 2, argv, TRUE, msg );
	if ( err )	UserMsg( msg );
	StatusSetID( IDS_DONE );
	ButtonBarPaint( ghMainDlg );
}

long GetMenuItemPos( HWND hWnd, long item, long *subMenuPos )
{
	long i;
	HMENU menu, subMenu;

	*subMenuPos = 0;
	menu = GetMenu( hWnd );
	while( subMenu = GetSubMenu( menu, *subMenuPos ) )
	{
		long menuCount = GetMenuItemCount( subMenu );
		for( i = 0; i < menuCount; i++ )
		{
			if ( (long)GetMenuItemID( subMenu, i ) == item )
				return i;
		}
		*subMenuPos++;
	}
	return -1;
}


// realize palette to screen DC, return number of changed entries{
int RealizeMyPalette( HWND hwnd, HPALETTE hPal )
{
	HDC hdc;
	int i = 0;

	if(hPal) {
		hdc = GetDC(hwnd);
		hPal = SelectPalette(hdc,hPal,FALSE);
		i = RealizePalette(hdc);   
		SelectPalette(hdc,hPal,FALSE);
		ReleaseDC(hwnd,hdc); 
	} 
	return i;
}


	
HPALETTE CreateMyPalette( HWND hWnd, HBITMAP hBMP, BITMAP *bitmap )
{
	LPLOGPALETTE lpPal;
	HPALETTE  	hPal = NULL;
	HDC 		hdc, newhdc;
	int			i, wNumColors=256;
	RGBQUAD		pRGB[256];
	HGDIOBJ 	hOld;

	hdc = GetDC( NULL );
	newhdc = CreateCompatibleDC(hdc);
	hOld = SelectObject( newhdc, hBMP );
	GetDIBColorTable( newhdc, 0, wNumColors, pRGB );

	lpPal = (LPLOGPALETTE)malloc(sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*wNumColors);
	lpPal->palVersion = 0x300;
	lpPal->palNumEntries = wNumColors;
	for( i=0; i < wNumColors; i++)		{
		lpPal->palPalEntry[i].peRed = pRGB[i].rgbRed;
		lpPal->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
		lpPal->palPalEntry[i].peBlue = pRGB[i].rgbBlue;
		lpPal->palPalEntry[i].peFlags = 0;
	}
	
	hPal = CreatePalette(lpPal);
	if ( !hPal ) UserMsg( "cant create palette " );
	free(lpPal);
	SelectObject( newhdc, hOld );
	ReleaseDC( NULL, hdc );
	DeleteDC (newhdc);
	return hPal;
}


#define	STATUSBAR_EVENT	101

#define	MYWAIT_SCHEVENT	102


//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  MESSAGES:
//
//	WM_COMMAND - process the application menu
//	WM_PAINT - Paint the main window
//	WM_DESTROY - post a quit message and return
//    WM_DISPLAYCHANGE - message sent to Plug & Play systems when the display changes
//    WM_RBUTTONDOWN - Right mouse click -- put up context menu here if appropriate
//    WM_NCRBUTTONUP - User has clicked the right button on the application's system menu
//
//

#define	SERIAL_WAITTIME		(200)

// Main Window processing....
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int		wmId, wmEvent;
static	long	timercount=SERIAL_WAITTIME;
static	int		dowindowready=0;
    POINT	pnt;
	HMENU	hMenu;

    
	switch (message) 
	{
	    case WM_TIMER:		// every 100ms
			switch (wParam)
			{
				case STATUSBAR_EVENT:
					if ( gProcessing ){
						UpdateProgressBar( -1 );
					}
					break;

				case MYWAIT_SCHEVENT:
					if ( gNoGUI == FALSE && timercount > 50 ){
						ForceCheckSchedule();		// only check the GUI schedule if the NTSERVICE is NOT running
													// so as to not have 2 schedules running at once...
					}

					if ( timercount >= 1 && ghMainDlg && !dowindowready)
					{
						char ret[32];
						ret[0] = 0;
						GetReg_Prefs( "List Open", ret );
						if ( ret[0] == 'y' && !gStartMinimized )
							CycleWindowSize( ghMainDlg );
						ret[0] = 0;
						GetReg_Prefs( "maximized", ret );
						if ( ret[0] == 'y' )
							ShowWindow( hwndParent, SW_MAXIMIZE );
						dowindowready = 1;
					}
					if ( (timercount%SERIAL_WAITTIME) == 0 )			// 10 = 1 second
						DoRegistrationProcedure();

					if ( (timercount%10) == 0 )
						ButtonBarUpdateInfoLine( hWnd );

					timercount++;
					break;
			}
			break;

		case WM_CREATE:
			{
				if ( hbmBackGround == NULL ){
					hbmBackGround = LoadBitmap( hInst, MAKEINTRESOURCE( IDB_BIGTOOLBAR24 ) );
					hbmBackGroundRoll = NULL;//LoadBitmap( hInst, MAKEINTRESOURCE( IDB_BIGTOOLBAR24B ) );
					hbmBackGroundHit = LoadBitmap( hInst, MAKEINTRESOURCE( IDB_BIGTOOLBAR24B ) );
				}
			}

			GetObject( hbmBackGround,sizeof(BITMAP), &bmBackGround );
			GetObject( hbmBackGroundRoll,sizeof(BITMAP), &bmBackGroundRoll );

			if( iDepth <= 8 )
			{
				hMainPal = (HPALETTE)CreateMyPalette( hWnd, hbmBackGround, &bmBackGround );
			}

			// make font for the other buttons to use
			hSmall8Font = CreateFont(10, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0,  FIXED_PITCH , "Lucida Console" );

			hButtonFont = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH, "Arial" );
			if ( !hButtonFont )
				hButtonFont = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH | FF_SWISS, "MS Sans Serif" );

			hSmallFont = CreateFont(13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH, "Tahoma" );
			if ( !hSmallFont )
				hSmallFont = CreateFont(13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH | FF_SWISS, "MS Sans Serif" );

			
			if ( (hLargeFont = CreateFont(32, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH, "Tahoma" )) == FALSE )
				hLargeFont = CreateFont(32, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH | FF_SWISS, "MS Sans Serif" );

			hSmallBoldFont = CreateFont(13, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH, "Tahoma" );
			if ( !hSmallBoldFont )
				hSmallBoldFont = CreateFont(13, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH | FF_SWISS, "MS Sans Serif" );

			// create the main button 'toolbar'
			ghMainDlg = CreateDialog(hInst, (LPCSTR)MAKEINTRESOURCE(IDD_MAINTOOLBAR), hWnd, (DLGPROC) ButtonBarProc);


/*
			gSpeed = 23;

			if ( !gCloneInstance && !gDebugPrint )
			{
				DialogBox(hInst, "SPLASH", hWnd, (DLGPROC)SplashDlgProc);
			}

			gSpeed = 85;

			{
				long i=0;
				AddMenuItem( hWnd, 1, 1, "Settings->Pre Process", IDM_PREFS_1 );
				AddMenuItem( hWnd, 1, 2, "Settings->Report", IDM_PREFS_2 );	
				AddMenuItem( hWnd, 1, 3, "Settings->Analysis", IDM_PREFS_3 );	
				AddMenuItem( hWnd, 1, 4, "Settings->Filters", IDM_PREFS_4 );	
				AddMenuItem( hWnd, 1, 5, "Settings->Statistics", IDM_PREFS_5);
#ifdef DEF_FULLVERSION
				AddMenuItem( hWnd, 1, 6, "Settings->Advertizing", IDM_PREFS_6 );
				AddMenuItem( hWnd, 1, 7, "Settings->Virtual", IDM_PREFS_7);	
				AddMenuItem( hWnd, 1, 8, "Settings->Post Process", IDM_PREFS_8 );
				AddMenuItem( hWnd, 1, 9, "Settings->Custom", IDM_PREFS_9 );
#else
				AddMenuItem( hWnd, 1, 6, "Settings->Virtual", IDM_PREFS_6);	
				AddMenuItem( hWnd, 1, 7, "Settings->Post Process", IDM_PREFS_7 );
				AddMenuItem( hWnd, 1, 8, "Settings->Custom", IDM_PREFS_8 );	
#endif
			}
			*/
			
			//AddMenuItem( hWnd, 0, 8, "Process log to DB...", IDM_PROCESSANDSAVE_DB );

#ifdef DEF_APP_FIREWALL
			{
				long subMenuPos;
				long menuPos;
				menuPos = GetMenuItemPos( hWnd, IDM_FTPPROCESS, &subMenuPos );
				if ( menuPos == -1 )
					menuPos = GetMenuItemPos( hWnd, IDM_PROCESS, &subMenuPos );
				if ( menuPos == -1 )
				{
					menuPos = 9;
					subMenuPos = 0;
				}
			}
#endif

			AddMenuItem( hWnd, 3, 1, "-", 0 );
			AddMenuItem( hWnd, 3, 2, "Language Builder", IDM_LANGBUILDER );

			IsFullReg();
			int i;

#ifdef	DEF_FULLVERSION
			AddMenuItem( hWnd, 0,10, ReturnString(IDS_MENU_CONVERT2W3C), IDM_CONVERT2W3C );
#endif
			AddMenuItem( hWnd, 2, 0, ReturnString(IDS_BACKGROUND), IDM_BACKGROUNDGRAD );
			CheckedMenuItem( hWnd, 2, 0, gUserGraduatedBackgrounds );
			AddMenuItem( hWnd, 2, 1, "-", 0 );

			if ( gDebugPrint )
			{
				AddMenuItem( hWnd, 3, 0, ReturnString(IDS_CONNECTTOREMOTE), IDM_REMOTECONNECT );
				AddMenuItem( hWnd, 3, 1, ReturnString(IDS_DISCONREMOTE), IDM_REMOTEDISCONNECT );
				i = 0;
				AddMenuItem( hWnd, 3, i++, ReturnString(IDS_WEB), IDM_WEBADMIN );
				AddMenuItem( hWnd, 3, i++, ReturnString(IDS_NEWSKIN), IDM_NEWSKIN );
			}

			DoRegistrationProcedure();

			hPopupMenu = LoadMenu( hInst, "LOG_MENU" );

			SetTimer (hWnd, MYWAIT_SCHEVENT, 100, NULL);
			SetTimer (hWnd, STATUSBAR_EVENT, 100, NULL);


		case WM_INITDIALOG:
			if ( gHttpInterface )
			{
				OutDebug( "InitHttpServer" );
				InitHttpServer( gHttpPort );
			}
			break;

		case WM_PAINT:{
				HDC hdc;
                PAINTSTRUCT ps;
                RECT rc;

                GetClientRect(hWnd, &rc);
				hdc = BeginPaint(hWnd, &ps);
   				if( iDepth <= 8 ){
   					if( hMainPal ){
   						hMainPalOld = SelectPalette(hdc,hMainPal,FALSE);
						RealizePalette(hdc);
					}
				}
                EndPaint(hWnd, &ps);
            }
			break;

		case WM_PALETTECHANGED:
			if( (HWND)wParam != hWnd && hMainPal )
				if( RealizeMyPalette(hWnd, hMainPal) )
					InvalidateRect(hWnd,NULL,TRUE);
			return 0;
			break;
			
		case WM_NOTIFY:
		case WM_QUERYNEWPALETTE:
			if ( hMainPal ){
				if(RealizeMyPalette(hWnd, hMainPal )){
					InvalidateRect(hWnd,NULL,TRUE);
					return TRUE;
				}
			}
			return 0;
			break;

		case WM_CLOSE:
			//Save prefs before closing
			if( AskSaveCurrentPrefs() )		// if user did not hit cancel button
			{
				if ( !gCloneInstance ){
					InitFader( hWnd );
					UpdateWindow( hWnd );
					FadeOut( hWnd );
				}
				DestroyWindow( hWnd );
			}
			break;

		case WM_DESTROY:
			KillTimer ( hWnd, MYWAIT_SCHEVENT );
			KillTimer ( hWnd, STATUSBAR_EVENT );
			DeleteProgressBar();
			DestroyWindow(ghMainDlg);
			DestroyMenu( hPopupMenu );
			DeleteObject (hSmall8Font);
			DeleteObject (hSmallFont);
			DeleteObject (hLargeFont);
			DeleteObject (hSmallBoldFont);
			DeleteObject (hButtonFont);
            //WinHelp (hWnd, "FUNNEL.HLP", HELP_QUIT,(DWORD)0);
			PostQuitMessage(0);
			return 0L;
			break;

		case WM_SIZE: {
				RECT        rc, statBar;
				LONG        mainHeight, statheight, newW, newH;
	
				GetWindowRect(ghMainDlg, &rc);
				GetRectofStatusBarItem( 1, &statBar );
				statheight = statBar.bottom - statBar.top;
				mainHeight = rc.bottom-rc.top;
				newW = LOWORD(lParam);
				newH = HIWORD(lParam);


				if ( dowindowready ){
					if ( wParam == SIZE_MAXIMIZED ){
						SetReg_Prefs( "maximized", "yes" );
					} else
					if ( wParam == SIZE_RESTORED )
						SetReg_Prefs( "maximized", "no" );
				}


				// CR!! Alternatively, this window can be created with cy
				//      equals to cy of the screen and saving this call
				//      altogether.
				if ( ghMainDlg )
					MoveWindow(ghMainDlg,  0, 0, newW, mainHeight, TRUE);

				if ( ghStatusWnd ){
					long statleft;
					INT widths[4];

					statleft = (newW-16)/2;

					widths[0] = statleft;
					widths[1] = -1;
					widths[2] = 0;
					SendMessage( ghStatusWnd, SB_SETPARTS, (WPARAM)2, (LPARAM) &widths[0] ); 

					MoveWindow(ghStatusWnd,  0, newH - statheight, newW, statheight, TRUE);
					if ( ghProgressWnd ){
						GetRectofStatusBarItem( 1, &statBar );
						MoveWindow(ghProgressWnd,  statleft, newH - statheight, (newW-16) - statleft, statheight, TRUE);
						//MoveWindow(ghProgressWnd,  statBar.left, newH - statheight, (newW-16) - statBar.left, statheight, TRUE);
					}
				}
				if ( hWndLogListView )
				{
					if ( gwSize ){
						MoveWindow( hWndLogListView,  15, LISTVIEW_TOP, newW-30, newH-LISTVIEW_TOP-statheight-15, TRUE);
						ListView_SetColumnWidth( hWndLogListView, 0, newW-295 );
					} else {
						RECT        rc2;
						GetObjectRect( ghMainDlg, hWndLogListView, &rc2 );
						MoveWindow( hWndLogListView,  15, LISTVIEW_TOP+30, newW-30, rc2.bottom-rc2.top, TRUE);
					}
				}
				return DefWindowProc(hWnd, message, wParam, lParam);
				break;
			}
		case WM_SIZING:
			{
				RECT *lprc = (LPRECT) lParam;
				long w,h, width;

				if ( !gwSize ) // If the log list is not being displayed, then don't allow resizing of the main window.
				{
					if ( gLastSizeHeight )
						lprc->bottom = lprc->top + gLastSizeHeight;
					else
						gLastSizeHeight = lprc->bottom - lprc->top;
					if ( gLastSizeWidth )
						lprc->right = lprc->left + gLastSizeWidth;
					else
						gLastSizeWidth = lprc->right - lprc->left;
					break;
				}

				w = lprc->right - lprc->left;
				h = lprc->bottom - lprc->top;
				width = TOOLBAR_WIDTH;
				// only allow resizing if you can see the Log List view
				if ( gwSize ){
					if (  w<width ) lprc->right+= width-w;
					if (  h<TOOLBAR_MAINHEIGHT+100 ) lprc->bottom+= TOOLBAR_MAINHEIGHT-h+100;
					//SetWindowSize( ghMainDlg, -1 );

				} else
				{ // dont allow resizing, readjust the coords so its not resized
					lprc->right+= width-w;
					if ( gLastSizeHeight )
						lprc->bottom+= gLastSizeHeight-h;
					else
						gLastSizeHeight = h;
				}
			}
			break;


		case WM_MENUSELECT:
			{
				
				wmId    = LOWORD(wParam); // Remember, these are...
				wmEvent = HIWORD(wParam); // ...different for Win32!
				
				if ( wmEvent & MF_MOUSESELECT ){
					//id = GetMenuItemID( (HMENU)lParam, wmId );
					switch( wmId ){
						case IDM_HELP:			StatusSetID( IDS_MENU_HELP); break;
						case IDM_ABOUT:			StatusSetID( IDS_MENU_ABOUT  ); break;
						case IDM_EXIT:			StatusSetID( IDS_MENU_EXIT); break;
						case IDM_GROW:			StatusSetID( IDS_MENU_GROW  ); break;
						case IDM_SHRINK:		StatusSetID( IDS_MENU_SHRINK ); break;
						case IDM_CLEARLIST:		StatusSetID( IDS_MENU_CLEARLIST ); break;
						case IDM_ONLINEMANUAL:	StatusSetID( IDS_MENU_ONLINEMANUAL ); break;
						case IDM_FTPPROCESS:	StatusSetID( IDS_MENU_FTPPROCESS  ); break;
						case IDM_NEWAPP:		StatusSetID( IDS_MENU_NEWAPP ); break;
						case IDM_BACKGROUNDGRAD:	StatusSetID( IDS_MENU_BACKGROUNDGRAD); break;
						case IDM_INSTALLNTS:	StatusSetID( IDS_MENU_INSTALLNTS ); break;
						case IDM_REMOVENTS:		StatusSetID( IDS_MENU_REMOVENTS ); break;
						case IDM_STARTUPSERVICE:StatusSetID( IDS_MENU_STARTUPSERVICE ); break;
						case IDM_ENDSERVICE:	StatusSetID( IDS_MENU_ENDSERVICE ); break;
						case IDM_SHOWNTSERVICES:  StatusSetID( IDS_MENU_SHOWNTSERVICES  ); break;
						case IDM_REGISTERSERIAL:  StatusSetID( IDS_MENU_REGISTERSERIAL  ); break;
						case IDM_CHECKFORUPDATE:  StatusSetID( IDS_MENU_CHECKFORUPDATE ); break;
						case IDM_REGISTERNOW:	StatusSetID( IDS_MENU_REGISTERNOW  ); break;
						case IDM_ORDERWWW:		StatusSetID( IDS_MENU_ORDERWWW ); break;
						case IDM_APPWWW:		StatusSet( "Analysis Software" ); break;
						case IDM_ACWWW:			StatusSetID( IDS_MENU_ACWWW ); break;
						case IDM_SUPPORTWWW:	StatusSetID( IDS_MENU_SUPPORTWWW ); break;
						case IDM_VIEWHTML:		StatusSetID( IDS_MENU_VIEWHTML ); break;
						case IDM_PROCESS:		StatusSetID( IDS_MENU_PROCESS ); break;
						case IDM_PREFS:			StatusSetID( IDS_MENU_PREFS  ); break;
						case IDM_SCHEDULE:		StatusSetID( IDS_MENU_SCHEDULE ); break;
						case IDM_NEW:			StatusSetID( IDS_MENU_NEW);break;
						case IDM_NEW_STREAMING:	StatusSet( "New Streaming Settings" );break;
						case IDM_NEW_FW:		StatusSet( "New Fireall Settings" ); break;
						case IDM_OPEN:			StatusSetID( IDS_MENU_OPEN ); break;
						case IDM_SAVE:			StatusSetID( IDS_MENU_SAVE); break;
						case IDM_SAVEAS:		StatusSetID( IDS_MENU_SAVEAS); break;
						case IDM_STOP:			StatusSetID( IDS_MENU_STOP ); break;
						case IDM_MONITORFILE:	StatusSetID( IDS_MENU_MONITORFILE ); break;
						case IDM_UPDATEONOFF:	StatusSetID( IDS_MENU_UPDATEONOFF ); break;
						case IDM_CONVERT2W3C:	StatusSetID( IDS_MENU_CONVERT2W3C_STATUS ); break;

						case IDM_VIEWTRANSCRIPT:	StatusSetID(IDS_MENU_VIEWSCHEDULETRANSCRIPT); break;
						case IDM_LOGS_REMOVELOGS:	StatusSetID(IDS_MENU_REMOVESELECTEDLOGS); break;
						case IDM_LOGS_EDITLOGS:		StatusSetID(IDS_MENU_EDITLOGS); break;
						case IDM_LOGS_COMPLOG:		StatusSetID(IDS_MENU_COMPRESSLOGTOGZIP); break;
						case IDM_LOGS_COMPLOG2:		StatusSetID(IDS_MENU_COMPRESSLOGTOBZIP2); break;
						case IDM_LOGS_SHOW_DURATION:StatusSetID(IDS_MENU_SHOWLOGDURATION); break;
						case IDM_LOGS_ADDLOGS:		StatusSetID(IDS_MENU_ADDLOGSTOLIST); break;
						case IDM_LOGS_SAVELOGS:		StatusSetID(IDS_MENU_SAVELIST); break;
						case IDM_LOGS_LOADLOGS:		StatusSetID(IDS_MENU_LOADLIST); break;
						
						case IDM_WEBADMIN:			StatusSetID(IDS_MENU_WEBADMINCONFIG); break;
						case IDM_NEWSKIN:			StatusSetID(IDS_MENU_SELECTNEWSKIN); break;
						case IDM_LANGBUILDER:		StatusSetID(IDS_MENU_LANGUAGEBUILDER); break;
						case IDM_INSTALLNTSBOOT:	StatusSetID(IDS_MENU_INSTALLAUTOSTART); break;

						default: StatusSet( "" );
							break;
					}
				}
			}			
			break;


		case WM_COMMAND:
			wmId    = LOWORD(wParam); // Remember, these are...
			wmEvent = HIWORD(wParam); // ...different for Win32!

			if ( wmId >= 2300 && wmId < 2310 ){
				if( AskSaveCurrentPrefs() )		// QCM:46336
					LoadFromMRU( wmId - 2300 );
			}

			//Parse the menu selections:
			switch ( wmId&0xffff ) 
			{
				//case IDM_PROCESSLOGWIZARD:
				//	DoProcessLogQ( hWnd, TRUE );
				//	break;

				case IDM_PROCESSLOGNOW:
					DoProcessLogQ( hWnd, FALSE );
					break;

				
				
				case IDM_HELP_SUPPORTZONESETUP:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_SUPPORTPASS), hWnd, (DLGPROC)SupportZoneDlgProc);
					break;

				case IDM_HELP_CHANGESUPPORTPASSWORD:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGEPASS), hWnd, (DLGPROC)ChangePasswordDlgProc);
					break;

				case IDM_LIVERADIO1:
					ShowHTMLShortcut( hWnd, "mplayer2", "http://www.cablemusic.com/radio.asp?program=Trance&bitrate=20" );
					break;
				case IDM_LIVERADIO2:
					ShowHTMLShortcut( hWnd, "mplayer2", "http://www.abc.net.au/streaming/triplej.asx" );
					break;
				case IDM_LIVERADIO3:
					ShowHTMLShortcut( hWnd, "mplayer2", "http://www.channelv.com.au/soundz/channelv.asx" );
					break;
				case IDM_LIVERADIO4:
					ShowHTMLShortcut( hWnd, "mplayer2", "mms://streamer.themaestro.net/pulsefm" );
					break;
				case IDM_LIVERADIO5:
					ShowHTMLShortcut( hWnd, "mplayer2", "http://www.abc.net.au/streaming/raflp.asx" );
					break;

				
				case IDM_STOCK:
					ShowHTMLShortcut( hWnd, "http://quote.bloomberg.com/analytics/bquote.cgi?version=markets99.cfg&view=extmult&ticker=QSFT", NULL );
					break;
				case IDM_WEBCHAT:
					ConnectToChatServer( hWnd );
					break;

				case IDM_ABOUT:
					gSpeed = 25;
					DialogBox(hInst, "ABOUT", hWnd, (DLGPROC)AboutDlgProc);
					gSpeed = 65;
					break;

				case IDM_EXIT:
					gSpeed = 25;
					SendMessage( hwndParent, WM_CLOSE, wParam, lParam );
					
					break;

				case IDM_GROW:
					GrowWindowSize( ghMainDlg );
					break;

				case IDM_SHRINK:
					ShrinkWindowSize( ghMainDlg );
					break;
                
				case IDM_CLEARLIST:
					ClearLogHistory();
					break;

				case IDM_HELPTOPICS: // Only called in Windows 95
 					PerformMainHelp( hWnd, HELP_FINDER );
					break;

				case IDM_HELPCONTENTS: // Not called in Windows 95
 					PerformMainHelp( hWnd, HELP_CONTENTS );
					break;

				case IDM_HELPSEARCH: // Not called in Windows 95
 					PerformMainHelp( hWnd, HELP_PARTIALKEY );
					break;

				case IDM_HELP:
					if ( ExplorerHelp() )
						break;
				case IDM_ONLINEMANUAL:
 					ShowHTMLShortcut( hWnd, URL_MANUAL, NULL );
					break;

				case IDM_FTPPROCESS:
					if ( !gProcessing && !gProcessingSchedule ) 
						DialogBox(hInst, "FTP_OPEN", hWnd, (DLGPROC)FtpOpenDlgProc);
					break;

				case IDM_NEWAPP:
 					ShowHTMLShortcut( hWnd, gApplicationExeName, "-clone" );
					break;

				case IDM_LANGBUILDER:
					ShowEditLangGuiDialog( hWnd );
					break;

				case IDM_BACKGROUNDGRAD:
					if ( gUserGraduatedBackgrounds )
						gUserGraduatedBackgrounds = 0;
					else
						gUserGraduatedBackgrounds = 1;
					CheckedMenuItem( hWnd, 2, 0, gUserGraduatedBackgrounds );
					break;

				case IDM_UPDATEONOFF:
					gCheckUpdate ^= 1;
					CheckedMenuItem( hWnd, 4, 3, gCheckUpdate );
					if ( gCheckUpdate )
						SetReg_Prefs( "updatecheckflag", "yes" );
					else
						SetReg_Prefs( "updatecheckflag", "off" );
					break;

				case IDM_NEWSKIN:
					if ( !gProcessing && !gProcessingSchedule ) {
						char	filename[512];
						StatusSetID( IDS_NEWSKIN);
						if ( GetOpenSkinsName( filename ) ){
							StatusSetID( IDS_LOADSKIN );
							SetReg_Prefs( "skin", filename );
							StatusSetID( IDS_DONE );
		 					ShowHTMLShortcut( hWnd, gApplicationExeName, "-clone" );
							DestroyWindow( hwndParent );
						} else
							StatusSetID( IDS_CANCELLED );
					}
					break;

				
				case IDM_HELPHELP: // Not called in Windows 95
					if(!WinHelp(hWnd, (LPSTR)NULL, HELP_HELPONHELP, 0)) {
						UserMsg( "Unable to activate help" );
					}
					break;

				case IDM_INSTALLNTS:
					InstallManualService( hWnd );
					GoStartService( hWnd );
					break;

				case IDM_INSTALLNTSBOOT:
					InstallAutoService( hWnd );
					GoStartService( hWnd );
					break;


				case IDM_REMOVENTS:
					RemoveService( hWnd );
					StopService( hWnd );
					break;


				
				case IDM_SHOWNTSERVICES:
						if ( IS_NT5 ){
							char szText[256], lpszSystemInfo[255];
							GetWindowsDirectory(lpszSystemInfo, MAX_PATH); 
							sprintf( szText, "%s\\system32\\com\\comexp.msc", lpszSystemInfo );
 							ShowHTMLShortcut( hWnd, "mmc", szText );
						} else
 							ShowHTMLShortcut( hWnd, "control", "srvmgr.cpl services" );
						break;

#ifndef DEF_FREEVERSION
				case IDM_REGISTERSERIAL:
						if ( DoRegistrationAgain() )
						{
							ChangeMainWindowTitle();
							if ( !IsDemoReg() ){
		 						ShowHTMLShortcut( hWnd, gApplicationExeName, "-clone" );
								DestroyWindow( hwndParent );
							}
						}
						break;

				case IDM_REGISTERNOW:
 						ShowHTMLShortcut( hWnd, URL_REGISTER, NULL );
						break;
#endif

				case IDM_CHECKFORUPDATE:
						CheckforUpdate();
						break;

				case IDM_APPWWW:
 						ShowHTMLShortcut( hWnd, URL_APP, NULL );
						break;

				case IDM_SUPPORTWWW:
 						ShowHTMLShortcut( hWnd, URL_SUPPORT, NULL );
						break;

 				case IDM_VIEWHTML:
						SpawnShowShellFile( MyPrefStruct.outfile );
						break;

					
#ifdef	DEF_FULLVERSION
				case IDM_CONVERT2W3C:
						gConvertLogToW3C_Flag = TRUE;
#endif
				case IDM_PROCESS:
						SendMessage( ghMainDlg, WM_COMMAND, IDC_PROCESS , lParam);
						break;

				case IDM_PREFS:
						StatusSetID( IDS_EDITSET);
						ShowPrefsDialog(0);
						UpdateWindow( hwndParent );
						if ( !gProcessing && !gProcessingSchedule )
							StatusSetID( IDS_OK );
						break;

				case IDM_PREFS_1:
						ShowPrefsDialog(1);
						break;
				case IDM_PREFS_2:
						ShowPrefsDialog(2);
						break;
				case IDM_PREFS_3:
						ShowPrefsDialog(3);
						break;
				case IDM_PREFS_4:
						ShowPrefsDialog(4);
						break;
				case IDM_PREFS_5:
						ShowPrefsDialog(5);
						break;
				case IDM_PREFS_6:
						ShowPrefsDialog(6);
						break;
				case IDM_PREFS_7:
						ShowPrefsDialog(7);
						break;
				case IDM_PREFS_8:
						ShowPrefsDialog(8);
						break;
   
				case IDM_SCHEDULE:
						if ( !gProcessingSchedule )
						{
							StatusSetID(IDS_EDITSCHEDULE);
							ShowScheduleDialog();
							if ( !gProcessing )
								StatusSetID( IDS_OK );
							RefreshStatusBar();
						}
						break;
				case IDM_NEW:
						if ( !gProcessing && !gProcessingSchedule ) 
						{
							if( AskSaveCurrentPrefs() )
							{
								CreateDefaults(&MyPrefStruct,1,WEB);	
								strcpy( gPrefsFilename, s_DefaultSettingsFileName );
								SetReg_PrefsFile( gPrefsFilename );
								gSaved=TRUE;
								s_PrefsFileIsNew=true;
							}
							ChangeMainWindowTitle();
							UpdateWindow( hwndParent );
							ButtonBarPaint( ghMainDlg );
						}
						break;


				case IDM_NEW_STREAMING:
						if ( !gProcessing && !gProcessingSchedule ) 
						{
							if( AskSaveCurrentPrefs() )
							{
								CreateDefaults(&MyPrefStruct,1,STREAM);	
								strcpy( gPrefsFilename, s_DefaultSettingsFileName );
								SetReg_PrefsFile( gPrefsFilename );
								gSaved=TRUE;
								s_PrefsFileIsNew=true;
							}
							ChangeMainWindowTitle();
							UpdateWindow( hwndParent );
							ButtonBarPaint( ghMainDlg );
						}
						break;

				case IDM_NEW_FW:
						if ( !gProcessing && !gProcessingSchedule ) 
						{
							if( AskSaveCurrentPrefs() )
							{
								CreateDefaults(&MyPrefStruct,1,FIRE);	
								strcpy( gPrefsFilename, s_DefaultSettingsFileName );
								SetReg_PrefsFile( gPrefsFilename );
								gSaved=TRUE;
								s_PrefsFileIsNew=true;
							}
							ChangeMainWindowTitle();
							UpdateWindow( hwndParent );
							ButtonBarPaint( ghMainDlg );
						}
						break;


				case IDM_OPEN:
					{
						if( !AskSaveCurrentPrefs() )	// if user was prompted to save and hit cancel
						{
							return 1;
						}

						if ( GetOpenPrefsName( gPrefsFilename ) )
						{
							ReadCurrentPrefs();						
						}
						UpdateWindow( hwndParent );
					}
					break;

				case IDM_SAVE:
					SaveCurrentPrefs( s_PrefsFileIsNew );
					break;

				case IDM_SAVEAS:
					SaveCurrentPrefs( true );
					break;

				// Log List POP UP MENU
				case IDM_STOPALL:
				case IDM_STOP:
					StopProcessing();
					break;

				case IDM_LOGS_EDITLOGS:
					EditLogFile( hwndParent, GetLogListViewItem(), NULL );
					break;
				case IDM_LOGS_VIEWLOGS:
				case IDM_MONITORFILE:
					StartFileView( GetLogListViewItem() );
					break;
				case IDM_VIEWTRANSCRIPT:
					ViewScheduleLog( hWnd );
					break;
				case IDM_LOGS_REMOVELOGS:
					DeleteSelectedLogHistory( hWnd );
					RepaintListView();
					break;
				case IDM_LOGS_COMPLOG:
					CompressSelectedLogHistory( hWnd );
					RepaintListView();
					break;
				case IDM_LOGS_COMPLOG2:
					CompressBzip2SelectedLogHistory( hWnd );
					RepaintListView();
					break;

				case IDM_LOGS_SHOW_DURATION:
					ShowLogDuration();
					break;

				case IDM_LOGS_ADDLOGS:
					Dialog_SelectLogs();
					RepaintListView();
					break;

				case IDM_LOGS_CLEARLOGS:
					ClearLogHistory();
					RepaintListView();
					break;

				case IDM_LOGS_SAVELOGS:
					{
						char filename[256] = "";

						if ( GetSaveLogListName( filename ) ){
							SaveLogListAsText( filename );
						}
					}
					break;

				case IDM_LOGS_REVERT:
					ReadLogListAsText( NULL );
					RepaintListView();
					break;

				case IDM_LOGS_LOADLOGS:
					{
						char filename[256];

						if ( GetOpenLogListName( filename ) ){
							ReadLogListAsText( filename );
						}
					}
					RepaintListView();
					break;

				case IDM_LOGS_PROCESSLOGS:
					SendMessage( ghMainDlg, WM_COMMAND, IDC_PROCESS , lParam);
					break;


				case IDM_UNDO:
				case IDM_CUT:
				case IDM_COPY:
				case IDM_PASTE:
				case IDM_LINK:
				case IDM_LINKS:
				default:
					return (DefWindowProc(hWnd, message, wParam, lParam));
			}
			break;

		case WM_NCRBUTTONUP: // RightClick on windows non-client area...
			if (IS_WIN95 && SendMessage(hWnd, WM_NCHITTEST, 0, lParam) == HTSYSMENU)
			{
				// The user has clicked the right button on the applications
				// 'System Menu'. Here is where you would alter the default
				// system menu to reflect your application. Notice how the
				// explorer deals with this. For this app, we aren't doing
				// anything
				return (DefWindowProc(hWnd, message, wParam, lParam));
			} else {
				// Nothing we are interested in, allow default handling...
				return (DefWindowProc(hWnd, message, wParam, lParam));
			}
            break;

        case WM_RBUTTONDOWN: // RightClick in windows client area...
            pnt.x = LOWORD(lParam);
            pnt.y = HIWORD(lParam);
            ClientToScreen(hWnd, (LPPOINT) &pnt);
			// This is where you would determine the appropriate 'context'
			// menu to bring up. Since this app has no real functionality,
			// we will just bring up the 'Help' menu:
            hMenu = GetSubMenu (GetMenu (hWnd), 2);
            if (hMenu) {
                TrackPopupMenu (hMenu, 0, pnt.x, pnt.y, 0, hWnd, NULL);
            } else {
				// Couldn't find the menu...
                MessageBeep(0);
            }
            break;

		case WM_SYSKEYDOWN:
			if ( wParam == VK_MENU )
				about_Special = TRUE;
			break;
		case WM_SYSKEYUP:
			if ( wParam == VK_MENU )
				about_Special = FALSE;
			break;


		// Only comes through on plug'n'play systems
		case WM_DISPLAYCHANGE: {
				SIZE szScreen;
				BOOL fChanged = (BOOL)wParam;
	
				szScreen.cx = LOWORD(lParam);
				szScreen.cy = HIWORD(lParam);
				
				if (fChanged) {
					 gHDCGlobal  = GetDC (NULL);
				     iRasterCaps = GetDeviceCaps(gHDCGlobal, RASTERCAPS);
				     iRasterCaps = (iRasterCaps & RC_PALETTE) ? TRUE : FALSE;
				     if (iRasterCaps)
				         iNumColors = GetDeviceCaps( gHDCGlobal, SIZEPALETTE);
				     else
				         iNumColors = GetDeviceCaps( gHDCGlobal, NUMCOLORS);
				
					 iDepth = GetDeviceCaps( gHDCGlobal, BITSPIXEL);
					 //{ char txt[128];  wsprintf( txt, "colors=%d, depth=%d", iNumColors, iDepth); ErrorMsg( txt ); }
				     ReleaseDC (NULL,gHDCGlobal);
     					// The display 'has' changed. szScreen reflects the
				} else {
					// The display 'is' changing. szScreen reflects the
					// original size.
					MessageBeep(0);
				}
			}
			break;


			default:
			return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return FALSE;
}

void RemapPalette( HDC hdc, HWND hWnd )
{
	HGDIOBJ 	hbmOld;
    HDC			hdcSrc;

	if ( hbmBackGround ){
        hdcSrc = CreateCompatibleDC (hdc);
        hbmOld = SelectObject (hdcSrc, hbmBackGround );
   		if( iDepth <= 8 ){
			hMainPal = (HPALETTE)CreateMyPalette( hWnd, hbmBackGround, &bmBackGround );
   			if( hMainPal ){
   				hMainPalOld = SelectPalette(hdc,hMainPal,FALSE);
				RealizePalette(hdc);
			}
		}
		SelectObject (hdcSrc, hbmOld);
        DeleteDC (hdcSrc);
	}
}



// button coord
void InitButtonImages( HWND hWnd  )
{

	// if we are in evil BIG FONT mode, move all the buttons to sane sizes
	if( toolbar_newskin ) {
		long	IDs[] = { IDC_PREFS,IDC_SCHED,IDC_PROCESS,IDC_VIEW,IDC_APPHELP,IDC_SHOWLIST, 0 };
		long	i=0;
		while( buttons_coord[i][0] ){
			MoveWindow( GetDlgItem( hWnd, IDs[i] ),	buttons_coord[i][0], buttons_coord[i][1], buttons_coord[i][2], buttons_coord[i][3], TRUE );
			i++;
		}
	} else
	if ( GetPixelXScale(100) != 100 ){
		long x[]= {17,75,125,182,238}, y=11;
		MoveWindow( GetDlgItem( hWnd, IDC_PREFS ),	x[0], y, 46, 40, TRUE );
		MoveWindow( GetDlgItem( hWnd, IDC_SCHED ),  x[1], y, 46, 40, TRUE );
		MoveWindow( GetDlgItem( hWnd, IDC_PROCESS ),x[2], y, 46, 40, TRUE );
		MoveWindow( GetDlgItem( hWnd, IDC_VIEW ),   x[3], y, 46, 40, TRUE );
		MoveWindow( GetDlgItem( hWnd, IDC_APPHELP ),  x[4], y,46, 40, TRUE );
		MoveWindow( GetDlgItem( hWnd, IDC_SHOWLIST ),  0, 74, 16, 14, TRUE );
	}
}

void RefreshProcButton( HWND hWnd )
{
	HWND hDlg = hWnd;

	if ( !gNoGUI ){
		SendDlgItemMessage( hWnd, IDC_PROCESS, BN_PAINT, 0, 0 );
	}
}


// plot
void DrawButtonText( HDC hdc, HFONT font, char *txt, long x, long y )
{
    RECT textrc;
	HGDIOBJ of;

	if ( hdc && txt ){
		SetTextColor( hdc, 0x000000 );
		SetBkMode( hdc, TRANSPARENT );
		//SetBkColor( hdc, 0x333333 );

		of = SelectObject ( hdc, font );

		textrc.left = x; textrc.top = y;
		textrc.right = x+46; textrc.bottom = y+14;

		DrawText( hdc, txt, strlen(txt), &textrc, DT_CENTER );

		if( of ) SelectObject (hdc, of);
	}
}


static char *ButtonNames[] =
{ "  Settings ", " Schedule ", " Process ", "  Stop    ", " View ", " Help ", 0 };


long DrawButton( HWND hWnd, HDC whdc, long id, long state )       
{
	RECT rc; HDC hdcSrc;
	HBITMAP	hbmBitmap=NULL;
	HGDIOBJ hbmOld;
	HDC hdc;
	long idx=0, dimx=46, dimy=40;
	char *txt=NULL;

	hdc = GetDC( hWnd );

	/*				{
					char t[256];
					sprintf( t, "coord =%d,%d", rc.left, rc.right-rc.left );
					OutDebug( t );
				}
*/
	switch( id )
	{
		case IDC_PREFS:		txt=ButtonNames[0];break;
		case IDC_SCHED:		txt=ButtonNames[1];break;
		case IDC_PROCESS:	if( gProcessing || gProcessingSchedule  )
								txt=ButtonNames[3];
							else
								txt=ButtonNames[2];break;
		case IDC_VIEW:		txt=ButtonNames[4];break;
		case IDC_APPHELP:	txt=ButtonNames[5];break;
		case IDC_APPLOGO:	txt=NULL;break;
		case IDC_SHOWLIST:
			ReleaseDC( hWnd, hdc );
			return 0; 
			if ( gwSize )
				hbmBitmap = hbmBackGroundRoll;
			else
				hbmBitmap = hbmBackGround;
			txt = 0;
			if( hdcSrc = CreateCompatibleDC (hdc) )
			{
				if( iDepth <= 8 && hMainPal )
				{
					hMainPalOld = SelectPalette(hdc,hMainPal,FALSE);
					RealizePalette(hdc);
				}
				hbmOld = SelectObject (hdcSrc, hbmBitmap );
				GetObjectCoord( hWnd, GetDlgItem( hWnd, id ), &rc );
				StretchBlt( hdc,	rc.left,rc.top, rc.right-rc.left,rc.bottom-rc.top,
							hdcSrc, rc.left,rc.top, rc.right-rc.left,rc.bottom-rc.top, SRCCOPY );
				DeleteDC( hdcSrc );
			}
			ReleaseDC( hWnd, hdc );
			return 0; 
			break;
		default	:
			ReleaseDC( hWnd, hdc );
			return 0; 
			break;
	}

	hdcSrc = CreateCompatibleDC (hdc);
	if( iDepth <= 8 && hMainPal ){
		hMainPalOld = SelectPalette(hdc,hMainPal,FALSE);
		RealizePalette(hdc);
	}

	GetObjectCoord( hWnd, GetDlgItem( hWnd, id ), &rc );

	if ( state==0 ){
		hbmOld = SelectObject (hdcSrc, hbmBackGround );
		StretchBlt( hdc,	rc.left,rc.top, rc.right-rc.left+1,rc.bottom-rc.top,
					hdcSrc, rc.left,rc.top, rc.right-rc.left+1,rc.bottom-rc.top, SRCCOPY );
		DeleteDC( hdcSrc );
	} else
/*	if ( state==1 ){
		hbmOld = SelectObject (hdcSrc, hbmBackGroundRoll );
		StretchBlt( hdc,	rc.left,rc.top, rc.right-rc.left+1,rc.bottom-rc.top,
					hdcSrc, rc.left,rc.top, rc.right-rc.left+1,rc.bottom-rc.top, SRCCOPY );
		DeleteDC( hdcSrc );
	} else */
	if ( state==2 ){
		hbmOld = SelectObject (hdcSrc, hbmBackGroundHit );
		StretchBlt( hdc,	rc.left,rc.top, rc.right-rc.left+1,rc.bottom-rc.top,
					hdcSrc, rc.left,rc.top, rc.right-rc.left+1,rc.bottom-rc.top, SRCCOPY );
		DeleteDC( hdcSrc );
	}

	// Update the PROCESS name on the button to STOP during processing.
	if ( txt ){
/*		hbmOld = SelectObject (hdcSrc, hbmBackGround );
		StretchBlt( hdc,	rc.left,rc.bottom, rc.right-rc.left+1,14,
					hdcSrc, rc.left,rc.bottom, rc.right-rc.left+1,14, SRCCOPY );
		DeleteDC( hdcSrc );
*/
		DrawButtonText( hdc, hButtonFont, txt, rc.left+8, rc.bottom-20 );
	}

	ReleaseDC( hWnd, hdc );

	return 1;
}




/*
		            hdcSrc = CreateCompatibleDC (hdc);
		            if( iDepth <= 8 ){
						if( hPal ){
   							hPalOld = SelectPalette(hdc,hPal,FALSE);
							RealizePalette(hdc);
						}
					}
*/



void ButtonBarUpdateInfoLine( HWND hWnd )
{
	char txt[256];
	long y;
	y = toolbar_infopos;

	if ( IsServiceRunning() )
	{
		//GetString( IDS_TOOLBARSERVICE, toolbar_service_str, 64 );
		WriteXY( hWnd, hSmallFont, "NTservice: Running", toolbar_service_xpos, y, toolbar_statuscolor );
	} else
	if ( IsServiceInstalled( 1 ) )
	{
		GetString( IDS_TOOLBARSERVICE, toolbar_service_str, 64 );
		WriteXY( hWnd, hSmallFont, toolbar_service_str, toolbar_service_xpos, y, toolbar_statuscolor );
	}


#ifndef DEF_FREEVERSION
	if ( !IsDemoReg() && !IsFullReg() )
	{
		sprintf( txt, "Product is unregistered, please register immediately" );
		WriteXY( hWnd, hSmallFont, txt, toolbar_statusxpos, y, 0x0000FF );
	}
	else
	if ( IsDemoReg() || (IsFullReg() == FALSE) )
	{
		double dl = GetDaysLeft();

		if ( dl < 0 ){
			dl = 0;
		}
		if ( dl == 0 )
		{
			sprintf( txt, "TRIAL VERSION EXPIRED!!!     Processing disabled" );
			WriteXY( hWnd, hSmallFont, txt, toolbar_statusxpos, y, 0x0000FF );
		} else 
		{
			sprintf( txt, "TRIAL VERSION (%.1f days left)", dl );
			WriteXY( hWnd, hSmallFont, txt, toolbar_statusxpos, y, toolbar_infocolor );
		}
	}
#endif


	{
		if ( MyPrefStruct.stat_style == WEB )
			sprintf( txt, "(WEB MODE)" );
		else
		if ( MyPrefStruct.stat_style == STREAM )
			sprintf( txt, "(STREAMING MODE)" );
		else
		if ( MyPrefStruct.stat_style == FIRE )
			sprintf( txt, "(FIREWALL MODE)" );

		WriteXY( hWnd, hSmallFont, txt, toolbar_statusxpos, y, toolbar_infocolor );
	}
}




void ButtonBarPaint(HWND hWnd )
{
	HGDIOBJ hbmOld;
    PAINTSTRUCT ps;
    HDC hdc, hdcSrc;
	long xS=1, yS=1;

    hdc = BeginPaint(hWnd, &ps);

	if ( hbmBackGround ){
		RECT rc, rc2;
		hdc = GetDC (hWnd);

		if( hdcSrc = CreateCompatibleDC (hdc) ){
			HBRUSH gbgBrush;
			gbgBrush = CreateSolidBrush( 0xb2b2b2 );		//BACKGROUND_COLOR
		    if( iDepth <= 8 && hMainPal ){
				hMainPalOld = SelectPalette(hdc,hMainPal,FALSE);
				RealizePalette(hdc);
			}
			GetClientRect( hWnd, &rc );
			GetObjectRect( ghMainDlg, hWndLogListView, &rc2 );
			rc.bottom = LISTVIEW_TOP-2;
			FillRect( hdc, &rc, gbgBrush );
			hbmOld = SelectObject (hdcSrc, hbmBackGround );

			// blit background toolbar stuff
			StretchBlt( hdc,0,0, xS*bmBackGround.bmWidth,yS*bmBackGround.bmHeight,
						hdcSrc, 0, 0, bmBackGround.bmWidth, bmBackGround.bmHeight,SRCCOPY );

			// blit the far right hand side filler
			StretchBlt( hdc,	bmBackGround.bmWidth-1,0,	GetSystemMetrics(SM_CXFULLSCREEN)-bmBackGround.bmWidth,yS*bmBackGround.bmHeight,
						hdcSrc, bmBackGround.bmWidth-4,0,	3, bmBackGround.bmHeight,SRCCOPY );

			// TILE THE METAL
			{
				int desty = bmBackGround.bmHeight, height=48;
				int srcy = bmBackGround.bmHeight-height;
				int bottom = rc2.bottom+height;

				while( desty < bottom ){
					if( desty+48 > bottom ){
						height = bottom - desty;
						//OutDebugs( "height is %d", height );
					}

					// left side metal
					BitBlt( hdc,	0,desty,	15, height,
									hdcSrc, 0,srcy,
									SRCCOPY );
					// right side metal
					BitBlt( hdc,	rc.right-15,desty,	15, height,
							hdcSrc, bmBackGround.bmWidth-15,srcy,
							SRCCOPY );
					desty += 48;
				}

				desty = bottom-23;

				StretchBlt( hdc,	bmBackGround.bmWidth-1,desty,  GetSystemMetrics(SM_CXFULLSCREEN)-bmBackGround.bmWidth,15,
							hdcSrc, bmBackGround.bmWidth-4,0,	3, 15,SRCCOPY );

				BitBlt( hdc,	0, desty,	rc.right-rc.left, 8,	hdcSrc, 0, 0,	SRCCOPY );
				desty+=8;
				BitBlt( hdc,	0, desty,	rc.right-rc.left, 8,	hdcSrc, 0, 0,	SRCCOPY );


			}
			SelectObject (hdcSrc, hbmOld);

			DeleteObject( gbgBrush );
	        DeleteDC (hdcSrc);
		}
		ReleaseDC( hWnd, hdc );
	}

	ButtonBarUpdateInfoLine( hWnd );

	DrawButton( hWnd, 0, IDC_PREFS, 0 );
	DrawButton( hWnd, 0, IDC_SCHED, 0 );
	DrawButton( hWnd, 0, IDC_PROCESS, 0 );
	DrawButton( hWnd, 0, IDC_VIEW, 0 );
	DrawButton( hWnd, 0, IDC_APPHELP, 0 );
	DrawButton( hWnd, 0, IDC_APPLOGO, 0 );
	DrawButton( hWnd, 0, IDC_SHOWLIST, 0 );

	RefreshListView();

	CustomUpdateProgressBar( ghProgressWnd, -1, 0 );

	RefreshStatusBar();

	EndPaint(hWnd, &ps);
}



/*
#include <ole2.h>

static IDropTarget myIDROP;


HRESULT HandleMyDrop(
  IDataObject * pDataObject,
                     //Pointer to the interface for the source data
  DWORD grfKeyState, //Current state of keyboard modifier keys
  POINTL pt,         //Current cursor coordinates
  DWORD * pdwEffect  //Pointer to the effect of the drag-and-drop 
                     // operation
)
{


}
 

void InitDragDrop( HWND hWnd )
{
	RegisterDragDrop( hWnd, &myIDROP );


}
*/

LONG APIENTRY ButtonBarProc(HWND hWnd, UINT msg, DWORD dwParam, LONG lParam)
{
    POINT pnt;
	HMENU hMenu;
	short	wParam = (short)dwParam;

	//{ char txt[64]; sprintf( txt, "%08lx: %08lx,%08lx", msg ,dwParam, lParam ); 	StatusSet( txt ); }

    switch (msg) {
        case WM_PAINT:
        	ButtonBarPaint( hWnd );
            break;

		case WM_CLOSE:
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;

		case WM_DESTROY:
            DeleteObject (hbmBackGround);
            DeleteObject (hbmBackGroundRoll);
            DeleteObject (hbmBackGroundHit);
            DeleteObject (hMainPal);
			break;

        case WM_INITDIALOG:
			OutDebug( "Init Tool bar..." );

			InitButtonImages( hWnd );

			if ( hWndLogListView == NULL)
			{
				OutDebug( "Init List" );
				hWndLogListView = InitLogListView( hWnd, 1 );
				OutDebug( "Init List Done" );

				if ( hWndLogListView )
				{
					OutDebug( "Read Log List" );
					ReadLogListAsText( NULL );
					OutDebug( "Read Log List Done." );
				} else
					UserMsg ("Log Listview not created!" );
			}

			OutDebug( "Init Tool bar done." );

			return TRUE;
			break;

		case WM_MBUTTONUP :
			MessageBeep(0);
            pnt.x = LOWORD(lParam);
            pnt.y = HIWORD(lParam);
            ClientToScreen(hWnd, (LPPOINT) &pnt);
            hMenu = GetSubMenu (GetMenu (hWnd), 2);
            if (hMenu) {
                TrackPopupMenu (hMenu, 0, pnt.x, pnt.y, 0, hWnd, NULL);
            } else {
				// Couldn't find the menu...
                MessageBeep(0);
            }
			break;
        
		case WM_RBUTTONUP: // RightClick in windows client area...
			CycleWindowSize( hWnd );
            break;

		case WM_NOTIFY:
			LogListNotifyHandler(hWnd, msg, wParam, lParam);
			break;

        case WM_LBUTTONDOWN: // RightClick in windows client area...
            pnt.x = LOWORD(lParam);
            pnt.y = HIWORD(lParam);
			if ( pnt.x > (TOOLBAR_WIDTH-25) && pnt.y < 100 )
				CycleWindowInfoPanel( hwndParent );
			else
			if ( pnt.x > (TOOLBAR_WIDTH-110) && pnt.y < 100 )
				SendMessage( hwndParent, WM_COMMAND, IDM_ABOUT, lParam);

			break;

		case WM_MOUSEMOVE:
			{
				POINT xy;

				xy.x = LOWORD(lParam);  // horizontal position of cursor 
				xy.y = HIWORD(lParam);  // vertical position of cursor 

			}
			break;

		case WM_SETCURSOR:

			UpdateToolBar(hWnd, GetWindowLong((HWND)dwParam, GWL_ID ));

			{	long bid=0; static long lastid=0;
				bid = GetWindowLong( (HWND)dwParam, GWL_ID );  
				if ( DrawButton( hWnd, 0, bid, 1 ) ){
					if ( lastid != bid )
						DrawButton( hWnd, 0, lastid, 0 );
					lastid = bid;
				}
			}
			break;

		case WM_DRAWITEM:
			{	LPDRAWITEMSTRUCT    lpdis;

				lpdis = (LPDRAWITEMSTRUCT)lParam;    // for convenience

				switch (lpdis->itemAction)
				{
					// handle normal drawing of button, but check if its
					// selected or focus
					case ODA_FOCUS:
						DrawButton( hWnd, lpdis->hDC, LOWORD(dwParam), 1 );
						return TRUE;
					case ODA_DRAWENTIRE:
						DrawButton( hWnd, lpdis->hDC, LOWORD(dwParam), 0 );
						return TRUE;
					case ODA_SELECT:
						DrawButton( hWnd, lpdis->hDC, LOWORD(dwParam), 2 );
						break;
				}  //itemAction
			}
			break;

		case WM_DROPFILES:
			// dragdrop
			if ( !gProcessing && !gProcessingSchedule ){
        		char	filename[MAX_PATH];
				HDROP hDrop = (HDROP) dwParam;  // handle of internal drop structure 
				long	filesNum, count, newlogs=0;

				memset( filename, 0 , MAX_PATH );
				DeSelectLogHistory();

				filesNum = DragQueryFile( hDrop, -1, filename, MAX_PATH );
				for ( count=0; count < filesNum; count++ ){
					DragQueryFile( hDrop, count, filename, MAX_PATH );
					if ( IsPrefsFile( filename ) ){
						strcpy( gPrefsFilename, filename );
						StatusSetID( IDS_LOADSETFILE );
						if ( ReadCurrentPrefs() == 0 )
							MsgBox_Error( IDS_ERR_SETTINGSBAD );
						else
							StatusSet( "Settings File Read OK." );
						ButtonBarPaint( ghMainDlg );
					} else {
						// if filename is already in the list, high light it.
						AddLogToHistory( filename );
						newlogs++;
					}
				}
				DragFinish( hDrop );
				RepaintListView();

				// if ALT is held down, dont process logs
				if ( newlogs>0 && !about_Special ){
					SortLogHistoryFrom( -1 , newlogs );
					StatusSetID( IDS_PROCESSING  );
			        SendMessage( hwndParent, WM_COMMAND, IDM_PROCESS, lParam);
				}
			} else
				StatusSetID( IDS_OK );
			break;


        case WM_COMMAND: {
			//UpdateWindow( hWndLogListView );
            switch (dwParam) {
				case IDM_PROCESS:
				case IDM_PREFS:
				case IDM_NEW:
				case IDM_OPEN:
				case IDM_SAVE:
				case IDM_SAVEAS:
				case IDM_UNDO:
				case IDM_CUT:
				case IDM_COPY:
				case IDM_PASTE:
				case IDM_LINK:
				case IDM_LINKS:
                    SendMessage(hwndParent, WM_COMMAND, dwParam, lParam);
                    break;

				case IDC_APPLOGO:
					DialogBox(hInst, "ABOUT", hWnd, (DLGPROC)AboutDlgProc);
					break;

				case IDC_APPHELP:
					SendMessage(hwndParent, WM_COMMAND, IDM_HELP, lParam);
					break;

				case IDC_SCHED:
					StatusSetID( IDS_EDITSCHEDULE );
					ShowScheduleDialog();
					StatusSetID( IDS_OK );
					UpdateWindow( hwndParent );
					break;

				case IDC_VIEW:
					{
						char virtHostFileName[128];
						mystrcpy( virtHostFileName, MyPrefStruct.outfile );
						// Check if we are doing a "Virtual Host" report, if so, then we look for 
						// the HTML index page (which links us to the specific format)
						//if ( Draw
						if ( MyPrefStruct.ShowVirtualHostGraph() )
							SetFileExtension( virtHostFileName, HTML_EXT );
						if ( GetAsyncKeyState(VK_MENU) )
							ExplorerView( virtHostFileName );
						else
	 						SpawnShowShellFile( virtHostFileName );
					}
					break;


				case IDC_SHOWLIST:
					CycleWindowSize( hWnd );
					break;

				case IDC_PREFS:
                    SendMessage( hWnd, WM_COMMAND, IDM_PREFS, lParam);
					break;

				case IDC_PROCESS:
					if ( !gProcessing && !gProcessingSchedule ){
						// perform preprocess ftp loads
						if ( MyPrefStruct.preproc_on ){
							StatusSetID( IDS_PREPROCESS);
							DefWindowProc(hWnd, msg, dwParam, lParam);
		                    SendMessage(hwndParent, WM_COMMAND, IDM_PROCESSLOGNOW, lParam);
						} else
						// select logs from the history list to process
						if ( CopySelectedHistory_ToLOGQ() ){
							StatusSetID( IDS_PROCESSFROMLIST );
							DefWindowProc(hWnd, msg, dwParam, lParam);
							SendMessage(hwndParent, WM_COMMAND, IDM_PROCESSLOGNOW, lParam);
						} else
						// select logs from the history list to process
						if ( CopySelectedHistory_ToLOGQ() ){
							StatusSetID( IDS_PROCESSFROMLIST );
							DefWindowProc(hWnd, msg, dwParam, lParam);
						} else
						// ask the user which logs to process
						if ( Dialog_SelectLogs() ){
							if ( CopySelectedHistory_ToLOGQ() ){
								StatusSetID( IDS_PROCESSLOG);
								UpdateWindow( hwndParent );
								RepaintListView();
								DefWindowProc(hWnd, msg, dwParam, lParam);
								SendMessage(hwndParent, WM_COMMAND, IDM_PROCESSLOGNOW, lParam);
							}
						} else
						{
							UpdateWindow( hwndParent );
							SysBeep(1);
							StatusSetID( IDS_CANCELLED );
						}					
					} else {
						// STOP all procesing... hopefully
						StopProcessing();
					}
					ButtonBarPaint( ghMainDlg );
					RefreshStatusBar();
					break;
            }
            break;
        }
    }
    return FALSE;
}



// ------------------------ ABOUT WINDOW -------------------------

#define		SCROLL_X	80
#define		SCROLL_Y	100
#define		SCROLL_H	164
#define		SCROLL_W	200
#define		SCROLL_HIDDEN		25

void DoScroll( HWND hDlg, int run )		// 0=init,1=scroll, 2=end;
{

    RECT textrc;
    PAINTSTRUCT ps;
	HFONT hFinePrint;	// Font for 'fine print' in dialog
	HBRUSH bgBrush;
static HGDIOBJ hbmOld;
static	HBITMAP bgBitMap,hbmTemp,hbmMask;
static  HDC		hdcSrc, hdc, hdcMask;
static	int		yoff=0;
static	char	*scrollTextPtr = scrollMsg;
	char txt[512];
	
    hdc = BeginPaint(hDlg, &ps);
    hdc = GetDC (hDlg);
    hdcSrc = CreateCompatibleDC (hdc);
    hdcMask = CreateCompatibleDC (hdc);

	if ( run == 0 ){
    
	    bgBitMap = CreateCompatibleBitmap( hdc, SCROLL_W,SCROLL_H ); //
	    hbmTemp = CreateCompatibleBitmap( hdc, SCROLL_W,SCROLL_H ); //
	    hbmMask = CreateCompatibleBitmap( hdc, SCROLL_W,SCROLL_H ); //

		// copy background
	    SelectObject (hdcSrc, bgBitMap );
	    BitBlt (hdcSrc, 0, 0, SCROLL_W, SCROLL_H, hdc, SCROLL_X, GetPixelYScale(SCROLL_Y), SRCCOPY);

		textrc.left = 0; textrc.top = 0;
		textrc.right = SCROLL_W; textrc.bottom = SCROLL_H;

		// draw in temp buffer
	    SelectObject (hdcSrc, hbmTemp );
		bgBrush = CreateSolidBrush( 0x000000 );
		FillRect( hdcSrc, &textrc, bgBrush );

		run=1; yoff=0;
	}
	
	if ( run == 1 ){
		yoff++;
		if( yoff % 14 == 0 ){
			char *p = txt, *ptr; unsigned long len=255; int bold;
			while( *scrollTextPtr && *scrollTextPtr != '\n' )
				*p++ = *scrollTextPtr++;
			*p = 0;

			if ( p=strstr( txt, "[NAME]" ) ) {
				ptr = GetRegisteredUsername();
				if ( !ptr ){
					GetUserName( p, &len );
					if ( len > 100 )
						sprintf( p, " " );
				} else
					sprintf( p, ptr );
			} else
			if ( p=strstr( txt, "[ORG]" ) ) {
				ptr = GetRegisteredOrganization();
				if ( !ptr ) {
					GetComputerName( p, &len );
					if ( len > 100 )
						sprintf( p, " " );
				} else
					sprintf( p, ptr );
			} else
			//if ( p=strstr( txt, "[ORG]" ) ) sprintf( p, GetRegisteredOrganization() ); else
			if ( p=strstr( txt, "[VER]" ) ) sprintf( p, VERSION_STRING ); else
			if ( p=strstr( txt, "[DAYS]" ) ) sprintf( p, "%.1f Days left in trial", GetDaysLeft() );
			if ( p=strstr( txt, "<B>" ) ) {
				mystrcpy( p, p+3 );
				bold = FW_BOLD;
			} else
				bold = 0;

			if ( *scrollTextPtr == 0 )
				scrollTextPtr = scrollMsg;
			else
				scrollTextPtr++;


			hFinePrint = CreateFont(15, 0, 0, 0, bold, 0, 0, 0, 0, 0, 0, 0, FF_SWISS, "");
			SelectObject (hdcSrc, hFinePrint);
			SetBkMode( hdcSrc, TRANSPARENT );
			
			textrc.left = 0; textrc.top = SCROLL_H-SCROLL_HIDDEN;
			textrc.right = SCROLL_W; textrc.bottom = SCROLL_H;

			SelectObject (hdcSrc, hbmTemp );

			{
				RECT shadrc;
				memcpy( &shadrc, &textrc, sizeof( RECT ) );
				SetTextColor( hdcSrc, 0x202020 );
				shadrc.left++; shadrc.top++;
				DrawText( hdcSrc, txt, strlen(txt), &shadrc, DT_LEFT );
			}
			SetTextColor( hdcSrc, 0x00ffffff );
			DrawText( hdcSrc, txt, strlen(txt), &textrc, DT_LEFT );
			DeleteObject (hFinePrint);
		}

		// restore background
	    hbmOld = SelectObject (hdcSrc, bgBitMap );
		BitBlt (hdc, SCROLL_X, GetPixelYScale(SCROLL_Y), SCROLL_W, SCROLL_H, hdcSrc, 0, 0, SRCCOPY); //
	
		// scroll temp buffer
	    SelectObject (hdcSrc, hbmTemp );
	    BitBlt (hdcSrc, 0, 0, SCROLL_W, SCROLL_H-1, hdcSrc, 0, 1, SRCCOPY );		//SRCINVERT

		// merge temp buffer to window
	    SelectObject (hdcSrc, hbmTemp );
	    BitBlt (hdc, SCROLL_X, GetPixelYScale(SCROLL_Y), SCROLL_W, SCROLL_H-SCROLL_HIDDEN, hdcSrc, 0, 0, SRCPAINT );		//SRCPAINT

		//TransparentBlt (hdc, SCROLL_X, GetPixelYScale(SCROLL_Y), SCROLL_W, SCROLL_H-SCROLL_HIDDEN, hdcSrc, 0,0, SCROLL_W, SCROLL_H-SCROLL_HIDDEN, 0x0000000 );
	
	}

	if ( run == 2 ){
		DeleteObject (bgBitMap);
		DeleteObject (hbmTemp);
		DeleteObject (hbmMask);
		DeleteObject( bgBrush );
	}
	SelectObject (hdcSrc, hbmOld );
    DeleteDC (hdcSrc);
    DeleteDC (hdcMask);
    //SelectObject (hdcSrc, hbmOld);			// Done...
	ReleaseDC( hDlg, hdc );
	EndPaint(hDlg, &ps);
}




/*
Get version info
=================


			// Now lets dive in and pull out the version information:
			dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
			if (dwVerInfoSize) {
				LPSTR   lpstrVffInfo;
				HANDLE  hMem;
				hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
				lpstrVffInfo  = GlobalLock(hMem);
				GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
				// The below 'hex' value looks a little confusing, but
				// essentially what it is, is the hexidecimal representation
				// of a couple different values that represent the language
				// and character set that we are wanting string values for.
				// 040904E4 is a very common one, because it means:
				//   US English, Windows MultiLingual characterset
				// Or to pull it all apart:
				// 04------        = SUBLANG_ENGLISH_USA
				// --09----        = LANG_ENGLISH
				// ----04E4 = 1252 = Codepage for Windows:Multilingual
				lstrcpy(szGetName, "\\StringFileInfo\\040904E4\\");	 
				wRootLen = lstrlen(szGetName); // Save this position
			
				// Set the title of the dialog:
				lstrcat (szGetName, "ProductName");
				bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
					(LPSTR)szGetName,
					(LPVOID)&lpVersion,
					(UINT *)&uVersionLen);

				GlobalUnlock(hMem);
				GlobalFree(hMem);

			} else {
				// No version information available.
			} // if (dwVerInfoSize)

			// We are  using GetVersion rather then GetVersionEx
			// because earlier versions of Windows NT and Win32s
			// didn't include GetVersionEx:
			dwVersion = GetVersion();

			if (dwVersion < 0x80000000) {
				// Windows NT
				wsprintf (szVersion, "Microsoft Windows NT %u.%u (Build: %u)",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
                    (DWORD)(HIWORD(dwVersion)) );
			} else if (LOBYTE(LOWORD(dwVersion))<4) {
				// Win32s
                wsprintf (szVersion, "Microsoft Win32s %u.%u (Build: %u)",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
                    (DWORD)(HIWORD(dwVersion) & ~0x8000) );
			} else {
				// Windows 95
                wsprintf (szVersion, "Microsoft Windows 95 %u.%u",
                    (DWORD)(LOBYTE(LOWORD(dwVersion))),
                    (DWORD)(HIBYTE(LOWORD(dwVersion))) );
			}



 */


static char about_txt[] = { 
"(C) 1997 - 2006 Analysis Software (FREE)\n"
"All Rights Reserved. Analysis Software\0"
};

// each pair per line
static char *aboutinfo_txt[] = { 
"Licensed to",				"[NAME]",
"              ",			"[ORG]",
"              ",			" ",
"Product information",		"[VER]",
"              ",			" ",
"Contact information",		"Phone      +1-800-NOT-FOUND",
"              ",			"Email      iReporter2@gmail.com",
"              ",			"WWW        www.google.com",
#ifdef	DEF_FREEVERSION
"              ",			"WWW        www.google.com",
#endif
"              ",			" ",
#ifndef	DEF_FREEVERSION
"Support       ",			"Email      iReporter2@gmail.com",
"              ",			"WWW        www.google.com",
"              ",			" ",
#endif
0,0

};

						
int ResolveAboutVariables( char *txt )
{
	char *ptr, *p;
	unsigned long len, bold;

	if ( p=strstr( txt, "[NAME]" ) ) {
		ptr = GetRegisteredUsername();
		if ( !ptr ){
			GetUserName( p, &len );
			if ( len > 100 )
				sprintf( p, " " );
		} else
			sprintf( p, ptr );
	} else
	if ( p=strstr( txt, "[ORG]" ) ) {
		ptr = GetRegisteredOrganization();
		if ( !ptr ) {
			GetComputerName( p, &len );
			if ( len > 100 )
				sprintf( p, " " );
		} else
			sprintf( p, ptr );
	} else
	if ( p=strstr( txt, "[VER]" ) )
		sprintf( p, "Version %s", VERSION_STRING );
	else
	if ( p=strstr( txt, "[BUILD]" ) )
	{
		char bdate[128];
		GetAppBuildDate( bdate );
		sprintf( p, "Release build %d (%s)", DaysSince01012001ForBuildDate(), bdate );
	} else
	if ( p=strstr( txt, "[DAYS]" ) )
	{
		if ( GetDaysLeft() > 1000 )
			sprintf( p, "Fully Registered" );
		else {
			if ( GetDaysLeft() <= 0 )
				sprintf( p, "Trial Expired" );
			else {
				char tStr[32];
				GetExpireDate( tStr );
				sprintf( p, "Expires on %s (%.1f Days left)", tStr, GetDaysLeft() );
			}
		}
	}

	if ( p=strstr( txt, "<B>" ) )
	{
		mystrcpy( p, p+3 );
		bold = 1;
	} else
		bold = 0;
	return bold;
}


void PlotAboutText( HWND hDlg )
{
	long i = 0, ycoord = 123;

	WriteXY( hDlg, hLargeFont,  "iReporter 2", 280, 40, 0 );

	// Do each line.
	while( aboutinfo_txt[i] )
	{
		char txt[256], bold;
		char *leftStr = aboutinfo_txt[i++];
		char *rightStr = aboutinfo_txt[i++];

		strcpy( txt, rightStr );

		bold = ResolveAboutVariables( txt );

		WriteXY( hDlg, hSmallBoldFont,      leftStr, 280, ycoord, 0x505050 );
		WriteXY( hDlg, hSmallFont,         txt, 400, ycoord, 0x707070 );

		ycoord += 14;

	}
}


void PlotCopyWriteText( HWND hDlg )
{
	WriteXY( hDlg, hSmallFont, about_txt, 20, 310, 0x222222 );
}



#define	MYWAIT_EVENT	100

      
LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD   dwVerHnd=0;			// An 'ignored' parameter, always '0'
	char    szFullPath[MAX_PATH];
	char    szResult[MAX_PATH];
static	HBITMAP		hbmAbout;
static	BITMAP		bmAbout;
static	HPALETTE	hPal=NULL, hPalOld;
static	int			cycles=0;

	switch (message) {
        case WM_PAINT:
            {
				HGDIOBJ hbmOld;
				RECT rc;
                PAINTSTRUCT ps;
                HDC hdc, hdcSrc;


                GetClientRect(hDlg, &rc);
                hdc = BeginPaint(hDlg, &ps);
				if ( hbmAbout ){
					hdc = GetDC (hDlg);
					hdcSrc = CreateCompatibleDC (hdc);
   					if( iDepth <= 8 ){
   						if( hPal ){
   							hPalOld = SelectPalette(hdc,hPal,FALSE);
							RealizePalette(hdc);
						}
					}
					hbmOld = SelectObject (hdcSrc, hbmAbout );
					SetStretchBltMode( hdc, HALFTONE );
					StretchBlt( hdc,0,0, (rc.right-rc.left), (rc.bottom-rc.top),
								hdcSrc, 0, 0, bmAbout.bmWidth, bmAbout.bmHeight,SRCCOPY );
					SelectObject (hdcSrc, hbmOld);
					DeleteDC (hdcSrc);
					ReleaseDC( hDlg, hdc );
					
					sprintf( szResult, "%s %s", ReturnString( IDS_ABOUT ), PRODUCT_TITLE );
					SetWindowText( hDlg, szResult );
					PlotAboutText( hDlg );
				}
                EndPaint(hDlg, &ps);
            }
            break;
            
		case WM_PALETTECHANGED:
			if( (HWND)wParam != hDlg )
				if( RealizeMyPalette(hDlg, hPal) )
					InvalidateRect(hDlg,NULL,TRUE);
			return 0;
			break;
			
		case WM_NOTIFY:
		case WM_QUERYNEWPALETTE:
			if(RealizeMyPalette(hDlg, hPal)) {
				InvalidateRect(hDlg,NULL,TRUE);
				return TRUE;
			}
			return 0;
			break;
			         
        case WM_DESTROY:
#ifndef _CUSTOM
			AudioStopPlay();
#endif
            DeleteObject (hbmAbout);
            DeleteObject (hPal);
			KillTimer (hDlg, MYWAIT_EVENT);
            break;


        case WM_INITDIALOG:
      		cycles = 0;

			if( !about_Special )
				InitFader( hDlg );

			hbmAbout = LoadBitmap( hInst, ABOUTLOGO );

            GetObject (hbmAbout,sizeof(BITMAP), &bmAbout);
			if ( iDepth <= 8 )
				hPal = (HPALETTE)CreateMyPalette( hDlg, hbmAbout, &bmAbout );
		
			//SendMessage( GetDlgItem(hDlg, IDOK), WM_SETFONT, (UINT)(hSmall8Font), TRUE);

			ShowWindow (hDlg, SW_HIDE);
			GetModuleFileName (hInst, szFullPath, sizeof(szFullPath));


			SetTimer (hDlg, MYWAIT_EVENT, 75, NULL);
			CenterWindowOnScreen( hDlg );
			ShowWindow (hDlg, SW_SHOW);
			if ( about_Special ){
				long w, h;
				w = GetSystemMetrics(SM_CXFULLSCREEN);
				h = GetSystemMetrics(SM_CYFULLSCREEN);

				MoveWindow( hDlg, 0, 0, w, h, TRUE );
				about_Special = FALSE;
			}
			return (TRUE);


	    case WM_TIMER:
			switch (wParam){
				case MYWAIT_EVENT:

					if ( cycles > 3*60*25 ){
			        	DoScroll( hDlg, 2 );
						EndDialog(hDlg, TRUE);
						return (TRUE);
					}
					if ( cycles == 0 )
			 			DoScroll( hDlg, 0 );
					cycles++;
					DoScroll( hDlg, 1 );

					if ( cycles == 25*1 ){
						//AudioPlayResource( "IDR_LOOP1" );
					}
				break;
			}
			break;
			
			
		case WM_LBUTTONUP:
			if ( cycles > 2 ){
				if ( !about_Special )
					FadeOut( hDlg );
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;
	}

    return FALSE;
}







// ------------------------------------------ SPLASH DIALOG ------------------------------------------------------






#define IN_MOTION(hwnd)         GetWindowLong(hwnd,GWL_STYLE) & SS_INMOTION \
                                           ? TRUE : FALSE


LRESULT CALLBACK SplashDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
static	int		cycles=0;
static	HBITMAP	hbmSplash=0;
static	BITMAP	bmSplash;
static	HPALETTE	hPal=NULL, hPalOld;

	switch (message) {
        case WM_PAINT:
            {
				HGDIOBJ hbmOld;
				RECT rc,textrc;
                PAINTSTRUCT ps;
                HDC hdc, hdcSrc;

                GetClientRect(hDlg, &rc);
                hdc = BeginPaint(hDlg, &ps);
				if ( hbmSplash ){
		            hdc = GetDC(hDlg);
		            hdcSrc = CreateCompatibleDC (hdc);
		            if( iDepth <= 8 ){
						if( hPal ){
   							hPalOld = SelectPalette(hdc,hPal,FALSE);
							RealizePalette(hdc);
						}
					}
		            hbmOld = SelectObject (hdcSrc, hbmSplash );
					StretchBlt( hdc,0,0, rc.right-rc.left,rc.bottom-rc.top,
						hdcSrc, 0, 0, bmSplash.bmWidth, bmSplash.bmHeight,SRCCOPY );
					SelectObject (hdcSrc, hbmOld);
		            DeleteDC (hdcSrc);

					{
						HFONT hFinePrint;	// Font for 'fine print' in dialog
						char txt[2000];
						
						textrc.left = 190; textrc.top = 180;
						textrc.right = 480; textrc.bottom = 365;
#ifndef	DEF_FREEVERSION
						{	char str1[256], str2[256], *ptr; unsigned long len(0);
							ptr = GetRegisteredUsername();
							if ( !ptr ){
								GetUserName( str1, &len );
							} else
								sprintf( str1, ptr );
							ptr = GetRegisteredOrganization();
							if ( !ptr ) {
								GetComputerName( str2, &len );
							} else
								sprintf( str2, ptr );
							sprintf( txt, "Registered to :\n%s\n%s\nVersion : %s\n(c) 1997-2002\nRedFlag Software",
								str1, str2, VERSION_STRING );
						}
#endif
						hFinePrint = CreateFont(8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,VARIABLE_PITCH | FF_SWISS, "MS Sans Serif");
						SelectObject (hdc, hFinePrint);
						SetBkMode( hdc, TRANSPARENT );
						SetTextColor( hdc, 0x00ffffff );
						DrawText( hdc, txt, strlen(txt), &textrc, DT_LEFT );
						DeleteObject (hFinePrint);
						//PlotCopyWriteText( hDlg );
					}
					ReleaseDC( hDlg, hdc );
				}
                EndPaint(hDlg, &ps);
            }
            break;

		case WM_CLOSE:
			DestroyWindow( hDlg );
			return 0;

        case WM_DESTROY:
			KillTimer (hDlg, MYWAIT_EVENT);
			if ( hbmSplash )
	            DeleteObject (hbmSplash);
            break;

        case WM_INITDIALOG:
			hbmSplash = LoadBitmap( hInst, SPLASHLOGO );

			if ( hbmSplash ) 
				GetObject (hbmSplash,sizeof(BITMAP), &bmSplash);

			if ( iDepth <= 8 )
				hPal = (HPALETTE)CreateMyPalette( hDlg, hbmSplash, &bmSplash);

			SetTimer (hDlg, MYWAIT_EVENT, 40, NULL);
			CenterWindowOnScreen( hDlg );

			PlotAboutText( hDlg );

			InitFader( hDlg );
			return TRUE;
			break;

	    case WM_TIMER:
			switch (wParam){
				case MYWAIT_EVENT:
					if ( cycles > 50 ){
						FadeOut( hDlg );
						EndDialog(hDlg, TRUE);
						return (TRUE);
					}
					cycles++;
				break;
			}
			break;

		case WM_COMMAND:
			break;
	}

    return FALSE;
}


LRESULT CALLBACK FtpOpenDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char name[64]="\0", site[256]="\0", path[256]="\0";

	switch (message) {
		case WM_PAINT:{
				HDC hdc;
                PAINTSTRUCT ps;
                RECT rc;

                GetClientRect(hDlg, &rc);
				hdc = BeginPaint(hDlg, &ps);
                EndPaint(hDlg, &ps);
            }
			break; 

        case WM_DESTROY:
            break;

        case WM_INITDIALOG:
			CenterWindowOnScreen( hDlg );
			if( !GetReg_Prefs( "ftp_host", site ) ) site[0] = 0;
			if( !GetReg_Prefs( "ftp_path", path ) ) path[0] = 0;
			if( !GetReg_Prefs( "ftp_user", name ) ) name[0] = 0;
			SetText( IDC_FTPUSER, name );
			SetText( IDC_FTPHOST, site );
			SetText( IDC_FTPPATH, path );
			return (TRUE);

		case WM_COMMAND:
			switch( LOWORD(wParam) ){
				case IDC_FTPOK:
					{	char	url[512], pass[16];
						long	fileNum;

						strcpy( url, "ftp://" );

						GetText( IDC_FTPUSER, name, 64 );
						GetText( IDC_FTPHOST, site, 255 );
						GetText( IDC_FTPPATH, path, 255 );
						SetReg_Prefs( "ftp_host", site );
						SetReg_Prefs( "ftp_path", path );
						SetReg_Prefs( "ftp_user", name );

						if ( strlen( name ) ){
							strcat( url, name );
							GetText( IDC_FTPPASS, pass, 16 );
							if ( strlen( pass ) ){
								strcat( url, ":" );
								strcat( url, pass );
							}
							strcat( url, "@" );
						}

						strcat( url, site );
						if ( *path != '/' )
							strcat( url, "/" );
						strcat( url, path );

						fileNum = AddWildcardFtpDirToHistory( url, 0 );

						RepaintListView();
						SendMessage( hwndParent, WM_COMMAND, IDM_PROCESS, lParam);
					}
					EndDialog(hDlg, TRUE);
					return (TRUE);
					break;
				case IDC_FTPCANCEL:
					GetText( IDC_FTPUSER, name, 64 );
					GetText( IDC_FTPHOST, site, 255 );
					GetText( IDC_FTPPATH, path, 255 );
					SetReg_Prefs( "ftp_host", site );
					SetReg_Prefs( "ftp_path", path );
					SetReg_Prefs( "ftp_user", name );
					EndDialog(hDlg, TRUE);
					return (TRUE);
					break;
			}
			break;
	}

    return FALSE;
}











static char support_changepassURL[] = "http://207.33.144.131/suppsite/action.lasso?-database=support.fp5&-layout=d\
efault&-anyerror=chngpass_error.html&-response=chngpass.html&-logicaloperator=and&-operator=eq&email=%s&-operator=eq&supppass=%s&-operator=neq&reg=%s&-search";

LRESULT CALLBACK ChangePasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char name[256]="\0", passwd[256]="\0", other[256]="\0";

	switch (message) {
		case WM_PAINT:{
				HDC hdc;
                PAINTSTRUCT ps;
                RECT rc;

                GetClientRect(hDlg, &rc);
				hdc = BeginPaint(hDlg, &ps);
                EndPaint(hDlg, &ps);
            }
			break; 

        case WM_DESTROY:
            break;

        case WM_INITDIALOG:
			CenterWindowOnScreen( hDlg );
			if( !GetReg_Prefs( "user_email", name ) ) name[0] = 0;
			SetText( IDC_SUPPEDIT_EMAIL, name );
			SetText( IDC_SUPPEDIT_OLD, "" );
			SetText( IDC_SUPPEDIT_NEW, "" );
			SetText( IDC_SUPPEDIT_VERIFY, "" );
			return (TRUE);

		case WM_COMMAND:
			switch( LOWORD(wParam) ){
				case IDC_SUPPEDIT_CLR:
					SetText( IDC_SUPPEDIT_EMAIL, "" );
					SetText( IDC_SUPPEDIT_OLD, "" );
					SetText( IDC_SUPPEDIT_NEW, "" );
					SetText( IDC_SUPPEDIT_VERIFY, "" );
					break;
				case IDC_SUPPEDIT_OK:
					{	char	url[1024], old[32], newp[32], verify[32];

						GetText( IDC_SUPPEDIT_EMAIL, name, 255 );
						GetText( IDC_SUPPEDIT_OLD, old, 32 );
						GetText( IDC_SUPPEDIT_NEW, newp, 32 );
						GetText( IDC_SUPPEDIT_VERIFY, verify, 32 );

						if ( !strcmp( newp, verify ) ){
							sprintf( url, support_changepassURL, name, old, newp );
							ShowHTMLShortcut( hDlg, url, NULL );

							SetReg_Prefs( "user_email", name );
							SetReg_Prefs( "user_pass", newp );
						}
					}
					EndDialog(hDlg, TRUE);
					return (TRUE);
					break;
				case IDC_SUPPEDIT_CANCEL:
					EndDialog(hDlg, TRUE);
					return (TRUE);
					break;
			}
			break;
	}
    return FALSE;
}









static char support_loginURL[] = "http://207.33.144.131/suppsite/action.lasso?-database=support.fp5&-layout=d\
efault&-anyerror=searcher.html&-response=welcome.html&-logicaloperator=and&-operator=eq&email=%s&-operator=eq&supppass=%s&-search";


LRESULT CALLBACK SupportZoneDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char	name[256]="\0", passwd[256]="\0";
	char	url[1024];

	switch (message) {
		case WM_PAINT:{
				HDC hdc;
                PAINTSTRUCT ps;
                RECT rc;

                GetClientRect(hDlg, &rc);
				hdc = BeginPaint(hDlg, &ps);
                EndPaint(hDlg, &ps);
            }
			break; 

        case WM_DESTROY:
            break;

        case WM_INITDIALOG:
			if( !GetReg_Prefs( "user_pass", passwd ) )	passwd[0] = 0;
			if( !GetReg_Prefs( "user_email", name ) )	name[0] = 0;

			if ( name[0] && passwd[0] ){
				sprintf( url, support_loginURL, name, passwd );
				ShowHTMLShortcut( hDlg, url, NULL );
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			SetText( IDC_SUPP_EMAIL, name );
			SetText( IDC_SUPP_PASS, passwd );
			CenterWindowOnScreen( hDlg );
			return (TRUE);

		case WM_COMMAND:
			switch( LOWORD(wParam) ){
				case IDC_SUPP_CLR:
					SetText( IDC_SUPP_EMAIL, "" );
					SetText( IDC_SUPP_PASS, "" );
					break;
				case IDC_SUPP_OK:
					GetText( IDC_SUPP_EMAIL, name, 255 );
					GetText( IDC_SUPP_PASS, passwd, 32 );

					SetReg_Prefs( "user_email", name );
					SetReg_Prefs( "user_pass", passwd );

					sprintf( url, support_loginURL, name, passwd );
					ShowHTMLShortcut( hDlg, url, NULL );

					EndDialog(hDlg, TRUE);
					return (TRUE);
					break;
				case IDC_SUPP_CANCEL:
					EndDialog(hDlg, TRUE);
					return (TRUE);
					break;
			}
			break;
	}
    return FALSE;
}




/*
The following example creates a status window that has a sizing grip and divides the window into four equal parts based on the width of the parent
window's client area. 

You set the text of any part of a status window by sending the SB_SETTEXT message, specifying the zero-based index of a part, a pointer to
the string to draw in the part, and the technique for drawing the string. The drawing technique determines whether the text has a border and, if it does, the
style of the border. It also determines whether the parent window is responsible for drawing the text. For more information, see the following topic. 

*/

// Creates a status window and divides it into 
//     the specified number of parts. 
// Returns the handle to the status window. 
// hwndParent - parent window for the status window 
// nStatusID - child window identifier 
// hinst - handle to the application instance 
// nParts - number of parts into which to divide the status window 

HWND CreateXStatusWindow( int nParts, HWND parentWnd )
{
	HWND hwndStatus; 
	RECT rcClient; 
	HLOCAL hloc; 
	LPINT lpParts; 
	int i, nWidth; 

	// Ensure that the common control DLL is loaded. 

	// Create the status window. 
	hwndStatus = CreateWindowEx( 
		0,                      // no extended styles 
		STATUSCLASSNAME,        // name of status window class 
		(LPCTSTR) "OK",		// no text when first created 
		//SBARS_SIZEGRIP|WS_CHILD|WS_VISIBLE,
		WS_CHILD|WS_VISIBLE,
						        // includes a sizing grip , creates a child window 
		0, 0, 0, 0,             // ignores size and position 
		parentWnd,              // handle to parent window 
		(HMENU) 0,				// child window identifier 
		hInst,                  // handle to application instance 
		NULL);                  // no window creation data 

	if ( hwndStatus ){
		GetClientRect(parentWnd, &rcClient); 		// Get the coordinates of the parent window's client area. 
		hloc = LocalAlloc(LHND, sizeof(int) * nParts); 		// Allocate an array for holding the right edge coordinates. 
		lpParts = (LPINT)LocalLock(hloc); 

		// Calculate the right edge coordinate for each part, and 
		// copy the coordinates to the array. 
		nWidth = (rcClient.right-18) / nParts; 
		for (i = 0; i < nParts; i++) { 
			lpParts[i] = nWidth; 
			nWidth += nWidth; 
		} 

		SendMessage(hwndStatus, SB_SETPARTS, (WPARAM) nParts, (LPARAM) lpParts); 		// Tell the status window to create the window parts. 

		// Free the array, and return. 
		LocalUnlock(hloc); 
		LocalFree(hloc); 
	}
	return hwndStatus; 
} 


static int statusTimer;


// NOT USED, but doesnt hurt to leave the code in
HWND CreateOneStatusWindow( HWND parentWnd )
{
	HWND hwndStatus; 
	RECT rcClient; 
	HLOCAL hloc; 
	LPINT lpParts; 
	int nParts=1; 

	// Ensure that the common control DLL is loaded. 

	// Create the status window. 
	hwndStatus = CreateWindowEx( 
		0,                      // no extended styles 
		STATUSCLASSNAME,        // name of status window class 
		(LPCTSTR) "OK",		// no text when first created 
		//SBARS_SIZEGRIP|WS_CHILD|WS_VISIBLE,
		WS_CHILD|WS_VISIBLE,
						        // includes a sizing grip , creates a child window 
		0, 0, 0, 0,             // ignores size and position 
		parentWnd,              // handle to parent window 
		(HMENU) 0,				// child window identifier 
		hInst,                  // handle to application instance 
		NULL);                  // no window creation data 

	if ( hwndStatus ){
		// Get the coordinates of the parent window's client area. 
		GetClientRect(parentWnd, &rcClient); 

		// Allocate an array for holding the right edge coordinates. 
		hloc = LocalAlloc(LHND, sizeof(int) * nParts); 
		lpParts = (LPINT)LocalLock(hloc); 

		// Calculate the right edge coordinate for each part, and 
		// copy the coordinates to the array. 
		lpParts[0] = (rcClient.right-18)/2;

		// Tell the status window to create the window parts. 
		SendMessage(hwndStatus, SB_SETPARTS, (WPARAM) nParts, (LPARAM) lpParts); 

		// Free the array, and return. 
		LocalUnlock(hloc); 
		LocalFree(hloc); 
	}
	return hwndStatus; 
} 


void GetRectofStatusBarItem( long iPart, RECT *lprc )
{
	DEF_PRECONDITION(lprc);

	if ( ghStatusWnd )
	{
		SendMessage( ghStatusWnd, SB_GETRECT, (WPARAM) iPart, (LPARAM) (LPRECT) lprc);
	}
	else
	{
		SetRectEmpty( lprc );
	}
}

void StatusWindowXSetText( char *txt,  HWND statWnd )
{
	if ( statWnd && gNoGUI==FALSE ){
		SendMessage( statWnd, SB_SETTEXT, (WPARAM) 0, (LPARAM) txt ); 
		UpdateWindow( statWnd );
	}
}





void StatusWindowSetText( char *txt )
{
	StatusSet( txt );
}


// ParseALargeFile - parses a large file and uses a progress bar to 
//   indicate the progress of the parsing operation. 
// Returns TRUE if successful or FALSE otherwise. 
// hwndParent - parent window of the progress bar 
// lpszFileName - name of the file to parse 



int CreateProgressBar( long totalVal ) 
{ 
	RECT rcClient;  // client area of parent window 
	RECT statBar;
	int width,height;

	GetRectofStatusBarItem( 1, &statBar );
	// Ensure that the common control DLL is loaded and create a 
	// progress bar along the bottom of the client area of the 
	// parent window. Base the height of the progress bar on 
	// the height of a scroll bar arrow. 
	if ( ghProgressWnd == NULL ){
		GetClientRect(hwndParent, &rcClient);  
		width = rcClient.right;
		height = statBar.bottom - statBar.top;

		ghProgressWnd = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR) NULL, 
			WS_CHILD | WS_VISIBLE,
			statBar.left, rcClient.bottom - height, 
			statBar.right - statBar.left, height, 
			hwndParent, (HMENU) 0, (HINSTANCE)hInst, NULL); 

		// Set the range and increment of the progress bar. 
		SendMessage(ghProgressWnd, PBM_SETRANGE, 0, MAKELPARAM(0, totalVal)); 
		SendMessage(ghProgressWnd, PBM_SETSTEP, (WPARAM) 1, 0); 

		if ( !hbmProgress1 && !hbmProgress2 ){
			//hbmProgress1 = LoadBitmap( hInst, "IDB_PROGRESSBAR1" );
			//hbmProgress2 = LoadBitmap( hInst, "IDB_PROGRESSBAR2" );
			hbmProgress1 = LoadBitmap( hInst, "IDB_PROGRESS_D" );
			hbmProgress2 = LoadBitmap( hInst, "IDB_PROGRESS_I" );
		}
	}
	return TRUE; 
} 

void SetRangeProgressBar( long totalVal );

void SetRangeProgressBar( long totalVal )
{
	SendMessage(ghProgressWnd, PBM_SETRANGE, 0, MAKELPARAM(0, totalVal)); 
}


/*
BOOL BitBlt(
  HDC hdcDest, // handle to destination DC
  int nXDest,  // x-coord of destination upper-left corner
  int nYDest,  // y-coord of destination upper-left corner
  int nWidth,  // width of destination rectangle
  int nHeight, // height of destination rectangle
  HDC hdcSrc,  // handle to source DC
  int nXSrc,   // x-coordinate of source upper-left corner
  int nYSrc,   // y-coordinate of source upper-left corner
  DWORD dwRop  // raster operation code
);*/


void TileCopy( HDC dest, int xpos, int width, int height, int tilewidth, HDC src, int xoffset )
{
	int x = 0, w = width;
	int xsr, total;

	total = width/tilewidth;
	xsr = width % tilewidth;

	while( x < total )
	{
		BitBlt( dest, xpos+(x*tilewidth), 0, tilewidth, height,   src, xoffset, 0, SRCCOPY );
		x++;
	}
	if ( xsr )
		BitBlt( dest, xpos+(x*tilewidth), 0, xsr, height,   src, xoffset, 0, SRCCOPY );
}



// Now this is tricky
// level is  [ 0...1000 ] for a percet of 0...100
// if level is -1, it means to redraw the last value
// if type is 1 , use other image type
// if the level is > 1000 , use other image type too.
void CustomUpdateProgressBar( HWND hWnd, long level, long type )
{
		HGDIOBJ hbmOld, hbmProg;
		HDC		hdc, hdcSrc;//, hdcDst;
		RECT	statBar;
		int	w, size, xs;
static	int	lastsize=-1, xoffset = 0;

	if ( hWnd ){
		GetWindowRect( hWnd, &statBar );
		w = statBar.right - statBar.left;

		if( level>=0 )
			size = w*level/1000;
		else
			size = lastsize;

		if( size>=0 && size!=lastsize || level == -1 ){
			if ( type == 0 && level<=1000 ){
				hbmProg = hbmProgress1;
				xs = 16;
			} else {
				hbmProg = hbmProgress2;
				xs = 32;
			}

			hdc = GetDC (hWnd);
			hdcSrc = CreateCompatibleDC (hdc);

			hbmOld = SelectObject (hdcSrc, hbmProg );

			TileCopy( hdc, 0, size, statBar.bottom-statBar.top, xs, hdcSrc, xoffset );
			//StretchBlt( hdc, 1,1, size, statBar.bottom-statBar.top, hdcSrc, 0,0, 1, 10, PATPAINT );
			//BitBlt( hdc, 1, 1, size, statBar.bottom-statBar.top,   hdcSrc, 0, 0, PATCOPY );
			if ( level == -1 )
				xoffset++;
			if ( xoffset > xs ) xoffset = 0;

			statBar.left = size;
			statBar.top = 0;
			FillRect( hdc, &statBar, GetSysColorBrush(COLOR_3DFACE) );
			DeleteDC (hdcSrc);

			lastsize = size;
			ReleaseDC( hWnd, hdc );
		}
	}
}




void CustomUpdateDualProgressBar( HWND hWnd, long level, long level2, long type )
{
		HGDIOBJ hbmOld,
				hbmProgressbar1 = hbmProgress1, 
				hbmProgressbar2 = hbmProgress1B;
		HDC		hdc, hdcSrc;//, hdcDst;
		RECT	statbar_rect;
		int		fullwidth, 
				barwidth;
static	int		lastbar1size=-1,
				lastbar2size=-1, 
				bar1size, 
				bar2size,
				xoffset = 0;

	if ( hWnd ){
		GetWindowRect( hWnd, &statbar_rect );
		fullwidth = statbar_rect.right - statbar_rect.left;

		if( level>=0 ){
			bar1size = (fullwidth*(level-level2))/1000;
			bar2size = (fullwidth*level2)/1000;
		} else {
			bar1size = lastbar1size;
			bar2size = lastbar2size;
		}

		if( bar1size>=0 && bar1size!=lastbar1size || level == -1 )
		{
			if ( type == 0 && level<=1000 ){
				hbmProgressbar1 = hbmProgress1;
				barwidth = 16;
			} else {
				hbmProgressbar1 = hbmProgress2;
				barwidth = 32;
			}

			hdc = GetDC (hWnd);
			hdcSrc = CreateCompatibleDC (hdc);

			// Fill COLOURED portion of bar
			hbmOld = SelectObject (hdcSrc, hbmProgressbar1 );
			TileCopy( hdc, bar2size, bar1size, statbar_rect.bottom-statbar_rect.top, barwidth, hdcSrc, xoffset );

			SelectObject (hdcSrc, hbmProgressbar2 );
			TileCopy( hdc, 0, bar2size, statbar_rect.bottom-statbar_rect.top, barwidth, hdcSrc, xoffset );

			// Fill GREY on the RIGHT side of bar (empty portion)
			statbar_rect.left = bar1size;
			statbar_rect.top = 0;
			FillRect( hdc, &statbar_rect, GetSysColorBrush(COLOR_3DFACE) );
			DeleteDC (hdcSrc);
			ReleaseDC( hWnd, hdc );

			// rotate patterns
			if ( level == -1 )
				xoffset++;
			if ( xoffset > barwidth ) 
				xoffset = 0;
			lastbar1size = bar1size;
			lastbar2size = bar2size;
		}
	}
}

int UpdateProgressBar( long level ) 
{ 
	// Advance the current position of the progress bar 
	// by the increment. 
	if ( ghProgressWnd  && gNoGUI==FALSE ){
		CustomUpdateProgressBar( ghProgressWnd, level, 0 );
	}
	return TRUE; 
} 

void RefreshStatusBar( void )
{
	if( ghStatusWnd )
	{
		UpdateWindow( ghStatusWnd );
	}

	if( ghProgressWnd )
	{
		UpdateWindow( ghProgressWnd );
	}
}

void StatusWindowSetProgress( long level, char *text )
{
	if ( text ) StatusSet( text );
	UpdateProgressBar( level );
}

int DeleteProgressBar( void ) 
{ 
	if ( ghProgressWnd )
		DestroyWindow( ghProgressWnd ); 
	return TRUE; 
} 





//
//
//
// ------------------->>>>>   funnel web code begins here
//
//
//
//

/*

www_OpenURL - Opens a URL to window or file.

www_ShowFile - Passes a file to a MIME type to be rendered.

www_Activate - Brings the browser to the front and displays a specified window.

HINSTANCE ShellExecute( HWND hwnd, 
 // handle to parent window 
 
LPCTSTR lpOperation, 
 // pointer to string that specifies operation to perform 
 
LPCTSTR lpFile, 
 // pointer to filename or folder name string 
 
LPCTSTR lpParameters, 
 // pointer to string that specifies executable-file parameters 
 
LPCTSTR lpDirectory, 
 // pointer to string that specifies default directory 
 
INT nShowCmd 
 // whether file is shown when opened 
 
); 
 

*/








long InitApp(void)
{	

	OutDebug( "Init Defaults" );

	std::string fileName( gPath );
	fileName += CQSettings::SETTINGS_APP_WIDE_DEFAULT;
	CQSettings::TheSettings().OpenAppWideSettings( fileName.c_str() );    
	initLookupGrid();
	stripxchar( gPath, '\\' );
	CreateDefaults(&MyPrefStruct,1);
	strcpy( gPrefsFilename, s_DefaultSettingsFileName );

	OutDebug( "Init Scheduler ...." );
	InitReadSchedule( 0 );
	OutDebug( "Init Scheduler Done" );
	return 1;
}


void ExitApp(void)
{	
	SaveLogListAsText( NULL );

	ClearLogList();		// this actually deallocates the filenames, which is of course quite usefull

	StatusSetID( IDS_QUIT);
	//ProfilerDump("\pProfiler.prof");
}









void AddItemToLogListView( HWND hwin, int index, long columns )
{
		LV_ITEM lvI; 
		int iSubItem;       // Index for inserting sub items

		lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
		lvI.state = 0;      //
		lvI.stateMask = LVIS_SELECTED;
		lvI.iItem = index;
		lvI.iSubItem = 0;

		lvI.pszText = LPSTR_TEXTCALLBACK; 
		lvI.cchTextMax = 256;
		lvI.iImage = 0;
		ListView_InsertItem(hwin, &lvI);

		for (iSubItem = 1; iSubItem <= columns; iSubItem++){
			ListView_SetItemText( hwin,	index,	iSubItem, LPSTR_TEXTCALLBACK);
		}
		ListView_SetItemState( hwin, index,	LVIS_SELECTED|LVIS_FOCUSED  , 0xf  );
		ListView_RedrawItems( hwin,  index , index );
}




long FindLogInHistoryAndSelect( char *txt )
{
	LVFINDINFO find;
	int index;

	find.flags = LVFI_STRING;
	find.psz = txt;

	index = ListView_FindItem( hWndLogListView, -1, &find );

	if ( index != -1 ){
		ListView_SetItemState( hWndLogListView, index,	LVIS_SELECTED|LVIS_FOCUSED  , 0xf  );
		ListView_RedrawItems( hWndLogListView,  index , index );
	}

	return index;
}

long	columnIDs[] = { IDS_LOGLIST_COL1, IDS_LOGLIST_COL2, IDS_LOGLIST_COL3, IDS_LOGLIST_COL4, IDS_LOGLIST_COL5, IDS_LOGLIST_COL6, 0 };
#define	MAINLIST_COLUMNS 4

long AddLogToGUIHistory( char *filename, long tot )
{
	AddItemToLogListView( hWndLogListView, tot, MAINLIST_COLUMNS );
	ListView_Update( hWndLogListView , tot );
	return 1;
}


// Add A log to the internal history and also to the GUI, but if its already
// there in the GUI, just highlight it.
long AddLogToHistory( char *log )
{
	long total;
	long l_total = logfilesTotal;		// remember current total

	total = AddLogToHistoryEx( log, false );

	// only if there has been logs added to the history, do we add them to the gui
	if ( total > 0 && total != l_total )
	{
		AddLogToGUIHistory( log, total-1 );
	} else
	// other wise, just try to select the log in the list with the same name.
		FindLogInHistoryAndSelect( log );

	return total;
}


void ClearLogHistory( void )
{
	ListView_DeleteAllItems( hWndLogListView );
	ClearLogList();
	UpdateWindow( hWndLogListView );
}

void DeleteSelectedLogHistory( HWND hWnd )
{
	long count, item, logcount=0;
	char	newlogfile[MAX_PATH];

	memset( newlogfile, 0, MAX_PATH );
	count = ListView_GetSelectedCount( hWndLogListView );
	if ( count ){
		item = ListView_GetNextItem( hWndLogListView, -1 , LVNI_SELECTED );
		while( item != -1 ){
			ListView_DeleteItem( hWndLogListView, item );
			DeleteLogXinHistory( item );
			item = ListView_GetNextItem( hWndLogListView, -1 , LVNI_SELECTED );
			logfilesTotal--;
		}
	}
	UpdateWindow( hWndLogListView );
}

void DeSelectLogHistory( void )
{
	long count, item, logcount=0;
	char	newlogfile[MAX_PATH];

	memset( newlogfile, 0, MAX_PATH );
	count = ListView_GetSelectedCount( hWndLogListView );
	if ( count ){
		item = ListView_GetNextItem( hWndLogListView, -1 , LVNI_SELECTED );
		while( item != -1 ){
			ListView_SetItemState( hWndLogListView, item, LVIS_FOCUSED, 0xf );
			item = ListView_GetNextItem( hWndLogListView, -1 , LVNI_SELECTED );
		}
	}
	UpdateWindow( hWndLogListView );
}



void CompressSelectedLogHistory( HWND hWnd )
{
	long count, item, logcount=0;
	char	newlogfile[MAX_PATH];

	memset( newlogfile, 0, MAX_PATH );
	count = ListView_GetSelectedCount( hWndLogListView );
	if ( count ){
		item = ListView_GetNextItem( hWndLogListView, -1 , LVNI_SELECTED );
		while( item != -1 ){
			char *logs[2], *p;
			logs[0] = p = FindLogXinHistory( item );
			if ( !CompressLogFiles( logs, 1, COMPRESS_GZIP, TRUE ) ){
				strcat( p, ".gz" );
				ListView_RedrawItems( hWndLogListView, item, item );
			}
			
			item = ListView_GetNextItem( hWndLogListView, item , LVNI_SELECTED );
		}
	}
	//UpdateWindow( hWndLogListView );
	//ListView_Update( hWndLogListView , count );
}



void CompressBzip2SelectedLogHistory( HWND hWnd )
{
	long count, item, logcount=0;
	char	newlogfile[MAX_PATH];

	memset( newlogfile, 0, MAX_PATH );
	count = ListView_GetSelectedCount( hWndLogListView );
	if ( count ){
		item = ListView_GetNextItem( hWndLogListView, -1 , LVNI_SELECTED );
		while( item != -1 ){
			char *logs[2], *p;
			logs[0] = p = FindLogXinHistory( item );
			if ( !CompressLogFiles( logs, 1, COMPRESS_BZIP2, TRUE ) ){
				if ( p=strstr(p, ".gz" ) )
					mystrcpy( p, ".bz2" );
				else
					strcat( logs[0], ".bz2" );
				ListView_RedrawItems( hWndLogListView, item, item );
			}
			
			item = ListView_GetNextItem( hWndLogListView, item , LVNI_SELECTED );
		}
	}
	UpdateWindow( hwndParent );
}


void ShowLogDuration()
{
	long count, item;

	count = ListView_GetSelectedCount( hWndLogListView );
	if ( count )
	{
		item = ListView_GetNextItem( hWndLogListView, -1 , LVNI_SELECTED );
		while( item != -1 )
		{
			if ( !LogFileHistory[item].endDateKnown )
				AddLogToHistoryEx( LogFileHistory[item].file, true );
			ListView_RedrawItems( hWndLogListView, item, item );
			item = ListView_GetNextItem( hWndLogListView, item , LVNI_SELECTED );
		}
	}
}



void RefreshListView( void )
{
	//UpdateWindow( hwndParent );

	if( hWndLogListView ){
//		ShowWindow ( hWndLogListView, SW_HIDE );/
//		ShowWindow ( hWndLogListView, SW_SHOW );
		UpdateWindow( hWndLogListView );
		//ListView_RedrawItems( hWndLogListView, 0 , logfilesTotal+ 1 );
	}

//	UpdateWindow( ghStatusWnd );
//	UpdateWindow( ghProgressWnd );
}



void RepaintListView( void )
{
	//UpdateWindow( hwndParent );

	if( hWndLogListView ){
//		ShowWindow ( hWndLogListView, SW_HIDE );/
//		ShowWindow ( hWndLogListView, SW_SHOW );
		UpdateWindow( hWndLogListView );
		ListView_RedrawItems( hWndLogListView, 0 , logfilesTotal+ 1 );
	}

//	UpdateWindow( ghStatusWnd );
//	UpdateWindow( ghProgressWnd );
}



long CopySelectedHistory_ToLOGQ( void )
{
	long count, item, logcount=0;
	char	newlogfile[MAX_PATH];

	memset( newlogfile, 0, MAX_PATH );
	count = ListView_GetSelectedCount( hWndLogListView );

	if ( count ){			// copy log files to global list
		item = ListView_GetNextItem( hWndLogListView, -1 , LVNI_SELECTED );

		while( item != -1 )
		{
			mystrcpy( newlogfile, FindLogXinHistory( item ) );

			AddFileToLogQ( newlogfile, logcount++ );
			item = ListView_GetNextItem( hWndLogListView, item , LVNI_SELECTED );
		}
	}
	return count;
}


char *GetLogListViewItem( void )
{
	long count, item, logcount=0;
	char	newlogfile[MAX_PATH];

	memset( newlogfile, 0, MAX_PATH );
	count = ListView_GetSelectedCount( hWndLogListView );
	if ( count ){
		item = ListView_GetNextItem( hWndLogListView, -1 , LVNI_SELECTED );
		if( item != -1 ){
			return FindLogXinHistory( item );
		}
	}
	return NULL;
}



//extern int IsFileDataBase( char *filename );

LRESULT LogListNotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
int				itemSel = pNm->iItem;
char			*logfilename;
static	char	szText[MAX_PATH]="(null)\0";    // Place to store some text

	if (wParam == IDC_LOGLIST)
	{
		switch(pLvdi->hdr.code)	
		{
			case LVN_KEYDOWN:
				{
					LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
					if ( pnkd->wVKey == VK_DELETE )
					{
						DeleteSelectedLogHistory( hWnd );
					} else
					if ( pnkd->wVKey == VK_F5 )
					{
						ResetLogList();
						RepaintListView();
					}
				}
				break;

			case LVN_GETDISPINFO:
				itemSel = pLvdi->item.iItem;
				if ( logfilename = FindLogXinHistory( itemSel ) )
				{
					long type=0,date1,date2;

					pLvdi->item.pszText = szText;
					switch (pLvdi->item.iSubItem)
					{
						case 0:		mystrcpy( szText, logfilename );
									//sprintf( szText, "%d", GetCurrentCTime() );
									MakeSafeURL( szText );
									if ( LogFileHistory[itemSel].type == -1 && !gProcessing )
									{
#ifdef DEF_DEBUG
										OutDebug( "LogListNotifyHandler : GetLogFileType()" );
#endif
										LogFileHistory[itemSel].size = GetLogFileType( logfilename, &type, &date1, &date2 );
										LogFileHistory[itemSel].type = (short)type;
										LogFileHistory[itemSel].startDate = date1;
										LogFileHistory[itemSel].endDate = date2;
									}
									break;
						
						case 1:		//OutDebugs( "size = %lld", LogFileHistory[itemSel].size );
									if (LogFileHistory[itemSel].size <= 1024)
										FormatLongNum( (long)LogFileHistory[itemSel].size, szText );
									else
									if (LogFileHistory[itemSel].size <= 1024*1024*1024)
									{
										FormatLongNum( (long)(LogFileHistory[itemSel].size/1024), szText );
										strcat( szText, " KB" );
									} else
									{
										FormatLongNum( (long)(LogFileHistory[itemSel].size/(1024*1024)), szText );
										strcat( szText, " MB" );
									}
									break;

						case 2:		GetLogTypeName( LogFileHistory[itemSel].type, szText ); break;
						case 3:		if ( LogFileHistory[itemSel].startDate )
										DateTimeToString( LogFileHistory[itemSel].startDate, szText );
									else
										szText[0] = 0;
									break;
						case 4:		if ( LogFileHistory[itemSel].endDate )
										DateTimeToString( LogFileHistory[itemSel].endDate, szText );
									else
										strcpy( szText, "Unknown" );
									break;
						case 5:		szText[0] = 0;
									if ( LogFileHistory[itemSel].endDate )
										DateTimeDurationToString( LogFileHistory[itemSel].startDate, LogFileHistory[itemSel].endDate, 7, szText );
									else
										strcpy( szText, "Unknown" );
									break;
						case 6:		if ( type & 0x1000 )
										strcpy( szText, "VHost" );
									else
										strcpy( szText, "" );
									break;
					}
				}
				break;
		
			case LVN_BEGINLABELEDIT:
				return TRUE;
				break;
			case NM_DBLCLK:
				SendMessage( ghMainDlg, WM_COMMAND, IDC_PROCESS , lParam);
				break;
			case NM_CLICK:
				if ( itemSel == -1 ){
					LVHITTESTINFO pinfo;

					pinfo.pt = pNm->ptAction;
					itemSel = SendMessage( hWnd, LVM_SUBITEMHITTEST , 0, (LPARAM)&pinfo );
				}
				break;
			case NM_RCLICK:
				{
					POINT	pnt;

					GetCursorPos( &pnt );
					// This is where you would determine the appropriate 'context'
					// menu to bring up. Since this app has no real functionality,
					// we will just bring up the 'Help' menu:
					if (hPopupMenu) {
						TrackPopupMenu (GetSubMenu (hPopupMenu, 0), 0, pnt.x, pnt.y, 0, hwndParent, NULL);
					} 
				}
				break;
			default:
				break;
		}
	}
	return 0L;
}

 
HWND InitLogListView (HWND hWndParent, long itemNum )
{
	LV_COLUMN lvC;      // List View Column structure
	HWND hWndList;      // Handle to the list view window
	RECT rcl, rclist;           // Rectangle for setting the size of the window
	char szText[MAX_PATH];    // Place to store some text
	int index;          // Index used in for loops

	// Get the size and position of the parent window
	GetClientRect( hWndParent, &rcl);
	GetObjectRect( hWndParent, GetDlgItem( hWndParent, IDC_LOGLIST ), &rclist );

	hWndList = GetDlgItem( hWndParent, IDC_LOGLIST );

	// Now initialize the columns we will need
	// Initialize the LV_COLUMN structure
	// the mask specifies that the .fmt, .ex, width, and .subitem members 
	// of the structure are valid,
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.cx = (rclist.right-rclist.left)/MAINLIST_COLUMNS;            // width of the column, in pixels
	lvC.pszText = szText;
	// Add the columns.
	for (index = 0; index < MAINLIST_COLUMNS; index++)
	{
		lvC.cx = 500;

		GetString( columnIDs[index], szText, 256 );

		// Set the width for each of the columns
		if ( index == 0 ) // Log file name
			lvC.cx = 300;
		else if ( index == 1 ) // Log file size
			lvC.cx = 70;
		else if ( index == 2 ) // Log file format
			lvC.cx = 70;
		else if ( index == 3 || index == 4 ) // Start & End Date/time
			lvC.cx = 120;
		else if ( index == 5 ) // Log file time duration
			lvC.cx = 100;

		// Set the allignment for the columns
		if ( index > 0 )
			lvC.fmt = LVCFMT_RIGHT;

		lvC.iSubItem = index;
		if (ListView_InsertColumn(hWndList, index, &lvC) == -1){
			UserMsg ( "ListView_InsertColumn() == -1" );
			return NULL;
		}
	}
	ListView_SetExtendedListViewStyleEx( hWndList, LVS_EX_FULLROWSELECT , LVS_EX_FULLROWSELECT );
	return (hWndList);

}


void SaveLogListAsText( char *filename )
{
	FILE *fp;
	long count=0;
	char fullname[MAX_PATH];

	if ( filename )
		strcpy( fullname, filename );
	else
		sprintf( fullname, "%s\\loglist.dat", gDataPath );

	if ( fp = fopen( fullname, "w+" ) ){
		fprintf( fp, "#-- Log List %d\n", logfilesTotal );
		while( count < logfilesTotal ){
			fprintf( fp, "%s\n", LogFileHistory[count].file );
			count++;
		}
		fclose( fp );
	}
}




void ReadLogListAsText( char *filename )
{
	FILE *fp;
	char szText[MAX_PATH];
	long len;

	if ( filename )
		strcpy( szText, filename );
	else {
		sprintf( szText, "%s\\loglist.dat", gDataPath );
		if ( !GetFileLength( szText ) )
			sprintf( szText, "%s\\loglist.dat", gPath );
	}

	fp = fopen( szText, "r" );

	if ( fp )
	{
		ClearLogHistory();
		while( !feof( fp ) )
		{
			fgets( szText, MAX_PATH, fp );
			if ( !feof( fp ) )
			{
				if ( szText[0] != '#' )
				{
					len = mystrlen( szText );
					szText[len-1] = 0;
				
					// attempt to add it regardless no matter what its contents
					AddLogToHistory( szText );
				}
			}
			szText[0] = 0;
		}
		fclose( fp );
		DeSelectLogHistory();
	}
	//ReadLogFileEndDate();		// Dont need it, i prooved it
}




BOOL AudioPlayResource(LPSTR lpName)
{
	BOOL bRtn;     
	LPSTR lpRes; 
	HRSRC hResInfo;
	HGLOBAL hRes;      // Find the WAVE resource.  

	hResInfo = FindResource(hInst, lpName, "WAVE");
	if (hResInfo == NULL) 
		return FALSE;      // Load the WAVE resource.  
	hRes = (LPSTR)LoadResource(hInst, hResInfo);
	if (hRes == NULL) 
		return FALSE;      // Lock the WAVE resource and play it.  
	lpRes = (LPSTR)LockResource(hRes);

	if (lpRes != NULL) { 
		bRtn = sndPlaySound(lpRes, SND_MEMORY | SND_ASYNC  | SND_NODEFAULT | SND_LOOP );
		UnlockResource(hRes);
	} else
		bRtn = 0;      // Free the WAVE resource and return success or failure. 
	FreeResource(hRes);
	return bRtn;
} 

void AudioStopPlay( void)
{
	sndPlaySound( NULL, SND_MEMORY | SND_ASYNC  | SND_NODEFAULT | SND_LOOP );
}







#include "gd.h"

#define	GRAD_LEVELS		32
static HBRUSH bgBrush[GRAD_LEVELS];
static long	  bgColor = -1;
// draw background rects to do a smooth gradient of any color
void DrawGradBackground( HDC hdc, RECT *rc, long color, long percent )
{
	if ( hdc && rc ){
			RECT dr;
			long gradLevel = GRAD_LEVELS, count;
			long y=0,ysize=rc->bottom;
			short docolor = FALSE;

			if ( bgColor == -1 ){
				docolor = TRUE;
				bgColor = color;
			}

			dr.left = 0;
			dr.right = rc->right;

			for( count=0; count<gradLevel; count++){
				long newcolor; double scale;

				if( docolor ){
					scale = ((count/(double)gradLevel)) * percent;
					newcolor = Scale_RGB( 100-scale, color );
					bgBrush[count] = CreateSolidBrush( newcolor );
				}
				dr.top = y+((count*ysize)/gradLevel);
				dr.bottom = y+(((1+count)*ysize)/gradLevel);
				FillRect( hdc, &dr, bgBrush[count] );
			}
	}
}

HBRUSH GradFromPos( HWND win, HWND ctl )
{
	RECT winrc, rc;
	long i=0;

	GetClientRect( win, &winrc );
	GetControlRect( win, ctl, &rc );

	i = (rc.top * GRAD_LEVELS) / winrc.bottom;
	return bgBrush[i];
}




void GradientBK( HWND hWnd, HDC hdc )
{
	RECT rc;

	if ( iDepth >= 15 && gUserGraduatedBackgrounds ){
		GetClientRect(hWnd, &rc);
		DrawGradBackground( hdc, &rc, GetSysColor(COLOR_3DFACE), 10 );
	}
}

LONG APIENTRY ForceTransparentClient( HWND hWnd, UINT msg, DWORD wParam, LONG lParam )
{
	if ( !gUserGraduatedBackgrounds )
		return 0;

    switch (msg) {
		case WM_ERASEBKGND:
			if ( iDepth >= 15 )
				return 1;
			break;
		case WM_CTLCOLORSTATIC:
			if ( iDepth >= 15 ){
				SetBkMode( (HDC)wParam, TRANSPARENT );
				return GradFromPos( hWnd, (HWND)lParam ) ? 1: 0;
			}
			break;
	}
	return 0;
}


LONG APIENTRY ForceBackgroundGradient( HWND hWnd, UINT msg, DWORD wParam, LONG lParam )
{
	if ( !gUserGraduatedBackgrounds )
		return 0;

    switch (msg) {
		case WM_ERASEBKGND:
			if ( iDepth >= 15 ){
				GradientBK( hWnd, (HDC)wParam );
				return 1;
			}
			break;
		case WM_CTLCOLORSTATIC:
			if ( iDepth >= 15 ){
				SetBkMode( (HDC)wParam, TRANSPARENT );
				return GradFromPos( hWnd, (HWND)lParam ) ? 1 : 0;
			}
			break;
	}
	return 0;
}














HANDLE 		ghProcessThread = NULL;


static long		last_level = 0;

long GetLastPercentProgress()
{
	return last_level/10;
}
extern long		GoProcessLogs( ProcessDataPtr logdata );
/*------------------------------------------------------------------------------------------
** Show progress routines
** Ver 4.0 : we now pass a level and not percent, level = [0..1000]
*/
void ShowProgressDetail( long level, long allTotalRequests, char *xtramsg, double starttime )
{
	short			progErr=0;
	long			percent = level/10;
	static char		txtmsg[256];
	static char		dbgmsg[256];
	char			statusLineText[256];
	static double	lastTimeFuncEntered = 0.0;
	double			thisTimeFuncEntered = 0.0;
	short			updateSecondsLeftProgress = 1;
	static short	lastDisplayWasInMinutesApprox = 0;
	static long		prevstarttime = 0;
	static long		lstarttime = 0;
	static long		staticeta = 0;

/* For debugging
	gDebugPrint = 1;
	lstarttime = starttime;
	if ( prevstarttime != lstarttime )
	{
		char errmsg[256];
		sprintf( errmsg, "Error: prevstarttime = %d, lstarttime = %d", prevstarttime, lstarttime );
		OutDebug( errmsg );
	}
	prevstarttime = starttime;*/

	// Check to see if we have entered this function within the last second, if so then we dont update the title bar
	thisTimeFuncEntered = timems();
	if ( thisTimeFuncEntered-1 < lastTimeFuncEntered && thisTimeFuncEntered > lastTimeFuncEntered )
		updateSecondsLeftProgress = 0;
	else
		lastTimeFuncEntered = thisTimeFuncEntered;

	// Don't do anything if the percent has gone below zero
	if (percent < 0)
	{	
		last_level = 0;
		return;
	}

	if ( level && starttime )
	{
		double eta=0, tt, timesofar;
		timesofar = timems() - starttime; 
		tt = timesofar*1000/level;
		eta = tt - timesofar;

		if ( eta )
		{
			char speedStr[32];
			if ( updateSecondsLeftProgress )
			{
				if ( eta < 60 )
				{
					lastDisplayWasInMinutesApprox = 0;
					sprintf( txtmsg, "      %2d sec Time remaining",	((int)eta)%60);
				}
				else if ( eta < 60*5 )
				{
					lastDisplayWasInMinutesApprox = 0;
					sprintf( txtmsg, "    %2d:%02d Time remaining",	((int)eta/60)%60, ((int)eta)%60);
				}
				else
				{
					lastDisplayWasInMinutesApprox++;
					if ( lastDisplayWasInMinutesApprox >= 10 ) // Display every 10 seconds...
					{
						lastDisplayWasInMinutesApprox = 0;
						staticeta = (long)eta;
					}
					if ( staticeta == 0.0 )
						staticeta = (long)eta;

					sprintf( txtmsg, "   approx. %2d mins remaining",	((int)staticeta/60)%60 );
				}

				if ( allTotalRequests )
				{
					sprintf( speedStr, "   (%d/sec)", (long)(allTotalRequests / timesofar) );
					strcat( txtmsg, speedStr );
				}
			
				ProgressChangeMainWindowTitle( percent, txtmsg );
/* For debugging
				sprintf( dbgmsg, "%s - eta = %f, timesofar = %f, level = %d, percent = %d, starttime = %d", txtmsg, eta, timesofar, level, percent, starttime );
				OutDebugs( dbgmsg );*/
			}
		}
	}

	statusLineText[0] = 0;
	//if ( (last_level!=level) )
	{
		if ( allTotalRequests )
		{
			FormatLongNum( allTotalRequests, statusLineText );
			mystrcat( statusLineText, " lines ..." );
			if ( xtramsg )
				mystrcat( statusLineText, xtramsg );
			StatusWindowSetProgress( level, statusLineText );
		}
		else
			StatusWindowSetProgress( level, xtramsg );
	}

	if ( gDebugPrint && gProcessing && allTotalRequests )
	{
		char text[2024];
		GetRealtimeSummary( NULL, text );
		ToolBarWriteLines( text, 25, 17 );
	}
	last_level = level;
}



// --------------------------------------------------------------------------------------------------



// General process cleanup stuff
void ProcessLogComplete( long stopped, ProcessData *logdata )
{
	DoPostProc( stopped );

	if ( !stopped )
	{
		char	statString[256];
		GetString( IDS_STATUST_CMPT, statString, 64 );		//report compelted msg
		StatusWindowSetProgress( 0, statString );
	} else
		StatusWindowSetProgress( 0, "Processing stopped..." );


	ChangeMainWindowTitle();

	if ( gProcessingSchedule )
	{
		LogScheduleTxt( "... Completed Process" , 0 );
		memcpy( &MyPrefStruct, &TempPrefStruct, CONFIG_SIZE );
		Sleep( 500 );
		gProcessingSchedule = FALSE;
	}

	gProcessing = FALSE;

	if ( !gNoGUI )
		ButtonBarPaint( ghMainDlg );
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// -------- MAIN PROCESS LOG ENTRY POINT
void ProcessLogNow( void )
{
	long		stopped = 0;
	ProcessData logdata;

#ifndef DEF_FREEVERSION
	if ( IsDemoTimedout() )
	{
		StatusSet( "Analyzer has expired, please register" );
		return;
	}
	if ( !IsDemoReg() && !IsFullReg() )
	{
		StatusSet( "Analyzer has not been registered, please register" );
		return;
	}
#endif

	gProcessing = TRUE;

	stopped = DoPreProc();

	if ( !stopped )
	{
		// Setup process structure
		logdata.prefs = &MyPrefStruct;
		logdata.ShowProgressDetail = ShowProgressDetail;
		logdata.reportType = REPORT_TYPE_NORMAL;	// you can use this to do http query modes or live reporting mode
		logdata.convert_Flag = CONVERT_NOTHING;

		if ( gConvertLogToW3C_Flag ) 
		{
			logdata.convert_Flag = CONVERT_TOW3C;
			gConvertLogToW3C_Flag = FALSE;
		}

		logdata.fsFile = glogFilenames;
		logdata.logNum = glogFilenamesNum;

		stopped = GoProcessLogs( &logdata );	//***************** DO THE PROCESS
		gProcessing = TRUE;
	}

	ProcessLogComplete( stopped, &logdata );

}
/////////////////////////////////////////////////////////////////////////////////////////////////



// Process LOG Thread Wrapper
DWORD WINAPI ProcessThreadFunc( LPVOID lpParam ) {

	ProcessLogNow();

	// View Report at the end if told to
	if ( (long)lpParam == 1 )
		SendMessage( hwndParent, WM_COMMAND, IDM_VIEWHTML, 0 );

	if ( ghStatusWnd )
		UpdateWindow( ghStatusWnd );

    return 0;
} 

// ---------------------------------------------------------------------------------------------------------


// Threaded
// High Level GO PROCESS function that triggers a thread which calls the process setup
// which then calls engine's PROCESSOR
short ProcessLog( long view_report )
{
    DWORD dwThreadId;
	long l_errno;

	// check valid outout location, if it doesnt exist
	if ( l_errno = ValidatePath( MyPrefStruct.outfile ) )
	{
		char	txt[512];
		// with no gui just make the dir.
		if ( gNoGUI || gProcessingSchedule ) 
		{
			char errtxt[512];
			GetLastErrorTxt( errtxt );
			sprintf( txt, ReturnString(IDS_ERR_OUTPUT), MyPrefStruct.outfile, errtxt );			/*  strerror(l_errno)  */
			DateFixFilename( txt,0 );
			StatusSet( txt );

			// do not attempt to make dirs if BEG/END are there as we dont know yet the log file times, report.cpp will do it
			if ( !strstr( MyPrefStruct.outfile, "_BEG}") && !strstr( MyPrefStruct.outfile, "_END}") )
			{
				PathFromFullPath( MyPrefStruct.outfile, txt );
				DateFixFilename( txt,0 );
				if ( MakeDir( txt ) ) 
					return 0;
			}
		} else
		// (QCM:46036) - do not report error for NO DIR or ask user to make dir, if these tokens exist since we dont know the times until reporting.
		if ( !strstr( MyPrefStruct.outfile, "_BEG}") && !strstr( MyPrefStruct.outfile, "_END}") )
		{
			// ask the user if he wants to make the dir.
			char errtxt[512];
			GetLastErrorTxt( errtxt );
			sprintf( txt, ReturnString(IDS_ERR_MAKEDIR), MyPrefStruct.outfile, errtxt );			/*  strerror(l_errno)  */
			DateFixFilename( txt,0 );
			if ( MessageBox( GetFocus(), txt, ReturnString(IDS_MAKEDIR), MB_YESNO|MB_ICONQUESTION ) == IDYES )
			{
				PathFromFullPath( MyPrefStruct.outfile, txt );
				DateFixFilename( txt,0 );
				if ( MakeDir( txt ) ) 
					return 0;
			} else
				return 0;
		}
	}

	
	{
		long	kbfree = (long)GetDiskFree( MyPrefStruct.outfile ) / 1024;
		if ( kbfree < 3500 && kbfree>=0 ){
			if( gNoGUI ){
				StatusSetID( IDS_LITTLEDISKSPACE, kbfree );
			} else {
				if ( MsgBox_yesno( IDS_LITTLEDISKSPACE, kbfree ) == IDNO )
					return 0;
			}
		}
	}

	if ( gNoThread )
	{// only filemaker applies the function
		StatusSetID( IDS_PROCESSING );
		ProcessThreadFunc( (LPVOID)view_report );
		return(1);
	}
	
	if ( ghProcessThread )
		CloseHandle( ghProcessThread );

	ghProcessThread = CreateThread( 
		NULL,                        // no security attributes 
		0,                           // use default stack size  
		ProcessThreadFunc,           // thread function 
		(LPVOID)view_report,		 // argument to thread function 
		0,                           // use default creation flags 
		&dwThreadId);                // returns the thread identifier  
    
	
	if (ghProcessThread) {
		SetThreadPriority( ghProcessThread, THREAD_PRIORITY_BELOW_NORMAL );
	} else {
		ErrorMsg( "cannot create processing thread!!!" );
		return 0;	// Check the return value for success.
	}

	return(1);
}

// Called by Wizard, which isnt used any more , but could be called by others soon like IDC_PROCESS:
void DoProcessLogQ( HWND hWnd, long viewreport )
{

#ifndef DEF_FREEVERSION
	if ( IsDemoTimedout() )
	{
		StatusSet( "Analyzer has expired, please register" );
		DoRegistrationAgain();
		return;
	}
	if ( !IsDemoReg() && !IsFullReg() )
	{
		StatusSet( "Analyzer has not been registered, please register" );
		DoRegistrationAgain();
		return;
	}
#endif

	if ( !gProcessing && !gProcessingSchedule )
	{
		if ( !ProcessLog( viewreport ) )
		{
			char errtxt[256];
			SysBeep(1);
			GetLastErrorTxt( errtxt );
			StatusSet( errtxt );		//IDS_FAILED
			gProcessing = FALSE;
		} else 
		{
			StatusSetID( IDS_PROCESSING );
			gProcessing = TRUE;
		}
	} else {
		StopProcessing();
	}
	
	UpdateWindow( ghMainDlg );
}



void StopProcessing( void )
{
	if ( gProcessing || gProcessingSchedule ){
		SysBeep(1);
		StopAll(1);
		StatusSetID( IDS_STOPPED );
	}
	UpdateWindow( hwndParent );
	ButtonBarPaint( ghMainDlg );
}



// --------------------------------------------------------------------------------------------------------------
// -		END OF MAIN CODE																					-
// --------------------------------------------------------------------------------------------------------------
