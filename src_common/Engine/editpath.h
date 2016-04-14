#ifndef	__EDITPATH
#define	__EDITPATH

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define	MAXEDITPATH_SIZE	512

typedef	struct EditPathStruct {
	char	vhost[MAXEDITPATH_SIZE];
	char	path[MAXEDITPATH_SIZE];
	long	index;
	struct	EditPathStruct	*next;
} EditPathRec, *EditPathRecPtr, **EditPathRecH;

#define	EDITPATHSIZE		sizeof( EditPathRec )

void ClearEditPath( EditPathRecPtr *dataPtr );

char *EditPathReplacePathFromHost( EditPathRecPtr dataPtr, char *host, char *origPath );
char *ReplacePathFromHost( char *host, char *origPath );


void RemoveEditPath( EditPathRecPtr *dataPtr );
void DelEditPath( EditPathRecPtr *, int n );
long GetTotalEditPath( EditPathRecPtr );
void AddEditPath( EditPathRecPtr *, char *host, char *path );
EditPathRecPtr GetEditPath( EditPathRecPtr, int n );
//long CheckEditPath( EditPathRecPtr schP, double currentTime );
void SaveEditPathNew( FILE *fp, EditPathRecPtr schP );
//void ReadEditPath( char *filename, EditPathRecPtr );
short FindHost( EditPathRecPtr, short num, char *host );


long CopyEditPath( EditPathRecPtr *destPtr, EditPathRecPtr *srcPtr );

#ifdef __cplusplus
}
#endif

#endif


