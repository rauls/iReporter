
#include "FWA.h"
#include "FileTypes.h"
#include "myansi.h"
#include "string.h"
#include "datetime.h"

#ifdef DEF_WINDOWS
	#include <windows.h>
#endif				

//  ftp://user:pass@hsot.com
void MakeSafeURL( char *url )
{
	if ( IsURL( url ) )
	{
		char *p;
		long col_count = 0;

		p = strchr( url, '/' );
		if ( p )
		{
			p = strchr( p, ':' );
			if ( p )
			{
				p++;
				while( *p && *p != '@' )
					*p++ = '*';
			}
		}
	}
}

// copy a site and path into one url making sure it doesnt have two //
void MakeURL( char *out, const char *site, const char *path )
{
	if ( out && site && path )
	{
		char* p=out + mystrcpy( out, site );

		if( p!=out && *(p-1) == '/' && *path == '/' )
		{
			++path;
		}

		mystrcpy( p, path );
	}
}


char *IsURL( const char *filename )
{
static	char *lastptr=0;

	if ( filename ){
		if ( (!strcmpd( "ftp:",filename )) || (!strcmpd( "http:",filename )) )
			lastptr = (char*)filename;
		else {
			char *p;
			p = mystrrchr( filename, '.' );
			if ( !strcmpd( ".url",p )  )
				lastptr = (char*)filename;
			else
				lastptr = 0;
		}
	}
	return lastptr;
}

const char *IsShortCut( const char *filename )
{
	if ( filename ){
		char *p;
		p = mystrrchr( filename, '.' );
		if ( !strcmpd( ".lnk",p )  )
			return filename;
		else
			return 0;
	}
	return 0;
}

const char *IsURLShortCut( const char *filename )
{
	if ( filename ){
		char *p;
		p = mystrrchr( filename, '.' );
		if ( !strcmpd( ".url",p )  )
			return filename;
		else
			return 0;
	}
	return 0;
}

const char *IsGZIP( const char *filename )
{
	if ( filename ){
		if ( (strstr( filename, ".gz" )) )
			return filename;
		else
			return 0;
	}
	return 0;
}

const char *IsBZIP( const char *filename )
{
	if ( filename ){
		if ( (strstr( filename, ".bz" )) )
			return filename;
		else
			return 0;
	}
	return 0;
}

const char *IsPKZIP( const char *filename )
{
	if ( filename ){
		if ( (strstr( filename, ".zip" )) )
			return filename;
		else
			return 0;
	}
	return 0;
}

long IsCompressed( char *filename )
{
	if ( IsFilePKZIP( filename ) )
		return 1;
	else if ( IsBZIP( filename ) )
		return 1;
	else if ( IsGZIP( filename ) )
		return 1;
	return 0;
}




long IsFileaFolder( const char *file )
{
#if DEF_WINDOWS
	WIN32_FIND_DATA FindFileData;
	HANDLE	f;

	f = FindFirstFile( file, &FindFileData );

	if ( f!=INVALID_HANDLE_VALUE){
		FindClose( f );
		if ( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			return 1;
	}
	return 0;
#endif
	return 0;
}

// ftp://user:pass@site.com/path/
void ExtractUserFromURL( char *url, char *server, char *username, char *passwd, char *file )
{
	char	*p, *d, *adr, *host;
	long	l;

	if ( IsURL( url ) )
	{
		if ( p = strstr( url, "ftp://" ) )
			adr = url+6;
		else
			adr = url;

		server[0] = 0;
		if ( host = mystrrchr( p, '@' ) )
		{
			strcpyx( server, host+1, '/', 1 );
			l = strlen( server );
			//server[ l-1 ] = 0;
			if ( p = mystrchr( adr, ':' ) )
			{
				p++;
				d = passwd;
				while( p<host )
					*d++ = *p++;
				*d = 0;
				//strcpyx( passwd, p+1, '@',1 );
				//l = strlen( passwd );
				//passwd[ l-1 ] = 0;
				strcpyx( username, adr, ':',1 );
				l = strlen( username );
				username[ l-1 ] = 0;
			}
			else
			{
				strcpyx( username, adr, '@',1 );
				l = strlen( server );
				username[ l-1 ] = 0;
			}
		}
		else
		{
			username[0] = 0;
			passwd[0] = 0;
			strcpyx( server, adr, '/',1 );
		}

		l = strlen( server );
		server[ l-1 ] = 0;
		file[0] = 0;
		if ( p = mystrchr( adr, '/' ) )
			mystrcpy( file, p );
	}
}

void MakeURLfromDetails( char *url, char *site, char *name, char *pass, char *path )
{
	if ( url )
	{
		if ( strlen( name ) )
		{
			strcat( url, name );
			if ( strlen( pass ) )
			{
				strcat( url, ":" );
				strcat( url, pass );
			}
			strcat( url, "@" );
		}
		strcat( url, site );

		if ( *path != '/' )
			strcat( url, "/" );

		strcat( url, path );
	}
}


