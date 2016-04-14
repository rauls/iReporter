// ****************************************************************************
// Copyright (C) 2000-2004
//
// File 	:	report_keypages.cpp
// Created 	:	Tuesday, 11 September 2001
// Author 	:	Julien Crawford [JMC]
//
// Abstract :	This file implements the classes and functions required to
//				produce the "Routes to Key Pages" report.
//				The function "DoRouteToKeyPages" is the beast that actually
//				does the work.
//
// ****************************************************************************
#include "FWA.h"

#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>

#ifdef DEF_MAC
#include <alloca.h>
#else
#include <malloc.h>				// for alloca
#endif
	
#include "VirtDomInfo.h"		// for VDinfoP
#include "Stats.h"				// for StatList
#include "engine_drill.h"		// for SessionStat.
#include "StatDefs.h"			// for SESS_<...> macros
#include "Hash.h"				// for HashIt(...)
#include "ReportClass.h"		// for baseFile.
#include "StandardPlot.h"		// for GraphWidth()
#include "Report.h"				// for RGBtableTitle and the like.
#include "Translate.h"			// for TranslateID()
#include "HelpCard.h"			// for HELPCARD_EXTENSION definition.


// ****************************************************************************
// Class:		QCUrlRoute
//
// Abstract:	This class represents a path though a website.  They are stored
//				as HashID's (ie:long's).  The methods < and == are implemented
//				so that we can use this class in a std::map to count the
//				frequency of path routes through the site.
// ****************************************************************************
class	QCUrlRoute
{
public:
	QCUrlRoute(bool bReverse) : m_bReverse(bReverse) {}
	virtual ~QCUrlRoute()	{}

public:
	long		GetHashId(void) const
	{
		if (m_bReverse)
			return *(m_list.rbegin());
		return *(m_list.begin());
	}

	size_t		size() const
	{
		return m_list.size();
	}

	void		push_back(long l)
	{
		m_list.push_back(l);
	}

	void		push_front(long l)
	{
		m_list.push_front(l);
	}

	std::list<long>::const_iterator	begin() const
	{
		return m_list.begin();
	}

	std::list<long>::const_iterator	end() const
	{
		return m_list.end();
	}


public:
	bool		operator<(const QCUrlRoute& rhs)	const;

private:	// Explicitly made private to ensure they are not used.
	bool		operator==(const QCUrlRoute& rhs)	const;	// No == operator
	bool		operator<=(const QCUrlRoute& rhs)	const;	// No <= operator.
	bool		operator>(const QCUrlRoute& rhs)	const;	// No > operator.
	bool		operator>=(const QCUrlRoute& rhs)	const;	// No >= operator.
	QCUrlRoute& operator=(const QCUrlRoute& rhs);			// No = operator.

private:
	 std::list<long>	m_list;			// This represents the Route. (The long is a HashId)
	 bool				m_bReverse;		// This indicates the direction of the Route.
};


// ****************************************************************************
// Method1:		QCUrlRoute::operator<
//
// Abstract:	This method is the one required by std::map to allow us to
//				store this object uniquely and thus do the frequency count.
//
// Declarations:
//				bool QCUrlRoute::operator<(const QCUrlRoute& rhs) const
//
// Arguments:	the const reference to the comparing object.
//
// Returns:		bool
//
// ****************************************************************************
bool QCUrlRoute::operator<(const QCUrlRoute& rhs) const
{
	int	iSizeA = m_list.size();
	int iSizeB = rhs.m_list.size();

	// *******************************************************
	// As we don't really care about the order they are stored
	// in the map we can do the simplest operation first.
	// The comparison of the sizes - its also easier.
	// *******************************************************
	if (iSizeA < iSizeB)
		return true;
	if (iSizeA > iSizeB)
		return false;

	std::list<long>::const_iterator it1	= m_list.begin();
	std::list<long>::const_iterator it2	= rhs.m_list.begin();
	
	while (it1!=m_list.end() && it2!=rhs.m_list.end())
	{
		if (*it1 < *it2)
			return true;
		if (*it1 > *it2)
			return false;
		++it1;
		++it2;
	}

	// they are equal!
	return false;
}


// *************************************************************************************************
// QCUrlRouteMap is the container we use to lookup and increment the Routes.
// Once we have created/found a QCUrlRoute, we then look it up in the Map using the [] operator.
// It returns an size_t which represents the number of time that Route has occurred.
// *************************************************************************************************
typedef		std::map<class QCUrlRoute,	size_t>	QCUrlRouteMap;
// *************************************************************************************************
// Once we have fully populated the Map, it is desired that we somehow sort them by access count.
// (That is the 'second' member in the map - which is NOT INDEXED)
// Therefore we create a vector of QCUrlRouteCounter's below), where the pointer to the QCUrlRoute
// actually points back into the instance in the QCUrlRouteMap.
// Now that we have this vector of pointers we can sort it by the access count. (Just like an Index)
// *************************************************************************************************
typedef		std::pair<const QCUrlRoute*,size_t>	QCUrlRouteCounter;


// ****************************************************************************
// Forwards:
// ****************************************************************************
void	
WriteSubtables	(VDinfoP		VDptr
				,FILE*			fp
				,baseFile*		pReport
				,QCUrlRouteMap& mapPathCounter
				,short			sSort
				,bool			bIsFrom
				);


// ****************************************************************************
// Method:		LookupPageHashId
//
// Abstract:	This method first looks in the byFile member for a matching
//				HashID 
//
// Declaration: char* LookupPageHashId(VDinfoP VDptr, long nHashId)
//
// Arguments:	VDptr	: the main data container. 
//				nHashId	: the HashId of the URL in question.
//
// Returns:		char* of the name of the requested page/download or NULL.
//
// ****************************************************************************
char*
LookupPageHashId(VDinfoP VDptr, long nHashId)
{
	Statistic*	pStat		= NULL;

	pStat = VDptr->byFile->FindHashStat(nHashId);
	if (!pStat)
		return	NULL;
	else
		return	pStat->GetName();
}



// *************************************************************************************************
// 
// *************************************************************************************************
class QCUrlRouteCounterCompare 
		: public std::binary_function<QCUrlRouteCounter&,QCUrlRouteCounter&,bool>
{
	VDinfoP	m_VDptr;
	short	m_sSort;
public:
	QCUrlRouteCounterCompare(VDinfoP VDptr, short sSort) : m_VDptr(VDptr), m_sSort(sSort) { }
	bool operator()(const QCUrlRouteCounter& x, const QCUrlRouteCounter& y) const;
};


// *************************************************************************************************
// 
// *************************************************************************************************
bool 
QCUrlRouteCounterCompare::operator()(const QCUrlRouteCounter& x, const QCUrlRouteCounter& y) const
{
	// *******************************************************
	// The last item in the vector is the actual KeyPage so
	// Lets sort the KeyPages together.
	// *******************************************************
	const QCUrlRoute*	pUrlRouteA = x.first;
	const QCUrlRoute*	pUrlRouteB = y.first;


	int	nHashIdA = pUrlRouteA->GetHashId();
	int	nHashIdB = pUrlRouteB->GetHashId();

	if (nHashIdA != nHashIdB)
	{
		Statistic*	pStatA;
		Statistic*	pStatB;

		pStatA = m_VDptr->byFile->FindHashStat(nHashIdA);
		pStatB = m_VDptr->byFile->FindHashStat(nHashIdB);


		switch(m_sSort)
		{
			case SORT_NAMES:
			{
				const char*	szURLA = pStatA->GetName();
				const char*	szURLB = pStatB->GetName();

				DEF_ASSERT(szURLA != NULL);
				DEF_ASSERT(szURLB != NULL);

				int iCmp = strcmp(szURLA, szURLB);
				if (iCmp > 0)
					return false;
				if (iCmp < 0)
					return true;
			}
			break;

			case SORT_BYTES:
			{
				if (pStatA->GetBytes() < pStatB->GetBytes())
					return false;
				if (pStatA->GetBytes() > pStatB->GetBytes())
					return true;
			}
			break;

			case SORT_REQUESTS:
			{
				if (pStatA->GetFiles() < pStatB->GetFiles())
					return false;
				if (pStatA->GetFiles() > pStatB->GetFiles())
					return true;
			}
			break;

			case SORT_ERRORS:
			{
				if (pStatA->GetErrors() < pStatB->GetErrors())
					return false;
				if (pStatA->GetErrors() > pStatB->GetErrors())
					return true;
			}
			break;

			default:
			{
				DEF_ASSERT(FALSE);
			}
			break;
		}

		return (nHashIdA < nHashIdB);
	}

	// *******************************************************
	// If they are they same HashId then let compare the sizes.
	// *******************************************************
	return (x.second > y.second);
}

int
FilterNonKeyPagesTo(StatList* byStat, int iIndex)
{
	VDinfoP		VDptr		= (VDinfoP)byStat->GetVD();
	char*		szURL		= 0;

	szURL = VDptr->byPages->GetName(iIndex);
	DEF_ASSERT(szURL != NULL);

	for (int i=0;i<MyPrefStruct.nToKeyPages;++i)
	{
		if (strcmp(szURL, MyPrefStruct.szToKeyPages[i]) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

int
FilterNonKeyPagesFrom(StatList* byStat, int iIndex)
{
	VDinfoP		VDptr		= (VDinfoP)byStat->GetVD();
	char*		szURL		= 0;

	szURL = VDptr->byPages->GetName(iIndex);
	DEF_ASSERT(szURL != NULL);

	for (int i=0;i<MyPrefStruct.nFromKeyPages;++i)
	{
		if (strcmp(szURL, MyPrefStruct.szFromKeyPages[i]) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}


// quicker better way
int
FilterNonKeyPagesTo(Statistic *stat )
{
	char*		szURL		= 0;

	szURL = stat->GetName();
	DEF_ASSERT(szURL != NULL);

	for (int i=0;i<MyPrefStruct.nToKeyPages;++i)
	{
		if (strcmp(szURL, MyPrefStruct.szToKeyPages[i]) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// quicker better way
int
FilterNonKeyPagesFrom(Statistic *stat )
{
	char*		szURL		= 0;

	szURL = stat->GetName();
	DEF_ASSERT(szURL != NULL);

	for (int i=0;i<MyPrefStruct.nFromKeyPages;++i)
	{
		if (strcmp(szURL, MyPrefStruct.szFromKeyPages[i]) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}



// ****************************************************************************
// Method:		DoRouteToKeyPages
//
// Abstract:	This method 
//
// ****************************************************************************
void	DoRouteToKeyPages(VDinfoP VDptr, FILE *fp, baseFile* pReport, short sSort)
{
	// ***************************************************************************
	// Correct bogus file settings.
	// ***************************************************************************
	if (MyPrefStruct.nToKeyPageRouteDepth < 1)
		MyPrefStruct.nToKeyPageRouteDepth = 1;
	if (MyPrefStruct.nToKeyPageMaxRows < 1)
		MyPrefStruct.nToKeyPageMaxRows = 1;
	if (MyPrefStruct.nToKeyPageRouteDepth > MAX_KEYPAGE_ROWS)
		MyPrefStruct.nToKeyPageRouteDepth = MAX_KEYPAGE_ROWS;

	// ***************************************************************************
	// First we need to traverse the data and accumlate the Routes to the Key pages.
	// ***************************************************************************
	QCUrlRouteMap				mapPathCounter;

	for	(int i=0; i<VDptr->byClient->num; i++)
	{
		Statistic*	pStat	= VDptr->byClient->GetStat( i );

		if (!pStat)
		{
			DEF_ASSERT(pStat != NULL);
			continue;
		}
		if (!pStat->sessionStat)
		{
			DEF_ASSERT(pStat->sessionStat != NULL);
			continue;
		}

		std::list<QCUrlRoute>			listUrlRoute;
		std::list<QCUrlRoute>::iterator	it;
		int								iLastPageHashId(SESS_START_REFERAL);
		for (int j=pStat->sessionStat->GetNum()-1; j>=0; --j)
		{
			int	iPageHashId	= pStat->sessionStat->GetSessionPage(j);

			if (iPageHashId == SESS_START_REFERAL)
			{
				// ***************************************************************************
				// We have traced back to the first referal in the session. This means we have
				// the whole route(s) to the key pages.
				// ***************************************************************************
				// Add each UrlRoute from the list to the UrlRouteMap container.
				// ***************************************************************************
				for	(it=listUrlRoute.begin(); it!=listUrlRoute.end(); ++it)
				{
					// ***************************************************************************
					// Ignore entries that are either empty (which should not occur)
					// or have one entry only.
					// ***************************************************************************
					if (it->size() > 1)
						mapPathCounter[*it] += 1;
				}

				// ***************************************************************************
				// Now we need to clear the list and continue.
				// ***************************************************************************
				listUrlRoute.clear();
				continue;
			}

			// ***************************************************************************
			// Ignore repeat pages.
			// ***************************************************************************
			if (iPageHashId == iLastPageHashId)
				continue;
			iLastPageHashId = iPageHashId;


			// ***************************************************************************
			// Ignore pages in the range of 0..SESS_ABOVE_IS_PAGE as they indicate
			// something special. (And we have already dealt with SESS_START_REFERAL)
			// ***************************************************************************
			if ( (iPageHashId <= SESS_ABOVE_IS_PAGE) && (iPageHashId > 0) )
			{
				continue;
			}

			// ***************************************************************************
			// If this is one of our key pages then we need to add a new UrlRoute to our
			// list of current UrlRoutes
			// ***************************************************************************
			for (int i=0;i<MyPrefStruct.nToKeyPages;++i)
			{
				int iPrefPageHashID = HashStr(MyPrefStruct.szToKeyPages[i], NULL);

				if (iPrefPageHashID == iPageHashId)
				{
					listUrlRoute.push_back(QCUrlRoute(true));
					break;
				}
			}

#ifdef	DEF_DEBUG
			// *************************************************************************
			// Verify the page.
			// *************************************************************************
			char*	szURL		= LookupPageHashId(VDptr, iPageHashId);
			if (szURL == NULL)
			{
				DEF_ASSERT(szURL != NULL);
				continue;
			}
			DEF_ASSERT(HashStr(szURL, 0) == iPageHashId);
#endif
			
			// ***************************************************************************
			// Add this page to all the currently identified UrlRoutes.
			// ***************************************************************************
			for	(it=listUrlRoute.begin(); it!=listUrlRoute.end(); ++it)
			{
				if (it->size() < MyPrefStruct.nToKeyPageRouteDepth)
					(*it).push_front(iPageHashId);
			}
		}


		// ***************************************************************************
		// Incase there are Routes in the list still then lets copy them into the map.
		// ***************************************************************************
		for	(it=listUrlRoute.begin(); it!=listUrlRoute.end(); ++it)
		{
			// ***************************************************************************
			// Ignore entries that are either empty (which should not occur)
			// or have one entry only.
			// ***************************************************************************
			if (it->size() > 1)
				mapPathCounter[*it] += 1;
		}
		// ***************************************************************************
		// Now we need to clear the list and continue.
		// ***************************************************************************
		listUrlRoute.clear();
	}


	// ********************************************************
	// We are committed to writing the table now.
	// ********************************************************
	WriteSubtables(VDptr, fp, pReport, mapPathCounter, sSort, false);
}

// ****************************************************************************
// Method:		DoRouteFromKeyPages
//
// Abstract:	This function is directly copied from DoRouteToKeyPages above.
//				There are a few key lines changed within it, but it was not
//				worth writing a generalised version and parameterising as 
//				It will only exist in this release.
//
// ****************************************************************************
void
DoRouteFromKeyPages(VDinfoP VDptr, FILE *fp, baseFile* pReport, short sSort)
{
	// ***************************************************************************
	// Correct bogus file settings.
	// ***************************************************************************
	if (MyPrefStruct.nFromKeyPageRouteDepth < 1)
		MyPrefStruct.nFromKeyPageRouteDepth = 1;
	if (MyPrefStruct.nFromKeyPageMaxRows < 1)
		MyPrefStruct.nFromKeyPageMaxRows = 1;
	if (MyPrefStruct.nFromKeyPageRouteDepth > MAX_KEYPAGE_ROWS)
		MyPrefStruct.nFromKeyPageRouteDepth = MAX_KEYPAGE_ROWS;

	// ***************************************************************************
	// First we need to traverse the data and accumlate the Routes to the Key pages.
	// ***************************************************************************
	QCUrlRouteMap				mapPathCounter;

	for	(int i=0; i<VDptr->byClient->num; i++)
	{
		Statistic*	pStat	= VDptr->byClient->GetStat( i );

		std::list<QCUrlRoute>			listUrlRoute;
		std::list<QCUrlRoute>::iterator	it;
		int								iLastPageHashId(SESS_START_REFERAL);
		for (int j=0; j<pStat->sessionStat->GetNum(); ++j)
		{
			int	iPageHashId	= pStat->sessionStat->GetSessionPage(j);

			if (iPageHashId == SESS_START_REFERAL)
			{
				// ***************************************************************************
				// We have foundthe start of a new session. This means we have the whole
				// route(s) to the key pages.
				// ***************************************************************************
				// Add each UrlRoute from the list to the UrlRouteMap container.
				// ***************************************************************************
				for	(it=listUrlRoute.begin(); it!=listUrlRoute.end(); ++it)
				{
					// ***************************************************************************
					// Ignore entries that are either empty (which should not occur)
					// or have one entry only.
					// ***************************************************************************
					if (it->size() > 1)
						mapPathCounter[*it] += 1;
				}

				// ***************************************************************************
				// Now we need to clear the list and continue.
				// ***************************************************************************
				listUrlRoute.clear();
				continue;
			}

			// ***************************************************************************
			// Ignore repeat pages.
			// ***************************************************************************
			if (iPageHashId == iLastPageHashId)
				continue;
			iLastPageHashId = iPageHashId;


			// ***************************************************************************
			// Ignore pages in the range of 0..SESS_ABOVE_IS_PAGE as they indicate
			// something special. (And we have already dealt with SESS_START_REFERAL)
			// ***************************************************************************
			if ( (iPageHashId <= SESS_ABOVE_IS_PAGE) && (iPageHashId > 0) )
			{
				continue;
			}

			// ***************************************************************************
			// If this is one of our key pages then we need to add a new UrlRoute to our
			// list of current UrlRoutes
			// ***************************************************************************
			for (int i=0;i<MyPrefStruct.nFromKeyPages;++i)
			{
				int iPrefPageHashID = HashStr(MyPrefStruct.szFromKeyPages[i], NULL);

				if (iPrefPageHashID == iPageHashId)
				{
					listUrlRoute.push_back(QCUrlRoute(false));
					break;
				}
			}

#ifdef	DEF_DEBUG
			// *************************************************************************
			// Verify the page.
			// *************************************************************************
			char*	szURL		= LookupPageHashId(VDptr, iPageHashId);
			if (szURL == NULL)
			{
				DEF_ASSERT(szURL != NULL);
				continue;
			}
			DEF_ASSERT(HashStr(szURL, 0) == iPageHashId);
#endif
			
			// ***************************************************************************
			// Add this page to all the currently identified UrlRoutes.
			// ***************************************************************************
			for	(it=listUrlRoute.begin(); it!=listUrlRoute.end(); ++it)
			{
				if (it->size() < MyPrefStruct.nFromKeyPageRouteDepth)
					(*it).push_back(iPageHashId);
			}
		}


		// ***************************************************************************
		// Incase there are Routes in the list still then lets copy them into the map.
		// ***************************************************************************
		for	(it=listUrlRoute.begin(); it!=listUrlRoute.end(); ++it)
		{
			// ***************************************************************************
			// Ignore entries that are either empty (which should not occur)
			// or have one entry only.
			// ***************************************************************************
			if (it->size() > 1)
				mapPathCounter[*it] += 1;
		}
		// ***************************************************************************
		// Now we need to clear the list and continue.
		// ***************************************************************************
		listUrlRoute.clear();
	}


	// ********************************************************
	// We are committed to writing the table now.
	// ********************************************************
	WriteSubtables(VDptr, fp, pReport, mapPathCounter, sSort, true);
}
	
void	
WriteSubtables	(VDinfoP		VDptr
				,FILE*			fp
				,baseFile*		pReport
				,QCUrlRouteMap& mapPathCounter
				,short			sSort
				,bool			bIsFrom
				)
{
	// ***************************************************************************
	// Build up a vector of pointers into the map paired with a counter.
	// Additionally build up a map of hashid's to access counter to get the
	// accumulated access count.  This is so we can write the percentages.
	// ***************************************************************************
	std::vector<QCUrlRouteCounter>				vecUrlRouteCounters;
	std::vector<QCUrlRouteCounter>::iterator	itVec;

	QCUrlRouteMap::iterator		itMap;
	vecUrlRouteCounters.reserve(mapPathCounter.size());
	for (itMap=mapPathCounter.begin();itMap!=mapPathCounter.end();++itMap)
	{
		long iTmpHashId = itMap->first.GetHashId();

#ifdef	DEF_DEBUG
		char* sz = LookupPageHashId(VDptr, iTmpHashId);
#endif

		vecUrlRouteCounters.push_back(QCUrlRouteCounter(&itMap->first,itMap->second));
	}

	// ***************************************************************************
	// Sort the vector such that we group the key pages together and have the most
	// commonly followed paths at the top (head).
	// ***************************************************************************
	std::sort(vecUrlRouteCounters.begin(),vecUrlRouteCounters.end(),QCUrlRouteCounterCompare(VDptr, sSort));


	// ***************************************************************************
	// Traverse the vector and accumulate data on the whole set.
	// ***************************************************************************
	std::vector<size_t>		nKeyPageTotalAccesses;
	std::vector<size_t>		nKeyPages;
	std::vector<size_t>		nKeyPageRows;
	size_t					nKeyPageCounter(-1);
	size_t					nKeyPageHashId(0);

	nKeyPageTotalAccesses.resize(NUMBER_OF_KEYPAGES);
	nKeyPages			 .resize(NUMBER_OF_KEYPAGES);
	nKeyPageRows		 .resize(NUMBER_OF_KEYPAGES);
 
	for (itVec=vecUrlRouteCounters.begin();itVec!=vecUrlRouteCounters.end();++itVec)
	{
		long iTmpHashId = itVec->first->GetHashId();
		if (nKeyPageHashId != iTmpHashId)
		{
			// We have a new (or first) KeyPage.
			++nKeyPageCounter;

			// We should never have more pages than the structure can contain.
			DEF_ASSERT(nKeyPageCounter < NUMBER_OF_KEYPAGES);

			// Record all the keypages found.
			nKeyPages[nKeyPageCounter] = iTmpHashId;

			// remember the 'current' keypage so we can detect the 'new' keypage.
			nKeyPageHashId = iTmpHashId;
		}

		// Count up the rows for each KeyPage.
		++nKeyPageRows[nKeyPageCounter];

		// Add this route's access count to the total for this keypage.
		nKeyPageTotalAccesses[nKeyPageCounter] += itVec->second;
	}


	// ***************************************************************************
	// The Data has now been collected and persists in vecUrlRouteCounters.
	// We can now product the reports.
	// ***************************************************************************
	size_t	nTabWidth	= GraphWidth();
	size_t	nColumns	= 4;


	// ********************************************************
	// We need to predetermin the filename & id of this report.
	// ********************************************************
	char*	szFileName;
	long	lId;
	char*	szLabel;
	size_t	nMaxRows;
	size_t	nRouteDepth;

	if (bIsFrom)
	{
		szFileName	= FindReportFilenameStr(KEYPAGEROUTEFROM_PAGE);
		lId			= KEYPAGEROUTEFROM_PAGE;
		szLabel		= TranslateID(LABEL_ROUTESTAKENFROM);
		nMaxRows	= MyPrefStruct.nFromKeyPageMaxRows;
		nRouteDepth	= MyPrefStruct.nFromKeyPageRouteDepth;
	}
	else
	{
		szFileName	= FindReportFilenameStr(KEYPAGEROUTE_PAGE);
		lId			= KEYPAGEROUTE_PAGE;
		szLabel		= TranslateID(LABEL_ROUTESTAKENTO);
		nMaxRows	= MyPrefStruct.nToKeyPageMaxRows;
		nRouteDepth	= MyPrefStruct.nToKeyPageRouteDepth;
	}


	// ***************************************************************************
	// Procude the tables :
	//		Route to <...> | Times Accessed | % of all routes taken.
	// once for each Key Page.
	// They are sorted by KeyPage then by frequency.
	// ***************************************************************************
	size_t	nKeyPageIndex(0);
	for (int i=0;i<nKeyPageCounter+1;++i)
	{
		pReport->Stat_WriteSpace( fp, 2 );

		char	szTitle[20 + KEYPAGE_LENGTH+1];
		char*	szURL = LookupPageHashId(VDptr, nKeyPages[i]);

		if (szURL == NULL)
		{
			DEF_ASSERT(szURL != NULL);
			continue;
		}

		(void)sprintf(szTitle, szLabel,	szURL);

		// ********************************************************
		// Create the Top row, containing the KeyVisitor.
		// ********************************************************
		pReport->Stat_WriteTableStart(	fp, nTabWidth );
		pReport->Stat_WriteRowStart(	fp, RGBtableTitle, 1 );
		pReport->Stat_WriteTitle(		fp, nColumns, nKeyPageRows[i]+1, szTitle );
		pReport->Stat_WriteRowEnd(		fp );
		
		// ********************************************************
		// Create the Column headers/titles.
		// - Route
		// - Access count
		// - % of routes taken.
		// ********************************************************
		pReport->Stat_WriteRowStart( fp, RGBtableHeaders, nColumns );
		pReport->Stat_WriteHeader( fp, "",	1 );
		pReport->Stat_WriteHeader( fp, TranslateID(LABEL_ROUTE),			1 );
		pReport->Stat_WriteHeader( fp, TranslateID(LABEL_TIMESACCESSED),	1 );
		pReport->Stat_WriteHeader( fp, TranslateID(LABEL_PERCENT_ROUTES),	1 );
		pReport->Stat_WriteRowEnd( fp );


		// ***************************************************************************
		// Produce a report for each of the KeyPages :
		// "Route to <insert KeyPage Here>" | Times accessed | % of all routes taken.
		// ***************************************************************************
		// Traverse the vector to produce the ROWS
		// ***************************************************************************
		int nOtherKeyPagesAccess(0);
		int	nRows(0);
		for ( ;nKeyPageIndex<vecUrlRouteCounters.size() ;++nKeyPageIndex)
		{
			const QCUrlRoute*	pRoute = vecUrlRouteCounters[nKeyPageIndex].first;

			// ********************************************************
			// Get the KeyPage HashId for this Route.
			// ********************************************************
			int	iPageHashId = pRoute->GetHashId();

			// ********************************************************
			// Accumulate the data for the Average & Total rows.
			// ********************************************************
			++nRows;

			// ********************************************************
			// The variable 'iPageHashId'  contains the last PageId
			// So we can compare it to the expected PageId to
			// see if we have moved on to a new table.
			// ********************************************************
			if (iPageHashId != nKeyPages[i])
			{
				break;
			}


			// ********************************************************
			// If are Not going to write more rows then just continue.
			// ********************************************************
			if (nRows > nMaxRows)
			{
				// ********************************************************
				// Accumulate the data for the "Other Routes" row.
				// ********************************************************
				nOtherKeyPagesAccess += vecUrlRouteCounters[nKeyPageIndex].second;

				continue;
			}


			// ********************************************************
			// Row Start. (Alternating colours)
			// ********************************************************
			pReport->Stat_WriteRowStart( fp, ScaleRGB( 100-((nRows&1)*5), RGBtableItems ), nColumns );


			// ********************************************************
			// Row #
			// ********************************************************
			pReport->Stat_WriteNumberData(	fp,	nRows);


			// ********************************************************
			// Construct the Route string.
			// ********************************************************
			std::list<long>::const_iterator	itRoute;
			char*						szURL;
			char						szNumber[10];
			PDFMultiLineCellTextStrList cellStrList;

			pReport->WriteCellStartLeft( fp );
			itRoute=pRoute->begin();
			for (int j=0 ; j<nRouteDepth && itRoute!=pRoute->end(); ++j, ++itRoute)
			{
				iPageHashId = (*itRoute);
				szURL		= LookupPageHashId(VDptr, iPageHashId);

				if (szURL == NULL)
				{
					DEF_ASSERT(szURL != NULL);
					continue;
				}

				if (pReport->m_style == FORMAT_PDF)
				{
					sprintf(szNumber, "%d.  ", j+1);
					std::string str(szNumber);
					str += szURL;
					cellStrList.push_back( PDFMultiLineCellTextStr(str,pReport->GetStringLen(str)));
				}
				else
				{
					pReport->Stat_WriteAnotherLinkInMultiLineCell( fp, szURL, szURL, j+1);
				}
			}

			if (pReport->m_style == FORMAT_PDF)
			{
				pReport->AddTableCellMultilineText( cellStrList, PDF_LEFTJUSTIFY );
			}
			pReport->WriteCellEnd( fp );


			// ********************************************************
			// To get the "Times Accessed" we have to lookup the KeyPage
			// ********************************************************
			long nAccesses		= vecUrlRouteCounters[nKeyPageIndex].second;
			pReport->Stat_WriteNumberData(	fp,	nAccesses);


			// ********************************************************
			// % of all routes to this Key page.
			// ********************************************************
			long iKeyPageHashId = vecUrlRouteCounters[nKeyPageIndex].first->GetHashId();
				//*(vecUrlRouteCounters[nKeyPageIndex].first->m_list.begin());		// the First item is the KeyPage.
			long nTotalAccesses	= nKeyPageTotalAccesses[i];
			pReport->Stat_WritePercent(	fp,	nAccesses, nTotalAccesses);


			// ********************************************************
			// Row End.
			// ********************************************************
			pReport->Stat_WriteRowEnd( fp );
		}


		// ********************************************************
		// If there is a requirement for footer column-labels.
		// (At the END_OF_DATA above the Others, Average & Total)
		// ********************************************************
		if (MyPrefStruct.footer_label && !MyPrefStruct.footer_label_trailing)
		{
			pReport->Stat_WriteRowStart( fp, RGBtableHeaders, nColumns );
			pReport->Stat_WriteHeader( fp, "",	1 );
			pReport->Stat_WriteHeader( fp, TranslateID(LABEL_ROUTE),			1 );	// "Routes", 1);
			pReport->Stat_WriteHeader( fp, TranslateID(LABEL_TIMESACCESSED),	1 );	// "Times Accessed", 1);
			pReport->Stat_WriteHeader( fp, TranslateID(LABEL_PERCENT_ROUTES),	1 );	// "% of routes taken", 1);
			pReport->Stat_WriteRowEnd( fp );
		}


		// ********************************************************
		// Add a row detailing the "Other Routes",
		// ********************************************************
		pReport->Stat_WriteRowStart( fp, RGBtableOthers, nColumns );
		pReport->Stat_WriteNumberData(fp, ((nRows < nMaxRows)?(0):(nRows-nMaxRows)));
		pReport->Stat_WriteBottomHeader( fp, TranslateID(LABEL_ALLOTHERROUTES), 1 );
		pReport->Stat_WriteNumberData(	fp,	nOtherKeyPagesAccess);
		pReport->Stat_WritePercent(	fp,		nOtherKeyPagesAccess, nKeyPageTotalAccesses[i]);
		pReport->Stat_WriteRowEnd( fp );


		// ********************************************************
		// Add a row detailing the Average,
		// ********************************************************
		pReport->Stat_WriteRowStart( fp, RGBtableAvg, nColumns );
		pReport->Stat_WriteText(	 fp, 1, -1, "");
		pReport->Stat_WriteBottomHeader( fp, TranslateID(LABEL_AVERAGE), 1 );			// Average Requests Column
		pReport->Stat_WriteNumberData(	fp,	nKeyPageTotalAccesses[i] / nKeyPageRows[i]);
		pReport->Stat_WritePercent(	fp,		nKeyPageTotalAccesses[i] / nKeyPageRows[i] , nKeyPageTotalAccesses[i]);
		pReport->Stat_WriteRowEnd( fp );


		// ********************************************************
		// Add a row detailing the Totals.
		// ********************************************************
		pReport->Stat_WriteRowStart( fp, RGBtableTotals, nColumns );				// # Column
		pReport->Stat_WriteNumberData(fp, nRows);
		pReport->Stat_WriteBottomHeader( fp, TranslateID(LABEL_TOTALS), 1 );		// Total Requests Column
		pReport->Stat_WriteNumberData(	fp,	nKeyPageTotalAccesses[i] );
		pReport->Stat_WriteRight(	fp,	1, -1, "100%");
		pReport->Stat_WriteRowEnd( fp );


		// ********************************************************
		// If there is a requirement for footer column-labels.
		// (At the END_OF_TABLE)
		// ********************************************************
		if (MyPrefStruct.footer_label && MyPrefStruct.footer_label_trailing)
		{
			pReport->Stat_WriteRowStart( fp, RGBtableHeaders, nColumns );
			pReport->Stat_WriteHeader( fp, "",	1 );
			pReport->Stat_WriteHeader( fp, TranslateID(LABEL_ROUTE),			1 );	// "Routes", 1);
			pReport->Stat_WriteHeader( fp, TranslateID(LABEL_TIMESACCESSED),	1 );	// "Times Accessed", 1);
			pReport->Stat_WriteHeader( fp, TranslateID(LABEL_PERCENT_ROUTES),	1 );	// "% of routes taken", 1);
			pReport->Stat_WriteRowEnd( fp );
		}


		// ********************************************************
		// Close the table.
		// ********************************************************
		pReport->Stat_WriteTableEnd( fp );
		pReport->Stat_InsertTable();

		// ******************************************************************************
		// If this report type suports HelpCards...
		// ******************************************************************************
		if ( pReport->SupportHelpCards() )
		{
			// ******************************************************************************
			// If this report has HelpCards turned on...
			// ******************************************************************************
			ReportTypesP	pReportType = FindReportTypeData( lId );
			if (pReportType && ReportCommentOn(lId) )
			{
				// ******************************************************************************
				// Add a table DIRECTly following the previous one, however this one has no
				// border - producing the look of footnote-like data.
				// In this case we want it to be just the linking icon with no text.
				// ******************************************************************************
				pReport->Stat_WriteTableStart(	fp, GraphWidth(), 0 );		// 0 is the border width!
				pReport->Stat_WriteRowStart(	fp, -1/* RGB*/, 1/*cols*/);
				pReport->Stat_WriteIconLink(	fp, "help.gif", szFileName,	HELPCARD_EXTENSION, "Go the Help Card.", false);
				pReport->Stat_WriteRowEnd(		fp );
				pReport->Stat_WriteTableEnd(	fp );
				pReport->Stat_InsertTable();
			}
		}
	}
}

// ****************************************************************************
// Copyright (C) 2000-2004
// ****************************************************************************
