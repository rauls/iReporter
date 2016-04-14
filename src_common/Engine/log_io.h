#ifndef _LOGIO_H
#define _LOGIO_H

#include "FWA.h"

#include "zlib.h"

#include "HitData.h"
#include "myansi.h"

#include "engine_proc.h"	// for LogSubFormat

#define	MAXREADBUFFSIZE	(1024*1024*2)
#define	READBUFFSIZE	(1024*768)
#define	NETREADBUFFSIZE	(1024*16)
#define	CLUSTERREADBUFFSIZE	(1024*128)


extern short logType;
extern short logStyle;

typedef struct {
	long index;
	long fp;
	long ReadCount;
	long readIndex;
	long posIndex;
	long buffCount;
	long logError;
	long linepos;
	char *filename;
	char *buff;
	char linebuff[10240];
	char *ReadBuff,
		 *ReadBuff1;		//ReadBuff1[CLUSTERREADBUFFSIZE];
	size_t buff1Size;
} LogReadData, *LogReadDataP;

long GetFileLastDate( char *file );
void ErrorOpeningFile( const char *txt );
__int64 GetPosIndex( void );
void SetPosIndex( __int64 val );
void AddPosIndex( __int64 val );
long LogOpenQuiet( char *filename, __int64 *len );		// open the log without a visible error dialog
long LogOpen( char *filename, __int64 *len );
long LogClose( long logref, char *filename );
long LogRead( void *refNum, char *filename, char *ReadBuff, long ReadCount );
short LogReadLineEx(char *buff, long logref, char *filename, long multiline );
short MLogReadLine( LogReadDataP fp, long logref, char *filename, long flags );
#ifndef DEF_MAC
short ValidateFile( char *filetocheck );
short ValidatePath( char *pathtocheck );
#endif
long LogGetFormat( long ref , HitDataRec *Line, char *fname );
char *GetLogTypeName( long logType, char *txt );
__int64 GetLogFileType( char *filename, long *type, long *date1, long *date2, LogSubFormat* logSubFormat=0 );
#ifdef DEF_WINDOWS
__int64 WinGetLogFileType( char *newlogfile, long *type, long *date1, long *date2 );
#endif // DEF_WINDOWS

#define LogReadLine(a,b,f)		LogReadLineEx( a,b,f,0 )
#define LogReadMultiLine(a,b,f)	LogReadLineEx( a,b,f,1 )
#define AsyncLogReadLine(a,b)		LogReadLineEx( a,b,0|2 )
#define AsyncLogReadMultiLine(a,b)	LogReadLineEx( a,b,1|2 )

#endif // _LOGIO_H
