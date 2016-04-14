/*****************************************************************************
Copyright (C) 2000-2004

File 	:	EngineCluster.cpp
Created :	Friday, 6 July 2001
Author 	:	Julien Crawford [JMC]

Abstract:	This file is one of the pieces of the old engine.[cpp|h].
			The functions it exposes are essentially the Cluster functions.

Uses	:	Store it in Prefs->ClusterNames[0...x], and ->ClusterTot
			each field will be , "IP,PathMatch=Name"

			Record each hitdetails in byClusters->stat[0...x]
******************************************************************************/
                          
// ****************************************************************************
// System includes.
// ****************************************************************************
#include "FWA.h"
#include <string.h>


// ****************************************************************************
// Project includes.
// ****************************************************************************
#include "EngineCluster.h"
#include "engine_dbio.h"
#include "log_io.h"
#include "engine_proc.h"
#include "serialReg.h"


// ****************************************************************************
// Other Engine based includes, as there is a fair bit of cross dependancy.
// ****************************************************************************
#include "engine_process.h"		// for LoadDataBase()
#include "EngineStatus.h"
#include "EngineBuff.h"
#include "EngineVirtualDomain.h"
#include "EngineParse.h"
#include "Utilities.h"

#ifdef DEF_WINDOWS				// WINDOWS include
	#include "resource.h"
#endif

#ifdef DEF_UNIX
	#include "ResDefs.h"
#endif

#ifdef DEF_MAC
	#include "MacStatus.h"
	#include "ResDefs.h"
#endif


// ****************************************************************************
// Globals located elsewhere that are NOT in *.h files.
// ****************************************************************************
extern  short		logStyle;
extern	__int64		gFilesize;
extern	long		logfiles_opened;
extern	time_t		logDays;
extern	char		statusStr[200];
extern	int			endall;


// ****************************************************************************
// Local Definitions
// ****************************************************************************
#define	TOTALCLUSTERS	256


// ****************************************************************************
// Globals
// ****************************************************************************
static LogReadData	*ClusterData[TOTALCLUSTERS];
static HitDataRec	*ClusterLines[TOTALCLUSTERS];


// ****************************************************************************
// Extern functions not defined in *.h files.
// ****************************************************************************
#ifndef DEF_MAC
extern "C" int StatusSetID( int id, ... );	// on Mac, is defined in MacStatus.h
#endif


// ****************************************************************************
// Private implementation.
// ****************************************************************************

static void ClusterCloseClusterFile( int lp )
{
	OutDebugs( "Closing Cluster %d. File %d %s", lp, ClusterData[lp]->index, ClusterData[lp]->filename );
	LogClose( ClusterData[lp]->fp, ClusterData[lp]->filename );
	ClusterData[lp]->fp = 0;

	ClusterData[lp]->ReadCount = 0;
	ClusterData[lp]->buffCount = 0;
	ClusterData[lp]->readIndex = 0;
	ClusterData[lp]->posIndex = 0;
	ClusterData[lp]->logError = 0;
	ClusterData[lp]->linepos = 0;
	ClusterData[lp]->filename = 0;
}


static int ClusterGetNextFile( int lp, int nextIdx,  char **fsFile, int logNum )
{
	long	LogFileStat = FALSE;

	ClusterCloseClusterFile( lp );

	if ( nextIdx < logNum )
	{
		ClusterData[lp]->fp = LogOpen( fsFile[nextIdx], &gFilesize );

		if ( ClusterData[lp]->fp )
		{
			ClusterData[lp]->index = nextIdx;
			ClusterData[lp]->filename = fsFile[nextIdx];
			ClusterData[lp]->buff = ClusterData[lp]->linebuff;
			ClusterData[lp]->ReadBuff = ClusterData[lp]->ReadBuff1;

			LogFileStat = MLogReadLine( ClusterData[lp], ClusterData[lp]->fp, ClusterData[lp]->filename, 0 );
			logfiles_opened++;
			ClusterData[lp]->linepos=1;
			OutDebugs( "Loading new Cluster file %d/%d %s", nextIdx,logNum, ClusterData[lp]->filename );
			return 1;
		}
		else
		{
			OutDebugs( "Cant read cluster %s", ClusterData[lp]->filename);
		}
	}
	else
	{
		OutDebugs( "End of clusters, last is %d", nextIdx );
	}
	return 0;
}

/*------------------------
Ok now, lets


DATA STORAGE:
	prefs->clusterNames[x]

FORMAT:
	IP\0PATH\0CLUSTERNAME

CALLING METHOD:
	if ( MyPrefStruct.clusterNamesTot )
	{
		if ( p = MatchClusterPaths( sourceAddr, logPath, MyPrefStruct.clusterNames, MyPrefStruct.clusterNamesTot ) )
		{
			byClusters->IncrStatsMore( BytesOut,BytesIn, p, logDays, urlerrorFlag );	
		}
	}

FUTURE:
	to speed things up, perhaps convert the 3 items in an array of reference them via pointers, such as;
	clusterPtrs[i][0...2]

  /-----------------------
*/


// ****************************************************************************
// Public Interface
// ****************************************************************************
void InitClusters()
{
	long count, ioBufferSize;

	// request dynamic buffer io size
	if ( MyPrefStruct.numClustersInUse < 5 )
		ioBufferSize = (1024*1024);
	else
		ioBufferSize = (1024*1024*8) / MyPrefStruct.numClustersInUse;

	// Make sure it cant be too small
	if ( ioBufferSize < NETREADBUFFSIZE )
		ioBufferSize = NETREADBUFFSIZE;

	for( count = 0; count < MyPrefStruct.numClustersInUse; count++ )
	{
		if( count < GetClusterValue() )
		{
			ClusterData[count] = (LogReadData*)malloc( sizeof(LogReadData) );
			MemClr( ClusterData[count], sizeof(LogReadData) );
			ClusterData[count]->buff1Size = ioBufferSize;
			ClusterData[count]->ReadBuff1 = (char*)malloc( ioBufferSize );
			//MemClr( ClusterData[count]->ReadBuff1, ioBufferSize );

			ClusterLines[count] = (HitDataRec*)malloc( sizeof(HitDataRec) );
			MemClr( ClusterLines[count], sizeof( HitDataRec ) );
		}
		else
		{
			ClusterData[count] = NULL;
			ClusterLines[count] = NULL;
		}
	}
}


void FreeClusters()
{
	long count;

	for( count = 0; count < MyPrefStruct.numClustersInUse; count++ )
	{
		if( count < GetClusterValue() )
		{
			if( ClusterData[count]->fp )
			{
				LogClose( ClusterData[count]->fp, ClusterData[count]->filename );
			}

			free(ClusterData[count]->ReadBuff1);
			free(ClusterData[count]);
			free(ClusterLines[count]);
		}
		else
		{
			ClusterData[count] = NULL;
			ClusterLines[count] = NULL;
		}
	}
}


// now this is hairy, and worth $1900 per cluster? Thats $1200/line of code using 32 clusters... hmmmm
VDinfoP ClusterGetLineDetails( VDinfoP VDptr, char **fsFile, int logNum, HitDataRec *Line, long *logError, const char* v4DatabaseFileName )
{
	long	LogFileStat = FALSE, i;
	long	lp, ret;
	static long earliestIdx=0, nextIdx=0, earliestDate;
	static long maxcluster;


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// ok no log files open, so lets open them all up and preread al their lines and decode the lines to 
	// structure data so it can be used on the next loops to decide which ones to use for processing.
	if ( VDptr == NULL ){
		maxcluster = MyPrefStruct.numClustersInUse;

		if ( maxcluster > GetClusterValue() )
			maxcluster = GetClusterValue();

		if ( maxcluster > TOTALCLUSTERS )
			maxcluster = TOTALCLUSTERS;

		if ( maxcluster > logNum )
			maxcluster = logNum;

		OutDebugs( "Init Clusters, reading %d logs of %d", maxcluster, logNum );

		// Get log format from FIRST log file
		{
			long fp;
			__int64 fs;
			fp = LogOpen( fsFile[0], &fs );
			LogGetFormat( fp , ClusterLines[0], fsFile[0] );
			LogClose( fp, fsFile[0] );
		}

		// open all cluster logs and go to first valid line
		for( lp=0; lp<maxcluster; lp++ )
		{
			ClusterData[lp]->fp = LogOpen( fsFile[lp], &gFilesize );

			if ( ClusterData[lp]->fp )
			{
				ClusterData[lp]->index = lp+1;
				ClusterData[lp]->filename = fsFile[lp];
				ClusterData[lp]->buff = ClusterData[lp]->linebuff;
				ClusterData[lp]->ReadBuff = ClusterData[lp]->ReadBuff1;
				ClusterData[lp]->linepos=1;
				logfiles_opened++;


				LogFileStat = MLogReadLine( ClusterData[lp], ClusterData[lp]->fp, ClusterData[lp]->filename, 0 );
				ClusterData[lp]->linepos++;

				i=0;
				ret = -1;
				while( i<10000 && ret<=0 && LogFileStat ){
					ret = ProcessLogLine( ClusterData[lp]->buff, ClusterLines[lp] );
					if ( ret <=0 ){
						LogFileStat = MLogReadLine( ClusterData[lp], ClusterData[lp]->fp, ClusterData[lp]->filename, 0 );
						ClusterData[lp]->linepos++;
					}
					i++;
				}
				ClusterData[lp]->logError = ret;

				OutDebugs( "Init Cluster %d %s, status = %d, line = %d", lp, ClusterData[lp]->filename, ClusterData[lp]->logError, ClusterData[lp]->linepos );
			} else
				OutDebugs( "Cant open Cluster %d %s", lp, fsFile[lp] );
		}
		earliestIdx = -1;
		nextIdx = lp;

		// Reload V4 database if specified via settings (reprocessing of databases in cluster mode not supported)
		if(v4DatabaseFileName)
		{
			StatusSetID( IDS_LOADDATABASE );
			long file( DBIO_Open( v4DatabaseFileName ) );
			if( file )
			{
				VDptr = LoadV4Database( file );
			}
			StatusSetID(0);						// clear Status message
		}

		if ( VDptr == NULL ){
			CorrectVirtualName( fsFile[0], buf2, 1 );
			VDptr = InitVirtualDomain( VDnum, buf2, logStyle );
			if ( !VDptr ){
				OutDebugs( "Cant init domain %s", buf2 );
				StopAll(CANTMAKEDOMAIN);
				*logError = -1;
			}
		}

	} else
	// once we init and start, we run this block all the time for each log file
	// processing only the earliest dated lines...
	if ( VDptr ){
		long	date=1;

		// Read next line.
		if ( earliestIdx >= 0 ){
			lp = earliestIdx;
			LogFileStat = MLogReadLine( ClusterData[lp], ClusterData[lp]->fp, ClusterData[lp]->filename, 0 );
			ClusterData[lp]->linepos++;


			// End of current log, go get next log
			if ( !LogFileStat ){
				if ( LogFileStat = ClusterGetNextFile( lp, nextIdx, fsFile, logNum ) ){
					nextIdx++;
				}
			}

			// Decode this line if its invalid, keep getting more lines until we find a valid line.
			{
				i=0;
				ret = -1;

				//OutDebugs( "Using line %d in %s, %s", ClusterData[lp]->linepos, ClusterData[lp]->filename, ClusterData[lp]->buff );

				while( ret<=0 && LogFileStat && !IsStopped() && i<20){
					ret = ProcessLogLine( ClusterData[lp]->buff, ClusterLines[lp] );
					if ( ret <=0 ){
						//if ( ret == -2 )
						//	OutDebugs( "skipping Format line %d in %s", ClusterData[lp]->linepos, ClusterData[lp]->filename );
						//else
						//	OutDebugs( "skipping bad line %d in %s", ClusterData[lp]->linepos, ClusterData[lp]->filename );

						LogFileStat = MLogReadLine( ClusterData[lp], ClusterData[lp]->fp, ClusterData[lp]->filename, 0 );
						ClusterData[lp]->linepos++;
					}
					i++;
				}
				ClusterData[lp]->logError = ret;
			}
		} //else OutDebugs( "earliestIdx %d", earliestIdx );
	}


	// Find next earliest line.
	if ( VDptr ){
		struct tm		logDate;
		long	tested = 0;
		earliestIdx = -1;
		earliestDate = 0x7fffffff;

		// read multiple log files lines to find the earliest time
		for( lp=0; lp<maxcluster; lp++ ){
			if( ClusterData[lp]->fp ){
				tested++;
				//memset( &logDate, 0, sizeof(struct tm) );
				StringToDaysDate( ClusterLines[lp]->date, &logDate, 0 );
				StringToDaysTime( ClusterLines[lp]->time, &logDate );
				Date2Days( &logDate, &logDays );
				if( logDays < earliestDate ) {
					earliestDate = logDays;
					earliestIdx = lp;
				}
			}
		}

		// Yes we found a line with earliest time
		if ( earliestIdx >= 0 ){
			if ( logNum>1 )
				sprintf( statusStr, " Cluster %d File %i/%i (time=%.8s)",  earliestIdx, ClusterData[earliestIdx]->index, logNum, ClusterLines[earliestIdx]->time );
				//sprintf( statusStr, " Cluster %d, File %i/%i (t1=%.8s,t2=%.8s,t3=%.8s,t4=%.8s", earliestIdx, ClusterData[earliestIdx]->index, logNum, ClusterLines[0]->time, ClusterLines[1]->time, ClusterLines[2]->time, ClusterLines[3]->time );
			else
				*statusStr = 0;

			memcpy( Line, ClusterLines[earliestIdx], sizeof( HitDataRec ) );
			Line->logFileName = ClusterData[earliestIdx]->filename;
			*logError = ClusterData[earliestIdx]->logError;
		} else {
			Line->logFileName=0;
			endall = TRUE;
			*logError = -1;

			OutDebugs( "End all clusters =  %d, tested=%d,logDays=%d, edate=%d", earliestIdx, tested, logDays, earliestDate );
		}
	}
	
	return VDptr;
}


// Cluster array stored as , NAME,IP,PATH
// fixed MatchClusterPaths() where it would ignore a check if the ipString was zero, ie if no vhost info was there at all.
char *MatchClusterPaths( char *ipString, char *logPath, char cluster[MAX_FILTERUNITS][MAX_FILTERSIZE], long clusterTot )
{
	if ( ipString || logPath )		// Fixed here, either of the first two params are optional.
	{
		long i;
		char *clusterName, *clusterIP, *clusterPath;

		for ( i = 0; i<clusterTot; i++) 
		{
			clusterName = cluster[i];
			clusterIP = strchr( clusterName, 0 )+1;
			clusterPath = strchr( clusterIP, 0 )+1;

			if ( clusterIP && *clusterIP && *clusterIP != '-' && ipString )
			{
				if ( match( ipString, clusterIP , TRUE ) )
					return clusterName;
			}

			// Ignore a path pattern of - or empty string
			if ( clusterPath && *clusterPath && *clusterPath != '-' && logPath )
			{
				if ( match( logPath, clusterPath,TRUE ) )
					return clusterName;
			}
			// TODO: handle multiple logPathmatches, "/*.log | *.access"
		}

	}
	return NULL;
}


// ****************************************************************************
// Copyright (C) 2000-2004
// ****************************************************************************

