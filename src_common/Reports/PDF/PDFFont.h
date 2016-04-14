#ifndef PDFFONT_H
#define PDFFONT_H

#include "Compiler.h"

extern const char *PDF_FONT_BLANK;

#include <string>
#include <list>
#include "PDFColors.h"

#if (DEF_MAC || DEF_UNIX)
#include <stdio.h>
#include <string.h>
#endif

static const int PDF_INVALID_SIZE = -1;
static const int PDF_NO_SIZECHANGE = -2;

static const char *THEFONTNAME = "TNRoman";

class PDFStrInt
{
public:
	PDFStrInt( std::string theStringP, int theIntP )
	{
		theString = theStringP;
		theInt = theIntP;
	}
	std::string theString;
	int theInt;
};
typedef std::list<PDFStrInt> PDFStrIntList;


//
// PDFFont - specifies a font which is used by PDF
//
class PDFFont
{
public:
	PDFFont( std::string fontNameP, int fontSizeP, int fontSpacingP );
	virtual ~PDFFont();
	virtual void InitFontStyleList();
	std::string FontChangeStr( int style = PDF_NORMAL, int size = PDF_NO_SIZECHANGE, int color = PDF_BLACK );
	std::string FontName() const;
	std::string StyleName( int style );
	int FontSize() const { return fontSize; };
	void FontSize( int fontSizeP ) { fontSize = fontSizeP; }
	int DefaultFontSize() const { return defaultFontSize; };
	float FontSpacing();
	float FontSpacing( int size );
	float FontHeight();
	float FontHeight( int size );
	float FontMultiLineExtraLineHeight( int size );
	void ResetTextColor();
	void ResetFillColor();
	void ResetLineColor() { currLineCol = PDF_INVALID_COLOR; }
	void ResetSolidColor() { currSolidCol = PDF_INVALID_COLOR; }
	int LineColor() const { return currLineCol; }
	int TextColor() const;
	int SolidColor() const { return currSolidCol; }
	std::string TextColor( long color, int forceOutput = 0 );
	std::string FillColor( long color, int forceOutput = 0 );
	std::string LineColor( long color, int forceOutput = 0 );
	std::string SolidColor( long color, int forceOutput = 0 );
	void ResetStyle() { currFontStyle = PDF_INVALID_STYLE; }
	int CurrentStyle()  const{ return currFontStyle; }
	void CurrentStyle( int fontStyle );
	void ResetSize() { FontSize( PDF_INVALID_SIZE ); }
	void ResetFont() { ResetTextColor(); ResetFillColor(); ResetSize(); ResetStyle(); }

	PDFStrIntList& StyleList() { return fontStyleNameList; }
protected:
	int fontSize;
	int defaultFontSize;
private:
	std::string floatToStr( float num )
	{
		char buf[32];
		sprintf( buf, "%.7g", num );
		return buf;
	}
	std::string fontName;
	int fontSpacing;
	int currLineCol;
	int currSolidCol;
	int currFontStyle;
	PDFStrIntList fontStyleNameList;
};

//
// CharOnePointSize allows (1) character & (2) it's width.  The width is stored in PDF pixels
// for a 1-point font size.
//
class CharOnePointSize
{
public:
	CharOnePointSize( unsigned char theCharP, float widthP )
	{
		theChar = theCharP;
		width = widthP;
	}
	unsigned char C() { return theChar; }
	double W() { return width; }
private:
	unsigned char theChar;
	double width;
};

typedef struct CharWidthsTag
{
	unsigned char code;
	double width;
} CharWidths;


typedef std::list<CharOnePointSize> CharOnePointSizeList;

// PDFMultiLineCellTextStr class represents a line of text in a cell which maybe multi-lined (ie. have serval lines)
class PDFMultiLineCellTextStr
{
public:
	PDFMultiLineCellTextStr( std::string strP, double lenP, std::string str2P = "", double len2P = 0 )
	{
		textOnaLineInACell = strP;
		lengthOfTextOnaLineInACell = lenP;
		dualTextOnaLineInACell = str2P;
		lengthOfDualTextOnaLineInACell = len2P;
	}
	std::string Str() { return textOnaLineInACell; }
	void SetStr( std::string strP ) { textOnaLineInACell = strP; }
	double StrLen() { return lengthOfTextOnaLineInACell; }
	void SetStrLen( int lenP ) { lengthOfTextOnaLineInACell = lenP; }
	std::string DualStr() { return dualTextOnaLineInACell; }
	void SetDualStr( std::string strP ) { dualTextOnaLineInACell = strP; }
	double DualStrLen() { return lengthOfDualTextOnaLineInACell; }
	void SetDualStrLen( int lenP ) { lengthOfDualTextOnaLineInACell = lenP; }
private:
	std::string textOnaLineInACell; // Text in on a line in cell in a table
	double lengthOfTextOnaLineInACell; // Length of the above text
	std::string dualTextOnaLineInACell; // For Dual Text on a line in cell in a table
	double lengthOfDualTextOnaLineInACell; // Length of the above Dual Text
};

typedef std::list< PDFMultiLineCellTextStr > PDFMultiLineCellTextStrList;

//
// PDFCharScaleFont is a base class which can be derived from to allow a font's character widths to be specified.
// The font's points size can be changed, or . for a give the specification of diff
//
class PDFCharScaleFont : public PDFFont
{
public:
	PDFCharScaleFont( std::string fontNameP, int fontSizeP, int fontSpacingP );
	//virtual ~PDFCharScaleFont();
	double GetCharWidth( unsigned char c, int specificFontSize /*= 0*/, int style /*= PDF_NORMAL*/ );
	double GetStringLen( std::string str, int specificFontSize, int style /*= PDF_NORMAL*/ );
	int GetStringLenWithSpacing( std::string str, int specificFontSize, int style /*= PDF_NORMAL*/ );
	std::string SetStringLen( const std::string& str, double lenWanted, int size /*= 0*/, short truncateTextLeftOrRight /*= PDF_LEFT_ELLIPSE*/, int style /*= PDF_NORMAL*/ );
	bool AnySplitChars( const std::string& str );
	// Change a normal string into a PDF string which has special characters which need to be 
	// preceeded by a "\" in order to keep a PDF file happy, characters a re
	std::string MakePDFString( std::string str );
	void TranslateHTMLSpecialTokens( std::string &str );
	bool ValidateHTMLString( std::string& str );
	void SplitHTMLStringAtParagraphs( std::string& splitAtParagraphs, short size, short style, PDFMultiLineCellTextStrList& cellStrList );
	const char* ConvertPDFEncoding( std::string str );//, int FromEncoding = MacRomanEncoding, int ToEncoding = StandardEncoding )
	// Need to convert a char to the from PDF "MacRomanEncoding" encoding to PDF "StandardEncoding",
	// however, if the char doesn't exist in "StandardEncoding" then we should convert it to the nearest equivalent.
	void ConvertPDFMacRomanEncodingToPDFDocEncodingCharCode( char *str );
protected:
	// Functions
	inline int CompareSpecificHTMLCode( std::string charCodeStr, std::string cmpCharCodeStr, int charCodeValue );
	std::string ConvertHTMLStrCharCodeToMacRomanEncoding( char *str );
	std::string ConvertHTMLStrCharCodeToPDFDocEncoding( char *str );
	void InitCharWidthOnePointArray();
	void InitHTMLTokenToPDFCharMaps();
	void SetNormalCharWidths( CharWidths charWidth[] );
	void SetBoldCharWidths( CharWidths charWidth[] );
	void SetItalicCharWidths( CharWidths charWidth[] );
	void SetBoldItalicCharWidths( CharWidths charWidth[] );
	double charOnePointSizeNormalArray[256];
	double charOnePointSizeBoldArray[256];
	double charOnePointSizeItalicArray[256];
	double charOnePointSizeBoldItalicArray[256];
	const char *splitChars;
};

class PDFTimesNewRomanFont : public PDFCharScaleFont
{
public:
	PDFTimesNewRomanFont( std::string fontNameP, int fontSizeP, int fontSpacingP = 4 );
	void InitNormalCharWidths();
	void InitBoldCharWidths();
	void InitItalicCharWidths();
	void InitBoldItalicCharWidths();
};



class PDFHelveticaFont : public PDFCharScaleFont
{
public:
	PDFHelveticaFont( std::string fontNameP, int fontSizeP, int fontSpacingP = 4 );
	void InitNormalCharWidths();
	void InitBoldCharWidths();
	void InitItalicCharWidths();
	void InitBoldItalicCharWidths();
};

// PDFTextStr X positions...
const int PDF_TEXT_AT_NEXT_TAB = -1;
const int PDF_NEW_SECTION = -2;
const int PDF_NON_COLUMN = -8;
const int PDF_X_POS_UNCHANGED = -3;
const int PDF_TEXT_CENTERED = -4;
const int PDF_TEXT_RIGHT = -5;
const int PDF_TEXT_COLUMN = -6;
const int PDF_TEXT_OUTLINE = -7;
const int PDF_TEXT_AT_FIRST_TAB = -9;

// PDFTextStr Y positions...
const int PDF_TEXT_ON_NEXT_LINE = -1;
// Used to represent that we want to draw a line
static const char *PDF_LINE = "** PDF LINE **";
static const char *PDF_BLANKLINE = "** PDF BLANKLINE **";

class PDFTextStr
{
public:
	PDFTextStr( std::string textP, int xP, int yP, int stateP, int sizeP = 9, int colorP = PDF_BLACK )
	{
		text = textP;
		x = xP;
		y = yP;
		state = stateP;
		size = sizeP;
		color = colorP;
	}
	std::string text;
	int x, y;
	int state;
	int size;
	int color;
};

typedef std::list<int> intList;
typedef intList::iterator IntIter;

typedef	std::list<PDFTextStr> PDFTextList;
typedef	PDFTextList::iterator PDFTextListIter;

#endif // PDFFONT_H

