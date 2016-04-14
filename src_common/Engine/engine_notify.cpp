/*
	notify/ realtime messages

	new realtime mode cant support sessionhistory page information.
	it will use currect raeltime option with ability to optionaly
	make a report or not. Also it will flush out old or unneeded data
	to keep memory low usage.
	1. collect data
	2. sort data top x*2
	3. write report optionlly
	4. check notify rules and notify
	5. purge/flush old useless data as much as possible
	6. loop back to 1.

  
	2chars of list type
	next 2 chars = type of var
	next 2 chars = logic check ">=" "==" etc
	next text = the value
	last char = "&" or "|" for "AND" / "OR"

	ie...

	"TYPE.FIELD MATH VALUE"
	"03.05.3.40000&"

	if TYPE is 00 it means GLOBAL STATS
	if VALUE has % in it it means percentage of total for that field if possible
	if VALUE is a non digit then use match() comparisons

*/

#include "Compiler.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "myansi.h"
#include "zlib.h"

#include "config_struct.h"
#include "datetime.h"
#include "Utilities.h"
#include "OSInfo.h"
//#include "AppInternalStats.h"

#include "engine_notify.h"



extern "C" struct App_config MyPrefStruct;

// for every 'empty' cell/record, move one from the end ontop of it, thereby
// moving all empty cells to the end of the list and making a contiguous list of
// valid used cells, ie turn "XXXXX0XXX0XX0X0X00X-----" into "XXXXXXXXXX------------"
// RETURN new total
static void CleanEmptyList( VDinfoP VDptr, StatList* list )
{
	size_t numStats( list->GetStatListNum() );

	for( size_t i(0); i<numStats; ++i )
	{
		Statistic* p=list->GetStat(i);

		// if current stat has been flagged to be killed
		if( p->hash==0)
		{
			Statistic* lastp=list->GetStat(numStats-1);

			// while there's a stat at the end of the list that has been flagged to be killed
			while( lastp->hash==0 )
			{
				// nuke it
				memset( lastp, 0, sizeof(Statistic) );

				// have we reached our current stat?
				if( lastp==p )
				{
					break;
				}

				lastp=list->GetStat(--numStats-1);
			}

			// if we managed to find a valid stat at the end of the list then
			// copy it over the current one
			if( lastp->hash )
			{
				memcpy( p, lastp, sizeof( Statistic ) );		// copy stat
				memset( lastp, 0, sizeof( Statistic ) );		// delete orig stat
				p->id=i;
			}

			--numStats;
		}
	}

	// mustn't forget to update collecton size here!!
	list->num=numStats;

	// mark for recalc all next and header quick lookup tables.
	list->MarkForRebuild();
}


static StatList* FlushStatList( VDinfoP VDptr, StatList* pStats, long date, short keeptopx )
{
	const size_t	keepAtLeast(250);
	long			dateThreshold( date-ONEDAY*( keeptopx ? 7 : 5 ) );
	size_t			numStatsFlushed(0);
	Statistic*		pRareVisitorsStat=0;
	size_t			numStats( pStats->GetStatListNum() );

	for( size_t i(keepAtLeast); i<numStats; ++i )
	{
		Statistic* pThisStat=pStats->GetStat(i);

		// decide whether to keep the stat or not...
		long lastday(pThisStat->lastday);

		bool keepThisStat( lastday>dateThreshold );

		if( !keepThisStat )
		{
			if( pStats->totalRequests )
			{
				__int64 FilesPerc=(((__int64)pThisStat->GetFiles())*100)/pStats->totalRequests;

				keepThisStat=lastday>date-FilesPerc*8*ONEDAY;
			}

			if( !keepThisStat && pStats->totalBytes )
			{
				__int64 BytesPerc=(((__int64)pThisStat->GetBytes())*100)/pStats->totalBytes;

				keepThisStat=lastday>date-BytesPerc*8*ONEDAY;
			}
		}

		// if we've decided not to keep the stat then flush away
		if( !keepThisStat )
		{	
			// calc other client-based stats before deleting			
			AddClientDetailsToOthers( VDptr, pThisStat, i );

			// make a combined entry which merges the other items
			if ( pRareVisitorsStat )
			{
				pStats->AddStatsToStat( pRareVisitorsStat, pThisStat, FALSE );
				pRareVisitorsStat->timeStat=NULL;
				pRareVisitorsStat->sessionStat=NULL;
			}
			else
			{
				pRareVisitorsStat=pStats->AddStatsFromStat( "-Rare Visitors-", pThisStat );
			}

			// delete entry in list
			pThisStat->FreeData();

			// important! set the hash to zero in order for CleanEmptyList() to
			// recognise the fact that it is to be flushed
			pThisStat->hash=0;

			++numStatsFlushed;
		}
	}

	if( numStatsFlushed )
	{
		OutDebugs( "Flushed %d visitors", numStatsFlushed );
		VDptr->totalFlushedClients += numStatsFlushed;

		// Zero fill the killed records and move all empty records to end of list
		CleanEmptyList( VDptr, pStats );
	
		VDptr->totalUniqueClients = pStats->num + VDptr->totalFlushedClients;
	}

	return pStats;
}


// of selected lists, keep only stats that are either with in the last time frame
// or that are with in the top X in the list
// this way we keep our memory usage at a minimum while providing the best possible data we can
void FlushOldData( VDinfoP VDptr, long date, short keeptopx )
{
	FlushStatList( VDptr, VDptr->byClient, date, keeptopx );		// PURGE data base list
}


// -------------------------------------------------------------------------------------------------

char *StatListStr[] = {
	"Hour", "Weekday", "Month", "Date", "User",
	"Client", "SourceAddress", "SecondDomain", "Country",
	"Organization", "Region", "File", "Browser",
	"OperatingSystem", "Referal", "ReferalSite", "Directory",
	"TopDirectory", "FileType", "Error", "ErrorURL",
	"Page", "Search String", "SearchEngine", "Download",
	"Advert", "AdCamp", "Billing", 0
};
char *StatFieldStr[] = {
	"Name", "Urls/Files", "Bytes", "Bytes-In", "Visits", "Errors", "VisitsTotal", 0
};
char *LogicStr[]= { ">=", "<=", "==", "!=", 0 };


static char *GetStatListStr( long n )
{
	return StatListStr[n];
}


static long GetStatListStrIdx( char *txt )
{
	long n=0;
	if ( txt ){
		while( StatListStr[n] ){
			if ( !strcmpd( StatListStr[n], txt ) ) return n;
			n++;
		}
	}
	return -1;
}


static long GetStatFieldtStrIdx( char *txt )
{
	long n=0;
	if ( txt ){
		while( StatFieldStr[n] ){
			if ( !strcmpd( StatFieldStr[n], txt ) ) return n;
			n++;
		}
	}
	return -1;
}


static long GetLogicStrIdx( char *txt )
{
	long n=0;
	if ( txt ){
		while( LogicStr[n] ){
			if ( !strcmpd( LogicStr[n], txt ) ) return n;
			n++;
		}
	}
	return -1;
}


static StatList* GetStatList( VDinfoP VDptr, long id )
{
	switch( id ){
		case 1:		return VDptr->byHour; break;
		case 2:		return VDptr->byWeekday; break;
		case 3:		return VDptr->byMonth; break;
		case 4:		return VDptr->byDate; break;
		case 5:		return VDptr->byUser; break;
		case 6:		return VDptr->byClient; break;
		case 7:		return VDptr->byServers; break;
		case 8:		return VDptr->bySecondDomain; break;
		case 9:		return VDptr->byDomain; break;
		case 10:	return VDptr->byOrgs; break;
		case 11:	return VDptr->byRegions; break;
		case 12:	return VDptr->byFile; break;
		case 13:	return VDptr->byBrowser; break;
		case 14:	return VDptr->byOperSys; break;
		case 15:	return VDptr->byRefer; break;
		case 16:	return VDptr->byReferSite; break;
		case 17:	return VDptr->byDir; break;
		case 18:	return VDptr->byTopDir; break;
		case 19:	return VDptr->byType; break;
		case 20:	return VDptr->byErrors; break;
		case 21:	return VDptr->byErrorURL; break;
		case 22:	return VDptr->byPages; break;
		case 23:	return VDptr->bySearchStr; break;
		case 24:	return VDptr->bySearchSite; break;
		case 25:	return VDptr->byDownload; break;
		case 26:	return VDptr->byAdvert; break;
		case 27:	return VDptr->byAdCamp; break;
		case 28:	return VDptr->byBilling; break;
	}
	return 0;
}


static char *GetStatField( Statistic *stat, long id, __int64 *val )
{
	char *string=0;

	switch( id ){
		case 1:		*val = 0; string = stat->GetName(); break;
		case 2:		*val = stat->files; break;
		case 3:		*val = stat->bytes; break;
		case 4:		*val = stat->bytesIn; break;
		case 5:		*val = stat->visits; break;
		case 6:		*val = stat->errors; break;
		case 7:		*val = stat->visitTot; break;
	}
	return string;
}


//"03.05.3.40000&"
// 0123456789
long CheckForNotifyRules( VDinfoP VDptr, long date, char notifyfules[MAX_FILTERUNITS][MAX_FILTERSIZE] )
{
	long	i, n=0, dbase, trueon, truecount, logic, percent=0, field;
	char	*line, *fieldStr, *logicStr, *valueStr, *dataStr;
	StatList		*byStat;
	FILE	*nout = 0;

	while( (line=notifyfules[n]) && n<MyPrefStruct.filterdata.notifyTot ){
		__int64		valuenum = -1, datavalue;
		char		tline[200], lline[200];

		lline[0] = 0;

		// get string details and convert to values!
		dbase = GetStatListStrIdx( line );
		if( fieldStr = mystrchr( line, ' ' ) ) fieldStr++;
		field = GetStatFieldtStrIdx( fieldStr );
		if( logicStr = mystrchr( fieldStr, ' ' ) ) logicStr++;
		logic = GetLogicStrIdx( logicStr );
		if( valueStr = mystrchr( logicStr, ' ' ) ) valueStr++;

		byStat = GetStatList( VDptr, dbase+1 );

		if ( !byStat ) return 0;

		truecount = 0;
		if ( isdigit( *valueStr ) )
			valuenum = (__int64)atof( valueStr );

		// loop thru all clients
		for( i=0; i<byStat->num; i++){

			dataStr = GetStatField( &byStat->stat[i], field+1, &datavalue );
			trueon = 0;
			if ( valuenum != -1 ) {
				switch( logic ){
					case 0: if ( datavalue >= valuenum ) trueon = 1; break;
					case 1: if ( datavalue == valuenum ) trueon = 1; break;
					case 2: if ( datavalue <= valuenum ) trueon = 1; break;
					case 3: if ( datavalue != valuenum ) trueon = 1; break;
				}
			} else {
				switch( logic ){
					case 0:
					case 1:	if ( match( dataStr, valueStr, 1 ) ) trueon = 1; break;
					case 2:
					case 3:	if ( !match( dataStr, valueStr, 1 ) ) trueon = 1; break;
				}
			}
			truecount += trueon;

			if ( trueon ) {
				char t1Str[40], t2Str[40];

				if ( !nout ){
					if ( ! (nout = fopen( NOTIFY_OUTFILENAME, "w+" )) ) return 0;
				}

				DateTimeToString( date, t1Str );
				DateTimeToString( byStat->stat[n].lastday, t2Str );

				if ( valuenum != -1 )
					sprintf( tline, "%s : (%s) %s->%s = '%ll' last %s\n", t1Str, byStat->stat[i].GetName() , GetStatListStr( dbase ), fieldStr, datavalue, t2Str );
				else
					sprintf( tline, "%s : (%s) %s->%s = '%s' last %s\n", t1Str, byStat->stat[i].GetName(), GetStatListStr( dbase ), fieldStr, dataStr, t2Str );

				if ( strcmp( tline, lline ) ){
					fprintf( nout, tline );
					mystrcpy( lline, tline );
				}
			}
		}
		n++;
	}
	if ( nout )	fclose( nout );
	return truecount;
}


extern "C" void ShowFile( char *doc );
extern "C" void MailTo( char*, char *, char *);

long PerformNotification( VDinfoP VDptr, long date, short keeptopx )
{
	long found=0;

	if ( MyPrefStruct.notify_type ){
		found = CheckForNotifyRules( VDptr, date, MyPrefStruct.filterdata.notify );
		switch( MyPrefStruct.notify_type ){
			case 1:
				SysBeep(1);
				// ShowFile( NOTIFY_OUTFILENAME );			- commented out for compat build RHF
				break;
			case 2:
#if DEF_WINDOWS
				if ( MyPrefStruct.notify_destination[0] )
					MailTo( MyPrefStruct.notify_destination, "Report Notification", "test" );
#endif
				break;
		}
	}

	return found;
}



