/* datetime.c */

#include "Compiler.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datetime.h"
#include "myansi.h"
#include "config.h"
#include "ResDefs.h"
#include "translate.h"

#if DEF_MAC
	#include "MacUtil.h"
#endif

#if DEF_WINDOWS
#include <windows.h>
#include "resource.h"
#endif

#define MONTHS			12
#define WEEKDAYS		7
#define BUF16			16

char	*monthsl= "jan feb mar apr may jun jul aug sep oct nov dec ";
char	*months = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ";
char	*Months = "JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC ";
char	*weekdays[] = { "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday", 0 };
static int weekdaysLoaded = 0;
static char monthsT[MONTHS][BUF16];
static char MonthsT[MONTHS][BUF16];
static char weekdaysT[MONTHS][BUF16];
static long monthsResIds[MONTHS] = { IDS_JAN,IDS_FEB,IDS_MAR,IDS_APR,IDS_MAY,IDS_JUN,IDS_JUL,IDS_AUG,IDS_SEP,IDS_OCT,IDS_NOV,IDS_DEC };




static char fast100[200];

static char fast10000[40000];
static char *fp=NULL;
static long *lp;
//static short *wp;

void ClearLoadWeekdaysAndMonths()
{
	weekdaysLoaded = 0;	
}

char *GetWeekday( time_t timed )
{
	struct tm date;
	long w;

	DaysDateToStruct( timed, &date );
	w = date.tm_wday;
	return weekdays[w];
}

char *GetWeekdayTranslated( time_t timed )
{
	struct tm date;
	long w;

	if ( !weekdaysLoaded )
	{
		LoadWeekdaysAndMonths( weekdaysT, monthsT, MonthsT );
		weekdaysLoaded = 1;
	}

	DaysDateToStruct( timed, &date );
	w = date.tm_wday;
	return weekdaysT[w];
}

char *GetWeekdayTranslatedByDayNum( int dayOfWeek )
{
	if ( !weekdaysLoaded )
	{
		LoadWeekdaysAndMonths( weekdaysT, monthsT, MonthsT );
		weekdaysLoaded = 1;
	}
	if ( dayOfWeek < 0 || dayOfWeek >= 7 )
		return "Invalid Day";
	return weekdaysT[dayOfWeek];
}

char *GetMonthTranslatedByMonthNum( int monthOfYear )
{
	static char buf[4];
	if ( monthOfYear < 0 || monthOfYear >= MONTHS  )
		return "Invalid Month";

	if ( !HaveReadLangFile() )
	{
		 // If the language file doesn't get loaded, use the English default strings
		mystrncpy( buf, &months[monthOfYear*4], 3 );
		buf[3] = 0;
		return buf;
	}

	if ( !weekdaysLoaded )
	{
		LoadWeekdaysAndMonths( weekdaysT, monthsT, MonthsT );
		weekdaysLoaded = 1;
	}
	return monthsT[monthOfYear];
}


char *GetMonthTranslated( time_t timed )
{
	struct tm date;
	long m;

	DaysDateToStruct( timed, &date );
	m = date.tm_mon;
	return GetMonthTranslatedByMonthNum( m );
}

// extremely fast integer (2 or 4 digits to ascii )
// technique is an old one from late 80's amiga demos done in MC680x0
// with leading zero

#ifdef _SUNOS
// this works in 64 bit CPUs ok
#define	FOURDIGIT2TXT(num,ptr)		{char* s = &fast10000[num<<2]; *ptr++=*s++; *ptr++=*s++; *ptr++=*s++; *ptr++=*s++;}
#define	TWODIGIT2TXT(num,ptr)		{char* s = &fast100[num<<1]; *ptr++=*s++; *ptr++=*s++; }
#define	TWODIGIT2TEXT(num,ptr,lz)	{if ( num < 10 && !lz ) { *ptr++ = num|0x30; } else TWODIGIT2TXT( num, ptr ); }
#else
// this is fine for 32bit
#define	FOURDIGIT2TXT(num,ptr)		{long *lp = (long*)(fast10000 + (num<<2)); (*(long*)(ptr)) = *lp; ptr+=4; }
#define	TWODIGIT2TXT(num,ptr)		{short *wp = (short*)(fast100 + (num<<1)); (*(short*)(ptr)) = *wp; ptr+=2; }
#define	TWODIGIT2TEXT(num,ptr,lz)	{if ( num < 10 && !lz ) { *ptr++ = num|0x30; } else TWODIGIT2TXT( num, ptr ); }
#endif

#define	MONTH_SIZE	256
static char fastmonths[MONTH_SIZE];
static char fastmonthslow[MONTH_SIZE];

char Month2Num( const char *p )
{
	unsigned char i;
   	i = (p[2] * p[1]) ^ p[0];
	if ( p[0] > 0x60 )
	 	return fastmonthslow[i];
	else
	 	return fastmonths[i];
}
 
#define	MONTH2NUM( p ) fastmonths[ ((*p++) ^ (*p++) ^ (*p++)) ]
	//(((strstrn( months, text, 3) - months)/4)+1);

void InitFastMonths( void )
{
	unsigned char i, v,v2,v3, *p;
	long	tmp[13];

	memset( fastmonths, 0, MONTH_SIZE );
	memset( fastmonthslow, 0, MONTH_SIZE );

	for( i=0;i<12; i++){
		//v2 = months[ (i*4) ];
		p = (unsigned char *)&months[ (i*4) ];
		v = (p[2] * p[1]) ^ p[0];
		fastmonths[v] = i+1;

		p = (unsigned char *)&Months[ (i*4) ];
		v2 = (p[2] * p[1]) ^ p[0];
		fastmonths[v2] = i+1;

		p = (unsigned char *)&monthsl[ (i*4) ];
		v3 = (p[2] * p[1]) ^ p[0];
		fastmonthslow[v3] = i+1;

		tmp[i] = v | (v2<<8) ;
	}
	return;
}

void InitFast10000( void )
{
	long i;
	char *ptr;
	short v;

	if( fp == 0 ){
		ptr = fast100;
		for(i=0;i<100;i++){
			v = i/10;
			*ptr++ = v+0x30;
			v = i%10;
			*ptr++ = v+0x30;
		}
		
		ptr = fast10000;
		for(i=0;i<10000;i++){
			v = i/100;
			*ptr++ = fast100[v*2];
			*ptr++ = fast100[v*2+1];
			v = i%100;
			*ptr++ = fast100[v*2];
			*ptr++ = fast100[v*2+1];
		}
		fp = fast10000;
		InitFastMonths();
	}
}


void StructToUSDate( struct tm *date, char *p )
{
	long y = date->tm_mon+1;
	TWODIGIT2TXT( y, p )
	*p++ = '/';
	y = date->tm_mday;
	TWODIGIT2TXT( y, p )
	*p++ = '/';
	y = date->tm_year/100;
	TWODIGIT2TXT( y, p )
	y = date->tm_year%100;
	TWODIGIT2TXT( y, p )
	*p++ = 0;
}

void StructToYYYYMMDDDate (struct tm *date, char *p)
{
	long	y;
	
	// make YYYY
	y = (date->tm_year + 1900) / 100;
	TWODIGIT2TXT (y, p)
	y = date->tm_year % 100;
	TWODIGIT2TXT (y, p)
	*p++ = '/';
	// make MM
	y = date->tm_mon + 1;
	TWODIGIT2TXT (y, p)
	*p++ = '/';
	// make DD
	y = date->tm_mday;
	TWODIGIT2TXT (y, p)
	*p++ = 0;
}

time_t UnixTimetoCTime( const char *utStr )
{
	long	v1;
	
	v1 = myatoint( utStr, 0 );
	return v1;
}

time_t JDToCTime( double jd )
{
	long	ct;	
	jd -= 2440587.501;
	ct = (long)(jd * ONEDAY);
	return ct;
}

void GetCurrentDate( struct tm *thisTimeData )
{
#if DEF_MAC		//TIMESTAT
	DateTimeRec	tmpTime;

	GetTime(&tmpTime);
	if ( thisTimeData )
		tmStruct( thisTimeData, &tmpTime );
#else
	if ( thisTimeData ){
		time_t timer;
		
		time( &timer );
		memcpy( thisTimeData, localtime ( &timer ), sizeof( struct tm ) );
	}
#endif
}

/*

function WeekOfYear( dDate: TDateTime ): Integer;
var
X, nDayCount: Integer;
nMonth, nDay, nYear: Word;
begin
nDayCount := 0;
deCodeDate( dDate, nYear, nMonth, nDay );
For X := 1 to ( nMonth - 1 ) do
nDayCount := nDayCount + MonthDays( X, nYear );
nDayCount := nDayCount + nDay;
Result := ( ( nDayCount div 7 ) + 1 );
end; 

  */

long GetCurrentWeek( struct tm *t )
{
	if ( t ){
		long week;
		struct tm dt;

		memcpy( (void *)&dt, (void *)t, sizeof( struct tm ) );


		week = (t->tm_yday - t->tm_wday)/7;
		return week+1;
	}
	return -1;
}


							// 31, 28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31
static long monthTots[] = { 0, 31, 59,  90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

long mymktime( struct tm *date )
{
	long	a, b, day, month, year;
	long	ctime;

	day = date->tm_mday;
	month = date->tm_mon;
	year = date->tm_year;
	if ( date->tm_year < 1900 )
		year += 1900;

	//a = (year / 2000);		// skipped leap years
	if ( year%4 == 0 && month <= 1 )
		a = 1; else a=0;
	b =  ((year-1968)/4) -a;
	year -= 1970;
	//ctime = floor(365.25*year) + floor(30.6001 *  (month + 1)) + b;
	ctime = (365*year) + (monthTots[month]) + b + day - 1;
	date->tm_wday = (ctime+4)%7;
	date->tm_yday = (monthTots[month]) + day;

	ctime *= ONEDAY;
	ctime += date->tm_hour * 3600;
	ctime += date->tm_min * 60;
	ctime += date->tm_sec;
	return ctime;
}

#define YEAR0           	1900                    	/* the first year */
#define EPOCH_YR        	1970            		/* EPOCH = Jan 1 1970 00:00:00 */
#define SECS_DAY        	(24L * 60L * 60L)
#define LEAPYEAR(year) 	(!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year) 	(LEAPYEAR(year) ? 366U : 365U)

const unsigned int _ytab[2][12] = {
                { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
                { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
        };

struct tm *mygmtime(register const unsigned long *timer)
{
#if DEF_UNIX
	return gmtime( timer );
#else	
	static struct tm br_time;
	register struct tm *timep = &br_time;
	unsigned long time = *timer;
	register unsigned long dayclock, dayno;
	int year = EPOCH_YR;

	dayclock = (unsigned long)time % SECS_DAY;
	dayno = (unsigned long)time / SECS_DAY;

	timep->tm_sec = dayclock % 60;
	timep->tm_min = (dayclock % 3600) / 60;
	timep->tm_hour = dayclock / 3600;
	timep->tm_wday = (dayno + 4) % 7;       /* day 0 was a thursday */

	while (dayno >= YEARSIZE(year)) {
			dayno -= YEARSIZE(year);
			year++;
	}
	timep->tm_year = year - YEAR0;
	timep->tm_yday = dayno;
	timep->tm_mon = 0;

	while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) {
			dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
			timep->tm_mon++;
	}
	timep->tm_mday = dayno + 1;
	timep->tm_isdst = 0;

	return timep;
#endif
}




// convert struct date to long ctime
time_t Date2Days(struct tm *date, time_t *time )
//long Date2Days(struct tm *date, long *time )
{
	if ( date ){
		long ct;
		ct = mymktime( date );
//		*time = mktime( date ) - timezone;
		if ( time )
			*time = ct;
		return ct;
	} else
		return 0;
}

time_t Time2CTime( void *datePtr, time_t *time )
//long Time2CTime( void *datePtr, long *time )
{
	long	hh,mm,ss;
	long	ct;
	struct	tm *date = (struct	tm *)datePtr;
	
	hh = date->tm_hour * 3600;
	mm = date->tm_min * 60;
	ss = date->tm_sec;

	ct = (hh+mm+ss);

	if ( time )
		*time = ct;

	return ct;
}



void DateTimeToString( time_t ctime, char *aStr )
{
	if ( ctime ){
		char *out = aStr;
		out = DaysDateToString( ctime, out, -1, '/', TRUE, TRUE );
		*(out-1) = ' ';
		DaysTimeToString( ctime, out, TRUE, FALSE, TRUE );
	} else {
		mystrcpy( aStr, "(N/A)" );
	}
}

void DateTimeToStringTranslated( /*long*/ time_t ctime, char *aStr )
{
	if ( ctime ){
		char *out = aStr;
		out = DaysDateToStringTranslated( ctime, out, 16, -1, '/', TRUE, TRUE );
		*(out-1) = ' ';
		DaysTimeToString( ctime, out, TRUE, FALSE, TRUE );
	} else {
		mystrcpy( aStr, "(N/A)" );
	}
}

//
// Function: DateTimeDurationToString()
// Description: Used to produce the time duration between 2 time values in the form of a string.
// If the period is greater than X days, only the number of days are given, if it is less X days,
// then we give the hours also.
//
void DateTimeDurationToString( time_t startTime, time_t endTime, short onlyShowDaysWhenBiggerThanThis, char *aStr )
{
	long		_HR,_MIN,_SEC,_MM,_YY,_DD;
	long		totalDays = 0;
	long		totalHours = 0;
	char		buf[64];
	time_t		ct = 0;
	struct tm	*logDate;
	buf[0] = 0;

	// Check that we have a positive duration
	if ( endTime < startTime )
		return;

	if ( endTime == startTime )
	{
		strcpy( aStr, "0" );
		return;
	}
	ct = endTime - startTime;
	logDate = mygmtime((const unsigned long *)&ct );
	if ( !logDate )
		return;

	_SEC = logDate->tm_sec;
	_MIN = logDate->tm_min;
	_HR = logDate->tm_hour;
	_DD = logDate->tm_mday;
	_MM = logDate->tm_mon;
	_YY = logDate->tm_year-70;

	// Count the total days & hours
	if ( _YY )
		totalDays += _YY * 365;
	if ( _MM )
		totalDays += (long)((double)_MM * 30.5);
	if ( _DD )
		totalDays += (_DD-1);

	// Check if we have more than X days, if so, then just give duration in days
	if ( totalDays >= onlyShowDaysWhenBiggerThanThis )
	{
		sprintf( aStr, "%d days", totalDays );
		return;
	}

	// If there is less than X days, then we need to show the amount of days
	if ( totalDays > 1 )
		sprintf( aStr, "%d days", totalDays );
	else if ( totalDays == 1 )
		sprintf( aStr, "%d day", totalDays );

	totalHours = _HR;
	// Check if we need to add a comma seperator between days and hours
	if ( totalDays && totalHours )
		strcat( aStr, ", " );

	// Check if we need to add the hours
	if ( totalHours )
	{
		if ( totalHours == 1 )
			sprintf( buf, "%d hour", totalHours );
		else
			sprintf( buf, "%d hours", totalHours );
		strcat( aStr, buf );
		return;
	}

	// If we already have days, but no hours, then this is ok - so return
	if ( totalDays && !totalHours )
		return;

	// If we have no days and no hours, then add minutes
	if ( totalDays == 0 && totalHours == 0 )
	{
		if ( _MIN == 1 )
			sprintf( buf, "%d min", _MIN );
		else if ( _MIN > 1 )
			sprintf( buf, "%d mins", _MIN );
		else
		{
			if ( _SEC == 1 )
				sprintf( buf, "%d sec", _SEC );
			else
				sprintf( buf, "%d secs", _SEC );
		}

		strcpy( aStr, buf );
	}
	return;
}


// write out time value into a stringTime, can be >24hrs and days:hh:mm:ss
void CTimetoString( /*long*/ time_t ct, char *outStr )
{
	long	ctime;
	long	dd,hh,mm,ss;
	register char 	*buffPtr;
	
	buffPtr = outStr;	

	ctime = ct;

	if ( ctime < 0 ){
		*buffPtr++ = '-';
		ctime = -ctime;
	}
	if ( ctime ) {
		ss = ctime % 60;
		mm = (ctime/60) % 60;
		hh = (ctime/3600) % 24;
		dd = (ctime/ONEDAY);
	} else
		ss = mm = hh = dd = 0;
	
	if (!hh && dd<9999) {
		;
	} else if (dd<9999) {
		buffPtr = StrAdd( buffPtr, Long2Str(dd*24+hh,2),":");	
	} else {
		TWODIGIT2TXT( hh, buffPtr)
		*buffPtr++ = ':';
	}
	TWODIGIT2TXT( mm, buffPtr)
	*buffPtr++ = ':';
	TWODIGIT2TXT( ss, buffPtr)
	*buffPtr++ = 0;
}


time_t GetCurrentCTime( void )
{
	long	thisTime;
	struct tm	thisTimeData;

	GetCurrentDate( &thisTimeData );
#if DEF_MAC		//TIMESTAT
	thisTime = mktime( &thisTimeData );
#else
	Date2Days( &thisTimeData, &thisTime );
	//thisTime = mktime( &thisTimeData );
#endif

	return thisTime;
}


void CTimeToDateTimeStr( /*long*/ time_t ct, char *outstr )
{
	char	dStr[32];
	char	tStr[16];

	if ( ct ) {
		DaysDateToString( ct, dStr, GetDateStyle(), '/', 1,1);
		CTimetoString( ct, tStr );
		sprintf( outstr, "%s %s", dStr, tStr );
	}
}



//convert DD/MM/YY into a ctime time
time_t DateStringToCTime( const char *date )
{
	struct tm		logDate;
	time_t			logDays;

	StringToDaysDate( date, &logDate, GetDateStyle() );
	Date2Days( &logDate, &logDays );
	return logDays;
}


long ctimetotm( long ct, struct tm *logDate )
{
	double      dj, f, z, a, b, c, d, e, alfa, day, help, help1, help2;
	long		_MM,_YY;
	short 		dg=0;

	dj = 2440587.500 + (ct/ONEDAY);
	f = modf(dj , &z);
	if(z < 2299161.0)
		a = z;
	else {
		alfa = floor((z - 1867216.25) / 36524.25);
		a = z + 1 + alfa - floor(alfa / 4);
	}
	b = a + 1524;
	c = floor((b - 122.1) / 365.25);
	d = floor(365.25 * c);
	e = floor((b - d) / 30.6001);
	day = b - d - floor(30.6001 * e) + f;
	help = modf(day, &help1);
	logDate->tm_mday = (int)help1;
	help1 = help * 24;
	help = modf(help1, &help2);
	logDate->tm_hour = (int)help2;
	help1 = help * 60;
	help = modf(help1, &help2);
	logDate->tm_min = (int)help2;
	logDate->tm_sec = (int)(help * 60);
	if(e < 14)
		_MM = (long)(e - 1);
	else
		_MM = (long)(e - 13);
	if( _MM > 2)
		_YY = (long)(c - 4716);
	else
		_YY = (long)(c - 4715);

	logDate->tm_mon = _MM;
	logDate->tm_year = _YY;

	return ct;
}


char *DaysDateToString( time_t ct, char *aStr,short aFrmt,char aDelim,int aLdZ,int aCent)
{
	long		_HR,_MIN,_SEC,_MM,_YY,_DD;
	short 		dg=0;
	char 		*p, *buffPtr;
	struct tm	*logDate;

//printf( "line %d\n", __LINE__ );
	logDate = mygmtime((const unsigned long *)&ct );

	if ( !logDate ) return aStr;

	_SEC = logDate->tm_sec;
	_MIN = logDate->tm_min;
	_HR = logDate->tm_hour;
	_DD = logDate->tm_mday;
	_MM = logDate->tm_mon+1;
	_YY = logDate->tm_year+1900;

	buffPtr = aStr;

	switch (aFrmt) {
		case -1: //custom format Mar 23 1998
			p = months+((_MM*4)-4);
			strncpy(buffPtr,p,3); buffPtr+=3;
			*buffPtr++ = ' ';

			TWODIGIT2TEXT( _DD, buffPtr, aLdZ );
			*buffPtr++ = ' ';

			if (aCent){
				FOURDIGIT2TXT( _YY, buffPtr );
			} else {
				_YY=_YY%100;
				TWODIGIT2TXT( _YY, buffPtr );
			}
			break;
		default:
		case 0: //US format m/d/y
			TWODIGIT2TEXT( _MM, buffPtr, aLdZ );
			*buffPtr++ = aDelim;
			TWODIGIT2TEXT( _DD, buffPtr, aLdZ );
			*buffPtr++ = aDelim;

			if (aCent){
				FOURDIGIT2TXT( _YY, buffPtr );
			} else {
				_YY=_YY%100;
				TWODIGIT2TXT( _YY, buffPtr );
			}
			break;
		case 1: //Au format d/m/y
			TWODIGIT2TEXT( _DD, buffPtr, aLdZ );
			*buffPtr++ = aDelim;

			TWODIGIT2TEXT( _MM, buffPtr, aLdZ );
			*buffPtr++ = aDelim;

			if (aCent){
				FOURDIGIT2TXT( _YY, buffPtr );
			} else {
				_YY=_YY%100;
				TWODIGIT2TXT( _YY, buffPtr );
			}
			break;
		case 2: //Swiss format y/m/d
			if (aCent){
				FOURDIGIT2TXT( _YY, buffPtr );
			} else {
				_YY=_YY%100;
				TWODIGIT2TXT( _YY, buffPtr );
			}
			*buffPtr++ = aDelim;

			TWODIGIT2TEXT( _MM, buffPtr, aLdZ );
			*buffPtr++ = aDelim;

			TWODIGIT2TEXT( _DD, buffPtr, aLdZ );
			break;
	}
	*buffPtr++ = 0;
//printf( "%ld, yy=%d, mm=%d, dd=%d = %s\n", ct, _YY, _MM, _DD, aStr );
	//printf( fast100 );
	return buffPtr;
}

char *DaysDateToStringTranslated( time_t ct, char *aStr, short bufS, short aFrmt,char aDelim,int aLdZ,int aCent )
{
	register	char 	*buffPtr;
	struct tm	*logDate;
	char		*p;
	short		len;

	logDate = mygmtime((const unsigned long *)&ct );
	if ( !logDate ) return aStr;

	switch (aFrmt) 
	{
		case -1: {
			long _DD = logDate->tm_mday;
			long _MM = logDate->tm_mon+1;
			long _YY = logDate->tm_year+1900;

			if ( weekdaysLoaded == 0 )
			{
				LoadWeekdaysAndMonths( weekdaysT, monthsT, MonthsT );
				weekdaysLoaded = 1;
			}

			buffPtr = aStr;
			p = GetMonthTranslatedByMonthNum(_MM-1);
			len = (short)mystrncpy(buffPtr,p,bufS);
			if ( len == bufS )
			{
				buffPtr[bufS-1] = 0;
				return buffPtr;
			}
			buffPtr+=len;
			*buffPtr++ = ' ';

			if ( bufS-len > 3 )
			{
				TWODIGIT2TEXT( _DD, buffPtr, aLdZ );
				*buffPtr++ = ' ';
			}
			len+=3;


			if (aCent){
				if ( bufS-len > 4 ){
					FOURDIGIT2TXT( _YY, buffPtr );
				}
				else if ( bufS-len > 2 ){
					_YY=_YY%100;
					TWODIGIT2TXT( _YY, buffPtr );
				}
			} else if ( bufS-len > 2 ){
				_YY=_YY%100;
				TWODIGIT2TXT( _YY, buffPtr );
			}
		}
			break;
		default:
			return DaysDateToString( ct, aStr, aFrmt, aDelim, aLdZ, aCent );
	};

	*buffPtr++ = 0;
	return buffPtr;
}

// Converts ctime to a string with options
// Leading Zero, 12Hr format, seconds display
char *DaysTimeToString( time_t aDay, char *aStr, int aLdZ, int a12Hr, int aSeconds)
{
	long	ctime;
	long	hh,mm,ss;
	short ah=0,dg=0;
	register char 	*buffPtr;
	
	ctime = aDay;
	buffPtr = aStr;

	if ( ctime < 0 ){
		*buffPtr++ = '-';
		ctime = -ctime;
	}

	ss = ctime % 60;
	mm = (ctime/60) % 60;
	hh = (ctime/3600) % 24;
	
	if (a12Hr) {
		if (hh>11)
			ah=1;
		hh = (ctime/3600) % 12;
		if (hh==0)
			hh=12;
	}
	
	TWODIGIT2TEXT( hh, buffPtr, aLdZ ); *buffPtr++ = ':';
	TWODIGIT2TEXT( mm, buffPtr, aLdZ );
	if (aSeconds) {
		*buffPtr++ = ':';
		TWODIGIT2TEXT( ss, buffPtr, aLdZ );
	}
	if (a12Hr) {
		*buffPtr++ = ' ';
		if (ah){
			*buffPtr++ = 'P';
		} else {
			*buffPtr++ = 'A';
		}
		*buffPtr++ = 'M';
	}
	*buffPtr++ = 0;
	return buffPtr;
}




void	DaysDateToStruct( time_t ctime, struct tm *data )
{
	struct tm *td;

	if ( ctime < 0 )
		ctime = 0; // This will avoid "td" being NULL when calling gmtime() with a ctime value < 0

	td = mygmtime( (const unsigned long *)&ctime );
	memcpy( data, td, sizeof( struct tm ) );
	data->tm_year+= 1900;
}


short GetDateStyle()
{
#if	DEF_WINDOWS
	{
		static char datef[32]="\0";
		static short dateFormat = -1;
		
		if ( dateFormat != -1 )
			return dateFormat;

		// Get the operating system date format, for the first time only...
		if ( datef[0] == 0 )
			GetLocaleInfo( LOCALE_USER_DEFAULT , LOCALE_SSHORTDATE , datef, 32 );

		if ( datef[0] == 'Y' || datef[0] == 'y' )
			return dateFormat = DATE_YY_MM_DD;
		else if ( datef[0] == 'D' || datef[0] == 'd' )
			return dateFormat = DATE_DD_MM_YY;
		else
			return dateFormat = DATE_MM_DD_YY;
	}
#elif DEF_MAC
	return DATE_DD_MM_YY;
#else
	return DATE_MM_DD_YY;
#endif // DEF_WINDOWS

}


/* aStr = _DD/_MM/_YY    12/18/96     to struct tm */
int StringToDaysDate( const char *aStr, struct tm *date, short aFrmt)
{
	char	*dt[5];
	int		*values[5];
	register char	*pt, c=1;
	register int		num=0, i=0;

	if (!aStr){
		memset( date, 0, sizeof(struct tm) );
		return FALSE;
	}

	pt = (char*)aStr;

	switch (aFrmt) {
		case 0: values[1]=&date->tm_mday; values[0]=&date->tm_mon; values[2]=&date->tm_year; break;	//MM-DD-YY
		case 1: values[0]=&date->tm_mday; values[1]=&date->tm_mon; values[2]=&date->tm_year; break;	//DD-MM-YY
		case 2: values[2]=&date->tm_mday; values[1]=&date->tm_mon; values[0]=&date->tm_year; break;	//YY-MM-DD
	}

	//memset( dt, 0, 3*4 );
	dt[0]=pt;
	while ( pt && i<3 ) {
		c = *pt++;
		if ( c < 0x30 ){		// seperator found
			if ( num == 0 ){
				num = Month2Num( dt[i] );
			}
			*values[i] = num;
			num = 0;
			i++;
			dt[i] = pt;
			if ( c == 0 ) pt = 0;
		} else
		if ( c <= 0x39 ){		// process digit
			num = (num*10) + (c-'0');
		}
	}
	if ( i==2 ){			// special MM/YY format
		date->tm_mday = 1;
		date->tm_mon = *values[0];
		date->tm_year = *values[1];
	} else
	if ( i<2)
		return FALSE;

	//year 2000 compliant
	if (date->tm_year<70)
		date->tm_year+=100;
	else
	if (date->tm_year>1000)
		date->tm_year-=1900;
	date->tm_mon--;

	date->tm_sec = 0;
	date->tm_min = 0;
	date->tm_hour = 0;
	date->tm_isdst = 0;

	return TRUE;
} 
/* read 12:44:01 */
time_t StringToDaysTime (const char *aStr, struct tm *date)
{
	char			*p = (char*)aStr;
	register int	hh, mm, ss, i;
	short			sign = 1;			// 1 for positive, 0 for negative
			
	if (p)
	{
		if ( !isdigit( *p ) )
		{
			if (*p == '-')
				sign = 0;	
			p++;
		}
		
		if ( !isdigit( *p ) )
			return 0; // Invalid time

		hh = *p++ & 0xf;
		i = *p++;
		if ( i != ':' ){
			hh = (hh*10) + (i & 0xf);
			p++;
		}

		mm = *p++ & 0xf;
		mm = (mm*10) + (*p++ & 0xf);
		p++;
			
		ss = *p++ & 0xf;
		ss = (ss*10) + (*p++ & 0xf);

		if ( date ) {
			date->tm_sec = ss;
			date->tm_min = mm;
			date->tm_hour = hh;
		}
	}

	if (sign)
		return ((hh * 3600) + (mm * 60) + ss);
	else
		return (-((hh * 3600) + (mm * 60) + ss));
}


// add a fixed DD/MM/YY and HH:SS to the date of jd
time_t CalcNextDays( time_t jd, char *incdate, char *inctime )
{
	long		iDD,iMM,iYY,   iHR=0,iMIN=0;
	time_t		jdate;
	struct tm	date;

	DaysDateToStruct( jd, &date );

	iDD = myatoi( incdate ); if (iDD<0) incdate++;		// HANDLE negative dates
	iMM = myatoi( incdate+3 );if (iMM<0) incdate++;
	iYY = myatoi( incdate+6 );
	iHR = myatoi( inctime ); if (iHR<0) inctime++;	
	iMIN = myatoi( inctime+3 );

	date.tm_year += iYY;
	date.tm_mon += iMM;
	while ( date.tm_mon > 11 ){
		date.tm_mon -= 12;
		date.tm_year++;
	}

	date.tm_isdst = 0;
	Date2Days( &date, &jdate );

	jdate += ( iMIN *60 );
	jdate += ( iHR *60*60 );
	jdate += ( iDD * 86400 );

	return jdate;
}

// ConvDate - calls ConvDateMMDDYear_To_MonDDYear() which convert date from EA date "28/03/94" to USA word date "Mar 28 1994"
char *ConvDate( const char *date )
{
	static char	dateReturned[DATE_SIZE];
	ConvDateMMDDYear_To_MonDDYear( date, dateReturned );
	return dateReturned;
}

// ConvDateFormat - convert various date formats to other formats, takes a string and returns a string
// This function calls the appropiate conversion function.
char *ConvDateFormat( const char *date, short formatFrom, short formatTo )
{
	static char	dateReturned[DATE_SIZE];
	if ( formatFrom == formatTo )
		return (char*)date;

	if ( formatFrom == DATE_MM_DD_YY && formatTo == DATE_Mon_DD_YY )
		ConvDateMMDDYear_To_MonDDYear( date, dateReturned );
	else if ( formatFrom == DATE_Mon_DD_YY && formatTo == DATE_MM_DD_YY )		
		ConvDateMonDDYear_To_MMDDYear( date, dateReturned );
	else
		ConvDateMMDDYear_To_MonDDYear( date, dateReturned );
	return dateReturned;
}

#define TRANSLATE_OFF			0
#define TRANSLATE_FROM_RESOURCE	1
#define TRANSLATE_FROM_LANGFILE	2

// ConvDateFormat - convert various date formats to other formats, takes a string and returns a string
// This function calls the appropiate conversion function.
char *ConvDateFormatResTrans( const char *date, short formatFrom, short formatTo )
{
	static char	dateReturned[DATE_SIZE];
	if ( formatFrom == formatTo )
		return (char*)date;

//	ConvDateMMDDYear_To_MonDDYearResTrans( date, dateReturned );
	//ConvDateMMDDYear_To_MonDDYear_Core( date, dateReturned, TRANSLATE_FROM_RESOURCE );
	ConvDateMMDDYear_To_MonDDYear_Core( date, dateReturned, 1 );
/*	if ( formatFrom == DATE_MM_DD_YY && formatTo == DATE_Mon_DD_YY )
		ConvDateMMDDYear_To_MonDDYearResTrans( date, dateReturned );
	else if ( formatFrom == DATE_Mon_DD_YY && formatTo == DATE_MM_DD_YY )		
		ConvDateMonDDYear_To_MMDDYearResTrans( date, dateReturned );
	else
		ConvDateMMDDYear_To_MonDDYearResTrans( date, dateReturned );*/
	return dateReturned;
}

void ConvDateMMDDYear_To_MonDDYear( const char *date, char *dateReturned )
{
	ConvDateMMDDYear_To_MonDDYear_Core( date, dateReturned, TRANSLATE_OFF );
}

void ConvDateMMDDYear_To_MonDDYearResTrans( const char *date, char *dateReturned )
{
	ConvDateMMDDYear_To_MonDDYear_Core( date, dateReturned, TRANSLATE_FROM_RESOURCE );
}


// ConvDateMMDDYear_To_MonDDYear - convert date from EA date "28/03/94" to USA word date "Mar 28 1994"
// Fixed by Raul, handles years that are 4digits shouldnt be +1900
void ConvDateMMDDYear_To_MonDDYear_Core( const char *date, char *dateReturned, short translate )
{
	long	dd,mm,yy;
	register char 	*buffPtr;

	if (!date )
	{
		dateReturned = 0;
		return;
	}
	
	if ( date[2]=='/' && date[5]=='/' ) {		// check if its "28/12/1994" or "28/12/94" format
		mm = myatoi((char*)date + 0);
		dd = myatoi((char*)date + 3);
		yy = myatoi((char*)date + 6);

		//year 2000 compliant
		if (yy<70)
			yy+=2000;
		else if (yy<1000)
			yy+=1900;

		if ( translate == TRANSLATE_FROM_RESOURCE ) // Translate from resources file, not lang file
			mystrncpy( dateReturned, ReturnString( monthsResIds[mm-1] ), 3 );
		else
			mystrncpy( dateReturned, months+(4*(mm-1)), 3 );

		dateReturned[3] = ' ';
		buffPtr = dateReturned+4;
		TWODIGIT2TXT( dd, buffPtr ); *buffPtr++ = ' ';
		FOURDIGIT2TXT( yy, buffPtr ); *buffPtr++ = 0;
		return;
	} else
	if ( date[2]=='/' && date[5]!='/' ) {		// check if its "12/1994" or "12/94" format
		mm = myatoi((char*)date + 0);
		yy = myatoi((char*)date + 3);

		//year 2000 compliant
		if (yy<70)
			yy+=2000;
		else if (yy<1000)
			yy+=1900;

		if ( translate == TRANSLATE_FROM_RESOURCE ) // Translate from resources file, not lang file
			mystrncpy( dateReturned, ReturnString( monthsResIds[mm-1] ), 3 );
		else
			mystrncpy( dateReturned, months+(4*(mm-1)), 3 );
		dateReturned[3] = ' ';
		buffPtr = dateReturned+4;
		FOURDIGIT2TXT( yy, buffPtr ); *buffPtr++ = 0;
		return;
	} else
		dateReturned = 0;
}

void ConvDateMonDDYear_To_MMDDYear( const char *date, char *dateReturned )
{
	ConvDateMonDDYear_To_MMDDYear_Core( date, dateReturned, TRANSLATE_OFF );
}

void ConvDateMonDDYear_To_MMDDYearResTrans( const char *date, char *dateReturned )
{
	ConvDateMonDDYear_To_MMDDYear_Core( date, dateReturned, TRANSLATE_FROM_RESOURCE );
}

long GetMonthNum( char *monthStr )
{
	long month;
	if ( monthStr[0] == 'J' ) 
	{
		if ( monthStr[1] == 'a' )
			month = 1; // January
		else if ( monthStr[2] == 'n' )
			month = 6; // June
		else
			month = 7; // July
	}
	else if ( monthStr[0] == 'A' ) 
	{
		if ( monthStr[1] == 'p' )
			month = 4; // April
		else
			month = 8; // August
	}
	else if ( monthStr[0] == 'M' ) 
	{
		if ( monthStr[2] == 'r' )
			month = 3; // March
		else
			month = 5; // May
	}
	else if ( monthStr[0] == 'F' ) 
		month = 2; // Feburary
	else if ( monthStr[0] == 'S' ) 
		month = 9; // September
	else if ( monthStr[0] == 'O' ) 
		month = 10; // October
	else if ( monthStr[0] == 'N' ) 
		month = 11; // November
	else
		month = 12; // December
	return month;
}

// ConvDateMonDDYear_To_MMDDYear - convert date from USA word date "Mar 28 1994" to EA date "28/03/94"
void ConvDateMonDDYear_To_MMDDYear_Core( const char *date, char *dateReturned, long translate )
{
	char monthStr[16];
	long day = 0, month = 0, year = 0;

	if ( sscanf( date, "%s %d %d", monthStr, &day, &year ) != 3 )
		mystrcpy( dateReturned, date ); // There were not 3 seperate items in this date, so treat as an errornous date, ie leave it as is

	month = GetMonthNum( monthStr );
	sprintf( dateReturned, "%02d/%02d/%02d", month, day, year );
}

//"Thu Aug  1 10:32:18 1996"
// 012345678901234567890123
// 0         1         2
//
char *ConvLongDate( char *date )
{
	short	monthnum;
	register char 	*buffPtr = date, *p;

	if (!date)
		return 0;

	p = date;

	//MM
	monthnum = Month2Num(p+4);
	TWODIGIT2TXT( monthnum, buffPtr );
	*buffPtr++ = '/';

	//DD
	if ( p[8] == ' ' )
		*buffPtr++ = '0';
	else
		*buffPtr++ = p[8];
	*buffPtr++ = p[9];
	*buffPtr++ = '/';

	//YY
	p+= 20;
	*buffPtr++ = *p++;
	*buffPtr++ = *p++;
	*buffPtr++ = *p++;
	*buffPtr++ = *p++;
	*buffPtr++ = 0;
	return(date);
}





//converts extended date format of the form YYYY-MM-DD to MM/DD/YYYY
//"--98/12/31
char *ConvEDate( const char *date, char *out )
{
	register char *buffPtr = out;
	
	if (!date ) return 0;

	*buffPtr++ = date[5];
	*buffPtr++ = date[6];
	*buffPtr++ = '/';
	*buffPtr++ = date[8];
	*buffPtr++ = date[9];
	*buffPtr++ = '/';
	*buffPtr++ = *date++;
	*buffPtr++ = *date++;
	*buffPtr++ = *date++;
	*buffPtr++ = *date++;
	*buffPtr++ = 0;
	*buffPtr++ = date[2];
	*buffPtr++ = date[3];

	return(out);
}
//converts unformatted date of the form YYMMDD to MM/DD/YYYY
char *ConvUDate( const char *date, char *out )
{
	register char 		*buffPtr = out;
	
	if (!date || !buffPtr ) return 0;

	*buffPtr++ = date[2];
	*buffPtr++ = date[3];
	*buffPtr++ = '/';
	*buffPtr++ = date[4];
	*buffPtr++ = date[5];
	*buffPtr++ = '/';

	if ( *date < '9' ){
		*buffPtr++ = '2';
		*buffPtr++ = '0';
		*buffPtr++ = *date++;
		*buffPtr++ = *date++;
	} else {
		*buffPtr++ = '1';
		*buffPtr++ = '9';
		*buffPtr++ = *date++;
		*buffPtr++ = *date++;
	}	
	return(out);
}


// ConvLDate - convert date from "Fri, 07 Mar 1997" or "28/Mar/1994" and "2/Mar/1994,
// or "28/Mar/94" or "3/28/94" to "03/28/94" also scans international dates
// need to handle "080Sep/1999" currupt format
char *ConvLDate( char *buff, char *out )
{
	short	i=0,monthnum, yearnum, daynum,num=0;
	short	*values[3];
	char	*pt=buff,*dt[5], swapdm=0;
	char 	*buffPtr, c;
	
	if (!buff || !out )
		return 0;

	values[0] = &monthnum;
	values[1] = &daynum;
	values[2] = &yearnum;

	if ( pt[3] == '.' ) pt+=5;
	dt[0]=pt;
	while ( pt && i<3 ) {
		c = *pt++;
		if ( c < 0x30 || c == 0x3A ){		// seperator found
			if ( num == 0 ){
				num = Month2Num( dt[i] );
				if ( i == 1 ) swapdm = 1;
			}
			*values[i] = num;
			num = 0;
			i++;
			dt[i] = pt;
			if ( c == 0 ) pt = 0;
		} else
		if ( c <= 0x39 ){		// process digit
			num = (num*10) + (c-'0');
		}
	}
	if (i<3) return 0;
	
	if (!daynum) { //must be alpha & really the month
		if (!monthnum)
			return 0;
	}
	//if it is the extended date format
	//YYYY-MM-DD
	if (monthnum>31) {
		i=yearnum;
		yearnum=monthnum;
		monthnum=daynum;
		daynum=i;
	} else
	if ( swapdm ) { //must be alpha & really the month
		i = daynum;
		daynum = monthnum;
		monthnum = i;
	}

	//year 2000 compliant
	if (yearnum<70)
		yearnum+=2000;
	else if (yearnum<1000)
		yearnum+=1900;

	buffPtr = out;
	TWODIGIT2TXT( monthnum, buffPtr );
	*buffPtr++ = '/';
	TWODIGIT2TXT( daynum, buffPtr );
	*buffPtr++ = '/';
	FOURDIGIT2TXT( yearnum, buffPtr );
	*buffPtr++ = 0;

	return(out);
}

// input = "MM/DD/YY"
// 2/7/98
// 2/07/98  = "DD/MM/YY"
// or 01-Apr-99
// 
// AUTO,DD/MM/YY,MM/DD/YY,YY/MM/DD
char *ConvFloatingDate( char *date, char *out, short forceddmm )
{
	static char	swapdm = 0;
	short 	monthnum, yearnum, daynum, i=0, num = 0;
	short	*values[3];
	register char 		*buffPtr, *pt=date, c;
	char	*dt[5];

	if (!date ) { swapdm = FALSE; return 0; }
	switch( forceddmm ){
		case 3: values[0] = &yearnum;	values[1] = &monthnum; values[2] = &daynum;break;
		case 1: values[0] = &daynum;	values[1] = &monthnum; values[2] = &yearnum;break;
		default:
		case 2:
		case 0: values[0] = &monthnum;	values[1] = &daynum; values[2] = &yearnum;break;
	}

	//memset( dt, 0, 3*4 );
	dt[0]=pt;
	while ( pt && i<3 ) {
		c = *pt++;
		if ( c < 0x30 || c == 0x3A ){		// seperator found
			if ( num == 0 ){
				num = Month2Num( dt[i] );
				if ( i == 1 ) swapdm = 1;
			}
			*values[i] = num;
			num = 0;
			i++;
			dt[i] = pt;
			if ( c == 0 ) pt = 0;
		} else
		if ( c <= 0x39 ){		// process digit
			num = (num*10) + (c-'0');
		}
	}
	
	if ( !forceddmm ){
		//if it is the extended date format
		//YYYY-MM-DD or YY-MM-DD
		if (monthnum>31) {
			i=yearnum;
			yearnum=monthnum;
			monthnum=daynum;
			daynum=i;
		} else
		// if its 2/06/98 then the 06 is a month not a day
		// if its 13/6/98 then the 13 is a day and 6 is a month
		if ( swapdm || monthnum>12 || (*dt[1]=='0' && *dt[0]!='0') ) {
			if ( daynum<= 12 ){
				i = daynum;
				daynum = monthnum;	
				monthnum = i;
				swapdm = 1;
			}
		}
	}

	//year 2000 compliant
	if (yearnum<70)
		yearnum+=2000;
	else if (yearnum<1000)
		yearnum+=1900;

	buffPtr = out;
	TWODIGIT2TXT( monthnum, buffPtr );
	*buffPtr++ = '/';
	TWODIGIT2TXT( daynum, buffPtr );
	*buffPtr++ = '/';
	FOURDIGIT2TXT( yearnum, buffPtr );
	*buffPtr++ = 0;

	return(out);
}








// converts 123015 time to 12:30:15 time
char *ConvUTime( const char *time )
{
	static char	buff[12];
	register char *p = buff;
	
	if (time){
		*p++ = *time++;
		*p++ = *time++;
		*p++ = ':';
		*p++ = *time++;
		*p++ = *time++;
		*p++ = ':';
		*p++ = *time++;
		*p++ = *time++;
		*p++ = 0;
		return buff;
	} else
		return 0;
}

// converts an AM/PM time of "1:40 PM" or "1:40:44 PM" into 24 hour time "13:40:44"
char *ConvTime( const char *time )
{
	register char *p = (char*)time, c, *d;
	static char	buff[12];
	short		hh,i=1;
	
	d = buff+2;
	hh = (*p++);
	c = *p++;
	if ( c <= 0x39 )
		hh = hh<<8 | c;
	else
		hh = hh | 0x3000;

	while( c ){
		c = *p++;
		if ( (c == 'P' || c == 'p') && hh != 0x3132 ){
			hh += 0x0102;
			c = 0;
		} else
		if ( (c == 'A' || c == 'a') && hh == 0x3132 ){
			hh = 0x3030;
			c = 0;
		}
		if ( c != ' ' )
			*d++ = c;
	}
	if ( *p != 'M' )
		return (char*)time;
	d = buff;
	*d++ = hh>>8;
	*d++ = hh&0xff;
	return(buff);
}

/* ConvertOldDateFormatToTimeT
	- takes a date string from pre-4.5 settings file (which may be in any international 
		format and converts to time_t
*/

void ConvertOldDateFormatToTimeT (const char *oldDate, time_t *outTimeT)
{
	char		fwDateStr[20], tempStr[20];
	struct tm	tmDate;	

	ConvFloatingDate ((char *)oldDate, tempStr, (short)false);
	StringToDaysDate (tempStr, &tmDate, DATE_MM_DD_YY);
	StructToYYYYMMDDDate (&tmDate, fwDateStr);
	ConvertFWDateStringToTimeT (fwDateStr, outTimeT);
}

/* ConvertFWDateStringToTimeT
	- takes an internal string, always in YYYY/MM/DD format, and converts to time_t
	- use instead of TimeStringToDays to convert settings start/end dates
*/

void ConvertFWDateStringToTimeT (const char *fwDateStr, time_t *outTimeT)
{
	struct tm	date1;

	if (StringToDaysDate (fwDateStr, &date1, DATE_YY_MM_DD))
	{
		Date2Days (&date1, outTimeT);
	}
}

/* ConvertTimeTToFWDateString
	- takes time_t in settings structure and formats for output as YYYY/MM/DD
*/

void ConvertTimeTToFWDateString (time_t inTimeT, char *outDateString)
{
	struct tm	*tempTm;
	
	tempTm = mygmtime ((const unsigned long *)&inTimeT);
	StructToYYYYMMDDDate (tempTm, outDateString);
}

void StructToTimeT (struct tm inDate, time_t *outTimeT)
{
	char	fwDateStr[20];
	
	StructToYYYYMMDDDate (&inDate, fwDateStr);
	ConvertFWDateStringToTimeT (fwDateStr, outTimeT);

}

void TimeStringToDays( const char* aStr, time_t *jd )
{
	struct tm date1;
	short	dFmt;

	dFmt = GetDateStyle();

	InitFast10000();

	if (StringToDaysDate( aStr, &date1, dFmt ) ) {
		Date2Days( &date1, jd );
	} else {
		if ( !strcmpd( "today", aStr ) ) {
			char *p;
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			// check if the date is "today-4"
			if ( p = strchr( aStr, '-' ) )
			{
				long offset = myatoi( p+1 );
				*jd -= (offset*ONEDAY);			
			}
		} else
		if ( !strcmpd( "endoftoday", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd += ONEDAY;
		} else
		if ( !mystrcmpi( "last24hrs", aStr ) ) {
			GetCurrentDate( &date1 );
			Date2Days( &date1, jd );
			*jd -= ONEDAY;
		} else
		if ( !mystrcmpi( "yesterday", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= ONEDAY;
		} else
		if ( !mystrcmpi( "thisweek", aStr ) ) {
			int wdayAadjusted;	// day of week adjusted according to user's first day of week setting

			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );

			// offset the day of week according to user's first day of week setting so that "thisweek"
			// takes that setting into consideration
			wdayAadjusted=(date1.tm_wday+7-MyPrefStruct.firstDayOfWeek)%7;

			*jd -= (wdayAadjusted*ONEDAY);
		} else
		if ( !mystrcmpi( "prevweek", aStr ) ) {
			int wdayAadjusted;	// day of week adjusted according to user's first day of week setting

			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );

			// offset the day of week according to user's first day of week setting so that "prevweek"
			// takes that setting into consideration
			wdayAadjusted=(date1.tm_wday+7-MyPrefStruct.firstDayOfWeek)%7;

			*jd -= (wdayAadjusted+7)*ONEDAY;
		} else
		if ( !mystrcmpi( "thismonth", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mday = 1;
			Date2Days( &date1, jd );
		} else
		if ( !mystrcmpi( "prevmonth", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mday = 1;
			date1.tm_mon -= 1;
			if ( date1.tm_mon < 0 ) {
				date1.tm_mon +=12;
				date1.tm_year -=1;
			}
			Date2Days( &date1, jd );
		} else
		if ( !mystrcmpi( "last7days", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= (6*ONEDAY);
		} else
		if ( !mystrcmpi( "last14days", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= (13*ONEDAY);
		} else
		if ( !mystrcmpi( "last21days", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= (20*ONEDAY);
		} else
		if ( !mystrcmpi( "last28days", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= (27*ONEDAY);
		} else
		if ( !mystrcmpi( "last60days", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= (59*ONEDAY);
		} else
		if ( !mystrcmpi( "prev7days", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= (7*ONEDAY);
		} else
		if ( !mystrcmpi( "prev14days", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= (14*ONEDAY);
		} else
		if ( !mystrcmpi( "prev21days", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= (21*ONEDAY);
		} else
		if ( !mystrcmpi( "prev28days", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			Date2Days( &date1, jd );
			*jd -= (28*ONEDAY);
		} else
		if ( !mystrcmpi( "thisquart", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mon = ((date1.tm_mon)/3) * 3;
			date1.tm_mday = 1;
			Date2Days( &date1, jd );
		} else
		if ( !mystrcmpi( "prevquart", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mon = ((date1.tm_mon)/3) * 3;
			date1.tm_mon -= 3;
			date1.tm_mday = 1;
			if ( date1.tm_mon < 0 ){
				date1.tm_mon +=12;
				date1.tm_year -=1;
			}
			Date2Days( &date1, jd );
		} else
		if ( !mystrcmpi( "this6m", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mon = ((date1.tm_mon)/6) * 6;
			date1.tm_mday = 1;
			Date2Days( &date1, jd );
		} else
		if ( !mystrcmpi( "prev6m", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mon = ((date1.tm_mon)/6) * 6;
			date1.tm_mon -= 6;
			date1.tm_mday = 1;
			if ( date1.tm_mon < 0 ){
				date1.tm_mon +=12;
				date1.tm_year -=1;
			}
			Date2Days( &date1, jd );
		} else
		if ( !mystrcmpi( "thisyear", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mon = 0;
			date1.tm_mday = 1;
			Date2Days( &date1, jd );
		} else
		if ( !mystrcmpi( "prevyear", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mon = 0;
			date1.tm_mday = 1;
			date1.tm_year -=1;
			Date2Days( &date1, jd );
		} else
		if ( !mystrcmpi( "begin", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mon = 0;
			date1.tm_mday = 1;
			date1.tm_year = 90;
			Date2Days( &date1, jd );
		} else
		if ( !mystrcmpi( "end", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mon = 0;
			date1.tm_mday = 1;
			date1.tm_year++;
			Date2Days( &date1, jd );
		} else 
		if ( !mystrcmpi( "month", aStr ) ) {
			GetCurrentDate( &date1 );
			date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
			date1.tm_mon = atoi( aStr+5 ) - 1;
			date1.tm_mday = 1;
			Date2Days( &date1, jd );
		} else
		{
			char *p; long mon = 0; char tmp[8];
			GetCurrentDate( &date1 );

			// check for Jan/Feb etc.. keywords
			if ( *aStr ){
				p = months;
				while( p && mon<12 ){
					mystrncpy( tmp, p, 3 );
					tmp[3] = 0;
					if ( !mystrcmpi( tmp, aStr ) ) {
						date1.tm_hour = date1.tm_min = date1.tm_sec = 0;
						date1.tm_mon = mon;	date1.tm_mday = 1;
						p = 0;
						continue;
					}
					p+=4;
					mon++;
				} // else if none found then leave it at the current date
			}

			Date2Days( &date1, jd );
		}
	}
}




/* convert the dateStyle to the actual date
   This is the older scheme, its IDS are fixed.
 */

void DatetypeToDate( time_t dateStyle, time_t *date1, time_t *date2 )
{
	time_t		daterange1,daterange2;

	TimeStringToDays( "today", &daterange2 );
	daterange1 = daterange2;

	switch( dateStyle ){
		case DATE_ALL:		TimeStringToDays( "begin", &daterange1 );			// All Dates
							TimeStringToDays( "end", &daterange2 );
							break;
		case DATE_TODAY:	TimeStringToDays( "today", &daterange1 );
							break;
		case DATE_YESTERDAY:TimeStringToDays( "yesterday", &daterange1 );
							TimeStringToDays( "yesterday", &daterange2 );
							break;
		case DATE_THISWEEK:	TimeStringToDays( "thisweek", &daterange1 );
							break;
		case DATE_PREVWEEK:	TimeStringToDays( "prevweek", &daterange1 );
							TimeStringToDays( "thisweek", &daterange2 ); daterange2-=ONEDAY;
							break;

		case DATE_LAST7:	TimeStringToDays( "last7days", &daterange1 );
							TimeStringToDays( "today", &daterange2 );
							break;
		case DATE_LAST14:	TimeStringToDays( "last14days", &daterange1 );
							TimeStringToDays( "today", &daterange2 );
							break;
		case DATE_LAST21:	TimeStringToDays( "last21days", &daterange1 );
							TimeStringToDays( "today", &daterange2 );
							break;
		case DATE_LAST28:	TimeStringToDays( "last28days", &daterange1 );
							TimeStringToDays( "today", &daterange2 );
							break;

		case DATE_PREV7:	TimeStringToDays( "prev7days", &daterange1 );
							TimeStringToDays( "yesterday", &daterange2 );
							break;
		case DATE_PREV14:	TimeStringToDays( "prev14days", &daterange1 );
							TimeStringToDays( "yesterday", &daterange2 );
							break;
		case DATE_PREV21:	TimeStringToDays( "prev21days", &daterange1 );
							TimeStringToDays( "yesterday", &daterange2 );
							break;
		case DATE_PREV28:	TimeStringToDays( "prev28days", &daterange1 );
							TimeStringToDays( "yesterday", &daterange2 );
							break;



		case DATE_THISMONTH:TimeStringToDays( "thismonth", &daterange1 );
							break;
		case DATE_PREVMONTH:TimeStringToDays( "prevmonth", &daterange1 );
							TimeStringToDays( "thismonth", &daterange2 ); daterange2-=ONEDAY;
							break;
		case DATE_THISQUART:TimeStringToDays( "thisquart", &daterange1 );
							break;
		case DATE_PREVQUART:TimeStringToDays( "prevquart", &daterange1 );
							TimeStringToDays( "thisquart", &daterange2 ); daterange2-=ONEDAY;
							break;
		case DATE_THIS6M:	TimeStringToDays( "this6m", &daterange1 );
							break;
		case DATE_PREV6M:	TimeStringToDays( "prev6m", &daterange1 );
							TimeStringToDays( "this6m", &daterange2 ); daterange2-=ONEDAY;
							break;
		case DATE_THISYEAR:	TimeStringToDays( "thisyear", &daterange1 );
							break;
		case DATE_PREVYEAR:	TimeStringToDays( "prevyear", &daterange1 );
							TimeStringToDays( "thisyear", &daterange2 ); daterange2-=ONEDAY;
							break;
		case DATE_JAN:		TimeStringToDays( "jan", &daterange1 ); TimeStringToDays( "feb", &daterange2 ); daterange2-=1; break;
		case DATE_FEB:		TimeStringToDays( "feb", &daterange1 ); TimeStringToDays( "mar", &daterange2 ); daterange2-=1; break;
		case DATE_MAR:		TimeStringToDays( "mar", &daterange1 ); TimeStringToDays( "apr", &daterange2 ); daterange2-=1; break;
		case DATE_APR:		TimeStringToDays( "apr", &daterange1 ); TimeStringToDays( "may", &daterange2 ); daterange2-=1; break;
		case DATE_MAY:		TimeStringToDays( "may", &daterange1 ); TimeStringToDays( "jun", &daterange2 ); daterange2-=1; break;
		case DATE_JUN:		TimeStringToDays( "jun", &daterange1 ); TimeStringToDays( "jul", &daterange2 ); daterange2-=1; break;
		case DATE_JUL:		TimeStringToDays( "jul", &daterange1 ); TimeStringToDays( "aug", &daterange2 ); daterange2-=1; break;
		case DATE_AUG:		TimeStringToDays( "aug", &daterange1 ); TimeStringToDays( "sep", &daterange2 ); daterange2-=1; break;
		case DATE_SEP:		TimeStringToDays( "sep", &daterange1 ); TimeStringToDays( "oct", &daterange2 ); daterange2-=1; break;
		case DATE_OCT:		TimeStringToDays( "oct", &daterange1 ); TimeStringToDays( "nov", &daterange2 ); daterange2-=1; break;
		case DATE_NOV:		TimeStringToDays( "nov", &daterange1 ); TimeStringToDays( "dec", &daterange2 ); daterange2-=1; break;
		case DATE_DEC:		TimeStringToDays( "dec", &daterange1 ); TimeStringToDays( "end", &daterange2 ); daterange2-=1; break;
	};

	*date1 = daterange1;
	*date2 = daterange2;
}


// ***********************************************************
// ** Madeby Raul Sobon
// ***********************************************************
// ** decode the string id of the daterange, into two time_t dates
void DecodeDateRangeString( const char *rangeId, time_t *startDate, time_t *endDate )
{
	time_t		daterange1,daterange2;

	if ( !strcmpd( "all", rangeId ) )
	{
		TimeStringToDays( "begin", &daterange1 );			// All Dates
		TimeStringToDays( "end", &daterange2 );
	} else
	if ( !strcmpd( "this", rangeId ) )
	{
		TimeStringToDays( rangeId, &daterange1 );
		TimeStringToDays( "today", &daterange2 );
	} else
	if ( !strcmpd( "prev", rangeId ) )
	{
		char newId[64];
		TimeStringToDays( rangeId, &daterange1 );
		strcpy( newId, rangeId );
		strncpy( newId, "this", 4 );
		TimeStringToDays( rangeId, &daterange2 );
		daterange2-=1;
	} else
	if ( !strcmpd( "last", rangeId ) )
	{
		TimeStringToDays( rangeId, &daterange1 );
		TimeStringToDays( "today", &daterange2 );
	} else
	if ( !strcmpd( "month", rangeId ) )
	{
		TimeStringToDays( rangeId, &daterange1 );
		daterange2 = CalcNextDays( daterange1, "00/01/00", "00:00:00" ) - 1;
	} else
	{
		TimeStringToDays( rangeId, &daterange1 );
		TimeStringToDays( rangeId, &daterange2 );
	}

	*startDate = daterange1;
	*endDate = daterange2;
}

// This has to be at least the same order as the GUI POPUP, (ie, the IDS_ stuff)
static char *ranges[] = {
		"",
		"all",
		"today",
		"yesterday",
		"thisweek",
		"prevweek",
		"last7days",
		"last14days",
		"last21days",
		"last28days",

		"thismonth",
		"prevmonth",
		"thisquart",
		"prevquart",
		"this6m",
		"prev6m",
		"thisyear",
		"prevyear",
		"month1",
		"month2",
		"month3",
		"month4",
		"month5",
		"month6",
		"month7",
		"month8",
		"month9",
		"month10",
		"month11",
		"month12", 
		
		"prev7days",
		"prev14days",
		"prev21days",
		"prev28days",

		0,0,0,0 };


// For X given ID, return the string id, and set the date1/date2 times.
char * DecodeDateRangeID( long dateStyle, time_t *date1, time_t *date2 )
{
	if ( dateStyle >= 0 && dateStyle < sizeof(ranges) )
	{
		if ( date1 && date2 )
		{
			DecodeDateRangeString( ranges[ dateStyle ] , date1, date2 );
		}
		return ranges[ dateStyle ];
	} else
		return ranges[ 0 ];
}

/***************************************************
 Convert a daterange String into the internal ID
 */
long ReturnIDFromDateRange( char *dateRangeStr )
{
	long i = 0;

	while( ranges[i] )
	{
		if ( !strcmpd( dateRangeStr, ranges[i] ) )
			return i;
		i++;
	}
	return -1;
}