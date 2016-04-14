#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "myansi.h"
#include "datetime.h"
#include "net_io.h"
#include "log_io.h"
#include "config.h"

#if _UNIX
#include <sys/errno.h>
#endif


extern struct		App_config MyPrefStruct;



int AreWeOnline( void )
{
	if ( 1 )
		return 1;
	else
		return 0;

}

void *InitInternet( long mode )
{
}

long INetError( char *szText, long l )
{
	long errval;

	return errval;
}

long INetErrorMsg( void )
{
	long errval, l=2000;
	char szText[2000];
	//GetLastErrorTxt( szText );
	return errval;
}


void *INetOpen( char *url, __int64 *len )
{
	if ( url ){
		long length = 0;
	
	}
	return 0;
}

void *INetClose( void *ih )
{
	if ( ih ){
	}
	return NULL;			
}

void NetClose( void *ih )
{
}


long NetWrite( void *fs, char *buffer, long len )
{
	long	lendone=0;
	return lendone;
}

long NetRead( void *ih, char *buffer, long len )
{
	long	lenread=0;
	return lenread;
}



void *FtpServerOpen( char *server, char *username, char *password )
{
	return 0;			
}

void FtpServerClose( void *ih )
{
	OutDebug( "Closing ftp connection." );
}



void *FtpOpenWrite( void *hConnect, char *file )
{
}

void *FtpOpenRead( void *hConnect, char *file )
{
}


void *FtpOpen( void *hConnect, char *file, char type )
{
	void *hNetFile;

	type = tolower( type );
	switch( type )
	{
		case 'r' : hNetFile = FtpOpenRead( hConnect, file ); break;
		case 'w' : hNetFile = FtpOpenWrite( hConnect, file ); break;
	}
	return hNetFile;
}

int FtpMakeDir( void *server, char *path )
{
}

int FtpDelFile( void *server, char *file )
{
	long ret;
	return ret;
}

long FtpFileGetSize( char *ftpsite )
{
	char	url[256];
	char	server[128];
	char	name[64];
	char	passwd[32];
	char	path[200];
	return 0;
}



int AddWildcardFtpDirList( char *ftpsite, long start )
{
	char	*file, url[256], newurl[256], *p;
	char	server[128];
	char	name[64];
	char	passwd[32];
	char	path[200];
	char	cwd[512]; 
	long	n;
	void *hServer;

	if ( IsURLShortCut( ftpsite ) ){
	} else
		strcpy( url, ftpsite );

	DateFixFilename( url, 0 );

	ExtractUserFromURL( url, server, name, passwd, path );

	if ( strstr( path, ".gz" ) ){
		ErrorMsg( "Cannot read compressed files over ftp" );
		return 0;
	}

	OutDebugs( "AddWildcardFtpDirList..." );

	hServer = FtpServerOpen( server, name, passwd );
	if ( hServer ){
		FtpServerClose( hServer );
	} 
	return n;
}




// history
//000607, Fixed the wildcard ftp listings
//ftp://wamltd01:R452TE@ftp.woods-fans.com/log/20010416.log
int AddWildcardFtpDirToHistory( char *ftpsite, long start )
{
	char	*file, url[256], newurl[256], *p;
	char	server[128];
	char	name[64];
	char	passwd[32];
	char	path[200];
	char	cwd[512]; 
	long	n;
	return n;
}








void FtpClose( void *fs )
{
}

