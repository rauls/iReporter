/******************************************************************************
** Filename:	SettingsRobotsBrowsers.h
**
** Overview:	Robots & Browsers Settings file - part of (Global) Settings
**				Application Wide.
**
** Note:	This file contains settings for Robots & Browsers
**
** Created:	By: Trevor Clayton		Date: A Hazy time in Oct 2001
**
** $Archive: $
** $Revision: $
** $Author: $
** $Date: $
******************************************************************************/

#ifndef SETTINGS_ROBOTS_BROWSWERS_H
#define SETTINGS_ROBOTS_BROWSWERS_H

#include <string>
#include <vector>


//
// class CQRobotSettingList
// Class that represents a list all the "Robots" which are searched for in a log file.
//
class CQRobotSettingList
{
public:
	CQRobotSettingList(){}
	bool Decode( const char *robots );
	std::string Extract();

	//
	// class RBLine
	// Class that represents a unique "name", with an associated "log stamp".
	//
	class RBLine
	{
	public:
		//
		// class Details
		// Class that maps a "name", to a "log stamp".
		//
		class Details
		{
		public:
			char *name;
			char *logStamp;
		};


		RBLine() {}
		RBLine( const RBLine& robotSettingLine );
		RBLine( const Details& robotDetails );

		// Conversion functions
		bool Decode( const char* robotLine );
		
		// Access functions
		const std::string& UserAssignedName() const { return m_userAssignedName; }
		void UserAssignedName( const std::string& userAssignedNameP ) { m_userAssignedName = userAssignedNameP; }
		const std::string& LogFileStamp() const { return m_logFileStamp; }
		void LogFileStamp( const std::string& logFileStampP ) { m_logFileStamp = logFileStampP; }
	private:
		std::string m_userAssignedName;
		std::string m_logFileStamp;
	};



	typedef std::vector<RBLine> NameToLogSettings;
	typedef NameToLogSettings::iterator NameToLogSettingsIter;

	NameToLogSettings& GetList() { return m_robotSettingLine; }
protected:
	bool AddRobotLine( const RBLine& robotLine, short count );
	NameToLogSettings m_robotSettingLine;
};



//
// class CQBrowserSettingList
// Class that represents a list all the "Browsers" which are searched for in a log file.
//
class CQBrowserSettingList : public CQRobotSettingList
{
public:
	CQBrowserSettingList(){}
};

#endif // SETTINGS_ROBOTS_BROWSWERS_H