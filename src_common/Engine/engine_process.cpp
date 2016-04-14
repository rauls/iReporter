/*  ================================================================================================

	##########
	##
	##
	##########
			 ##
			 ##
			 ##
	##########

	Copyright © Software 2001, 2002
*/

#include "FWA.h"

// ***************************************************************
// If you are using TrueTime then just turn on this define (1)
// You want to do it in the release builds.
// ***************************************************************
#ifndef	DEF_DEBUG
#	define	USE_TRUE_TIME	0
#endif

#if USE_TRUE_TIME
#include "C:/Program Files/Compuware/PCShared/nmtxapi.h"
#endif

#include <string>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>	// for stable_sort()

#ifdef DEF_MAC
	#include <string.h>
	#include <OpenTransport.h>
	#include <OpenTptInternet.h>
	//#include <profiler.h>
	//#include "main.h"			// for ShowProgressDetail()
	#include "MacStatus.h"
	#include "MacUtil.h"
	#include "progress.h"
	#include "MacDNR.h"
	//#include "macdns.h"
	//#include "server.h"
	#include "Processing.h"
#endif								

#include "datetime.h"
#include "myansi.h"
#include "engine_process.h"
#include "engine_process_firewall.h"
#include "log_io.h"
#include "gd.h"
#include "config_struct.h"
#include "engine_drill.h"
#include "engine_notify.h"
#include "engine_proc.h"
#include "Report.h"
#include "ReportInterface.h"
#include "ResDefs.h"
#include "engine_dbio.h"
#include "editpath.h"
#include "asyncdnr.h"
#include "Utilities.h"
#include "OSInfo.h"
#include "Hash.h"					
#include "EngineStatus.h"			
#include "EngineRegion.h"			
#include "translate.h"				
#include "EngineRestoreStat.h"		
#include "StatList.h"				
#include "EngineCluster.h"			
#include "EngineParse.h"			
#include "EngineAddStats.h"			
#include "EngineClientDNR.h"		
#include "EngineMeanPath.h"			
#include "EngineVirtualDomain.h"	
#include "V5Database.h"



#if DEF_WINDOWS		// WINDOWS include
	#include <windows.h>
	#include <sys/stat.h>
	#include "Windnr.h"
	#include "Winmain.h"
	#include "resource.h"
#endif				
#if DEF_UNIX		// UNIX include
	#include <sys/stat.h>
	#include "unistd.h"
	#include "main.h"
#endif


/*
	10.4.1    400 Bad Request .........................................65
	10.4.2    401 Unauthorized ........................................66
	10.4.3    402 Payment Required ....................................66
	10.4.4    403 Forbidden ...........................................66
	10.4.5    404 Not Found ...........................................66
	10.4.6    405 Method Not Allowed ..................................66
	10.4.7    406 Not Acceptable ......................................67
	10.4.8    407 Proxy Authentication Required .......................67
	10.4.9    408 Request Timeout .....................................67
	10.4.10   409 Conflict ............................................67
	10.4.11   410 Gone ................................................68
	10.4.12   411 Length Required .....................................68
	10.4.13   412 Precondition Failed .................................68
	10.4.14   413 Request Entity Too Large ............................69
	10.4.15   414 Request-URI Too Long ................................69
	10.4.16   415 Unsupported Media Type ..............................69
	10.4.17   416 Requested Range Not Satisfiable .....................69
	10.4.18   417 Expectation Failed ..................................70
	10.5  Server Error 5xx ............................................70
	10.5.1   500 Internal Server Error ................................70
	10.5.2   501 Not Implemented ......................................70
	10.5.3   502 Bad Gateway ..........................................70
	10.5.4   503 Service Unavailable ..................................70
	10.5.5   504 Gateway Timeout ......................................71
	10.5.6   505 HTTP Version Not Supported ...........................71
*/

static	long pageHashs[256] = { 0 };
static	long downloadHashs[256] = { 0 };
static	long audioHashs[256] = { 0 };
static	long videoHashs[256] = { 0 };

static	char *pageStrings[256];
static	char *downloadStrings[256];
static	char *audioStrings[256];
static	char *videoStrings[256];

// --------- Engine Global Static Strings to speed up database string access -------------------- //

#include "Statdefs.h"

char *protocolStrings[] = {
	"Unknown", "web", "secureweb", "mail", "ftp", "telnet", "realaudio", "dns", "pop3",	0 };

static char RootLevelStr[] = "root-level";

char statusStr[200] = { "\0" };

static char s_NoReferrerStr[]=			"[NO REFERRAL]";
static char s_BookmarkReferrerStr[]=	"[BOOKMARKED]";
static char s_UnknownStr[]=				"[UNKNOWN]";

static const char s_V5DBModeMismatchStr[]=	"An extended database created in %s mode cannot be processed in %s mode!";
static const char s_WebStr[]=				"web";
static const char s_StreamingStr[]=			"streaming";

long			allTotalDatabaseRequests;
long			allTotalRequests;
long			allTotalFailedRequests;
long			allTotalCachedHits;
long			allTotalRedirectedHits;
long			allTotal404Hits;

__int64			allTotalBytes,
				allTotalBytesIn;
long			allfirstTime,
				allendTime;
static	long	badones;
long			totalDomains;	/* total virtual domains processed */

static	long	ignoredbyDBDateRange;
static	long	ignoredbyDateRange;
static	long	ignoredbyZeroBytes;
static	long	ignoredbyFilterIn;
static	long	noclientsExist;
__int64			gFilesize;

short			logDelim;
short			prevLogDelim;
short			logNumDelimInLine;
time_t			logDays = 0;
long			logIdx = 0;
static long		logDateStart,
				logDateEnd;
int			 	endall;


static long		db_starttime,
				db_endtime;
long			logfiles_opened;

static long		logRef[ MAX_LOGFILES+10 ];	// open log file handles corresponding to the "log history" files

// Made the following variables file static to avoid having to pass too many params to GetLineDetails() etc
static time_t s_startingDate_t, s_endingDate_t;		// standard date range to process variables
static CQV5Database* s_v5ReaderDB=0;				// the V5 database from which we're currently reading hits

// *********************
// Forwards.
// *********************

//=================================================================================================
//								P R I V A T E   F U N C T I O N S
//=================================================================================================

static void GenerateExtPointers( void )
{
	long lp;

	// Clear Local String pointers
	for(lp=0;lp<256;lp++){
		pageStrings[lp] = 
		downloadStrings[lp] = 
		audioStrings[lp] = 
		videoStrings[lp] = NULL;

		pageHashs[lp] = 0;
		downloadHashs[lp] = 0;
		audioHashs[lp] = 0;
		videoHashs[lp] = 0;
	}

	for(lp=0; MyPrefStruct.pageStr[lp][0]; lp++ )
		pageStrings[lp] = MyPrefStruct.pageStr[lp];
	
	for(lp=0; MyPrefStruct.downloadStr[lp][0]; lp++ )
		downloadStrings[lp] = MyPrefStruct.downloadStr[lp];

	for(lp=0; MyPrefStruct.audioStr[lp][0]; lp++ )
		audioStrings[lp] = MyPrefStruct.audioStr[lp];

	for(lp=0; MyPrefStruct.videoStr[lp][0]; lp++ )
		videoStrings[lp] = MyPrefStruct.videoStr[lp];
}

	
static void ShowClientsETA( long clientnum, long total, double timesofar )
{
	if( timesofar ) {
		double eta=0, tt;
		long percent;
		timesofar = timems() - timesofar;
		percent = clientnum*1000/total;
		if( percent )
			tt = timesofar*1000/percent;
		eta = tt - timesofar;

		if( eta ){
			char etaStr[256];
			etaStr[0]=0;

			if( eta < 60 )
				sprintf( etaStr, "%2d sec Time remaining", ((int)eta)%60);
			else
				sprintf( etaStr, "%2d:%02d Time remaining", ((int)eta/60)%60, ((int)eta)%60);

#ifdef DEF_MAC
			ShowProgressDetail (percent, TRUE, NULL, 0);
#else			
			ProgressChangeMainWindowTitle( percent/10, etaStr );

			sprintf( etaStr, "Processing client %d/%d ..." , clientnum, total );
			StatusWindowSetProgress( percent, etaStr );
#endif
		}
	}
}


// **************************************************************************
// Regenerate the Meanpath report from the Client data.
// **************************************************************************
static void PostCalcMeanpathdata(VDinfoP VDptr)
{
	DEF_ASSERT(VDptr);
	DEF_ASSERT(VDptr->byClient);

	for	(int i=0; i<VDptr->byClient->num; i++)
	{
		Statistic*	pStat	= VDptr->byClient->GetStat( i );

		if (!pStat)
		{
			DEF_ASSERT(pStat != NULL);
			continue;
		}
		if (!pStat->sessionStat)
		{
			// NOTE: It's actually valid, in the case of the "-Rare Visitors-" client
			//		 that is added by flushing mode, to have a null sessionStat value
			//		 so we do not need to DEF_ASSERT here - ZT.
			continue;
		}

		long lHashId;
		for (int j=0;j<pStat->sessionStat->GetNum(); ++j)
		{
			lHashId = pStat->sessionStat->GetSessionPage(j);
			if (lHashId == SESS_START_PAGE)
			{
				pStat->sessionStat->SessionLen = 0;
			}

			DEF_ASSERT(lHashId != -1);
#if DEF_DEBUG
			if (lHashId < SESS_START_PAGE || lHashId > SESS_ABOVE_IS_PAGE)
			{
				char*	LookupPageHashId(VDinfoP VDptr, long nHashId);
				char*	szURL		= LookupPageHashId(VDptr, lHashId);
				if (szURL == NULL)
				{
					DEF_ASSERT(szURL != NULL);
					continue;
				}
				DEF_ASSERT(HashStr(szURL, 0) == lHashId);
			}
#endif

			if (lHashId != -1)
			{
				if (lHashId < SESS_START_PAGE || lHashId > SESS_ABOVE_IS_PAGE)
					++(pStat->sessionStat->SessionLen);
				pStat->sessionStat->RecordMeanPath(lHashId, j);
			}
		}
	}
}


//--------------------------------------------------------------------------------------
// Sets the various statistic counters that are displayed by the Pages Containing Broken
// Link ('pcbl') report's main table.  We iterate over the stats within either 
// virtualDomain.byBrokenLinkReferal (if doInternalReferals==false) or virtualDomain.
// byIntBrokenLinkReferal (if doInternalReferals==true) and for each one, find the
// matching virtualDomain.byRefer stat and copy its values across. 
//--------------------------------------------------------------------------------------
static void PostCalcBrokenLinkReferals( VDinfo& virtualDomain, bool doInternalReferals )
{
	StatList* pBrokenLinkReferals=doInternalReferals ?
								  virtualDomain.byIntBrokenLinkReferal : 
								  virtualDomain.byBrokenLinkReferal;
	DEF_ASSERT(pBrokenLinkReferals);								  

	StatList* pReferals=virtualDomain.byRefer;
	DEF_ASSERT(pReferals);								  

	// reset relevant totals
	pBrokenLinkReferals->totalRequests=0;
	pBrokenLinkReferals->totalCounters=0;
	pBrokenLinkReferals->totalCounters4=0;

	size_t numBrokenLinkReferals( static_cast<size_t>(pBrokenLinkReferals->GetStatListNum()) );

	// for each broken link referal...
	for( size_t i(0); !IsStopped() && i<numBrokenLinkReferals; i++ )
	{
		// ref current broken link referal stat
		Statistic* pBrokenLinkReferal=pBrokenLinkReferals->GetStat(i);					
		// ref corresponding referal stat
		Statistic* pReferal=pReferals->FindHashStat( pBrokenLinkReferal->GetHash() );
		DEF_ASSERT(pReferal);

		// copy requests counter & total
		pBrokenLinkReferal->files=pReferal->files;										
		pBrokenLinkReferals->totalRequests+=pBrokenLinkReferal->files;

		// ref relevant FailedRequestInfo for referal page
		const CQFailedRequestInfo& failReqInfo=virtualDomain.GetFailedRequestInfo( pReferal );

		// copy unique visitors counter & inc total
		pBrokenLinkReferal->counter=failReqInfo.getNumUniqueVisitors();
		pBrokenLinkReferals->totalCounters+=pBrokenLinkReferal->counter;

		// copy num broken links counter & inc total
		pBrokenLinkReferal->counter4=failReqInfo.getNumFailedRequests();
		pBrokenLinkReferals->totalCounters4+=pBrokenLinkReferal->counter4;
	}
}


//-----------------------------------------------------------------------------------------
// Performs any post-processing that's required.  Returns true on error, otherwise false.
//-----------------------------------------------------------------------------------------
static short PostCalcDatabases( VDinfoP VDptr, long logStyle )
{
	// loop through all clients and do calculations
	if( VDptr->byClient )
	{
		long clientnum, maxclients, percent, lastpercent=0;
		double	time1;
		double	lastUpdate;
		double	now;
		Statistic *p;

		InitOtherStatlists( VDptr );
		maxclients = VDptr->byClient->GetStatListNum();
		lastUpdate = time1 = timems();

#if DEF_MAC
		ShowProgressDetail (1000, TRUE, NULL, 0);
		if (MyPrefStruct.dnslookup)
			StatusWindowSetMessage ("Processing Visitor DNS lookups ...");
#else
		StatusSetID( IDS_PROCESSCLIENTS );
#endif

		if( MyPrefStruct.dnslookup )
		{
			PostCalcResolveClients( VDptr, logStyle );
		}
		
		
		// clear all ->lastday variables to make sure the session calc is clean
		ClearLastday( VDptr->byPages );

		for ( clientnum=0; clientnum < maxclients && !IsStopped(); clientnum++ )
		{

			p = VDptr->byClient->GetStat( clientnum );

			percent = (100*clientnum/maxclients);
			now = timems();
			if( (now - lastUpdate) > .10 ) 
			{
				lastUpdate = now;
				MACOS_PROGRESS_IDLE
				ShowClientsETA( clientnum, maxclients, time1 );
			}
			lastpercent = percent;

			// only if the client has a name and is really valid then add its details to other lists.
			if( p )
			{
				if( p->GetName() )
				{
					AddClientDetailsToOthers( VDptr, p, clientnum );
				}
			}
		}

// Err why?
#if DEF_MAC
		StatusWindowSetText( "" );
#endif
	}


	// *********************************************************
	// Regenerate the Meanpath report from the Client data.
	// *********************************************************
	if	( !IsStopped() && VDptr && VDptr->byClient && STAT(MyPrefStruct.stat_meanpath))
	{
		PostCalcMeanpathdata(VDptr);
	}


	//--------------------------------------------------------------------------------------
	// Set the various statistic counters that are displayed by the Pages Containing Broken
	// Link ('pcbl') report's main table.
	//--------------------------------------------------------------------------------------
	if( !IsStopped() && VDptr->byBrokenLinkReferal && VDptr->byRefer )
	{
		// always process external/all broken link referals
		PostCalcBrokenLinkReferals( *VDptr, false );

		// if we have internal broken link referals
		if( VDptr->byIntBrokenLinkReferal )
		{
			// process those as well
			PostCalcBrokenLinkReferals( *VDptr, true );
		}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------------------

// this will keep all parameters in the url that are defined in the config
static long RetainURLParam( char *url, size_t len )
{
	char varname[256];
	char *newurl = new char [len+1];
	char *retainlist, *var;
	long i = 0, d=0, varindex = 0, done=0;

	retainlist = MyPrefStruct.retain_variablelist;
	while( retainlist ){
		// reached end of variable, so look for contents
		if( retainlist[i] == ',' || !retainlist[i] )
		{
			if( !retainlist[i] ) retainlist = NULL;
			else
			if( retainlist[i] == ' ' ) i++;

			varname[d++] = '=';
			varname[d++] = 0;
			var = mystrstr( url, varname );
			
			if( var ){
				char c = '&';
				if( var != url )		// read char before variable to verify that its a variable
					c = *(var-1);

				// double check to see if the variable found IS really valid , and not just
				// a parameter with in another variable.
				if( c == '&' )
				{
					if( varindex ) // if its the second variable add the ampersand
						newurl[varindex++] = '&';
					// copy variable contents
					varindex += strcpyuntil( newurl+varindex, var, '&' );
					varindex--;
					done++;
				}
			}
			d = 0; // restart at begining of varname[]
			i++;
		} else // Copy variable to use into temp string
		{
			varname[d++] = retainlist[i++];
		}
	}
	if( varindex ){
		strcpy( url, newurl );
	}
	delete [] newurl;

	return done;
}




static long CloseAndQuitProcessing( ProcessDataPtr logdata )
{
	long lp, VDcount, time1;
	VDinfo *VDptr;
	char **fsFile=logdata->fsFile;
	int numLogs=logdata->logNum;

	// It has been stopped, close all files to free memory
	if( IsStopped() || (MyPrefStruct.live_sleeptype > 0)) {
		for ( lp=0; lp<numLogs; lp++) {
			LogClose( logRef[lp], fsFile[lp] );
			logRef[lp] = 0;
		}
	}
	StatusSetID(IDS_CLEAN);
	time1 = GetCurrentCTime();

	/* delete all VDptr's data structs and records */
	for( VDcount=0; VDcount <= VDnum; VDcount++){
		VDptr = VD[ VDcount ];
		if( VDptr ){
			OutDebugs( "%ld: Cleaning up domain %s", GetCurrentCTime(), VDptr->domainName );
			CloseVirtualDomain( VDptr, logStyle );
		}
	}
	time1 = GetCurrentCTime() - time1;
	OutDebugs( "Time taken to clear %d Hosts = %d sec", VDnum, time1 );


	OutDebugs( "Freeing DNS cache" );
	/* free DNS cache if allocated */
	if (MyPrefStruct.dnslookup) {
		CloseDNRDriver();
		ClearAllDNR();
		delete addressList;
	}

	logdata->ShowProgressDetail(-1*10, 0, 0, 0);
	return IsStopped();
}


//-----------------------------------------------------------------------------------------
// Creates the first virtual domain entry within the VD[] array.  Uses site URL for name
// of the domain if specified within settings, otherwise uses <logFileName> param.  This 
// method would not normally be called if virtual domain by log file is turned on.
//-----------------------------------------------------------------------------------------
static VDinfoP CreatePrimaryVirtualDomain( const char* logFileName )
{
	DEF_PRECONDITION( !VDnum );

	char* domainName;
	if( MyPrefStruct.siteurl[0] )
	{
		char szStrippedSiteUrl[DOMAINNAME_LENGTH+1]="\0";
		stripReferURL( MyPrefStruct.siteurl, szStrippedSiteUrl );
		CorrectVirtualName( szStrippedSiteUrl, buf2, 0 );
		domainName=buf2;
	}
	else
	{
		domainName=FileFromPath( logFileName, 0 );
	}
	return InitVirtualDomain( VDnum, domainName, logStyle );
}


#ifdef DEF_FULLVERSION
//-----------------------------------------------------------------------------------------
// Creates and initialises a V5 database object.  The <fileName> param specifies the
// database's file name.  If the <initForReading> param is true then the database will be 
// initialised for reading over the read range specified by the two file statics
// <s_startingDate_t> and <s_endingDate_t>.
// If the function succeeds then the newly created database object will be returned,
// otherwise zero will be returned.
// NOTE: The V5 database object is created on the heap via the new operator and it's the
// responsibility of the caller to ensure that it gets deleted.
//-----------------------------------------------------------------------------------------
static CQV5Database* LoadV5Database( const char* fileName, bool initForReading )
{
	static const char errorStr[]="Cannot open database file:\n\n%s\n\n%s";

	CQV5Database* v5Database=new CQV5Database;
	if( !v5Database->init( fileName ) )
	{
		ErrorMsg( errorStr, "An error occurred whilst loading!", v5Database->fileName().c_str() );
		delete v5Database;
		return 0;
	}

	if( MyPrefStruct.streaming && v5Database->type()==CQV5Database::Web )
	{
		char buffo[200];
		sprintf( buffo, s_V5DBModeMismatchStr, s_WebStr, s_StreamingStr );
		ErrorMsg( errorStr, v5Database->fileName().c_str(), buffo );
		delete v5Database;
		return 0;
	}

	if( !MyPrefStruct.streaming && v5Database->type()==CQV5Database::Streaming )
	{
		char buffo[200];
		sprintf( buffo, s_V5DBModeMismatchStr, s_StreamingStr, s_WebStr );
		ErrorMsg( errorStr, v5Database->fileName().c_str(), buffo );
		delete v5Database;
		return 0;
	}

	if( v5Database && initForReading )
	{
		if( v5Database->beginRead( &gFilesize, s_startingDate_t, s_endingDate_t+ONEDAY ) )
		{
			SetPosIndex(0);		// for logs, the log_io.cpp read functions would normally do this
			logType=LOGFORMAT_V5DATABASE;
		}
		else if( v5Database->status()==CQV5Database::IOErrorOccurred )
		{
			ErrorMsg( "Cannot begin reading database file:\n\n%s\n\nNo database processing will occur!", v5Database->fileName().c_str() );
			delete v5Database;
			return 0;
		}
	}

	return v5Database;
}
#endif // DEF_FULLVERSION


static VDinfoP GetLineDetails( VDinfoP VDptr, char **fsFile, int numLogs, HitDataRec *Line, long *logError, const char* v4DatabaseFileName )
{
	int			LogFileStat = FALSE;
	long i;

	if( VDptr )
	{
		switch( logType )
		{
			case LOGFORMAT_RADIUS:
				LogFileStat = LogReadMultiLine( buff, logRef[logIdx], fsFile[logIdx] );
				break;
			case LOGFORMAT_V5DATABASE:
				// we've just finished reading from a V5 database so we don't do anything here
				break;
			default:
				LogFileStat = LogReadLine( buff, logRef[logIdx], fsFile[logIdx] );
				break;
		}
	}

	if( !LogFileStat )
	{
		// load the database into ram, then continue on as normal, loading in the other log files
		if( !VDptr )
		{
			long file = 0;

			// if an FWA database was specified
			if( v4DatabaseFileName )
			{
				StatusSetID( IDS_LOADDATABASE );
				file = DBIO_Open( v4DatabaseFileName );
				if( !file )
					OutDebugs( "database not opened : %s", v4DatabaseFileName );
			}
			else
			{
				// Check that the current log file might actually be an FWA database (which should be the
				// first if it is).  We do this by calling the appropriate database open function.  If a valid
				// handle is returned then it is indeed an FWA database.
				file = DBIO_Open( fsFile[logIdx] );
				
				// if the log file did turn out to be an FWA database
				if( file )
				{
					DEF_ASSERT(logIdx==0);	// should be first one in list

					++logIdx;	// maintain current log list index
				}
			}

			if (file)
			{
				VDptr=LoadV4Database(file);
			}
			StatusSetID(0);						// clear Status message		(kEmptyString)
		}
		
		// reached end of log, now try 'next' log or end
		if( VDptr )
		{
			// if we haven't just finished reading from a V5 database
			if( logType!=LOGFORMAT_V5DATABASE )
			{
				// close down files in normal mode
				// else leave them open and reread
				if( !MyPrefStruct.live_sleeptype && logRef[logIdx] ) {
					i = LogClose( logRef[logIdx], fsFile[logIdx] );
					logRef[logIdx] = 0;
					++logIdx;
				}
			}

			if( MyPrefStruct.multidomains > 0 ) {
				VDptr->totalInDataSize = GetPosIndex();			// make total file size = total read size so far
			} else {
				VDptr->totalInDataSize += GetPosIndex();			// make total file size = total read size so far
			}

			VDptr->time2 = timems() - VDptr->time1;
			if( VDptr->time2 == 0 ) VDptr->time2=1;
		}
		// continue with next log file...
		if( logIdx < numLogs )
		{
#ifdef DEF_FULLVERSION
			// if current file is a V5 database then open it now and assign our file static s_v5ReaderDB
			if( CQV5Database::isV5Database( fsFile[logIdx] ) )
			{
				DEF_ASSERT(!s_v5ReaderDB);
				logType=LOGFORMAT_UNKNOWN;
				s_v5ReaderDB=LoadV5Database( fsFile[logIdx], true );	// sets logType
				if( !s_v5ReaderDB )
				{
					if( ++logIdx==numLogs )
					{
						endall=TRUE;
					}
					*logError=-1;
					return VDptr;
				}
			}
			// else if there's another log file not open, then start from it
			else
#endif // DEF_FULLVERSION			// else if theres another log file not open, then start from it
			if( logRef[logIdx] == 0 )
			{
				// if we are trying to process a database, then dont let it.
				if( IsFileV4DataBase(fsFile[logIdx]) )
				{
					if( logIdx+1 == numLogs )	// only for last file, force stop.
						endall = TRUE;
					logIdx++;
					*logError = -1;
					return VDptr;
				}
				logRef[logIdx] = LogOpen( fsFile[logIdx], &gFilesize );
				if( logRef[logIdx] == 0 ) {
					if( logIdx+1 == numLogs )	// only for last file, force stop.
						StopAll(CANTOPENLOG);
					logIdx++;
					*logError = -1;
					return VDptr;
				}
				logfiles_opened++;
				logDays = GetTimeFromFilename( fsFile[logIdx] );
				logType	= LOGFORMAT_UNKNOWN;
				LogGetFormat( logRef[logIdx] , Line, fsFile[logIdx] );
			}

			// Virtual HOSTS by log file
			if( MyPrefStruct.multidomains > 0 ) {
				// if we're not about to read from a V5 database (virtual host by log file handled in V5DBGetLineDetails())
				if( logType!=LOGFORMAT_V5DATABASE )
				{
					if( MyPrefStruct.live_sleeptype > 0){
						VDnum++;
						VDptr = VD[VDnum];
						VDptr->time1 = timems();
					} else {
						if( VDnum < MyPrefStruct.multidomains ){
							VDinfoP newvd;
							if( VDptr )
								VDnum++;
							CorrectVirtualName( fsFile[logIdx], buf2, 1 );
							newvd = InitVirtualDomain( VDnum, buf2, logStyle );		/* initialize virt domains info/data */
							if( VDptr ){
								VDptr->next = newvd;
								VDptr = newvd;
							} else
								VDptr = newvd;

							if( !VDptr ){
								StopAll(CANTMAKEDOMAIN);
								*logError = -1;
								return VDptr;
							}
						}
					}
				}
			}
			else if( !VDptr && !(VDptr=CreatePrimaryVirtualDomain( fsFile[logIdx] )))
			{
				StopAll(CANTMAKEDOMAIN);
				*logError = -1;
				return VDptr;
			}

			MemClr( Line, sizeof( HitDataRec ) );

			// set up the filename in our hit structure - useful for many things but especially for the V5 database
			Line->logFileName=FileFromPath( fsFile[logIdx], 0);

			if( numLogs>1 )
				sprintf( statusStr, " File %i of %i", logIdx+1, numLogs );
			else
				*statusStr = 0;
			sprintf( statusStr+strlen(statusStr), " (%s)", Line->logFileName );

#ifdef DEF_MAC
			StatusWindowSetFile (statusStr);
#endif

			// if we're about to read from a V5 database then bail out here
			if( logType==LOGFORMAT_V5DATABASE )
			{
				// the next Kahoona iteration will call V5DBGetLineDetails()
				++logIdx;
				*logError = -1;
				return VDptr;
			}
		}
		else
		{
			// no more log files....go away and write out the report then exit
			endall = TRUE;
			*logError = -1;
			return VDptr;		// nothing more to decode, all done , lets end.
		}
	}

	*logError = ProcessLogLine( buff, Line );

	return VDptr;
}


//-----------------------------------------------------------------------------------------
// Sort of like GetLineDetails() but handles reading from a V5 database.  Reads hits from
// the database specified by our file static <s_v5ReaderDB>, which must be valid and have
// all ready been opened.  The other parameters are as for good old GetLineDetails().  This
// function handles virtual host by log file for a V5 database, which is the primary reason
// for it's existence.
//-----------------------------------------------------------------------------------------
static VDinfoP V5DBGetLineDetails( VDinfoP VDptr, HitDataRec& Line, long& logError )
{
	DEF_PRECONDITION(logType==LOGFORMAT_V5DATABASE);
	DEF_PRECONDITION(s_v5ReaderDB);

	size_t bytesRead;	// num bytes of real data read from database

	// if we can successfully read the next hit from the V5 database
	if( s_v5ReaderDB->read( Line, &bytesRead ) )
	{
		logError=LOGLINE_CORRECT;
		AddPosIndex( bytesRead );	// for logs, the log_io.cpp read functions would normally do this

		// if virtual domains by log file is turned on
		if( MyPrefStruct.multidomains>0 && VDnum<MyPrefStruct.multidomains && Line.logFileName )
		{
			DEF_ASSERT(!MyPrefStruct.live_sleeptype);	// don't know what to do about this for V5 db and don't care

			// NOTE: Because a V5 database stores hits sorted in time order rather than log file order, we need to use
			// the FindVirtualDomain() way of implementing virtual domain by log file.  If we did it the same way as
			// for normal logs, then we'd end up with a hell of a lot of virtual domains being created for each log
			// stored within the database.

			// if we can't find the domain for the current log...
			CorrectVirtualName( const_cast<char*>(Line.logFileName), buf2, 1 );
			VDinfoP currVDptr=FindVirtualDomain( buf2 );
			if( !currVDptr )
			{
				// then create it now...
				if( VDptr )
				{
					++VDnum;
				}
				if( !(currVDptr=InitVirtualDomain( VDnum, buf2, logStyle )) )
				{
					StopAll(CANTMAKEDOMAIN);
					logError = -1;
					return VDptr;
				}
				if( VDptr )
				{
					VDptr->next=currVDptr;
				}
			}
			VDptr=currVDptr;
		}
	}
	else
	{
		if( s_v5ReaderDB->status()==CQV5Database::IOErrorOccurred )
		{
			ErrorMsg( "Could not read database file:\n\n%s\n\nNo more database processing will occur!", s_v5ReaderDB->fileName().c_str() );
		}

		logError=-1;	// indicate that there aint any more hits to read
	}

	return VDptr;
}


#ifdef DEF_MAC
// This should be done inside the mac somehow via call backs in the LogData struct.
static void ShowFileOfStatus( long filex )
{
	char status[64];
	sprintf(status, "File 1 of %i", filex);
	StatusWindowSetFile (status);
}
#endif


// The mac should support logdata-> call backs, makes life easier
static long ShowProcessStatus( VDinfoP VDptr, __int64 pos, __int64 totalsize, double starttime, ProcessDataPtr logdata )
{
	long progressLevel( totalsize ? static_cast<long>(1000*pos/totalsize) : 0 );	// a progress value between 0 and 1000

#ifdef DEF_MAC
	ShowProgressDetail( progressLevel, TRUE, NULL, starttime );
#else
	logdata->ShowProgressDetail( progressLevel, allTotalRequests, statusStr, starttime );
#endif

	return pos;
}


//-----------------------------------------------------------------------------------------
// This structure is used by FilterAndSortProcessingQueue() to sort the log processing 
// queue according to log start time. 
//-----------------------------------------------------------------------------------------
struct ProcessingQueueSortItem
{
	char*	m_fileName;
	time_t	m_startTime;

	ProcessingQueueSortItem( char* fileName, time_t startTime ) : m_fileName(fileName), m_startTime(startTime) {}
	bool operator<( const ProcessingQueueSortItem& rhs ) const { return m_startTime<rhs.m_startTime; }
};


//-----------------------------------------------------------------------------------------
// This function first removes invalid combinations of V4 & V5 databases that the user may
// be attempting to reprocess.  This is done taking any database specified via settings into
// consideration.  Once this is done, the remaining list of files to be processed is sorted
// according to start time.  The <queue> param is the log processing queue that gets sorted
// by this function.  The <v4DatabaseFileName> param, which may be null, is the V4 database
// as specified by the settings.
// Returns the resulting number of files to be processed that remain within <queue>.
//-----------------------------------------------------------------------------------------
static int FilterAndSortProcessingQueue( std::vector<char*>& queue, const char* v4DatabaseFileName )
{
	size_t queueSize( queue.size() );
	std::vector<ProcessingQueueSortItem> sortingQueue;
	sortingQueue.reserve(queueSize);
	
	bool v4DBFound(false);	// true if a V4 database has been found in queue

	for( size_t i(0); i<queueSize; ++i )
	{
		long type;
		time_t startTime;
		time_t endTime;
		LogSubFormat logSubFormat;

		char* fileName=queue[i];
		
		GetLogFileType( fileName, &type, (long*)(&startTime), (long*)(&endTime), &logSubFormat );
		
		bool addToQueue(false);	// true if we should process the current file

		static const char s_ignoreMessage[]="Ignoring database file:\n\n%s\n\n%s";

		if( type==LOGFORMAT_V4DATABASE )
		{
			std::string preproV4DBFileName( CQV5Database::preprocessFilename( fileName, false ) );
			DEF_ASSERT( preproV4DBFileName.size() ); 

			if( MyPrefStruct.clusteringActive )
			{
				NotifyMsg( s_ignoreMessage, preproV4DBFileName.c_str(),
				           "A compact database cannot be reprocessed when cluster analysis is enabled." );
			}
			else if( v4DatabaseFileName )
			{
				NotifyMsg( s_ignoreMessage, preproV4DBFileName.c_str(),
						   "A compact database is already specified within your settings." );
			}
			else if( v4DBFound )
			{
				NotifyMsg( s_ignoreMessage, preproV4DBFileName.c_str(),
					       "Only a single compact database may be reprocessed at a time." );
			}
			else
			{
				addToQueue=true;
				v4DBFound=true;
				startTime=-1;	// ensure gets processed first after sorting
			}
		}
		else if( type==LOGFORMAT_V5DATABASE )
		{
#ifdef DEF_FULLVERSION
			std::string preproV5DBFileName( CQV5Database::preprocessFilename( fileName ) );
			DEF_ASSERT( preproV5DBFileName.size() );

			if( logSubFormat==LOGSUBFORMAT_V5DATABASE_WEB && MyPrefStruct.streaming )
			{
				char buffo[200];
				sprintf( buffo, s_V5DBModeMismatchStr, s_WebStr, s_StreamingStr );
				NotifyMsg( s_ignoreMessage, preproV5DBFileName.c_str(), buffo );
			}
			else if( logSubFormat==LOGSUBFORMAT_V5DATABASE_STREAMING && !MyPrefStruct.streaming )
			{
				char buffo[200];
				sprintf( buffo, s_V5DBModeMismatchStr, s_StreamingStr, s_WebStr );
				NotifyMsg( s_ignoreMessage, preproV5DBFileName.c_str(), buffo );

			}
			else if( MyPrefStruct.clusteringActive && queueSize>1 )
			{
				NotifyMsg( s_ignoreMessage, preproV5DBFileName.c_str(),
						   "An extended database must not be reprocessed with other logs when cluster analysis is enabled." );
			}
			else if( MyPrefStruct.database_active && MyPrefStruct.database_extended && *MyPrefStruct.database_file &&
					 CQV5Database::preprocessFilename( MyPrefStruct.database_file )==preproV5DBFileName
					 )
			{
				NotifyMsg( s_ignoreMessage, preproV5DBFileName.c_str(),
						   "This database is already specified within your settings." );
			}
			else
			{
				addToQueue=true;
			}
#endif // DEF_FULLVERSION
		}
		else
		{
			addToQueue=true;
		}

		if( addToQueue )
		{
			sortingQueue.push_back( ProcessingQueueSortItem( fileName, startTime ) );
		}
	}

	queueSize=sortingQueue.size();

	queue.resize( queueSize );

	if( queueSize )
	{
		std::stable_sort( sortingQueue.begin(), sortingQueue.end() );

		for( size_t j(0); j<queueSize; ++j )
		{
			queue[j]=sortingQueue[j].m_fileName;
		}
	}

	return static_cast<int>( queueSize );
}


/********************************************************************************


Main Proccessor Brain

********************************************************************************/

#define	REFERRAL_IS_EXTERNAL		1
#define	REFERRAL_IS_LOCAL			2


static long PerformLogFileStatistics( ProcessDataPtr logdata,				// main source of information
									  const char*	 v4DatabaseFileName,	// name of V4 FWA database file to use, if any
									  bool			 produceReport			// create output report if true
									  )
{
	int				isReverse,
					doreferral	= FALSE;
	bool			dopage		= FALSE,
					dodownload	= FALSE,
					domedia		= FALSE,
					long_addr	= TRUE,
					outstat		= FALSE;
	Statistic		*currentPage = NULL,
					*currentOper = NULL,
					*currentBrow = NULL;
	double			alltime1, alltime2, lastUpdate = 0;		// status update time
	char			*pt=0, *country=0;
	char			tmpDate[32];
	long			logTime = 0, //bytes,
					nowTime = GetCurrentCTime();
	long			editNum, nextDumpCount, nextClientCount,
					statsDrillFlag=0,	i, currentPageHash, 
					VDcount,live_count;
	long			logError;
	long 			statcode;	// status code, 200,404 stuff
	long			errorcode;	// only status codes >304 or what is classified really as an error are really put into here.
	__int64			BytesIn, BytesOut;
	long			defaultindexLen = 0 , siteurlLen;
	char			defaultindex[256], *defaultindexarray[32];
	register VDinfoP	VDptr=0;			/* current virt domain pointer */
	VDinfoP			oldVDptr=0;
	struct tm		logDate;
	HitDataRec		Line;

	////
	// Make a copy of our list of logs to process and then sort by time whilst filtering out
	// any invalid databases that the user may be attempting to reprocess.  This ensures that all
	// logs get processed in the correct order which in turn ensures that sessions don't get all
	// fucked up due to processing logs out of order.
	////
	std::vector<char*> processingQueue( logdata->fsFile, logdata->fsFile+logdata->logNum );
	int	numLogs=FilterAndSortProcessingQueue( processingQueue, v4DatabaseFileName );
	if( numLogs==0 )
	{
		NotifyMsg( "No more valid files left to process!" );
		return 0;
	}
	char** fsFile= &(*processingQueue.begin());

	// stop leakage of last client from last report into this report for the case where 
	// the last report contained clients but we don't actually find any clients for this report
	rev_domain[0]=0;

	// NOTE: all future references should be logdata->prefs-> instead of MyPref
	siteurlLen = mystrlen( MyPrefStruct.siteurl );

	// Set the start and end times for the date ranges
	
	if (MyPrefStruct.alldates == DATE_SPECIFY)	// "Custom" date item
	{
		s_startingDate_t = MyPrefStruct.startTimeT;
		s_endingDate_t = MyPrefStruct.endTimeT;
	} else
		DatetypeToDate (MyPrefStruct.alldates, &s_startingDate_t, &s_endingDate_t);
	
	// Initialize DEFAULT INDEX array and values.
	if( MyPrefStruct.defaultindex[0] )
	{
		defaultindexLen = mystrcpy( defaultindex, MyPrefStruct.defaultindex );
		i  = 0;
		pt = defaultindexarray[i] = defaultindex;
		i++;
		while( *pt && i<32 )
		{
			if( *pt == ',' )
			{
				*pt++ = 0;
				if( *pt == ' ' ) pt++;
				defaultindexarray[i] = pt;
				i++;
			} else
				pt++;
		}
		defaultindexarray[i] = 0;
		i++;
	} else
		defaultindexarray[0] = 0;


	MemClr( &Line, sizeof( HitDataRec ) );
	MemClr( &logDate, sizeof( struct tm ) );

	for( i=0; i<MAX_DOMAINS; i++)
		VD[i] = 0;
	for( i=0; i<MAX_LOGFILES; i++)
		logRef[i] = 0;

	GenerateExtPointers();

	logStyle = 1;

	logDateEnd = 1;
	logDateStart = 1<<30;		// real big number sometime in 2050 or whatever...
	db_starttime = 0;
	db_endtime = 0;

	logIdx = VDnum = live_count = badones = 0;

	//reset global variables incase of second run through
	stopall = endall		= FALSE;
	logType					= LOGFORMAT_UNKNOWN;
	logDelim				= 0;
	allTotalRequests		= 0;
	allTotalFailedRequests	= 0;
	allTotalCachedHits		= 0;
	allTotalRedirectedHits	= 0;
	allTotal404Hits			= 0;
	allTotalBytes			=
	allTotalBytesIn			= 0;
	allfirstTime			=
	allendTime				=
	totalDomains			= 0;

	allTotalDatabaseRequests = 0;

	
	ignoredbyDBDateRange =
	ignoredbyDateRange =
	ignoredbyZeroBytes =
	ignoredbyFilterIn = noclientsExist = 0;

	logfiles_opened = 0;

	editNum	= GetTotalEditPath( MyPrefStruct.EditPaths );

	if( MyPrefStruct.session_timewindow == 0 )
		MyPrefStruct.session_timewindow = 10;

	char	szStrippedSiteUrl[DOMAINNAME_LENGTH+1];
	stripReferURL(MyPrefStruct.siteurl, szStrippedSiteUrl);

	sprintf( buf2 , "Processing %d logs", numLogs );
	StatusSet( buf2 );

#ifdef DEF_MAC
	ShowProgressDetail( 0, TRUE, buf2, 0 );
#endif

#ifndef	DEF_FULLVERSION
	MyPrefStruct.multivhosts &= 0x0f;
#endif

	/* init DNS lookups */
	if (MyPrefStruct.dnslookup) {
		addressList = new CQDNSCache(kDNSListIncr,FALSE);		/* initialize DNS lookup cache table */
		if (!addressList)
			MemoryError( "Process(), new CQDNSCache(kDNSListIncr,FALSE);", kDNSListIncr );

		addressList->LoadLookups();

#if defined(DEF_WINDOWS) || defined(DEF_MAC)
		// these guys aren't used for these platforms at the moment
		logdata->dns_server=0;
		logdata->dns_timeout=0;
		logdata->dns_retries=0;
#endif // defined(DEF_WINDOWS) || (defined DEF_MAC)

		// most of these parameters areonly used for linux now, but feel free to use em for mac
		if( OpenDNRDriver( logdata->dns_server, logdata->dns_timeout, logdata->dns_retries, MyPrefStruct.dnsAmount ) ) { //also sets asyncDNR
			UserMsg( ReturnString(IDS_NETINIT_ERR));
			return CloseAndQuitProcessing( logdata );
		}

		ClearAllDNR();
	}

#ifdef DEF_MAC
	ShowFileOfStatus( numLogs );
#endif
	alltime1 = timems();

	VDptr = NULL;

	// ------------------------------------------------
	// Init output log file thats being created.
	if( logdata->convert_Flag == CONVERT_TOW3C )
	{
		char logFile[256], *p;
		strcpy( logFile, MyPrefStruct.outfile );
		if( p = strrchr( logFile, '.' ) )
			strcpy( p, ".log" );
		Write_W3C( logFile, NULL );
	}

	nextDumpCount = 60000;
	nextClientCount = 4000;

	long phyPerc = 80;

	//------------------------ BEGIN V5 DATABASE INITIALISATIONS --------------------------------

	DEF_ASSERT(!s_v5ReaderDB);
	CQV5Database* v5AdderDB=0;	// the V5 database to which we'll be adding hits

#ifdef DEF_FULLVERSION
	// if a V5 database has been specified within settings then load it up now
	if( MyPrefStruct.database_active && MyPrefStruct.database_extended && *MyPrefStruct.database_file ) 
	{
		// if we're reprocessing a V4 database then load the muther and then
		// remove it from the processing queue
		if( IsFileV4DataBase( fsFile[0] ) )	// will be first file in processing queue - see FilterAndSortProcessingQueue()
		{
			DEF_ASSERT( !VDptr );

			StatusSetID( IDS_LOADDATABASE );
			long file( DBIO_Open( fsFile[0] ) );
			if( file )
			{
				VDptr=LoadV4Database( file );
			}
			StatusSetID(0);						// clear Status message

			++fsFile;		// pretend it was never in the processing queue - no one will ever know...
			--numLogs;
		}

		// create V5 database
		bool readFromDB( produceReport && !MyPrefStruct.database_excluded );	// true if we should read hits from the V5 database
		v5AdderDB=LoadV5Database( MyPrefStruct.database_file, readFromDB );

		// if we were able to initialise the database for reading
		if( readFromDB && v5AdderDB && v5AdderDB->status()==CQV5Database::Reading )
		{
			// set our file static so that the database is read from by the Kahoona loop
			s_v5ReaderDB=v5AdderDB;		

			// set progress status message
			sprintf( statusStr, " (%s)", FileFromPath( v5AdderDB->fileName().c_str(), 0 ) );

			// if we didn't just load a V4 database and if virtual domains by log file is NOT turned on
			if( !VDptr && MyPrefStruct.multidomains==0 )
			{
				VDptr=CreatePrimaryVirtualDomain( s_v5ReaderDB->fileName().c_str() );
				if( !VDptr )
				{
					StopAll(CANTMAKEDOMAIN);
				}
			}
		}
	}
#endif // DEF_FULLVERSION

	//------------------------ END V5 DATABASE INITIALISATIONS --------------------------------

while_proc:
	// ----------------------------- MAIN KAHOONA MUTHER LOOP ----------------------------------------------------
	while ( (!endall) && (!IsStopped())) {

		//-------------- FUNKY STREAMING DATA FLUSHING STUFF --------------------------------
		// In realtime /streaming mode , flush out data too old or irrelevent every (50000 lines/2000 clients) to keep
		// down the memory usage and to keep up the speed.
#ifdef	DEF_FULLVERSION
		if( MyPrefStruct.streaming && VDptr )
		{
			if( VDptr->byClient )
			{

				if( VDptr->byClient->GetStatListNum() > nextClientCount || VDptr->totalRequests > nextDumpCount ) {
					nextClientCount = VDptr->byClient->GetStatListNum() + 2000;
					nextDumpCount = VDptr->totalRequests + 50000;

					SortRecords( VDptr, logStyle );

					FlushOldData( VDptr, logDays, TRUE );
				}
			}
		}
		//-------------- FUNKY STREAMING DATA FLUSHING STUFF --------------------------------
#endif

		++Line.lineNum;


		// ------------- READ SOURCE LOG LINE DETAIL ----------------

		// if we're currently reading from a V5 database then do it now baby 
		if( s_v5ReaderDB )
		{
			// read the next hit from s_v5ReaderDB
			VDptr=V5DBGetLineDetails( VDptr, Line, logError );

			// if there's no more to read from the database				
			if( logError!=LOGLINE_CORRECT )
			{
				// if the database was not specified via settings (i.e. is in the log list and is being
				// reprocessed) then delete it now
				if( s_v5ReaderDB!=v5AdderDB )
				{
					delete s_v5ReaderDB;

					// if clustering is turned on
					if( MyPrefStruct.clusteringActive )
					{
						// a V5 database cannot be reprocessed with other files - see FilterAndSortProcessingQueue()
						DEF_ASSERT( numLogs==1 );

						// no more log files....go away and write out the report then exit
						endall = TRUE;
						logError = -1;
					}
				}
		
				s_v5ReaderDB=0;	// we'll start reading logs on the next Kahoona iteration
			}
		}
		else if( MyPrefStruct.clusteringActive && numLogs > 1)
		{
			VDptr=ClusterGetLineDetails( VDptr, fsFile, numLogs, &Line, &logError, v4DatabaseFileName );
		}
		else
		{
			VDptr=GetLineDetails( VDptr, fsFile, numLogs, &Line, &logError, v4DatabaseFileName );
		}
		// ------------- READ SOURCE LOG LINE DETAIL ----------------



		// ------------- STATUS UPDATES -----------------
		if( (timems() - lastUpdate) > .08 ) 
		{
			lastUpdate = timems();
			ShowProcessStatus( VDptr, GetPosIndex(), gFilesize, alltime1, logdata );
		}
		// ------------- STATUS UPDATES -----------------



		// ------------- DECIDE IF TO PROCEED or IGNORE THIS LINE -----------------

		// Check for errors
		if( logError == LOGLINE_ERROR ) {   
			badones++;
			VDptr->badones++;
			ShowBadLine( static_cast<long>(Line.lineNum) );
			continue; 
		} else
		if( logError < LOGLINE_ERROR ) // Not an error, just a blank line or a comment line or something
			continue;

		// If we're processing a V5 database that's been specified via settings (as opposed to being
		// reprocessed) and if we're currently not reading from that database then now's the time to add
		// the current hit to it.  We do this here, instead of lower down, in order to prevent any filter-
		// like settings from stopping the current hit from being added to the database.
		if( v5AdderDB && v5AdderDB!=s_v5ReaderDB )
		{
			v5AdderDB->add( Line );

			// no need to do anything else with the current hit if we're just adding it to a V5 database
			if( !produceReport )
			{
				continue;
			}
		}

		//if bytes =-1 bytes are not logged in this file
		if (Line.bytes == -1) Line.bytes=0;

		if (MyPrefStruct.filter_zerobyte) 
		{
			if (Line.bytes == 0) {
				ignoredbyZeroBytes++;
				continue;
			}
		}

		// Filter out all robots that can be detected........
		if (MyPrefStruct.filter_robots)
		{
			long	botid=0;
			char	*robotString = (char*)InterpretRobotString( Line.agent, &botid);
			if( botid )
				continue;
		}



		if( Line.clientaddr == 0 )
		{
			if( VDptr->byClient ){
				noclientsExist++;
				// process as normal if no client exists.
				//continue;
			}
		}

		/* ------------- convert the time and date strings ------------- */
		if( Line.date )
		{
			StringToDaysDate( Line.date, &logDate, 0);
			StringToDaysTime( Line.time, &logDate);
			Date2Days( &logDate, &logDays );
			if( MyPrefStruct.time_adjust ){
				logDays += MyPrefStruct.time_adjust;
				DaysDateToStruct( logDays, &logDate );
				Date2Days( &logDate, &logDays );
				StructToUSDate( &logDate, tmpDate );
				Line.date = tmpDate;
			}
			Line.ctime = logDays;

			if( logDays < logDateStart )
			{
				logDateStart = logDays;
			}
			else if( logDays > logDateEnd )
			{
				logDateEnd = logDays;
			}

			if( (logDays - nowTime) > (ONEDAY*90) ) {
				ignoredbyDateRange++;
				continue;		// if its too far into the future, then WTF? ignore it
			}

			/* DATES check for time boundaries, and if it is out of boundaries then skip it */
			if( !(MyPrefStruct.alldates==DATE_ALL || ((logDays >= s_startingDate_t) && (logDays < s_endingDate_t+(ONEDAY)))) ){
				ignoredbyDateRange++;
				continue;
			}
			// if we've pre-loaded an FWA database then do not add new data that is within its
			// date range so as not to add duplicated data.
			if( v4DatabaseFileName && ((logDays >= db_starttime) && (logDays <= db_endtime)) ){
				ignoredbyDBDateRange++;
				continue;
			}
		} else {
			DaysDateToStruct( logDays, &logDate );
			DaysDateToString( logDays, tmpDate, 0,'/',TRUE,TRUE);
			Line.date = tmpDate;
		}

		// ------------- DECODE STATUS CODES ----------------
		if( (pt=Line.stat )) 
		{
			if( (*pt == 'O' && pt[1] == 'K') || *pt == '-' )	// Only applies to Webstar logs!
				statcode = 200;
			else
			if( (*pt == 'E' && pt[1] == 'R') )					// Only applies to Webstar logs!
				statcode = 404;
			else
				statcode = myatoi( Line.stat );
		}

		statsDrillFlag = 0; // Make sure this variable is ZERO or ERROR, nothing else, ie it must be reset to default.
		// ------------- DECODE STATUS CODES ----------------



		// handle crap webstar format where it logs the full PATH and not the URL, 
		// we strip out the common path using the defaultindex string if it has a path in it,
		// ie, we dual use the option here.
		if( siteurlLen )
		{
			char c;
			pt = MyPrefStruct.siteurl;
			c = *pt;

			if( c == ':' || c == '/'){
				if( !strcmpd( MyPrefStruct.siteurl, pt ) )
					Line.file = mystrchr( Line.file + siteurlLen,c );
			}
		}
		////////////////////////////////////////////////////////////////////////////



		/* parse filespec (RS2000-01-24)
		 * Also fix up the url, by detecting http:// and also removing port #s, and
		 * optionaly remove or decode the cgi parameters...
		*/
		if (Line.file) 
		{
			register char c = 0;

			// Turn empty or null urls into a "/" - this is also done after RetainURLParam() below (***)
			if( Line.file[0] == 0 || Line.file[0] == ' ' ){
				Line.file[0] = '/';
				Line.file[1] = 0;
			} else
			// if the URL has http:// as the start, take it out
			if( !strcmpd( "http://", Line.file ) ){
				Line.file+=7;
				c = 1;
			}
			pt = Line.file;

			// if a /%7E is found, convert it to a /~   (RS990909)
			if( !strcmpd( "/%7E", pt ) ){
				pt+=2;
				Line.file = pt;
				*pt++ = '/';
				*pt++ = '~';
			}
			//if( c || MyPrefStruct.useCGI==0 ){
			// convert mac paths to unix paths, and erase port #s in urls
			if( c || *pt == ':' || MyPrefStruct.useCGI==0 ){
				for (; c=*pt; pt++) {
					switch( c ){
						case ':': // if a ":81/" port number found, strip it by moving remainder back
							c = pt[1];
							if( !c )
								*pt = 0;
							else
							if( c != '/' ) {
								char *from;

								from = mystrchr( pt, '/' );
								if( from )
									mystrcpy( pt, from );
								else
									*pt = '/';		// convert ':' to '/'
							}
							break;
						case '?':
						case '=':
							if( MyPrefStruct.useCGI==0 ){	//c=='$' || 
								*pt = 0;
								break;			// truncate file names at '?' or '$'
							}
							break;
					}
				}
			}

			// search replace the FULL URL, (not including the host)
			if( MyPrefStruct.searchfor[0] ) {
				// complex search for PATTERN and replace patterned find with NEWSTRING
				if( strchr( MyPrefStruct.searchfor, '*' ) ){
					long ret;
					if( ret=match( Line.file, MyPrefStruct.searchfor, 1 ) ){
						long ch1,ch2;

						ch1 = ret>>16;
						ch2 = (ret&0xff) - ch1;

						mystrncpy( buf2, Line.file + ch1, ch2 );
						buf2[ch2] = 0;

						ReplaceStr( Line.file, NULL, buf2, MyPrefStruct.replacewith, 0 );
					}
				} 
				// simple search for STRING and replace with NEWSTRING
				else {
					ReplaceStr( Line.file, NULL, MyPrefStruct.searchfor, MyPrefStruct.replacewith, 0 );
				}
			}
	
		}


		// ------------- CONVERT OPTIONS ----------------
	 	if( logdata->convert_Flag == CONVERT_TOW3C )
		{
			sprintf( buf2, "%d", statcode );
			Line.stat = buf2;
			/* increment totals for all domains */
			allTotalRequests++;
			/* increment totals for this domain as a whole... */
			VDptr->totalRequests++;
			Write_W3C( NULL, &Line );
			continue;
		}
		// ------------- CONVERT OPTIONS ----------------




		// ---- FILTERS 
		if( PerformFilterCheck( VDptr, &Line ) ){
			ignoredbyFilterIn++;
			continue;
		}


		// ---- Fix username/client details
		if( (pt=Line.user) ){
			if( (*pt) && (*pt != '-') && !MyPrefStruct.ignore_usernames ){
				if( strcmpd( "anonymous", pt ) )	// make sure not to use Anonymous users
					Line.clientaddr = pt;
			}
		}
		// ---- handle cookie user tracking
		if( (pt=Line.cookie) ) {
			if( (*pt) && (*pt != '-') && MyPrefStruct.cookievar[0] ){
				// if cookie variable is * then use the whole cookie value
				if( MyPrefStruct.cookievar[0] == '*' )
					Line.clientaddr = Line.cookie;		// QCM43741
				else
				if( pt = mystrstr( pt, MyPrefStruct.cookievar ) ){
					char *start = NULL, c;
					while ( c=*pt ){
						if( c == '=' ) start = pt+1;
						if( c == ';' || !c ) { *pt = 0; continue; }
						pt++;
					}
					Line.clientaddr = start;
				}
			}
		} else {
			// ----extract a URL CGI parameter and use its value as the USER name
			if( (pt=Line.file) ){
				if( (*pt) && (*MyPrefStruct.urlsessionvar) ){
					if( pt = mystrstr( pt, MyPrefStruct.urlsessionvar ) ){
						char* start=NULL;
						char* end=Line.newusername+sizeof(Line.newusername)-1;
						bool foundTerminator(false);
						while( !foundTerminator && start!=end && *pt!=0 )
						{
							char c=*pt;

							if( c == '=' )
							{
								c=*++pt;
								start=Line.newusername;
							}
							if( start )
							{
								if( c == ';' || c == '&' || c== '|' )
								{
									c = 0;
									foundTerminator=true;
								}
								*start++ = c;
							}
							pt++;
						}
						if( start )
						{
							if( !foundTerminator )	// if we ran out of buffer
							{	
								DEF_ASSERT(start==end);
								*end = 0;
							}
							Line.clientaddr = Line.newusername;
						}
					}
				}
			}
		}

		// ---- clean up address string and determine if it is reversed
		isReverse = CleanupClientString( &Line );


		// ############################# MULTIHOST INITS #######################################3
#ifdef	DEF_FULLVERSION
		// if we do router stuff, we split it up into INTERNAL/INCOMING/OUTGOING reports....
		if( ISLOG_FIREWALL(logStyle) && MyPrefStruct.filterdata.routerTot>0 )
		{
			VDinfoP retVD;
			retVD = FireWall_MultiHostInit( VDptr , &Line );
			if( retVD == NULL )
				continue;
		} else
#endif
		// handle virtual hosts within one log file, if there is a new host then create a new struct for it //
		if( MyPrefStruct.multivhosts == 1 ) {
			char *rch, *vhost = Line.vhost;

			if( vhost ){
				if( *vhost || *vhost == '-' ){
					// trim host name of :
					if( rch=strrchr( vhost, ':' ) )
						*rch = 0;
					// ignore IP host names if selected
					if( MyPrefStruct.ignore_ipvhosts ){
						if( *vhost <= 0x39 )
							vhost = ErrVhostStr;
					}
				} else vhost = ErrVhostStr;
			} else vhost = ErrVhostStr;

			Line.vhost = vhost;

			oldVDptr = VDptr;

			// Virtual Host mapping, to make *.blahISP.com equal www.isp.com or somethnig what ever u choose
			vhost = CheckHostMappingMatch( Line.vhost, Line.file, MyPrefStruct.filterdata.vhostmap, MyPrefStruct.filterdata.vhostmapTot, 1 );
			if( !vhost )
				vhost = Line.vhost;

			//do we need to log all domains?
			if (editNum>0) {
				if( !FindHost( MyPrefStruct.EditPaths, (short)editNum, vhost) )
					continue;
			}

			CorrectVirtualName( vhost, buf2, 0 );
			VDptr = FindVirtualDomain( buf2 );

			// if host is not found, make a new one
			if( !VDptr ){
				// check virtual host name for garbage and ignore if so
				if( IsNameBad( buf2 ) )
					continue;
				
				VDptr = InitVirtualDomain( VDnum+1, buf2, logStyle ); /* initialize virt domains info/data */
				if( VDptr ){
					VDnum++;
					oldVDptr->next = VDptr;
				} else {
					VDptr = oldVDptr; continue;
				}
			} else
			if( (long)VDptr == -1 )
				continue;
		} else
		// Virual Host by CLIENT
		if( MyPrefStruct.multivhosts == 5 ){
			if( Line.clientaddr ){
				pt = Line.clientaddr;
				oldVDptr = VDptr;
				VDptr = FindVirtualDomain( pt );
				if( VDptr == 0 ){
					VDptr = InitVirtualDomain( VDnum+1, pt, logStyle );
					if( VDptr ){
						VDnum++;
						oldVDptr->next = VDptr;
					} else {
						VDptr = oldVDptr; continue;
					}
				} else

				if( (long)VDptr == -1 )
					continue;
			}
		}
		else

		// use top level dirs as virtial hosts
		// ie
		// for a given url, such as "/www.host.com/images/top.gif"
		// domainName = www.host.com
		// v4.x  url = "/www.host.com/images/top.gif"
		// it should be url = "/images/top.gif"

		if( MyPrefStruct.multivhosts >= 2 ) {
			char *tmp, *hostname;
			pt = Line.file;

			if( !strcmpd( "http://", pt ) ) {				// skip the http:// portion
				if( (tmp = mystrchr(pt+6, ':')) ){
					Line.file = mystrchr( tmp, '/' );		// set line.file to the first file after the host name
					*tmp++ = '/';							// kill port number details
					*tmp = 0;								// terminate string.
				} else
					Line.file = mystrchr( pt+7, '/' );		// skip the "host.com/" portion to the filename.
			}
			// *** 990718RS, strip /.. dirs
			if( !strcmpd( "/..", pt ) ) {
				pt+=3;
			}

			if( PathFromURL( pt, buf2 ) ) {
				short count = MyPrefStruct.multivhosts;

				// Virtual host by Specific Directory.
				if( count == 4 ){
					if( !strcmpd( MyPrefStruct.multireport_path, buf2 ) ){
						hostname = buf2 + strlen(MyPrefStruct.multireport_path );
						if( breakatx( hostname, *hostname, 2 ) < 2 )
							hostname = RootLevelStr;
					} else
						hostname = RootLevelStr;
				} else {
					if (*buf2==':') {		//mac based path (this really doesnt ever get called since all mac paths are converted to /'s
						if( breakatx( buf2, ':', count ) < count )
							hostname = RootLevelStr;
						else {
							hostname = buf2;
							if( count == 3 ){
								if( tmp = mystrchr( buf2+1, ':' ) )
									*tmp = '.';
							}
						}
					} else {			//PC based path
						if( breakatx( buf2, '/', count ) < count )
							hostname = RootLevelStr;
						else {
							hostname = buf2;
							if( count == 3 ){
								if( tmp = mystrchr( buf2+1, '/' ) )
									*tmp = '.';
							}
						}
					}
				}
				// Adjust the / in host name so it will still map properly
				if( *hostname == '/' )
					hostname++;

				if( *hostname == 0 )
					hostname = RootLevelStr;
				// Needed addition - re-adjust the Line.file to point to the path after the host....
				/*  // This will adjust the filename
					if( (hostname != RootLevelStr) && (tmp = strchr( hostname, 0 )) )
						Line.file = tmp+1;
				*/
			} else
				hostname = RootLevelStr;

			oldVDptr = VDptr;
			
			//////
			// Virtual Host mapping, to make *.blahISP.com equal www.isp.com or somethnig what ever u choose
			tmp = CheckMappingMatch( hostname, MyPrefStruct.filterdata.vhostmap, MyPrefStruct.filterdata.vhostmapTot, 1 );
			if( tmp ) hostname = tmp;

			//do we need to log all domains?
			if( editNum>0 ) {
				if( !FindHost( MyPrefStruct.EditPaths, (short)editNum,hostname+1) )			// +1 to skip the / or : char
					continue;
			}
			////////
			
			char domainName[MAX_DOMAINNAMESIZE+1];
			CorrectVirtualName( hostname, domainName, 1 );
			CleanStringOfBadChars( domainName );		// make sure virtualhostname has no invalid characters.
			VDptr = FindVirtualDomain( domainName );

			if( VDptr == 0 ){
				VDptr = InitVirtualDomain( VDnum+1, domainName, logStyle );
				if( VDptr ){
					VDnum++;
					oldVDptr->next = VDptr;
				} else {
					VDptr = oldVDptr; continue;
				}
			} else
			if( (long)VDptr == -1 )
				continue;
		} else
		if( MyPrefStruct.multimonths )
		{
			if( Line.date && MyPrefStruct.multimonths == 1 ){
				pt = buf2;
				strncpy( pt, Line.date, 2 );				// new vhost name for MultiMonths, this works 100% with multiple filenames and propper dates. (990310)
				mystrcpy( pt+2, Line.date+5 ); pt[2]='-';

				oldVDptr = VDptr;
				VDptr = FindVirtualDomain( buf2 );
				if( VDptr == 0 ){
					VDptr = InitVirtualDomain( VDnum+1, buf2, logStyle );
					if( VDptr ){
						VDnum++;
						oldVDptr->next = VDptr;
					} else {
						VDptr = oldVDptr; continue;
					}
				} else

				if( (long)VDptr == -1 )
					continue;
			} else
			if( Line.date && MyPrefStruct.multimonths == 2 ){
				pt = buf2;

				int iWeek(0);
				int	iYear = 1900+logDate.tm_year;
				size_t	nDayAdd(0);

				if ((logDate.tm_yday - logDate.tm_wday) <= 0)
				{
					--iYear;
					if (iYear%4 == 0)
						nDayAdd = 366;
					else
						nDayAdd = 365;
				}
				iWeek = (logDate.tm_yday + nDayAdd - logDate.tm_wday + 6) / 7;
				DEF_ASSERT(iWeek != 0);
				sprintf(pt, "week_%02d_%04d", iWeek, iYear);

				
				oldVDptr = VDptr;
				VDptr = FindVirtualDomain( buf2 );
				if( VDptr == 0 ){
					VDptr = InitVirtualDomain( VDnum+1, buf2, logStyle );
					if( VDptr ){
						VDnum++;
						oldVDptr->next = VDptr;
					} else {
						VDptr = oldVDptr; continue;
					}
				}
				else
				{
					if( (long)VDptr == -1 )
						continue;
				}
			}
		}
		// ############################# MULTIHOST INITS #######################################


		/* DATES check for time boundaries, and if it is out of boundaries then skip it */
		if (VDptr->firstTime==0){
			VDptr->firstTime = logDays;	//TIMESTAT
			if( allfirstTime==0  ) allendTime = allfirstTime = logDays;
		}
		if( logDays < VDptr->firstTime ){
			VDptr->firstTime = logDays;	//TIMESTAT
		}
		if( logDays > VDptr->lastTime ){
			VDptr->lastTime = logDays;
		}
		
		/* --------------------- start talling up totals for the record -----------------------*/

		// maintain report time range variables
		if( logDays < allfirstTime  && logDays ) allfirstTime = logDays;
		if( logDays > allendTime && logDays ) allendTime = logDays;

		/* increment totals for all domains */
		allTotalRequests++;
		/* increment totals for this domain as a whole... */
		VDptr->totalRequests++;
		
		if( MyPrefStruct.stat_style == STREAM )
		{
			BytesOut = Line.bytes;
			if( BytesOut<0 )
				BytesOut = 0;
			BytesIn = ((__int64)Line.s_packets_rec<<32);
			BytesIn |= (Line.s_packets_sent);
		} else {
			BytesOut = Line.bytes;
			BytesIn = Line.bytesIn;

			// Handle BYTESIN differently, where before a PUT would use its bytessend as bytesin.
			if( !BytesIn )
			{
				BytesIn = mystrlen( Line.file );
			}
		}


		allTotalBytes+= BytesOut;
		allTotalBytesIn+= BytesIn;
		VDptr->totalBytes+= BytesOut;
		VDptr->totalBytesIn+= BytesIn;

		Statistic* currentError=0;	// current error Statistic, if there is one

		/* totals by errors */
		if( Line.stat ) 
		{
			if( statcode == 304 ){
				VDptr->totalCachedHits++;
				allTotalCachedHits++;
			} else
			if( statcode == 307 ){
				VDptr->totalRedirectedHits++;
				allTotalRedirectedHits++;
			} else
			if( statcode == 404 ){
				VDptr->total404Hits++;
				allTotal404Hits++;
			}
		//	if( statcode>=400 ){		// RS: should be changed to this....
			if( statcode>304 ) 		// removed because its not really an error || statcode == 206 ) {
			{
				errorcode = statcode;
				statsDrillFlag = DRILLF_INCERR;

				if( VDptr->byErrors ) 
				{
					const char* errTxt=GetErrorString( statcode );
					DEF_ASSERT(errTxt);
					currentError=VDptr->byErrors->IncrStatsDrill( BytesOut, BytesIn, (char*)errTxt, logDays, 0, 0 );
					DEF_ASSERT(currentError);
					currentError->counter=statcode;		// FWA database now depends apon this!!
				}
				if( VDptr->byErrorURL )
				{
					char	*tmpP = buf2;

					tmpP+= mystrcpy( tmpP, Line.file );
					*tmpP++ = ' ';
					*tmpP++ = '(';
					tmpP+= mystrcpy( tmpP, Line.stat );
					*tmpP++ = ')';
					*tmpP++ = 0;
					VDptr->byErrorURL->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, 0, 0 );
				}
				allTotalFailedRequests++;
				VDptr->totalFailedRequests++;
			} else {
				errorcode = 0;
			}
		}


		char *urlStr = pt = Line.file;

		currentPageHash = i = 0;
		Statistic* currentURL = NULL;	// the current url/file etc stat requested

		if( logStyle )
		{
			// if there is no file, then use source addr as a file
			if( !urlStr )
				urlStr = Line.sourceaddr;

			/* totals by file */
			//------- URL Stats
			if( urlStr ) {
				dopage = dodownload = FALSE;
				size_t len( mystrlen( urlStr ) );
				char* endpt=urlStr+len-1;
				char* filenameStr=0;				// the current url/file etc requested
				char* currentURLStr;

				if( MyPrefStruct.useCGI ){ // handle /tbapps/track/?partnerId=2&MURL=/guides/a_2_167_.phtml?partnerId=2
					if( filenameStr = mystrchr( urlStr, '?' ) ){
						endpt = filenameStr;
						if( MyPrefStruct.retain_variable ){
							if( RetainURLParam( endpt+1, len ) == 0 ){
								// convert an empty default URL into a "/" in a similar manner to above (***)
								if( filenameStr==urlStr )  
								{
									*urlStr='/';
									*(urlStr+1) = 0;
									endpt = urlStr;
								}
								else
								{
									*filenameStr = 0;
									endpt = filenameStr-1;
								}
							}
							// update length in order to handle case of having no dots at all
							len = mystrlen( urlStr );
						}
						dopage = TRUE;
					} 
				}

				// make filenameStr the actual filename without path.
				if( (filenameStr = (char*)revstrrchr( urlStr, endpt, '/' )) == NULL)
					filenameStr = urlStr;

				char* urlExtensionStr=(char*)revstrrchr( filenameStr, endpt, '.' );

				if( !urlExtensionStr )		// no dots at all
				{
					//
					// remove all /'s from the end of a URL page hit, except a single /
					if( len>1 )
					{
						char c;

						while( (c=*endpt) && (endpt!=filenameStr) )
						{
							if( (c == '/' || c == '\\') )
								*endpt-- = 0;
							else
								break;
						}
					}
					//
					dopage = TRUE;
					urlExtensionStr = urlStr;
				} else
				// *** RS990718, add ability to turn /index.html into /
				// *** RS000808, fixed it to not wipe a /index.html to ""
				if( defaultindexLen ){
					//if( !strcmpd( MyPrefStruct.defaultindex, filenameStr+1 ) )
					if( !strcmpMany( filenameStr+1, defaultindexarray ) ){
						filenameStr[1] = 0;
					}
				}


				// dots and optionaly slashes
				if( ISLOG_FIREWALL(logStyle) )
				{
					if( isdigit( urlExtensionStr[1] ) )
						dopage = TRUE;
					else
					if( !strrchr( urlStr, '/' ) )
						dopage = TRUE;
					else
					if( strrchr( urlStr, '@' ) )
						dopage = TRUE;
				}

				if( VDptr->byFile )
				{
					// remember the current URL stat
					currentURL = VDptr->byFile->IncrStatsDrill( BytesOut,BytesIn, urlStr, logDays, statsDrillFlag, statcode );
					DEF_ASSERT(currentURL);
					// remember the current URL stat's string
					currentURLStr = currentURL->GetName();
					currentPageHash = currentURL->GetHash();
				} else {
					currentPageHash = 0;
					currentURLStr = NULL;
				}


				// All of this block depends on the string added to byFile, they just copy the string pointer
				if( urlExtensionStr ) 
				{
					int url_id = URLID_UNKNOWN;
		
					domedia = FALSE;
					// -------------------------------------- STREAMING MEDIA MODE -------------------------------------------
					if( MyPrefStruct.stat_style == STREAM )
					{
						// work out what file url is audio or video content.
						// Store all clips/audio/video into byPages , counter2 will tell us what sort of page it is, ie video/audio etc....
						if( VDptr->byPages ) 
						{
							if( (Line.s_audiocodec && !Line.s_videocodec) || !strcmpManyFast( urlExtensionStr, audioHashs, audioStrings ) )
							{
								domedia = TRUE;
								if ( Line.s_filedur == 0 && (BytesOut || statcode<400) )
									url_id = URLID_LIVEAUDIO;
								else
									url_id = URLID_CLIPAUDIO;
							}
							// Checking for Video is more strict, since most things can be assumed audio unless they specifically tell us its video
							if( (Line.s_videocodec) && !strcmpManyFast( urlExtensionStr, videoHashs, videoStrings ) )
							{
								domedia = TRUE;
								if ( Line.s_filedur == 0 && (BytesOut || statcode<400) )
									url_id = URLID_LIVEVIDEO;
								else
									url_id = URLID_CLIPVIDEO;
							}

							if( !domedia && Line.protocol && Line.protocol[0] ){
								if ( strcmpd( Line.protocol, "http" ) )
									domedia = TRUE;
							}

							if( domedia )
							{
								long audio_id=-1, video_id=-1;
								Statistic		*current;

								if( VDptr->byMediaTypes )
									VDptr->byMediaTypes->IncrStatsMore( BytesOut,BytesIn, urlExtensionStr, logDays, statsDrillFlag );

								if ( VDptr->byAudioCodecs && Line.s_audiocodec ){
									current = VDptr->byAudioCodecs->IncrStatsMore( BytesOut,BytesIn, Line.s_audiocodec, logDays, statsDrillFlag );
									audio_id = current->GetHash();
								}

								if ( VDptr->byVideoCodecs && Line.s_videocodec ){
									current = VDptr->byVideoCodecs->IncrStatsMore( BytesOut,BytesIn, Line.s_videocodec, logDays, statsDrillFlag );
									video_id = current->GetHash();
								}

								// Add the clip to the stats... (all clip types are here)
								// This will also add the errors table to each clip for future references too
								currentPage = VDptr->byPages->IncrStatsDrill( BytesOut,BytesIn, currentURLStr, logDays, statsDrillFlag ,errorcode );
								currentPageHash = VDptr->byPages->CurrentGetHash();
								if ( currentPage->clipData )
								{
									if ( currentPage->files==1 )
										currentPage->clipData->SetStats( Line.s_filedur, Line.s_filesize, video_id, audio_id );
									currentPage->clipData->AddStats( Line.s_cpu_util, Line.s_percOK, Line.s_meanbps, Line.s_wm_streamduration, Line.s_buffercount );
								}
								statsDrillFlag |= DRILLF_INCPAGES;

								// Turn OFF Page flag so we dont add stats twice to this special page type.
								dopage = FALSE;
							}
						}
					} else {
						// -------------------------------------- WEB MULTI MEDIA STATS -------------------------------------------
						if( VDptr->byAudio ) {
							if( !strcmpManyFast( urlExtensionStr, audioHashs, audioStrings ) )
							{
								domedia = TRUE;
								url_id = URLID_CLIPAUDIO;
								VDptr->byAudio->IncrStatsDrill( BytesOut,BytesIn, currentURLStr, logDays, statsDrillFlag,0 );
							}
						}
						if( VDptr->byVideo && !domedia ) {
							if( !strcmpManyFast( urlExtensionStr, videoHashs, videoStrings ) )
							{
								domedia = TRUE;
								url_id = URLID_CLIPVIDEO;
								VDptr->byVideo->IncrStatsDrill( BytesOut,BytesIn, currentURLStr, logDays, statsDrillFlag,0 );
							}
						}
						if ( domedia )
						{
							if( VDptr->byMediaTypes )
								VDptr->byMediaTypes->IncrStatsMore( BytesOut,BytesIn, urlExtensionStr, logDays, statsDrillFlag );
						}
					}

					//------- Web Pages Stats, work out what file url really is a webpage...
					if( VDptr->byPages ) 
					{
						if( dopage || !strcmpManyFast( urlExtensionStr, pageHashs, pageStrings ) ) {
							dopage = TRUE;
							url_id = URLID_PAGE;
							if( currentURLStr ){
								currentPage = VDptr->byPages->IncrStatsDrill( BytesOut,BytesIn, currentURLStr, logDays, statsDrillFlag, 0 );
								currentPageHash = VDptr->byPages->CurrentGetHash();
							}
							statsDrillFlag |= DRILLF_INCPAGES;
						}
					}
					// Uploads... (ftp/firewall only)
					if( VDptr->byUpload && (logStyle == LOGFORMAT_FTPSERVERS || logStyle == LOGFORMAT_FIREWALLS) && Line.bytesIn>0 ) {
						if( !strcmpManyFast( urlExtensionStr, downloadHashs, downloadStrings ) ) {
							url_id = URLID_UPLOAD;
							if( currentURLStr )
							{
								currentPage = VDptr->byUpload->IncrStatsDrill( BytesOut,BytesIn, currentURLStr, logDays, statsDrillFlag, 0 );
								currentPageHash = VDptr->byUpload->CurrentGetHash();
							}
						}
					} else
					// work out what file url is actualy just a downloaded file
					if( VDptr->byDownload ) {
						if( !strcmpManyFast( urlExtensionStr, downloadHashs, downloadStrings ) ) {
							url_id = URLID_DOWNLOAD;
							if( currentURLStr )
							{
								currentPage = VDptr->byDownload->IncrStatsDrill( BytesOut,BytesIn, currentURLStr, logDays, statsDrillFlag, 0 );
								currentPageHash = currentPage->GetHash();
							}
						}
					}


					// Set the byFiles URL to the type of url it is....
					if( currentURL )
						currentURL->SetPageType( url_id );

					if( currentPage )
						currentPage->SetPageType( url_id );

					if( VDptr->byClient ){
						if( domedia )
							VDptr->byClient->temp = Line.s_wm_streamduration;
						else
							VDptr->byClient->temp = 0;
					}

				}

				//------- work out what url is an actual advertisment or a advertisement view
				if( VDptr->byAdvert ){
					if( MyPrefStruct.filterdata.advertTot>0 ){
						char *pstr;
						pstr = AdvertMappingMatch( urlStr, MyPrefStruct.filterdata.advert, &i, MyPrefStruct.filterdata.advertTot, 1 );
						if( pstr ){
							strcpyuntil( buf2 , pstr, ',' );
							VDptr->byAdvert->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, statsDrillFlag | i, 0 );
						}
					}
				}
				if( VDptr->byAdCamp ){
					if( MyPrefStruct.filterdata.advert2Tot>0 ){
						char *pstr;
						pstr = AdvertCampMappingMatch( Line.refer, MyPrefStruct.filterdata.advert2, &i, MyPrefStruct.filterdata.advert2Tot, 1 );
						if( pstr ){
							strcpyuntil( buf2 , pstr, ',' );
							VDptr->byAdCamp->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, statsDrillFlag, 0 );
							VDptr->byAdCamp->CurrentSetCounter4( i );
						}
					}
				}

				
				//------- Directory Stats  (these should ideally be done before the url/pages components)
				if( VDptr->byDir ) {
					if( PathFromURL( urlStr, buf2 ) ) {
						VDptr->byDir->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, statsDrillFlag, 0 );
					}
					//------- Top Directory Stats
					if( VDptr->byTopDir ) {
						char *s1 = buf2; short words=2, cnt=0;
						// handle TopDirs if the vhost report is based on topdirs (Virtual host by Top directory)
						// then make sure topdirs is relative to that and not just 1 layer.
						if( MyPrefStruct.multivhosts == 2 ) words = 3; else
						if( MyPrefStruct.multivhosts == 3 ) words = 4; else
						if( MyPrefStruct.multivhosts == 4 ) {
							if( !strcmpd( MyPrefStruct.multireport_path, buf2 ) ){
								s1 += strlen( MyPrefStruct.multireport_path );
							}
						}
						char c;

						// Top Level Directory Test, This is the proper working way that handles dirs with DOTS.
						while( (c=*s1) && cnt<words ){
							if( c == '/' ) {
								cnt++;
							}
							s1++;
						}
						*s1 = 0;

						if( cnt == words ){
							VDptr->byTopDir->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, statsDrillFlag, 0 );
						}
					}
				}

				//------- Content (URL) Group Stats
				if( VDptr->byGroups ) 
				{
					if( MyPrefStruct.filterdata.groupTot>0 ){
						long start=0;
						while( pt = MultiMappingMatch( urlStr, MyPrefStruct.filterdata.group, &start, MyPrefStruct.filterdata.groupTot, 1 ) ){
							VDptr->byGroups->IncrStatsDrill( BytesOut,BytesIn, pt, logDays, statsDrillFlag, 0 );
						}
					}
				}

				// ***************************************************************************
				// Add to the byBilling statistic!
				// ***************************************************************************
				if	(	VDptr->byBilling
					&&	MyPrefStruct.billingSetup.bEnabled
					&&	mydatecmp(Line.date, MyPrefStruct.billingSetup.szStartDate) >= 0
					&&	mydatecmp(Line.date, MyPrefStruct.billingSetup.szEndDate) <= 0
					)
				{
					VDptr->byBilling;
					BillingCustomerStruct*	pCustomer;
					for (pCustomer=MyPrefStruct.billingCharges.pFirstCustomer; pCustomer; pCustomer=pCustomer->pNext)
					{
						if (strcmpiPart(pCustomer->szURL, urlStr) == 0)	// If urlStr starts with pCustomer->szURL!
							break;
					}

					if (pCustomer)
					{
						VDptr->byBilling->IncrStatsDrill(BytesOut,BytesIn,pCustomer->szURL,logDays, 0, 0);
					}
				}
				

				//------- File Type Stats
				if( VDptr->byType && urlExtensionStr )
				{
					pt = urlExtensionStr;
					
					while( *pt )
					{
						if( *pt=='/' || *pt=='\\' || *pt=='?' || *pt=='&' || *pt=='%' )
						{
							*pt = 0;
							break;
						}
						++pt;
					}

					if( *urlExtensionStr==0 )
					{
						urlExtensionStr=s_UnknownStr;
					}

					VDptr->byType->IncrStatsMore( BytesOut,BytesIn, urlExtensionStr, logDays, statsDrillFlag );
				}
			}
		}




		// -------------------------------- handle all REFERRAL ASPECTS -----------------------------
		doreferral = FALSE;

		if( VDptr->byClient )
			VDptr->byClient->temp2 = 0;	//clear the referral hash ID for the client

		Statistic* currentReferral=0;				// current referral stat, if we have one
		Statistic* currentBrokenLinkReferal=0;		// current referral of a broken link stat, if we have one

		bool haveMissingReferal( !Line.refer );		// true if we need to add a referral stat entry for a missing referral
		
		/* totals by referrals */
		if( Line.refer )
		{
			char	*referral = Line.refer,
					*refUrl,
					*referralParam;

			// This should be ok , since before trimrefer was called any way which searched for '?' to kill, and doing it here prevents
			// unneeded searchengine decodes which would slow down even more... so we may do 1% more strchr()s but we do 80% less search engine decodes.
			referralParam = strchr( referral, '?' );

			//search criteria
			if( VDptr->bySearchStr && referralParam )
			{
				char *sp;

				sp = DecodeSearch( buf2, referral, referralParam );
				if( sp ) {
					Statistic *srch;

					srch = VDptr->bySearchStr->IncrStatsDrill( BytesOut,BytesIn, sp, logDays, statsDrillFlag, 0 );
					if( VDptr->bySearchSite ){
						strcpydomain( buf2, referral );
						if( !isdigit( *buf2 ) ){
							long strHASH;
							// 990415: store the search string's hash into the time array, so we can cross reference Engines->Strings
							strHASH = srch->GetHash();
							VDptr->bySearchSite->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, statsDrillFlag, strHASH );
						}
					}
				}
			}

			//------- Referral Group Stats
			if( MyPrefStruct.filterdata.referralTot )
			{
				pt = CheckMappingMatch( referral, MyPrefStruct.filterdata.referralMap, MyPrefStruct.filterdata.referralTot, 1 );
				if( pt )
					referral = pt;
			}

			if( logType<30 || logType == LOGFORMAT_V5DATABASE || logType == LOGFORMAT_CUSTOM )
			{
				if( *referral ) 
				{
					char *hostName;
					// NOTE: referrals are assumed to be external by default
					doreferral = REFERRAL_IS_EXTERNAL;

					refUrl = referral;

					//normal referral collection 
					if( MyPrefStruct.useCGI == 0 && referralParam )		// KILL url parameters
						*referralParam = 0;
						//trimrefer( referral );	

					// if the URL has http:// in it, use the stuff after it.
					if( Line.vhost && MyPrefStruct.multivhosts > 0)
						hostName = Line.vhost;
					else
						hostName = szStrippedSiteUrl;
					//hostname = VDptr->domainName;		// A BETTER WAY for the above block.


					if( hostName && (*hostName) )
					{
						// Convert a referral URL to one without the host name if its a local url
						if( strstri( referral,hostName ) )
						{
							// Decide if to ignore SELF REFERRALS or NOT
							if( MyPrefStruct.ignore_selfreferral )
								doreferral = FALSE;
							else 
							{
								doreferral = REFERRAL_IS_LOCAL;		// Still TRUE, but more info.
								// Yes we are local url
								char *t;
								// find first // in url, http://me.com/
								t = strstr( referral, "//" );
								if( t )
									t = strchr( t+2, '/' );
								if( t )
									refUrl = t;
								// else refUTL = referral (from above)
							}
						}
					}

					// ----------------------------------------------------------------
					// At this point both, referral/refUrl should be set appropriately.
					DEF_ASSERT(refUrl != NULL);

					if (MyPrefStruct.ignore_bookmarkreferral) 
					{
						if (strstri(refUrl, "file:" )) {
							doreferral = FALSE;
						}
					}
					if( doreferral ) 
					{
						if (strstri(refUrl, "file:" ))
						{
							if( VDptr->byRefer )
							{
								currentReferral=VDptr->byRefer->IncrStatsMore( BytesOut,BytesIn, s_BookmarkReferrerStr, logDays, statsDrillFlag );
								DEF_ASSERT(currentReferral);
							}

							if( VDptr->byReferSite )
							{
								VDptr->byReferSite->IncrStatsDrill( BytesOut,BytesIn, s_BookmarkReferrerStr, logDays, statsDrillFlag, 0 );
							}
						}
						else
						{
							if( VDptr->byRefer )
							{
								currentReferral=VDptr->byRefer->IncrStatsMore( BytesOut,BytesIn, refUrl, logDays, statsDrillFlag );
								DEF_ASSERT(currentReferral);

								// if we've got a broken link and if the pages containing broken links
								// report is turned on then go forth and increment
								if( statcode==404 && currentURL && VDptr->byBrokenLinkReferal )
								{
									// decide to which StatList the current referral will be added -
									// external or internal
									StatList* pBrokenLinkReferals=doreferral==REFERRAL_IS_LOCAL ? 
                                                                  VDptr->byIntBrokenLinkReferal :
									                              VDptr->byBrokenLinkReferal;
									DEF_ASSERT(pBrokenLinkReferals);
										
									// Note that the DRILLF_INCERR flag here is turned on always.  The 'blnk' report
									// interprets the corresponding stat error counter to mean 404 errors only.
									currentBrokenLinkReferal=
										pBrokenLinkReferals->IncrStatsMore( BytesOut, BytesIn, currentReferral->GetName(), logDays, DRILLF_INCERR );
								}
							}

							if( VDptr->byReferSite ) 
							{
								strcpybrk( buf2, referral, '/', 3 ); //strip out trailing referral info
								/* This will strip all www. prefixes for all referrals sothey are the same
								if( strip_www_referral_prefix ){
									if( strcmpd( "http://www.", buf2 ) )
										mystrcpy( buf2+7, buf2+11 );
								}
								*/
								VDptr->byReferSite->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, statsDrillFlag, 0 );
							}
						}

						// store referral hash into the clients lists temp2 for usage later
						// in the session adder
						if( currentReferral && VDptr->byClient )
							VDptr->byClient->temp2=currentReferral->GetHash();
					} // endif doreferral
				}
				else
				{
					if( !MyPrefStruct.ignore_bookmarkreferral )
					{
						haveMissingReferal=true;
					}
				}
			}
		}

		// if the current log entry aint got a referral then create an appropriate stat entry
		if( haveMissingReferal && VDptr->byRefer )
		{
			currentReferral=VDptr->byRefer->IncrStatsMore( BytesOut,BytesIn, s_NoReferrerStr, logDays, statsDrillFlag );
			DEF_ASSERT(currentReferral);
			doreferral = TRUE;
			if( VDptr->byClient )
				VDptr->byClient->temp2 = VDptr->byRefer->CurrentGetHash();
		}



		// ----------------------------------------------------------
		// Add more current details to the URL statistic (byFile)
		// These can be used in extra columns and new reports in some far distant future….
		// each URL entry in byFiles will now have information saying how many 404s it had
		// and how many cached requests it had, and how many external referrals it had.
		if (currentURL)
		{
			// add cache counts to the current URL statistic
			if( statcode == 304 )
				currentURL->counter++;

			// add FILENOTFOUND counts to the current URL statistic
			if( statcode == 404 )
				currentURL->counter4++;

			// add EXTERNALREFERRAL counts to the current URL statistic
			if (doreferral == REFERRAL_IS_EXTERNAL  )
				currentURL->visits++;
		}

		
		/* totals by client */
		i = DRILLF_SETFIRST | DRILLF_SESSIONSTAT | statsDrillFlag;
		//i = i & (DRILLF_INCPAGES^0xFFFFFFFF);		// turn off counter4

		Statistic* currentClient=0;
		unsigned long currentClientHash(0);

		if( VDptr->byClient && *rev_domain )
		{
			// remember number of clients before call to IncrStatDrill()
			long prevNumClients( VDptr->byClient->num );

			currentClient = VDptr->byClient->IncrStatsClickstream( BytesOut,BytesIn, rev_domain, logDays, i|DRILLF_SESSIONADDREF, currentPageHash, statcode );		//TIMESTAT
			DEF_ASSERT(currentClient);
			currentClientHash=currentClient->GetHash();
			
			// if current number of clients is different to before call to IncrStatDrill() then
			// we must have just added a client so initialise its counter to -1 to ensure correct counting
			// of the number of visitors per unrecognised agent (see below)
			if( VDptr->byClient->num!=prevNumClients )
			{
				currentClient->counter=-1;
			}

			// set NEW SESSION flag
			if( VDptr->byClient->newses ) {
				// store the Referral HASH for this session in the sessionStat area
				// this is used for the first referral filter
				if( currentClient && VDptr->byRefer )
					currentClient->sessionStat->SetReferral( VDptr->byClient->temp2 );
				// inc visits for referrals
				statsDrillFlag |= DRILLF_INCVISITS;
				if( VDptr->byRefer && doreferral ) VDptr->byRefer->CurrentIncrVisits();
				if( VDptr->byReferSite && doreferral ) VDptr->byReferSite->CurrentIncrVisits();
			}
		}

#ifdef DEF_FULLVERSION

		// NOTE: The Top Referrals on Errors report doesn't have its own stat_XXXXX entry. It is
		//		 merely and addition of sub-tables to the existing Server Errors report.  Therefore,
		//		 the easiest way to stop the Top Referrals on Errors sub-tables from being produced
		//		 by FWA Standard builds is by not recording the information in the first place.  The
		//		 report code will handle the situation correctly by not creating sub-tables.  While
		//		 I was here, I thought I might as well macro out the recording of info for the Pages
		//		 Containing Broken Links report as well.

		// if we have a page conaining a broken link...
		if( currentBrokenLinkReferal ) 
		{
			DEF_ASSERT(statcode==404 && currentURL && VDptr->byBrokenLinkReferal);

			// ...then record that sucker
			VDptr->GetFailedRequestInfo( currentBrokenLinkReferal ).
				recordFailedRequest( currentURL->GetHash(), currentClientHash );
		}

		// if we have an error and if we know its referral...
		if( STAT(MyPrefStruct.stat_errors) && currentError && currentReferral )
		{
			DEF_ASSERT(statcode>304);

			// ...then record that sucker
			VDptr->GetFailedRequestInfo( currentError ).
				recordFailedRequest( currentReferral->GetHash(), currentClientHash );
		}

#endif	// DEF_FULLVERSION

		// add unique page counts or session counts to other stuff
		if( VDptr->byPages && currentPage && (dopage||domedia) ){
			// add "Page Entry count" to the currentpage
			if( VDptr->byClient && VDptr->byClient->newses ) {
				VDptr->byPages->CurrentIncrPages();
				VDptr->byFile->CurrentIncrPages();
			}

			// set NEW PAGE flag
			statsDrillFlag |= DRILLF_INCPAGES;

			// --------------------------------------------------
			// manually add page count to errors list
			if( VDptr->byErrors && statsDrillFlag&DRILLF_INCERR ){
				VDptr->byErrors->CurrentIncrPages();			// add pages count to errors
			}
		}
		
		if( Line.sourceaddr )
			VDptr->byServers->IncrStatsDrill( BytesOut,BytesIn, Line.sourceaddr, logDays, statsDrillFlag, 0 );

		/* router/firewall specific stuff */
		if( ISLOG_FIREWALL(logStyle) )
		{
			if( Line.protocol )
			{
				long protocol = ConvProtocol(Line.protocol);
				VDptr->byProtocol->IncrStatsDrill( BytesOut,BytesIn, protocolStrings[protocol], logDays, statsDrillFlag, 0 );
				if( dopage && currentPage )
					currentPage->SetProtocolType( protocol );				// store Protocol type for page in Counter2

				if( VDptr->byClient )
					AddFirewallProtocolStats( VDptr , BytesOut, BytesIn, logDays, currentClient->GetName(), protocol, statsDrillFlag );
			}
		} else
		// record protocol types from Streaming, (rtsp/mms/http/whatever....)
		if( VDptr->byProtocol && Line.protocol )
		{
			VDptr->byProtocol->IncrStatsDrill( BytesOut,BytesIn, Line.protocol, logDays, statsDrillFlag, errorcode );
		}
		
		if( logDays ){
			 // currently, sessions are attributed to traffic report time slots in AddSessionRecords() - EngineAddStats.cpp
			long statsDrillFlagTraffic( statsDrillFlag&~DRILLF_INCVISITS );

			/* hourly totals */
			i = logDate.tm_hour;
			if( i<24 )
				VDptr->byHour->IncrStatsDrill( BytesOut,BytesIn, hourStrings[i], logDays, statsDrillFlagTraffic,0);
			else							// fix for bad logDate.tm_hour for bad WebSTAR log RHF 06/02/01
				continue;
				
			/* weekly totals */
			i = logDate.tm_wday;
			if( i >= 0 && i <= 6 ){
				VDptr->byWeekday->IncrStatsMore( BytesOut,BytesIn, weekdays[i], logDays, statsDrillFlagTraffic );
				if( VDptr->byWeekdays[i] ) {
					VDptr->byWeekdays[i]->IncrStatsMore( BytesOut,BytesIn, hourStrings[logDate.tm_hour], logDays, statsDrillFlagTraffic );
				}
			}
			/* daily totals */
			VDptr->byDate->IncrStatsMore(BytesOut,BytesIn, Line.date, logDays, statsDrillFlagTraffic );

			/* monthy totals */
			if( VDptr->byMonth && Line.date ){		/* month totals    03/14/1996    */
				strncpy( buf2, Line.date, 3 );
				mystrcpy( buf2+3, Line.date+6 );
				VDptr->byMonth->IncrStatsMore( BytesOut,BytesIn, buf2, logDays, statsDrillFlagTraffic );
			}
		}
		


		// ----------------------------------------------- DECODE AGENTS into BROWSERS/OPERSYS/ROBOTS ----------------------------------------------------------
		Statistic*	pStatCurrAgent	= NULL;
		Statistic*	pMediaPlayer	= NULL;
		currentOper = currentBrow	= NULL;

		// -------------------   STREAMING  -----------------------
		if( MyPrefStruct.stat_style == STREAM )
		{
			long	browserValid=0, mplayerValid=0,
					ostype, osId=0;

			pt=Line.agent;
			/* totals by OS manufacturer */
			if( VDptr->byOperSys ) 
			{
				char *osPtr;
				if( Line.s_playeros )
					osPtr = InterpretOperSysString( Line.s_playeros, &ostype, &osId );
				else
					osPtr = InterpretOperSysString( pt, &ostype, &osId );

				if( osPtr ){		// returns OS type
					currentOper = VDptr->byOperSys->IncrStatsDrill( BytesOut, BytesIn, osPtr, logDays, statsDrillFlag, 0 );
					currentOper->counter2 = osId;
				}
			}

			// Now here we can add our details to only one of them....
			// The agent can only be BROWSERS/ROBOT/MEDIAPLAYER/UNKNOWN 
			/* totals by browser manufacturer */
			if( VDptr->byBrowser ) 
			{
				if( browserValid=InterpretBrowserString( buf2, pt ) )
					currentBrow = VDptr->byBrowser->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, statsDrillFlag, ostype );
			}
			if( !browserValid )
			{
				if( VDptr->byMediaPlayers ) 
				{
					if( mplayerValid=InterpretMediaPlayerString( buf2, pt ) )
						pMediaPlayer = VDptr->byMediaPlayers->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, DRILLF_SESSIONSTAT | statsDrillFlag, ostype );
				}
				if( !mplayerValid && VDptr->byUnrecognizedAgents )
				{
					if (!(*pt))
						pt = "Anonymous Agent";
					pStatCurrAgent = VDptr->byUnrecognizedAgents->IncrStatsDrill( BytesOut,BytesIn, pt, logDays, statsDrillFlag, 0 );
				}
			}

			if( VDptr->byCPU && Line.s_playercpu )
			{
				VDptr->byCPU->IncrStatsDrill( BytesOut,BytesIn, Line.s_playercpu, logDays, statsDrillFlag, 0 );
			}
			if( VDptr->byLang && Line.s_playerlang )
			{
				VDptr->byLang->IncrStatsDrill( BytesOut,BytesIn, Line.s_playerlang, logDays, statsDrillFlag, 0 );
			}
		} else
		// Handle web agents....
		if( (pt=Line.agent) ) 
		{
			long	robotValid=0, browserValid=0, mplayerValid=0,
					ostype, osId=0;

			if( *pt != '-' )
			{
				/* totals by OS manufacturer */
				if( VDptr->byOperSys ) 
				{
					char *osPtr = InterpretOperSysString( pt, &ostype, &osId );
					if( osPtr ){		// returns OS type
						currentOper = VDptr->byOperSys->IncrStatsDrill( BytesOut, BytesIn, osPtr, logDays, statsDrillFlag, 0 );
						currentOper->counter2 = osId;
					}
				}

				// Now here we can add our details to only one of them....
				// The agent can only be BROWSERS/ROBOT/MEDIAPLAYER/UNKNOWN 
				/* totals by browser manufacturer */
				if( VDptr->byBrowser ) 
				{
					if( browserValid=InterpretBrowserString( buf2, pt ) )
						currentBrow = VDptr->byBrowser->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, statsDrillFlag, ostype );
				}
				if( !browserValid )
				{
					if( VDptr->byRobot ) 
					{ 
						char	*robotString = (char*)InterpretRobotString( pt, &robotValid);
						if( robotValid && robotString && *robotString )
							VDptr->byRobot->IncrStatsDrill( BytesOut, BytesIn, robotString, logDays, statsDrillFlag, 0 );
					}
					if( !robotValid ) 
					{
						if( VDptr->byMediaPlayers ) 
						{
							if( mplayerValid=InterpretMediaPlayerString( buf2, pt ) )
								pMediaPlayer = VDptr->byMediaPlayers->IncrStatsDrill( BytesOut,BytesIn, buf2, logDays, DRILLF_SESSIONSTAT | statsDrillFlag, ostype );
						}
						if( !mplayerValid && VDptr->byUnrecognizedAgents )
						{
							if (!(*pt))
								pt = "Anonymous Agent";
							pStatCurrAgent = VDptr->byUnrecognizedAgents->IncrStatsDrill( BytesOut,BytesIn, pt, logDays, statsDrillFlag, 0 );
						}
					}
				}
			}
		}

		// 990428, if client changes browsers, add 1
		if( currentClient )
		{
			if( pStatCurrAgent )
			{
				if( currentClient->counter != pStatCurrAgent->id )
				{
					VDptr->byUnrecognizedAgents->CurrentIncVisitor();
					currentClient->counter = pStatCurrAgent->id;
				}
			}

			// Handle Visitors changing browsers.......
			if( currentBrow ){
				if( currentClient->counter2 != currentBrow->id )
				{
					VDptr->byBrowser->CurrentIncVisitor();
					currentClient->counter2 = currentBrow->id;
					// It would be nice to record the browser change in the session history...
					// currentClient->SessionStat->RecordSessionHistory( SESS_BROWSERCHANGED, currentBrow->hashid );
				}
			}
			if( currentOper ){
				if( currentClient->counter3 != currentOper->id ){
					VDptr->byOperSys->CurrentIncVisitor();
					currentClient->counter3 = currentOper->id;
					// It would be nice to record the OperSYS change in the session history...
					// currentClient->SessionStat->RecordSessionHistory( SESS_OPERSYSCHANGED, currentOper->hashid );
				}
			}
			if( pMediaPlayer ){
				// We can use 'counter' as the Page counter is not meaningful.
				if( currentClient->counter != pMediaPlayer->id ){
					// Increment the Unique Visitor count.
					VDptr->byMediaPlayers->CurrentIncVisitor();
					currentClient->counter = pMediaPlayer->id;
				}
			}
		}


		/***** Handle Clusters, add details to each cluster (this should always be last) ******/
		if( MyPrefStruct.clusteringActive &&
			MyPrefStruct.clusterNamesTot && 
			VDptr->byClusters && 
			Line.logFileName 
			)
		{
			if( pt = MatchClusterPaths( Line.vhost, const_cast<char*>(Line.logFileName), MyPrefStruct.clusterNames, MyPrefStruct.clusterNamesTot ) )
			{
				if( statcode == 304 )
					statsDrillFlag |= DRILLF_COUNTER;		// add cache hit count....
				VDptr->byClusters->IncrStatsDrill( BytesOut,BytesIn, pt, logDays, statsDrillFlag, 0 );	
			}
		}

		if( MyPrefStruct.dnslookup&DNR_LOOKUPSON && MyPrefStruct.dnslookup&DNR_BACKGROUNDMODE )
		{
			BackgroundResolveClients( VDptr, logStyle );
		}

		
		//		statsDrillFlag = statsDrillFlag & (DRILLF_INCPAGES^0xFFFFFFFF);		// turn off counter4 (pages)


	} // while( ... )

	alltime2 = timems() - alltime1;
	
	//---------------------------------------END LOOP----------------------------------------


	//------------------------- BEGIN V5 DATABASE CLEAN UPS ---------------------------------
	
	// if we have a V5 database from which we've been reading
	if( s_v5ReaderDB )
	{	
		// NOTE: If we get to here then the user most likely hit the stop button as normally,
		// these cleanups are done within the Kahaoona loop

		// if the V5 database was being reprocessed then clean it up now
		if( s_v5ReaderDB!=v5AdderDB )
		{
			delete s_v5ReaderDB;
		}
		s_v5ReaderDB=0;
	}
	// if we have a V5 database to which we've been adding hits then clean it up now
	if( v5AdderDB )
	{
		v5AdderDB->endAdd();	// just to make it explicit
		delete v5AdderDB;
		v5AdderDB=0;		
	}

	//------------------------- END V5 DATABASE CLEAN UPS ---------------------------------


	if( IsStopped() ) {
		switch( IsStopped() ){
			case OUTOFRAM:			StatusSet( "Process cancelled: out of ram" ); break;
			case POSTCALCFAILURE:	StatusSet( "Process cancelled: post calc" ); break;
			case CANTMAKEDOMAIN:	StatusSet( "Process cancelled: cant init domain" ); break;
			case CANTOPENLOG:		StatusSet( "Process cancelled: cant open log" ); break;
			case ENGINEBUSY:		StatusSet( "Process cancelled: already processing" ); break;
			case USERSTOPPED:
			default:				StatusSetID( IDS_CANCELLED ); break;
		}
	} else {
		if( allTotalRequests ){
			// add post processing data that is calculated based on already gathered data
			// this prevents needless data crunching during the process loop
			for( VDcount=0; VDcount <= VDnum; VDcount++){
				OutDebugs( "DEBUG: postcalc hosts %d/%d", VDcount, VDnum );
				VDptr = VD[ VDcount ];
				if( VDptr ){
					VDptr->totalDays = (long)(VDptr->lastTime/ONEDAY) - (VDptr->firstTime/ONEDAY)+1;
					// only re-calculate records from real log files or v5 databases but not V4 databases
					if( VDptr->logType != LOGFORMAT_V4DATABASE ) {
						if( PostCalcDatabases( VDptr, logStyle ) ){
							StopAll( POSTCALCFAILURE );
							break;
						}
					}

					// Cut this out..............
					size_t	nWeek;
					size_t	nYear;
					if( 2 == sscanf(VDptr->domainName, "week_%d_%d", &nWeek, &nYear) )
					{
						// Populate the domainPath so that it is used in the directory creation.
						mystrcpy(VDptr->domainPath, VDptr->domainName);

						struct tm tmSunday;
						const char*	szFormatString = TranslateID(TIME_WEEKSTARTING);
						if (GetStartOfWeek(&tmSunday, nWeek, nYear) == 0 && szFormatString)
							strftime(VDptr->domainName, 1024, szFormatString, &tmSunday);
					}

				}
			}


			// --------- SAVE DNR CACHE TO DISC
			if (MyPrefStruct.dnslookup) {
				StatusSetID( IDS_DNSCOMPLETE );	
				addressList->SaveLookups();
			}
		
			// If we havent hit STOP
			if( IsStopped() == 0 ) {

				// --------- SAVE DATABASE TO DB FILE
				// if we're processing into an FWA database then now is the time to save it
				if( v4DatabaseFileName ){
					if( logfiles_opened ){
						if( ignoredbyDBDateRange ){
							char date1[64], date2[64], date3[64], date4[64];

							DateTimeToString( db_starttime, date1 );
							DateTimeToString( db_endtime, date2 );
							DateTimeToString( logDateStart, date3 );
							DateTimeToString( logDateEnd, date4 );
							sprintf( buf2, "No hits were added to the database,\nbecause the database date range (%s - %s)\nIgnored all hits from logfile (%s - %s).\n", date1, date2, date3, date4 );
							UserMsg( buf2 );
						}
						else
						if( allTotalDatabaseRequests == allTotalRequests ){
							OutDebug( "DBASE: No new data added, so we dont need to save." );
						}
						else
						{
							StatusSetID( IDS_SAVEDATABASE );
							if( VDnum > 0 )
								DBIO_SaveAllToFile( VD, VDnum, const_cast<char*>(v4DatabaseFileName) );
							else
								DBIO_SaveToFile( VDptr, const_cast<char*>(v4DatabaseFileName) );
							StatusSetID(0);			// clear Status message (kEmptyString)
						}
					} else
						OutDebug( "DBASE: No log files have been opened to add data to dbase" );
				}

				
				//----------- CONVERT DATA TO LOG FILE
				if( logdata->convert_Flag == CONVERT_TOW3C )
				{
					Write_W3C( "", NULL );		// close open file.
				}

				//---------- WRITE REPORT
				if( produceReport )
				{
					OutDebug( "DEBUG: Writing report" );
					switch( logdata->reportType ) {
						case REPORT_TYPE_NORMAL: 
							if( !logdata->convert_Flag ) // Just converting a log file to another format, so don't write the reports
								WriteReportToDisk( logStyle, numLogs );
							break;
							// goodies
						case REPORT_TYPE_XML:
							// do nothing
							break;
					}
				}
			} else StatusSet( "stopped" );
		} 
		else
		if( ignoredbyDateRange ){
			char date1[64], date2[64], date3[64], date4[64];

			DateTimeToString( s_startingDate_t, date1 );
			// NOTE: add 1 day's worth of seconds minus 1 to the end date in order
			//       to add "23:59:59" to displayed end date
			DateTimeToString( s_endingDate_t+ONEDAY-1, date2 );
			DateTimeToString( logDateStart, date3 );
			DateTimeToString( logDateEnd, date4 );

			sprintf( buf2, ReturnString(IDS_ERR_DATERANGE), date1, date2, date3, date4 );
			UserMsg( buf2 );
		} else
		if( ignoredbyZeroBytes )
			UserMsg( ReturnString(IDS_ERR_ZEROBYTE) );
		else
		if( ignoredbyFilterIn )
			UserMsg( ReturnString(IDS_ERR_NOFILTER) );
		else
		if (noclientsExist)
			UserMsg( "No Clients we're found in this log data" );
		else
		if( badones )
			UserMsg( ReturnString(IDS_ERR_BADLOG));

		// ########################## SLEEP MODE ############################
		if( ( MyPrefStruct.live_sleeptype > 0) ){
			live_count++;
			//PerformNotification( VDptr, logDays, TRUE );

			// sleep for a while, and let the log file build up some more
			StatusSet( "Sleeping..." );
			i = GetSleeptypeSecs(MyPrefStruct.live_sleeptype);
			while( i>0 && !IsStopped() ){
				SleepSecs( 1 );
				i--;
			}

			if (!IsStopped()){
				logIdx = 0;
				VDnum = 0;
				VDptr = VD[VDnum];
				VDptr->time1 = alltime1 = timems();
				LogReadLine( 0, logRef[logIdx], fsFile[logIdx] );		// init readahead routines
				gFilesize = GetFileLength( fsFile[logIdx] );
				endall = FALSE;
				goto while_proc;
			}
		} else
			// NOTE: we may also do notification at the same time here also 
			PerformNotification( VDptr, logDays, TRUE );
		// ########################## SLEEP MODE ############################

	} //if( IsStopped() ) {

	if (logdata->reportType == REPORT_TYPE_NORMAL)
		return CloseAndQuitProcessing( logdata );
	else
		return IsStopped();
}


//=================================================================================================
//				P U B L I C   F U N C T I O N S   O N L Y   B E L O W   H E R E ! !
//=================================================================================================

long GoProcessLogs( ProcessDataPtr logdata )
{
#if USE_TRUE_TIME
	NMClear();
#endif
	long isStopped(0);

	// if we are doing clusters, lets alloc the ram for them
	if (MyPrefStruct.clusteringActive)
		InitClusters();

	InitLanguage( MyPrefStruct.language, 0 );

	// This variable indicates that we need to process the log set twice - once to produce
	// a report on the log set (only) and once to add the log set to an FWA database which 
	// we've determined to exist.
	bool performDBUpdatePhase(false);

	// This variable specifies the name of the FWA database that the first call to
	// PerformLogFileStatistics() is to use.  A zero means don't process into an FWA database.
	const char* v4DatabaseFileName=0;

	// if compact (v4) database setting is switched on
	if( MyPrefStruct.database_active && !MyPrefStruct.database_extended ) 
	{
		DEF_ASSERT( MyPrefStruct.database_file );

		// if user wants to add logs to the database (which exists) but wants the report to only include data from those logs
		if( !MyPrefStruct.database_no_report && MyPrefStruct.database_excluded && IsFileV4DataBase( MyPrefStruct.database_file ) )
		{
			// indicate that we need to carry out a seperate run in order to add the log set
			// to the FWA database
			performDBUpdatePhase=true;
		}
		// else performing normal database processing
		else
		{
			// setup the database file name
			v4DatabaseFileName=const_cast<const char*>( MyPrefStruct.database_file );
		}
	}

	// process the log set, possibly produce a report and update the FWA database if specified
	bool produceReport=!MyPrefStruct.database_active || !MyPrefStruct.database_no_report;
#if USE_TRUE_TIME
	produceReport=false;	// useful for profiling only statistic accumulation etc
#endif
	isStopped=PerformLogFileStatistics( logdata, v4DatabaseFileName, produceReport );

	// if the user didn't abort processing and if we need to carry out our second phase of 
	// processing then do so now
	if( !isStopped && performDBUpdatePhase )
	{
		// reprocess the log set, don't produce a report but do update the specified FWA database
		isStopped=PerformLogFileStatistics( logdata, MyPrefStruct.database_file, false );
	}

	// close our cluster structs
	if (MyPrefStruct.clusteringActive)
		FreeClusters();

#if USE_TRUE_TIME
	NMSaveNow(0);
#endif
	return isStopped;
}


VDinfoP LoadV4Database( long file )
{
	DEF_PRECONDITION( file );

	long	i;
	VDinfoP VDptr = NULL;

	VDnum = DBIO_LoadFromFile( file, VDnum, &allTotalDatabaseRequests );
	DBIO_Close( file );

	allfirstTime = allendTime = 0;

	for( i=0;i<=VDnum;i++)
	{
		VDptr = VD[i];

		if( allfirstTime==0  ) 	allendTime = allfirstTime = VDptr->firstTime;

		if( VDptr->firstTime < allfirstTime )	allfirstTime = VDptr->firstTime;
		if( VDptr->lastTime > allendTime )		allendTime = VDptr->lastTime;

		allTotalFailedRequests += VDptr->totalFailedRequests;
		allTotalRequests += VDptr->totalRequests;
		allTotalCachedHits += VDptr->totalCachedHits;
		allTotal404Hits += VDptr->total404Hits;
		allTotalRedirectedHits += VDptr->totalRedirectedHits;
		allTotalBytes +=	VDptr->totalBytes;
		allTotalBytesIn += VDptr->totalBytesIn;
	}
	db_starttime = allfirstTime;
	db_endtime = allendTime;

	return VD[0];
}


void ClearLookupCacheFile()
{
	CQDNSCache	tmpCQDNSCache(kDNSListIncr,FALSE);
	tmpCQDNSCache.SaveLookups();
}
