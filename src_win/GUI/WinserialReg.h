
#ifndef	__WINSERIALREG_H
#define	__WINSERIALREG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "serialreg.h"

void OpenSerialDlg( SerialInfoPtr userInfo );

SerialInfoH	GetSerialInfoData( SerialInfoH Data, char *regStr );
int	SaveSerialData(SerialInfoH userInfo, char *name, char *regStr);

long GetReg_Prefs( char *id, char *ptr );
long GetReg_PrefsNumber( char *id, long *val );
void SetReg_Prefs( char *id, char *ptr );
void SetReg_PrefsNumber( char *id, long val );

long GetReg_PrefsFile( char *ptr );
void SetReg_PrefsFile( char *ptr );
short	Fetch_SerialInfo( SerialInfoPtr userInfo );

int GetLocalHostName( char *name, long len );

void ShowRegMessage( void );
void ShowMsg_TrialOver( void );
void ShowMsg_FreeVersionExpired( void );
void ShowMsg_ReEnterSerial( char *badSerialStr );


#ifdef __cplusplus
}
#endif

#endif
