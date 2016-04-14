
#include "compiler.h"
#include "EngineMeanPath.h"

#include <string.h>

long		kMeanPathIncr		= 1000;		// for meanpath entries

long MP_GetMeanPathTotal( VDinfoP vd )
{
	return vd->meanpath_totals;
}

long MP_GetMeanPath( VDinfoP vd, long idx, long *hash, long *hits, long*pageIndex, short *count )
{
	if ( idx < vd->meanpath_totals && vd->MeanPaths ){
		*hash = vd->MeanPaths[idx].clientHashId;
		*pageIndex = vd->MeanPaths[idx].pageIndex;
		*hits = vd->MeanPaths[idx].hits;
		*count = vd->MeanPaths[idx].count;
		return 1;
	}
	return 0;
}



void MP_FreeMeanPath( VDinfoP vd )
{
	struct MeanPathRec *mp = vd->MeanPaths;
	
	if( mp ){
		delete [] vd->MeanPaths;
	}
	vd->meanpath_totals = 0;
	vd->meanpath_max = kMeanPathIncr;
	vd->MeanPaths = NULL;
}

struct MeanPathRec *MP_FindMeanPath( VDinfoP vd, long pathHash )
{
	struct MeanPathRec *mp = vd->MeanPaths;

	if( mp ){
		long i = 0;
		while( i<vd->meanpath_totals ){
			if ( mp[i].hashseq == pathHash ){
				return &mp[i];
			}
			i++;
		}
	}
	return NULL;
}

void MP_AddMeanPath( VDinfoP vd, long sessionHash, long clientHash, long sessionIndex, short count )
{
	struct MeanPathRec *mp = vd->MeanPaths;
	long	index;

	index = vd->meanpath_totals;

	if( mp ){
		// increase buffer size
		if ( index >= vd->meanpath_max ){
			struct MeanPathRec *newmp;
			vd->meanpath_max += kMeanPathIncr;

			newmp = new MeanPathData[ vd->meanpath_max ];
			if( newmp ){
				memcpy( newmp, mp, vd->meanpath_totals * sizeof( struct MeanPathRec ) );
				delete [] vd->MeanPaths;
				mp = vd->MeanPaths = newmp;
			}
		}
		// add the data to the table
		if ( index < vd->meanpath_max ){
			mp[index].hashseq = sessionHash;
			mp[index].clientHashId = clientHash;
			mp[index].pageIndex = sessionIndex;
			mp[index].count = count;
			mp[index].hits = 1;
			vd->meanpath_totals++;
			return;
		}
	}
}

void MP_IncMeanPath( VDinfoP vd, long sessionHash, long clientHash, long sessionIndex, short count )
{

	if( !vd->MeanPaths ){
		MP_FreeMeanPath( vd );
		//vd->MeanPaths = (struct MeanPathRec *)malloc( vd->meanpath_max * sizeof( struct MeanPathRec ) );
		vd->MeanPaths = new MeanPathData[ vd->meanpath_max ];
	}

	if( vd->MeanPaths ){
		struct MeanPathRec *data;

		data = MP_FindMeanPath( vd, sessionHash );
		if ( data ){
			data->hits++;
			data->count = count;
		} else {
			MP_AddMeanPath( vd, sessionHash, clientHash, sessionIndex, count );
		}
	}
}


long CompMeanPath( void *p1, void *p2, long *result)
{
	struct MeanPathRec *d1, *d2;
	d1 = (struct MeanPathRec *)p1;
	d2 = (struct MeanPathRec *)p2;
	if ( d1 && d2 ){
		*result = (long)(d2->hits - d1->hits);
	}
	return 0;
}



void MP_SortMeanPath( VDinfoP vd )
{
	if ( vd->meanpath_totals )
		FastQSort( vd->MeanPaths, vd->meanpath_totals, sizeof( struct MeanPathRec ), CompMeanPath, TOP_SORT );
}
