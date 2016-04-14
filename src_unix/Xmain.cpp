/*
	File:		main.c

	Contains:	unix shell CLI version layer

	Written by:	Raul Sobon

	Copyright:	1996,97 by Active Concepts, all rights reserved.

	PLATFORMS:	*.*

	Change History (most recent first):
		15/Aug/1997 :	Unix Port of engine working... will improve args opts
						then later do GUI that uses the engine binary.
	To Do:
				More options???
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>

#if ( __FreeBSD__ && !__MACOSX__ )
#include <floatingpoint.h>
#endif

/* Local Header Files */
#include "main.h"
#include "util.h"

#include "gtk/gtkwin.h"
#include "gtk/xapplication.h"

#include "remotecontrol.h"
#include "serialReg.h"
#include "unixreg.h"
#include "LogFileHistory.h"
#include "version.h"
#include "OSInfo.h"


//extern "C" int MessageBox( char *txt, char *title, int );
//extern "C" int ListView_AddOneItem( char *name, char * );
//extern "C" int BeginGUI( int argc, char *argv[] );

void StatusSet( char * txt )
{
	XStatusWindowSetText( txt );

	if ( gRemoteControlled )
		RemoteStatusSetText( txt );
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

void StatusWindowSetProgress( long perc, char *txt )
{
	OutDebugs( "msg='%s'", txt );
	StatusSet( txt );
	SetStatusBar( txt, perc );
}

void ProgressChangeMainWindowTitle( long percent, char *etaStr )
{
	char	txt[512], fname[64];
	int		ret;
	
	if ( gNoGUI==FALSE ) {
		FileFromPath( gPrefsFilename, fname );

		if ( gSaved )
			sprintf( txt, "%d%% - %s %s", percent, fname, etaStr );
		else
			sprintf( txt, "%d%% - *%s %s", percent, fname, etaStr );
			
		SetWindowText( txt );
	}
}


extern int	processing_http_request;
extern void HttpOutputError( const char* errorString );

int UnixErrorMsgEx( const char *txt, int type )
{
	long ret=0;
	if ( txt ) 
	{
		OutDebug( txt );
		if ( mystrlen( txt ) )
		{
			if( processing_http_request )
			{
				HttpOutputError(txt);
			} else {
				if ( !gNoGUI && !gRemoteControlled && !gProcessingSchedule ){
					switch( type ){
						case 0: ret = MessageBox ( (char*)txt, "Notify", 2); break;
						case 1: ret = MessageBox ( (char*)txt, "Caution", 2); break;
						case 2: ret = MessageBox ( (char*)txt, "Warning!", 1 ); break;
					}
				}
			}
		}
	}
	return ret;
}                             


long UnixNotifyMsg( const char *txt )
{
	return UnixErrorMsgEx( txt, 0 );
}                             


long UnixCautionMsg( const char *txt )
{
	return UnixErrorMsgEx( txt, 1 );
}                             

long UnixErrorMsg( const char *txt )
{
	return UnixErrorMsgEx( txt, 2 );
}                             





// ----------------



long AddLogToGUIHistory( char *szText, long item )
{
	ListView_AddOneItem( "IDC_LOGLIST", szText );
}


// Add A log to the internal history and also to the GUI, but if its already
// there in the GUI, just highlight it.

long AddLogToHistory( char *log )
{
	long total;

	total = AddLogToHistoryEx( log, false );

	if ( total >0 ){
		AddLogToGUIHistory( log, total );
	} //else	FindLogInHistoryAndSelect( log );

	return total;
}












static char about_txt[] = { 
"(C) 1997 - 2002 Software (C)\n"
"All Rights Reserved. Software.\0"
};

// each pair per line
static char *aboutinfo_txt[] = { 
"Licensed to",				"[NAME]                                         ",
"              ",			"[ORG]                                          ",
"              ",			"[DAYS]                                         ",
"              ",			" ",
"Product information",		"[VER]                                          ",
"              ",			"[BUILD]                                        ",
"              ",			" ",
"Contact information",		"Phone    +1-800-red-flag",
"              ",			"Email    info@redflagsoftware.com",
"              ",			"WWW      www.redflagsoftware.com",
"              ",			" ",
#ifndef	DEF_FREEVERSION
"Support    ",				"Email    support@redflagsoftware.com",
"              ",			" ",
#endif
0,0

};

int ResolveAboutVariables( char *txt )
{
	char *ptr, *p=0;
	unsigned long len=0, bold;

	if ( p=strstr( txt, "[NAME]" ) ) {
		ptr = GetRegisteredUsername();
		if ( !ptr ){
			len = strlen(p);
			len = GetUserName( p, &len );
		} else {
			sprintf( p, ptr );
		}
	} else
	if ( p=strstr( txt, "[ORG]" ) ) {
		ptr = GetRegisteredOrganization();
		if ( !ptr ) {
			len = strlen(p);
			len = GetComputerName( p, &len );
		} else {
			sprintf( p, ptr );
		}
	} else
	if ( p=strstr( txt, "[VER]" ) )
		sprintf( p, "Version %s (%d clusters)", VERSION_STRING, GetClusterValue() );
	else
	if ( p=strstr( txt, "[BUILD]" ) )
	{
		char bdate[128];
		GetAppBuildDate( bdate );
		sprintf( p, "Release build %d (%s)", DaysSince01012001ForBuildDate(), bdate );
	} else
	if ( p=strstr( txt, "[DAYS]" ) )
	{
		if ( GetDaysLeft() > 1000 || IsFullReg() )
		{
			sprintf( p, "Fully Registered" );
		} else 
		{
			if ( GetDaysLeft() <= 0 )
				sprintf( p, "Trial Expired" );
			else {
				char tStr[32];
				GetExpireDate( tStr );
#if (DEF_FREEVERSIONDYNAMIC)
				sprintf( p, "Expires on %s (%.1f Days left)", tStr, GetDaysLeft() );
#else
				sprintf( p, "Fixed Expiry on %s (%.1f Days left)", tStr, GetDaysLeft() );
#endif
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


void PlotAboutText( void )
{
	long i = 0, ycoord = 145;

	// Do each line.
	while( aboutinfo_txt[i] )
	{
		char txt[256], bold;
		char *leftStr = aboutinfo_txt[i++];
		char *rightStr = aboutinfo_txt[i++];

		strcpy( txt, rightStr );

		bold = ResolveAboutVariables( txt );

		printf( "%-40s %s\n", leftStr, txt );
		ycoord += 15;
	}
	printf( "%s\n", about_txt );
}


void ShowVersionInfo( void )
{

	PlotAboutText();
	return;

	char tmp[MAXFILENAMESSIZE];
	GetAppBuildDetails( tmp );

	printf( "%s  %s\n", tmp, GetOSType() );
	if ( IsDemoReg() || !IsFullReg() )
	{
		char tmp[256];
		GetExpireDate( tmp );
#if (DEF_FREEVERSIONDYNAMIC)
		printf( "\nFixed Expiry at %s (%.2f days left)\n", tmp, GetDaysLeft() );
#else
		printf( "\nExpires at %s (%.2f days left)\n", tmp, GetDaysLeft() );
#endif
	} else {
		printf( "\nFully Registered to <%s>, <%s>, %d clusters\n", GetRegisteredUsername(), GetRegisteredOrganization(), GetClusterValue() );
	}
}



//return TRUE if hit OK else FALSE if hit CANCEL
short Fetch_SerialInfo( SerialInfoPtr SerialDataP )
{
	OutDebug( "Fetch_SerialInfo" );
	if ( SerialDataP ){
		SerialDataP->startTime = time(0);
		return XAskForRegistration( &SerialDataP->userName[1],&SerialDataP->organization[1], &SerialDataP->serialNumber[1] );
	} else
		return 0;
} 

time_t	gBinaryDate;


int main( int argc, char **argv)
{
	int			count=1, skip;
	int			setDest = 0, success=0;
	char		*destFilename;

	gBinaryDate = GetFileDate( argv[0] );

#if ( __FreeBSD__ && !__MACOSX__ )
	fpsetmask(0L);
#endif
	if ( argc <2 ){
		fprintf ( stdout, "try %s -h\n", argv[0] );
	}


	gVerbose = 2;

	char *home = getenv( "APPHOME" );
	if ( home )
	{
		int l;
		sprintf( gPath, home );
		l = strlen( gPath );
		if ( gPath[l-1] != '/' )
			strcat( gPath, "/" );
	} else {
#if _SUNOS
		sprintf( gPath, "/opt" );
#else
		sprintf( gPath, "/usr/local" );
#endif


#if DEF_FREEVERSION
		strcat( gPath, "/AnalyzerFree/" );
#else
		strcat( gPath, "/Analyzer/" );
#endif

	}

	InitApp();
	success = DoRegistrationProcedure();
	SetupArgs( argc, argv );


	OutDebugs( "Funnel Web Home Dir is '%s'", gPath );
	if ( gDebugPrint ){
		OutDebug( "Debug state on\n");
	}


	/* Handle the init for App */
	if ( 1 ){
		if ( setDest )
			strcpy( MyPrefStruct.outfile, destFilename );   /* change dest path if specified */
		if ( !strcmpd( "-daemon", argv[1] ) || !strcmpd( "-server", argv[1] ) ){
			OutDebug( "Daemon mode...");
			DoDaemonMode();
		} else {
			gNoGUI = 1;

			if ( strcmpd( "-nogui" , argv[1] ) ){
				gNoGUI = 0;
				BeginGUI( argc, argv );
			} else
			if ( success=DoRegistrationProcedure() ){
				DoProcessLogQ( 0 );		/* process the log file... */
			}
		}
	} else {
		OutDebug( "Init failed");
	}

	ExitApp();	/* cleanup and exit now */
}


