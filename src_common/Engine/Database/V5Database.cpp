//=================================================================================================
// Copyright © Analysis Software 2001, 2002
//
// File:            V5Database.cpp
// Subsystem:       NewDatabase
// Created By:      Zoltan, 17/04/02
// Description:     Implements class CQV5Database.
//					Notes:	1: investigate files larger than 2GB - Coded. Tested OK on Win32.
//							2: Open database file for exclusive access only - Coded. Tested OK on Win32.
//
// $Archive: /iReporter/Dev/src_common/engine/Database/V5Database.cpp $
// $Revision: 12 $
// $Author: Zoltan.toth $
// $Date: 30/07/02 5:27p $
//================================================================================================= 

//==========================================================================================
// System Includes
//==========================================================================================
#include "FWA.h"
#include <climits>		// for USHRT_MAX etc
#include <cstring>
#if DEF_WINDOWS
#	include <io.h>		// for open(), dup() etc
#	include <share.h>
#elif DEF_UNIX
#	include <unistd.h>	// dup() etc
#endif
#include <fcntl.h>		// for open() etc
#include <sys/types.h>	// ditto
#include <sys/stat.h>	// ditto
#include <algorithm>	// for lower_bound() etc

//==========================================================================================
// Project Includes
//==========================================================================================
#include "V5Database.h"
#include "HitData.h"
#include "myansi.h"
#include "datetime.h"
#include "EngineBuff.h"		// for horrible global buffers
#include "DateFixFileName.h"
#include "GlobalPaths.h"	// for MAXFILENAMESSIZE
#if DEF_WINDOWS
#	include "createshortcuts.h"	// for GetShortCut()
#endif
#include "FileTypes.h"	// for IsURL()

//==========================================================================================
// Initialisations & Declarations
//==========================================================================================

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This stuff addresses cross platform porting issues for the low level i/o funcitons.  It
// may be better for this stuff to be in myansi.c.
//__________________________________________________________________________________________

// For Windows, we map the contants to their Unix counterparts.
#if DEF_WINDOWS
#	define O_RDONLY			_O_RDONLY
#	define O_RDWR			_O_RDWR
#	define O_APPEND			_O_APPEND
#	define O_CREAT			_O_CREAT
#	define O_BINARY			_O_BINARY
#	define S_IREAD			_S_IREAD
#	define S_IWRITE			_S_IWRITE
	inline int dup( int handle )										{ return _dup( handle ); }
	inline int read( int handle, void* buffer, size_t count )			{ return _read( handle, buffer, count ); }
	inline int write( int handle, const void* buffer, size_t count )	{ return _write( handle, buffer, count ); }
	inline int close( int handle )										{ return _close( handle ); }
	inline __int64 tell64( int handle )									{ return _telli64( handle ); }
	inline __int64 lseek64( int handle, __int64 offset, int origin )	{ return _lseeki64( handle, offset, origin ); }
#endif


// This function addresses low level opening of a file with exclusive access and also
// ensuring that 64bit file offsets can be used.
inline int myOpen( const char *filename, int oflag, int pmode=0 )
{
#if DEF_WINDOWS
	// under Windows, we use _sopen to gain exclusive access the the database file
	return _sopen( filename, oflag, _SH_DENYRW, pmode );
#elif DEF_UNIX
	// under Unix, we use open64() to enable subsequent use of tell64() & lseek64()
	int file=open64( filename, oflag, pmode );
	if( file!=-1 )
	{
		// attempt to gain exclusive access to the database file and stop other processes
		// from accessing it whilst we've got it open
		fshare lck={0};
		lck.f_access=F_RWACC;
		lck.f_deny=F_NODNY;
		if( fcntl(fd, F_SHARE, &lck)==-1 )
		{
			close(file);
			file=-1;
		}
	}
	return file;
#else
#	error Don't know how I should be calling open()!
#endif
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Miscelaneous defintitions.
//__________________________________________________________________________________________

// Constants
enum
{
	NumIndexEntriesPerBlock=3650,			// enough for about 10 years of date transitions in a single index block
	MaxHitsInBuf=10,						// max number of hits that we cache within our gzip file IO buffer before reading/writting
	NumSecondsPerDay=86400
};

static const char s_identifier[]="FWXDB";	// helps to identify file type
static const char s_version[]="5.0";		// current file version

// This is the file header that occurs at the beginning of the database file.
#if DEF_WINDOWS
#pragma pack(1)
#endif
struct Header
{
	char		m_identifier[sizeof(s_identifier)];		// as above
	char		m_version[sizeof(s_version)];			// ditto
	size_t		m_numIndexEntriesUsed;					// num index entries used in database file (<=total num index entries)
	time_t		m_startTime;							// time of first hit stored within database
	__int64		m_totalBytes;							// size of the original uncompressed log data that was added to this database
	char		m_type;									// database type corresponding to CQV5Database::Type enum

	Header(CQV5Database::Type type=CQV5Database::Web)
	:	m_numIndexEntriesUsed(0),
		m_startTime(0),
		m_totalBytes(0),
		m_type(static_cast<char>(type))
	{
		strcpy( m_identifier, s_identifier );
		strcpy( m_version, s_version );
	}
};
#if DEF_WINDOWS
#pragma pack()
#endif


// Old faithful macro which returns the byte offset of data member M within data type T
#define OFFSET(T,M) (reinterpret_cast<size_t>(&static_cast<T*>(0)->M))


// Validate a tm struct ignoring tm_wday & tm_yday which are not even set by
// StringToDaysDate() & StringToDaysTime() 
inline bool valid( const tm& time )
{
	return	time.tm_sec>=0 && time.tm_sec<=59 &&
			time.tm_min>=0 && time.tm_min<=59 &&
			time.tm_hour>=0 && time.tm_hour<=23 &&
			time.tm_mday>=1 && time.tm_mday<=31 &&
			time.tm_mon>=0 && time.tm_mon<=11 &&
			time.tm_year>=70 && time.tm_year<=138 &&
			time.tm_isdst==0;
}


//==========================================================================================
// Construction, Destruction, Initialisation & Assignment
//==========================================================================================

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Boring public ctor.  Call init() to do the actual initialisation.
//__________________________________________________________________________________________
CQV5Database::CQV5Database()
:	m_status(NotInitialised),
	m_fileName(),
	m_physDBFile(0),
	m_logGzipFile(0),
	m_indexEntries(),
	m_startTime(0),
	m_totalBytes(0),
	m_type(Web),	// by default
	m_dayBeingAdded(),
	m_numTxRead(0),
	m_buffer(sizeof(buff)*MaxHitsInBuf),	// we use the global hit buffer "buf" as a guide to the size of our own gzip IO buffer
	m_numHitsInBuf(0),
	m_logFileName()
{
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Boring public dtor.  Just calls cleanUp() to do all the work.
//__________________________________________________________________________________________
/*virtual*/ CQV5Database::~CQV5Database()
{
	cleanUp();
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Public initialisation function.  The <filename> param must be specified.  Will create a
// new database file if <filename> doesn't exist and will set the database type to <type>.
// Otherwise, will open existing database and <type> param will be ignored.
// Returns false to indicate file could not be opened.  If successful then status will be 
// set to Idle.
//__________________________________________________________________________________________
bool CQV5Database::init( const char* fileName, Type type/*=Web*/ )
{
	DEF_PRECONDITION(fileName);
	DEF_PRECONDITION(*fileName);

	// as always, we clean ourselves up before proceeding
	cleanUp();

	////
	// Open databsase file and read header
	////

	// preprocess filename and ensure valid
	// NOTE: we set this attrib regardless of whether or not we will initialise correctly
	m_fileName=preprocessFilename( fileName );
	if( m_fileName.empty() )
	{
		return false;
	}

	// open existing database file or create an new one if it doesn't exist
	m_physDBFile=::myOpen( m_fileName.c_str(), O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE );
	if(m_physDBFile==-1)
	{
		return false;
	}

	Header header(type);	// construct header with specified database type

	// read header
	int bytesRead( ::read( m_physDBFile, &header, sizeof(header) ) );
	if(	bytesRead==-1 )
	{
		return false;
	}

	// if a new database file has just been created (which is what we assume if no header bytes were read)
	if( bytesRead==0 )
	{
		////
		// write out (thus reserving space for) new header & an empty index block 
		////

		// create a dummy empty index entry block to write out
		std::vector<IndexEntry> emptyIndexEntryBlock( NumIndexEntriesPerBlock );
		size_t numBytesInBlock( sizeof(IndexEntry)*NumIndexEntriesPerBlock );
		
		size_t numIndexEntries(0);	// no index entries used in block
		__int64 nextBlockPos(0);	// zero means there aint another block

		if(	write( m_physDBFile, &header, sizeof(header) )!=sizeof(header) ||
			write( m_physDBFile, &numIndexEntries, sizeof(numIndexEntries) )!=sizeof(numIndexEntries) ||
			//write( m_physDBFile, (const void*)(emptyIndexEntryBlock.begin()), numBytesInBlock )!=numBytesInBlock ||
			write( m_physDBFile, &nextBlockPos, sizeof(nextBlockPos) )!=sizeof(nextBlockPos)
			)
		{
			return false;
		}
	}
	// else if the correct number of header bytes were read in
	else if( bytesRead==sizeof(header) )
	{
		// validate id & version within header
		if( strcmp( header.m_identifier, s_identifier ) || strcmp( header.m_version, s_version ) )
		{
			return false;
		}

		////
		// Set required variables from header
		////

		m_startTime=header.m_startTime;
		m_totalBytes=header.m_totalBytes;
		m_type=header.m_type==static_cast<char>(Web) ? Web : Streaming;

		////
		// Read in the day index.  This may actually be spread across multiple index blocks within the file.
		////

		// preallocate space in vector
		size_t numIndexEntriesRemaining( header.m_numIndexEntriesUsed );	// total num in database file
		m_indexEntries.resize( numIndexEntriesRemaining );

		IndexEntry* nextBlockToRead=&m_indexEntries[0];			// ref next bloc in memory to read in to

		// while there are more index entries (i.e. more blocks) to read in
		while( numIndexEntriesRemaining )
		{
			////
			// Read in the current index entry block
			////

			// read num entries in block
			size_t numIndexEntriesInBlock;
			if( ::read( m_physDBFile, &numIndexEntriesInBlock, sizeof(numIndexEntriesInBlock) )!=sizeof(numIndexEntriesInBlock) )
			{
				return false;
			}

			// read the block itself
			size_t numBytesInBlock( sizeof(IndexEntry)*numIndexEntriesInBlock );	
			if( ::read( m_physDBFile, nextBlockToRead, numBytesInBlock )!=numBytesInBlock )
			{
				return false;
			}

			numIndexEntriesRemaining-=numIndexEntriesInBlock;

			// if there is another index entry block to read in
			if( numIndexEntriesRemaining )
			{
				////
				// read in the position of the next index entry block and then seek to that positon
				////

				nextBlockToRead+=numIndexEntriesInBlock;	// ref next bloc in memory to read in to

				__int64 nextBlockPos;
				if( ::read( m_physDBFile, &nextBlockPos, sizeof(nextBlockPos) )!=sizeof(nextBlockPos) ||
					lseek64( m_physDBFile, nextBlockPos, SEEK_SET )==-1
					)
				{
					return false;
				}
			}
		}
	}
	// else incorrect number of header bytes were read in so we're a bit suss on the specified file
	else
	{
		return false;
	}

	m_status=Idle;	// initialised ok

	return true;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Does all the work of uninitialisation.  Returns status to NotInitialised.
//__________________________________________________________________________________________
void CQV5Database::cleanUp()
{
	if( m_status==Adding )
	{
		endAdd();
	}
	else if( m_logGzipFile )
	{
		DEF_ASSERT( m_status==Reading || m_status==IOErrorOccurred );

		gzclose(m_logGzipFile);
		m_logGzipFile=0;
	}

	DEF_ASSERT( !m_logGzipFile );
		
	if(m_physDBFile)
	{
		close(m_physDBFile);
		m_physDBFile=0;
	}

	m_status=NotInitialised;
	m_fileName.erase();
	m_indexEntries.clear();
	m_dayBeingAdded.reset();
	m_startTime=0;
	m_totalBytes=0;
	m_type=Web;	// by default
//	m_dayBeingRead=0;
//	m_readEnd=0;
	m_numTxRead=0;
	m_logFileName.erase();
}

//==========================================================================================
// Public Operations
//==========================================================================================

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Do all of the nasty stuff to the specfied filename that the Analyzer application 
// requires.  Returns the finished result.  Returned string will be empty if an invalid
// filename was specified.  Note that this function may be used for V4 databases as well 
// as V5 databases.
//__________________________________________________________________________________________
/*static*/ std::string CQV5Database::preprocessFilename( const char* filename, bool isV5/*=true*/ )
{
	DEF_PRECONDITION(filename);

	// copy orig filename
	std::string file(filename);

	// remove leading & trailing white space
	static const char trimChars[]=" \n\r\t";
	file.erase( 0, file.find_first_not_of(trimChars) );
	file.erase( file.find_last_not_of(trimChars)+1 );  

	// ensure filename wasn't just hot air
	if( file.empty() )
	{
		return file;
	}

	// ensure extension is correct
	const char* ext=isV5 ? ".fxdb" : ".fdb";
	size_t pos=file.rfind( ext );
	if( pos==std::string::npos || file[pos+strlen(ext)] )
	{
		file+=ext;
	}

	// convert any path tokens
	enum { BUFSIZE=1024 };
	DEF_ASSERT( file.length()<BUFSIZE );
	char dateFixedFile[BUFSIZE];
	strcpy( dateFixedFile, file.c_str() );
	DateFixFilename( dateFixedFile, 0 );

	// resolve shortcut if applicable
#if DEF_WINDOWS
	if( IsShortCut( dateFixedFile ) )
	{
		char shortCutFile[BUFSIZE];
		DEF_ASSERT( strlen(dateFixedFile)<BUFSIZE ); 
		strcpy( shortCutFile, dateFixedFile );
		GetShortCut( shortCutFile, dateFixedFile );
	}
#endif

	// make sure it aint a URL
	if( IsURL( dateFixedFile ) )
	{
		dateFixedFile[0]=0;	// invalidate
	}

	return dateFixedFile;
}


/*static*/ bool CQV5Database::isV5Database( const char* fileName, time_t* startTimePtr/*=0*/, __int64* totalBytes/*=0*/, Type* typePtr/*=0*/ )
{
	DEF_PRECONDITION(fileName);
	DEF_PRECONDITION(*fileName);

	////
	// Open databsase file and read header if we can
	////

	// preprocess filename and ensure valid
	std::string processedFileName( preprocessFilename( fileName ) );
	if( processedFileName.empty() )
	{
		return false;
	}

	// attempt to open database file
	int	physDBFile=myOpen( processedFileName.c_str(), O_RDONLY|O_BINARY );
	if(physDBFile==-1)
	{
		return false;
	}

	// read header
	Header header;
	int bytesRead( ::read( physDBFile, &header, sizeof(header) ) );
	if(	bytesRead==-1 )
	{
		close(physDBFile);
		return false;
	}

	// ensure the correct number of header bytes were read in
	if( bytesRead!=sizeof(header) )
	{
		close(physDBFile);
		return false;
	}

	// validate id & version within header
	if( strcmp( header.m_identifier, s_identifier ) || strcmp( header.m_version, s_version ) )
	{
		close(physDBFile);
		return false;
	}

	////
	// At this stage we know the file is in fact a V5 database so assign startTimePt, sizePtr & typePTr if required.
	////

	if( startTimePtr )
	{
		*startTimePtr=header.m_startTime;
	}

	if( totalBytes )
	{
		*totalBytes=header.m_totalBytes;
	}

	if( typePtr )
	{
		*typePtr=header.m_type==static_cast<char>(Web) ? Web : Streaming;
	}

	close(physDBFile);

	return true;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Add the specfied hit to the database.  The hit will logically be inserted into the
// database at its correct position with respect to time.
// If operation succeeded, will return true and set status to Adding if not all ready so.
// If an i/o error occurred, will set status to IOErrorOccurred and will return false.
//__________________________________________________________________________________________
bool CQV5Database::add( const HitDataRec& hit )
{
	// can't add if we're not initialised or if we're writting etc 
	if( m_status!=Idle && m_status!=Adding )
	{
		return false;
	}

	////
	// Manage saving each individual day's worth of hits into a seperate logical gzip file.  We also 
	// ensure that a new log file will also be added to a new logical gzip file, even if its date is 
	// the same as the last date of the previous log file.
	////

	bool beginNewDay(false);		// true if we need to create a new logical gzip file
	time_t newDayStartTime(0);		// the time of the first hit to be stored within the logical gzip file
	tm time1;						// hit time as a struct tm
	time_t time2(0);				// hit time as a time_t

	// if the hit has a valid date
	if( hit.date && hit.date[0] )
	{
		// convert date to a struct tm
		StringToDaysDate( hit.date, &time1, 0);

		// if hit has valid time then add it to the struct tm
		if( hit.time && hit.time[0] )
		{
			StringToDaysTime( hit.time, &time1);
		}
	}
	else
	{
		time1.tm_sec=-1;	// an efficient way to invalidate our struct tm so that valid() returns false
	}

	// if the struct tm is valid
	if( valid( time1 ) )
	{
		// convert struct tm to time_t
		Date2Days( &time1, &time2 );

		// if time_t date is different to date of current day being added
		if( time2/NumSecondsPerDay != m_dayBeingAdded.m_startTime/NumSecondsPerDay )
		{
			beginNewDay=true;
			newDayStartTime=time2;
		}
	}
	// else the struct tm aint valid so if we're not adding currently
	else if( m_status!=Adding )
	{
		////
		// NOTE: This code will probably never be executed but is here as a sanity check.
		//
		// We need to create an index entry and we really need to specify some kind of sane start time
		// for it. If this is a new database we use a start time of zero thus ensuring that the index
		// entry will be the first one read (also means that its data will not be found by a date range search
		// but that's ok).  If we all ready have data in the database then we use the start time of last
		// index entry in our collection, which hopefully is that last one added.  This is about the best
		// we can do under these circumstances.
		////

		beginNewDay=true;
		newDayStartTime=m_indexEntries.size() ? m_indexEntries.back().m_startTime : 0;
	}

	// if we've decided that we want to begin a new day
	if( beginNewDay )
	{
		// finish off any previous day that we may have been adding and begin adding a new day
		if( m_status==Adding && !endAddingCurrentDay() || !beginAddingNewDay( newDayStartTime ) )
		{
			m_status=IOErrorOccurred;
			return false;
		}

		// maintain our start time attribute which will end up being written out to the header
		// once we've finished adding (see endAdd())
		if( m_startTime==0 || newDayStartTime<m_startTime )
		{
			m_startTime=newDayStartTime;
		}

		m_status=Adding;	// yep, we're now adding
	}

	const char* logFileNameToWrite=0;

	// if the current hit being added is from a different log file than the 
	// previous hit or if we've began a new day
	if( hit.logFileName && (m_logFileName!=hit.logFileName || beginNewDay) )
	{
		// update m_logFileName (current log file being added) and set logFileNameToWrite so
		// that the new log file name gets written out rather than just an empty string
		m_logFileName=logFileNameToWrite=hit.logFileName;
	}

	////
	// Write out the various constituents of the hit to our gzip IO buffer and add up the amount of data written
	////

	// first write out all of the hit's non-streaming constituents
	size_t bytesWritten=m_buffer.write( logFileNameToWrite );
	bytesWritten+=m_buffer.write( hit.date );
	bytesWritten+=m_buffer.write( hit.time );
	bytesWritten+=m_buffer.write( hit.clientaddr );
	bytesWritten+=m_buffer.write( hit.file );
	bytesWritten+=m_buffer.write( hit.agent );
	bytesWritten+=m_buffer.write( hit.stat );
	bytesWritten+=m_buffer.write( hit.method );
	bytesWritten+=m_buffer.write( hit.vhost );
	bytesWritten+=m_buffer.write( hit.refer );
	bytesWritten+=m_buffer.write( hit.cookie );
	bytesWritten+=m_buffer.write( hit.user );
	bytesWritten+=m_buffer.write( hit.sourceaddr );
	bytesWritten+=m_buffer.write( hit.protocol );
	bytesWritten+=m_buffer.write( hit.port );
	bytesWritten+=m_buffer.write( hit.ms );
	bytesWritten+=m_buffer.write( hit.bytes );
	bytesWritten+=m_buffer.write( hit.bytesIn );

	// if this database is a streaming one then write out all of the hit's 
	// streaming only constituents
	if( m_type==Streaming )
	{
		bytesWritten+=m_buffer.write( hit.s_playercpu );
		bytesWritten+=m_buffer.write( hit.s_playeros );
		bytesWritten+=m_buffer.write( hit.s_playerosver );
		bytesWritten+=m_buffer.write( hit.s_playerGUID );
		bytesWritten+=m_buffer.write( hit.s_playerlang );
		bytesWritten+=m_buffer.write( hit.s_playerver );
		bytesWritten+=m_buffer.write( hit.s_audiocodec );
		bytesWritten+=m_buffer.write( hit.s_videocodec );
		bytesWritten+=m_buffer.write( hit.s_transport );
		bytesWritten+=m_buffer.write( hit.s_starttime );
		bytesWritten+=m_buffer.write( hit.s_cpu_util );
		bytesWritten+=m_buffer.write( hit.s_filedur );
		bytesWritten+=m_buffer.write( hit.s_filesize );
		bytesWritten+=m_buffer.write( hit.s_buffercount );
		bytesWritten+=m_buffer.write( hit.s_buffertime );
		bytesWritten+=m_buffer.write( hit.s_percOK );
		bytesWritten+=m_buffer.write( hit.s_packets_sent );
		bytesWritten+=m_buffer.write( hit.s_packets_rec );
		bytesWritten+=m_buffer.write( hit.s_meanbps );
		bytesWritten+=m_buffer.write( hit.s_wm_streamposition );
		bytesWritten+=m_buffer.write( hit.s_wm_streamduration );

		//long	*s_events;	
	}

	////
	// Flush our gzip IO buffer if required
	////

	if( ++m_numHitsInBuf==MaxHitsInBuf )
	{
		if( !m_buffer.flush(m_logGzipFile) )
		{
			m_status=IOErrorOccurred;
			return false;
		}

		m_numHitsInBuf=0;
	}

	////
	// maintain counters for current day being written out
	////

	++m_dayBeingAdded.m_numTx;					// num hits for current day
	m_dayBeingAdded.m_numBytes+=bytesWritten;	// num uncompressed data bytes written out

	return true;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This method should be called immediately once you have finished calling add() for all of
// the log data that you wish to add to the database.  This method cleans up and purges to
// disk the data structures used by add().  The sooner this method is called, the better.
// If operation succeeded, will return true and set status back to Idle.
// If an i/o error occurred, will set status to IOErrorOccurred and will return false.
//__________________________________________________________________________________________
bool CQV5Database::endAdd()
{
	// can't end adding if we're not adding
	if( m_status!=Adding )
	{
		return false;
	}

	m_logFileName.erase();

	DEF_ASSERT(m_logGzipFile);

	////
	// Finish up adding the last day that had been added
	////

	if( !endAddingCurrentDay() )
	{
		m_status=IOErrorOccurred;
		return false;
	}

	////
	// Sort our index entries according to their start times
	////

	std::stable_sort( m_indexEntries.begin(), m_indexEntries.end() );			

	////
	// At this stage we should be at the end of the database file so remember this 
	// position as it may become the position of another index entry block if we 
	// end up filling up the one currently being used
	////

	__int64 endOfDataPos( tell64( m_physDBFile ) );
	if( endOfDataPos==-1 )
	{
		m_status=IOErrorOccurred;
		return false;
	}
	DEF_ASSERT( endOfDataPos==lseek64( m_physDBFile, 0, SEEK_END ) ); 

	////
	// Write out the day index.  This may actually end up being spread across multiple index blocks within the file.
	////

	size_t numIndexEntriesRemaining( m_indexEntries.size() );	// total num in database file

	const IndexEntry* nextBlockToWrite=&m_indexEntries[0];		// ref next bloc in memory to write out

	__int64 nextBlockPos(sizeof(Header));	// file pos of next index entry block

	do
	{
		////
		// Write out the current index entry block
		////

		// seek to file pos of current block
		if( lseek64( m_physDBFile, nextBlockPos, SEEK_SET )==-1 )
		{
			m_status=IOErrorOccurred;
			return false;
		}

		// write out the number of entries in block as well as the block itself
		size_t numIndexEntriesInBlock( numIndexEntriesRemaining > NumIndexEntriesPerBlock ?
                                       NumIndexEntriesPerBlock : numIndexEntriesRemaining
									   );
		
		size_t numBytesInBlock( sizeof(IndexEntry)*numIndexEntriesInBlock );	

		if( write( m_physDBFile, &numIndexEntriesInBlock, sizeof(numIndexEntriesInBlock) )!=sizeof(numIndexEntriesInBlock) ||
			write( m_physDBFile, nextBlockToWrite, numBytesInBlock )!=numBytesInBlock
			)
		{
			m_status=IOErrorOccurred;
			return false;
		}

		numIndexEntriesRemaining-=numIndexEntriesInBlock;

		// if there is another index entry block to write out
		if( numIndexEntriesRemaining )
		{
			nextBlockToWrite+=numIndexEntriesInBlock;	// ref next bloc in memory to write out

			////
			// Determine file position of next block
			////

			// if current block is first one in file
			if( nextBlockPos==sizeof(Header) )
			{
				// next index block will be at end of file
				nextBlockPos=endOfDataPos; 
			}
			else
			{
				// next index block will be directly after this one
				nextBlockPos+=sizeof(size_t) + NumIndexEntriesPerBlock*sizeof(IndexEntry) + sizeof(__int64);
			}
		}
		else
		{
			nextBlockPos=0;
		}

		////
		// Write out file position of next index entry block or zero to indicate there isn't another
		////

		if( write( m_physDBFile, &nextBlockPos, sizeof(nextBlockPos) )!=sizeof(nextBlockPos) )
		{
			m_status=IOErrorOccurred;
			return false;
		}
	}
	// while there are more index entries (i.e. more blocks) to write out
	while( numIndexEntriesRemaining );

	////
	// Update the file header with the new number of index entries used in the file
	////
	
	size_t numIndexEntriesUsed( m_indexEntries.size() );

	if( lseek64( m_physDBFile, OFFSET( Header, m_numIndexEntriesUsed ), SEEK_SET )==-1 ||
		write( m_physDBFile, &numIndexEntriesUsed, sizeof(numIndexEntriesUsed) )!=sizeof(numIndexEntriesUsed)
		)
	{
		m_status=IOErrorOccurred;
		return false;
	}

	////
	// Update the file header with the new database start time (time of earliest hit).  Ok, so we may not
	// actually have to do this always.
	////

	if( lseek64( m_physDBFile, OFFSET( Header, m_startTime ), SEEK_SET )==-1 ||
		write( m_physDBFile, &m_startTime, sizeof(m_startTime) )!=sizeof(m_startTime)
		)
	{
		m_status=IOErrorOccurred;
		return false;
	}

	////
	// Update the file header with the new total number of uncompressed bytes stored in the database
	////

	if( lseek64( m_physDBFile, OFFSET( Header, m_totalBytes ), SEEK_SET )==-1 ||
		write( m_physDBFile, &m_totalBytes, sizeof(m_totalBytes) )!=sizeof(m_totalBytes)
		)
	{
		m_status=IOErrorOccurred;
		return false;
	}

	m_status=Idle;	// finished up adding
	
	return true;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This method should be called to initialse the date range within which the read() method
// will return data.  You should call this method before you call read().  The <fromTime>
// & <toTime> parameters specificy the minium and maximum dates inclusively.  After calling
// this method, the status will be set to Reading (if successful).  The read() function will
// continue to return true until the end of the date range has been read.  If you call this
// method with default time paramters or if you do not call this method before calling
// read() then, then read() will retrieve all hits in the database before returning false.
// If this operation succeeded, the number of uncompressed bytes found within the specified
// date range will be assigned to the value referenced by the <resultBytes> param, status will
// be set to Reading and true will be returned.
// If no data was found within the specified date range then will return false and status will
// remain as Idle.
// If an i/o error occurred, will set status to IOErrorOccurred and will return false.
//__________________________________________________________________________________________
bool CQV5Database::beginRead( __int64* resultBytes/*=0*/, time_t fromTime/*=0*/, time_t toTime/*=0x7FFFFFFF*/ )
{
	if( m_status==NotInitialised || m_status==Reading )	// can't restart reading until reading has entirely finished!
	{
		return false;
	}
	else if( m_status==Adding )	// end adding before we start reading
	{		
		endAdd();
	}

	if( m_status==IOErrorOccurred )	// we're stuffed
	{
		return false;
	}

	DEF_ASSERT(m_status==Idle);
	DEF_ASSERT(!m_logGzipFile);

	////
	// Determine our iterator range between which we need to read
	////

	ConstIndexEntryIter lower=std::lower_bound( m_indexEntries.begin(),
		                                        m_indexEntries.end(),
												IndexEntry( fromTime )
												);
	if( lower==m_indexEntries.end() )
	{
		// no transactions found within date range - status remains idle
		return false;
	}

	ConstIndexEntryIter upper=std::upper_bound( lower,
												static_cast<ConstIndexEntryIter>(m_indexEntries.end()),
												IndexEntry( toTime )
												);

	DEF_ASSERT( lower!=upper );

	////
	// Set up to read the first day's worth of data in our range
	////

	if( !beginReadingNewDay( lower->m_startPos ) )
	{
		m_status=IOErrorOccurred;
		return false;
	}

	////
	// Determine resultBytes if required
	////
	if( resultBytes )
	{
		*resultBytes=0;

		for( ConstIndexEntryIter it(lower); it!=upper; ++it )
		{
			*resultBytes+=it->m_numBytes;
		}
	}

	////
	// Everything succeeded so set up our read range variables
	////

	m_dayBeingRead=lower;	
	m_readEnd=upper;
	m_status=Reading;

	return true;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Reads in the next hit in the database or if beginRead() was called, reads in the next hit
// within the range specified during that call.
// If operation succeeds, will set status to Reading (if not all ready so), will assign the
// number of uncompressed bytes read in for the hit to the value referenced by the
// <bytesRead> param and then will return true;
// If end of read range reached then will set status back to Idle and return false.
// If an i/o error occurred, will set status to IOErrorOccurred and will return false.
//__________________________________________________________________________________________
bool CQV5Database::read( HitData& hit, size_t* bytesRead/*=0*/ )
{
	// if we're not all ready reading then initialise reading to read the entire database
	if( m_status!=Reading && !beginRead() )    
	{
		return false;
	}

	DEF_PRECONDITION( m_logGzipFile!=0 );

	////
	// Manage reading each individual day's worth of hits from a seperate logical gzip file.
	////

	// if we've read all of the hits belonging to the current day being read
	if( m_numTxRead==m_dayBeingRead->m_numTx )
	{
		ConstIndexEntryIter nextDay=m_dayBeingRead+1;	// ref the next day within the date range to read

		// end reading the current day
		if( !endReadingCurrentDay() )
		{
			m_status=IOErrorOccurred;
			return false;
		}

		// if we've finished reading the required date range		
		if( nextDay>=m_readEnd )
		{
			// reset our reading variables 
			//m_dayBeingRead=0;
			//m_readEnd=0;
			m_numTxRead=0;
			m_status=Idle;	// finished reading
			return false;
		}

		// begin reading the next day in our date range
		if( !beginReadingNewDay( nextDay->m_startPos ) )
		{
			m_status=IOErrorOccurred;
			return false;
		}
		
		m_dayBeingRead=nextDay;
	}

	////
	// Refill our gzip IO buffer if required
	////

	if( m_numHitsInBuf==0 )
	{
		if( !m_buffer.fill(m_logGzipFile) )
		{
			m_status=IOErrorOccurred;
			return false;
		}

		m_numHitsInBuf=MaxHitsInBuf;
	}

	////
	// Read in the various constituents of the hit from our gzip IO buffer
	////

	char* logFileName;	// non-const version which can be passed to read()

	// we use one of our global buffers as storage space for the hit's strings - this is
	// consitent with the way that the log reading code does it.
	char* bufPtr=buff;

	// first read in all of the hit's non-streaming constituents
	size_t myBytesRead=m_buffer.read( logFileName, bufPtr );
	myBytesRead+=m_buffer.read( hit.date, bufPtr );
	myBytesRead+=m_buffer.read( hit.time, bufPtr );
	myBytesRead+=m_buffer.read( hit.clientaddr, bufPtr );
	myBytesRead+=m_buffer.read( hit.file, bufPtr );
	myBytesRead+=m_buffer.read( hit.agent, bufPtr );
	myBytesRead+=m_buffer.read( hit.stat, bufPtr );
	myBytesRead+=m_buffer.read( hit.method, bufPtr );
	myBytesRead+=m_buffer.read( hit.vhost, bufPtr );
	myBytesRead+=m_buffer.read( hit.refer, bufPtr );
	myBytesRead+=m_buffer.read( hit.cookie, bufPtr );
	myBytesRead+=m_buffer.read( hit.user, bufPtr );
	myBytesRead+=m_buffer.read( hit.sourceaddr, bufPtr );
	myBytesRead+=m_buffer.read( hit.protocol, bufPtr );
	myBytesRead+=m_buffer.read(	hit.port );
	myBytesRead+=m_buffer.read(	hit.ms );
	myBytesRead+=m_buffer.read( hit.bytes );
	myBytesRead+=m_buffer.read( hit.bytesIn );

	// if this database is a streaming one then read in all of the hit's 
	// streaming only constituents
	if( m_type==Streaming )
	{
		myBytesRead+=m_buffer.read( hit.s_playercpu, bufPtr );
		myBytesRead+=m_buffer.read( hit.s_playeros, bufPtr );
		myBytesRead+=m_buffer.read( hit.s_playerosver, bufPtr );
		myBytesRead+=m_buffer.read( hit.s_playerGUID, bufPtr );
		myBytesRead+=m_buffer.read( hit.s_playerlang, bufPtr );
		myBytesRead+=m_buffer.read( hit.s_playerver, bufPtr );
		myBytesRead+=m_buffer.read( hit.s_audiocodec, bufPtr );
		myBytesRead+=m_buffer.read( hit.s_videocodec, bufPtr );
		myBytesRead+=m_buffer.read( hit.s_transport, bufPtr );
		myBytesRead+=m_buffer.read( hit.s_starttime );
		myBytesRead+=m_buffer.read( hit.s_cpu_util );
		myBytesRead+=m_buffer.read( hit.s_filedur );
		myBytesRead+=m_buffer.read( hit.s_filesize );
		myBytesRead+=m_buffer.read( hit.s_buffercount );
		myBytesRead+=m_buffer.read( hit.s_buffertime );
		myBytesRead+=m_buffer.read( hit.s_percOK );
		myBytesRead+=m_buffer.read( hit.s_packets_sent );
		myBytesRead+=m_buffer.read( hit.s_packets_rec );
		myBytesRead+=m_buffer.read( hit.s_meanbps );
		myBytesRead+=m_buffer.read( hit.s_wm_streamposition );
		myBytesRead+=m_buffer.read( hit.s_wm_streamduration );

		//long	*s_events;	
	}		

	////
	// Maintain various counters
	////

	--m_numHitsInBuf;
	++m_numTxRead;		// count the hits being read for the current day

	// determine bytesRead if required
	if( bytesRead )
	{
		*bytesRead=myBytesRead;
	}

	// if a valid log file name was read in
	if( logFileName )
	{
		m_logFileName=hit.logFileName=logFileName;	// remember it for next time and set it in the hit
	}
	// else no valid log file name was read in
	else
	{
		hit.logFileName=m_logFileName.c_str();		// set the hit's log file name to the last one read in
	}

	return true;
}


//==========================================================================================
// Non-Inline Accessors
//==========================================================================================

//==========================================================================================
// Protected Operations
//==========================================================================================

//==========================================================================================
// Private Operations
//==========================================================================================


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This function should be called when a new date has been detected during the process of  
// adding data.  This function does a number of things including opening a new logical gzip
// file for that date's data.  The <startTime> param represents the time at which the
// first hit for that day occurred.
// Returns true if operation succeeded.
//__________________________________________________________________________________________
bool CQV5Database::beginAddingNewDay( time_t startTime )
{
	DEF_PRECONDITION(!m_logGzipFile);

	////
	// Open a new logical gzip file for the day's hits at the end of the file
	////

	// may not currenlty be at end of file
	__int64 startPos( lseek64( m_physDBFile, 0, SEEK_END ) );
	if( startPos==-1 )
	{
		return false;
	}

	// dup main file handle so it doesn't get closed by gzclose() in endAddingCurrentDay()
	int tempTxFile( dup(m_physDBFile) );	
	if( tempTxFile==-1 )
	{
		return false;
	}

	m_logGzipFile=gzdopen( tempTxFile, "wb" );
	if(!m_logGzipFile)
	{
		return false;
	}

	////
	// Set up our our index entry for the specified date.  This index entry will end up
	// being appended to our m_indexEntries collection once this day's worth of data
	// has been completed by the endAddingCurrentDay() function.
	////

	m_dayBeingAdded.m_startTime=startTime;
	m_dayBeingAdded.m_startPos=startPos;
	DEF_ASSERT( m_dayBeingAdded.m_numTx==0 );
	DEF_ASSERT( m_dayBeingAdded.m_numBytes==0 );

	////
	// gzip io buffer management
	////

	m_buffer.clear();
	m_numHitsInBuf=0;
	
	return true;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This function should be called to finish adding the data for the current date being
// added.  This function does a number of things including closing the logical gzip
// file for the current date's data.  
// Returns true if operation succeeded.
//__________________________________________________________________________________________
bool CQV5Database::endAddingCurrentDay()
{
	DEF_PRECONDITION(m_status==Adding);
	DEF_PRECONDITION(m_logGzipFile);

	// flush any remaining hits within our gzip IO buffer
	if( !m_buffer.flush(m_logGzipFile) )
	{
		return false;
	}
	m_numHitsInBuf=0;

	// append the current date's index entry to our collecton 
	m_indexEntries.push_back( m_dayBeingAdded );

	// close the current date's logical gzip file
	bool ok( gzclose(m_logGzipFile)==Z_OK );
	m_logGzipFile=0;

	if( ok )
	{
		// update total uncompressed bytes stored in database
		m_totalBytes+=m_dayBeingAdded.m_numBytes;
	}

	// get ready for the next day to be added
	m_dayBeingAdded.reset();

	return ok;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This function should be called to begin reading the data for the date whose logical gzip
// file starts at the file position specified by <startPos>.
// Returns true if operation succeeded.
//__________________________________________________________________________________________
bool CQV5Database::beginReadingNewDay( __int64 startPos )
{
	DEF_PRECONDITION(!m_logGzipFile);

	////
	// Open the logical gzip file for the day's hits at the specified file position
	////

	if( lseek64( m_physDBFile, startPos, SEEK_SET )==-1 )
	{
		return false;
	}

	// dup main file handle so it doesn't get closed by gzclose() in endReadingCurrentDay()
	int tempTxFile( dup(m_physDBFile) );
	if( tempTxFile==-1 )
	{
		return false;
	}

	m_logGzipFile=gzdopen( tempTxFile, "rb" );
	if(!m_logGzipFile)
	{
		return false;
	}

	m_numHitsInBuf=0;
	m_numTxRead=0;

	return true;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This function should be called to finish reading the current date's data.  It closes the
// corresponding logical gzip file.
// Returns true if operation succeeded.
//__________________________________________________________________________________________
bool CQV5Database::endReadingCurrentDay()
{
	DEF_PRECONDITION(m_status==Reading);
	DEF_PRECONDITION(m_logGzipFile);

	bool ok( gzclose(m_logGzipFile)==Z_OK );
	m_logGzipFile=0;

	return ok;
}


//==========================================================================================
// Implementation of our nested gzIOBuffer class 
//==========================================================================================


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Constructs a logically empty gzIOBuffer which has <size> bytes of internal storage
// reserved for future use.  The buffer will reallocate if more space is required so the
// value if <size> is not all that critical.
//__________________________________________________________________________________________
CQV5Database::gzIOBuffer::gzIOBuffer( size_t size )
:	m_bytes(),
	m_readPtr(0)
{
	m_bytes.reserve( size );
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Writes (i.e appends) the string specified by <str> to the internal storage buffer.  First
// writes out the string's length followed by the string itself (without the terminating
// null).  Handles the case where the <str> param is null.
// Returns the length of <str>.
//__________________________________________________________________________________________
size_t CQV5Database::gzIOBuffer::write( const char* str )
{
	////
	// Determine the string's length. Will be >=0 if specified string is non-null otherwise
	// will be USHRT_MAX.  Write it out.
	////

	unsigned short length( str ? strlen( str ) : USHRT_MAX );

	doWrite( length );

	if( length==USHRT_MAX )
	{
		return 0;
	}

	////
	// Specified string is non-null so write out the string (without the null terminator).
	////

	const unsigned char* ptr=reinterpret_cast<const unsigned char*>(str);

	m_bytes.insert( m_bytes.end(), ptr, ptr+length );

	return length;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Reads (i.e. copies) a string from the the internal storage buffer at the current read
// position of.  The <str> param is a reference to the char* that will be assigned to point
// to the string once it's been read in.  The <buf> param is a reference to a char*
// specifying where the string should be read into in the first place.  Once the string has
// been read in and terminited with a null, <buf> will be advanced to point to just after
// the null terminator, ready for the next call to this method. Thus, with a large enough
// buffer, this method can be called repeatedly to read strings into that buffer and after
// each call, <str> will be pointed to the position within that buffer where the string was
// read into. Note that <str> will be set to zero if the string that was read in was
// originally a null (see write() method).
// Returns the length of the string that was read in.
//__________________________________________________________________________________________
size_t CQV5Database::gzIOBuffer::read( char*& str, char*& buf )
{
	DEF_PRECONDITION(buf);

	////
	// Read in length of string
	////

	unsigned short length;
	doRead( length );
	
	////
	//	Read in the string itself (if not null)
	///

	// if string is null
	if( length==USHRT_MAX )
	{
		str=0;	// set the string to null
		length=0;	// want to return 0 for a null string
	}
	// else string is valid and non-null
	else
	{		
		str=buf;

		memcpy( str, m_readPtr, length );
		m_readPtr+=length;

		str[length]=0;

		buf+=length+1;
	}

	DEF_POSTCONDITION( m_readPtr<=&m_bytes[0]+m_bytes.size() );

	return length;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Flushes the contents of the internal storae buffer to <file>.  First writes out the size
// of the buffer followed by the actual contents.
// Returns true unless an IO operation failed.
//__________________________________________________________________________________________
bool CQV5Database::gzIOBuffer::flush( gzFile file )
{
	size_t length( m_bytes.size() ); 

	if( length )
	{
		if( gzwrite( file, &length, sizeof(length) )!=sizeof(length) ||
			gzwrite( file, &m_bytes[0], length )!=length
			)
		{
			return false;
		}

		m_bytes.clear();
	}

	return true;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Fills the internal storage buffer from the <file>.  First reads in the size of the buffer
// followed by the actual contents.
// Returns true unless an IO operation failed.
//__________________________________________________________________________________________
bool CQV5Database::gzIOBuffer::fill( gzFile file )
{
	size_t length;	
	if( gzread( file, &length, sizeof(length) )!=sizeof(length) )
	{
		return false;
	}

	m_bytes.resize( length );

	if(	gzread( file, &m_bytes[0], length )!=length )
	{
		return false;
	}

	m_readPtr=&m_bytes[0];	// set read pos to start of buffer

	return true;
}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Clears the contents of the internal storage buffer and resets the read position.
//__________________________________________________________________________________________
void CQV5Database::gzIOBuffer::clear()
{
	m_bytes.clear();
	m_readPtr=0;
}


//==========================================================================================
// Stuff Under Development/Consideration - Do not Delete
//==========================================================================================

/*	

  #ifdef GUNGE

// We want the offset of the connection point relative to the connection
// point container base class
#define BEGIN_CONNECTION_POINT_MAP(x)\
	typedef x _atl_conn_classtype;\
	static const _ATL_CONNMAP_ENTRY* GetConnMap(int* pnEntries) {\
	static const _ATL_CONNMAP_ENTRY _entries[] = {
// CONNECTION_POINT_ENTRY computes the offset of the connection point to the
// IConnectionPointContainer interface
#define CONNECTION_POINT_ENTRY(iid){offsetofclass(_ICPLocator<&iid>, _atl_conn_classtype)-\
	offsetofclass(IConnectionPointContainerImpl<_atl_conn_classtype>, _atl_conn_classtype)},
#define END_CONNECTION_POINT_MAP() {(DWORD)-1} }; \
	if (pnEntries) *pnEntries = sizeof(_entries)/sizeof(_ATL_CONNMAP_ENTRY) - 1; \
	return _entries;}


#define OFFSET(T,M) (reinterpret_cast<size_t>(&static_cast<T*>(0)->M))

struct Test
{
	const char* logFileName;
	long	bytesIn;


	const char* foo(size_t i)
	{
		static size_t storageEntries[]=
		{
			OFFSET(Test,logFileName),
		};

		return *reinterpret_cast<const char**>(this+storageEntries[i]);
	}

};

#endif

Under Consideration:

  class MUI
{
	unsigned char m_b0;
	unsigned char m_b1;
	unsigned char m_b2;

public:
	MUI( unsigned int val=0 ) 
	:	m_b0( static_cast<unsigned char>(val) ),
		m_b1( static_cast<unsigned char>(val>>8) ),
		m_b2( static_cast<unsigned char>(val>>16) )
	{}

	operator unsigned int()
	{
		return m_b0==UCHAR_MAX&&m_b1==UCHAR_MAX&&m_b2==UCHAR_MAX ? USHRT_MAX : (static_cast<unsigned int>(m_b2)<<16)+(static_cast<unsigned int>(m_b1)<<8)+m_b0;
	}
};
MUI muiLength(length);
if( gzwrite( file, &muiLength, sizeof(muiLength) )!=sizeof(muiLength) )
MUI muiLength;
if( gzread( file, &muiLength, sizeof(muiLength) )!=sizeof(muiLength) )
length=muiLength;

	size_t bytesWritten=m_buffer.write( logFileNameToWrite );
	size_t i(0);
	char** strItem;
	while( strItem=const_cast<HitDataRec&>(hit).getStrItem( i++ ) )
	{
		bytesWritten+=m_buffer.write( *strItem );
	}

	long* longItem;
	i=0;
	while( longItem=const_cast<HitDataRec&>(hit).getLongItem( i++ ) )
	{
		bytesWritten+=m_buffer.write( *longItem );
	}

	size_t myBytesRead=m_buffer.read( logFileName, bufPtr );
	size_t i(0);
	char** strItem;
	while( strItem=hit.getStrItem( i++ ) )
	{
		myBytesRead+=m_buffer.read( *strItem, bufPtr );
	}

	long* longItem;
	i=0;
	while( longItem=hit.getLongItem( i++ ) )
	{
		myBytesRead+=m_buffer.read( *longItem ); 
	}

	char** getStrItem( size_t i )
	{
		char** items[]=
		{
			&date,
			&time,
			&stat,
			&clientaddr,
			&file,
			&agent,
			&refer,
			&cookie,
			&user,
			&vhost,
			&method,

			&sourceaddr,
			&protocol,

			&s_playercpu,
			&s_playeros,
			&s_playerosver,
			&s_playerGUID,
			&s_playerlang,
			&s_playerver,
			&s_audiocodec,
			&s_videocodec,
			&s_transport,

			0
		};

		return items[i];
	}

	long* getLongItem( size_t i )
	{
		long* items[]=
		{
			&bytes,
			&bytesIn,

			&port,
			&ms,

			&s_starttime,
			&s_cpu_util,

			&s_filedur,
			&s_filesize,
			&s_buffercount,
			&s_buffertime,
			&s_percOK,
			&s_packets_sent,
			&s_packets_rec,
			&s_meanbps,
			&s_wm_streamposition,
			&s_wm_streamduration,

			0
		};

		return items[i];
	}
*/
