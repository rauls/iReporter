#ifndef	ENGINEADDSTATS_H
#define	ENGINEADDSTATS_H

#include "VirtDomInfo.h"	// for VDinfoP.

#include "hitdata.h"		// for HitDataRec

extern char ErrVhostStr[];

void InitOtherStatlists( VDinfoP VDptr );
void CalcAgentVisitors( VDinfoP	VDptr, Statistic *client );
void AddVisitorInfo( VDinfoP VDptr, long sessionDate );
long AddPageSessionDetail( VDinfoP	VDptr, long pageHashID, long dur, long clientnum, long sessions );
void ShowSessionStatus( long pos, long tot );
long AddSessionRecords( VDinfoP	VDptr, Statistic *p, long clientnum );
void AddOrganizations( VDinfoP VDptr, StatList *byStat, Statistic *p );
void AddUsers( VDinfoP VDptr, StatList *byStat, Statistic *p );
void AddRegionsFromClient( VDinfoP VDptr, StatList *byStat, Statistic *src, const char *regionName, long reg );
void AddCountryFromClients( VDinfoP VDptr, StatList *byStat, Statistic *src );
void AddTimeonFromClient( VDinfoP VDptr, StatList *byStat, Statistic *p );
void AddLoyaltyFromClient( VDinfoP VDptr, StatList *byStat, Statistic *p );
void AddCirculationFromClient( VDinfoP VDptr, StatList *byStat, Statistic *p, long diff );
void AddClientDetailsToOthers( VDinfoP VDptr, Statistic *p, long clientnum );


#endif
