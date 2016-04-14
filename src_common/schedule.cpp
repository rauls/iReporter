/* some changes....

   22.Jun	:	fixed the debug logs to not log the passwords inside an ftp URL.
 *
 *
 */

#include "FWA.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "log_io.h"

#ifdef DEF_MAC
	#include "MacUtil.h"
	#include "Processing.h"
#endif

#if DEF_WINDOWS
#include <windows.h>
//#include "NTServApp.h"
#include "Winmain.h"
#include "Winutil.h"
#include "winnet_io.h"
#endif

#if DEF_UNIX
#include "unistd.h"
#include "errno.h"
#include "main.h"
#include "unixnet_io.h"
#endif

#include "myansi.h"
#include "schedule.h"
#include "datetime.h"
#include "OSInfo.h"
#include "version.h"

#include "config_struct.h"
#include "config.h"		// Would be nice to not need this in future.
#include "FileTypes.h"
#include "GlobalPaths.h"
#include "DateFixFileName.h"

extern int IsServiceInstalled( long flag );

long CalcNextSchedule( long jd, long repeatCount, long repeatType )
{
	long jdate;
	char incdate[12] = "00/00/00", inctime[12] = "00:00";

	switch( repeatType ){
		case 0 : sprintf( inctime, "00:%02d", repeatCount ); break;
		case 1 : sprintf( inctime, "%02d:00", repeatCount ); break;
		case 2 : sprintf( incdate, "%02d/00/00", repeatCount ); break;
		case 3 : sprintf( incdate, "00/%02d/00", repeatCount ); break;
		case 4 : sprintf( incdate, "00/00/%02d", repeatCount ); break;
	}

	jdate = CalcNextDays( jd, incdate, inctime );
	return jdate;
}


long CalcIncTime( long jd, long repeatCount, long repeatType )
{
	long jdate;
	char incdate[12] = "00/00/00", inctime[12] = "00:00";

	switch( repeatType ){
		case 0 : sprintf( inctime, "00:%02d", repeatCount ); break;
		case 1 : sprintf( inctime, "%02d:00", repeatCount ); break;
		case 2 : sprintf( incdate, "%02d/00/00", repeatCount ); break;
		case 3 : sprintf( incdate, "00/%02d/00", repeatCount ); break;
		case 4 : sprintf( incdate, "00/00/%02d", repeatCount ); break;
	}

	jdate = CalcNextDays( jd, incdate, inctime );
	return jdate-jd;
}

// mins,hrs,days,months,years
long ComputeNextSchedule( ScheduleRecPtr schP )
{
	time_t		currentTime = GetCurrentCTime(), inctime, nexttime;
	struct tm	date;
	long		iYY=0,iMM=0, repeatCount = schP->repeatCount, units;

	//currentTime *= ONEDAY;
	nexttime = schP->startdate;

	if ( repeatCount <1 ) repeatCount = 1;
	units = 1;

	if ( currentTime > nexttime ){
		switch( schP->repeatType ){
			case 0: inctime = ONEMINUTE*repeatCount;
					if ( inctime ){
						units = (((currentTime - nexttime)/inctime)+1);
						nexttime += (units*inctime);
						return schP->nextRun = nexttime;
					}
					break;
			case 1: inctime = ONEHOUR*repeatCount;
					if ( inctime ){
						units = (((currentTime - nexttime)/inctime)+1);
						nexttime += (units*inctime);
						return schP->nextRun = nexttime;
					}
					break;
			case 2: inctime = ONEDAY*repeatCount;
					if ( inctime ){
						units = (((currentTime - nexttime)/inctime)+1);
						nexttime += (units*inctime);
						return schP->nextRun = nexttime;
					}
					break;
			case 3 : iMM = repeatCount; break;
			case 4 : iYY = repeatCount; break;
		}
	} else
		return schP->nextRun = nexttime;

	currentTime = GetCurrentCTime();
	nexttime = schP->startdate;
	DaysDateToStruct( nexttime, &date );

	while( currentTime > nexttime ){
		date.tm_year += iYY;
		date.tm_mon += iMM;
		while( date.tm_mon > 11 ){
			date.tm_mon -= 12;
			date.tm_year++;
		}
		date.tm_isdst = 0;
		Date2Days( &date, &nexttime );
		//nexttime += (Time2JD( &date,0 ));
	}
	schP->nextRun = nexttime;
	return schP->nextRun;
}

/* ----------------------------------------------------------

	scheduling

*------------------------------------------------------------*/

ScheduleRecPtr	gSchedules = NULL;


void ActivateAllSchedules( long value )
{
	ScheduleRecPtr	schP = gSchedules;

	while( schP ){
		schP->active = value;
		schP = schP->next;
	}
}

long GetTotalSchedule( void )
{
	ScheduleRecPtr schP;
	long	index=0;


	schP = gSchedules;

	if ( schP ){
		while( schP ){
			schP = schP->next;
			index++;
		}
	}
	return index;
}

ScheduleRecPtr AddSchedule( void )
{
	ScheduleRecPtr schP, nextP;
	struct tm nowtime;
	long	index=1;

	schP = gSchedules;

	if ( schP ){
		while( schP ){
			nextP = schP->next;
			index++;

			if ( ! nextP ) {
				nextP = schP->next = (ScheduleRecPtr)malloc( SCHEDULESIZE );
				memset( nextP, 0, SCHEDULESIZE );
				schP = 0;
			} else
				schP = nextP;
		}
	} else {
		nextP = gSchedules = (ScheduleRecPtr)malloc( SCHEDULESIZE );
	}

	nextP->active = 0;
	nextP->runonce = 0;

	GetCurrentDate( &nowtime );
	Date2Days( &nowtime, &nextP->startdate );

	nextP->startdate += (Time2CTime( &nowtime,0 ));			// only do thison NON-MACOSs

	nextP->repeattime = 0;
	nextP->repeatCount = 1;
	nextP->repeatType = 2;
	//ComputeNextSchedule( schP );
	//nextP->startdate = = CalcNextSchedule( nextP->startdate, nextP->repeatCount, nextP->repeatType );

	nextP->nextRun = 0;
	strcpy( nextP->title, "Schedule" );
	nextP->logfile[0] = 0;
	strcpy( nextP->prefsfile, "Untitled.conf" );
	strcpy( nextP->upload, "\0" );
	nextP->index = index;
	nextP->next = 0;
	
	
	return nextP;
}



ScheduleRecPtr GetScheduleX( int n )
{
	ScheduleRecPtr schP, schlast=0, schnext;
	int index=1;

	schP = gSchedules;

	while( schP ){
		schnext = schP->next;
		if ( n ){
			if ( n == index ){
				return schP;
			}
		}
		schP = schnext;
		index++;
	}
	return NULL;
}


void CopySchedule( ScheduleRecPtr newP, int from )
{
	ScheduleRecPtr otherP = GetScheduleX( from );

	if ( newP && otherP ){
		char *p;
		newP->active = otherP->active;
		newP->startdate = otherP->startdate;
		newP->repeattime = otherP->repeattime;
		newP->repeatCount = otherP->repeatCount;
		newP->repeatType = otherP->repeatType;
		newP->startdate = otherP->startdate;
		newP->nextRun = otherP->nextRun;
		strcpy( newP->title, otherP->title );
		strcpy( newP->logfile, otherP->logfile );
		if ( p=strstr( otherP->logfile, "W3SVC" ) ){
			long i; char *d;
			p+=5;
			i = myatoi(p);
			i++;
			d = newP->logfile + (p - otherP->logfile);
			while ( myisdigit(*p) ) p++;
			sprintf( d, "%d%s", i, p );
		}
		strcpy( newP->prefsfile, otherP->prefsfile );
		strcpy( newP->upload, otherP->upload );
	}
}


void DelSchedule( int n )
{
	ScheduleRecPtr schP, schlast=0, schnext;
	int index=1;

	schP = gSchedules;

	if ( schP ){
		while( schP ){
			schnext = schP->next;
			if ( n ){
				if ( n == index ){
					free( schP );
					if ( schlast )
						schlast->next = schnext;
					else
						gSchedules = schnext;
				} else
					schlast = schP;
			} else
				free( schP );
			index++;
			schP = schnext;
		}
		if( !n ) gSchedules = NULL;
	}
}

ScheduleRecPtr GetSchedule( int n )
{
	if ( gSchedules ){
		int index=1;
		ScheduleRecPtr schP=0, schlast=0, schnext=0;

		schP = gSchedules;
		while( schP ){
			schnext = schP->next;
			if ( n ){
				if ( n == index ){
					return schP;
				}
			}
			index++;
			schP = schnext;
		}
	}
	return NULL;
}


void ClearSchedule( ScheduleRecPtr schP )
{
	if ( !schP ) schP = gSchedules;

	while( schP ){
		schP->nextRun = 0;
		schP = schP->next;
	}
}

void LogScheduleTxt( const char *txt, long err )
{
	char	szText[MAXFILENAMESSIZE];

#ifdef DEF_MAC
	char	logString[MAXFILENAMESSIZE];

	sprintf( szText, "%s:%s", gPath, SCHEDULE_LOGFILENAME );
	strcpy(logString,szText);
	strcat(logString,txt);
	WriteLog(logString);
#elif DEF_UNIX
	char *home = getenv( "HOME" );
	sprintf( szText, "%s/.%s", home, SCHEDULE_LOGFILENAME );
	LogMessage( szText, (char*)txt, err );
#else
	sprintf( szText, "%s\\%s", gPath, SCHEDULE_LOGFILENAME );
	LogMessage( szText, (char*)txt, err );
	OutDebug( txt );
#endif
}

static 	char *numTypeStr[]= { "mins", "hrs", "days", "months", "years",0,0,0,0 };


void LogSchedule( ScheduleRecPtr schP )
{
	char msg[MAXLOGLENGTH+500], tmp[MAXLOGLENGTH], tmp2[64];
	
	strcpyuntil( tmp, schP->logfile, ',' );
	MakeSafeURL( tmp );
	DateTimeToString( schP->nextRun, tmp2 );
	if ( schP->runonce )
		sprintf( msg, "#%d. Running once (%s)", schP->index, tmp );
	else
		sprintf( msg, "#%d. Doing Schedule (%s), next=%s every %d %s", schP->index, tmp, tmp2, schP->repeatCount, numTypeStr[schP->repeatType] );
	LogScheduleTxt( msg, 0 );

	
	DateFixFilename( MyPrefStruct.outfile, tmp );
	if ( ValidatePath( tmp ) ){
		sprintf( msg, "#%d. Output path not found (%s)", schP->index, MyPrefStruct.outfile );
		LogScheduleTxt( msg, 0 );
		sprintf( msg, "#%d. Output path mapped to (%s)", schP->index, tmp );
		LogScheduleTxt( msg, 0 );

		PathFromFullPath( tmp, 0 );
		sprintf( msg, "Making dir (%s)", tmp );
		MakeDir( tmp );
		LogScheduleTxt( msg, 0 );
	} else {
		sprintf( msg, "#%d. Output path (%s)", schP->index, tmp );
		LogScheduleTxt( msg, 0 );
	}

	if ( strlen(schP->upload)>2 ){
		sprintf( msg, "#%d. Upload to (%s)", schP->index, schP->upload );
		LogScheduleTxt( msg, 0 );
	}
}

void LogProcessList( void )
{
	char msg[MAXFILENAMESSIZE];
	long lp;
	
	for(lp=0;lp<glogFilenamesNum;lp++){
		char tmp[MAXLOGLENGTH];

		sprintf( tmp, glogFilenames[lp] );
		MakeSafeURL( tmp );
		sprintf( msg, "Log to process - %d.'%s'", lp, tmp );
		LogScheduleTxt( msg, 0 );
	}
}

void LogFailedWildcard( char *log )
{
	char msg[MAXFILENAMESSIZE];

	sprintf( msg, "No files matched wildcard '%s'", log );
	LogScheduleTxt( msg , 0 );
}

//OutDebug( "Process args..." );

extern int				gDebugPrint;

void DebugLogSchedule( ScheduleRecPtr schP, long now )
{
	if ( gDebugPrint && schP )
	{
		char msg[MAXLOGLENGTH+500], tmp[MAXLOGLENGTH];

		DateTimeToString( schP->startdate, tmp );
		sprintf( msg, "Now = %s (%d,%d)", tmp, now, GetCurrentCTime() );
		OutDebug( msg );

		sprintf( tmp, schP->logfile );
		MakeSafeURL( tmp );

		if ( schP->runonce )
			sprintf( msg, "Running once : #%d. log (%s) at ", schP->index, tmp );
		else
			sprintf( msg, "Schedule checking : #%d. log (%s) at ", schP->index, tmp );
		DateTimeToString( schP->startdate, tmp );
		strcat( msg, tmp );
		OutDebug( msg );

		DateTimeToString( schP->nextRun, tmp );
		sprintf( msg, "Schedule Next Run = %s, Repeat = %d %s", tmp, schP->repeatCount, numTypeStr[schP->repeatType] );
		OutDebug( msg );

	}
}



long RunScheduleEvent( ScheduleRecPtr schP, int doitnow )
{
	memcpy( &TempPrefStruct, &MyPrefStruct, CONFIG_SIZE );
	if ( DoReadPrefsFile( (char *)schP->prefsfile, (Prefs *) &MyPrefStruct ) ){
		long done, logn = 0;

		if ( strlen(schP->upload)>2 )
			mystrcpy( MyPrefStruct.postproc_uploadreportlocation, schP->upload );		// copy upload path to config

		gProcessingSchedule = TRUE;

		LogSchedule( schP );

		// multi log files per log file string, "log1,log2"
		if ( strlen( schP->logfile ) ){
			char *logPtr, *p;
			char log1[MAXLOGLENGTH];

			mystrcpy( log1, schP->logfile );
			logPtr = log1;
			logn = 0;

			while( logPtr ){
				if( (p = mystrchr( logPtr, ',' )) )
					*p++ = 0;
				// always call wildcard ftp since
				if ( IsURL( logPtr ) ){
					if ( !strcmpd( "ftp:", logPtr ) )
						logn = done = AddWildcardFtpDirList( logPtr, logn );
					else
						logn = AddFileToLogQ( logPtr, logn );
						//logn = AddLogFile( logPtr, logn );
				}
				else {
					if ( mystrchr( logPtr, '*' ) ) {
						if ( done = AddWildCardFilenames( logPtr, logn ) )
							logn = done;
						else
							LogFailedWildcard( logPtr );
					} else
						logn = AddFileToLogQ( logPtr, logn );
						//logn = AddLogFile( logPtr, logn );
				}

				if ( p ) { while( *p == ' ' ) *p++ = 0; }
				logPtr = p;
			}
		}
		
		LogProcessList();
		
		if ( doitnow ) {
			ProcessLogNow();
		} else {
#ifdef DEF_MAC
			if ( ProcessLog() ){
#else
			if ( ProcessLog(0) ){
#endif			
				gProcessingSchedule = TRUE;
			} else
				gProcessingSchedule = FALSE;
		}
		return logn;

	} else {
		char txt[MAXFILENAMESSIZE];
		sprintf( txt, "Schedule #%d. '%s', failed to open settings %s", schP->index, schP->title, schP->prefsfile );
		LogSchedule( schP );
		LogProcessList();
		LogScheduleTxt( txt , 0 );
	}
	return 0;
}

//
long RunScheduleX( long schedulenumber )
{
	long	n=1, index=0, logn=0;
	ScheduleRecPtr schP;

	schP = gSchedules;

	while( schP ){
		if ( !gProcessing && !gProcessingSchedule && schP->index == schedulenumber ){
			
			RunScheduleEvent( schP, TRUE );
			return 1;
		}
		schP = schP->next;
		n++;
	}
	return 0;
}





void SpawnScheduleEventX( long x )
{
	char tmp[MAXFILENAMESSIZE];
	static int busy = FALSE;
	long ret;

	LogScheduleTxt( "Spawning Scheduled",0 );
	sprintf( tmp, "-nogui -runschedule %d", x );
	ret = RunSelf( tmp );

	if ( ret != 1 )
		sprintf( tmp, "Spawning Schedule done err. returncode = %d (%s)", ret, strerror(errno) );
	else
		sprintf( tmp, "Spawning Schedule done ok." );
	
	LogScheduleTxt( tmp ,0 );
}


// call this every 5 seconds or whatever interval you see fit to work well
// called during Schedule-Window and the global prefs are saved before
// schedule is run and restored when closed
//
long CheckSchedule( ScheduleRecPtr schP, long currentTime, int spawnit )
{
	long	startTime, now;
	long	n=1, index=0, logn=0;
	long	ServiceInstalled = 0;


	if ( !schP ) schP = gSchedules;
	if ( gProcessing ) { return 0; }

#if DEF_WINDOWS
	if ( !gNoGUI )
		ServiceInstalled = IsServiceInstalled(1);
#endif

	now = GetCurrentCTime();
	while( schP ){
		startTime = schP->startdate;

		if ( schP->nextRun == 0 ){
			schP->nextRun = ComputeNextSchedule( schP );
		}	
	
#ifdef DEF_MAC
		if ( !gProcessing ){ //!gProcessingSchedule
#else
		if ( !gProcessing && !gProcessingSchedule ){
#endif
		if ( schP->active && (now > schP->nextRun) && !ServiceInstalled ){
				now = GetCurrentCTime();
				schP->nextRun = ComputeNextSchedule( schP );
				if ( spawnit )
					SpawnScheduleEventX( schP->index );
				else
					RunScheduleEvent( schP, FALSE );
				SaveSchedule();		// save current NEXT RUN details only if it did the schedule process completely, so if it fails it can do it again
			} else
			if ( schP->runonce ){
				RunScheduleEvent( schP, FALSE );
				schP->runonce = 0;
			}
		}
		schP = schP->next; n++;
	}
	return index;
}



void ForceCheckSchedule( void )
{
	CheckSchedule( 0, GetCurrentCTime(), FALSE );
}

void ServiceCheckSchedule( void )
{
	CheckSchedule( 0, GetCurrentCTime(), TRUE );
}


double Round( double x )
{
	double f,i;

	f = modf( x, &i );

	if ( f > 0.5 ) return ceil( x );
	else
	if ( f < 0.5 ) return floor( x );
	else
	if ( (long)i%1 == 0 ) return floor( x );
	else
	return ceil( x );
}


void TimeComponents( long mins, long *num, long *numtype )
{
	if ( (mins/60)>=1 && ((mins/60)%24==0) ){
		*num = mins/(24*60);  *numtype = 2;		// DAYS
	} else
	if ( mins%60==0 ){
		*num = mins/60;  *numtype = 1;			// HRS
	} else {
		*num = mins;  *numtype = 0;				// MINS
	}
}


long SaveScheduleAsText( ScheduleRecPtr schP, char *filename )
{
	FILE *fp;
	long count=1;

	if ( !schP ) schP = gSchedules;

	if ( fp = fopen( filename, "w+" ) ){
		char	line[MAXFILENAMESSIZE], tmp[64], tmp2[64];

		fprintf( fp, "; --------------------------------\n" );
		fprintf( fp, "; Schedule File %s Version %s format\n", PRODUCT_TITLE, VERSION_STRING );
		fflush( fp );

		while( schP ){
			DateTimeToString( schP->startdate, tmp );
			DateTimeToString( schP->nextRun, tmp2 );
			if ( schP->title[0] == 0 )
				fprintf( fp, "#Schedule %d\n", count );
			else
				fprintf( fp, "#%s\n", schP->title );
			if ( schP->active )
				fprintf( fp, "On\n" );
			else
				fprintf( fp, "Off\n" );
			fprintf( fp, "%s\n", schP->logfile );
			fprintf( fp, "%s\n", schP->prefsfile );
			sprintf( line, "%ld,%ld # StartDate=%s, NextRun=%s\n", schP->startdate,schP->nextRun, tmp, tmp2 );
			fprintf( fp, line );
			sprintf( line, "%d,%d # Repeat every %d %s\n", schP->repeatCount, schP->repeatType, schP->repeatCount, numTypeStr[schP->repeatType] );
			fprintf( fp, line );

			if( strlen(schP->upload)>2 ) fprintf( fp, "sendto: %s\n", schP->upload );

			count++;
			schP = schP->next;
		}
		sprintf( line, "Saved %d schedules to %s", GetTotalSchedule(), filename );
		LogScheduleTxt( line , 0 );
		fclose( fp );
		return 1;
	} else {
		char txt[300];
		sprintf( txt, "failed to write schedule file %s because %s",filename, strerror(errno) );
		LogScheduleTxt( txt , 0 );
	}
	return 0;
}


void WriteCommaToMultiline( FILE *fp, char *string )
{
	char *p;
	long len = 0;

	while( string ){
		p = mystrchr( string, ',' );
		//if ( len>1 )
		//	fprintf( fp, "\n" );

		if( p ){
			*p = 0;
			len = strlen( string );
			if ( len>1 )
				fprintf( fp, "Log %s\n", string );
			*p++ = ',';
		} else {
			fprintf( fp, "Log %s\n", string );
		}
		string = p;
	}
}


long SaveScheduleConfig( ScheduleRecPtr schP, char *filename )
{
	FILE *fp;
	long count=1;

	if ( !schP ) schP = gSchedules;

	if ( fp = fopen( filename, "w+" ) ){
		char	line[MAXFILENAMESSIZE], tmp[64], tmp2[64];

		fprintf( fp, "#Schedule File\n" );
		fprintf( fp, "#--------------------------------\n" );
		fflush( fp );

		while( schP ){
			DateTimeToString( schP->startdate, tmp );
			DateTimeToString( schP->nextRun, tmp2 );
			if ( schP->title[0] == 0 )
				fprintf( fp, "Name Schedule %d\n", count );
			else
				fprintf( fp, "Name %s\n", schP->title );
			if ( schP->active )
				fprintf( fp, "Active On\n" );
			else
				fprintf( fp, "Active Off\n" );

			fflush( fp );

			WriteCommaToMultiline( fp, schP->logfile );
			//fprintf( fp, "Log %s\n", schP->logfile );
			fprintf( fp, "Prefs %s\n", schP->prefsfile );
			sprintf( line, "Startdate %ld # (%s)\n", schP->startdate, tmp );
			fprintf( fp, line );
			sprintf( line, "NextRun %ld # (%s)\n", schP->nextRun, tmp2 );
			fprintf( fp, line );
			sprintf( line, "Repeat %d # %s\n", schP->repeatCount,  numTypeStr[schP->repeatType] );
			fprintf( fp, line );
			sprintf( line, "RepeatType %d\n", schP->repeatType );
			fprintf( fp, line );

			if( strlen(schP->upload)>2 )
				fprintf( fp, "Sendto %s\n", schP->upload );

			fprintf( fp, "\n\n" );
			fflush( fp );

			count++;
			schP = schP->next;
		}

		sprintf( line, "Saved %d schedules to %s", GetTotalSchedule(), filename );
		LogScheduleTxt( line , 0 );
		fclose( fp );
		return 1;
	} else {
		char txt[300];
		sprintf( txt, "failed to write schedule file %s because %s",filename, strerror(errno) );
		LogScheduleTxt( txt , 0 );
	}
	return 0;
}



int ReadScheduleConfig( char *filename, int quietmode )
{
	FILE *fp;
	long count=0;
	ScheduleRecPtr schP;
	char szText[MAXLOGLENGTH+8], tmp[100],tmp2[100], *p;

	if ( fp = fopen( filename, "r" ) ){

		DelSchedule( 0 );

		while( !feof( fp ) ){
			char *param;
			szText[0]=0;
			myfgets( szText, MAXLOGLENGTH, fp );
			//trimLine( szText );

			if (szText[0] == 0)
				continue;

			if ( szText[0] != '#' ){
				param = mystrchr( szText, ' ' );
				if (param ) param++;

				if ( !strcmpd( "Name", szText ) ) {
					schP = AddSchedule();
					strcpy( schP->title, param );
					schP->nextRun = 0;
					*schP->upload = 0;
					count++;
				} else
				if ( !strcmpd( "Active", szText ) ) {
					if ( param[1] == 'n' ) schP->active = TRUE;
				} else
				if ( !strcmpd( "Log", szText ) ) {
					if ( schP->logfile[0] == 0 )
						mystrcpy( schP->logfile, param ); 
					else {
						strcat( schP->logfile, "," ); 
						strcat( schP->logfile, param ); 
					}
				} else
				if ( !strcmpd( "Prefs", szText ) ) {
					mystrcpy( schP->prefsfile, param );
				} else
				if ( !strcmpd( "StartDate", szText ) ) {
					sscanf( param, "%ld", &schP->startdate );
				} else
				if ( !strcmpd( "NextRun", szText ) ) {
					sscanf( param, "%ld", &schP->nextRun );
				} else
				if ( !strcmpd( "RepeatType", szText ) ) {
					schP->repeatType = myatoi( param );
				} else
				if ( !strcmpd( "Repeat", szText ) ) {
					schP->repeatCount = myatoi( param );
				} else
				if ( !strcmpd( "Sendto", szText ) ) {
					mystrcpy( schP->upload, param );
				}

				DateTimeToString( schP->startdate, tmp );
				DateTimeToString( schP->nextRun, tmp2 );
				if ( schP->repeatType <0 || schP->repeatType >= 5 ) schP->repeatType = 0;
				if ( schP->repeatCount > 10000 ) schP->repeatCount = 1;


				if ( schP->active && !quietmode && !strcmpd( "RepeatType", szText ) ){
					p = szText;
					p += sprintf( szText, "Active Schedule %d: Start=%s every %d %s, Next=%s, Log=", count, tmp, schP->repeatCount,numTypeStr[schP->repeatType], tmp2 );
					strcpyuntil( p, schP->logfile, ',' );
					MakeSafeURL( p );
					LogScheduleTxt( szText , 0 );
				}
			
			}
		}
		fclose( fp );
		if ( !quietmode ){
			sprintf( szText, "Done Reading %d schedules from config file %s", GetTotalSchedule(), filename );
			LogScheduleTxt( szText , 0 );
		}
	} else {
		sprintf( szText, "Cannot read schedule file %s", filename );
		LogScheduleTxt( szText , 0 );
		return -1;
	}
	return count;
}




void ReadScheduleAsText( char *filename, int quietmode )
{
	FILE *fp;
	long count=0;
	ScheduleRecPtr schP;
	char szText[MAXLOGLENGTH+8], tmp[100],tmp2[100], *p;

	if ( fp = fopen( filename, "r" ) ){

		DelSchedule( 0 );

		myfgets( szText, MAXLOGLENGTH, fp );
		if ( !strcmpd( "#Schedule", szText ) ){
			fseek( fp, 0, SEEK_SET );
			fclose( fp );
			ReadScheduleConfig( filename, quietmode);
			return;
		} else
			fseek( fp, 0, SEEK_SET );

				
		while( !feof( fp ) ){
			szText[0]=0;
			myfgets( szText, MAXLOGLENGTH, fp );
			if ( szText[0] == '#' ){
				schP = AddSchedule();
				trimLine( szText );
				strcpy( schP->title, szText+1 );
				schP->nextRun = 0;
				*schP->upload = 0;
				myfgets( szText, MAXLOGLENGTH, fp );
				if ( szText[1] == 'n' ) schP->active = TRUE;
				myfgets( szText, MAXLOGLENGTH, fp );	trimLine( szText ); mystrcpy( schP->logfile, szText ); 
				myfgets( szText, MAXLOGLENGTH, fp );	trimLine( szText ); mystrcpy( schP->prefsfile, szText );
				myfgets( szText, MAXLOGLENGTH, fp );
				if ( p=mystrchr( szText, ',' ) ){
					sscanf( szText, "%ld", &schP->startdate );
					sscanf( p+1, "%ld", &schP->nextRun );
				} else {
					// handle old JD floating point start date
					// else use the new long ctime format
					if ( p=mystrchr( szText, '.' ) ){		
						double jd;
						sscanf( szText, "%lf", &jd );
						schP->startdate = JDToCTime( jd );
					} else
						sscanf( szText, "%ld", &schP->startdate );
				}

				myfgets( szText, MAXLOGLENGTH, fp );
				if ( p=mystrchr( szText, ',' )){
					schP->repeatCount = myatoi( szText );
					schP->repeatType = myatoi( p+1 );
				} else {
					sscanf( szText, "%lf", &schP->repeattime );
					TimeComponents( (long)(schP->repeattime*ONEDAY), &schP->repeatCount, &schP->repeatType );
				}
				count++;

				DateTimeToString( schP->startdate, tmp );
				DateTimeToString( schP->nextRun, tmp2 );
				if ( schP->repeatType <0 || schP->repeatType >= 5 ) schP->repeatType = 0;
				if ( schP->repeatCount > 10000 ) schP->repeatCount = 1;
				if ( schP->active && !quietmode ){
					p = szText;
					p += sprintf( szText, "Active Schedule %d: Start=%s every %d %s, Next=%s, Log=", count, tmp, schP->repeatCount,numTypeStr[schP->repeatType], tmp2 );
					strcpyuntil( p, schP->logfile, ',' );
					MakeSafeURL( p );
					LogScheduleTxt( szText , 0 );
				}
			
			} else
			if ( !strcmpd( "sendto: ", szText ) ){
				trimLine( szText );  mystrcpy( schP->upload, szText+8 );
			}
		}
		fclose( fp );
		if ( !quietmode ){
			sprintf( szText, "Done Reading %d old schedules from config file %s", GetTotalSchedule(), filename );
			LogScheduleTxt( szText , 0 );
		}
	} else {
		sprintf( szText, "Cannot read old schedule file %s", filename );
		LogScheduleTxt( szText , 0 );
	}

}

long GetScheduleFiledate( void )
{
	char szText[MAXFILENAMESSIZE];
#if DEF_UNIX
	char *home = getenv( "HOME" );
	sprintf( szText, "%s/.%s", home, SCHEDULE_FILENAME );
#else
	char *home = gPath;
	sprintf( szText, "%s%c%s", home, PATHSEP, SCHEDULE_FILENAME );
#endif
	return GetFileLastDate( szText );
}

static long scheduleFiledate = 0;


void InitReadSchedule( long calltype )
{
	char szText[MAXFILENAMESSIZE], *home;
	char appBuildDetails[MAXFILENAMESSIZE];
	long readparam;

	if ( calltype == 2 ){
		long i;
		i = GetScheduleFiledate();
		if( i != scheduleFiledate ){
			scheduleFiledate = i;
			//OutDebug("Schedule file changed/updated.");
		} else {
			return;
		}
	}

	GetAppBuildDetails( appBuildDetails );

	switch( calltype ){
		default:
		case 0 : sprintf( szText, "Read Schedule: %s", appBuildDetails  ); break;
		case 1 : sprintf( szText, "NT Service : Rereading Schedule: %s", appBuildDetails ); break;
		case 2 : sprintf( szText, "ReRead Schedule: %s", appBuildDetails ); break;
	}

	if( calltype ){
		LogScheduleTxt( szText , 0 );
	} else
		LogScheduleTxt( szText , 0 );

#if DEF_UNIX
	home = getenv( "HOME" );
#else
	home = gPath;
#endif

	if ( calltype == 2 )
		readparam = TRUE;
	else
		readparam = FALSE;

#ifdef DEF_MAC
	char	prefsPath[MAXFILENAMESSIZE];
	
	GetPreferencesPath (prefsPath);
	sprintf (szText, "%s%c%s", prefsPath, PATHSEP, kMacScheduleFile);
	ReadScheduleConfig (szText, readparam);
#else
	sprintf( szText, "%s%c%s", home, PATHSEP, SCHEDULE_NEWFILENAME );

	if( ReadScheduleConfig( szText, readparam ) < 0 ){
		sprintf( szText, "%s%c%s", home, PATHSEP, SCHEDULE_FILENAME );
		ReadScheduleAsText( szText, readparam );
	}
#endif
}

long SaveSchedule( void )
{
	char szText[MAXFILENAMESSIZE];
	long	ret;

#if DEF_UNIX
	char *home = getenv( "HOME" );
#else
	char *home = gPath;
#endif

#ifdef DEF_MAC
	char	prefsPath[MAXFILENAMESSIZE];
	
	GetPreferencesPath (prefsPath);
	sprintf (szText, "%s%c%s", prefsPath, PATHSEP, kMacScheduleFile);
	ret = SaveScheduleConfig (0, szText);
#else
	sprintf( szText, "%s%c%s", home, PATHSEP, SCHEDULE_NEWFILENAME );
	SaveScheduleConfig( 0, szText );

	sprintf( szText, "%s%c%s", home, PATHSEP, SCHEDULE_FILENAME );
	ret = SaveScheduleAsText( 0, szText );
#endif

	scheduleFiledate = GetScheduleFiledate();

	return ret;
}



