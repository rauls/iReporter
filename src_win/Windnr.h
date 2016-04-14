#ifndef	__WINDNR_H
#define	__WINDNR_H

#ifdef __cplusplus
extern "C"{
#endif


#include <winsock.h>

int DNRInit( void );
long	WSInit( void );
void	WSClose( void );
long	AddrToName( char *, void * userDataPtr);
long WinAddrToName( char *addrStr, void * userDataPtr);
struct hostent *GetHostByName( char *name );
char *SocketError( long err );

void *AsyncLookup( void *dnr, long laddr, long *dwThreadId );

#ifdef __cplusplus
}
#endif


#endif	//__DNR_H
