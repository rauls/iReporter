#ifndef	ENGINEVIRTUALDOMAIN_H
#define	ENGINEVIRTUALDOMAIN_H

#include "VirtDomInfo.h"
#include "GlobalPaths.h"

VDinfoP	FindVirtualDomain( char *vname );
void CloseVirtualDomain( VDinfoP VDptr, long logstyle );
short NeedClientStats();
VDinfoP	InitVirtualDomain( int num, char *name, short logstyle );
void LimitIncrValues( long vdnum );
void SortRecords( VDinfoP VDptr, long logstyle );

short AnyTrafficStatsValid( VDinfo *VDptr );
short AnyDiagnosticStatsValid( VDinfo *VDptr );
short AnyServerStatsValid( VDinfo *VDptr );
short AnyDemographicStatsValid( VDinfo *VDptr );
short AnyReferralStatsValid( VDinfo *VDptr );
short AnyMultiMediaStatsValid( VDinfo *VDptr );
short AnySystemsStatsValid( VDinfo *VDptr );
short AnyAdvertisingStatsValid( VDinfo *VDptr );
short AnyMarketingStatsValid( VDinfo *VDptr );
short AnyClustersStatsValid( VDinfo *VDptr );
short AnyFirewallStatsValid( VDinfo *VDptr );
short AnyStreamingMediaStatsValid( VDinfo *VDptr );



#include "config_struct.h"		// for MAX_DOMAINS
#include "config.h"				// for MAX_LOGFILES
extern	long		VDnum;
extern	VDinfoP		VD[ MAX_DOMAINS+10 ];			/* 16/32 virtual domain pointers */


extern	long		kUserStart			;
extern	long		kDefaultFileIncr	;
extern	long		kDateIncr			;
extern	long		kHourIncr			;
extern	long		kWDayIncr			;
extern	long		kDomainIncr;
extern	long		kUserIncr			;
extern	long		kSubDomainIncr;
extern	long		kSecondDomainIncr;
extern	long		kFileIncr			;
extern	long		kReferIncr			;
extern	long		kDirIncr			;
extern	long		kTypeIncr			;
extern	long		kErrorsIncr			;
extern	long		kPagesIncr			;
extern	long		kSearchStrIncr		;
extern	long		kDownloadIncr		;
extern	long		kCountryIncr		;
extern	long		kBrowserIncr		;
extern	long		kOSIncr				;
extern	long		kRobotIncr			;
extern	long		kDNSListIncr		;

#endif
