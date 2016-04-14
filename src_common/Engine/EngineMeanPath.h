
#ifndef	ENGINEMEANPATH_H
#define	ENGINEMEANPATH_H

#include "VirtDomInfo.h"	// for MeanPathRec & VDinfoP.


long MP_GetMeanPathTotal( VDinfoP vd );
long MP_GetMeanPath( VDinfoP vd, long idx, long *hash, long *hits, long*pageIndex, short *count );
void MP_FreeMeanPath( VDinfoP vd );
struct MeanPathRec *MP_FindMeanPath( VDinfoP vd, long pathHash );
void MP_AddMeanPath( VDinfoP vd, long sessionHash, long clientHash, long sessionIndex, short count );
void MP_IncMeanPath( VDinfoP vd, long sessionHash, long clientHash, long sessionIndex, short count );
long CompMeanPath( void *p1, void *p2, long *result);
void MP_SortMeanPath( VDinfoP vd );

extern long		kMeanPathIncr;


#endif
