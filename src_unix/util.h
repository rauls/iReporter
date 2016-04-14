
#ifndef	__UTIL_H
#define __UTIL_H


#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "GlobalPaths.h"


#define		DEFAULT_LOG_NAME		"test.log"
#define		DEFAULT_PREF_NAME		"test.report"
#define		MSG_ID		(0xb007)
             
extern int			gNoGUI;
extern int			gProcessLog;		
extern int			gProcessing ;
extern int			gProcessingSchedule;
extern int			gRemoteControlled;
extern int			gSaved;
extern int			gConvertLogToW3C_Flag;

#define	THREAD_PRIORITY_BELOW_NORMAL	-1

typedef	void * HANDLE;

void *CreateThread( void *, long stack, int func(long), void *data, long x, unsigned long *id );
void *ExitThread( long ret );
void *CloseHandle( void *h );
void SetThreadPriority( void *ptr, long pri );
void TerminateThread( void *ptr, long pri );

long GetLastPercentProgress( void );
void ShowProgressDetail( long level, long forceShow, char *msg, double timesofar );
void ShowDNSProgress( long level, long forceShow, char *msg, double start );
#define ShowProgress( percent, forceShow, msg )		ShowProgressDetail( percent*10,  forceShow, msg, 0 )

int StatusSetID( int id, ... );

                
void SetupArgs( int argc, char **argv );
long InitApp(void);
void ExitApp(void);

char *GetOutputFile( void );

void StatusWindowSetText( char *txt );
void StatusWindowXSetText( char *txt,  void *statWnd );
int CreateProgressBar( long totalVal) ;
int UpdateProgressBar( long percent );
int DeleteProgressBar( void ); 
long LogMessage ( char *file, char *str, long );
FILE *GetFile(void);
short ProcessLog( long );
void ProcessLogNow( void );
void DoProcessLogQ( long viewreport );
void Preferences(void);
void DatetypeToDate( long dateStyle, long *date1, long *date2 );
long AddWildCardFilenames( char *szTempFile , long start );
long AddWildCardReportFiles( char *szTempFile );

void EnableCron( long status );

int MsgBox_Error( long id, ... );

void ShowMsg_TrialOver( void );
void ShowMsg_FreeVersionExpired( void );
void ShowMsg_ReEnterSerial( char *badSerialStr );

long CheckReportFreeSpace( void );

#define LIGHTGRAY               RGB(192, 192, 192)
#define DARKGRAY                RGB(128, 128, 128)

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif
  






#endif
