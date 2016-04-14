/* PDFBase.h */

/* Change History

	30/07/01	RHF		Added missing #include <string.h>, Cleanup for Mac 4.0.4
	
*/

#ifndef PDFBASE_H
#define PDFBASE_H

#include <string>
#include <list>
#include <stdio.h>
#include <string.h>
#include "PDFSettings.h"



/*std::string intToStr( int num )
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
	sprintf( buf, "%.3g", num );
	intStr = buf;
	return intStr;
}*/

//static const int PDF_INVALID_COLOR = -1;
//static const char * THEFONTNAME = "TNRoman";
static const char *drawCommands = "0 g\rBX /GS1 gs EX\r";

/*// Text X positions...
const int PDF_TEXT_AT_NEXT_TAB = -1;
const int PDF_NEW_SECTION = -2;
const int PDF_X_POS_UNCHANGED = -3;
const int PDF_TEXT_CENTERED = -4;
static const char *PDF_LINE = "** PDF LINE **";

// Text Y positions...
const int PDF_TEXT_ON_NEXT_LINE = -1;*/
extern const char *PDF_BLANK;


//
// Base class for all of the PDF "Body" Objects
//
class PDFBodyObject
{
public:
	PDFBodyObject( int idP, int genNumberP, char useStateP ) 
	{
		id = idP;
		genNumber = genNumberP;
		useState = useStateP;
		byteOffset = -1;
		invisible = false;
		contents.reserve(5*1024);
		imageBuf = 0;
		imageLen = 0;
	}

	virtual ~PDFBodyObject() { if (imageBuf) delete[] imageBuf; imageBuf = 0; imageLen = 0; } 

	std::string ObjectHeader()
	{
		header =  GetIdStr();
		header += " ";
		header += GenNumberStr();
		header += " ";
		header += "obj\r";
#if _DEBUG
		header += "%";
		header += ClassName();
		header += "\r";
#endif
/*		header += "% Object byteoffset: ";
		header += intToStrInHex( ByteOffset() );
		header += "\r";*/
		return header;	
	}

	std::string GetContents() { return contents; }

	void AddContents( std::string str )
	{
		if (contents.length() == 0)
		{
			contents = ObjectHeader();
		}
		contents += str;
	}
	
	// Outputs the "main" data contents of this PDF body object
	virtual std::string ObjectEntry()
	{
		if (Invisible())
			return PDF_BLANK;

		contents += "endobj\r";
		return contents;
	}

	// Outputs the cross reference data, which is a used to keep track of where in a PDF file the "main" data
	// is located
	virtual	std::string XRefEntry()
	{
		if (Invisible())
			return PDF_BLANK;

		char buf[64];
		if ( ByteOffset() != -1 )
			sprintf( buf, "%0.10d %0.5d %c \r", ByteOffset(), GenNumber(), UseState() );
		else
		{
			sprintf( buf, "%0.10d %0.5d %c \r", ByteOffset(), GenNumber(), UseState() );
			strcat( buf, "% Error with byte offset for object above\r" );
		}
//		sprintf( buf, "%0.10d %0.5d %c %% %d\r", ByteOffset(), GenNumber(), UseState(), Id() );
		xRefEntry = buf;
		return xRefEntry;
	}

	// Access Functions
	int GenNumber() { return genNumber; } const
	void ByteOffset( int byteOffsetP ) { byteOffset = byteOffsetP; }
	int ByteOffset() { return byteOffset; }
	int Id() { return id; }
	int UseState() { return useState; }
	virtual const char *ClassName() { return "PDFBodyObject"; }
	
	// Convert integer values to strings
	std::string GetIdStr() { return intToStr( id ); } 
	std::string GenNumberStr() { return intToStr( genNumber ); } 
	void EraseContents() { contents.resize( 0 ); }

	bool Invisible() { return invisible; }
	//void SetInvisible( bool state ) { invisible = state; }
	void SetInvisible() { invisible = true; }
	void SetImageData( char *buf, int len )
	{
		imageBuf = buf;
		imageLen = len;
	}
	void GetImageData( char **buf, int &len )
	{
		*buf = imageBuf;
		imageBuf = 0;
		len = imageLen;
		imageLen = 0;
	}
	int ImageLen() const { return imageLen; }
protected:
	std::string intToStr( int num )
	{
		char buf[32];
		sprintf( buf, "%d", num );
		intStr = buf;
		return intStr;
	}
	std::string intToStrInHex( int num )
	{
		char buf[32];
		sprintf( buf, "%x", num );
		intStr = buf;
		return intStr;
	}

protected: // Data that is to be accessed by this class and inherited objects only
	std::string buffer;
	char *imageBuf;
	int imageLen;
private: // Data that is to be accessed by this class only
	int id;
	int genNumber;
	int byteOffset;
	char useState;
	std::string contents;
	std::string xRefEntry;
	std::string temp;
	std::string header;
	std::string intStr;
	bool invisible;
};

#define PDF_NO_LINK -1
class PDFLinkToPageObject : public PDFBodyObject
{
public:
	PDFLinkToPageObject( int idP, int genNumberP, char useStateP, int pageNumP = PDF_NO_LINK )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
		page = pageNumP;
	}
	~PDFLinkToPageObject() {}

	std::string ObjectEntry();
	void PageNum( int pageP ) { page = pageP; } 
	int PageNum() { return page; }
	virtual const char *ClassName() { return "PDFLinkToPageObject"; }
private:
	int page;
	std::string PageStr() { return intToStr( page ); }
};

class PDFLinkObject;
typedef std::list<PDFLinkObject*> PDFLinkObjectsListData;
typedef PDFLinkObjectsListData::iterator PDFLinkObjectsListDataIter;

class PDFLinkObjectsList
{
public:
	long Size();
	PDFLinkObject* Front();
	PDFLinkObject* Back();
	PDFLinkObjectsListDataIter Begin() { return listData.begin(); }
	PDFLinkObjectsListDataIter End() { return listData.end(); }
	void Next( PDFLinkObjectsListDataIter& iter );
	void Clear() { listData.clear(); }
	short Empty() { return listData.empty(); }
	void AddAtBack( PDFLinkObject* obj );
	void RemoveFromBack();
private:
	PDFLinkObjectsListData listData;
};
class PDFLinkObject : public PDFBodyObject
{
public:
	PDFLinkObject( int idP, int genNumberP, char useStateP, std::string text, PDFLinkToPageObject *aPDFLinkToPageObjectP )
		: PDFBodyObject( idP, genNumberP, useStateP ) 
	{
		title = text;
		thePDFLinkToPageObject = aPDFLinkToPageObjectP;
		/*if ( thePDFLinkToPageObject ) 
			Annote( thePDFLinkToPageObject->Id() );*/
		dest = PDF_BLANK;
		parent = PDF_NO_LINK;
		prev = PDF_NO_LINK;
		next = PDF_NO_LINK;
	}
	~PDFLinkObject() {}

	std::string ObjectEntry()
	{
		buffer =  "<<\r";
		buffer += "/Title (";
		buffer += title;
//		buffer += timesNRFont->ConvertPDFEncoding( timesNRFont->Translate( title ) );
		buffer += ")\r";
		if ( dest != PDF_BLANK )
		{
			buffer += "/Dest ";
			buffer += dest;
		}
		if ( AnnoteNum() != PDF_NO_LINK )
		{
			buffer += "/A ";
			buffer += AnnoteStr();
			buffer += " 0 R\r";
		}

		buffer += "/Parent ";
		buffer += ParentStr();
		buffer += " 0 R\r";

		if ( prev != PDF_NO_LINK )
		{
			buffer += "/Prev ";
			buffer += PrevStr();
			buffer += " 0 R\r";
		}
		if ( next != PDF_NO_LINK )
		{
			buffer += "/Next ";
			buffer += NextStr();
			buffer += " 0 R\r";
		}
		if ( PDFLinkObjects.Size() != 0  )
		{
			buffer += "/First ";
			buffer += FirstStr();
			buffer += " 0 R\r";
			buffer += "/Last ";
			buffer += LastStr();
			buffer += " 0 R\r";
			buffer += "/Count ";
			buffer += CountStr();
			buffer += "\r";
		}
		buffer += ">>\r";
		AddContents( buffer );
		return PDFBodyObject::ObjectEntry();
	}
	long WriteLinkedObjects( long byteOffset, FILE *theFP );
	void Title( std::string text ) { title = text; }
	const char *Title() { return title.c_str(); }
	void Parent( int parentP ) { parent = parentP; }
	void Prev( int prevP ) { prev = prevP; }
	int Prev() { return prev; }
	void Next( int nextP ) { next = nextP; }
	void Annote( int annoteNumP );
	virtual const char *ClassName() { return "PDFLinkObject"; }

	PDFLinkToPageObject *thePDFLinkToPageObject; 
	PDFLinkObjectsList PDFLinkObjects;
private:
	std::string title;
	std::string dest;
	int parent;
	std::string ParentStr() { return intToStr( parent ); }
	int AnnoteNum() { return thePDFLinkToPageObject->PageNum(); }
	std::string AnnoteStr() { return intToStr( thePDFLinkToPageObject->Id() ); }
	int prev;
	std::string PrevStr() { return intToStr( prev ); }
	int next;
	std::string NextStr() { return intToStr( next ); }
	std::string FirstStr()
	{
		if ( PDFLinkObjects.Size() )
//			if ( PDFLinkObjects.Front() )
				return intToStr( PDFLinkObjects.Front()->Id() );
		return "-1";
	}
	std::string LastStr()
	{
		if ( PDFLinkObjects.Size() )
			return intToStr( (PDFLinkObjects.Back())->Id() );
		return "-1";
	}
	std::string CountStr() { return intToStr( PDFLinkObjects.Size() * -1 ); }
};

#endif // PDFBASE_H
