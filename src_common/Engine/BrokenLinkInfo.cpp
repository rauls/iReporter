//=================================================================================================
// Copyright © RedFlag Software 2001, 2002
//
// File:            BrokenLinkInfo.h
// Subsystem:       Engine
// Created By:      Zoltan, 26/09/01
// Description:     Implements class CQBrokenLinkInfo.  This class represents info gathered for the 
//					broken links that an individual page contains.
//
// $Archive: /iReporter/Dev/src_common/engine/BrokenLinkInfo.cpp $
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
#include "BrokenLinkInfo.h"
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

CQBrokenLinkInfo::BrokenLink::BrokenLink()
:	m_numFailedRequests(0),
	m_uniqueClients()
{
}

CQBrokenLinkInfo::CQBrokenLinkInfo()
:	m_brokenLinks()
{
}

CQBrokenLinkInfo::~CQBrokenLinkInfo()
{
}


//==========================================================================================
// Public Operations
//==========================================================================================

//------------------------------------------------------------------------------------------
// Record the fact that the specified client attempted to traverse the specified
// broken link.
//------------------------------------------------------------------------------------------
void CQBrokenLinkInfo::
recordFailedRequest( HashID brokenLinkURL, HashID clientInvolved )
{
	// add/find BrokenLink entry for broken link url
	BrokenLinksMap::iterator it( m_brokenLinks.find(brokenLinkURL) );
	if( it==m_brokenLinks.end() )
	{
		it=m_brokenLinks.insert( BrokenLinksMap::value_type( brokenLinkURL, BrokenLink() ) ).first;
	}

	// record for sake of posterity
	it->second.recordFailedRequest( clientInvolved );
}


//------------------------------------------------------------------------------------------
// Populates the specifed StatList <statList> with broken link Statistic entries.  A single
// entry will be created for each broken link that we know about.  The entry created will
// be for the broken link's URL.  As usual, the <virtualDomain> parameter sets the context
// in which the above occurs.
//------------------------------------------------------------------------------------------
void CQBrokenLinkInfo::
addBrokenLinkStats( StatList& statList, const VDinfo& virtualDomain ) const
{
	// for each broken link...
	for( BrokenLinksMap::const_iterator blit( m_brokenLinks.begin() );
		 blit!=m_brokenLinks.end();
		 ++blit
		 )
	{
		// ...get its url string and...
		Statistic* pURL=virtualDomain.byFile->FindHashStat( blit->first );
		char* pURLStr=pURL->GetName();

		// ...create a Statistic entry for it
		DEF_ASSERT( !statList.FindStatbyName(pURLStr) );		// shouldn't all ready exist
		Statistic* pBrokenLinkUrl=statList.SearchStat(pURLStr);
		DEF_ASSERT( pBrokenLinkUrl );

		// ref the current broken link's BrokenLink entry
		const BrokenLink& brokenLink=blit->second;

		// copy requests counter & total
		pBrokenLinkUrl->errors=brokenLink.m_numFailedRequests;										
		statList.totalErrors+=pBrokenLinkUrl->errors;

		// copy unique visitors counter & inc total
		pBrokenLinkUrl->counter=brokenLink.m_uniqueClients.size();
		statList.totalCounters+=pBrokenLinkUrl->counter;
	}
}


//------------------------------------------------------------------------------------------
// Save current state into the specified gz file.
//------------------------------------------------------------------------------------------
void CQBrokenLinkInfo::
saveTo( gzFile file ) const
{
	GZStream stream( file );

	// write how many broken links we've got
	stream << getNumBrokenLinks();

	// for each broken link...
	for( BrokenLinksMap::const_iterator blit( m_brokenLinks.begin() );
		 blit!=m_brokenLinks.end();
		 ++blit
		 )
	{
		// ...write out the hash id of its url string...
		stream << blit->first;

		// ...ref the current broken link's BrokenLink entry
		const BrokenLink& brokenLink=blit->second;

		// ...write out its failed requests counter...
		stream << brokenLink.m_numFailedRequests;

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
void CQBrokenLinkInfo::
loadFrom( gzFile file )
{
	GZStream stream( file );

	m_brokenLinks.clear();	// start afresh

	// read how many broken links we've got to read in
	size_t numBrokenLinks;
	stream >> numBrokenLinks;

	// for each broken link...
	for( size_t i(0); i<numBrokenLinks; ++i )
	{
		// ..read in hash id of its URL string...
		HashID brokenLinkURL;
		stream >> brokenLinkURL;

		// ...create and ref a new BrokenLink entry for it...
		BrokenLink& brokenLink=m_brokenLinks.
			insert( BrokenLinksMap::value_type( brokenLinkURL, BrokenLink() ) ).first->second;

		// ...read in its failed requests counter...
		stream >> brokenLink.m_numFailedRequests;

		// ...ref its set of unique clients...
		std::set<HashID>& uniqueClients=brokenLink.m_uniqueClients;

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
		DEF_ASSERT( uniqueClients.size()<=brokenLink.m_numFailedRequests );
	}
}


//==========================================================================================
// Non-Inline Accessors
//==========================================================================================

//------------------------------------------------------------------------------------------
// Returns the total number of unique visitors (clients) for all failed requests recorded.
//------------------------------------------------------------------------------------------
unsigned long CQBrokenLinkInfo::
getNumUniqueVisitors() const
{
	// got to determine the superset of unique visitors (clients) for
	// all failed requests recorded

	std::set<HashID> uniqueClientSuperSet;	// all of dem

	// for each broken link...
	for( BrokenLinksMap::const_iterator blit( m_brokenLinks.begin() );
		 blit!=m_brokenLinks.end();
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
// request to this broken link.
//------------------------------------------------------------------------------------------
void CQBrokenLinkInfo::BrokenLink::
recordFailedRequest( HashID clientInvolved )
{
	m_uniqueClients.insert( clientInvolved );	// inserts if not present
	++m_numFailedRequests;						// count dem 404 requests
}


















