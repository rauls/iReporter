
#ifndef PDFCORE_H
#define PDFCORE_H

#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <vector>
#include "PDFBase.h"
#include "PDFFont.h"
#include "PDFImages.h"

// Typedefs
typedef std::list<PDFBodyObject*> PDFBodyObjectsList;


//
// Represents the PDF header section
//
class PDFHeader
{
public:
	PDFHeader();
	std::string ObjectEntry() { return str;	}
	int len() { return str.length(); }
private:
	std::string str;
};


//
// Extented Graphics State object - don't really know exactly why we need this object... but who cares??
//
class PDFExtGState : public PDFBodyObject
{
public:
	PDFExtGState( int idP, int genNumberP, char useStateP )
		: PDFBodyObject( idP, genNumberP, useStateP ) {}
	~PDFExtGState() {}
	std::string ObjectEntry();
	virtual const char *ClassName() { return "PDFExtGState"; }
};


class ImageLinkData
{
public:
	ImageLinkData( std::string imageNameP, int imageNumP, int colorSpaceNumP )
	{
		imageName = imageNameP;
		imageNum = imageNumP;
		colorSpaceNum = colorSpaceNumP;
	} 
	std::string imageName;
	int imageNum;
	int colorSpaceNum;
};

//
// Resources Object which specifies the (1) the font object to be used, and (2) the Ext Graphics Object
//
class PDFResourcesObject : public PDFBodyObject
{
public:
	PDFResourcesObject( int idP, int genNumberP, char useStateP )
		: PDFBodyObject( idP, genNumberP, useStateP ) {}
	~PDFResourcesObject() {}
	std::string ObjectEntry();
	void SetExtGStateNum( int extGStateNumP ) { extGStateNum = extGStateNumP; }
	void SetFontList( PDFStrIntList thePDFFontListP ) { thePDFFontList = thePDFFontListP; }
	void AddImage( ImageLinkData& imageLinkData )
	{
		imageLinkDataList.insert( imageLinkDataList.end(), imageLinkData );
	}
	std::string GetExtGStateStr() {	return intToStr( extGStateNum ); }
	virtual const char *ClassName() { return "PDFResourcesObject"; }
private:
	std::string temp;
	int extGStateNum;
	std::vector< ImageLinkData > imageLinkDataList;
	PDFStrIntList thePDFFontList;
};


//
// PDFASCII85Decode Object 
//
class PDFASCII85DecodeObject : public PDFBodyObject
{
public:
	PDFASCII85DecodeObject( int idP, int genNumberP, char useStateP )
		: PDFBodyObject( idP, genNumberP, useStateP ) {}
	~PDFASCII85DecodeObject() {}
	void SetDecodeData( std::string decodeDataP ) {	decodeData = decodeDataP; }
	std::string ObjectEntry();
	virtual const char *ClassName() { return "PDFASCII85DecodeObject"; }
private:
	std::string decodeData;
};


//
// PDFColorSpace Object 
//
class PDFColorSpaceObject : public PDFBodyObject
{
public:
	PDFColorSpaceObject( int idP, int genNumberP, char useStateP, int PDFASCII85DecodeObjectNumP )
		: PDFBodyObject( idP, genNumberP, useStateP ) { PDFASCII85DecodeObjectNum = PDFASCII85DecodeObjectNumP; }
	~PDFColorSpaceObject() {}
	std::string ObjectEntry();
	virtual const char *ClassName() { return "PDFColorSpaceObject"; }
private:
	int PDFASCII85DecodeObjectNum;
};


//
// Image Object 
//
class PDFImageObject : public PDFBodyObject
{
public:
	PDFImageObject( int idP, int genNumberP, char useStateP, std::string imageNameP )
		: PDFBodyObject( idP, genNumberP, useStateP ) { imageName = imageNameP; }
	~PDFImageObject() {}
	void SetImageData( std::string imageDataP, int colorSpaceNumP, int widthP, int heightP, int bitsPerComponentP,
		std::string colorSpaceP, std::string filterP )
	{
		imageData = imageDataP;
		colorSpaceNum = colorSpaceNumP;
		width = widthP;
		height = heightP;
		bitsPerComponent = bitsPerComponentP;
		colorSpace = colorSpaceP;
		filter = filterP;
	}
	void SetImageData( PDFImage *thePDFImage, int colorSpaceNumP, /*int widthP, int heightP, int bitsPerComponentP,
		std::string colorSpaceP,*/ std::string filterP )
	{
		thePDFImage->GetImageData( &imageBuf, imageLen );
//		imageLen = thePDFImage->len;
//		imageBuf = thePDFImage->imageDataBytes;
		colorSpaceNum = colorSpaceNumP;
		width = thePDFImage->width;
		height = thePDFImage->height;
		bitsPerComponent = thePDFImage->bitsPerComponent;
		colorSpace = "";
		filter = filterP;
	}
	std::string ObjectEntry();
	std::string GetColorSpaceObjIdStr()
	{ 
		if ( colorSpaceNum != -1 )
		{
			std::string temp;
			temp += intToStr( colorSpaceNum );
			temp += " 0 R\r";
			return temp;
		}
		else
			return "/DeviceRGB\r";
	}
	std::string GetImageName() { return imageName; }
	std::string GetWidth() { return intToStr( width ); }
	std::string GetHeight() { return intToStr( height ); }
	std::string GetBitsPerComponent() { return intToStr( bitsPerComponent ); }
	std::string GetColorSpace() { return colorSpace; }
	std::string GetFilter() { return filter; }
	int Width() { return width; }
	int Height() { return height; }
	virtual const char *ClassName() { return "PDFImageObject"; }
private:
	std::string imageData;
	std::string imageName;
	int colorSpaceNum;
	int width;
	int height;
	int bitsPerComponent;
	std::string colorSpace;
	std::string filter;
};




//
// Halftone object - just using 'cause it was in an example I was trying to duplicate...
//
class PDFHalftone : public PDFBodyObject
{
public:
	PDFHalftone( int idP, int genNumberP, char useStateP )
		: PDFBodyObject( idP, genNumberP, useStateP ) {}
	~PDFHalftone() {}
	std::string ObjectEntry();
	virtual const char *ClassName() { return "PDFHalftone"; }
};


//
// Data Object - The actual data which is drawn by PDF file readers.
//
class PDFDataContentObject : public PDFBodyObject
{
public:
	PDFDataContentObject( int idP, int genNumberP, char useStateP, int commentsOnP, int pageNum )
		: PDFBodyObject( idP, genNumberP, useStateP )
	{
		commentsOn = commentsOnP;
		physicalPageNumber = pageNum;
	}
	~PDFDataContentObject() {}
	std::string ObjectEntry()
	{
		AddContents( buffer );
		return PDFBodyObject::ObjectEntry();
	}
	void AddText( std::string textToAdd ) { text += textToAdd;	}
	void AddDataContentsData( std::string data, std::string data2 );
	virtual const char *ClassName() { return "PDFDataContentObject"; }
private:
	std::string text;
	int commentsOn;
	int physicalPageNumber;
};




//
// URI Object - link to ...
//
class PDFURIObject : public PDFBodyObject
{
public:
	PDFURIObject( int idP, int genNumberP, char useStateP, std::string textP, std::string urlP )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
		text = textP;
		url = urlP;
	}
	~PDFURIObject(){}
	std::string ObjectEntry();
	std::string GetLinkRectStr() { return position; }
	std::string Text() { return text; }
	std::string URL() { return url; }
	void Position( std::string positionP ) { position = positionP; }
	virtual const char *ClassName() { return "PDFURIObject"; }
private:
	std::string text;
	std::string position;
	std::string url;
	std::string str;
};


//
// Annote Object - link to ...
//
class PDFAnnotObject : public PDFBodyObject
{
public:
	PDFAnnotObject( int idP, int genNumberP, char useStateP )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
	}
	~PDFAnnotObject(){}
	virtual std::string ObjectEntry(){ return ""; }
	virtual std::string Text(){ return ""; }
	virtual std::string URL() { return ""; }
	virtual void Position( std::string position ){}
	virtual void Dest( std::string destP ){}
	virtual const char *ClassName() { return "PDFAnnotObject"; }
public:
	PDFURIObject *URIObj;
protected:
	std::string str;
};


//
// Annote URI Object - link to ...
//
class PDFAnnotURILinkObject : public PDFAnnotObject
{
public:
	PDFAnnotURILinkObject( int idP, int genNumberP, char useStateP, PDFURIObject* URIObjP )
		: PDFAnnotObject( idP, genNumberP, useStateP ) 
	{
		URIObj = URIObjP;
	}
	~PDFAnnotURILinkObject(){}
	virtual std::string ObjectEntry();
	virtual std::string Text() { return URIObj->Text(); }
	virtual void Position( std::string position ) { URIObj->Position( position ); }
};


//
// Annote Internal Link Object - link to ...
//
class PDFAnnotInternalLinkObject : public PDFAnnotObject
{
public:
	PDFAnnotInternalLinkObject( int idP, int genNumberP, char useStateP, std::string textP, std::string urlP )
		: PDFAnnotObject( idP, genNumberP, useStateP ) 
	{
		text = textP;
		url = urlP;
		URIObj = NULL;
	}
	~PDFAnnotInternalLinkObject(){}
	virtual std::string ObjectEntry();
	virtual std::string Text() { return text; }
	virtual std::string URL() { return url; }
	virtual void Position( std::string positionP ) { position = positionP; }
	virtual void Dest( std::string destP ) { dest = destP; }
	std::string GetLinkRectStr() { return position; }
private:
	std::string text;
	std::string url;
	std::string dest;
	std::string position;
};


//
// Page Object - each page that is viewed in a PDF file is one of these...
//
class PDFPageObject : public PDFBodyObject
{
public:
	PDFPageObject( int idP, int genNumberP, char useStateP, int commentsOnP, int pageNum )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
		parentObjectNum = -1;
		resourceObjectNum = -1;
		dataContentObjectNum = -1;
		commentsOn = commentsOnP;
		physicalPageNumber = pageNum;
	}
	~PDFPageObject(){}
	std::string ObjectEntry();
	std::string GetParentObjectStr() { return intToStr( parentObjectNum ); }
	std::string GetResourceObjectStr() { return intToStr( resourceObjectNum ); }
	std::string GetContentObjectStr() { return intToStr( dataContentObjectNum ); }
	void ParentObjectNum( int parentObjectP ) { parentObjectNum = parentObjectP; }
	void ResourceObjectNum( int resourceObjectP ) { resourceObjectNum = resourceObjectP; }
	void DataContentObjectNum( int dataContentObjectP ) { dataContentObjectNum = dataContentObjectP; }
	std::string GetAnnotsStr();
	void AddAnnote( PDFAnnotObject *linkToURI );
	PDFBodyObjectsList annotsList;
	virtual const char *ClassName() { return "PDFPageObject"; }
private:
	int parentObjectNum;
	int resourceObjectNum;
	int dataContentObjectNum;
	std::string str;
	int commentsOn;
	int physicalPageNumber;
};


typedef std::list<PDFPageObject*> PDFPageList;

//
// Pages List Object - this is a list of the pages (using each page's object number) in the PDF file
//
class PDFPagesListObject : public PDFBodyObject
{
public:
	PDFPagesListObject( int idP, int genNumberP, char useStateP, int pageHeightP, int pageWidthP )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
		pageHeight = pageHeightP;
		pageWidth = pageWidthP;
	}
	~PDFPagesListObject() {}
	void AddPage( PDFPageObject *pPage ) { kidPages.push_back( pPage );	}
	std::string ObjectEntry();
	int PageCount() { return kidPages.size(); }
	int PageWidth() { return pageWidth; }
	int PageHeight() { return pageHeight; }
	PDFPageList::iterator StartPage() { return kidPages.begin(); }
	PDFPageList::iterator EndPage() { return kidPages.end(); }
	virtual const char *ClassName() { return "PDFPagesListObject"; }
private:
	PDFPageList kidPages;
	const char *KidPagesStr();
	std::string	SizeStr();
	std::string temp;
	int pageWidth;
	int pageHeight;
};


//
// Catelog Object - this is a high level object which points to the Pages List object
// 
class PDFCatelogObject : public PDFBodyObject
{
public:
	PDFCatelogObject( int idP, int genNumberP, char useStateP, int pagesObjectNumP = -1, int outlinesObjectNumP = -1 )//, namesObjectNumP )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
		pagesObjectNum = pagesObjectNumP;
		outlinesObjectNum = outlinesObjectNumP;
	}
	~PDFCatelogObject() {}
	std::string ObjectEntry();
	virtual const char *ClassName() { return "PDFCatelogObject"; }
private:
	std::string PagesObjectStr() { return intToStr( pagesObjectNum ); }
	int pagesObjectNum;
	std::string OutlinesObjectStr() { return intToStr( outlinesObjectNum ); }
	int outlinesObjectNum;
	std::string NamesObjectStr() { return intToStr( namesObjectNum ); }
	int namesObjectNum;
};




//
// Outlines Object - this is a high level object which starts the list containing links to pages within
// a PDF document.
// 
class PDFOutlinesObject : public PDFBodyObject
{
public:
	PDFOutlinesObject( int idP, int genNumberP, char useStateP );
	~PDFOutlinesObject() {}
	void ClearCurrLinkObjectList();
	bool AreThereCurrLinkObjects();
	long CurrLinkObjectListSize();
	PDFLinkObject *GetFrontCurrLinkObject();
	PDFLinkObject *GetBackCurrLinkObject();
	void LinkAllCurrLinkObjectsToPage( int currPageNum );
	std::string ObjectEntry();
	std::string LinkToPageString( std::string& title, int currPageNum );
	int AddLinkObject( PDFLinkObject* aPDFLinkObject, int level, std::string& text, int forceLink = 0 );
	void AddLinkToText( std::string& text, int outlineNum );
	virtual const char *ClassName() { return "PDFOutlinesObject"; }
private:
	void AddOutline( PDFLinkObject* aPDFLinkObject, int level = 0 );
	PDFLinkObjectsList currLinkObjectList;
public:
	PDFLinkObjectsList PDFLinkObjects;
private:
	std::string CountStr() { return intToStr( PDFLinkObjects.Size() ); }
	std::string FirstObjectStr();
	std::string LastObjectStr();
};


//
// Infomation Object
//
class PDFInfoObject : public PDFBodyObject
{
public:
	PDFInfoObject( int idP, int genNumberP, char useStateP )
		: PDFBodyObject( idP, genNumberP, useStateP ) {}
	~PDFInfoObject() {}
	std::string ObjectEntry();
	void SetInfoTitle( std::string titleP ) { title = titleP; }
	virtual const char *ClassName() { return "PDFInfoObject"; }
protected:
	std::string title;
};


//
// Font Object
//
class PDFFontObject : public PDFBodyObject
{
public:
	PDFFontObject( int idP, int genNumberP, char useStateP, std::string refFontNameP, std::string actFontNameP, int stateP )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
		refFontName = refFontNameP;
		actFontName = actFontNameP;
		state = stateP;
	}
	~PDFFontObject() {}
	std::string ObjectEntry();
	void EncodingObject( int encodingObjectNumP ) { encodingObjectNum = encodingObjectNumP; }
	std::string EncodingObjectStr() { return intToStr( encodingObjectNum ); }
	std::string FontName() { return refFontName; }
	virtual const char *ClassName() { return "PDFFontObject"; }
private:
	int encodingObjectNum;
	std::string refFontName;
	std::string actFontName;
	int state;
};


//
// Font Description Object
//
class PDFFontDesc : public PDFBodyObject
{
public:
	PDFFontDesc( int idP, int genNumberP, char useStateP )//, std::string fontNameP )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
		fontName = "NCOEIF+MS-Gothic";
	}
	~PDFFontDesc() {}
	std::string ObjectEntry();
	virtual const char *ClassName() { return "PDFFontDesc"; }
	int fontFileObjNum;
	std::string fontName;
};


//
// JapFlateDecode Font Object
//
/*class JapFlateDecode : public PDFBodyObject
{
public:
	JapFlateDecode( int idP, int genNumberP, char useStateP )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
		fontName = "NCOEIF+MS-Gothic";
	}
	~JapFlateDecode() {}
	std::string ObjectEntry();

	std::string ObjectEntry()
	{
		buffer += "<< /Filter /FlateDecode /Length 38739 /Length1 152568 >>\r";
		buffer += "stream\r";
		buffer += "<<\r";
		//buffer += japFontData; 
		buffer += ">>\r";
		AddContents( buffer );
		return PDFBodyObject::ObjectEntry();
	}
	int fontFileObjNum;
	std::string fontName;
};*/



//
// Encoding Object - used by the Font Object
//
class PDFEncodingObject : public PDFBodyObject
{
public:
	PDFEncodingObject( int idP, int genNumberP, char useStateP )
		: PDFBodyObject( idP, genNumberP, useStateP ) {}
	~PDFEncodingObject() {}
	virtual const char *ClassName() { return "PDFEncodingObject"; }
	std::string ObjectEntry();
};

#endif // PDFCORE_H
