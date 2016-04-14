
#ifndef PDFFILE_H
#define PDFFILE_H

#include <map>
#include <vector>
#include "PDFCore.h"
#include "PDFTableData.h"

class PDFImageObjectList
{
public:
	PDFImageObjectList( PDFImageObject *imageObjectP, PDFASCII85DecodeObject *ASCII85DecodeObjectP, PDFColorSpaceObject *colorSpaceObjectP, int subPageNumP = 1 )
	{
		imageObject = imageObjectP;
		ASCII85DecodeObject = ASCII85DecodeObjectP;
		colorSpaceObject = colorSpaceObjectP;
		subPageNum = subPageNumP;
	}
	~PDFImageObjectList(){}

	PDFImageObject *imageObject;
	PDFASCII85DecodeObject *ASCII85DecodeObject;
	PDFColorSpaceObject *colorSpaceObject;
	int subPageNum;
	std::string imagePosData;
	std::string imageData;
};
typedef std::vector< PDFImageObjectList > ImagesList;

#define PDF_MAX_SUBPAGES 2

//
// PDF File - The main class which represents all the essential (and a few optional) features of a PDF file
//
class PDFFile 
{
public:
	PDFFile( PDFSettings thePDFSettingsP, PDFTableSettings thePDFTableSettingsP );
	~PDFFile();
	void Initialise();
	bool Open( const char *fileName = "C:\temp\report.pdf", FILE *filePtr = NULL );
	bool WillDataFitOnCurrentPage( int lengthOfData );
	void CreateURIObjects( std::string text, std::string url );
	inline void IncrementPDFFieldObjectId();
	inline void DecrementPDFFieldObjectId();
	inline int PDFFieldObjectId();

	void WriteHelpCard(FILE *fp, const class CQHelpCard& rCQHelpCard, bool bTableOn, bool bGraphOn);
	PDFImageObjectList*	m_imageOverview;
	PDFImageObjectList*	m_imageGraph;
	PDFImageObjectList*	m_imageTable;


	void CreateInternalLinkObjects( std::string text, std::string url );
	void CreateAPageObject();
	void AddPageToPageListObject();
	void WritePageObjects();
	void CreatePageListObject();
	void WritePageListObject();
	void WritePDFHeader();
	void CreateDataContentsObject();
	void WriteDataContentsObject();
	void CreateResourcesObject();
	void WriteResourcesObject();
	void CreateExtGStateObject();
	void WriteExtGStateObject();
	void CreateFontObject();
	void WriteFontObject();
	void CreateEncodingObject();
	void WriteEncodingObject();
	void WriteCurrentPage();
	void WriteImageData( PDFImageObjectList& imageObjectList );
	void CreateHalftone();
	void WriteHalftone();
	void CreateInfoObject();
	void WriteInfoObject();
	void WriteXRef();
	void CreateCatelogObject();
	void WriteCatelogObject();
	void WriteOutlinesObject();
	void WriteTrailer();
	void WriteRemainingPDF();
	bool Close();
	void DeletePDFObjects();
	void DrawGraph( std::string graphDataP, int graphHeightP);/*, std::string tableColHeadings, std::string text, intList colWidths, int numOfRows*///, int xOffsetP = 40, int yOffsetP = 24 );
	void DrawTable( int numOfRows, std::string continuedOnNextP, std::string continuedFromPrevP, std::string HTMLLinkFilename, int outlineLevel );
	void ResetTableDetails();
	void DisplayPageNumbering();
	void DisplayDemoString();
	void DrawPDFText( PDFTextList& summaryPDFTextList, int Ypos = 0, int vertCenter = 0 );
	void SessionOutline( const char *text );
	void SessionHeading( std::string text );
//	void SessionText( std::string text, int size = 6, int xPos = PDF_TEXT_COLUMN, int state = PDF_NORMAL );
	std::string AddPDFText( PDFTextList& summaryPDFTextList );
	int AddOutline( std::string &text, int level = 0, int forceLink = 0 );
	int DrawImage( const char* filename );
	void DrawPageBanner( float x, float y );
	void DrawDefaultBanner( float x, float y );
	PDFImageObjectList* CreatePDFImageObjects( char *imageName );
	PDFImageObjectList *CreateAPDFImageObject( const char *name );
	PDFImageObjectList* CreateNewPDFImageObject( const char *name );

	int InsertJPEGImageFromFile( const char *filename, PDFImageObjectList *imageObjectList );
	void PositionImage( std::string& posData, float x, float y, float width, float height );
	int CalculateColumnWidths( intList& colWidths, IndexedInts& colDataWidths );
	void FrontPageTableTitle( const char * text );
	void FrontPageTableEntry( const char *text, const char *number );
	void FrontPageTableSubTitle( const char *text );
	void SetDocumentTitle( std::string txt ) { docTitle = txt; }
	std::string GetDocumentTitle();
	double GetStringLen( std::string str );

public:
	int PageWidth() { return thePDFSettings.PageWidth(); } // the width of each page
	int PageHeight() { return thePDFSettings.PageHeight(); } // the length of each page
	int LeftMargin() { return thePDFSettings.LeftMargin(); }
	int RightMargin() { return thePDFSettings.RightMargin(); }
	int TopMargin() { return thePDFSettings.TopMargin(); }
	int BottomMargin() { return thePDFSettings.BottomMargin(); }
	int BannerTrailSpace() { return thePDFSettings.BannerTrailSpace(); }
	int GraphTrailSpace() { return thePDFSettings.GraphTrailSpace(); }
	int TableTrailSpace() { return thePDFSettings.TableTrailSpace(); }
	int ShowBanner() { return thePDFSettings.ShowBanner(); }
	char* BannerFile() { return thePDFSettings.BannerFile(); }
	char* SkipPastPath( char *filename );
	int DrawWidth() { return thePDFSettings.DrawWidth(); }
	int DrawHeight() { return thePDFSettings.DrawHeight(); }
	int FontSize() { return timesNRFont->FontSize(); }
	float FontHeight() { return timesNRFont->FontHeight(); }
	int SubPageDrawWidth();
	int SubPageLeftMargin();

	std::string intToStr( int num )
	{
		static std::string intStr;
		static char buf[32];
		sprintf( buf, "%d", num );
		intStr = buf;
		return intStr;
	}
	std::string floatToStr( float num )
	{
		static std::string intStr;
		static char buf[32];
		sprintf( buf, "%.7g", num );
		intStr = buf;
		return intStr;
	}

	int numOfRows;
	int numOfCols;
	int dataColNum;
	IndexedInts colDataWidths;
	const char *PageData( short page = 0 );
	void PageDataAdd( std::string str, short page = 0 );
	void PageDataReset( short page = 0 );
	long PageDataLength( short page = 0 );

public:
	// Functions
	float& CurrYPos() { return currY; }
	bool StartNewSectionInReport();
	void StartNewSectionInReport( bool state ) {	startNewSectionInReport = state; }
	bool ShowGraph() { return showGraph; }
	void ShowGraph( bool state ) { showGraph = state; }
	void SubPageNum( int subPageNumP );// { if ( subPageNumP < PDF_MAX_SUBPAGES ) subPageNum = subPageNumP; }
	int SubPageNum() { return subPageNum; }
	void AdjustTableSettingsForSingleCellFlowingText();
	void AdjustTableSettingsBackAfterSingleCellFlowingText();

	// Data
	FILE *theFP; // the PDF output file ptr 
	int pdfFieldObjectId; // current pdf field object Id
	int byteOffset; // The current byte offset position in the PDF output file
	int XRefPosition; // The position where the cross reference table starts

	PDFSettings thePDFSettings;
	PDFTableSettings thePDFTableSettings;
	int SummaryMainTitleSize();
	int SummarySubTitleSize();
	int SummaryNormalSize();
	void WriteFrontPage( char* );

	int pageNum;
	bool forceDraw;
	int graphNum;
	int tableNum;
	int tableNumInSection;
	short outlineDone;

	PDFBodyObjectsList PDFBodyObjects; // The list of objects
	// The PDF field objects used which need to be maintained for cross-referrencing
	PDFBodyObject *aPDFBodyObject;
	PDFPageObject *thePDFPageObject; // the current page object which is being worked on
	PDFDataContentObject *thePDFDataContentObject; // the current data content object which is being worked on
	PDFResourcesObject *thePDFResObject; // the current resource object which is being worked on
	PDFExtGState *thePDFExtGStateObject; // the ExtGState object, only one is created
	bool thePDFExtGStateObjectAlreadyWritten;
	PDFPagesListObject *thePDFPagesListObject; // the current page list object which is being worked on
	PDFCatelogObject *thePDFCatelogObject;
	PDFOutlinesObject *thePDFOutlinesObject;
	PDFInfoObject *thePDFInfoObject;
	PDFFontObject *thePDFNormalFontObject;
	PDFFontObject *thePDFBoldItalicFontObject;
	PDFFontObject *thePDFBoldFontObject;
	PDFFontObject *thePDFItalicFontObject;
	PDFEncodingObject *thePDFEncodingObject;
	PDFHalftone *thePDFHalftone;
	PDFImageObject *thePDFImageObject;
	PDFASCII85DecodeObject *thePDFASCII85DecodeObject;
	PDFColorSpaceObject *thePDFColorSpaceObject;

	PDFStrIntList thePDFFontList;
//	PDFLinkObjectsList currLinkObjectList;
	PDFTextList summaryPDFTextList;
//	ClickStreamTextLists clickStreamTextLists;
	ImagesList imagesList;
	ImagesList imageListToKeepToWriteAtEnd;
	intList tableWidths; // list of positions for table text data (Tab Stop)

	std::string pageNumAtTopData[PDF_MAX_SUBPAGES];
	std::string demoData;
	int demoStringYOffset;
	bool demoOffsetDoneForThisPage;
	bool displayDemo;

	std::string fileData; // string to hold data before output to the pdf file
	std::string pageData[PDF_MAX_SUBPAGES];
	std::string buffer; // buffer for 

	std::string bannerData;
	std::string bannerPagePosData[PDF_MAX_SUBPAGES];
	int yMaintainAspectRatioFromOrigGIF;

	std::string tableTitle;
	PDFTableData theTableData;
	std::string currSectionName;
	void LinkOutlineToPage( int currPageNum );
	std::string sectionName;
	PDFBodyObjectsList annotsList;
	PDFBodyObjectsList annotsInternalLinksList;

	PDFCharScaleFont *timesNRFont;
	std::string docTitle;
	bool startNewSectionInReport;
	bool showGraph;

	void CurrYPos( float currYP ) { currY = currYP; }
	int currPage; // current page writing too
	float currX; // current X position
	float currY; // current Y position
	int subPageNum;
};

#endif // PDFFILE_H
