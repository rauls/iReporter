
#include "Compiler.h"
#include "EngineAddStats.h"

#include <ctype.h>

#include "StatDefs.h"			// for SESS_START_PAGE etc
#include "EngineParse.h"		// for ReverseAddress etc
#include "EngineBuff.h"
#include "EngineRestoreStat.h"	// for extern of 'regionStrings'
#include "EngineRegion.h"		// for LookupDomain
#include "report.h"				// for char *GetTimeonStringsTrans( short lp );
#include "StatList.h"			// for ClearStat()
#include "Stats.h"
#include "EngineVirtualDomain.h"
#include "engine_drill.h"
#include "Translate.h"

// for PC & UNIX.
extern "C" void ProgressChangeMainWindowTitle( long percent, char *etaStr );

char ErrVhostStr[]=			"Unknown Host";

static char UnresolvedIPStr[]=	"[UNRESOLVED IP]";
static char AuthUsersStr[]=		"[AUTH USERS]";
static char OthersStr[]=		"[OTHERS]";



void InitOtherStatlists( VDinfoP VDptr )
{
	if ( VDptr->byClient && VDptr->byUser == NULL ){
		if ( MyPrefStruct.filterdata.orgTot>0 && VDptr->byOrgs )
			VDptr->byOrgs = ClearStat( VDptr, VDptr->byOrgs, kSecondDomainIncr );
		if ( 1 ){
			if ( VDptr->byUser )
				VDptr->byUser->ClearTimeHistoryPointers(  );
			if ( VDptr->byUser = ClearStat( VDptr, VDptr->byUser, kSubDomainIncr ) )
				VDptr->byUser->useOtherNames = NAMEIS_COPY;
			if ( STAT(MyPrefStruct.stat_circulation) )
				VDptr->byCirculation = ClearStat( VDptr, VDptr->byCirculation, 10 );
			if ( STAT(MyPrefStruct.stat_loyalty) )
				VDptr->byLoyalty = ClearStat( VDptr, VDptr->byLoyalty, 10 );
			if ( STAT(MyPrefStruct.stat_timeon) )
				VDptr->byTimeon = ClearStat( VDptr, VDptr->byTimeon, 10 );
		}
		if ( STAT(MyPrefStruct.stat_country) )
			VDptr->byDomain = ClearStat( VDptr, VDptr->byDomain, kDomainIncr );
		if ( STAT(MyPrefStruct.stat_regions) )
			VDptr->byRegions = ClearStat( VDptr, VDptr->byRegions, 8 );
		if ( STAT(MyPrefStruct.stat_seconddomain) )
			VDptr->bySecondDomain = ClearStat( VDptr, VDptr->bySecondDomain, kSecondDomainIncr );
	}
}


// ---------------------------------- Post calculation stuff
// based on each client having a tag that records what Browser and what Operation system
// that client is... sift through all browsers of each client and work out how many
// clients there are for each browsers / operationsystem
//
void CalcAgentVisitors( VDinfoP	VDptr, Statistic *client )
{
	long	bnum=0,onum=0;

	if ( VDptr->byBrowser ){
		bnum = VDptr->byBrowser->GetStatListNum();
		if ( bnum ){
			VDptr->byBrowser->IncrVisitors( client->counter2 );
		}
	}
	if ( VDptr->byOperSys ){
		onum = VDptr->byOperSys->GetStatListNum();
		if ( onum ){
			VDptr->byOperSys->IncrVisitors( client->counter3 );
		}
	}
}


// Increment visitor counts for the hourly traffic report time slots that
// contain <sessionDate>.
void AddVisitorHourlyInfo( VDinfoP VDptr, long sessionDate )
{
	struct tm logDate;
	DaysDateToStruct( sessionDate, &logDate );
		
	if( logDate.tm_wday>=0 && logDate.tm_wday<=6 )
	{
		if( VDptr->byWeekdays[logDate.tm_wday] )
		{
			VDptr->byWeekdays[logDate.tm_wday]->AddToVisitors( hourStrings[logDate.tm_hour] );
		}

		if( VDptr->byHour )
		{
			VDptr->byHour->AddToVisitors( hourStrings[logDate.tm_hour] );
		}
	}
}


// Increment session counts for the hourly traffic report time slots that
// contain <sessionDate>.
void AddSessionHourlyInfo( VDinfoP VDptr, long sessionDate )
{
	struct tm logDate;
	DaysDateToStruct( sessionDate, &logDate );
	
	if( logDate.tm_wday>=0 && logDate.tm_wday<=6 )
	{
		if( VDptr->byWeekdays[logDate.tm_wday] )
		{
			VDptr->byWeekdays[logDate.tm_wday]->AddToVisits( hourStrings[logDate.tm_hour] ); // i.e. sessions
		}

		if( VDptr->byHour )
		{
			VDptr->byHour->AddToVisits( hourStrings[logDate.tm_hour] ); // i.e. sessions
		}
	}
}


// Increment visitor counts for the daily traffic report time slots that
// contain <sessionDate>.
void AddVisitorDailyInfo( VDinfoP VDptr, long sessionDate )
{
	struct tm logDate;
	DaysDateToStruct( sessionDate, &logDate );
	
	if( VDptr->byWeekday && logDate.tm_wday>=0 && logDate.tm_wday<=6  )
	{
		VDptr->byWeekday->AddToVisitors( weekdays[logDate.tm_wday] );
	}

	char timebuff[32];
	StructToUSDate( &logDate, timebuff );

	if( VDptr->byDate )
	{
		VDptr->byDate->AddToVisitors( timebuff );
	}

	if( VDptr->byMonth )
	{
		mystrcpy( timebuff+3, timebuff+6 );
		VDptr->byMonth->AddToVisitors( timebuff );
	}
}


// Increment session counts for the daily traffic report time slots that
// contain <sessionDate>.
void AddSessionDailyInfo( VDinfoP VDptr, long sessionDate )
{
	struct tm logDate;
	DaysDateToStruct( sessionDate, &logDate );
	
	if( VDptr->byWeekday && logDate.tm_wday>=0 && logDate.tm_wday<=6  )
	{
		VDptr->byWeekday->AddToVisits( weekdays[logDate.tm_wday] ); // i.e. sessions
	}

	char timebuff[32];
	StructToUSDate( &logDate, timebuff );

	if( VDptr->byDate )
	{
		VDptr->byDate->AddToVisits( timebuff ); // i.e. sessions
	}

	if( VDptr->byMonth )
	{
		mystrcpy( timebuff+3, timebuff+6 );
		VDptr->byMonth->AddToVisits( timebuff ); // i.e. sessions
	}
}





long AddStatSessionDetail( StatList *list, Statistic *p, long dur, long clientnum, long sessions )
{
	if ( p ){
		long SessionID, lastSessionID = -1;

		SessionID = clientnum<<16 | sessions;
		lastSessionID = p->lastday;

		if ( dur <0 ) dur = 30;
		// add dur to its time total
		p->visitTot += dur;
		list->totalTime += dur;

		// SESSIONS++
		if ( lastSessionID != SessionID ){			// if page is in a new session
			p->visits++;
			list->totalVisits++;
		}
		// VISITORS++
		if ( (lastSessionID>>16) != (SessionID>>16) ){	// if page is in a new users
			p->counter++;
			list->totalCounters++;
		}
		p->lastday = SessionID;
		// if page->lastsessionid != sessionID, add 1 to session count

		return p->visitTot;			// return total viewing time for item.
	}
	return 0;
}


// add pages session counter +1 when its in a new session
// add duration of page visit to the page detail too
// add unique visitors to pages counter
long AddPageSessionDetail( VDinfoP	VDptr, long pageHashID, long dur, long clientnum, long sessions )
{
	Statistic	*p;

	if ( VDptr )
	{
		// add to byFile
		if( VDptr->byFile && (p = VDptr->byFile->FindHashStat( pageHashID )) )
			AddStatSessionDetail( VDptr->byFile, p, dur, clientnum, sessions );

		// find page from hashid
		if( VDptr->byPages && (p = VDptr->byPages->FindHashStat( pageHashID )) )
			AddStatSessionDetail( VDptr->byPages, p, dur, clientnum, sessions );
		else
		if( VDptr->byDownload && (p = VDptr->byDownload->FindHashStat( pageHashID )) )
			AddStatSessionDetail( VDptr->byDownload, p, dur, clientnum, sessions );
	}
	if ( p )
		return p->id;
	else
		return -1;
}


void ShowSessionStatus( long pos, long tot )
{
	char msg[256];

	if ( pos%1000 == 0 ){
		sprintf( msg, "Processing session %d/%d ..." , pos, tot );
#ifndef DEF_MAC
		ProgressChangeMainWindowTitle( pos*100/tot, msg );
		//StatusWindowSetProgress( pos*1000/tot, msg );
#endif
	}
}


// add session counter to various databases, calculated from client->sessionslist
long AddSessionRecords( VDinfoP	VDptr, Statistic *p, long clientnum )
{
	if ( !p->sessionStat )
		 return 0;

	SessionStat* sessionStat=p->sessionStat;
	long sessionNum=1, pageid=0, curr_hashid=0;
	long diff=0, meandiff=0;
	long prev_hashid(SESS_START_PAGE);
	long prevTime(p->lastday);
	long sessionStartTime(VDptr->firstTime);
	long lastSessionNumAddedToHourly(-1);
	long lastSessionHourAdded(-1);
	long lastSessionNumAddedToDaily(-1);
	long lastSessionDayAdded(-1);

	long numSessionEntries( sessionStat->GetNum() );
	if( numSessionEntries )	
	{
		for( long i(0); i<numSessionEntries; i++ )		
		{
			curr_hashid=sessionStat->GetSessionPage( i );
			long time( sessionStat->GetSessionTime( i ) );

			 // if prev_hashid is a page
			if( prev_hashid < SESS_START_PAGE || prev_hashid > SESS_ABOVE_IS_PAGE )
			{
				long dur(0);

				// curr_hashid is a page
				if ( curr_hashid < SESS_START_PAGE || curr_hashid > SESS_ABOVE_IS_PAGE )
					dur = time - prevTime;
				else
					dur = 30; // Default to 30 seconds

				// The previous session entry was a page... so add it's details
				if ( numSessionEntries>500 )
					ShowSessionStatus( i, numSessionEntries );
				AddPageSessionDetail( VDptr, prev_hashid, dur, clientnum, sessionNum );
			}
			// ------------------ start of session ------------------
			if( curr_hashid == SESS_START_PAGE )
			{ 
				// A new session will be starting
				diff = (time-sessionStartTime);
				if ( meandiff )
					meandiff = (meandiff + diff) / 2;
				else
					meandiff = diff;
				sessionStartTime = time;
			}
			else if( curr_hashid == SESS_BYTES ) // end of session , date
			{
				// The previous hashid was a page
				if ( VDptr->byPages )
				{
					long item=VDptr->byPages->FindHash( pageid );
					if ( item >= 0 ){
						VDptr->byPages->IncrPageExitCount( item ); // set Page EXIT counter
					}
				}
			}
			else if( curr_hashid == SESS_TIME )
			{
				// The previous hashid was the session bytes
				++sessionNum;
			}
			else if( curr_hashid < SESS_START_PAGE || curr_hashid > SESS_ABOVE_IS_PAGE ) // curr_hashid is a page
			{
				pageid = curr_hashid;
			}
			else
				pageid = 0;

			// if current session entry has a valid time value
			if( curr_hashid!=SESS_START_REFERAL && curr_hashid!=SESS_BYTES )
			{
				// if last session entry was a referral entry then it doesn't have a valid time
				// so use the session start time instead for comparisons
				if( prev_hashid==SESS_START_REFERAL )
				{
					prevTime=sessionStartTime;
				}

				// check if its a sane date - as raul used to do within the individual AddVisitor functions
				if( time>86400*365*10 )
				{
					long hour( time/3600 );		// hour number since start date of time_t
					long day( time/86400 );		// day number since start date of time_t

					// if current page's hour differs to that of previous page within this session
					if( i==0 || (hour - prevTime/3600>0) )
					{
						AddVisitorHourlyInfo( VDptr, time );
					}

					// if current page's day differs to that of previous page within this session
					if( i==0 || (day - prevTime/86400>0) )
					{
						AddVisitorDailyInfo( VDptr, time );
					}

					// if we've got a new session or if the hour has changed since the last time we counted a session
					if( sessionNum!=lastSessionNumAddedToHourly || hour!=lastSessionHourAdded )
					{
						AddSessionHourlyInfo( VDptr, time );

						lastSessionNumAddedToHourly=sessionNum;
						lastSessionHourAdded=hour;
					}

					// if we've got a new session or if the day has changed since the last time we counted a session
					if( sessionNum!=lastSessionNumAddedToDaily || day!=lastSessionDayAdded )
					{
						AddSessionDailyInfo( VDptr, time );

						lastSessionNumAddedToDaily=sessionNum;
						lastSessionDayAdded=day;
					}
				}

				prevTime=time; // remember previous time as we know we have a valid time here
			}

			prev_hashid=curr_hashid;
		} // for loop

		// If the current session is not end of time
		if ( curr_hashid != SESS_TIME )
		{
			AddPageSessionDetail( VDptr, curr_hashid, 30, clientnum, sessionNum );
			diff = (VDptr->lastTime - sessionStartTime);
			meandiff = (meandiff + diff) / 2;
			if ( VDptr->byPages )
			{
				long item=VDptr->byPages->FindHash( curr_hashid );
				if ( item >= 0 )
					VDptr->byPages->IncrPageExitCount( item );		// set Page EXIT counter
			}
		}
	} // if ( numSessionEntries )
	else
	{
		AddVisitorDailyInfo( VDptr, p->GetVisitIn() );
		AddVisitorHourlyInfo( VDptr, p->GetVisitIn() );
	}
	if (sessionNum == 1)
		meandiff = 0;
		
	return meandiff;
}


int IsClientaUsername( char *clientStr )
{
	char *p, c;
	int	userStatus = TRUE, dots = 0;

	p = clientStr;

	while( c=*p++ )
	{
		if ( c == '.' )		dots++;
		if ( c == '/' )		return TRUE;
		if ( c == '.' )		userStatus = FALSE;
	}
	return userStatus;
}

/*
NEW DOMAINS SPEC
----------------

New TLDs to be added to Countries report and World regions if known
other wise new entries to be added to Organizations.



  */
//
// using 2nd level domains (blah.com), add the mapped names to the list
//
//
void AddOrganizations( VDinfoP VDptr, StatList *byStat, Statistic *p )
{
	__int64  BytesOut, BytesIn;
	long	day,  dur;
	long	visits, hits, itempos, pages, errs;
	Statistic *org;
	char	*name, *pt;
	char	rnameStr[1200], *rname;

//------- Org Names
	BytesOut = p->bytes;
	BytesIn = p->bytesIn;
	day = p->lastday;
	visits = p->visits;
	hits = p->files-1;
	pages = p->counter4;
	dur = p->visitTot;
	errs = p->errors;

	name = p->GetName();
	// Only reverse a name, not an IP, since the IP is already correct.
	if ( p->length < 0 )
		rname = name;
	else
		ReverseAddress( name, rname=rnameStr );


	// -----------------------------------------
	// ORGANIZATIONS
	// -----------------------------------------
	if ( MyPrefStruct.filterdata.orgTot>0 )
	{
		StatList	*byOrgs = VDptr->byOrgs;
		long		start = 0;

		itempos = -1;
		org = NULL;
		while( start<MyPrefStruct.filterdata.orgTot ){
			pt = MultiMappingMatch( rname, MyPrefStruct.filterdata.org, &start, MyPrefStruct.filterdata.orgTot, 1 );
			if ( pt ) {
				org = byOrgs->IncrStatsDrill( 0,0, pt, day, DRILLF_COUNTER, 0 );
				if ( org ){
					org->bytes += BytesOut;
					byOrgs->totalBytes += BytesOut;
					org->bytesIn += BytesIn;
					byOrgs->totalBytesIn += BytesIn;
					org->files += hits;
					byOrgs->totalRequests += hits;
					org->visits += visits;
					byOrgs->totalVisits += visits;
					org->counter4 += pages;
					byOrgs->totalCounters4 += pages;
					org->visitTot += dur;
					byOrgs->totalTime += dur;
					org->errors += errs;
					byOrgs->totalErrors += errs;
				}
			}
		}

		if ( !org )
		{
			if ( p->length < 0 )
				rname = UnresolvedIPStr;
			else
				rname = OthersStr;
		
			org = byOrgs->IncrStatsDrill( 0,0, rname, day, DRILLF_COUNTER, 0 );
			if ( org ){
				org->bytes += BytesOut;
				byOrgs->totalBytes += BytesOut;
				org->bytesIn += BytesIn;
				byOrgs->totalBytesIn += BytesIn;
				org->files += hits;
				byOrgs->totalRequests += hits;
				org->visits += visits;
				byOrgs->totalVisits += visits;
				org->counter4 += pages;
				byOrgs->totalCounters4 += pages;
				org->visitTot += dur;
				byOrgs->totalTime += dur;
				org->errors += errs;
				byOrgs->totalErrors += errs;
			}
		}
	}

	itempos=0;






	// -----------------------------------------
	// DOMAINS Report
	// -----------------------------------------
	if ( VDptr->bySecondDomain ) 
	{
		StatList *byDom = VDptr->bySecondDomain;
		org = NULL;

		// if NAME is a real name and not an IP
		if ( p->length < 0 )
		{
			// New test code.
			// Perhaps put all these into a byAClass database.
			//strcpyuntil( buf2 , name, '.' );		// get first name, "au."
			//strcat( buf2, ".X.X.X" );
			/////////////////////////////////////////////////////////////////////////////
			org = byDom->IncrStatsDrill( 0,0, UnresolvedIPStr, day, DRILLF_COUNTER, 0 );
		} else
		if ( !mystrchr( name, '.' ) )
		{
			org = byDom->IncrStatsDrill( 0,0, AuthUsersStr, day, DRILLF_COUNTER, 0 );
		} else
		{
			unsigned long dm = 0; char c;

			c = name[0]; dm = c;
			c = name[1]; dm = (dm <<8) | c;
			c = name[2]; dm = (dm <<8) | c;
			c = name[3]; dm = (dm <<8) | c;

			switch( dm ){
				case 'com.':
				case 'org.':
				case 'net.':
				case 'edu.':
				case 'mil.':
				case 'gov.':
				case 'COM.':
				case 'ORG.':
				case 'NET.':
				case 'EDU.':
				case 'MIL.':
				case 'GOV.':
					if ( strcpyx( buf2, name, '.', 2 ) == 2 )
						org = byDom->IncrStatsDrill( 0,0, buf2, day, DRILLF_COUNTER, 0 );
					break;
				default:
					if ( strcpyx( buf2, name, '.', 3 ) == 3 )
						org = byDom->IncrStatsDrill( 0,0, buf2, day, DRILLF_COUNTER, 0 );
					else
					if ( strcpyx( buf2, name, '.', 2 ) == 2 )
						org = byDom->IncrStatsDrill( 0,0, buf2, day, DRILLF_COUNTER, 0 );
					else
					if ( strcpyx( buf2, name, '.', 1 ) == 1 )
						org = byDom->IncrStatsDrill( 0,0, buf2, day, DRILLF_COUNTER, 0 );
					break;
			}
		}


		if ( !org ) {
			org = byDom->IncrStatsDrill( 0,0, OthersStr, day, DRILLF_COUNTER, 0 );
		}

		if ( org ){
			org->bytes += BytesOut;
			byDom->totalBytes += BytesOut;
			org->bytesIn += BytesIn;
			byDom->totalBytesIn += BytesIn;
			org->files += hits;
			byDom->totalRequests += hits;
			org->visits += visits;
			byDom->totalVisits += visits;
			org->counter4 += pages;
			byDom->totalCounters4 += pages;
			org->visitTot += dur;
			byDom->totalTime += dur;
			org->errors += errs;
			byDom->totalErrors += errs;
		}
	}
}


// add clients that are users to the users-database
void AddUsers( VDinfoP VDptr, StatList *byStat, Statistic *p )
{
	char	*name;

	if( p->length >0 ){
		name = p->GetName();
		if ( !mystrchr( name, '.' ) ){		//copy stat item data
			VDptr->byUser->doTimeStat = 0;
			VDptr->byUser->doSessionStat = 0;
			VDptr->byUser->IncrStatsFromStat( p );
			VDptr->byUser->doTimeStat = byStat->doTimeStat;
			VDptr->byUser->doSessionStat = byStat->doSessionStat;
		}
	}
}

//
// using 2nd level domains (blah.com), add the mapped names to the list
//
//
void AddRegionsFromClient( VDinfoP VDptr, StatList *byStat, Statistic *src, const char *regionName, long reg )
{
	__int64	BytesOut, BytesIn;
	long	day,  dur;
	long	visits, pages, hits, errs;
	Statistic *p;

	BytesOut = src->bytes;
	BytesIn = src->bytesIn;
	day = src->lastday;
	hits = src->files - 1;
	dur = src->visitTot;
	visits = src->visits;
	pages = src->counter4;
	errs = src->errors;

	p = VDptr->byRegions->IncrStatsDrill( BytesOut,BytesIn, (char*)regionName, day,DRILLF_COUNTER, 0 );

	if ( p ){
		p->counter2 = static_cast<unsigned char>(reg);
		p->files += hits;
		p->visits += visits;
		p->counter4 += pages;
		p->visitTot += dur;
		p->errors += errs;
		VDptr->byRegions->totalRequests += hits;
		VDptr->byRegions->totalVisits += visits;
		VDptr->byRegions->totalCounters4 += pages;
		VDptr->byRegions->totalTime += dur;
		VDptr->byRegions->totalErrors += errs;
	}
}


// add clients that are users to the users-database
void AddCountryFromClients( VDinfoP VDptr, StatList *byStat, Statistic *src )
{
	__int64	BytesOut, BytesIn;
	long	day, dur;
	long	visits, pagecount, hits, reg, errs;
	char	*name;
	char	*country;

	if ( VDptr->byDomain && byStat )
	{
		Statistic *p;

		reg = -1;		// region is UNDEFINED
		name = src->GetName();
		if ( src->length < 0 )
		{
			country = UnresolvedIPStr;
		} else 
		if ( !mystrchr( name, '.' ) )
		{
			country = AuthUsersStr;
		} else
		{
			strcpyuntil( buf2 , name, '.' );		// get first name, "au."
			country = LookupCountryRegion( buf2, &reg );		// find which country it is
			if (!country)
				country = OthersStr;
		}

		BytesOut = src->bytes;
		BytesIn = src->bytesIn;
		day = src->lastday;
		visits = src->visits;
		pagecount = src->counter4;
		dur = src->visitTot;
		hits = src->files - 1;
		errs = src->errors;

		//copy stat item data
		p = VDptr->byDomain->IncrStatsDrill( BytesOut,BytesIn,country, day, DRILLF_COUNTER, 0 );
		if ( p ){
			p->counter2 = static_cast<unsigned char>(reg);
			p->files += hits;
			p->visits += visits;
			p->counter4 += pagecount;
			p->visitTot += dur;
			p->errors += errs;
			VDptr->byDomain->totalRequests += hits;
			VDptr->byDomain->totalVisits += visits;
			VDptr->byDomain->totalCounters4 += pagecount;
			VDptr->byDomain->totalTime += dur;
			VDptr->byDomain->totalErrors += errs;

			if ( VDptr->byRegions )
			{
				if ( reg == 0 )
					reg = 0;
				if ( reg == -1 )	// if region is UNDEFINED, give it a reason
					AddRegionsFromClient( VDptr, byStat, src, country, 0 );
				else	// valid region , 0...7
					AddRegionsFromClient( VDptr, byStat, src, regionStrings[reg], reg );
			}
		}
	}
	VDptr->byDomain->doTimeStat = 0;
	VDptr->byDomain->doSessionStat = 0;
}

// loyalty index
// clients repeat visits status
void AddTimeonFromClient( VDinfoP VDptr, StatList *byStat, Statistic *p )
{
	long visitTot;
	short i;
	Statistic *retstat;

	visitTot = p->visitTot;
	
	if ( visitTot < ONEMINUTE )
		i = 0;
	else
	if ( visitTot < ONEMINUTE*4 )
		i = 1;
	else
	if ( visitTot < ONEMINUTE*9 )
		i = 2;
	else
	if ( visitTot < ONEMINUTE*29 )
		i = 3;
	else
	if ( visitTot < ONEMINUTE*44 )
		i = 4;
	else
	if ( visitTot < ONEMINUTE*59 )
		i = 5;
	else
		i = 6;

	if ( byStat->num == 0 ){
		short lp=0;
		while( GetTimeonStrings(lp)){
			byStat->SearchStat( GetTimeonStrings(lp) );
			byStat->GetNameTrans = GetTimeonStringsTrans;
			lp++;
		}
	}

	retstat = byStat->IncrStatsDrill( p->bytes, p->bytesIn, GetTimeonStrings(i), p->lastday, DRILLF_COUNTER, 0 );
	byStat->AddFiles( retstat, p->files-1 );
	byStat->AddCounters4( retstat, p->counter4 );
}

// loyalty index
// clients repeat visits status
void AddLoyaltyFromClient( VDinfoP VDptr, StatList *byStat, Statistic *p )
{
	long visits;
	short i;
	Statistic *retstat;

	visits = p->visits;
	
	if ( visits <= 1 )
		i = 0;
	else
	if ( visits <= 4 )
		i = 1;
	else
	if ( visits <= 9 )
		i = 2;
	else
	if ( visits <= 24 )
		i = 3;
	else
	if ( visits <= 49 )
		i = 4;
	else
	if ( visits <= 499 )
		i = 5;
	else
		i = 6;

	if ( byStat->num == 0 ){
		short lp=0;
		while( GetLoyaltyStrings(lp) ){
			byStat->SearchStat( GetLoyaltyStrings(lp) );
			lp++;
		}
	}
	if ( i )
		VDptr->totalRepeatClients++;

	retstat = byStat->IncrStatsDrill( p->bytes, p->bytesIn, GetLoyaltyStrings(i), p->lastday, DRILLF_COUNTER, 0 );
	byStat->AddFiles( retstat, p->files-1 );
	byStat->AddCounters4( retstat, p->counter4 );
}


static char* GetCirculationStringsTrans( short i )
{
	static long CirculationStringsTransID[]=
	{
		TIME_MANYTIMESADAY,
		TIME_TWICEADAY,
		TIME_ONCEADAY,
		TIME_TWICEAWEEK,
		TIME_ONCEAWEEK,
		TIME_TWICEAMONTH,
		TIME_ONCEAMONTH,
		TIME_RARELY,
		TIME_ONCEONLY,
		-1
	}; 

	return TranslateID( CirculationStringsTransID[i] );
}
 

// circulation index
// clients repeat time status, once a day, once a week etc...
void AddCirculationFromClient( VDinfoP VDptr, StatList *byStat, Statistic *p, long diff )
{
	static const char* CirculationStrings[]=
	{
		"Many Times a day",
		"Twice a day",
		"Once a day",
		"Twice a week",
		"Once a week",
		"Every 2 weeks",
		"Once a month",
		"Rarely",
		"Once Only",
		0
	};

	short i(7);

	if ( diff == 0 )
		i=8;
	else if ( diff < (0.5*ONEDAY) )
		i=0;
	else if ( diff < (1.0*ONEDAY) )
		i=1;
	else if ( diff < (2.0*ONEDAY) )
		i=2;
	else if ( diff < (3.5*ONEDAY) )
		i=3;
	else if ( diff < (7.0*ONEDAY) )
		i=4;
	else if ( diff < (10.5*ONEDAY) )
		i=5;
	else if ( diff < (31.0*ONEDAY) )
		i=6;

	if ( byStat->num==0 )
	{
		size_t j(0);  

		while( CirculationStrings[j] )
		{
			byStat->SearchStat( (char*)CirculationStrings[j++] );
		}

		byStat->GetNameTrans=GetCirculationStringsTrans;
	}

	Statistic* retstat=byStat->IncrStatsDrill( p->bytes, p->bytesIn, (char*)CirculationStrings[i], p->lastday, DRILLF_COUNTER, 0 );
	byStat->AddFiles( retstat, p->files-1 );
	byStat->AddCounters4( retstat, p->counter4 );
}

void AddClientDetailsToOthers( VDinfoP VDptr, Statistic *p, long clientnum )
{
	if ( VDptr && p )
	{
		//if ( VDptr->byClient )			CalcAgentVisitors( VDptr, p );
		if ( VDptr->byUser )			AddUsers( VDptr, VDptr->byClient, p );
		if ( VDptr->byOrgs )			AddOrganizations( VDptr, VDptr->byClient, p );
		if ( VDptr->byDomain )			AddCountryFromClients( VDptr, VDptr->byClient, p );

		if ( MyPrefStruct.stat_sessionHistory )
		{
			long diff( AddSessionRecords( VDptr, p, clientnum ) );

			if ( VDptr->byCirculation )
				AddCirculationFromClient( VDptr, VDptr->byCirculation, p, diff );
		}

		if ( VDptr->byLoyalty )			AddLoyaltyFromClient( VDptr, VDptr->byLoyalty, p );
		if ( VDptr->byTimeon )			AddTimeonFromClient( VDptr, VDptr->byTimeon, p );
	}
}


