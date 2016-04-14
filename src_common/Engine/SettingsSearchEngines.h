/******************************************************************************
** Filename:	SearchEngines.h
**
** Overview:	Search Engines Settings file - part of (Global) Settings
**				Application Wide.
**
** Note:	This file contains settings for Search Engines
**
** Created:	By: Trevor Clayton		Date: A Hazy time in Oct 2001
**
** $Archive: $
** $Revision: $
** $Author: $
** $Date: $
******************************************************************************/

#ifndef SETTINGS_SEARCH_ENGINES_H
#define SETTINGS_SEARCH_ENGINES_H

#include <string>
#include <vector>

//
// class CQSearchEngineList
// Class that represents a list all the "Search Engines" which are searched for in a log file.
//
class CQSearchEngineList
{
public:
	CQSearchEngineList(){};
	bool DecodeSearchEngines( const char *searchEngines );
	std::string Extract();

	typedef std::vector<std::string> Strings;
	typedef Strings::iterator StringsIter;
	typedef Strings::const_iterator StringsConstIter;

	//
	// class SELine
	// Class that represents a unique "Search Engine" entry, with it's associated "log stamp"
	// and "search parameter identifier(s)".  The "log stamp" is the element which is compared
	// with each referral field in the log file to find a match.
	//
	class SELine
	{
	public:
		// Constructors
		SELine(){}
		SELine( const SELine& searchEngineDetails );
		SELine& operator=( const SELine& newSearchEngineSettingLine );

		// Conversion functions
		bool Decode( const char *searchEngineLine );

		// Access functions
		const std::string& UserAssignedName() const { return m_userAssignedName; }
		const std::string& LogFileStamp() const { return m_logFileStamp; }
		const Strings& LogFileStampParams() const { return m_logFileStampParams; }

	private:
		std::string m_userAssignedName;
		std::string m_logFileStamp;
		Strings m_logFileStampParams;
	};
	typedef std::vector<SELine> SearchEngineSettings;
	typedef SearchEngineSettings::iterator SearchEngineSettingLineIter;

	SearchEngineSettings& GetList() { return m_searchEngineSettingLine; }

	const char* GetSearchEngineName( const char* logStamp );
	std::string GetSearchEngineCombinedName( const char* explictSiteName );

private:
	SearchEngineSettings m_searchEngineSettingLine;
};

#endif // SETTINGS_SEARCH_ENGINES_H

