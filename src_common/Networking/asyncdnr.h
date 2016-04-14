#ifndef	__DNR_H
#define	__DNR_H

#include "Compiler.h"

#ifdef __cplusplus
extern "C"{
#endif

#if !defined(MAXGETHOSTSTRUCT)
	#ifndef DEF_WINDOWS
		#define	MAXGETHOSTSTRUCT 256
	#else
		#define	MAXGETHOSTSTRUCT 1024
	#endif
#endif
#define	MAXRETURNDATA	MAXGETHOSTSTRUCT



#if DEF_UNIX
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifdef DEF_MAC
#include <OpenTransport.h>
#include <OpenTptInternet.h>
#endif

typedef struct DNRrec {
#ifdef DEF_MAC
	InetDomainName	returnData;
	InetHost		lAddr;
#else
	unsigned long	lAddr;
	char	returnData[MAXRETURNDATA];
#endif
	struct DNRrec *next, *prev;
	long	num;
	void	*thread;						/* if NULL, slot is empty to spawn a DNR */
	long	ready;							/* if TRUE, slot is ready to be added to database NOW */
	long	trys;
	long	errcode;

	char	addrStr[256];
	long	uid;							// This is the client index, to where the result goes
	struct	DNRrec *me;

#if DEF_UNIX
	struct in_addr addr;
#endif
} DNR, *DNRPtr;

#define	DNRSIZE	(sizeof(struct DNRrec))

long GetEmptyDNR( long flag );
long GetUnique( void );
long GetWaitingDNR( void );
long GetTotalDNR( void );
long GetReturnedDNR( void );

DNRPtr FindEmptyDNR( void );
DNRPtr FindReadyDNR( void );
DNRPtr FindDNR( void *thread );
DNRPtr FindWaitingDNR( void );

long CopyDNRtoclones( DNRPtr dnr );

char *GetDNRHost( DNRPtr dnr );
char *GetDNRaddr( DNRPtr dnr );
long GetDNRUID( DNRPtr dnr );
long GetDNRerrcode( DNRPtr dnr );

void MakeFreeDNR( DNRPtr dnr );
void FreeAllDNR( void );
void ClearAllDNR( void );

void *SpawnWinAddrToName( char *addr, long uid );
void *AsyncWinAddrToName( char *addr, char *lineBuff );
void AsyncRecieveDNR( long dwParam, long lParam );

void *SpawnAddrToName( char *addr, char *lineBuff );

long OpenDNRDriver( char *server, long timeout, long retries, long max);
long CloseDNRDriver(void);

#ifdef __cplusplus
}
#endif


#endif	//__DNR_H

