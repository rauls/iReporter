#include "Compiler.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "zlib.h"

#include "myansi.h"
#include "log_io.h"
#include "engine_dbio.h"
#include "engine_drill.h"
#include "statdefs.h"
#include "EngineRestoreStat.h"
#include "EngineVirtualDomain.h"
#include "Stats.h"
#include "FileTypes.h"
#include "DateFixFileName.h"

#if DEF_WINDOWS
	#include <sys/stat.h>
	#include "createshortcuts.h"	// for GetShortCut()
	#include "Winmain.h"			// for StatusWindowSetProgress()
#endif				
#if DEF_UNIX
	#include <sys/stat.h>
	#include "main.h"
#endif
#ifdef DEF_MAC
	#include "MacStatus.h"
	#include "stdfile.h"
#endif


//==========================================================================================
// Initialisations & Declarations
//==========================================================================================

#define	FWDBASE_VERSION		"4.2"
static double	db_version = 4.2;

// id tags for StatLists written out to an FWA database
// NOTE: must be <= 10 chars in length:			="1234567890"	OK!
static const char s_brokenLinkReferalsTag[]		="broklnk";
static const char s_intBrokenLinkReferalsTag[]	="intbroklnk";
static const char s_unrecognisedAgentsTag[]		="unrecagent";
static const char s_errorsWithTopReferralsTag[]	="errtoprefs";	// for top referrals on errors
static const char s_robotsTag[]					="robot";
static const char s_szClustersTag[]				="clusters";

static	gzFile outfp(0);
static 	char dbName[1024];
static	char tmpName[1024];


//==========================================================================================
// Local Functions
//==========================================================================================

static void DBIO_SaveTimeHistory( Statistic *stat , gzFile fh )
{
	long	num;
	TimeRecStat *tr;

	tr = stat->timeStat;
	if ( tr ){
		gzwrite( fh, tr , sizeof(TimeRecStat) );
		if( num = tr->GetNum() )
			gzwrite( fh, tr->byTimerec , sizeof(TimeRec)*num );

		tr = stat->timeStat->timeStat2;
		if ( tr ){
			gzwrite( fh, tr , sizeof(TimeRecStat) );
			if( num = tr->GetNum() )
				gzwrite( fh, tr->byTimerec , sizeof(TimeRec)*num );
		}
	}
}

// save a session history from a record
static void DBIO_SaveSessionHistory( Statistic *stat , gzFile fh )
{
	long	tot;

	if ( stat->sessionStat ){
		if( tot = stat->sessionStat->GetNum() ) {
			gzwrite( fh, stat->sessionStat , sizeof(SessionStat) );
			if ( stat->sessionStat->Session )
				gzwrite( fh, stat->sessionStat->Session , sizeof(SessionRec)*tot );
		}
	}
}

// save one record from a stat
static void DBIO_SaveStatistic( Statistic *stat , gzFile fh, long useOtherNames, const char* id )
{
	Statistic savestat;
	void	*nullvalue = (void*)1;

	memcpy( &savestat, stat, sizeof(Statistic) );

	if ( savestat.timeStat )		savestat.timeStat = (TimeRecStat *)nullvalue;
	if ( savestat.sessionStat )		savestat.sessionStat = (SessionStat*)nullvalue;
	if ( savestat.next )			savestat.next = (Statistic*)nullvalue;
	if ( savestat.length>=0 )		savestat.name = (char*)nullvalue;

	gzwrite( fh, &savestat ,sizeof(Statistic) );

	if ( stat->GetName() && stat->length >0 )
	{
		switch( useOtherNames )
		{
			case NAMEIS_NORMAL:
			case NAMEIS_IP:	gzwrite( fh, stat->GetName() ,stat->length ); break;
		}
	}

	memset( &savestat, 0, sizeof(Statistic) );
}


static void DBIO_UpdateProgress( long value, long valuemax, char *message )
{
#ifndef DEF_MAC
	StatusWindowSetProgress( 100*value/valuemax, message );
#endif
}


static int savecounter = 0;

// save just one stat to file
static void DBIO_SaveStatList( StatList* byStat, gzFile fh, const char* id )
{
	DEF_PRECONDITION( id );
	DEF_PRECONDITION( strlen(id)<11 );	// crap implementation only allows up to 10 chars

	long n;

	if ( byStat ){
		if ( byStat->num ){
			char	statid[17] = "STAT";   

			memset( statid, 0, 16 );
			sprintf( statid, "STAT-%s", id );
			gzwrite( fh, statid, 16 );
			sprintf( byStat->stat_id, "(%s)", id );
			gzwrite( fh, byStat, sizeof(StatList) );

			OutDebugs( "saving %d records into %s...", byStat->num, id );

			DBIO_UpdateProgress( savecounter++, 40, NULL );

			VDinfo* pVirtualDomain=reinterpret_cast<VDinfo*>(byStat->vd);
			DEF_ASSERT(pVirtualDomain);

			for(n=0; n<byStat->num; n++)
			{
				Statistic *stat=0;

				// if we're currently saving the weekly stats
				if( byStat==pVirtualDomain->byWeekday )
				{
					// NOTE: We need to save the weekly stats in an absolute order (ie sun, mon, tue...) regardless
					// of how they may currently be ordered (due to user's first day of week setting) so we 
					// map the current postion "n" to an actual day of the week index "dayIndex" and then
					// save the corresponding stat.

					// offset the current day index according to user's first day of week setting so that
					// the specified day of week appears first within the weekly traffic reports
					size_t dayIndex=(n+7-MyPrefStruct.firstDayOfWeek)%7;

					stat=byStat->GetStat(dayIndex);
				}
				// else not currently saving weekly stats
				else
				{
					stat=byStat->GetStat(n);
				}

				DBIO_SaveStatistic( stat , fh, byStat->useOtherNames, id );

				if( byStat->doTimeStat && stat->timeStat )
				{
					DBIO_SaveTimeHistory( stat , fh );
				}

				if( byStat->doSessionStat && stat->sessionStat )
				{
					DBIO_SaveSessionHistory( stat , fh );
				}

				// if there are failed requests to store in the database for this Statistic
				if( byStat==pVirtualDomain->byBrokenLinkReferal || 
					byStat==pVirtualDomain->byIntBrokenLinkReferal ||
					byStat==pVirtualDomain->byErrors
					)
				{
					// ref the FailedRequestInfo for current stat
					const CQFailedRequestInfo& failReqInfo=pVirtualDomain->GetFailedRequestInfo( stat );

					// save current stat's FailedRequestInfo
					failReqInfo.saveTo( fh );
				}
			}
		}
	}
}


static void DBIO_SaveClose( int success )
{
	OutDebug( "Database closing" );

	if ( outfp ){
		gzclose( outfp );
		outfp=0;
		OutDebug( "Database closed" );
	} else
		OutDebug( "? not open" );

	if ( success ){
		int rc;
		remove( dbName );
		rc = rename( tmpName, dbName );
		OutDebugs( "renaming %s to %s", tmpName, dbName );
	}
}

// save a whole virtualhost/host to the file
static int DBIO_SaveVhost( VDinfoP VDptr, char *filename, int type )
{
	long	n, success = 0;

	if ( VDptr ){
		char msg[256];

		sprintf( msg, "Saving host %s", VDptr->domainName );
		DBIO_UpdateProgress( 0, 1, msg );
		if ( !outfp ){
			if ( filename == NULL ){
				sprintf( dbName, "%s.fdb", VDptr->domainName );
				sprintf( tmpName, "%s.fdb.tmp", VDptr->domainName );
			} else {
				sprintf( dbName, "%s", filename );
				sprintf( tmpName, "%s.tmp", filename );
			}
#ifdef DEF_MAC
			SetType (kFWDatabaseFormat);
#endif		
			outfp = gzopen( tmpName, "wb" );
		}

		if ( outfp ){
			char id[10];		// = "FWDB4.1\0";
			sprintf( id, "FWDB%s", FWDBASE_VERSION );
			gzwrite( outfp, id, 8 );

			savecounter = 0;

			gzwrite( outfp, VDptr, sizeof(VDinfo) );
			DBIO_SaveStatList( VDptr->byHour, outfp, "hour" );
			DBIO_SaveStatList( VDptr->byWeekday, outfp, "wkday" );
			for( n=0; n < 7; n++ )
				DBIO_SaveStatList( VDptr->byWeekdays[n], outfp, "wdays" );
			DBIO_SaveStatList( VDptr->byMonth, outfp, "month" );
			DBIO_SaveStatList( VDptr->byDate, outfp, "date" );
			DBIO_SaveStatList( VDptr->byClient, outfp, "client" );
			DBIO_SaveStatList( VDptr->byServers, outfp, "src" );

			DBIO_SaveStatList( VDptr->byFile, outfp, "url" );
			DBIO_SaveStatList( VDptr->byBrowser, outfp, "browser" );
			DBIO_SaveStatList( VDptr->byOperSys, outfp, "oper" );
			DBIO_SaveStatList( VDptr->byRobot, outfp, s_robotsTag );
			DBIO_SaveStatList( VDptr->byRefer, outfp, "refer" );
			DBIO_SaveStatList( VDptr->byReferSite, outfp, "rsite" );
			DBIO_SaveStatList( VDptr->byDir, outfp, "dir" );
			DBIO_SaveStatList( VDptr->byGroups, outfp, "groups" );
			DBIO_SaveStatList( VDptr->byTopDir, outfp, "topdir" );
			DBIO_SaveStatList( VDptr->byType, outfp, "type" );
			DBIO_SaveStatList( VDptr->byErrors, outfp, s_errorsWithTopReferralsTag );
			DBIO_SaveStatList( VDptr->byErrorURL, outfp, "errurl" );
			DBIO_SaveStatList( VDptr->byPages, outfp, "pages" );
			DBIO_SaveStatList( VDptr->bySearchStr, outfp, "searchstr" );
			DBIO_SaveStatList( VDptr->bySearchSite, outfp, "searchsite" );
			DBIO_SaveStatList( VDptr->byDownload, outfp, "down" );
			DBIO_SaveStatList( VDptr->byAdvert, outfp, "advert" );
			DBIO_SaveStatList( VDptr->byAdCamp, outfp, "adcamp" );

			DBIO_SaveStatList( VDptr->byMediaPlayers, outfp, "mplayers" );
			DBIO_SaveStatList( VDptr->byAudio, outfp, "audio" );
			DBIO_SaveStatList( VDptr->byVideo, outfp, "video" );
			DBIO_SaveStatList( VDptr->byMediaTypes, outfp, "mtypes" );

			DBIO_SaveStatList( VDptr->byProtocol, outfp, "prot" );
			DBIO_SaveStatList( VDptr->byHTTP, outfp, "http" );
			DBIO_SaveStatList( VDptr->byHTTPS, outfp, "https" );
			DBIO_SaveStatList( VDptr->byMail, outfp, "mail" );
			DBIO_SaveStatList( VDptr->byFTP, outfp, "ftp" );
			DBIO_SaveStatList( VDptr->byTelnet, outfp, "telnet" );
			DBIO_SaveStatList( VDptr->byDNS, outfp, "dns" );
			DBIO_SaveStatList( VDptr->byPOP3, outfp, "pop3" );
			DBIO_SaveStatList( VDptr->byReal, outfp, "real" );
			DBIO_SaveStatList( VDptr->byOthers, outfp, "others" );
			DBIO_SaveStatList( VDptr->byBrokenLinkReferal, outfp, s_brokenLinkReferalsTag );
			DBIO_SaveStatList( VDptr->byIntBrokenLinkReferal, outfp, s_intBrokenLinkReferalsTag );
			DBIO_SaveStatList( VDptr->byUnrecognizedAgents, outfp, s_unrecognisedAgentsTag );
			DBIO_SaveStatList( VDptr->byClusters, outfp, s_szClustersTag );

			success = TRUE;

		} else {
			ErrorMsg( "Cannot open database %s", tmpName );
		}

	}
	return success;
}


static void DBIO_LoadTimeHistory( StatList *byStat, Statistic *stat , gzFile fh )
{
	long	tot, num;

	if ( (byStat->doTimeStat) && stat->timeStat ) {
		TimeRecStat *tr;

		tr = stat->timeStat = new TimeRecStat;
		if ( tr ){
			// if == 1 , then alloc a data space for it.
			gzread( fh, tr , sizeof(TimeRecStat) );
			tot = tr->GetTot();
			num = tr->GetNum();

			if ( tot > 0 ){
				tr->byTimerec = new TimeRec[num>tot ? num : tot];
				if( num )
					gzread( fh, tr->byTimerec , sizeof(TimeRec)*num );
			} else
				tr->byTimerec = NULL;

			if ( (byStat->doTimeStat & 2) && stat->timeStat->timeStat2  ){
				tr = stat->timeStat->timeStat2 = new TimeRecStat;
				if ( tr ){
					gzread( fh, tr , sizeof(TimeRecStat) );
					tot = tr->GetTot();
					num = tr->GetNum();

					if ( tot > 0 ){
						tr->byTimerec = new TimeRec[num>tot ? num : tot];
						if ( num )
							gzread( fh, tr->byTimerec , sizeof(TimeRec)*num );
					} else
						tr->byTimerec = NULL;
				} 
			} else
				stat->timeStat->timeStat2 = NULL;
			if ( (long)stat->timeStat->timeStat2 == 1 )
				stat->timeStat->timeStat2 = NULL;
		}
	}
	if ( (long)stat->timeStat == 1 ) stat->timeStat = 0;
}


static void DBIO_LoadSessionHistory( VDinfoP VDptr, StatList *byStat, Statistic *stat , gzFile fh )
{
	long	tot,num;

	if( stat->sessionStat ) {
		stat->sessionStat = new SessionStat;
		size_t long_s = sizeof(long);

		if( db_version == 4.0 )	// this format had 2 less longs that the default.
			gzread( fh, stat->sessionStat , sizeof(SessionStat)-(long_s*2) );
		else
		if( db_version == 4.1 )	// this is 4.04 + format....
			gzread( fh, stat->sessionStat , sizeof(SessionStat)-(long_s*1) );
		else // Current Style is normal.
			gzread( fh, stat->sessionStat , sizeof(SessionStat) );

		stat->sessionStat->vd = VDptr;
		num = stat->sessionStat->GetNum();
		tot = stat->sessionStat->GetTot();
		if ( tot>0 ){
			stat->sessionStat->Session = new SessionRec[tot];
			gzread( fh, stat->sessionStat->Session , sizeof(SessionRec)*num );
		}
	}
	if ( (long)stat->sessionStat == 1 ) stat->sessionStat = 0;
}



static void ClearMeanTimes(StatList *stat )
{
	long i = 0;
	Statistic *p;

	stat->totalTime = 0;

	while( p = stat->GetStat(i) ){
		p->visitTot = 0;
		i++;
	}
}


static void ClearSessions(StatList *stat )
{
	long i = 0;
	Statistic *p;

	stat->totalVisits = 0;

	while( p = stat->GetStat(i) ){
		p->visits = 0;
		p->visitTot = 0;
		i++;
	}
}


static void ClearVisitors(StatList *stat )
{
	long i = 0;
	Statistic *p;

	stat->totalCounters = 0;

	while( p = stat->GetStat(i) ){
		p->counter = 0;
		i++;
	}
}


static long ReadString( gzFile fh, char *dest, long length )
{
	long n;

	n = gzread( fh, dest, length );
	if ( n>0 )
		dest[n] = 0;
	return n;
}


static long DBIO_LoadStatistic( VDinfoP VDptr, Statistic *stat, gzFile fh, long useOtherNames, char *id )
{
	long	dataread = 0;

	if ( !fh ) return 0;

	// read one statistic
	if ( stat ) {
		dataread += gzread( fh, stat ,sizeof(Statistic) );

		if ( useOtherNames == NAMEIS_NORMAL || useOtherNames == NAMEIS_IP ) {
			if ( stat->GetName() && stat->length>0 ){
				char *name;
				name = stat->AllocateName( stat->length );
				dataread += ReadString( fh, name, stat->length );
			}
		} else 
		if ( (long)stat->GetName() == NAMEIS_STATIC )
		{
			if( !mystrcmpi( "hour", id ) )					RestoreHourNames( stat );
			else if( !mystrcmpi( "wkday", id ) )			RestoreWeekdaysNames( stat );
			else if( !mystrcmpi( "wdays", id ) )			RestoreHourNames( stat );
			else if( !mystrcmpi( "oper", id ) )				RestoreOpersysNames( stat );
			else if( !mystrcmpi( "errs", id ) )				RestoreErrorsNames( stat );
			else if( !mystrcmpi( s_errorsWithTopReferralsTag, id ) ) RestoreErrorsNames( stat );
			else if( !mystrcmpi( "pages", id ) )			RestoreStatisticName( stat, VDptr->byFile );
			else if( !mystrcmpi( "down", id ) )				RestoreStatisticName( stat, VDptr->byFile );
			else if( !mystrcmpi( "audio", id ) ) 			RestoreStatisticName( stat, VDptr->byFile );
			else if( !mystrcmpi( "video", id ) ) 			RestoreStatisticName( stat, VDptr->byFile );
			else if( !mystrcmpi( s_brokenLinkReferalsTag, id ) ) 	RestoreStatisticName( stat, VDptr->byRefer );
			else if( !mystrcmpi( s_intBrokenLinkReferalsTag, id ) ) RestoreStatisticName( stat, VDptr->byRefer );
			else											stat->name = NULL;
		}
	} else {
	// read to where? empty area just incase
		Statistic Lstat;
		dataread += gzread( fh, &Lstat ,sizeof(Statistic) );
		if ( Lstat.GetName() && !useOtherNames ) {
			char tmp[1024];
			dataread += gzread( fh, tmp , stat->length );
		}
		if ( (long)stat->GetName() == NAMEIS_STATIC ) 
			stat->name = NULL;
	}
	return dataread;
}

// read a stat from the file to the memory stat
static long DBIO_LoadStatListID( gzFile fh, char *id )
{
	long n = 0;

	if ( fh ){
		char		statid[17];

		gzread( fh, statid, 16 );

		strcpy( id, statid );
		return 1;
	}
	return 0;
}


// read a stat from the file to the memory stat
static long DBIO_LoadStatList( VDinfoP VDptr, StatList *byStat , gzFile fh, char *id )
{
	DEF_PRECONDITION(VDptr); 
	DEF_PRECONDITION(byStat); 
	DEF_PRECONDITION(!strcmpd( "STAT", id )); 

	// Sanity Check to avoid crash, maybe just delays the real issue to another location in the app.
	if ( !byStat )
	{
		OutDebugs( "byStat is 'NULL', due to report setting being turned off.  We need to look at DB loading issues relating to Settings files." );
		return 0;
	}


	// create a temp buffer into which we can read the StatList-like info and then read it in
	char tempByStatBuf[sizeof(StatList)];
	StatList* tempByStat=reinterpret_cast<StatList*>(tempByStatBuf);
	if( !gzread( fh, tempByStat, sizeof(StatList) ) )
	{
		return 0;
	}

	// copy the required attributes from our temporary
	byStat->doTimeStat = tempByStat->doTimeStat;
	byStat->doSessionStat = tempByStat->doSessionStat;
	byStat->useOtherNames = tempByStat->useOtherNames;
	byStat->useFixedNames = tempByStat->useFixedNames;
	byStat->num = tempByStat->num;
	byStat->maxNum = tempByStat->maxNum;
	byStat->incrSize = tempByStat->incrSize;
	byStat->totalRequests = tempByStat->totalRequests;
	byStat->totalCounters = tempByStat->totalCounters;
	byStat->totalCounters4 = tempByStat->totalCounters4;
	byStat->totalErrors = tempByStat->totalErrors;
	byStat->totalVisits = tempByStat->totalVisits;
	byStat->totalBytes = tempByStat->totalBytes;
	byStat->totalBytesIn = tempByStat->totalBytesIn;
	byStat->totalTime = tempByStat->totalTime;
	delete [] byStat->stat;
	byStat->stat = new Statistic[ byStat->maxNum ];
	memset( byStat->stat, 0, byStat->maxNum*sizeof(Statistic) );
	byStat->MarkForRebuild();

	OutDebugs( "Reading %d records into %s...", byStat->num, id );

	for(long n(0); n<byStat->num; n++)
	{
		Statistic* stat=0;

		// if we're currently loading the weekly stats
		if( byStat==VDptr->byWeekday )
		{
			// NOTE: The weekly stats get saved in an absolute order (ie sun, mon, tue...) regardless
			// of how the user may want them ordered (due to user's first day of week setting) so we 
			// map the current postion "n" to a day of the week index "dayIndex" and then read
			// into the corresponding stat.

			// offset the current day index according to user's first day of week setting so that
			// the specified day of week appears first within the weekly traffic reports
			size_t dayIndex=(n+7-MyPrefStruct.firstDayOfWeek)%7;

			stat=byStat->GetStat(dayIndex);
		}
		// else not currently loading weekly stats
		else
		{
			stat=byStat->GetStat(n);
		}

		DBIO_LoadStatistic( VDptr, stat , fh, byStat->useOtherNames, id+5 );

		if( byStat->doTimeStat && stat->timeStat )
		{
			DBIO_LoadTimeHistory( byStat, stat , fh );
		}

		if( byStat->doSessionStat && stat->sessionStat )
		{
			DBIO_LoadSessionHistory( VDptr, byStat, stat , fh );
		}

		if( (long)stat->GetName() == NAMEIS_STATIC )
		{
			OutDebug( "GetName Error" );
		}

		// if there are failed requests stored in the database
		if( byStat==VDptr->byBrokenLinkReferal ||
			byStat==VDptr->byIntBrokenLinkReferal ||
			byStat==VDptr->byErrors && !mystrcmpi( id+5, s_errorsWithTopReferralsTag ) // only read top referals in for post 4.5 databases
			)
		{
			// ref FailedRequestInfo for current stat
			CQFailedRequestInfo& failReqInfo=VDptr->GetFailedRequestInfo( stat );

			// read current stat's FailedRequestInfo
			failReqInfo.loadFrom( fh );
		}
	}

	if ( !mystrcmpi( "pages", id+5 ) )
	{	
		ClearSessions( byStat );
		ClearVisitors( byStat );
		ClearMeanTimes( byStat );
	}

	if (    !mystrcmpi( "wkday", id+5 )
		 || !mystrcmpi( "wkdays", id+5 )
		 || !mystrcmpi( "hourly", id+5 )
		 || !mystrcmpi( "date", id+5 )
		 || !mystrcmpi( "month", id+5 )
		// !mystrcmpi( "browser", id+5 )
		// !mystrcmpi( "oper", id+5 )
	   )
	{	
		ClearVisitors( byStat );
	}

	return 1;
}


static gzFile DBIO_OpenFile( const char *filename )
{
	char rawfileName[256];

	strcpy( rawfileName, filename );

	DateFixFilename( rawfileName, 0 );

	if ( IsShortCut( rawfileName ) ){
		char linkpath[256];
		mystrcpy( linkpath, rawfileName );
#if DEF_WINDOWS
		GetShortCut( linkpath, rawfileName );
#endif
	}

	gzFile fh=0;

	if ( !IsURL( rawfileName ) )
	{
		char format[256];

		// determine if its PKZIP file and if so... dont open it, YET
		if ( IsFileInvalid( rawfileName, format ) )
		{
			OutDebugs( "Cannot open %s because it is a %s file.", rawfileName, format );
			return 0;
		}
		else 
		{
			if ( IsBzFile( rawfileName ) )
				; // fh=bzopen( rawfileName, "rb" ); removed for compat build RHF
			else
				fh=gzopen( rawfileName, "rb" );
		}
	}
	return fh;
}


//==========================================================================================
// Public Functions
//==========================================================================================

long DBIO_Open( const char* filename )
{
	gzFile handle( DBIO_OpenFile( filename ) );

	if ( handle )
	{
		char id[32];

		gzread( handle, id, 8 );

		if ( strcmpd( "FWDB", id ) ){
			DBIO_Close( (long)handle );
			return 0;
		} else {
			DBIO_Close( (long)handle );
			handle = DBIO_OpenFile( filename );
		}
	}
	return (long)handle;
}


void DBIO_SaveToFile( VDinfoP VDptr, char *filename )
{
	long ret;

	if ( VDptr && filename ){
		char	dbpath[512];
		char	dbname[512];

		PathFromFullPath( filename, dbpath );
		// only attach a .fdb if the file hasnt got a .fdb
		if ( strstr( filename, ".fdb" ) )
			sprintf( dbname , FileFromPath( filename, 0 ) );
		else
			sprintf( dbname , "%s.fdb", FileFromPath( filename, 0 ) );
		strcat( dbpath , dbname );

		ret = DBIO_SaveVhost( VDptr, dbpath, 0 );

		DBIO_SaveClose( ret );
	}
}


void DBIO_SaveAllToFile( VDinfoP *VDptrs, long count, char *filename )
{
	long i, ret;
	VDinfoP VD;

	if ( VDptrs && filename ){
		char	dbpath[512];
		char	dbname[512];

		PathFromFullPath( filename, dbpath );
		// only attach a .fdb if the file hasnt got a .fdb
		if ( strstr( filename, ".fdb" ) )
			sprintf( dbname , FileFromPath( filename, 0 ) );
		else
			sprintf( dbname , "%s.fdb", FileFromPath( filename, 0 ) );
		strcat( dbpath , dbname );

		for( i=0; i<=count; i++){
			VD = VDptrs[i];
			ret = DBIO_SaveVhost( VD, dbpath, TRUE );
		}

		DBIO_SaveClose( ret );
	}
}


// open/load a whole statdatabase from file to ram ready for process adding
// return 0....VDnums.
// -1 for failure
long DBIO_LoadFromFile( long f, long VDcount, long *allreq )
{
	gzFile fp( (gzFile)f );		// cast to native file type

	long totalReq=0, n;
	VDinfoP VDptr = NULL,oldvd;

	if ( fp ){
		{
			long	VhostStat, DBStat, alltime1, x;
			char	*listName;
			char	id[32];
			StatList *stat = NULL;

			alltime1 = (long)timems();

			DBStat = gzread( fp, id, 8 );

			while( DBStat )
			{
				if ( strcmpd( "FWDB", id ) ){
					OutDebug( "File is not a database" );
					return -1;
				}

				if ( !strcmpd( "FWDBASE", id ) )
					db_version = 4;
				else
					db_version = atof( id+4 );

				// Read the VDinfo into a temp buffer of appropriate size.  Note that we don't
				// actually construct a VDinfo instance here as we do not want its dtor to be
				// called on exit of the current scope.
				char vdataBuf[sizeof(VDinfo)];	
				VhostStat = gzread( fp, vdataBuf, sizeof(VDinfo) );

				if( VhostStat ){
					char msg[256];
					long listnum = 0;
					// reinterpret our temp VDinfo buffer for convenience
					VDinfo& VDdata=*reinterpret_cast<VDinfo*>(vdataBuf);

					OutDebugs( "Reading Host #%d, %s", VDcount, VDdata.domainName );
					sprintf( msg, "Loading Host #%d, %s", VDcount, VDdata.domainName );
					DBIO_UpdateProgress( 0, 1, msg );

					oldvd = VDptr;
					VDptr = InitVirtualDomain( VDcount, VDdata.domainName, 1 );
					if ( oldvd )
						oldvd->next = VDptr;
					VDptr->logType = VDdata.logType;		// = LOGFORMAT_DATABASE;
					VDptr->firstTime = VDdata.firstTime;
					VDptr->lastTime = VDdata.lastTime;
					VDptr->totalInDataSize = VDdata.totalInDataSize;
					VDptr->totalFailedRequests = VDdata.totalFailedRequests;
					VDptr->totalRequests = VDdata.totalRequests;
					VDptr->totalCachedHits = VDdata.totalCachedHits;
					VDptr->totalUniqueClients = VDdata.totalUniqueClients;
					VDptr->totalFlushedClients = VDdata.totalFlushedClients;
					VDptr->totalRepeatClients = VDdata.totalRepeatClients;
					VDptr->totalDays = VDdata.totalDays;
					VDptr->totalBytes = VDdata.totalBytes;
					VDptr->totalBytesIn = VDdata.totalBytesIn;
					totalReq += VDptr->totalRequests;
					VDcount++;

					n = 0;
					
					// The proper way to read the database which may change in future.....
					while( VhostStat ){
						memset( id, 0, 16 );
						listName = &id[5];

						DBStat = gzread( fp, id, 8 );

						if ( !strcmpd( "FWDB", id ) )	{
							OutDebug( "New host found" );
							x = 0;	// Another VHOST header, stop and restart new vhost
						} else
							x = gzread( fp, id+8, 8 );

						if ( !strcmpd( listName, "hour" ) )			 x = DBIO_LoadStatList( VDptr, VDptr->byHour, fp, id );
						else if ( !strcmpd( listName, "wkday" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byWeekday, fp, id );
						else if ( !strcmpd( listName, "wdays" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byWeekdays[n++], fp, id );
						else if ( !strcmpd( listName, "month" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byMonth, fp, id );
						else if ( !strcmpd( listName, "date" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byDate, fp, id );
						else if ( !strcmpd( listName, "client" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byClient, fp, id );
						else if ( !strcmpd( listName, "src" ) )		 x = DBIO_LoadStatList( VDptr, VDptr->byServers, fp, id );

						else if ( !strcmpd( listName, "url" ) )		 x = DBIO_LoadStatList( VDptr, VDptr->byFile, fp, id );
						else if ( !strcmpd( listName, "browser" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byBrowser, fp, id );
						else if ( !strcmpd( listName, "oper" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byOperSys, fp, id );
						else if ( !strcmpd( listName, s_robotsTag ) ) x = DBIO_LoadStatList( VDptr, VDptr->byRobot, fp, id );
						else if ( !strcmpd( listName, "refer" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byRefer, fp, id );
						else if ( !strcmpd( listName, "rsite" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byReferSite, fp, id );
						else if ( !strcmpd( listName, "dir" ) )		 x = DBIO_LoadStatList( VDptr, VDptr->byDir, fp, id );
						else if ( !strcmpd( listName, "groups" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byGroups, fp, id );
						else if ( !strcmpd( listName, "topdir" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byTopDir, fp, id );
						else if ( !strcmpd( listName, "type" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byType, fp, id );
						// "errs" only for older databases (pre 4.5) without top referals on errors info
		 				else if ( !strcmpd( listName, "errs" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byErrors, fp, id );
						// s_errorsWithTopReferralsTag only for newer databases (post 4.5) with top referals on errors info
						else if ( !strcmpd( listName, s_errorsWithTopReferralsTag ) ) x = DBIO_LoadStatList( VDptr, VDptr->byErrors, fp, id );
						else if ( !strcmpd( listName, "errurl" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byErrorURL, fp, id );
						else if ( !strcmpd( listName, "pages" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byPages, fp, id );
						else if ( !strcmpd( listName, "searchstr"))	 x = DBIO_LoadStatList( VDptr, VDptr->bySearchStr, fp, id );
						else if ( !strcmpd( listName, "searchsite")) x = DBIO_LoadStatList( VDptr, VDptr->bySearchSite, fp, id );
						else if ( !strcmpd( listName, "down" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byDownload, fp, id );
						else if ( !strcmpd( listName, "advert" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byAdvert, fp, id );
						else if ( !strcmpd( listName, "adcamp" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byAdCamp, fp, id );

						else if ( !strcmpd( listName, "mplayers" ) ) x = DBIO_LoadStatList( VDptr, VDptr->byMediaPlayers, fp, id );
						else if ( !strcmpd( listName, "audio" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byAudio, fp, id );
						else if ( !strcmpd( listName, "video" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byVideo, fp, id );
						else if ( !strcmpd( listName, "mtypes" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byMediaTypes, fp, id );

						else if ( !strcmpd( listName, "prot" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byProtocol, fp, id );
						else if ( !strcmpd( listName, "https" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byHTTPS, fp, id );
						else if ( !strcmpd( listName, "http" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byHTTP, fp, id );
						else if ( !strcmpd( listName, "mail" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byMail, fp, id );
						else if ( !strcmpd( listName, "ftp" ) )		 x = DBIO_LoadStatList( VDptr, VDptr->byFTP, fp, id );
						else if ( !strcmpd( listName, "telnet" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byTelnet, fp, id );
						else if ( !strcmpd( listName, "dns" ) )		 x = DBIO_LoadStatList( VDptr, VDptr->byDNS, fp, id );
						else if ( !strcmpd( listName, "pop3" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byPOP3, fp, id );
						else if ( !strcmpd( listName, "real" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byReal, fp, id );
						else if ( !strcmpd( listName, "others" ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byOthers, fp, id );
						else if ( !strcmpd( listName, s_brokenLinkReferalsTag ) )	 x = DBIO_LoadStatList( VDptr, VDptr->byBrokenLinkReferal, fp, id );
						else if ( !strcmpd( listName, s_intBrokenLinkReferalsTag ) ) x = DBIO_LoadStatList( VDptr, VDptr->byIntBrokenLinkReferal, fp, id );
						else if ( !strcmpd( listName, s_unrecognisedAgentsTag) )	 x = DBIO_LoadStatList( VDptr, VDptr->byUnrecognizedAgents, fp, id );
						else if ( !strcmpd( listName, s_szClustersTag) )	 x = DBIO_LoadStatList( VDptr, VDptr->byClusters, fp, id );
					
						DBIO_UpdateProgress( listnum++, 37, NULL );
						VhostStat = x;
					} // while doing stats

					FixClientBrowserOS_IDS( VDptr );

				} // if vhost
			} //while loop in dbase

			if ( !DBStat )
				OutDebug( "End of database" );

			VDptr->time2 = timems() - VDptr->time1;
			if ( VDptr->time2 == 0 ) VDptr->time2=1;
		}
	}
	*allreq += totalReq;
	// make sure its 1 less, VDcount of 1 means 1 host, but must be 0,   VDcount of 8 is 7hosts, so its returned as 7.
	// as the first host info is the file not the hosts.
	return VDcount-1;
}


void DBIO_Close( long f )
{
	if ( f )
		gzclose( (gzFile) f );
}


int IsFileV4DataBase( const char* filename )
{
	long dbFile;
	if ( dbFile = DBIO_Open( filename ) )
	{
		DBIO_Close( dbFile );
		return TRUE;
	} else
		return FALSE;
}


