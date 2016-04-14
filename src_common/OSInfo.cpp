#include "FWA.h"
#include "myansi.h"
#include "OSInfo.h"
#include "datetime.h"
#include "config.h"
#if DEF_UNIX || DEF_WINDOWS
#include <sys/stat.h>
#endif

#if DEF_MAC
	#include <Gestalt.h>
	//#include "main.h"
#endif

#include "ResDefs.h"

// 
// Windows only functions - Functions which are chameleons of Windows Functions
// 

#if DEF_WINDOWS

unsigned long GetOSVersion( char *lpBuffer, unsigned long *nSize )
{
	static unsigned long strAlreadyCalculatedLen = 0;
	static char OSVersion[64];

	if ( lpBuffer )
	{
		if ( strAlreadyCalculatedLen && strAlreadyCalculatedLen <= *nSize )
		{
			strcpy( lpBuffer, OSVersion );
			return strAlreadyCalculatedLen;
		}
		size_t currLen(0);
		OSVERSIONINFO info;
		info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if ( GetVersionEx( &info ) )
		{
			if ( info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
			{
				if ( info.dwMinorVersion == 0 )
					mystrncpy( lpBuffer, "Windows 95", static_cast<short>(*nSize) );
				else
					mystrncpy( lpBuffer, "Windows 98", static_cast<short>(*nSize) );
				lpBuffer[*nSize] = 0; 
				currLen = strlen( lpBuffer );
			}
			else // == VER_PLATFORM_WIN32_NT
			{
				if ( info.dwMajorVersion >= 5 )
				{
					mystrncpy( lpBuffer, "Windows 2000 ", static_cast<short>(*nSize) );
				}
				else // info.dwMajorVersion < 5
				{
					mystrncpy( lpBuffer, "Windows NT ", static_cast<short>(*nSize) );
				}
				lpBuffer[*nSize] = 0;
				currLen = mystrlen(lpBuffer);
				if ( currLen + 5 <= *nSize )
				{
					lpBuffer[currLen++] = static_cast<char>(info.dwMajorVersion) + '0';
					lpBuffer[currLen++] = '.';
					lpBuffer[currLen++] = static_cast<char>(info.dwMinorVersion) + '0';
					if ( info.dwMinorVersion > 9 )
					{
						lpBuffer[currLen++] = static_cast<char>(info.dwMinorVersion) + '0';
					}
					lpBuffer[currLen] = 0;
				}
			}
			int infoStrLen = mystrlen( info.szCSDVersion ) + 1;
			if ( infoStrLen && currLen + infoStrLen + 3 > *nSize )
			{
				lpBuffer[currLen++] = ' ';
				lpBuffer[currLen++] = '(';
				mystrcpy( &lpBuffer[currLen], info.szCSDVersion );
				currLen += infoStrLen;
				lpBuffer[currLen++] = ')';
				lpBuffer[currLen++] = 0;

				if ( currLen < 64 )
				{
					strAlreadyCalculatedLen = currLen;
					mystrcpy( OSVersion, lpBuffer );
				}
			}
			return currLen; // returns the actual amount of chars copied
		}
		mystrncpy( lpBuffer, "Unknown", static_cast<short>(*nSize) );
		return mystrlen( lpBuffer ) + 1; // returns the actual amount of chars copied
	}
	return 0;
}

void GetMemoryStatus( MemStatus *memStatus )
{
	MEMORYSTATUS winMemStatus;
	GlobalMemoryStatus( &winMemStatus );
	memStatus->physLoad = static_cast<short>(winMemStatus.dwMemoryLoad);
	memStatus->physTotal = winMemStatus.dwTotalPhys;
	memStatus->physAvail = winMemStatus.dwAvailPhys;
	memStatus->virtTotal = winMemStatus.dwTotalVirtual;
	memStatus->virtAvail = winMemStatus.dwAvailVirtual;
}


#endif // DEF_WINDOWS


// 
// Mac only functions -  Functions which are chameleons of Windows Functions
// 

#ifdef DEF_MAC

unsigned long GetUserName( char *lpBuffer, unsigned long *nSize )
{
	if ( lpBuffer )
	{
		// Please replace this code with mac function...
		mystrncpy( lpBuffer, "Unknown", *nSize );
		return mystrlen( lpBuffer ) + 1; // returns the actual amount of chars copied
	}
	return 0;
}

unsigned long GetComputerName( char *lpBuffer, unsigned long *nSize )
{
    SInt32				response = 0;
	char				name[128];
	OSErr				err = noErr;
	
	if (lpBuffer)
	{
		err = Gestalt (gestaltUserVisibleMachineName, &response);

		sprintf (name, (char *)response);
		// Please replace this code with mac function...
		mystrncpy( lpBuffer, "Unknown", *nSize );
		return mystrlen( lpBuffer ) + 1; // returns the actual amount of chars copied
	}
	return 0;
}

unsigned long GetOSVersion( char *lpBuffer, unsigned long *nSize )
{
    SInt32	response = 0;
	char	osTemp[5], osString[16];
	OSErr 	err = noErr;
	
	if (lpBuffer)
	{
#if !TARGET_API_MAC_CARBON
		err = Gestalt (gestaltSystemVersion, &response);
		sprintf (osTemp, "%x", response);
		sprintf (osString, "Mac OS %c.%c.%c", osTemp[0], osTemp[1], osTemp[2]);

		mystrncpy (lpBuffer, osString, *nSize);
#else
		mystrncpy (lpBuffer, "Mac OS X", *nSize);		// until versioning scheme for OS X known
#endif
		return (mystrlen (lpBuffer) + 1); // returns the actual amount of chars copied
	}
	return 0;
}

void GetMemoryStatus( MemStatus *memStatus )
{
    SInt32	response = 0;
	OSErr 	err = noErr;
	
	err = Gestalt (gestaltPhysicalRAMSize, &response);
	memStatus->physTotal = response;

	err = Gestalt (gestaltLogicalRAMSize, &response);
	memStatus->virtTotal = response - memStatus->physTotal;

	memStatus->physLoad = 0;//macMemStatus.xxx;			// what is physLoad for?
	memStatus->physAvail = 0;//macMemStatus.xxx;
	memStatus->virtAvail = 0;//macMemStatus.xxx;
}

#endif // #ifdef DEF_MAC


// 
// Unix only functions -  Functions which are chameleons of Windows Functions
// 

#if DEF_UNIX
#include "unistd.h"
#include "stdlib.h"

unsigned long GetUserName( char *lpBuffer, unsigned long *nSize )
{
	if ( lpBuffer )
	{
		// Please replace this code with unix function...
		mystrncpy( lpBuffer, getlogin(), *nSize );
		return mystrlen( lpBuffer ) + 1; // returns the actual amount of chars copied
	}
	return 0;
}

unsigned long GetComputerName( char *lpBuffer, unsigned long *nSize )
{
	if ( lpBuffer )
	{
		// Please replace this code with unix function...
		gethostname( lpBuffer, *nSize );
		return mystrlen( lpBuffer ) + 1; // returns the actual amount of chars copied
	}
	return 0;
}

unsigned long GetOSVersion( char *lpBuffer, unsigned long *nSize )
{
	if ( lpBuffer )
	{
		// Please replace this code with unix function...
		mystrncpy( lpBuffer, "Unix", *nSize );
		return mystrlen( lpBuffer ) + 1; // returns the actual amount of chars copied
	}
	return 0;
}


/*
struct mallinfo {
  int arena;    /* total space allocated from system 
  int ordblks;  /* number of non-inuse chunks 
  int smblks;   /* unused -- always zero 
  int hblks;    /* number of mmapped regions 
  int hblkhd;   /* total space in mmapped regions 
  int usmblks;  /* unused -- always zero 
  int fsmblks;  /* unused -- always zero 
  int uordblks; /* total allocated space 
  int fordblks; /* total non-inuse space 
  int keepcost; /* top-most, releasable (via malloc_trim) space 
};
*/

#include "malloc.h"
void GetMemoryStatus( MemStatus *memStatus )
{
#if !__FreeBSD__
	struct mallinfo mi;

	mi = mallinfo ();
	//UnixMemStatus unixMemStatus;
	//UnixMemoryFunction( &unixMemStatus );
	memStatus->physLoad  = mi.uordblks;
	memStatus->physTotal = mi.hblkhd;
	memStatus->physAvail = mi.fordblks;
	memStatus->virtTotal = mi.hblkhd;
	memStatus->virtAvail = mi.fordblks;
#endif
}

#endif //DEF_UNIX

//
// All OS General Functions...
//

unsigned long GetCurrDateTime( char *lpBuffer, unsigned long *nSize )
{
	if ( lpBuffer )
	{
		time_t dateTime = GetCurrentCTime();
		DateTimeToStringTranslated( dateTime, lpBuffer );
		return mystrlen( lpBuffer ) + 1; // returns the actual amount of chars copied
	}
	return 0;
}

char *GetOSType()
{
	static char* osString = 0;
	if ( osString )
		return osString;
#ifdef DEF_MAC
	#if !TARGET_API_MAC_CARBON
		osString = "[Mac OS]";
	#else
		osString = "[Mac OS X]";
	#endif
#endif
#if DEF_UNIX
	osString = "[UNIX]";
#endif
#if DEF_WINDOWS
	osString = "[Windows]";
#endif
	return osString;
}

