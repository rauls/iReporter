#include "FWA.h"
#include "EngineClientDNR.h"

#include <string.h>
#include <ctype.h>

#include "AsyncDNR.h"
#include "EngineBuff.h"
#include "EngineParse.h"
#include "EngineStatus.h"	// for IsStopped.
#include "Stats.h"
#include "config.h"			// for MyPrefStruct

#ifdef DEF_WINDOWS				// WINDOWS include
	#include "resource.h"	// for IDS_PROCESSCLIENTS.
	#include "Winmain.h"	// temp for progress bar...
#endif				

#ifdef DEF_UNIX
	#include "ResDefs.h"
	#include "util.h"
#endif

#ifdef DEF_MAC
	#include "MacStatus.h"
	#include "progress.h"
#endif

CQDNSCache	*addressList;

#ifndef DEF_MAC									// these are defined in MacStatus.h
extern "C" void StatusSet( char *txt );
extern "C" void StatusWindowSetText( char *txt );
extern "C" void StatusWindowSetProgress( long percent, char *text );
extern "C" int StatusSetID( int id, ... );
#endif

extern long		allTotalRequests;

char	rev_domain[10240];

int CleanupClientString( HitDataRec *Line )
{
	int isReverse = FALSE;
	if ( Line->clientaddr ) {
		int i;
		i = mystrlen(Line->clientaddr)-1;
		if ( Line->clientaddr[ i ] == '.' ) {			// only if there is a . (DOT) at the end then
			Line->clientaddr[ i ] = 0;				// get rid of trailing '.'
			i--;
		}
		//strip out port information '128.250.23.2:8000'
		i = stripPort(Line->clientaddr,i,':');
		
		if (isdigit(Line->clientaddr[0])) {
			char *pt;
			pt = mystrstr(Line->clientaddr,".in-addr.arpa");		// get rid of ".in-addr.arpa"
			if (pt) {
				*pt = 0;
				ReverseAddressARPA( Line->clientaddr, rev_domain );
				mystrncpy( Line->clientaddr, rev_domain, 512 );
				isReverse = TRUE;			// IP address, therefore already reversed
			} else
			if (isdigit(Line->clientaddr[i]))
				isReverse = TRUE;			// IP address, therefore already reversed
		}

		/* reverse the subdomain if necessary */
		if (isReverse) {
			mystrncpy(rev_domain,Line->clientaddr,512);
		} else {
			ReverseAddress( Line->clientaddr, rev_domain );
		}		
	}
	return isReverse;
}


// ---------------------------------------------------------------------------------
// RESOLVE ALL CLIENTS
// ---------------------------------------------------------------------------------
// find a DNR that has resolved and extract the info back to our global data
// and continue on processing transparently....
long ProcessReadyDNR( char *out, long *uid, long *errcode )
{
	DNRPtr	dnr;
	
	dnr = FindReadyDNR();

	if ( dnr ){
		char	*dnrNAME, *ipStr;
		long	ip;

		ipStr = GetDNRaddr( dnr );
		ip = IPStrToIPnum( ipStr );

	
		*uid = GetDNRUID( dnr );
		*errcode = GetDNRerrcode( dnr );

		if ( *errcode ){		// if IP didnt resolve remember it
			if ( addressList->LookupIP( ip ) == 0 ) {			/* does this entry exist in address cache? */
				addressList->Add( ip, NULL, NULL ); 			/* add to name cache */
			}
			*out = 0;
			sprintf( buff, "Can not resolve #%d , %s (waiting %d)", dnr->uid , ipStr, GetWaitingDNR() );
			OutDebugs( buff );
		} else {
			dnrNAME = GetDNRHost( dnr );	// if IP resolved remember reversed 
			ReverseAddress( dnrNAME, rev_domain ); 				/* must reverse the name returned by DNS */
			if ( addressList->LookupIP(ip) == 0 ) 				/* does this entry exist in address cache? */
				addressList->Add( ip, rev_domain, NULL ); 		/* add to name cache */

			strcpy( out, rev_domain );

			sprintf( buff, "Resolved IP #%d , %s (waiting %d)", dnr->uid , ipStr, GetWaitingDNR() );
			OutDebugs( buff );
//#ifndef DEF_MAC
//#endif
		}
		MakeFreeDNR( dnr );
		return TRUE;
	}
	return FALSE;
}



double gStartTime;		// time of when DNS started to be resolved...

void ShowProgressDNRLevel( long value, long total )
{
#ifdef DEF_WINDOWS
	StatusWindowSetProgress( value*1000/total, NULL );
	if ( gStartTime )
	{
		double timesofar;
		timesofar = (double)(timems()-gStartTime);
		sprintf( buff, "Resolving IP %d of %d.  (%.1f/sec in %02d secs)", value, total, GetReturnedDNR() / (double)timesofar, (long)timesofar );
		ProgressChangeMainWindowTitle( value*100/total, buff );
	}
#else
	sprintf( buf2, "Resolving IP %d of %d.", value, total );
	ShowDNSProgress( value*1000/total, TRUE, buf2, gStartTime );
#endif
}




long GoResolveRemainder( VDinfoP VDptr, long clientnum )
{
	if ( MyPrefStruct.dnslookup ){
		Statistic *p;
		/* Go resolve remaining resolves at the end of all SPAWNS*/
		long	maxwait = MyPrefStruct.dnr_ttl, i, errcode;
		DNRPtr	readyDNR;
		time_t	timeToStop;

		sprintf( buf2, "Resolving %d idle IPs...", GetWaitingDNR() );
		StatusWindowSetText( buf2 );

		timeToStop = time(0) + maxwait;

		while( (GetWaitingDNR()>0 || (readyDNR=FindReadyDNR())) && !IsStopped() && (time(0) < timeToStop) ) 
		{
			i = GetEmptyDNR(1);		// Force background lookups for unix/mac
			if( ProcessReadyDNR( buf2, &clientnum, &errcode ) )
			{
				if( clientnum >= 0 ){
					p = VDptr->byClient->GetStat(clientnum);
					if ( p ) {
						if ( !errcode )
						{
							p->ChangeName( buf2 );
							sprintf( buf2, "Resolved %d, %d left, IP#%d = %s Good", GetReturnedDNR(), GetWaitingDNR(), clientnum, p->GetName() );
							OutDebug( buf2 );
						} else 
						{
							sprintf( buf2, "Resolved %d, %d left, IP#%d = %s Failed", GetReturnedDNR(), GetWaitingDNR(), clientnum, p->GetName() );
							OutDebug( buf2 );
						}
					} else SleepTicks( 3 );
					//AddClientDetailsToOthers( VDptr, p, clientnum );
				} else SleepTicks( 3 );
			} else {
				// still waiting for something to resolve... twiddle twiddle...
				SleepSecs(1);
			}		

			sprintf( buf2, "%d IPs remaining, (timeout in 00:%d)", GetWaitingDNR(), (timeToStop-time(0)) );
			StatusWindowSetText( buf2 );
			ShowProgressDNRLevel( GetWaitingDNR(), MyPrefStruct.dnsAmount );
		}
		if ( GetWaitingDNR()>0 ) 
		{
			sprintf( buf2, "%d IPs Flushed ...", GetWaitingDNR() );
			StatusWindowSetText( buf2 );
			SleepTicks( 3 );
			ClearAllDNR();
		}
	}
	return 1;
}



// returns clientID  if resolved  (0...n)
// returns -1  if didnt resolve
long GetResolvedClient( VDinfoP VDptr, long show_status )
{

	while( FindReadyDNR() && !IsStopped() )
	{
		long clientnum, errcode;

		if( ProcessReadyDNR( buf2, &clientnum, &errcode ) )
		{
			Statistic *p;

			if( clientnum >= 0 )
			{
				p = VDptr->byClient->GetStat(clientnum);
				if ( p )
				{
					char msgTxt[256];
					if ( !errcode && *buf2 )
					{
						if ( show_status ){
							sprintf( msgTxt, "Resolved %d, IP#%d=%s Good", GetReturnedDNR(), clientnum, p->GetName() );
							StatusWindowSetText( msgTxt );
						}

						p->ChangeName( buf2 );
						return clientnum;
					} else {
						if ( show_status ){
							sprintf( msgTxt, "Resolved %d, IP#%d=%s Failed", GetReturnedDNR(), clientnum, p->GetName() );
							StatusWindowSetText( msgTxt );
						}
						return -1;
					}
				}
			}
		}
	}
	GetEmptyDNR(0);
	return -1;
}

// resolve client at end, this is faster than resolving as you go.
// returns
// 0 = name not resolved , continue as normal
// 1 = name is in cache, rename client
// 2 = name is resolved, rename client

long GoResolveClient( VDinfoP VDptr, Statistic *p, long clientnum, long background_mode )
{
	/* Go resolve this client now... 	*/
	if ( MyPrefStruct.dnslookup ){
		if ( p ) {
			if ( p->length == -1 ) {			//isdigit(*cname) && *cname!='0' 
				char *cname;
				char *dname;
				void *thread;

				cname = p->GetName();

				// lookup name in cache
				dname = addressList->Lookup( cname );

				// if ip is in cache , use it if its valid
				if ( dname ){
					if ( dname[0] ) {		// IP doesnt have a name, so its always IP
						p->ChangeName( dname );
						return 1;		// ah its already in the cache, return that.
					} else
						return 2;		// cache says DNS doesnt resolve... return status
				}

				// if its not in cache, go throw a resolve for the IP
				if ( !dname && MyPrefStruct.dnslookup==1 ){

					// ###############################################
					if ( GetReturnedDNR() < 1 )
					{
						sprintf( buf2, "Resolving #%d, IP = %s", clientnum, cname );
						StatusWindowSetText( buf2 );
					} else
					{
						OutDebugs( "Resolving #%d, IP = %s", clientnum, cname );
					}
#if (DEF_MAC)
					SleepTicks( 1 );		// let it go into carbon to do background lookups for a moment
#endif

#if (DEF_UNIX)
					// every 100 clients do a little background lookups
					if ( clientnum && clientnum%100 == 0 )
						GetEmptyDNR(1);
#endif

					thread = SpawnWinAddrToName( cname, (long)clientnum );

					// Spawn a request, if it fails, keep trying
					while ( background_mode && thread == NULL && !IsStopped() )
					{
						// if all slots are full, try to resolve some names first.
						long	waiting,
								empty,
								waitingSeconds=0;

						waiting = GetWaitingDNR();

						SleepTicks( 1 );	// let it go into carbon to do background lookups for a moment

						empty = GetEmptyDNR(1);
						//OutDebugs( "DNR FULL, waiting=%d, empty=%d", waiting, empty );

						// All Slots full, lets wait till some get resolved...
						while( !FindReadyDNR() && !IsStopped() )
						{
							SleepSecs(1);
							waitingSeconds++;

							sprintf( buf2, "Resolved %d, DNR Full %d secs", GetReturnedDNR(), waitingSeconds );
							StatusWindowSetText( buf2 );
							ShowProgressDNRLevel( clientnum, VDptr->byClient->GetStatListNum() );

							empty = GetEmptyDNR(1);		// make sure unix/mac does some background OS lookups
						}
						//OutDebugs( "Full Loop finished..., ready=%d", FindReadyDNR() );
						GetResolvedClient( VDptr, TRUE );

						thread = SpawnWinAddrToName( cname, (long)clientnum );
					}

					if ( background_mode )
						ShowProgressDNRLevel( clientnum, VDptr->byClient->GetStatListNum() );

					// Find any resolved names and use them
					GetResolvedClient( VDptr, background_mode );
					return (long)thread;
				}
			}
		}
	}
	return -1;
}


// Dynamicly adjust the DNS amount value according to how we are going
// so if we are doing too slow we reduce the amount , but if we are doing real well
// we can up the amount...
// Only for testing at the moment, we just wont call it for release...
extern "C" int DNRmax;
void DynamicDNS( long timesofar )
{
	long rate_level, dnr_rate;


	if ( timesofar && DNRmax )
	{
		dnr_rate = GetReturnedDNR() / timesofar;

		rate_level = (100 * dnr_rate) / DNRmax;

		if ( rate_level < 33 && (DNRmax>10) ) 
			DNRmax--;

		if ( rate_level > 50 && (DNRmax<10000) ) 
			DNRmax++;
	}
}


long PostCalcResolveClients( VDinfoP VDptr, long logStyle )
{
	long clientnum, maxclients, lastpercent=0;

	if( VDptr->byClient )
	{
		Statistic *p;
		long ips = 0;

#if DEF_MAC
		ShowProgressDetail (1000, TRUE, NULL, 0);
		if (MyPrefStruct.dnslookup)
			StatusWindowSetMessage ("Resolving Visitor IPs ...");
#else
		StatusSetID( IDS_PROCESSCLIENTS );
#endif
		maxclients = VDptr->byClient->GetStatListNum();
		gStartTime = timems();

		//GoResolveRemainder( VDptr, 0 );
		//return maxclients;

		for ( clientnum=0; clientnum < maxclients && !IsStopped(); clientnum++ ){

			p = VDptr->byClient->GetStat( clientnum );
			// only if the client has a name and is really valid then add its details to other lists.
			if( p->length == -1 ) {
				ips++;
				GoResolveClient( VDptr, p, clientnum, TRUE /* keep trying if DNR queue full */ );
			}

			// Adjust dns amount to a sustainable/workable level with least amount of lag.
			if( MyPrefStruct.dnslookup&DNR_DYNAMICMODE && (clientnum % 30) == 0 )
				DynamicDNS( (timems()-gStartTime) );
		}

		gStartTime = 0;
		// resolve remaining clients...
		GoResolveRemainder( VDptr, 0 );
	}
	return maxclients;
}



// Perform one lookup of a client, and remember the last one which was done,so we can can
// call here again to do the next client.
//
// Call with ZERO args to reset.
//
long BackgroundResolveClients( VDinfoP VDptr, long logStyle )
{
	long maxclients, lastpercent=0;
static	long clientnum = 0;

	if ( VDptr && VDptr->byClient )
	{
		Statistic *p;

		maxclients = VDptr->byClient->GetStatListNum();

		if ( clientnum == 1	)
			gStartTime = timems();

		if ( clientnum < maxclients )
		{
			p = VDptr->byClient->GetStat( clientnum );
			// only if the client is an IP , resolve it
			if ( p->length == -1 ) 
			{
				if ( GoResolveClient( VDptr, p, clientnum, FALSE /* return quickly if DNR queue full */ ) != NULL )
					clientnum++;
			} else
				clientnum++;

			// Adjust dns amount to a sustainable/workable level with least amount of lag.
			if( MyPrefStruct.dnslookup&DNR_DYNAMICMODE && (clientnum % 30) == 0 )
				DynamicDNS( (timems()-gStartTime) );
		}

	} else {
		clientnum = 0;
		gStartTime = timems();
	}
	return maxclients;
}



// ---------------------------------------------------------------------------------
// END OF RESOLVE CLIENTS
// ---------------------------------------------------------------------------------




