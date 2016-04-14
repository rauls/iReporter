

/*
##
##      ##
##      ##
##      ##  ##
##      #########
##          ##
##          ##
##          ##
##
##		V4 Analyzer -	Write module, this outputs all html/rtf/, and spits out calls to make
##						Doc/Excel/PDF to other modules, but basically handles all the reports.
##
##
##
####################################################*/ 
#include "FWA.h"

#include <string>
#define NO_INLINE
#include <list>
#include <map>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "datetime.h"

#include "myansi.h"
#include "config_struct.h"

#include "engine_drill.h"
#include "Stats.h"
#include "editpath.h"
#include "translate.h"
#include "ReportPDF.h"

#include "serialReg.h"

#include "Icons.h"

#ifdef DEF_MAC
#include "main.h"
#include "progress.h"
#endif

extern "C" struct App_config MyPrefStruct;


#include "PDFPlot.h"
#include "FileTypes.h"
#include "DateFixFileName.h"

#if DEF_MAC
	#include "post_comp.h"
#endif

#if DEF_WINDOWS
	#include <sys/stat.h>
	#include "Windnr.h"
	#include "Winmain.h"
	#include "postproc.h"
	#include "httpinterface.h"
#endif				

#if DEF_UNIX		// UNIX include
	#include <sys/stat.h>
	#include <sys/errno.h>
	#include "unistd.h"
	#include "unixmain.h"
	#include "postproc.h"
	#include "httpinterface.h"
#endif


extern long	VDnum;
extern char *Extensions[];

#include "gd.h"
#include "PDFColors.h"

#include "Report.h"
#include "HelpCard.h"
#include "GlobalPaths.h"		// for MAXFILENAMESSIZE

void WriteTimeTakenInfo( VDinfoP VDptr, std::string infoDesc, int info )
{
	double timeTaken( timems() - VDptr->time1 );

	char buf[512];
	sprintf( buf, "Time: %02d:%02d:%02d.%d, %s: %d", 
		     (int) timeTaken/(60*60),
		     (int)(timeTaken/60)%60,
			 (int)(timeTaken)%60,
			 (int)(timeTaken*10)%10,
			 infoDesc.c_str(), info
			 );

	OutDebug( buf );
}

pdfFile::pdfFile( PDFSettings thePDFSettings, PDFTableSettings thePDFTableSettings ) 
	: baseFile(), PDFFile( thePDFSettings, thePDFTableSettings )
{
	m_style = FORMAT_PDF;
	fileExtension = PDF_EXT;
	ResetTable();
	sessionWriter = new CQPDFSessionWriter( fileExtension, *this, thePDFTableSettings, theFP, thePDFSettings.SubPageDrawWidth() );
}


pdfFile::~pdfFile(){}

int pdfFile::Init( const char *fileName, FILE *filePtr )
{
	return Open( fileName, filePtr );
}


// **********************************************************************************************
// These are the HelpCard Methods.
// **********************************************************************************************

void pdfFile::Stat_WriteIconAndText(  FILE* fp, const char* szText,  const char* szIcon, const char* szPopHelp)
{
	std::string splitAtParagraphs = szText;

	short size = theTableData.data.fontSize;
	short style = theTableData.data.fontStyle; // PDF_NORMAL;
	if ( timesNRFont->ValidateHTMLString( splitAtParagraphs ) )
		style = PDF_HTML_FORMATTING;

	PDFMultiLineCellTextStrList cellStrList;
	timesNRFont->SplitHTMLStringAtParagraphs( splitAtParagraphs, size, style, cellStrList );
	AddTableCellMultilineText( cellStrList, PDF_LEFTJUSTIFY );
}

void pdfFile::BuildHelpCardText(const class CQHelpCardText& rCardText, std::string& rstr)
{
	CQHelpCardText::const_iterator	it;

	rstr = "";

	for (it = rCardText.begin(); it!=rCardText.end(); ++it)
	{
//		if (it->first & HELPTEXT_FORMATMASK_BOLD)		rstr += STARTTAG_BOLD;
//		if (it->first & HELPTEXT_FORMATMASK_ITALICS)	rstr += STARTTAG_ITALICS;
//		if (it->first & HELPTEXT_FORMATMASK_UNDERLINE)	rstr += STARTTAG_UNDERLINE;
		rstr += (*it).second;
//		if (it->first & HELPTEXT_FORMATMASK_BOLD)		rstr += ENDTAG_BOLD;
//		if (it->first & HELPTEXT_FORMATMASK_ITALICS)	rstr += ENDTAG_ITALICS;
//		if (it->first & HELPTEXT_FORMATMASK_UNDERLINE)	rstr += ENDTAG_UNDERLINE;

		if (it->first & HELPTEXT_FORMATMASK_NEWLINE)
			rstr += "\n";
//			rstr += TAG_NEWLINE;
	}
}

void pdfFile::WriteHelpCardIcons(VDinfoP VDptr)
{
	std::string	strFileName;
	FILE*		fIcon;
	char		szPath[MAXFILENAMESSIZE+1];
	
	szPath[0] = szPath[MAXFILENAMESSIZE] = 0;

	PathFromFullPath( MyPrefStruct.outfile, szPath); 


	strFileName =	szPath;
	strFileName +=	"OverviewIcon.jpg";
	fIcon = fopen(strFileName.c_str(), "wb");
	if (fIcon)
	{
		::fwrite(QS::g_ucIconJpg_Overview,  sizeof(unsigned char), QS::g_ulSizeJpg_Overview, fIcon);
	}
	(void)fclose(fIcon);
	(void)fflush(fIcon);


	strFileName =	szPath;
	strFileName +=	"GraphIcon.jpg";
	fIcon = fopen(strFileName.c_str(), "wb");
	if (fIcon)
	{
		::fwrite(QS::g_ucIconJpg_Graph,  sizeof(unsigned char), QS::g_ulSizeJpg_Graph, fIcon);
	}
	(void)fclose(fIcon);
	(void)fflush(fIcon);


	strFileName =	szPath;
	strFileName +=	"TableIcon.jpg";
	fIcon = fopen(strFileName.c_str(), "wb");
	if (fIcon)
	{
		::fwrite(QS::g_ucIconJpg_Table,  sizeof(unsigned char), QS::g_ulSizeJpg_Table, fIcon);
	}
	(void)fclose(fIcon);
	(void)fflush(fIcon);


	strFileName = szPath;
	strFileName += "OverviewIcon.jpg";
	m_imageOverview = this->CreateNewPDFImageObject( strFileName.c_str() );
	InsertJPEGImageFromFile( strFileName.c_str(), m_imageOverview );
	m_imageOverview->imageData = "/OverviewIcon.jpg Do\rQ\r";

	strFileName = szPath;
	strFileName += "GraphIcon.jpg";
	m_imageGraph = this->CreateNewPDFImageObject( strFileName.c_str() );
	InsertJPEGImageFromFile( strFileName.c_str(), m_imageGraph );
	m_imageGraph->imageData = "/GraphIcon.jpg Do\rQ\r";

	strFileName = szPath;
	strFileName += "TableIcon.jpg";
	m_imageTable = this->CreateNewPDFImageObject( strFileName.c_str() );
	InsertJPEGImageFromFile( strFileName.c_str(), m_imageTable );
	m_imageTable->imageData = "/TableIcon.jpg Do\rQ\r";
}

void pdfFile::CleanUpHelpCardIcons(VDinfoP VDptr)
{
	std::string	strFileName;
	char		szPath[MAXFILENAMESSIZE+1];
	
	szPath[0] = szPath[MAXFILENAMESSIZE] = 0;

	PathFromFullPath( MyPrefStruct.outfile, szPath); 

	strFileName =	szPath;
	strFileName +=	"OverviewIcon.jpg";
	(void)::remove(strFileName.c_str());

	strFileName =	szPath;
	strFileName +=	"GraphIcon.jpg";
	(void)::remove(strFileName.c_str());

	strFileName =	szPath;
	strFileName +=	"TableIcon.jpg";
	(void)::remove(strFileName.c_str());
}

void pdfFile::Stat_WriteCenterStart( FILE* fp )
{
}

void pdfFile::Stat_WriteCenterEnd( FILE* fp )
{
}

// **********************************************************************************************
// End HelpCard Methods.
// **********************************************************************************************

void pdfFile::FinishReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs )
{
	Close();
}


int pdfFile::WriteLine( FILE *fp, const char *txt )
{
	AddTableText( txt, PDF_LEFTJUSTIFY );
	return 1;
}



void pdfFile::Stat_WriteSpace( FILE *fp, long size )
{
	while (size--)
	{
		PDFFile::CurrYPos(CurrYPos() + PDFFile::FontHeight());
	}

#ifdef	NOT_DEFINED
	PDFTextList	pdfTextFilter;

	while (size--)
		pdfTextFilter.push_back(PDFTextStr("",0,PDF_TEXT_ON_NEXT_LINE,1));
	DrawPDFText( pdfTextFilter );
#endif
}

void pdfFile::Stat_WriteItal( FILE *fp, const char *txt )
{
	WriteLine( fp, txt );
}

void pdfFile::Stat_WriteBold( FILE *fp, const char *txt )
{
	WriteLine( fp, txt );
}

void pdfFile::Stat_WriteLine( FILE *fp, const char *txt )
{
	WriteLine( fp, txt );
}

void pdfFile::Stat_WriteCenterSmall( FILE *fp, const char *txt )
{
	WriteLine( fp, txt );
}

void pdfFile::WriteFilterText(FILE* fp, const char* szFilterString)
{
	PDFTextList	pdfTextFilter;
	
	int	iStrLen;
	for (const char*	sz = szFilterString; iStrLen=mystrlen(sz); sz += (iStrLen+1))
	{
		pdfTextFilter.push_back(PDFTextStr(sz,0,PDF_TEXT_ON_NEXT_LINE,1));
	}

	DrawPDFText( pdfTextFilter );
}

void SessionEntry()
{

}


void SessionSummaryInfo()
{
//	summaryPDFTextList.push_back( PDFTextStr( PDF_LINE, 0, 0, 1 ) );
//	summaryPDFTextList.push_back( PDFTextStr( text, 0, PDF_NEW_SECTION, PDF_BOLD, PDF_SUMM_SUBTITLE_SIZE ) );
//	summaryPDFTextList.push_back( PDFTextStr( PDF_BLANK, PDF_TEXT_CENTERED, PDF_SUMM_SUBTITLE_SIZE/2, 1, PDF_SUMM_SUBTITLE_SIZE/2 ) );
}




void pdfFile::WriteSessionsList()
{
	while( ((CQPDFSessionWriter*)sessionWriter)->clickStreamTextLists.size() != 0 )
	{
		if ( ((CQPDFSessionWriter*)sessionWriter)->clickStreamTextLists.size() )
		{
			PDFTextList& textList = ((CQPDFSessionWriter*)sessionWriter)->clickStreamTextLists.front();
			if ( textList.size() )
			{
				DrawPDFText( textList );
			}
			((CQPDFSessionWriter*)sessionWriter)->clickStreamTextLists.pop_front();
		}
	}

	// **********************************************************************
	// If we have HelpCards turned on then we want to trick it into starting
	// it on a new page.  If its not on then it has no effect.
	// **********************************************************************
	CurrYPos(PageHeight());
}

void pdfFile::Stat_WriteCentreHeading( FILE *fp, char *txt )
{
}


void pdfFile::Stat_WriteRowStart( FILE *fp, long rgb, long col )
{
	theTableData.cellColorList.push_back( rgb ); // Adds the color to the list of background colors
	numOfRows++;
}
void pdfFile::Stat_WriteRowEnd( FILE *fp )
{
}

void pdfFile::Stat_WriteBottomHeader( FILE *fp, const char *txt, long space )
{
	AddTableText( Translate( txt ), PDF_CENTERED | PDF_HEADING_STYLE );
}

void pdfFile::Stat_WriteHeader( FILE* fp, const char* txt, long space )
{
	const char* txtTranslated=Translate( txt );

	const double& len=timesNRFont->GetStringLenWithSpacing( txtTranslated,
															theTableData.heads.fontSize,
															theTableData.heads.fontStyle
															);

	theTableData.heads.Insert( numOfCols, txtTranslated, len, PDF_CENTERED );

	++numOfCols;
}


void pdfFile::Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title )
{
	tableTitle = title;
}



void pdfFile::Stat_WriteNumberData( FILE *fp, long num )
{
	char	numStr[32];
	FormatLongNum( num, numStr );
	AddTableText( Translate( numStr ), PDF_RIGHTJUSTIFY );
}

void pdfFile::Stat_WriteFractionData( FILE *fp, long num, long num2 )
{
	char numStr[32];
	FormatLongNum( num, numStr );
	std::string str( numStr );
	if ( num != num2 )
	{
		str += " ";
		str += TranslateID(LABEL_OF);
		str += " ";
		FormatLongNum( num2, numStr );
		str += numStr;
	}

	AddTableText( str.c_str(), PDF_RIGHTJUSTIFY );
}


void pdfFile::Stat_WriteFloatData( FILE *fp, double num )
{
	char	numStr[32];
	Double2CStr( num, numStr, 1 );
	FormatNum( numStr, 0 );
	AddTableText( numStr, PDF_RIGHTJUSTIFY );
}




void pdfFile::Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name  )
{
	WriteLine( fp, name );
//	CreateURIObjects( name );
//	AddTableText( name, PDF_LEFTJUSTIFY | PDF_URI );
}


void pdfFile::Stat_WriteLink( FILE *fp, const char *url, const char *name, long num  )
{
	if ( thePDFSettings.Links() )
	{
		CreateURIObjects( name, url );
		AddTableText( name, PDF_LEFTJUSTIFY | PDF_INTERNAL_LINK );
	}
	else
		AddTableText( name, PDF_LEFTJUSTIFY );
}


void pdfFile::Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	// ***************************************************************
	// There is no actual way to turn this on, therefore I doubt
	// that the code has ever been tested in all checks.
	// So rather than setting it to true, I will NOT check in this
	// case - leaving all the other checks returning FALSE!!
	// ***************************************************************
//	if ( thePDFSettings.Links() )
	{
		CreateURIObjects( name, url );
		AddTableText( name, PDF_LEFTJUSTIFY | PDF_URI );
	}
//	else
//		AddTableText( name, PDF_LEFTJUSTIFY );
	theTableData.urlCellBkgrndColorList.push_back( rgb ); // Adds the color to the list of background colors
	theTableData.urlCellBkgrndColorCol = cols;
}

void pdfFile::Stat_WriteLocalDiffFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	if ( thePDFSettings.Links() )
	{
		CreateInternalLinkObjects( name, url ); // Use the url which is a HTML filename as the link...
		AddTableText( name, PDF_LEFTJUSTIFY | PDF_INTERNAL_LINK );
	}
	else
		AddTableText( name, PDF_LEFTJUSTIFY );
	theTableData.urlCellBkgrndColorList.push_back( rgb ); // Adds the color to the list of background colors
	theTableData.urlCellBkgrndColorCol = cols;
}

void pdfFile::Stat_WriteLocalSameFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	if ( thePDFSettings.Links() )
	{
		CreateInternalLinkObjects( name, name ); // Don't use the URL as it is an internal HTML file link, which we don't want... use the "name" instead
		AddTableText( name, PDF_LEFTJUSTIFY | PDF_INTERNAL_LINK );
	}
	else
		AddTableText( name, PDF_LEFTJUSTIFY );
	theTableData.urlCellBkgrndColorList.push_back( rgb ); // Adds the color to the list of background colors
	theTableData.urlCellBkgrndColorCol = cols;
}

void pdfFile::Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	if ( thePDFSettings.Links() )
	{
		CreateURIObjects( name, url );
		AddTableText( name, PDF_RIGHTJUSTIFY | PDF_URI );
	}
	else
		AddTableText( name, PDF_RIGHTJUSTIFY );
}


void pdfFile::Stat_WriteTextTooltip( FILE *fp, short cols, const char *name, const char *tooltip )
{
	//AddTableText( "PDF- HTML TextTooltip - what to do???" );
}

void pdfFile::Stat_WriteText( FILE *fp, short cols, long rgb, const char *name )
{
	// Trev use the color...
	WriteLine( fp, Translate( name ) );
//	AddTableText( name );
}
void pdfFile::Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name )
{
//	Stat_WriteText( fp, cols, rgb, name );
	AddTableText( name, PDF_RIGHTJUSTIFY );
//	WriteLine( fp, name );
}

void pdfFile::DoSearchEngines( VDinfoP VDptr, FILE *fp, StatList *byStat, long list, long sort )
{
	long count;

	Stat_WriteSpace( fp, 2 );
	for( count=0; count< list; count++){
		Stat_WriteAnchor( fp, count );
		Stat_WriteSearchEngine( VDptr, byStat, fp, count, byStat->num, sort );
		Stat_WriteSpace( fp, 2 );
		AddPDFTable( 2 );
	}
}

void
pdfFile::Stat_InsertTable(void)
{
	AddPDFTable( 2 );
}


/* Write - write list to file */
void pdfFile::WriteTable( void *vp, StatList *byStat, FILE *fp,
							   char *filename, char *title, char *heading,
							   short id, long typeID, short liststart, short list, short graph )
{
	// Check if we have a "Stat" object to output
	if ( !byStat )
		return;

	if ( title == NULL ) title = heading;
	if ( heading == NULL ) heading = title;

	// Determine the width of the table
	long tabwidth=GRAPH_WIDTH;
	if( MyPrefStruct.graph_wider )
		tabwidth = (long)(tabwidth*1.5);

	// Get URL
	char stripped_url[MAXURLSIZE];
	stripReferURL( MyPrefStruct.siteurl, stripped_url );

	// Get the Domain address 
	char thishost[MAXURLSIZE];
	VDinfoP	VDptr = (VDinfoP)vp;
	if ( MyPrefStruct.multivhosts == 1 && VDptr->domainName[0] != '-' ) {
		if ( strstr( VDptr->domainName, "http" ) )
			mystrcpy( thishost , VDptr->domainName );
		else
			sprintf( thishost, "http://%s", VDptr->domainName );
	} else
	mystrcpy( thishost , MyPrefStruct.siteurl );

	long num = byStat->num;
	
	// Write unit totals in comments
	sprintf( tempBuff2, "Total Units = %d (bytes=%ld) / %d (%ld)",
		num, sizeof(Statistic)*num, byStat->maxNum, byStat->maxNum*sizeof(Statistic) );
	Stat_WriteComment( fp, tempBuff2 );

	WritePageShadowInit( fp );

	int extras = 0; // = 0 added by TREV as was not being initialised...
	if ( MyPrefStruct.stat_sessionHistory )		// uppercase ABD sessionhistory
		extras = 1;

	char *columnStrPtr[16];
	long columnIDs[16];
	int	colspan=9, cols;
	long hashID;

	GetTableColumnHeadings( columnStrPtr, columnIDs, colspan, cols, hashID, typeID );
	WriteTableHeadings( tabwidth, fp, colspan, list, title, heading, cols, columnIDs, typeID, extras );
	WriteTableData( vp, byStat, fp, filename, title,
					heading, typeID, liststart,list, graph,
					id, cols, colspan, columnIDs, columnStrPtr, thishost	);

	int val = theTableData.data.Columns(); 
	WriteTimeTakenInfo( VDptr, "PDF Table Info: Number of Data Columns", val );
	val = theTableData.data.Rows(); 
	WriteTimeTakenInfo( VDptr, "PDF Table Info: Number of Data    Rows", val );
	int outlineLevel = 1;
	if ( (id >= SUNDAY_PAGE && id <= SATDAY_PAGE) || id == -1 )
		outlineLevel = 2;
	AddPDFTable( outlineLevel );
}


void pdfFile::AddPDFTable( int outlineLevel /*= 1*/ )
{
	if ( theTableData.data.Columns() && theTableData.data.Rows() )
	{
		// Translate anything which is required... (i.e. everything except data)
		std::string continedOnNext = TranslateID( COMM_CONTNEXTPAGE );
		std::string continedFromPrev = TranslateID( COMM_CONTPREVPAGE );
		theTableData.linkTransTitle = Translate( tableTitle.c_str() );
		// if current report has no graph then we'd be missing an outline level within the index, if it 
		// were not for the following code.  This is because pdfFile::WriteGraphStat() actually calls
		// AddOutline() for some reason.  So we need to compensate for the fact that AddOutline() was
		// not called (because WriteGraphStat() was not called) by explicitly calling the function below.
		if( !ShowGraph() )
		{
			std::string tempLinkTransTitle( theTableData.linkTransTitle );
			AddOutline( tempLinkTransTitle, 1 );	// need to pass in a temp string here 'cause of Trevor - just don't ask...
		}
		DrawTable( numOfRows-1, continedOnNext, continedFromPrev, HTMLLinkFilename, outlineLevel );
	}
	ResetTable();
}


void pdfFile::ResetTable()
{
	ResetTableDetails();
	numOfRows = 0;
	numOfCols = 0;
	dataColNum = 0;
	colDataWidths.clear();
	if ( thePDFOutlinesObject )
		thePDFOutlinesObject->ClearCurrLinkObjectList();
}




void pdfFile::AddTableCellMultilineText( const PDFMultiLineCellTextStrList& cellStrList, int justify )
{
	if ( dataColNum == numOfCols )
		dataColNum = 0;


	int maxLen = 0;
	PDFMultiLineCellTextStrList::const_iterator iter( cellStrList.begin() );
	for ( ; iter != cellStrList.end(); iter++ )
	{
		PDFMultiLineCellTextStr multiLineStr = (*iter);
		int len;
		if ( justify & PDF_HEADING_STYLE )
			len = timesNRFont->GetStringLenWithSpacing( multiLineStr.Str().c_str(), theTableData.data.fontSize, theTableData.heads.fontStyle );
		else
			len = timesNRFont->GetStringLenWithSpacing( multiLineStr.Str().c_str(), theTableData.data.fontSize, theTableData.data.fontStyle );
		if ( len > maxLen )
			maxLen = len;
		multiLineStr.SetStrLen( len );
	}

	int a = 0;
	if (!theTableData.InsertData( dataColNum, numOfRows-3, cellStrList, justify ))
		a = 1;

	dataColNum++;
}


void pdfFile::AddTableText( std::string str, int justify )
{
	if ( dataColNum == numOfCols )
		dataColNum = 0;

	int len;
	if ( justify & PDF_HEADING_STYLE )
		len = timesNRFont->GetStringLenWithSpacing( str.c_str(), theTableData.data.fontSize, theTableData.heads.fontStyle );
	else
		len = timesNRFont->GetStringLenWithSpacing( str.c_str(), theTableData.data.fontSize, theTableData.data.fontStyle );

	PDFMultiLineCellTextStrList cellStrList;
	PDFMultiLineCellTextStr temp( str, len );
	cellStrList.push_back( temp );
	AddTableCellMultilineText( cellStrList, justify );

}

void pdfFile ::AddDualTableText( std::string str, int justify, std::string str2, int justify2  )
{
	if ( dataColNum == numOfCols )
		dataColNum = 0;

	int len;
	if ( justify & PDF_HEADING_STYLE )
		len = timesNRFont->GetStringLenWithSpacing( str.c_str(), theTableData.heads.fontSize, theTableData.heads.fontStyle );
	else
		len = timesNRFont->GetStringLenWithSpacing( str.c_str(), theTableData.data.fontSize, theTableData.data.fontStyle );

	int len2;
	if ( justify & PDF_HEADING_STYLE )
		len2 = timesNRFont->GetStringLenWithSpacing( str2.c_str(), theTableData.heads.fontSize, theTableData.heads.fontStyle );
	else
		len2 = timesNRFont->GetStringLenWithSpacing( str2.c_str(), theTableData.data.fontSize, theTableData.data.fontStyle );

	PDFMultiLineCellTextStrList cellStrList;
	PDFMultiLineCellTextStr temp( str, len, str2, len2 );
	cellStrList.push_back( temp );
	AddTableCellMultilineText( cellStrList, justify );
}

void pdfFile::WriteGraphStat( VDinfoP VDptr, StatList *byStat , long id, long typeID, char *filename, char *title, char *header, short sort, short depth, long labelTransId/*=0*/ )
{
	int outlineLevel = 1;
	if ( id >= SUNDAY_PAGE && id <= SATDAY_PAGE )
		outlineLevel = 2;

	if ( id == VHOST_PAGE )
	{
		StandardPlot thePlot;
		thePlot.DrawbyDataStyle( VDptr, byStat , id, typeID, filename, title, header, sort, depth, labelTransId );
	}
	else
	{
		StartNewSectionInReport( false );
		WriteCurrentPage();
		CreateAPageObject();
		PDFGraphSettings thePDFGraphSettings( &MyPrefStruct.pdfAllSetC.pdfGraphSetC );
		std::string linktitle = Translate( title );
		AddOutline( linktitle, outlineLevel );
		float x = SubPageLeftMargin(); 
		float w = SubPageDrawWidth();

		if (id != REGION_PAGE )
		{
			PDFPlot thePlot( VDptr, thePDFSettings, &thePDFGraphSettings, timesNRFont, x, CurrYPos(), w );
			thePlot.DrawbyDataStyle( VDptr, byStat, id, typeID, filename, title, header, sort, depth, labelTransId );
			DrawGraph( thePlot.GraphData(), thePlot.GraphHeight() );
		}
		else
		{
			StandardPlot thePlot;
			thePlot.DrawbyDataStyle( VDptr, byStat, id, typeID, filename, title, header, sort, depth, labelTransId );
			std::string& worldRegionFilename = thePlot.GetWorldRegionJPGFilename();
			DrawImage( worldRegionFilename.c_str() );

			// *************************************************************************************************
			// The image has been added to the PDF now in both the position where it is stored and the reference 
			// so it appears in the corrent position.  So lets delete the file.
			// *************************************************************************************************
			(void)::remove(worldRegionFilename.c_str());

			// hmm, mustn't forget to remove the file history entry created via Fopen() !!!
			RemoveFileHistory(worldRegionFilename.c_str());
		}
		if ( thePDFOutlinesObject)
		{
			thePDFOutlinesObject->LinkToPageString( linktitle, thePDFPageObject->Id() );
			thePDFOutlinesObject->ClearCurrLinkObjectList();
		}
	}
}


void pdfFile::AddPageBanner()
{
	DrawPageBanner( SubPageLeftMargin(), TopMargin() );
}

void pdfFile::AddPDFGraph( std::string graphData, int graphHeight )
{
}

void pdfFile::SummaryTableTitle( FILE *fp, const char *text, long rgb )
{
	FrontPageTableTitle( Translate( text ) );
}

void pdfFile::ProduceSummaryTitle( FILE *fp, int logNum, const char *domainName )
{
	std::string title;
	char		txt[256];

	sprintf( txt, "%s:", TranslateID( SUMM_SERVERLOADSTATS ) );
	if (!txt)
		title = "";
	else
		title += txt;

	SummaryTableTitle( fp, txt, -1 );

	if (MyPrefStruct.head_title){
		if( MyPrefStruct.multivhosts == 0 && logNum >1 ) {
			if ( strlen(MyPrefStruct.siteurl)>7 )
				sprintf( txt, "%s", MyPrefStruct.siteurl );
			else
				sprintf( txt, "%d log files", logNum );
		} else
			sprintf( txt, "%s", domainName );

		title += txt;

		ConvertHTMLCharacterTokens( title );
		DateFixFilename( txt, NULL );
		SetReportTitle( txt );
		SetDocumentTitle( title );
	}
}

/* WriteReportSummary - print summary of total day statistics */
void pdfFile::WriteReportSummary( VDinfoP VDptr, FILE *fp, char *outfile, long logNum )
{
	if ( !REPORT_CHECK_ON(MyPrefStruct.stat_summary) )
		return;

	baseFile::WriteReportSummary( VDptr, fp, outfile, logNum );

	// Write the very first page (the report cover) which can be used to put your own text
	WriteFrontPage( MyPrefStruct.report_title );

	std::string summarytitle = TranslateID(REPORT_SUMMARY);
	WriteOutline( summarytitle, 0, 1 );
	sectionName = currSectionName;
	if ( thePDFOutlinesObject )
	{
		CreateAPageObject();
		LinkOutlineToPage( thePDFPageObject->Id() );
		std::string link = "<LINK0>";
		link += summarytitle;
		thePDFOutlinesObject->LinkToPageString( link, thePDFPageObject->Id() );
		thePDFOutlinesObject->ClearCurrLinkObjectList();
	}

	// Write the summary page
	WriteDemoBanner( VDptr, fp );
	DrawPDFText( summaryPDFTextList );
	WriteCurrentPage();
	CreateAPageObject();
	return;
}

void pdfFile::WriteDemoBanner( VDinfoP VDptr, FILE *fp )
{
	if ( IsDemoReg() )
	{
		displayDemo = true;
		DisplayDemoString();
	}
}

void pdfFile::WriteOutline( std::string outlineTitle, int level /*= 0*/, int forceLink /*= 0*/ )
{
	currSectionName = Translate( outlineTitle.c_str() );
	std::string outlineName = currSectionName;
	AddOutline( outlineName, level, forceLink );
}


const char* pdfFile::Translate( const char *text )
{
	static std::string transTextStr;
	if ( MyPrefStruct.language )
	{
		transTextStr = text;
		timesNRFont->TranslateHTMLSpecialTokens( transTextStr );
		return transTextStr.c_str();
	}
	return text;
}

void pdfFile::SummaryTableSubTitle( FILE *fp, const char *text, int x, int y )
{
	FrontPageTableSubTitle( Translate( text ) );
}


void pdfFile::SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal )
{
	FrontPageTableEntry( Translate( text ), number );
}

void pdfFile::WriteLogProcessingTimeInfo( VDinfoP VDptr, FILE *fp )
{
	char	name[512];

	long temp = VDptr->time2;
	VDptr->time2 = timems() - VDptr->time1;

	if ( VDptr && fp ){
		if ( VDptr->time2 > 0 ){
			sprintf( tempBuff2, "%02d:%02d:%02d.%d, %.0f Files/Min, %.2f Megs/Min",
				(int)VDptr->time2/(60*60),(int)(VDptr->time2/60)%60,(int)(VDptr->time2)%60,(int)(VDptr->time2*10)%10,
				60*(VDptr->totalRequests/VDptr->time2),
				60*((VDptr->totalInDataSize/VDptr->time2)/1048576.0) );

		}

		sprintf( name, "Time taken: %s", tempBuff2 );
	}
	VDptr->time2 = temp;

	OutDebug( name );
}

extern void GetCorrectSiteURL( VDinfoP VDptr, char *host );


void pdfFile::Stat_WriteMeanPath( VDinfoP VDptr, FILE *fp, int depth )
{
	baseFile::Stat_WriteMeanPath( VDptr, fp, depth );

	int val = theTableData.data.Columns(); 
	WriteTimeTakenInfo( VDptr, "PDF Table Info - Number of Data Columns:", val );
	val = theTableData.data.Rows(); 
	WriteTimeTakenInfo( VDptr, "PDF Table Info - Number of Data Rows   :", val );
	AddPDFTable();
}


double pdfFile::GetStringLen( std::string str )
{
	return PDFFile::GetStringLen( str );
}

void pdfFile::PutReportOnNextPage( bool state )
{
	StartNewSectionInReport( state );
}

void pdfFile::ShowingGraph( bool state )
{
	ShowGraph( state );
}

void pdfFile::Stat_WriteDualHeader( FILE *fp, const char *head1, const char *head2 )
{
	Stat_WriteHeader( fp, head1, 1 );
	Stat_WriteHeader( fp, head2, 1 );
}

void pdfFile::Stat_WriteDualText( FILE *fp, char *text1, char *text2 )
{
	AddTableText( text1, PDF_RIGHTJUSTIFY );
	AddTableText( text2, PDF_RIGHTJUSTIFY );
}

void pdfFile::Stat_WriteBoldDual( FILE *fp, const char *txt, const char *txt2 )
{
	Stat_WriteBold( fp, txt );
	Stat_WriteBold( fp, txt2 );
}

void pdfFile::Stat_WritingHelpCard( bool flag )
{
	if ( flag )
		AdjustTableSettingsForSingleCellFlowingText(); // Crappy names... but it means something...
	else
		AdjustTableSettingsBackAfterSingleCellFlowingText(); // Crappy names... but it means something...
}


void pdfFile::Stat_WriteHelpCard(FILE *fp, const class CQHelpCard& rCQHelpCard, bool bTableOn, bool bGraphOn)
{
	PDFFile::WriteHelpCard(fp,rCQHelpCard,bTableOn,bGraphOn);
}
