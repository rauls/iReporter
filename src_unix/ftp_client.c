/**********************************************************************/
/* Copyright (c) July 1996 Robert M. Jansen                           */
/*                                                                    */
/* Redistribution and use in source and binary forms, with or without */
/* modification, are permitted provided that the following condition  */
/* is met:                                                            */
/*                                                                    */
/* Redistributions of source code must retain the above copyright     */
/* notice, this list of conditions and the following disclaimer.      */
/*                                                                    */
/* THIS SOFTWARE IS PROVIDED BY Robert M. Jansen "AS IS" AND ANY      */
/* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR */
/* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Robert M. Jansen BE      */
/* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,   */
/* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,           */
/* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR */
/* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY       */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR     */
/* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF */
/* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF    */
/* SUCH DAMAGE.                                                       */
/**********************************************************************/
/**********************************************************************/
/*                                                                    */
/* Author   : Robert M. Jansen                                        */
/*                                                                    */
/* Program  : FTP-client (RFC 959)                                    */
/*                                                                    */
/* Filename : ftpcl.c                                                 */
/*                                                                    */
/* Usage    : ftpcl [-d] <remote-host>:<filename> <filename>          */
/*                                                                    */
/*            ftpcl [-d] <filename> <remote-host>:<filename>          */
/*                                                                    */
/*            ftpcl [-d] <remote-host>:<directory>                    */
/*                                                                    */
/**********************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sgtty.h>
#include <netinet/in.h>


#include "ftp_client.h"




static ULONG local_ip, server_ip;


#define	WaitFor( x ) while( x ) usleep(200);




// returns SOCKET or ZERO if failed
int FTP_ServerOpen( char *server, char *name, char *pass )
{
	int gServer, gPort = 21;
	char *p;

	// if server contains "address:34" then use port 34
	if ( p = strchr( server, ':' ) )
		gPort = atoi( p+1 );

	gServer = OpenHost( server, gPort );

	if ( gServer>0 ){
		int ret;
		ret = FTP_Login( gServer, name, pass );
		return ret;
	}
	return gServer;
}


int FTP_Write( int fsock, char *buffer, long len )
{
	int ret;
	if ( fsock )
		ret = write( fsock, buffer, len );
	return ret;
}

int FTP_Read( int fsock, char *buffer, long len )
{
	int ret;
	if ( fsock )
		ret = read( fsock, buffer, len );
	return ret;
}

int FTP_Close( int fsock )
{
	close( fsock );
}




// ------------- Lower Level Support Funcs


//opencc
int OpenHost( char *rhostname, long port )
{
    int ftpCommsSocket, rc;
    char lhostname[50];
    HOSTENT *phostent;
    SOCKADDR srcaddr, serveraddr;
    char buffer[BUFMAXLEN], *serverName;
    int buflen;

    gethostname(lhostname, 50);
    phostent = gethostbyname(lhostname);
    local_ip = *((ULONG *) phostent->h_addr_list[0]);
    phostent = gethostbyname(rhostname);
    server_ip = *((ULONG *) phostent->h_addr_list[0]);

    ftpCommsSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ftpCommsSocket < 0) {
		return(0);
    }

    memset(&serveraddr, 0, sizeof(SOCKADDR));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = server_ip;
    rc = connect( ftpCommsSocket, (struct sockaddr *) &serveraddr, sizeof(SOCKADDR) );
    if (rc < 0) {
		close( ftpCommsSocket );
		return(0);
    }
    buflen = read( ftpCommsSocket, buffer, BUFMAXLEN );
	serverName = strstr( buffer, "220 " );

	OutDebugs( "Connected to %d : %s ", ftpCommsSocket, serverName );
    return( ftpCommsSocket );
}




int	ReadSock( int s, char *buffer )
{
	int buflen;

    buflen = read( s, buffer, BUFMAXLEN );
	if ( buflen>0 )
		*(buffer+buflen) = NULL;
	return buflen;
}

int	SendCmd( int s, char *buffer )
{
    write( s, buffer, strlen(buffer) );
}

int	SendCmdReply( int s, char *buffer, char *reply )
{
	int buflen;

    SendCmd( s, buffer );
    buflen = ReadSock( s, reply );

	OutDebugs( buffer );
	OutDebugs( reply );

	return buflen;
}


//ftplogin
int FTP_Login(int ftpCommsSocket, char *userid, char *passwd)
{
    char buffer[BUFMAXLEN];
    int buflen;

	OutDebugs( "FTP Login as '%s'", userid );

    sprintf(buffer, "USER %s\r\n", userid);
    write( ftpCommsSocket, buffer, strlen(buffer) );
    buflen = ReadSock( ftpCommsSocket, buffer );

    if ( buffer[0] == '3') {
		sprintf( buffer, "PASS %s\r\n", passwd );
		write(ftpCommsSocket, buffer, strlen(buffer));
		buflen = ReadSock( ftpCommsSocket, buffer );
    }
    if ( buffer[0] == '5') {
		OutDebugs( "Error logging in : %s", buffer );
		return(0);
    }
	if ( buffer[0] == '2') {
		OutDebugs( "User Logged In %s", buffer );
	    return(ftpCommsSocket);
    }
    return(0);
}



long FTP_StartDataPort( int *dataPortPtr )
{
    SOCKADDR srcaddr, destaddr;
    int rc, addrlen, dataPort, dataSocket;

    dataSocket = socket(AF_INET, SOCK_STREAM, 0);

    memset(&srcaddr, 0, sizeof(SOCKADDR));
    srcaddr.sin_family = AF_INET;
    srcaddr.sin_addr.s_addr = local_ip;
    rc = bind( dataSocket, (struct sockaddr *) &srcaddr, sizeof(SOCKADDR));

    addrlen = sizeof(SOCKADDR);
    rc = getsockname( dataSocket, (struct sockaddr *) &srcaddr, &addrlen );

    dataPort = srcaddr.sin_port;
    listen( dataSocket, 5 );

	*dataPortPtr = dataPort;
	return dataSocket;
}

long FTP_AcceptDataSocket( int dataSocket )
{
    SOCKADDR srcaddr, destaddr;
    int rc, addrlen, dataPort, clientSocket;

	addrlen = sizeof(SOCKADDR);
	clientSocket = accept(dataSocket, (struct sockaddr *) &destaddr, &addrlen);
	return clientSocket;
}


long FTP_DataConnectTo( int data_port )
{
    SOCKADDR destaddr;
    int rc, addrlen, dataPort, dataSocket;

	addrlen = sizeof(SOCKADDR);

    dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&destaddr, 0, sizeof(SOCKADDR));
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(data_port);
    destaddr.sin_addr.s_addr = server_ip;
    rc = connect( dataSocket, (struct sockaddr *) &destaddr, sizeof(SOCKADDR) );
    if (rc < 0) {
		close( dataSocket );
		return(0);
    }
	return(dataSocket);
}


int FTP_InitTransfer( int ftpCommsSocket, int passive_mode )
{
    char buffer[BUFMAXLEN], reply[BUFMAXLEN];
    int buflen, dataPort, dataSocket;
    unsigned char *port_ptr, *ip_ptr;

    sprintf(buffer, "MODE S\r\n");
    write( ftpCommsSocket, buffer, strlen(buffer) );
    buflen = ReadSock( ftpCommsSocket, reply );
	OutDebugs( "Sending %s, reply = %s", buffer , reply );

    sprintf(buffer, "TYPE I\r\n");
    write( ftpCommsSocket, buffer, strlen(buffer) );
    buflen = ReadSock( ftpCommsSocket, reply );
	OutDebugs( "Sending %s, reply = %s", buffer , reply );

	// PASSIVE MODE
	if ( passive_mode ){
		char *p;
		int host, port;

		SendCmdReply( ftpCommsSocket, "PASV\r\n\0", reply );

		if ( p = strchr( reply, '(' ) ){
			int c = 0;
			char *numbers[8];
			p++;

			while( (p=strtok( p, "," )) && (c<8) ){
		 		numbers[ c++ ] = p;
		 		p=NULL;
		 	}
			
			port = atoi( numbers[4] ) <<8;
			port |= atoi( numbers[5] );

			//OutDebugs( "port = %d", port );

			dataSocket = FTP_DataConnectTo( port );
		}
	} else 
	// ACTIVE MODE< not really reliable
	{
		unsigned char port_1, port_2;
		OutDebugs( "FTP_StartDataPort" );
		dataSocket = FTP_StartDataPort( &dataPort );
		OutDebugs( "FTP_StartDataPort socket = %d, port = %d", dataSocket, dataPort );

		port_1 = dataPort>>8;
		port_2 = dataPort&0xff;
		port_ptr = (unsigned char *) &dataPort;
		ip_ptr = (unsigned char *) &local_ip;
		sprintf(buffer, "PORT %u,%u,%u,%u,%u,%u\r\n", (unsigned int) ip_ptr[0], (unsigned int) ip_ptr[1], (unsigned int) ip_ptr[2], (unsigned int) ip_ptr[3], 
			port_1, port_2 );
		write(ftpCommsSocket, buffer, strlen(buffer));
		buflen = ReadSock(ftpCommsSocket, reply);
		OutDebugs( "Sending %s, reply = %s", buffer , reply );

		FTP_AcceptDataSocket( dataSocket );
	}

	OutDebugs( "FTP_InitTransfer done." );

    return(dataSocket);
}


// upload file
int FTP_OpenWrite(int ftpCommsSocket, char *rname)
{
	char buffer[BUFMAXLEN];
	int buflen, dataSocket;

	dataSocket = FTP_InitTransfer( ftpCommsSocket, 1 );

	sprintf(buffer, "STOR %s\r\n", rname);
	write(ftpCommsSocket, buffer, strlen(buffer));
	buflen = ReadSock(ftpCommsSocket, buffer);
	if (buffer[0] != '1') {
		return(-1);
	}
	return dataSocket;
}

int FTP_OpenRead(int ftpCommsSocket, char *rname )
{
	char buffer[BUFMAXLEN];
	int buflen, dataSocket;;

	OutDebugs( "FTP_OpenRead %s", rname );

	dataSocket = FTP_InitTransfer( ftpCommsSocket, 1 );

	sprintf(buffer, "RETR %s\r\n", rname);
	write(ftpCommsSocket, buffer, strlen(buffer));
	buflen = ReadSock(ftpCommsSocket, buffer);
	if (buffer[0] != '1') {
		return(0);
	}
	return dataSocket;
}


void *FTP_OpenFile( void *hConnect, char *file, char type )
{
	void *hNetFile;

	type = tolower( type );
	switch( type )
	{
		case 'r' : hNetFile = FTP_OpenRead( (int)hConnect, file ); break;
		case 'w' : hNetFile = FTP_OpenWrite( (int)hConnect, file ); break;
	}
	return hNetFile;
}




int FTP_Logout(int ftpCommsSocket)
{
    char buffer[BUFMAXLEN];
    int buflen;

    sprintf(buffer, "QUIT\r\n");
    write(ftpCommsSocket, buffer, strlen(buffer));
    buflen = ReadSock(ftpCommsSocket, buffer);
	close(ftpCommsSocket);
    return(0);
}




/*
PASSIVE SAMPLE TRANSFER

TYPE I
200 Type set to I.
PASV
227 Passive mode entered (203,111,20,140,105,125)
RETR ation.html
150 Opening BINARY mode data connection for ation.html (7942 bytes).
226 Transfer complete.
Transferred: ation.html 7,942 bytes in 00:01 (7.76 k/sec)
Transfer queue completed
Transferred 1 file Totaling 7,942 bytes in 00:01 (7.76 k/sec)



ACTIVE TRANSFER

PORT 10,20,13,3,8,16
200 PORT command successful.
RETR ation.html
150 Opening BINARY mode data connection for ation.html (7942 bytes).
226 Transfer complete.
Transferred: ation.html 7,942 bytes in 00:01 (7.76 k/sec)
Transfer queue completed
Transferred 1 file Totaling 7,942 bytes in 00:01 (7.76 k/sec)






  */



/*
int ftplist(int ccsock, char *dirname)
{
    char buffer[BUFMAXLEN];
    int buflen;

    localfh = STDOUT_FILENO;
    dtpcmd = DTP_RECV;
    ftpdone = 0;
    sprintf(buffer, "LIST %s\r\n", dirname);
    write(ccsock, buffer, strlen(buffer));
    buflen = read(ccsock, buffer, BUFMAXLEN);
    if (buffer[0] != '1') {
		return(-1);
    }
    while (!ftpdone) {
		condition_wait(cond_ftpdone, mutex_ftpdone);
    }
    buflen = read(ccsock, buffer, BUFMAXLEN);
    return(0);
}
int mutex_getport = 1;
void dtpclient(THREAD_INP *thr_inp)
{
    if (thr_inp->dtpcmd == DTP_RECV) {
		char buffer[BUFMAXLEN];
		int buflen;

		buflen = read(thr_inp->dtpsock, buffer, BUFMAXLEN);
		while (buflen > 0) {
			write(localfh, buffer, buflen);
			buflen = read(thr_inp->dtpsock, buffer, BUFMAXLEN);
		}
    }
    else if (thr_inp->dtpcmd == DTP_SEND) {
		char buffer[BUFMAXLEN];
		int buflen;

		buflen = read(localfh, buffer, BUFMAXLEN);
		while (buflen > 0) {
			write(thr_inp->dtpsock, buffer, buflen);
			buflen = read(localfh, buffer, BUFMAXLEN);
		}
    }
    close(thr_inp->dtpsock);
    free(thr_inp);
    mutex_lock(mutex_ftpdone);
    ftpdone = 1;
    mutex_unlock(mutex_ftpdone);
    condition_signal(cond_ftpdone);
    cthread_exit(0);
}



// Threaded internals
static	pthread_t         dtpmain_thread, ftpclient_thread;
static	pthread_attr_t    a_thread_attribute;

void dtpmain( long param )
{
    SOCKADDR srcaddr, destaddr;
    int rc, addrlen;

    dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&srcaddr, 0, sizeof(SOCKADDR));
    srcaddr.sin_family = AF_INET;
    srcaddr.sin_addr.s_addr = local_ip;
    rc = bind(dataSocket, (struct sockaddr *) &srcaddr, sizeof(SOCKADDR));
    addrlen = sizeof(SOCKADDR);
    rc = getsockname(dataSocket, (struct sockaddr *) &srcaddr, &addrlen);
    dataPort = srcaddr.sin_port;
    cond_getport = 1;
    listen(dataSocket, 5);

    while (1) {
		addrlen = sizeof(SOCKADDR);
		clientSocket = accept(dataSocket, (struct sockaddr *) &destaddr, &addrlen);

		cond_ftpready = 1;

		WaitFor( cond_ftpdone );

		//cthread_detach(cthread_fork((cthread_fn_t) dtpclient, (any_t) thr_inp));
		//pthread_create( &dtpclient_thread, &a_thread_attribute, (void*)dtpclient, (any_t) thr_inp));
    }
}



void startdtp(void)
{
	cond_getport = 0;
	dataPort = -1;

OutDebugs( "Start FTP Data thread" );

	pthread_create( &dtpmain_thread, &a_thread_attribute, (void*)dtpmain, (void*)0 );

    while (dataPort < 0) {
		WaitFor( cond_getport );
    }

OutDebugs( "FTP Data thread ended" );

}




*/
