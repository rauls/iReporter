/******************************************************************************
** Filename:	SettingsSearchEngines.cpp
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

#include "FWA.h"

#include <string.h>
#include "SettingsSearchEngines.h"

CQSearchEngineList::SELine::SELine( const CQSearchEngineList::SELine& searchEngineSettingLine )
	:	m_userAssignedName(searchEngineSettingLine.UserAssignedName()),
		m_logFileStamp(searchEngineSettingLine.LogFileStamp()),
		m_logFileStampParams( searchEngineSettingLine.LogFileStampParams() )
{
}

CQSearchEngineList::SELine& CQSearchEngineList::SELine::operator=( const CQSearchEngineList::SELine& searchEngineSettingLine )
{
	m_userAssignedName = searchEngineSettingLine.m_userAssignedName;
	m_logFileStamp = searchEngineSettingLine.m_logFileStamp;
	m_logFileStampParams = searchEngineSettingLine.m_logFileStampParams;
	return (*this);
}

bool CQSearchEngineList::SELine::Decode( const char *searchEngineLineP )
{
	if ( !searchEngineLineP )
		return false;
	if ( *searchEngineLineP == '#' )
		return false;
	char *searchEngineLineBuf = new char[ strlen( searchEngineLineP ) + 1];
	strcpy( searchEngineLineBuf, searchEngineLineP );
	char *searchEngineLine = searchEngineLineBuf;

	// Get the Name
	char *element = (char *)searchEngineLine;
	searchEngineLine = strchr( searchEngineLine, ',' );
	if ( searchEngineLine )
	{
		*searchEngineLine = 0;
		m_userAssignedName = element;
	}
	else
	{
		m_userAssignedName = element;
		delete[] searchEngineLineBuf;
		return false;
	}
	searchEngineLine++;

	// Get the Logfile Stamp
	element = searchEngineLine;
	searchEngineLine = strchr( searchEngineLine, ',' );
	if ( searchEngineLine )
	{
		*searchEngineLine = 0;
		 m_logFileStamp = element;
	}
	else
	{
		 m_logFileStamp = element;
		delete[] searchEngineLineBuf;
		return false;
	}

	// Get the logfile Parameters
	const char*p=searchEngineLine+1;

	std::string param;

	while( *p && *p!='\n' )
	{
		if( *p==',' )
		{
			m_logFileStampParams.push_back( param );
			param.erase();
		}
		else
		{
			param+=*p;
		}

		++p;
	}

	// add the last param
	if( param.length() )
	{		
		m_logFileStampParams.push_back( param );
	}

	delete[] searchEngineLineBuf;

	return true;
}

const char* CQSearchEngineList::GetSearchEngineName( const char* logStamp )
{
	for( size_t i = 0; i < GetList().size(); i++ )
	{
		if ( strstr( logStamp, GetList()[i].LogFileStamp().c_str() ) )
			return GetList()[i].UserAssignedName().c_str();
	}
	return 0;
}

std::string CQSearchEngineList::GetSearchEngineCombinedName( const char *explictSiteName )
{
	const char *name = GetSearchEngineName( explictSiteName );
	if ( !name )
		return explictSiteName;

	std::string currentCombinedName(name);
	currentCombinedName += " (";
	currentCombinedName += explictSiteName;
	currentCombinedName += ")";
	return currentCombinedName;
}


bool CQSearchEngineList::DecodeSearchEngines( const char *searchEnginesP )
{
	char *searchEnginesBuf = new char[strlen(  searchEnginesP ) + 1];
	char* searchEngines;

	strcpy( searchEnginesBuf, searchEnginesP );
	searchEngines = searchEnginesBuf;

	GetList().clear();

	int finished = 0;
	int count = 0;
	while( *searchEngines )
	{
		char *searchEngineLine;
		searchEngineLine = searchEngines;
		while( *searchEngines && *searchEngines != '\n' )
			searchEngines++;
		if ( *searchEngines == '\n' )
			*searchEngines = 0;
		else
			finished = 1;

		SELine searchEngineItem;
		if ( searchEngineItem.Decode( searchEngineLine ) )
		{
			GetList().push_back( searchEngineItem );
			count++;
		}

		if ( !finished )
			searchEngines++;
	}
	delete[] searchEnginesBuf;
	return true;
}

std::string CQSearchEngineList::Extract()
{
	std::string searchEngines;

	SELine *searchEngineSettingLine;
	SearchEngineSettingLineIter iter = GetList().begin();
	SearchEngineSettingLineIter iterEnd = GetList().end();

	for ( ; iter != iterEnd; iter++ )
	{
		searchEngineSettingLine = &(*iter);

		searchEngines += searchEngineSettingLine->UserAssignedName();
		searchEngines += ",";
		searchEngines += searchEngineSettingLine->LogFileStamp();
		searchEngines += ",";
		if ( searchEngineSettingLine->LogFileStampParams().size() )
		{
			// Search for the required
			StringsConstIter altIter = searchEngineSettingLine->LogFileStampParams().begin();
			StringsConstIter altIterEnd = searchEngineSettingLine->LogFileStampParams().end();
			for ( int i = 0; altIter != altIterEnd; altIter++, i++ )
			{
				searchEngines += ",";
				searchEngines +=  (*altIter).c_str();
			}
		}
		searchEngines += "\n";
	}
	searchEngines += "\n";
	return searchEngines;
}





