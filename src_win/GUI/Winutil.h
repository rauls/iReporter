#ifndef	__WINUTIL_H
#define __WINUTIL_H

#ifdef __cplusplus
extern "C" {
#endif


#include "config_struct.h"	
//local declarations

long WinNotifyMsg( const char *txt );
long WinCautionMsg( const char *txt );
long WinErrorMsg( const char *txt );


void GetApplicationDataPath( char *out );
void GetPersonalDataPath( char *out );

int Convert_Temp_Image(char * lpszFilename, char type);

void GetControlRect( HWND hwnd, HWND objectWnd, RECT *rc );
#define LIST_VIEW_SEP "," // Standard ListView seperator, can be different for different lists, but comma is suffcient for most lists 
void AddItemToListView( HWND hwin, int index, long columns, char* sepData, char *sep );
long ChooseRGBColor( HWND hWnd, DWORD rgbCurrent );

long AddWildCardFilenames( char * , long );
long AddWildCardReportFiles( const char *szTempFile );


BOOL GetLogFileName(char *putNamehere, char *initDir, long max );
long GetLogFileNames( char *initDir, char * out );
BOOL GetOpenPrefsName( char *putNamehere );
BOOL GetSaveDialog( char *putNamehere, char *defaultfile, char *dir, char *filter, char *title, char *defaultExt, long );
BOOL GetSavePrefsName( char *putNamehere );
BOOL GetSaveDomainName( char *putNamehere );
int GetFile( char *fileName, char *initDir, long max );
int GetFiles( char **fileName, char *initDir );

void SetSaveFolder( char *fullName );
BOOL GetSaveFolderName( char *putNamehere );

BOOL GetOpenSkinsName( char *putNamehere );
 
int AddMultiFilesToHistory( char *src );

long IsPrefsFile(char *prefsname);

short Dialog_SelectLogs( void );


short OpenLog( char *putnamehere, long max );
short ProcessLog( long );
short OpenAndProcessLog( void );
void Preferences(void);

void GetObjectRect( HWND hwnd, HWND objectWnd, RECT *rc );
void GetObjectCoord( HWND hwnd, HWND objectWnd, RECT *rc );

void SelectListViewItem( HWND hwin, long index );
long CheckReportFreeSpace( void );

long CheckforUpdate( void );

BOOL SelectFolder( char *out, char *initDir, char *title );

BOOL GetSaveLogListName( char *putNamehere );
BOOL GetOpenLogListName( char *putNamehere );

#ifdef __cplusplus
}
#endif

#endif




/*

|| Speed improvement specs/details for processing...

OLD---
short ProcessLogLine(char *buffer, char **date, char **time, char **status, char **host, char **file,
 char **agent, double *bytes, char **refer, char **user, char **vhost );

NEW---
short ProcessLogLine(char *buffer, HitDataPtr );

  
	
typedef struct HitData {
	short	dateLen;
	short	timeLen;
	short	statusLen;
	short	hostLen;
	short	fileLen;
	short	agentLen;
	short	bytesLen;
	short	bytesInLen;
	short	referLen;
	short	userLen;
	short	vhostLen;
	char	*date;
	char	*time;
	char	*status;
	char	*host;
	char	*file;
	char	*agent;
	char	*bytes;
	char	*bytesIn;
	char	*refer;
	char	*user;
	char	*vhost;
} HitDataRec, *HitDataPtr;


#ifdef DEF_MAC
					short theLen; Handle fpath; char fullPath[256];
					
					FSpGetFullPath(&tempSpec,':', &theLen,&fpath);
					memcpy(fullPath,*fpath,theLen);
					fullPath[theLen]=0;
					
					PathFromFullPath( fullPath, buf2 );
					sprintf( dirName, "%s%s:", buf2, VDptr->domainName );
					ReplacePathFromHost( VDptr->domainName, dirName );





















*/
