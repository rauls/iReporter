#ifndef	REPORT_ROUTE_TO_KEYPAGES
#define	REPORT_ROUTE_TO_KEYPAGES

#include "ReportClass.h"		// For baseFile
#include "VirtDomInfo.h"		// For VDinfoP
#include "Stats.h"				// For StatList

// *********************************************************************
// This is the main producer of subtables.
// *********************************************************************
void	DoRouteToKeyPages(		VDinfoP VDptr, FILE *fp, baseFile* pReport, short sSort);
void	DoRouteFromKeyPages(	VDinfoP VDptr, FILE *fp, baseFile* pReport, short sSort);

// *********************************************************************
// This method is actually a callback function.
// *********************************************************************
int		FilterNonKeyPagesTo(	Statistic *);
int		FilterNonKeyPagesFrom(	Statistic *);


#endif