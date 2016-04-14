//=================================================================================================
// Copyright © RedFlag Software 2001, 2002
//
// File:            BrokenLinkInfo.h
// Subsystem:       Engine
// Description:     Defines class CQBrokenLinkInfo.  This class represents info gathered for the 
//					broken links that an individual page contains.
//
// $Archive: /iReporter/Dev/src_common/engine/BrokenLinkInfo.h $
// $Revision: 4 $
// $Date: 9/10/01 10:38a $
//================================================================================================= 

#ifndef	BROKENLINKINFO_H
#define	BROKENLINKINFO_H

//------------------------------------------------------------------------------------------
// System Includes
//------------------------------------------------------------------------------------------
#include "FWA.h"
#include <cstddef>	// for size_t
#include <set>
#include <map>

#include "zlib.h"

//------------------------------------------------------------------------------------------
// Project Includes
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Declarations
//------------------------------------------------------------------------------------------
class StatList;
struct VDinfo;

//------------------------------------------------------------------------------------------
// Class Declaration
//------------------------------------------------------------------------------------------
class CQBrokenLinkInfo
{
public:

	typedef unsigned long HashID;					// should really come from Statistics struct

	// Construction, Destruction, Initialisation & Assignment
	CQBrokenLinkInfo();
	~CQBrokenLinkInfo();	// intentionally non-virtual - this is not a base class!!
	//CQBrokenLinkInfo( const CQBrokenLinkInfo& );				// compiler generated version ok
	//CQBrokenLinkInfo& operator=( const CQBrokenLinkInfo& );	// compiler generated version ok

	// Public operations
	void recordFailedRequest( HashID brokenLinkURL, HashID clientInvolved );
	void addBrokenLinkStats( StatList& statList, const VDinfo& virtualDomain ) const; 
	void saveTo( gzFile file ) const;
	void loadFrom( gzFile file );

	// Public Accessors
	size_t getNumBrokenLinks() const { return m_brokenLinks.size(); }
	unsigned long getNumUniqueVisitors() const;

	// Public Mutators

protected:
	// Protected operations

	// Protected Accessors

	// Protected Mutators

private:
	// Private operations

	// Private Accessors

	// Private Mutators

	// information for an individual broken link
	struct BrokenLink	
	{
		unsigned long		m_numFailedRequests;	// number of failed requests for this broken link
		std::set<HashID>	m_uniqueClients;		// hash ids of clients attempting to traverse this broken link

		BrokenLink();
		void recordFailedRequest( HashID clientInvolved );
	};

	typedef std::map<HashID, BrokenLink> BrokenLinksMap;

	// Attributes
	BrokenLinksMap m_brokenLinks;	// maps hash id of a broken link's url to its BrokenLink struct
};

#endif // BROKENLINKINFO_H












