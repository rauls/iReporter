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
/* Filename : ftpcl.h                                                 */
/*                                                                    */
/**********************************************************************/
#ifndef _FTPIO_H
#define _FTPIO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sockaddr_in	SOCKADDR;
typedef struct in_addr		IN_ADDR;
typedef struct hostent		HOSTENT;
typedef unsigned long		ULONG;

#define BUFMAXLEN		4096

#define FTPSTOR			1
#define FTPRETR			2
#define FTPLIST			3

#define DTP_SEND		1
#define DTP_RECV		2

typedef struct {
    int dtpsock;
    int dtpcmd;
} THREAD_INP;


int FTP_ServerOpen( char *server, char *name, char *pass );
int FTP_Write( int fsock, char *buffer, long len );
int FTP_Read( int fsock, char *buffer, long len );
int FTP_Close( int fsock );
int FTP_OpenWrite(int ftpCommsSocket, char *rname);
int FTP_OpenRead(int ftpCommsSocket, char *rname );
void *FTP_OpenFile( void *hConnect, char *file, char type );
int FTP_Logout(int ccsock);
int OpenHost( char *rhostname, long port );

int	SendCmd( int s, char *buffer );
int	SendCmdReply( int s, char *buffer, char *reply );
int	ReadSock( int s, char *buffer );

void		startdtp(void);

#ifdef __cplusplus
}
#endif

#endif

