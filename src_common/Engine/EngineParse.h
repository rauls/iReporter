
#ifndef	ENGINEPARSE_H
#define	ENGINEPARSE_H

#include "VirtDomInfo.h"	// for VDinfoP.

#include "hitdata.h"		// for HitDataRec
#include "config_struct.h"	// for MAX_FILTERUNITS.

long strcmpManyFast( char *string, long *hash, char **comps );
char *GetDomainPath( VDinfoP VDptr );
char *GetNewOutpath( VDinfoP VDptr, char *path, char *dirName );
size_t	GetReportPath(VDinfoP VDptr, char* szDestPath, size_t nLength);
char *ReverseAddressARPA(char *srcDomain, char *dst);
char *ReverseAddress(char *srcDomain, char *dst);
long GetIPAddress(char *hostname);
void swapdata( void *s1, void *s2, long datasize );
void trimrefer(char *txt);

int InterpretBrowserString( char* outStr, const char* browserStr );
int InterpretMediaPlayerString( char *out, char *agentstr );
char *InterpretOperSysString( char *opersys_str, long *outOStype, long *outOSid  );
const char* InterpretRobotString( const char* robotStr, long* robotId );
char* DecodeSearch( char* outStr, const char* referralURLStr, const char* referralParamStr );

int PerformFilterCheck( VDinfoP VDptr, HitDataRec *Line );
int CheckFilterMatch2( char *mydata, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, char fcase );
int CheckFilterMatch( char *mydata, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, char fcase );
long CheckMultiFilterMatch( VDinfoP VDptr, HitDataRec *Line, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, long fcase );
char *CheckHostMappingMatch( char *host, char *url, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, char fcase );
char *CheckMappingMatch( char *mydata, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, char fcase );
char *AdvertCampMappingMatch( char *url, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long *ret, long tot, char fcase );
char *AdvertMappingMatch( char *url, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long *ret, long tot, char fcase );
char *MultiMappingMatch( char *mydata, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long *start, long tot, char fcase );

int CopyDomainNameOld( char *d, char *src );
int CopyDomainName( char *d, char *src );
extern "C" void CorrectVirtualName( char *name, char *vname, char nameisfile );
int IsNameBad( char *name );


#endif