// ****************************************************************************
// Copyright (C) 2000-2004
//
// File 	:	FilterString.cpp
// Created 	:	Wednesday, 1 August 2001
// Author 	:	Julien Crawford [JMC]
//
// Abstract :	
//		This module exposes the function FormatFilterString, which is
//	is used to create verbose stirngs explaining the filters used.		
//
// ****************************************************************************
#include "Compiler.h"

#include <cstring>			// for strcmp
#include <string.h>	

#include "config.h"				// for the <reporttype>_PAGE ids.
#include "FilterString.h"
#include "myansi.h"				// for the my... string functions (mystrcpy mystrlen etc).
#include "ResDefs.h"			// for GetString

#ifdef DEF_MAC
	#include <alloca.h>
#else
	#include "Resource.h"		// for the resource ids.
	#include <malloc.h>			// for alloca
#endif




// ************************************************************************************
// These are the current filter types available, defined as bit masks.
// ************************************************************************************
const int	N_FILTERS	= 10;

enum eFilterGroup {
	e_FileURL				= 0x0001,
	e_Client				= 0x0002,
	e_Agent					= 0x0004,
	e_Referal				= 0x0008,
	e_Error					= 0x0010,
	e_VirtualHost			= 0x0020,
	e_Cookie				= 0x0040,
	e_UserName				= 0x0080,
	e_Method				= 0x0100,
	e_1stReferalInSession	= 0x0200
};

// ************************************************************************************
// This an array of unique eFilterGroup's, in the order they appear in the filter string
// Thus, the character '0' cooresponds to e_FileURL... 
// ************************************************************************************
static eFilterGroup	sg_eFilterList[N_FILTERS] = { 
	e_FileURL			,
	e_Client			,
	e_Agent				,
	e_Referal			,
	e_Error				,
	e_VirtualHost		,
	e_Cookie			,
	e_UserName			,
	e_Method			,
	e_1stReferalInSession
};

// ************************************************************************************
// This is the mapping between the reports and the filters.
// It essentially defines which filters are to be DISPLAYED for each report type.
// This does NOT turn filters on or off, only there appearance on the reports.
// ************************************************************************************
static int sg_iaReportGroups[] = 
{
	// Traffic
	HOUR_PAGE,					e_Client	| e_Cookie	| e_1stReferalInSession,
	HOURHIST_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	DATE_PAGE,					e_Client	| e_Cookie	| e_1stReferalInSession,
	RECENTDATE_PAGE,			e_Client	| e_Cookie	| e_1stReferalInSession,
	WEEK_PAGE,					e_Client	| e_Cookie	| e_1stReferalInSession,
	SUNDAY_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	MONDAY_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	TUEDAY_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	WEDDAY_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	THUDAY_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	FRIDAY_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	SATDAY_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	MONTH_PAGE,					e_Client	| e_Cookie	| e_1stReferalInSession,
	
	// Diagnostic
	ERRORS_PAGE,				e_Error		| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	ERRORSHIST_PAGE,			e_Error		| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	ERRORURL_PAGE,				e_Method	| e_Error	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	ERRORURLHIST_PAGE,			e_Method	| e_Error	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	BROKENLINKS_PAGE,			e_Error		| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	EXTBROKENLINKS_PAGE,		e_Error		| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	INTBROKENLINKS_PAGE,		e_Error		| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	
	// Server
	PAGES_PAGE,					e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	PAGEHIST_PAGE,				e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	PAGESLEAST_PAGE,			e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	PAGESFIRST_PAGE,			e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	PAGESFIRSTHIST_PAGE,		e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	PAGESLAST_PAGE,				e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	PAGESLASTHIST_PAGE,			e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	DIRS_PAGE,					e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	TOPDIRS_PAGE,				e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	TOPDIRSHIST_PAGE,			e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	FILE_PAGE,					e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	MEANPATH_PAGE,				e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	GROUPS_PAGE,				e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	GROUPSHIST_PAGE,			e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	DOWNLOAD_PAGE,				e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	DOWNLOADHIST_PAGE,			e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	FILETYPE_PAGE,				e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	KEYPAGEROUTE_PAGE,			e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,
	KEYPAGEROUTEFROM_PAGE,		e_Method	| e_FileURL	| e_Client	| e_Cookie	| e_1stReferalInSession,

	// Demographics
	CLIENT_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	CLIENTSTREAM_PAGE,			e_Client	| e_Cookie	| e_1stReferalInSession,
	CLIENTHIST_PAGE,			e_Client	| e_Cookie	| e_1stReferalInSession,
	USER_PAGE,					e_Client	| e_Cookie	| e_1stReferalInSession | e_UserName,
	USERHIST_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession | e_UserName,
	SECONDDOMAIN_PAGE,			e_Client	| e_Cookie	| e_1stReferalInSession,
	DOMAIN_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	REGION_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	ORGNAMES_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	SESSIONS_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	ROBOT_PAGE,					e_Client	| e_Cookie	| e_1stReferalInSession,
	KEYVISITORS_PAGE,			e_Client	| e_Cookie	| e_1stReferalInSession,

	// Referals
	REFERURL_PAGE,				e_Referal	| e_Client	| e_Cookie	| e_1stReferalInSession,
	REFERSITE_PAGE,				e_Referal	| e_Client	| e_Cookie	| e_1stReferalInSession,
	REFERSITEHIST_PAGE,			e_Referal	| e_Client	| e_Cookie	| e_1stReferalInSession,
	SEARCHSITE_PAGE,			e_Referal	| e_Client	| e_Cookie	| e_1stReferalInSession,
	SEARCHSTR_PAGE,				e_Referal	| e_Client	| e_Cookie	| e_1stReferalInSession,

	// Streaming
	MEDIATYPES_PAGE,			e_Client	| e_Cookie	| e_1stReferalInSession,
	VIDEO_PAGE,					e_Method	| e_Client	| e_Cookie	| e_1stReferalInSession,
	AUDIO_PAGE,					e_Method	| e_Client	| e_Cookie	| e_1stReferalInSession,
	MPLAYERS_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,

	// Systems
	BROWSER_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession	| e_Agent,
	BROWSEROS_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession	| e_Agent,
	OPERSYS_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession	| e_Agent,
	UNRECOGNIZEDAGENTS_PAGE,	e_Client	| e_Cookie	| e_1stReferalInSession	| e_Agent,

	// Advertising
	ADVERT_PAGE,				e_Referal	| e_Client	| e_Cookie	| e_1stReferalInSession,
	ADVERTHIST_PAGE,			e_Referal	| e_Client	| e_Cookie	| e_1stReferalInSession,
	ADVERTCAMP_PAGE,			e_Referal	| e_Client	| e_Cookie	| e_1stReferalInSession,
	ADVERTCAMPHIST_PAGE,		e_Referal	| e_Client	| e_Cookie	| e_1stReferalInSession,

	// Marketing
	CIRC_PAGE,					e_Client	| e_Cookie	| e_1stReferalInSession,
	LOYALTY_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	TIMEON_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	
	// Firewall
	SRCADDR_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTSUMMARY_PAGE,			e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTHTTP_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTHTTPS_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTMAIL_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTFTP_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTTELNET_PAGE,			e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTDNS_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTPOP3_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTREAL_PAGE,				e_Client	| e_Cookie	| e_1stReferalInSession,
	PROTOTHERS_PAGE,			e_Client	| e_Cookie	| e_1stReferalInSession,

	// Virtual Hosts.
	VHOST_PAGE,					e_Client	| e_Cookie	| e_1stReferalInSession,

	// NULL Terminate.
	0,0
};


// ************************************************************************************
// Method:		QS::LookupReportFilter
//
// Abstract:	This method does the processing required to lookup a report
//				type and see if it is assigned to display a supplied filter identifier.
//
// Declaration: QS::LookupReportFilter(char cFilterCharacter, int iReportType)
//
// Arguments:	
//		char	cFilterCharacter	: the character that indexs the eFilterList. '0'->'9'
//		int		iReportType			: Which report we are generating text for.
//
// Returns:		true for "Yes, this report displays this filter".
//				false for "No, this report does not display this filter".
// ************************************************************************************
bool
QS::LookupReportFilter(char cFilterCharacter, int iReportType)
{
	if (cFilterCharacter < '0' || cFilterCharacter > '9')
		return false;

	// Do a linear search!
	int i;
	for (i=0; sg_iaReportGroups[i] != 0; i+=2)
	{
		if (iReportType == sg_iaReportGroups[i])
			break;
	}
	
	if (sg_iaReportGroups[i+1] & sg_eFilterList[cFilterCharacter-'0'])
		return true;
	return false;
}



// ****************************************************************************
// These are the exclude/include strings that are used when the user selects
// and of
//		'contains'
//		'starts with'
//		'ends with'
// in the Filter3 combo box.	(any but 'is equal to').
// ****************************************************************************
static const int	sg_pStringMapping[] = 
{
	IDS_FILTERSTRING_0_EXCLUDE,		IDS_FILTERSTRING_0_INCLUDE,
	IDS_FILTERSTRING_1_EXCLUDE,		IDS_FILTERSTRING_1_INCLUDE,
	IDS_FILTERSTRING_2_EXCLUDE,		IDS_FILTERSTRING_2_INCLUDE,
	IDS_FILTERSTRING_3_EXCLUDE,		IDS_FILTERSTRING_3_INCLUDE,
	IDS_FILTERSTRING_4_EXCLUDE,		IDS_FILTERSTRING_4_INCLUDE,
	IDS_FILTERSTRING_5_EXCLUDE,		IDS_FILTERSTRING_5_INCLUDE,
	IDS_FILTERSTRING_6_EXCLUDE,		IDS_FILTERSTRING_6_INCLUDE,
	IDS_FILTERSTRING_7_EXCLUDE,		IDS_FILTERSTRING_7_INCLUDE,
	IDS_FILTERSTRING_8_EXCLUDE,		IDS_FILTERSTRING_8_INCLUDE,
	IDS_FILTERSTRING_9_EXCLUDE,		IDS_FILTERSTRING_9_INCLUDE,
	0,0
};

// ****************************************************************************
// These are the exclude/include strings that are used when the user selects
// 'is equal to' in the Filter3 combo box.
// ****************************************************************************
static const int	sg_pStringExactMapping[] = 
{
	IDS_FILTERSTRING_EXACT_0_EXCLUDE,		IDS_FILTERSTRING_EXACT_0_INCLUDE,
	IDS_FILTERSTRING_EXACT_1_EXCLUDE,		IDS_FILTERSTRING_EXACT_1_INCLUDE,
	IDS_FILTERSTRING_EXACT_2_EXCLUDE,		IDS_FILTERSTRING_EXACT_2_INCLUDE,
	IDS_FILTERSTRING_EXACT_3_EXCLUDE,		IDS_FILTERSTRING_EXACT_3_INCLUDE,
	IDS_FILTERSTRING_EXACT_4_EXCLUDE,		IDS_FILTERSTRING_EXACT_4_INCLUDE,
	IDS_FILTERSTRING_EXACT_5_EXCLUDE,		IDS_FILTERSTRING_EXACT_5_INCLUDE,
	IDS_FILTERSTRING_EXACT_6_EXCLUDE,		IDS_FILTERSTRING_EXACT_6_INCLUDE,
	IDS_FILTERSTRING_EXACT_7_EXCLUDE,		IDS_FILTERSTRING_EXACT_7_INCLUDE,
	IDS_FILTERSTRING_EXACT_8_EXCLUDE,		IDS_FILTERSTRING_EXACT_8_INCLUDE,
	IDS_FILTERSTRING_EXACT_9,				IDS_FILTERSTRING_EXACT_9,	// no include or exclude.
	0,0
};



// ****************************************************************************
// Method:		pattern_strcpy
//
// Abstract:	This method copies the text from the szSrc to the szDst skipping
//				the leading '!' or '!*' and also the ending '*' if they exist.
//
// Declaration: pattern_strcpy(char* szDest, size_t nStrLen, const char* szSrc)
//
// Returns:		The number of characters copied
//
// ****************************************************************************
static int
pattern_strcpy(char* szDest, size_t nStrLen, const char* szSrc)
{
	const char* szStart = szDest;
	if (*szSrc == '!')	// Skip the leading '!' if there is one.
		++szSrc;
	if (*szSrc == '*')	// Skip the leading '*' if there is one.
		++szSrc;

	if (nStrLen < 2)	// needs space for atleast the 's.
		return -1;

	--nStrLen;
	*(szDest++) = '\'';

	for (const char* sz=szSrc;*sz && *sz!='*' && nStrLen;++sz,++szDest,--nStrLen)
		*szDest = *sz;

	if (!nStrLen)
		return -1;

	*(szDest++) = '\'';
	*szDest		= NULL;

	return (szDest - szStart);
}

// ****************************************************************************
// Method:		pattern_listcpy
//
// Abstract:	This function copies into szDest the comma separated strings
//				from the Filter patterns.	It takes the filter string list of
//				the format <digit>[!][*]pattern[*] where [] is optional.
//
//				appends "'A','B' or 'C'" to the string where the pattern is the same.
//
// Declaration: pattern_listcpy(char* szDest, size_t nStrLen, char** ppFilterId, int iStart, int iNPatterns)
//
// Returns:		The number of characters copied
//
// ****************************************************************************
static int
pattern_listcpy(char* szDest, size_t nStrLen, char** ppFilterId, int iStart, int iNPatterns)
{
	int		iChars;
	char* c = szDest;

	if (iNPatterns <= 0)
		return -1;

	if (iNPatterns == 1)
	{
		iChars = pattern_strcpy(c, nStrLen-(c-szDest), ppFilterId[iStart]+1);
		if (iChars == -1)
			return -1;
		c += iChars;
	}
	else 
	{
		int i;
		for (i=iStart; (i-iStart) < (iNPatterns-1); ++i)
		{
			iChars = pattern_strcpy(c, nStrLen-(c-szDest), ppFilterId[i]+1);
			if (iChars == -1)
				return -1;
			c += iChars;

			iChars = mystrncpy(c, ", ", nStrLen-(c-szDest));
			if (iChars == -1)
				return -1;
			c += iChars;
		}
		c -= 2;
		iChars = mystrncpy(c, " or ", nStrLen-(c-szDest));
		if (iChars == -1)
			return -1;
		c += iChars;

		iChars = pattern_strcpy(c, nStrLen-(c-szDest), ppFilterId[i]+1);
		if (iChars == -1)
			return -1;
		c += iChars;
	}

	return (c - szDest);
}

// ****************************************************************************
// Method:		switch_pattern
//
// Abstract:	This method returns a number value to indicate the type of
//				pattern of the string supplied.
//
// Declaration: switch_pattern(const char* sz)
//
// Returns:		0	- for equal to			str == "AAA"
//				1	- for ends with			str == "*AA"
//				2	- for starts with		str == "AA*"
//				3	- for contains			str == "*A*"
// ****************************************************************************
static int
switch_pattern(const char* sz)
{
	if (*sz == '!')
		++sz;

	const char* szEnd;
	for (szEnd=sz;*szEnd;++szEnd);
	--szEnd;

	int	iRetVal = 0;
	if	(*sz == '*')	iRetVal += 1;
	if	(*szEnd == '*')	iRetVal += 2;

	return iRetVal;
}

// ****************************************************************************
// Method:		pattern_cmp
//
// Abstract:	This method compares the type of pattern and NOT the text within
//				the pattern.  It uses switch_pattern above to disern the pattern
//				type.
//
// Declaration: int pattern_cmp(const char* sz1, const char* sz2)
//
// Returns:		0	- if they are equal.
//				1	- if sz1.pattern > sz2.pattern
//				-1	- if sz1.pattern < sz2.pattern
// ****************************************************************************
static int
pattern_cmp(const char* sz1, const char* sz2)
{
	int		iVal1 = switch_pattern(sz1);
	int		iVal2 = switch_pattern(sz2);

	if (iVal1 > iVal2)
		return 1;
	if (iVal1 < iVal2)
		return -1;
	return 0;
}

// ********************************************************************
// The filters need to be sorted so we can group them into appropriate
// sentences.  This is the compare method which we will pass to qsort.
// ********************************************************************
// ****************************************************************************
// Method:		filter_strcmp
//
// Abstract:	This method compares two filters, in the supplied string format.
//				The filter type is the first comparison in the following order.
//					File/URL
//					Client
//					Agent
//					Referral
//					Errors
//					VirtualHost
//					Cookie
//					Username
//					Method
//					1st Referral in Session
//				If these are equal then we compare the exclude/include option.
//					includes are < excludes.
//				If these are equal then we compare pattern type
//					as per pattern_cmp (and therefore switch_pattern) above.
//
// Declaration: filter_strcmp(const void* szStr1, const void* szStr2)
//
// Returns:		0	- if they are equal.
//				1	- if szStr1 > szStr2
//				-1	- if szStr1 < szStr2
// ****************************************************************************
extern "C" int	filter_strcmp(const void* szStr1, const void* szStr2)
{
	const char* sz1 = *(const char**)szStr1;
	const char* sz2 = *(const char**)szStr2;

	// Compare the Filter2 type.  (FileURL, Client, Agent, etc...)
	if (*sz1 < *sz2)
		return -1;
	if (*sz1 > *sz2)
		return 1;

	// step past the number.
	++sz1;
	++sz2;

	// Compare the Include&Exclude.
	if	(	( (*sz1 == '!') && (*sz2 != '!') )	||
			( (*sz1 != '!') && (*sz2 == '!') )
		)
	{
		if (*sz1 == '!')
			return -1;
		else
			return 1;
	}
	// else they are either both '!' or neither '!'.

	int iRetVal = pattern_cmp(sz1, sz2);
	if (iRetVal != 0)
		return iRetVal;

	return strcmp (sz1, sz2);
}

// ****************************************************************************
// Method:		CountSimilarFilterStrings
//
// Abstract:	This method counts from iStart the number of filters that
//				have the same options set.
//
// Declaration: CountSimilarFilterStrings(char* pszFilter[MAX_FILTERUNITS], long nFilters, int iStart)
//
// Arguments:	
//	char*	pszFilter[MAX_FILTERUNITS]	:	The list of filters.
//	long	nFilters					:	The number of filters in the list.
//	int		iStart						:	The position to start counting from.
//
// Returns:		returns the position the last matching filter + 1
//
// ****************************************************************************
static int
CountSimilarFilterStrings(char* pszFilter[MAX_FILTERUNITS], long nFilters, int iStart)
{
	int	iStartPattern = switch_pattern(pszFilter[iStart]+1);

	int i;
	for (i=iStart+1;i<nFilters;++i)
	{
		// If they are the same Filter2 (FileUrl, Client, ...)
		if (pszFilter[iStart][0] != pszFilter[i][0])
			break;
		// If they are not both either Include nor Exclude.
		if	(	( (pszFilter[iStart][1] == '!') && (pszFilter[i][1] != '!') )	||
				( (pszFilter[iStart][1] != '!') && (pszFilter[i][1] == '!') )
			)
		{
			break;
		}

		if (iStartPattern != switch_pattern(pszFilter[i]+1))
		{
			break;
		}

	}

	return i;
}

// ****************************************************************************
// Method:		FormatFilterString
//
// Abstract:	This method 
//
// Declaration: FormatFilterString(struct FilterData* pFilterData, char* szDest, size_t nStrLen)
//
// Arguments:	
//	struct FilterData*	pFilterData	:	The filter data.
//	char*				szDest		:	The destination string for the verbose
//										description of the filters used.
//	size_t				nStrLen		:	The length of the destination buffer.
//
// Returns:		The number of characters copied or -1 on failure.
//
// ****************************************************************************
int	
QS::FormatFilterString(struct FilterData* pFilterData, char* szDest, size_t nStrLen, int iReportType)
{
	char**	ppFilterId = NULL;	//[MAX_FILTERUNITS];
	long	nfilterInTot = pFilterData->filterInTot;

	try
	{
		ppFilterId = (char**)alloca(sizeof(char*) * (nfilterInTot) );
	}
	catch(...)	{	}
	// **********************************************************************************
	// alloca failure return
	// **********************************************************************************
	if (ppFilterId == NULL)
		return -1;


	// **********************************************************************************
	// Create a list of string pointers into the filter data.
	// **********************************************************************************
	int	iOrigFilter = 0, i;
	for (i=0;iOrigFilter<nfilterInTot;++i,++iOrigFilter)
	{
		// **********************************************************************************
		// Skip the filters that are not Checked.
		// **********************************************************************************
		while (pFilterData->filterIn[iOrigFilter][0] == '-')
			++iOrigFilter;

		ppFilterId[i] = pFilterData->filterIn[iOrigFilter];
	}
	// **********************************************************************************
	// Set nfilterInTot to the number of filters actually used.
	// **********************************************************************************
	nfilterInTot = i;


	// **********************************************************************************
	// We can sort this pointer list without altering the content of
	// the original FilterData - thus not changing the order that
	// the user entered them into the Filter Dialog.
	// **********************************************************************************
	// It is worth mentioning that the in the way that we process
	// filters, the order is NOT significant.  This means that we
	// can list them in any order we choose without implying a
	// different behaviour.
	// **********************************************************************************
	(void)qsort(ppFilterId,nfilterInTot,4,filter_strcmp);


	// **********************************************************************************
	// After sorting the string pointer list we know that they are
	// grouped into like filters.  This means that we can comma
	// separate filters that have the same filters but different
	// pattern text.
	// eg:
	// "This report includes only URL's that 'start with' any of 'Aaa','Bbb' or 'Ccc'."
	//
	// **********************************************************************************

	// Count strings that match Filter2s & Incl/Excl's
	int		iNext;
	int		iNChars;
	int		iResID;
	int		iPattern; 
	char* c = szDest;
	
	i = 0;
	while (i < nfilterInTot)
	{
		// **********************************************************
		// Count the filters that have the same settings.
		// (where only the text is different)
		// **********************************************************
		iNext = CountSimilarFilterStrings(ppFilterId, nfilterInTot, i);
		if (iNext > nfilterInTot)
			break;

		// **********************************************************
		// Check if this type is to be included in the message.
		// **********************************************************
		if (!LookupReportFilter(ppFilterId[i][0], iReportType))
		{
			// Skip this filter.
			i = iNext;
			continue;
		}

		
		int iIndex = (ppFilterId[i][0]-'0')*2 + ((ppFilterId[i][1] == '!')?0:1);

		iPattern = switch_pattern(ppFilterId[i]+1);
		if (iPattern == 0)	// exact match.
		{
			iResID = sg_pStringExactMapping[iIndex];
		}
		else
		{
			// "This report excludes all URL's that" VS "This report includes only URL's that"
			iResID = sg_pStringMapping[iIndex];
		}

		iNChars = GetString (iResID, c, nStrLen-(c-szDest));
		if (!iNChars)
		{
			// Failed to load the string.
			return -1;
		}
		c += iNChars;


		if (iPattern == 0)
		{
			c += mystrncpy(c, " ",	nStrLen-(c-szDest));
		}
		else if (iPattern == 1)
		{
			c += mystrncpy(c, " end with ",	nStrLen-(c-szDest));
		}
		else if (iPattern == 2)
		{
			c += mystrncpy(c, " start with ",	nStrLen-(c-szDest));
		}
		else if (iPattern == 3)
		{
			c += mystrncpy(c, " contain ",		nStrLen-(c-szDest));
		}


		// I need to know in advance how many of the current pattern there are from 'i'.
		int j;
		for (j=i;j<iNext && iPattern == switch_pattern(ppFilterId[j]+1); ++j);
		int iNPattern = j;

		// Append the comma separated list of patterns.
		iNChars = pattern_listcpy(c,nStrLen-(c-szDest),ppFilterId,i,iNPattern-i);
		if (iNChars == -1)
			return -1;
		c += iNChars;

		// Append a null character.
		if (nStrLen-(c-szDest) < 1)	
			return -1;
		*(c++) = '\0';


		i = iNext;
	}

	// ***************************************************
	// Double null terminate to indicate end of strings.
	// ***************************************************
	if (nStrLen-(c-szDest) < 1)		
		return -1;
	*(c++) = '\0';

	return (c-szDest);
}

// ****************************************************************************
// Copyright (C) 2000-2004
// ****************************************************************************

