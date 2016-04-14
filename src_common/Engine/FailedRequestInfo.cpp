//=================================================================================================
// Copyright © Analysis Software 2001, 2002
//
// File:            FailedRequestInfo.cpp
// Subsystem:       Engine
// Created By:      Zoltan, 26/09/01
// Description:     Implements class CQFailedRequestInfo.  This class represents info gathered for 
//					the pages containing broken links and top referals on errors reports.  These
//					reports share a similar structure in that they both have a main table as well
//					as a sub-table for each row item within the main table.  There is a many-to-
//					many relationship between row items in the main table and those within its
//					sub-tables.  To put it another way, the same row item could appear in more
//					than one sub-table.  Each instance of this class represents the information
//					contained within a single sub-table.  It does not however, store a reference
//					back to the main table row item.  That knowledge is stored elsewhere...
//
// $Archive: /iReporter/Dev/src_common/engine/FailedRequestInfo.cpp $
// $Revision: 4 $
// $Author: Zoltan $
// $Date: 9/10/01 10:38a $
//================================================================================================= 

//==========================================================================================
// System Includes
//==========================================================================================
#include "FWA.h"


//==========================================================================================
// Project Includes
//==========================================================================================
#include "FailedRequestInfo.h"
#include "VirtDomInfo.h"


//==========================================================================================
// Initialisations & Declarations
//==========================================================================================

// This is a gzFile dapter class that provides a cute, asthetically pleasing, 
// stream-like interface around a gzFile handle.
class GZStream
{
	gzFile m_file;

	// writer implementation method
	template <class T> inline GZStream& 
	write( const T& val )
	{
		gzwrite( m_file, &const_cast<T&>(val), sizeof(T) );
		return *this;
	}

	// reader implementation method
	template <class T> inline GZStream& 
	read( T& val )
	{
		gzread( m_file, &val, sizeof(T) );
		return *this;
	}

public:
	GZStream( gzFile file ) : m_file( file ) {}

	// These are implemented in terms of the template methods obove.  I didn't want
	// the public interface of this class to be templated because I want to control
	// what is allowed to be streamed.
	inline GZStream& operator<<( unsigned int val ) { return write(val); }
	inline GZStream& operator>>( unsigned int& val ) { return read(val); }
	inline GZStream& operator<<( unsigned long val ) { return write(val); }
	inline GZStream& operator>>( unsigned long& val ) { return read(val); }
};


//==========================================================================================
// Construction, Destruction, Initialisation & Assignment
//==========================================================================================

CQFailedRequestInfo::FailedRequestURL::FailedRequestURL()
:	m_numFailedRequests(0),
	m_uniqueClients()
{
}

CQFailedRequestInfo::CQFailedRequestInfo()
:	m_failedRequestURLs()
{
}

CQFailedRequestInfo::~CQFailedRequestInfo()
{
}


//==========================================================================================
// Public Operations
//==========================================================================================

//------------------------------------------------------------------------------------------
// Record the fact that the specified client <clientInvolved> attempted a request that
// failed.  The request involved the specified URL <failedURL>.  That's about all we need 
// to know here.  We don't know or even care about the nature of the URL.  It could be either
// a referal URL or a broken link URL, depending on for which report this instance is being
// used.  We also don't know or care about the nature of the failed request.  That's all
// ready being stored elsewhere.  Ignorance is bliss...
//------------------------------------------------------------------------------------------
void CQFailedRequestInfo::
recordFailedRequest( HashID failedURL, HashID clientInvolved )
{
	// add/find FailedRequestURL entry for specified URL
	FailedRequestURLsMap::iterator it( m_failedRequestURLs.find(failedURL) );
	if( it==m_failedRequestURLs.end() )
	{
		it=m_failedRequestURLs.insert( FailedRequestURLsMap::value_type( failedURL, FailedRequestURL() ) ).first;
	}

	// record for sake of posterity
	it->second.recordFailedRequest( clientInvolved );
}


//------------------------------------------------------------------------------------------
// This method is used to populate the specifed StatList <dstStatList> with failed request
// statistics.  This is in order for it to be used to produce a failed request report
// sub-table. A single Statistic entry will be created for each failed request URL that we
// know about.  Since we don't actually know about the nature of the failed request URLs
// (see above) the <srcURLStatList> parameter is required to act as a source of the URL
// Statistics.  It should be the StatList which owns the Statistics whose hash ids were
// specified, via the <failedURL> parameter, to the original recordFailedRequest() calls.
//------------------------------------------------------------------------------------------
void CQFailedRequestInfo::
addFailedRequestStats( StatList& dstStatList, const StatList& srcURLStatList ) const
{
	// for each failed request...
	for( FailedRequestURLsMap::const_iterator blit( m_failedRequestURLs.begin() );
		 blit!=m_failedRequestURLs.end();
		 ++blit
		 )
	{
		// ...get its url string and...
		Statistic* pURL=const_cast<StatList&>(srcURLStatList).FindHashStat( blit->first );
		char* pURLStr=pURL->GetName();

		// ...create a Statistic entry for it
		DEF_ASSERT( !dstStatList.FindStatbyName(pURLStr) );				// shouldn't all ready exist
		Statistic* pFailedRequestUrl=dstStatList.SearchStat(pURLStr);	// this will actually add an entry
		DEF_ASSERT( pFailedRequestUrl );

		// ref the current failed request's FailedRequestURL entry
		const FailedRequestURL& failedRequestURL=blit->second;

		// copy requests counter & total
		pFailedRequestUrl->errors=failedRequestURL.m_numFailedRequests;										
		dstStatList.totalErrors+=pFailedRequestUrl->errors;

		// copy unique visitors counter & inc total
		pFailedRequestUrl->counter=failedRequestURL.m_uniqueClients.size();
		dstStatList.totalCounters+=pFailedRequestUrl->counter;
	}
}


//------------------------------------------------------------------------------------------
// Save current state into the specified gz file.
//------------------------------------------------------------------------------------------
void CQFailedRequestInfo::
saveTo( gzFile file ) const
{
	GZStream stream( file );

	// write how many failed requests we've got
	stream << getNumFailedRequests();

	// for each failed request...
	for( FailedRequestURLsMap::const_iterator blit( m_failedRequestURLs.begin() );
		 blit!=m_failedRequestURLs.end();
		 ++blit
		 )
	{
		// ...write out the hash id of its url string...
		stream << blit->first;

		// ...ref the current failed request's FailedRequestURL entry
		const FailedRequestURL& failedRequestURL=blit->second;

		// ...write out its failed requests counter...
		stream << failedRequestURL.m_numFailedRequests;

		// ...ref its set of unique clients...
		const std::set<HashID>& uniqueClients=blit->second.m_uniqueClients;

		// ...write out number of unique clients in set and...
		stream << uniqueClients.size();

		//...for each unique client in the set...
		for( std::set<HashID>::const_iterator ucit( uniqueClients.begin() );
			 ucit!=uniqueClients.end();	
			 ++ucit
			 )
		{
			// write out its hash id
			stream << *ucit;
		}
	}
}


//------------------------------------------------------------------------------------------
// Restore state from the specified gz file.
//------------------------------------------------------------------------------------------
void CQFailedRequestInfo::
loadFrom( gzFile file )
{
	GZStream stream( file );

	m_failedRequestURLs.clear();	// start afresh

	// read how many failed requests we've got to read in
	size_t numFailedRequests;
	stream >> numFailedRequests;

	// for each failed request...
	for( size_t i(0); i<numFailedRequests; ++i )
	{
		// ..read in hash id of its URL string...
		HashID failedURLHashID;
		stream >> failedURLHashID;

		// ...create and ref a new FailedRequestURL entry for it...
		FailedRequestURL& failedRequestURL=m_failedRequestURLs.
			insert( FailedRequestURLsMap::value_type( failedURLHashID, FailedRequestURL() ) ).first->second;

		// ...read in its failed requests counter...
		stream >> failedRequestURL.m_numFailedRequests;

		// ...ref its set of unique clients...
		std::set<HashID>& uniqueClients=failedRequestURL.m_uniqueClients;

		// ...read in number of unique clients and...
		size_t numUniqueClients;
		stream >> numUniqueClients;

		// ...for each unique client that it has...
		for( size_t j(0); j<numUniqueClients; ++j )
		{
			// ...read in the client's hash id and...
			HashID uniqueClient;
			stream >> uniqueClient;

			// add it to the set
			uniqueClients.insert(uniqueClient);
		}

		// can't have more clients than failed requests
		DEF_ASSERT( uniqueClients.size()<=failedRequestURL.m_numFailedRequests );
	}
}


//==========================================================================================
// Non-Inline Accessors
//==========================================================================================

//------------------------------------------------------------------------------------------
// Returns the total number of unique visitors (clients) for all failed requests recorded.
//------------------------------------------------------------------------------------------
unsigned long CQFailedRequestInfo::
getNumUniqueVisitors() const
{
	// got to determine the superset of unique visitors (clients) for
	// all failed requests recorded

	std::set<HashID> uniqueClientSuperSet;	// all of dem

	// for each failed request...
	for( FailedRequestURLsMap::const_iterator blit( m_failedRequestURLs.begin() );
		 blit!=m_failedRequestURLs.end();
		 ++blit
		 )
	{
		// ...ref its set of unique clients...
		const std::set<HashID>& uniqueClients=blit->second.m_uniqueClients;

		//...for each unique client in the set...
		for( std::set<HashID>::const_iterator ucit( uniqueClients.begin() );
			 ucit!=uniqueClients.end();	
			 ++ucit
			 )
		{
			// ...merge it into the superset
			uniqueClientSuperSet.insert( *ucit );
		}
	}		

	// the number of unique visitors (clients) in the superset is the number we want
	return uniqueClientSuperSet.size();
}


//==========================================================================================
// Protected Operations
//==========================================================================================


//==========================================================================================
// Private Operations
//==========================================================================================

//------------------------------------------------------------------------------------------
// Records the fact that the specified client <clientInvolved> was involved in a failed
// request.
//------------------------------------------------------------------------------------------
void CQFailedRequestInfo::FailedRequestURL::
recordFailedRequest( HashID clientInvolved )
{
	// if no client was specified then don't count it - of course this assumes that
	// a client string could never produce a hash id of zero....cough, cough 
	if( clientInvolved )
	{
		m_uniqueClients.insert( clientInvolved );	// inserts if not present
	}

	++m_numFailedRequests;						// count dem failed requests
}
