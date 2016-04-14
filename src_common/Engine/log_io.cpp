#include "FWA.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "myansi.h"
#include "datetime.h"
#include "config.h"	
#include "config_struct.h"	
#include "ResDefs.h"
#include "zlib.h"
#include "log_io.h"
#include "net_io.h"
#include "engine_process.h"
#include "unzip_layer.h"
#include "LogFileHistory.h"
#include "LogFileFormat.h"
#include "FileTypes.h"
#include "DateFixFileName.h"
#include "V5Database.h"

#ifdef DEF_MAC
	#include <errno.h>
	#ifdef DEF_APP_45

	#else
		#include "engine.h"
	#endif
	#include "MacStatus.h"
#else
#endif

#ifdef DEF_UNIX
	#include <errno.h>
	#include "main.h"
	//#include <bzlib.h>
#endif

#ifdef DEF_WINDOWS
	#include <sys/stat.h>
	#include <windows.h>
	#include "winmain.h"
	#include "resource.h"
	#include "createshortcuts.h"
extern HWND hwndParent;
#endif				



/*
typedef struct _WIN32_FIND_DATA {
    DWORD dwFileAttributes; 
    FILETIME ftCreationTime; 
    FILETIME ftLastAccessTime; 
    FILETIME ftLastWriteTime; 
    DWORD    nFileSizeHigh; 
    DWORD    nFileSizeLow; 
    DWORD    dwReserved0; 
    DWORD    dwReserved1; 
    TCHAR    cFileName[ MAX_PATH ]; 
    TCHAR    cAlternateFileName[ 14 ]; 
} WIN32_FIND_DATA; 

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

*/



long GetFileLastDate( char *file )
{
#if DEF_WINDOWS
	WIN32_FIND_DATA FindFileData;
	HANDLE	f;

	f = FindFirstFile( file, &FindFileData );

	if ( f!=INVALID_HANDLE_VALUE){
		FindClose( f );
		return FindFileData.ftLastWriteTime.dwLowDateTime;
	} else
		return 0;
#endif
	return 0;	
}


/* ---------------------------------------------------------------------------------------------------------- */



void ErrorOpeningFile( const char *filename )
{
	char errtxt[256];
	char msg[511];

#ifdef DEF_WINDOWS
	GetLastErrorTxt( errtxt );
#else
	mystrcpy( errtxt, strerror(errno) );
#endif

	sprintf( msg, ReturnString( IDS_ERR_CANTOPENLOG ), filename );
	strcat( msg, "\n" );
	strcat( msg, errtxt );
	ErrorMsg( msg );
}


long LogOpenQuiet( char *filename, __int64 *filelen )
{
	char	rawfileName[256];
	long	hnd = 0;

	DateFixFilename( filename, rawfileName );

	// Check if we trying to open a shortcut
	if ( IsShortCut( rawfileName ) ){
		char linkpath[256];
		mystrcpy( linkpath, rawfileName );
#if DEF_WINDOWS
		GetShortCut( linkpath, rawfileName );
#endif
	}

	// Now see if the we want to open a short cut to a URL
	if ( IsURLShortCut( rawfileName ) ){
		char linkpath[256];
		mystrcpy( linkpath, rawfileName );
#if DEF_WINDOWS
		GetURLShortCut( linkpath, rawfileName );
#endif
	}

	// Check for a just a plain URL
	if ( IsURL( rawfileName ) ){
		StatusSetID( IDS_REMOTELOG , strrchr( rawfileName,'/' ) );
		hnd = (long)INetOpen( rawfileName, filelen );
		return hnd;
	}

	// Check other types
	char format[100];
	// determine if its PKZIP file and if so... dont open it, YET
	if ( IsFileInvalid( rawfileName, format ) )
		return 0;

#ifndef DEF_MAC
	if ( IsPKZIP( rawfileName ) )
		hnd = (long)UnzipOpen( rawfileName, NULL );
	else
#ifdef _BZLIB_H
	if ( aIsBzFile( rawfileName ) )		// PLEASE DO THIS SOON FOR MAC.... at least for manual completeness, hey OSX people will love it.
		hnd = (long)BZ2_bzopen( rawfileName, "rb" );
	else 
#endif
#endif
	{
		hnd = (long)gzopen( rawfileName, "rb" );
	}

	if ( filelen && hnd )
		*filelen = GetFileLength( rawfileName );

	return hnd;
}


long LogOpen( char *filename, __int64 *filelen )
{
	long fileHandle;

	fileHandle = LogOpenQuiet( filename, filelen );

	if( !fileHandle )
		ErrorOpeningFile( filename );

	return fileHandle;
}

/*

FormatMessage(     FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,    0,    NULL );

  */
long LogClose( long logref, char *filename )
{
	long ret = 0;
	if ( logref )
	{
		// It would be easy to support ZIP/BZIP2 for mac...  
#ifndef DEF_MAC
		if ( IsURL( filename ) )
			NetClose( (void*)logref );
		else
		if ( IsPKZIP( filename ) )
			UnzipClose( (void*)logref );
		else
 #ifdef _BZLIB_H
		if ( IsBZIP( filename ) )
			BZ2_bzclose( (void*)logref );
		else
 #endif
#endif
			ret = gzclose( (gzFile)logref );
	}
	return ret;
}





static 	char		ReadBuff1[MAXREADBUFFSIZE], buff1_empty = 0;
static 	char		ReadBuff2[MAXREADBUFFSIZE], buff2_empty = 0;

static	char		*ReadBuff = ReadBuff1;

static __int64 posIndex;
long readIndex, ReadCount = READBUFFSIZE, buffCount = 0, res[32];


__int64 GetPosIndex( void )
{
	return posIndex;
}

void SetPosIndex( __int64 val )
{
	posIndex=val;
}

void AddPosIndex( __int64 val )
{
	posIndex+=val;
}

long LogRead( void *refNum, char *filename, char *databuffer, long ReadCount )
{
	if ( refNum ){
#ifndef DEF_MAC
		if ( IsURL( filename )  )
			return NetRead( (void*)refNum, databuffer, ReadCount );
		else 
		if ( IsPKZIP( filename ) )
			return UnzipGetData( (void*)refNum, (unsigned char *)databuffer, (size_t)ReadCount );
		else 
#ifdef _BZLIB_H
		if ( IsBZIP( filename ) )	// please let the mac do this one day... for completeness sake and less #ifdefs
			return BZ2_bzread( (void*)refNum, databuffer, ReadCount );
		else
#endif
#endif
			return gzread( (gzFile)refNum, databuffer, ReadCount );
	}
	return 0;
}




/*------------------------------------------------------

	RAfgets  -- read ahead get line from a log
	using buffering

	callas
		LogGetFormat( logRef[logIdx] , &Line);

-------------------------------------------------------*/
#define	LOGIO_ASYNC		0x02
#define LOGIO_MULTILINE 0x01
#define LOGIO_MASK		0x03

// static variable reader, not thread safe
short LogReadLineEx( char *buff, long logref, char *filename, long flags )
{
	short 	rindex=0,
			running=1;

	if ( IsURL( filename ) || *filename == '\\' )
		ReadCount = NETREADBUFFSIZE;
	else
		ReadCount = READBUFFSIZE;

	if ( buff == (char*)-1 ){	// go back to beginning of buffer
		readIndex=0;				
		posIndex = 0;
		ReadBuff = ReadBuff1;
	} else
	if ( buff ) {
		register char	c,lc,*src, *dst, eol=0;
		src = ReadBuff + readIndex;
		dst = buff;

		while(running) {	
			// if the buffer is empty, top it up first to prevent overun
			if (readIndex >= buffCount) {
				// dual thread reading...
				//buffCount = ReadCount = TellThreadtoRead( refNum );
				// syncrounous reading
				buffCount = ReadCount = LogRead( (void*)logref, filename, ReadBuff, ReadCount );
#ifdef DEF_DEBUG
				OutDebugs( "LogRead() more %d bytes from %s", buffCount, filename );
#endif

				if (buffCount<=0 && buffCount>=-9) {
					*dst = 0;		//buff[rindex]='\0';
					running = 0;
				}
				readIndex = 0;		//store the index to the bytes read
				src = ReadBuff;
			} else {
				c = *(src++);		//c = ReadBuff[readIndex];
				*dst = c;			//buff[rindex] = c;

				// check if the line has finished
				if ( flags & LOGIO_MULTILINE ){
					if ( lc == '\n' && c == '\n' )
						eol = 1;
				} else {
					if ( c == '\r' || c == '\n' )
					{
						eol = 1;
						while ( *src == '\r' || *src == '\n' )
						{
							src++; // Skip over extra end-of-line chars
							readIndex++;
							rindex++;
						}
					}
				}

				if ( eol || (rindex >= (10240-1)) ){
					running = 0;
					*dst = 0;		//buff[rindex]=0;
				}

				dst++;
				lc=c;
				rindex++;
				readIndex++;
			}
		}
	} else {
		//initialise readahead routines when buffer is nil
		//buffCount = ReadCount = TellThreadtoRead( refNum );
		ReadBuff = ReadBuff2;
		ReadCount = 4*1024;
#ifdef DEF_DEBUG
		OutDebugs( "LogRead(): try %d bytes from %s", ReadCount, filename );
#endif
		buffCount = ReadCount = LogRead( (void*)logref, filename, ReadBuff, ReadCount );
#ifdef DEF_DEBUG
		OutDebugs( "LogRead(): done %d bytes from %s", buffCount, filename );
#endif
		if ( buffCount > 0 )
			memcpy( ReadBuff1, ReadBuff, buffCount );
		readIndex = 0;	//store the index to the bytes read
		posIndex = 0;
	}

	posIndex += rindex;
	if (rindex>0) 
		return 1;
	else
		return 0;
}





// dynamic buffer/varialbe log reader, the must to use
short MLogReadLine( LogReadDataP fp, long logref, char *filename, long flags )
{
	short 	rindex=0,
			running=1;

	if ( IsURL( filename ) )
		fp->ReadCount = NETREADBUFFSIZE;
	else
		fp->ReadCount = fp->buff1Size;

	if ( fp->buff == (char*)-1 ){	// go back to beginning of buffer
		fp->readIndex=0;				
		fp->posIndex = 0;
		fp->ReadBuff = fp->ReadBuff1;
	} else
	if ( fp->buff ) {
		char	c,lc, *src, *dst, eol=0;
		src = fp->ReadBuff + fp->readIndex;
		dst = fp->buff;

		while(running && src ) {	
			// if the buffer is empty, top it up first to prevent overun
			if (fp->readIndex >= fp->buffCount) {
				fp->buffCount = fp->ReadCount = LogRead( (void*)logref, filename, fp->ReadBuff, fp->ReadCount );
#ifdef DEF_DEBUG
				OutDebugs( "LogRead() %d bytes from %s", fp->ReadCount, filename );
#endif
				if (fp->buffCount<=0 && fp->buffCount>=-9) {
					*dst = 0;	running = 0;
				}
				fp->readIndex = 0;		//store the index to the bytes read
				src = fp->ReadBuff;
			} else {
				c = *(src++);
				*dst = c;

				// check if the line has finished (We dont need multiline for clusters)
				//if ( flags & LOGIO_MULTILINE ){
				//	if ( lc == '\n' && c == '\n' )
				//		eol = 1;
				//} else 
				{
					if ( c == '\r' || c == '\n' ){
						eol = 1;
						if ( *src == '\n' ){
							src++; rindex++;  fp->readIndex++;
						}
					}
				}
 
				if ( eol || (rindex >= 10240) ){
					running = 0;		//buff[rindex]=0;
					*dst = 0;
				}

				dst++;
				lc = c;
				rindex++;
				fp->readIndex++;
			}
		}
	} else {
		//initialise readahead routines when buffer is nil
		//buffCount = ReadCount = TellThreadtoRead( refNum );
		fp->buffCount = fp->ReadCount = LogRead( (void*)logref, filename, fp->ReadBuff, 8000 );
		fp->readIndex = 0;
		fp->posIndex = 0;
	}

	fp->posIndex += rindex;
	if ( rindex>0 )
		return 1;
	else
		return 0;
}



/*
 CLUSTERING NOTES:
 PRE-READ buffers for clusters will be 32k each to make sure not too much ram is used, although 128k might be better.

 1. open all files at once (ClusterOpen())
 2. pre-read X K for all files  (use a new ClusterRead() call or something simpler)
 3. in the loop, read one line for each file
 4. then decode the one line into the localy stored LINE structure for each file stream
 5. scan all LINES to pick the LINE with the earliest date.
 6. process that LINE
 7. read next line and decode it , replace that LINE now with that line in that file stream.
 8. goto 5

 Thats it and we have our million $ cluster code.




  MULTI_CPU_ARCHITECTURE:
  MCPU_ReadLine(),

  PROCESS_A.
  1. this will pre-load as normal, but also decode all the lines into field structures, of all of the 256k
     or whatever buffer it is, into Line[100]. lines worth.  (engine_proc.cpp handling)
  2. it will wait until there is an empty Line[] structure and re-fill it with the next line.
  3. then loop back up repeating this on and on.


  PROCESS_B.
  this is the normal processing, ie the LogReadLine() that will just find the next Line[], ready to be used
  copy it to a global area and proceed as normal so the main process loop has no idea its in multicpu mode, 
  it just gets the lines as they come.

  if ( cpu>1 ) CPU_ReadLine = MCPU_ReadLine; else CPU_ReadLine = SCPU_ReadLine;		//init function caller
  
  */








/*
static		char *threadstate_buff = 0;
static		long threadstate_count = 0;
static HANDLE 		ghLogReadThread = NULL;

DWORD WINAPI LogReadThread( PVOID ref )
{
	OutDebug( "Starting thread" );
	while( !ghLogReadThread );

	while( ghLogReadThread ){
		if( buff1_empty && threadstate_count == 0 ){
			buff2_empty = 1;
			OutDebug( "Read 1" );
			threadstate_count = LogRead( ref, ReadBuff1, ReadCount );
			if ( threadstate_count == 0 ) {
				threadstate_count = -1;
				OutDebug( "read=0, SuspendThread, 1" );
				SuspendThread( ghLogReadThread );
			}
			OutDebug( "Read 1 done" );
			threadstate_buff = ReadBuff1;
			buff1_empty = 0;
		}

		if( buff2_empty && threadstate_count == 0 ){
			buff1_empty = 1;
			OutDebug( "Read 2" );
			threadstate_count = LogRead( ref, ReadBuff2, ReadCount );
			if ( threadstate_count == 0 ) {
				threadstate_count = -1;
				OutDebug( "read=0, SuspendThread, 2" );
				SuspendThread( ghLogReadThread );
			}
			OutDebug( "Read 2 done" );
			threadstate_buff = ReadBuff2;
			buff2_empty = 0;
		}
		Sleep(5);
	}
	OutDebug( "Exiting thread" );
}


long SpawnLogRead( void *ref )
{
	DWORD dwThreadId, dwThrdParam = 1;

	if ( ghLogReadThread ){
		OutDebug( "Kill LogRead thread" );
		CloseHandle( ghLogReadThread );
	}

	OutDebug( "Create LogRead thread" );
	ghLogReadThread = 1;
	ghLogReadThread = CreateThread( 
		NULL,                        // no security attributes 
		0,                           // use default stack size  
		LogReadThread,     // thread function 
		ref,                // argument to thread function 
		0,                           // use default creation flags 
		&dwThreadId);                // returns the thread identifier  

    OutDebug( "SetThreadPriority" );

	if (ghLogReadThread) {
		SetThreadPriority( ghLogReadThread, THREAD_PRIORITY_BELOW_NORMAL );		//THREAD_PRIORITY_BELOW_NORMAL
	}
	return(1);
}

long TellThreadtoRead( void *ref )
{
	long rc;

	if ( ref ){
		if ( !ghLogReadThread ) {
			OutDebug( "SpawnLogRead" );
			threadstate_count = 0;
			buff1_empty = 1;
			SpawnLogRead( ref );
		}
		if ( threadstate_count == -1 ){
			OutDebug( "ResumeThread" );
			threadstate_count = 0;
			buff1_empty = 1;
			ResumeThread( ghLogReadThread );
		}

		//OutDebug( "wait for read" );
		while( threadstate_count == 0 );
		OutDebug( "wait for read done" );
		rc = threadstate_count;
		ReadBuff = threadstate_buff;

		threadstate_count = 0;
		return rc;
	} else {
		threadstate_count = 0;
	}
}

  */







#ifndef DEF_MAC
// This include is needed, why was it taken away?
// stat should work on a mac, if not CW sucks
#include <sys/stat.h>

short ValidatePath( char *pathtocheck )
{
	char	path[512];
	struct	stat	pathStat;
	long	err = 0;

	PathFromFullPath( pathtocheck, path );

	DateFixFilename( path, NULL );

	if ( long len=strlen( path ) ){
		if ( path[len-1] == '\\' )
			path[len-1] = 0;
		err = stat( path, &pathStat );
	}

	return (short)err;
}
    

short ValidateFile( char *filetocheck )
{
	//struct	stat	fileStat;
	long	hnd;

	if ( IsURL( filetocheck ) ){
		return 1;
	} else {
		hnd = LogOpen( filetocheck, NULL );
	}
	//err = stat( filetocheck, &fileStat );

	if ( hnd ){
		LogClose( hnd, filetocheck );
		return 1;
	}
	return 0;
}

#endif

#include "config_struct.h"
#include "HitData.h"
#include "myansi.h"
#include <string.h>

#define FWA_VHOST_LOG_MASK		0x1000

char *GetLogTypeName( long logType, char *txt )
{
	switch( logType&0xff )
	{
		case LOGFORMAT_COMMON:		mystrcpy( txt, "Common" ); break;
		case LOGFORMAT_NCSA:		mystrcpy( txt, "NCSA Extended" ); break;
		case LOGFORMAT_MACHTTP:		mystrcpy( txt, "Webstar" ); break;
		case LOGFORMAT_PURVEYER:	mystrcpy( txt, "Purveyer" ); break;
		case LOGFORMAT_NETSCAPE:	mystrcpy( txt, "Netscape" ); break;
		case LOGFORMAT_IIS:			mystrcpy( txt, "MS-IIS" ); break;
		case LOGFORMAT_NETCACHE:	mystrcpy( txt, "NetCache NetApp" ); break;
		case LOGFORMAT_WEBSITE:		mystrcpy( txt, "Website" ); break;
		case LOGFORMAT_FILEMAKER:	mystrcpy( txt, "Filemaker" ); break;
		case LOGFORMAT_FIRSTCLASS:	mystrcpy( txt, "FirstClass" ); break;
		case LOGFORMAT_IIS4:		mystrcpy( txt, "W3C" ); break;
		case LOGFORMAT_W3C:			mystrcpy( txt, "W3C" ); break;
		case LOGFORMAT_MSISA:		mystrcpy( txt, "MS-ISA" ); break;
		case LOGFORMAT_WELCOME:		mystrcpy( txt, "Welcome" ); break;
		case LOGFORMAT_HOTLINE:		mystrcpy( txt, "Hotline" ); break;
		case LOGFORMAT_OPENMARKET:	mystrcpy( txt, "OpenMarket" ); break;
		case LOGFORMAT_SENDMAIL:	mystrcpy( txt, "Sendmail" ); break;
		case LOGFORMAT_WUFTPD:		mystrcpy( txt, "Wu.FTPd Server" ); break;
		
		case LOGFORMAT_REALSERVER:  mystrcpy( txt, "RealServer" ); break;
		case LOGFORMAT_QTSS:		mystrcpy( txt, "QTSS" ); break;
		case LOGFORMAT_WINDOWSMEDIA:mystrcpy( txt, "WindowsMedia" ); break;

		case LOGFORMAT_UNIXFTPD:	mystrcpy( txt, "UnixFTP" ); break;
		case LOGFORMAT_HOMEDOOR:	mystrcpy( txt, "HomeDoor" ); break;
		case LOGFORMAT_ZEUS:		mystrcpy( txt, "Zeus" ); break;
		case LOGFORMAT_BOUNCE:		mystrcpy( txt, "Bounce" ); break;
		
		case LOGFORMAT_SQUID:		mystrcpy( txt, "Squid" ); break;
		case LOGFORMAT_IISPROXY:	mystrcpy( txt, "MSproxy" ); break;
		case LOGFORMAT_WSPROXY:		mystrcpy( txt, "WSproxy" ); break;
		case LOGFORMAT_RADIUS:		mystrcpy( txt, "Radius" ); break;
		case LOGFORMAT_CISCO:		mystrcpy( txt, "Cisco" ); break;
		case LOGFORMAT_FIREWALL1:	mystrcpy( txt, "FireWall-1" ); break;
		case LOGFORMAT_RAPTOR:		mystrcpy( txt, "Raptor Firewall" ); break;
		case LOGFORMAT_CUSTOM:		mystrcpy( txt, "Custom" ); break;
		case LOGFORMAT_V4DATABASE:	mystrcpy( txt, "FWA DB" ); break;
		case LOGFORMAT_V5DATABASE:	mystrcpy( txt, "FWA XDB" ); break;
		case LOGFORMAT_UNKNOWN:
		default:
			mystrcpy( txt, "[Unknown]" );
			return txt;
	}
	// Show Virtual Host status inthe log name too 
	if ( IsDebugMode() ){
		if ( logType & FWA_VHOST_LOG_MASK )		strcat( txt, " Vhost" );
	}
	return txt;
}

short logType, logStyle;


long LogGetFormat( long ref , HitDataRec *Line, char *fname )
{
	char	log_buff[10240];
	short i = 0;

	logType = LOGFORMAT_UNKNOWN;

	LogReadLine( 0, ref, fname );		// init readahead routines
	//for( i=0; i<7; i++){
	while( (logType == LOGFORMAT_UNKNOWN || !Line->date) && i<100 )
	{
		long err=LogReadLine( log_buff, ref, fname );	// read one line
		if ( err != 0 && err != -2 )
			err = ProcessLogLine( log_buff, Line );	// determine what format it is in
		i++;
	}
	LogReadLine( (char*)-1, ref, fname );		// reinit readahead routines back to 0

	return 1;
}

// 990525 RS, get the log's raw filesize and log type
__int64 GetLogFileType( char *filename, long *type, long *date1, long *date2, LogSubFormat* logSubFormat/*=0*/ )
{
	DEF_ASSERT(type);

	// If its a URL or some external link, then dont get the info since its too slow
	// accept it as is.
	if ( strstr( filename, "ftp://" ) ||
		 strstr( filename, "http://" ) ||
		 IsFileaFolder( filename ) ||
		 IsURLShortCut( filename )
		){
		if ( date1 && date2 )
			*date1 = *date2 = *type = 0;
		return 0;
	}

	time_t logDays;
	__int64 fileSize;
	CQV5Database::Type v5DBType;

	if( CQV5Database::isV5Database( filename, &logDays, &fileSize, &v5DBType ) )
	{
		*type=LOGFORMAT_V5DATABASE;

		if( date1 )
		{
			*date1=logDays;
		}

		if( logSubFormat )
		{
			*logSubFormat=v5DBType==CQV5Database::Web ? LOGSUBFORMAT_V5DATABASE_WEB : LOGSUBFORMAT_V5DATABASE_STREAMING;
		}

		return fileSize;
	}

	if( logSubFormat )
	{
		*logSubFormat=LOGSUBFORMAT_UNKNOWN;
	}

	long fp=LogOpen( filename, &fileSize );

	if ( fp )
	{
		HitDataRec		Line;
		memset( &Line, 0, sizeof( HitDataRec ) );
		LogGetFormat( fp , &Line, filename );
		LogClose( fp, filename );

		if ( date1 && date2 )
		{
			struct tm		logDate;
			
			*date1 = *date2 = 0;
			if ( Line.date ){
				StringToDaysDate( Line.date, &logDate, 0);
				StringToDaysTime( Line.time, &logDate);
				Date2Days( &logDate, &logDays );
				*date1 = logDays;
			}
			if ( Line.vhost ){
				if ( Line.vhost[0] != '-' )
					logType |= FWA_VHOST_LOG_MASK;					
			}

		}
		*type = logType;
		logType = LOGFORMAT_UNKNOWN;
		return fileSize;
	}
	return -1;
}

#ifdef DEF_WINDOWS
__int64 WinGetLogFileType( char *newlogfile, long *type, long *date1, long *date2 )
{
	__int64 size = 0;
	if ( newlogfile )
	{
		size = GetLogFileType( newlogfile, type, date1, date2 );
		return size;
	}
	return -1;
}
#endif // DEF_WINDOWS



