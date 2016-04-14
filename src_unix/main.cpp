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

#include "remotecontrol.h"
#include "serialReg.h"
#include "unixreg.h"
#include "GlobalPaths.h"
#include "version.h"
#include "OSInfo.h"

void ShowFile( char *file )
{
	return;
}



/* show prefences dialog
 */
void Preferences(void)
{
	return;	
}


void ChangeMainWindowTitle( void )
{
	return;	
}

static char blankline[] = "                                                                               \r";



void StatusSet( char * txt )
{

	if ( gVerbose > 1)
	{
		static int lastout = 0;
		OutDebug( txt );

		for(int i=0; i<lastout; i++) fputc( ' ', stdout );
		fputc( '\r', stdout );

		fputs( txt, stdout );
		fputc( '\r', stdout );
        fflush( stdout );

		lastout = strlen(txt);
	} else
		fputc( '\n', stdout );

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
	if ( perc == 0 )
		StatusSet( "\n" );

	if ( txt )
		StatusSet( txt );
}



void ProgressChangeMainWindowTitle(  long perc, char *txt )
{

	StatusWindowSetProgress( perc, txt );
}


int UnixErrorMsgEx( const char *txt, long type )
{
	if ( txt ) 
	{
		if ( strlen( txt ) )
		{
			printf( txt );
			OutDebug( txt );
		}
	}
	return 0;
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


long AddLogToGUIHistory( char *log, long item )
{
	;
}

static char about_txt[] = { 
"(C) 2001 - 2004 Software (C)\n"
"All Rights Reserved. Software\0"
};

// each pair per line
static char *aboutinfo_txt[] = { 
"Licensed to",				"[NAME]                                         ",
"              ",			"[ORG]                                          ",
"              ",			" ",
"Product information",		"[VER]                                          ",
"              ",			" ",
"Contact information",		"Phone    +1-800-red-flag",
"              ",			"Email    info@redflagsoftware.com",
"              ",			"WWW      www.redflagsoftware.com",
#ifndef	DEF_FREEVERSION
"              ",			" ",
"Support    ",				"Email    support@redflagsoftware.com",
"           ",				"WWW      support.redflagsoftware.com",
"           ",				" ",
#else
"           ",				"WWW      www.redflagsoftware.com",
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
	if ( SerialDataP )
	{
#if DEF_FREEVERSION
		sprintf( &SerialDataP->userName[1], "NA" );
		sprintf( &SerialDataP->organization[1], "NA" );
		sprintf( &SerialDataP->serialNumber[1], "DEMO" );
#else
		printf( "\n\nEnter 'DEMO' for serial for a 15 day trial period.\n\n " );

		printf( "\nEnter name: " );
		fscanf( stdin, "%s", &SerialDataP->userName[1] );
		
		printf( "Enter company: " );
		fscanf( stdin, "%s", &SerialDataP->organization[1] );

		printf( "Enter serialNumber: " );
		fscanf( stdin, "%s", &SerialDataP->serialNumber[1] );
#endif
		
		SerialDataP->userName[0] = strlen( SerialDataP->userName+1 );
		SerialDataP->organization[0] = strlen( SerialDataP->organization+1 );
		SerialDataP->serialNumber[0] = strlen( SerialDataP->serialNumber+1 );
		if ( SerialDataP->startTime == 0 )
		{
			SerialDataP->startTime = time(0);
		}
	}// else printf( "XXX\n");
	return 1;
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
#elif __MACOSX__
		sprintf( gPath, "/Applications" );
#else
		sprintf( gPath, "/usr/local" );
#endif

#if __MACOSX__
	#if DEF_FREEVERSION
			strcat( gPath, "/FW Analyzer Free 4.5/" );
	#else
			strcat( gPath, "/FW Analyzer Enterprise 4.5/" );
	#endif
#else
	#if DEF_FREEVERSION
			strcat( gPath, "/AnalyzerFree/" );
	#else
			strcat( gPath, "/Analyzer/" );
	#endif
#endif

	}

	InitApp();
	success = DoRegistrationProcedure();
	SetupArgs( argc, argv );

	{
		char filename[256];
		sprintf( filename, "%s/FWASettings.txt", gPath );
		if ( !GetFileLength( filename ) )
			ErrorMsg( "\nCould not find the global settings file '%s'\nYou may need to set your IRHOME environment variable\nto the correct path where App is installed\n\n", filename );

	}

	OutDebugs( "Application Home Dir is '%s'", gPath );
	if ( gDebugPrint ){
		OutDebug( "Debug state on\n");
	}

	/* Handle the init for App */
	if ( success )
	{
		if ( setDest )
			strcpy( MyPrefStruct.outfile, destFilename );   /* change dest path if specified */

		if ( !strcmpd( "-daemon", argv[1] ) || !strcmpd( "-server", argv[1] ) )
		{
			OutDebug( "Daemon mode...\n");
			DoDaemonMode();
		} else {
			gNoGUI = 1;
			DoProcessLogQ( 0 );		/* process the log file... */
		}
	} else 
	{
		OutDebug( "Init failed\n");
	}

	printf( "\n" );

	ExitApp();	/* cleanup and exit now */

	printf( "\n" );
}




