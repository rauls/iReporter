/* 	
	File:		DNR.c 
*/

//#define	USE_THREADS

#include <ansi_parms.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <netdb.h>



#include "asyncdnr.h"
#include <ares.h>

static int dnractive = 0, status;

static ares_channel channel;


int GetLocalHostName( char *name, long len )
{
	return gethostname(name,len);
}


long DNRInit( void )
{
	if ( dnractive )
		return dnractive;

	status = ares_init(&channel);
	//
	// set option in ares.
	//
	if (status != ARES_SUCCESS){
		char	*errmem;
		fprintf(stderr, "ares_init: %s\n", ares_strerror(status, &errmem));
		ares_free_errmem(errmem);
		return 0;
	}
	dnractive = 1;
	return 1;
}


/*
struct ares_options {
  int flags;
  int timeout;
  int tries;
  int ndots;
  unsigned short udp_port;
  unsigned short tcp_port;
  struct in_addr *servers;
  int nservers;
  char **domains;
  int ndomains;
  char *lookups;



	  hostent = gethostbyname(optarg);
	  if (!hostent || hostent->h_addrtype != AF_INET)
	    {
	      fprintf(stderr, "adig: server %s not found.\n", optarg);
	      return 1;
	    }
	  options.servers = realloc(options.servers, (options.nservers + 1)
				    * sizeof(struct in_addr));
	  if (!options.servers)
	    {
	      fprintf(stderr, "Out of memory!\n");
	      return 1;
	    }
	  memcpy(&options.servers[options.nservers], hostent->h_addr,
		 sizeof(struct in_addr));
	  options.nservers++;
	  optmask |= ARES_OPT_SERVERS;





};*/


long SetDNROptions( char *server, long timeout, long retries )
{
	if ( dnractive ){
		struct ares_options opt; long mask=0;
		struct hostent *hostent = NULL;
		static struct in_addr dns_servers;

		if ( server ){
			if ( *server )
				hostent = gethostbyname(server);

			if ( hostent ){
				mask |= ARES_OPT_SERVERS;
				opt.servers = &dns_servers;
				memcpy( &dns_servers, hostent->h_addr, sizeof(struct in_addr));
				opt.nservers = 1;
			}
		}
		if ( timeout ){
			mask |= ARES_OPT_TIMEOUT;
			opt.timeout = timeout;
		}
		if ( retries ){
			mask |= ARES_OPT_TRIES;
			opt.tries = retries;
		}

		return ares_init_options( &channel, &opt, mask );
	}
	return 0;
}



int AddrToName( char *addrStr, void * userDataPtr)
{
	struct hostent *thehost;
	long 	err=0, lAddr;
  
  	lAddr = inet_addr(addrStr);
	
	thehost = gethostbyaddr ( (char*)&lAddr, sizeof(lAddr), AF_INET );
	
	if ( thehost ){
		strcpy( (char*)userDataPtr, thehost->h_name );
		return 0;
	} else
		return -1;
}




long DobackgroundDNS( void )
{
	int retcode = 0;
	static int nfds, count;
	static fd_set readers, writers;
	static struct timeval tv, max, *tvp;

	{
		FD_ZERO(&readers);
		FD_ZERO(&writers);
		nfds = ares_fds(channel, &readers, &writers);
		if ( nfds )
		{
			memset( &max, 0, sizeof(struct timeval) );
			max.tv_sec = 0;
			max.tv_usec = 250;
			tvp = ares_timeout(channel, NULL, &tv);
			count = select(nfds, &readers, &writers, NULL, tvp);
			
//fprintf( stderr, "%ld : ares_process....\n", time(0) );	fflush(stderr);
			ares_process(channel, &readers, &writers);
//fprintf( stderr, "%ld : ares_process.... Done\n", time(0) );	fflush(stderr);
			retcode = 0;
		} else {
			retcode = 1;
		}
	}
	return retcode;
}





// Dont stay here for too long, max 1 second.
long DobackgroundLookups( int duration )
{
	long int ct;

//fprintf( stderr, "%ld : DobackgroundLookups.........\n", ct );	fflush(stderr);
	ct = time(0);
	while ( (time(0)-ct) <= duration )
	{
		if ( DobackgroundDNS() )
			break;
	}
//fprintf( stderr, "%ld : DobackgroundLookups done.\n", time(0) );	fflush(stderr);
	return 0;
}





static void callback(void *arg, int status, struct hostent *host)
{
static	struct in_addr addr;
	char **p;

	if (status != ARES_SUCCESS)
	{
		char *mem;
		//fprintf(stderr, "%d: %s\n", status, ares_strerror(status, &mem));
		//ares_free_errmem(mem);
		ReturnedAddrToName( arg, 0 );
		return;
	}

	if ( p = host->h_addr_list ){
		memcpy(&addr, *p, sizeof(struct in_addr));
		ReturnedAddrToName( arg, host->h_name );
	}
	return;
	for (p = host->h_addr_list; *p; p++)
	{
		memcpy(&addr, *p, sizeof(struct in_addr));
		printf("**** %-32s\t%s\n", host->h_name, inet_ntoa(addr));
	}
}


/*
 void ares_gethostbyaddr(ares_channel channel, const void *addr,
            int addrlen, int family, ares_host_callback callback,
            void *arg)
	*/
void *AsyncLookup( void *data, long lAddr, long *id )
{
	DNRPtr dnr = (DNRPtr)data;

	dnr->ready = 0;
	dnr->addr.s_addr = lAddr;

	ares_gethostbyaddr( channel, &dnr->addr, sizeof(dnr->addr), AF_INET, callback, data );

	return 1;
}
























