/* version.c */

#include "FWA.h"
#include "version.h"
#include "myansi.h"
#include "datetime.h"
#include "ResDefs.h"

#ifdef DEF_MAC
	#include "main.h"		// for gResNum
#endif

char gVersionString[256];

char scrollMsg[] =
{
"\n\n\nWelcome to iReporter\n"
"Version : \n [VER]\n\n"
"Registered to :\n [NAME]\n\n"
"Built ("__DATE__" "__TIME__")\n"
"(c) 1997-2002\nRedFlag Software\n"
" \n"



"\n"

"Please also visit our website\n"
"<B>http://www.reflag.ch\n\n\n"
"\n\n\n\n\n\n\n\n\n\0"
};

// Gets the number of days since 01/01/2001 to when the application was last built
long DaysSince01012001ForBuildDate()
{
	static long days = 0;
	static long daysSince2001 = 0;
	char *dateStr;
	time_t date2001;
	time_t dateNow;
	struct tm date;
	const char *buildDateStr = __DATE__;

	if ( daysSince2001 )
		return daysSince2001;

	StringToDaysDate( "1/1/2004", &date, DATE_DD_MM_YY );
	Date2Days( &date, &date2001 );

#if ( __MACOSX__ && DEF_UNIX )
	dateStr = buildDateStr;
#else
	dateStr = ConvDateFormat( buildDateStr, DATE_Mon_DD_YY, DATE_MM_DD_YY );
#endif

	StringToDaysDate( dateStr, &date, DATE_MM_DD_YY );
	Date2Days( &date, &dateNow );

	daysSince2001 = ((dateNow - date2001)/ONEDAY) + 1;

	return daysSince2001;
} 


unsigned long GetProductTitle(char *lpBuffer)
{
	return mystrcpy( lpBuffer, PRODUCT_TITLE );
}


unsigned long GetAppBuildDate(char *lpBuffer)
{
	char	dateConverted[DATE_SIZE];

#if ( __MACOSX__ && DEF_UNIX )
	mystrcpy (dateConverted, __DATE__ );
#else
	mystrcpy (dateConverted, ConvDateFormat (__DATE__, DATE_Mon_DD_YY, DATE_MM_DD_YY));
#endif
	mystrcpy (dateConverted, ConvDateFormatResTrans (dateConverted, DATE_MM_DD_YY, DATE_Mon_DD_YY));
	if (lpBuffer)
	{
		sprintf (lpBuffer, "%s %s", dateConverted, __TIME__);
		return (mystrlen (lpBuffer) + 1); // returns the actual amount of chars copied
	}
	return (0);
}



unsigned long GetAppBuildDetails (char *lpBuffer)
{
	char	build[32];
	char	builtOn[32];

	mystrcpy (build, ReturnString (IDS_BUILD));
	mystrcpy (builtOn, ReturnString (IDS_BUILTON));
	if (lpBuffer)
	{
		char buildDate[256];
		GetAppBuildDate( buildDate );
		// WHy have a mac area?????? (RS)
#ifdef DEF_MAC
		char	verStr[64];

		GetVersionString (verStr);
		sprintf (lpBuffer, "%s %s (%s %d - %s %s)", PRODUCT_TITLE, verStr,         build, DaysSince01012001ForBuildDate(), builtOn, buildDate);
#else
		sprintf (lpBuffer, "%s %s (%s %d - %s %s)", PRODUCT_TITLE, VERSION_STRING, build, DaysSince01012001ForBuildDate(), builtOn, buildDate);
#endif
		return (mystrlen (lpBuffer) + 1); // returns the actual amount of chars copied
	}
	return (0);
}

//Mac only???? If so IFDEF it all and prefix it MacOS_
OSErr GetVersionString (char *versionString)
{
#ifdef DEF_MAC
	VersRecHndl		theVersionH;
	char			shortVersionString[256];
	SInt16			currentResRef = 0;

	currentResRef = CurResFile();
	UseResFile (gResNum);
	theVersionH = (VersRecHndl)Get1Resource ('vers', 1);
	strcpyp2c (shortVersionString, (**theVersionH).shortVersion);
	ReleaseResource ((Handle)theVersionH);
	UseResFile (currentResRef);

#ifdef DEF_FULLVERSION
	mystrcpy (versionString, "Enterprise ");
#else
	mystrcpy (versionString, "");
#endif
	mystrcat (versionString, shortVersionString);

	return (noErr);
#endif // #ifdef DEF_MAC
	return (0);
}


