
///////////////////////////////////////////////////////////
//
//   httpinterface.c
//
//   - Implements a tiny web server to handle such things
//     as integration with Funnel Web Profiler and 
//     Benchmark Factory
//
//   Oct 2001 - implemented OpenTransport server for MacOS
//
//
///////////////////////////////////////////////////////////


#include "FWA.h"

#ifdef	DEF_MAC
	#include <OpenTransport.h>
	#include <OpenTptInternet.h>
	#include <Threads.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "config.h"
	#include "myansi.h"
	#include "server.h"
	#include "Processing.h"
	#include "main.h"
	#include "MacStatus.h"
	#include "MacUtil.h"
	#include "MPDutilities.h"
	#include "progress.h"
	#include "WindowExtensions.h"
	#include "Carbonize.h"

	extern "C" Boolean gDone ;
	Boolean gShutdownRequestedViaHTTP = false ;
#else
	#include <ctype.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <errno.h>
	#include "myansi.h"

#ifdef DEF_WINDOWS
	#include "Windnr.h"
	#include "Winmain.h"
	#include "Winutil.h"
	#include <winsock.h>
	#include "resource.h"
	extern long GetLastPercentProgress();
#endif

#ifdef DEF_UNIX	
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include "main.h"
#endif

#endif 

#include "version.h"

#include "config_struct.h"
#include "config.h"

#include "httpinterface.h"
#include "http_query.h"
#include "LogFileHistory.h"
#include "OutputMessages.h"
#include "ReportInterface.h"
#include "GlobalPaths.h"   


long SpawnHTTPD( void );


#ifndef DEF_MAC

static SOCKET http_socket = 0, peersock = 0;
static struct sockaddr_in A, Peer;
static long	msgcount=0;

void CloseHttpPeer( void ){
	if( peersock ){
		shutdown(peersock,2);
		closesocket(peersock);
		peersock = 0;
	}
}

static int threadcount = 1;


#ifdef DEF_WINDOWS
typedef int socklen_t;
#endif

#ifdef DEF_WINDOWS
DWORD WINAPI HTTPDThread( PVOID lpParam )
#else
int HTTPDThread( long lpParam )
#endif
{
	long ret, count=0;

	if ( lpParam ){
		SOCKET client_socket;
		struct sockaddr_in client;
		socklen_t client_len;
		long	client_ip;
		char	ipStr[32];
		char	*p;

		threadcount++;

		client_len = sizeof(client);
		client_socket = accept(http_socket,(struct sockaddr *) &client,&client_len);
		SpawnHTTPD();

		if ( client_socket != INVALID_SOCKET ){
			long keepalive = 1;
			char line[10240];
			char recbuf[20000];

	        struct linger lin;
            int buffsize = 64000 ;
	        int	opt      = 1;
	        int	optlen   = sizeof(opt);

	        lin.l_onoff =  1 ; lin.l_linger = 60;
	        setsockopt(client_socket, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
	        setsockopt(client_socket, SOL_SOCKET, SO_RCVBUF, (char *)&buffsize, sizeof(buffsize));
	        setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt, optlen);

			client_ip = (long)client.sin_addr.s_addr;
			iplongtoipstring( client_ip, ipStr );
			sprintf( line, "HTTPD%d: Accepting connection from %s ...", lpParam, ipStr );
			StatusSet( line );

            Sleep(100) ;
			ret = recv( client_socket, recbuf, 20000, 0 );
			// while recv() is open... keep geting data...
			while ( ret>0 || ret==keepalive ){
				if ( ret>0 ){
					char httpurl[10240];

					sprintf( line, "HTTPD%d: recieved request (%d bytes)", lpParam, ret );
					StatusSet( line );
					recbuf[ret] = 0;
					if ( recbuf[ret-1] == '\n' ) {
						p = recbuf;
						while( p ){
							p = CopyLine( line, p );
							if ( strstr( line, "GET " ) || strstr( line, "POST " ) )
								CopyLine( httpurl, line );
							//if ( strstr( line, "Keep-Alive" ) )
							//	keepalive = -1;
							sprintf( ipStr, "HTTPD%d", lpParam );
                            line[256] = '\0' ; // truncate because ShowReceivedLine can't cope with big URLS and line is finished with now
							ShowRecievedLine( ipStr, line );
						}
						if ( HandleHttpURL( client_socket, httpurl ) )
							count=0;
						else
							count++;
					}
				}
				if ( ret == keepalive ) {
					ret = recv( client_socket, recbuf, 30000, 0 );
					//Sleep(1);
					//StatusSet( "sleep" );
				} else ret = 0;
			}
			if ( ret<0 ){
				sprintf( line, "ret=%d, errno=%d", ret, errno );
				StatusSet( line );
			}
			if ( ret == SOCKET_ERROR ){
				sprintf( line, "HTTPD%d: socket recv error", lpParam );
				StatusSet( line );
			}

			shutdown(client_socket,2);
			closesocket(client_socket);

			sprintf( line, "HTTPD%d: closing socket", lpParam );
			StatusSet( line );



		} else {
			StatusSetID( IDS_FAILEDLISTEN );
		}
	}
	threadcount--;

    StatusSet("Ready") ;

	ExitThread( count );
    return(TRUE) ;
}

#ifdef DEF_UNIX
#include "util.h"
#endif


long SpawnHTTPD( void )
{
	HANDLE 	ghProcessThread = NULL;
	unsigned long dwThreadId;

	CreateThread( 
		NULL,                        // no security attributes 
		0,                           // use default stack size  
		HTTPDThread,				 // thread function 
		(void*)threadcount,	     // argument to thread function 
		0,                           // use default creation flags 
		&dwThreadId);                // returns the thread identifier  
    
#ifdef DEF_WINDOWS
	if (ghProcessThread) {
		SetThreadPriority( ghProcessThread, THREAD_PRIORITY_BELOW_NORMAL );
	}
#endif
	return(1);
}


void CloseHttpServer( void ){
	if( http_socket ){
		shutdown(http_socket,2);
		closesocket(http_socket);
		http_socket = 0;
	}
}

void InitHttpServer( short port )
{
	int success = 0;

#ifdef DEF_WINDOWS
	success = WSInit();
#else
	success = TRUE;
#endif

	if( success ){
		long ret;
		http_socket = socket(AF_INET,SOCK_STREAM,0);
		A.sin_family=AF_INET;
		A.sin_port = htons( port );
		A.sin_addr.s_addr = inet_addr("127.0.0.1");
		ret=bind(http_socket,(struct sockaddr *)&A,sizeof(A));
		if ( !ret ){
			ret=listen( http_socket,25 );
			SpawnHTTPD();
		}
	}
}

#endif

long SendData( long os, const char *data, long len )
{
	long ret=0;
	if ( os )
	{
#ifdef DEF_MAC
		OTSnd((TEndpoint *)os,(void *)data,(unsigned long)len,0) ;
#else
		ret = send( os, data, len, 0 );
#endif		
	}

	return ret;
}

long SendStrHTTP( long os, char *txt )
{
	long ret;
	if ( os )
		ret = SendData( os, txt,strlen(txt));

	return ret;
}







#define	BUFFSIZE	(1024*10)
void SendFile( long socket, char *filename )
{
	FILE *fp;

	if ( fp = fopen( filename, "rb" ) ){
		char *ram = (char *)malloc( BUFFSIZE );
		long dataread;
		while( !feof( fp ) ){
			dataread = fread( ram, 1, BUFFSIZE, fp );
			if ( dataread>0 )
				SendData( socket, ram, dataread );
		}
		fclose(fp);
		free( ram );
	}
}




long MakeHTTPHeader( char *d, char *file, long type )
{
	if ( type >= 0 )
		mystrcpy( d, "HTTP/1.0 200 OK\r\n" );
	else
		mystrcpy( d, "HTTP/1.0 404 ERR\r\n" );
	strcat( d, "Server: Version 4.5 (Windows)\r\n" );
	//strcat( d, "Last-Modified: Thu, 25 Mar 1999 06:14:37 GMT\r\n" );

	if ( type <= 0 ){
		strcat( d, "Accept-Ranges: bytes\r\n" );
		strcat( d, "Connection: close\r\n" );
		strcat( d, "Content-Type: text/html\r\n\r\n\0" );
	} else {
		if (type == 2){
			strcat( d, "Accept-Ranges: bytes\r\n" );
			strcat( d, "Connection: close\r\n" );
			strcat( d, "Content-Type: text/plain\r\n\r\n\0" );
		} else {
			char *ext;
			ext = strrchr( file, '.' );
			strcat( d, "Accept-Ranges: bytes\r\n" );
			strcat( d, "Connection: close\r\n" );
			strcat( d, "Content-Type: images/" );
			strcat( d, ext+1 );
			strcat( d, "\r\n\r\n\0" );
		}	}
	return strlen(d);
}

char *HTMLheader = "<HTML>\
<HEAD>\
   <TITLE>Analysis Reporter</TITLE>\
</HEAD>\
<BODY TEXT=0 BGCOLOR=white>\
<P><BR>\r\n";



// convert any %20 keys to ' '
void ConvertURLtoPlain( char *string )
{
	register char c, c2, *p = string;
	while( (c=*string++) ){
		if ( c == '%' ){
			c = *string++;
			c = mytoupper(c);
			if ( c>= 'A' ) c -= 'A'-10;
			else c -= '0';

			c2 = *string++;
			c2 = mytoupper(c2);
			if ( c2>= 'A' ) c2 -= 'A'-10;
			else c2 -= '0';
			c = c<<4 | c2;
		}
        else
        if ( c == '+' ){
           c = ' ' ;
        }
		*p++ = c;
	}

    *p = 0;
}

// load a file from HD into ram and return pointer
char *LoadFile( char *filename, long length )
{
	FILE *fp;
	char *ram;
	long dataread;

	if ( !length )
		length = (long)GetFileLength( filename );
	if ( fp = fopen( filename, "rb" ) ){
		ram = (char *)malloc( length+16 );
		dataread = fread( ram, 1, length, fp );
		ram[dataread] = 0;
		fclose(fp);
	}
	return ram;
}


typedef struct {
	short type;		//1=string, 2=longnum,3=short,4=radio box, 5=8bit radio, 6=8bit checkbox
	char *name;
	void *data;
} DataMap, *DataMapPtr;

#define NO_TYPE			0
#define CHAR_SET_TYPE	1
#define LONG_SET_TYPE	2
#define SHORT_SET_TYPE	3
#define CHARVAL_SET_TYPE 4
#define SHORTVAL_SET_TYPE 5

DataMap settingsmap[] = {
	//  REPORT SETTINGS
	CHAR_SET_TYPE, "[OUT]", MyPrefStruct.outfile ,
	CHAR_SET_TYPE, "[SITEURL]", MyPrefStruct.siteurl,
	CHAR_SET_TYPE, "[DEFAULT]", MyPrefStruct.defaultindex,
	LONG_SET_TYPE, "[DNSNUM]", &MyPrefStruct.dnsAmount,
	SHORTVAL_SET_TYPE, "[reportformat]", &MyPrefStruct.report_format,
	SHORTVAL_SET_TYPE, "[DNR]", &MyPrefStruct.dnslookup,
	SHORTVAL_SET_TYPE, "[sort]", &MyPrefStruct.sortby,
	6, "[ignoreself]", &MyPrefStruct.ignore_selfreferral,
	6, "[ignorebookmark]", &MyPrefStruct.ignore_bookmarkreferral,
	6, "[filterzero]", &MyPrefStruct.filter_zerobyte,
	6, "[ignorecase]", &MyPrefStruct.ignorecase,
	6, "[usecgi]", &MyPrefStruct.useCGI,
	NO_TYPE,"",0
};



// saving config into output format.
// for a given template file with [keywords] in form objects, replace them
// with actual values/numbers whatever to fill in the data.
long FillInVariables( char *htmltext, char *out )
{
	DataMapPtr p = settingsmap;
	long len = 0;
	char *txt, *vp;
	char numStr[32];
	long *longdata;
	short *shortdata, num;
	char	*chardata;

	while( p->type ){
		if ( txt=strstr( htmltext,p->name ) ){
			switch( p->type ){
				case CHAR_SET_TYPE:
					len = ReplaceStr( htmltext, out, p->name, (char*) p->data, 0 );
					break;
				case LONG_SET_TYPE:
					longdata = (long *)p->data;
					sprintf( numStr, "%d", *longdata );
					len = ReplaceStr( htmltext, out, p->name, numStr, 0 );
					break;
				case 3 :
					shortdata = (short *)p->data;
					sprintf( numStr, "%d", *shortdata );
					len = ReplaceStr( htmltext, out, p->name, numStr, 0 );
					break;
				case 4 :
					shortdata = (short *) p->data;
					if( vp = strstr( txt, "value=" ) ){
						vp+=6;
						if( *vp == 34 ) vp++;
						num = (short)myatoi( vp+6 );
						if ( num == *shortdata )
							len = ReplaceStr( htmltext, out, "unchecked", "  checked", 0 );
					}
					break;
				case SHORTVAL_SET_TYPE:
					chardata = (char *)p->data;
					if( vp = strstr( txt, "value=" ) ){
						vp+=6;
						if( *vp == 34 ) vp++;
						num = (short)myatoi( vp );
						if ( num == *chardata )
							len = ReplaceStr( htmltext, out, "unchecked", "  checked", 0 );
					}
					break;
				case 6 :
					chardata = (char *)p->data;
					if ( *chardata )
						len = ReplaceStr( htmltext, out, "unchecked", "  checked", 0 );
					break;

			}

		}
		p++;
	}
	return len;
}


// for a block of data/html page, replace tokens with variable data
// and copy into a new block of ram.
// Send out this data via socket tcp to client.
long SendRemappedHtml( long os, char *htmldata, long datalen )
{
	char *out, *ram, *data = htmldata;
	char *p,c, foundbracket = 0;
	long len = 0, done=0, outlen = 0;

	ram = (char *)malloc( datalen+12048 );
	p = data;
	out = ram;
	while( c=*data ){
		if ( c == '[' )
			foundbracket++;

		if ( (c == '\n') && foundbracket ){
			*data = 0;
			if( len=FillInVariables( p, out ) ){
				len = strlen(out);
				outlen += len;
				out += len;
				done++;
			} else {
				len = mystrcpy( out, p );
				outlen += len;
				out += len;
				done++;
			}
			foundbracket=0;
			*out++ = '\r';
			outlen++;
			p = data+1;
		}
		data++;
	}
	if ( done ){
		len = mystrcpy( out, p );
		outlen += len;
		out += len;
		done++;
		SendData( os, ram, outlen );
	} else
		SendData( os, htmldata, datalen );

	free( ram );

	return done;
}

long ServeFile( long os, char *name, char *outline )
{
	long d = 0, type , len;

	if( len = (long)GetFileLength( name ) ){
		if ( strstr( name, ".htm" ) )
			type = 0;
		else
			type = 1;

		d = MakeHTTPHeader( outline, name, type );

		SendData( os, outline, d );

		if ( type == 0 ){
			char *ram;
			if( ram = LoadFile( name, len ) ){
				if( SendRemappedHtml( os, ram, len ) == 0 ){
					;//SendData( os, ram, len );
				}
				free( ram );
			}
		} else {
			OutDebug( "Sending raw" );
			SendFile( os, name );
		}
	}
	return d;
}

#define	ROOTPATH "http_config/"

void GetURLPath( char *url, char *path )
{
	char tmp[512];
	sprintf( tmp, "%s\\http_config\\", gPath );

	if ( *url == 0 )
		CopyFilenameUsingPath( path, tmp, "index.html" );
	else
		CopyFilenameUsingPath( path, tmp, url );
}


// simulate a process dialog using tables that refresh ver often

void ProcessShowProgressCGI( long os, char *url, char *buffer )
{
#ifndef DEF_MAC
	long failed = 0;
	char *log = strstr( url, "log=" );

	if ( !gProcessing ){
	}
	{
		char tmp[256], *ram;
		long len;

		MakeHTTPHeader( tmp, NULL, 0 );

		SendData( os, tmp, 0 );

		GetURLPath( "process.html", tmp );
		if( ram = LoadFile( tmp, 0 ) ){
			if ( gProcessing )
				sprintf( tmp, "%d; URL=/showprogress.cgi", 1 );
			else
				sprintf( tmp, "%d; URL=/report/", 2 );	// when processing is done, show report

			ReplaceStr( ram, buffer, "[repeat_time]", tmp, 0 );

			sprintf( tmp, "%d %% Processed...", GetLastPercentProgress() );
			ReplaceStr( buffer, 0, "[percent]", tmp, 0 );

			sprintf( tmp, "%d%%", GetLastPercentProgress() );
			ReplaceStr( buffer, 0, "[width]", tmp, 0 );
			free( ram );
		}
		len = strlen(buffer);
		SendData( os, buffer, len );
		sprintf( tmp, "%d len, %s", len, buffer+len-7 );
		//OutDebug( tmp );
	}
#endif
}

// lets start processing
void ProcessCGI( long os, char *url, char *buffer )
{
#ifndef DEF_MAC
	long failed = 0;
	char *log = strstr( url, "log=" );

	if ( !gProcessing ){
		if ( log )
			AddFileToLogQ( log+4, 0 );

		if ( !ProcessLog(0) ){
			SysBeep(1);
			StatusSetID( IDS_FAILED );
			gProcessing = FALSE;
			failed = 1;
		} else {
			StatusSetID( IDS_PROCESSING );
			gProcessing = TRUE;
			failed = 0;
		}
	}
	ProcessShowProgressCGI( os, url, buffer );
#endif
}




// ------------------------------------- INTERFACE LEVEL CODE HANDLING -----------------------------


#define	SENDHTML(x)		MakeHTTPHeader( outline, NULL, 0 ); strcat( outline, x ); d = SendStrHTTP( os, outline )




// ------------------------------------- CGI/PATH LEVEL CODE HANDLING -----------------------------



// handle incoming url requests, decide weather to serve a file or do internal 
// html page generation

long HandleHttpURL( long os, char *line )
{
	long	out=0,d=0,len;
	char	*outline, tmp[400];
	char	*p;


	if ( !strcmpd( "GET", line ) ){
		outline = (char *) malloc( 65365 );

		if ( p = mystrchr( line+4, ' ' ) )
			*(p) = 0;

/*		if ( !strcmpd( "/index.html", line+4 ) || !strcmpd( "/ ", line+4 ) ){
			ShowRecievedLine( "SEND", "index" );
			MakeHTTPHeader( outline, 0 );
			strcat( outline, HTMLindex );
			d = SendStrHTTP( os, outline );
		} else
		if ( !strcmpd( "/menu.html", line+4 ) ){
			ShowRecievedLine( "SEND", "menu" );
			MakeHTTPHeader( outline, 0 );
			strcat( outline, HTMLmenu );
			d = SendStrHTTP( os, outline );
		} else
*/
		if ( !strcmpd( "/query?", line+4 )  ){
// 			MakeHTTPHeader( outline, NULL, 2 );
//			d = SendStrHTTP( os, outline );

			ProcessHTTPQuery(os, line+11);
			d = 1;
		} else
		if ( !strcmpd( "/ver", line+4 )  ){
			MakeHTTPHeader( outline, NULL, 0 );
			strcat( outline, HTMLheader );
			strcat( outline, "<H3>Version information</H3><HR><BR>" );
			sprintf( tmp, "%s (socket=%08lx<BR>",VERSION_STRING, os );
			strcat( outline, tmp );
			d = SendStrHTTP( os, outline );
		} else
		if ( !strcmpd( "/logs", line+4 )  ){
			char *log; long fl;
			MakeHTTPHeader( outline, NULL, 0 );
			strcat( outline, HTMLheader );
			strcat( outline, "<H3>Current Logs in history</H3><BR><HR><PRE>" );
			d = 0;
			while( (log = FindLogXinHistory(d)) ){
				fl = (long)GetFileLength( log );
				sprintf( tmp, "<B>%-2d </B>%-10.1fKb   <A HREF=process.cgi?log=%s>%s</A><BR>", d, fl/1024.0, log, log );
				strcat( outline, tmp );
				d++;
			}
			strcat( outline, "</PRE><HR><BR>" );
			d = SendStrHTTP( os, outline );
		} else
		// convert the cgi type url with post commands to a preferences style
		// lines, ie, one command per line.
		if ( !strcmpd( "/savesettings.cgi", line+4 )  ){
			char *d, *p;
			char *argv[6];
			p = mystrchr( line, '?' );
			while( p ){
				if ( (d = mystrchr( p, '&' )) ) *d = 0;

				if( argv[0] = p )	ConvertURLtoPlain( argv[0] );
				argv[1] = mystrchr( p, '=' );
				if ( argv[1] ){
					ConvertURLtoPlain( argv[1] );
					ProcessPrefsLine( 2, argv, 0, 0, &MyPrefStruct );
				} else
					ProcessPrefsLine( 1, argv, 0, 0, &MyPrefStruct );

				if ( d ) p = d+1; else p = 0;
			}
			MakeHTTPHeader( outline, NULL, 0 );
			strcat( outline, HTMLheader );
			d = (char*)SendStrHTTP( os, outline );
		} else
		if ( !strcmpd( "/livereport/", line+4 )  ){
#ifndef DEF_MAC
			if ( (MyPrefStruct.live_sleeptype > 0) ){
				 d = LiveViewReport( 0, os );
			} else {
				GetURLPath( "nolive.html", tmp );
				ServeFile( os, tmp, outline );
			}
#endif
		} else
		if ( !strcmpd( "/report/", line+4 )  ){
			if ( line[12] )
				CopyFilenameUsingPath( tmp, MyPrefStruct.outfile, line+12 );
			else
				mystrcpy( tmp, MyPrefStruct.outfile );

			if( len = (long)GetFileLength( tmp ) ){
				if ( strstr( tmp, ".htm" ) ){
					d = MakeHTTPHeader( outline, tmp, 0 );
				} else {
					d = MakeHTTPHeader( outline, tmp, 1 );
				}
				SendData( os, outline, d );
				SendFile( os, tmp );
				//FileToMem( tmp, fileram+d, len );
				//SendData( os, fileram, len+d );
				
			}
		} else
		if ( !strcmpd( "/process.cgi", line+4 )  ){
			ProcessCGI( os , line+4, outline );
			d = 1;
		} else
		if ( !strcmpd( "/showprogress.cgi", line+4 )  ){
			ProcessShowProgressCGI( os , line+4, outline );
			d = 1;
		} else
		{
			GetURLPath( line+5, tmp );
			//StatusSet( tmp );
			if( ServeFile( os, tmp, outline ) )
				d = 1;
		}

		if ( !d ){
			len = MakeHTTPHeader( outline, NULL, -1 );
			sprintf( outline+len, "<H3>Error 404, file not found</H3><HR><BR>%s", line+4);
			d = SendStrHTTP( os, outline );
		}

		out = 1;
		free(outline);
	}
	return out;
}


#ifdef DEF_MAC


//  MacOS Compatible Mini WebServer using OpenTransport
//
//
//
/////////////////////////////////////////////////////////////////////
// Carbon.h

/////////////////////////////////////////////////////////////////////

char line[16384] ;

Boolean gQuitNow = false;

extern Boolean gShowErrorDialogs ;

extern OSStatus DoNegotiateIPReuseAddrOption(EndpointRef ep, Boolean enableReuseIPMode);

/////////////////////////////////////////////////////////////////////

void MoreAssertQ(Boolean mustBeTrue)
{
    if ( ! mustBeTrue ) {
        DebugStr("\pMoreAssertQ: Assertion failure.");
    }
}

OTNotifyUPP gYieldingNotifierUPP = NULL;
static pascal void YieldingNotifier(EndpointRef ep, OTEventCode code, 
									   OTResult result, void* cookie)
	// This simple notifier checks for kOTSyncIdleEvent and
	// when it gets one calls the Thread Manager routine
	// YieldToAnyThread.  Open Transport sends kOTSyncIdleEvent
	// whenever it's waiting for something, eg data to arrive
	// inside a sync/blocking OTRcv call.  In such cases, we
	// yield the processor to some other thread that might
	// be doing useful work.
	//
	// The routine also checks the gQuitNow boolean to see if the
	// the host application wants us to quit.  This roundabout technique
	// avoids a number of problems including:
	//
	// 1. Threads stuck inside OT synchronous calls -- You can't just
	//    call DisposeThread on a thread that's waiting for an OT
	//    synchronous call to complete.  Trust me, it would be bad!
	//    Instead, this routine calls OTCancelSynchronousCalls to get
	//    out of the call.  The given error code (userCanceledErr) 
	//    propagates out to the caller, which causes the calling
	//    thread to eventually terminate.
	// 2. Threads holding resources -- You can't just DisposeThread
	//    a networking thread because it might be holding resouces,
	//    like memory or endpoints, that need to be cleaned up properly.
	//    Cancelling the thread in this way causes the thread's own
	//    code to clean up those resources just like it would for any
	//    any other error.
	//
	// I could have used a more sophisticated mechanism to support
	// quitting (such as a boolean per thread, or returning some
	// "thread object" to which the application can send a "cancel"
	// message, but this way is easy and works just fine for this
	// simple sample
{
	#pragma unused(result)
	#pragma unused(cookie)
	OSStatus junk;
	
	switch (code) {
		case kOTSyncIdleEvent:
			junk = YieldToAnyThread();
			MoreAssertQ(junk == noErr);
			
			if (gQuitNow) {
				junk = OTCancelSynchronousCalls(ep, userCanceledErr);
				MoreAssertQ(junk == noErr);
			}
			break;
		default:
			// do nothing
			break;
	}
}

/////////////////////////////////////////////////////////////////////

static void SetDefaultEndpointModes(EndpointRef ep)
	// This routine sets the supplied endpoint into the default
	// mode used in this application.  The specifics are:
	// blocking, synchronous, and using synch idle events with
	// the standard YieldingNotifier.
{
	OSStatus junk;
	 
	
	if (gYieldingNotifierUPP == NULL)
	{
		gYieldingNotifierUPP = NewOTNotifyUPP((OTNotifyProcPtr)YieldingNotifier);
	}
	
	junk = OTSetBlocking(ep);
	MoreAssertQ(junk == noErr);
	junk = OTSetSynchronous(ep);
	MoreAssertQ(junk == noErr);
	junk = OTInstallNotifier(ep, gYieldingNotifierUPP, ep);
	MoreAssertQ(junk == noErr);
	junk = OTUseSyncIdleEvents(ep, true);
	MoreAssertQ(junk == noErr);
}

/////////////////////////////////////////////////////////////////////

static OSStatus OTSndQ(EndpointRef ep, void *buf, size_t nbytes)
	// My own personal wrapper around the OTSnd routine that cleans
	// up the error result.
{
	OTResult bytesSent;
	
	bytesSent = OTSnd(ep, buf, nbytes, 0);
	if (bytesSent >= 0) {
	
		// Because we're running in synchronous blocking mode, OTSnd
		// should not return until it has sent all the bytes unless it
		// gets an error.  If it does, we want to hear about it.
		MoreAssertQ(bytesSent == nbytes);
	
		return (noErr);
	} else {
		return (bytesSent);
	}
}

/////////////////////////////////////////////////////////////////////

static OSErr FSReadQ(short refNum, long count, void *buffPtr)
	// My own wrapper for FSRead.  Whose bright idea was it for
	// it to return the count anyway!
{
	OSStatus err;
	long tmpCount;
	
	tmpCount = count;
	err = FSRead(refNum, &count, buffPtr);
	
	MoreAssertQ((err != noErr) || (count == tmpCount));
	
	return (err);
}

/////////////////////////////////////////////////////////////////////

static Boolean StringHasSuffix(const char *str, const char *suffix)
	// Returns true if the end of str is suffix.
{
	Boolean result;
	
	result = false;
	if ( OTStrLength(str) >= OTStrLength(suffix) ) {
		if ( OTStrEqual(str + OTStrLength(str) - OTStrLength(suffix) , suffix) ) {
			result = true;
		}
	}
	
	return (result);
}

static OSStatus ExtractRequestedFileName(const char *buffer,
											char *fileName, char *mimeType)
	// Assuming that buffer is a C string contain an HTTP request,
	// extract the name of the file that's being requested.
	// Also check to see if the file has one of the common suffixes,
	// and set mimeType appropriately.
	//
	// Obviously this routine should use Internet Config to
	// map the file type/creator/extension to a MIME type,
	// but I don't want to complicate the sample with that code.
{
	OSStatus err;
	
	// Default the result to empty.
	fileName[0] = 0;
	
	// Scan the request looking for the fileName.  Obviously this is not
	// a very good validation of the request, but this is an OT sample,
	// not an HTTP one.  Also note that we require HTTP/1.0, but some
	// ancient clients might just generate "GET %s<cr><lf>"
	
	(void) sscanf(buffer, "GET %s HTTP/1.0", fileName);
	
	// If the file name is still blank, scanf must have failed.
	// Note that I don't rely on the result from scanf because in a
	// previous life I learnt to mistrust it.
	
	if (fileName[0] == 0) {
		err = -1;
	} else {
	
		// So the request is cool.  Normalise the file name.
		// Requests for the root return "index.html".
		
		if ( OTStrEqual(fileName, "/") ) {
			OTStrCopy(fileName, "index.html");
		}
		
		// Remove the prefix slash.  Note that we don't deal with
		// "slashes" embedded in the fileName, so we don't handle
		// any directories other than the root.  This would be
		// easy to do, but again this is not an HTTP sample.
		
		if ( fileName[0] == '/' ) {
			BlockMoveData(&fileName[1], &fileName[0], OTStrLength(fileName));
		}
	
		// Set mimeType based on the file's suffix.
		
		if ( StringHasSuffix(fileName, ".html") ) {
			OTStrCopy(mimeType, "text/html");
		} else if ( StringHasSuffix(fileName, ".gif") ) {
			OTStrCopy(mimeType, "image/gif");
		} else if ( StringHasSuffix(fileName, ".jpg") ) {
			OTStrCopy(mimeType, "image/jpeg");
		} else {
			OTStrCopy(mimeType, "text/plain");
		}
		err = noErr;
	}
	
	#ifdef qDebug
		printf("ExtractRequestedFileName: Returning %d, “%s”, “%s”\n", err, fileName, mimeType);
	#endif
	
	return (err);
}

/////////////////////////////////////////////////////////////////////

// The worker thread reads characters one at a time from the endpoint
// and uses the following state machine to determine when the request is
// finished.  For HTTP/1.0 requests, the request is terminated by
// two consecutive CR LF pairs.  Each time we read one of the appropriate
// characters we increment the state until we get to kDone, at which
// point we go off to process the request.

enum {
	kWorkerWaitingForCR1,
	kWorkerWaitingForLF1,
	kWorkerWaitingForCR2,
	kWorkerWaitingForLF2,
	kWorkerDone
};

// This is the size of the transfer buffer that each worker thread
// allocates to read file system data and write network data.

enum {
	kTransferBufferSize = 4096
};

// WorkerContext holds the information needed by a worker endpoint to
// operate.  A WorkerContext is created by the listener endpoint
// and passed as the thread parameter to the worker thread.  If the
// listener successfully does this, it's assumed that the worker
// thread has taken responsibility for freeing the context.

struct WorkerContext {
	EndpointRef worker;
	short vRefNum;
	long dirID;
};
typedef struct WorkerContext WorkerContext, *WorkerContextPtr;

struct ServerContext {
   long portNum;
   };
typedef struct ServerContext ServerContext, *ServerContextPtr ;

// The two buffers hold standard HTTP responses.  The first is the 
// default text we spit out when we get an error.  The second is
// the header that we use when we successfully field a request.
// Again note that this sample is not about HTTP, so these responses
// are probably not particularly compliant to the HTTP protocol.

static char gDefaultOutputText[] = "HTTP/1.0 200 OK\15\12Content-Type: text/html\15\12\15\12<H1>Say what!</H1><P>\15\12Error Number (%d), Error Text (%s)";
static char gHTTPHeader[] = "HTTP/1.0 200 OK\15\12Content-Type: %s\15\12\15\12";



/////////////////////////////////////////////////////////////////////

static OSStatus ReadHTTPRequest(EndpointRef worker, char *buffer)
	// This routine reads the HTTP request from the worker endpoint,
	// using the state machine described above, and puts it into the
	// indicated buffer.  The buffer must be at least kTransferBufferSize
	// bytes big.
	//
	// This is pretty feeble
	// code (reading data one byte at a time is bad for performance),
	// but it works and I'm not quite sure how to fix it.  Perhaps
	// OTCountDataBytes?
	//
	// Also, the code does not support with requests bigger than
	// kTransferBufferSize.  In practise, this isn't a problem.
{
	OSStatus err;
	long bufferIndex;
	int state;
	char ch;
	OTResult bytesReceived;
	OTFlags junkFlags;
	
	MoreAssertQ(worker != nil);
	MoreAssertQ(buffer != nil);
	
	bufferIndex = 0;
	state = kWorkerWaitingForCR1;
	do {	
		bytesReceived = OTRcv(worker, &ch, sizeof(char), &junkFlags);
		if (bytesReceived >= 0) {
			MoreAssertQ(bytesReceived == sizeof(char));
			
			err = noErr;

			// Put the character into the buffer.
			
			buffer[bufferIndex] = ch;
			bufferIndex += 1;
			
			// Check that we still have space to include our null terminator.
			
			if (bufferIndex >= kTransferBufferSize) {
				err = -1;
			}
			
			// Do the magic state machine.  Note the use of
			// hardwired numbers for CR and LF.  This is correct
			// because the Internet standards say that these
			// numbers can't change.  I don't use \n and \r
			// because these values change between various C
			// compilers on the Mac.
			
			switch (ch) {
				case 13:
					switch (state) {
						case kWorkerWaitingForCR1:
							state = kWorkerWaitingForLF1;
							break;
						case kWorkerWaitingForCR2:
							state = kWorkerWaitingForLF2;
							break;
						default:
							state = kWorkerWaitingForCR1;
							break;
					}
					break;
				case 10:
					switch (state) {
						case kWorkerWaitingForLF1:
							state = kWorkerWaitingForCR2;
							break;
						case kWorkerWaitingForLF2:
							state = kWorkerDone;
							break;
						default:
							state = kWorkerWaitingForCR1;
							break;
					}
					break;
				default:
					state = kWorkerWaitingForCR1;
					break;
			}
		} else {
			err = bytesReceived;
		}
	} while ( err == noErr && state != kWorkerDone );

	if (err == noErr) {
		// Append the null terminator that turns the HTTP request into a C string.
		buffer[bufferIndex] = 0;
	}

	return (err);		
}

/////////////////////////////////////////////////////////////////////

static OSStatus CopyFileToEndpoint(const FSSpec *fileSpec, char *buffer, EndpointRef worker)
	// Copy the file denoted by fileSpec to the endpoint.  buffer is a
	// temporary buffer of size kTransferBufferSize.  Initially buffer
	// contains a C string that is the HTTP header to output.  After that,
	// the routine uses buffer as temporary storage.  We do this because
	// we want any errors opening the file to be noticed before we send
	// the header saying that the request went through successfully.
{
	OSStatus err;
	OSStatus junk;
	long bytesToSend;
	long bytesThisTime;
	short fileRefNum;
	
	err = FSpOpenDF(fileSpec, fsRdPerm, &fileRefNum);
	if (err == noErr) {
		err = GetEOF(fileRefNum, &bytesToSend);
		
		// Write the HTTP header out to the endpoint.
		
		if (err == noErr) {
			err = OTSndQ(worker, buffer, OTStrLength(buffer));
		}
		
		// Copy the file in kTransferBufferSize chunks to the endpoint.
		
		while (err == noErr && bytesToSend > 0) {
			if (bytesToSend > kTransferBufferSize) {
				bytesThisTime = kTransferBufferSize;
			} else {
				bytesThisTime = bytesToSend;
			}
			err = FSReadQ(fileRefNum, bytesThisTime, buffer);
			if (err == noErr) {
				err = OTSndQ(worker, buffer, bytesThisTime);
			}
			bytesToSend -= bytesThisTime;
		}
		
		// Clean up.
		junk = FSClose(fileRefNum);
		MoreAssertQ(junk == noErr);
	}
	
	return (err);
}


static OSStatus OrderlyDisconnect(EndpointRef ep)
	// Gosh XTI is lame.  RcvOrderlyDisconnect is non-blocking 
	// (it doesn't wait for the T_ORDREL event) so we have to 
	// block in a Rcv call.  This is standard XTI practice.
{
	OSStatus 	err;
	UInt8 		tmp;
	OTFlags 	junkFlags;
	OTResult 	look;
	
	err = OTSndOrderlyDisconnect(ep);
	if (err == noErr) {
		err = OTRcv(ep, &tmp, sizeof(tmp), &junkFlags);
		if (err == kOTLookErr) {
			look = OTLook(ep);
			switch (look) {
				case T_ORDREL:
					err = OTRcvOrderlyDisconnect(ep);
					break;
				default:
					// leave err set to kOTLookErr
					break;
			}
		} else if (err == noErr) {
			err = kOTLookErr;			// something is happening, but it's not a disconnect
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////////
ThreadEntryUPP gWorkerThreadProcUPP = NULL;
static pascal OSStatus WorkerThreadProc(WorkerContextPtr context)
	// This routine is the starting routine for the worker thread.
	// The thread is responsible for reading an HTTP request from
	// an endpoint, processing the requesting and writing the results
	// back to the endpoint.
{
	OSStatus err;
	OSStatus junk;
	char *buffer ;
	
	
	MoreAssertQ(context != nil);
	MoreAssertQ(context->worker != nil);

	// Allocate the transfer buffer in the heap.
	
	err = noErr;
	buffer = (char *) OTAllocMemInContext(kTransferBufferSize, NULL);
	if (buffer == nil) {
		err = kENOMEMErr;
	}

	// Read the request into the transfer buffer.
	
	if (err == noErr) {
		err = ReadHTTPRequest(context->worker, buffer);
	}
	
	if (err == noErr) {

		HandleHttpURL( (long) context->worker, buffer );

	}
	
	// Shut down the endpoint and clean up the WorkerContext.
	
        if (err == noErr) {
		err = OrderlyDisconnect(context->worker);
	}
        
	junk = OTCloseProvider(context->worker);
	MoreAssertQ(junk == noErr);
	
	OTFreeMem(context);

	if (buffer != nil) {
		OTFreeMem(buffer);
	}


    if (gShutdownRequestedViaHTTP) gDone = true ;
	
	return (noErr);
}

/////////////////////////////////////////////////////////////////////
OSStatus RunHTTPServer(InetHost ipAddr, int portNum) ;

ThreadEntryUPP gServerThreadProcUPP = NULL;

static   ServerContext sContext ;

static pascal OSStatus HTTPServerThreadProc(ServerContextPtr context)
{
  RunHTTPServer(kOTAnyInetAddress, context->portNum) ;
  
  return(noErr) ;
}

void InitHttpServer( short portNum )
{
  ThreadID HTTPServerThread ; 
  
  Boolean oldShow = gShowErrorDialogs ;
  gShowErrorDialogs = false ;
  sContext.portNum = portNum ;
  
  InitOpenTransportInContext (kInitOTForApplicationMask, NULL);

  // Spawn a new thread with a HTTP server in it
  gServerThreadProcUPP = NewThreadEntryUPP((ThreadEntryProcPtr)HTTPServerThreadProc);
  NewThread(kCooperativeThread, gServerThreadProcUPP, &sContext,
						0, kCreateIfNeeded,
						nil,
						&HTTPServerThread);
//  SetThreadState(HTTPServerThread, kReadyThreadState, kNoThreadID);
  gShowErrorDialogs = oldShow ;

}

///

OSStatus RunHTTPServer(InetHost ipAddr, int portNum)
	// This routine runs an HTTP server.  It doesn't return until
	// someone sets gQuitNow, so you should most probably call this
	// routine on its own thread.  ipAddr is the IP address that
	// the server listens on.  Specify kOTAnyInetAddress to listen
	// on all IP addresses on the machine; specify an IP address
	// to listen on a specific address.  vRefNum and dirID point
	// to the root directory of the HTTP information to be served.
	//
	// The routine creates a listening endpoint and listens for connection 
	// requests on that endpoint.  When a connection request arrives, it creates 
	// a new worker thread (with accompanying endpoint) and accepts the connection
	// on that thread.
	//
	// Note the use of the "tilisten" module which prevents multiple
	// simultaneous T_LISTEN events coming from the transport provider,
	// thereby greatly simplifying the listen/accept sequence.
{
	OSStatus err;
	EndpointRef listener;
	TBind bindReq;
	InetAddress ipAddress;
	InetAddress remoteIPAddress;
	TCall call;
	ThreadID workerThread;
	OSStatus junk;
	WorkerContextPtr workerContext;
	TEndpointInfo Info;
	char	buf[128];
	
	OTInetHostToString(ipAddr, buf);

	listener = OTOpenEndpointInContext(OTCreateConfiguration("tilisten,tcp"), 0, &Info, &err, NULL);
	
	if (err == noErr) {
		junk = DoNegotiateIPReuseAddrOption(listener, true);
		MoreAssertQ(junk == noErr);
	}	
	
	if (err == noErr) {
		SetDefaultEndpointModes(listener);
		OTInitInetAddress(&ipAddress, portNum, ipAddr);	// port & host ip
		bindReq.addr.buf = (UInt8 *) &ipAddress;
		bindReq.addr.len = sizeof(ipAddress);
		bindReq.qlen = 1;
		err = OTBind(listener, &bindReq, nil);
	}
	
	while (err == noErr) {

		OTMemzero(&call, sizeof(TCall));
		call.addr.buf = (UInt8 *) &remoteIPAddress;
		call.addr.maxlen = sizeof(remoteIPAddress);
		err = OTListen(listener, &call);

		// ... then spool a worker thread for this connection.
		
		if (err == noErr) {
		
			// Create the worker context.
		
			workerThread = kNoThreadID;
			workerContext = (WorkerContext *)OTAllocMemInContext(sizeof(WorkerContext), NULL);
			if (workerContext == nil) {
				err = kENOMEMErr;
			} else {
				workerContext->worker = nil;
//				workerContext->vRefNum = vRefNum;
//				workerContext->dirID = dirID;
			}
			
			// Open the worker endpoint.
			
			if (err == noErr) {
				workerContext->worker = OTOpenEndpointInContext(OTCreateConfiguration("tcp"), 0, nil, &err, NULL);
				if (err == noErr) {
					SetDefaultEndpointModes(workerContext->worker);
				}
			}
			
			
			if (gWorkerThreadProcUPP == NULL)
			{
				gWorkerThreadProcUPP = NewThreadEntryUPP((ThreadEntryProcPtr)WorkerThreadProc);
			}
			
			// Create the worker thread.
			if (err == noErr) {
				err = NewThread(kCooperativeThread, gWorkerThreadProcUPP, workerContext,
								0, kNewSuspend | kCreateIfNeeded,
								nil,
								&workerThread);
			}
			
			// Accept the connection on the thread.
			
			if (err == noErr) {
				err = OTAccept(listener, workerContext->worker, &call);
			}
			
			// Schedule the thread for execution.
			
			if (err == noErr) {
				err = SetThreadState(workerThread, kReadyThreadState, kNoThreadID);
			}
			
			// Clean up on error.
			
			if (err != noErr) {
				if (workerContext != nil) {
					if (workerContext->worker != nil) {
						junk = OTCloseProvider(workerContext->worker);
						MoreAssertQ(junk == noErr);
					}
					OTFreeMem(workerContext);
				}
				if (workerThread != kNoThreadID) {
					junk = DisposeThread(workerThread, nil, true);
					MoreAssertQ(junk == noErr);
				}

				err = noErr;
			}
		}
	}
	
	// Clean up the listener endpoint.
	
	if (listener != nil) {
		junk = OTCloseProvider(listener);
		MoreAssertQ(junk == noErr);
	}

	
	return (err);
}


#endif



