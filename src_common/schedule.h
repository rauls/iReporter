#ifndef	__SCHEDULE
#define	__SCHEDULE

#include "Compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "datetime.h"

#define	MAXLOGLENGTH	8000	

#define	SCH_STRINGMAX	260

typedef	struct ScheduleStruct {
	int		active;
	int		runonce;
	/*long*/ time_t	startdate;
	double	repeattime;
	long	nextRun;
	long	repeatCount;
	long	repeatType;
	char	logfile[MAXLOGLENGTH];
	char	title[SCH_STRINGMAX];
	char	prefsfile[SCH_STRINGMAX];
	char	upload[SCH_STRINGMAX];
	long	index;
	struct ScheduleStruct	*next;
} ScheduleRec, *ScheduleRecPtr;

#if DEF_UNIX
#define	SCHEDULE_FILENAME			".def_sched.txt"
#define	SCHEDULE_NEWFILENAME		".schedule.txt"
#else
#define	SCHEDULE_FILENAME			"def_sched.txt"
#define	SCHEDULE_NEWFILENAME		"schedule.txt"
#endif

#ifdef DEF_MAC
	#define kMacScheduleFile		"Schedule"
	OSErr GetPreferencesPath (char *prefsPath);
#endif

#define	SCHEDULE_LOGFILENAME		"schedlog.txt"
#define	SCHEDULESIZE		sizeof( ScheduleRec )

void ActivateAllSchedules( long value );

void LogScheduleTxt( const char *txt, long err );
long GetScheduleFiledate( void );

long RunScheduleX( long schedulenumber );

long CalcNextSchedule( long jd, long repeatCount, long repeatType );
void DelSchedule( int n );
long GetTotalSchedule( void );
ScheduleRecPtr AddSchedule( void );
ScheduleRecPtr GetSchedule( int n );

long CheckSchedule( ScheduleRecPtr schP, long currentTime, int );
void CopySchedule( ScheduleRecPtr newP, int from );
long ComputeNextSchedule( ScheduleRecPtr schP );

long RunScheduleEvent( ScheduleRecPtr schP, int );


long SaveScheduleAsText( ScheduleRecPtr schP, char *filename );
void ReadScheduleAsText( char *filename, int  );
void ClearSchedule( ScheduleRecPtr schP );
void ForceCheckSchedule( void );
void ServiceCheckSchedule( void );
void InitReadSchedule( long calltype);

long SaveSchedule( void );

#ifdef __cplusplus
}
#endif

#endif
