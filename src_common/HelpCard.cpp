// ****************************************************************************
// Copyright (C) 2000-2004
//
// File 	:	HelpCard.cpp
// Created 	:	Tuesday, 16 October 2001
//
// Abstract :	This file implements the classes and functions needed to generate
//				HelpCards for the reports.
//
// ****************************************************************************


#include "FWA.h"
#include "HelpCard.h"


#include "Report.h"				// for RGBtableTitle and the like.
#include "ReportClass.h"		// for baseFile.
#include "StandardPlot.h"		// for GraphWidth()
#include "Translate.h"			// for TranslateID
#include "GlobalPaths.h"
#include "Icons.h"				// for the binary versions of the icons for this report.
#include "myansi.h"				// for mystrncmpi
#include "PDFFile.h"

// ****************************************************************************
// These are the tokens that can be used the the Helpcard files.
// ****************************************************************************
const char	TAG_NEWLINE[]			= "<br>";

const char	STARTTAG_BOLD[]			= "<b>";
const char	STARTTAG_ITALICS[]		= "<i>";
const char	STARTTAG_UNDERLINE[]	= "<u>";
const char	ENDTAG_BOLD[]			= "</b>";
const char	ENDTAG_ITALICS[]		= "</i>";
const char	ENDTAG_UNDERLINE[]		= "</u>";

const char	TOKEN_OVERVIEW[]		= "<!--OVERVIEW-->";
const char	TOKEN_GRAPH[]			= "<!--GRAPH-->";
const char	TOKEN_TABLE[]			= "<!--TABLE-->";
const char	TOKEN_END[]				= "<!--END-->";

const unsigned char	HELPTEXT_FORMATMASK_BOLD		= 1;
const unsigned char	HELPTEXT_FORMATMASK_ITALICS		= 2;
const unsigned char	HELPTEXT_FORMATMASK_UNDERLINE	= 4;
const unsigned char	HELPTEXT_FORMATMASK_NEWLINE		= 8;

const char	HELPCARD_EXTENSION[]	= "HELPCARD";



// ****************************************************************************
// These have come from translate.cpp
// ****************************************************************************
extern const char*	LANGDIR_V41		;
extern const char*	LANGDIR_V37		;
extern const char*	LANG_EXT		;
extern const char*	HELP_CARDS		;

void
BuildHelpcardFilename(std::string& rstrFullFileName, const char* szLanguage, const char* szFilename)
{
	rstrFullFileName =	gPath;
#ifndef DEF_MAC
	rstrFullFileName +=	PATHSEPSTR;
#endif
	rstrFullFileName +=	LANGDIR_V41;
	rstrFullFileName +=	PATHSEPSTR;
	rstrFullFileName += szLanguage;
	rstrFullFileName +=	PATHSEPSTR;
	rstrFullFileName +=	HELP_CARDS;
	rstrFullFileName +=	PATHSEPSTR;
	rstrFullFileName +=	szFilename;
	rstrFullFileName +=	".txt";
}

// ****************************************************************************
// Function:	WriteHelpCard()
//
// Abstract:	Rather than forcing the base class (baseFile) to implement
//				this method, it passes the report as a parameter.
//				This function gathers the content of the HelpCard and writes
//				it to the report object.
// ****************************************************************************
void
WriteHelpCard(const char* szFileName, VDinfoP VDptr, FILE *fp, baseFile* pReport, long lId)
{
	DEF_PRECONDITION(szFileName);
	DEF_PRECONDITION(VDptr);
	DEF_PRECONDITION(fp);
	DEF_PRECONDITION(pReport);


	// ****************************************************************************
	// Does this report-type support Help Cards?
	// ****************************************************************************
	if (!pReport->SupportHelpCards())
	{
		// HelpCard not supported by this type of report.
		return;
	}


	std::string		strFileName;
	std::ifstream	strmHelpCard;
	const char*		szTitle = "Report";	// For Defensive coding.
	ReportTypesP	pReportType;
	long*			pReportFlags;
	bool			bTableOn(true);
	bool			bGraphOn(true);
	bool			bHelpCardOn(true);


	// ****************************************************************************
	// We need to lookup some of the flags set for this report.
	// Additionally we can lookup the Title of the report to go in the HelpCard
	// title. "Help Card: <title> Report."
	// ****************************************************************************
	pReportType = FindReportTypeData( lId );
	pReportFlags = FindReportFlags( lId );
	if (pReportType && pReportFlags)
	{
		bTableOn	= TABLE(*pReportFlags);
		bGraphOn	= GRAPH(*pReportFlags);
		bHelpCardOn	= COMMENT(*pReportFlags);
		szTitle		= TranslateID(pReportType->titleStringID);
		DEF_ASSERT(szTitle != NULL);	// This should be in the Language file.
		if (!szTitle)
			szTitle = "Report";
	}

	
	// ****************************************************************************
	// If the HelpCard is turned off for this report then lets just return now.
	// ****************************************************************************
	if (!bHelpCardOn)
	{
		// HelpCard turned off.
		return;
	}


	// ****************************************************************************
	// Build up the title string (hard-coded!)
	// ****************************************************************************
	std::string	str;
	str =	"Help Card: ";
	str +=	STARTTAG_BOLD;
	str +=	szTitle;
	str +=	ENDTAG_BOLD;


	CQHelpCard		card(str);


	// ****************************************************************************
	// Load up the CQHelpCard variable from the file.
	// ****************************************************************************

	// ****************************************************************************
	// Construct the 1st choice of file for this card - In the current language folder.
	// ****************************************************************************
	(void)BuildHelpcardFilename(strFileName, MyPrefStruct.language, szFileName);


	// ****************************************************************************
	// Open the above named file.
	// ****************************************************************************
	strmHelpCard.open(strFileName.c_str()/*, std::ios_base::in*/);
	// ****************************************************************************
	// Then if we found it...
	// ****************************************************************************
	if (strmHelpCard.is_open())	
	{
		// ****************************************************************************
		// load the file into the CQHelpCard variable 'card'. 
		// ****************************************************************************
		card.Create(strmHelpCard);
		// ****************************************************************************
		// and close the file.
		// ****************************************************************************
		strmHelpCard.close();
	}
	else
	{
		// ****************************************************************************
		// For some reason the std::fstream object is in a wierd state where failing to
		// open a file means it can not open another file without a 'clear' first.
		// (The symption is the ever call to getline produces an empty line).
		// ****************************************************************************
		strmHelpCard.clear();


		// ****************************************************************************
		// Construct the 2nd choice of file for this card - In the 'English' language folder.
		// ****************************************************************************
		(void)BuildHelpcardFilename(strFileName, ENGLISH_LANG, szFileName);


		// ****************************************************************************
		// Open the above named file.
		// ****************************************************************************
		strmHelpCard.open(strFileName.c_str()/*, std::ios_base::in*/);
		// ****************************************************************************
		// Then if we found it...
		// ****************************************************************************
		if (strmHelpCard.is_open())	
		{
			// ****************************************************************************
			// load the file into the CQHelpCard variable 'card'. 
			// ****************************************************************************
			card.Create(strmHelpCard);
			// ****************************************************************************
			// and close the file.
			// ****************************************************************************
			strmHelpCard.close();
		}
		else
		{
			// ****************************************************************************
			// No HelpCard as there is no file in the default or English directories.
			// ****************************************************************************
			return;
		}
	}


	// ******************************************************************************
	// Separate this table from the other content of the report.
	// ******************************************************************************
	pReport->Stat_WriteSpace( fp, 0 );


	// ******************************************************************************
	// Lets just make it easier to find the HelpCard html when we view the source.
	// ******************************************************************************
	pReport->Stat_WriteComment(fp, "Start-HelpCard");


	// ******************************************************************************
	// Label/Anchor this position as the HELPCARD_EXTENSION!
	// ******************************************************************************
	pReport->Stat_WriteAnchor(fp, HELPCARD_EXTENSION);


	// ******************************************************************************
	// Write the Helpcard table.
	// ******************************************************************************
	pReport->Stat_WriteHelpCard(fp, card, bTableOn, bGraphOn);


	// ******************************************************************************
	// Lets just make it easier to find the HelpCard html when we view the source.
	// ******************************************************************************
	pReport->Stat_WriteComment(fp, "End-HelpCard");


	// ******************************************************************************
	// Separate this table from the other content of the report.
	// ******************************************************************************
	pReport->Stat_WriteSpace( fp, 1 );
}


// ******************************************************************************
// Method:		void CQHelpCard::Create(std::istream& stmHelpCard)
//
// Abstract:	This method populates this object with the information from the
//				file.  The contents of the file are - :
//					"Ignored text"
//					"that can span many lines."
//					"<!--OVERVIEW-->"	on its own line
//					{This is text that will appear in the overview section of the
//					HelpCard.  This can span multiple lines and can include <br> for
//					newlines, <b>...</b> for bold, <i>...</i> for italics and
//					<u>...</u> for underline. }
//					"<!--GRAPH-->"	on its own line
//					{ as per overview above }
//					"<!--TABLE-->"	on its own line
//					{ as per overview above }
//				
// ******************************************************************************
void
CQHelpCard::Create(std::istream& stmHelpCard)
{
	std::string		strLine;
	CQHelpCardText	tmp;
	
	// ******************************************************************************
	// Load into a local variable the text upto TOKEN_OVERVIEW, so that the
	// fstream is upto the Overview section.  This object is discarded.
	// ******************************************************************************
	tmp.load(stmHelpCard, TOKEN_OVERVIEW);

	// ******************************************************************************
	// Now load the Overview, graph and Table sections into the relevant 
	// CQHelpCardText members.
	// ******************************************************************************
	m_textOverview.	load(stmHelpCard, TOKEN_GRAPH);
	m_textGraph.	load(stmHelpCard, TOKEN_TABLE);
	m_textTable.	load(stmHelpCard);
}

void	
CQHelpCardText::load(std::istream& stm, const char* szTerminatingToken)
{
	std::string	strLine;
	std::string	strContent;

	// **************************************************************
	// Get the whole string for this section of the HelpFile.
	// **************************************************************
	while (!std::getline(stm, strLine).eof())
	{
		if (strLine == szTerminatingToken)
			break;

		// Ignore lines beginning with '#'.
		if (strLine.size() && strLine.c_str()[0]=='#')
			continue;

		strContent += strLine;
		strContent += TAG_NEWLINE;
	}

	// **************************************************************
	// Once we have the whole string for a section pass it to the
	// load(std::string) function to parse for the formatting tokens.
	// **************************************************************
	load(strContent);
}

void	
CQHelpCardText::load(const std::string& strContent)
{
	// **************************************************************
	// Parse the string for the known tokens.
	// **************************************************************
	const char*		szBegin;
	const char*		szEnd;
	unsigned char	ucMask(0);

	szBegin=szEnd=strContent.c_str();
	for (;;)
	{
		// **************************************************************
		// All the tags start with '<' so lets just skip to the next
		// instance of it.
		// **************************************************************
		szEnd = strstr(szEnd, "<");
		if (!szEnd)
			break;


		// **************************************************************
		// If we find a NEWLINE tag then we need to just add this line
		// go the the next, preserving the format-mask.
		// **************************************************************
		if (mystrncmpi(szEnd, TAG_NEWLINE, strlen(TAG_NEWLINE)) == 0)
		{
			this->push_back(CQHelpCardTextFragment(ucMask|HELPTEXT_FORMATMASK_NEWLINE,std::string(szBegin, szEnd-szBegin)));
			szEnd += strlen(TAG_NEWLINE);
			szBegin = szEnd;
			continue;
		}


		// **************************************************************
		// The 1st three conditionals here are the start tags for the
		// text formatting (Bold, Italics & Underline) the following
		// three are the respective end tags.
		// **************************************************************
		if (mystrncmpi(szEnd, STARTTAG_BOLD, strlen(STARTTAG_BOLD)) == 0)
		{
			// **************************************************************
			// Add the string we have just processed with its current format.
			// Add BOLD to the format.
			// Move the szEnd pointer past the tag, so the search for the next will work.
			// Set the szbegin pointer to the start of the next string.
			// **************************************************************
			if (szEnd != szBegin)
				this->push_back(CQHelpCardTextFragment(ucMask,std::string(szBegin, szEnd-szBegin)));
			ucMask	|=	HELPTEXT_FORMATMASK_BOLD;
			szEnd	+=	strlen(STARTTAG_BOLD);
			szBegin	=	szEnd;
		}
		else if (mystrncmpi(szEnd, STARTTAG_ITALICS, strlen(STARTTAG_ITALICS)) == 0)
		{
			if (szEnd != szBegin)
				this->push_back(CQHelpCardTextFragment(ucMask,std::string(szBegin, szEnd-szBegin)));
			ucMask	|=	HELPTEXT_FORMATMASK_ITALICS;
			szEnd	+=	strlen(STARTTAG_ITALICS);
			szBegin	=	szEnd;
		}
		else if (mystrncmpi(szEnd, STARTTAG_UNDERLINE, strlen(STARTTAG_UNDERLINE)) == 0)
		{
			if (szEnd != szBegin)
				this->push_back(CQHelpCardTextFragment(ucMask,std::string(szBegin, szEnd-szBegin)));
			ucMask	|=	HELPTEXT_FORMATMASK_UNDERLINE;
			szEnd	+=	strlen(STARTTAG_UNDERLINE);
			szBegin	=	szEnd;
		}
		else if (mystrncmpi(szEnd, ENDTAG_BOLD, strlen(ENDTAG_BOLD)) == 0)
		{
			if (szEnd != szBegin)
				this->push_back(CQHelpCardTextFragment(ucMask,std::string(szBegin, szEnd-szBegin)));
			ucMask	&=	~HELPTEXT_FORMATMASK_BOLD;
			szEnd	+=	strlen(ENDTAG_BOLD);
			szBegin	=	szEnd;
		}
		else if (mystrncmpi(szEnd, ENDTAG_ITALICS, strlen(ENDTAG_ITALICS)) == 0)
		{
			if (szEnd != szBegin)
				this->push_back(CQHelpCardTextFragment(ucMask,std::string(szBegin, szEnd-szBegin)));
			ucMask	&=	~HELPTEXT_FORMATMASK_ITALICS;
			szEnd	+=	strlen(ENDTAG_ITALICS);
			szBegin	=	szEnd;
		}
		else if (mystrncmpi(szEnd, ENDTAG_UNDERLINE, strlen(ENDTAG_UNDERLINE)) == 0)
		{
			if (szEnd != szBegin)
				this->push_back(CQHelpCardTextFragment(ucMask,std::string(szBegin, szEnd-szBegin)));
			ucMask	&=	~HELPTEXT_FORMATMASK_UNDERLINE;
			szEnd	+=	strlen(ENDTAG_UNDERLINE);
			szBegin	=	szEnd;
		}
		else
		{
			// Not a recognised tag, might just be an innocent '<'.
			++szEnd;
		}
	}

	// Add the trailing string.
	if (szBegin && *szBegin)
		this->push_back(CQHelpCardTextFragment(ucMask,std::string(szBegin)));
}

// ****************************************************************************
// Copyright (C) 2000-2004
// ****************************************************************************

