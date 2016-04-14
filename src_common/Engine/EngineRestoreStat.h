
#ifndef	ENGINERESTORESTAT_H
#define	ENGINERESTORESTAT_H

#include "Stats.h"

void RestoreHourNames( Statistic *stat );
void RestoreWeekdaysNames( Statistic *stat );
void RestoreRegionNames( Statistic *stat );
void RestoreCountryNames( Statistic *stat );
void RestoreOpersysNames( Statistic *stat );
void RestoreRobotsNames( Statistic *stat );
void RestoreErrorsNames( Statistic *stat );
void RestoreStatisticName( Statistic* pToStat, StatList* pFromStatList );
void FixClientBrowserOS_IDS( VDinfoP vd );

extern char *hourStrings[];
extern char *regionStrings[];
extern char *opersysStrings[];

#endif

