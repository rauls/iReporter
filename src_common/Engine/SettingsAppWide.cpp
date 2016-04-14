/******************************************************************************
** Filename:	SettingsAppWide.cpp
**
** Overview:	Designed as a (Global) Settings file.
**
** Note:	This file contains settings for (1) Search Engines, (2) Robots,
**			(3) Browsers
**
** Created:	By: Trevor Clayton		Date: A Hazy time in Oct 2001
**
** $Archive: $
** $Revision: $
** $Author: $
** $Date: $
******************************************************************************/

#include "FWA.h"

#include "SettingsAppWide.h"
#include "GlobalPaths.h"

#include <string.h>
#include <stdio.h>
#include "myansi.h"

#ifdef DEF_MAC
	#include "MacStatus.h"
#endif

#define SEARCH_ENGINES_SECTION	"Search Engines"
#define ROBOTS_SECTION			"Robots"
#define BROWSWERS_SECTION		"Browsers"
#define DEFAULT_SETTINGSFILE	"GlobalSettings.conf"

const char *CQSettings::SETTINGS_APP_WIDE_DEFAULT = DEFAULT_SETTINGSFILE;

//
// Global settings object
//

CQSettings& CQSettings::TheSettings()
{
	static CQSettings s_settingsAppWide;

	return s_settingsAppWide;
}

CQSettings::CQSettings()
{
	m_searchEngineSettingLine.GetList().empty();
	m_robotSettingLine.GetList().empty();
	m_browserSettingLine.GetList().empty();
	m_filedata = 0;
}

CQSettings::~CQSettings()
{
	delete[] m_filedata;
}



bool CQSettings::ReadAll( const char* fileNameSptr )
{
	// ZT: temporarily, we look for the app settings file within the same folder as the executable
	std::string filename;
	if ( !fileNameSptr )
	{
		filename = gPath;
		filename += SETTINGS_APP_WIDE_DEFAULT;
	}
	else
		filename = fileNameSptr;

	FILE *fp = fopen( filename.c_str(), "r" );

	if ( !fp )
		fp = fopen( SETTINGS_APP_WIDE_DEFAULT, "r" );
		
	if ( !fp )
	{
		filename = gPath;
		filename += "..\\";
		filename += SETTINGS_APP_WIDE_DEFAULT;
		fp = fopen( filename.c_str(), "r" );
	}

	if ( fp )
	{
		ReadSearchEngines( fp );
		ReadRobots( fp );
		ReadBrowsers( fp );

		fclose( fp );
		return true;
	}
	return false;
}

bool CQSettings::ReadSearchEngines( FILE *fp )
{
	std::string searchEngines;
	if ( !ReadSetting( fp, SEARCH_ENGINES_SECTION, "", searchEngines ) )
		return false;
	SearchEngines().DecodeSearchEngines( searchEngines.c_str() );
	return true;
}

bool CQSettings::SaveAll( const char *fileNameSptr )
{
	FILE *fp = fopen( fileNameSptr, "w+" );
	if ( fp )
	{
		if ( !SaveSearchEngines( fp ) )
			return false;

		if ( !SaveRobots( fp ) )
			return false;

		if ( !SaveBrowsers( fp ) )
			return false;

		fclose( fp );
		return true;
	}
	return false;
}

bool CQSettings::SaveSearchEngines( FILE *fp )
{
	std::string searchEngines = SearchEngines().Extract();
	
	if ( !SaveSetting( fp, SEARCH_ENGINES_SECTION, "", searchEngines.c_str() ) )
		return false;
	return true;
}


bool CQSettings::ReadRobots( FILE *fp )
{
	std::string robots;
	if ( !ReadSetting( fp, ROBOTS_SECTION, "", robots ) )
		return false;
	Robots().Decode( robots.c_str() );
	return true;
}

bool CQSettings::SaveRobots( FILE *fp )
{
	std::string robots = Robots().Extract();
	
	if ( !SaveSetting( fp, ROBOTS_SECTION, "", robots.c_str() ) )
		return false;
	return true;
}

bool CQSettings::ReadBrowsers( FILE *fp )
{
	std::string browsers;
	if ( !ReadSetting( fp, BROWSWERS_SECTION, "", browsers ) )
		return false;
	Browsers().Decode( browsers.c_str() );
	return true;
}

bool CQSettings::SaveBrowsers( FILE *fp )
{
	std::string browsers = Browsers().Extract();
	
	if ( !SaveSetting( fp, BROWSWERS_SECTION, "", browsers.c_str() ) )
		return false;
	return true;
}

bool CQSettings::ReadSetting( FILE *fp, const char *sectionName, const char *settingName, std::string& settingValue )
{
	char *buf;
	if ( !m_filedata )
	{
		int initLen = 1024;
		m_filedata = new char[initLen];
		buf = m_filedata;

		int readsize = fread( m_filedata, 1, initLen, fp );
		int i = 2;
		for ( ; readsize == initLen; i++ ) // File maybe bigger than what we've read... so increase read size
		{
			char *temp = m_filedata;
			m_filedata = new char[initLen*i];
			buf = m_filedata;
			memcpy( m_filedata, temp, initLen*(i-1) );
			delete[] temp;
			readsize = fread( m_filedata+(initLen*(i-1)), 1, initLen, fp );
		}
		buf[initLen*(i-2)+readsize] = 0;
	}
	else
		buf = m_filedata;

	while( *buf )
	{
		if ( *buf == '[' )
		{
			buf++;
			int sectionNameLen = strlen( sectionName );
			if ( !memcmp( sectionName, buf, sectionNameLen ) && *(buf+sectionNameLen) == ']' )
			{
				// If we have a setting name, then we only read the one line for this setting item
				if ( *settingName && settingName[0] )
				{
					
				}
				else // No specific setting, so read all the setting section.
				{
					buf += sectionNameLen+1;
					char *ptr = buf;
					char ch;
					while( *buf && *buf != '[' )
						buf++;
					ch = *buf;
					*buf = 0;
					settingValue = ptr;
					*buf = ch;
				}
				return true;
			}
		}
		else
			buf++;
	}

	return false;
}

bool CQSettings::SaveSetting( FILE *fp, const char *sectionName, const char *settingName, const char* settingValue )
{
	std::string data = "[";
	data += sectionName;
	data += "]\n";
	if ( settingName && settingName[0] )
	{
		data += settingName;
		data += "=";
	}
	data += settingValue;

	size_t len = fwrite( data.c_str(), 1, data.length(), fp );
	if ( len != data.length() )
		return false;
	return true;
}

void CQSettings::OpenAppWideSettings( const char* fileNameSptr )
{
	if (!ReadAll( fileNameSptr ))
#ifdef DEF_MAC
		HandleGenericError (kCoreErrorAppWideSettingsMissing);
#else
		ErrorMsg ("Couldn't find configuration file \"GlobalSettings.conf\". Please reinstall or update the file.");
#endif		
}

CQSearchEngineList::SELine* CQSettings::GetSearchEngine( short index )
{
	if ( index >= SearchEngines().GetList().size() )
		return 0;

	return &SearchEngines().GetList()[index];
}

CQRobotSettingList::RBLine* CQSettings::GetRobot( short index )
{
	if ( index >= Robots().GetList().size() )
		return 0;

	return &Robots().GetList()[index];
}

CQRobotSettingList::RBLine* CQSettings::GetBrowser( size_t index )
{
	return index < m_browserSettingLine.GetList().size() ?
		   &m_browserSettingLine.GetList()[index] : 0;
}
