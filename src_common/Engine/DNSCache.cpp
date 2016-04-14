
#include "Compiler.h"

#include <cstdio>
#include <cstring>
#include <string.h>

#include "DNSCache.h"
#include "myansi.h"
#include "datetime.h"
#include "config.h"		// for extern of "MyPrefStruct"

#include "EngineStatus.h"

#ifdef DEF_MAC
	#include "MacUtil.h"
#endif

/* CQDNSCache - class constructor */
CQDNSCache::CQDNSCache(long incr, int part)
			 : num(0),maxNum(0),incrNum(incr),partialCompare(part)
{
	DNSdata = new DNScache [maxNum + incrNum];

	memset( headref, -1, 256*sizeof(long) );
}

static char empty_dnr[2] = { "\0" };



/* ~CQDNSCache - class destructor */
CQDNSCache::~CQDNSCache()
{
	long		i;
	
	/* reset name list */
	if (maxNum) {
		for (i=0; i<num; i++){
			if ( DNSdata[i].name != empty_dnr )
			delete [] DNSdata[i].name;
		}
		maxNum = num = 0;
		delete [] DNSdata;
	}
}




/* AddString - add string to lookup list */
void CQDNSCache::Add( long ip, char *name, char *expire )
{
	long		len,i;
	char		*p;
	DNScache	*tempDNSdata;
	
	/* expand list if necessary */
	if (num >= maxNum) {
		tempDNSdata = new DNScache [maxNum + incrNum];
		if (tempDNSdata) {
			memcpy( tempDNSdata, DNSdata, maxNum * sizeof(DNScache));
			delete [] DNSdata;
			DNSdata = tempDNSdata;
			maxNum += incrNum;
		} else {
			MemoryError( "Could not duplicate DNS list",maxNum + incrNum );
			return;
		}
	}
	
	/* add country to list */
	len = mystrlen( name );
	if ( len ){
		len++;
		p = DNSdata[num].name = new char[ len ];
		mystrcpy( p, name );
	} else
		DNSdata[num].name = empty_dnr;			// NON RESOLVED IP

	if ( ip ) {
		DNSdata[num].ip = ip;
		if ( expire )
			DNSdata[num].expire = myatoi( expire );
		else
			DNSdata[num].expire = GetCurrentCTime() + (ONEDAY*MyPrefStruct.dnr_expire);

		i = ip & 0xff;
		DNSdata[num].next = headref[ i ];
		headref[i] = num;
	}
	num++;
}


char *CQDNSCache::LookupIP( long ip )
{
	long	i, index;
	
	//hash = HashIt( code, -1 );

	index = ip & 0xff;
	i = headref[index];
	
	while( i!=-1 ) {
		if ( DNSdata[i].ip == ip ){
			return DNSdata[i].name;
		} else
			i = DNSdata[i].next;
	}

	return((char *)0);
}

/* Lookup - look up an entry in the list */
char *CQDNSCache::Lookup( char *ipstring )
{
	long	ip;
	

	ip = IPStrToIPnum( ipstring );

	return LookupIP( ip );
}





// call with addressList->SaveLookups
// before 'CloseTCPDriver();'
long CQDNSCache::SaveLookups( void )
{
	long	i, 
			total=0,
			totalip=0,
			totalname=0,
			totalexp=0,
			ct = GetCurrentCTime();
	FILE *fp;
	char	*addr1, *addr2;
	
	//if ( !num ) return 0;
#ifdef DEF_MAC
//	if ( fp = prefopen( "dnr.cache", "w+" ) ){
	if (fp = OpenCacheFile ("w+"))
	{
#else
	if ( fp = fopen( MyPrefStruct.dnr_cache_file, "w+" ) ){
#endif
		fprintf( fp, "###############################################\n");
		fprintf( fp, "# DNS Cache data file (C) Analysis Software 1997-2002\n");
		fprintf( fp, "# format: ipaddress,resolved_address,expire_unixtime\n");
		fflush( fp );
		fprintf( fp, "# saved time = %d\n", ct);
		fflush( fp );
		fprintf( fp, "###############################################\n");
		fflush( fp );

		OutDebugs( "Saving %d DNR Lookups", num );

		for (i=0; i<num; i++) {
			if ( DNSdata[i].expire > ct ){
				char ipstr[32];

				addr1 = IPnumToIPStr( DNSdata[i].ip, ipstr );
				addr2 = DNSdata[i].name;
				if ( addr1 ){
					if ( addr2 ){
						if ( *addr2 ){
							fprintf( fp, "%s,%s,%d\n", addr1, addr2, DNSdata[i].expire );
							totalname++;
						} else {
							fprintf( fp, "%s, ,%d\n", addr1, DNSdata[i].expire );
							totalip++;
						}
					}
					total++;
					fflush( fp );
				}
			} else {
				totalexp++;
			}
		}

		fflush( fp );
		fprintf( fp, "###############################################\n");fflush( fp );
		fprintf( fp, "# total addresses  = %d\n", total );fflush( fp );
		fprintf( fp, "# total resolved   = %d\n", totalname );fflush( fp );
		fprintf( fp, "# total unresolved = %d\n", totalip );fflush( fp );
		fprintf( fp, "###############################################\n");
		fflush( fp );
		fclose( fp );
	}
	OutDebugs( "Saved DNR, total_ips=%d, names=%d, unresolved=%d, expired=%d", total, totalname, totalip,totalexp );
	return num;
}


//
//
//
//
//
// call with addressList->LoadLookups(  )
// before 'OpenTCPDriver()'
//
long CQDNSCache::LoadLookups( void )
{
	char	*ipStr;
	char	*dnsStr, *expStr;
	char	cacheline[256+64];
	long	count=0, ip;
	FILE *fp;
	long	loadip=0, loadname=0, expired =0;

#ifdef DEF_MAC
	// if ( fp = prefopen( "dnr.cache", "r" ) ){
	if (fp = OpenCacheFile ("r"))
#else
	if ( fp = fopen( MyPrefStruct.dnr_cache_file, "r" ) )
#endif
	{
		OutDebugs( "Loading DNR File %s", MyPrefStruct.dnr_cache_file );
		while( !feof(fp ) ) 
		{
			fgets( cacheline, 255, fp );
			trimLine( cacheline );
			//fscanf( fp, "%s\n", cacheline );
			ipStr = cacheline;
			if ( *ipStr != '#' ){
				dnsStr = mystrchr( cacheline, ',' );		// get resolved string

				if ( dnsStr )
					expStr = mystrchr( dnsStr+1, ',' );		// get expire string
				else
					expStr = NULL;

				if ( expStr ){
					long extime;
					*expStr++ = 0;
					extime = myatoi(expStr);
					if ( extime < GetCurrentCTime() && extime>0 ){
						expired++;
						continue;
					}
				}

				if ( dnsStr ){
					*dnsStr++ = 0;
					ip = IPStrToIPnum( ipStr );
					if ( *dnsStr != ' ' ){				// if has resolved then use else add empty
						Add( ip, dnsStr, expStr );
						loadname++;
					} else {
						Add( ip, NULL, expStr );
						loadip++;
					}
					count++;
				} else {
					Add( ip, NULL, NULL );
					count++;
					loadip++;
				}
			}
		}
		fclose( fp );
	}
	OutDebugs( "Loaded DNS, %d ips, %d names, expired=%d, total=%d", loadip, loadname, expired, num );
	return count;
}
