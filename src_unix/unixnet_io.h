#ifndef _UNETIO_H
#define _UNETIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "myansi.h"

void *InitInternet( long );
const char *NetworkErr( long *lpErrorCode );
void *INetOpen( char *url, __int64 *len );
void *INetClose( void *h );

#if __dest_os != __mac_os
void NetClose( void *h );
#endif

long NetWrite( void *fs, char *buffer, long len );
long NetRead( void *ih, char *buffer, long len );


long FtpFileGetSize( char *ftpsite );
void *FtpOpen( void *fs, char *file, char type);
void FtpClose( void *fs );
void *FtpServerOpen( char *server, char *username, char *password );
long FtpServerOpenError( const char *server );
void FtpServerClose( void *ih );
int FtpMakeDir( void *server, char *path );
int FtpDelFile( void *server, char *file );
long FtpFileGetSize( char *ftpsite );
int AddWildcardFtpDirList( char *ftpsite, long start );

char *HTTPURLGetTitle( char *url, char *title );
#ifdef __cplusplus
}
#endif

#endif
