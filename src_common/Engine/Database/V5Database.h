//=================================================================================================
// Copyright © Analysis Software 2001, 2002
//
// File:            V5Database.h
// Subsystem:       NewDatabase
// Description:     Defines class CQV5Database.  See implementation file for more info.
//
// $Archive: /iReporter/Dev/src_common/engine/Database/V5Database.h $
// $Revision: 11 $
// $Date: 26/07/02 3:57p $
//================================================================================================= 

#ifndef	V5DATABASE_H
#define	V5DATABASE_H

//------------------------------------------------------------------------------------------
// System Includes
//------------------------------------------------------------------------------------------
#include "FWA.h"
#include <cstddef>	// for size_t
#include <ctime>	// for time_t
#include <vector>
#include <string>
#include "zlib.h"

//------------------------------------------------------------------------------------------
// Project Includes
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Declarations
//------------------------------------------------------------------------------------------

struct HitData;

//------------------------------------------------------------------------------------------
// Class Declaration
//------------------------------------------------------------------------------------------
class CQV5Database
{
public:
	//---- Public Declarations
	
	// these codes are returned by status()
	enum Status
	{
		NotInitialised,		// init() not called yet or cleanUp() just called
		Idle,				// init() called successfully but not currently doing any thing
		Adding,				// currenly adding hits to database
		Reading,			// currently reading hits from database
		IOErrorOccurred		// some kind of i/o error has occurred during an add or read operation
	};

	// these are possible types of V5 database that may be created
	enum Type
	{
		Web,
		Streaming
	};

	//---- Construction, Destruction, Initialisation & Assignment
	CQV5Database();
	virtual ~CQV5Database();
	bool init( const char* fileName, Type type=Web );
	void cleanUp();

	//---- Public operations
	static std::string preprocessFilename( const char* filename, bool isV5=true );
	static bool isV5Database( const char* fileName, time_t* startTimePtr=0, __int64* totalBytes=0, Type* typePtr=0 );
	bool add( const HitData& hit );
	bool endAdd();
	bool beginRead( __int64* resultBytes=0, time_t fromTime=0, time_t toTime=0x7FFFFFFF );
	bool read( HitData& hit, size_t* bytesRead=0 );

	//---- Public Accessors
	Status status() const { return m_status; }
	std::string fileName() const { return m_fileName; }
	time_t startTime() const { return  m_startTime; }
	__int64 totalBytes() const { return m_totalBytes; } // NOTE: Only updated after endAdd() is called.
	Type type() const { return m_type; }

	//---- Public Mutators

protected:
	//---- Protected operations

	//---- Protected Accessors

	//---- Protected Mutators

private:
	//---- Private Declarations

	// Used to buffer data before writting it to or reading it from our gzip file 
	class gzIOBuffer
	{
		std::vector<unsigned char>	m_bytes;		// the actual bytes of data that get buffered
		unsigned char*				m_readPtr;		// points into m_bytes to signify where to copy bytes from

		template<class T> size_t doWrite( T val )
		{
			const unsigned char* ptr=reinterpret_cast<const unsigned char*>(&val);
			m_bytes.insert( m_bytes.end(), ptr, ptr+sizeof(val) );
			return sizeof(val);
		}

		template<class T> size_t doRead( T& val )
		{
			memcpy( &val, m_readPtr, sizeof(val) );
			m_readPtr+=sizeof(val);
			return sizeof(val);
		}

	public:

		gzIOBuffer( size_t size );
		
		inline size_t write( long val ) { return doWrite( val ); }
		inline size_t read( long& val ) { return doRead( val ); }
		inline void read2( long& val )
		{
			memcpy( &val, m_readPtr, sizeof(val) );
			m_readPtr+=sizeof(val);
		}

		size_t write( const char* str );
		size_t read( char*& str, char*& buf );

		bool flush( gzFile file );
		bool fill( gzFile file );
		void clear();
	};

	// Each IndexEntry identifies an individual day's worth of data sored within the database
	struct IndexEntry
	{
		time_t		m_startTime;	// the date/time at which the first hit for this day occurred
		__int64		m_startPos;		// the physical file postion where the logical gzip file for this day begins
		size_t		m_numTx;		// the number of hits stored in the logical gzip file for this day
		size_t		m_numBytes;		// the number of uncompressed bytes stored in the logical gzip file for this day

		IndexEntry( time_t startTime=0 ) : m_startTime(startTime), m_startPos(0), m_numTx(0), m_numBytes(0) {}
		void reset() { m_startTime=0; m_startPos=0; m_numTx=0; m_numBytes=0; }
		bool operator<( const IndexEntry& rhs ) const { return m_startTime<rhs.m_startTime; }	// required by sort(), lower_bound() etc
	};

	typedef std::vector<IndexEntry>			IndexEntryVect;
	typedef IndexEntryVect::iterator		IndexEntryIter;
	typedef IndexEntryVect::const_iterator	ConstIndexEntryIter;

	//---- Private operations
	bool beginAddingNewDay( time_t startTime );
	bool endAddingCurrentDay();
	bool beginReadingNewDay( __int64 startPos );
	bool endReadingCurrentDay();
	CQV5Database( const CQV5Database& );				// not allowed
	CQV5Database& operator=( const CQV5Database& );		// not allowed

	//---- Private Accessors

	//---- Private Mutators

	//---- Attributes
	Status					m_status;			// what we're currently doing
	std::string				m_fileName;			// file name of database after initialisation
	int						m_physDBFile;		// main physical database file handle
	gzFile					m_logGzipFile;		// logical gzip file handle used to read/write data for each date stored in database
	IndexEntryVect			m_indexEntries;		// our collection of index entries sorted by start time (i.e. date)

	// the following attributes correspond to values stored in the database file header
	time_t					m_startTime;		// time of earliest hit in database
	__int64					m_totalBytes;		// total number of uncompressed bytes stored in database
	Type					m_type;				// the type of hits stored int the database i.e. web or streaming

	// the following attributes are only used when adding data (add() function):
	IndexEntry				m_dayBeingAdded;	// maintained for each individual date's worth of data added

	// the following attributes are used only when reading data (read() function):
	ConstIndexEntryIter		m_dayBeingRead;		// references the index entry for the date that's currently being read
	ConstIndexEntryIter		m_readEnd;			// references just beyond the last index entry for the current read range
	size_t					m_numTxRead;		// number of transactions read so far for the current index entry's data

	// the following attributes are used when adding data (add() function) AND reading data (read() function):
	gzIOBuffer				m_buffer;			// used to buffer data before writting it to or reading it from our gzip file 
	size_t					m_numHitsInBuf;		// number of hits currently stored within m_buffer
	std::string				m_logFileName;		// Name of log file whose hits are currently being added or read
};

#endif // V5DATABASE_H

