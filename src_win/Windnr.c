/* 	

	File:		DNR.c 
	
*/

#include <winsock.h>

#include "Windnr.h"
#include "asyncdnr.h"
#include "net_io.h"

// from config.cpp
extern long OutDebugs( const char *txt, ... );
extern long OutDebug( const char *txt );

#ifdef _WINE
#define WSA_MAJOR_VERSION 1
#define WSA_MINOR_VERSION 0
#else
#define WSA_MAJOR_VERSION 1
#define WSA_MINOR_VERSION 1
#endif
#define WSA_VERSION MAKEWORD(WSA_MAJOR_VERSION, WSA_MINOR_VERSION)

WSADATA wsaData;
INT winsockInit = FALSE;


/*
#define WSAEAFNOSUPPORT         (WSABASEERR+47)
#define WSAEADDRINUSE           (WSABASEERR+48)
#define WSAEADDRNOTAVAIL        (WSABASEERR+49)
#define WSAENETDOWN             (WSABASEERR+50)
#define WSAENETUNREACH          (WSABASEERR+51)
#define WSAENETRESET            (WSABASEERR+52)
#define WSAECONNABORTED         (WSABASEERR+53)
#define WSAECONNRESET           (WSABASEERR+54)
#define WSAENOBUFS              (WSABASEERR+55)
#define WSAEISCONN              (WSABASEERR+56)
#define WSAENOTCONN             (WSABASEERR+57)
#define WSAESHUTDOWN            (WSABASEERR+58)
#define WSAETOOMANYREFS         (WSABASEERR+59)
#define WSAETIMEDOUT            (WSABASEERR+60)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAELOOP                (WSABASEERR+62)
#define WSAENAMETOOLONG         (WSABASEERR+63)
#define WSAEHOSTDOWN            (WSABASEERR+64)
#define WSAEHOSTUNREACH         (WSABASEERR+65)
#define WSAENOTEMPTY            (WSABASEERR+66)
#define WSAEPROCLIM             (WSABASEERR+67)
#define WSAEUSERS               (WSABASEERR+68)
#define WSAEDQUOT               (WSABASEERR+69)
#define WSAESTALE               (WSABASEERR+70)
#define WSAEREMOTE              (WSABASEERR+71)
*/

char *SocketErrorsStrings[] = {
	"000000000000000                ",
	"Network is down",
	"Network is unreachable",
	"Connection aborted",
	"Connection timedout",
	"Connection refused",
	"Host down",
	"Host unreachable", 0
};



char *SocketError( long err )
{
	switch( err ){
		case WSAENETDOWN : return SocketErrorsStrings[1]; break;
		case WSAENETUNREACH : return SocketErrorsStrings[2]; break;
		case WSAECONNABORTED : return SocketErrorsStrings[3]; break;
		case WSAETIMEDOUT : return SocketErrorsStrings[4]; break;
		case WSAECONNREFUSED : return SocketErrorsStrings[5]; break;
		case WSAEHOSTDOWN : return SocketErrorsStrings[6]; break;
		case WSAEHOSTUNREACH : return SocketErrorsStrings[7]; break;
		default :
			sprintf( SocketErrorsStrings[0], "Error %d", err );
			break;
	}
	return SocketErrorsStrings[0];
}

int DNRInit( void )
{
	return winsockInit;
}


long WSInit( void )
{
	int err;
	
	if ( !winsockInit ){
		err = WSAStartup ( WSA_VERSION, &wsaData  ); 
		if ( err ) {
			/* Tell the user that we couldn't find a usable */
			/* WinSock DLL.*/
			ErrorMsg( "Couldn't open up Winsock" );
			winsockInit = FALSE;
		} else
			winsockInit = TRUE;
	} else
		winsockInit = TRUE;
	return winsockInit;
}

void WSClose( void )
{
	if ( winsockInit )
		WSACleanup( );
	winsockInit = FALSE;
}

int GetLocalHostName( void * userDataPtr, int len )
{
	WSInit();
	return gethostname ( userDataPtr, len );
}


struct hostent *GetHostByName( char *name )
{
	WSInit();
	return gethostbyname ( name );
}

/*
typedef struct _WSAQuerySetW
{
Ê Ê DWORD           dwSize;
Ê Ê LPWSTR          lpszServiceInstanceName;
Ê Ê LPGUID          lpServiceClassId;
Ê Ê LPWSAVERSION    lpVersion;
Ê Ê LPWSTR          lpszComment;
Ê Ê DWORD           dwNameSpace;
Ê Ê LPGUID          lpNSProviderId;
Ê Ê LPWSTR          lpszContext;
Ê Ê DWORD           dwNumberOfProtocols;
Ê Ê LPAFPROTOCOLS   lpafpProtocols;
Ê Ê LPWSTR          lpszQueryString;
Ê Ê DWORD           dwNumberOfCsAddrs;
Ê Ê LPCSADDR_INFO   lpcsaBuffer;
Ê Ê DWORD           dwOutputFlags;
Ê Ê LPBLOB          lpBlob;
} WSAQUERYSETW, *PWSAQUERYSETW, *LPWSAQUERYSETW;
*/
long AddrToName( char *addrStr, void * userDataPtr)
{
	HOSTENT *thehost;
	long 	err=0, lAddr;
	//char	*errtxt;

	if ( winsockInit ){
   		//wsprintf( txt, "resolv %s ...", ipStr );
		//StatusWindowSetText( txt );

   		lAddr = inet_addr(addrStr); 

		//ErrorMsg( ipStr );
		thehost = gethostbyaddr ( (char*)&lAddr, sizeof(lAddr), AF_INET );
		if ( thehost ){
			mystrcpy( (char*)userDataPtr, thehost->h_name );
			//wsprintf( txt, "name=%s", thehost->h_name );
			//StatusWindowSetText( txt );
		} else {
			err=WSAGetLastError();
			//StatusWindowSetText( "IP has no name" );
		}
	} else
		err = -1;

/*
	switch( err ){
		case WSAHOST_NOT_FOUND : errtxt= "Authoritative Answer: Host not found" ; break;
		case WSATRY_AGAIN : errtxt= "Non-Authoritative: Host not found, or SERVERFAIL" ; break;
		case WSANO_RECOVERY : errtxt= "Non-recoverable errors, FORMERR, REFUSED, NOTIMP" ; break;
		case WSANO_DATA : errtxt= "Valid name, no data record of requested type" ; break;
		default: errtxt="IP resolved."; break;
	}
*/	
	return err;
}

long WinAddrToName( char *addrStr, void * userDataPtr)
{
	return AddrToName( addrStr, userDataPtr);
}


void *SyncAddrToName( char *addr, char *name )
{
	long errcode=0;
	if ( DNRInit() ){
		long	lAddr;
		DNR		host;

#ifndef NOSOCKETS
		lAddr = inet_addr( addr );
#else
		lAddr = IPStrToIPnum( addr );
#endif
		strcpy( host.addrStr, addr );
		host.lAddr = lAddr;

		errcode = AddrToName( host.addrStr, host.returnData );

		if ( !errcode )
			mystrcpy( name, host.returnData );
	}
	return (void*)errcode;
}





static short	lockthread = 0;

DWORD WINAPI ThreadWinAddrToName( LPVOID lpParam  )
{
	long errcode, tryagain=3, timeout=300000;
	DNRPtr dnr = (DNRPtr)lpParam;

	if ( DNRInit() )
	{
		char *p;
		if ( p=(char *)mystrchr( (char *)dnr->addrStr, '/' ))
			*p = 0;

		while( tryagain ) 
		{
			dnr->returnData[0] = 0;
			errcode = AddrToName( dnr->addrStr, dnr->returnData );
			if ( errcode == TRY_AGAIN ) {
				tryagain--;
				//OutDebug( "trying again" );
			} else
				tryagain = 0;
		}

		//while( lockthread )	timeout--;
		lockthread = 1;

		if ( !errcode )
		{
			if ( mystrstr( dnr->returnData, ".in-addr." ) )
				errcode = -1;
			//OutDebugs( "resolved to (%d) (%s)", strlen(dnr->returnData), dnr->returnData );
			//else OutDebugs( "Resolved %s = %s", dnr->addrStr, dnr->returnData );
		} //else OutDebugs( "No name for %s", dnr->addrStr );

		dnr->errcode = errcode;

		CopyDNRtoclones( dnr );
		//OutDebugs( "Copied %s to %d clones", dnr->addrStr, clones );

		lockthread = 0;
	}
	ExitThread( 0 );
	return 0;
}


static long GetLastErrorTxt( char *p )
{
	long	er = GetLastError();
    DWORD dwRet =	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
		0,
		er,
		LANG_NEUTRAL,
		(LPTSTR)p,
		255,
		0
	);
	return er;
}

//unsigned long _beginthread(_USERENTRY (*start_address)(void *), unsigned stack_size, void *arglist)

void *AsyncLookup( void *dnr, long laddr, long *dwThreadId )
{
	HANDLE *thread = NULL;

	if ( AreWeOnline() )
	{
		thread = CreateThread( NULL, 0, ThreadWinAddrToName, (void *)dnr, 0, dwThreadId );

		if ( thread )
			SetThreadPriority( thread, THREAD_PRIORITY_BELOW_NORMAL );
		else
		{
			char errTxt[256];
			long errCode;

			errCode = GetLastErrorTxt( errTxt );
			OutDebugs( "CreateThread failed (%d) - %s", errCode, errTxt );
		}
	}

	return (void*)thread;
}






