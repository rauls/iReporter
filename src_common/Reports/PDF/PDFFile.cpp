#include "Compiler.h"

#include "PDFFile.h"
#include "PDFElements.h"
#include "PDFImages.h"
#include "myansi.h"


//
// PDFFile class functions
//
extern "C" int CreatePDFImageFromFile(int argc, const char **argv, PDFImageJpeg *pdfImageJpeg);

PDFFile::PDFFile( PDFSettings thePDFSettingsP, PDFTableSettings thePDFTableSettingsP ) 
	: theTableData( thePDFTableSettingsP.minimumTableColumnHeadingChars,
					(100-thePDFTableSettingsP.reduceBiggestDataColWidthByPercent)/100,
					thePDFTableSettingsP.textWrap, thePDFTableSettingsP.textCellXIndent )
{
	m_imageOverview	= NULL;
	m_imageGraph	= NULL;	
	m_imageTable	= NULL;	

	// Set the passed values
	thePDFSettings = thePDFSettingsP;
	thePDFTableSettings = thePDFTableSettingsP;
	PDFTableSettings *p = &thePDFTableSettings;

	// Initialise what should ONLY be initialised ONCE
	timesNRFont = NULL;
	switch( thePDFSettings.Font() )
	{
	case PDF_FONT_TIMESNEWROMAN:
		timesNRFont = new PDFTimesNewRomanFont( THEFONTNAME, thePDFTableSettings.DataSize() );
		break;
	case PDF_FONT_HELVETICA:
		timesNRFont = new PDFHelveticaFont( "Helvetica", thePDFTableSettings.DataSize() );
		break;
	default:
		timesNRFont = new PDFTimesNewRomanFont( THEFONTNAME, thePDFTableSettings.DataSize() );
	}

	theTableData.SetPDFFont( timesNRFont );
	int subPages = thePDFSettings.SubPagesPerPage();
	theTableData.SetFonts( p->TitleSize(), p->TitleStyle(), p->TitleColor(),
						   p->ColHeadingSize(), p->ColHeadingStyle(), p->ColHeadingColor(),
						   p->DataSize(), p->DataStyle(), p->DataColor() );


	p->textCellXIndent = timesNRFont->FontSpacing( p->DataSize() ) / 2;
	theTableData.textCellXIndent = p->textCellXIndent;

//	demoStringYOffset = 0;
//	displayDemo = false;

	// Initialise stuff that is repeatable
	Initialise();
}

void PDFFile::Initialise()
{
	theFP = NULL;
	currPage = 1;
	pageNum = 0;
	SubPageNum( 1 );
	graphNum = 0;
	tableNum = 0;
	currX = 0;
	CurrYPos( 0 );
	forceDraw = false;

	pdfFieldObjectId = 0;
	byteOffset = 0;
	XRefPosition = 0;

	DeletePDFObjects();
	aPDFBodyObject = NULL;
	thePDFPageObject = NULL;
	thePDFDataContentObject = NULL;
	thePDFResObject = NULL;
	thePDFExtGStateObject = NULL;
	thePDFExtGStateObjectAlreadyWritten = false;
	thePDFPagesListObject = NULL;
	thePDFCatelogObject = NULL;
	thePDFOutlinesObject = NULL;
	thePDFInfoObject = NULL;
	thePDFNormalFontObject = NULL;
	thePDFBoldItalicFontObject = NULL;
	thePDFBoldFontObject = NULL;
	thePDFItalicFontObject = NULL;
	thePDFEncodingObject = NULL;
	thePDFHalftone = NULL;
	thePDFImageObject = NULL;
	thePDFASCII85DecodeObject = NULL;
	thePDFColorSpaceObject = NULL;

	imagesList.clear();
	imageListToKeepToWriteAtEnd.clear();

	demoData = PDF_BLANK;
	demoStringYOffset = 0;
	displayDemo = false;
	demoOffsetDoneForThisPage = false;

	fileData = PDF_BLANK;
	pageData[0] = PDF_BLANK;
	pageData[1] = PDF_BLANK;
	buffer = PDF_BLANK;

	bannerData = PDF_BLANK;
	yMaintainAspectRatioFromOrigGIF = 0;
	tableTitle = PDF_BLANK;
	currSectionName = PDF_BLANK;
	outlineDone = 0;

	tableNumInSection = 0;

	thePDFFontList.clear();
//	currLinkObjectList.clear();
	summaryPDFTextList.clear();
	imagesList.clear();
	imageListToKeepToWriteAtEnd.clear();
	tableWidths.clear();


	fileData.reserve(5*1024);
	buffer.reserve(5*1024);
}


PDFFile::~PDFFile()
{
	Close();

	delete timesNRFont;
}


void PDFFile::DeletePDFObjects()
{
	int count = 0;
	// Delete all the body objects used
	PDFBodyObject *aPDFBodyObject;
	while ( PDFBodyObjects.size() != 0 )
	{
		count++;
		aPDFBodyObject = PDFBodyObjects.back();
		delete aPDFBodyObject;
		PDFBodyObjects.pop_back();
	}
	PDFBodyObjects.clear();
}

bool PDFFile::Open( const char *fileName /*= PDF_BLANK*/, FILE *filePtr /*= NULL*/ )
{
	if ( filePtr == NULL)
		theFP = fopen( fileName, "w" );
	else
		theFP = filePtr;
	if (theFP)
	{
		// Header
		WritePDFHeader();
		return true;
	}
	return false;
}

bool PDFFile::WillDataFitOnCurrentPage( int lengthOfData )
{
	// Check positioning info to see if we need to start a new page
	if (!thePDFPageObject)
		CreateAPageObject();
	
	if (thePDFPageObject)
	{
		if ( CurrYPos() + lengthOfData + BottomMargin() > PageHeight() )
			return false;
		else
			return true;
	}
	return false;
}

void PDFFile::IncrementPDFFieldObjectId()
{
	pdfFieldObjectId++;
}

void PDFFile::DecrementPDFFieldObjectId()
{
	pdfFieldObjectId--;
}

int PDFFile::PDFFieldObjectId()
{
	return pdfFieldObjectId;
}

void PDFFile::CreateInternalLinkObjects( std::string text, std::string internalLink )
{
	// Create a 
	IncrementPDFFieldObjectId();
	PDFAnnotObject *aPDFAnnotObject = new PDFAnnotInternalLinkObject( pdfFieldObjectId, 0, 'n', text, internalLink );
	PDFBodyObjects.push_back( (PDFBodyObject*) aPDFAnnotObject );

	annotsInternalLinksList.push_back( (PDFBodyObject*) aPDFAnnotObject );
}

void PDFFile::CreateURIObjects( std::string text, std::string url )
{
	// Create a 
	IncrementPDFFieldObjectId();
	PDFURIObject *aPDFURIObject = new PDFURIObject( pdfFieldObjectId, 0, 'n', text, url );
	PDFBodyObjects.push_back( (PDFBodyObject*) aPDFURIObject );

	IncrementPDFFieldObjectId();
	PDFAnnotObject *aPDFAnnotObject = new PDFAnnotURILinkObject( pdfFieldObjectId, 0, 'n', aPDFURIObject );
	PDFBodyObjects.push_back( (PDFBodyObject*) aPDFAnnotObject );

	annotsList.push_back( (PDFBodyObject*) aPDFAnnotObject );
}

void PDFFile::CreateAPageObject()
{
	// Create a XRef entry Object for the Page object
	if (!thePDFPageObject)
	{
		pageNum++;
		currPage++;
		SubPageNum( 1 );
		currX = 0;
		CurrYPos( TopMargin() );
		IncrementPDFFieldObjectId();
		thePDFPageObject = new PDFPageObject( pdfFieldObjectId, 0, 'n', thePDFSettings.CommentsOn(), pageNum );
		// Add the page object to the list of page objects
		PDFBodyObjects.push_back( thePDFPageObject );
		DisplayPageNumbering();
		DrawPageBanner( SubPageLeftMargin(), CurrYPos() );
		//CurrYPos( TopMargin() );
		demoOffsetDoneForThisPage = false;
	}
	//CurrYPos( thePDFSettings.topMargin );
	//DrawPageBanner( thePDFSettings.leftMargin, thePDFSettings.topMargin );

}


void PDFFile::AddPageToPageListObject()
{
	CreatePageListObject();
	// Add
	thePDFPageObject->DataContentObjectNum( thePDFDataContentObject->Id() );
	thePDFPageObject->ResourceObjectNum( thePDFResObject->Id() );
	thePDFPageObject->ParentObjectNum( thePDFPagesListObject->Id() );
	thePDFPagesListObject->AddPage( thePDFPageObject );
}

void PDFFile::WritePageObjects()
{
	if (!thePDFPagesListObject)
		return;

	// Write out all of the Pages Objects
	PDFPageList::iterator iter = thePDFPagesListObject->StartPage();
	while( iter != thePDFPagesListObject->EndPage() )
	{
		PDFPageObject *aPDFPageObject = (*iter);
		if ( aPDFPageObject->annotsList.size() )
		{
			for( PDFBodyObjectsList::iterator annotsIter = aPDFPageObject->annotsList.begin(); annotsIter != aPDFPageObject->annotsList.end(); annotsIter++ )
			{
				PDFAnnotObject* aPDFAnnotObject = (PDFAnnotObject*)(*annotsIter);
				if ( aPDFAnnotObject->URIObj )
				{
					aPDFAnnotObject->URIObj->ByteOffset( byteOffset );
					fileData = aPDFAnnotObject->URIObj->ObjectEntry();
					byteOffset += fileData.length(); 
					fputs(fileData.c_str(), theFP);
				}

				aPDFAnnotObject->ByteOffset( byteOffset );
				fileData = aPDFAnnotObject->ObjectEntry();
				byteOffset += fileData.length(); 
				fputs(fileData.c_str(), theFP);
			}
		}

		aPDFPageObject->ByteOffset( byteOffset );
		fileData = aPDFPageObject->ObjectEntry();
		byteOffset += fileData.length(); 
		fputs(fileData.c_str(), theFP);
		iter++;
	}
}

void PDFFile::CreatePageListObject()
{
	if (!thePDFPagesListObject)
	{
		IncrementPDFFieldObjectId();
		thePDFPagesListObject = new PDFPagesListObject( pdfFieldObjectId, 0, 'n', PageHeight(), PageWidth() );
		PDFBodyObjects.push_back( (PDFBodyObject*)thePDFPagesListObject );
	}
}

void PDFFile::WritePageListObject()
{
	if (!thePDFPagesListObject)
		return;

	// Write out the PageList Object
	thePDFPagesListObject->ByteOffset( byteOffset );
	fileData = thePDFPagesListObject->ObjectEntry();
	byteOffset += fileData.length(); 
	fputs(fileData.c_str(), theFP);
	thePDFPagesListObject->EraseContents();
}

void PDFFile::WritePDFHeader()
{
	if (!theFP)
		return;

	// Create & Write the Header data
	PDFHeader header;
	fileData = header.ObjectEntry();
	fputs(fileData.c_str(), theFP);
	byteOffset = header.len();// - 1/*2/* for the extra %'s used in the string */;

	// Create the compulsory "Free" Object with ID = 0, 65535 etc...
	PDFBodyObject *aPDFBodyObject = new PDFBodyObject( 0, 65535, 'f' );
	aPDFBodyObject->ByteOffset( 0 );
	PDFBodyObjects.push_back( aPDFBodyObject );
}

void PDFFile::CreateDataContentsObject()
{
	// Create a XRef entry Object for the Contents object
	IncrementPDFFieldObjectId();
	thePDFDataContentObject = new PDFDataContentObject( pdfFieldObjectId, 0, 'n', thePDFSettings.CommentsOn(), pageNum );
	PDFBodyObjects.push_back( thePDFDataContentObject );
}

void PDFFile::WriteDataContentsObject()
{
	if (!thePDFDataContentObject)
		return;

	// Write out the Content
//	DisplayPageNumbering();
	for ( int page = 0; page < SubPageNum(); page++ )
	{
		thePDFDataContentObject->AddText( pageNumAtTopData[page] );
		thePDFDataContentObject->AddText( bannerPagePosData[page] );
		thePDFDataContentObject->AddText( bannerData );

		if (m_imageOverview && m_imageOverview->imagePosData.size())
		{
			thePDFDataContentObject->AddText( m_imageOverview->imagePosData );
			thePDFDataContentObject->AddText( m_imageOverview->imageData );
		}
		if (m_imageGraph && m_imageGraph->imagePosData.size())
		{
			thePDFDataContentObject->AddText( m_imageGraph->imagePosData );
			thePDFDataContentObject->AddText( m_imageGraph->imageData );
		}
		if (m_imageTable&& m_imageTable->imagePosData.size())
		{
			thePDFDataContentObject->AddText( m_imageTable->imagePosData );
			thePDFDataContentObject->AddText( m_imageTable->imageData );
		}

		for ( int image = 0; image < imagesList.size(); image++ )
		{
			if ( imagesList[image].subPageNum == page+1 )
			{
				PDFImageObjectList&	rList = imagesList[image];
				thePDFDataContentObject->AddText( rList.imagePosData );
				thePDFDataContentObject->AddText( rList.imageData );
			}
		}
	}

	thePDFDataContentObject->AddText( demoData );
	thePDFDataContentObject->AddDataContentsData( PageData( 1 ), PageData( 2 ) );
	thePDFDataContentObject->ByteOffset( byteOffset );
	fileData = thePDFDataContentObject->ObjectEntry();
	byteOffset += fileData.length(); 
	bool error = false;
	int len = fputs(fileData.c_str(), theFP);
	if ( len != fileData.length() )
		error = true;
	PageDataReset( 1 );
	PageDataReset( 2 );
	thePDFDataContentObject->EraseContents();
}

void PDFFile::CreateResourcesObject()
{
	// Create a XRef entry Object for the Resources object
	IncrementPDFFieldObjectId();
	thePDFResObject = new PDFResourcesObject( pdfFieldObjectId, 0, 'n' );
	PDFBodyObjects.push_back( (PDFBodyObject*)thePDFResObject );
}

void PDFFile::WriteResourcesObject()
{
	if (!thePDFResObject)
		return;

	if (thePDFImageObject)
	{
		ImageLinkData imageLinkData( thePDFImageObject->GetImageName(), thePDFImageObject->Id(), thePDFColorSpaceObject->Id() );
		thePDFResObject->AddImage( imageLinkData );
	}

	// ******************************************************************
	// Write the Icon references if they have been given locations.
	// ******************************************************************
	if (m_imageOverview && m_imageOverview->imagePosData.size())
	{
		ImageLinkData imageLinkData( m_imageOverview->imageObject->GetImageName(), m_imageOverview->imageObject->Id(), m_imageOverview->colorSpaceObject->Id() );
		thePDFResObject->AddImage( imageLinkData );
		m_imageOverview->imagePosData = "";
	}
	if (m_imageGraph && m_imageGraph->imagePosData.size())
	{
		ImageLinkData imageLinkData( m_imageGraph->imageObject->GetImageName(), m_imageGraph->imageObject->Id(), m_imageGraph->colorSpaceObject->Id() );
		thePDFResObject->AddImage( imageLinkData );
		m_imageGraph->imagePosData = "";
	}
	if (m_imageTable && m_imageTable->imagePosData.size())
	{
		ImageLinkData imageLinkData( m_imageTable->imageObject->GetImageName(), m_imageTable->imageObject->Id(), m_imageTable->colorSpaceObject->Id() );
		thePDFResObject->AddImage( imageLinkData );
		m_imageTable->imagePosData = "";
	}

	for ( int i = 0; i < imagesList.size(); i++ )
	{
		ImageLinkData imageLinkData( imagesList[i].imageObject->GetImageName(), imagesList[i].imageObject->Id(), imagesList[i].colorSpaceObject->Id() );
		thePDFResObject->AddImage( imageLinkData );
		imageListToKeepToWriteAtEnd.push_back( imagesList[i] );
	}
	imagesList.clear();

	// Now write out the Resources object
	thePDFResObject->ByteOffset( byteOffset );
	fileData = thePDFResObject->ObjectEntry();
//	thePDFResObject->SetFontName( fontName );
	byteOffset += fileData.length(); 
	fputs(fileData.c_str(), theFP);
	thePDFResObject->EraseContents();
}

void PDFFile::CreateExtGStateObject()
{
	if (!thePDFExtGStateObject)
	{
		// Create a XRef entry Object for the ExtGState object
		IncrementPDFFieldObjectId();
		thePDFExtGStateObject = new PDFExtGState( pdfFieldObjectId, 0, 'n' );
		PDFBodyObjects.push_back( (PDFBodyObject*)thePDFExtGStateObject );
	}
	thePDFResObject->SetExtGStateNum( thePDFExtGStateObject->Id() );
}

void PDFFile::WriteExtGStateObject()
{
	if (!thePDFExtGStateObjectAlreadyWritten && thePDFExtGStateObject)
	{
		// Now write out the ExtGState object
		thePDFExtGStateObjectAlreadyWritten = true;
		thePDFExtGStateObject->ByteOffset( byteOffset );
		fileData = thePDFExtGStateObject->ObjectEntry();
		byteOffset += fileData.length(); 
		fputs(fileData.c_str(), theFP);
		thePDFExtGStateObject->EraseContents();
	}
}

void PDFFile::CreateFontObject()
{
	if (!thePDFNormalFontObject)
	{
		/*	thePDFBoldItalicFontObject
		thePDFBoldFontObject
		thePDFItalicFontObject*/
	
		std::string refFontName;
	//	PDFStrIntList thePDFFontList;

		PDFFontObject *aPDFFontObject;
		for( PDFStrIntList::iterator iter = timesNRFont->StyleList().begin(); iter != timesNRFont->StyleList().end(); iter++ )
		{
			int style = (*iter).theInt;
			refFontName = timesNRFont->FontName();
			if ( !(*iter).theString.empty() )
			{
				refFontName += (*iter).theString; 
			}
			IncrementPDFFieldObjectId();
			
			switch( thePDFSettings.Font() )
			{
			case PDF_FONT_HELVETICA:
				aPDFFontObject = new PDFFontObject( pdfFieldObjectId, 0, 'n', refFontName, "Helvetica", style );
				break;
			case PDF_FONT_TIMESNEWROMAN:
			default:
				aPDFFontObject = new PDFFontObject( pdfFieldObjectId, 0, 'n', refFontName, "Times", style );
			}

			PDFBodyObjects.push_back( (PDFBodyObject*)aPDFFontObject );
			thePDFFontList.push_back( PDFStrInt( refFontName, aPDFFontObject->Id() ) );
			switch( style )
			{
			case PDF_NORMAL:		thePDFNormalFontObject = aPDFFontObject; break;
			case PDF_BOLD:			thePDFBoldFontObject   = aPDFFontObject; break;
			case PDF_ITALIC:		thePDFItalicFontObject = aPDFFontObject; break;
			case PDF_BOLDITALIC:	thePDFBoldItalicFontObject = aPDFFontObject; break;
			default: continue;
			}
		}

		/*if (!thePDFNormalFontObject)
		{
			IncrementPDFFieldObjectId();
			specialFontName = timesNRFont->FontName();
			thePDFNormalFontObject = new PDFFontObject( pdfFieldObjectId, 0, 'n', specialFontName, 0 );
			PDFBodyObjects.push_back( (PDFBodyObject*)thePDFNormalFontObject );
			aPDFFontList.push_back( PDFStrInt( specialFontName, thePDFNormalFontObject->Id() ) );
		}
		if (!thePDFBoldItalicFontObject)
		{
			IncrementPDFFieldObjectId();
			specialFontName = timesNRFont->FontName() + timesNRFont->StyleName( PDF_BOLDITALIC );
			thePDFBoldItalicFontObject = new PDFFontObject( pdfFieldObjectId, 0, 'n', specialFontName, PDF_BOLD & PDF_ITALIC );
			PDFBodyObjects.push_back( (PDFBodyObject*)thePDFBoldItalicFontObject );
			aPDFFontList.push_back( PDFStrInt( specialFontName, thePDFBoldItalicFontObject->Id() ) );
		}
		if (!thePDFBoldFontObject)
		{
			IncrementPDFFieldObjectId();
			specialFontName = timesNRFont->FontName() + timesNRFont->StyleName( PDF_BOLD );
			thePDFBoldFontObject = new PDFFontObject( pdfFieldObjectId, 0, 'n', specialFontName, PDF_BOLD );
			PDFBodyObjects.push_back( (PDFBodyObject*)thePDFBoldFontObject );
			aPDFFontList.push_back( PDFStrInt( specialFontName, thePDFBoldFontObject->Id() ) );
		}
		if (!thePDFItalicFontObject)
		{
			IncrementPDFFieldObjectId();
			specialFontName = timesNRFont->FontName() + timesNRFont->StyleName( PDF_ITALIC );
			thePDFItalicFontObject = new PDFFontObject( pdfFieldObjectId, 0, 'n', specialFontName, PDF_ITALIC );
			PDFBodyObjects.push_back( (PDFBodyObject*)thePDFItalicFontObject );
			aPDFFontList.push_back( PDFStrInt( specialFontName, thePDFItalicFontObject->Id() ) );
		}*/
	}
	/*if (!thePDFFontObject)
	{
		IncrementPDFFieldObjectId();
		thePDFFontObject = new PDFFontObject( pdfFieldObjectId, 0, 'n', fontName );
		PDFBodyObjects.push_back( (PDFBodyObject*)thePDFFontObject );
	}*/
	thePDFResObject->SetFontList( thePDFFontList );
	//thePDFResObject->SetFontObjNum( thePDFFontObject->Id() );
}

void PDFFile::WriteFontObject()
{
//	if ( !thePDFFontObject )
	if ( !thePDFNormalFontObject )
		return;

/*	thePDFNormalFontObject
	thePDFBoldItalicFontObject
	thePDFBoldFontObject
	thePDFItalicFontObject*/

	PDFFontObject *aPDFFontObject;
	if (thePDFNormalFontObject)
	{
		for( PDFStrIntList::iterator iter = timesNRFont->StyleList().begin(); iter != timesNRFont->StyleList().end(); iter++ )
		{
			int style = (*iter).theInt;
			switch( style )
			{
			case PDF_NORMAL: aPDFFontObject = thePDFNormalFontObject; break;
			case PDF_BOLD: aPDFFontObject = thePDFBoldFontObject; break;
			case PDF_ITALIC: aPDFFontObject = thePDFItalicFontObject; break;
			case PDF_BOLDITALIC: aPDFFontObject = thePDFBoldItalicFontObject; break;
			}
			if (aPDFFontObject)
			{
				aPDFFontObject->EncodingObject( thePDFEncodingObject->Id() );
				aPDFFontObject->ByteOffset( byteOffset );
				fileData = aPDFFontObject->ObjectEntry();
				byteOffset += fileData.length(); 
				fputs(fileData.c_str(), theFP);
				aPDFFontObject->EraseContents();
			}
		}
	}

/*	thePDFFontObject->EncodingObject( thePDFEncodingObject->Id() );
	thePDFFontObject->ByteOffset( byteOffset );
	fileData = thePDFFontObject->ObjectEntry();
	byteOffset += fileData.length(); 
	fputs(fileData.c_str(), theFP);
	thePDFFontObject->EraseContents();*/
}

void PDFFile::CreateEncodingObject()
{
	IncrementPDFFieldObjectId();
	thePDFEncodingObject = new PDFEncodingObject( pdfFieldObjectId, 0, 'n' );
	PDFBodyObjects.push_back( (PDFBodyObject*)thePDFEncodingObject );
}

void PDFFile::WriteEncodingObject()
{
	if (!thePDFEncodingObject)
		return;

	thePDFEncodingObject->ByteOffset( byteOffset );
	fileData = thePDFEncodingObject->ObjectEntry();
	byteOffset += fileData.length();
	fputs(fileData.c_str(), theFP);
	thePDFEncodingObject->EraseContents();
}

void PDFFile::WriteCurrentPage()
{
	static long len;
	static long lastLen = 0;

	// Check that we have a page
	if (!thePDFPageObject)
		return;

	len = PageDataLength();
	if ( len == 0)
	{
		lastLen = len;
		return;
	}

	if ( SubPageNum() < thePDFSettings.SubPagesPerPage() && !forceDraw )
	{
		if ( lastLen != len ) // Making sure we only increment SubPageNum() count if the page has changed...
		{
			SubPageNum( SubPageNum()+1 );
			lastLen = len;
		}
		CurrYPos( TopMargin() );
		currX = SubPageLeftMargin();
		DisplayPageNumbering();
		DrawPageBanner( SubPageLeftMargin(), CurrYPos() );
		return;
	}

	if ( thePDFOutlinesObject )
		thePDFOutlinesObject->ClearCurrLinkObjectList();

	forceDraw = false;
//	SubPageNum( 1 );
//	CurrYPos( TopMargin() );

	// Data
	CreateDataContentsObject();
	WriteDataContentsObject();

	// Resources
	CreateResourcesObject();
	CreateFontObject();
	CreateExtGStateObject();		
	WriteResourcesObject();

	AddPageToPageListObject( /*thePDFPageObject*/ );
	thePDFPageObject = NULL;
}

void PDFFile::CreateHalftone()
{
	// Create a XRef entry Object for the Halftone object
	IncrementPDFFieldObjectId();
	thePDFHalftone = new PDFHalftone( pdfFieldObjectId, 0, 'n' );
	PDFBodyObjects.push_back( thePDFHalftone );
}

void PDFFile::WriteHalftone()
{
	if (!thePDFHalftone)
		return;

	// Write out the Halftone
	thePDFHalftone->ByteOffset( byteOffset );
	fileData = thePDFHalftone->ObjectEntry();
	byteOffset += fileData.length(); 
	fputs(fileData.c_str(), theFP);
	thePDFHalftone->EraseContents();
}

void PDFFile::CreateInfoObject()
{
	// Info
	IncrementPDFFieldObjectId();
	thePDFInfoObject = new PDFInfoObject( pdfFieldObjectId, 0, 'n' );
	PDFBodyObjects.push_back( thePDFInfoObject );
}

void PDFFile::WriteInfoObject()
{
	// Write out the Info Object
	thePDFInfoObject->ByteOffset( byteOffset );
	thePDFInfoObject->SetInfoTitle( GetDocumentTitle() );
	fileData = thePDFInfoObject->ObjectEntry();
	byteOffset += fileData.length(); 
	fputs(fileData.c_str(), theFP);
	thePDFInfoObject->EraseContents();
}

void PDFFile::WriteXRef()
{
	// Cross Ref
	XRefPosition = byteOffset;
	fputs( "xref\r0 ", theFP);

	int count = 0;
	PDFBodyObjectsList::iterator bodyObjIter;
	for ( bodyObjIter = PDFBodyObjects.begin();
					bodyObjIter != PDFBodyObjects.end(); bodyObjIter++ )
	{
		PDFBodyObject* bodyObj = (*bodyObjIter);
		if ( bodyObj )
		{
			//if (!bodyObj->Invisible() && bodyObj->ByteOffset() != -1 )
			if ( !bodyObj->Invisible() )
				count++;
		}
	}

	fileData = intToStr( count );//PDFBodyObjects.size() );
	fileData += "\r";
	fputs(fileData.c_str(), theFP);

	for ( bodyObjIter = PDFBodyObjects.begin();
					bodyObjIter != PDFBodyObjects.end(); bodyObjIter++ )
	{
		PDFBodyObject* bodyObj = (*bodyObjIter);
		if ( bodyObj )
		{
			if (!bodyObj->Invisible())
				fputs(bodyObj->XRefEntry().c_str(), theFP);
		}
	}
}

void PDFFile::CreateCatelogObject()
{
	// Create a XRef entry Object for the Catelog object
	IncrementPDFFieldObjectId();
	if ( thePDFOutlinesObject && thePDFPagesListObject )
		thePDFCatelogObject = new PDFCatelogObject( pdfFieldObjectId, 0, 'n', thePDFPagesListObject->Id(), thePDFOutlinesObject->Id() );
	else if ( thePDFPagesListObject )
		thePDFCatelogObject = new PDFCatelogObject( pdfFieldObjectId, 0, 'n', thePDFPagesListObject->Id() );
	else
		thePDFCatelogObject = new PDFCatelogObject( pdfFieldObjectId, 0, 'n' );
	PDFBodyObjects.push_back( thePDFCatelogObject );
}

void PDFFile::WriteCatelogObject()
{
	// Write out the Catelog
	thePDFCatelogObject->ByteOffset( byteOffset );
	fileData = thePDFCatelogObject->ObjectEntry();
	byteOffset += fileData.length(); 
	fputs(fileData.c_str(), theFP);
	thePDFCatelogObject->EraseContents();
}

void PDFFile::WriteOutlinesObject()
{
	if (!thePDFOutlinesObject)
		return;

	// Before writing out any of the outlines, remove the last level 0 outline if it is unused (i.e. if it doesn't point to a page)
	if (thePDFOutlinesObject->PDFLinkObjects.Size() != 0)
	{
		PDFLinkObject *lastLinkObj = thePDFOutlinesObject->PDFLinkObjects.Back();
		if ( lastLinkObj )
		{
/*			if (lastLinkObj->PDFLinkObjects.Size() == 0)
			{
				PDFLinkToPageObject *aPDFLinkToPageObject = lastLinkObj->thePDFLinkToPageObject;
				aPDFLinkToPageObject->SetInvisible();// true );
				lastLinkObj->SetInvisible();// true );
				int prev = lastLinkObj->Prev();
				thePDFOutlinesObject->PDFLinkObjects.RemoveFromBack();
				lastLinkObj = thePDFOutlinesObject->PDFLinkObjects.Back();
				if ( lastLinkObj )
				{
					lastLinkObj->Next( PDF_NO_LINK );
				}
			}*/
		}
	}


	// Write out the Outlines main object
	thePDFOutlinesObject->ByteOffset( byteOffset );
	fileData = thePDFOutlinesObject->ObjectEntry();
	byteOffset += fileData.length(); 
	fputs(fileData.c_str(), theFP);

	PDFLinkObjectsListDataIter currIter = thePDFOutlinesObject->PDFLinkObjects.Begin();
	//PDFLinkObjectsList::iterator tempIter = currIter;
	
	for ( ; currIter != thePDFOutlinesObject->PDFLinkObjects.End(); currIter++ )
	{
		// Write the Link objects
		PDFLinkObject *aPDFLinkObject = (*currIter);
		aPDFLinkObject->ByteOffset( byteOffset );
		fileData = aPDFLinkObject->ObjectEntry();
		byteOffset += fileData.length(); 
		fputs(fileData.c_str(), theFP);

		// Write the LinkToPage objects
		PDFLinkToPageObject *aPDFLinkToPageObject = aPDFLinkObject->thePDFLinkToPageObject;
		aPDFLinkToPageObject->ByteOffset( byteOffset );
		fileData = aPDFLinkToPageObject->ObjectEntry();
		byteOffset += fileData.length(); 
		fputs(fileData.c_str(), theFP);

		PDFLinkObject *link = (*currIter);
		byteOffset = link->WriteLinkedObjects( byteOffset, theFP );
	}

	// Write out the list of Link objects with the LinkToPage objects
/*	for( PDFLinkObjectsList::iterator iter = thePDFOutlinesObject->PDFLinkObjects.begin();
		iter != thePDFOutlinesObject->PDFLinkObjects.end(); iter++ )
	{
		// Write the Link objects
		PDFLinkObject *aPDFLinkObject = (*iter);
		aPDFLinkObject->ByteOffset( byteOffset );
		fileData = aPDFLinkObject->ObjectEntry();
		byteOffset += fileData.length(); 
		fputs(fileData.c_str(), theFP);

		// Write the LinkToPage objects
		PDFLinkToPageObject *aPDFLinkToPageObject = aPDFLinkObject->aPDFLinkToPageObject;
		aPDFLinkToPageObject->ByteOffset( byteOffset );
		fileData = aPDFLinkToPageObject->ObjectEntry();
		byteOffset += fileData.length(); 
		fputs(fileData.c_str(), theFP);

		// Erase the contents of the Link and LineToPage objects
		aPDFLinkToPageObject->EraseContents();
		aPDFLinkObject->EraseContents();
	}*/
	// Erase the contents
	thePDFOutlinesObject->EraseContents();
}

void PDFFile::WriteTrailer()
{
	// Trailer
	fileData =  "trailer\r";
	fileData += "<<\r";
	fileData += "/Size ";
	int count = 0;
	PDFBodyObjectsList::iterator bodyObjIter;
	for ( bodyObjIter = PDFBodyObjects.begin();
					bodyObjIter != PDFBodyObjects.end(); bodyObjIter++ )
	{
		PDFBodyObject* bodyObj = (*bodyObjIter);
		if ( bodyObj )
		{
			if (!bodyObj->Invisible())
				count++;
		}
	}
	fileData += intToStr( count );//PDFBodyObjects.size() );
	fileData += "\r/Root ";
	fileData += intToStr( thePDFCatelogObject->Id() );
	fileData += " 0 R\r";
	if (thePDFInfoObject)
	{
		fileData += "/Info ";
		fileData += intToStr( thePDFInfoObject->Id() );
		fileData += " 0 R\r";
	}
		fileData += "/ID [<f007420477206cd8b0615cb53a417503><f007420477206cd8b0615cb53a417503>]\r";
//	fileData += "/ID [<00><00>]\r";
	fileData += ">>\r";
	fileData += "startxref\r";
	fileData += intToStr( XRefPosition );
	fileData += "\r";
	fileData += "%%EOF\r";
	fputs(fileData.c_str(), theFP);
}



void PDFFile::WriteImageData( PDFImageObjectList& imageObjectList )
{
	if (!imageObjectList.imageObject)
		return;

	// Write out the Image Object
	imageObjectList.imageObject->ByteOffset( byteOffset );
	char *buf;
	buf = 0;
	int len = 0;

	fileData = imageObjectList.imageObject->ObjectEntry();
	fputs(fileData.c_str(), theFP);
	byteOffset += fileData.length();

	len = imageObjectList.imageObject->ImageLen();
	if ( len )
	{
		imageObjectList.imageObject->GetImageData( &buf, len );
		int lineLen = imageObjectList.imageObject->Width()*3;
		char *PDFImageLineData = new char[ lineLen + 64 ];
		char *dataBufPtr;
		char *temp = buf;
		int extra;
		char replaceChar = 0x0B;
		for(int i = 0; i < imageObjectList.imageObject->Height(); i++ )
		{
			extra = 0;
			dataBufPtr = PDFImageLineData;
			for( int j = 0; j < lineLen; j++ )
			{
				if (*temp == '\n' )
				{
					//*dataBufPtr = '\\';
					//dataBufPtr++;
					//extra++;
					*temp = replaceChar;
				}
				*dataBufPtr = *temp;
				temp++;
				dataBufPtr++;
			}

			fwrite( PDFImageLineData, lineLen+extra, 1, theFP );
		}
		delete[] PDFImageLineData;
		byteOffset += len;
		fileData = "\r% Trev4\rendstream\rendobj\r";
		fputs(fileData.c_str(), theFP);
		byteOffset += fileData.length();
	}

	imageObjectList.imageObject->EraseContents();

	// Write out the Decode Object
	if ( imageObjectList.ASCII85DecodeObject )
	{
		imageObjectList.ASCII85DecodeObject->ByteOffset( byteOffset );
		fileData = imageObjectList.ASCII85DecodeObject->ObjectEntry();
		byteOffset += fileData.length(); 
		fputs(fileData.c_str(), theFP);
		imageObjectList.ASCII85DecodeObject->EraseContents();
	}

	// Write out the ColorSpace Object
	if ( imageObjectList.colorSpaceObject )
	{
		imageObjectList.colorSpaceObject->ByteOffset( byteOffset );
		fileData = imageObjectList.colorSpaceObject->ObjectEntry();
		byteOffset += fileData.length(); 
		fputs(fileData.c_str(), theFP);
		imageObjectList.colorSpaceObject->EraseContents();
	}
}


void PDFFile::WriteRemainingPDF()
{
	short error = 0;
	if ( PageDataLength() == 0 )
	{
		PDFBodyObject *hopefullyThePDFPageObject = PDFBodyObjects.back();
		if ( thePDFPageObject == hopefullyThePDFPageObject )
		{
			PDFBodyObjects.pop_back();
			DecrementPDFFieldObjectId();
		}
		else
			error++;//Error
	}

	PDFImageObjectList imageObjectList( thePDFImageObject, thePDFASCII85DecodeObject, thePDFColorSpaceObject );
	WriteImageData( imageObjectList );
	if ( imageListToKeepToWriteAtEnd.size() )
	{
		for ( int i = 0; i < imageListToKeepToWriteAtEnd.size(); i++ )
		{
			WriteImageData( imageListToKeepToWriteAtEnd[i] );
		}
	}

	if (m_imageOverview)
		WriteImageData(*m_imageOverview);
	if (m_imageGraph)
		WriteImageData(*m_imageGraph);
	if (m_imageTable)
		WriteImageData(*m_imageTable);

	CreateHalftone();
	WriteHalftone();
	WriteExtGStateObject();		

	CreateEncodingObject();
	WriteFontObject();
	WriteEncodingObject();

	WritePageObjects();
	WritePageListObject();		

	CreateCatelogObject();
	WriteCatelogObject();

	WriteOutlinesObject();

	CreateInfoObject();
	WriteInfoObject();

	WriteXRef();

	WriteTrailer();
}

bool PDFFile::Close()
{
	if (!theFP)
		return false;

	forceDraw = true;//SubPageNum( thePDFSettings.SubPagesPerPage() ); // This will force the any pages which have not been written to be written out...
	//CreateAPageObject();
	WriteCurrentPage();
	WriteRemainingPDF();

	fclose(theFP);

	Initialise(); // Put the PDF file back in it's original state

	return true;
}

bool PDFFile::StartNewSectionInReport()
{
	return startNewSectionInReport;
}


int PDFFile::DrawImage( const char* filename )
{
	PDFImageObjectList* imageObjectList = CreateAPDFImageObject( filename );
	if ( !InsertJPEGImageFromFile( filename, imageObjectList ) )
	{
		imagesList.pop_back();	
		return 0;
	}

	// Position the image
	float width = imageObjectList->imageObject->Width();
	float height = imageObjectList->imageObject->Height();
	int yAspectRatio = height * (SubPageDrawWidth() / width);

	PositionImage( imageObjectList->imagePosData, SubPageLeftMargin(), CurrYPos(), SubPageDrawWidth(), yAspectRatio );

	imageObjectList->imageData += "/";
	imageObjectList->imageData += imageObjectList->imageObject->GetImageName();
	imageObjectList->imageData += " Do\r";
	imageObjectList->imageData += "Q\r";

	CurrYPos( CurrYPos() + yAspectRatio + GraphTrailSpace() );
	currX = SubPageLeftMargin();

	return 1;
}

void PDFFile::DrawPageBanner( float x, float y )
{
	if ( !ShowBanner() )
		return;

	char*	szBannerFile = BannerFile();
	if (!szBannerFile || !*szBannerFile)
		return;
	if ( GetFileLength(szBannerFile) == 0 )
		return;


	if ( !thePDFImageObject )
	{
		PDFImageObjectList* imageObjectList;

		
		std::string imageStr = BannerFile();

		{	char *p;
			p = (char*)imageStr.c_str();
			while ( *p ) {
				if (*p == ' ' ) *p = '_';
				p++;
			}
		}


		imageObjectList = CreatePDFImageObjects( (char*)imageStr.c_str() );
		//imageObjectList = CreatePDFImageObjects( BannerFile() );		//old

		if ( strlen(BannerFile()) )
		{
			if ( !InsertJPEGImageFromFile( BannerFile(), imageObjectList ) )
			{
				DrawDefaultBanner( x, y );
				imagesList.pop_back();
				return;
			}
		}
		else
			DrawDefaultBanner( x, y );

		yMaintainAspectRatioFromOrigGIF = SubPageDrawWidth() * thePDFImageObject->Height()/thePDFImageObject->Width();
		// Make sure the image looks like a banner, so it only has a max height of 200, so that there is room left for data on the page 
		if ( yMaintainAspectRatioFromOrigGIF > 200 ) 
			yMaintainAspectRatioFromOrigGIF = 200;
		imagesList.pop_back();
	}

	// Position the banner
	std::string& bannerPagePos = bannerPagePosData[SubPageNum()-1];
	PositionImage( bannerPagePos, x, y, SubPageDrawWidth(), yMaintainAspectRatioFromOrigGIF );

	if ( bannerData.empty() ) 
	{
		bannerData += "/";
		bannerData += thePDFImageObject->GetImageName();
		bannerData += " Do\r";
		bannerData += "Q\r";
	}

	CurrYPos( CurrYPos() + yMaintainAspectRatioFromOrigGIF + BannerTrailSpace() );
	currX = SubPageLeftMargin();
}

void PDFFile::DrawDefaultBanner( float x, float y )
{
	if ( !thePDFImageObject )
		return;

	PDFAppBannerASCII85Decode aPDFAppBannerASCII85Decode;

	PDFImageAppBanner aPDFImageAppBanner;
	yMaintainAspectRatioFromOrigGIF = SubPageDrawWidth() * 42/431.04; 
	thePDFImageObject->SetImageData( aPDFImageAppBanner.Image(), thePDFColorSpaceObject->Id(), 600, 60, 8, "", "[/ASCII85Decode /LZWDecode]" );

	thePDFASCII85DecodeObject->SetDecodeData( aPDFAppBannerASCII85Decode.Data() );
}

PDFImageObjectList* PDFFile::CreateNewPDFImageObject( const char *name )
{
	// from CreateAPDFImageObject below
	IncrementPDFFieldObjectId();
	PDFImageObject *imageObject = new PDFImageObject( pdfFieldObjectId, 0, 'n', SkipPastPath( (char*)name ) );
	PDFBodyObjects.push_back( imageObject );

	IncrementPDFFieldObjectId();
	PDFASCII85DecodeObject *ASCII85DecodeObject = new PDFASCII85DecodeObject( pdfFieldObjectId, 0, 'n' );
	PDFBodyObjects.push_back( ASCII85DecodeObject );

	IncrementPDFFieldObjectId();
	PDFColorSpaceObject *colorSpaceObject = new PDFColorSpaceObject( pdfFieldObjectId, 0, 'n', ASCII85DecodeObject->Id() );
	PDFBodyObjects.push_back( colorSpaceObject );

	PDFImageObjectList* imageObjectList = new PDFImageObjectList( imageObject, ASCII85DecodeObject, colorSpaceObject, SubPageNum() );
	// Note the we did NOT add it to the 'imagesList' as the other versions of this function seem to.
	return imageObjectList;
}

PDFImageObjectList* PDFFile::CreateAPDFImageObject( const char *name )
{
	IncrementPDFFieldObjectId();
	PDFImageObject *imageObject = new PDFImageObject( pdfFieldObjectId, 0, 'n', SkipPastPath( (char*)name ) );
	PDFBodyObjects.push_back( imageObject );

	IncrementPDFFieldObjectId();
	PDFASCII85DecodeObject *ASCII85DecodeObject = new PDFASCII85DecodeObject( pdfFieldObjectId, 0, 'n' );
	PDFBodyObjects.push_back( ASCII85DecodeObject );

	IncrementPDFFieldObjectId();
	PDFColorSpaceObject *colorSpaceObject = new PDFColorSpaceObject( pdfFieldObjectId, 0, 'n', ASCII85DecodeObject->Id() );
	PDFBodyObjects.push_back( colorSpaceObject );

	PDFImageObjectList imageObjectList( imageObject, ASCII85DecodeObject, colorSpaceObject, SubPageNum() );
	imagesList.push_back( imageObjectList );

	return &imagesList.back();
}


PDFImageObjectList* PDFFile::CreatePDFImageObjects( char *imageName )
{
	IncrementPDFFieldObjectId();
	thePDFImageObject = new PDFImageObject( pdfFieldObjectId, 0, 'n', imageName );
	PDFBodyObjects.push_back( thePDFImageObject );

	IncrementPDFFieldObjectId();
	thePDFASCII85DecodeObject = new PDFASCII85DecodeObject( pdfFieldObjectId, 0, 'n' );
	PDFBodyObjects.push_back( thePDFASCII85DecodeObject );

	IncrementPDFFieldObjectId();
	thePDFColorSpaceObject = new PDFColorSpaceObject( pdfFieldObjectId, 0, 'n', thePDFASCII85DecodeObject->Id() );
	PDFBodyObjects.push_back( thePDFColorSpaceObject );

	PDFImageObjectList imageObjectList( thePDFImageObject, thePDFASCII85DecodeObject, thePDFColorSpaceObject, SubPageNum() );
	imagesList.push_back( imageObjectList );

	return &imagesList.back();
}

int PDFFile::InsertJPEGImageFromFile( const char *filename, PDFImageObjectList* imageObjectList )
{
	if ( !imageObjectList || filename[0] == 0 )
		return 0;

	const char* djpeg = "djpeg";
	const char* jpegImageFile = filename;
	const char* array[2] = { djpeg, jpegImageFile };
	PDFImageJpeg *pdfImageJpeg = new PDFImageJpeg();
	
	int result = CreatePDFImageFromFile( 3, array, pdfImageJpeg );
	if ( result == -1 )
	{
		delete pdfImageJpeg;
		return 0;
	}

	imageObjectList->imageObject->SetImageData( pdfImageJpeg, -1, "" );

	delete pdfImageJpeg;
	return 1;
}

void PDFFile::PositionImage( std::string& posData, float x, float y, float width, float height )
{
	if ( posData.empty() ) 
	{
		posData = "q\r";
		posData += floatToStr( width );
		posData += " 0 0 ";
		posData += floatToStr( height ); 
		posData += " ";
		posData += floatToStr( x ); // X position
		posData += " ";
		float PDFY = PageHeight() - y - height;
		posData += floatToStr( PDFY ); // Y position
		posData += " cm\r";
	}
}

int PDFFile::SubPageDrawWidth()
{
	if ( thePDFSettings.SubPagesPerPage() > 1 )
	{
		int pageDrawWidth, subPageMiddleMarginWidth, subPageWidth;
		pageDrawWidth = DrawWidth();
		subPageMiddleMarginWidth = RightMargin() * (thePDFSettings.SubPagesPerPage()-1);
		subPageWidth = (pageDrawWidth - subPageMiddleMarginWidth) / thePDFSettings.SubPagesPerPage();
		return subPageWidth;
	}
	else
		return DrawWidth();
}

int PDFFile::SubPageLeftMargin()
{
	if ( thePDFSettings.SubPagesPerPage() > 1 && SubPageNum() > 1 )
	{
		int pageXPos, subPageMiddleMarginWidth, subPageWidth;
		subPageMiddleMarginWidth = RightMargin() * (thePDFSettings.SubPagesPerPage()-1);
		subPageWidth = (DrawWidth() - subPageMiddleMarginWidth) / thePDFSettings.SubPagesPerPage();
		pageXPos = LeftMargin() + ( (SubPageNum()-1) * (subPageWidth + RightMargin()) );
		return pageXPos;
	}
	else
		return LeftMargin();
}

void PDFFile::DrawGraph( std::string graphDataP, int graphHeightP )//, int xOffset /*= 80*/, int yOffset /*= 4 Lines*/ )
{
/*	if ( !ShowGraph() )
		return;

/*	if (StartNewSectionInReport())
	{
		StartNewSectionInReport( false );
		currX = SubPageLeftMargin();
		CurrYPos( TopMargin() );
		WriteCurrentPage();
		//if ( !ShowBanner() )
		//	WriteCurrentPage();
	}*/
	graphNum++;
	PDFGraph graph( graphDataP, graphHeightP );

	// Check the file state, such as the position on a page of the
	// next output, and page numbers etc,
	int gh = graph.Height();
	if (!WillDataFitOnCurrentPage( gh )) // Trev
	{
		WriteCurrentPage();
		CreateAPageObject();
	}
	PageDataAdd( graph.Draw( CurrYPos() ) );

	CurrYPos( CurrYPos() + graph.Height() + GraphTrailSpace() );
}

void PDFFile::LinkOutlineToPage( int currPageNum )
{
	// If there is a "Section" Outline Entry, provide the link to the page
	if (sectionName.length())
	{
		thePDFOutlinesObject->LinkAllCurrLinkObjectsToPage( currPageNum );
		sectionName.erase();
	}
}

void PDFFile::WriteHelpCard(FILE *fp, const class CQHelpCard& rCQHelpCard, bool bTableOn, bool bGraphOn)
{
	tableNumInSection++;
	PDFTable table( &theTableData, theTableData.data.Rows(), "", "", &thePDFSettings, &thePDFTableSettings, timesNRFont, CurrYPos(), SubPageNum() );
	tableNum++;

	table.tableLineData = PDF_BLANK;
	table.CreateHelpCard(this, rCQHelpCard, thePDFSettings.DrawWidth(), bTableOn, bGraphOn);
}

void PDFFile::DrawTable( int numOfRows, std::string continuedOnNext, std::string continuedFromPrev, std::string HTMLLinkFilename, int outlineLevel )
{
	theTableData.CalculateColumnWidths( timesNRFont, SubPageDrawWidth() );

	if (StartNewSectionInReport())
	{
		StartNewSectionInReport( false );
		tableNumInSection = 0;
		WriteCurrentPage();
		CreateAPageObject();
	}
	
	tableNumInSection++;
	PDFTable table( &theTableData, theTableData.data.Rows(), continuedOnNext, continuedFromPrev, &thePDFSettings, &thePDFTableSettings, timesNRFont, CurrYPos(), SubPageNum() );
	tableNum++;

	// AddOutline here for table, not earlier!!
	int minSize = thePDFTableSettings.TitleSize() + thePDFTableSettings.ColHeadingSize() + (thePDFSettings.MinimumTableSizeAtPageEnd()-2) * thePDFTableSettings.DataSize() + thePDFSettings.BottomMargin();
	if (!WillDataFitOnCurrentPage( minSize )) 
	{
		WriteCurrentPage();
		CreateAPageObject();
	}

	AddOutline( theTableData.linkTransTitle, outlineLevel );
	if ( thePDFOutlinesObject )
	{
		thePDFOutlinesObject->LinkToPageString( theTableData.linkTransTitle, thePDFPageObject->Id() );
	}

	sectionName = currSectionName;

	table.FormatTableData();

	table.tableLineData = PDF_BLANK;

	int numOfPagesForTable = 0;
	while( table.DataLeft() ) // keep dividing the table up...
	{
		//
		table.SubPageNum( SubPageNum() );

		if ( tableNumInSection > 1 && !demoOffsetDoneForThisPage )
			DisplayDemoString();

		// Produce the table lines and text
		table.DrawTableLines( CurrYPos() );
		if ( SubPageNum() <= 1 )
			LinkOutlineToPage( thePDFPageObject->Id() );
		std::string title;
		if ( thePDFOutlinesObject)
			title = thePDFOutlinesObject->LinkToPageString( theTableData.linkTransTitle, thePDFPageObject->Id() );
		else
			title = theTableData.linkTransTitle;
		PageDataAdd( table.DrawTableText( CurrYPos(), title, annotsList, annotsInternalLinksList, thePDFPageObject, HTMLLinkFilename ) );
		outlineDone = 0;

		// See if there is any table data left, if so, then write out the current page
		if ( table.DataLeft() )
		{
			if ( SubPageNum() < thePDFSettings.SubPagesPerPage() )
			{
				//pageData = PDF_BLANK;
				SubPageNum( SubPageNum()+1 );
				currX = SubPageLeftMargin();
				CurrYPos( TopMargin() );
				DisplayPageNumbering();
				DrawPageBanner( currX, CurrYPos() );				
				numOfPagesForTable++;
			}
			else
			{
				WriteCurrentPage();
				CreateAPageObject();
				DisplayDemoString();
				SubPageNum( 1 );
				table.tableLineData = PDF_BLANK;
				numOfPagesForTable++;
			}
		}
		else
		{
			CurrYPos( CurrYPos() + table.LastTableHeight() + TableTrailSpace() );
			//WriteCurrentPage();
			//CreateAPageObject();
			//DisplayDemoString();
		}
	}
	//HTMLLinkFilename = "";
	//annotsList.clear();
}

void PDFFile::ResetTableDetails()
{
	tableTitle = PDF_BLANK;
//	pageNumAtTopData = PDF_BLANK;
//	pageNumAtTop2Data = PDF_BLANK;
	theTableData.Clear();
	PDFTableSettings *p = &thePDFTableSettings;
	theTableData.SetFonts( p->TitleSize(), p->TitleStyle(), p->TitleColor(),
						   p->ColHeadingSize(), p->ColHeadingStyle(), p->ColHeadingColor(),
						   p->DataSize(), p->DataStyle(), p->DataColor() );
}

void PDFFile::DisplayPageNumbering()
{
	short displayPageNum = (pageNum-1)*thePDFSettings.SubPagesPerPage() + SubPageNum();

	if ( !thePDFSettings.PageNumberingPosition() || pageNum == 1 )
		return;

	PDFTextList pageNumberTextList;
	std::string pageNumberStrLines = "--- Page " + intToStr( displayPageNum ) + " ---";
	std::string pageNumberStr = "Page " + intToStr( displayPageNum );
	int pageNumberSize = thePDFTableSettings.DataSize();
	int pos = (thePDFSettings.TopMargin()+pageNumberSize)/2;
	if ( thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPCENTER ||
		 thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPRIGHT ||
		 thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPANDBOTTOMCENTER  ||
		 thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPANDBOTTOMRIGHT )
	{
		if ( thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPCENTER || thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPANDBOTTOMCENTER )
			pageNumberTextList.push_back( PDFTextStr( pageNumberStrLines, PDF_TEXT_CENTERED, pos, PDF_NORMAL, pageNumberSize, PDF_BLACK ) );
		else if ( thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPRIGHT || thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPANDBOTTOMRIGHT )
			pageNumberTextList.push_back( PDFTextStr( pageNumberStr, PDF_TEXT_RIGHT, pos, PDF_NORMAL, pageNumberSize, PDF_BLACK ) );
	}
	pos = thePDFSettings.PageHeight() - thePDFSettings.BottomMargin() + (thePDFSettings.BottomMargin()+pageNumberSize)/2 - pos;
	if ( thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_BOTTOMCENTER  ||
		 thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_BOTTOMRIGHT ||
		 thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPANDBOTTOMCENTER  ||
		 thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPANDBOTTOMRIGHT )
	{
		if ( thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_BOTTOMCENTER || thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPANDBOTTOMCENTER )
			pageNumberTextList.push_back( PDFTextStr( pageNumberStrLines, PDF_TEXT_CENTERED, pos, PDF_NORMAL, pageNumberSize, PDF_BLACK ) );
		else if ( thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_BOTTOMRIGHT || thePDFSettings.PageNumberingPosition() == PDF_PAGENUMBER_TOPANDBOTTOMRIGHT )
			pageNumberTextList.push_back( PDFTextStr( pageNumberStr, PDF_TEXT_RIGHT, pos, PDF_NORMAL, pageNumberSize, PDF_BLACK ) );
	}
	PDFText text( &thePDFSettings, timesNRFont, SubPageNum() );
	pageNumAtTopData[SubPageNum()-1] = text.Draw( pageNumberTextList, 1, 0, 1 );
}

void PDFFile::DisplayDemoString()
{
	if (!displayDemo)
		return;

	WriteCurrentPage();
	CreateAPageObject();
	if ( demoData.empty() ) 
	{
		PDFTextList demoPDFTextList;
//		demoPDFTextList.push_back( PDFTextStr( PDF_BLANK, PDF_TEXT_CENTERED, 10, PDF_NORMAL, 10 ) );
		demoPDFTextList.push_back( PDFTextStr( "Demonstration", PDF_TEXT_CENTERED, PDF_TEXT_ON_NEXT_LINE, PDF_BOLDITALIC, 40, PDF_RED ) );
//		demoPDFTextList.push_back( PDFTextStr( PDF_BLANK, PDF_TEXT_CENTERED, 30, PDF_NORMAL, 30 ) );
		PDFText text( &thePDFSettings, timesNRFont, SubPageNum() );
		demoData = text.Draw( demoPDFTextList, CurrYPos() );
		demoStringYOffset = text.LastDrawYPos() + 20;
	}
	CurrYPos( demoStringYOffset );
	demoOffsetDoneForThisPage = true;
}

static const int PDF_SUMM_MAINTITLE_SIZE = 18;
static const int PDF_SUMM_SUBTITLE_SIZE = 14;
static const int PDF_SUMM_NORMAL_SIZE = 10;

int PDFFile::SummaryMainTitleSize()
{
	return thePDFTableSettings.TitleSize();

/*	if ( thePDFSettings.SubPagesPerPage() > 1 )
		return (PDF_SUMM_MAINTITLE_SIZE-PDF_FONTSIZE_ADJ)/thePDFSettings.SubPagesPerPage() + PDF_FONTSIZE_ADJ;
	return PDF_SUMM_MAINTITLE_SIZE;*/
}

int PDFFile::SummarySubTitleSize()
{
	return thePDFTableSettings.ColHeadingSize();
/*	if ( thePDFSettings.SubPagesPerPage() > 1 )
		return (PDF_SUMM_SUBTITLE_SIZE-PDF_FONTSIZE_ADJ)/thePDFSettings.SubPagesPerPage() + PDF_FONTSIZE_ADJ;
	return PDF_SUMM_SUBTITLE_SIZE;*/
}

int PDFFile::SummaryNormalSize()
{
	return thePDFTableSettings.DataSize();
/*	if ( thePDFSettings.SubPagesPerPage() > 1 )
		return (PDF_SUMM_NORMAL_SIZE-PDF_FONTSIZE_ADJ)/thePDFSettings.SubPagesPerPage() + PDF_FONTSIZE_ADJ;
	return PDF_SUMM_NORMAL_SIZE;*/
}

void PDFFile::WriteFrontPage( char *title )
{
	PDFTextList frontPageTextList;
	//frontPageTextList.push_back( PDFTextStr( PDF_BLANK, PDF_TEXT_CENTERED, 0, PDF_BOLD, SummaryMainTitleSize(), PDF_BLACK ) );
	//frontPageTextList.push_back( PDFTextStr( PDF_BLANK, PDF_TEXT_CENTERED, SummaryMainTitleSize()*1.5, 1, SummaryMainTitleSize()*1.5 ) );

	if ( *title )
		frontPageTextList.push_back( PDFTextStr( title, PDF_TEXT_CENTERED, SummaryMainTitleSize()*1.5, 1, SummaryMainTitleSize()*1.5 ) );
	else
		frontPageTextList.push_back( PDFTextStr( GetDocumentTitle().c_str(), PDF_TEXT_CENTERED, SummaryMainTitleSize()*1.5, 1, SummaryMainTitleSize()*1.5 ) );

	CreateAPageObject();
	DrawPDFText( frontPageTextList, CurrYPos() );
	WriteCurrentPage();
}


void PDFFile::FrontPageTableTitle( const char * text )
{
	summaryPDFTextList.push_back( PDFTextStr( text, PDF_TEXT_CENTERED, thePDFTableSettings.TitleSize(), PDF_BOLD, SummaryMainTitleSize(), PDF_BLACK ) );
	summaryPDFTextList.push_back( PDFTextStr( PDF_BLANK, PDF_TEXT_CENTERED, SummaryMainTitleSize()*1.5, 1, SummaryMainTitleSize()*1.5 ) );
}

void PDFFile::FrontPageTableEntry( const char *text, const char *number )
{
	summaryPDFTextList.push_back( PDFTextStr( text, PDF_TEXT_AT_FIRST_TAB, PDF_TEXT_ON_NEXT_LINE, PDF_NORMAL, SummaryNormalSize() ) );
	if (number)
		summaryPDFTextList.push_back( PDFTextStr( number, PDF_TEXT_AT_NEXT_TAB, 0, PDF_NORMAL, SummaryNormalSize() ) );
	summaryPDFTextList.push_back( PDFTextStr( PDF_BLANK, PDF_TEXT_CENTERED, 1, 1, 1 ) );
}

void PDFFile::FrontPageTableSubTitle( const char *text )
{
	summaryPDFTextList.push_back( PDFTextStr( PDF_LINE, 0, 0, 1 ) );
	summaryPDFTextList.push_back( PDFTextStr( text, 0, PDF_NEW_SECTION, PDF_BOLD, SummarySubTitleSize() ) );
	summaryPDFTextList.push_back( PDFTextStr( PDF_BLANK, PDF_TEXT_CENTERED, SummarySubTitleSize()/2, 1, SummarySubTitleSize()/2 ) );
}


std::string PDFFile::GetDocumentTitle()
{
	return docTitle;
}

const char *PDFFile::PageData( short page /*= 0*/ )
{
	if ( page != 0 && page >= 1 && page <= 2 )
		return pageData[page-1].c_str();
	if ( SubPageNum() == 2 )
		return pageData[1].c_str();
	else
		return pageData[0].c_str();
}

void PDFFile::PageDataAdd( std::string str, short page /*= 0*/ )
{
	if ( page != 0 && page >= 1 && page <= 2 )
		pageData[page-1] += str;
	if ( SubPageNum() == 2 )
		pageData[1] += str;
	else
		pageData[0] += str;;
}

void PDFFile::PageDataReset( short page /*= 0*/ )
{
	if ( page != 0 && page >= 1 && page <= 2 )
		pageData[page-1] = PDF_BLANK;
	if ( SubPageNum() == 2 )
		pageData[1] = PDF_BLANK;
	else
		pageData[0] = PDF_BLANK;
}

long PDFFile::PageDataLength( short page /*= 0*/ )
{
	if ( page != 0 && page >= 1 && page <= 2 )
		return pageData[page-1].length();
	if ( SubPageNum() == 2 )
		return pageData[1].length();
	else
		return pageData[0].length();
}

void PDFFile::DrawPDFText( PDFTextList& textList, int Ypos /*= 0*/, int vertCenter /*= 0*/ )
{
	// Is this text to be written in a new section, if so, then write the current page, and continue
	if ( StartNewSectionInReport() )
	{
		StartNewSectionInReport( false );
		WriteCurrentPage();
	}
	
	// 
	PDFText text( &thePDFSettings, timesNRFont, SubPageNum() );

	bool doingOutline = false;
	std::string outlineStr;
	PDFTextStr& possibleOutlineStr = textList.front();
	if ( possibleOutlineStr.y == PDF_TEXT_OUTLINE )
	{
		doingOutline = true;
		outlineStr = possibleOutlineStr.text;
		textList.pop_front();
	}

	int initialSize = textList.size();
	// Keep writing all the text until we have emptied the list 
	while ( textList.size() )
	{
		text.SubPageNum( SubPageNum() );
		std::string textToAdd;
		if ( Ypos )
			textToAdd = text.Draw( textList, Ypos, vertCenter );
		else
			textToAdd = text.Draw( textList, CurrYPos(), vertCenter );
		PageDataAdd( textToAdd );
		CurrYPos( text.LastDrawYPos() ); // So that we position the y value

		if ( doingOutline )
		{
			if ( initialSize != textList.size() ) // Indicates we have drawn some text...
			{
				AddOutline( outlineStr, 2 );
				if ( thePDFOutlinesObject )
				{
					thePDFOutlinesObject->LinkToPageString( outlineStr, thePDFPageObject->Id() );
				}
				doingOutline = false;
			}
		}

		// If the text is not completely empty, then we need to write out the current page
		if ( textList.size() )
		{
			// Have we written all the "logical" pages to this phyisical page 
			if ( SubPageNum() < thePDFSettings.SubPagesPerPage() ) 
			{
				// Up the page number, and reset drawing positions
				SubPageNum( SubPageNum()+1 );
				currX = SubPageLeftMargin();
				CurrYPos( TopMargin() );
				DisplayPageNumbering();
				DrawPageBanner( currX, CurrYPos() );				
			}
			else // Yes, we have written the amount of "logical" pages to this phyisical page 
			{
				WriteCurrentPage();
				CreateAPageObject();
			}

			if ( doingOutline )
			{
				if ( initialSize == textList.size() ) // Indicates we have NOT drawn some text...
				{
					// So add the outline after the page has been drawn
					AddOutline( outlineStr, 2 );
					if ( thePDFOutlinesObject )
					{
						thePDFOutlinesObject->LinkToPageString( outlineStr, thePDFPageObject->Id() );
					}
					doingOutline = false;
				}
			}
		}
	}
}

int PDFFile::AddOutline( std::string &text, int level, int forceLink /*= 0*/ )
{
	if ( level != 0 && level != 2 )
	{
		if ( outlineDone )
			return 0;
		outlineDone = 1;
	}

	if ( !thePDFOutlinesObject )
	{
		// Trev this code doesn't stop this object being created if there are no reports written...
		if ( text == "" ) // No Previous outlines have been added...
			return 0;
		IncrementPDFFieldObjectId();
		thePDFOutlinesObject = new PDFOutlinesObject( pdfFieldObjectId, 0, 'n' );
		PDFBodyObjects.push_back( (PDFBodyObject*)thePDFOutlinesObject );
	}

	if (text == PDF_BLANK && level == 0) // Denotes that there are no more sections, now we check if the last section contained any pages
	{
		// If the last section is empty, then remove the objects created...
		PDFLinkObject *lastLinkObj = thePDFOutlinesObject->PDFLinkObjects.Back();
/*		if (lastLinkObj->PDFLinkObjects.Empty())
		{
			thePDFOutlinesObject->PDFLinkObjects.RemoveFromBack();

			PDFLinkToPageObject *aPDFLinkToPageObject = lastLinkObj->thePDFLinkToPageObject;
			aPDFLinkToPageObject->SetInvisible();// true );
			lastLinkObj->SetInvisible();// true );
			int prev = lastLinkObj->Prev();
			if ( thePDFOutlinesObject->PDFLinkObjects.Size() != 0 )
			{
				lastLinkObj = thePDFOutlinesObject->PDFLinkObjects.Back();
				if ( lastLinkObj )
				{
					lastLinkObj->Next( PDF_NO_LINK );
				}
			}

			// Also remove the 2 objects from the Body objects list
			/*PDFBodyObject *obj = PDFBodyObjects.back();
			PDFBodyObjects.pop_back();
			delete obj;
			obj = PDFBodyObjects.back();
			PDFBodyObjects.pop_back();
			delete obj;
			pdfFieldObjectId-=2;*/

		//}*/
		return 0;	
	}

	std::string newText = timesNRFont->MakePDFString( text );

	// Check to make sure we don't put a duplicate outline entry in the outlines
	if ( level == 2 && thePDFOutlinesObject->PDFLinkObjects.Size() )
	{
		PDFLinkObject *obj = thePDFOutlinesObject->PDFLinkObjects.Back();
		if ( obj->PDFLinkObjects.Size() )
		{
			PDFLinkObject *obj2 = obj->PDFLinkObjects.Back();
			if ( obj2->PDFLinkObjects.Size() )
			{
				PDFLinkObject *obj3 = obj2->PDFLinkObjects.Back();
				if ( !strcmp( obj3->Title(), newText.c_str() ) )
					return -1;//thePDFOutlinesObject->CurrLinkObjectListSize();
			}
		}
	}

	if ( level == 0 && thePDFOutlinesObject->AreThereCurrLinkObjects() ) // We haven't written any reports in the section, so reuse the link objects
	{
		PDFLinkObject *obj = thePDFOutlinesObject->GetFrontCurrLinkObject();
		obj->Title( timesNRFont->MakePDFString( newText ) );
		thePDFOutlinesObject->AddLinkToText( text, thePDFOutlinesObject->CurrLinkObjectListSize() );
		return thePDFOutlinesObject->CurrLinkObjectListSize();
	}

	IncrementPDFFieldObjectId();
	PDFLinkToPageObject* aPDFLinkToPageObject = new PDFLinkToPageObject( pdfFieldObjectId, 0, 'n' );
	if ( aPDFLinkToPageObject )
	{
		PDFBodyObjects.push_back( (PDFBodyObject*)aPDFLinkToPageObject );
		//PageNum( annoteNumP );

		IncrementPDFFieldObjectId();
		PDFLinkObject* aPDFLinkObject = new PDFLinkObject( pdfFieldObjectId, 0, 'n', newText, aPDFLinkToPageObject );//, "destP", parentP), prevP, nextP, firstP, lastP, countP )
		PDFBodyObjects.push_back( (PDFBodyObject*)aPDFLinkObject );
		return thePDFOutlinesObject->AddLinkObject( aPDFLinkObject, level, text, forceLink );
	}
	return -1;
}

double PDFFile::GetStringLen( std::string str )
{
	return timesNRFont->GetStringLen( str, thePDFTableSettings.DataSize(), thePDFTableSettings.DataStyle() );//, PDF_CURRENT_STYLE );
}

char* PDFFile::SkipPastPath( char *filename )
{
	char *lastSeperator = filename;
	for ( char *ptr = lastSeperator; *ptr; ptr++ )
	{
		if ( *ptr == PATHSEP )
		{
			lastSeperator = ptr;
			lastSeperator++;
		}
	}
	return lastSeperator;
}

void PDFFile::SubPageNum( int subPageNumP )
{
	if ( subPageNumP < PDF_MAX_SUBPAGES )
		subPageNum = subPageNumP;
	else
		subPageNum = PDF_MAX_SUBPAGES;
}

static size_t OldtextWrap = 0;

void PDFFile::AdjustTableSettingsForSingleCellFlowingText()
{
	OldtextWrap = thePDFTableSettings.textWrap;
	thePDFTableSettings.textWrap = 1;
}

void PDFFile::AdjustTableSettingsBackAfterSingleCellFlowingText()
{
	thePDFTableSettings.textWrap = OldtextWrap;
}
