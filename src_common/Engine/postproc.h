#ifndef	__FHIST_H
#define	__FHIST_H

#ifdef __cplusplus
extern "C" {
#endif


typedef	struct LLStruct {
	void	*data;
	struct LLStruct	*next;
} LLStructRec, *LLStructRecPtr;

#define	LLISTSIZE		sizeof( LLStructRec )

long LListGetTotal( LLStructRecPtr LLPtr );
LLStructRecPtr LListAdd( LLStructRecPtr LLptr, void *data );
void LListDelete( LLStructRecPtr LLptr, int n );
void LListDeleteAll( LLStructRecPtr LLptr );


void StartFopenHistory( void );
void StopFopenHistory( void );
void AddFileHistory(char *fname);
bool RemoveFileHistory( const char* filename );
bool WithinFileHistory( const char* filename );


long PostProc_GzipLogFiles( char **logfiles, long n, int );
long PostProc_Bzip2LogFiles( char **logfiles, long n, int );
long CompressLogFiles( char **logfiles, long n, int type, int deletelog );

#define	COMPRESS_GZIP	0
#define	COMPRESS_BZIP2	1

void AddFileHistory(char *fname);
FILE *Fopen( char *file, char *type );
void *FileHistoryGet( long n );

long FileHistoryGetTotal( void );
int DoPreProc( void );
int DoPostProc( long );
void DoEmailSend( void );
#ifdef __cplusplus
}
#endif

#endif
