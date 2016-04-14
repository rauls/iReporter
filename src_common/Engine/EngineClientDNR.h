#ifndef	ENGINECLIENTDNR_H
#define	ENGINECLIENTDNR_H

#include "VirtDomInfo.h"
#include "hitdata.h"		// for HitDataRec

#include "Stats.h"
#include "DNSCache.h"

int CleanupClientString( HitDataRec *Line );
long ProcessReadyDNR( char *out, long *uid, long *errcode );
long GetResolvedClient( VDinfoP VDptr, long background_mode );
void ShowProgressDNRLevel( void );
long GoResolveRemainder( VDinfoP VDptr, long clientnum );
long GoResolveClient( VDinfoP VDptr, Statistic *p, long clientnum, long );
long PostCalcResolveClients( VDinfoP VDptr, long logStyle );
long BackgroundResolveClients( VDinfoP VDptr, long logStyle );
extern CQDNSCache	*addressList;

extern char	rev_domain[10240];
extern char	fwd_domain[10240];

#endif
