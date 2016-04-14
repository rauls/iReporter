
#include "FWA.h"

#include "DateFixFileName.h"
#include "string.h"
#include "datetime.h"
#include "myansi.h"
#include "config.h"
#include "FileTypes.h"
#include "GlobalPaths.h"

long ExtractOffsetFromToken( char *szSourcetoken )
{
	long offset = 0;

	if ( szSourcetoken )
	{
		char token[256], *t;

		strcpyx( token, szSourcetoken, '}', 1 );

		if( t = strstr( token, "-" ) ){
			offset = - (long)myatoi( t+1 );
		} else
		if( t = strstr( token, "+" ) ){
			offset = myatoi( t+1 );
		}
	}
	return offset;
}

long TokenGet_YEAR( time_t time, char *szSourcetoken )
{
	struct tm today;
	long year;

	DaysDateToStruct( time, &today );
	// adjust year if wrong	
	if ( today.tm_year < 70 )
		year = today.tm_year+2000;		// 0...70
	else
	if ( today.tm_year < 999 )
		year = today.tm_year+1900;		// 70...1000
	else
		year = today.tm_year;		// 1000...3000
	return year + ExtractOffsetFromToken( szSourcetoken );
}

long TokenGet_MM( time_t time, char *token )
{
	struct tm today;
	
	DaysDateToStruct( time, &today );
	today.tm_mon += ExtractOffsetFromToken( token );
	while ( today.tm_mon > 11 ){
		today.tm_mon -= 12;
		today.tm_year++;
	}
	while ( today.tm_mon < 0 ){
		today.tm_mon += 12;
		today.tm_year--;
	}
	return today.tm_mon+1;
}

long TokenGet_DD( time_t time, char *token )
{
	struct tm today;

	time += (ExtractOffsetFromToken( token ) * ONEDAY );
	DaysDateToStruct( time, &today );
	return today.tm_mday;
}

extern char		gPrefsFilename[MAXFILENAMESSIZE];

/*
  New additions include allowing maths on {*_BEG} and {*_END} tokens
  and also each tokens maths {DD-4} is independant and doesnt effect other {DD} tokens.
  and also {MMM+4} will work too now.
  {PREFSNAME} is new, its nice to have since you can make output paths dependant on the prefs name, good
  for automatic output dirs.

  Future Ideas:
	{SCHEDULENAME}
	{SCHEDULEPOS}
*/
char *DateFixFilename( char *filename, char *outfilename )
{
	extern long	allfirstTime;	// yuck - for now
	extern long allendTime;		// yuck - for now

	struct tm today;
	long	time;
	char	newfileName[1024], tempfileName[1024], token[32], newnum[1024],*p, *t;

	if ( !filename )
		return NULL;

	mystrcpy( tempfileName, filename );
	p = tempfileName;

	while( p = mystrchr( p, '{' ) )
	{
		strcpyx( token, p, '}', 1 );

		// ------------------------------- work out the time
		if( strstr( token, "_BEG}" ) ){
			time=allfirstTime;
		} else
		if( strstr( token, "_END}" ) ){
			time=allendTime;
		} else {
			time = GetCurrentCTime();
		}
		DaysDateToStruct( time, &today );

		// ------------------------------- Now check which token is used
		// This is only good for OUTPUT locations (maybe these 2 should be in myansi.cpp/CopyFilenameUsingPath() ?
		if( !strcmpd( "SITENAME", token+1 ) ){
			char *host = (char*)MyPrefStruct.siteurl;
			if ( !strcmpd( "http://", host ) )
				sprintf( newnum, "%s", host+7 );
			else
				sprintf( newnum, "%s", host );
		} else
		// This is only good for OUTPUT locations
		if( !strcmpd( "PREFSNAME", token+1 ) ){
			FileFromPath( gPrefsFilename, newnum );
			if ( t = strchr( newnum, '.' ) )
				*t = 0;
		} else

		if( !strcmpd( "DD", token+1 ) ){
			sprintf( newnum, "%02d", TokenGet_DD( time, token ) );
		} else
		if( !strcmpd( "MMM", token+1 ) ){
			int mm = TokenGet_MM( time, token );
			mystrncpy( newnum, months+(4*(mm-1)), 3 );		//mm-1 used to be today.tm_mon
			newnum[3] = 0;
		} else
		if( !strcmpd( "MM", token+1 ) ){
			sprintf( newnum, "%02d", TokenGet_MM( time, token ) );
		} else
		if( !strcmpd( "YYYY", token+1 ) ){
			sprintf( newnum, "%04d", TokenGet_YEAR( time, token ) );
		} else
		if( !strcmpd( "YY", token+1 ) )
			sprintf( newnum, "%02d", TokenGet_YEAR( time, token )%100 );
		else
		//{Q} 1-4 or {QQ} 1st-4th {QQQQ} first-fourth?
		if( !strcmpd( "QQQQ", token+1 ) ){
			char quarters[][8] = { "first", "second", "third", "fourth" };

			sprintf( newnum, "%s", quarters[ ((today.tm_mon+1)/4)+1 ] );
		} else
		if( !strcmpd( "QQ", token+1 ) ){
			char quarters[][8] = { "1st", "2nd", "3rd", "4th" };

			sprintf( newnum, "%s", quarters[ ((today.tm_mon+1)/4)+1 ] );
		} else
		if( !strcmpd( "Q", token+1 ) ){
			sprintf( newnum, "%d", ((today.tm_mon+1)/4)+1 );
		} else
		if( !strcmpd( "DAYN", token+1 ) )
			sprintf( newnum, "%d", today.tm_wday );
		else
		if( !strcmpd( "DAY", token+1 ) )
			sprintf( newnum, "%s", weekdays[ today.tm_wday ] );
		else
		if( !strcmpd( "HH", token+1 ) )
			sprintf( newnum, "%02d", today.tm_hour );

		ReplaceStr( tempfileName, newfileName, token, newnum, FALSE );
		mystrcpy( tempfileName, newfileName );
		p++;
	}
	if ( outfilename )
		mystrcpy( outfilename, tempfileName );
	else
		mystrcpy( filename, tempfileName );
	return outfilename;
}

// ---------------------------------- END OF DATE TOKEN CODE --------------------------------------------------------
