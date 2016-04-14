#include "FWA.h"

#include <cstdio>
#include <cstring>
#include <string.h>

#include "StatList.h"
#include "myansi.h"
#include "datetime.h"
#include "config.h"				// for extern of "MyPrefStruct"
#include "config_struct.h"		// for extern of MAX_DOMAINS
#include "VirtDomInfo.h"		// for VDinfoP.
#include "Hash.h"

#include "StatDefs.h"
#include "StatCmp.h"

#include "EngineStatus.h"
#include "engine_drill.h"


void ClearLastday( StatList *stat )
{
	long i = 0;
	Statistic *p;

	if ( stat ){
		while( p = stat->GetStat(i) ){
			p->lastday = -1;
			i++;
		}
	}
}








bool
StatList::TestAnyMatches( FilterFunc pFilterIndex )
{
	// **************************************************************************
	// If no filter is supplied then behave as the 'no-filter' function.
	// That is, if there are any values then we can sasfely return true.
	// **************************************************************************
	if (!pFilterIndex)
		return this->GetStatListNum()>0;

	for (int i=0; i<this->num; ++i)
	{
		Statistic *p = GetStat(i);
		if (pFilterIndex(p))
			return true;
	}

	return false;
}



// ------- STAT FILTER CALL BACK FITLERS
int Filter_IsOther( class Statistic *p ){
	return (p->GetPageType() == URLID_UNKNOWN);
}

int Filter_IsPage( class Statistic *p ){
	return (p->GetPageType() == URLID_PAGE);
}

int Filter_IsPageCGI( class Statistic *p ){
	return (p->GetPageType() == URLID_PAGECGI);
}

int Filter_IsDownload( class Statistic *p ){
	return (p->GetPageType() == URLID_DOWNLOAD);
}

int Filter_IsUpload( class Statistic *p ){
	return (p->GetPageType() == URLID_UPLOAD);
}

int Filter_IsAudioClip( class Statistic *p ){
	return (p->GetPageType() == URLID_CLIPAUDIO);
}

int Filter_IsVideoClip( class Statistic *p ){
	return (p->GetPageType() == URLID_CLIPVIDEO);
}

int Filter_IsClip( class Statistic *p ){
	return (p->GetPageType() == URLID_CLIPVIDEO) || (p->GetPageType() == URLID_CLIPAUDIO);
}

int Filter_IsLiveAudioClip( class Statistic *p ){
	return (p->GetPageType() == URLID_LIVEAUDIO);
}

int Filter_IsLiveVideoClip( class Statistic *p ){
	return (p->GetPageType() == URLID_LIVEVIDEO);
}

int Filter_IsLiveClip( class Statistic *p ){
	return (p->GetPageType() == URLID_LIVEVIDEO) || (p->GetPageType() == URLID_LIVEAUDIO);
}

int Filter_IsAnyClip( class Statistic *p ){
	return (p->GetPageType() == URLID_CLIPVIDEO) || (p->GetPageType() == URLID_CLIPAUDIO ||
			p->GetPageType() == URLID_LIVEVIDEO) || (p->GetPageType() == URLID_LIVEAUDIO );
}

int Filter_IsUndefined( class Statistic *p ){
	return (p->GetPageType() == URLID_UNDEFINED);
}
// -----------------------------------------------------------------------------------


unsigned long
StatList::GetTotalbyFilter( FilterFunc pFilterIndex )
{
	unsigned long lTotal = 0;
	if (!pFilterIndex)
		pFilterIndex = m_FilterIndex;

	if (!pFilterIndex)
		return this->GetStatListNum();

	for (int i=0; i<this->num; ++i)
	{
		Statistic *p = GetStat(i);
		if (pFilterIndex(p))
			lTotal++;
	}

	return lTotal;
}


#define	SUMOF(x)	for (i=0; i<this->num; ++i){  p = GetStat(i);	if( p->GetPageType() == urlid ){ lTotal+=x;} }

unsigned long
StatList::GetTotalSum_byFile( int valuetype, unsigned char urlid )
{
	unsigned long lTotal = 0, i;
	Statistic *p;

	switch( valuetype )
	{
		case VALUE_TOTALNUM:		SUMOF(1); break;
		case VALUE_TOTALHITS:		SUMOF(p->files); break;
		case VALUE_TOTALBYTES:		SUMOF(p->bytes); break;
		case VALUE_TOTALBYTESIN:	SUMOF(p->bytesIn); break;
		case VALUE_TOTALVISITS:		SUMOF(p->visits); break;
		case VALUE_TOTALVISITORS:	SUMOF(p->counter); break;
		case VALUE_TOTALPAGES:		SUMOF(p->counter4); break;
		case VALUE_TOTALERRORS:		SUMOF(p->errors); break;
	}
	return lTotal;
}
















////////////////////////////////////////////////////////////////////////////
//	Global methods to manipulate Statlist.
//	This should be a member function, even if it has to be static.
////////////////////////////////////////////////////////////////////////////
StatList *ClearStat( VDinfoP VDptr, StatList *byStat, long units )
{
	if ( byStat ){
		char a,b,c;
		a = byStat->doTimeStat;
		b = byStat->doSessionStat;
		c = byStat->useOtherNames;

		if ( byStat->num > 0 ){
			delete byStat;
			byStat	= new StatList(VDptr,units);
			byStat->doTimeStat = a;
			byStat->doSessionStat = b;
			byStat->useOtherNames = c;
		}
	} else
		byStat	= new StatList(VDptr,units);

	return byStat;
}



Statistic	*StatList::GetStat( int index )
{
	if( index < num ) 
	{
#ifdef SEGMENTED_STATS
		tableNum = index>>15;
		tableIdx = index&0x7fff;
		stat = tables[tableNum];
		return &stat[tableIdx];
#else
		return &stat[index];
#endif
	} else
		return NULL;
}



//
// Get first stat based on filter
// Store the 'current_position' in temp2 of the stat
Statistic	*StatList::GetFirstStat( void )
{
	Statistic *p = GetStat( 0 );
	temp2 = 0;
	if ( m_FilterIndex( p )==FALSE )
		p = GetNextStat( p );
	return p;
}

// Follow to next item based on the filter index...
// using temp2 as the 'current_position'
Statistic	*StatList::GetNextStat( Statistic *p )
{
	if( stat && p )
	{
		temp2++;
		p = GetStat( temp2 );
		if( m_FilterIndex )
		{
			while( (temp2<num) && (p) && (m_FilterIndex(p)==FALSE) )
			{
				temp2++;
				p = GetStat( temp2 );
			}
		}
		return GetStat( temp2 );
	} else
		return NULL;
}


Statistic	*StatList::GetEmptyStat( void )
{
	if ( num < maxNum ) {
#ifdef SEGMENTED_STATS
		tableIdx = num&0x7fff;
		stat = tables[tableTot];
		return &stat[tableIdx];
#else
		return &stat[num];
#endif
	} else
		return NULL;
}


/* FindStatbyName - search list for matching string */
Statistic* StatList::FindStatbyName(char* str)
{
	return str ? FindHashStat( HashStr(str, 0 ) ) : 0;
}


// seach for said item using HASH, and return the item Statistic
Statistic* StatList::FindHashStat( unsigned long hash )
{
	if( current && current->hash==hash )
	{
		return current;
	}

	// This call only rebuilds the headers iff they have been invalidated.
	ReBuildHeaders();

	unsigned long headIndex( hash & indexSize );
	DEF_ASSERT(headIndex >= 0);
	DEF_ASSERT(headIndex <= indexSize);
	
	Statistic* p=headPtr[headIndex];
	
	// search thru all strings of same hash pattern part
	while(p)
	{		
		if( hash==p->hash )
		{			
			return current=p;
		}

		p=p->next;					// next string...
	}

	return NULL;
}

// seach for said item using HASH, and return its index location
int StatList::FindHash( unsigned long hash )
{
	register	Statistic	*p;

	p = FindHashStat( hash );

	if ( p )
		return p->id;
	else
		return -1;
}


/* SearchStat - search list for matching string, creating new entry if not found */
Statistic	*StatList::SearchStat(char *str)
{
    DEF_PRECONDITION( str );
    DEF_PRECONDITION( *str );
 
	// make sure its a pointer and not a null or a small number by accident
	if( ! ((long)str & 0xfffff000) )
		return NULL;

	unsigned 	long hash, headIndex;
	long		len;
	register Statistic	*p, *h;
	/* search through stat names (most recent first for speed) */
	hash = HashStr( str,&len);
	p = FindHashStat( hash );
	if ( p )
		return p;

	headIndex = hash & (indexSize);

	DEF_ASSERT(headIndex >= 0);
	DEF_ASSERT(headIndex <= indexSize);
	h = headPtr[headIndex];

	// if no room to put new data in , make more data
	if (num >= maxNum) {
		h = GrowStatList( headIndex );
	}

	// find address of new data at the end of the table stat
	current = p = GetEmptyStat();	//GetStat(num);

	// New method to store IPs into the clients details, instead of wastefull strings
	if( useOtherNames == NAMEIS_IP ){
		long ip;
		if ( ip=IPStrToIPnum(str) ){
			p->name = (char*)ip;
			p->length = -1;
		} else
			p->StoreName( str );			// ADD A NEW STAT TO DATA BASE
	} else
	if( useOtherNames ){
		// All static or mirror names have length zero , only REAL name strings have a length here.
		p->name = str;
		p->length = 0;
	} else {
		p->StoreName( str );			// ADD A NEW STAT TO DATA BASE
	}
	
	if ( p->GetName() ) {
		if ( (doTimeStat&TIMESTAT_DAYSHISTORY) )
		{
			p->timeStat = new TimeRecStat;
			//Add secondary TIMESTAT, using TimeRecStat pointer var to conserve space
			if ( (doTimeStat&TIMESTAT_DIRECT2) )
				p->timeStat->timeStat2 = new TimeRecStat;
		}
		if ( doSessionStat == SESSIONSTAT_CLICKSTREAM) {
			p->sessionStat = new SessionStat;	//SESSION HISTORY
			p->sessionStat->parenthash = hash;
			p->sessionStat->vd = vd;
		} else
		if ( doSessionStat == SESSIONSTAT_CLIPDATA) {
			p->clipData = new ClipStat;
		}

		p->id		= num;		//add array index number
		p->hash		= hash;
		p->next		= h;		//add ptr to next entry in list
		p->counter	= 0;
		p->counter2	= 0xff;
		p->counter3	= 0xff;
		p->errors	= 0;
		p->visits 	= 0;
		p->bytes 	= 0;		//4294000000;

		if ( h ){
			p->nextHash	= h->hash;
		} else {
			p->nextHash	= 0;
		}

		DEF_ASSERT(headIndex >= 0);
		DEF_ASSERT(headIndex <= indexSize);
		headPtr[headIndex]		= p;			//this is the new index
		headHashRef[headIndex]	= hash;
	} else {
		MemoryError( "Cant alloc - new character", len+1 );
		return p;
	}
	num++;
	return p;
}

void StatList::IncrStatsTotals( __int64 byte_count, __int64 byte_countin, long ctime_now)
{
	totalRequests++;
	totalBytes += byte_count;
	totalBytesIn += byte_countin;
}


// IncrStats implementation function - increment the stats for given Statistic
void StatList::IncrStats( __int64 byte_count, __int64 byte_countin, Statistic* p, long ctime_now)
{
	DEF_PRECONDITION(p);

	p->IncrStats(byte_count,byte_countin);	// increment individual sums
	p->lastday = ctime_now;
	IncrStatsTotals( byte_count, byte_countin, ctime_now );
}



// IncrStats - increment the stats for given string
Statistic* StatList::IncrStats( __int64 byte_count, __int64 byte_countin, char *str, long ctime_now)
{
	register Statistic* p = NULL;

	p = SearchStat(str);	// find/create the Statistic

	if( p )
		IncrStats( byte_count, byte_countin, p, ctime_now );  // call our implementation function
	
	return p;
}


// IncrStatsMore implementation function - increment the stats for given Statistic and include error count
void StatList::IncrStatsTotalsMore( __int64 byte_count, __int64 byte_countin, long ctime_now, long flags )
{
	IncrStatsTotals( byte_count, byte_countin, ctime_now );

	if ( (flags & DRILLF_COUNTER) ){
		totalCounters++;
	}
	if ( (flags & DRILLF_COUNTER4) ){
		totalCounters4++;
	}
	if ( (flags & DRILLF_INCVISITS) ){
		totalVisits++;
	}
	if ( (flags & DRILLF_INCERR) ){
		totalErrors++;
	}
}


// IncrStatsMore implementation function - increment the stats for given Statistic and include error count
void StatList::IncrStatsMore( __int64 byte_count, __int64 byte_countin, Statistic* p, long ctime_now, long flags )
{
	DEF_PRECONDITION(p);

	IncrStats( byte_count, byte_countin, p, ctime_now );

	if ( (flags & DRILLF_COUNTER) ){
		p->counter++;
		totalCounters++;
	}
	if ( (flags & DRILLF_COUNTER4) ){
		p->counter4++;
		totalCounters4++;
	}
	if ( (flags & DRILLF_INCVISITS) ){
		p->visits++;
		totalVisits++;
	}
	if ( (flags & DRILLF_INCERR) ){
		p->errors++;
		totalErrors++;
	}
}


// IncrStatsMore - increment the stats for given string and include error count
Statistic* StatList::IncrStatsMore( __int64 byte_count, __int64 byte_countin, char *str, long ctime_now, long flags )
{
	// find/create the Statistic
	register Statistic* p=SearchStat(str);

	if( p )
	{
		IncrStatsMore( byte_count, byte_countin, p, ctime_now, flags );	  // call our implementation function 
	}

	return p;
}



// ----------------------------------------------------------------------------------------------------------------------------



Statistic* StatList::AddStatsSession(
	Statistic* p,	__int64 byte_count, __int64 byte_countin,
	time_t ctime_now,
	time_t ctime_last,
	long flags,
	long param,
	long errorcode )
{
	{
		// try to detect new sessions for clients...
		// NOTE: This is only for CLIENTS, ie source addresses that access our content
		// this cannot work or is not useless in other stats
		if ( p )
		{
			long difftime(ctime_now-ctime_last);
			if( difftime < 0 ) difftime = -difftime;
			register SessionStat *sessionStat = p->sessionStat;

			newses = 0;

			// FIRST SESSION
			if ( p->visits == 0 )
			{
				p->visitTot = 30;
				p->SetVisitIn( ctime_now );
				p->visits++;
				totalVisits++;
				totalTime += 30;
				if ( doSessionStat==SESSIONSTAT_CLICKSTREAM && sessionStat )			// record end of session marker
					sessionStat->SessionBytes = 0;
				newses = 1;
			}
			// END of SESSION
			else if ( difftime > SILENT_TIME ) 
			{
				long visitDur(ctime_last - p->GetVisitIn());
				if ( visitDur < 0 )	visitDur = -visitDur;
				visitDur += 60;			// add default page looking length
				if ( p->visits == 1 )
				{
					p->visitTot = visitDur;
				}
				else
				{
					p->visitTot += visitDur;
				}
				p->SetVisitIn( ctime_now );
				p->visits++;
				totalVisits++;
				totalTime += visitDur;

				if ( doSessionStat==SESSIONSTAT_CLICKSTREAM && sessionStat )			// record end of session marker
				{
					sessionStat->RecordSessionHistory( SESS_BYTES, sessionStat->SessionBytes );		// store bytes used in session
					sessionStat->RecordSessionHistory( SESS_TIME, ctime_now );			// store date of next session, not current one

					if ( visitDur > sessionStat->SessionMaxDur )
						sessionStat->SessionMaxDur = visitDur;
					if( sessionStat->SessionLen > sessionStat->SessionMaxLen )
						sessionStat->SessionMaxLen = sessionStat->SessionLen;
				}

				newses = 2;
			}
			// EVERY TIME during the SESSION (add time details)
			else if ( p->visits == 1 )
			{
				p->visitTot += difftime;
				totalTime += difftime;
			}

			// This isnt used any where so its not needed at all...
			//temp = p->id<<16 | p->visits;		// store the client/session combined ID #

			if ( doSessionStat==SESSIONSTAT_CLICKSTREAM && sessionStat ){
				// if its at the start of a session.
				if ( newses ){
					sessionStat->SessionLen = 0;
					sessionStat->RecordSessionHistory( SESS_START_PAGE, ctime_now );		// record start of click stream

					if ( (flags & DRILLF_SESSIONADDREF) && temp2 ){
						//sessionStat->SessionLen++;
						sessionStat->RecordSessionHistory( SESS_START_REFERAL, temp2 );	// record referral into click stream
					}
				}
				if ( param ){
					sessionStat->SessionLen++;
					// Record Error code for pages/clips that have errors
					if ( errorcode > 307 )
						sessionStat->RecordSessionHistory( SESS_ERROR_CODE, errorcode );	// record error code in click stream
					sessionStat->RecordSessionHistory( param, ctime_now );	// record page into click stream
					// pagerecorded = param;
				}
				sessionStat->SessionBytes += (long)byte_count;
			}
		}
	}
	return p;
}



Statistic* StatList::IncrStatsDrill( __int64 byte_count, __int64 byte_countin, char *str, long ctime_now, long flags, long param )
{
	DEF_PRECONDITION(str);

	// find/create the Statistic
	register Statistic* p=SearchStat(str);

	if( p )
	{		
		IncrStatsMore( byte_count, byte_countin, p, ctime_now, flags );

		if ( p->timeStat )
		{
			// record detailed time history of bytes/req for each day per record
			if ( (doTimeStat&TIMESTAT_DIRECT1) )
				p->timeStat->AddTimeRec( (long)(param), (long)byte_count );
			else
			if ( (doTimeStat&TIMESTAT_DAYSHISTORY) )
				p->timeStat->AddTimeRec( (long)(ctime_now/86400), (long)byte_count );


			if ( (doTimeStat&TIMESTAT_DIRECT2) && p->timeStat->timeStat2 )			//DRILLF_STOREVALUE
				p->timeStat->timeStat2->AddTimeRec( (long)(param), (long)byte_count );
		}
		// try to detect new sessions for clients...
		// NOTE: This is only for CLIENTS, ie source addresses that access our content
		// this cannot work or is not useless in other stats
		//if ( (flags & DRILLF_SESSIONSTAT) )
		//		AddStatsSession( p , byte_count, byte_countin, ctime_now, flags, param, 0 );
	}
	return p;
}



Statistic* StatList::IncrStatsClickstream( __int64 byte_count, __int64 byte_countin, char *str, long ctime_now, long flags, long param, long errorcode )
{
	// find/create the Statistic
	register Statistic* p=SearchStat(str);

	if( p )
	{	
		time_t	ctime_lasthit (p->lastday);

		IncrStatsMore( byte_count, byte_countin, p, ctime_now, flags );

		if ( p->timeStat )
		{
			// record detailed time history of bytes/req for each day per record
			if ( (doTimeStat&TIMESTAT_DAYSHISTORY) )				//DRILLF_TIME
				p->timeStat->AddTimeRec( (long)(ctime_now/86400), (long)byte_count );

			if ( (doTimeStat&TIMESTAT_DIRECT2) && p->timeStat->timeStat2 )			//DRILLF_STOREVALUE
				p->timeStat->timeStat2->AddTimeRec( (long)(param), (long)byte_count );
		}

		// try to detect new sessions for clients...
		// NOTE: This is only for CLIENTS, ie source addresses that access our content
		// this cannot work or is not useless in other stats
		if ( (flags & DRILLF_SESSIONSTAT) )
			AddStatsSession( p , byte_count, byte_countin, ctime_now, ctime_lasthit, flags, param, errorcode );
	}
	return p;
}


long StatList::AddStatsToStat( Statistic *tostat, Statistic *stat1, long inctotals )
{
	register Statistic	*p;

	if ( tostat ){			// if its a sane index # its ok
		p = tostat;

		if ( inctotals ){
			totalRequests += stat1->files;
			totalVisits += stat1->visits;
			totalBytes += stat1->bytes;
			totalBytesIn += stat1->bytesIn;
			totalCounters += stat1->counter;
			totalCounters4 += stat1->counter4;
			totalErrors += stat1->errors;
			totalTime += stat1->visitTot;
		}
		p->files += stat1->files;
		p->bytes += stat1->bytes;
		p->bytesIn += stat1->bytesIn;
		p->visits += stat1->visits;
		p->counter += stat1->counter;
		p->counter2 = stat1->counter2;
		p->counter3 = stat1->counter3;
		p->counter4 += stat1->counter4;
		p->errors += stat1->errors;
		p->SetVisitIn( stat1->GetVisitIn() );
		p->lastday = stat1->lastday;
		p->visitTot += stat1->visitTot;
		p->timeStat = stat1->timeStat;
		p->sessionStat = stat1->sessionStat;
	}
	return 1;
}

Statistic	* StatList::IncrStatsFromStat( Statistic *stat1 )
{
	register Statistic	*p;

	p = SearchStat( stat1->GetName() );
	if ( p ){			// if its a sane index # its ok
		AddStatsToStat( p, stat1, TRUE );
	}
	return p;
}

void StatList::ClearTimeHistoryPointers( void )
{

	if ( this ){
		long	lp;
		Statistic *p;

		for( lp=0; lp<num; lp++ ){
			p = GetStat( lp );
			p->timeStat = 0;
			p->sessionStat = 0;
			//p++;
		}
		doTimeStat = 0;
		doSessionStat = 0;
	}
}


Statistic	* StatList::AddStatsFromStat( char *name, Statistic *stat1 )
{
	register Statistic	*p;

	p = SearchStat( name );
	if ( p ){			// if its a sane index # its ok
		AddStatsToStat( p, stat1, FALSE );
		p->timeStat = 0;//stat1->timeStat;
		p->sessionStat = 0;//stat1->sessionStat;
	}
	return p;
}

void StatList::IncrCounter4( void )
{
	if ( current ){
		current->counter4++;
		totalCounters4++;
	}
}

long StatList::GetVisitIn( long n )
{
	if( current = GetStat(n) )
		return current->GetVisitIn();
	else return 0;
}

long StatList::GetCounter( long n )
{
	if( current = GetStat(n) )
		return current->counter;
	else return 0;
}

short StatList::GetCounter2( long n )
{
	if( current = GetStat(n) )
		return current->counter2;
	else return 0;
}

short StatList::GetCounter3( long n )
{
	if( current = GetStat(n) )
		return current->counter3;
	else return 0;
}

long StatList::GetCounter4( long n )
{
	if( current = GetStat(n) )
		return current->counter4;
	else return 0;
}


long StatList::CurrentGetCounter( void )
{
	if ( current )
		return current->counter;
	else
		return 0;
}
void StatList::CurrentSetCounter( long x )
{
	if ( current )
		current->counter = x;
}
void StatList::CurrentSetCounter4( long x )
{
	if ( current )
		current->counter4 = x;
}

void StatList::CurrentIncCounter( void )
{
	if ( current ){
		current->counter++;
		totalCounters++;
	}
}


void StatList::SetCounter( long n, long x )
{
	if( current = GetStat(n) )
		current->counter = x;
}

void StatList::SetCounter2( long n, unsigned char x )
{
	if( current = GetStat(n) )
		current->counter2 = x;
}

void StatList::SetCounter3( long n, unsigned char x )
{
	if( current = GetStat(n) )
		current->counter3 = x;
}


void StatList::SetCounter4( long n, long x )
{
	if( current = GetStat(n) )
		current->counter4 = x;
}




long StatList::IncrVisits( long n )
{
	if( current = GetStat(n) ){
		current->visits++;
		totalVisits++;
	}
	return totalVisits;
}


long StatList::IncrVisitors( long n )
{
	if( current = GetStat(n) ){
		current->counter++;
		totalCounters++;
	}
	return totalCounters;
}

//visitin variable for pages is used as a exitCounter
long StatList::IncrPageExitCount( long n )
{
	if( current = GetStat(n) )
		current->SetVisitIn( current->GetVisitIn()+1 );
	return n;
}

long StatList::CurrentIncrVisits( void )
{
	if ( current ){
		current->visits++;
		totalVisits++;
	}
	return totalVisits;
}


long StatList::CurrentIncrVisitors( void )
{
	if ( current ){
		current->counter++;
		totalCounters++;
	}
	return totalCounters;
}


long StatList::AddToVisits( char *name )
{
	register Statistic	*p;

	p = FindStatbyName( name );
	if ( p ){			// if its a sane index # its ok
		p->visits++;
		totalVisits++;
	}
	return totalVisits;
}


long StatList::AddToVisitors( char *name  )
{
	register Statistic	*p;

	p = FindStatbyName( name );
	if ( p ){			// if its a sane index # its ok
		p->counter++;
		totalCounters++;
	}
	return totalCounters;
}

// -------------------------------------------------------------------

unsigned long StatList::GetVisits( int n )
{
	if( current = GetStat(n) )
		return current->GetVisits();
	else return 0;
}


void StatList::SetVisits( int n, long x )
{
	if( current = GetStat(n) )
		current->SetVisits( x );
}

void StatList::AddVisits( int n, long x )
{
	if( current = GetStat(n) ){
		current->AddVisits( x );
		totalVisits += x;
	}
}


void StatList::IncCounter( int n )
{
	if( current = GetStat(n) ){
		current->counter++;
		totalCounters++;
	}
}

void StatList::AddCounters( int n, long x )
{
	if( current = GetStat(n) ){
		current->AddCounters( x );
		totalCounters += x;
	}
}

void StatList::AddCounters4( Statistic *p, long x )
{
	if( p ){
		p->AddCounters4( x );
		totalCounters4 += x;
	}
}

void StatList::AddCounters4To( int n, long x )
{
	if( current = GetStat(n) ){
		current->AddCounters4( x );
		totalCounters4 += x;
	}
}


void StatList::AddBytes( int n, __int64 x )
{
	if( current = GetStat(n) ){
		current->AddBytes( x );
		totalBytes += x;
	}
}

void StatList::AddBytesIn( int n, __int64 x )
{
	if( current = GetStat(n) ){
		current->AddBytesIn( x );
		totalBytesIn += x;
	}
}


void StatList::AddFiles( Statistic *p, long x )
{
	if( p ){
		p->AddFiles( x );
		totalRequests += x;
	}
}

void StatList::AddTimeDur( int n, long x )
{
	if( current = GetStat(n) ){
		current->AddTimeDur( x );
		totalTime += x;
	}
}

/* return requests */
unsigned long StatList::GetFiles( int n )
{
	if( current = GetStat(n) )
		return current->GetFiles();
	else
		return 0;
}

/* return bytes */
__int64 StatList::GetBytes( int n )
{
	if( current = GetStat(n) )
		return current->GetBytes();
	else
		return 0;
}

/* return bytes */

__int64 StatList::GetBytesIn( int n )
{
	if( current = GetStat(n) )
		return current->GetBytesIn();
	else
		return 0;
}



/* return # packets */
unsigned long StatList::GetPacketsRec( int n )
{
	if( current = GetStat(n) )
		return current->GetPacketsRec();
	else
		return 0;
}

/* return # packets */

unsigned long StatList::GetPacketsSent( int n )
{
	if( current = GetStat(n) )
		return current->GetPacketsSent();
	else
		return 0;
}


// durection the item was being accessed for (estimated)
long StatList::GetDur( int n )
{
	if( current = GetStat(n) )
		return current->GetDur();
	else
		return 0;
}


/* return errors */
unsigned long StatList::GetErrors( int n )
{
	if( current = GetStat(n) )
		return current->GetErrors();	// return count
	else
		return 0;
}

/* return name */
char * StatList::GetName( int n )
{
	if ( GetNameTrans )
		return GetNameTrans( n );

	if( current = GetStat(n) )
		return current->GetName();	// return name
	else
		return 0;
}

/* GetDay - Get the last accessed date from this record */
long StatList::GetDay( int n )
{
	if( current = GetStat(n) )
		return (long)(current->lastday);
	else
		return 0;
}

/* GetDay - Get the last accessed date from this record */
long StatList::GetLength( int n )
{
	if( current = GetStat(n) )
		return (current->length);
	else
		return 0;
}

unsigned long StatList::CurrentGetHash( void )
{
	if( current )
		return (current->hash);
	else
		return 0;
}
unsigned long StatList::GetHash( int n )
{
	if( current = GetStat(n) )
		return (current->hash);
	else
		return 0;
}


// ------------------------------------------------------------------------

long StatList::SortTimeStat( int n, int type )
{
	if( (doTimeStat&TIMESTAT_DAYSHISTORY) && (current = GetStat(n)) )
		return current->timeStat->Sort(type);
	else
		return 0;
}


long StatList::GetDaysAt( int n, int index )
{
	if( (doTimeStat&TIMESTAT_DAYSHISTORY) && (current = GetStat(n)) )
		return current->timeStat->GetDaysIdx(index);
	else
		return 0;
}

long StatList::GetFilesAt( int n, int index )
{
	if( (doTimeStat&TIMESTAT_DAYSHISTORY) && (current = GetStat(n)) )
		return current->timeStat->GetFilesIdx(index);
	else
		return 0;
}

long StatList::GetBytesAt( int n, int index )
{
	if( (doTimeStat&TIMESTAT_DAYSHISTORY) && (current = GetStat(n)) )
		return current->timeStat->GetBytesIdx(index);
	else
		return 0;
}

long StatList::GetFilesHistory( int n, int day )
{
	if( (doTimeStat&TIMESTAT_DAYSHISTORY) && (current = GetStat(n)) )
		return current->timeStat->GetFiles(day);	// return count
	else
		return 0;
}

/* return bytes */
long StatList::GetBytesHistory( int n, int day )
{
	if( (doTimeStat&TIMESTAT_DAYSHISTORY) && (current = GetStat(n)) )
		return current->timeStat->GetBytes(day);	// return count
	else
		return 0;
}


// ------------------ TIME STAT 2


// Get history data from the 2nd timestat inside the sessionStat, used only rarely
long StatList::SortTimeStat2( int n, int type )
{
	if( (doTimeStat&TIMESTAT_DIRECT2) && (current = GetStat(n)) )
		return current->timeStat->timeStat2->Sort(type);
	else
		return 0;
}
long StatList::GetDaysAt2( int n, int index )
{
	if( (doTimeStat&TIMESTAT_DIRECT2) && (current = GetStat(n)) )
		return current->timeStat->timeStat2->GetDaysIdx(index);
	else
		return 0;
}
long StatList::GetFilesAt2( int n, int index )
{
	if( (doTimeStat&TIMESTAT_DIRECT2) && (current = GetStat(n)) )
		return current->timeStat->timeStat2->GetFilesIdx(index);
	else
		return 0;
}long StatList::GetBytesAt2( int n, int index )
{
	if( (doTimeStat&TIMESTAT_DIRECT2) && (current = GetStat(n)) )
		return current->timeStat->timeStat2->GetBytesIdx(index);
	else
		return 0;
}
long StatList::GetFilesHistory2( int n, int day )
{
	if( (doTimeStat&TIMESTAT_DIRECT2) && (current = GetStat(n)) )
		return current->timeStat->timeStat2->GetFiles(day);	// return count
	else
		return 0;
}
long StatList::GetBytesHistory2( int n, int day )
{
	if( (doTimeStat&TIMESTAT_DIRECT2) && (current = GetStat(n)) )
		return current->timeStat->timeStat2->GetBytes(day);	// return count
	else
		return 0;
}
long StatList::GetHistoryNum2( int n )
{
	if( (doTimeStat&TIMESTAT_DIRECT2) && (current = GetStat(n)) )
		return current->timeStat->timeStat2->GetNum();	// return count
	else
		return 0;
}


long StatList::GetHistoryNum( int n )
{
	if( (doTimeStat&TIMESTAT_DAYSHISTORY) && (current = GetStat(n)) )
		return current->timeStat->GetNum();	// return count
	else
		return 0;
}



// ------------------------------------------------------------------------



/* Substitute - substitute names in list */
void StatList::Substitute(char *(*func)(const char *))
{
	register Statistic *p;
	char *str;
	
	for (long n=0; n<num; n++) {
		p = GetStat( n );
		str = func(p->GetName());		// get new string
		if (str) 
			p->ChangeName( str );
	}
}

void StatList::MarkForRebuild( void )
{
	DEF_ASSERT(indexSize >= 0);

	// Mark the table to indicate that the headers need rebuilding.
	headPtr[indexSize+1] = (Statistic*)1;
}

void StatList::ReBuildHeaders( void )
{
	register	Statistic	*p, *h;
	long 		n, hash, len, headIndex;

	// ****************************************************************
	// This is a guard against rebuilding the headers multiple times.
	// ****************************************************************
	if (!headPtr[indexSize+1])
		return;


	// Add 1 for the ReBuildHeaders flag.
	// Add 1 for the fact that indexSize was decremented at initialization time.
	MemClr( headPtr, (indexSize+1+1)*sizeof(void*) );


	for (n=0; n<num; n++) {
		p = GetStat( n );
		hash = p->hash;
		len = p->length;

		headIndex = hash & (indexSize);

		DEF_ASSERT(headIndex >= 0);
		DEF_ASSERT(headIndex <= indexSize);

		// from head, find the tail...
		h = headPtr[ headIndex ];

		p->next	= h; 	//add ptr to next entry in list

		// -----------
		if ( h ){
			p->nextHash	= h->hash;
		} else {
			p->nextHash = 0;			//tailPtr[headIndex] = p;	
		}

		headPtr[headIndex]		= p;			//this is the new index
		headHashRef[headIndex]	= hash;
	}
}


/*
 * Rebuild .next and Header pointers into currect values
 */
void StatList::ReBuildPtrs( void )
{
	ReBuildHeaders();
	return;
}


void StatList::StatSort( void *base, long members, size_t datasize, CompFunc cmp, long topn, short sortOrder )
{
	long outer, inner, highest;
	char *s1, *s2;
	long result=0;

	if ( topn > members )
		topn = members;

	// loop X from N to N+TOPN
	for( outer = 0; outer<topn; outer++ ){
		highest = outer;
		// loop Y from X+1 to X+END
		for( inner=outer+1; inner<members; inner++ ){
			s1 = (char*)base+(datasize*highest);
			s2 = (char*)base+(datasize*inner);
			cmp( s1, s2, &result);
			if ( sortOrder ){ // TOP_SORT
				if ( result >= 0 )
					highest = inner;
			} else { // BOTTOM_SORT
				if ( result < 0 ) 
					highest = inner;
			}

		}
		if ( sortOrder ) // TOP_SORT
		{
			s1 = (char*)base+(datasize*outer);
			s2 = (char*)base+(datasize*highest);
		} else { // BOTTOM_SORT
			s2 = (char*)base+(datasize*outer);
			s1 = (char*)base+(datasize*highest);
		}

		MemSwap( s1, s2, datasize );
	}

}


/* Sort - sort the list alphabetically */
void StatList::Sort(CompFunc func, long list, short sortOrder )
{
	if( num )
	{
		// if all items are to be sorted...
		if( list<=0 )
		{
			// ...fix up param
			list=num;
		}

		// if a relatively small number of items are to be sorted then use
		// our good old fashioned, but stable, sorting mechanism
		if( list<500 )
		{
			StatSort( stat, num, sizeof(Statistic), func, list, sortOrder );
		}
		// else a relatively large number of items are to be sorted so use 
		// our speedy, but unstable (random results for identical values), sorting mechanism
		else
		{
			FastQSort( stat, num, sizeof(Statistic), func, sortOrder  );
		}

		// the sort currupts all the pointers so they must be recomputed only if in daemon sleep mode
		if ( (MyPrefStruct.live_sleeptype > 0) )
		{
			MarkForRebuild();
		}
	}
}


/* Sort - sort the list alphabetically */
void StatList::DoSort( long type, long list, short sortOrder )
{
	switch ( type ){
		case SORT_NAMES:	Sort( CompStat, list, sortOrder ); break;
		case SORT_BYTES:	Sort( CompStatBytes, list, sortOrder ); break;
		case SORT_REQUESTS:	Sort( CompStatFiles, list, sortOrder ); break;
		case SORT_PAGES:	Sort( CompStatCounter4, list, sortOrder ); break;
		case SORT_NUMBER:	Sort( CompStatNum, list, sortOrder ); break;
		case SORT_BYTESIN:	Sort( CompStatBytesIn, list, sortOrder ); break;
		case SORT_COUNTER:	Sort( CompStatCounter, list, sortOrder ); break;
		case SORT_COUNTER2:	Sort( CompStatCounter2, list, sortOrder ); break;
		case SORT_COUNTER3:	Sort( CompStatCounter3, list, sortOrder ); break;
		case SORT_COUNTER4:	Sort( CompStatCounter4, list, sortOrder ); break;
		case SORT_VISITIN:	Sort( CompStatVisitIn, list, sortOrder ); break;
		case SORT_ERRORS:	Sort( CompStatErrors, list, sortOrder ); break;
		case SORT_VISITS:	Sort( CompStatVisits, list, sortOrder ); break;
		case SORT_DATE:		Sort( CompStatDate, list, sortOrder ); break;
		case SORT_LASTDAY:	Sort( CompStatLastDay, list, sortOrder ); break;
		case SORT_COUNTER4THENERRORS:	Sort( CompStatCounter4ThenErrors, list, sortOrder ); break;
		default: DEF_ASSERT( false ); break;
	}

	// Mark the table to indicate that the headers need rebuilding.
	headPtr[indexSize+1] = (Statistic*)1;
}

/* return total */
long StatList::GetStatListMaxNum( void )
{
	if ( this )
		return maxNum;
	else
		return 0;
}

long StatList::GetStatListNum( void )
{
	if ( this )
		return num;
	else
		return 0;
}

/* return total */

__int64 StatList::GetStatListTotalBytes( void )
{
	if ( this )
		return totalBytes;
	else
		return 0;

}



/* return total */

__int64 StatList::GetStatListTotalBytesIn( void )
{
	if ( this )
		return totalBytesIn;
	else
		return 0;
}



/* return total */
unsigned long StatList::GetStatListTotalRequests( void )
{
	if ( this )
		return totalRequests;
	else
		return 0;
}

/* return total */
unsigned long StatList::GetStatListTotalErrors( void )
{
	if ( this )
		return totalErrors;
	else
		return 0;
}
/* return total */

unsigned long StatList::GetStatListTotalCounters( void )
{
	if ( this )
		return totalCounters;
	else
		return 0;
}

unsigned long StatList::GetStatListTotalCounters4( void )
{
	if ( this )
		return totalCounters4;
	else
		return 0;
}


//TIMESTAT   (add this func)
unsigned long StatList::GetStatListTotalVisits( void )
{
	if ( this )
		return totalVisits;
	else
		return 0;
}

long StatList::GetStatListTotalTime( void )
{
	if ( this )
		return totalTime;
	else
		return 0;
}


char *StatList::GetStatDetails( long hashid, Statistic** ppStat, long *bytes )
{
	char *url;
	Statistic*	pStat;

	if ( hashid && bytes && this )	// like all the previous Get's, make sure 'this' is valid
	{
		pStat = this->FindHashStat(hashid);
		*bytes = 0;
		if ( pStat )
		{
			url = pStat->GetName();
			*bytes = static_cast<long>( pStat->GetBytes() );

			DEF_ASSERT(hashid == HashStr(url, NULL));

			if (ppStat)
				*ppStat = pStat;

			return url;
		}
	}
	return NULL;
}

/*-----------------------------------------------------------------------------------------
** StatList class methods
*/
/* StatList - constructor */

StatList::StatList( void *VDptr, long iSize ) : num(0)
{
	long	ptrSize = sizeof(void *),
			headAlloc;

	MemClr( this, sizeof(StatList) );

	vd = (VDinfoP)VDptr;

	// If the size is valid, lets make it, else leave it all blank and setup as a mirror to another ->stat
	if ( iSize>0 )
	{
		incrSize = maxNum = iSize;
		if ( iSize<8 ){
			incrSize = iSize*10;
		}
		#ifdef DEF_FREEVERSION
			indexSize = STATLIST_SMALLINDEXSIZE;
			iSize = iSize & 0xffff;		// client limit statlists to no more than 65536 at most
		#else
			if ( !MyPrefStruct.multivhosts && iSize>=500 )
				indexSize = STATLIST_MEGAINDEXSIZE;
			else
			if ( !MyPrefStruct.multivhosts && iSize>100 )
				indexSize = STATLIST_HUGEINDEXSIZE;
			else
			if ( MyPrefStruct.multivhosts && ((((VDinfoP)VDptr)->domainNum < 5) || (iSize>100)) )
				indexSize = STATLIST_BIGINDEXSIZE;
			else
				indexSize = STATLIST_SMALLINDEXSIZE;
		#endif

		if( !(stat = new Statistic[iSize]) ){
			MemoryError( "Failed alloc stat", iSize*sizeof( Statistic ) );
			return;
		}
		MemClr( stat, iSize*sizeof( Statistic ) );

		headAlloc = indexSize * ptrSize;

		// We are adding an additional 1 pointer to use as a flag for indicating
		// if the list needs to be rebuilt (see ReBuildHeaders).
		if( !(headPtr = new Statistic*[indexSize+1]) ){
			MemoryError( "Failed alloc headPtr", headAlloc );
			return;
		}
		MemClr( headPtr, (indexSize+1)*sizeof(Statistic*) );

		// Mark the headers to be rebuilt, as initially they will not be!
		headPtr[indexSize] = (Statistic*)1;

		if( !(headHashRef = new unsigned long[indexSize]) ){
			MemoryError( "Failed alloc headHashRef", headAlloc );
			return;
		}

		if ( (MyPrefStruct.live_sleeptype>0) ){
			MemClr( headHashRef, headAlloc );
		}

		indexSize--;		// make sure the value is of 0xffff style, so a 65536 is 0xffff
	}
}

//


/* ~StatList - destructor */
StatList::~StatList()
{
#ifdef DEF_DEBUG
	OutDebugs( "%d: Clearing statlist %d units", GetCurrentCTime(), num );
#endif

	if ( headHashRef )
		delete [] headHashRef;

	if ( headPtr )
		delete [] headPtr;

	if ( doSessionStat == SESSIONSTAT_CLIPDATA) {
		for(int i=0;i<num;i++){
			delete stat[i].clipData;
			stat[i].clipData = NULL;
		}
	}

	if ( stat && (useOtherNames != NAMEIS_FILESTAT) )
		delete [] stat;
	
	headHashRef = 0;
	headPtr = 0;
	stat = 0;
}



Statistic	*StatList::GrowStatList( long headIndex )
{
	Statistic	*tempStat;
	long newsize;

	if ( maxNum < 5000 )
		newsize = maxNum+incrSize;
	else
		newsize = maxNum + (maxNum>>3);

	tempStat = new Statistic[ newsize ];

#ifdef DEF_DEBUG
	OutDebugs( "*** resizing stat from %d to %d units (%ld Kb)", maxNum, newsize, (newsize*sizeof(Statistic))/1024 );
#endif

	if (tempStat)
	{
		long n;
		// Copy the old memory used to the new
		memcpy( tempStat, stat, (maxNum)*sizeof( Statistic ) );
		// Clear the extra memory added
		MemClr( &tempStat[maxNum], (newsize-maxNum)*sizeof( Statistic ) );

		register Statistic	*p, *h;
		// Clone the 'stat'
		for (n=0; n<maxNum; n++)
		{
			p = &stat[n];
			p->timeStat = NULL;
			p->sessionStat = NULL;
			// make sure memory is not deallocated for name
			p->name = NULL;
			p->length = 0;		
			if ( (p = p->next) ) {		// get next Ptr
				tempStat[n].next = &tempStat[p->id] ;
			} else
				tempStat[n].next = 0;
		}
		//rebuild headPtr
		for (n=0; n<=indexSize; n++)
		{
			if ( p = headPtr[n] ){		// get Ptr
				headPtr[n] = &tempStat[p->id];
			}
		}

		DEF_ASSERT(headIndex >= 0);
		DEF_ASSERT(headIndex <= indexSize);
		h = headPtr[headIndex];

		delete[] stat;
		stat = tempStat;
		maxNum = newsize;
		tableTot++;
		return h;
	} else {
		//ErrorCode( str, incrSize );
		MemoryError( "StatList Failed to Add", newsize );
	}
	return NULL;
}

Statistic	*StatList::GrowSegStatList( long headIndex )
{
	Statistic	*tempStat;
	register Statistic	*p, *h;
	long n;

	tempStat = new Statistic[ maxNum+incrSize ];

	if (tempStat) {
		memcpy( tempStat, stat, (maxNum)*sizeof( Statistic ) );
		MemClr( &tempStat[maxNum], (incrSize)*sizeof( Statistic ) );
		//clone the 'stat'
		for (n=0; n<maxNum; n++) {
			p = &stat[n];
			p->timeStat = NULL;
			p->sessionStat = NULL;
			// make sure memory is not deallocated for name
			p->name = NULL;
			p->length = 0;		
			if ( (p = p->next) ) {		// get next Ptr
				tempStat[n].next = &tempStat[p->id] ;
			} else
				tempStat[n].next = 0;
		}
		//rebuild headPtr
		for (n=0; n<=indexSize; n++) {
			if ( (p = headPtr[n]) ){		// get Ptr
				headPtr[n] = &tempStat[p->id];
			}
		}
		DEF_ASSERT(headIndex >= 0);
		DEF_ASSERT(headIndex <= indexSize);
		h = headPtr[headIndex];

		delete [] stat;
		stat = tempStat;
		maxNum += incrSize;
		tableTot++;
		return h;
	}
	else {
		//ErrorCode( str, incrSize );
		MemoryError( "StatList Failed to Add in GrowStatList()", maxNum + incrSize );
	}
	return NULL;
}

