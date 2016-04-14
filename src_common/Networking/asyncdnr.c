/* 	
	File:		DNR.c 
	Too MUCH MAC STUFF here!  yuk
*/

#include "Compiler.h"
#include <stdio.h>
#include <string.h>
#include "asyncdnr.h"
#include "myansi.h"

#ifdef DEF_MAC
	#include <Threads.h>
	#include <OpenTransport.h>
	#include <OpenTptInternet.h>
	#include "Carbonize.h"
	#include "MacUtil.h"
	#include "MacDNR.h"
	#include "main.h"
	#include "progress.h"

	#define NEWDNR		0

	#define NOSOCKETS	1
#endif // #ifdef DEF_MAC

#if DEF_UNIX
	#include "dnr.h"
#endif

#if DEF_WINDOWS             /* WINDOWS include */
	#include "Windnr.h"
#endif

extern long OutDebugs( const char *txt, ... );
extern long OutDebug( const char *txt );


long	DNRmax;
long	DNRtotalRequests = 0;		// total LInes being resolved
long	DNRuniqueRequests = 0;		// total unique domain IPs being resolved
long	DNRwaitingRequests = 0;
long	DNRReturnedRequests = 0;

long	countMalloc = 0;
long	countFree = 0;

#define	MAXDNRS		DNRmax



//DNR		DNRdata[MAXDNRS+1];
static DNRPtr	DNRHead = NULL;

long GetEmptyDNR( long flag )
{
#if (DEF_UNIX)
	if ( flag )
	{
		//OutDebug( "DobackgroundLookups" );
		DobackgroundLookups(0);
	} else
	{
		long ct;
		while( DNRwaitingRequests > (MAXDNRS*.8) )
		{
			//OutDebug( "DobackgroundDNS" );
			DobackgroundLookups(0);
		}
	}

#endif
// Maintain old mac functionality
#if (DEF_MACOS)
	while( DNRwaitingRequests > (MAXDNRS/4) )
		DobackgroundLookups();

	if ( flag )
		DobackgroundLookups();
#endif
	return MAXDNRS - DNRwaitingRequests;
}

long GetUnique( void )
{
	return DNRuniqueRequests;
}

long GetWaitingDNR( void )
{
	return DNRwaitingRequests;
}

long GetTotalDNR( void )
{
	return DNRtotalRequests;
}

long GetReturnedDNR( void )
{
	return DNRReturnedRequests;
}

DNRPtr DNRAdd( void )
{
	DNRPtr newP;
	DNRPtr p = DNRHead;

	if (newP = (DNRPtr)malloc( DNRSIZE )) {
		countMalloc++;
		newP->num = DNRtotalRequests;
		memset( newP, 0, DNRSIZE );
		newP->next = p;
		newP->prev = 0;
		newP->me = newP;		// from Mac code RHF
		if(p) p->prev = newP;
	}
	return newP;
}

DNRPtr FindEmptyDNR( void )
{
	if( DNRwaitingRequests < MAXDNRS )
		return DNRAdd();
	else
		return NULL;
}

void DNRRemove( DNRPtr p )
{
	DNRPtr	prev,next;

	if ( p ){
		prev = p->prev;
		next = p->next;

		if( prev ) prev->next = next;
		if( next ) next->prev = prev;

		if( p == DNRHead ) DNRHead = next;
// new
		memset( p, 0, DNRSIZE );
//
		free( p );
		countFree++;
	}
}

void DNRDeleteX( int n )
{
	DNRPtr last=0, next, p = DNRHead;
	int index=1;

	if ( p ){
		while( p ){
			next = p->next;
			if ( n ){
				if ( n == index ){
					free( p );
					if ( last )
						last->next = next;
				}
			} else {
				free( p );
			}
			index++;
			last = p;
			p = next;
		}
	}
}

#define	FINDEMPTY		1
#define	FINDREADY		2
#define	FINDTHREAD		3
#define	FINDWAITING		4
#define	FINDADDR		5
#define	FINDLADDR		6

DNRPtr FindDNRItem( int type, void *data )
{
	DNRPtr p = DNRHead, last=0, next;
	int index=1;

	if ( p ){
		while( p ){
			next = p->next;
			switch( type ){
				case FINDEMPTY:		if( p->thread == 0 ) return p; break;
				case FINDREADY:		if( p->ready == 1) return p; break;
				case FINDWAITING:	if( p->thread && p->ready==0 && (long)p->thread != 0xBEEF ) return p; break;
				case FINDADDR:		if( strcmp(p->addrStr, (char*)data ) == 0 ) return p; break;
				case FINDLADDR:		if( p->lAddr == (unsigned long)data ) return p; break;
			}
			index++;
			last = p;
			p = next;
		}
	}
	return 0;
}


DNRPtr FindWaitingDNR( void )
{
	return FindDNRItem( FINDWAITING, NULL );
}

DNRPtr FindReadyDNR( void )
{
	return FindDNRItem( FINDREADY, NULL );
}

DNRPtr FindDNRaddr( char *ipStr )
{
	return FindDNRItem( FINDADDR, (void*)ipStr );
}

// -----------------------------------------------------------------
// This makes the DNR read for use and also copies to any virtual lookups if any.
long CopyDNRtoclones( DNRPtr dnr )
{
	DNRPtr p = DNRHead, next;
	long clones=0;

	while( p ){
		next = p->next;

		/* if the DNR slot has the same IP as ours then copy our HOST info to it
		 * if ( strcmp(DNRdata[lp].addrStr, DNRdata[dnr].addrStr) == 0 ){
		 */
		if ( p->lAddr == dnr->lAddr ) {
			if( (long)p->thread == 0xBEEF ) {
				memcpy( p->returnData, dnr->returnData, MAXRETURNDATA );
				p->errcode = dnr->errcode;
				p->ready = 1;
			}
			clones++;
		}
		p = next;
	}
	dnr->ready = 1;
	DNRReturnedRequests++;
	DNRwaitingRequests--;
	return clones;
}


char *GetDNRHostname( DNRPtr dnr )
{
#if DEF_WINDOWS             /* WINDOWS include */
	HOSTENT			*thehost;

	if ( dnr ) {
		thehost = (HOSTENT *) dnr->returnData;
		return thehost->h_name;
	}
#endif
	return 0;
}

char *GetDNRHost( DNRPtr dnr )
{
	if ( dnr ) return dnr->returnData; else return 0;
}

char *GetDNRaddr( DNRPtr dnr )
{
	if ( dnr ) return dnr->addrStr; else return 0;
}

char *GetDNRlAddr( DNRPtr dnr )
{
	if ( dnr ) return (char*)dnr->lAddr; else return 0;
}


long GetDNRUID( DNRPtr dnr )
{
	if ( dnr ) return dnr->uid; else return -1;
}

long GetDNRerrcode( DNRPtr dnr )
{
	if ( dnr ) return dnr->errcode; else return 0;
}

void MakeFreeDNR( DNRPtr dnr )
{
	if ( dnr ) 
	{
		if ( dnr->thread && (long)dnr->thread != 0xBEEF && dnr->ready == 1 )
		{
#ifndef DEF_MAC
			CloseHandle( dnr->thread );
#endif
			DNRRemove( dnr );
		}
	}
}

void FreeAllDNR( void )
{
	DNRPtr last=0, next, p = DNRHead;

	while( p ){
		next = p->next;
		MakeFreeDNR( p );
		p = next;
	}
	DNRtotalRequests = 0;
	DNRuniqueRequests = 0;
}


// new
void FreeEmptyDNRs( void )
{
	DNRPtr last=0, next, p = DNRHead;

	while( p )
	{
		next = p->next;
		if ( p->thread == 0 )
			MakeFreeDNR( p );
		else
			p->thread = (void*)0xffffffff;
			
		p = next;
	}
}
//

void ClearAllDNR( void )
{
// new
#ifdef DEF_MAC
	FreeEmptyDNRs();
#else
//
	FreeAllDNR();
#endif
}



#ifndef	TRY_AGAIN
#define	TRY_AGAIN	3
#endif

void ReturnedAddrToName( void *lpParam , char *name )
{
	long errcode, tryagain=3, timeout=10000;
	DNRPtr dnr = (DNRPtr)lpParam;
	long ip = (long)lpParam;

	if ( dnr ){
		if ( name ) {
			errcode = 0;
			strcpy( dnr->returnData, name );
		} else {
			errcode = -1;
			dnr->returnData[0] = 0;
		}

		if ( !errcode ){
			if ( strstr( dnr->returnData, ".in-addr." ) )
				errcode = -1;
		}

		dnr->errcode = errcode;
		CopyDNRtoclones( dnr );
	}
	return;
}

 
void *SpawnWinAddrToName( char *addr, long uid )
{
	unsigned long	dwThreadId;
	void			*thread = NULL;

	if ( DNRInit() && addr )
	{
		long	tries = 0;
		DNRPtr	dnr, other;

		dnr = FindEmptyDNR();

		if ( dnr )
		{
			unsigned long	lAddr;									// correct type on Mac is InetHost

			DNRHead = dnr;

#ifndef NOSOCKETS
			lAddr = inet_addr( addr );
#else
			lAddr = IPStrToIPnum( addr );
#endif

			other = FindDNRItem( FINDLADDR, (void*)lAddr );

			strcpy( dnr->addrStr, addr );
			dnr->uid = uid;
			dnr->lAddr = lAddr;

			if ( !other )
			{					// if request for X IP is not there, async DNS
				thread = AsyncLookup( (void*)dnr, lAddr, &dwThreadId );
				if ( !thread ) 
					OutDebug( "failed to asynclookup" );

				DNRuniqueRequests++;
				DNRwaitingRequests++;
			} else {
				thread = (void*)0xBEEF;
				dnr->ready = other->ready;
				OutDebugs( "add clone %s, countMalloc=%d/%d", addr, countMalloc,countFree );
			}

			DNRtotalRequests++;
			dnr->thread = thread;
			dnr->trys = 1;
		} else
			OutDebug( "no empty dnrs left" );
	}
#ifndef DEF_MAC
	else
	{
		if ( !DNRInit() )
			OutDebug( "Winsock init failed" );

		if ( addr == NULL )
			OutDebug( "no address to lookup, addr = NULL" );
	}
#endif
	return thread;
}

void *SpawnAddrToName( char *addr, char *lineBuff )
{
	return SpawnWinAddrToName( addr, (long)lineBuff );
}

void *AsyncWinAddrToName( char *addr, char *lineBuff )
{
	return SpawnWinAddrToName( addr, (long)lineBuff );
}

long OpenDNRDriver( char *server, long timeout, long retries, long max)
{
	long		err = 0;

	DNRtotalRequests = 0;
	DNRuniqueRequests = 0;
	DNRReturnedRequests = 0;

	DNRmax = max;
#ifdef DEF_MAC
	err = InitLookup();
#endif

#if DEF_WINDOWS
	WSInit();
	err = 0;
#endif

#if DEF_UNIX
	DNRInit();
	SetDNROptions( server, timeout, retries );
	err = 0;
#endif

	return (err);
}

long CloseDNRDriver(void)
{
#ifdef DEF_MAC
	return (CloseLookup());
#else
	return 0;
#endif
}

