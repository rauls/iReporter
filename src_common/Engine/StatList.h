#ifndef	STATLIST_H
#define	STATLIST_H

#include "VirtDomInfo.h"	// for VDinfoP.

void ClearLastday( StatList *stat );
StatList *ClearStat( VDinfoP vd, StatList *byStat, long units );

#endif