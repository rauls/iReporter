#ifndef _WNETIO_H
#define _WNETIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "myansi.h"



extern int				gFtpPassiveFlag;

int AreWeOnline( void );

void *InitInternet( long );
void *INetOpen( char *url, __int64 *len );
void *INetClose( void *h );

const char *NetworkErr( long *lpErrorCode );
void NetClose( void *h );
long NetWrite( void *fs, char *buffer, long len );
long NetRead( void *ih, char *buffer, long len );


long FtpFileGetSize( char *ftpsite );
void *FtpOpen( void *fs, char *file, char type );

long FtpServerOpenError( const char *server );
void *FtpServerOpen( char *server, char *username, char *password );

void FtpServerClose( void *ih );
int FtpMakeDir( void *server, char *path );
int FtpDelFile( void *server, char *file );
long FtpFileGetSize( char *ftpsite );

int AddWildcardFtpDirList( char *ftpsite, long start );
int AddWildcardFtpDirToHistory( char *ftpsite, long start );

char *HTTPURLGetTitle( char *url, char *title );

#ifdef __cplusplus
}
#endif

#endif
