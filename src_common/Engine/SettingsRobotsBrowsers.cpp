/******************************************************************************
** Filename:	SettingsRobotsBrowsers.cpp
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

#include "FWA.h"

#include <string.h>
#include "myansi.h"
#include "SettingsRobotsBrowsers.h"

CQRobotSettingList::RBLine::RBLine( const RBLine& robotSettingLine )
	:	m_userAssignedName(robotSettingLine.UserAssignedName()),
		m_logFileStamp(robotSettingLine.LogFileStamp())
{
}

CQRobotSettingList::RBLine::RBLine( const CQRobotSettingList::RBLine::Details& robotDetails )
	:	m_userAssignedName(robotDetails.name),
		m_logFileStamp(robotDetails.logStamp)
{
}

bool CQRobotSettingList::RBLine::Decode( const char *lineP )
{
	if ( !lineP || *lineP==0 || *lineP == '#' )
		return false;

	char *lineBuf = new char[ strlen( lineP )+1 ];
	strcpy( lineBuf, lineP );
	char *line = lineBuf;

	// Get the Name
	char *element = line;
	line = strchr( line, ',' );
	if ( line )
	{
		*line = 0;
		m_userAssignedName = element;
	}
	else
	{
		m_userAssignedName = element;
		delete[] lineBuf;
		return false;
	}
	line++;

	// Get the Logfile Stamp
	element=line;

#if DEF_UNIX
	trimLine( line );		// Proper way for all OperSYSs
#else
	line = strchr( line, '\n' );
	if( line )
	{
		*line = 0;
	}
#endif
	m_logFileStamp=element;

	delete[] lineBuf;

	return true;
}


bool CQRobotSettingList::Decode( const char *robotsP )
{
	if ( !robotsP )
		return false;
	if ( *robotsP == '#' )
		return false;
	char *robotsBuf = new char[ strlen( robotsP ) + 1];
	strcpy( robotsBuf, robotsP );
	char *robots = robotsBuf;

	GetList().clear();

	int finished = 0;
	int count = 0;
	while( *robots )
	{
		char *robotLine;
		robotLine = robots;
		while( *robots && *robots != '\n' )
			robots++;
		if ( *robots == '\n' )
			*robots = 0;
		else
			finished = 1;

		RBLine robotItem;
		if ( robotItem.Decode( robotLine ) )
		{
			GetList().push_back( robotItem );
			count++;
		}

		if ( !finished )
			robots++;
	}
	delete[] robotsBuf;
	return true;
}

std::string CQRobotSettingList::Extract()
{
	std::string robots;

	RBLine *robotSettingLine;
	NameToLogSettingsIter iter = GetList().begin();
	NameToLogSettingsIter iterEnd = GetList().end();

	for ( ; iter != iterEnd; iter++ )
	{
		robotSettingLine = &(*iter);

		robots += robotSettingLine->UserAssignedName();
		robots += ",";
		robots += robotSettingLine->LogFileStamp();
		robots += "\n";
	}
	robots += "\n";
	return robots;
}

bool CQRobotSettingList::AddRobotLine( const RBLine& newRobotLine, short count )
{
	GetList().push_back( newRobotLine );
	return true;
}
