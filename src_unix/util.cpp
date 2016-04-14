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
#include <string.h>
#include <math.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/time.h>

#if ( __FreeBSD__ && !__MACOSX__ )
#include <floatingpoint.h>
#endif

#include <errno.h>


#ifdef USE_THREADS
#include <pthread.h>
#include <time.h>
#endif


/* Local Header Files */
#include "util.h"
#include "main.h"
#include "editpath.h"
#include "main.h"
#include "remotecontrol.h"
#include "serialReg.h"
#include "unixreg.h"

#include "Utilities.h"
#include "httpinterface.h"
#include "schedule.h"
#include "datetime.h"
#include "log_io.h"
#include "config.h"
#include "postproc.h"
#include "version.h"
#include "weburls.h"
#include "engine_process.h"
#include "engineregion.h"
#include "EngineStatus.h"
#include "DateFixFilename.h"
#include "SettingsAppWide.h"
#include "ResDefs.h"


extern long GetSleeptypeSecs( long type );

struct		App_config BackupPrefStruct;

int			gNoGUI=0;
int			gProcessLog;		
int			gProcessing = 0;
int			gProcessingSchedule = 0;
int			gRemoteControlled = 0;
int			gSaved = 0;
int			gHttpInterface = FALSE;			// true if controlling via http:8000
short		gHttpPort = 800;
int			gConvertLogToW3C_Flag = 0;


int			runID=1;

char		logFilename[256] = DEFAULT_LOG_NAME;
char		prefFilename[256] = DEFAULT_PREF_NAME;

long		gDNRTTL = 120;


char		dns_server[256] = "\0";
int			dns_timeout = 0;
int			dns_retries = 0;



/*
 * ------------------->>>>>   funnel web code begins here
 */

void *CreateThread( void *ptr, long stack, int func(long), void *data, long laddr, unsigned long *id )
{
#ifdef USE_THREADS
static	pthread_t         a_thread;
static	pthread_attr_t    a_thread_attribute;
	pthread_create( &a_thread, &a_thread_attribute, (void*)func, (void*)data );
	return (void*)a_thread;
#endif
}

long GetShortCut( char *linkpath, char *newpath )
{
	return TRUE;
}


void SetThreadPriority( void *ptr, long pri )
{
	return;
}

void TerminateThread( void *ptr, long pri )
{
	return;
}

void *ExitThread( long ret )
{
	return (void*)ret;
}


void *CloseHandle( void *h )
{
	return h;
}



#ifdef _IPC_MESSAGES_

/* Init IPC messaging
 */
int InitMsg( )
{
	int id;
	key_t	key;
	
	/* key = ftok( "/etc/hosts", 1 );
	 */
	id = msgget( key , IPC_CREAT);
	if ( id <0) fprintf( stderr, "err in msgget() %d,key=%d\n", id,key );
	return id;
}


struct msgdata {
	long mtype;    /* message type */
	char mtext[128];
};

/* send a message to listening GUI client...
 */
int SendMesg( char *msg )
{
static	long	sendcount=0;
static	int		id;
	int len,err;
	struct msgdata ipcdata;
	
	if ( sendcount == 0 )  err = id = InitMsg();
	if ( id > 0 ) {
		len = 128;
		ipcdata.mtype = 1;
		strcpy( ipcdata.mtext, msg );
		err = msgsnd( id, (void*)&ipcdata, len, IPC_NOWAIT );
		/* fprintf( stderr, "msgsnd( id=%ld ) = %d\n", id,err );
		 */
		sendcount++;
	}

	return err;
}

#endif





/*
////////////////////////////////////////////////////////////////////////////
//
// Preferences handling routines
//
////////////////////////////////////////////////////////////////////////////
*/




#include <stdarg.h>

int StatusSetID( int id, ... )
{
	long count, len, ret;
	va_list		args;
	char lineout[4000], txt[512];
	if ( id ){
		strcpy( txt, ReturnString( id ) );
		if ( txt ){
			va_start( args, id );
			vsprintf( lineout, txt, args );
			StatusSet( lineout );
			va_end( args );
		}
	}
	return 1;
}

void StatusWindowSetText( char *txt )
{
	StatusSet( txt );
}

int MsgBox_Error( long id, ... )
{
	long count, len, ret;
	va_list		args;
	char lineout[4000], txt[500];

	if ( id )
	{
		strcpy( txt, ReturnString( id ) );
		if ( txt )
		{
			va_start( args, id );
			vsprintf( lineout, txt, args );
			UserMsg( lineout );
			va_end( args );
		}
	}
	return 1;
}



char *GetOutputFile( void )
{
	return MyPrefStruct.outfile;
}



int ValidateOutput( char *outpath )
{
	FILE *fp; int ret=1;
	
	if( outpath ){
		if ( fp=fopen( outpath, "a+" ) ){
			ret=0;
			fclose( fp );
		}
	}
	return ret;
}


 
 
 



static long		last_request = 0, last_percent = 0;
long GetLastPercentProgress( void )
{
	return last_percent/10;
}




double gettimems( void )
{
	long	time1[2];
	double  usec1;
	
	gettimeofday( (struct timeval *)&time1, NULL );

 	usec1 = ((double)time1[0]) + ((double)time1[1]/1000000.0);
	return usec1;
}


/*------------------------------------------------------------------------------------------
** Show progress routines
** Ver 4.0 : we now pass a level and not percent, level = [0..1000]
*/
void ShowProgressDetail( long level, long allTotalRequests, char *xtramsg, double start )
{
	double			timesofar, eta, tot, now = 0;
	long			percent = level/10, x;
	char			txtmsg[256];
	
	if (percent < 0) {	
		last_percent = 0;
		last_request = 0;
	} else {
		char speedStr[256];

		speedStr[0]=0;
		txtmsg[0]=0;

		if ( level && start ) {
			int64_t tsfms, etams, totms;

			now = gettimems();
			timesofar = now - start; 
			tot = (1000*timesofar)/level;
			eta = tot - timesofar;
			//printf ( "st=%.1f, now=%.1f, tsf=%.1f/%d eta=%.1f tot=%.1f\n", start, now,timesofar,level, eta, timesofar/level );


			tsfms = (1000*timesofar);
			totms = (1000*tsfms)/level;
			etams = totms - tsfms;
			eta = etams/1000;

			//printf ( "st=-%f, tsf=%f/%d eta=%f tot=%f\n", start, timesofar,level, eta, timesofar/level );
			if ( eta ){

				if ( eta < 60 )
					sprintf( txtmsg, "      %2d sec remaining",	((int)eta)%60);
				else
					sprintf( txtmsg, "    %2d:%02d Time remaining",	((int)eta/60)%60, ((int)eta)%60);

				if ( allTotalRequests ){
					sprintf( speedStr, "   (%d/sec)", (long)(allTotalRequests / timesofar) );
					strcat( txtmsg, speedStr );
				}
			}

		
		}

		{
			char	message[320];

			if ( allTotalRequests ){
				FormatLongNum( allTotalRequests, txtmsg );
				mystrcat( txtmsg, " lines ..." );
			}
			if ( xtramsg ) mystrcat( txtmsg, xtramsg );
			sprintf(message, "%d%%, %s %s", percent, txtmsg, speedStr );

			StatusWindowSetProgress( percent, message );
		}

		last_percent = percent;
	}
}





void ShowDNSProgress( long level, long forceShow, char *msg, double start )
{
	double			timesofar, tt, tot, now = 0;
	long			percent = level/10, x;
	

	if ( msg )
	{
		char	message[320];

		mystrcpy( message, msg );

		if ( start )
		{
			char	txtmsg[256];

			tt = timems() - start;
			if ( tt < 60 )
				sprintf( txtmsg, "    %02d sec Taken",	((int)tt)%60);
			else
				sprintf( txtmsg, " %2d:%02d Taken",	((int)tt/60)%60, ((int)tt)%60);
			strcat( message, txtmsg );
		}

		StatusWindowSetProgress( percent, message );
	}

	last_percent = percent;
}


// General process cleanup stuff
void ProcessLogComplete( long stopped, ProcessData *logdata )
{
	DoPostProc( stopped );

	if ( !stopped )
	{
		char	statString[256];
		strcpy( statString, ReturnString( IDS_STATUST_CMPT ) );
		StatusWindowSetProgress( 0, statString );
	} else
		StatusWindowSetProgress( 0, "Processing stopped..." );


	if ( gProcessingSchedule )
	{
		LogScheduleTxt( "... Completed Process" , 0 );
		memcpy( &MyPrefStruct, &TempPrefStruct, CONFIG_SIZE );
		Sleep( 500 );
		gProcessingSchedule = FALSE;
	}

	gProcessing = FALSE;
}




void ProcessLogNow( void )
{
	long stopped;
	ProcessData logdata;

	gProcessing = TRUE;

	stopped = DoPreProc();

	if ( !stopped )
	{
		logdata.fsFile = glogFilenames;
		logdata.logNum = glogFilenamesNum;
		logdata.prefs = &MyPrefStruct;
		logdata.ShowProgressDetail = ShowProgressDetail;
		logdata.reportType = REPORT_TYPE_NORMAL;
		if ( gConvertLogToW3C_Flag ) 
		{
			logdata.convert_Flag = CONVERT_TOW3C;
			gConvertLogToW3C_Flag = FALSE;
		} else
			logdata.convert_Flag = 0;

		logdata.dns_server = dns_server;
		logdata.dns_timeout = dns_timeout;
		logdata.dns_retries = dns_retries;

		stopped = GoProcessLogs( &logdata );
		gProcessing = TRUE;
	}

	ProcessLogComplete( stopped, &logdata );

	if ( !stopped )
		StatusWindowSetProgress( 0, "Report Completed." );
	else
		StatusWindowSetProgress( stopped, "Report Stopped." );

	if ( gProcessingSchedule )
	{
		LogScheduleTxt( "... Completed Process" , 0 );
		memcpy( &MyPrefStruct, &TempPrefStruct, CONFIG_SIZE );
		usleep( 500 );
		gProcessingSchedule = 0;
	}

	gProcessing = 0;
}

#define	MAKEDIR(dir)	mkdir( dir, 0x3c0 )


short ProcessLog( long remote )
{
	if ( ValidatePath( MyPrefStruct.outfile ) )
	{
		char szText[255], errtxt[340];
		int retcode, l;
		l = PathFromFullPath( MyPrefStruct.outfile, szText );
		if ( szText[l-1] == '/' )
			szText[l-1] = 0;
		retcode = MakeDir( szText );
		sprintf( errtxt, "Making dir '%s'\n", szText );
		StatusSet( errtxt );
		
		if ( ValidatePath( MyPrefStruct.outfile ) )
		{
			sprintf( szText, "Output path/file\n%s\ncannot be written!\n", MyPrefStruct.outfile );
			ErrorMsg( szText );
			return 0;
		}
	}
	
	if ( glogFilenamesNum>0 ) 
	{
		if ( gVerbose > 0)
		{
			int lp;
			char tmp[512];

			sprintf( tmp, "Report output is '%s'\n", MyPrefStruct.outfile );
			StatusSet( tmp );

			for (lp=0; lp<glogFilenamesNum; lp++ )
			{
		 		sprintf( tmp, "Processing Log %d. '%s'\n", lp+1, glogFilenames[lp] );
				StatusSet( tmp );
			}
		}
		gRemoteControlled = remote;

		ProcessLogNow();

		gRemoteControlled = 0;
		return(1);
	} else {
		StatusSet( "There are no logs to be processed...\n" );
		return(0);
	}
}


void CompressLogs_Gzip( void )
{
	if ( glogFilenamesNum>0 ) 
	{
		{
			int lp;
			char tmp[512];
			for (lp=0; lp<glogFilenamesNum; lp++ )
			{
				CompressLogFiles( &glogFilenames[lp], 1, COMPRESS_GZIP, TRUE );
			}
		}
		return(1);
	} else {
		StatusSet( "There are no logs to be converted...\n" );
		return(0);
	}
}


void CompressLogs_Bzip2( void )
{
	if ( glogFilenamesNum>0 ) 
	{
		{
			int lp;
			char tmp[512];
			for (lp=0; lp<glogFilenamesNum; lp++ )
			{
				CompressLogFiles( &glogFilenames[lp], 1, COMPRESS_BZIP2, TRUE );
			}
		}
		return(1);
	} else {
		StatusSet( "There are no logs to be converted...\n" );
		return(0);
	}
}


#include <errno.h>
long GetLastErrorTxt( char * errtxt )
{
	strcpy( errtxt, strerror( errno ) );
	return errno;
}

void StopProcessing( void )
{
	if ( gProcessing || gProcessingSchedule ){
		SysBeep(1);
		StopAll(1);
	}
}

// Called by Wizard, which isnt used any more , but could be called by others soon like IDC_PROCESS:
void DoProcessLogQ( long viewreport )
{

#ifdef DEF_FREEVERSION
	if ( IsDemoTimedout() )
	{
		StatusSet( "Free Analyzer has expired!!" );
		ShowMsg_FreeVersionExpired();
		return;
	}
#else
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
		} else {
			//StatusSetID( IDS_PROCESSING );
			gProcessing = TRUE;
		}
	} else {
		StopProcessing();
	}
}



 




#include <dirent.h>
/*---------------------------------------------------------------
 * Function: GetLclDir()
 *
 * Description: Get the local file directory and write to
 *   temporary file for later display.
 */
long AddWildCardFilenames( char *szTempFile , long start )
{
	DIR *dp;
	struct dirent *ep;
	char	newPath[512], firstFile[512], pattern[64];
	int 	fFound=0,len, lastn;
	int nNext = start;

	DateFixFilename( szTempFile, firstFile );

	FileFromPath( firstFile, pattern );
	
	PathFromFullPath( firstFile, NULL );

	if ( !firstFile[0] )
		sprintf( firstFile, "./" );

	dp = opendir ( firstFile );
	if (dp != NULL){

		while (ep = readdir (dp)){
			FileFromPath( ep->d_name, newPath );
			if ( match( newPath,pattern, 1 ) ){
				sprintf( newPath, "%s/%s", firstFile, ep->d_name );
				//mystrcpy( newPath, ep->d_name );

				if( !IsPrefsFile( newPath ) ){
					nNext = AddFileToLogQ( newPath, nNext );
					if ( lastn == nNext ){
						fFound = 0;
						continue;
					}
				}
			}
		}
		(void) closedir (dp);
	} else
		printf ("Couldn't open the directory (%s)\n", firstFile);

	return nNext;
}


// yeah yeah its here, only for testing for now, itll move L8r!!
#include "postproc.h"


/*---------------------------------------------------------------
  Add all files in X directory to the internal file report list, thats
  used to decide which files get uploaded
 */
long AddWildCardReportFiles( char *szTempFile )
{
	DIR *dp;
	struct dirent *ep;
	char	newPath[512], firstFile[512], pattern[64];
	int 	fFound=0,len, lastn;
	int		nFilesDone;

	DateFixFilename( szTempFile, firstFile );

	FileFromPath( firstFile, pattern );
	
	PathFromFullPath( firstFile, NULL );

	if ( !firstFile[0] )
		sprintf( firstFile, "./" );

	dp = opendir ( firstFile );
	if (dp != NULL){

		StopFopenHistory();
		StartFopenHistory();

		while (ep = readdir (dp)){
			FileFromPath( ep->d_name, newPath );
			if ( match( newPath,pattern, 1 ) ){
				sprintf( newPath, "%s/%s", firstFile, ep->d_name );
				//mystrcpy( newPath, ep->d_name );

				if( !IsPrefsFile( newPath ) )
				{
					AddFileHistory( newPath );
					nFilesDone++;
				}
			}
		}
		(void) closedir (dp);
	} else
		printf ("Couldn't open the directory (%s)\n", firstFile);

	return nFilesDone;
}

void SetupArgs( int argc, char **argv )
{
	int			count=1, skip;
	for ( count=1 ; count < argc;  ){
		// Custom args for unix only
		if ( !strcmpd( "-dns_server", argv[count] ) ){
			strcpy( dns_server, argv[count+1] );
			*argv[count] = '#';
		}
		if ( !strcmpd( "-dns_timeout", argv[count] ) ){
			dns_timeout = atoi(argv[count+1]);
			*argv[count] = '#';
		}
		if ( !strcmpd( "-dns_retries", argv[count] ) ){
			dns_retries = atoi(argv[count+1]);
			*argv[count] = '#';
		}
		if ( !strcmpd( "-http", argv[count] ) ){
			gHttpInterface = 1;
			gHttpPort = atoi(argv[count+1]);
			*argv[count] = '#';
		}
		if ( !strcmpd( "-nohttp", argv[count] ) ){
			gHttpInterface = 0;
			*argv[count] = '#';
		}
		if ( !strcmpd( "-enablecron", argv[count] ) ){
			EnableCron( 1 );
			*argv[count] = '#';
		}
		if ( !strcmpd( "-convert2gzip", argv[count] ) ){
			CompressLogs_Gzip();
			printf( "\n" );
			*argv[count] = '#';
			exit(0);
		}
		if ( !strcmpd( "-convert2bzip", argv[count] ) ){
			CompressLogs_Bzip2();
			printf( "\n" );
			*argv[count] = '#';
			exit(0);
		}

		skip = ProcessPrefsLine( argc, argv, 0, count, &MyPrefStruct );
		count += skip;
	}

}


static const char s_DefaultSettingsFileName[]="Untitled.fwp";

long InitApp(void)
{	
	std::string fileName( gPath );
	fileName += CQSettings::SETTINGS_APP_WIDE_DEFAULT;

	CQSettings::TheSettings().OpenAppWideSettings( fileName.c_str() );    
	stripxchar( gPath, '\\' );

	CreateDefaults(&MyPrefStruct,1);
	strcpy( gPrefsFilename, s_DefaultSettingsFileName );

	if ( gHttpInterface )
		InitHttpServer( gHttpPort );

	initLookupGrid();

	//printf( "Init Defaults from %s\n", fileName.c_str() );
	//OutDebug( "Init Scheduler ...." );
	//InitReadSchedule( 0 );
	//OutDebug( "Init Scheduler Done" );

	return 1;
}


long InitAppOld(void)
{	
	long success=0;


	/*-------------        Check registration ID here */

	initLookupGrid();

	CreateDefaults( &MyPrefStruct, 1);

	if ( gHttpInterface )
		InitHttpServer( gHttpPort );

	return success=1;
}



void ExitApp(void)
{	
	StatusSet( "Done." );
}


long CheckReportFreeSpace( void )
{
	return 0;

	long kbfree;
	kbfree = GetDiskFree( MyPrefStruct.outfile ) / 1024;
	if ( kbfree < 200 && kbfree>= 0 )
		return 1;
	else
		return 0;
}

#include <unistd.h>    


void EnableCron( long status )
{
	long	ret, schOn=0;
	char	cmd[256], szText[1024];
	FILE	*fp, *np;
	
	sprintf( cmd, "/usr/bin/crontab -l >/tmp/t.list" );
	ret = system( cmd );
	np=fopen( "/tmp/t2.list", "w" );
	fp=fopen( "/tmp/t.list", "r" );
	
	if ( fp && np ){
		while( !feof(fp) ){
			fgets( szText, 1024, fp );
			if ( feof(fp) ) break;
			if ( strstr( szText, "-checkschedule" ) ){
				schOn = 1;
				if ( !status ) continue;
			}
			fprintf( np, szText );
		}
		fclose( fp );
		
		if ( !schOn && status ){
			//sprintf( szText, "0,5,10,15,20,25,30,35,40,45,50,55 * * * * %s -nogui -q -checkschedule 1>/dev/null 2>/dev/null\n", "xapplication" );
			sprintf( szText, "*/2 * * * * %s -nogui -q -checkschedule\n", "xapplication" );
			fputs( szText, np );
			fclose( np );
			sprintf( cmd, "/usr/bin/crontab /tmp/t2.list" );
			ret = system( cmd );
		} else
			fclose( np );
		unlink( "/tmp/t.list" );
		unlink( "/tmp/t2.list" );
	}
}

// Determin the status of argv[1]s flag setting
// if the command is , "nodns" or "dns false" or "dns 0" or "dns 1"
// NOTE: not called at present, failure factor is 0.00%
long GetFlagSetting( char *command, char *arg )
{
	long flag = 1;

	if ( !strcmpd( "no", command ) )
		flag = 0;

	if ( arg )
	{	// -------- ON MODE
		if ( !strcmpd( "on", arg ) )
			flag = 1;
		else
		if ( !strcmpd( "yes", arg ) )
			flag = 1;
		else
		if ( !strcmpd( "1", arg ) )
			flag = 1;
		else
		if ( !strcmpd( "true", arg ) )
			flag = 1;
		else
		// -------- OFF MODE
		if ( !strcmpd( "off", arg ) )
			flag = 0;
		else
		if ( !strcmpd( "no", arg ) )
			flag = 0;
		else
		if ( !strcmpd( "0", arg ) )
			flag = 0;
		else
		if ( !strcmpd( "false", arg ) )
			flag = 0;
	}
	
	return flag;
}


long PopulateArgsFromLine( char *line, char **argv, int &argc )
{
	char *p;

	p = line;
	// --- construct the argc/argv variables, but take quotes into account
	// Fixes CR-4088, and probably good for many more in future.
	argv[ argc++ ] = p;
	while( *p && p && argc<255 )
	{
		if( *p == ' ' )
		{
			*p++ = 0;
			argv[ argc ] = p;

			if( *p == 34 ) // find QUOTE "
			{
				p++;
				argv[ argc ] = p;
				if( p = strchr( p, 34 ) )
					*p = 0;
			}
			argc++;
		}
		p++;
	}
	return argc;
}


// 15 day trial expires
void ShowMsg_TrialOver( void )
{
#ifdef DEF_FREEVERSION
	const char *szText = "Sorry , your free 90 day trial period has completed.\n\nTo purchase , go to\n http://www.redflagsoftware.com/shop ?";
#elif DEF_FULLVERSION
	const char *szText = "Sorry , your free 15 day trial period has completed.\n\nTo purchase , go to\n http://www.redflagsoftware.com/shop ?";
#else
	const char *szText = "Sorry , your free 15 day trial period has completed.\n\nTo purchase , go to\n http://www.redflagsoftware.com/shop ?";
#endif
	if( gNoGUI ) return;

	// It asks you if you want to go to the website because NO ONE will try to re-type the URL
	if ( ErrorMsg( szText, "iReporter Expired" ) )
		;//ShowHTMLShortcut( GetFocus(), URL_APPLICATION, NULL );

	StatusSet( "\n" );
}

// Free version OVER
void ShowMsg_FreeVersionExpired( void )
{
	const char *szText = "iReporter Free edition has expired.\n\nTo purchase iReporter , go to\n buy.redflagsoftware.com ?";
	if( gNoGUI ) return;

	// asks i fyou want to go to the website too, coz A) you cant cut/paste the URL, B) no one will re-type it in , its just stupid and unprofessional, this is quicker to BUY IT
	if ( ErrorMsg( szText, "iReporter Expired" ) )
		;//ShowHTMLShortcut( GetFocus(), URL_STORE, NULL );

	StatusSet( "\n" );
}


void ShowMsg_ReEnterSerial( char *badSerialStr )
{
	char szText[256];
	if( gNoGUI ) return;
	sprintf( szText, "Serial Reg code <%s> is incorrect\n", badSerialStr );
	ErrorMsg( szText, "iReporter Registration" );

	StatusSet( "\n" );
}
