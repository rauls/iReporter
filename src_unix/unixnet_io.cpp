#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>



#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>
      
#include <netdb.h>


#include "myansi.h"
#include "datetime.h"
#include "ftp_client.h"
#include "unixnet_io.h"
#include "log_io.h"
#include "config.h"
#include "FileTypes.h"
#include "DateFixFilename.h"
#include "EngineStatus.h"


extern struct		App_config MyPrefStruct;

void *hServer;

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


const char *NetworkErr( long *lpErrorCode )
{
	long	err = errno;
	static	char szError[2000];

	if ( err )
		strcpy( szError, strerror( err ) );
	else
		szError[0] = 0;

	if ( lpErrorCode )
		*lpErrorCode = err;

	if ( err )
		return szError;
	else
		return NULL;
}

long NetworkErrMsg( void )
{
	const char *errMsg;
	long err;
	
	if ( errMsg = NetworkErr(&err) )
		ErrorMsg( (char*)errMsg );
	return err;
}


int OpenSocket( char *host, long port )
{
	struct sockaddr_in sa;
	struct hostent *hp;
	int sockfd;
	
	memset(&sa, 0, sizeof(sa));

	if ((hp = gethostbyname(host)) == (struct hostent *)0) {
		OutDebugs( "No domain entry for %s\n", host);
		return -1;
	}
	memcpy( (char *) &sa.sin_addr, hp->h_addr, hp->h_length);
	
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	
	if (!(sockfd = socket(AF_INET, SOCK_STREAM, 0))) {
		OutDebugs( "Couldn't create a socket?\n");
		return -2;
	}
	
	if (connect(sockfd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
		OutDebugs( "Can't connect to host %s: %s\n", host, strerror(errno));
		return -3;
	}
	return sockfd;
}




long HttpFileOpen( const char *server, const char *filename )
{
	long socket;

	OutDebugs( "OpenHost()...." );
	socket = OpenSocket( (char*)server, 80 );
	OutDebugs( "OpenHost()....returned - %d", socket );
	if ( socket ){
		char request[256];
		OutDebugs( "OpenHost( ) done" );
		sprintf( request, "GET %s HTTP1.0\n\n", filename );
		OutDebugs( "sending request" );
		SendCmd( socket, request );

		// Read HTTP Header, then stop.
	}
	return socket;
}


void *INetOpen( char *url, __int64 *len )
{
	OutDebugs( "INetOpen( %s )", url );

	if ( url ){
		long length = 0;
		void *hNetFile;
		int	hFtpConnect;

		if ( strstr( url, "http:" ) ){
			char	server[128], path[256], name[64], passwd[32], *urlptr;
			long	l;

			if ( strstr( url, "http://" ) )
				urlptr = url+7;
			else
				urlptr = url;

			strcpyx( server, urlptr, '/',1 );
			l = strlen( server );
			server[ l-1 ] = 0;
			mystrcpy( path, urlptr + strlen( server ) );

			OutDebugs( "HttpFileOpen( %s )", server );

			if ( *server )
			{
				hNetFile = (void*)HttpFileOpen( server, path );

				if ( hNetFile>0 )
					OutDebugs( "HTTP File Open %s size = %d", path, length );
				else
					NetworkErrMsg();
			}
		} else
		if ( strstr( url, "ftp:" ) ){
			char	server[128], path[256], name[64], passwd[32];
			ExtractUserFromURL( url, server, name, passwd, path );

			if ( !name[0]   ) strcpy( name, "ftp" );
			if ( !passwd[0] ) strcpy( passwd, "me@home.com" );

			hFtpConnect = (int)FtpServerOpen( server, name, passwd );

			if ( hFtpConnect >0 ){
				char newpath[256];

				mystrcpy( newpath, path+1 );

				// Now open the FILE after getting its size.
				hNetFile = (void*)FtpOpen( (void*)hFtpConnect, newpath, 'r' );

				if ( hNetFile>0 )
					OutDebugs( "Ftp File Open %s size = %d", path, length );
				else
					NetworkErrMsg();

			} else
				OutDebugs( "Cannot connect to ftp %s@%s", name, server );
		}

		if ( len && length )
			*len = (__int64)length;

		return (void*)hNetFile;
	}
	return 0;
}

void *INetClose( void *ih )
{
	if ( ih ){
		FTP_Logout( (int)ih );
		FTP_Close( (int)ih );
	}
	return NULL;			
}

void NetClose( void *ih )
{
	INetClose( ih );
}


long NetWrite( void *fs, char *buffer, long len )
{
	long	lendone=0;
	lendone = FTP_Write( (int)fs, buffer, len );
	return lendone;
}

long NetRead( void *ih, char *buffer, long len )
{
	long	lenread=0;
	OutDebug( "NetRead..." );
	lenread = FTP_Read( (int)ih, buffer, len );
	OutDebugs( "NetRead %d", lenread );
	return lenread;
}

long FtpServerOpenError( const char *server )
{
	const char *msg;
	long code;

	msg = NetworkErr( &code );

	if ( !code )
		ErrorMsg( "Server %s is unreachable\n\nMake sure you have spelled the domain name correctly.", server );
	else
		ErrorMsg( "Server %s is unreachable because ...\n%s", server, msg );
	//MsgBox_Error( IDS_ERR_CONNECTING, server, username, msg );		//Error connecting to ftp server (%s) as user (%s) because ..\n%s.

	OutDebugs( "FtpServerOpen failed return code = %d", code );
	return code;
}


void *FtpServerOpen( char *server, char *username, char *password )
{
	return (void*)FTP_ServerOpen( server, username, password );
}

void FtpServerClose( void *ih )
{
	OutDebug( "Closing ftp connection." );
	FTP_Close( (int)ih );
}



void *FtpOpenWrite( void *hConnect, char *file )
{
	return (void*)FTP_OpenWrite( (int)hConnect, file );
}

void *FtpOpenRead( void *hConnect, char *file )
{
	return (void*)FTP_OpenRead( (int)hConnect, file );
}


void *FtpOpen( void *hConnect, char *file, char type )
{
	void *hNetFile;

	type = tolower( type );
	switch( type )
	{
		case 'r' : hNetFile = (void*)FtpOpenRead( hConnect, file ); break;
		case 'w' : hNetFile = (void*)FtpOpenWrite( hConnect, file ); break;
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







#define	HTMLBUFFERSIZE	10240

char *HTTPURLGetTitleOld( char *url, char *title )
{
	void	*hnd;
	char	*buffer, *p=0;
	__int64	length;

	if ( url ){
		if ( IsURL(url) ){
			//StatusSetID( IDS_GETURLTITLE );					RHF
			hnd = (void*)INetOpen( url, &length );
			if ( hnd ){
				if (length > 8000 ) length = 8000;
				if (length == 0 ) {
					INetClose( hnd );
					return 0;
				} else
				if ( buffer = (char *)malloc( (long)length ) ){
					NetRead( hnd, buffer, (long)(length-1) );
					INetClose( hnd );

					if ( strstri( buffer, "404 Not Found" ) )
						return NULL;

					p = strstri( buffer, "<TITLE>" );
					if ( p ){
						strcpybrk( title, p+7, '<', 1 );
					}
					free( buffer );
				}

			}
		}
	}
	return p;
}


#define	HTMLBUFFERSIZE		10000
#define	HTMLBUFFERREADSIZE	200

// Retrieve syncronously a page title from a URL
char *HTTPURLGetTitle( char *url, char *title )
{
	void	*hnd;
	char	*buffer, *p=0;
	__int64	length = 4000;  //default the length, if we cant get it.

	if( url )
	{
		if( IsURL(url) )
		{
			hnd = (void*)INetOpen( url, &length );
//OutDebugs( "INetOpen" );

			if( hnd )
			{
				if( length > HTMLBUFFERSIZE )
					length = HTMLBUFFERSIZE;

				if( length == 0 ) {
					OutDebugs( "INetClose, source is 0" );
					INetClose( hnd );
					return 0;
				} else
				if( buffer = (char *)malloc( (long)length ) )
				{
					MemClr( buffer, length );
					int foundtitle = FALSE, dataread = 0, lenreturned;

					while( !foundtitle && dataread<length && !IsStopped() )
					{
						lenreturned = NetRead( hnd, buffer+dataread, (long)(HTMLBUFFERREADSIZE) );
						if ( lenreturned ){
							dataread += lenreturned;
							if ( strstri( buffer, "<TITLE>" ) && strstri( buffer, "</TITLE>" ) )
								foundtitle = TRUE;
						} else {
							length = dataread;
						}
					}
					INetClose( hnd );

					if ( strstri( buffer, "401 Authorization Required" ) )
						return NULL;

					if ( strstri( buffer, "404 Not Found" ) )
						return NULL;

					if ( strstri( buffer, "site cannot be found" ) )
						return NULL;

					if ( strstri( buffer, "Firewall Error" ) )
						return NULL;

					p = strstri( buffer, "<TITLE>" );
					if ( p ){
						strcpybrk( title, p+7, '<', 1 );
					}
					free( buffer );

					OutDebugs( "Url Title = %s", title );
				}

			} else OutDebugs( "failed to open %s", url );
		}
	}
	return p;
}

void FtpClose( void *fs )
{
	INetClose( fs );
}
