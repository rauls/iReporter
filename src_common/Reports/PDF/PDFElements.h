
#ifndef PDFELEMENTS_H
#define PDFELEMENTS_H

#include <string>
#include "PDFBase.h"
#include "PDFCore.h"
#include "PDFFont.h"
#include "PDFTableData.h"

#include "HelpCard.h"


class TableLinePosInfo
{
public:

	TableLinePosInfo( float x, float y )
	{
		currXPos = x;
		currYPos = y;
	}

public:
	float currXPos;
	float currYPos;
};


class TableTextPosInfo
{
public:

	TableTextPosInfo( float x, float y/*, int titleSizeP, int headingSizeP, int dataSizeP*/ )
	{
		lastXPos = x;
		lastYPos = y;
		currXPos = lastXPos;
		currYPos = lastYPos;
		currXOffset = currXPos;
		currYOffset = currYPos;
		lowestYCellDown = 0;
	}

	float lastXPos;
	float lastYPos;

	float currXPos;
	float currYPos;

	float currXOffset;
	float currYOffset;

	float movedX;

	float xAcross;
	float yDown;

	int currColumnNum;
	int numOfCols;

	float rowHeight;
	float colHeadingYPos;
	float dataYPos;

	int currLine;
	float yCellDown;
	float lowestYCellDown;

	void UpdateOffsets()
	{
		currXOffset = currXPos - lastXPos;
		currYOffset = currYPos - lastYPos;
		lastXPos = currXPos;
		lastYPos = currYPos;
	}
};

//
// PDF Table - used to represent a PDF table
//
class PDFTable
{
public:
	PDFTable( PDFTableData *theTableDataP, int numOfRowsP, std::string continuedOnNextP, std::string continuedFromPrevP, PDFSettings *thePDFSettingsP, PDFTableSettings *tableSetP, PDFCharScaleFont *timesNRFontP, int currPagePosP, int subPageNumP );
	~PDFTable();
	// Functions used to draw the table lines
	void	CreateHelpCard(class PDFFile* pPDFFile, const CQHelpCard& rCQHelpCard, double dWidth, bool bTableOn, bool bGraphOn);
	void	WriteTextFragments(const CQHelpCardText& rHCText, float fLineHeight, PDFTextList& rPDFTextList);

	void FormatTableData();
	std::string DrawTableLines( float &yPos );
	int TableHeadingColor( intList& cellColorList );
	float DrawTableTitleBackgroundColor( float x, float y );
	void DrawHorizontalLinesAndBackgrounds( TableLinePosInfo *linePosInfo, float& dataSectionTableHeight, float& currPageLeft );
	void DrawCellBackGroundColor( float x, float y, float height );
	void DrawHeadingRowBackGroundColor( float x, float y, float height );
	void DrawDataRowBackGroundColor( float x, float y, float height );
	void DrawVerticalLines( float x, float y, float height, int curveSize );
	void PlotLineTo( float x, float y );
	void PlotLine( float x, float y, float x2, float y2 );
	void PlotDrawLine( float x, float y, float x2, float y2 );
	void PlotFilledRect( float x, float y, float x2, float y2, long frcol );
	std::string GetRGBFillColorStr( long rgb );

	// Functions used to draw the table text
	std::string DrawTableText( float& yPos, std::string title, PDFBodyObjectsList& annotsList, PDFBodyObjectsList& annotsInternalLinksList, PDFPageObject *thePDFPageObject, std::string HTMLLinkFilename );
	void ReduceCellTextSize( int colNum, int allowedColWidth, std::string &cellText/*cellLineText*/, PDFCellTextStr *textStr );
	void WriteCellText( PDFPageObject *thePDFPageObject, PDFBodyObjectsList* pAnnotsList, TableTextPosInfo *posInfo, TableColWidth& allowedColWidth, std::string cellText/*cellLineText*/, PDFCellTextStr *textStr, int lineNum = 0 );
	void ContinuedFromPrevPage( float& currXPos, float& currYPos );
	void ContinuedOnNextPage( float xAcross, float yDown );
	void DisplayTitle( TableTextPosInfo& posInfo, std::string title, PDFBodyObjectsList& annotsInternalLinksList, PDFPageObject *thePDFPageObject, std::string HTMLLinkFilename );
	void CreateLinkToText( float x, float y, PDFPageObject *thePDFPageObject, PDFBodyObjectsList *listPtr, PDFCellTextStr *textStr );
	float LastTableHeight() { return lastTableHeight; }
	bool DataLeft()
	{ 
		if (currLineRowNum < numOfRows)
			return true;
		return false;
	}
	void PositionNextText( float x, float y )
	{
		tableLineData += floatToStr( x ); 
		tableLineData += " ";
		tableLineData += floatToStr( y ); 
		tableLineData += " Td\r"; 
	}
	void SubPageNum( int subPageNumP ) { subPageNum = subPageNumP; }
	int SubPageNum() { return subPageNum; }

public:
	PDFSettings *thePDFSettings;
	PDFTableData *theTableData;
	std::string tableLineData;
	std::string curveData;

private:
	PDFCharScaleFont *timesNRFont;
	int tableTitleBackColor;
	int tableColumnHeadingsBackColor;
	int numOfRows;
	int reduceStrCol;
	int currPagePos;
	std::string continuedOnNext;
	std::string continuedFromPrev;
	PDFCellTextStr* PDFError;
private:
	int subPageNum;
	PDFTableSettings *tableSet;
	bool continueFromPrevPage;
	int headingRowNum;
	int lineDataExtraRows;
	int textDataExtraRows;
	float lastTableHeight;
	int currLineRowNum;
	int currTextRowNum;
	int tableWidth;
	intList::iterator iter;
	int extraTextLen;
	float lineWidth;
	std::string tabSepText;
	int tabSepTextPos;
	int tabSepColHeadingPos;
	std::string intStr;
	std::string intToStr( int num )
	{
		sprintf( buf, "%d", num );
		intStr = buf;
		return intStr;
	}
	std::string floatToStr( float num )
	{
		sprintf( buf, "%.7g", num );
		intStr = buf;
		return intStr;
	}
	long currFillCol;
	std::string colBuf;
	char buf[32];
	int FontSize() { return timesNRFont->FontSize(); }
	int PageWidth() { return thePDFSettings->PageWidth(); } // the width of each page
	int PageHeight() { return thePDFSettings->PageHeight(); } // the length of each page
	int LeftMargin() { return thePDFSettings->LeftMargin(); }
	int SubPageLeftMargin();

	int RightMargin() { return thePDFSettings->RightMargin(); }
	int TopMargin() { return thePDFSettings->TopMargin(); }
	int BottomMargin() { return thePDFSettings->BottomMargin(); }
	int BannerTrailSpace() { return thePDFSettings->BannerTrailSpace(); }
	int GraphTrailSpace() { return thePDFSettings->GraphTrailSpace(); }
	int TableTrailSpace() { return thePDFSettings->TableTrailSpace(); }
	float PDFMakeYCoord( float yCo ) { return PageHeight() - yCo; }
public:
	float FontHeight() { return timesNRFont->FontHeight(); }
};


//
// PDF Graph - used to represent a PDF graph
//
class PDFGraph
{
public:
	PDFGraph( std::string graphDataP, int graphHeightP )//, int xOffsetP = 20, int yOffsetP = 24  )
	{
		graphData = graphDataP;
		graphHeight = graphHeightP;
	}
	int Height() { return graphHeight; }
	std::string Draw( int yPos )
	{
		return graphData;
	}

private:
	int xOffset;
	int yOffset;
	int pageHeight;
	int pageWidth;
	std::string graphData;
	int graphHeight;
};


class ColInfo
{
public:
	ColInfo( int colItems, int colLengthP ) { successiveColItems = colItems; colLength = colLengthP; }
	int successiveColItems;
	int colLength;
};
typedef std::list<ColInfo> ColInfoList;

//
// PDF Text - used to represent PDF text
//
class PDFText
{
public:
	PDFText( PDFSettings *thePDFSettingsP, PDFCharScaleFont *timesNRFontP, int subPageNumP )
		: PDF_TEXT_MAX_NUM_COLUMNS( 2 ), haveAlreadySplitText( false ), colSpacing( 5 )
	{ 
		thePDFSettings = thePDFSettingsP;
		timesNRFont = timesNRFontP;
		lastDrawYPos = 0;
		textData.reserve( 10 * 1024 );
		subPageNum = subPageNumP;
	}
	std::string Draw( PDFTextList& textListP, int specificYPos = 0, short vertCenter = 0, bool allowOutsideMargins = 0 );
	int LastDrawYPos() { return lastDrawYPos; }
	int SubPageNum() { return subPageNum; }
	void SubPageNum( int subPageNumP ) { subPageNum = subPageNumP; }

private:
	// Functions
	void TabSpace( int tab ) { tabSpace = tab; }
	int TabSpace() { return tabSpace; }
	int SubPageDrawWidth() { return thePDFSettings->SubPageDrawWidth(); }
	int ColumnSpace() { return (SubPageDrawWidth()-((PDF_TEXT_MAX_NUM_COLUMNS-1)*colSpacing))/PDF_TEXT_MAX_NUM_COLUMNS; }
	std::string SplitText( std::string& str, double lenWanted, int size /*= 0*/, short truncateTextLeftOrRight /*= PDF_LEFT_ELLIPSE*/, int style /*= PDF_NORMAL*/ );
	void SplitTextThatIsTooLong( PDFTextList& textList, bool indentSplitTextInColumns );
	void CalculateColumnLayoutInfo( PDFTextList& textList, ColInfoList& successiveColLengths );
	int GetDrawWidthMax( int textXPosType );
	void PlotDrawLine( float x, float y, float x2, float y2  );
	inline int GetTextHeight( PDFTextList& textList );
	std::string floatToStr( float num )
	{
		char buf[32];
		sprintf( buf, "%.3g", num );
		intStr = buf;
		return intStr;
	}
	int SubPageLeftMargin()
	{
		if ( thePDFSettings->SubPagesPerPage() > 1 && SubPageNum() > 1 )
		{
			int pageXPos, subPageMiddleMarginWidth, subPageWidth;
			subPageMiddleMarginWidth = thePDFSettings->RightMargin() * (thePDFSettings->SubPagesPerPage()-1);
			subPageWidth = (thePDFSettings->DrawWidth() - subPageMiddleMarginWidth) / thePDFSettings->SubPagesPerPage();
			pageXPos = thePDFSettings->LeftMargin() + ( (SubPageNum()-1) * (subPageWidth + thePDFSettings->RightMargin()) );
			return pageXPos;
		}
		else
			return thePDFSettings->LeftMargin();
	}
	int PageHeight() { return thePDFSettings->PageHeight(); }
	int BottomMargin() { return thePDFSettings->BottomMargin(); }
	int TopMargin() { return thePDFSettings->TopMargin(); }
	int DrawHeight() { return thePDFSettings->DrawHeight(); }
	// Data
	PDFSettings *thePDFSettings;
	PDFCharScaleFont *timesNRFont;
	std::string textData;
	std::string lineData;
	int tabSpace;
	std::string intStr;
	int lastDrawYPos;
	int currFontSize;
	int subPageNum;
	const short PDF_TEXT_MAX_NUM_COLUMNS;
	const short colSpacing;
	bool haveAlreadySplitText;
};

typedef struct {
	float x;
	float y;
} PDFPoint, *PDFPointPtr;

class PDFMarker
{
public:
	void DrawQuarterCircle( float x1, float y1, float x2, float y2 );
	void DrawCurve( float x1, float y1, float x2, float y2, float cx, float cy, float rad, double yScale, double angle1, double angle2 );
	void CalculateArc( PDFPointPtr p, float cxP, float cyP, float xP, float yP, double yScale, double a1, double a2, float rad );
	void PlotArc( PDFPointPtr p, double a1, double a2, short reverse = 0 );
	void PlotMoveTo( float x, float y );
	std::string floatToStr( float num )
	{
		char buf[32];
		sprintf( buf, "%g", num );
		return buf;
	}
	std::string graphData;
};


#endif // PDFELEMENTS_H
