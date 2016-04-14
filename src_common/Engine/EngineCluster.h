
#ifndef	ENGINECLUSTER_H
#define	ENGINECLUSTER_H

#include "FWA.h"

#include "VirtDomInfo.h"
#include "config_struct.h"
#include "hitdata.h"

void InitClusters();
void FreeClusters();
VDinfoP ClusterGetLineDetails( VDinfoP VDptr, char **fsFile, int logNum, HitDataRec *Line, long *logError, const char* v4DatabaseFileName );
char *MatchClusterPaths( char *ipString, char *logPath, char cluster[MAX_FILTERUNITS][MAX_FILTERSIZE], long clusterTot );

#endif