/*

 more compresion codecs are at http://web.mathematik.uni-stuttgart.de/tex-archive/archive-tools/

*/
#include "FWA.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>


#include "myansi.h"
#include "zlib.h"
#include "log_io.h"
#include "net_io.h"

#if		DEF_WINDOWS
	#include <windows.h>
	#include "winmain.h"
	#include "winutil.h"
	#include "resource.h"
	#include "zip.h"
#elif	DEF_UNIX
	#include "main.h"
	#include "zip.h"
	#include "bzlib.h"
#elif	DEF_MAC
	#include "setutil.h"
#endif

#include "config.h"
#include "postproc.h"
#include "Utilities.h"
#include "EngineStatus.h"
#include "FileTypes.h"
#include "DateFixFileName.h"
#include "GlobalPaths.h"

void FtpClose( void *fs );
extern "C" int MailTo( char *to, char *from, char *subject, char *body, char *smtpServer );

long LListGetTotal( LLStructRecPtr LLPtr )
{
	long	index=0;

	if ( LLPtr ){
		while( LLPtr ){
			LLPtr = LLPtr->next;
			index++;
		}
	}
	return index;
}

LLStructRecPtr LListAdd( LLStructRecPtr LLptr, void *data )
{
	LLStructRecPtr nextP;

	nextP = (LLStructRecPtr)malloc( LLISTSIZE );
	if ( nextP ){
		nextP->data = data;
		nextP->next = LLptr;
	}

	return nextP;
}

void LListDelete( LLStructRecPtr LLptr, int n )
{
	LLStructRecPtr schlast=0, schnext;
	int index=1;

	if ( LLptr ){
		while( LLptr ){
			schnext = LLptr->next;
			if ( n ){
				if ( n == index ){
					LLptr->next = 0;
					free( LLptr->data );
					free( LLptr );
					if ( schlast )
						schlast->next = schnext;
				} else
					schlast = LLptr;
			} else {
				LLptr->next = 0;
				free( LLptr->data );
				free( LLptr );
			}
			index++;
			LLptr = schnext;
		}
	}
}

void *LListGetData( LLStructRecPtr LLptr, int n )
{
	LLStructRecPtr schlast=0, schnext;
	int index=1;

	if ( LLptr ){
		while( LLptr ){
			schnext = LLptr->next;
			if ( n ){
				if ( n == index ){
					return LLptr->data;
				}
			}
			index++;
			LLptr = schnext;
		}
	}
	return 0;
}


void LListDeleteAll( LLStructRecPtr LLptr )
{
	LLStructRecPtr next;

	if ( LLptr ){
		while( LLptr ){
			next = LLptr->next;
			LLptr->next = 0;
			free( LLptr->data );
			free( LLptr );

			LLptr = next;
		}
	}
}


static	LLStructRecPtr gFopenHistoryPtr = NULL;
static	int doFopenHistory = 0;

void StartFopenHistory( void )
{
	if ( gFopenHistoryPtr ) {
		LListDeleteAll( gFopenHistoryPtr );
		gFopenHistoryPtr = NULL;
	}
	doFopenHistory = 1;
}

void StopFopenHistory( void )
{
	if ( gFopenHistoryPtr ) {
		LListDeleteAll( gFopenHistoryPtr );
		gFopenHistoryPtr = NULL;
	}
	doFopenHistory = 0;
}



void AddFileHistory(char *fname)
{
    char *filename;

	if( doFopenHistory && !WithinFileHistory( fname ) ) {
        if (filename = (char *)malloc( strlen(fname)+32 )) {
			LLStructRecPtr t;
            mystrcpy( filename, fname );
			t = LListAdd( gFopenHistoryPtr, filename );
			if ( t )
				gFopenHistoryPtr = t;
        }
    }
}




bool RemoveFileHistory( const char* filename )
{
	DEF_PRECONDITION( filename );

	bool found( false );

	LLStructRecPtr current=gFopenHistoryPtr;
	LLStructRecPtr previous=0;

	// for each list item
	while( current )
	{
		// if the current item matches the required filename
		if( !mystrcmpi( filename, reinterpret_cast<char*>(current->data) ) )
		{
			// remove (isolate) the current item from the list
			if( previous )
			{
				DEF_ASSERT( previous->next==current );
				previous->next=current->next;
			}
			else
			{
				DEF_ASSERT( current==gFopenHistoryPtr );
				gFopenHistoryPtr=current->next;
			}

			// clean up the current item
			free( current->data );
			free( current );

			found=true;
			break;
		}

		previous=current;
		current=current->next;
	}

	return found;
}


bool WithinFileHistory( const char* filename )
{
	DEF_PRECONDITION( filename );

	bool found( false );

	LLStructRecPtr current=gFopenHistoryPtr;

	// for each list item
	while( current )
	{
		// if the current item matches the required filename
		if( !mystrcmpi( filename, reinterpret_cast<char*>(current->data) ) )
		{
			found=true;
			break;
		}

		current=current->next;
	}

	return found;
}


FILE *Fopen( char *fname, char *type )
{
	FILE *fp;
	char file[300];

	if ( !fname ) return 0;

	fp = NULL;

	DateFixFilename( fname, file );

	// If file name contains bad chars, clean them
	if ( strchr( type, 'w' ) )
		CleanStringOfBadChars( file );

#ifdef DEF_MAC
	setType(getType(file));
#endif
#ifdef DEF_DEBUG
	OutDebugs( "Fopen() opening file %s", file );
#endif
	fp = fopen( file, type );

	if ( fp )
	{
		AddFileHistory( file );
	}
	return fp;
}

void *FileHistoryGet( long n )
{
	return LListGetData( gFopenHistoryPtr, n );
}

long FileHistoryGetTotal( void )
{
	return LListGetTotal( gFopenHistoryPtr );
}

short CopyFileTo( char *src,char *dest )
{
	if ( !IsURL( dest ) ){
#if DEF_WINDOWS
		return CopyFile( src, dest, 0 );
#endif
	}
	return 0;
}
		
	
// returns BYTES transfered or erroers in negative format
// 0   stopped
#define	FTPERR_COMPLETE		1
#define	FTPERR_STOPPED		0
#define	FTPERR_NOFILE		-1
#define	FTPERR_CANTMAKEDIR	-2
#define	FTPERR_COMPLETEFAIL	-3

// -2  cant make dir
long FtpFileUpload( void *fs, char *localfilename, char *remotepath, char *file )
{
	long length=0, failedmkdir=FALSE, datadone;

	if ( fs && !IsStopped() )
	{
		void	*hFtpFile;
		FILE	*ff;
		long	dataleft, dataread, perc;
		char	*buffer=NULL;
		char	msg[500];

		if ( !(ff = fopen( localfilename, "rb" )) )
		{
			StatusSetID( IDS_ERR_FILEFAILED, localfilename );
			return FTPERR_NOFILE;
		}

		if ( fs && ff )
		{
			char dir[256], *p;
			static char lastdir[256];
			int tries = 0;

			// Convert all Windows slashes to Unix ones.
			p = remotepath;
			while( *p )
			{
				if ( *p == '\\' ) *p = '/';
				p++;
			}

			// Try to create the remote directories first.
			if ( p )
			{
				// Try to open remote file, if we cant make the remote dir.
				while( (hFtpFile = (void*)FtpOpen( fs, remotepath, 'W' )) == NULL && tries<5 )
				{
					PathFromFullPath( remotepath, dir );
					if ( strcmp( dir, lastdir ) ){
						//long l=mystrcpy( lastdir, dir );
						//if ( dir[l-1] == '/' ) dir[l-1]=0;
						sprintf( msg, "Making dir %s...", dir );
						StatusSet( msg );

						if ( !FtpMakeDir( fs, dir ) )
						{
							failedmkdir = TRUE;
							OutDebugs( "Cannot create remote ftp directory %s", dir );
						} else {
							failedmkdir = FALSE;
						}
					}
					tries++;
					OutDebug( "Trying again to write to server..." );
				}
			}


			// If remote file is opened.
			if ( hFtpFile ) 
			{
				StatusSetID( IDS_UPLOADINGTO, remotepath );
				const long read_size = (4*1024);

				datadone = 0;
				length = dataleft = (long)GetFileLength( localfilename );
				if ( length )
					buffer = (char*)malloc( read_size );

				if ( buffer )
				{
					while( dataleft>0 && buffer && !IsStopped() )
					{
						dataread = fread( buffer, 1, read_size, ff );
						if ( dataread )
							NetWrite( hFtpFile, buffer, dataread );

						dataleft -= dataread;
						datadone += dataread;

						// ------------------------------------------------------
						perc = (long)(100*((length-dataleft)/(float)length));
						sprintf( msg, "Uploading %s  (%d bytes %d%%)", remotepath, datadone, perc );
						StatusSet( msg );
						// ------------------------------------------------------
					}
					if ( datadone == length )
						OutDebug( "Uploaded complete." );

					free( buffer );
				}
				FtpClose( hFtpFile );
				OutDebug( "Ftp File Closed" );
			}
		}
		if ( ff )
			fclose( ff );
	}

	if ( failedmkdir )
		return FTPERR_CANTMAKEDIR;

	if ( IsStopped() )
		return FTPERR_STOPPED;

	if ( datadone == length )
		return FTPERR_COMPLETE;

	return FTPERR_COMPLETEFAIL;
}



int UploadFiles( void *fs, char *dest, char *sourcepath, char *ftppath, int deletesourcefiles )
{
	long	sourcepathlen;
	LLStructRecPtr next;
	long	count=0, filenum, perc, ret;

	next = gFopenHistoryPtr;
	sourcepathlen = strlen( sourcepath );

	filenum = LListGetTotal( gFopenHistoryPtr );

	while( next && !IsStopped() && fs )
	{
		perc = ((100*count)/filenum);
		StatusWindowSetProgress( perc, NULL );
	
		if ( IsURL( dest ) ){
			char remoteFullPath[512], localPath[512], *fileName;

			fileName = (char*)next->data + sourcepathlen;
			sprintf( remoteFullPath, "%s%s", ftppath, fileName );
			mystrcpy( localPath, (char*)next->data );

			ret = FtpFileUpload( fs, localPath, remoteFullPath, fileName );
			switch( ret )
			{
				case FTPERR_NOFILE:
					ErrorMsg( "Cannot open local file ...\n%s", localPath );
					return -1;
				case FTPERR_STOPPED:
					return -1;
				case FTPERR_CANTMAKEDIR:
					if ( CautionMsg( "Could not make directory %s on ftp server .\nMaybe path does not exist?", ftppath ) )
						return -2;
					break;
			}
		}

		// Delete source file after uploading.
		if( deletesourcefiles )
		{
			ret = remove( (char*)next->data );
		}
		next = next->next;
		count++;
	}
	return count;
}


// Use all FILES in output dir to upload, not just the written files.
void ResetUploadReportQueue( char *reportLocation )
{
	if ( gFopenHistoryPtr && !IsStopped() )
	{
		char	sourcepath[256];
		PathFromFullPath( reportLocation, sourcepath );
		DateFixFilename( sourcepath, 0 );
		AddWildCardReportFiles( sourcepath );
	}
}



int PostProc_UploadReport( const char *uploadURL, int deletereport, int zipreport, char *reportLocation )
{
	if ( gFopenHistoryPtr && !IsStopped() )
	{
		char	sourcepath[256];
		char	server[256], ftppath[256], name[64], passwd[32];
		char	*source;
		void	*fs;

		if ( IsURL( uploadURL ) )
		{
			char msg[256];
			ExtractUserFromURL( (char*)uploadURL, server, name, passwd, ftppath );
			DateFixFilename( ftppath, 0 );
			fs = (void*)FtpServerOpen( server, name, passwd );
			if ( fs ){
				sprintf( msg, "FTP %s@%s/%s", name,server,ftppath );
				StatusSet( msg );
			} else 
			{
				FtpServerOpenError( server );
				return 1;
			}
		}

		PathFromFullPath( reportLocation, sourcepath );
		DateFixFilename( sourcepath, 0 );

		// ---------------- UPLOAD ALL INDIVIDUAL FILES
		if ( !zipreport )
		{
			long files;
			files = UploadFiles( fs, (char*)uploadURL, sourcepath, ftppath, deletereport );
			if ( files>0 )
				OutDebugs( "%d files uploaded.", files );
			else
				OutDebugs( "Upload canceled" );

			FtpClose( fs );
			OutDebugs( "Ftp Site Closed" );
		} else
		// ----------------- UPLOAD SINGLE ZIP REPORT
		{
			long	uploadStatus = 0;
			char	srczipname[256];
			char	newfilename[256];

			mystrcpy( srczipname, reportLocation );
			DateFixFilename( srczipname, 0 );
			PathFromFullPath( srczipname, sourcepath );

			if ( !strstr( srczipname, ".zip" ) )
			{
				source = strrchr( srczipname, '.' );
				if ( source )
					mystrcpy( source, ".zip" );
				else
					return 3;
			}
			source = srczipname;

			// Find the FILENAME Component
			if ( strstr( srczipname, sourcepath ) )
			{
				source += strlen( sourcepath );
			}

			// generate remote full path PATH+NAME
			CopyFilenameUsingPath( newfilename, (char*)uploadURL, source );

			// Ok things are ok, lets now upload the zip file
			if ( IsURL( uploadURL ) )
			{
				char fullURLPath[256];
				sprintf( fullURLPath, "%s%s", ftppath, source );

				// upload the zip file
				uploadStatus = FtpFileUpload( fs, srczipname, fullURLPath, source );
				if ( uploadStatus == FTPERR_CANTMAKEDIR )
					ErrorMsg( "Could not make directory %s on ftp server .\nMaybe path does not exist?", ftppath );

				FtpClose( fs );
			}

			if( deletereport && uploadStatus == FTPERR_COMPLETE )
			{
				OutDebugs( "Deleting local zip file %s", srczipname );
				remove( srczipname );
			}
		}
	}
	return 0;
}






int PostProc_UploadLogs( char *uploadloglocation, int deletelog, int compressed_logs, char **files , long fileCount )
{
	char	newfilename[256];
	char	sourcepath[256];

	if ( files && fileCount )
	{
		char	*source, *p;
		long	count=0, filenum, perc, doftp=FALSE, ret;
		LLStructRecPtr next = gFopenHistoryPtr;
		char	server[128];
		char	ftppath[256],
				file[128];
		char	name[64];
		char	passwd[32];
		char	msg[256];
		void	*fs;

		if ( IsURL( uploadloglocation ) )
		{
			ExtractUserFromURL( uploadloglocation, server, name, passwd, ftppath );
			fs = (void*)FtpServerOpen( server, name, passwd );
			if ( fs ){
				doftp = TRUE;
				sprintf( msg, "FTP: %s@%s/%s", name,server,ftppath );
				StatusSet( msg );
			} else {
				FtpServerOpenError( server );
				return 1;
			}
		} else
			StatusSetID( IDS_COPYINGFILE, "." );

		// upload log files.....
		if ( fs )
		{
			count = 0;
			filenum = fileCount;

			// go through all logs.....
			while( files[count] && !IsStopped() )
			{
				perc = ((100*count)/filenum);
				StatusWindowSetProgress( perc, NULL );

				PathFromFullPath( files[count], sourcepath );
				source = files[count];
				if ( strstr( source, sourcepath ) )
				{
					source += strlen( sourcepath );
				}

				// Upload LOGS
				if ( doftp )
				{
					long upstatus;
					p = files[count] + strlen( sourcepath );
					if ( compressed_logs && !strstr( p, ".gz" ) ) 
						sprintf( file, "%s%s.gz", ftppath, p );
					else
						sprintf( file, "%s%s", ftppath, p );

					upstatus = FtpFileUpload( fs, files[count], file, p );
					if ( upstatus == FTPERR_COMPLETE  )
					{
						if( deletelog )
							ret = remove( files[count] );
					}
				}
				else 
				{
					CopyFilenameUsingPath( newfilename, uploadloglocation, source );
				}
				count++;
			}
		}

		if ( doftp )
			FtpClose( fs );
	} else
		return 2;
	return 0;
}


//	ShowProgress( 0, TRUE, buf2 );
// archive the report into ONE big ZIP file
void PostProc_ZipReport( char *reportLocation, char *zipfilename, int deletereport )
{

	if ( gFopenHistoryPtr ){
		char	newfilename[256];
		char	sourcepath[256], msg[128];
		long	count=0,filenum,perc = 0;

		LLStructRecPtr next = gFopenHistoryPtr;

		filenum = LListGetTotal( gFopenHistoryPtr );

		PathFromFullPath( reportLocation, sourcepath );
		DateFixFilename( sourcepath, 0 );

		if ( !strstr( reportLocation, ".zip" ) )
		{
			char *p;
			mystrcpy( newfilename, reportLocation );
			DateFixFilename( newfilename, 0 );

			p = strrchr( newfilename, '.' );
			if ( p )
				mystrcpy( p, ".zip" );
			else
				return;

			mystrcpy( zipfilename, newfilename );
			sprintf( msg, "Making ZIP %s",  newfilename );
			ShowProgress( perc, FALSE, msg );
		}

#ifdef _zip_H
		{
			zipFile zipData;
			char	*source;

			zipData = zipOpen( (const char *)newfilename, 0 );
			while( next && !IsStopped() )
			{
				perc = ((count*100)/filenum);
				source = (char*)next->data;

				sprintf( msg, "Adding to zip file %s",  FileFromPath( source,0 ) );
				ShowProgress( perc, FALSE, msg );

				long infilelen = GetFileLength( source );
				char *ram = (char*)malloc( infilelen+1 );
				if ( ram )
				{
					zip_fileinfo zinfo;

					memset( &zinfo, 0, sizeof(zip_fileinfo) );
					FileToMem( source, ram, infilelen );

					zipOpenNewFileInZip( zipData, source, &zinfo, 0, 0, 0, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION );
					zipWriteInFileInZip( zipData, ram , infilelen );
					zipCloseFileInZip( zipData );
					free( ram );
				}

				if( deletereport )
					remove( source );
				next = next->next;
				count++;
			}

			zipClose( zipData, NULL );
		}

#endif

#if DEF_WINZIP
		{
			ZCL		zipData;
			char	*source;

			memset( &zipData, 0, sizeof(ZCL) );
			zipData.Stdout = stdout;
			zipData.fQuiet = TRUE;
			zipData.fUpdate = TRUE;
			zipData.lpszZipFN = newfilename;		//"C:\install\test.zip";
			zipData.fLevel = 1;
			zipData.FNV = &source;
			zipData.argc = 1;
			while( next && !IsStopped() ){
				perc = ((count*100)/filenum);
				source = (char*)next->data;

				sprintf( msg, "Adding to zip file %s",  FileFromPath( source,0 ) );
				ShowProgress( perc, FALSE, msg );

				ZipUpFiles( &zipData );
				if( deletereport )
					remove( source );
				next = next->next;
				count++;
			}
		}
#endif
	}
}

/*
# Gzip compression size/level table	(about 1-2 seconds to compress)
8961532		RAW
			Compressed
392106		-1
388176		-2
374581		-3
324324		-4
305822		-5
295640		-6
286040		-7
276981		-8
272875		-9

# BZip2 compression size/level table (about 14-18seconds to compress)
8961532		RAW
			Compressed
242031		b1
187504		b3
180667		b4
174672		b5
170596		b6
168310		b8
165040		b9
*/


// This now supports gzip or bzip2 to compress with.
long CompressLogFiles( char **logfiles, long n, int type, int deletelog )
{
	long	count,  dataread, dataout, perc;
	char	newlogname[512],
			*logFN;
	long	failed = 1;

	for( count=0; count<n ; count++){
		logFN = logfiles[count];

		if ( strstr( logFN, ".gz" ) && type==0 )
			continue;
		if ( !IsURL(logFN) )
		{
			void *fp;
			void *outfp;
			char *ram, *p;
			long blocksize = 1024*32;

			StopAll( 0 );

			if ( ram = (char*)malloc( blocksize ) ){
				__int64 dataleft, length;

				if ( fp=(void*)LogOpen( logFN, &length ) ){
					int ret;

					sprintf( newlogname, "%s.gz", logFN );

					switch( type ){
						default:
						case COMPRESS_GZIP :
							sprintf( newlogname, "%s.gz", logFN );
							if ( p = strstr( newlogname, ".bz2" ) )
								mystrcpy( p, ".gz" );
							outfp = gzopen( newlogname, "wb6" );
							break;
#ifdef  _BZLIB_H
						// bzip2 is about 15X slower for 1/2 the size files, level6 is the best time-vs-size level roughly
						case COMPRESS_BZIP2 :
							sprintf( newlogname, "%s.bz2", logFN );
							if ( p = strstr( newlogname, ".gz" ) )
								mystrcpy( p, ".bz2" );
							outfp = BZ2_bzopen( newlogname, "wb6" );
							break;
#endif
					}

					dataout = 0;
					if ( outfp ){
						dataleft = length;
						dataread = 1;
						while( dataread>0 && !IsStopped() ){
							//OutDebugs( "dataleft = %d", dataleft );
							perc = (long)(100*((length-dataleft)/(float)length));
							//sprintf( msgtext, "Compressing %s ...", 100*((length-dataleft)/length) );
							ShowProgress( perc, FALSE, NULL );
							StatusSetID( IDS_COMPRESSING, perc, dataout/1024 );

							dataread = LogRead( fp, logFN, ram, blocksize );

							if ( dataread>0 )
							{
								dataleft -= dataread;
	
								if ( type == COMPRESS_GZIP )	dataout+=gzwrite( outfp, ram , dataread );
#ifdef _BZLIB_H
								if ( type == COMPRESS_BZIP2 )	dataout+=BZ2_bzwrite( outfp, ram , dataread );
#endif
							}
						}
						if ( type == COMPRESS_GZIP )	gzclose( outfp );
#ifdef _BZLIB_H
						if ( type == COMPRESS_BZIP2 )	BZ2_bzclose( outfp );
#endif
						if ( !IsStopped() ){
							__int64 newsize;
							FILE *newfp;
							failed = 0;
							if ( (newfp = fopen( newlogname, "ab+" )) ) {
								newsize = GetFPLength( newfp );

								if ( type == COMPRESS_BZIP2 ){
									long value;
									value = 0;
									fwrite( &value, 1, 4, newfp );
									value = (long)length;
									fwrite( &value, 1, 4, newfp );
								}
								fclose(newfp);
							}
							StatusSetID( IDS_COMPRESSDONE, dataout/1024, newsize/1024, 100*newsize/dataout );
						} else
							StatusSet( "Stopped" );

					}
					ret = LogClose( (long)fp, logFN );
					if ( deletelog && !failed){
						remove( logFN );
					}
				}
				free( ram );
			}
		}
	}
	return failed;
}




long PostProc_GzipLogFiles( char **logfiles, long n, int deletelog )
{
	return CompressLogFiles( logfiles, n, COMPRESS_GZIP, deletelog );
}


long PostProc_Bzip2LogFiles( char **logfiles, long n, int deletelog )
{
	return CompressLogFiles( logfiles, n, COMPRESS_BZIP2, deletelog );
}









long FtpFileDownload( void *fs, char *url, char *dest, char *fullpath, long length )
{
	long	dataread, datatotal=0;

	if ( fs )
	{
		void	*fp;
		FILE	*ff;
		char	*buffer=NULL;

		if ( fs && !IsStopped() )
		{
			if ( !(ff = fopen( dest, "wb+" )) )
			{
				StatusSetf( "Cannot open local file %s for writing.", dest );
				return -2;
			}
	
			fp = (void*)FtpOpen( fs, fullpath, 'R' );
			if ( fp ) 
			{
				if ( !length )
					length = FtpFileGetSize( url );

				StatusSetID( IDS_DOWNLOADING, fullpath );

				buffer = (char*)malloc( 1024*4 );

				if ( length && buffer )
				{
					long writesize;

					dataread = 1;
					while( dataread>0 && buffer && !IsStopped() )
					{
						//sprintf( msg, "Downloading %s...%d KB (%.2f%%)", fullpath, (datatotal)/1024, (100*((datatotal)/(float)length)) );
						StatusSetID( IDS_DOWNLOADINGKB, fullpath, (datatotal)/1024, (100*((datatotal)/(float)length))  );
	
						dataread = NetRead( fp, buffer, 1024*1 );
						datatotal+= dataread;
						if ( dataread ){
							writesize = fwrite( buffer, 1, dataread, ff );
							if ( writesize == 0 ){
								dataread = 0;
								datatotal = -2;
							}
						}
					}
					free( buffer );
				}
				FtpClose( fp );
			} else 
			{
				const char *msg = NetworkErr( NULL );
				if ( !msg ) msg = "Maybe path does not exist?";
				ErrorMsg( "Cannot open ftp file %s\nbecause ...\n%s", fullpath, msg ); 
				datatotal = -1;
			}
			fclose( ff );
		}

		if ( IsStopped() )
		{
			StatusSetID( IDS_DOWNLOADSTOP );
			remove( dest );
			datatotal = -1;
		}

	}
	return datatotal;
}


#include <fcntl.h>

#if DEF_WINDOWS
	#include <wininet.h>
#endif

int PreProc_FtpDownloadFiles( char *downloadURL, char *tempLocation, int deleteFiles )
{

	if ( IsURL( downloadURL ) && !IsStopped() )
	{
		char	*p;
		long	count=0, filenum;
		char	server[128], path[256], name[64], passwd[32];
#if DEF_WINDOWS
		WIN32_FIND_DATA  lpFindFileData;
		HINTERNET hFind, fs;

		char msg[256];

		ExtractUserFromURL(  downloadURL, server, name, passwd, path );
		DateFixFilename( path, 0 );

		OutDebugs( "Doing PreProcess Download %s...", path );

		fs = (void*)FtpServerOpen( server, name, passwd );

		if ( !fs )
		{
			FtpServerOpenError( server );
			return -1;
		}

		if ( fs )
		{
			long flags = INTERNET_FLAG_NO_CACHE_WRITE;
			char newpath[512];

			// *****************************************************************
			// To be more friendly with various FTP servers, we need to break
			// up the filename from the directory that it is contained in.
			// We can then change to the directory prior to fetching the file.
			// The issue here is that some FTP servers can not get a file that
			// has path information in it (ie: /directory1/dir2/file.dat)
			// *****************************************************************
			int iLen;
			for (iLen=mystrlen(path);iLen && path[iLen]!='/';--iLen)
				;
			if (!iLen)	// then there is no '/'
			{
				mystrcpy( newpath, path+1 );
			}
			else
			{
				path[iLen] = NULL;	// Set the '/' to a NULL so we have a path up to the name.
				if (!::FtpSetCurrentDirectory(fs,"/"))	// Set it to root just to be sure.
				{
					// we have a problem, however there is no real way to action this.
					// so lets just hope that the fetch will still work.
				}

				if (!::FtpSetCurrentDirectory(fs,path))
				{
					// again.
					// we have a problem, however there is no real way to action this.
					// so lets just hope that the fetch will still work.
					path[iLen] = '/';
				}

				mystrcpy( newpath, path+iLen+1 );
			}


			hFind = FtpFindFirstFile( fs, newpath, &lpFindFileData, flags , 0 );
			if ( !hFind ){
				unsigned long len = 512;
				FtpServerClose( fs );

				OutDebugs( "%s Not Found....Trying root level path instead...", newpath );
				fs = (void*)FtpServerOpen( server, name, passwd );
				if ( fs )
				{
					FtpGetCurrentDirectory( fs, newpath, &len );
					strcat( newpath, path );
					hFind = FtpFindFirstFile( fs, newpath, &lpFindFileData, flags , 0 );
				} else
					FtpServerOpenError( server );
			}

			if ( hFind )
				OutDebugs( "Ftp File Found %s size = %d", lpFindFileData.cFileName, lpFindFileData.nFileSizeLow );
			else {
				ErrorMsg( "Cannot open ftp file ...\n%s\nBecause %s", newpath, NetworkErr(NULL) );
				FtpServerClose( fs );
				return -1;
			}

			filenum = 0;

			while( hFind && !IsStopped() )
			{
				long ret;
				char ftpfilename[256], localfilename[256];

				if( hFind )
				{
					ftpfilename[0] = 0;
					if ( !strchr( lpFindFileData.cFileName , '/' ) ){
						// only if NEWPATH has a / in it copy it.
						
						if ( strchr( newpath , '/' ) ){
							mystrcpy( ftpfilename, newpath );
							p = strrchr( ftpfilename, '/');
							if ( p ) *p = 0;
						}
						strcat( ftpfilename, "/" );
						strcat( ftpfilename, lpFindFileData.cFileName );
					} else
						mystrcpy( ftpfilename, lpFindFileData.cFileName );

					// Figure out local file name
					if ( *tempLocation == 0 || GetFileAttributes( tempLocation ) != FILE_ATTRIBUTE_DIRECTORY )
					{
						sprintf( msg, "%%TEMP%%\\%s", FileFromPath( ftpfilename,NULL ) );
						ExpandEnvironmentStrings( msg, localfilename, 255 );
						StatusSetID(  IDS_STOREFILEINTEMP  );
						OutDebugs( "Using system temp location %s", localfilename );
					} else {
						PathFromFullPath( tempLocation, localfilename );
						if( strlen( localfilename ) )
						{
							strcat ( localfilename, "\\" );
							p = FileFromPath( ftpfilename,NULL );
							if ( !p ) p = "temp.log";
							strcat ( localfilename, p );
						} else
							mystrcpy( localfilename, "temp.log" );
						OutDebugs( "Using user temp location %s", localfilename );
					}

					OutDebugs( "Trying to download %d byte file '%s' into '%s' ...", lpFindFileData.nFileSizeLow, ftpfilename, localfilename );

					ret = FtpFileDownload( fs, downloadURL, localfilename, ftpfilename, lpFindFileData.nFileSizeLow );
					if ( ret > 0 )
					{
						if ( deleteFiles ){			// delete remote ftp file after downloading
							StatusSetID( IDS_DELETEFTP, ftpfilename );
							FtpDelFile( fs, ftpfilename );
						}
						AddFileToLogQ( localfilename, filenum++ );
					} else {
						OutDebugs( "error downloading (%d)", ret );
						ErrorMsg( "Cannot download file %s\n%s", NetworkErr(NULL) );
						hFind = NULL;
						FtpServerClose( fs );
						return -2;
					}
					//------------------------
				} //if( hFind )

				if ( hFind ) {
					if( InternetFindNextFile( hFind, &lpFindFileData ) == FALSE )
						hFind = NULL;
				}
			} //while
			FtpServerClose( fs );
		} 
#endif
	}
	return IsStopped();
}


// ----------------------------------------------------------------------------------------------------------------------------

// Do we really need this crappy way to ftp logs????  just use ftp urls.
int DoPreProc( void )
{
	int ret = 0;

	StartFopenHistory();

	if ( MyPrefStruct.preproc_on )
	{
		ret = PreProc_FtpDownloadFiles(  MyPrefStruct.preproc_location,
										 MyPrefStruct.preproc_tmp,
										 MyPrefStruct.preproc_delete
										 );
	}
	return ret;
}

// Send the mail damn it... go go go now....
void DoEmailSend( void )
{
	StatusSetID( IDS_EMAILNOTIFY );

	if ( strlen(MyPrefStruct.postproc_emailmsg)>2 )
	{
		char *msgContent = NULL;

		// decode any variables that contain usefull cool stats for the email mesg
		//msgContent =  ResolveAllVariables( VDptr, MyPrefStruct.postproc_emailmsg, strlen(MyPrefStruct.postproc_emailmsg) );
		if ( msgContent )
		{
			MailTo( MyPrefStruct.postproc_email, MyPrefStruct.postproc_emailfrom, MyPrefStruct.postproc_emailsub, msgContent, MyPrefStruct.postproc_emailsmtp );
			free( msgContent );
		} else
			MailTo( MyPrefStruct.postproc_email, MyPrefStruct.postproc_emailfrom, MyPrefStruct.postproc_emailsub, MyPrefStruct.postproc_emailmsg, MyPrefStruct.postproc_emailsmtp );
	} else 
	{
		MailTo( MyPrefStruct.postproc_email, MyPrefStruct.postproc_emailfrom, MyPrefStruct.postproc_emailsub, "Report Notification" , MyPrefStruct.postproc_emailsmtp );
	}
}



// main and only entry point.....
int DoPostProc( long stopped )
{
	char msg[2024];

	if ( !stopped )
	{
		// COMPRESS
		if ( MyPrefStruct.postproc_rarchive )
		{
			StatusSet( "Archiving zip...");			//StatusSetID( IDS_MAKEZIP );
			PostProc_ZipReport( MyPrefStruct.outfile,
								msg,  // final filename
								MyPrefStruct.postproc_deletereport );					//make a zip file
			ErrorMsg( "Your usage report has been compressed.\nYou can access it at\n%s\n\n", msg );
		}

		if ( MyPrefStruct.postproc_larchive && glogFilenamesNum )
		{
			StatusSetID( IDS_COMPRESSING );
			PostProc_GzipLogFiles( glogFilenames, glogFilenamesNum, MyPrefStruct.postproc_deletelog );		// compress log if need to
			sprintf( msg, "Your log file been compressed.\n\n" );
			OutDebug( msg );
		}



		long uploadreporterr = 0;
		long uploadlogserr = 0;

		// UPLOAD
		if ( MyPrefStruct.postproc_uploadreport )
		{
			StatusSetID( IDS_UPLOADREPORT );

			if ( MyPrefStruct.postproc_uploadreport == 2 )
				ResetUploadReportQueue( MyPrefStruct.outfile );
			// copy htmls or zip report to dest...
			uploadreporterr = PostProc_UploadReport( (const char *)MyPrefStruct.postproc_uploadreportlocation,
													 MyPrefStruct.postproc_deletereport,
													 MyPrefStruct.postproc_rarchive,
													 MyPrefStruct.outfile
													);						
			sprintf( msg, "Your usage report has been uploaded to %s.\n\n" , MyPrefStruct.postproc_uploadreportlocation );
			OutDebug( msg );
		}

		if ( MyPrefStruct.postproc_uploadlog )
		{
			StatusSetID( IDS_UPLOADLOGS );
			uploadlogserr = PostProc_UploadLogs( MyPrefStruct.postproc_uploadloglocation,
												 MyPrefStruct.postproc_deletelog,
												 MyPrefStruct.postproc_larchive,
												 glogFilenames, 
												 glogFilenamesNum
												);						// copy htmls or zip report to dest...
			sprintf( msg, "Logs uploaded to %s.\n\n" , MyPrefStruct.postproc_uploadreportlocation );
			OutDebug( msg );
		}

		// EMAIL
		if ( MyPrefStruct.postproc_emailon ){
			DoEmailSend();
		}
	}
	else
		OutDebug( "Post processing skipped... stopped." );

	StopFopenHistory();

	return IsStopped();
}







































