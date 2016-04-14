

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char *months;
extern char *weekdays[];

time_t UnixTimetoCTime( const char *utStr );
time_t JDToCTime( double jd );

void InitFast10000( void );

short GetDateStyle( void );
char Month2Num( const char *p );

void ClearLoadWeekdaysAndMonths();
char *GetWeekday( time_t timed );
char *GetWeekdayTranslated( time_t timed );
char *GetWeekdayTranslatedByDayNum( int dayOfWeek );
char *GetMonthTranslated( time_t timed );
char *GetMonthTranslatedByMonthNum( int monthOfYear );
long GetCurrentWeek( struct tm *t );
void DatetypeToDate( time_t dateStyle, time_t *date1, time_t *date2 );


void StructToUSDate( struct tm *date, char *p );
void StructToYYYYMMDDDate (struct tm *date, char *p);
time_t DateStringToCTime( const char *date );		//convert DD/MM/YY into a JD time
time_t Time2CTime( void *datePtr, time_t *time );
time_t Date2Days( struct tm *date, time_t *aDbl );
int StringToDaysDate( const char * aStr, struct tm *date, short aFrmt );
time_t StringToDaysTime( const char * aStr, struct tm *date ); 
char *DaysDateToString( time_t jd, char *aStr,short aFrmt,char aDelim,int aLdZ,int aCent);
char *DaysDateToStringTranslated( time_t ct, char *aStr, short bufS, short aFrmt,char aDelim,int aLdZ,int aCent);
char *DaysTimeToString(time_t aDay,char *aStr, int aLdZ, int a12Hr, int aSeconds);
void DateTimeToString(time_t jd, char *aStr);
void DateTimeToStringTranslated( time_t ctime, char *aStr );
void DateTimeDurationToString( time_t startTime, time_t endTime, short days, char *aStr );
void DaysDateToStruct(time_t jd, struct tm *data );

// new for 4.5, handle settings date strings in/out
void ConvertOldDateFormatToTimeT (const char *oldDate, time_t *outTimeT);
void ConvertFWDateStringToTimeT (const char *fwDateStr, time_t *outTimeT);
void StructToTimeT (struct tm inDate, time_t *outTimeT);
void ConvertTimeTToFWDateString (time_t inTimeT, char *outDateString);

void TimeStringToDays( const char *aStr, time_t *jd );
void GetCurrentDate( struct tm *thisTimeData );
void CTimetoString( time_t ct, char *outStr );
time_t GetCurrentCTime( void );
void CTimeToDateTimeStr( time_t ct, char *outstr );

char *ConvTime( const char *time );
char *ConvUTime( const char *time );
char *ConvDate( const char *date );
char *ConvDateFormat( const char *date, short formatFrom, short formatTo );
char *ConvDateFormatResTrans( const char *date, short formatFrom, short formatTo );
void ConvDateMMDDYear_To_MonDDYear( const char *date, char *dateReturned );
void ConvDateMMDDYear_To_MonDDYearResTrans( const char *date, char *dateReturned );
void ConvDateMMDDYear_To_MonDDYear_Core( const char *date, char *dateReturned, short translate );
void ConvDateMonDDYear_To_MMDDYear( const char *date, char *dateReturned );
void ConvDateMonDDYear_To_MMDDYearResTrans( const char *date, char *dateReturned );
void ConvDateMonDDYear_To_MMDDYear_Core( const char *date, char *dateReturned, long translate );

char *ConvFloatingDate(char *date, char *out, short forceddmm );
char *ConvUDate( const char *date, char *out );
char *ConvEDate( const char *date,char *out );
char *ConvLDate( char *date, char *out );

char *ConvLongDate(char *date);

time_t CalcNextDays( time_t jd, char *incdate, char *inctime );


char * DecodeDateRangeID( long dateStyle, time_t *date1, time_t *date2 );
long ReturnIDFromDateRange( char *dateRangeStr );

// These don't appear to be used in Mac or Win, how about Unix?
// If not, they can be deleted
//#define JDToDateTimeStr	CTimeToDateTimeStr
//#define DateStringToJD	DateStringToCTime

#define	ONEMINUTE	(60)
#define	ONEHOUR		(ONEMINUTE*60)
#define	ONEDAY		(ONEHOUR*24)
#define	ONEWEEK		(ONEDAY*7)
#define	ONEMONTH	(ONEDAY*31)

#define	JSEC		(1/86400.0)
#define	JMINUTE		(60/86400.0)
#define	JHOUR		((60*60)/86400.0)
#define	JDAY		(1.0)
#define	JWEEK		(JDAY*7)
#define	JMONTH		(JDAY*31)

#define	JSECS(x)	((x)/86400.0)
#define	JMINUTES(x)	((x*60)/86400.0)
#define	JHOURS(x)	((x*3600)/86400.0)

#define DATE_SIZE	32

#define	DATE_SPECIFY		0
#define	DATE_ALL			1
#define	DATE_TODAY			2
#define	DATE_YESTERDAY		3
#define	DATE_THISWEEK		4
#define	DATE_PREVWEEK		5
#define	DATE_LAST7			6
#define	DATE_LAST14			7
#define	DATE_LAST21			8
#define	DATE_LAST28			9
#define	DATE_THISMONTH		10
#define	DATE_PREVMONTH		11
#define	DATE_THISQUART		12
#define	DATE_PREVQUART		13
#define	DATE_THIS6M			14
#define	DATE_PREV6M			15
#define	DATE_THISYEAR		16
#define	DATE_PREVYEAR		17

#define	DATE_JAN			18
#define	DATE_FEB			19
#define	DATE_MAR			20
#define	DATE_APR			21
#define	DATE_MAY			22
#define	DATE_JUN			23
#define	DATE_JUL			24
#define	DATE_AUG			25
#define	DATE_SEP			26
#define	DATE_OCT			27
#define	DATE_NOV			28
#define	DATE_DEC			29

#define	DATE_PREV7			30
#define	DATE_PREV14			31
#define	DATE_PREV21			32
#define	DATE_PREV28			33


#define DATE_MM_DD_YY		0
#define DATE_DD_MM_YY		1
#define DATE_YY_MM_DD		2
#define DATE_Mon_DD_YY		3

#ifdef __cplusplus
}
#endif
