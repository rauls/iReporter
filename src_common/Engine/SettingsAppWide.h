/******************************************************************************
** Filename:	SettingsAppWide.h
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

#ifndef SETTINGS_APP_WIDE_H
#define SETTINGS_APP_WIDE_H

#include <stdio.h>
#include <string>
#include "SettingsSearchEngines.h"
#include "SettingsRobotsBrowsers.h"

//
// CQSettings class 
//
class CQSettings
{
public:
	// General
	CQSettings();
	~CQSettings();
	static CQSettings& TheSettings();
	void OpenAppWideSettings( const char* fileName = 0 );

	CQSearchEngineList::SELine* GetSearchEngine( short index );
	CQRobotSettingList::RBLine* GetRobot( short index );
	CQRobotSettingList::RBLine* GetBrowser( size_t index );

	CQSearchEngineList& SearchEngines() { return m_searchEngineSettingLine; }
	CQRobotSettingList& Robots() { return m_robotSettingLine; };
	CQBrowserSettingList& Browsers() { return m_browserSettingLine; }
	static const char *SETTINGS_APP_WIDE_DEFAULT;

protected:
	bool ReadAll( const char* fileName );
	bool SaveAll( const char* fileName );
	bool ReadSetting( FILE *fp, const char *sectionName, const char *settingName, std::string& settingValue );
	bool SaveSetting( FILE *fp, const char *sectionName, const char *settingName, const char* settingValue );

	// Search Engines
	bool ReadSearchEngines( FILE *fp );
	bool SaveSearchEngines( FILE *fp );
	// Robots
	bool ReadRobots( FILE *fp );
	bool SaveRobots( FILE *fp );
	// Browsers
	bool ReadBrowsers( FILE *fp );
	bool SaveBrowsers( FILE *fp );

private:
	CQSearchEngineList m_searchEngineSettingLine;
	CQRobotSettingList m_robotSettingLine;
	CQBrowserSettingList m_browserSettingLine;
	char *m_filedata;
};

#endif // SETTINGS_APP_WIDE_H
