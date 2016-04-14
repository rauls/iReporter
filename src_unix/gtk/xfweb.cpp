/* example-start pixmap pixmap.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "gtk/gtkwin.h"
#include "gtk/xfweb.h"
#include "main.h"

#include "XSchedule.h"

#include "config.h"
#include "schedule.h"
#include "LogFileHistory.h"
#include "PrefsIO.h"
#include "serialReg.h"
#include "engine_process.h"
#include "EngineStatus.h"
#include "weburls.h"


#define	MAX_PATH 512
#define	LOGLIST_FILENAME	"loglist.dat"

// -------------------------------------------------------------------------------------------------------
static char		*appexe;
static int		guiOn = FALSE;


void ShowSplash( void );
void ShowAbout( void );
int InitGUI( void );



void SetWindowText( const char * content )
{
	if ( guiOn )
		SetWindowTitle( "IDD_XMAINTOOLBAR", (char*)content );

}

void UpdateTitleBar( void )
{
	char wt[1024];
	
	if ( gSaved )
		sprintf( wt, "iReporter - %s", FileFromPath(gPrefsFilename,0) );
	else
		sprintf( wt, "*iReporter - %s", FileFromPath(gPrefsFilename,0) );

	if ( guiOn )
		SetWindowTitle( "IDD_XMAINTOOLBAR", wt );
}



void SaveLogListAsText( void )
{
	FILE *fp;
	long row=0;
	char fullname[MAX_PATH];

	sprintf( fullname, "%s/.%s", getenv("HOME"), LOGLIST_FILENAME );
	if ( (fp = fopen( fullname, "w+" )) ){
		char texts[3][256];
		char *textarray[] = { texts[0], texts[1], texts[2], 0 };
		char *logname;
		int ret = 0;

		fprintf( fp, "#-- Log List\n" );
		while( row>=0 && ret>0 ){
			ret = ListView_GetItem( "IDC_LOGLIST", row, 0, textarray );
			if ( ret>0 && *texts[0] )
				fprintf( fp, "%s\n", texts[0] );
			row++;
		}
		fclose( fp );
	} else perror( "cant write\n");
}

void SetStatusBar( char *txt, long val )
{
	if ( guiOn ){
		int stat;
		if ( txt ) {
			SetLabelText( "IDC_XSTATUS", txt );
		}
		SetProgressValue( "IDC_XPROGRESS", val );
		WaitNextEvent();
	}
}


void XStatusWindowSetText( char *txt )
{
	if ( txt && guiOn ) {
		SetLabelText( "IDC_XSTATUS", txt );
	}
}




#define	MAINLIST_COLUMNS			4



long FindLogInGUIHistory( char *logtofind )
{
	if( logtofind && guiOn ){
		char texts[3][256];
		char *textarray[] = { texts[0], texts[1], texts[2], 0 };
		long row=0; char *logname=NULL;

		while( row>=0 && logtofind ){
			row = ListView_GetItem( "IDC_LOGLIST", row, 0, textarray );
			if ( !texts[0] )
				return -1;
			if ( !strcmp( texts[0], logtofind ) )
				return row;
		}
	}
	return -1;
}


long XAddLogToGUIHistory( long item )
{
	if ( guiOn ) {
		char *newlogfile;
		newlogfile = FindLogXinHistory( item );
OutDebugs( "find item %d = %s", item, newlogfile );

		if ( newlogfile ){
			int ret;
			if ( ret = ListView_AddOneItem( "IDC_LOGLIST", newlogfile ) )
				printf( "ListView_AddOneItem error (%s) %d\n", newlogfile, ret );
			return 0;
		}
	}
	return -1;
}


void ReDrawListView( )
{
	int item = 0;
	char *file;

	ListView_DeleteAll( "IDC_LOGLIST" );
	while( file=FindLogXinHistory( item ) )
	{
		ListView_AddOneItem( "IDC_LOGLIST", file );
		item++;
	}
}




void ReadLogListAsText( void )
{
	FILE *fp;
	char szText[MAX_PATH];
	long len;

	sprintf( szText, "%s/.%s", getenv("HOME"), LOGLIST_FILENAME );
	if ( !(fp=fopen( szText, "r" )) )
		fp=fopen( LOGLIST_FILENAME, "r" );

	if ( fp ){
		while( !feof( fp ) ){
			fgets( szText, MAX_PATH, fp );
			if ( !feof( fp ) ){
				if ( szText[0] != '#' ){
					len = mystrlen( szText );
					if( len>0 ){
						szText[len-1] = 0;
						AddLogToHistoryEx( szText, FALSE );
						ListView_AddOneItem( "IDC_LOGLIST", szText );
					}
				}
			}
		}
		fclose( fp );
	}
}

extern void ShowPrefsDialog( void *w, void *d);
extern void InitPrefsDialog( void );

void OnProcessLog( void *w, void *d )
{
	char *logItem;
	long row=0, num=0;
	
	if ( !gProcessing ){
		// add selected logs from list to the process list
		while( row>=0 ){
			row = ListView_IsItSelectedItem( "IDC_LOGLIST", row, 0, &logItem );
			//printf( "item=%d,%s\n", row,logItem );
			if ( row >= 0 )
				AddFileToLogQ( logItem, num++ );
		}
		//ListView_AddOneItem( "IDC_LOGLIST", "test1234" );
		// otherwise bring a filerequester box up!
		if( num == 0 ){
			char *file;
			file = GetFileName( "IDD_XMAINTOOLBAR", "Open log file to process..." );
			if ( file ) {
				OutDebugs( "file = %s", file );
				//add wild cards names
				if ( strchr( file, '*' ) ){
					AddWildCardFilenames( file, num );
					ListView_AddOneItem( "IDC_LOGLIST", file );
				} else
				//add 1 log name
				if ( file ) {
					AddLogToHistoryEx( file, FALSE );
					ListView_AddOneItem( "IDC_LOGLIST", file );
					num = AddFileToLogQ( file, num );
					//printf( "%s\n", file );
				}
			} else
				SetStatusBar( "File open Cancelled.", 0 );
		}
		if( num >0 ){
			gProcessing = 1;
			ProcessLog( 0 );

			SetStatusBar( "Ok.", 0 );

			gProcessing = 0;
		}
	} else {
		StopAll(1);
		gProcessing = 0;
		ErrorMsg( "Processing stopped!" );
	}
}


void ShowHTMLURL( char *url )
{
	long ret;
	char	cmd[256];
	
	sprintf( cmd, "netscape -remote 'openURL(%s)'", url );
	ret = system( cmd );
	if ( ret ){
		sprintf( cmd, "netscape %s &", url );
		ret = system( cmd );
	}
}


void ViewLocalFile( char *filepath )
{
	long ret;
	char	cmd[256];

	if ( GetFileLength( filepath )){ 
		char path[256];

		if ( strstr( filepath, ".pdf" ) ){
			sprintf( cmd, "xpdf %s 1> /dev/null 2>/dev/null", filepath );
			ret = system( cmd );

		} else
		if ( strstr( filepath, ".htm" ) ){
			if ( *filepath != '/' ){
				getcwd( path, 255 );
				sprintf( cmd, "netscape -remote 'openURL(file://%s/%s)'  1> /dev/null 2>/dev/null", path, filepath );
			} else
				sprintf( cmd, "netscape -remote 'openURL(file://%s)' 1> /dev/null 2>/dev/null", filepath );
				
			ret = system( cmd );
			if ( ret ){
				sprintf( cmd, "netscape %s & >/dev/null", filepath );
				ret = system( cmd );
			}
		}
	}
}

char *getwd(char *buf); 


void OnSchedule( void *w, void *d )
{
	ShowScheduleDialog(w,d);
}


void OnViewReport( void *w, void *d )
{
	ViewLocalFile( GetOutputFile() );
}


void OnViewHelp( void *w, void *d )
{
	ViewLocalFile( "manual/index.htm" );
}

void OnAbout( void *w, void *d )
{
	ShowAbout();
}

void OnViewMan( void *w, void *d )
{
	ShowHTMLURL( URL_MANUAL );
}

void OnViewQuestpage( void *w, void *d )
{
	ShowHTMLURL( URL_APP );
}


void OnViewHomepage( void *w, void *d )
{
	ShowHTMLURL( URL_APPLICATION );
}

void OnViewSupportpage( void *w, void *d )
{
	ShowHTMLURL( URL_SUPPORT );
}

void ExitGUI( void *w, void *d )
{
	gtk_main_quit ();
}





void SaveSettingsAs( void )
{
	char *file;
	
	if( file = GetFileName( "IDD_XMAINTOOLBAR", "Save Settings..." ) ){
		gSaved = TRUE;
		mystrcpy( gPrefsFilename, file );
		DoSavePrefsFile(gPrefsFilename,&MyPrefStruct);
	}
	UpdateTitleBar();
}

void OpenSettings( void )
{
	char *file;
	
	if ( file = GetFileName( "IDD_XMAINTOOLBAR", "Open Settings..." ) ){
		gSaved = TRUE;
		mystrcpy( gPrefsFilename, file );
		DoReadPrefsFile( gPrefsFilename, &MyPrefStruct );
	}
	UpdateTitleBar();
}

 

void HandleMenuEvents( long mid )
{
	//printf( "event=%08lx\n", mid );
	switch( mid ){
		case 0x101 : if ( !gProcessing ){
						gSaved=FALSE;
						CreateDefaults( &MyPrefStruct, 1);
						mystrcpy( gPrefsFilename, "unknown.fwp" );
					 }
					 break;
		case 0x102 : OpenSettings(); break;
		//---------
		case 0x104 : DoSavePrefsFile(gPrefsFilename,&MyPrefStruct);break;
		case 0x105 : SaveSettingsAs(); break;
		//---------
		case 0x107 : OnProcessLog( NULL,NULL ); break;
		//---------
		case 0x109 : ExitGUI( NULL,NULL ); break;


		case 0x201 : ShowPrefsDialog( NULL,NULL ); break;
		case 0x202 : OnSchedule( NULL,NULL ); break;

		case 0x301 : OnViewReport( NULL,NULL ); break;
		case 0x302 : ViewLocalFile( "select_log_file" ); break;
		case 0x303 : ViewLocalFile( "debug_log.txt" ); break;
		//---------
		case 0x305 : ClearLogList();InitGUI(); break;
		case 0x306 : ErrorMsg( "Remove log not functional" ); break;
		case 0x307 : ErrorMsg( "Gzip log not functional" ); break;
		case 0x308 : ErrorMsg( "Bzip2 log not functional" ); break;
		case 0x309 : ErrorMsg( "Add log not functional" ); break;
		case 0x30A : ErrorMsg( "SaveList not functional" ); break;
		case 0x30B : ErrorMsg( "LoadList not functional" ); break;
		
		//---------
		case 0x401 : {
						char calltxt[256];
						sprintf( calltxt, "%s&", appexe );
						system( calltxt );
					 }
					break;

		//---------
		case 0x501 : OnViewHelp( NULL,NULL ); break;
		case 0x502 : OnViewMan(NULL,NULL); break;
		//---------
		case 0x504 : OnViewQuestpage( NULL, NULL ); break;
		case 0x505 : OnViewHomepage( NULL, NULL ); break;
		case 0x506 : OnViewSupportpage( NULL, NULL ); break;
		//---------
		case 0x508 : ShowAbout(); break;
	}
}

static char *reg_name = NULL,
			  *reg_comp = NULL,
			  *reg_serial = NULL;
void XRegisterDone( void *w, void *d )
{
	GetText( "IDC_REGNAME", reg_name, 200 );
	GetText( "IDC_REGORG", reg_comp, 200 );
	GetText( "IDC_REGSERIAL", reg_serial, 32 );
	CloseWindow( "SERIALREG" );
	gtk_main_quit ();
}
void XRegisterDemo( void *w, void *d )
{
	char rettxt[256];

	GetText( "IDC_REGNAME", rettxt, 200 ); mystrcpy( reg_name, rettxt );
	GetText( "IDC_REGORG", rettxt, 200 ); mystrcpy( reg_comp, rettxt );

	if ( reg_serial ) mystrcpy( reg_serial, "DEMO" );
	if ( !mystrlen(reg_comp) ) mystrcpy( reg_comp, "DEMO" );
	if ( !mystrlen(reg_name) ) mystrcpy( reg_name, "DEMO" );

	CloseWindow( "SERIALREG" );
	gtk_main_quit ();
}


void XRegisterCancel( void *w, void *d )
{
	CloseWindow( "SERIALREG" );
	gtk_main_quit ();
	reg_name = 0;
}

long XAskForRegistration( char *name, char *comp, char *serial )
{
	reg_name = name;
	reg_comp = comp;
	reg_serial = serial;

	OutDebug( "asking for registration" );

	ShowWindow( "SERIALREG" );
	AttachProcToButtonName( "SERIALREG", "IDC_REGOK", (void*)XRegisterDone,0 );
	AttachProcToButtonName( "SERIALREG", "IDC_REGDEMO", (void*)XRegisterDemo,0 );
	AttachProcToButtonName( "SERIALREG", "IDC_CANCEL", (void*)XRegisterCancel,0 );

	gtk_main();

	if ( reg_name )
		return 1;
	else
		return 0;
}




char *toolbarButtons[][4] = {
	{ "IDC_PREFS", 		"buttonSettings", "buttonSettingsOn", "Settings" },
	{ "IDC_SCHED",		"buttonSched", "buttonSchedOn", "Schedule" },
	{ "IDC_PROCESS",	"buttonProcess", "buttonProcessOn", "Process" },
	{ "IDC_VIEW",		"buttonView", "buttonViewOn", "View" },
	{ "IDC_APPHELP",	"buttonHelp", "buttonHelpOn", "Help" },
	{ "IDC_APPLOGO",	"buttonApp", "buttonAppOn", "-" },
	//{ "IDC_SHOWLIST",	"buttonList", "buttonListOn", "-" },
	{ 0,0,0,0 }
};

long buttonSizes[][3] = {
	15,	15, 64 ,
	108,15, 64 ,
	194,15, 64 ,
	284,15, 64 ,
	366,15, 64 ,
	453,15, 64 ,
	1,	80, 18 ,
	0, 0, 0
};

void ButtonRollOverEnter( void *w, void *d, void *unit )
{
	long i=(long)unit;
	HideWidget( toolbarButtons[i][1] );
	ShowWidget( toolbarButtons[i][2] );
}
void ButtonRollOverLeave( void *w, void *d, void *unit  )
{
	long i=(long)unit;
	HideWidget( toolbarButtons[i][2] );
	ShowWidget( toolbarButtons[i][1] );
}


long	toolbar_width = 543;
long	toolbar_mainheight = 168;
long	toolbar_listviewtop = 102;
long	buttons_coord[8][4];
long	toolbar_infopos = 12;
long	toolbar_service_xpos = 400;
char	toolbar_service_str[64];

long	toolbar_infocolor = 0xffffff;
long	toolbar_statuscolor = 0xffffff;//0x00ffff;
long	toolbar_newskin = 0;
long	toolbar_statusxpos = 650;


int CreateToolbarButtons( void )
{
	long i, ret;
	int hDlg;
	long txtx[]= {13,68,125,186,242,453,0};
	
	i = 0;
	//for(i=0;i<7;i++){
	while( toolbarButtons[i][0] ) {
		long sz = buttonSizes[i][2];
		AddPixmapControlFromPixmap( "bigtoolbar8" , toolbarButtons[i][1], buttonSizes[i][0], buttonSizes[i][1], sz, sz );
		AddPixmapControlFromPixmap( "bigtoolbar8b", toolbarButtons[i][2], buttonSizes[i][0], buttonSizes[i][1], sz, sz );
		
		ret = AddImagesToButton( toolbarButtons[i][1], toolbarButtons[i][2], toolbarButtons[i][0] );
		if ( ret )	printf( "AddImagesToButton failed %d\n", ret );

		ret = AttachProcToControl( "IDD_XMAINTOOLBAR", toolbarButtons[i][0],  (void*)ButtonRollOverEnter, "enter-notify-event", (void*)i );
		if ( ret )	printf( "AttachProcToControl failed %d\n", ret );

		ret = AttachProcToControl( "IDD_XMAINTOOLBAR", toolbarButtons[i][0],  (void*)ButtonRollOverLeave, "leave-notify-event", (void*)i );
		if ( ret )	printf( "AttachProcToControl failed %d\n", ret );
		
		//WriteXY( "bigtoolbar8", txtx[i]+6, y+42+10, toolbarButtons[i][3] );
		i++;
	}
	if ( IsDemoReg() )
		WriteXY( "bigtoolbar8", 22, toolbar_infopos, "Demo licence only" );

	HideWidget( "IDC_SHOWLIST" );

	return ret;
}


#include "images/bigtoolbar8.xpm"
#include "images/bigtoolbar8b.xpm"

int InitMainToolbar( void )
{
	int ret;

	if ( ret = ShowWindow( "IDD_XMAINTOOLBAR" ) ){
		printf( "failed - %d\n" , ret );
		return ret;
	} else {
		WindowPreventResize( "IDD_XMAINTOOLBAR" );
		
		ret = AddMenuToWindow( "APPUNIX", "IDD_XMAINTOOLBAR", (void*)HandleMenuEvents );
		if ( ret )	printf( "AddMenuToWindow failed %d\n", ret );

		ret = AddPixmapControl( "IDD_XMAINTOOLBAR", "bigtoolbar8b", (char*)bigtoolbar8b_xpm );
		if ( ret )	printf( "AddPixmapControl failed %d\n", ret );

		ret = AddPixmapControl( "IDD_XMAINTOOLBAR", "bigtoolbar8", (char*)bigtoolbar8_xpm );
		if ( ret )	printf( "AddPixmapControl failed %d\n", ret );

		ret = AttachPixmapControlToWindow( "IDD_XMAINTOOLBAR","bigtoolbar8" );
		if ( ret )	printf( "AttachPixmapControlToWindow failed %d\n", ret );

		ret = CreateToolbarButtons();
		
		AttachProcToButtonName( "IDD_XMAINTOOLBAR", "IDC_PREFS", (void*)ShowPrefsDialog,0 );
		AttachProcToButtonName( "IDD_XMAINTOOLBAR", "IDC_SCHED", (void*)OnSchedule,0 );
		AttachProcToButtonName( "IDD_XMAINTOOLBAR", "IDC_PROCESS", (void*)OnProcessLog,0 );
		AttachProcToButtonName( "IDD_XMAINTOOLBAR", "IDC_VIEW", (void*)OnViewReport,0 );
		AttachProcToButtonName( "IDD_XMAINTOOLBAR", "IDC_APPHELP", (void*)OnViewHelp,0 );
		AttachProcToButtonName( "IDD_XMAINTOOLBAR", "IDC_APPLOGO", (void*)OnAbout,0 );
		
		ReadLogListAsText();

		UpdateTitleBar();
		return ret;
	}
}

#include "images/fwebpro8.xpm"

static int splashTimer;
static int runs=0;

void SplashTimer( void *d )
{
	static int count=0;
	
	if( count > 10 ){
		KillWidgetTimer( splashTimer );
		CloseWindow( "ABOUT" );
		if ( runs == 0 ){
			if ( InitMainToolbar() )
			{
				printf( "InitMainToolbar failed\n" );
				//return;
			}
		}
		SetStatusBar( "Ok.", 0 );
		runs++;
		count = 0;
	}
	count++;
}



static char scrollMsg[] =
{
"Version : 4.1\n"
"Registered to :\n [NAME]\n\n\n"
"Built ("__DATE__" "__TIME__")\n"
"(c) 1997-2002 Software\n"
" \n"
"Funnel Web is a Trademark of\n"
"Software\n\n\n"
"Unix Programming\n\n"
"Raul Sobon\n"
"\0"
};


static char about_txt[] = { 
"(C) 1997 - 2002 Software (C)\n"
"All Rights Reserved. " };


void WriteLines( char *src, long x, long y )
{
	char *txt;
	char line[256];

	txt = line;
	while( *src ){
		*txt++ = *src++;
		if ( *src == '\n' ){
			*txt++ = 0;
			if ( strstr( line, "[NAME]" ) ){
				if ( GetRegisteredUsername() )	ReplaceStr( line, NULL, "[NAME]", GetRegisteredUsername(), 0 );
			}
			WriteXY( "fwebpro8", x, y,line );
			y+=10;
			txt = line;
		}
	}
}


void ShowAbout( void )
{
	ShowWindow( "ABOUT" );
	SetWindowTitle( "ABOUT", "About iReporter" );
	
	if ( runs == 0 ){
		WindowPreventResize( "ABOUT" );

		AddPixmapControl( "ABOUT", "aboutlogo", (char*)fwebpro8_xpm );
		AttachPixmapControlToWindow( "ABOUT","aboutlogo" );

		WriteLines( scrollMsg, 280, 110 );

		WriteLines( about_txt, 20, 220 );
	}
	//splashTimer = SetWidgetTimer( 200, "IDC_ABOUTIMAGE", SplashTimer );
}

void ShowSplash( void )
{
	ShowWindow( "ABOUT" );
	SetWindowTitle( "ABOUT", "About iReporter" );
	
	if ( runs == 0 ){
		WindowPreventResize( "ABOUT" );

		AddPixmapControl( "ABOUT", "aboutlogo", (char*)fwebpro8_xpm );
		AttachPixmapControlToWindow( "ABOUT","aboutlogo" );

		WriteLines( scrollMsg, 280, 120 );
	}
	splashTimer = SetWidgetTimer( 200, "IDC_ABOUTIMAGE", (void*)SplashTimer );
}




int InitGUI( void )
{
	char *loglistStr[] = { "Log file", 0 };
	
	if ( ReplaceListView( "IDC_LOGLIST", (char**)loglistStr, 1, 0, GTK_SELECTION_EXTENDED ) ){
		SetWindowExitProc( "IDD_XMAINTOOLBAR", (void*)ExitGUI );
		return 1;
	} else {
		printf( "cant replacelistview\n" );
	}
	return 0;
}


void SaveDef( void )
{
	char *home = getenv( "HOME" );
	char file[255];
	FILE *fp;
	
	sprintf( file, "%s/.fweb.rc\n\0", home );
	if (fp=fopen( file, "r") ){
		fprintf( fp, "prefsfile %s\n", gPrefsFilename );
		fclose( fp );
	}
	
}
void LoadDef( void )
{
	char *home = getenv( "HOME" );
	char file[255];
	
	sprintf( file, "%s/.fweb.rc\n\0", home );
	DoReadPrefsFile( file, &MyPrefStruct );
	
	InitReadSchedule(0);
}



extern unsigned char fweb_data[];



int BeginGUI( int argc, char *argv[] )
{
	char *display = getenv( "DISPLAY" ), *altrc = NULL;
	long rereg=0, nosplash=0, debugrc = 0, i = 1, scalex=150, scaley=150;
	
	mystrcpy( gPrefsFilename, "unknown.fwp" );
	LoadDef();
	

	while( i<argc){
		if ( !strcmpd( "-register", argv[i] ) )
			rereg = 1;
		else
		if ( !strcmpd( "-nosplash", argv[i] ) )
			nosplash = 1;
		else
		if ( !strcmpd( "-debugrc2", argv[i] ) )
		{
			printf( "Now debugging resource......\n" );
			debugrc = 2;
		} else
		if ( !strcmpd( "-debugrc", argv[i] ) )
		{
			printf( "Now debugging resource...\n" );
			debugrc = 1;
		} else
		if ( !strcmpd( "-rc", argv[i] ) )
			altrc = argv[i+1];
		else
		if ( !strcmpd( "-scalex", argv[i] ) )
			scalex = myatoi( argv[i+1] );
		else
		if ( !strcmpd( "-scaley", argv[i] ) )
			scaley = myatoi( argv[i+1] );
		i++;
	}		
	
	if ( display ){
		appexe = argv[0];
		
	//	g_set_error_handler(&do_null);
	//	g_set_warning_handler(&do_null);
	//	g_set_message_handler(&do_null);

		SetGlobalScale( scalex, scaley );

		OutDebug("gtk_init");
		gtk_init( &argc, &argv );
		guiOn = TRUE;
		
		OutDebug("SetDefaultFont");
		if ( !SetDefaultFont( "helvetica", 100 ) )
			SetDefaultFont( NULL, 100 );
			

		if ( altrc ) {
			printf( "Loading alternative resource %s ...\n", altrc );
			OutDebug("ParseWindowsRC");
			ParseWindowsRC( altrc, "DEF_FULLVERSION", debugrc );
		} else {
			OutDebug("ParseWindowsRC");
			ParseWindowsRC( (char*)fweb_data, "DEF_FULLVERSION", debugrc );
		}
		OutDebug("ParseWindowsRC done");

		if ( InitGUI() )
		{
			int success;
			OutDebug("InitGUI done");

			if ( rereg )
				success = DoRegistrationAgain();
			else
				success = DoRegistrationProcedure();

			if( success )
			{
				OutDebug("CheckRegistration done");
				if ( nosplash ){
					OutDebug( "Splash Screen disabled" );
					InitMainToolbar();
				} else {
					ShowSplash();
					OutDebug("ShowSplash done");
				}
				gtk_main ();

				SaveLogListAsText();
				DoSavePrefsFile(gPrefsFilename,&MyPrefStruct);
			} else {
				return 3;
			}
		} else {
			perror( "Failed to initiazlie GUI\n" );
			return 2;
		}
	} else
		return 1;

	return 0;		// FAIL boohoo
} 
 

/*

	if ( argc>1 ){
		if ( !ShowWindow( argv[1] ) )
			printf( "\n%s not found\n", argv[1] );
	} else
		ShowAllWindows();
*/
