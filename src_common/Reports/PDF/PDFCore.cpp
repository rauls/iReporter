
#include "PDFCore.h"
#include "myansi.h"
#include "OSInfo.h"
#include "version.h"


PDFHeader::PDFHeader()
{ 
	str =  "%PDF-1.3\r";	// Version number of PDF compatibility 
	str += "%‚„œ”\r";		// String to make this file a BINARY PDF file
}

std::string PDFExtGState::ObjectEntry()
{
	buffer =  "<<\r";
	buffer += "/Type /ExtGState\r";
	buffer += "/SA false\r";
	buffer += "/OP false\r";
	buffer += "/HT /Default\r";
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}


std::string PDFResourcesObject::ObjectEntry()
{
	buffer =  "<<\r";
	if ( !imageLinkDataList.size() )
		buffer += "/ProcSet [/PDF /Text ]\r";
	else
		buffer += "/ProcSet [/PDF /Text /ImageC /ImageI]\r";
	buffer += "/Font <<";

	for( PDFStrIntList::iterator iter = thePDFFontList.begin(); iter != thePDFFontList.end(); iter++ )
	{
		buffer += " ";
		buffer += "/";
		buffer += (*iter).theString;
		buffer += " ";
		buffer += intToStr( (*iter).theInt );
		buffer += " 0 R";
	}
	//buffer += "\r";
	/*buffer += GetFontName();
	buffer += " ";
	buffer += GetFontObjIdStr();
	buffer += " 0 R\r";*/
	buffer += " >>\r";
	if ( imageLinkDataList.size() )
	{
		buffer += "/XObject <<\r";
		for ( int i = 0; i < imageLinkDataList.size(); i++ )
		{
			buffer += "/";
			buffer += imageLinkDataList[i].imageName;
			buffer += " ";
			buffer += intToStr( imageLinkDataList[i].imageNum );
			buffer += " 0 R\r";
		}
		buffer += ">>\r";
	}

	buffer += "/ExtGState <<\r";
	buffer += "/GS1 ";
	buffer += GetExtGStateStr();
	buffer += " 0 R\r";
	buffer += ">>\r";
	if ( imageLinkDataList.size() )
	{
		for ( int i = 0; i < imageLinkDataList.size(); i++ )
		{
			buffer += "/ColorSpace <<\r";
			buffer += "/CS1 ";
			buffer += intToStr( imageLinkDataList[i].colorSpaceNum );
			buffer += " 0 R\r";
			buffer += ">>\r";
		}
	}
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFASCII85DecodeObject::ObjectEntry()
{
	buffer =  "<<\r";
	buffer += "/Filter /ASCII85Decode\r";
	buffer += "/Length ";
	buffer += intToStr( decodeData.length() );
	buffer += "\r>>\r";
	buffer += "stream\r";
	buffer += decodeData;
	buffer += "\r% Trev1\rendstream\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFColorSpaceObject::ObjectEntry()
{
	buffer += "[/Indexed /DeviceRGB 255 ";
	buffer += intToStr( PDFASCII85DecodeObjectNum );
	buffer += " 0 R]\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFImageObject::ObjectEntry()
{
//	if ( !ImageLen() )
//		buffer = ObjectHeader();
	buffer +=  "<<\r";
	buffer += "/Type /XObject\r";
	buffer += "/Subtype /Image\r";
	buffer += "/Name /";
	buffer += GetImageName();
	buffer += "\r";
	buffer += "/Width ";
	buffer += GetWidth();
	buffer += "\r";
	buffer += "/Height ";
	buffer += GetHeight();
	buffer += "\r";
	buffer += "/BitsPerComponent ";
	buffer += GetBitsPerComponent();
	buffer += "\r";

	buffer += "/ColorSpace ";
//	buffer += GetColorSpace();
//	buffer += "\r";
	buffer += GetColorSpaceObjIdStr();

	if ( !GetFilter().empty() ) 
	{
		buffer += "/Filter ";
		buffer += GetFilter();
		buffer += "\r";
	}

	buffer += "/Length ";
	if ( !ImageLen() )
		buffer += intToStr( imageData.length() );
	else
		buffer += intToStr( imageLen );
	buffer += "\r";

	buffer += ">>\r";
	buffer += "stream\r";
	if ( !ImageLen() )
	{
		buffer += imageData;
		buffer += "\r% Trev2\rendstream\r";
	}
	AddContents( buffer );
	if ( !ImageLen() )
		return PDFBodyObject::ObjectEntry();
	else
		return GetContents();
}

std::string PDFHalftone::ObjectEntry()
{
	buffer =  "<<\r";
	buffer += "/Type /Halftone\r";
	buffer += "/HalftoneType 1\r";
	buffer += "/HalftoneName (Default)\r";
	buffer += "/Frequency 60\r";
	buffer += "/Angle 45\r";
	buffer += "/SpotFunction /Round\r";
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

void PDFDataContentObject::AddDataContentsData( std::string data, std::string data2 )
{
	buffer = "<<\r";
	if ( commentsOn )
	{
		buffer += "% Contents Page Num: ";
		buffer += intToStr( physicalPageNumber );
		buffer += "\r";
	}
	buffer += "/Length ";
	buffer += intToStr( text.length() + data.length() + data2.length() );
	buffer += "\r>>\r";
	buffer += "stream\r";
	buffer += text;
	buffer += data;
	buffer += data2;
	buffer += "\r% Trev3\rendstream\r";
}

std::string PDFURIObject::ObjectEntry()
{
	str =  "<<\r";
	str += "/Type /Action\r"; 
	str += "/S /URI\r"; 
	str += "/URI (";
	str += URL();
	str += ")\r>>\r";
	AddContents( str );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFAnnotURILinkObject::ObjectEntry()
{
	str =  "<<\r";
	str +=  "/Type /Annot\r"; 
	str +=  "/Subtype /Link\r"; 
	str +=  "/Rect ";
	str +=  URIObj->GetLinkRectStr();
	str +=  "\r/A ";
	str +=  URIObj->GetIdStr();
	str += " 0 R\r"; 
	str +=  "/Border [ 0 0 0 ]\r";			// If we want to set the border then use "[ 0 0 1 ]"
//	str +=  "/C [1.0000 0.0000 0.0000]\r";	// Make the border red!
	str +=  "/F 0\r";

	str += ">>\r";
	AddContents( str );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFAnnotInternalLinkObject::ObjectEntry()
{
	str =  "<<\r";
	str +=  "/Type /Annot\r"; 
	str +=  "/Subtype /Link\r"; 
	str +=  "/Dest ";
	str +=  dest;
	str +=  "/Rect ";
	str +=  GetLinkRectStr();
	str +=  "\r";
	str +=  "/Border [ 0 0 0 ]\r"; 
	str += ">>\r";
	AddContents( str );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFPageObject::ObjectEntry()
{
	str =  "<<\r";
	if ( commentsOn )
	{
		str += "% Phys Page Num: ";
		str += intToStr( physicalPageNumber );
		str += "\r";
	}
	str += "/Type /Page\r";
	str += "/Parent ";
	str += GetParentObjectStr();
	str += " 0 R\r";
	str += "/Resources ";
	str += GetResourceObjectStr();
	str += " 0 R\r";
	str += "/Contents ";
	str += GetContentObjectStr();
	str += " 0 R\r";
	if (annotsList.size() )
	{
		str += "/Annots ";
		str += GetAnnotsStr(); 
	}
	str += ">>\r";
	AddContents( str );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFPageObject::GetAnnotsStr()
{
	std::string list = "[ ";
	PDFAnnotObject *aPDFAnnotObject;
	int i = 0;
	while ( annotsList.size() != 0 )
	{
		aPDFAnnotObject = (PDFAnnotObject*)annotsList.front();
		list += aPDFAnnotObject->GetIdStr();
		list += " 0 R ";
		if ( i == 10 )
		{
			list += "\r"; // Just to break the line up...
			i = 0;
		}
		annotsList.pop_front();
		i++;
	}
	list += "]\n";
	annotsList.clear();
	return list;
}

void PDFPageObject::AddAnnote( PDFAnnotObject *linkToURI )
{
	annotsList.push_back( (PDFBodyObject*) linkToURI );
}

std::string PDFPagesListObject::ObjectEntry()
{
	buffer =  "<<\r";
	buffer += "/Type /Pages\r";
	buffer += KidPagesStr();
	buffer += "/Count ";
	buffer += intToStr( PageCount() );
	buffer += "\r";
	buffer += SizeStr();
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

const char *PDFPagesListObject::KidPagesStr()
{
	temp = "/Kids [";
	PDFPageList::iterator iter;
	int i = 0;
	int count = 1;
	for ( iter = kidPages.begin(); iter != kidPages.end(); iter++ )
	{
		if (count > 1)
			temp += " ";
		temp += intToStr( (*iter)->Id() );
		temp += " 0 R";
		if ( i == 10 )
		{
			temp += "\r"; // Just to break the line up...
			i = 0;
		}
		count++;
		i++;
	}
	temp += "]\r";
	return temp.c_str();
}

std::string	PDFPagesListObject::SizeStr()
{
	temp =  "/MediaBox [0 0 ";
	temp += intToStr( pageWidth );
	temp += " ";
	temp += intToStr( pageHeight );
	temp += "]\r";
	return temp;
}

std::string PDFCatelogObject::ObjectEntry()
{
	buffer =  "<<\r";
	buffer += "/Type /Catalog\r";

	if ( pagesObjectNum != -1 )
	{
		buffer += "/Pages ";
		buffer += PagesObjectStr();
		buffer += " 0 R\r";
	}

	if ( outlinesObjectNum != -1 )
	{
		buffer += "/Outlines ";
		buffer += OutlinesObjectStr();
		buffer += " 0 R\r";
		buffer += "/PageMode /UseOutlines\r";
	}
//	buffer += "/ViewerPreferences /FitWindow\r";
	buffer += "/ViewerPreferences /CenterWindow\r";
	
	//buffer += "/Names ";
	//buffer += NamesObjectStr();
	//buffer += " 0 R\r";
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

PDFOutlinesObject::PDFOutlinesObject( int idP, int genNumberP, char useStateP )
	: PDFBodyObject( idP, genNumberP, useStateP )
{
	//ClearCurrLinkObjectList();
}

void PDFOutlinesObject::ClearCurrLinkObjectList()
{
	bool integrityCheck = true;
	for( PDFLinkObjectsListDataIter iter = currLinkObjectList.Begin(); iter != currLinkObjectList.End(); currLinkObjectList.Next( iter ) )
	{
		PDFLinkObject *linkObj = (*iter);
		if ( linkObj->thePDFLinkToPageObject->PageNum() == PDF_NO_LINK )
			integrityCheck = false;
	}
	
	currLinkObjectList.Clear();
}

bool PDFOutlinesObject::AreThereCurrLinkObjects()
{
	return (currLinkObjectList.Size() != 0);
}

PDFLinkObject *PDFOutlinesObject::GetFrontCurrLinkObject()
{
	return currLinkObjectList.Front();
}

PDFLinkObject *PDFOutlinesObject::GetBackCurrLinkObject()
{
	return currLinkObjectList.Back();
}

long PDFOutlinesObject::CurrLinkObjectListSize()
{
	return currLinkObjectList.Size();
}

void PDFOutlinesObject::LinkAllCurrLinkObjectsToPage( int currPageNum )
{
	for( PDFLinkObjectsListDataIter iter = currLinkObjectList.Begin(); iter != currLinkObjectList.End(); currLinkObjectList.Next( iter ) )
	{
		PDFLinkObject *linkObj = (*iter);
		linkObj->Annote( currPageNum );
	}
}


std::string PDFOutlinesObject::ObjectEntry()
{
	buffer =  "<<\r";
	buffer += "/Count ";
	buffer += CountStr();
	buffer += "\r/First ";
	buffer += FirstObjectStr();
	buffer += " 0 R\r";
	buffer += "/Last ";
	buffer += LastObjectStr();
	buffer += " 0 R\r";
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

void PDFOutlinesObject::AddOutline( PDFLinkObject* aPDFLinkObject, int level /*= 0*/ )
{
	if (level == 0)
	{
		aPDFLinkObject->Parent( Id() );
		if (PDFLinkObjects.Size() > 0)
		{
			PDFLinkObject *prevPDFLinkObject = PDFLinkObjects.Back();
			// Check that the 
			prevPDFLinkObject->Next( aPDFLinkObject->Id() );
			aPDFLinkObject->Prev( prevPDFLinkObject->Id() );
		}
		PDFLinkObjects.AddAtBack( aPDFLinkObject );
		return;
	}
	if (level == 1)
	{
		PDFLinkObject *parentLinkObj = PDFLinkObjects.Back();
		if (parentLinkObj)
		{
			aPDFLinkObject->Parent( parentLinkObj->Id() );
			if (parentLinkObj->PDFLinkObjects.Size() > 0)
			{
				PDFLinkObject *prevPDFLinkObject = parentLinkObj->PDFLinkObjects.Back();
				prevPDFLinkObject->Next( aPDFLinkObject->Id() );
				aPDFLinkObject->Prev( prevPDFLinkObject->Id() );
			}
			parentLinkObj->PDFLinkObjects.AddAtBack( aPDFLinkObject );
		}
		return;
	}
	if (level == 2)
	{
		PDFLinkObject *parentLinkObj = PDFLinkObjects.Back();
		PDFLinkObject *secondLevelParentLinkObj = parentLinkObj->PDFLinkObjects.Back();
		if (secondLevelParentLinkObj)
		{
			aPDFLinkObject->Parent( secondLevelParentLinkObj->Id() );
			if (secondLevelParentLinkObj->PDFLinkObjects.Size() > 0)
			{
				PDFLinkObject *prevPDFLinkObject = secondLevelParentLinkObj->PDFLinkObjects.Back();
				prevPDFLinkObject->Next( aPDFLinkObject->Id() );
				aPDFLinkObject->Prev( prevPDFLinkObject->Id() );
			}
			secondLevelParentLinkObj->PDFLinkObjects.AddAtBack( aPDFLinkObject );
		}
		return;
	}
}

std::string PDFOutlinesObject::FirstObjectStr()
{
	if ( PDFLinkObjects.Size() )
		return intToStr( (PDFLinkObjects.Front())->Id() );
	return "-1";
}

std::string PDFOutlinesObject::LastObjectStr()
{
	if ( PDFLinkObjects.Size() )
		return intToStr( (PDFLinkObjects.Back())->Id() );
	return "-1";
}

std::string PDFOutlinesObject::LinkToPageString( std::string& title, int currPageNum )
{
	if ( title.find("<LINK") != std::string::npos )
	{
		int linkNumPos = title.find_first_of( "0123456789" );
		char c = title[linkNumPos];
		std::string linkStr;
		while( title[linkNumPos] >= '0' && title[linkNumPos] <= '9' )
		{
			linkStr += title[linkNumPos];
			linkNumPos++;
		}
		int linkNum = myatoi( (char*)linkStr.c_str() );
		int i=0;
		for( PDFLinkObjectsListDataIter iter = currLinkObjectList.Begin(); iter != currLinkObjectList.End(); currLinkObjectList.Next( iter ), i++ )
		{
			PDFLinkObject *linkObj; 
			//if (linkNum == i)
			linkObj = (*iter); 
			linkObj->Annote( currPageNum );
			/*for( PDFLinkObjectsListDataIter iter2 = linkObj->PDFLinkObjects.Begin(); iter2 != linkObj->PDFLinkObjects.End(); linkObj->PDFLinkObjects.Next( iter2 ) )
			{
				PDFLinkObject *linkObj2; 
				linkObj2 = (*iter2); 
				linkObj2->Annote( currPageNum );
			}*/
		}
		int pos = title.find(">");
		std::string newTitle = title.erase(0, pos+1);
		return newTitle;
	}
	return title;
}

int PDFOutlinesObject::AddLinkObject( PDFLinkObject* aPDFLinkObject, int level, std::string& text, int forceLink /*= 0*/ )
{
	currLinkObjectList.AddAtBack( aPDFLinkObject );
	AddOutline( aPDFLinkObject, level );
	if ( level != 0 || forceLink )
		AddLinkToText( text, currLinkObjectList.Size()-1 );
	return currLinkObjectList.Size();
}

void PDFOutlinesObject::AddLinkToText( std::string& text, int outlineNum )
{
	std::string temp = text;
	text = "<LINK";
	text += intToStr( outlineNum );
	text += ">";
	text += temp;
}

std::string PDFInfoObject::ObjectEntry()
{
	unsigned long origLen = 128;
	unsigned long len = origLen;
	unsigned long retLen;

	char buf[128+1];

	buffer += "<<\r";
	buffer += "/CreationDate (";
	retLen = GetCurrDateTime( buf, &len );
	len = origLen;
	buffer += buf;
	buffer += ")\r";

	buffer += "/Author (";
	GetUserName( buf, &len );
	len = origLen;
	buffer += buf;
	buffer += ")\r";

	buffer += "/Creator (";
	retLen = GetAppBuildDetails( buf );
	len = origLen;
	buffer += buf;
	buffer += ")\r";
	
	buffer += "/Producer (OS - ";
	GetOSVersion( buf, &len );
	len = origLen;
	buffer += buf;
	buffer += ")\r";

	buffer += "/Title (";
	buffer += title;
	buffer += ")\r";

	buffer += "/Subject (Web Server log file analysis.)\r";

	buffer += ">>\r";

	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFFontObject::ObjectEntry()
{
	buffer += "<<\r";
	buffer += "/Type /Font\r";
	buffer += "/Subtype /Type1\r";
	buffer += "/Name /"; 
	buffer += FontName();
	buffer += "\r/Encoding ";
	buffer += EncodingObjectStr();
	buffer += " 0 R\r";
//		buffer += "/BaseFont /MS Gothic\r";
	buffer += "/BaseFont /";
	buffer += actFontName;// "Times";
//		buffer += "/BaseFont /Courier\r";
//		buffer += "/BaseFont /Geneva\r";
	
	if (actFontName == "Times")
	{
		if ( state & PDF_BOLD && state & PDF_ITALIC )
			buffer += "-BoldItalic";
		else if ( state & PDF_BOLD )
			buffer += "-Bold";
		else if ( state & PDF_ITALIC )
			buffer += "-Italic";
		else
			buffer += "-Roman";
	}
	else if (actFontName == "Helvetica")
	{
		if ( state & PDF_BOLD && state & PDF_ITALIC )
			buffer += "-BoldOblique";
		else if ( state & PDF_BOLD )
			buffer += "-Bold";
		else if ( state & PDF_ITALIC )
			buffer += "-Oblique";
	}
	
	buffer += "\r";

//		buffer += "/Leading 4\r";
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFFontDesc::ObjectEntry()
{
	buffer += "<<\r";
	buffer += "/Type /FontDescriptor\r";
	buffer += "/Ascent 859\r";
	buffer += "/CapHeight 0\r";
	buffer += "/Descent -136\r";
	buffer += "/Flags 5\r";
	buffer += "/FontBBox [ 0 -137 996 859 ]\r";
	buffer += "/FontName /";
	buffer += fontName;
	buffer += "\r";
	buffer += "/ItalicAngle 0\r";
	buffer += "/StemV 0\r";
	buffer += "/FontFile2 ";
	buffer += intToStr( fontFileObjNum );
	buffer += "0 R\r"; 
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFEncodingObject::ObjectEntry()
{
	buffer += "<<\r";
	buffer += "/Type /Encoding\r";
//	buffer += "/BaseEncoding /MacRomanEncoding\r";
//	buffer += "/BaseEncoding /PDFDocEncoding\r";
	buffer += "/BaseEncoding /WinAnsiEncoding\r";
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

std::string PDFLinkToPageObject::ObjectEntry()
{
	buffer =  "<<\r";
//	buffer += "/Type /Action\r";
	buffer += "/S /GoTo\r";
	buffer += "/D [ ";
	buffer += PageStr();
	buffer += " 0 R /Fit ]\r";
	buffer += ">>\r";
	AddContents( buffer );
	return PDFBodyObject::ObjectEntry();
}

long PDFLinkObjectsList::Size()
{
	return listData.size();
}

PDFLinkObject* PDFLinkObjectsList::Front()
{
	if ( !Size() )
		return 0;
	PDFLinkObject *obj = listData.front();
	return obj;
}

PDFLinkObject* PDFLinkObjectsList::Back()
{
	if ( !Size() )
		return 0;
	PDFLinkObject *obj = listData.back();
	return obj;
}


void PDFLinkObjectsList::AddAtBack( PDFLinkObject* obj )
{
	listData.push_back( obj );
}

void PDFLinkObjectsList::RemoveFromBack()
{
	listData.pop_back();
}

void PDFLinkObjectsList::Next( PDFLinkObjectsListDataIter& iter )
{
	iter++;
}

void PDFLinkObject::Annote( int annoteNumP )
{
	thePDFLinkToPageObject->PageNum( annoteNumP );
}

long PDFLinkObject::WriteLinkedObjects( long byteOffset, FILE *theFP )
{
	std::string fileData;
	for( PDFLinkObjectsListDataIter iter = PDFLinkObjects.Begin();
		iter != PDFLinkObjects.End(); PDFLinkObjects.Next( iter ) )
	{
		// Write the Link objects
		PDFLinkObject *aPDFLinkObject = (*iter);
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

		// Write all the sub-Linkobjects
		byteOffset = aPDFLinkObject->WriteLinkedObjects( byteOffset, theFP );

		// Erase the contents of the Link and LineToPage objects
//			aPDFLinkToPageObject->EraseContents();
//			aPDFLinkObject->EraseContents();
	}
	return byteOffset;
}


