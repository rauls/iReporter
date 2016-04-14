#ifndef __VERSION_H
#define __VERSION_H

#include "FWA.h" 

#define OBSOLETE_PREF_VERSION			4		// for FWA 4.0.x
#define CURRENT_PREF_VERSION			5		// for FWA 4.5
#define DEF_APP_SETTINGS_VERSION_4_5		"4.5"	// for FWA 4.5

#ifdef DEF_MAC
	#define VERSION_STRING		gVersionString
#else
	#ifdef	DEF_DEBUG
		#define	VERSION_STRING	"5.0 (Debug)"
	#elif	DEF_FREEVERSION
		#define	VERSION_STRING	"5.0.1"
	#else
		#define	VERSION_STRING	"5.0"
	#endif
#endif

#ifdef	DEF_FULLVERSION
	#define	PRODUCT_TITLE	"iReporter Enterprise"
#elif	DEF_FREEVERSION
	#define	PRODUCT_TITLE	"iReporter Free"
#else
	#define	PRODUCT_TITLE	"iReporter"
#endif


#ifdef __cplusplus
extern "C" {
#endif

extern char gVersionString[256];
extern char scrollMsg[];

#if DEF_UNIX || DEF_WINDOWS
typedef short OSErr;
#endif

long DaysSince01012001ForBuildDate();
unsigned long GetProductTitle( char *lpBuffer );
unsigned long GetAppBuildDate(char *lpBuffer);
unsigned long GetAppBuildDetails( char *lpBuffer );
OSErr GetVersionString(char *versionString);

#ifdef __cplusplus
}
#endif




#endif // VERSION_H
