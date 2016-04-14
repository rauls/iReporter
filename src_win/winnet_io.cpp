#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "myansi.h"
#include "datetime.h"
#include "winnet_io.h"
#include "log_io.h"
#include "config.h"
#include "FileTypes.h"
#include "DateFixFileName.h"



#include <sys/stat.h>
#include <windows.h>
#include <wininet.h>
#include "winmain.h"
#include "resource.h"
#include "createshortcuts.h"

HINTERNET		hNetSession = NULL;
HINTERNET		hFTPSession;
HINTERNET		hFileConnection;           // handle to file enumeration

#define			gFtpPassiveFlag		MyPrefStruct.ftp_passive
//t				gFtpPassiveFlag = 0;

extern HWND		hwndParent;
extern long		OutDebugs( const char *txt, ... );
extern long		OutDebug( const char *txt );


#define	MAXFTPNAMESIZE		(256*2)
#define	MAXDOMAINNAMESIZE	128
#define	MAXERRORMSG			3000
#define	MAX_INETERRSIZE		3000


// -----------------------------------------------------------------------------------------------------
/*

  All the ftp/inet stuff is in

  http://msdn.microsoft.com/workshop/networking/wininet/overview/overview.asp

*/

int AreWeOnline( void )
{
	if ( InternetAttemptConnect( 0 ) == ERROR_SUCCESS )
		return 1;
	else
		return 0;

}

void *InitInternet( long mode )
{
	if ( AreWeOnline() ){
		if ( hNetSession == 0 ){

			if ( mode )
				mode = INTERNET_OPEN_TYPE_DIRECT;
			else
				mode = INTERNET_OPEN_TYPE_PRECONFIG;

			// Open Internet session.
			hNetSession = InternetOpen("iReporter",
							mode,			//INTERNET_OPEN_TYPE_PRECONFIG,INTERNET_OPEN_TYPE_DIRECT,
							NULL, 
							NULL,
							INTERNET_FLAG_RELOAD ) ;

			OutDebug( "Init Internet" );
		}
	} else
		hNetSession = 0;
	return hNetSession;
}

const char *NetworkErr( long *lpErrorCode )
{
	unsigned long	err=-1,l=MAX_INETERRSIZE;
	static	char szError[MAX_INETERRSIZE];

	if ( !InternetGetLastResponseInfo( &err, szError, &l ) )
		err = 0;

	if ( lpErrorCode )
		*lpErrorCode = err;

	return szError;
}


long NetworkErrMsg( void )
{
	long errval;
	const char *msg;

	msg = NetworkErr( &errval );

	if( errval )
		ErrorMsg( msg );

	return errval;
}

HINTERNET hFtpConnect = NULL;


void *INetOpenHTTP( char *url, __int64 *len )
{
	if ( url )
	{
		unsigned long length = 0;

		HINTERNET hNetFile = 0;

		if ( hNetSession == 0 )
		{
			InitInternet(0);
		}

		if ( hNetSession ) 
		{
			OutDebugs( "Opening URL: %s", url );

			if ( strstr( url, "http" ))
			{
				hNetFile = InternetOpenUrl( hNetSession, url, 0, 0, INTERNET_FLAG_HYPERLINK|INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_EXISTING_CONNECT /*flags*/ , 0 );
				OutDebugs( "InternetOpenUrl returned %08x", hNetFile );
				if ( hNetFile ){
					char szText[256];
					int ok;

					length = 32;
					OutDebugs( "Calling HttpQueryInfo()" );
					ok = HttpQueryInfo( hNetFile, HTTP_QUERY_CONTENT_LENGTH, szText, &length, NULL );
					OutDebugs( "HttpQueryInfo() returned %d", ok );
					if ( ok )
						length = myatoi(szText) ;
					else
						length = 0;
				}
				if ( hNetFile )
					StatusSetf( "Connected ok to URL %s", url );
				else {
					StatusSetID( IDS_FAILEDCONNECT );
					NetworkErrMsg();
				}
			}//else its not FTP and not HTTP

			if ( len )
				*len = (__int64)length;

		}
		return (void*)hNetFile;
	}
	return 0;
}



void *INetOpen( char *url, __int64 *len )
{
	if ( url )
	{
		unsigned long length = 0;

		HINTERNET hNetFile = 0;

		if ( hNetSession == 0 )
		{
			InitInternet(0);
		}


		if ( hNetSession ) 
		{
			OutDebugs( "Opening URL..." );

			if ( strstr( url, "http" ))
			{
				hNetFile = InternetOpenUrl( hNetSession, url, 0, 0, INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RESYNCHRONIZE , 0 );		//
				if ( hNetFile ){
					char szText[256];

					length = 32;
					HttpQueryInfo( hNetFile, HTTP_QUERY_CONTENT_LENGTH, szText, &length, NULL );
					length = myatoi(szText) ;
				}
				if ( hNetFile )
					StatusSetID( IDS_CONNECTED );
				else {
					StatusSetID( IDS_FAILEDCONNECT );
					NetworkErrMsg();
				}
			} else
			if ( strstr( url, "ftp" ) )
			{
				char	server[128],
						name[128],
						passwd[128],
						path[MAXFTPNAMESIZE];

				ExtractUserFromURL( url, server, name, passwd, path );
				hFtpConnect = (void*)FtpServerOpen( server, name, passwd );
				if ( !hFtpConnect )
				{
					FtpServerOpenError( server );
					OutDebugs( "FTP Server %s@%s is unreachable", name, server );
				}
				else
				{
					WIN32_FIND_DATA  lpFindFileData;
					HINTERNET hFind;
					char newpath[MAXFTPNAMESIZE];
					long flags = INTERNET_FLAG_NO_CACHE_WRITE;

					mystrcpy( newpath, path+1 );

					// First check if file exists and get its size.
					hFind = FtpFindFirstFile( hFtpConnect, newpath, &lpFindFileData, flags , 0 );
					if ( !hFind )
					{
						unsigned long size;
						FtpServerClose( hFtpConnect );

						OutDebugs( "No File....Trying root level path instead..." );
						hFtpConnect = (void*)FtpServerOpen( server, name, passwd );
						if ( hFtpConnect )
						{
							FtpGetCurrentDirectory( hFtpConnect, newpath, &size );
							strcat( newpath, path );
							hFind = FtpFindFirstFile( hFtpConnect, newpath, &lpFindFileData, flags , 0 );
						}
					}

					if ( hFind )
					{
						length = lpFindFileData.nFileSizeLow;

						// Now open the FILE after getting its size.
						hNetFile = FtpOpen( hFtpConnect, newpath, 'r' );

						if ( hNetFile )
							OutDebugs( "Ftp File Open %s size = %d", path, length );
						else
							NetworkErrMsg();
					} else
						OutDebugs( "No File found." );

				} // server cannot be opened.
			}//else its not FTP and not HTTP

			if ( len )
				*len = (__int64)length;

		}
		return (void*)hNetFile;
	}
	return 0;
}

void *INetClose( void *ih )
{
	if ( ih ){
		InternetCloseHandle( (HINTERNET)ih );
	}
	return NULL;			
}

void NetClose( void *ih )
{
	INetClose( ih );
	if ( hFtpConnect )
		hFtpConnect = INetClose( hFtpConnect );
	hNetSession = INetClose( hNetSession );
}


long NetWrite( void *fs, char *buffer, long len )
{
	unsigned long	lendone=0;

	if ( InternetWriteFile( fs, buffer, len, &lendone ) ){
	} else {
		StatusSetID( IDS_FAILEDWRITE );
	}
	return lendone;
}

long NetRead( void *ih, char *buffer, long len )
{
	unsigned long	lenread=0;

	if ( ih ){
		if ( InternetReadFile( ih, buffer, len, &lenread ) ){
			//lenread = len;
			OutDebugs( "read %d bytes", len );
		} else
			StatusSetID(IDS_FAILEDREAD );
	}
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
	HINTERNET hServer=0;
	long flags;
	short port = INTERNET_DEFAULT_FTP_PORT;
	char *portPtr;

	portPtr = strchr( server, ':' );
	if ( portPtr ){
		port = (short)myatoi( portPtr+1 );
		*portPtr = 0;
	}

	OutDebugs( "Doing FtpServerOpen..." );
	if ( hNetSession == 0 ){
		InitInternet(0);
	}

	flags = INTERNET_SERVICE_FTP | INTERNET_FLAG_RESYNCHRONIZE;
	if ( gFtpPassiveFlag )
		flags |= INTERNET_FLAG_PASSIVE;

	StatusSetID( IDS_CONNECTINGTO, username, server );

	hServer = InternetConnect(  hNetSession, server, port , username, password	, INTERNET_SERVICE_FTP , flags , 1 );

	if ( hServer ){
		StatusSetID( IDS_CONNECTED );
	} else
		StatusSetID( IDS_FAILEDCONNECT );

	return hServer;
}

void FtpServerClose( void *ih )
{
	OutDebug( "Closing ftp connection." );
	INetClose( ih );
}


long FtpOpenWriteError( char *file )
{
	long err;
	const char *msg;

	msg = NetworkErr( &err );

	if ( err )
	{
		//Check if we really need to report on this error
		// if the last error is <300 then we dont need to really.
		//
		ErrorMsg( "Cannot open file %s because ...\n%s", file, msg );
	}
	return err;
}

void *FtpOpenWrite( void *hConnect, char *file, int passive )
{
	HINTERNET hNetFile = 0;

	if ( hNetSession && hConnect ) {
		char cwd[MAXFTPNAMESIZE];
		unsigned long len;
		long flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RESYNCHRONIZE | INTERNET_FLAG_RELOAD;
		long access;
		access = GENERIC_WRITE;

		if ( gFtpPassiveFlag )
			flags |= INTERNET_FLAG_PASSIVE;

		len = MAXFTPNAMESIZE-1;

		FtpGetCurrentDirectory( hConnect, cwd, &len );
		strcat( cwd, file );
		DateFixFilename( cwd, 0 );
		hNetFile = FtpOpenFile( hConnect, cwd, access, FTP_TRANSFER_TYPE_BINARY ,0 );

		if( !hNetFile )
		{
			FtpOpenWriteError( file );
		}
	}
	return (void*)hNetFile;
}

void *FtpOpenRead( void *hConnect, char *file, int passive )
{
	HINTERNET hNetFile = 0;

	if ( hNetSession && hConnect ) {
		char cwd[MAXFTPNAMESIZE];
		unsigned long len;
		long flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RESYNCHRONIZE | INTERNET_FLAG_RELOAD;
		long access;

		access = GENERIC_READ;

		if ( passive )
			flags |= INTERNET_FLAG_PASSIVE;

		len = MAXFTPNAMESIZE-1;

		DateFixFilename( file, cwd );
		hNetFile = FtpOpenFile( hConnect, cwd, access, FTP_TRANSFER_TYPE_BINARY ,0 );

		if ( !hNetFile  ){
			OutDebugs( "No File....Trying root level path instead..." );
			FtpGetCurrentDirectory( hConnect, cwd, &len );
			strcat( cwd, "/" );
			strcat( cwd, file );
			DateFixFilename( cwd, 0 );
			hNetFile = FtpOpenFile( hConnect, cwd, access, FTP_TRANSFER_TYPE_BINARY ,0 );
		}

		if( !hNetFile )
			NetworkErrMsg();
	}
	return (void*)hNetFile;
}


void *FtpOpen( void *hConnect, char *file, char type )
{
	void *hNetFile;

	type = tolower( type );
	switch( type )
	{
		case 'r' : hNetFile = FtpOpenRead( hConnect, file, gFtpPassiveFlag ); break;
		case 'w' : hNetFile = FtpOpenWrite( hConnect, file, gFtpPassiveFlag ); break;
	}
	return hNetFile;
}

int FtpMakeDir( void *server, char *path )
{
	char cwd[MAXFTPNAMESIZE]; 
	unsigned long len;
	long ret;

	len = MAXFTPNAMESIZE-1;
	FtpGetCurrentDirectory( server, cwd, &len );
	strcat( cwd, path );
	DateFixFilename( cwd, 0 );

	ret = FtpCreateDirectory( server, cwd );
	if ( !ret )
	{
		DateFixFilename( path, cwd );
		ret = FtpCreateDirectory( server, cwd );
	}

	return ret;
}

int FtpDelFile( void *server, char *file )
{
	long ret;
	ret = FtpDeleteFile( server, file );
	// Try Root Level
	if ( !ret ){
		char cwd[MAXFTPNAMESIZE];
		unsigned long len;
		len = MAXFTPNAMESIZE-1;
		FtpGetCurrentDirectory( server, cwd, &len );
		strcat( cwd, file );
		DateFixFilename( cwd, 0 );
		ret = FtpDeleteFile( server, cwd );

		// last resort, try it with out /
		if ( !ret && file[0] == '/' )
			ret = FtpDeleteFile( server, file+1 );
	}

	if( !ret )
		NetworkErrMsg();
	return ret;
}

long FtpFileGetSize( char *ftpsite )
{
	char	url[MAXFTPNAMESIZE],
			server[128],
			name[128],
			passwd[128],
			path[MAXFTPNAMESIZE];

	WIN32_FIND_DATA  lpFindFileData;
	HINTERNET hFind, hServer;

	if ( ftpsite )
	{
		OutDebug( "FtpFileGetSize..." );
		if ( IsURLShortCut( ftpsite ) )
		{
			GetURLShortCut( ftpsite, url );
		} else
			strcpy( url, ftpsite );

		DateFixFilename( url, 0 );

		ExtractUserFromURL( url, server, name, passwd, path );

		hServer = FtpServerOpen( server, name, passwd );

		if ( hServer )
		{
			long flags = INTERNET_FLAG_NO_CACHE_WRITE;

			hFind = FtpFindFirstFile( hServer, path+1, &lpFindFileData, flags , 0 );
			if ( !hFind )
			{	// failed, so try ROOT path
				char cwd[MAXFTPNAMESIZE]; 
				unsigned long len = MAXFTPNAMESIZE-1;

				FtpServerClose( hServer );

				OutDebugs( "No File....Trying root level path instead..." );
				hServer = FtpServerOpen( server, name, passwd );
				if ( hServer )
				{
					FtpGetCurrentDirectory( hServer, cwd, &len );
					strcat( cwd, path );
					hFind = FtpFindFirstFile( hServer, cwd, &lpFindFileData, flags , 0 );
				}
			}

			FtpServerClose( hServer );
			if ( hFind ) 
				return lpFindFileData.nFileSizeLow;
		}
	}
	return 0;
}



/*
http://msdn.microsoft.com/workshop/networking/wininet/reference/functions/FtpFindFirstFile.asp
HINTERNET FtpFindFirstFile(
    IN HINTERNET hConnect,
    IN LPCTSTR lpszSearchFile,
    OUT LPWIN32_FIND_DATA lpFindFileData,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
);

http://msdn.microsoft.com/workshop/networking/wininet/reference/functions/InternetFindNextFile.asp
BOOL InternetFindNextFile(
    IN HINTERNET hFind,
    OUT LPVOID lpvFindData
);

typedef struct _WIN32_FIND_DATA {
    DWORD       dwFileAttributes;
    FILETIME    ftCreationTime;
    FILETIME    ftLastAccessTime;
    FILETIME    ftLastWriteTime;
    DWORD       nFileSizeHigh;
    DWORD       nFileSizeLow;
    DWORD       dwReserved0;
    DWORD       dwReserved1;
    CHAR        cFileName[ MAX_PATH ];
    CHAR        cAlternateFileName[ 16 ];
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;
*/

// history
//000607, Fixed the wildcard ftp listings


// This shouldnt call AddFiletoLogQ(), but instead return an char ** array of
// filename pointers that the caller can free();
int AddWildcardFtpDirList( char *ftpsite, long start )
{
	WIN32_FIND_DATA  lpFindFileData;
	HINTERNET hFind, hServer;
	long	n;
	char	*file, *p;
	char	newurl[MAXFTPNAMESIZE];
	char	url[MAXFTPNAMESIZE],
			server[128],
			name[128],
			passwd[128],
			path[MAXFTPNAMESIZE];
	

	if ( IsURLShortCut( ftpsite ) ){
		GetURLShortCut( ftpsite, url );
	} else
		strcpy( url, ftpsite );

	DateFixFilename( url, 0 );

	ExtractUserFromURL( url, server, name, passwd, path );

	if ( strstr( path, ".gz" ) )
	{
		ErrorMsg( "Cannot read compressed files over ftp" );
		return 0;
	}

	OutDebugs( "AddWildcardFtpDirList..." );

	hServer = FtpServerOpen( server, name, passwd );
	if ( !hServer )
		FtpServerOpenError( server );
	else
	{
		long	flags = INTERNET_FLAG_NO_CACHE_WRITE;
		char	cwd[MAXFTPNAMESIZE];

		hFind = FtpFindFirstFile( hServer, path+1, &lpFindFileData, flags , 0 );
		if ( !hFind ){
			unsigned long len = MAXFTPNAMESIZE-1;
			FtpServerClose( hServer );

			OutDebugs( "No File....Trying root level path instead..." );
			hServer = FtpServerOpen( server, name, passwd );
			if ( hServer )
			{
				FtpGetCurrentDirectory( hServer, cwd, &len );
				strcat( cwd, path );
				hFind = FtpFindFirstFile( hServer, cwd, &lpFindFileData, flags , 0 );
			} else
				FtpServerOpenError( server );
		}

		if ( !hFind )
			MsgBox_Error( IDS_ERR_FTPREAD, path+1 );

		n = start;
		while( hFind ){
			if( hFind ){
				file = lpFindFileData.cFileName;
				p = newurl;

				strcpybrk( p, url, '/', 3 );	// copy ftp://host.com
				if ( !strchr( file,'/' ) ){			// if no path, copy orig path
					strcat( p, path );
				} else
					strcat( p, "/" );
				p = strrchr( p, '/' );
				p += mystrcpy( p+1, file );		// copy filename

				n = AddFileToLogQ( newurl, n );	// add to process list.
			}

			if( InternetFindNextFile( hFind, &lpFindFileData ) == FALSE )
				hFind = NULL;
		}
		FtpServerClose( hServer );
	} 
	return n;
}




// history
//000607, Fixed the wildcard ftp listings
//ftp://wamltd01:R452TE@ftp.woods-fans.com/log/20010416.log
int AddWildcardFtpDirToHistory( char *ftpsite, long start )
{
	char	*file, *p;

	char	newurl[MAXFTPNAMESIZE];
	char 	url[MAXFTPNAMESIZE], 
			server[128],
			name[128],
			passwd[128],
			path[MAXFTPNAMESIZE]; 

	long	n;
	HINTERNET hFind, hServer;

	if ( IsURLShortCut( ftpsite ) ){
		GetURLShortCut( ftpsite, url );
	} else
		strcpy( url, ftpsite );

	DateFixFilename( url, 0 );

	ExtractUserFromURL( url, server, name, passwd, path );

	if ( strstr( path, ".gz" ) ){
		ErrorMsg( "Cannot read compressed files over ftp" );
		return 0;
	}

	OutDebugs( "AddWildcardFtpDirToHistory..." );
	
	hServer = FtpServerOpen( server, name, passwd );
	if ( !hServer )
		FtpServerOpenError( server );
	else
	{
		char				cwd[MAXFTPNAMESIZE];
		WIN32_FIND_DATA		lpFindFileData;
		long				flags = INTERNET_FLAG_NO_CACHE_WRITE;

		// Try absolute lever first
		hFind = FtpFindFirstFile( hServer, path, &lpFindFileData, flags , 0 );

		// if it fails, try relative level to current path
		if ( !hFind )
			hFind = FtpFindFirstFile( hServer, path+1, &lpFindFileData, flags , 0 );

		// if that fails, try the relative path to the 'cwd' path.
		if ( !hFind ){
			unsigned long len = MAXFTPNAMESIZE-1;
			FtpServerClose( hServer );

			OutDebugs( "No File....Trying root level path instead..." );
			hServer = FtpServerOpen( server, name, passwd );
			if ( hServer )
			{
				FtpGetCurrentDirectory( hServer, cwd, &len );
				strcat( cwd, path );
				hFind = FtpFindFirstFile( hServer, cwd, &lpFindFileData, flags , 0 );
			} else
				FtpServerOpenError( server );
		}

		if ( !hFind )
			MsgBox_Error( IDS_ERR_FTPREAD, path+1 );

		n = start;
		while( hFind ){
			if( hFind ){
				file = lpFindFileData.cFileName;
				p = newurl;

				strcpybrk( p, url, '/', 3 );	// copy ftp://host.com
				if ( !strchr( file,'/' ) ){		// if no path, copy orig path
					strcat( p, path );
				} else
					strcat( p, "/" );
				p = strrchr( p, '/' );
				p += mystrcpy( p+1, file );		// copy filename

				AddLogToHistory( newurl );	// add to process list.
				n++;
			}

			if( InternetFindNextFile( hFind, &lpFindFileData ) == FALSE )
				hFind = NULL;
		}
		FtpServerClose( hServer );
	}
	return n;
}



#define	HTMLBUFFERSIZE		10000
#define	HTMLBUFFERREADSIZE	200

// Retrieve syncronously a page title from a URL
char *HTTPURLGetTitle( char *url, char *title )
{
	void	*hnd;
	char	*buffer, *p=0;
	__int64	length;

	if( url )
	{
		if( IsURL(url) )
		{
			hnd = (void*)INetOpenHTTP( url, &length );

			if( hnd )
			{
				if( length > HTMLBUFFERSIZE )
					length = HTMLBUFFERSIZE;
				if( length == 0 )		// if we dont know the length, then get 4000 bytes max
					length = 4000;

				if(length < 0 ) 
				{
					INetClose( hnd );
					return 0;
				} else
				if( buffer = (char *)malloc( (long)length ) )
				{
					MemClr( buffer, length );

					int foundtitle = FALSE, dataread = 0, lenreturned;

					while( !foundtitle && dataread<length )
					{
						lenreturned = NetRead( hnd, buffer+dataread, (long)(HTMLBUFFERREADSIZE) );
						OutDebugs( "returned %d bytes, total %d, length=%d", lenreturned, dataread, length );
						if ( lenreturned ){
							dataread += lenreturned;
							if ( strstri( buffer, "<TITLE>" ) && strstri( buffer, "</TITLE>" ) )
								foundtitle = TRUE;
						} else {
							length = dataread;
						}
					}
					INetClose( hnd );

					// Handle erors, ie dont use these as page titles.
					if( strstri( buffer, "404 Not Found" ) )
						return NULL;

					if( strstri( buffer, "site cannot be found" ) )
						return NULL;

					if ( strstri( buffer, "Firewall Error" ) )
						return NULL;
					// -----------------------------------------------

					p = strstri( buffer, "<TITLE>" );
					if( p ){
						strcpybrk( title, p+7, '<', 1 );
					}
					free( buffer );
				}

			}
		}
	}
	return p;
}


/*

  BOOL InternetGetLastResponseInfo(
    OUT LPDWORD lpdwError,
    OUT LPTSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength
);


*/


void FtpClose( void *fs )
{
	INetClose( fs );
}

