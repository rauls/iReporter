#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <netdb.h>

#include "myansi.h"
#include "remotecontrol.h"
#include "main.h"


long HandleTCPMessage( long msg , long os );


long WSInit( void ) { return 1; }


static struct sockaddr_in A,Peer;
static int s=0,peersock=0;
static int dw;

long InitFWServer( long port )
{
	long ret=-1, len;
	if( !s ){
		s=socket(AF_INET,SOCK_STREAM,0);
		if ( s<0 ) { perror("cant init socket\r\n"); return s; }
		A.sin_family=AF_INET;
		A.sin_port = htons( port );
		A.sin_addr.s_addr = INADDR_ANY;
		ret = bind(s,(struct sockaddr *)&A,sizeof(A));
		if ( ret ) { perror( "bind failed\r\n"); return ret; }
		ret = listen( s, 5 );
		if ( ret ) { perror( "listen failed\r\n"); return ret; }
	}
	return ret;
}

#define	FD_ACCEPT	8
#define	FD_READ		16
#define	FD_CLOSE	32


void CloseFWServer( void )
{
	if( s )
	{
		shutdown(s,2);
		closesocket(s);
		s = 0;
		sleep(1);
	}
}
void CloseFWPeer( void )
{
	if( peersock )
	{
		int er;
		if ( shutdown(peersock,2) == -1 )
			perror( "shutdown failed" );
		close(peersock);
		peersock = 0;
		sleep(1);
	}
}


void DoDaemonMode( void )
{
	long run=1, ret;
	
	ret = InitFWServer( 911 );
	if ( ret ) return;

	while( run ){
		HandleTCPMessage( FD_ACCEPT, 0 );
		while( peersock>0 ){
			HandleTCPMessage( FD_READ, peersock );
		}
		HandleTCPMessage( FD_CLOSE, peersock );
		sleep(1);
	}
}




char cmdHelp[] = {
	"help        settings command list\r\n"
	"?           control command list\r\n"
	"ver         query version\r\n"
	"quitapp     quit application\r\n"
	"!           run [FILE]\r\n"
	"proclog     process log file\r\n"
	"stoplog     stop processing\r\n"
	"log X       add log X to process-list\r\n"
	"host        list host/peer ips\r\n"
	"setprogress set progress stat bar %%\r\n"
	"getconfig   get remote config\r\n"
	"showlogs    show all logs in memory\r\n"
	"addlog	     add log to servers memory\r\n"
	"clearlog    clear memory of logs\r\n"
	"logout      exit session\r\n"
};

char	ioBuffer[20000];
long	msgcount, passwd=0;


long SendStr( long os, char *txt )
{
	//printf( "send:%s\r\n", txt );
	if ( s )
		return send( os, txt,strlen(txt),0 );
	else
		return -1;
}


void RemoteStatusSetText( char *message )
{
	sprintf( ioBuffer, "status %s\r\n", message );
	SendStr( peersock, ioBuffer );
}


long HandleServerMessage( long os, char *line )
{
	long	out=0,d;
	char	tline[256];
static	int err=0;

	if ( !strcmpd( "logout", line ) ){
		d = SendStr( os, "Bye!\r\n" );
		CloseFWPeer();
		out = 1;
	} else
	if ( !passwd ){
		char *p = line;
		if ( p ){
			if ( !strcmpd( MyPrefStruct.remotelogin_passwd, p ) )
				passwd = 1;
			else
			if ( err<5 ){
				err++;
				d = SendStr( os, "Login incorrect\r\n" );
				return out;
			}
		}
		if ( !passwd ){
			d = SendStr( os, "Login incorrect, Bye!\r\n" );
			sleep(1);
			CloseFWPeer();
			out = 1;
		} else {
			d = SendStr( os, "Granted!\r\n" );
		}
		err = 0;
	} else
	if ( passwd ){
		if ( !strcmpd( "getconfig", line ) ){
			char *p = cmdHelp; FILE *fp; long len;
			if ( (fp = fopen( "netconfig.txt", "w+" ))) {
				DoSaveAsciiPrefs( fp, &MyPrefStruct );
				fseek( fp, 0, SEEK_SET );
				while( !feof( fp ) ){
					fgets( tline, 256, fp );
					d=SendStr(os,tline);
					if ( d == -1 ) break;
				}
				fclose(fp);
				RemoteStatusSetText( "0% Ok" );
			}
		} else
		if ( !strcmpd( "?", line ) ){
			d=SendStr(os,cmdHelp);
		} else
		if ( !strcmpd( "help", line ) ){
			char **helptext;
			long i = 0;

			helptext = GetHelpText();

			while( helptext[i] ){
				d=SendStr( os, helptext[i] );
				i++;
			}
			//d=SendStr(os,GetHelpText());
		} else
		if ( !strcmpd( "ver", line ) ){
			d = SendStr( os, "iReporter v5.0 (Unix)\r\n\r\n" );
		} else

		if ( !strcmpd( "clearlog", line ) ){
			glogFilenamesNum = 0;
		} else
		if ( !strcmpd( "proclog", line ) ){
			d = SendStr( os, "Processing..." );
			ProcessLog( 1 );
		} else
		if ( !strcmpd( "stoplog", line ) ){
			d = SendStr( os, "Stopping..." );
		} else
		if ( !strcmpd( "quitapp", line ) ){
			d = SendStr( os, "Quit." );
			exit(0);
		} else
		if ( !strcmpd( "setprogress", line ) ){
			char *p = mystrchr( line, ' ' );
			if ( p ) {
				StatusSet( "#" );
			}
		} else
		if ( !strcmpd( "showlogs", line ) ){
			FILE *fp; char name[512];
			system( "find ../* -name \"*.log*\" > loglist.dat" );
			system( "find /etc/apache/* -name \"*access_log\" >> loglist.dat" );
			system( "find /etc/apache/* -name \"*.log\" >> loglist.dat" );
			if ( (fp = fopen( "loglist.dat", "r" ))) {
				while( !feof( fp ) ){
					fgets( name, 511, fp );
					sprintf( tline, "addlog %s", name);
					d=send(os,tline,strlen(tline),0);
					if ( d == -1 ) printf("cant send\r\n" );
					//d=recv(os,ioBuffer,2000,0);
				}
				fclose(fp);
			}
		} else
		if ( !strcmpd( "host", line ) ){
			char hostname[256];
			gethostname( hostname, 256 );
			sprintf( tline,"Host is %s\r\n", hostname);
			d=SendStr(os,tline);

			iplongtoipstring( (long)Peer.sin_addr.s_addr, hostname );
			sprintf( tline,"Peer is %s\r\n\r\n", hostname);
			d=SendStr(os,tline);
		} else 
		if ( *line == '!' ){
			char *p = mystrchr(line,' ');
			if ( p ) p++;
			system( line );
		} else 
		if ( *line != '#' ){
			char *argv[64] ,*p; int argc;

			mystrcpy( tline, line );
		 	p = line; argc=0;
		 	memset( argv, 0, 63*sizeof(void*) );
		 	while( (p=strtok( p, " " )) && (argc<64) ){
		 		argv[ argc++ ] = p;
		 		p=NULL;
		 	}
		 	ProcessPrefsLine( argc, argv, tline, 0, &MyPrefStruct );
		}
	}
	return out;
}


long GetPeerIP( void )
{
	if ( msgcount ){
		return (long)Peer.sin_addr.s_addr;
	} else
		return 0;
}

long GetHostIP( void )
{
	if ( msgcount ){
		return (long)A.sin_addr.s_addr;
	} else
		return 0;
}


long HandleTCPMessage( long z , long os )
{
	int peer_len, peerip=0, ret; char *p, line[256];

    if (z) {
        if ( ((z)&FD_ACCEPT) == FD_ACCEPT){
			char ipStr[32];
			peer_len=sizeof(Peer);
			peersock=accept(s,(struct sockaddr *) &Peer,&peer_len);
			peerip = (long)Peer.sin_addr.s_addr;
			iplongtoipstring( peerip, ipStr );
			sprintf( ioBuffer, "Accepting %s ...", ipStr );
			StatusSet( ioBuffer );

			sprintf( ioBuffer,"Welcome %s\r\nLogin: ", ipStr);
			ret=send(peersock,ioBuffer,strlen(ioBuffer),0);

			msgcount = 1;
			passwd = 0;
        }
        if ( ((z)&FD_READ) == FD_READ){
			ret = recv(os,ioBuffer,2000,0);
			if ( ret > 0 ){
				ioBuffer[ret] = 0;

				p = ioBuffer;
				while( p ){
					p = CopyLine( line, p );
					printf( "rec:" );
					StatusSet( line );
					if ( HandleServerMessage( os, line ) )
						msgcount=0;
					else
						msgcount++;
				}
			} else z = FD_CLOSE;
        }
        if ( ((z)&FD_CLOSE) == FD_CLOSE ){
        	CloseFWPeer();
			peerip = 0;
			msgcount=0;
		}
    }
	return msgcount;
}








