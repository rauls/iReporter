// ****************************************************************************
// Copyright (C) 2000-2004
//
// File 	:	HelpCard.h
// Created 	:	Tuesday, 16 October 2001
//
// Abstract :	See HelpCard.cpp
//
// ****************************************************************************

#ifndef	HELPCARD_H
#define HELPCARD_H

#include	<list>
#include	<string>
#include	<fstream>

#include	"VirtDomInfo.h"			// for VDinfoP

extern const char			TAG_NEWLINE[]		;

extern const char			STARTTAG_BOLD[]		;
extern const char			STARTTAG_ITALICS[]	;
extern const char			STARTTAG_UNDERLINE[];
extern const char			ENDTAG_BOLD[]		;
extern const char			ENDTAG_ITALICS[]	;
extern const char			ENDTAG_UNDERLINE[]	;

extern const char			TOKEN_OVERVIEW[]	;
extern const char			TOKEN_GRAPH[]		;
extern const char			TOKEN_TABLE[]		;
extern const char			TOKEN_END[]			;

extern const unsigned char	HELPTEXT_FORMATMASK_BOLD		;
extern const unsigned char	HELPTEXT_FORMATMASK_ITALICS		;
extern const unsigned char	HELPTEXT_FORMATMASK_UNDERLINE	;
extern const unsigned char	HELPTEXT_FORMATMASK_NEWLINE		;

extern const char			HELPCARD_EXTENSION[];


// ****************************************************************************
// We need to decide the pallettes for the HelpCard, so lets define
// them all in one place.
// ****************************************************************************
#define		RGBHelpCardBackground		-1
#define		RGBHelpCardTitle			RGBtableTitle
#define		RGBHelpCardOverview			RGBtableItems
#define		RGBHelpCardGraph			RGBtableItems
#define		RGBHelpCardTable			RGBtableItems


// ***************************************************************************
// CQHelpCardTextFragment is a string with a mask.
// The masks are:
//		HELPTEXT_FORMATMASK_BOLD		- indicates that this string is bold.
//		HELPTEXT_FORMATMASK_ITALICS		- indicates that this string is italics.
//		HELPTEXT_FORMATMASK_UNDERLINE	- indicates that this string is underlined.
//		HELPTEXT_FORMATMASK_NEWLINE		- indicates a following newline.
// ***************************************************************************
typedef	std::pair<unsigned char, std::string>	CQHelpCardTextFragment;


// ***************************************************************************
// CQHelpCardText is a container of CQHelpCardTextFragment's.
// It has method(s) that understand the grammar for the HelpCard files and
// ***************************************************************************
class CQHelpCardText : public std::list<CQHelpCardTextFragment>
{
public:
	void	load(std::istream&	stm, const char* szTerminatingToken = TOKEN_END);
	void	load(const std::string&	str);
};


// ***************************************************************************
// CQHelpCard is the object that contains all the info for one HelpCard.
// It contains the Title, Overview, Graph desc and Table Desc.
// ***************************************************************************
class CQHelpCard
{
public:
	CQHelpCard(const char* szTitle)				: m_strTitle(szTitle)	{}
	CQHelpCard(const std::string& rstrTitle)	: m_strTitle(rstrTitle) {}
	virtual ~CQHelpCard()		{}
public:
	void	Create(std::istream& stmHelpCard);

	const std::string&		GetTitle(void)		const	{ return m_strTitle;	 }
	const CQHelpCardText&	GetOverview(void)	const	{ return m_textOverview; }
	const CQHelpCardText&	GetGraph(void)		const	{ return m_textGraph; }
	const CQHelpCardText&	GetTable(void)		const	{ return m_textTable; }

private:
	std::string			m_strTitle;
	CQHelpCardText		m_textOverview;
	CQHelpCardText		m_textGraph;
	CQHelpCardText		m_textTable;
};


// ***************************************************************************
// Function:	WriteHelpCard().
// Abstract:	Rather than forcing the base class (baseFile) to implement
//				this method, it passes the report as a parameter.
//				This function gathers the content of the HelpCard and writes
//				it to the report object.
// ***************************************************************************
void		WriteHelpCard(const char* szFileName, VDinfoP VDptr, FILE *fp, class baseFile* pReport, long lId);

void		BuildHelpcardFilename(std::string& rstrFullFileName, const char* szLanguage, const char* szFilename);

#endif

// ****************************************************************************
// Copyright (C) 2000-2004
// ****************************************************************************