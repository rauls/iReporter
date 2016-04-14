#ifndef	_ENGINE_DBIO_
#define	_ENGINE_DBIO_

#include "VirtDomInfo.h"

void DBIO_SaveAllToFile( VDinfoP *VDptrs, long count, char *filename );
void DBIO_SaveToFile( VDinfoP VDptr, char *filename );
long DBIO_LoadFromFile( long outfp, long fileSize, long *allreq );

long DBIO_Open( const char* filename );
void DBIO_Close( long f );

int IsFileV4DataBase( const char *filename );

#endif

