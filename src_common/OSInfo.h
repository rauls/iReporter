

#ifndef OSINFO_H
#define OSINFO_H

#include "FWA.h"
#include "myansi.h"

#ifdef __cplusplus
extern "C" {
#endif


#if DEF_WINDOWS
#include <windows.h>
#else

// Not Windows
unsigned long GetUserName( char *lpBuffer, unsigned long *nSize );
unsigned long GetComputerName( char *lpBuffer, unsigned long *nSize );

/*#ifndef DWORD
#define DWORD long
#endif
#ifndef DWORD
#define TCHAR char
#endif
typedef struct _OSVERSIONINFO{ 
    DWORD dwOSVersionInfoSize; 
    DWORD dwMajorVersion; 
    DWORD dwMinorVersion; 
    DWORD dwBuildNumber; 
    DWORD dwPlatformId; 
    TCHAR szCSDVersion[ 128 ]; 
} OSVERSIONINFO; */

#endif

// All OS's
typedef struct _MemStatus
{
	short physLoad;
	long physTotal;
	long physAvail;
	long virtTotal;
	long virtAvail;
} MemStatus;

unsigned long GetOSVersion( char *lpBuffer, unsigned long *nSize );
unsigned long GetCurrDateTime( char *lpBuffer, unsigned long *nSize );
char *GetOSType();
void GetMemoryStatus( MemStatus *memStatus );

#ifdef __cplusplus
}
#endif

#endif // OSINFO_H
