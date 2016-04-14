/*
Windows App main.h
*/
#ifndef	__MAIN_H
#define __MAIN_H


#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <alloca.h>
#include "config.h"
#include "util.h"
#include "serialReg.h"
#include "ResDefs.h"

#define closesocket(x)	close(x)

void StatusSet( char* );
int StatusSetf( char *msg, ... );
void StatusWindowSetProgress( long perc, char *txt );
void ProgressChangeMainWindowTitle( long percent, char *etaStr );
void ChangeMainWindowTitle( void );

long UnixErrorMsg( const char *txt );
long UnixCautionMsg( const char *txt );
long UnixNotifyMsg( const char *txt );


void Preferences(void);
void ShowFile( char *file );

long AddLogToGUIHistory( char *, long item );

short Fetch_SerialInfo( SerialInfoPtr SerialDataP );
void ShowVersionInfo( void );
#define	Sleep(x)	usleep(x)

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif
  


#endif





