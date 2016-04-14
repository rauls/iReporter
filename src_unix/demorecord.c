//#include "extern.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>
      
#include <netdb.h>

char *ConvertParam(char *);
long ConvertResult(char *, char **);
char *search_yahoo(char *);
int doSocket(char *);
int reqDocument(char *);
int getLine(int, char *);

int sockfd;

char *ConvertParam(char *param)
{
static
	char	newParam[256];
	char	*s, *d;
	
	d = newParam;
	s = param;

	while (*s) {
		if (*s < 33) {
			sprintf( d, "%%%02x", *s );
			d+= 3;
		} else {
			*d = *s;
			d++;
		}
		s++;
	}
	*d = 0;
	return newParam;
}



char *demorecord( long runcount )
{
static	char	tmpStr[3000], thishost[128];
	struct utsname box;
	
	long 	num;

	uname( &box );	
#ifndef NOSOCKETS
	gethostname( thishost, 128 );
	sprintf(tmpStr, "/demo/unix?machine=%s-%s.%s&domain=%s&host=%s&count=%ld", box.sysname, box.release, box.version, "", thishost, runcount );
	//printf( "%s\n\n", tmpStr );
	//doSocket("localhost");
	
	switch( fork() ){
		case 0:
			doSocket("www.funelweb4.com");
			reqDocument( ConvertParam(tmpStr) );
			num = GetWebResult( 0, tmpStr);
			close(sockfd);
			//printf( "DONE\n" );
			exit(0);
			break;
	}
#endif
	return tmpStr;
}




#ifndef NOSOCKETS
long GetWebResult(char *param, char *result )
{
	char	lineStr[2048], *space;
	FILE 	*fp;
	long	count = 0;

	*result = 0;

		while ( getLine(sockfd, lineStr) != 0 ) {
			strcat( result, lineStr );
			count++;
		}

	return count;
}



int doSocket(char *host)
{
	struct sockaddr_in sa;
	struct hostent *hp;
	
	bzero(&sa, sizeof(sa));

	if ((hp = gethostbyname(host)) == (struct hostent *)0) {
		fprintf(stderr, "No domain entry for %s\n", host);
		exit(1);
	}
	bcopy(hp->h_addr, (char *) &sa.sin_addr, hp->h_length);
	
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);
	
	if (!(sockfd = socket(AF_INET, SOCK_STREAM, 0))) {
		fprintf(stderr, "Couldn't create a socket?\n");
		exit(1);
	}
	
	if (connect(sockfd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
		fprintf(stderr, "Can't connect to host %s: %s\n", host, strerror(errno));
		exit(1);
	}
	return 0;
}

int reqDocument(char *document)
{
	char buf[256];
	sprintf(buf, "GET %s HTTP1.0\n\n", document);
	send(sockfd, buf, strlen(buf), 0);
	return 0;
}

int getLine(int mysock, char *buf)
{
	char *p, c;
	int rv;
    struct timeval tout;
    fd_set *readset = (fd_set *)malloc(sizeof(fd_set));

	p = buf;
	tout.tv_sec = 20;
	tout.tv_usec = 0;
	
	FD_SET(sockfd, readset);
	while (1){
		if (select(sockfd+1, readset, NULL, NULL, &tout) > 0){
			if (FD_ISSET(sockfd, readset)) {
				if (recv(sockfd, &c, 1, 0) == 0)
					return 0;
				if (c == '\n') {
					*p = '\0';
					return 1;
				} else
					*p++ = c;
			}
		} else return 0;
	}
}
#endif

















