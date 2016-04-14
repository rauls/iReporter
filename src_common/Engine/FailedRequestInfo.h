//=================================================================================================
// Copyright © Analysis Software 2001, 2002
//
// File:            FailedRequestInfo.h
// Subsystem:       Engine
// Description:     Defines class CQFailedRequestInfo.  See implementation file for more info.
//
// $Archive: /iReporter/Dev/src_common/engine/FailedRequestInfo.h $
// $Revision: 4 $
// $Date: 9/10/01 10:38a $
//================================================================================================= 

#ifndef	FAILEDREQUESTINFO_H
#define	FAILEDREQUESTINFO_H

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
class CQFailedRequestInfo
{
public:

	typedef unsigned long HashID;					// should really come from Statistics struct

	// Construction, Destruction, Initialisation & Assignment
	CQFailedRequestInfo();
	~CQFailedRequestInfo();	// intentionally non-virtual - this is not a base class!!
	//CQFailedRequestInfo( const CQFailedRequestInfo& );				// compiler generated version ok
	//CQFailedRequestInfo& operator=( const CQFailedRequestInfo& );	// compiler generated version ok

	// Public operations
	void recordFailedRequest( HashID failedURL, HashID clientInvolved );
	void addFailedRequestStats( StatList& dstStatList, const StatList& srcURLStatList ) const; 
	void saveTo( gzFile file ) const;
	void loadFrom( gzFile file );

	// Public Accessors
	size_t getNumFailedRequests() const { return m_failedRequestURLs.size(); }
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

	// information for an individual failed request URL
	struct FailedRequestURL	
	{
		unsigned long		m_numFailedRequests;	// number of failed requests associated with this URL 
		std::set<HashID>	m_uniqueClients;		// hash ids of clients who attempted those failed requests

		FailedRequestURL();
		void recordFailedRequest( HashID clientInvolved );
	};

	// maps a failed request URL to its FailedRequest instance
	typedef std::map<HashID, FailedRequestURL> FailedRequestURLsMap;

	// Attributes
	FailedRequestURLsMap m_failedRequestURLs;	// maps hash id of a failed request URL to its FailedRequest struct
};

#endif // FAILEDREQUESTINFO_H












