/****************************************************************************
*
*
*    PROGRAM: EditPathing GUI
*
*    PURPOSE: 
*
****************************************************************************/
#include "Compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef DEF_MAC
#include "main.h"
#endif
// Local Header Files
#include "myansi.h"
#include "config_struct.h"
#include "editpath.h"
#include "Utilities.h"

extern struct		App_config MyPrefStruct;



static EditPathRecPtr	gBackupEditPaths = NULL;


/*
	replace a path/file string of "dir/this.html" with
	a custom new path using the same file to make "new/cool/this.html"


  */
char *EditPathReplacePathFromHost( EditPathRecPtr dataPtr, char *host, char *origPath )
{
	EditPathRecPtr editP;
	long	count,num;
	char	lFile[256], *newPath=0, tch;

	num = GetTotalEditPath( dataPtr );

	for( count=1; count<=num; count++){
		editP = GetEditPath( dataPtr, count );
		if( match( host,editP->vhost,0 ) ){
		//if( strcmp( editP->vhost, host ) == 0 ){
			newPath = editP->path;
			if ( *newPath == 0 )
				newPath = NULL;
			break;
		}
		if (CheckWWWprefix( editP->vhost)) {
			//if there is a www. prefix only
			//compare remainder
			if ( strcmp( editP->vhost+4, host ) == 0 ) {
				newPath = editP->path;
				break;
			}
		}	
	}
	if ( newPath ) {
		FileFromPath( origPath, lFile );
		PathFromFullPath( newPath, origPath );
		strcat( origPath, lFile );
		num = mystrlen( origPath );
		tch = origPath[num-1];
		switch( tch ){
			case '/' :
			case ':' :
			case '\\' :
				origPath[num-1] = 0; break;
		}
		return origPath;
	} else
		return NULL;
}

char *ReplacePathFromHost( char *host, char *origPath )
{
	return EditPathReplacePathFromHost( MyPrefStruct.EditPaths, host, origPath );
}

short FindHost( EditPathRecPtr dataPtr, short num, char *host )
{
	EditPathRecPtr editP = dataPtr;

	while( editP ){
	//for( count=1; count<=num; count++){
		//editP = GetEditPath( dataPtr, count );
		if( match( host,editP->vhost,0 ) ){
			return 1;
		}
		editP = editP->next;
	}
	return 0;
}


long GetCountOfEditPath( EditPathRecPtr dataPtr )
{
	long	index=0;
	EditPathRecPtr editP = dataPtr;

	if ( editP ) {
		while( editP ){
			editP = editP->next;
			index++;
		}
	}
	return index;
}


long GetTotalEditPath( EditPathRecPtr dataPtr )
{
	return GetCountOfEditPath( dataPtr );
}

EditPathRecPtr CopyToEditPath( EditPathRecPtr *dataPtr, char *host, char *path )
{
	EditPathRecPtr nextP, editP;
	long	index=1;

	if ( !dataPtr )
		return NULL;

	editP = *dataPtr;

	if ( editP ){
		while( editP ){
			nextP = editP->next;
			index++;

			if ( ! nextP ) {
				nextP = editP->next = (EditPathRec *)malloc( EDITPATHSIZE );
				editP = 0;
			} else
				editP = nextP;
		}
	} else {
		nextP = *dataPtr = (EditPathRec *)malloc( EDITPATHSIZE );
	}

	trimLine(host);
	trimLine(path);

	if ( host )
		strcpy( nextP->vhost, host );
	else
		*nextP->vhost = 0;

	if ( path )
		strcpy( nextP->path, path );
	else
		*nextP->path = 0;

	nextP->index = index;
	nextP->next = 0;
	
	return nextP;
}

void AddEditPath( EditPathRecPtr *dataPtr, char *host, char *path )
{
	CopyToEditPath( dataPtr, host, path );
}



void RemoveEditPath( EditPathRecPtr *dataPtr )
{
	EditPathRecPtr nextP, editP = *dataPtr;

	if ( editP ) {
		while( editP ){
			nextP = editP->next;
			free( editP );
			editP = nextP;
		}
		*dataPtr = NULL;
	}
}

long CopyEditPath( EditPathRecPtr *destPtr, EditPathRecPtr *srcPtr )
{
	EditPathRecPtr editP;
	long		count, num;

	if ( destPtr && srcPtr ){
		num = GetTotalEditPath( *srcPtr );
		*destPtr = NULL;
		for( count=1; count<=num; count++){
			editP = GetEditPath( *srcPtr, count );
			if ( editP )
				CopyToEditPath( destPtr, editP->vhost, editP->path );	
		}
	}
	return num;
}


long BackupEditPath( EditPathRecPtr *dataPtr )
{
	return CopyEditPath( &gBackupEditPaths, dataPtr );
}


EditPathRecPtr RestoreBackupEditPath( EditPathRecPtr *dataPtr )
{
	RemoveEditPath( dataPtr );

	return gBackupEditPaths;
}

void DelEditPath( EditPathRecPtr *dataPtr, int n )
{
	EditPathRecPtr editP, schlast=0, schnext;
	int index=1,nindex=1;

	if ( dataPtr ){
		editP = *dataPtr;
		while( editP ){
			schnext = editP->next;
			if ( n ) {
				if ( n == index ){
					free( editP );
					if ( schlast )
						schlast->next = schnext;
					else
						*dataPtr = schnext;
				} else {
					schlast = editP;
					editP->index = nindex;
					nindex++; 
				}
			} else {
				free( editP );
			}
			index++;
			editP = schnext;
		}
	}
}

void ClearEditPath( EditPathRecPtr *dataPtr )
{
	long tot,cnt;

	if ( *dataPtr ){
		tot = GetTotalEditPath( *dataPtr );
		for( cnt=1; cnt<=tot; cnt++){
			DelEditPath( dataPtr, 1 );
		}
	}
}

EditPathRecPtr GetEditPath( EditPathRecPtr dataPtr, int n )
{
	EditPathRecPtr editP, schlast=0, schnext;
	int index=1;

	if ( (editP=dataPtr) ){
		while( editP ){
			schnext = editP->next;
			if ( n ){
				if ( n == index ){
					return editP;
				}
			}
			index++;
			editP = schnext;
		}
	}
	return 0;
}

extern struct		App_config MyPrefStruct;

void SaveEditPathNew( FILE *fp, EditPathRecPtr dataP )
{
	long count=1;

	if ( fp && dataP ){
		while( dataP ){
			fprintf( fp, "vhostpath " );
			if ( dataP->vhost )		fprintf( fp, "%s ", dataP->vhost );
			if ( dataP->path )		fprintf( fp, "%s\n", dataP->path );
			//fprintf( fp, "vhostpath %s %s\n", dataP->vhost, dataP->path );
			dataP = dataP->next;
		}
	}
}


