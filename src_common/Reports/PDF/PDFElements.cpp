
#include "FWA.h"

#include <math.h>
#include "PDFBase.h"

#include "PDFElements.h"
#include "PDFFont.h"

const char *PDF_BLANK = "";

#include "config.h"
#include "Translate.h"			// for TranslateID
#include "report.h"
#include "PDFFile.h"
#include "GlobalPaths.h"		// for MAXFILENAMESSIZE
#include <ctype.h>				// for isspace()

bool showingColumnHeadings = true;

//
// PDFTable class functions
//

PDFTable::PDFTable( PDFTableData *theTableDataP, int numOfRowsP, std::string continuedOnNextP, std::string continuedFromPrevP, 
				   PDFSettings *thePDFSettingsP, PDFTableSettings *tableSetP, PDFCharScaleFont *timesNRFontP, int currPagePosP, int subPageNumP )
{
	thePDFSettings = thePDFSettingsP;
	tableSet = tableSetP;
	numOfRows = numOfRowsP;
	continuedOnNext = continuedOnNextP;
	continuedFromPrev = continuedFromPrevP;
	currPagePos = currPagePosP;
	SubPageNum( subPageNumP );
	timesNRFont = timesNRFontP;
	theTableData = theTableDataP;
	tabSepTextPos = 0;
	lineWidth = (float)0.01;
	extraTextLen = 0;
	currFillCol = -1;
	lastTableHeight = 0;
	currLineRowNum = 0;
	currTextRowNum = 0;
	lineDataExtraRows = 0;
	textDataExtraRows = 0;
	continueFromPrevPage = false;
	tableTitleBackColor = PDF_INVALID_COLOR;
	tableColumnHeadingsBackColor = PDF_INVALID_COLOR;
//	tableTextData.reserve(5*1024);
	tableLineData.reserve(5*1024);
	PDFError = new PDFCellTextStr( "PDFError", timesNRFont->GetStringLen( "PDFError", theTableData->titleFontSize, theTableData->titleFontStyle), PDF_LEFTJUSTIFY );

	// Set the table width
	tableWidth = theTableData->ColumnsWidth();
}

PDFTable::~PDFTable()
{
	delete PDFError;

}

static const int PDF_BACKGRND_WHITE = 255;
static const int COLUMN_HEADINGS_ROW = 0;


void PDFTable::DisplayTitle( TableTextPosInfo& posInfo, std::string title, PDFBodyObjectsList& annotsInternalLinksList, PDFPageObject *thePDFPageObject, std::string HTMLLinkFilename )
{
	// Title Text
	tableLineData += timesNRFont->FontChangeStr( theTableData->titleFontStyle, theTableData->titleFontSize, theTableData->titleFontColor );
	posInfo.currXPos += tableSet->textCellXIndent;
	float rowHeight = timesNRFont->FontMultiLineExtraLineHeight( theTableData->titleFontSize );
	posInfo.currYPos -= rowHeight;
	//float titleYPos = (theTableData->fontSize + timesNRFont->FontSpacing()) / posInfo.moveTextUpFromLine;
	float titleYPos = 0;//timesNRFont->FontSpacing()/2;
	posInfo.currYPos += titleYPos;
	posInfo.currXOffset += posInfo.currXPos - posInfo.lastXPos;
	posInfo.currYOffset += posInfo.currYPos - posInfo.lastYPos;
	posInfo.lastXPos = posInfo.currXPos;
	posInfo.lastYPos = posInfo.currYPos;
	PositionNextText( posInfo.currXOffset, posInfo.currYOffset );

	if (HTMLLinkFilename != "")
	{
		short found = 0;
		for ( PDFBodyObjectsList::iterator iter = annotsInternalLinksList.begin(); iter != annotsInternalLinksList.end() && !found; iter++ )
		{
			PDFAnnotObject *aPDFAnnotObject = (PDFAnnotObject*)(*iter);
			std::string link = aPDFAnnotObject->URL();
			if ( HTMLLinkFilename == link || title == link )
			{
				std::string dest = "[ ";
				dest += thePDFPageObject->GetIdStr();
				dest += " 0 R /XYZ null null null ]";
				aPDFAnnotObject->Dest( dest );
				found = 1;
				// Now that we've linked to a page, remove the link item
				annotsInternalLinksList.remove( (*iter) );
			}
		}
	}

//	tableLineData += timesNRFont->FontChangeStr( theTableData->titleFontStyle, theTableData->titleFontSize, theTableData->titleFontColor );
	tableLineData += "(";
	tableLineData += timesNRFont->MakePDFString( title );
	tableLineData += ")Tj\r";

	// Move the current position back to it's original position
	posInfo.currXPos -= tableSet->textCellXIndent;
//	posInfo.movedY = posInfo.currYPos;
	posInfo.currYPos -= titleYPos;
	posInfo.currYPos -= (timesNRFont->FontHeight( theTableData->titleFontSize ) - rowHeight);
}

void PDFTable::CreateLinkToText( float x, float y, PDFPageObject *thePDFPageObject, PDFBodyObjectsList *listPtr, PDFCellTextStr *textStr )
{
	short found = 0;
	for ( PDFBodyObjectsList::iterator iter = listPtr->begin(); iter != listPtr->end() && !found; iter++ )
	{
		PDFAnnotObject *aPDFAnnotObject = (PDFAnnotObject*)(*iter);
		std::string text = aPDFAnnotObject->Text();
		if ( textStr->Str() == text )
		{
//			int x,y;
			std::string pos = "[ ";	// % [ L T R B ]
//			x = SubPageLeftMargin() - posInfo.movedX + centeredXPos;
			pos += floatToStr( x );
			pos += " ";
//			y = posInfo.movedY + theTableData->data.fontSize;
			pos += floatToStr( y );
			pos += " ";
			x += textStr->StrLen();
			pos += floatToStr( x );
			pos += " ";
			y -= theTableData->data.fontSize;
			pos += floatToStr( y );
			pos += " ]";
			aPDFAnnotObject->Position( pos );
			
			aPDFAnnotObject;

			// Add this URI to the page
			thePDFPageObject->AddAnnote( aPDFAnnotObject );
			found = 1;
			if ( textStr->Justify() & PDF_URI )
			{
				listPtr->remove( (*iter) );
				tableLineData += timesNRFont->FontChangeStr( timesNRFont->CurrentStyle(), timesNRFont->FontSize(), PDF_BLUE );
			}
			else if ( textStr->Justify() & PDF_INTERNAL_LINK )
			{
				tableLineData += timesNRFont->FontChangeStr( timesNRFont->CurrentStyle(), timesNRFont->FontSize(), PDF_BLUE );
			}
		}
	}
}


void PDFTable::FormatTableData()
{
	if ( !tableSet->textWrap )
		return;

	//theTableData->heads;
//	StrList::Iterator iter;
	//for ( theTableData->heads; i<theTableData->heads.
	/*PDFMultiLineCellTextStrList cellStrList;
	for ( lp=0; lp<count; lp++)
	{
		PDFMultiLineCellTextStr temp( displayName, timesNRFont->GetStringLen( displayName, thePDFTableSettings.dataSize, thePDFTableSettings.dataStyle ) );
		cellStrList.push_back( temp );
	}
	AddTableCellMultilineText( cellStrList, PDF_LEFTJUSTIFY );*/
}



std::string PDFTable::DrawTableLines( float& yPos )
{
	if (continueFromPrevPage)
		yPos += FontHeight(); // Allow for the line of text "continued from previous page" by moving yPos down 1 line
	
	float pageVerticalRemaining = PageHeight() - BottomMargin() - yPos;// - (lineDataExtraRows * timesNRFont->FontHeight());
	float currPageLeft = pageVerticalRemaining;

	curveData = PDF_BLANK;
//	tableLineData = PDF_BLANK;

	TableLinePosInfo linePosInfo( SubPageLeftMargin(), PDFMakeYCoord( yPos ) );

	// Useful PDF comments
	if ( thePDFSettings->CommentsOn() )
		tableLineData += "% Report Table\r";

	// PDF line thickness (0.05)... dunno about the other stuff??
	tableLineData += "0 G\r";
	tableLineData += "0 J 0 j 0.005 w 10 M []0 d\r";

	// Draw the table title and it's surrounding lines & background and Keep track of any writing of table lines
	lastTableHeight = DrawTableTitleBackgroundColor( linePosInfo.currXPos, linePosInfo.currYPos );

	// Move the Y position down a line, this was to allow for the drawing of the heading box (done above)
	linePosInfo.currYPos -= lastTableHeight;

//	int totalCalRowsHeight = theTableData->data.TableLines();

	// Draw horizontal lines and background colors for each row
	float dataSectionTableHeight = 0;
	DrawHorizontalLinesAndBackgrounds( &linePosInfo, dataSectionTableHeight, currPageLeft );

	// Move the Y position down a line to allow for the drawing of the heading box (done later when width is known)
	linePosInfo.currYPos = PDFMakeYCoord( yPos );//thePDFSettings.PageTopDrawPos() - yPos;// - yOffset;
	linePosInfo.currYPos -= timesNRFont->FontHeight( theTableData->titleFontSize );

	lastTableHeight += dataSectionTableHeight;

	// Draw vertical lines
	DrawVerticalLines( linePosInfo.currXPos, linePosInfo.currYPos, dataSectionTableHeight, tableSet->DataSize() );

	curveData += "S\r";
	tableLineData += curveData;

	return tableLineData;
}

int PDFTable::TableHeadingColor( intList& cellColorList )
{
	if (tableTitleBackColor != PDF_INVALID_COLOR)
		return tableTitleBackColor;

	intList::iterator colorIter = cellColorList.begin();
	if (colorIter != cellColorList.end())
	{
		tableTitleBackColor = (*colorIter);
		theTableData->cellColorList.pop_front();
		return tableTitleBackColor;
	}
	return PDF_BACKGRND_WHITE;
}

float PDFTable::DrawTableTitleBackgroundColor( float x, float y )
{
	// Draw the top of the table where the heading goes, now that we know the width
	tableTitleBackColor = TableHeadingColor( theTableData->cellColorList );

	// Draw the background color for the table title
	float height = timesNRFont->FontHeight( theTableData->titleFontSize );
	PlotFilledRect( x, y-height, tableWidth, height, tableTitleBackColor );

	// Draw the EXTRA lines for the table heading
	if ( !thePDFSettings->CurvedTables() )
	{
		PlotLine( x, y-height, x, y );
		PlotLineTo( x+tableWidth, y );
		PlotLineTo( x+tableWidth, y-height );
		tableLineData += "S\r";
	}
	else
	{
		PlotLine( x, y-height, x, y-(height/2) );
		PlotLine( x+(height/2), y, x+tableWidth-(height/2), y );
		PlotLine( x+tableWidth, y-(height/2), x+tableWidth, y-height );
		tableLineData += "S\r";
		PDFMarker aPDFMarker;
		aPDFMarker.DrawQuarterCircle( x, y-(height/2), x+(height/2), y );
		aPDFMarker.DrawQuarterCircle( x+tableWidth-(height/2), y, x+tableWidth, y-(height/2) );
		curveData += aPDFMarker.graphData;
	}

	return height;
}

void PDFTable::DrawHorizontalLinesAndBackgrounds( TableLinePosInfo *linePosInfo, float& dataSectionTableHeight, float& currPageLeft  )
{
	float currRowHeight;
//	int tableRowBeingDrawn;
//	int rowsCurrentlyAdded = 0;
	int extraLinesForRow; // This indicates the current row is multi-lined, so we have to allow extra height

	headingRowNum = currLineRowNum;
	for( ; currLineRowNum < (numOfRows + lineDataExtraRows) ; currLineRowNum++ )
	{
		if ( currLineRowNum == headingRowNum && showingColumnHeadings ) // Column Headings
		{
			lineDataExtraRows++;
			extraLinesForRow = theTableData->heads.RowCellLines()-1;
			currRowHeight = timesNRFont->FontHeight( theTableData->heads.fontSize ) // First line
						  //+ (extraLinesForRow * timesNRFont->FontMultiLineExtraLineHeight( theTableData->heads.fontSize )); // All other lines in a multi-line cell
						  + (extraLinesForRow * theTableData->heads.fontSize); // All other lines in a multi-line cell
		}
		else // Data
		{
			extraLinesForRow = theTableData->data.RowCellLines( currLineRowNum-lineDataExtraRows )-1; 
			currRowHeight = timesNRFont->FontHeight( theTableData->data.fontSize )
						  + (extraLinesForRow * theTableData->data.fontSize);
		}

		currPageLeft -= currRowHeight;
		if ( currPageLeft < BottomMargin() )
			break; 

		// Draw the background color
		dataSectionTableHeight += currRowHeight;
		if ( currLineRowNum == headingRowNum ) // Column Headings
		{
//			dataSectionTableHeight += currRowHeight;
			//if (tableRowBeingDrawn == 0)
			//	rowsCurrentlyAdded++;

			// Get the color for the column headings
			DrawHeadingRowBackGroundColor( linePosInfo->currXPos, linePosInfo->currYPos, currRowHeight );
		}
		else // Data
		{
//			dataSectionTableHeight += currRowHeight;
//			rowsCurrentlyAdded++;
			DrawDataRowBackGroundColor( linePosInfo->currXPos, linePosInfo->currYPos, currRowHeight );
			DrawCellBackGroundColor( linePosInfo->currXPos, linePosInfo->currYPos, currRowHeight );
		}

		// Draw the horizontal line
		PlotDrawLine( linePosInfo->currXPos, linePosInfo->currYPos, linePosInfo->currXPos + tableWidth, linePosInfo->currYPos );
		linePosInfo->currYPos -= currRowHeight;

	}
	// Draw the last horizontal line
	if ( !thePDFSettings->CurvedTables() )
		PlotDrawLine( linePosInfo->currXPos, linePosInfo->currYPos, linePosInfo->currXPos + tableWidth, linePosInfo->currYPos );
	else
	{
		PlotDrawLine( linePosInfo->currXPos+(currRowHeight/2), linePosInfo->currYPos, linePosInfo->currXPos-(currRowHeight/2) + tableWidth, linePosInfo->currYPos );
		PDFMarker aPDFMarker;
		// Right side
		aPDFMarker.DrawQuarterCircle( linePosInfo->currXPos+(currRowHeight/2), linePosInfo->currYPos,
										linePosInfo->currXPos, linePosInfo->currYPos+(currRowHeight/2) );
		// Left side
		aPDFMarker.DrawQuarterCircle( linePosInfo->currXPos+tableWidth, linePosInfo->currYPos+(currRowHeight/2),
										linePosInfo->currXPos+tableWidth-(currRowHeight/2), linePosInfo->currYPos );
		curveData += aPDFMarker.graphData;
	}
}

void PDFTable::DrawHeadingRowBackGroundColor( float x, float y, float height )
{
	long columnHeadingsColor = PDF_BACKGRND_WHITE;
	intList::iterator colorIter = theTableData->cellColorList.begin();
	if (colorIter != theTableData->cellColorList.end())
	{
		if (tableColumnHeadingsBackColor == PDF_INVALID_COLOR)
		{
			columnHeadingsColor = (*colorIter);
			tableColumnHeadingsBackColor = columnHeadingsColor;		
			theTableData->cellColorList.pop_front();
		}
		else
			columnHeadingsColor = tableColumnHeadingsBackColor;

		// Draw the background color for the column headings
		PlotFilledRect( x, y-height, tableWidth, height, columnHeadingsColor );
	}
}


void PDFTable::DrawDataRowBackGroundColor( float x, float y, float height )
{
	// Get the color for the current row
	long rowColor = PDF_BACKGRND_WHITE;
	intList::iterator colorIter = theTableData->cellColorList.begin();
	if (colorIter != theTableData->cellColorList.end())
	{
		// Draw the background color for the row
		rowColor = (*colorIter);
		PlotFilledRect( x, y-height, tableWidth, height, rowColor );
		theTableData->cellColorList.pop_front(); 
	}
}


void PDFTable::DrawCellBackGroundColor( float x, float y, float height )
{
	// Set the background color for the cell text
	long cellColor = PDF_BACKGRND_WHITE;
	IntIter colorIter = theTableData->urlCellBkgrndColorList.begin();
	//intList::iterator colorIter = theTableData->urlCellBkgrndColorList.begin();
	if (colorIter != theTableData->urlCellBkgrndColorList.end())
	{
		cellColor = (*colorIter);
		int i = 0;
		float urlXPos = x;
		PDFTableData *p = theTableData;
		for( TableColWidthIter colWidthsIter = p->FirstColumnPtr(); colWidthsIter != p->BeyondLastColumnPtr(); p->NextColumnPtr( colWidthsIter ), i++ )
		{
			if ( i == theTableData->urlCellBkgrndColorCol )
			{
				// Draw the background color for the cell
				PlotFilledRect( urlXPos+lineWidth, y-height+lineWidth, (*colWidthsIter).ColWidth()-lineWidth*2, height-lineWidth*2, cellColor );
			}
			urlXPos += (*colWidthsIter).ColWidth();

		}
		theTableData->urlCellBkgrndColorList.pop_front(); 
	}
}


void PDFTable::DrawVerticalLines( float x, float y, float height, int curveSize )
{
	TableColWidthIter iter;
	PDFTableData *p = theTableData;
	int size = p->Columns();
	int i = 0;
	for( iter = p->FirstColumnPtr(); iter != p->BeyondLastColumnPtr(); p->NextColumnPtr( iter ), i++ )
	{
		// Draw the vertical line
		if ( !thePDFSettings->CurvedTables() )
			PlotLine( x, y, x, y-height );
		else
		{
			if ( i == 0 )
				PlotLine( x, y, x, y-height );
			else
				PlotLine( x, y, x, y-height+curveSize );
		}
		x += (*iter).ColWidth();
	}
	// Draw the last vertical line
	if ( !thePDFSettings->CurvedTables() )
		PlotDrawLine( x, y, x, y-height );
	else
		PlotDrawLine( x, y, x, y-height+curveSize );
}

static void
WordWrapHelpText(CQHelpCardText& rCQHelpCardText, double dWidth, PDFCharScaleFont* pFont)	// non const.
{
	int	style	= pFont->CurrentStyle();
	int	size	= pFont->FontSize();

	CQHelpCardText::iterator	it;
	std::string					str;
	bool	bModified(false);	// if we modify the list then delete the 'too-large' string.

	for (it=rCQHelpCardText.begin(); it!=rCQHelpCardText.end(); )
	{
		CQHelpCardTextFragment&	rFrag = *it;

		str += rFrag.second;
		if (!(rFrag.first & HELPTEXT_FORMATMASK_NEWLINE))
		{	
			bModified = true;
			it = rCQHelpCardText.erase(it);
			continue;
		}

		double	dTotalLength(0.0);
		const char*	c1 = str.c_str();
		const char* c2 = str.c_str();
		for (; *c2; ++c2)
		{
			double	dSize = pFont->GetCharWidth(*c2, size, style);
			if (dTotalLength + dSize > dWidth)
			{
				bModified = true;	// don't forget to delete the long string fragment.

				// This is here to watch out for words longer that the width!
				const char* c3 = c2;
				while (!isspace(*c3) && c3 != c1)	// Word wrap!
					--c3;
				while (isspace(*c3))
					++c3;
				// This is how we deal with a single word that is longer than the width
				// That is we chop the word at the wrapping point.
				if (c3 != c1)
					c2 = c3;
				rCQHelpCardText.insert(it,CQHelpCardTextFragment(it->first | HELPTEXT_FORMATMASK_NEWLINE, std::string(c1,c2-c1)));
				c1 = c2;
				dTotalLength = 0.0;
			}
			else
			{
				dTotalLength += dSize;
			}
		}

		if (bModified)
		{
			bModified = false;
			rCQHelpCardText.insert(it,CQHelpCardTextFragment(it->first | HELPTEXT_FORMATMASK_NEWLINE, std::string(c1,c2-c1)));
			it = rCQHelpCardText.erase(it);
		}
		else
		{
			++it;
		}

		str = "";
	}

	if (str.size())
	{
		rCQHelpCardText.push_back(CQHelpCardTextFragment(HELPTEXT_FORMATMASK_NEWLINE, str));
	}
}

void	PDFTable::CreateHelpCard(class PDFFile* pPDFFile, const CQHelpCard& rCQHelpCard, double dWidth, bool bTableOn, bool bGraphOn)
{
	const size_t	DEMO_STRING_SIZE	= (pPDFFile->displayDemo?(40+20):0);
	// ***********************************************************************************************
	// These are const variables as I just want to cache the values.
	// ***********************************************************************************************
	const double	dXPos			= SubPageLeftMargin();
	const double	dTextOffset_Y	= ((theTableData->titleFontSize + timesNRFont->FontSpacing()) / 5);
	const double	dTextOffset_X	= tableSet->textCellXIndent;
	const float		fTitleHeight	= timesNRFont->FontHeight( theTableData->titleFontSize );
	const float		fLineHeight		= timesNRFont->FontHeight( theTableData->font->FontSize() );
	const size_t	nLineTollerance	= 3;

	double			dIconHeight		= 32;
	double			dIconWidth		= 32;
	float			fHeight;
	double			dYPos			= PageHeight() - pPDFFile->CurrYPos();
	double			dTotalHeight;

	// ***********************************************************************************************
	// Ensure the title background colour, is accurate.
	// ***********************************************************************************************
	tableTitleBackColor		= RGBtableTitle;

	// ***********************************************************************************************
	// Create a copy of the HelpCardText objects that contain the content of the cells.
	// ***********************************************************************************************
	CQHelpCardText	hpText_Overview;
	CQHelpCardText	hpText_Graph;
	CQHelpCardText	hpText_Table;
	CQHelpCardText	hpText_Title;

	hpText_Overview		= rCQHelpCard.GetOverview();
	if (bGraphOn)
		hpText_Graph	= rCQHelpCard.GetGraph();
	if (bTableOn)
		hpText_Table	= rCQHelpCard.GetTable();
	hpText_Title.load(rCQHelpCard.GetTitle());


	// *************************************************************************************
	// Ensure that word-wrapping will occur for this text, in this width, using this font.
	// *************************************************************************************
	timesNRFont->FontChangeStr( theTableData->data.fontStyle, theTableData->data.fontSize, theTableData->data.fontColor );
	WordWrapHelpText(hpText_Overview,	dWidth-(2*dTextOffset_X)-dIconWidth,	timesNRFont);
	WordWrapHelpText(hpText_Graph,		dWidth-(2*dTextOffset_X)-dIconWidth,	timesNRFont);
	WordWrapHelpText(hpText_Table,		dWidth-(2*dTextOffset_X)-dIconWidth,	timesNRFont);
	// And the Title which has a different font (& size)
	timesNRFont->FontChangeStr( theTableData->titleFontStyle, theTableData->titleFontSize, theTableData->titleFontColor );
	WordWrapHelpText(hpText_Title,		dWidth-(2*dTextOffset_X),				timesNRFont);


	// *************************************************************************************
	// Calculate the Total height of the HelpCard.
	// *************************************************************************************
	{
		double		dOverviewHeight	= hpText_Overview.size()*fLineHeight;
		double		dGraphHeight	= hpText_Graph.size()*fLineHeight;
		double		dTableHeight	= hpText_Table.size()*fLineHeight;
		double		dTitleHeight	= hpText_Title.size()*fTitleHeight;

		if (dGraphHeight == 0.0)
			bGraphOn = false;
		if (dTableHeight == 0.0)
			bTableOn = false;

		if (dOverviewHeight < dIconHeight)
			dOverviewHeight = dIconHeight;
		if (bGraphOn && dGraphHeight < dIconHeight)
			dGraphHeight = dIconHeight;
		if (bTableOn && dTableHeight < dIconHeight)
			dTableHeight = dIconHeight;

		dTotalHeight	= dTitleHeight		+ dTextOffset_Y
						+ dOverviewHeight	+ dTextOffset_Y
						+ dGraphHeight		+ dTextOffset_Y
						+ dTableHeight		+ dTextOffset_Y
						;
	}


	// *************************************************************************************
	// If the HelpCard will Not fit in the remaining space AND
	//	the HelpCard WILL fit in a single page then
	// Start a new page.
	// *************************************************************************************
	if ( (dTotalHeight > (dYPos - (dIconHeight+fLineHeight)) ) &&
		 (dTotalHeight < (PageHeight() ) ) )
	{
		// *************************************************************************************
		// Write the page to the file.
		// *************************************************************************************
		pPDFFile->PageDataAdd(tableLineData);				
		tableLineData = "";
		pPDFFile->WriteCurrentPage();
		pPDFFile->CreateAPageObject();
		SubPageNum(SubPageNum() + 1);			// Start a new page
		
		// *************************************************************************************
		// Reset the page data as we are now on a new page.
		// *************************************************************************************
		dYPos		= PageHeight() - pPDFFile->CurrYPos();		// Reset the vertical position.
		dYPos		-= DEMO_STRING_SIZE;
		pPDFFile->CurrYPos(dYPos);								// (and tell the Page about it)
	}

	// *************************************************************************************
	// Ensure the corrent colours & probably other things also.
	// *************************************************************************************
	tableLineData += "0 G\r0 J 0 j 0.005 w 10 M []0 d\r";

	// *******************************************************
	// Fill in the background of the Title row.
	// *******************************************************
	PlotFilledRect( dXPos, dYPos-fTitleHeight, dWidth, fTitleHeight, tableTitleBackColor );

	// *******************************************************
	// Write the Top Border
	// *******************************************************
	PlotLine( dXPos,		dYPos,				dXPos+dWidth,	dYPos);					// Line at Top.
	tableLineData += "S\r";


	// ***********************************************************************************************
	// Instanciate an array of pointers to the HelpCardText objects so that we can put the code
	// in a loop.
	// ***********************************************************************************************
	CQHelpCardText*		apCQHelpCardText[]	= { &hpText_Title, &hpText_Overview, &hpText_Graph, &hpText_Table };
	PDFImageObjectList*	apImages[]			= { 0, pPDFFile->m_imageOverview, pPDFFile->m_imageGraph, pPDFFile->m_imageTable };
	
	// ***********************************************************************************************
	// Turn off cells that are not required.
	// ***********************************************************************************************
	if (!bGraphOn || !hpText_Graph.size())
		apCQHelpCardText[2] = 0;
	if (!bTableOn || !hpText_Table.size())
		apCQHelpCardText[3] = 0;

	// *******************************************************
	// For each of the three types of HelpCard ...
	// *******************************************************
	for(int i=0; i<(sizeof(apCQHelpCardText)/sizeof(&hpText_Overview)); ++i)
	{
		// *************************************************************************************
		// Skip disabled cells.
		// *************************************************************************************
		if (!apCQHelpCardText[i])
			continue;

		// *************************************************************************************
		// Remember where the top of this cell is so we can write the vertical bars.
		// *************************************************************************************
		double		dTopOfCell = dYPos;

		// *************************************************************************************
		// Are we too close to the bottom of the page to start a new cell (with Icon)
		// *************************************************************************************
		if (dYPos < (32+fLineHeight))
		{
				// *************************************************************************************
				// Write the page to the file.
				// *************************************************************************************
				pPDFFile->PageDataAdd(tableLineData);				
				tableLineData = "";
				pPDFFile->WriteCurrentPage();
				pPDFFile->CreateAPageObject();
				SubPageNum(SubPageNum() + 1);			// Start a new page
				
				// *************************************************************************************
				// Reset the page data as we are now on a new page.
				// *************************************************************************************
				dYPos		= PageHeight() - pPDFFile->CurrYPos();		// Reset the vertical position.
				dYPos		-= DEMO_STRING_SIZE;
				pPDFFile->CurrYPos(dYPos);								// (and tell the Page about it)

				// *************************************************************************************
				// Set the Font for the new page.
				// *************************************************************************************
				timesNRFont->ResetFont();
				tableLineData += timesNRFont->FontChangeStr( theTableData->data.fontStyle, theTableData->data.fontSize, theTableData->data.fontColor );

				// *************************************************************************************
				// Continued from Previous page
				// *************************************************************************************
				tableLineData += "BT\r";
				tableLineData += drawCommands;
				dYPos	-= fHeight;
				PositionNextText( dXPos+dTextOffset_X, dYPos+dTextOffset_Y );	// This is the 1st positioning therefore we can use the explicit values.
				tableLineData += "(";
				tableLineData += TranslateID( COMM_CONTPREVPAGE );
				tableLineData += ")Tj\rET\r";
		
				// *************************************************************************************
				// Line at Top. (continued cell on next page!)
				// *************************************************************************************
				PlotLine(	dXPos, dYPos, dXPos+dWidth, dYPos);
				// *************************************************************************************
				// End the Lines Section.
				// *************************************************************************************
				tableLineData += "S\r";

				// *************************************************************************************
				// Start a new text section.
				// *************************************************************************************
				tableLineData += "BT\r";
				tableLineData += drawCommands;
				PositionNextText( dXPos+dTextOffset_X+dIconWidth+dTextOffset_X, dYPos+dTextOffset_Y );	// This is the 1st positioning therefore we can use the explicit values.

			// *************************************************************************************
			// remember this pos so we can draw the Verticals later.
			// *************************************************************************************
			dTopOfCell	= dYPos;
		}

		timesNRFont->ResetFont();
		
		// *************************************************************************************
		// Set the appropriate font & Line Height, (Title is different to the others)
		// *************************************************************************************
		std::string	strFontChange;
		if	(apCQHelpCardText[i]==&hpText_Title)
		{
			strFontChange	= timesNRFont->FontChangeStr( theTableData->titleFontStyle, theTableData->titleFontSize, theTableData->titleFontColor );
			fHeight			= fTitleHeight;
			dIconHeight		=
			dIconWidth		= 0.0;
		}
		else
		{
			// *************************************************************************************
			// If there is an Icon for this cell then sets its position and set the icon width.
			// *************************************************************************************
			if (apImages[i])
			{
				dIconHeight	=
				dIconWidth	= 32;

				pPDFFile->PositionImage( apImages[i]->imagePosData, pPDFFile->SubPageLeftMargin()+dTextOffset_X, PageHeight()-dYPos+dTextOffset_Y, dIconWidth, dIconHeight );
			}

			strFontChange	= timesNRFont->FontChangeStr( theTableData->data.fontStyle, theTableData->data.fontSize, theTableData->data.fontColor );
			fHeight			= fLineHeight;
		}

		// *******************************************************
		// Start a Text section.
		// *******************************************************
		tableLineData += "BT\r";
		tableLineData += drawCommands;
		PositionNextText( dXPos+dTextOffset_X+dIconWidth+dTextOffset_X, dYPos+dTextOffset_Y );	// This is the 1st positioning therefore we can use the explicit values.

		tableLineData	+= strFontChange;


		// *************************************************************************************
		// for each line in the cell
		// *************************************************************************************
		CQHelpCardText::const_iterator	it;
		for (it=apCQHelpCardText[i]->begin(); it!=apCQHelpCardText[i]->end(); ++it)
		{
			// *************************************************************************************
			// We have a HelpCard Text Fragment.
			// *************************************************************************************
			const CQHelpCardTextFragment&	rFrag = *it;

			// *************************************************************************************
			// If we have reached the each of the page (or within a 'tollerance' of it)
			// then we need to close the current text section, draw the box and write the page.
			// Then we need to do all the things to start a new page.
			// *************************************************************************************
			if (dYPos < (nLineTollerance*fHeight))
			{
				// *************************************************************************************
				// End the Text Section.
				// *************************************************************************************
				tableLineData += "ET\r";											

				if ( (dTopOfCell-dYPos) < dIconHeight)
					dYPos = dTopOfCell - dIconHeight;

				// *************************************************************************************
				// Draw the box around the cell (minus the top line as it is already there)
				// *************************************************************************************
				PlotLine( dXPos, dYPos, dXPos, dTopOfCell);					// Left Vertical Line.
				PlotLine( dXPos+dWidth, dYPos, dXPos+dWidth, dTopOfCell);	// Right Vertical Line.
				PlotLine( dXPos, dYPos, dXPos+dWidth, dYPos);				// Line at Bottom.
				tableLineData += "S\r";										// End the Lines Section

				// *************************************************************************************
				// Continued on next page
				// *************************************************************************************
				timesNRFont->ResetFont();
				tableLineData += timesNRFont->FontChangeStr( theTableData->data.fontStyle, theTableData->data.fontSize, theTableData->data.fontColor );

				tableLineData += "BT\r";
				tableLineData += drawCommands;
				dYPos	-= fHeight;
				PositionNextText( dXPos+dTextOffset_X, dYPos+dTextOffset_Y );	// This is the 1st positioning therefore we can use the explicit values.
				tableLineData += "(";
				tableLineData += TranslateID( COMM_CONTNEXTPAGE );
				tableLineData += ")Tj\rET\r";

				// *************************************************************************************
				// Write the page to the file.
				// *************************************************************************************
				pPDFFile->PageDataAdd(tableLineData);				
				tableLineData = "";
				pPDFFile->WriteCurrentPage();
				pPDFFile->CreateAPageObject();
				SubPageNum(SubPageNum() + 1);			// Start a new page
				
				// *************************************************************************************
				// Reset the page data as we are now on a new page.
				// *************************************************************************************
				dYPos		= PageHeight() - pPDFFile->CurrYPos();		// Reset the vertical position.
				dYPos		-= DEMO_STRING_SIZE;
				pPDFFile->CurrYPos(dYPos);								// (and tell the Page about it)

				// *************************************************************************************
				// Set the Font for the new page.
				// *************************************************************************************
				timesNRFont->ResetFont();
				tableLineData += timesNRFont->FontChangeStr( theTableData->data.fontStyle, theTableData->data.fontSize, theTableData->data.fontColor );

				// *************************************************************************************
				// Continued from Previous page
				// *************************************************************************************
				tableLineData += "BT\r";
				tableLineData += drawCommands;
				dYPos	-= fHeight;
				PositionNextText( dXPos+dTextOffset_X, dYPos+dTextOffset_Y );	// This is the 1st positioning therefore we can use the explicit values.
				tableLineData += "(";
				tableLineData += TranslateID( COMM_CONTPREVPAGE );
				tableLineData += ")Tj\rET\r";

				// *************************************************************************************
				// Line at Top. (continued cell on next page!)
				// *************************************************************************************
				PlotLine(	dXPos, dYPos, dXPos+dWidth, dYPos);
				// *************************************************************************************
				// End the Lines Section.
				// *************************************************************************************
				tableLineData += "S\r";

				// *************************************************************************************
				// Start a new text section.
				// *************************************************************************************
				tableLineData += "BT\r";
				tableLineData += drawCommands;
				PositionNextText( dXPos+dTextOffset_X+dIconWidth+dTextOffset_X, dYPos+dTextOffset_Y );	// This is the 1st positioning therefore we can use the explicit values.

				// *************************************************************************************
				// remember this pos so we can draw the Verticals later.
				// *************************************************************************************
				dTopOfCell	= dYPos;
			}

			// *************************************************************************************
			// This is a new line, so move the Vertical position down one line.
			// *************************************************************************************
			dYPos	-= fHeight;
			PositionNextText( 0.0, -fHeight );

			// *************************************************************************************
			// Write the text.
			// *************************************************************************************
			tableLineData += "(";
			tableLineData += timesNRFont->MakePDFString( rFrag.second.c_str() );
			tableLineData += ")Tj\r";
		}


		// *************************************************************************************
		// Make sure there is space for the icon.
		// *************************************************************************************
		if (apCQHelpCardText[i]->size()*fHeight < dIconHeight)
		{
			dYPos -= (dIconHeight - apCQHelpCardText[i]->size()*fHeight);
			dYPos -= dTextOffset_Y;
		}

		// *************************************************************************************
		// Close the text section.
		// *************************************************************************************
		tableLineData += "ET\r";

		// *************************************************************************************
		// Draw the box around the cell (minus the top line as it is already there)
		// *************************************************************************************
		PlotLine(	dXPos,			dYPos, dXPos,			dTopOfCell);	// Left Vertical Line.
		PlotLine(	dXPos+dWidth,	dYPos, dXPos+dWidth,	dTopOfCell);	// Right Vertical Line.
		PlotLine(	dXPos,			dYPos, dXPos+dWidth,	dYPos);			// Line at Bottom.
		tableLineData += "S\r";												// Close the Line section.
	}

	// *************************************************************************************
	// Write this accumulated data to the page and start a new page for the following report.
	// *************************************************************************************
	pPDFFile->PageDataAdd(tableLineData);
	tableLineData = "";
	dYPos		= PageHeight() - pPDFFile->CurrYPos();
	pPDFFile->CurrYPos(dYPos);
}

std::string PDFTable::DrawTableText( float& yPos, std::string title, PDFBodyObjectsList& annotsList, PDFBodyObjectsList& annotsInternalLinksList, PDFPageObject *thePDFPageObject, std::string HTMLLinkFilename )
{
	//if (continueFromPrevPage)
	//	yPos += FontHeight(); // Allow for the line of text "continued from previous page" by moving yPos down 1 line

	float pageVerticalRemaining = PageHeight() - BottomMargin() - yPos;// - (textDataExtraRows * timesNRFont->FontHeight());
	float currPageLeft = pageVerticalRemaining;

	// Begin the text
	tableLineData += "BT\r";
	tableLineData += drawCommands;

	// Specify the Font, Reset the font First
	timesNRFont->ResetFont();

	TableTextPosInfo posInfo( SubPageLeftMargin(), PDFMakeYCoord( yPos ) );

	std::string cellText;
	PDFCellTextStr *textStr;

	// Add "continued from previous page" if we are...
	ContinuedFromPrevPage( posInfo.currXOffset, posInfo.currYOffset );

	// Display the title
	DisplayTitle( posInfo, title, annotsInternalLinksList, thePDFPageObject, HTMLLinkFilename );

	// This is the row where the column headings are...
	headingRowNum = currTextRowNum;

	// Column Headings and Data Text
	tableLineData += timesNRFont->FontChangeStr( theTableData->heads.fontStyle, theTableData->heads.fontSize, theTableData->heads.fontColor );
	posInfo.currXPos += tableSet->textCellXIndent;
// JMC
	posInfo.rowHeight		= timesNRFont->FontMultiLineExtraLineHeight( theTableData->heads.fontSize );
	posInfo.currYPos		-= posInfo.rowHeight;
	posInfo.colHeadingYPos	= timesNRFont->FontMultiLineExtraLineHeight( theTableData->heads.fontSize );
	posInfo.UpdateOffsets();
	posInfo.xAcross			= posInfo.currXOffset;
	posInfo.yDown			= posInfo.currYOffset;
	posInfo.dataYPos		= timesNRFont->FontMultiLineExtraLineHeight( theTableData->data.fontSize );
	posInfo.rowHeight		= timesNRFont->FontHeight( theTableData->data.fontSize );

	float currRowHeight;
	int extraLinesForRow;

	size_t	nOffsetRows(theTableData->data.currRow);
	float	fColumnHeadingHeight(timesNRFont->FontHeight( theTableData->heads.fontSize ) );
	float	fFirstRowYPos	= (posInfo.lastYPos - fColumnHeadingHeight);

	for ( ; currTextRowNum < (numOfRows + textDataExtraRows) ; currTextRowNum++ )
	{
		if ( currTextRowNum == headingRowNum ) // Column Headings
		{
			textDataExtraRows++;
			extraLinesForRow = theTableData->heads.RowCellLines()-1; 
			currRowHeight	= fColumnHeadingHeight;
//			currRowHeight = timesNRFont->FontHeight( theTableData->heads.fontSize ) // First line
//						  + (extraLinesForRow * theTableData->heads.fontSize ); // All other lines in a multi-line cell
		}
		else // Data
		{
			// Check for first row after heading... so that we can change the Font to the data style...
			if ( currTextRowNum == headingRowNum+1 )
				tableLineData += timesNRFont->FontChangeStr( theTableData->data.fontStyle, theTableData->data.fontSize, theTableData->data.fontColor );
			extraLinesForRow = theTableData->data.RowCellLines( currTextRowNum-textDataExtraRows )-1; 
			currRowHeight = timesNRFont->FontHeight( theTableData->data.fontSize )
						  + (extraLinesForRow * theTableData->data.fontSize );
		}

		// Update the amount of page we will have left and check that it doesn't overwrite our bottom margin
		currPageLeft -= currRowHeight;
		if ( currPageLeft < BottomMargin() )
			break;

		posInfo.movedX = 0;
		posInfo.currColumnNum = 1;
		posInfo.numOfCols = theTableData->Columns();
		int colNum = 0;
		PDFTableData *p = theTableData;
		for( TableColWidthIter colWidthsIter = p->FirstColumnPtr(); colWidthsIter != p->BeyondLastColumnPtr(); p->NextColumnPtr( colWidthsIter ), colNum++ )
		{
			posInfo.currLine = 1;
			posInfo.yCellDown = 0;
			// Get the text for this cell
			if ( currTextRowNum == headingRowNum )
			{
				textStr = theTableData->GetNextCellHeadingText();
				if ( textStr )
				{
					if ( textStr->Lines() > 1 )
					{
						int i = 0;
						while ( i < textStr->Lines() )
						{
							cellText = textStr->Str( i );
							WriteCellText( thePDFPageObject, &annotsList, &posInfo, (*colWidthsIter), cellText, textStr, textStr->StrLen( i ) );
							i++;
						}
					}
					else
					{
						cellText = textStr->Str();
						WriteCellText( thePDFPageObject, &annotsList, &posInfo, (*colWidthsIter), cellText, textStr, textStr->StrLen() );
					}
				}
				else
				{
					WriteCellText( thePDFPageObject, &annotsList, &posInfo, (*colWidthsIter), PDFError->Str(), PDFError, PDFError->StrLen() );
				}
			}
			else
			{
				textStr = theTableData->GetNextCellDataText();
				if ( textStr )
				{
					if ( textStr->Lines() > 1 )
					{
						int i = 0;
						while ( i < textStr->Lines() )
						{
							cellText = textStr->Str( i );
							ReduceCellTextSize( colNum, (*colWidthsIter).ColWidth(), cellText, textStr );
							WriteCellText( thePDFPageObject, &annotsList, &posInfo, (*colWidthsIter), cellText, textStr, textStr->StrLen( i ) );
							i++;
						}
					}
					else
					{
						float	fMovedX = posInfo.movedX;
						cellText = textStr->Str();
						ReduceCellTextSize( colNum, (*colWidthsIter).ColWidth(), cellText, textStr );
						WriteCellText( thePDFPageObject, &annotsList, &posInfo, (*colWidthsIter), cellText, textStr, textStr->StrLen() );

#pragma Warning(This is where we are adding the code to add the links)
//						if ( textStr->Justify() & PDF_URI || textStr->Justify() & PDF_INTERNAL_LINK)
						if ( textStr->Justify() & PDF_URI)
						{
							float	fX1				= posInfo.lastXPos - fMovedX;
							float	fHeight			= posInfo.rowHeight;
							int		nRows			= theTableData->data.currRow - nOffsetRows - 1;
							float	fY1				= fFirstRowYPos - (fHeight * nRows);
							CreateLinkToText(fX1, fY1, thePDFPageObject, &annotsList, textStr);
						}
					}
				}
				else
				{
					ReduceCellTextSize( colNum, (*colWidthsIter).ColWidth(), cellText, textStr );
					WriteCellText( thePDFPageObject, &annotsList, &posInfo, (*colWidthsIter), PDFError->Str(), PDFError, PDFError->StrLen() );
				}
			}
		}


	/* Here was the text write code */

	}


	ContinuedOnNextPage( posInfo.xAcross, posInfo.yDown );

	// End the text
	tableLineData += "ET\r";

	return tableLineData;
}





void PDFTable::ReduceCellTextSize( int colNum, int allowedColWidth, std::string &cellText/*cellLineText*/, PDFCellTextStr *textStr )
{
	// Check that the text is not too long for the cell
	if ( colNum == theTableData->reduceStrCol )
	{
		// Get the width of the text
		float len = textStr->StrLen();
		float allowedLen = allowedColWidth - timesNRFont->FontSpacing();
		if ( len > allowedLen && !tableSet->textWrap ) // Text too long
		{
			if ( tableSet->reduceFontSizeOfTooLargeData )
			{
				textStr->HaveReducedTextSize( 1 );
				int fontSizeLargestThatWillFit = allowedLen / len * theTableData->data.fontSize;
				int fontSizeMinimumAllowed = theTableData->data.fontSize*tableSet->reduceFontSizeOfTooLargeDataByPercent/100;
				if ( fontSizeLargestThatWillFit >= fontSizeMinimumAllowed )
				{
					// Reduce the font size to larger than the minimum allowed and truncate the text if needed
					tableLineData += timesNRFont->FontChangeStr( theTableData->data.fontStyle, fontSizeLargestThatWillFit );
					cellText = timesNRFont->SetStringLen( cellText.c_str(), allowedLen, fontSizeLargestThatWillFit, thePDFSettings->TruncateTextLeftOrRight(), theTableData->data.fontStyle );
				}
				else
				{
					// Reduce the font size to the minimum allowed and truncate the text if needed
					tableLineData += timesNRFont->FontChangeStr( theTableData->data.fontStyle, fontSizeMinimumAllowed );
					cellText = timesNRFont->SetStringLen( cellText.c_str(), allowedLen, fontSizeMinimumAllowed, thePDFSettings->TruncateTextLeftOrRight(), theTableData->data.fontStyle );
				}
			}
			else // Just need to truncate the text 
				cellText = timesNRFont->SetStringLen( cellText.c_str(), allowedLen, theTableData->data.fontSize, thePDFSettings->TruncateTextLeftOrRight(), theTableData->data.fontStyle );
		}
	}
}

void PDFTable::WriteCellText( PDFPageObject *thePDFPageObject, PDFBodyObjectsList* pAnnotsList, TableTextPosInfo *posInfo, TableColWidth& allowedColWidth, std::string cellText/*cellLineText*/, PDFCellTextStr *textStr, int len /*= 0*/ )
{
	float centeredXPos = 0;

	if ( textStr->Justify() & PDF_RIGHTJUSTIFY )
	{
		if ( textStr->Justify() & PDF_CENTERED )
			centeredXPos = allowedColWidth.ColWidth() - (len-timesNRFont->FontSpacing()) - (tableSet->textCellXIndent*2) - allowedColWidth.AjustmentOffset();
		else //if ( textStr->Justify() & PDF_RIGHTJUSTIFY )
			centeredXPos = allowedColWidth.ColWidth() - (len-timesNRFont->FontSpacing()) - (tableSet->textCellXIndent*2);// - allowedColWidth.AjustmentOffset();
	}
	else if ( textStr->Justify() & PDF_LEFTJUSTIFY )
	{
		if ( textStr->Justify() & PDF_CENTERED )
			centeredXPos = allowedColWidth.AjustmentOffset();
		else //if ( textStr->Justify() & PDF_LEFTJUSTIFY )
			centeredXPos = 0;//allowedColWidth.AjustmentOffset();
	}
	else if ( textStr->Justify() & PDF_CENTERED )
		centeredXPos = (allowedColWidth.ColWidth() - (len-timesNRFont->FontSpacing()))/2 - tableSet->textCellXIndent;
	else // No justification... should have some
		centeredXPos = 0;

	if ( textStr->Justify() & PDF_HEADING_STYLE )
		tableLineData += timesNRFont->FontChangeStr( theTableData->heads.fontStyle, theTableData->data.fontSize, theTableData->heads.fontColor );

	// Position and display the text
	PositionNextText( posInfo->xAcross + centeredXPos, posInfo->yDown );
	tableLineData += "(";
	tableLineData += timesNRFont->MakePDFString( cellText );
	tableLineData += ")Tj\r";

	// Check if we have changed the text color for a link and change it back to that of the "data"
	if ( textStr->Justify() & PDF_URI || textStr->Justify() & PDF_INTERNAL_LINK )
		tableLineData += timesNRFont->FontChangeStr( theTableData->data.fontStyle, theTableData->data.fontSize, theTableData->data.fontColor );
	
	// See if we need to reset the text size and style back to that of the "data"
	if ( textStr->HaveReducedTextSize() || textStr->Justify() & PDF_HEADING_STYLE )
		tableLineData += timesNRFont->FontChangeStr( theTableData->data.fontStyle, theTableData->data.fontSize, theTableData->data.fontColor );

	// Set the location for the next cell
	if ( posInfo->currLine != textStr->Lines() ) // We are going to be writing in the same cell again, so go down a line 
	{
		posInfo->xAcross = -centeredXPos;
		if ( currTextRowNum == headingRowNum )
			posInfo->yDown = -theTableData->heads.fontSize;
		else
			posInfo->yDown = -theTableData->data.fontSize;
		//posInfo->yDown = -posInfo->rowHeight;
//		if ( currTextRowNum == headingRowNum ) // First row after heading... so move the text Y position to the correct location for data text...
//			posInfo->yDown -= (posInfo->colHeadingYPos-posInfo->dataYPos);
//		posInfo->movedY += posInfo->yDown;
		posInfo->yCellDown -= posInfo->yDown;
		if ( posInfo->yCellDown > posInfo->lowestYCellDown )
			posInfo->lowestYCellDown = posInfo->yCellDown;
	}
	else // Finished writing cell... see if we have more cell columns or go to the next row
	{
		if ( posInfo->currColumnNum < posInfo->numOfCols ) // Still have more columns to go, so move across
		{
			posInfo->xAcross = allowedColWidth.ColWidth() - centeredXPos;
			posInfo->yDown = 0;
			posInfo->yDown += posInfo->yCellDown; // Moves back up... if we have gone down in this cell
		}
		else // We are in the last column, so move down a row
		{
			posInfo->xAcross = posInfo->movedX - centeredXPos;
//			posInfo->yDown = -posInfo->rowHeight;
			//if ( currTextRowNum == headingRowNum )
			//	posInfo->yDown = -theTableData->heads.fontSize;
			//else
				posInfo->yDown = -timesNRFont->FontHeight( theTableData->data.fontSize );
//			if ( currTextRowNum == headingRowNum ) // First row after heading... so move the text Y position to the correct location for data text...
//				posInfo->yDown -= (posInfo->colHeadingYPos-posInfo->dataYPos);
			posInfo->yDown -= (posInfo->lowestYCellDown - posInfo->yCellDown); // Need to move the Y position down if we have 
//			posInfo->movedY += posInfo->yDown;
			posInfo->lowestYCellDown = 0;
			posInfo->yCellDown = 0;
		}
	}
	centeredXPos = 0;

	// Check to see if we have finished writing to this cell...
	if ( posInfo->currLine == textStr->Lines() ) // We have finished writing out this cell's data
	{
		posInfo->currLine = 1;
		posInfo->currColumnNum++;
		// Update the value that we have moved across so that we can move back 
		posInfo->movedX -= allowedColWidth.ColWidth();	// Update the value that we have moved across so that we can move back 
	}
	else
	{
		posInfo->currLine++; // Not finished writing the cell's data, so update the cell line count
		//posInfo->sameCellMovedX -= allowedColWidth;
//		posInfo->movedX -= allowedColWidth;	// Update the value that we have moved across so that we can move back 
	}
}

void PDFTable::ContinuedFromPrevPage( float& x, float& y )
{
	if ( !continueFromPrevPage )
		return;

	// Write "continued from previous page..."
	tableLineData += timesNRFont->FontChangeStr( PDF_NORMAL );
	float rowHeight = timesNRFont->FontHeight( theTableData->titleFontSize );
	x += tableSet->textCellXIndent;
	int continuedFromYPos = (theTableData->titleFontSize + timesNRFont->FontSpacing()) / 5/*moveTextUpFromLine*/;
	y += continuedFromYPos;
	PositionNextText( x, y );
	tableLineData += "(";
	tableLineData += timesNRFont->MakePDFString( continuedFromPrev );
	tableLineData += ")Tj\r"; 
	// Move the current position back to it's original position
	x = -tableSet->textCellXIndent;
	y = -continuedFromYPos;
}

void PDFTable::ContinuedOnNextPage( float xAcross, float yDown )
{
	if ( DataLeft() ) // Check if we're gunna have another page of data
	{
		tableLineData += timesNRFont->FontChangeStr( PDF_NORMAL );
		PositionNextText( xAcross, yDown );
		tableLineData += "(";
		tableLineData += timesNRFont->MakePDFString( continuedOnNext );
		tableLineData += ")Tj\r"; 
		continueFromPrevPage = true;
	}
}

void PDFTable::PlotLineTo( float x, float y )
{
	tableLineData += floatToStr( x );
	tableLineData += " ";
	tableLineData += floatToStr( y );
	tableLineData += " l\r";
}

void PDFTable::PlotLine( float x, float y, float x2, float y2 )
{
	tableLineData += floatToStr( x );
	tableLineData += " ";
	tableLineData += floatToStr( y );
	tableLineData += " m\r";
	tableLineData += floatToStr( x2 );
	tableLineData += " ";
	tableLineData += floatToStr( y2 );
	tableLineData += " l\r";
}

void PDFTable::PlotDrawLine( float x, float y, float x2, float y2 )
{
	PlotLine( x, y, x2, y2 );
	tableLineData += "S\r";
}

void PDFTable::PlotFilledRect( float x, float y, float x2, float y2, long frcol )
{
	if (frcol == PDF_INVALID_COLOR)
		return; // Don't draw the rect if the color is invalid
	if ( thePDFSettings->CommentsOn() )
		tableLineData += "% Filled Rect\r";
	tableLineData += GetRGBFillColorStr( frcol );
	tableLineData += floatToStr( x );
	tableLineData += " ";
	tableLineData += floatToStr( y );
	tableLineData += " ";
	tableLineData += floatToStr( (x2) );
	tableLineData += " ";
	tableLineData += floatToStr( (y2) );
	tableLineData += " re\rf\r";
}

std::string PDFTable::GetRGBFillColorStr( long rgb )
{
	if (rgb == currFillCol)
		return PDF_BLANK;

	currFillCol = rgb;
	long colorElement;
	colorElement = (rgb & 0xff0000) >> 16;
	float r = colorElement / (float)255;
	colorElement = (rgb & 0xff00) >> 8;
	float g = colorElement / (float)255;
	colorElement = (rgb & 0xff);
	float b = colorElement / (float)255;

	// If all colors are the same use short-hand notation
	if (r == g && r == b)
	{
		colBuf = floatToStr( r );
		colBuf += " g\r";
		return colBuf;
	}

	colBuf = floatToStr( r );
	colBuf += " ";
	colBuf += floatToStr( g );
	colBuf += " ";
	colBuf += floatToStr( b );
	colBuf += " rg\r";
	return colBuf;
}

int PDFTable::SubPageLeftMargin()
{
	if ( thePDFSettings->SubPagesPerPage() > 1 && SubPageNum() > 1 )
	{
		int pageDrawWidth, pageXPos, subPageMiddleMarginWidth, subPageWidth;
		pageDrawWidth = thePDFSettings->DrawWidth();
		subPageMiddleMarginWidth = RightMargin() * (thePDFSettings->SubPagesPerPage()-1);
		subPageWidth = (pageDrawWidth - subPageMiddleMarginWidth) / thePDFSettings->SubPagesPerPage();
		pageXPos = LeftMargin() + ( (SubPageNum()-1) * (subPageWidth + RightMargin()) );
		return pageXPos;
	}
	else
		return LeftMargin();
}


static const int NEW_SECTION_LINES_BEFORE = 0;
static const int NEW_SECTION_LINES_AFTER = 1;

//
// PDFText class functions
//

int PDFText::GetDrawWidthMax( int textXPosType )
{
	if ( textXPosType >= 0 )
		return SubPageDrawWidth() - textXPosType;

	switch( textXPosType )
	{
	case PDF_TEXT_COLUMN:
		return ColumnSpace();
		break;
	case PDF_TEXT_AT_FIRST_TAB:
	case PDF_TEXT_AT_NEXT_TAB:
		return TabSpace();
		break;

	};
	return SubPageDrawWidth();
}

void PDFText::SplitTextThatIsTooLong( PDFTextList& textList, bool indentSplitTextInColumns )
{
	if ( haveAlreadySplitText )
		return;

	const char indentStr[]="   ";

	for( PDFTextListIter iter( textList.begin() ); iter!=textList.end(); ++iter )
	{
		PDFTextStr& textStr=*iter;
		
		if( textStr.text!=PDF_BLANKLINE && textStr.text != PDF_LINE )
		{
			// Make sure the text is not too big for the column width that we are drawing in
			int strLen = timesNRFont->GetStringLen( textStr.text, textStr.size, textStr.state );
			int maxLen = GetDrawWidthMax( textStr.x ) - colSpacing;

			while ( strLen > maxLen && textStr.x == PDF_TEXT_COLUMN )
			{
				std::string splitText = SplitText( textStr.text, maxLen, textStr.size, PDF_RIGHT_NOELLIPSE | PDF_BREAK_AT_SPACE, textStr.state );
				PDFTextStr textStrSplit( splitText, textStr.x, textStr.y, textStr.state, textStr.size, textStr.color );
				iter=textList.insert( iter, textStrSplit );
				++iter;
				if ( indentSplitTextInColumns && textStr.x == PDF_TEXT_COLUMN )
				{
					// There already has to be some char's which cause a split to indent the string,
					// otherwise the split will occur in the last space of the indent string
					strLen = timesNRFont->GetStringLen( textStr.text, textStr.size, textStr.state );
					int indentLen = timesNRFont->GetStringLen( indentStr, textStr.size, textStr.state );
					if ( indentLen + strLen > maxLen ) // Check to see if an indented string is will be too big
					{
						// Only indent the string which is this big if it has a seperator char in it...
						// otherwise we will have problems, as adding spaces will cause the seperation at
						// the last space char, and we'll get stuck in a loop...
						if ( timesNRFont->AnySplitChars( textStr.text ) )
							textStr.text.insert( 0, indentStr );
					}
					else
						textStr.text.insert( 0, indentStr );
				}
				strLen = timesNRFont->GetStringLen( textStr.text, textStr.size, textStr.state );
			}
		}
	}
	haveAlreadySplitText = true;
}

void PrintTextListToFile( const char* filename, PDFTextList& textList )
{
	FILE *file = fopen( filename, "w+" );
	
	PDFTextList textList2 = textList;

	PDFTextListIter iter = textList2.begin();
	int count = textList2.size();
	//while ( count )
	for( ; iter != textList2.end(); ++iter )
	{
		PDFTextStr& textStr = (*iter);
		fwrite( (void*)textStr.text.c_str(), textStr.text.length(), 1, file );
		fwrite( "\n", 1, 1, file );
	}
	fclose( file );
}

void PDFText::CalculateColumnLayoutInfo( PDFTextList& textList, ColInfoList& successiveColLengths )
{
	// Work out how many rows of data we will have approximately, especially the number of column data...
	int totalColItemsCount = 0;
	int successiveColItemsCount = 0;
	int blanklineStart = 0;
	int blanklineEnd = 0;
	for( PDFTextListIter iter = textList.begin(); iter != textList.end(); iter++ )
	{
		PDFTextStr& textStr = (*iter);
		if ( textStr.x == PDF_TEXT_COLUMN )
		{
			successiveColItemsCount++;
			if ( textStr.text == PDF_BLANKLINE || textStr.text == PDF_LINE )
			{
				if ( blanklineStart )
					blanklineEnd = successiveColItemsCount;
				else
					blanklineStart = successiveColItemsCount;
			}
			else
			{
				blanklineStart = 0;
				blanklineEnd = 0;
			}
		}
		else
		{
			// Check number
			if ( successiveColItemsCount )
			{
				// We need to see if there are blank lines at the end of the column data... 
				// and take these off the total as they will not be drawn
				if ( blanklineStart )
				{
					if ( blanklineEnd ) // More than one blankline at the end of column
						successiveColItemsCount -= (blanklineEnd-blanklineStart);
					else // Only one blankline at the end of column
						successiveColItemsCount--;
				}
				totalColItemsCount += successiveColItemsCount/PDF_TEXT_MAX_NUM_COLUMNS;
				// See if there will be a remainder, as this effects whether we add 1 or not to the total 
				if ( successiveColItemsCount%PDF_TEXT_MAX_NUM_COLUMNS ) // There is a remainder
					totalColItemsCount++;
				ColInfo conlInfo( successiveColItemsCount, totalColItemsCount );
				successiveColLengths.push_back( conlInfo );
				totalColItemsCount = 0;
				successiveColItemsCount = 0;
			}
		}
	}
	if ( successiveColItemsCount )
	{
		// See if there will be a remainder, as this effects whether we add 1 or not to the total 
		if ( successiveColItemsCount%PDF_TEXT_MAX_NUM_COLUMNS ) // There is a remainder
			totalColItemsCount++;
		totalColItemsCount += successiveColItemsCount/PDF_TEXT_MAX_NUM_COLUMNS;
		successiveColLengths.push_back( ColInfoList::value_type(successiveColItemsCount, totalColItemsCount) );
		successiveColItemsCount = 0;
	}
}

std::string PDFText::SplitText( std::string& str, double lenWanted, int size /*= 0*/, short truncateTextLeftOrRight /*= PDF_LEFT_ELLIPSE*/, int style /*= PDF_NORMAL*/ )
{
	std::string splitText = timesNRFont->SetStringLen( str, lenWanted, size, truncateTextLeftOrRight, style );
	int len = splitText.length();
	str.erase( 0, len );
	return splitText;
}

typedef struct PDFTextPosInfoS
{
	float xTextPos;
	float yTextPos;
	float x;
	float y;
	float xMove;
	float yMove;
	void Init()
	{
		xTextPos = 0;
		yTextPos = 0;
		x = 0;
		y = 0;
		xMove = 0;
		yMove = 0;
	}
} PDFTextPosInfo;

typedef struct PDFTextColPosInfoS
{
	int currCol;
	float colYStartPos;
	float colYEndPos;
	float colXPos;// = SubPageLeftMargin();
	float colXPosTab;// = thePDFSettings->SubPageDrawWidth()/PDF_TEXT_MAX_NUM_COLUMNS;
	int colItemsCount;
	int maxColumnItems;
//	int maxColumnHeight;
	int columnEndYPos;
	int remainder;
	void Init()
	{
		currCol = 0;
		colYStartPos = -1;
		colYEndPos = 0;
		colXPos = 0;//SubPageLeftMargin();
		colXPosTab = 0;//thePDFSettings->SubPageDrawWidth()/PDF_TEXT_MAX_NUM_COLUMNS;
		colItemsCount = 0;
		maxColumnItems = 0;
//		maxColumnHeight = 0;
		columnEndYPos = 0;
		remainder = 0;
	}

} PDFTextColPosInfo;


std::string PDFText::Draw( PDFTextList& textList, int specificYPos /*= 0*/, short vertCenter /*= 0*/, bool allowOutsideMargins /*= 0*/ )
{
	textData = "";
	textData.reserve( 10 * 1024 );

	// Set the distance we move text which is tabbed.
	int drawWidth = SubPageDrawWidth();
	TabSpace( ColumnSpace()+colSpacing );

	float xLastPos = 0;
	static float yLastPos = 0;
	float origXPos = SubPageLeftMargin();
	std::string convertedToPDFText;
	std::string splitText;
	std::string *displayText;
	PDFTextPosInfo pos;	pos.Init();
	float blankLineY = 0;
	short firstText = 1;
	short addedLineSpec = 0;
//	short currTabCol = 1;
//	short tabSplitCol = -1;
//	short sameTabPos = 0;
//	short splitTabText = 0;
	short tabStrSplitY = 0;
	short lastTabStrSplitY = 0;
	short highestTabStrSplitY = 0;
	long lastStringCount, thisStringCount;
	bool stillWritingSameText = false;
	short yColumnHeightAdjust = 0;

	// Store the previous color
	int prevColor = timesNRFont->TextColor();

	// Reset the all font details
	timesNRFont->ResetFont();

	// Begin the text
	textData += "BT\r";
	
	PDFTextColPosInfo colPos; colPos.Init();
	colPos.colXPos = SubPageLeftMargin();
	colPos.colXPosTab = ColumnSpace();

	bool popStringOffList = true;

	if ( specificYPos > 0 )
	{
		// If a specific location has been given to start the text, then use it
		pos.y = specificYPos;
		yLastPos = specificYPos;
	}
	else
		pos.y = yLastPos;

	if ( vertCenter )
	{
		float heightOfText = GetTextHeight( textList );	// Work out the height of the text
		// Set the current (last) Y position to be centered vertically on the page
		yLastPos = specificYPos + (thePDFSettings->DrawHeight()-specificYPos)/2 - heightOfText/2;
	}
	//else if ( specificYPos )
	/*{
		
		yLastPos = specificYPos;
	}*/

	//PrintTextListToFile( "C:\\TextListOrig.txt", textList );

	// Make sure all of the text strings are the right length
	SplitTextThatIsTooLong( textList, true );

	//PrintTextListToFile( "C:\\TextList.txt", textList );

	ColInfoList successiveColLengths;
	CalculateColumnLayoutInfo( textList, successiveColLengths );
	lastStringCount = textList.size()+1;
	while ( thisStringCount = textList.size() )
	{
	 	PDFTextStr& textStr = textList.front();
		// Set the color of text
//		textData += timesNRFont->Color( (*iter).color );

		pos.xTextPos = textStr.x;
		pos.yTextPos = textStr.y;
		
		if ( textStr.x == PDF_TEXT_COLUMN )
			colPos.colItemsCount++;

		if ( lastStringCount == thisStringCount )
			stillWritingSameText = true;
		else
			tabStrSplitY = 0;

		yColumnHeightAdjust = 0;
		if ( pos.xTextPos != PDF_TEXT_AT_NEXT_TAB && !stillWritingSameText )
		{
			if ( highestTabStrSplitY > lastTabStrSplitY )
				 yColumnHeightAdjust = (highestTabStrSplitY - lastTabStrSplitY); 
			highestTabStrSplitY = 0;
			lastTabStrSplitY = 0;
		}

		// Check to see if we want a blank line drawn, instead of text...
		if ( textStr.text == PDF_BLANKLINE )
		{
			if ( pos.yTextPos < 1 )
				blankLineY += timesNRFont->DefaultFontSize();
			else
				blankLineY += pos.yTextPos;
			textList.pop_front();
			continue;
		}

		// Check to see if we want a line drawn, instead of text...
		if ( textStr.text == PDF_LINE )
		{
			float lineYPos = PageHeight()-(yLastPos+blankLineY+timesNRFont->DefaultFontSize());
			PlotDrawLine( SubPageLeftMargin()+pos.xTextPos, lineYPos, SubPageLeftMargin()+SubPageDrawWidth()-pos.xTextPos, lineYPos );

			textList.pop_front();
			continue; // Go onto next text...
		}

/*		if ( pos.xTextPos == PDF_TEXT_AT_FIRST_TAB )
			currTabCol = 1;
		else if ( pos.xTextPos == PDF_TEXT_AT_FIRST_TAB && !splitTabText )
			currTabCol++;*/

		// Make sure the text is not too big for the current width that we are drawing in
		int strLen = timesNRFont->GetStringLen( textStr.text, textStr.size, textStr.state );
		int maxLen = GetDrawWidthMax( pos.xTextPos );
		//splitTabText = 0;
		if ( strLen > maxLen )
		{
			if ( pos.xTextPos == PDF_TEXT_AT_FIRST_TAB || pos.xTextPos == PDF_TEXT_AT_NEXT_TAB )
			{
				//splitTabText = 1;
				tabStrSplitY += textStr.size;
				lastTabStrSplitY = tabStrSplitY;
				if ( highestTabStrSplitY < tabStrSplitY )
					highestTabStrSplitY = tabStrSplitY;
			}
			popStringOffList = false;
			splitText = SplitText( textStr.text, maxLen, textStr.size, PDF_RIGHT_NOELLIPSE | PDF_BREAK_AT_SPACE, textStr.state );
			displayText = &splitText; 
		}
		else
		{
			popStringOffList = true;
			displayText = &textStr.text;
		}

		// Specify the Font to be used for this text
		textData += timesNRFont->FontChangeStr( textStr.state, textStr.size, textStr.color );

		// Work out the moved X value
		if ( pos.xTextPos < 0 ) // This means write text somewhere else besides a given X-coordinate
		{
			switch ( (int)pos.xTextPos )
			{
			case PDF_TEXT_CENTERED:
				{int textLen = timesNRFont->GetStringLen( *displayText, textStr.size, textStr.state );
				pos.x = SubPageLeftMargin() + drawWidth/2 - textLen/2;}
				break;
			case PDF_TEXT_RIGHT:
				{int textLen = timesNRFont->GetStringLen( *displayText, textStr.size, textStr.state );
				pos.x = SubPageLeftMargin() + SubPageDrawWidth() - textLen;}
				break;
			case PDF_TEXT_COLUMN:
				pos.x = colPos.colXPos;
				break;
			case PDF_NEW_SECTION:
				pos.x = origXPos;
				break;
			case PDF_NON_COLUMN:
				pos.x = origXPos;
				break;
			case PDF_TEXT_AT_NEXT_TAB:
				if ( stillWritingSameText )
					pos.x = xLastPos;
				else
					pos.x = xLastPos + TabSpace();
				if ( pos.x >= (SubPageLeftMargin() + drawWidth) )
				{
					pos.x = origXPos;
					yLastPos += textStr.size;
					//pos.y = yLastPos + textStr.size;
				}
				break;
			case PDF_TEXT_AT_FIRST_TAB:
				pos.x = origXPos;
				break;
			default:
				pos.x = origXPos;
			}
		}
		else // X pos is at a given position
		{
			if ( pos.yTextPos == PDF_TEXT_ON_NEXT_LINE && pos.xTextPos == PDF_X_POS_UNCHANGED ) // This means go down one line, b
				pos.x = origXPos;
			else
			{
				pos.x = pos.xTextPos + SubPageLeftMargin();
				origXPos = pos.x;
				if ( colPos.currCol != 0 ) // Means we have used columns some time... so add we need to position stuff
				{
					if ( colPos.colYEndPos == 0 )
						pos.y = yLastPos;
					else
						pos.y = colPos.colYEndPos;
					colPos.currCol = 0;
				}
			}
		}

		// Work out the moved Y value
		if ( pos.yTextPos <= PDF_TEXT_ON_NEXT_LINE ) // This means go down one line
		{
			if ( pos.yTextPos == PDF_NEW_SECTION ) //
				pos.y = yLastPos + textStr.size*(NEW_SECTION_LINES_BEFORE + 1 + NEW_SECTION_LINES_AFTER);
			else // yTextPos == PDF_TEXT_ON_NEXT_LINE
			{
				pos.y = yLastPos + textStr.size;//timesNRFont->FontHeight();
				if ( pos.xTextPos == PDF_TEXT_COLUMN )
				{
					if ( colPos.currCol == 0 ) // Means we have not used columns yet...
					{
						ColInfo& colInfo = successiveColLengths.front();
						colPos.maxColumnItems = colInfo.colLength;
						colPos.remainder = colInfo.successiveColItems%PDF_TEXT_MAX_NUM_COLUMNS;
						colPos.colItemsCount = 0;
						colPos.currCol = 1;
					}
					if ( colPos.colItemsCount == colPos.maxColumnItems || (colPos.currCol != 1 && colPos.remainder+colPos.colItemsCount == colPos.maxColumnItems) )
					{
						if ( colPos.currCol == PDF_TEXT_MAX_NUM_COLUMNS )
						{
							successiveColLengths.pop_front();
							colPos.columnEndYPos = pos.y;
							break;
						}
						colPos.colItemsCount = 0;
						colPos.currCol++; 
						colPos.colYEndPos = pos.y; // This is the bottom of the columns (well actually... of the current column... but this does matter)
						pos.y = colPos.colYStartPos; // Move the Y position back to the top of the next column
						colPos.colXPos += colPos.colXPosTab; // Move the "column" X position across one column
						pos.x = colPos.colXPos; // Move the X position to the new column
					}

					if ( colPos.colYStartPos == -1 )
						colPos.colYStartPos = pos.y + blankLineY;

					// Check if We need to move to another column if this one is full
					if ( pos.y + blankLineY > DrawHeight() + BottomMargin() ) // Trev
					{
						// Check that the page is full, ie. all the columns are used...
						if ( colPos.currCol == PDF_TEXT_MAX_NUM_COLUMNS )
						{
							ColInfo& colInfo = successiveColLengths.front();
							colInfo.colLength -= colPos.colItemsCount; 
							colInfo.successiveColItems -= (colPos.colItemsCount*2);
							colPos.columnEndYPos = TopMargin() + DrawHeight();
							break; // Finished writing to this page, so break of the while loop...
						}
						colPos.colItemsCount = 0;
						colPos.currCol++; 
						colPos.colYEndPos = pos.y; // This is the bottom of the columns (well actually... of the current column... but this does matter)
						pos.y = colPos.colYStartPos; // Move the Y position back to the top of the next column
						colPos.colXPos += colPos.colXPosTab; // Move the "column" X position across one column
						pos.x = colPos.colXPos; // Move the X position to the new column
					}
				}
				else if ( pos.xTextPos == PDF_X_POS_UNCHANGED && colPos.currCol != 0 ) // Means we have used columns some time... so add we need to position stuff
				{
					if ( !colPos.colYEndPos )
						colPos.colYEndPos = pos.y;
					pos.y = colPos.colYEndPos;
					colPos.currCol = 0;
				}

			}
		}
		else // Either putting text at next tab spot or at a specific location
		{
			if ( pos.xTextPos == PDF_TEXT_AT_NEXT_TAB )
			{
				pos.y = yLastPos; // Staying on same line, so make Y value the same as the last
				if ( stillWritingSameText )
				{
					pos.y += textStr.size;
				}
				else // if ( !stillWritingSameText )
				{
					if ( lastTabStrSplitY )
					{
						pos.y -= lastTabStrSplitY;
						lastTabStrSplitY = 0;
					}
				}
			}
			else if ( pos.xTextPos == PDF_TEXT_AT_FIRST_TAB )
			{
				pos.y = yLastPos + pos.yTextPos + yColumnHeightAdjust; // Putting text in a given location from the current
			}
			else
			{
				pos.y = yLastPos + pos.yTextPos + yColumnHeightAdjust; // Putting text in a given location from the current
			}
		}

		// Write out the X postion
		pos.xMove = pos.x - xLastPos; // Work out how much we change the X value
		textData += floatToStr( pos.xMove ); 
		textData += " ";
		xLastPos = pos.x; // Remember the current location of the X value

		// Write out the Y postion
		pos.yMove = yLastPos - pos.y - blankLineY;
		if ( firstText )
		{
			firstText = 0;
			textData += floatToStr( PageHeight() - pos.y );// - CurrYPos() ); 
		}
		else
			textData += floatToStr( pos.yMove ); 
		textData += " Td\r"; 
		yLastPos = pos.y + blankLineY;
		if ( pos.y > DrawHeight()+TopMargin() && !allowOutsideMargins ) // Too low she cried...
		{
			if ( colPos.currCol == 1 ) // Use the next column, if we are using columns...
			{
				colPos.maxColumnItems = colPos.colItemsCount; 
				colPos.colItemsCount = 0;
				colPos.currCol++; 
				colPos.colYEndPos = pos.y; // This is the bottom of the columns (well actually... of the current column... but this does matter)
				pos.y = colPos.colYStartPos; // Move the Y position back to the top of the next column
				colPos.colXPos += colPos.colXPosTab; // Move the "column" X position across one column
				continue;
			}
			else
				break; // We are either not using columns or we have used up our columns...
		}

		blankLineY = 0;

		// Write out the text
		textData += "(";
		convertedToPDFText = timesNRFont->MakePDFString( *displayText ); 
		textData += convertedToPDFText;
		textData += ")Tj\r";
		lastStringCount = thisStringCount;

		if ( popStringOffList )
		{
			stillWritingSameText = false;
			textList.pop_front();
		}
	} // end while

	if (blankLineY)
	{
		textData += "0 ";
		textData += floatToStr( blankLineY );
		textData += " Td\r()Tj\r";
	}
	// End the text
	textData += "ET\r";

	if ( !addedLineSpec )
	{
		addedLineSpec = 1;
		textData += "0 G\r";
		textData += "0 J 0 j 0.005 w 10 M []0 d\r";
	}
	// Add any line data that was drawn
	textData += lineData;

	// Set the color of text back to the previous color
	textData += timesNRFont->TextColor( prevColor );

	if ( colPos.currCol > 0 ) // We have filled a complete column, so we must indicate that the last Y Pos was the bottom of the first (largest) column
		lastDrawYPos = colPos.columnEndYPos;
	else
		lastDrawYPos = yLastPos;

	// If we're near the bottom of the page, then set the last Y Pos to be the end of the page so that the next time we draw we don't write a few lines of text only
	if ( TopMargin() + DrawHeight() - lastDrawYPos < DrawHeight()/5 )
		lastDrawYPos = TopMargin() + DrawHeight();

	return textData;
}

int PDFText::GetTextHeight( PDFTextList& textList )
{
	int ptsHigh = 0;
	int alteredFontSizeOffset = 0;
	for( PDFTextList::iterator iter = textList.begin(); iter != textList.end(); iter++ )
	{
		if ( (*iter).text == PDF_BLANKLINE )
			ptsHigh += (*iter).y;//(*iter).size;
		else if ( (*iter).text == PDF_LINE )
			ptsHigh += timesNRFont->DefaultFontSize();
		else if ( (*iter).x == PDF_NEW_SECTION )
			ptsHigh += (*iter).size * (NEW_SECTION_LINES_BEFORE + 1 + NEW_SECTION_LINES_AFTER);
		else if ( (*iter).x == PDF_NON_COLUMN )
			ptsHigh += (*iter).size;
		else if ( (*iter).x != PDF_TEXT_AT_NEXT_TAB )
			ptsHigh += (*iter).size;
			continue; // These text elements are on the same line, so no need to add a line to the line counter
	}
	return ptsHigh;
}

/*std::string PDFText::SetFontSize( int fontSize )
{
	std::string fontData;
	if ( fontSize != currFontSize )
	{
		// Specify/Change the Font
		fontData += "/";
		fontData += timesNRFont->FontName();
		fontData += " ";
		fontData += floatToStr( fontSize );
		fontData += " Tf\r";
		currFontSize = fontSize;
	}
	return fontData;
}*/


void PDFText::PlotDrawLine( float x, float y, float x2, float y2 )
{
	lineData += floatToStr( x );
	lineData += " ";
	lineData += floatToStr( y );
	lineData += " m\r";
	lineData += floatToStr( x2 );
	lineData += " ";
	lineData += floatToStr( y2 );
	lineData += " l\r";
	lineData += "S\r";
}




#define	PI				3.1415926535897932384626433832795
#define	RADTODEG(x)		(x*180/PI)
#define	DEGTORAD(x)		(x*PI/180)

void PDFMarker::CalculateArc( PDFPointPtr p, float cxP, float cyP, float xP, float yP, double yScale, double a1, double a2, float rad ) 
{
	//
	// The Arc of the "Pie Slice"
	//
	double angle1, angle2;

	double rangle1, rangle2;
	double diffRad1, diffRad2;
	double lenPtFromCirRad1, lenPtFromCirRad2;
	double realCirPosX1, realCirPosY1, realCirPosX2, realCirPosY2;
	double extraX1, extraY1, extraX2, extraY2;
	double x = xP, y = yP, cx = cxP, cy = cyP;

	// Start point of the Arc
	p[0].x = cx + x;			p[0].y = cy + y * yScale;

	//
	// To draw the Arc, we need to have two points which "guide" the drawing to the Arc from
	// the beginning point to the end point
	//

	// 1st guide point of Arc
	angle1 = a2-((a2-a1)/3*2);
	rangle1 = DEGTORAD(angle1);
	diffRad1 = DEGTORAD( (a1 - angle1) );
	lenPtFromCirRad1 = (1-cos(diffRad1))*rad; // Distance of tangent of "air" point from radius
	realCirPosX1 = (cos(rangle1)*rad);
	extraX1 = cos( rangle1 ) * lenPtFromCirRad1 ;
	x = realCirPosX1 + extraX1; // Radius + "x" value from Radius 
	realCirPosY1 = (sin(rangle1)*rad) ;
	extraY1 = sin( rangle1 ) * lenPtFromCirRad1;
	y = realCirPosY1 + extraY1; // Radius + "y" value from Radius 

	p[0].x = cx + x;			p[0].y = cy + y * yScale;

	// 2nd guide point of Arc
	angle2 = a2-((a2-a1)/3);
	rangle2 = DEGTORAD(angle2);
	diffRad2 = DEGTORAD( (angle1 - angle2) );
	lenPtFromCirRad2 = (1-cos(diffRad2))*rad; // Distance of tangent of "air" point from radius
	realCirPosX2 = (cos(rangle2)*rad);
	extraX2 = cos( rangle2 ) * lenPtFromCirRad2 ;
	x = realCirPosX2 + extraX2; // Radius + "x" value from Radius 
	realCirPosY2 = (sin(rangle2)*rad) ;
	extraY2 = sin( rangle2 ) * lenPtFromCirRad2;
	y = realCirPosY2 + extraY2; // Radius + "y" value from Radius 

	p[1].x = cx + x;			p[1].y = cy + y * yScale;

	// End point of Arc
	double rad_2 = DEGTORAD(a2);
	x = cos(rad_2)*rad;
	y = sin(rad_2)*rad;

	p[2].x = cx + x;			p[2].y = cy + y * yScale;
}

void PDFMarker::DrawQuarterCircle( float x1, float y1, float x2, float y2 )
{
	float cx, cy, rad;
	double yScale = 1;
	double	angle1, angle2;
	double lastpercent, current_percent;

	current_percent = 25;

	if ( x1 < x2 )
	{
		// This does top left Quarter of Circle
		if ( y1 < y2 )
		{
			cx = x2;
			cy = y1;
			rad = x2 - x1;
			lastpercent = 25;
		}
		// This does top right Quarter of Circle
		else //if ( y1 > y2 )
		{
			cx = x1;
			cy = y2;
			rad = x2 - x1;
			lastpercent = 0;
		}
	}
	else //if ( x1 > x2 )
	{
		// This does bottom right Quarter of Circle
		if ( y1 > y2 )
		{
			cx = x2;
			cy = y1;
			rad = x1 - x2;
			lastpercent = 75;
		}
		// This does bottom left Quarter of Circle
		else
		{
			cx = x1;
			cy = y2;
			rad = x1 - x2;
			lastpercent = 50;
		}
	}

	angle1 = ((360*lastpercent)/100.0);
	angle2 = ((360*(lastpercent+current_percent))/100.0);

	DrawCurve( x1, y1, x2, y2, cx, cy, rad, yScale, angle1, angle2 );
}

void PDFMarker::DrawCurve( float x1, float y1, float x2, float y2, float cx, float cy, float rad, double yScale, double angle1, double angle2 )
{
	double x, y;
	double rad_1;

	PDFPoint	polyr[5]; // Structure to hold the Arc points

	// Center of the pie
	polyr[0].x = cx;		polyr[0].y = cy; // Center
	
	// Line from centre of the "Pie" to the start of the Arc
	rad_1 = DEGTORAD(angle1);

	x = cos( rad_1 )*rad;
	y = sin( rad_1 )*rad;
//	polyr[1].x = cx + x;	polyr[1].y = cy + y * yScale; // Start of Arc
	polyr[1].x = x2;	polyr[1].y = y2; // Start of Arc

	// Calculate the arc points
	CalculateArc( &polyr[2], cx, cy, x, y, yScale, angle1, angle2, rad );
	polyr[4].x = x1;	polyr[4].y = y1; // End of Arc

	PlotMoveTo( polyr[1].x, polyr[1].y ); 
	PlotArc( &polyr[2], angle1, angle2 );

	// Plot the Arc which goes along the bottom of the pie slice
//	PlotMoveTo( pie3dCylLook[0].x, pie3dCylLook[0].y ); 
//	PlotArc( &pie3dCylLook[1], a1, a2 );

	// Plot the 2nd Arc which is offset from the first so that we get the 3d cyclinder look
//	PlotLineTo( pie3dCylLook[7].x, pie3dCylLook[7].y, color ); 
//	PlotArc( &pie3dCylLook[4], a1, a2, 1 );


}

void PDFMarker::PlotArc( PDFPointPtr p, double a1, double a2, short reverse /*= 0*/ )
{
	// PDF Comments
	//if ( thePDFSettings.CommentsOn() )
	{
		graphData += "% Arc ";
		graphData += floatToStr( a1 );
		graphData += " ";
		graphData += floatToStr( a2 );
		graphData += "\r";
	}

	// Plot the Arc
	if ( !reverse )
	{
		for (int i=0; (i <= 2); i++) {
			graphData += floatToStr( p[i].x );
			graphData += " ";
			graphData += floatToStr( p[i].y );
			graphData += " ";
		}
	}
	else
	{
		for (int i=2; (i >= 0); i--) {
			graphData += floatToStr( p[i].x );
			graphData += " ";
			graphData += floatToStr( p[i].y );
			graphData += " ";
		}
	}
	graphData += "c\r";
}

void PDFMarker::PlotMoveTo( float x, float y )
{
	graphData += floatToStr( x );
	graphData += " ";
	graphData += floatToStr( y );
	graphData += " m\r";
}
