#include "Compiler.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#if DEF_WINDOWS
#include <windows.h>
#include <winsock.h>
#include <io.h>		// for close()
#include "registry.h"
#include "resource.h"
#include "Winutil.h"
#include "WinDNR.h"
#include "Winmain.h"	// for StatusSet()
#elif DEF_UNIX
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <netdb.h>
#include "main.h"
#define	GetHostByName	gethostbyname
#define	SOCKET	int
#endif

#include "smtp.h"
#include "myansi.h"
#include "datetime.h"


extern long OutDebug( const char *txt );


#define	OUTLOOK_DEFACCOUNT		"Software\\Microsoft\\Internet Account Manager"		//\\Default Mail Account"
#define	OUTLOOK_SMTPSERVER		"Software\\Microsoft\\Internet Account Manager\\Accounts\\%s"	//\\SMTP Server"

#define	NETSCAPE_DEFACCOUNT		"Software\\Netscape\\Netscape Navigator\\biff"		//\\CurrentUser"
#define	NETSCAPE_SMTPSERVER		"Software\\Netscape\\Netscape Navigator\\biff\\users\\%s"		//\\defaultServer"

char* GetSMTPServer( char* email )
{
#if DEF_WINDOWS
	enum { MAX_SERVER_LEN=255 };

	static char	sl_Server[MAX_SERVER_LEN+1];		// returned by this function
	char	server[MAX_SERVER_LEN+1],
			text[MAX_SERVER_LEN+1];
	long	len=MAX_SERVER_LEN+1;

	if ( !GetUserRegKey ( OUTLOOK_DEFACCOUNT, "Default Mail Account", server, &len ) )
	{
		sprintf( text, OUTLOOK_SMTPSERVER, server );
		len=MAX_SERVER_LEN;
		if ( !GetUserRegKey ( text, "SMTP Server", sl_Server, &len ) )
		{
			GetUserRegKey ( text, "SMTP Email Address", email, &len );
			return sl_Server;
		}
	}
	else
	{
		if ( !GetUserRegKey ( NETSCAPE_DEFACCOUNT, "CurrentUser", server, &len ) )
		{
			sprintf( text, NETSCAPE_SMTPSERVER, server );
			len=MAX_SERVER_LEN;
			if ( !GetUserRegKey ( text, "defaultServer", sl_Server, &len ) )
			{
				return sl_Server;
			}
		}
	}
#endif
	return NULL;	// UNIX always get to here and WINDOWS does on failure	
}


//WSADATA W;

#define DEBUGREC		if( strlen(R)<100){ sprintf( szText, "REC:%s", R) ; OutDebug( szText ); }
#define DEBUGSND		if( strlen(R)<100){ sprintf( szText, "SND:%s", R) ; OutDebug( szText ); }

#define	GetResponse(S,R)	i=recv(S,R,5000,0); DEBUGREC
#define	SendCmd( S,R )		i=send(S,R,strlen(R),0); DEBUGSND



/*
   smtpServer = GetSMTPServer( returnEmail );

  if ( !smtpServer ){
	  char *p;
	  WinGetHostName( szText, 255 );
	  if ( p = mystrchr( szText, '.' ) ){
		sprintf( gServer, "mail%s", p );
		smtpServer = gServer;
	  }
	  //smtpServer = "proxy1.ba.best.com";
  }
  if( !(H=GetHostByName( smtpServer )) ){
	sprintf( szText, "%s server not found", smtpServer );
	StatusSet( szText );
	return 1;
  }

  */


// email TO: field can have   person@host.com,smtp.server.com
int MailTo( char *to, char *from, char *subject, char *body, char	*smtpServer )
{
	int	i;
	char	R[5000];
	char	*next,
		szText[256],
		returnEmail[256],
		toEmail[2048];
	struct hostent *H;
	SOCKET S;
	struct sockaddr_in A;

	if ( from[0] )
		mystrcpy( returnEmail, from );
	else
		mystrcpy( returnEmail, "iReporter@gmail.com" );

	if ( !strcmpd( "mailto:", to ) )					//ignore mailto: prefix, if some stupid user types it in.
		mystrcpy( toEmail, to+7 );
	else
		mystrcpy( toEmail, to );

	if( !smtpServer[0] ){
		sprintf( szText, "No SMTP mail server defined" );
		StatusSet( szText );
		ErrorMsg( szText );
		return 1;
	}

	if( !(H = (struct hostent *)GetHostByName( smtpServer )) ){
		sprintf( szText, "%s server not found", smtpServer );
		StatusSet( szText );
		return 1;
	}

	next = toEmail;
	while( next ){
		char out[256];
		register char **cp;

		strcpyuntil( out, next, ',' );

		S = socket(AF_INET, SOCK_STREAM,0);
		if ( !S ) {
			StatusSetID( IDS_FAILEDSOCKET );
			return 1;
		}

		sprintf( szText, "Sending mail to %s", out );
		StatusSet( szText );
		SleepTicks( 50 );

		sprintf( szText, "Connecting to SMTP %s", smtpServer );
		StatusSet( szText );
		SleepTicks( 30 );

		A.sin_family=AF_INET;
		A.sin_port = htons(25);

		cp = (char**)H->h_addr_list;
		memcpy ((char *)&A.sin_addr, *cp, 4 );

		i=connect(S,(struct sockaddr *) &A,sizeof(A));
		if( i ) {
			sprintf( R, "failed to connect to SMTP server %s", smtpServer );
			StatusSet( R );
			ErrorMsg( R ); return 2;
		}

		GetResponse( S,R )

		StatusSetID( IDS_SENDMAILHEADER );

		strcpy(R,"HELO localhost\r\n");
		SendCmd( S,R )
		GetResponse( S,R )

		sprintf(R,"MAIL FROM: <%s>\r\n", returnEmail );
		SendCmd( S,R )
		GetResponse( S,R )

		sprintf(R,"RCPT TO:<%s>\r\n", out );
		SendCmd( S,R )
		GetResponse( S,R )

		strcpy(R,"DATA\r\n");
		SendCmd( S,R )
		GetResponse( S,R )

		sprintf( szText, "Sending email to %s", out );
		StatusSet( szText );

		sprintf(R,"To: %s\r\n", out );
		SendCmd( S,R )
		sprintf(R,"FROM: <%s>\r\n", returnEmail );
		SendCmd( S,R )
		if ( subject[0] )
			sprintf( R,"SUBJECT: %s\r\n", subject );
		else
			strcpy( R,"SUBJECT: Post Process Notify\r\n" );
		SendCmd( S,R )
		CTimeToDateTimeStr( GetCurrentCTime(), szText );
		sprintf(R,"DATE: %s\r\n", szText );
		SendCmd( S,R )
		strcpy( R,"MESSAGE_ID: <123@e.com>\r\n");
		SendCmd( S,R )

		if ( strstr( body, "<HTML>" ) ){
			char ctype[] = "Content-Type: text/html;\r\n         charset=\"iso-8859-1\"\r\nContent-Transfer-Encoding: quoted-printable\r\n";
			SendCmd( S,ctype )
		}
		strcpy(R,"\r\n");
		SendCmd( S,R )

		SendCmd( S,body )

		strcpy(R,"\r\n");
		SendCmd( S,R )

		strcpy(R,".\r\n");
		SendCmd( S,R )
		GetResponse( S,R )

		strcpy(R,"QUIT\r\n");
		SendCmd( S,R )
		GetResponse( S,R )

#ifdef DEF_WINDOWS
		closesocket( S );
#else
		close( S );
#endif		
		// not sure if the above close() call is working under Windows - can't hurt for other platforms
		assert(errno!=EBADF);

		StatusSetID(IDS_MAILSENT );
		SleepTicks( 50 );

		if ( next = mystrchr( next, ',' ) ){
			*next++ = 0;
			while( *next == ' ' ) *next++ = 0;
		}
	}


	return 0;
}
