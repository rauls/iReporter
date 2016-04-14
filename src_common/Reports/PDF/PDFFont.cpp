
#include "compiler.h"

#include <map>
#include <vector>
#include "PDFFont.h"

const char *PDF_FONT_BLANK = "";

PDFFont::PDFFont( std::string fontNameP, int fontSizeP, int fontSpacingP )
{
	fontName = fontNameP;
	fontSize = fontSizeP;
	defaultFontSize = fontSize;
	fontSpacing = fontSpacingP;
	ResetTextColor();
	ResetFillColor();
	InitFontStyleList();
}

PDFFont::~PDFFont()
{
	fontStyleNameList.clear();
}

void PDFFont::InitFontStyleList()
{
	ResetStyle();
	fontStyleNameList.push_back( PDFStrInt( PDF_FONT_BLANK, PDF_NORMAL ) ); 
	fontStyleNameList.push_back( PDFStrInt( "BldItlc", PDF_BOLDITALIC) ); 
	fontStyleNameList.push_back( PDFStrInt( "Bld", PDF_BOLD ) ); 
	fontStyleNameList.push_back( PDFStrInt( "Itlc", PDF_ITALIC ) ); 
}

#define FONT_SPACING_RATIO	1.25
#define FONT_SPACING_MIN	2

float PDFFont::FontSpacing()
{
	//return fontSpacing;
	static float spacing;
	//static int spacing;
	spacing = (FontSize()*FONT_SPACING_RATIO)-FontSize();
	if ( spacing < FONT_SPACING_MIN )
		return FONT_SPACING_MIN;
	return spacing;
}

float PDFFont::FontSpacing( int size )
{
	static float spacing;
	spacing = (size*FONT_SPACING_RATIO)-size;
	if ( spacing < FONT_SPACING_MIN )
		return FONT_SPACING_MIN;
	return spacing;
}

float PDFFont::FontHeight()
{
	return FontSize() + FontSpacing();
}

float PDFFont::FontHeight( int size )
{
	/*static float height;
	//static int height;
	height = size*FONT_SPACING_RATIO;
	if ( height < size+2 )
		return size+2;
	return height;*/
	return size + FontSpacing( size );
}

float PDFFont::FontMultiLineExtraLineHeight( int size )
{
	/*static float height;
	//return size + FontSpacing()/2;
	height = size + FontSpacing()/2;//size * FONT_SPACING_RATIO/2;
	return height;*/
	return size + FontSpacing( size )/2;
}

std::string PDFFont::FontChangeStr( int style /*= PDF_NORMAL*/, int size /*= PDF_NO_SIZECHANGE*/, int color /*= PDF_BLACK*/ )
{
	if ( (size == PDF_NO_SIZECHANGE || size == FontSize()) && size != PDF_INVALID_SIZE && style == CurrentStyle() )
	{
		if ( size != PDF_INVALID_SIZE )
		{
			if ( color == TextColor() && TextColor() != PDF_INVALID_COLOR )
				return PDF_FONT_BLANK; // No need to change font
			else
				return TextColor( color );
		}
		else
			size = defaultFontSize;
	}

	// Need to change the font
	std::string fontChangeStr;
	fontChangeStr = "/";
	fontChangeStr += FontName();
	fontChangeStr += StyleName( style );
	fontChangeStr += " ";
	if ( size == PDF_INVALID_SIZE )
		FontSize( DefaultFontSize() );
	else if ( size != PDF_NO_SIZECHANGE )
		FontSize( size );
	else if ( FontSize() == PDF_INVALID_SIZE )
		FontSize( DefaultFontSize() );
	fontChangeStr += floatToStr( FontSize() );
	fontChangeStr += " Tf\r";
	currFontStyle = style;

	if ( color != TextColor() || TextColor() == PDF_INVALID_COLOR  )
		fontChangeStr += TextColor( color );

	return fontChangeStr;
}

std::string PDFFont::FontName() const
{
/*	std::string fontStyleStr;
	for( PDFStrIntList::iterator iter = fontStyleNameList.begin(); iter != fontStyleNameList.end(); iter++ )
	{
		if (style == (*iter).theInt)
			fontStyleStr = (*iter).theString;
	}*/
//	if (!fontStyleStr.empty())
//		return fontName + "," + fontStyleStr;
//	else
		return fontName;
}

std::string PDFFont::StyleName( int style )
{
	std::string fontStyleStr;
	for( PDFStrIntList::iterator iter = fontStyleNameList.begin(); iter != fontStyleNameList.end(); iter++ )
	{
		if (style == (*iter).theInt)
			fontStyleStr = (*iter).theString;
	}

	if (fontStyleStr.empty()) // We have not found a style, so use the standard normal font style
	{
		for( PDFStrIntList::iterator iter = fontStyleNameList.begin(); iter != fontStyleNameList.end(); iter++ )
		{
			if (PDF_NORMAL == (*iter).theInt)
				fontStyleStr = (*iter).theString;
		}
	}

	// 
	CurrentStyle( style );

	return fontStyleStr;
}

void PDFFont::CurrentStyle( int fontStyle )
{
	//if ( fontStyle < PDF_NO
	currFontStyle = fontStyle;
}

int PDFFont::TextColor() const
{
	if (LineColor() != SolidColor())
		return PDF_INVALID_COLOR;
	else
		return SolidColor();
}

void PDFFont::ResetTextColor()
{
	ResetLineColor();
	ResetSolidColor();
}

void PDFFont::ResetFillColor()
{
	ResetLineColor();
	ResetSolidColor();
}


std::string PDFFont::TextColor( long color, int forceOutput /*= 0*/ )
{
	return SolidColor( color, forceOutput ) + LineColor( color, forceOutput );
}

std::string PDFFont::FillColor( long color, int forceOutput /*= 0*/ )
{
	return SolidColor( color, forceOutput ) + LineColor( color, forceOutput );
}

std::string PDFFont::LineColor( long color, int forceOutput /*= 0*/ )
{
	// Make sure the color we are changing to 
	if (color < PDF_BLACK)
		color = PDF_BLACK;

	if ( (color == LineColor() && LineColor() != PDF_INVALID_COLOR) && !forceOutput) // Check to see if we need to change the current text color or not
		return PDF_FONT_BLANK;

	currLineCol = color; // Set the new color as the current

	// Divide the color up into RGB components
	long red,green,blue;
	red = (color & 0xff0000) >> 16;
	green = (color & 0xff00) >> 8;
	blue = (color & 0xff);

	// Get the RGB color parts as a value between 0 and 1
	float r = red /(float) 255;
	float g = green /(float) 255;
	float b = blue /(float) 255;

	std::string colBuf;
	if (r == g && r == b) // If all colors are the same use short-hand notation to set the color
	{
		colBuf = floatToStr( r );
		colBuf += " G\r";
		return colBuf;
	}

	// Set the color using normal (not short-hand) RGB values
	colBuf = floatToStr( r );
	colBuf += " ";
	colBuf += floatToStr( g );
	colBuf += " ";
	colBuf += floatToStr( b );
	colBuf += " RG\r";

	return colBuf;
}


std::string PDFFont::SolidColor( long color, int forceOutput /*= 0*/ )
{
	// Make sure the color we are changing to 
	if (color < PDF_BLACK)
		color = PDF_BLACK;

	if ( (color == SolidColor() && SolidColor() != PDF_INVALID_COLOR) && !forceOutput) // Check to see if we need to change the current text color or not
		return PDF_FONT_BLANK;

	currSolidCol = color; // Set the new color as the current

	// Divide the color up into RGB components
	long red,green,blue;
	red = (color & 0xff0000) >> 16;
	green = (color & 0xff00) >> 8;
	blue = (color & 0xff);

	// Get the RGB color parts as a value between 0 and 1
	float r = red /(float) 255;
	float g = green /(float) 255;
	float b = blue /(float) 255;

	std::string colBuf;
	if (r == g && r == b) // If all colors are the same use short-hand notation to set the color
	{
		colBuf = floatToStr( r );
		colBuf += " g\r";
		return colBuf;
	}

	// Set the color using normal (not short-hand) RGB values
	colBuf = floatToStr( r );
	colBuf += " ";
	colBuf += floatToStr( g );
	colBuf += " ";
	colBuf += floatToStr( b );
	colBuf += " rg\r";

	return colBuf;
}

typedef struct htmlTokenToPDFCharMap
{
	int htmlIndexStrNameIndex;
	int htmlIndexHashNumIndex;
	char *htmlTokenName;
	int pdfEncodingNum;
} HTMLTokenToPDFCharMap;

static HTMLTokenToPDFCharMap HTMLTokenToPDFCharMapArray[] = 
{
'Dot\0',-1,		"Dot",		56,		// Dot 
'iexc',	'#161', "iexcl",	241,	// inverted exclamation
'cent', '#162', "cent",		242,	// cent
'curr', '#164', "curren",	244,	// curren
'yen\0','#165', "yen",		245,	// yen 
'brvb', '#166', "brvbar",	246,	// brvbar, becomes standard bar '|'
'poun', '#163', "pound,",	243,	// pound, bcomes 'sterling'
'sect', '#167', "sect",		247,	// section
'uml\0','#168', "uml",		250,	// uml
'copy', '#169', "copy",		251,	// copyright
'ordf', '#170', "ordf",		252,	// ordfeminine
'laqu', '#171', "laquo",	253,	// Left angle quote, guillemot left 
'not\0','#172', "not",		254,	// Not sign  
'shy\0','#173', "shy",		55,		// Soft hyphen  - make it a normal hyphen
'reg\0','#174', "reg",		256,	// Registered trademark  
'macr', '#175', "macr",		257,	// Macron accent 
'deg\0','#176', "deg",		260,	// Degree sign  
'plus', '#177', "plusmn",	261,	// Plus or minus
'sup2', '#178', "sup2",		262,	// Superscript two 
'sup3', '#179', "sup3",		263,	// Superscript three
'acut', '#180', "acute",	264,	// acute
'micr', '#181', "micro",	265,	// Micro sign 
'para', '#182', "para",		266,	// Paragraph sign 
'midd', '#183', "middot",	267,	// Middle dot, using 'period centered'
'cedi', '#184', "cedil",	270,	// cedilla
'sup1', '#185', "sup1",		271,	// Superscript one
'ordm', '#186', "ordm",		272,	// Masculine ordinal 
'raqu', '#187', "raquo",	273,	// Right angle quote, guillemot right 
'fr14', '#188', "frac14",	274,	// frac14
'fr12', '#189', "frac12",	275,	// frac12
'fr34', '#190', "frac34",	276,	// frac34
'ique', '#191', "iquest",	277,	// Inverted question mark
'Agra', '#192', "Agrave",	300,	// Agrave
'Aacu', '#193', "Aacute",	301,	// Aacute
'Acir', '#194', "Acirc",	302,	// Acirc
'Atil', '#195', "Atilde",	303,	// Atilde
'Auml', '#196', "Auml",		304,	// Auml
'Arin', '#197', "Aring",	305,	// Aring
'AEli', '#198', "AElig",	306,	// AElig
'Cced', '#199', "Ccedil",	307,	// Ccedil
'Egra', '#200', "Egrave",	310,	// Egrave
'Eacu', '#201', "Eacute",	311,	// Eacute
'Ecir', '#202', "Ecirc",	312,	// Ecirc
'Euml', '#203', "Euml",		313,	// Euml
'Igra', '#204', "Igrave",	314,	// Igrave
'Iacu', '#205', "Iacute",	315,	// Iacute
'Icir', '#206', "Icirc",	316,	// Icirc
'Iuml', '#207', "Iuml",		317,	// Iuml
//'ETH\0','#208': return "Fi"; // ETH - special case ETH = 'Fi'
'Ntil', '#209', "Ntilde",	321,	// Ntilde
'Ogra', '#210', "Ograve",	322,	// Ograve
'Oacu', '#211', "Oacute",	323,	// Oacute
'Ocir', '#212', "Ocirc",	324,	// Ocirc
'Otil', '#213', "Otilde",	325,	// Otilde
'Ouml', '#214', "Ouml",		326,	// Ouml
'time', '#215', "times",	327,	// Multiply sign 
'Osla', '#216', "Oslash",	330,	// Oslash
'Ugra', '#217', "Ugrave",	331,	// Ugrave
'Uacu', '#218', "Uacute",	332,	// Uacute
'Ucir', '#219', "Ucirc",	333,	// Ucirc
'Uuml', '#220', "Uuml",		334,	// Uuml
'Yacu', '#221', "Yacute",	335,	// Yacute, becomes 'standard Y'
'THOR', '#222', "THORN,",	336,	// THORN, becomes 'standard P'
'szli', '#223', "szlig",	337,	// szlig
'agra', '#224', "agrave",	340,	// agrave
'aacu', '#225', "aacute",	341,	// aacute
'acir', '#226', "acirc",	342,	// acirc
'atil', '#227', "atilde",	343,	// atilde
'auml', '#228', "auml",		344,	// auml
'arin', '#229', "aring",	345,	// aring
'aeli', '#230', "aelig",	346,	// aelig
'cced', '#231', "ccedil",	347,	// ccedil
'egra', '#232', "egrave",	350,	// egrave
'eacu', '#233', "eacute",	351,	// eacute 
'ecir', '#234', "ecirc",	352,	// ecirc 
'euml', '#235', "euml",		353,	// euml
'eth\0','#240', "eth",		360,	// eth, Icelandic
'igra', '#236', "igrave",	354,	// igrave
'iacu', '#237', "iacute",	355,	// iacute
'icir', '#238', "icirc",	356,	// icirc
'iuml', '#239', "iuml",		357,	// iuml
'ntil', '#241', "ntilde",	361,	// ntilde 
'ogra', '#242', "ograve",	362,	// ograve
'oacu', '#243', "oacute",	363,	// oacute 
'ocir', '#244', "ocirc",	364,	// ocirc  
'otil', '#245', "otilde",	365,	// otilde  
'ouml', '#246', "ouml",		366,	// ouml
'osla', '#248', "oslash",	370,	// oslash
'ugra', '#249', "ugrave",	371,	// ugrave  
'uacu', '#250', "uacute",	372,	// uacute 
'ucir', '#251', "ucirc",	373,	// ucirc  
'uuml', '#252', "uuml",		374,	// uuml
'yacu', '#253', "yacute",	375,	// yacute 
'thor', '#254', "thorn",	376,	// thorn
'yuml', '#255', "yuml",		377,	// yuml 
-1,		'#848', "#8482",	222,	// trademark
0,		0,		"",			0		// Denotes end of table :o))))
};

typedef std::map<int,int> ArrayIndex;
static ArrayIndex htmlShortStrNameIntIndex;
static ArrayIndex htmlHashNumIntIndex;

void PDFCharScaleFont::InitHTMLTokenToPDFCharMaps()
{
	int i = 0;
	if ( htmlShortStrNameIntIndex.size() != 0 )
		return;

	while ( HTMLTokenToPDFCharMapArray[i].htmlIndexStrNameIndex )
	{
		ArrayIndex::value_type indexName(HTMLTokenToPDFCharMapArray[i].htmlIndexStrNameIndex, i);
		htmlShortStrNameIntIndex.insert( htmlShortStrNameIntIndex.end(), indexName );
		ArrayIndex::value_type indexHash(HTMLTokenToPDFCharMapArray[i].htmlIndexStrNameIndex, i);
		htmlHashNumIntIndex.insert( htmlHashNumIntIndex.end(), indexHash );
		i++;
	}
}

//
// PDFCharScaleFont
//
PDFCharScaleFont::PDFCharScaleFont( std::string fontNameP, int fontSizeP, int fontSpacingP )
	: PDFFont( fontNameP, fontSizeP, fontSpacingP ), splitChars( " /\\-" )
{
	InitCharWidthOnePointArray();
	InitHTMLTokenToPDFCharMaps();
}

/*PDFCharScaleFont::~PDFCharScaleFont()
{
}*/

double PDFCharScaleFont::GetCharWidth( unsigned char c, int specificFontSize /*= 0*/, int style /*= PDF_NORMAL*/ )
{
	/*if ( specificFontSize == 0 || specificFontSize == DefaultFontSize() )
		return currSizeCharWidths[c];
	else*/
	switch( style )
	{
	case PDF_NORMAL:
		return charOnePointSizeNormalArray[c]*specificFontSize;
	case PDF_BOLD:
		return charOnePointSizeBoldArray[c]*specificFontSize;
	case PDF_ITALIC:
		return charOnePointSizeItalicArray[c]*specificFontSize;
	case PDF_BOLDITALIC:
		return charOnePointSizeBoldItalicArray[c]*specificFontSize;
	default:
		CurrentStyle( PDF_NORMAL );
		return charOnePointSizeNormalArray[c]*specificFontSize;
	}
}

static std::string FORMAT_BOLD					= "<b>";
static std::string FORMAT_BOLD_CLOSE			= "</b>";
static std::string FORMAT_ITALIC				= "<i>";
static std::string FORMAT_ITALIC_CLOSE			= "</i>";
static std::string FORMAT_UNDERLINE				= "<u>";
static std::string FORMAT_UNDERLINE_CLOSE		= "</u>";
static std::string FORMAT_HEADING				= "<h";
static std::string FORMAT_HEADING_CLOSE			= "</h";
static std::string FORMAT_BOLD_UPPER			= "<B>";
static std::string FORMAT_BOLD_CLOSE_UPPER		= "</B>";
static std::string FORMAT_ITALIC_UPPER			= "<I>";
static std::string FORMAT_ITALIC_CLOSE_UPPER	= "</I>";
static std::string FORMAT_UNDERLINE_UPPER		= "<U>";
static std::string FORMAT_UNDERLINE_CLOSE_UPPER	= "</U>";
static std::string FORMAT_HEADING_UPPER			= "<H";
static std::string FORMAT_HEADING_CLOSE_UPPER	= "</H";

class FormatPair
{
public:
	FormatPair( std::string HTMLTokenP, short PDFStyleP )
		: HTMLToken( HTMLTokenP ), PDFStyle( PDFStyleP ){}
	std::string HTMLToken;
	short PDFStyle;
};

static std::vector<FormatPair> htmlTokens;

void InitialiseHTMLTokens()
{
	if ( htmlTokens.size() )
		return;
	// Add the lowercase
	FormatPair formatBold( FORMAT_BOLD, PDF_BOLD );
	htmlTokens.push_back( formatBold );
	FormatPair formatBoldClose( FORMAT_BOLD_CLOSE, ~PDF_BOLD ); // Problem here...
	htmlTokens.push_back( formatBoldClose );
	FormatPair formatItalic( FORMAT_ITALIC, PDF_ITALIC );
	htmlTokens.push_back( formatItalic );
	FormatPair formatItalicClose( FORMAT_ITALIC_CLOSE, ~PDF_ITALIC ); // Problem here...
	htmlTokens.push_back( formatItalicClose );
	FormatPair formatUnderline( FORMAT_UNDERLINE, PDF_CURRENT_STYLE );
	htmlTokens.push_back( formatUnderline );
	FormatPair formatUnderlineClose( FORMAT_UNDERLINE_CLOSE, PDF_CURRENT_STYLE );
	htmlTokens.push_back( formatUnderlineClose );

	// Add the uppercase tokens...
	FormatPair formatBoldUpper( FORMAT_BOLD_UPPER, PDF_BOLD );
	htmlTokens.push_back( formatBoldUpper );
	FormatPair formatBoldCloseUpper( FORMAT_BOLD_CLOSE_UPPER, ~PDF_BOLD ); // Problem here...
	htmlTokens.push_back( formatBoldCloseUpper );
	FormatPair formatItalicUpper( FORMAT_ITALIC_UPPER, PDF_ITALIC );
	htmlTokens.push_back( formatItalicUpper );
	FormatPair formatItalicCloseUpper( FORMAT_ITALIC_CLOSE_UPPER, ~PDF_ITALIC ); // Problem here...
	htmlTokens.push_back( formatItalicCloseUpper );
	FormatPair formatUnderlineUpper( FORMAT_UNDERLINE_UPPER, PDF_CURRENT_STYLE );
	htmlTokens.push_back( formatUnderlineUpper );
	FormatPair formatUnderlineCloseUpper( FORMAT_UNDERLINE_CLOSE_UPPER, PDF_CURRENT_STYLE );
	htmlTokens.push_back( formatUnderlineCloseUpper );

	//htmlTokens.push_back( FORMAT_BOLD );
	//htmlTokens.push_back( FORMAT_ITALIC );
	//htmlTokens.push_back( FORMAT_UNDERLINE );
	//htmlTokens.push_back( FORMAT_HEADING );
	//htmlTokens.push_back( FORMAT_HEADING_CLOSE );
}



static std::string FORMAT_LINE_BREAK			= "<br>";
static std::string FORMAT_PARAGRAPH_BREAK		= "<p>";
static std::string FORMAT_PARAGRAPH_BREAK_CLOSE	= "</p>";
static std::string FORMAT_NEWLINE				= "\n";
static std::string FORMAT_RETURN				= "\r";
static std::string FORMAT_LINE_BREAK_UPPER		= "<BR>";
static std::string FORMAT_PARAGRAPH_BREAK_UPPER	= "<P>";
static std::string FORMAT_PARAGRAPH_BREAK_CLOSE_UPPER = "</P>";

static std::list<std::string> lineBreakTokens;

void InitialiseLineBreakTokens()
{
	if ( lineBreakTokens.size() )
		return;
	lineBreakTokens.push_back( FORMAT_PARAGRAPH_BREAK );
	lineBreakTokens.push_back( FORMAT_PARAGRAPH_BREAK_CLOSE );
	lineBreakTokens.push_back( FORMAT_LINE_BREAK );
	lineBreakTokens.push_back( FORMAT_PARAGRAPH_BREAK_UPPER );
	lineBreakTokens.push_back( FORMAT_PARAGRAPH_BREAK_CLOSE_UPPER );
	lineBreakTokens.push_back( FORMAT_LINE_BREAK_UPPER );
	lineBreakTokens.push_back( FORMAT_NEWLINE );
	lineBreakTokens.push_back( FORMAT_RETURN );
}

short GetLengthOfHTMLToken( short PDFStyle )
{
	switch( PDFStyle )
	{
	case PDF_BOLD:
		return FORMAT_BOLD.length();
		break;
	case ~PDF_BOLD:
		return FORMAT_BOLD_CLOSE.length();
		break;
	case PDF_ITALIC:
		return FORMAT_ITALIC.length();
		break;
	case ~PDF_ITALIC:
		return FORMAT_ITALIC_CLOSE.length();
		break;
	case PDF_UNDERLINE:
		return FORMAT_UNDERLINE.length();
		break;
	case ~PDF_UNDERLINE:
		return FORMAT_UNDERLINE_CLOSE.length();
		break;
	};
	return 0;
}

void PDFCharScaleFont::SplitHTMLStringAtParagraphs( std::string& splitAtParagraphs, short size, short style, PDFMultiLineCellTextStrList& cellStrList )
{
	std::list<std::string> linesOfText;
	std::string aLineOfText;
	long pos;

	// First off... split at paragraph starts
/*	while ( (pos = splitAtParagraphs.find( FORMAT_PARAGRAPH_BREAK ) != std::string::npos ) )
	{
		aLineOfText = splitAtParagraphs.substr( 0, pos );
		splitAtParagraphs.erase( 0, pos+strlen(FORMAT_PARAGRAPH_BREAK) );
		linesOfText.push_back( aLineOfText );
	}*/

	linesOfText.push_back( splitAtParagraphs );

	if ( lineBreakTokens.size() )
		InitialiseLineBreakTokens();

	// Next... split at line breaks, "\n" & "\r" chars... etc...
	std::list<std::string>::iterator tokIter = lineBreakTokens.begin();
	for( ; tokIter != lineBreakTokens.end(); ++tokIter )
	{
		std::string& token = (*tokIter);
		std::list<std::string>::iterator iter = linesOfText.begin();
		for( ; iter != linesOfText.end(); ++iter )
		{
			std::string& para = (*iter);
			pos = para.find(token); 
			for( ; pos != std::string::npos; pos = para.find(token) )
			{
				aLineOfText = para.substr( 0, pos );
				para.erase( 0, pos+token.length() );
				linesOfText.insert( iter, aLineOfText );
			}
		}
	}

	// Copy the list of string into a PDF string structure... with the length
//	PDFMultiLineCellTextStrList cellStrList;
	short len;
	std::list<std::string>::iterator iter = linesOfText.begin();
	for( ;iter != linesOfText.end(); ++iter )
	{
		len = GetStringLenWithSpacing( (*iter).c_str(), size, style );
		PDFMultiLineCellTextStr temp( (*iter), len );
		cellStrList.push_back( temp );
	}

}

bool PDFCharScaleFont::ValidateHTMLString( std::string& str )
{
	if ( !str.length() )
		return false;

	if ( !htmlTokens.size() ) 
		InitialiseHTMLTokens();

	short boldCount = 0, italicCount = 0;

	// First off... try and find a HTML token bracket...
	short thisTokenPos = str.find( "<" );
	if ( thisTokenPos == std::string::npos )
		false; // There are no HTML tokens, so pretend the string is not HTML compliant,
			   // so that it will not be treated as a HTML string in

	/*{
		std::vector<FormatPair>::iterator tokIter = htmlTokens.begin();
		for( ; tokIter != htmlTokens.end(); ++tokIter )
		{
			std::string& token = (*tokIter).HTMLToken;
			thisTokenPos = HTMLFormattedStr.find(token, lastPos);
			if ( thisTokenPos != std::string::npos )
			{
				if ( thisTokenPos < nearestTokenPos )
				{
					nearestTokenPos = thisTokenPos;
					nearestTokenStyle = (*tokIter).PDFStyle;
				}
			}
		}
	}*/
	return true;
}


short GetNextStyleAndTokenPosition( short& lastPos, std::string& HTMLFormattedStr )
{
	//static std::vector<std::string> htmlTokens;
	// If the HTML tokens have not been filled, then fill them once only
	if ( !htmlTokens.size() ) 
		InitialiseHTMLTokens();

	short thisTokenPos = std::string::npos;
	short nearestTokenPos = HTMLFormattedStr.length() + 1;
	short nearestTokenStyle = PDF_CURRENT_STYLE;
	// Search through the string for the nearest HTML token
/*	std::list<std::string>::iterator tokIter = htmlTokens.begin();
	for( ; tokIter != htmlTokens.end(); ++tokIter )
	{
		std::string& token = (*tokIter);
		thisTokenPos = HTMLFormattedStr.find(token, lastPos+1);
		if ( pos != std::string::npos )
		{
			if ( thisTokenPos < nearestTokenPos )
				nearestTokenPos = thisTokenPos;
		}
	}
	lastPos = nearestTokenPos;
	return*/

	// First off... try and find a HTML token bracket...
	thisTokenPos = HTMLFormattedStr.find( "<", lastPos );
	lastPos = thisTokenPos;
	if ( thisTokenPos != std::string::npos )
	{
		std::vector<FormatPair>::iterator tokIter = htmlTokens.begin();
		for( ; tokIter != htmlTokens.end(); ++tokIter )
		{
			std::string& token = (*tokIter).HTMLToken;
			thisTokenPos = HTMLFormattedStr.find(token, lastPos);
			if ( thisTokenPos != std::string::npos )
			{
				if ( thisTokenPos < nearestTokenPos )
				{
					nearestTokenPos = thisTokenPos;
					nearestTokenStyle = (*tokIter).PDFStyle;
				}
			}
		}
	}
	lastPos = nearestTokenPos;
	return nearestTokenStyle;
}

double PDFCharScaleFont::GetStringLen( std::string str, int specificFontSize /*= 0*/, int style /*= PDF_NORMAL*/ )
{
	double len = 0;
	int badChars = 0;
	unsigned char c;
	bool interpretHTMLTokens = false;
	short nextTokenPos = 0;
	short lastPDFToken = PDF_NORMAL;

	short strlen = str.length();
	if ( !strlen )
		return 0;

	// Check to see if we must change formatting during the string
	if ( style == PDF_HTML_FORMATTING )
	{
		interpretHTMLTokens = true;
		lastPDFToken = GetNextStyleAndTokenPosition( nextTokenPos, str );
		style = PDF_NORMAL;
	}
	else if ( style == PDF_CURRENT_STYLE )
		style = CurrentStyle();

	// Go through the string and add the width of the character (based on the style)
	for( short i=0; i < strlen ; i++ )
	{
		// Check if we are in HTML mode... i.e. the style can change in the string itself... like HTML...
		if ( interpretHTMLTokens )
		{
			if ( i == nextTokenPos ) // We have reached the next Token... so change style
			{
				i += GetLengthOfHTMLToken( lastPDFToken ); // Skip over the token...
				nextTokenPos = i;
				style = style & lastPDFToken;
				lastPDFToken = GetNextStyleAndTokenPosition( nextTokenPos, str );
			}
		}

		c = str[i];
		if ( GetCharWidth( c, specificFontSize, style ) == 0 )
		//if ( currSizeCharWidths[c] == 0 )
			badChars++;
		if (c == '\\')
		{
			unsigned char c2 = str[i+1];
			if( c2 == '\\' || c2 == '(' || c2 == ')' || c2 == 0 )
			{
				len += GetCharWidth( c, specificFontSize, style );
				continue;
			}
			else
			{
				badChars++;
				continue;
			}
		}
		len += GetCharWidth( c, specificFontSize, style );
	}
	return len;
}

int PDFCharScaleFont::GetStringLenWithSpacing( std::string str, int specificFontSize, int style /*= PDF_NORMAL*/ )
{
	return GetStringLen( str, specificFontSize, style ) + FontSpacing( specificFontSize ) + 1; 
}

bool PDFCharScaleFont::AnySplitChars( const std::string& str )
{
	size_t pos = str.find_first_of( splitChars );
	if ( pos == std::string::npos )
		return false;
	return true;
}
std::string PDFCharScaleFont::SetStringLen( const std::string& str, double lenWanted, int size /*= 0*/, short truncateTextLeftOrRight /*= PDF_LEFT_ELLIPSE*/, int style /*= PDF_NORMAL*/ )
{
	if ( style == PDF_CURRENT_STYLE )
		style = CurrentStyle();

	if (size == 0)
		size = DefaultFontSize();

	const char ellipse[]="...";

	if ( GetStringLen( str, size, style ) < lenWanted )
		return str;

	double len = 0;

	// Make sure we don't use the combination of ellipse and breaking at a space... it does not make sense
	if ( truncateTextLeftOrRight & PDF_ELLIPSE && truncateTextLeftOrRight & PDF_BREAK_AT_SPACE )
	{
		// breaking at a space takes precedence... so turn off the ellipse
		truncateTextLeftOrRight = truncateTextLeftOrRight & !PDF_ELLIPSE;
	}

	if ( truncateTextLeftOrRight & PDF_ELLIPSE )
	{
		lenWanted -= GetStringLen( ellipse, size, style );
		if ( len > lenWanted )
			return PDF_FONT_BLANK;
	}

	unsigned char c;
	std::string pdfStr;
	pdfStr.reserve(str.length());
	int i;

	if ( truncateTextLeftOrRight & PDF_RIGHT )
	{
		for( i=0; i < str.length(); i++ )
		{
			c = str[i];

			if ( len + GetCharWidth(c, size, style ) > lenWanted )
				break;
			
			len += GetCharWidth(c, size, style );
			pdfStr += c;
		}
		if ( truncateTextLeftOrRight & PDF_BREAK_AT_SPACE )
		{
			size_t pos=pdfStr.find_last_of( splitChars );

			// NOTE: if a split character isn't found then we just go with the string as it is
			
			// if a split character was found
			if( pos!=std::string::npos )
			{
				// if plit char was a space
				if( pdfStr[pos]==' ' )
				{
					// determine if we're trying to split on leading spaces
					int spacePos(pos);
					while( --spacePos>=0 && pdfStr[spacePos]==' ' )
					{
						;
					}

					// if we're not trying to split on leading spaces
					if( spacePos>=0 )
					{
						pdfStr.resize( pos ); // Don't want a space starting in the next line/row
					}
				}
				else
				{
					pdfStr.resize( pos+1 );
				}
			}
		}

		if ( truncateTextLeftOrRight & PDF_ELLIPSE )
			pdfStr += ellipse;
	}
	else // PDF_LEFT
	{
		// Start from the end of the string and copy chars until we've hit our string length limit,
		// Copy each char that fits to another string which will be in reverse order	
		std::string reverseStr;
		reverseStr.reserve(str.length());
		for( i=str.length()-1; i >= 0; i-- )
		{
			c = str[i];
			if ( len + GetCharWidth( c, size, style ) > lenWanted )
				break;
			
			len += GetCharWidth( c, size, style );
			reverseStr += c;
		}

		// Construct the new string
		if ( truncateTextLeftOrRight & PDF_ELLIPSE )
			pdfStr += ellipse;
		for( i=reverseStr.length()-1; i >= 0; i-- )
		{
			pdfStr += reverseStr[i];
		}
	}
	return pdfStr;
}

std::string PDFCharScaleFont::MakePDFString( std::string str )
{
	int badChars = 0;
	unsigned char c;
	std::string pdfStr;
	pdfStr.reserve(str.length()*1.1);
	for( int i=0; i < str.length(); i++ )
	{
		c = str[i];

		if (c == '\\')
		{
			unsigned char c2 = str[i+1];
			if ( c2 < '0' || c2 > '9' ) // If the next character is a not a number then we want to "display" a slash, so we add another slash
				pdfStr += c; // Add another slash
		}
		if ( c == '(' || c == ')' )
			pdfStr += '\\';  // We haven't got a slash in front already, so add one

		pdfStr += c; // Add the current character
	}
	return pdfStr;
}

void PDFCharScaleFont::TranslateHTMLSpecialTokens( std::string &str )
{
	if ( str.find( "&" ) == std::string::npos )
		return;

	unsigned char c;
	std::string replaceHTMLTokensStr;
	replaceHTMLTokensStr = PDF_FONT_BLANK;
	replaceHTMLTokensStr.reserve(str.length());
	for( int i=0; i < str.length(); i++ )
	{
		c = str[i];
		if (c == '&')
		{
			// Check for the special HTML characters which are shown as tokens,
			// We need to translate these back into their ascii character code 
			//std::string charCode = ConvertHTMLStrCharCodeToMacRomanEncoding( &str[i+1] );
			std::string charCode = ConvertHTMLStrCharCodeToPDFDocEncoding( &str[i+1] );
			if ( charCode.length() > 0 )
			{
				while (str[i] != ';')
					i++;
				replaceHTMLTokensStr += charCode;
				continue;
			}
		}
		replaceHTMLTokensStr += c; // Add the current character
	}
	str = replaceHTMLTokensStr;
	return;
}

const char* PDFCharScaleFont::ConvertPDFEncoding( std::string str )
{
/*	if ( !str.find( "\\" ) == std::string::npos )
		return str.c_str(); // return as there are no special encoding chars

	unsigned char c;
	static std::string newEncodingStr;
	newEncodingStr = str;
	for( int i=0; i < newEncodingStr.length(); i++ )
	{
		c = str[i];
		if (c == '\\')
		{
			ConvertPDFMacRomanEncodingToPDFDocEncodingCharCode( &newEncodingStr[i+1] );
			i+=3;
		}
	}
	return newEncodingStr.c_str();*/
	return str.c_str(); // return as there are no special encoding chars
}

void PDFCharScaleFont::ConvertPDFMacRomanEncodingToPDFDocEncodingCharCode( char *str )
{
/*	int code = 0;
	std::string charCodeStr;
	int count = 0;
	for( int i = 0; i<3 && str[i] != 0; i++ )
	{
		charCodeStr += str[i];
	}
	code = atoi( charCodeStr.c_str() );
	switch( code )
	{
		// octal number in case statement is converted to octal number in str 
		case 256: str[0] = '3'; str[1] = '0'; str[2] = '6'; return; // 'AE' stays 'AE'
		case 347: str[0] = '3'; str[1] = '0'; str[2] = '1'; return; // 'Aacute' stays 'Aacute'
		case 345: str[0] = '3'; str[1] = '0'; str[2] = '2'; return; // 'Acircumflex' stays 'Acircumflex'
		case 200: str[0] = '3'; str[1] = '0'; str[2] = '4'; return; // 'Adieresis' stays 'Adieresis' - also known as Umlout A
		case 313: str[0] = '3'; str[1] = '0'; str[2] = '0'; return; // 'Agrave' stays 'Agrave'
		case 201: str[0] = '3'; str[1] = '0'; str[2] = '5'; return; // 'Aring' stays 'Aring'
		case 314: str[0] = '3'; str[1] = '0'; str[2] = '3'; return; // 'Atilde' stays 'Atilde'
		case 202: str[0] = '3'; str[1] = '0'; str[2] = '7'; return; // 'Ccedilla' stays 'Ccedilla'
		case 203: str[0] = '3'; str[1] = '1'; str[2] = '1'; return; // 'Eacute' stays 'xxx'
		case 346: str[0] = '3'; str[1] = '1'; str[2] = '2'; return; // 'Ecircumflex' stays 'xxx'
		case 350: str[0] = '3'; str[1] = '1'; str[2] = '3'; return; // 'Edieresis' stays 'xxx'
		case 351: str[0] = '3'; str[1] = '1'; str[2] = '0'; return; // 'Egrave' stays 'Egrave'
		case 352: str[0] = '3'; str[1] = '1'; str[2] = '5'; return; // 'Iacute' stays 'xxx'
		case 353: str[0] = '3'; str[1] = '1'; str[2] = '6'; return; // 'Icircumflex' stays 'xxx'
		case 354: str[0] = '3'; str[1] = '1'; str[2] = '7'; return; // 'Idieresis' stays 'xxx'
		case 355: str[0] = '3'; str[1] = '1'; str[2] = '4'; return; // 'Igrave' stays 'xxx'
		case 204: str[0] = '3'; str[1] = '2'; str[2] = '1'; return; // 'Ntilde' stays 'xxx'
		case 316: str[0] = '2'; str[1] = '2'; str[2] = '6'; return; // 'OE' stays 'xxx'
		case 356: str[0] = '3'; str[1] = '2'; str[2] = '3'; return; // 'Oacute' stays 'xxx'
		case 357: str[0] = '3'; str[1] = '2'; str[2] = '4'; return; // 'Ocircumflex' stays 'xxx'
		case 205: str[0] = '3'; str[1] = '2'; str[2] = '6'; return; // 'Odieresis' stays 'xxx'
		case 361: str[0] = '3'; str[1] = '2'; str[2] = '2'; return; // 'Ograve' stays 'xxx'
		case 257: str[0] = '3'; str[1] = '3'; str[2] = '0'; return; // 'Oslash' stays 'Oslash'
		case 315: str[0] = '3'; str[1] = '2'; str[2] = '5'; return; // 'Otilde' stays 'xxx'
		case 362: str[0] = '3'; str[1] = '3'; str[2] = '2'; return; // 'Uacute' stays 'xxx'
		case 363: str[0] = '3'; str[1] = '3'; str[2] = '3'; return; // 'Ucircumflex' stays 'xxx'
		case 206: str[0] = '3'; str[1] = '3'; str[2] = '4'; return; // 'Udieresis' stays 'xxx'
		case 364: str[0] = '3'; str[1] = '3'; str[2] = '1'; return; // 'Ugrave' stays 'xxx'
		case 331: str[0] = '2'; str[1] = '3'; str[2] = '0'; return; // 'Ydieresis' stays 'xxx'
		case 207: str[0] = '3'; str[1] = '4'; str[2] = '1'; return; // 'aacute' stays 'xxx'
		case 211: str[0] = '3'; str[1] = '4'; str[2] = '2'; return; // 'acircumflex' stays 'xxx'
		case 253: str[0] = '2'; str[1] = '6'; str[2] = '4'; return; // 'acute' stays 'xxx'
		case 212: str[0] = '3'; str[1] = '4'; str[2] = '4'; return; // 'adieresis' stays 'xxx'
		case 276: str[0] = '3'; str[1] = '4'; str[2] = '6'; return; // 'ae' stays 'xxx'
		case 210: str[0] = '3'; str[1] = '4'; str[2] = '0'; return; // 'agrave' stays 'xxx'
		case 214: str[0] = '3'; str[1] = '4'; str[2] = '5'; return; // 'aring' stays 'xxx'
		case 213: str[0] = '3'; str[1] = '4'; str[2] = '3'; return; // 'atilde' stays 'xxx'
		case 371: str[0] = '0'; str[1] = '3'; str[2] = '0'; return; // 'breve' stays 'xxx'
		case 245: str[0] = '2'; str[1] = '0'; str[2] = '0'; return; // 'bullet' stays 'xxx'
		case 377: str[0] = '0'; str[1] = '3'; str[2] = '1'; return; // 'caron' stays 'xxx'
		case 215: str[0] = '3'; str[1] = '4'; str[2] = '7'; return; // 'ccedilla' stays 'xxx'
		case 374: str[0] = '2'; str[1] = '7'; str[2] = '0'; return; // 'cedilla' stays 'xxx'
		case 366: str[0] = '0'; str[1] = '3'; str[2] = '2'; return; // 'circumflex' stays 'xxx'
		case 333: str[0] = '2'; str[1] = '4'; str[2] = '4'; return; // 'currency' stays 'xxx'
		case 240: str[0] = '2'; str[1] = '0'; str[2] = '1'; return; // 'dagger' stays 'xxx'
		case 340: str[0] = '2'; str[1] = '0'; str[2] = '2'; return; // 'daggerdbl' stays 'xxx'
		case 241: str[0] = '2'; str[1] = '6'; str[2] = '0'; return; // 'degree' stays 'xxx'
		case 254: str[0] = '2'; str[1] = '5'; str[2] = '0'; return; // 'dieresis' stays 'xxx'
		case 326: str[0] = '3'; str[1] = '6'; str[2] = '7'; return; // 'divide' stays 'xxx'
		case 372: str[0] = '0'; str[1] = '3'; str[2] = '3'; return; // 'dotaccent' stays 'xxx'
		case 365: str[0] = '2'; str[1] = '3'; str[2] = '2'; return; // 'dotlessi' stays 'xxx'
		case 216: str[0] = '3'; str[1] = '5'; str[2] = '1'; return; // 'eacute' stays 'xxx'
		case 220: str[0] = '3'; str[1] = '5'; str[2] = '2'; return; // 'ecircumflex' stays 'xxx'
		case 221: str[0] = '3'; str[1] = '5'; str[2] = '3'; return; // 'edieresis' stays 'xxx'
		case 217: str[0] = '3'; str[1] = '5'; str[2] = '0'; return; // 'egrave' stays 'xxx'
		case 311: str[0] = '2'; str[1] = '0'; str[2] = '3'; return; // 'ellipsis' stays 'xxx'
		case 321: str[0] = '2'; str[1] = '0'; str[2] = '4'; return; // 'emdash' stays 'xxx'
		case 320: str[0] = '2'; str[1] = '0'; str[2] = '5'; return; // 'endash' stays 'xxx'
		case 301: str[0] = '2'; str[1] = '4'; str[2] = '1'; return; // 'exclamdown' stays 'xxx'
		case 336: str[0] = '2'; str[1] = '2'; str[2] = '3'; return; // 'fi' stays 'xxx'
		case 337: str[0] = '2'; str[1] = '3'; str[2] = '4'; return; // 'fl' stays 'xxx'
		case 304: str[0] = '2'; str[1] = '0'; str[2] = '6'; return; // 'florin' stays 'xxx'
		case 332: str[0] = '2'; str[1] = '0'; str[2] = '7'; return; // 'fraction' stays 'xxx'
		case 247: str[0] = '3'; str[1] = '3'; str[2] = '7'; return; // 'germandbls' stays 'xxx'
		case 307: str[0] = '2'; str[1] = '5'; str[2] = '3'; return; // 'guillemotleft' stays 'xxx'
		case 310: str[0] = '2'; str[1] = '7'; str[2] = '3'; return; // 'guillemotright' stays 'xxx'
		case 334: str[0] = '2'; str[1] = '1'; str[2] = '0'; return; // 'guilsinglleft' stays 'xxx'
		case 335: str[0] = '2'; str[1] = '1'; str[2] = '1'; return; // 'guilsinglright' stays 'xxx'
		case 375: str[0] = '0'; str[1] = '3'; str[2] = '4'; return; // 'hungarumlaut' stays 'xxx'
		case 222: str[0] = '3'; str[1] = '5'; str[2] = '5'; return; // 'iacute' stays 'xxx'
		case 224: str[0] = '3'; str[1] = '5'; str[2] = '6'; return; // 'icircumflex' stays 'xxx'
		case 225: str[0] = '3'; str[1] = '5'; str[2] = '7'; return; // 'idieresis' stays 'xxx'
		case 223: str[0] = '3'; str[1] = '5'; str[2] = '4'; return; // 'igrave' stays 'xxx'
		case 302: str[0] = '2'; str[1] = '5'; str[2] = '4'; return; // 'logicalnot' stays 'xxx'
		case 370: str[0] = '2'; str[1] = '5'; str[2] = '7'; return; // 'macron' stays 'xxx'
		case 226: str[0] = '3'; str[1] = '6'; str[2] = '1'; return; // 'ntilde' stays 'xxx'
		case 227: str[0] = '3'; str[1] = '6'; str[2] = '3'; return; // 'oacute' stays 'xxx'
		case 231: str[0] = '3'; str[1] = '6'; str[2] = '4'; return; // 'ocircumflex' stays 'xxx'
		case 232: str[0] = '3'; str[1] = '6'; str[2] = '6'; return; // 'odieresis' stays 'xxx'
		case 317: str[0] = '2'; str[1] = '3'; str[2] = '4'; return; // 'oe' stays 'xxx'
		case 376: str[0] = '0'; str[1] = '3'; str[2] = '5'; return; // 'ogonek' stays 'xxx'
		case 230: str[0] = '3'; str[1] = '6'; str[2] = '2'; return; // 'ograve' stays 'xxx'
		case 273: str[0] = '2'; str[1] = '7'; str[2] = '3'; return; // 'ordfeminine' stays 'xxx'
		case 274: str[0] = '2'; str[1] = '7'; str[2] = '2'; return; // 'ordmasculine' stays 'xxx'
		case 277: str[0] = '3'; str[1] = '7'; str[2] = '0'; return; // 'oslash' stays 'oslash'
		case 233: str[0] = '3'; str[1] = '6'; str[2] = '5'; return; // 'otilde' stays 'xxx'
		case 246: str[0] = '2'; str[1] = '6'; str[2] = '6'; return; // 'paragraph' stays 'xxx'
		case 341: str[0] = '2'; str[1] = '6'; str[2] = '7'; return; // 'periodcentered' stays 'xxx'
		case 344: str[0] = '2'; str[1] = '1'; str[2] = '3'; return; // 'perthousand' stays 'xxx'
		case 300: str[0] = '2'; str[1] = '7'; str[2] = '7'; return; // 'questiondown' stays 'xxx'
		case 343: str[0] = '2'; str[1] = '1'; str[2] = '4'; return; // 'quotedblbase' stays 'xxx'
		case 322: str[0] = '2'; str[1] = '1'; str[2] = '5'; return; // 'quotedblleft' stays 'xxx'
		case 323: str[0] = '2'; str[1] = '1'; str[2] = '6'; return; // 'quotedblright' stays 'xxx'
		case 324: str[0] = '2'; str[1] = '1'; str[2] = '7'; return; // 'quoteleft' stays 'xxx'
		case 325: str[0] = '2'; str[1] = '2'; str[2] = '0'; return; // 'quoteright' stays 'xxx'
		case 342: str[0] = '2'; str[1] = '2'; str[2] = '1'; return; // 'quotesinglbase' stays 'xxx'
		case 250: str[0] = '2'; str[1] = '5'; str[2] = '6'; return; // 'registered' stays 'xxx'
		case 373: str[0] = '0'; str[1] = '3'; str[2] = '6'; return; // 'ring' stays 'xxx'
		case 244: str[0] = '2'; str[1] = '4'; str[2] = '7'; return; // 'section' stays 'xxx'
		case 367: str[0] = '0'; str[1] = '3'; str[2] = '7'; return; // 'tilde' stays 'xxx'
		case 252: str[0] = '2'; str[1] = '2'; str[2] = '2'; return; // 'trademark' stays 'xxx'
		case 234: str[0] = '3'; str[1] = '7'; str[2] = '2'; return; // 'uacute' stays 'xxx'
		case 236: str[0] = '3'; str[1] = '7'; str[2] = '3'; return; // 'ucircumflex' stays 'xxx'
		case 237: str[0] = '3'; str[1] = '7'; str[2] = '4'; return; // 'udieresis' stays 'xxx'
		case 235: str[0] = '3'; str[1] = '7'; str[2] = '1'; return; // 'ugrave' stays 'xxx'
		case 330: str[0] = '3'; str[1] = '7'; str[2] = '7'; return; // 'ydieresis' stays 'xxx'
		case 264: str[0] = '2'; str[1] = '3'; str[2] = '6'; return; // 'yen' stays 'xxx'
	}*/
}

inline int PDFCharScaleFont::CompareSpecificHTMLCode( std::string charCodeStr, std::string cmpCharCodeStr, int charCodeValue )
{
	if ( charCodeStr[0] == '#' || charCodeStr == cmpCharCodeStr )
		return charCodeValue;
	return 0;
}


  
std::string PDFCharScaleFont::ConvertHTMLStrCharCodeToMacRomanEncoding( char *str )
{
/*	int code = 0;
	std::string charCodeStr = PDF_FONT_BLANK;
	charCodeStr.reserve(16);
	int count = 0;
	while( *str != 0 && count < 10 )
	{
		if (*str == ';')
		{
			// Found a possible HTML special character
			char charCodeIntStr[4];
			strncpy(charCodeIntStr, charCodeStr.c_str(), 4);
			int charCodeInt = 0;
			for(int i=0; i<3; i++)
			{
				charCodeInt = ((charCodeInt + (int)charCodeIntStr[i]) * 256);
			}
			charCodeInt += (int)charCodeIntStr[3];
			switch( charCodeInt )
			{
			case 'Dot' :			  code = CompareSpecificHTMLCode( charCodeStr, "Dot", 56 ); break;		// Dot 
			case 'iexc': case '#161': code = CompareSpecificHTMLCode( charCodeStr, "iexcl", 301 ); break;	// iexcl
			case 'cent': case '#162': code = CompareSpecificHTMLCode( charCodeStr, "cent", 242 ); break;	// cent
			case 'curr': case '#164': code = CompareSpecificHTMLCode( charCodeStr, "curren", 333 ); break;	// curren
			case 'yen\0':case '#165': code = CompareSpecificHTMLCode( charCodeStr, "yen", 264 ); break;		// yen 
			case 'brvb': case '#166': code = CompareSpecificHTMLCode( charCodeStr, "brvbar", 174 ); break;	// brvbar, becomes standard bar '|'
			case 'poun': case '#163': code = CompareSpecificHTMLCode( charCodeStr, "pound,", 243 ); break;	// pound, bcomes 'sterling'
			case 'sect': case '#167': code = CompareSpecificHTMLCode( charCodeStr, "sect", 244 ); break;	// sect
			case 'uml\0':case '#168': code = CompareSpecificHTMLCode( charCodeStr, "uml", 254 ); break;		// uml
			case 'copy': case '#169': code = CompareSpecificHTMLCode( charCodeStr, "copy", 251 ); break;	// copy
			case 'ordf': case '#170': code = CompareSpecificHTMLCode( charCodeStr, "ordf", 273 ); break;	// ordf
			case 'laqu': case '#171': code = CompareSpecificHTMLCode( charCodeStr, "laquo", 307 ); break;	// Left angle quote, guillemot left 
			case 'not\0':case '#172': code = CompareSpecificHTMLCode( charCodeStr, "not", 302 ); break;		// not 
			case 'shy\0':case '#173': code = CompareSpecificHTMLCode( charCodeStr, "shy", 55 ); break;		// shy 
			case 'reg\0':case '#174': code = CompareSpecificHTMLCode( charCodeStr, "reg", 250 ); break;		// reg 
			case 'macr': case '#175': code = CompareSpecificHTMLCode( charCodeStr, "macr", 370 ); break;	// macr
			case 'deg\0':case '#176': code = CompareSpecificHTMLCode( charCodeStr, "deg", 241 ); break;		// deg 
			case 'plus': case '#177': code = CompareSpecificHTMLCode( charCodeStr, "plusmn", 261 ); break;	// plusmn
//			case 'sup2': case '#178': code = CompareSpecificHTMLCode( charCodeStr, "sup2", NOCHAR ); break;	// sup2
//			case 'sup3': case '#179': code = CompareSpecificHTMLCode( charCodeStr, "sup3", NOCHAR ); break;	// sup3
			case 'acut': case '#180': code = CompareSpecificHTMLCode( charCodeStr, "acute", 253 ); break;	// acute
			case 'micr': case '#181': code = CompareSpecificHTMLCode( charCodeStr, "micro", 265 ); break;	// micro
			case 'para': case '#182': code = CompareSpecificHTMLCode( charCodeStr, "para", 246 ); break;	// para
			case 'midd': case '#183': code = CompareSpecificHTMLCode( charCodeStr, "middot", 341 ); break;	// middot, using 'period centered'
			case 'cedi': case '#184': code = CompareSpecificHTMLCode( charCodeStr, "cedil", 374 ); break;	// cedil
//			case 'sup1': case '#185': code = CompareSpecificHTMLCode( charCodeStr, "sup1", NOCHAR ); break;	// sup1
			case 'ordm': case '#186': code = CompareSpecificHTMLCode( charCodeStr, "ordm", 274 ); break;	// ordm
			case 'raqu': case '#187': code = CompareSpecificHTMLCode( charCodeStr, "raquo", 310 ); break;	// raquo
			case 'frac':
				if (charCodeStr == "frac14") return "1/4";		// frac14
				else if (charCodeStr == "frac12") return "1/2"; // frac14
				else if (charCodeStr == "frac34") return "3/4"; // frac14
				else return PDF_FONT_BLANK; // nothing
						 case '#188': return "1/4"; // frac14
						 case '#189': return "1/2"; // frac12
						 case '#190': return "3/4"; 	// frac34
			case 'ique': case '#191': code = CompareSpecificHTMLCode( charCodeStr, "iquest", 300 ); break;	// iquest
			case 'Agra': case '#192': code = CompareSpecificHTMLCode( charCodeStr, "Agrave", 313 ); break;	// Agrave
			case 'Aacu': case '#193': code = CompareSpecificHTMLCode( charCodeStr, "Aacute", 347 ); break;	// Aacute
			case 'Acir': case '#194': code = CompareSpecificHTMLCode( charCodeStr, "Acirc", 345 ); break;	// Acirc
			case 'Atil': case '#195': code = CompareSpecificHTMLCode( charCodeStr, "Atilde", 314 ); break;	// Atilde
			case 'Auml': case '#196': code = CompareSpecificHTMLCode( charCodeStr, "Auml", 200 ); break;	// Auml
			case 'Arin': case '#197': code = CompareSpecificHTMLCode( charCodeStr, "Aring", 201 ); break;	// Aring
			case 'AEli': case '#198': code = CompareSpecificHTMLCode( charCodeStr, "AElig", 256 ); break;	// AElig
			case 'Cced': case '#199': code = CompareSpecificHTMLCode( charCodeStr, "Ccedil", 202 ); break;	// Ccedil
			case 'Egra': case '#200': code = CompareSpecificHTMLCode( charCodeStr, "Egrave", 351 ); break;	// Egrave
			case 'Eacu': case '#201': code = CompareSpecificHTMLCode( charCodeStr, "Eacute", 203 ); break;	// Eacute
			case 'Ecir': case '#202': code = CompareSpecificHTMLCode( charCodeStr, "Ecirc", 346 ); break;	// Ecirc
			case 'Euml': case '#203': code = CompareSpecificHTMLCode( charCodeStr, "Euml", 350 ); break;	// Euml
			case 'Igra': case '#204': code = CompareSpecificHTMLCode( charCodeStr, "Igrave", 355 ); break;	// Igrave
			case 'Iacu': case '#205': code = CompareSpecificHTMLCode( charCodeStr, "Iacute", 352 ); break;	// Iacute
			case 'Icir': case '#206': code = CompareSpecificHTMLCode( charCodeStr, "Icirc", 353 ); break;	// Icirc
			case 'Iuml': case '#207': code = CompareSpecificHTMLCode( charCodeStr, "Iuml", 354 ); break;	// Iuml
			case 'ETH\0':case '#208': return "Fi"; // ETH - special case ETH = 'Fi'
			case 'Ntil': case '#209': code = CompareSpecificHTMLCode( charCodeStr, "Ntilde", 204 ); break;	// Ntilde
			case 'Ogra': case '#210': code = CompareSpecificHTMLCode( charCodeStr, "Ograve", 361 ); break;	// Ograve
			case 'Oacu': case '#211': code = CompareSpecificHTMLCode( charCodeStr, "Oacute", 356 ); break;	// Oacute
			case 'Ocir': case '#212': code = CompareSpecificHTMLCode( charCodeStr, "Ocirc", 357 ); break;	// Ocirc
			case 'Otil': case '#213': code = CompareSpecificHTMLCode( charCodeStr, "Otilde", 315 ); break;	// Otilde
			case 'time': case '#215': code = CompareSpecificHTMLCode( charCodeStr, "times", 52 ); break;	// times
			case 'Ouml': case '#214': code = CompareSpecificHTMLCode( charCodeStr, "Ouml", 205 ); break;	// Ouml
			case 'Osla': case '#216': code = CompareSpecificHTMLCode( charCodeStr, "Oslash", 257 ); break;	// Oslash
			case 'Ugra': case '#217': code = CompareSpecificHTMLCode( charCodeStr, "Ugrave", 364 ); break;	// Ugrave
			case 'Uacu': case '#218': code = CompareSpecificHTMLCode( charCodeStr, "Uacute", 362 ); break;	// Uacute
			case 'Ucir': case '#219': code = CompareSpecificHTMLCode( charCodeStr, "Ucirc", 363 ); break;	// Ucirc
			case 'Uuml': case '#220': code = CompareSpecificHTMLCode( charCodeStr, "Uuml", 206 ); break;	// Uuml
			case 'Yacu': case '#221': code = CompareSpecificHTMLCode( charCodeStr, "Yacute", 131 ); break;	// Yacute, becomes 'standard Y'
			case 'THOR': case '#222': code = CompareSpecificHTMLCode( charCodeStr, "THORN,", 120 ); break;	// THORN, becomes 'standard P'
			case 'szli': case '#223': code = CompareSpecificHTMLCode( charCodeStr, "szlig", 247 ); break;	// szlig
			case 'agra': case '#224': code = CompareSpecificHTMLCode( charCodeStr, "agrave", 210 ); break;	// agrave
			case 'aacu': case '#225': code = CompareSpecificHTMLCode( charCodeStr, "aacute", 207 ); break;	// aacute
			case 'acir': case '#226': code = CompareSpecificHTMLCode( charCodeStr, "acirc", 211 ); break;	// acirc
			case 'atil': case '#227': code = CompareSpecificHTMLCode( charCodeStr, "atilde", 213 ); break;	// atilde
			case 'auml': case '#228': code = CompareSpecificHTMLCode( charCodeStr, "auml", 212 ); break;	// auml
			case 'arin': case '#229': code = CompareSpecificHTMLCode( charCodeStr, "aring", 214 ); break;	// aring
			case 'aeli': case '#230': code = CompareSpecificHTMLCode( charCodeStr, "aelig", 276 ); break;	// aelig
			case 'cced': case '#231': code = CompareSpecificHTMLCode( charCodeStr, "ccedil", 215 ); break;	// ccedil
			case 'egra': case '#232': code = CompareSpecificHTMLCode( charCodeStr, "egrave", 217 ); break;	// egrave
			case 'eacu': case '#233': code = CompareSpecificHTMLCode( charCodeStr, "eacute", 216 ); break;	// eacute 
			case 'ecir': case '#234': code = CompareSpecificHTMLCode( charCodeStr, "ecirc", 220 ); break;	// ecirc 
			case 'euml': case '#235': code = CompareSpecificHTMLCode( charCodeStr, "euml", 221 ); break;	// euml
			case 'eth\0':case '#240': code = CompareSpecificHTMLCode( charCodeStr, "eth", 144 ); break;		// eth, problem!! becomes "d' as this is what it loks most like?
			case 'igra': case '#236': code = CompareSpecificHTMLCode( charCodeStr, "igrave", 223 ); break;	// igrave
			case 'iacu': case '#237': code = CompareSpecificHTMLCode( charCodeStr, "iacute", 222 ); break;	// iacute
			case 'icir': case '#238': code = CompareSpecificHTMLCode( charCodeStr, "icirc", 224 ); break;	// icirc
			case 'iuml': case '#239': code = CompareSpecificHTMLCode( charCodeStr, "iuml", 225 ); break;	// iuml
			case 'ntil': case '#241': code = CompareSpecificHTMLCode( charCodeStr, "ntilde", 226 ); break;	// ntilde 
			case 'ogra': case '#242': code = CompareSpecificHTMLCode( charCodeStr, "ograve", 230 ); break;	// ograve
			case 'oacu': case '#243': code = CompareSpecificHTMLCode( charCodeStr, "oacute", 227 ); break;	// oacute 
			case 'ocir': case '#244': code = CompareSpecificHTMLCode( charCodeStr, "ocirc", 231 ); break;	// ocirc  
			case 'otil': case '#245': code = CompareSpecificHTMLCode( charCodeStr, "otilde", 233 ); break;	// otilde  
			case 'ouml': case '#246': code = CompareSpecificHTMLCode( charCodeStr, "ouml", 232 ); break;	// ouml
			case 'osla': case '#248': code = CompareSpecificHTMLCode( charCodeStr, "oslash", 277 ); break;	// oslash
			case 'ugra': case '#249': code = CompareSpecificHTMLCode( charCodeStr, "ugrave", 235 ); break;	// ugrave  
			case 'uacu': case '#250': code = CompareSpecificHTMLCode( charCodeStr, "uacute", 234 ); break;	// uacute 
			case 'ucir': case '#251': code = CompareSpecificHTMLCode( charCodeStr, "ucirc", 236 ); break;	// ucirc  
			case 'uuml': case '#252': code = CompareSpecificHTMLCode( charCodeStr, "uuml", 237 ); break;	// uuml
			case 'yacu': case '#253': code = CompareSpecificHTMLCode( charCodeStr, "yacute", 171 ); break;	// yacute - becomes standard y
			case 'thor': case '#254': code = CompareSpecificHTMLCode( charCodeStr, "thorn", 376 ); break;	// thorn
			case 'yuml': case '#255': code = CompareSpecificHTMLCode( charCodeStr, "yuml", 330 ); break;	// yuml 
						 case '#848'/*#8482*//*: code = 252; break; // trademark
			default: return PDF_FONT_BLANK;
			}
			static char codeStr[16];
			sprintf(codeStr, "\\%03.d", code);
			return codeStr;	
		}
		charCodeStr += *str;
		str++;
		count++;
	}*/
	return PDF_FONT_BLANK;
}

std::string PDFCharScaleFont::ConvertHTMLStrCharCodeToPDFDocEncoding( char *str )
{
	int code = 0;
	std::string charCodeStr = PDF_FONT_BLANK;
	charCodeStr.reserve(16);
	int count = 0;
	while( *str != 0 && count < 10 )
	{
		if (*str == ';')
		{
			// Found a possible HTML special character
			char charCodeIntStr[6];
			strncpy(charCodeIntStr, charCodeStr.c_str(), 6);
			int charCodeInt = 0;
			for(int i=0; i<3; i++)
			{
				charCodeInt = ((charCodeInt + (int)charCodeIntStr[i]) * 256);
			}
			charCodeInt += (int)charCodeIntStr[3];

			// Special case for "frac14", "frac14", "frac34" as they all have commom 4 letters so they...
			// become 'fr12', 'fr12', or 'fr34'...
			if ( charCodeInt == 'frac' )
			{
				charCodeInt = 0;
				for(int i=0; i<2; i++)
				{
					charCodeInt = ((charCodeInt + (int)charCodeIntStr[i]) * 256);
				}
				charCodeInt = ((charCodeInt + (int)charCodeIntStr[4]) * 256);
				charCodeInt += (int)charCodeIntStr[5];
			}

			ArrayIndex::iterator posIter;;
			int pos;

			if ( charCodeIntStr[0] != '#' )
			{
				posIter = htmlShortStrNameIntIndex.find( charCodeInt );
				if ( posIter == htmlShortStrNameIntIndex.end() )
					return "";
				pos = (*posIter).second;
				if ( pos > htmlShortStrNameIntIndex.size() || pos < 0 )
					return "";
				code = HTMLTokenToPDFCharMapArray[pos].pdfEncodingNum;
			}
			else
			{
				posIter = htmlHashNumIntIndex.find( charCodeInt );
				if ( posIter == htmlHashNumIntIndex.end() )
					return "";
				pos = (*posIter).second;
				if ( pos >= htmlHashNumIntIndex.size() )
					return "";
				// Maybe check the text here... too...
				code = HTMLTokenToPDFCharMapArray[pos].pdfEncodingNum;
			}
			static char codeStr[16];
			sprintf(codeStr, "\\%03.d", code);
			return codeStr;	
		}
		charCodeStr += *str;
		str++;
		count++;
	}
	return PDF_FONT_BLANK;
}

void PDFCharScaleFont::InitCharWidthOnePointArray()
{
	for(int i=0; i < 256; i++)
	{
		charOnePointSizeNormalArray[i] = 0;
		charOnePointSizeBoldArray[i] = 0;
		charOnePointSizeItalicArray[i] = 0;
		charOnePointSizeBoldItalicArray[i] = 0;
	}
}

void PDFCharScaleFont::SetNormalCharWidths( CharWidths charWidths[] )
{
	int i = 0;
	unsigned char c = charWidths[i].code;

	while ( c )
	{
		if ( c >= 0 && c <= 255 )
		{
			charOnePointSizeNormalArray[c] = charWidths[i].width;
		}
		i++;
		c = charWidths[i].code;
	}
}
void PDFCharScaleFont::SetBoldCharWidths( CharWidths charWidths[] )
{
	int i = 0;
	unsigned char c = charWidths[i].code;

	while ( c )
	{
		if ( c >= 0 && c <= 255 )
		{
			charOnePointSizeBoldArray[c] = charWidths[i].width;
		}
		i++;
		c = charWidths[i].code;
	}
	
}
void PDFCharScaleFont::SetItalicCharWidths( CharWidths charWidths[] )
{
	int i = 0;
	unsigned char c = charWidths[i].code;

	while ( c )
	{
		if ( c >= 0 && c <= 255 )
		{
			charOnePointSizeItalicArray[c] = charWidths[i].width;
		}
		i++;
		c = charWidths[i].code;
	}
	
}
void PDFCharScaleFont::SetBoldItalicCharWidths( CharWidths charWidths[] )
{
	int i = 0;
	unsigned char c = charWidths[i].code;

	while ( c )
	{
		if ( c >= 0 && c <= 255 )
		{
			charOnePointSizeBoldItalicArray[c] = charWidths[i].width;
		}
		i++;
		c = charWidths[i].code;
	}
	
}


PDFTimesNewRomanFont::PDFTimesNewRomanFont( std::string fontNameP, int fontSizeP, int fontSpacingP /*= 4*/ )
	: PDFCharScaleFont( fontNameP, fontSizeP, fontSpacingP )
{
	InitNormalCharWidths();
	InitBoldCharWidths();
	InitItalicCharWidths();
	InitBoldItalicCharWidths();
}


const double FONT_SIZE_01667 = 50.0/300.0;
const double FONT_SIZE_01799 = 50.0/278.0;
const double FONT_SIZE_01908 = 50.0/262.0;
const double FONT_SIZE_02144 = 50.0/233.2;
const double FONT_SIZE_02222 = 50.0/225.0;
const double FONT_SIZE_02242 = 50.0/223.0;
const double FONT_SIZE_02381 = 50.0/210.0;
const double FONT_SIZE_02444 = 50.0/204.6;
const double FONT_SIZE_02500 = 50.0/200.0;
const double FONT_SIZE_02662 = 50.0/187.8;
const double FONT_SIZE_02750 = 50.0/181.8;
const double FONT_SIZE_02762 = 50.0/181.0;
const double FONT_SIZE_02778 = 50.0/180.0;
const double FONT_SIZE_02784 = 50.0/179.6;
const double FONT_SIZE_02857 = 50.0/175.0;
const double FONT_SIZE_02998 = 50.0/166.8;
const double FONT_SIZE_03005 = 50.0/166.4;
const double FONT_SIZE_03102 = 50.0/161.2;
const double FONT_SIZE_03106 = 50.0/161.0;
const double FONT_SIZE_03300 = 50.0/151.5;
const double FONT_SIZE_03333 = 50.0/150.0;
const double FONT_SIZE_03356 = 50.0/149.0;
const double FONT_SIZE_03484 = 50.0/143.5;
const double FONT_SIZE_03501 = 50.0/142.8;
const double FONT_SIZE_03546 = 50.0/141.0;
const double FONT_SIZE_03647 = 50.0/137.0;
const double FONT_SIZE_03704 = 50.0/135.0;
const double FONT_SIZE_03888 = 50.0/128.6;
const double FONT_SIZE_03891 = 50.0/128.5;
const double FONT_SIZE_04000 = 50.0/125.0;
const double FONT_SIZE_04078 = 50.0/122.6;
const double FONT_SIZE_04440 = 50.0/112.6;
const double FONT_SIZE_04537 = 50.0/110.2;
const double FONT_SIZE_04739 = 50.0/105.5;
const double FONT_SIZE_04753 = 50.0/105.2;
const double FONT_SIZE_05000 = 50.0/100.0;
const double FONT_SIZE_05102 = 50.0/98.0;
const double FONT_SIZE_05144 = 50.0/97.2;
const double FONT_SIZE_05196 = 50.0/96.2;
const double FONT_SIZE_05236 = 50.0/95.5;
const double FONT_SIZE_05319 = 50.0/94.0;
const double FONT_SIZE_05376 = 50.0/93.0;
const double FONT_SIZE_05556 = 50.0/90.0;
const double FONT_SIZE_05631 = 50.0/88.8;
const double FONT_SIZE_05643 = 50.0/88.6;
const double FONT_SIZE_05698 = 50.0/87.75;
const double FONT_SIZE_05708 = 50.0/87.6;
const double FONT_SIZE_05787 = 50.0/86.4;
const double FONT_SIZE_05841 = 50.0/85.6;
const double FONT_SIZE_05896 = 50.0/84.8;
const double FONT_SIZE_06010 = 50.0/83.2;
const double FONT_SIZE_06068 = 50.0/82.4;
const double FONT_SIZE_06098 = 50.0/82.0;
const double FONT_SIZE_06667 = 50.0/75.0;
const double FONT_SIZE_06757 = 50.0/74.0;
const double FONT_SIZE_07225 = 50.0/69.2;
const double FONT_SIZE_07375 = 50.0/67.8;
const double FONT_SIZE_07463 = 50.0/67.0;
const double FONT_SIZE_07599 = 50.0/65.8;
const double FONT_SIZE_07788 = 50.0/64.2;
const double FONT_SIZE_08333 = 50.0/60.0;
const double FONT_SIZE_08897 = 50.0/56.2;
const double FONT_SIZE_09434 = 50.0/53.0;
const double FONT_SIZE_09456 = 50.0/52.875;
const double FONT_SIZE_09804 = 50.0/51.0;
const double FONT_SIZE_10000 = 1.0;

void PDFTimesNewRomanFont::InitNormalCharWidths()
{
	const double TIMESNR_LOWER_ACEZ = 0.4437;
	const double TIMESNR_LOWER_BDGHKNOPQUVXY = 0.5;
	const double TIMESNR_LOWER_FR = 0.3333;
	const double TIMESNR_LOWER_IJLT = 0.2778;
	const double TIMESNR_LOWER_M = 0.7778;
	const double TIMESNR_LOWER_S  = 0.3889;
	const double TIMESNR_LOWER_W = 0.7222;

	const double TIMESNR_UPPER_ADGHKNOQUVXY = TIMESNR_LOWER_W;
	const double TIMESNR_UPPER_BCR = 0.6667;
	const double TIMESNR_UPPER_ELTZ = 0.611;
	const double TIMESNR_UPPER_FPS = 0.556;
	const double TIMESNR_UPPER_I = 0.3347;
	const double TIMESNR_UPPER_J = TIMESNR_LOWER_S;
	const double TIMESNR_UPPER_M = 0.8892;
	const double TIMESNR_UPPER_W = 0.9444;

	const double TIMESNR_NUMBERS = TIMESNR_LOWER_BDGHKNOPQUVXY;

	const double TIMESNR_BAR /* | */ = 0.2;
	const double TIMESNR_COMMADOT /* ,. */ = 0.25;
	const double TIMESNR_SLASHES_ANDMORE /* \/:;spacechar */ = TIMESNR_LOWER_IJLT;
	const double TIMESNR_SQUAREBRACES_ANDMORE /* []`-()! */ = TIMESNR_LOWER_FR;
	const double TIMESNR_APPIONMARK /* ? */ = TIMESNR_LOWER_ACEZ;
	const double TIMESNR_QUOTES /* " */ = FONT_SIZE_04078;
	const double TIMESNR_SINGLEQUOTE /* ' */ = FONT_SIZE_01799;
	const double TIMESNR_CIRCUMFLEX /* ^ */ = 0.469;
	const double TIMESNR_CURLYBRACES /* {} */ = 0.48;
	const double TIMESNR_HASH_ANDMORE /* _#$* */ = TIMESNR_LOWER_BDGHKNOPQUVXY;
	const double TIMESNR_SQUIGGLE /* ~ */ = 0.541;
	const double TIMESNR_EQUALSIGN_ANDMORE /* =+<> */ = 0.5641;
	const double TIMESNR_AMPERSAND /* & */ = TIMESNR_LOWER_M;
	const double TIMESNR_PERCENT /* % */ = 0.8326;
	const double TIMESNR_ATSYMBOL /* @ */ = 0.9208;

	CharOnePointSizeList list;

	CharWidths charWidths[] = {
	(unsigned char)0347, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)0345, TIMESNR_UPPER_ADGHKNOQUVXY,	// Acirc
	(unsigned char)0313, TIMESNR_UPPER_ADGHKNOQUVXY,	// Agrave
	(unsigned char)0256, TIMESNR_UPPER_ADGHKNOQUVXY, //????*/) );	// AElig
	(unsigned char)0201, TIMESNR_UPPER_ADGHKNOQUVXY,	// Aring
	(unsigned char)0314, TIMESNR_UPPER_ADGHKNOQUVXY,	// Atilde
	(unsigned char)0200, TIMESNR_UPPER_ADGHKNOQUVXY,	// Auml
	(unsigned char)0202, TIMESNR_LOWER_ACEZ, //*????*/) );	// Ccedil
	(unsigned char)0056, TIMESNR_LOWER_ACEZ, //*????*/) );	// Dot 
	(unsigned char)0203, TIMESNR_UPPER_ELTZ,	// Eacute
	(unsigned char)0346, TIMESNR_UPPER_ELTZ,	// Ecirc
	(unsigned char)0351, TIMESNR_UPPER_ELTZ,	// Egrave
	(unsigned char)0350, TIMESNR_UPPER_ELTZ,	// Euml
	(unsigned char)0352, TIMESNR_UPPER_I,	// Iacute
	(unsigned char)0353, TIMESNR_UPPER_I,	// Icirc
	(unsigned char)0355, TIMESNR_UPPER_I,	// Igrave
	(unsigned char)0354, TIMESNR_UPPER_I,	// Iuml
	(unsigned char)0204, TIMESNR_UPPER_ADGHKNOQUVXY,	// Ntilde
	(unsigned char)0356, TIMESNR_UPPER_ADGHKNOQUVXY,	// Oacute  
	(unsigned char)0357, TIMESNR_UPPER_ADGHKNOQUVXY,	// Ocirc
	(unsigned char)0361, TIMESNR_UPPER_ADGHKNOQUVXY,	// Ograve
	(unsigned char)0257, TIMESNR_UPPER_ADGHKNOQUVXY,	// Oslash
	(unsigned char)0315, TIMESNR_UPPER_ADGHKNOQUVXY,	// Otilde
	(unsigned char)0235, TIMESNR_UPPER_ADGHKNOQUVXY,	// Ouml
	(unsigned char)0120, TIMESNR_UPPER_FPS, //*????*/) );	// THORN, becomes 'standard P'
	(unsigned char)0362, TIMESNR_UPPER_ADGHKNOQUVXY,	// Uacute
	(unsigned char)0363, TIMESNR_UPPER_ADGHKNOQUVXY,	// Ucirc
	(unsigned char)0364, TIMESNR_UPPER_ADGHKNOQUVXY,	// Ugrave
	(unsigned char)0206, TIMESNR_UPPER_ADGHKNOQUVXY,	// Uuml
	(unsigned char)0131, TIMESNR_UPPER_ADGHKNOQUVXY,	// Yacute, becomes 'standard Y'
	(unsigned char)0207, TIMESNR_LOWER_ACEZ,	// aacute
	(unsigned char)0235, TIMESNR_LOWER_ACEZ,	// acute
	(unsigned char)0235, TIMESNR_LOWER_ACEZ,	// acirc
	(unsigned char)0276, TIMESNR_LOWER_ACEZ,	// aelig
	(unsigned char)0210, TIMESNR_LOWER_ACEZ,	// agrave
	(unsigned char)0214, TIMESNR_LOWER_ACEZ,	// aring
	(unsigned char)0235, TIMESNR_LOWER_ACEZ,	// atilde
	(unsigned char)0212, TIMESNR_LOWER_ACEZ,	// auml
	(unsigned char)0174, TIMESNR_BAR,	// brvbar, becomes standard bar '|'
	(unsigned char)0215, TIMESNR_LOWER_ACEZ,	// ccedil
	(unsigned char)0374, TIMESNR_LOWER_ACEZ,	// cedil
	(unsigned char)0242, TIMESNR_LOWER_ACEZ,	// cent
	(unsigned char)0251, TIMESNR_ATSYMBOL,	// copy
	(unsigned char)0333, TIMESNR_LOWER_ACEZ, //*????*/) );	// curren
	(unsigned char)0241, TIMESNR_LOWER_ACEZ, //*????*/) );	// deg 
	(unsigned char)0216, TIMESNR_LOWER_ACEZ,	// eacute 
	(unsigned char)0220, TIMESNR_LOWER_ACEZ,	// ecirc 
	(unsigned char)0217, TIMESNR_LOWER_ACEZ,	// egrave
	(unsigned char)0144, TIMESNR_LOWER_BDGHKNOPQUVXY,	// eth, problem!! becomes "d' as this is what it loks most like?
	(unsigned char)0221, TIMESNR_LOWER_ACEZ,	// euml
	(unsigned char)0222, TIMESNR_LOWER_IJLT,	// iacute
	(unsigned char)0224, TIMESNR_LOWER_IJLT,	// icirc
	(unsigned char)0223, TIMESNR_LOWER_IJLT,	// igrave
	(unsigned char)0225, TIMESNR_LOWER_IJLT,	// iuml
	(unsigned char)0370, TIMESNR_LOWER_ACEZ, //*????*/) );	// macr
	(unsigned char)0341, TIMESNR_LOWER_ACEZ, //*????*/) );	// middot, using 'period centered'
	(unsigned char)0226, TIMESNR_LOWER_BDGHKNOPQUVXY,	// ntilde 
	(unsigned char)0227, TIMESNR_LOWER_BDGHKNOPQUVXY,	// oacute  
	(unsigned char)0231, TIMESNR_LOWER_BDGHKNOPQUVXY,	// ocirc  
	(unsigned char)0230, TIMESNR_LOWER_BDGHKNOPQUVXY,	// ograve
	(unsigned char)0273, TIMESNR_LOWER_BDGHKNOPQUVXY,	// ordf
	(unsigned char)0274, TIMESNR_LOWER_BDGHKNOPQUVXY,	// ordm
	(unsigned char)0277, TIMESNR_LOWER_BDGHKNOPQUVXY,	// oslash
	(unsigned char)0233, TIMESNR_LOWER_BDGHKNOPQUVXY,	// otilde  
	(unsigned char)0232, TIMESNR_LOWER_ACEZ,	// ouml
	(unsigned char)0246, TIMESNR_LOWER_ACEZ, //*????*/) );	// para
	(unsigned char)0053, TIMESNR_LOWER_ACEZ, //*????*/) );	// plusmn
	(unsigned char)0243, TIMESNR_LOWER_ACEZ, //*????*/) );	// pound, bcomes 'sterling'
	(unsigned char)0250, TIMESNR_ATSYMBOL, //*????*/) );	// reg 
	(unsigned char)0244, TIMESNR_LOWER_ACEZ, //*????*/) );	// sect
	(unsigned char)0234, TIMESNR_LOWER_ACEZ, //*????*/) );  
	(unsigned char)0236, TIMESNR_LOWER_BDGHKNOPQUVXY,	// ucirc  
	(unsigned char)0235, TIMESNR_LOWER_BDGHKNOPQUVXY,	// ugrave  
	(unsigned char)0237, TIMESNR_LOWER_BDGHKNOPQUVXY,	// uml
	(unsigned char)0237, TIMESNR_LOWER_BDGHKNOPQUVXY,	// uuml
	(unsigned char)0264, TIMESNR_LOWER_ACEZ,	// yen 
	(unsigned char)0330, TIMESNR_LOWER_BDGHKNOPQUVXY,	// yuml 
	(unsigned char)0235, TIMESNR_LOWER_ACEZ,	// code = xxx; break325";
	(unsigned char)0252, TIMESNR_ATSYMBOL,	// trademark
	// Foreign letters and symbols
	(unsigned char)130 /* C cedil		*/, TIMESNR_UPPER_BCR,
	(unsigned char)131 /* E egrave	*/, TIMESNR_UPPER_ELTZ,
	(unsigned char)132 /* N tilde		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)133 /* 0 umlaut	*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)134 /* U umlaut	*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)135 /* a acute		*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)136 /* a grave		*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)137 /* a circum	*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)138 /* a umlaut	*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)139 /* a tilde		*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)140 /* a ring		*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)141 /* c cedil		*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)142 /* e acute		*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)143 /* e grave		*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)144 /* e circum	*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)145 /* e umlaut	*/, TIMESNR_LOWER_ACEZ,
	(unsigned char)146 /* l acute		*/, TIMESNR_LOWER_IJLT /* for foreign i */,
	(unsigned char)147 /* l grave		*/, TIMESNR_LOWER_IJLT /* for foreign i */,
	(unsigned char)148 /* l circum	*/, TIMESNR_LOWER_IJLT /* for foreign i */,
	(unsigned char)149 /* l umlaut	*/, TIMESNR_LOWER_IJLT /* for foreign i */,
	(unsigned char)150 /* n titde		*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)151 /* o acute		*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)152 /* o grave		*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)153 /* o circum	*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)154 /* o umlaut	*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)155 /* o titde		*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)156 /* u acute		*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)157 /* u grave		*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)158 /* u circum	*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)159 /* u umlaut	*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)160 /* cross		*/, FONT_SIZE_05000,
	(unsigned char)161 /* ring		*/, FONT_SIZE_04000,
	(unsigned char)162 /* cent		*/, FONT_SIZE_05000,
	(unsigned char)163 /* pound		*/, FONT_SIZE_05000,
	(unsigned char)164 /* double S	*/, FONT_SIZE_05000,
	(unsigned char)165 /* dot			*/, FONT_SIZE_03501,
	(unsigned char)166 /* para end	*/, FONT_SIZE_04537,
	(unsigned char)167 /* german SS	*/, FONT_SIZE_05000,
	(unsigned char)168 /* register	*/, FONT_SIZE_07599,
	(unsigned char)169 /* copyright	*/, FONT_SIZE_07599,
	(unsigned char)170 /* trademark	*/, FONT_SIZE_09804,
	(unsigned char)171 /* apostrophe	*/, FONT_SIZE_03333,
	(unsigned char)172 /* umlaut		*/, FONT_SIZE_03333,
	//( (unsigned char)173 /* space		*/, 0,
	(unsigned char)174 /* AE ligature	*/, FONT_SIZE_08897,
	(unsigned char)175 /* O slash		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	//( (unsigned char)176 /* space		*/, 0,
	(unsigned char)177 /* plusminus	*/, FONT_SIZE_05643,
	//( (unsigned char)178 /* space		*/, 0,
	//( (unsigned char)179 /* space		*/, 0,
	(unsigned char)180 /* yen			*/, FONT_SIZE_05000,
	(unsigned char)181 /* u long stem	*/, FONT_SIZE_05102,
	//( (unsigned char)182 /* space		*/, 0,
	//( (unsigned char)183 /* space		*/, 0,
	//( (unsigned char)184 /* space		*/, 0,
	//( (unsigned char)185 /* space		*/, 0,
	//( (unsigned char)186 /* space		*/, 0,
	(unsigned char)187 /* a little	*/, FONT_SIZE_02762,
	(unsigned char)188 /* o little	*/, FONT_SIZE_03102,
	//( (unsigned char)189 /* space		*/, 0,
	(unsigned char)190 /* ae ligature	*/, FONT_SIZE_06667,
	(unsigned char)191 /* o slash		*/, FONT_SIZE_05000,
	(unsigned char)192 /* ? inverted	*/, FONT_SIZE_04440,
	(unsigned char)193 /* ! inverted	*/, FONT_SIZE_03333,
	(unsigned char)194 /* right angle	*/, FONT_SIZE_05643,
	//( (unsigned char)195 /* space		*/, 0,
	(unsigned char)196 /* f fancy		*/, FONT_SIZE_05000,
	//( (unsigned char)197 /* space		*/, 0,
	//( (unsigned char)198 /* space		*/, 0,
	(unsigned char)199 /* <<			*/, FONT_SIZE_05000,
	(unsigned char)200 /* >>			*/, FONT_SIZE_05000,
	(unsigned char)201 /* ... ellipse	*/, FONT_SIZE_10000,
	//( (unsigned char)202 /* space		*/, 0,
	(unsigned char)203 /* A grave		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)204 /* A tilde		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)205 /* O tilde		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)206 /* OE ligature	*/, FONT_SIZE_08897,
	(unsigned char)207 /* oe ligature	*/, FONT_SIZE_07225,
	(unsigned char)208 /* underscore	*/, FONT_SIZE_05000,
	(unsigned char)209 /* long under	*/, FONT_SIZE_10000,
	(unsigned char)210 /* dquotesopen	*/, FONT_SIZE_04440,
	(unsigned char)211 /* dquotesclose*/, FONT_SIZE_04440,
	(unsigned char)212 /* squotesopen	*/, FONT_SIZE_03333,
	(unsigned char)213 /* squotesclose*/, FONT_SIZE_03333,
	(unsigned char)214 /* divide sign	*/, FONT_SIZE_05643,
	//( (unsigned char)215 /* space		*/, 0,
	(unsigned char)216 /* y umlaut	*/, TIMESNR_LOWER_BDGHKNOPQUVXY,
	(unsigned char)217 /* Y umlaut	*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)218 /* / 45d slash	*/, FONT_SIZE_01667,
	(unsigned char)219 /* circlestar	*/, FONT_SIZE_05000,
	(unsigned char)220 /* <			*/, FONT_SIZE_03333,
	(unsigned char)221 /* >			*/, FONT_SIZE_03333,
	(unsigned char)222 /* fi ligature */, FONT_SIZE_05556,
	(unsigned char)223 /* fl ligature	*/, FONT_SIZE_05556,
	(unsigned char)224 /* bar 2 slash	*/, FONT_SIZE_05000,
	(unsigned char)225 /* . dot		*/, FONT_SIZE_02500,
	(unsigned char)226 /* , comma		*/, FONT_SIZE_03333,
	(unsigned char)227 /* ,, 2 comma	*/, FONT_SIZE_04440,
	(unsigned char)228 /* % with o	*/, FONT_SIZE_10000,
	(unsigned char)229 /* A accent	*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)230 /* E accent	*/, TIMESNR_UPPER_ELTZ,
	(unsigned char)231 /* A acute		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)232 /* E umlaut	*/, TIMESNR_UPPER_ELTZ,
	(unsigned char)233 /* E grave		*/, TIMESNR_UPPER_ELTZ,
	(unsigned char)234 /* I acute		*/, FONT_SIZE_03356,
	(unsigned char)235 /* I accent	*/, FONT_SIZE_03356,
	(unsigned char)236 /* I umlaut	*/, FONT_SIZE_03356,
	(unsigned char)237 /* I grave		*/, FONT_SIZE_03356,
	(unsigned char)238 /* O acute		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)239 /* O accent	*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	//( (unsigned char)240 /* space		*/, 0,
	(unsigned char)241 /* O grave		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)242 /* U acute		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)243 /* U accent	*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)244 /* U grave		*/, TIMESNR_UPPER_ADGHKNOQUVXY,
	(unsigned char)245 /* / slash		*/, FONT_SIZE_02778,
	(unsigned char)246 /* ^ accent	*/, FONT_SIZE_03333,
	(unsigned char)247 /* ~ squiggle	*/, FONT_SIZE_03333,
	(unsigned char)248 /* - high dash	*/, FONT_SIZE_03333,
	(unsigned char)249 /* high u		*/, FONT_SIZE_03333,
	(unsigned char)250 /* high dot	*/, FONT_SIZE_03333,
	(unsigned char)251 /* high circle	*/, FONT_SIZE_03333,
	(unsigned char)252 /* curl left	*/, FONT_SIZE_03333,
	(unsigned char)253 /* acute		*/, FONT_SIZE_03333,
	(unsigned char)254 /* curl right	*/, FONT_SIZE_03333,
	(unsigned char)255 /* accent inv	*/, FONT_SIZE_03333,
	// English letters and symbols
	'a', TIMESNR_LOWER_ACEZ,
	'b', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'c', TIMESNR_LOWER_ACEZ,
	'd', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'e', TIMESNR_LOWER_ACEZ,
	'f', TIMESNR_LOWER_FR,
	'g', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'h', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'i', TIMESNR_LOWER_IJLT,
	'j', TIMESNR_LOWER_IJLT,
	'k', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'l', TIMESNR_LOWER_IJLT,
	'm', TIMESNR_LOWER_M,
	'n', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'o', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'p', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'q', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'r', TIMESNR_LOWER_FR,
	's', TIMESNR_LOWER_S,
	't', TIMESNR_LOWER_IJLT,
	'u', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'v', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'w', TIMESNR_LOWER_W,
	'x', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'y', TIMESNR_LOWER_BDGHKNOPQUVXY,
	'z', TIMESNR_LOWER_ACEZ,
	'A', TIMESNR_UPPER_ADGHKNOQUVXY,
	'B', TIMESNR_UPPER_BCR,
	'C', TIMESNR_UPPER_BCR,
	'D', TIMESNR_UPPER_ADGHKNOQUVXY,
	'E', TIMESNR_UPPER_ELTZ,
	'F', TIMESNR_UPPER_FPS,
	'G', TIMESNR_UPPER_ADGHKNOQUVXY,
	'H', TIMESNR_UPPER_ADGHKNOQUVXY,
	'I', TIMESNR_UPPER_I,
	'J', TIMESNR_UPPER_J,
	'K', TIMESNR_UPPER_ADGHKNOQUVXY,
	'L', TIMESNR_UPPER_ELTZ,
	'M', TIMESNR_UPPER_M,
	'N', TIMESNR_UPPER_ADGHKNOQUVXY,
	'O', TIMESNR_UPPER_ADGHKNOQUVXY,
	'P', TIMESNR_UPPER_FPS,
	'Q', TIMESNR_UPPER_ADGHKNOQUVXY,
	'R', TIMESNR_UPPER_BCR,
	'S', TIMESNR_UPPER_FPS,
	'T', TIMESNR_UPPER_ELTZ,
	'U', TIMESNR_UPPER_ADGHKNOQUVXY,
	'V', TIMESNR_UPPER_ADGHKNOQUVXY,
	'W', TIMESNR_UPPER_W,
	'X', TIMESNR_UPPER_ADGHKNOQUVXY,
	'Y', TIMESNR_UPPER_ADGHKNOQUVXY,
	'Z', TIMESNR_UPPER_ELTZ,
	'0', TIMESNR_NUMBERS,
	'1', TIMESNR_NUMBERS,
	'2', TIMESNR_NUMBERS,
	'3', TIMESNR_NUMBERS,
	'4', TIMESNR_NUMBERS,
	'5', TIMESNR_NUMBERS,
	'6', TIMESNR_NUMBERS,
	'7', TIMESNR_NUMBERS,
	'8', TIMESNR_NUMBERS,
	'9', TIMESNR_NUMBERS,
	'|', TIMESNR_BAR,
	',', TIMESNR_COMMADOT,
	'.', TIMESNR_COMMADOT,
	'\\',TIMESNR_SLASHES_ANDMORE,
	'/', TIMESNR_SLASHES_ANDMORE,
	':', TIMESNR_SLASHES_ANDMORE,
	';', TIMESNR_SLASHES_ANDMORE,
	' ', TIMESNR_SLASHES_ANDMORE,
	'[', TIMESNR_SQUAREBRACES_ANDMORE,
	']', TIMESNR_SQUAREBRACES_ANDMORE,
	'`', TIMESNR_SQUAREBRACES_ANDMORE,
	'-', TIMESNR_SQUAREBRACES_ANDMORE,
	'(', TIMESNR_SQUAREBRACES_ANDMORE,
	')', TIMESNR_SQUAREBRACES_ANDMORE,
	'!', TIMESNR_SQUAREBRACES_ANDMORE,
	'?', TIMESNR_APPIONMARK,
	'"', TIMESNR_QUOTES,
	'\'',TIMESNR_SINGLEQUOTE,
	'^', TIMESNR_CIRCUMFLEX,
	'{', TIMESNR_CURLYBRACES,
	'}', TIMESNR_CURLYBRACES,
	'_', TIMESNR_HASH_ANDMORE,
	'#', TIMESNR_HASH_ANDMORE,
	'$', TIMESNR_HASH_ANDMORE,
	'*', TIMESNR_HASH_ANDMORE,
	'~', TIMESNR_SQUIGGLE,
	'=', TIMESNR_EQUALSIGN_ANDMORE,
	'+', TIMESNR_EQUALSIGN_ANDMORE,
	'<', TIMESNR_EQUALSIGN_ANDMORE,
	'>', TIMESNR_EQUALSIGN_ANDMORE,
	'&', TIMESNR_AMPERSAND,
	'%', TIMESNR_PERCENT,
	'@', TIMESNR_ATSYMBOL,
	0, 0
	};

	//SetCharCodeList( charWidths, &list );
	SetNormalCharWidths( charWidths ); // Set the list of Scaled 
}


void PDFTimesNewRomanFont::InitBoldCharWidths()
{
	const double TIMESNR_BOLD_LOWER_AVXY = FONT_SIZE_05000;
	const double TIMESNR_BOLD_LOWER_BDHKNPQU = FONT_SIZE_05556;
	const double TIMESNR_BOLD_LOWER_CERZ = FONT_SIZE_04440;
	const double TIMESNR_BOLD_LOWER_FJT = FONT_SIZE_03333;
	const double TIMESNR_BOLD_LOWER_GO = FONT_SIZE_05000;
	const double TIMESNR_BOLD_LOWER_IL = FONT_SIZE_02778;
	const double TIMESNR_BOLD_LOWER_M = FONT_SIZE_08333;
	const double TIMESNR_BOLD_LOWER_S  = FONT_SIZE_03888;
	const double TIMESNR_BOLD_LOWER_W = FONT_SIZE_07225;

	const double TIMESNR_BOLD_UPPER_ACDNRUVXY = FONT_SIZE_07225;
	const double TIMESNR_BOLD_UPPER_BELTZ = FONT_SIZE_06667;
	const double TIMESNR_BOLD_UPPER_S = FONT_SIZE_05556;
	const double TIMESNR_BOLD_UPPER_FP = FONT_SIZE_06098;
	const double TIMESNR_BOLD_UPPER_HKGOQ = FONT_SIZE_07788;
	const double TIMESNR_BOLD_UPPER_I = FONT_SIZE_03888;
	const double TIMESNR_BOLD_UPPER_J = FONT_SIZE_05000;
	const double TIMESNR_BOLD_UPPER_M = FONT_SIZE_09434;
	const double TIMESNR_BOLD_UPPER_W = FONT_SIZE_10000;

	const double TIMESNR_BOLD_NUMBERS = FONT_SIZE_05000;

	const double TIMESNR_BOLD_BAR /* | */ = 0.2203;
	const double TIMESNR_BOLD_COMMADOT_SPACE /* ,.spacechar */ = 0.25;
	const double TIMESNR_BOLD_SLASHES /* \/ */ = 0.2778;
	const double TIMESNR_BOLD_SQUAREBRACES_ANDMORE /* :;[]!-`() */ = FONT_SIZE_03333;
	const double TIMESNR_BOLD_APPIONMARK /* ? */ = FONT_SIZE_05000;
	const double TIMESNR_BOLD_QUOTES /* " */ = FONT_SIZE_05556;
	const double TIMESNR_BOLD_SINGLEQUOTE /* ' */ = FONT_SIZE_02778;
	const double TIMESNR_BOLD_CIRCUMFLEX /* ^ */ = 0.5814;
	const double TIMESNR_BOLD_STAR /* * */ = FONT_SIZE_05000;
	const double TIMESNR_BOLD_CURLYBRACES /* {} */ = 0.3937;
	const double TIMESNR_BOLD_HASH_ANDMORE /* _#$ */ = FONT_SIZE_05000;
	const double TIMESNR_BOLD_SQUIGGLE /* ~ */ = FONT_SIZE_05196;
	const double TIMESNR_BOLD_EQUALSIGN_ANDMORE /* =+<> */ = 0.5714;
	const double TIMESNR_BOLD_AMPERSAND /* & */ = FONT_SIZE_08333;
	const double TIMESNR_BOLD_PERCENT /* % */ = FONT_SIZE_10000;
	const double TIMESNR_BOLD_ATSYMBOL /* @ */ = 0.9294;

	CharOnePointSizeList list;
	CharWidths charWidths[] = {
	(unsigned char)0347, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Aacute
	(unsigned char)0345, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Acirc
	(unsigned char)0313, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Agrave
	(unsigned char)0256, TIMESNR_BOLD_UPPER_ACDNRUVXY /*????*/,	// AElig
	(unsigned char)0201, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Aring
	(unsigned char)0314, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Atilde
	(unsigned char)0200, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Auml
	(unsigned char)0202, TIMESNR_BOLD_UPPER_ACDNRUVXY /*????*/,	// Ccedil
	(unsigned char)0056, TIMESNR_BOLD_COMMADOT_SPACE /*????*/,	// Dot 
	(unsigned char)0203, TIMESNR_BOLD_UPPER_BELTZ,	// Eacute
	(unsigned char)0346, TIMESNR_BOLD_UPPER_BELTZ,	// Ecirc
	(unsigned char)0351, TIMESNR_BOLD_UPPER_BELTZ,	// Egrave
	(unsigned char)0350, TIMESNR_BOLD_UPPER_BELTZ,	// Euml
	(unsigned char)0352, TIMESNR_BOLD_UPPER_I,	// Iacute
	(unsigned char)0353, TIMESNR_BOLD_UPPER_I,	// Icirc
	(unsigned char)0355, TIMESNR_BOLD_UPPER_I,	// Igrave
	(unsigned char)0354, TIMESNR_BOLD_UPPER_I,	// Iuml
	(unsigned char)0204, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Ntilde
	(unsigned char)0356, TIMESNR_BOLD_UPPER_HKGOQ,	// Oacute  
	(unsigned char)0357, TIMESNR_BOLD_UPPER_HKGOQ,	// Ocirc
	(unsigned char)0361, TIMESNR_BOLD_UPPER_HKGOQ,	// Ograve
	(unsigned char)0257, TIMESNR_BOLD_UPPER_HKGOQ,	// Oslash
	(unsigned char)0315, TIMESNR_BOLD_UPPER_HKGOQ,	// Otilde
	(unsigned char)0235, TIMESNR_BOLD_UPPER_HKGOQ,	// Ouml
	(unsigned char)0120, TIMESNR_BOLD_UPPER_FP /*????*/,	// THORN, becomes 'standard P'
	(unsigned char)0362, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Uacute
	(unsigned char)0363, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Ucirc
	(unsigned char)0364, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Ugrave
	(unsigned char)0206, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Uuml
	(unsigned char)0131, TIMESNR_BOLD_UPPER_ACDNRUVXY,	// Yacute, becomes 'standard Y'
	(unsigned char)0207, TIMESNR_BOLD_LOWER_AVXY,	// aacute
	(unsigned char)0235, TIMESNR_BOLD_LOWER_AVXY,	// acute
	(unsigned char)0235, TIMESNR_BOLD_LOWER_AVXY,	// acirc
	(unsigned char)0276, TIMESNR_BOLD_LOWER_AVXY,	// aelig
	(unsigned char)0210, TIMESNR_BOLD_LOWER_AVXY,	// agrave
	(unsigned char)0214, TIMESNR_BOLD_LOWER_AVXY,	// aring
	(unsigned char)0235, TIMESNR_BOLD_LOWER_AVXY,	// atilde
	(unsigned char)0212, TIMESNR_BOLD_LOWER_AVXY,	// auml
	(unsigned char)0174, TIMESNR_BOLD_BAR,	// brvbar, becomes standard bar '|'
	(unsigned char)0215, TIMESNR_BOLD_LOWER_CERZ,	// ccedil
	(unsigned char)0374, TIMESNR_BOLD_LOWER_CERZ,	// cedil
	(unsigned char)0242, TIMESNR_BOLD_LOWER_CERZ,	// cent
	(unsigned char)0251, TIMESNR_BOLD_ATSYMBOL,	// copy
	(unsigned char)0333, TIMESNR_BOLD_LOWER_CERZ /*????*/,	// curren
	(unsigned char)0241, TIMESNR_BOLD_LOWER_CERZ /*????*/,	// deg 
	(unsigned char)0216, TIMESNR_BOLD_LOWER_CERZ,	// eacute 
	(unsigned char)0220, TIMESNR_BOLD_LOWER_CERZ,	// ecirc 
	(unsigned char)0217, TIMESNR_BOLD_LOWER_CERZ,	// egrave
	(unsigned char)0144, TIMESNR_BOLD_LOWER_BDHKNPQU,	// eth, problem!! becomes "d' as this is what it loks most like?
	(unsigned char)0221, TIMESNR_BOLD_LOWER_CERZ,	// euml
	(unsigned char)0222, TIMESNR_BOLD_LOWER_IL,	// iacute
	(unsigned char)0224, TIMESNR_BOLD_LOWER_IL,	// icirc
	(unsigned char)0223, TIMESNR_BOLD_LOWER_IL,	// igrave
	(unsigned char)0225, TIMESNR_BOLD_LOWER_IL,	// iuml
	(unsigned char)0370, TIMESNR_BOLD_LOWER_AVXY /*????*/,	// macr
	(unsigned char)0341, TIMESNR_BOLD_COMMADOT_SPACE /*????*/,	// middot, using 'period centered'
	(unsigned char)0226, TIMESNR_BOLD_LOWER_BDHKNPQU,	// ntilde 
	(unsigned char)0227, TIMESNR_BOLD_UPPER_HKGOQ,	// oacute  
	(unsigned char)0231, TIMESNR_BOLD_UPPER_HKGOQ,	// ocirc  
	(unsigned char)0230, TIMESNR_BOLD_UPPER_HKGOQ,	// ograve
	(unsigned char)0273, TIMESNR_BOLD_UPPER_HKGOQ,	// ordf
	(unsigned char)0274, TIMESNR_BOLD_UPPER_HKGOQ,	// ordm
	(unsigned char)0277, TIMESNR_BOLD_UPPER_HKGOQ,	// oslash
	(unsigned char)0233, TIMESNR_BOLD_UPPER_HKGOQ,	// otilde  
	(unsigned char)0232, TIMESNR_BOLD_UPPER_HKGOQ,	// ouml
	(unsigned char)0246, TIMESNR_BOLD_LOWER_AVXY /*????*/,	// para
	(unsigned char)0053, TIMESNR_BOLD_LOWER_AVXY /*????*/,	// plusmn
	(unsigned char)0243, TIMESNR_BOLD_LOWER_AVXY /*????*/,	// pound, bcomes 'sterling'
	(unsigned char)0250, TIMESNR_BOLD_ATSYMBOL /*????*/,	// reg 
	(unsigned char)0244, TIMESNR_BOLD_LOWER_AVXY /*????*/,	// sect
	(unsigned char)0234, TIMESNR_BOLD_LOWER_AVXY /*????*/,  
	(unsigned char)0236, TIMESNR_BOLD_LOWER_AVXY,	// ucirc  
	(unsigned char)0235, TIMESNR_BOLD_LOWER_AVXY,	// ugrave  
	(unsigned char)0237, TIMESNR_BOLD_LOWER_AVXY,	// uml
	(unsigned char)0237, TIMESNR_BOLD_LOWER_AVXY,	// uuml
	(unsigned char)0264, TIMESNR_BOLD_HASH_ANDMORE,	// yen 
	(unsigned char)0330, TIMESNR_BOLD_LOWER_AVXY,	// yuml 
	(unsigned char)0235, TIMESNR_BOLD_LOWER_AVXY,	// code = xxx; break325";
	(unsigned char)0252, TIMESNR_BOLD_ATSYMBOL,	// trademark

	// Foreign letters and symbols
	(unsigned char)130 /* C cedil		*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)131 /* E egrave	*/, TIMESNR_BOLD_UPPER_BELTZ,
	(unsigned char)132 /* N tilde		*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)133 /* 0 umlaut	*/, TIMESNR_BOLD_UPPER_HKGOQ,
	(unsigned char)134 /* U umlaut	*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)135 /* a acute		*/, TIMESNR_BOLD_LOWER_AVXY,
	(unsigned char)136 /* a grave		*/, TIMESNR_BOLD_LOWER_AVXY,
	(unsigned char)137 /* a circum	*/, TIMESNR_BOLD_LOWER_AVXY,
	(unsigned char)138 /* a umlaut	*/, TIMESNR_BOLD_LOWER_AVXY,
	(unsigned char)139 /* a tilde		*/, TIMESNR_BOLD_LOWER_AVXY,
	(unsigned char)140 /* a ring		*/, TIMESNR_BOLD_LOWER_AVXY,
	(unsigned char)141 /* c cedil		*/, TIMESNR_BOLD_LOWER_CERZ,
	(unsigned char)142 /* e acute		*/, TIMESNR_BOLD_LOWER_CERZ,
	(unsigned char)143 /* e grave		*/, TIMESNR_BOLD_LOWER_CERZ,
	(unsigned char)144 /* e circum	*/, TIMESNR_BOLD_LOWER_CERZ,
	(unsigned char)145 /* e umlaut	*/, TIMESNR_BOLD_LOWER_CERZ,
	(unsigned char)146 /* l acute		*/, TIMESNR_BOLD_LOWER_IL /* for foreign i */,
	(unsigned char)147 /* l grave		*/, TIMESNR_BOLD_LOWER_IL /* for foreign i */,
	(unsigned char)148 /* l circum	*/, TIMESNR_BOLD_LOWER_IL /* for foreign i */,
	(unsigned char)149 /* l umlaut	*/, TIMESNR_BOLD_LOWER_IL /* for foreign i */,
	(unsigned char)150 /* n titde		*/, TIMESNR_BOLD_LOWER_BDHKNPQU,
	(unsigned char)151 /* o acute		*/, TIMESNR_BOLD_LOWER_GO,
	(unsigned char)152 /* o grave		*/, TIMESNR_BOLD_LOWER_GO,
	(unsigned char)153 /* o circum	*/, TIMESNR_BOLD_LOWER_GO,
	(unsigned char)154 /* o umlaut	*/, TIMESNR_BOLD_LOWER_GO,
	(unsigned char)155 /* o titde		*/, TIMESNR_BOLD_LOWER_GO,
	(unsigned char)156 /* u acute		*/, TIMESNR_BOLD_LOWER_BDHKNPQU,
	(unsigned char)157 /* u grave		*/, TIMESNR_BOLD_LOWER_BDHKNPQU,
	(unsigned char)158 /* u circum	*/, TIMESNR_BOLD_LOWER_BDHKNPQU,
	(unsigned char)159 /* u umlaut	*/, TIMESNR_BOLD_LOWER_BDHKNPQU,
	(unsigned char)160 /* cross		*/, FONT_SIZE_05000,
	(unsigned char)161 /* ring		*/, FONT_SIZE_04000,
	(unsigned char)162 /* cent		*/, FONT_SIZE_05000,
	(unsigned char)163 /* pound		*/, FONT_SIZE_05000,
	(unsigned char)164 /* double S	*/, FONT_SIZE_05000,
	(unsigned char)165 /* dot			*/, FONT_SIZE_03501,
	(unsigned char)166 /* para end	*/, FONT_SIZE_05376,
	(unsigned char)167 /* german SS	*/, FONT_SIZE_05556,
	(unsigned char)168 /* register	*/, FONT_SIZE_07463,
	(unsigned char)169 /* copyright	*/, FONT_SIZE_07463,
	(unsigned char)170 /* trademark	*/, FONT_SIZE_10000,
	(unsigned char)171 /* apostrophe	*/, FONT_SIZE_03333,
	(unsigned char)172 /* umlaut		*/, FONT_SIZE_03333,
	//(unsigned char)173 /* space		*/, 0,
	(unsigned char)174 /* AE ligature	*/, FONT_SIZE_10000,
	(unsigned char)175 /* O slash		*/, TIMESNR_BOLD_UPPER_HKGOQ,
	//(unsigned char)176 /* space		*/, 0,
	(unsigned char)177 /* plusminus	*/, FONT_SIZE_05698,
	//(unsigned char)178 /* space		*/, 0,
	//(unsigned char)179 /* space		*/, 0,
	(unsigned char)180 /* yen			*/, FONT_SIZE_05000,
	(unsigned char)181 /* u long stem	*/, FONT_SIZE_06010,
	//(unsigned char)182 /* space		*/, 0,
	//(unsigned char)183 /* space		*/, 0,
	//(unsigned char)184 /* space		*/, 0,
	//(unsigned char)185 /* space		*/, 0,
	//(unsigned char)186 /* space		*/, 0,
	(unsigned char)187 /* a little	*/, FONT_SIZE_02998,
	(unsigned char)188 /* o little	*/, FONT_SIZE_03300,
	//(unsigned char)189 /* space		*/, 0,
	(unsigned char)190 /* ae ligature	*/, FONT_SIZE_07225,
	(unsigned char)191 /* o slash		*/, FONT_SIZE_05000,
	(unsigned char)192 /* ? inverted	*/, FONT_SIZE_05000,
	(unsigned char)193 /* ! inverted	*/, FONT_SIZE_03333,
	(unsigned char)194 /* right angle	*/, FONT_SIZE_05708,
	//(unsigned char)195 /* space		*/, 0,
	(unsigned char)196 /* f fancy		*/, FONT_SIZE_05000,
	//(unsigned char)197 /* space		*/, 0,
	//(unsigned char)198 /* space		*/, 0,
	(unsigned char)199 /* <<			*/, FONT_SIZE_05000,
	(unsigned char)200 /* >>			*/, FONT_SIZE_05000,
	(unsigned char)201 /* ... ellipse	*/, FONT_SIZE_10000,
	//(unsigned char)202 /* space		*/, 0,
	(unsigned char)203 /* A grave		*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)204 /* A tilde		*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)205 /* O tilde		*/, TIMESNR_BOLD_UPPER_HKGOQ,
	(unsigned char)206 /* OE ligature	*/, FONT_SIZE_10000,
	(unsigned char)207 /* oe ligature	*/, FONT_SIZE_07225,
	(unsigned char)208 /* underscore	*/, FONT_SIZE_05000,
	(unsigned char)209 /* long under	*/, FONT_SIZE_10000,
	(unsigned char)210 /* dquotesopen	*/, FONT_SIZE_05000,
	(unsigned char)211 /* dquotesclose*/, FONT_SIZE_05000,
	(unsigned char)212 /* squotesopen	*/, FONT_SIZE_03333,
	(unsigned char)213 /* squotesclose*/, FONT_SIZE_03333,
	(unsigned char)214 /* divide sign	*/, FONT_SIZE_05698,
	//(unsigned char)215 /* space		*/, 0,
	(unsigned char)216 /* y umlaut	*/, TIMESNR_BOLD_LOWER_AVXY,
	(unsigned char)217 /* Y umlaut	*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)218 /* / 45d slash	*/, FONT_SIZE_01667,
	(unsigned char)219 /* circlestar	*/, FONT_SIZE_05000,
	(unsigned char)220 /* <			*/, FONT_SIZE_03333,
	(unsigned char)221 /* >			*/, FONT_SIZE_03333,
	(unsigned char)222 /* fi ligature */, FONT_SIZE_05556,
	(unsigned char)223 /* fl ligature	*/, FONT_SIZE_05556,
	(unsigned char)224 /* bar 2 slash	*/, FONT_SIZE_05000,
	(unsigned char)225 /* . dot		*/, FONT_SIZE_02500,
	(unsigned char)226 /* , comma		*/, FONT_SIZE_03333,
	(unsigned char)227 /* ,, 2 comma	*/, FONT_SIZE_05000,
	(unsigned char)228 /* % with o	*/, FONT_SIZE_10000,
	(unsigned char)229 /* A accent	*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)230 /* E accent	*/, TIMESNR_BOLD_UPPER_BELTZ,
	(unsigned char)231 /* A acute		*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)232 /* E umlaut	*/, TIMESNR_BOLD_UPPER_BELTZ,
	(unsigned char)233 /* E grave		*/, TIMESNR_BOLD_UPPER_BELTZ,
	(unsigned char)234 /* I acute		*/, FONT_SIZE_03891,
	(unsigned char)235 /* I accent	*/, FONT_SIZE_03891,
	(unsigned char)236 /* I umlaut	*/, FONT_SIZE_03891,
	(unsigned char)237 /* I grave		*/, FONT_SIZE_03891,
	(unsigned char)238 /* O acute		*/, TIMESNR_BOLD_UPPER_HKGOQ,
	(unsigned char)239 /* O accent	*/, TIMESNR_BOLD_UPPER_HKGOQ,
	//(unsigned char)240 /* space		*/, 0,
	(unsigned char)241 /* O grave		*/, TIMESNR_BOLD_UPPER_HKGOQ,
	(unsigned char)242 /* U acute		*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)243 /* U accent	*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)244 /* U grave		*/, TIMESNR_BOLD_UPPER_ACDNRUVXY,
	(unsigned char)245 /* / slash		*/, FONT_SIZE_02778,
	(unsigned char)246 /* ^ accent	*/, FONT_SIZE_03333,
	(unsigned char)247 /* ~ squiggle	*/, FONT_SIZE_03333,
	(unsigned char)248 /* - high dash	*/, FONT_SIZE_03333,
	(unsigned char)249 /* high u		*/, FONT_SIZE_03333,
	(unsigned char)250 /* high dot	*/, FONT_SIZE_03333,
	(unsigned char)251 /* high circle	*/, FONT_SIZE_03333,
	(unsigned char)252 /* curl left	*/, FONT_SIZE_03333,
	(unsigned char)253 /* acute		*/, FONT_SIZE_03333,
	(unsigned char)254 /* curl right	*/, FONT_SIZE_03333,
	(unsigned char)255 /* accent inv	*/, FONT_SIZE_03333,

	// English letters and symbols
	'a', TIMESNR_BOLD_LOWER_AVXY,
	'b', TIMESNR_BOLD_LOWER_BDHKNPQU,
	'c', TIMESNR_BOLD_LOWER_CERZ,
	'd', TIMESNR_BOLD_LOWER_BDHKNPQU,
	'e', TIMESNR_BOLD_LOWER_CERZ,
	'f', TIMESNR_BOLD_LOWER_FJT,
	'g', TIMESNR_BOLD_LOWER_GO,
	'h', TIMESNR_BOLD_LOWER_BDHKNPQU,
	'i', TIMESNR_BOLD_LOWER_IL,
	'j', TIMESNR_BOLD_LOWER_FJT,
	'k', TIMESNR_BOLD_LOWER_BDHKNPQU,
	'l', TIMESNR_BOLD_LOWER_IL,
	'm', TIMESNR_BOLD_LOWER_M,
	'n', TIMESNR_BOLD_LOWER_BDHKNPQU,
	'o', TIMESNR_BOLD_LOWER_GO,
	'p', TIMESNR_BOLD_LOWER_BDHKNPQU,
	'q', TIMESNR_BOLD_LOWER_BDHKNPQU,
	'r', TIMESNR_BOLD_LOWER_CERZ,
	's', TIMESNR_BOLD_LOWER_S,
	't', TIMESNR_BOLD_LOWER_FJT,
	'u', TIMESNR_BOLD_LOWER_BDHKNPQU,
	'v', TIMESNR_BOLD_LOWER_AVXY,
	'w', TIMESNR_BOLD_LOWER_W,
	'x', TIMESNR_BOLD_LOWER_AVXY,
	'y', TIMESNR_BOLD_LOWER_AVXY,
	'z', TIMESNR_BOLD_LOWER_CERZ,
	'A', TIMESNR_BOLD_UPPER_ACDNRUVXY,
	'B', TIMESNR_BOLD_UPPER_BELTZ,
	'C', TIMESNR_BOLD_UPPER_ACDNRUVXY,
	'D', TIMESNR_BOLD_UPPER_ACDNRUVXY,
	'E', TIMESNR_BOLD_UPPER_BELTZ,
	'F', TIMESNR_BOLD_UPPER_FP,
	'G', TIMESNR_BOLD_UPPER_HKGOQ,
	'H', TIMESNR_BOLD_UPPER_HKGOQ,
	'I', TIMESNR_BOLD_UPPER_I,
	'J', TIMESNR_BOLD_UPPER_J,
	'K', TIMESNR_BOLD_UPPER_HKGOQ,
	'L', TIMESNR_BOLD_UPPER_BELTZ,
	'M', TIMESNR_BOLD_UPPER_M,
	'N', TIMESNR_BOLD_UPPER_ACDNRUVXY,
	'O', TIMESNR_BOLD_UPPER_HKGOQ,
	'P', TIMESNR_BOLD_UPPER_FP,
	'Q', TIMESNR_BOLD_UPPER_HKGOQ,
	'R', TIMESNR_BOLD_UPPER_ACDNRUVXY,
	'S', TIMESNR_BOLD_UPPER_S,
	'T', TIMESNR_BOLD_UPPER_BELTZ,
	'U', TIMESNR_BOLD_UPPER_ACDNRUVXY,
	'V', TIMESNR_BOLD_UPPER_ACDNRUVXY,
	'W', TIMESNR_BOLD_UPPER_W,
	'X', TIMESNR_BOLD_UPPER_ACDNRUVXY,
	'Y', TIMESNR_BOLD_UPPER_ACDNRUVXY,
	'Z', TIMESNR_BOLD_UPPER_BELTZ,
	'0', TIMESNR_BOLD_NUMBERS,
	'1', TIMESNR_BOLD_NUMBERS,
	'2', TIMESNR_BOLD_NUMBERS,
	'3', TIMESNR_BOLD_NUMBERS,
	'4', TIMESNR_BOLD_NUMBERS,
	'5', TIMESNR_BOLD_NUMBERS,
	'6', TIMESNR_BOLD_NUMBERS,
	'7', TIMESNR_BOLD_NUMBERS,
	'8', TIMESNR_BOLD_NUMBERS,
	'9', TIMESNR_BOLD_NUMBERS,
	'|', TIMESNR_BOLD_BAR,
	',', TIMESNR_BOLD_COMMADOT_SPACE,
	'.', TIMESNR_BOLD_COMMADOT_SPACE,
	'\\',TIMESNR_BOLD_SLASHES,
	'/', TIMESNR_BOLD_SLASHES,
	':', TIMESNR_BOLD_SQUAREBRACES_ANDMORE,
	';', TIMESNR_BOLD_SQUAREBRACES_ANDMORE,
	' ', TIMESNR_BOLD_COMMADOT_SPACE,
	'[', TIMESNR_BOLD_SQUAREBRACES_ANDMORE,
	']', TIMESNR_BOLD_SQUAREBRACES_ANDMORE,
	'`', TIMESNR_BOLD_SQUAREBRACES_ANDMORE,
	'-', TIMESNR_BOLD_SQUAREBRACES_ANDMORE,
	'(', TIMESNR_BOLD_SQUAREBRACES_ANDMORE,
	')', TIMESNR_BOLD_SQUAREBRACES_ANDMORE,
	'!', TIMESNR_BOLD_SQUAREBRACES_ANDMORE,
	'?', TIMESNR_BOLD_APPIONMARK,
	'"', TIMESNR_BOLD_QUOTES,
	'\'',TIMESNR_BOLD_SINGLEQUOTE,
	'^', TIMESNR_BOLD_CIRCUMFLEX,
	'{', TIMESNR_BOLD_CURLYBRACES,
	'}', TIMESNR_BOLD_CURLYBRACES,
	'_', TIMESNR_BOLD_HASH_ANDMORE,
	'#', TIMESNR_BOLD_HASH_ANDMORE,
	'$', TIMESNR_BOLD_HASH_ANDMORE,
	'*', TIMESNR_BOLD_STAR,
	'~', TIMESNR_BOLD_SQUIGGLE,
	'=', TIMESNR_BOLD_EQUALSIGN_ANDMORE,
	'+', TIMESNR_BOLD_EQUALSIGN_ANDMORE,
	'<', TIMESNR_BOLD_EQUALSIGN_ANDMORE,
	'>', TIMESNR_BOLD_EQUALSIGN_ANDMORE,
	'&', TIMESNR_BOLD_AMPERSAND,
	'%', TIMESNR_BOLD_PERCENT,
	'@', TIMESNR_BOLD_ATSYMBOL,
	0, 0
	};

	//SetCharCodeList( charWidths, &list );
	SetBoldCharWidths( charWidths );
}


void PDFTimesNewRomanFont::InitItalicCharWidths()
{
	const double TIMESNR_ITALIC_LOWER_ABDGHNOPQU = 0.5;
	const double TIMESNR_ITALIC_LOWER_CEKVXY = 0.444;
	const double TIMESNR_ITALIC_LOWER_FIJLT = 0.2778;
	const double TIMESNR_ITALIC_LOWER_M = 0.7225;
	const double TIMESNR_ITALIC_LOWER_RSZ  = 0.3888;
	const double TIMESNR_ITALIC_LOWER_W = 0.6667;

	const double TIMESNR_ITALIC_UPPER_ABEFPRVX = 0.6098;
	const double TIMESNR_ITALIC_UPPER_CKN = 0.6667;
	const double TIMESNR_ITALIC_UPPER_DGHOQU = 0.7225;
	const double TIMESNR_ITALIC_UPPER_I = 0.3333;
	const double TIMESNR_ITALIC_UPPER_J = 0.444;
	const double TIMESNR_ITALIC_UPPER_LTYZ = 0.5556;
	const double TIMESNR_ITALIC_UPPER_MW = 0.8333;
	const double TIMESNR_ITALIC_UPPER_S = 0.5;

	const double TIMESNR_ITALIC_NUMBERS = TIMESNR_ITALIC_LOWER_ABDGHNOPQU;

	const double TIMESNR_ITALIC_BAR /* | */ = FONT_SIZE_02750;
	const double TIMESNR_ITALIC_COMMADOT_SPACE /* ,.spacechar */ = 0.25;
	const double TIMESNR_ITALIC_FORWARDSLASH /* \ */ = FONT_SIZE_02778;
	const double TIMESNR_ITALIC_BACKSLASH /* / */ = FONT_SIZE_02784;
	const double TIMESNR_ITALIC_ROUNDBRACES_ANDMORE /* :;`-()! */ = FONT_SIZE_03333;
	const double TIMESNR_ITALIC_SQUAREBRACES /* [] */ = FONT_SIZE_03888;
	const double TIMESNR_ITALIC_APPIONMARK /* ? */ = FONT_SIZE_05000;
	const double TIMESNR_ITALIC_QUOTES /* " */ = 0.4237;//0.3556;
	const double TIMESNR_ITALIC_SINGLEQUOTE /* ' */ = FONT_SIZE_02144;//50.0/260.0;//0.1914;
	const double TIMESNR_ITALIC_CIRCUMFLEX /* ^ */ = 0.4223;
	const double TIMESNR_ITALIC_CURLYBRACES /* {} */ = FONT_SIZE_04000;
	const double TIMESNR_ITALIC_HASH_ANDMORE /* _#$ */ = FONT_SIZE_05000;
	const double TIMESNR_ITALIC_STAR /* * */ = FONT_SIZE_05000;
	const double TIMESNR_ITALIC_SQUIGGLE /* ~ */ = 0.5411;
	const double TIMESNR_ITALIC_EQUALSIGN_ANDMORE /* =+<> */ = 0.6757;
	const double TIMESNR_ITALIC_AMPERSAND /* & */ = FONT_SIZE_07788;
	const double TIMESNR_ITALIC_PERCENT /* % */ = FONT_SIZE_08333;
	const double TIMESNR_ITALIC_ATSYMBOL /* @ */ = 0.9225;

	CharOnePointSizeList list;

	CharWidths charWidths[] = {
	(unsigned char)0347, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Aacute
	(unsigned char)0345, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Acirc
	(unsigned char)0313, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Agrave
	(unsigned char)0256, TIMESNR_ITALIC_UPPER_ABEFPRVX /*????*/,	// AElig
	(unsigned char)0201, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Aring
	(unsigned char)0314, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Atilde
	(unsigned char)0200, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Auml
	(unsigned char)0202, TIMESNR_ITALIC_UPPER_CKN /*????*/,	// Ccedil
	(unsigned char)0056, TIMESNR_ITALIC_COMMADOT_SPACE /*????*/,	// Dot 
	(unsigned char)0203, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Eacute
	(unsigned char)0346, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Ecirc
	(unsigned char)0351, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Egrave
	(unsigned char)0350, TIMESNR_ITALIC_UPPER_ABEFPRVX,	// Euml
	(unsigned char)0352, TIMESNR_ITALIC_UPPER_I,	// Iacute
	(unsigned char)0353, TIMESNR_ITALIC_UPPER_I,	// Icirc
	(unsigned char)0355, TIMESNR_ITALIC_UPPER_I,	// Igrave
	(unsigned char)0354, TIMESNR_ITALIC_UPPER_I,	// Iuml
	(unsigned char)0204, TIMESNR_ITALIC_UPPER_CKN,	// Ntilde
	(unsigned char)0356, TIMESNR_ITALIC_UPPER_DGHOQU,	// Oacute  
	(unsigned char)0357, TIMESNR_ITALIC_UPPER_DGHOQU,	// Ocirc
	(unsigned char)0361, TIMESNR_ITALIC_UPPER_DGHOQU,	// Ograve
	(unsigned char)0257, TIMESNR_ITALIC_UPPER_DGHOQU,	// Oslash
	(unsigned char)0315, TIMESNR_ITALIC_UPPER_DGHOQU,	// Otilde
	(unsigned char)0235, TIMESNR_ITALIC_UPPER_DGHOQU,	// Ouml
	(unsigned char)0120, TIMESNR_ITALIC_UPPER_ABEFPRVX /*????*/,	// THORN, becomes 'standard P'
	(unsigned char)0362, TIMESNR_ITALIC_UPPER_DGHOQU,	// Uacute
	(unsigned char)0363, TIMESNR_ITALIC_UPPER_DGHOQU,	// Ucirc
	(unsigned char)0364, TIMESNR_ITALIC_UPPER_DGHOQU,	// Ugrave
	(unsigned char)0206, TIMESNR_ITALIC_UPPER_DGHOQU,	// Uuml
	(unsigned char)0131, TIMESNR_ITALIC_UPPER_LTYZ,	// Yacute, becomes 'standard Y'
	(unsigned char)0207, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// aacute
	(unsigned char)0235, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// acute
	(unsigned char)0235, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// acirc
	(unsigned char)0276, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// aelig
	(unsigned char)0210, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// agrave
	(unsigned char)0214, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// aring
	(unsigned char)0235, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// atilde
	(unsigned char)0212, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// auml
	(unsigned char)0174, TIMESNR_ITALIC_BAR,	// brvbar, becomes standard bar '|'
	(unsigned char)0215, TIMESNR_ITALIC_LOWER_CEKVXY,	// ccedil
	(unsigned char)0374, TIMESNR_ITALIC_LOWER_CEKVXY,	// cedil
	(unsigned char)0242, TIMESNR_ITALIC_LOWER_CEKVXY,	// cent
	(unsigned char)0251, TIMESNR_ITALIC_ATSYMBOL,	// copy
	(unsigned char)0333, TIMESNR_ITALIC_LOWER_CEKVXY /*????*/,	// curren
	(unsigned char)0241, TIMESNR_ITALIC_LOWER_CEKVXY /*????*/,	// deg 
	(unsigned char)0216, TIMESNR_ITALIC_LOWER_CEKVXY,	// eacute 
	(unsigned char)0220, TIMESNR_ITALIC_LOWER_CEKVXY,	// ecirc 
	(unsigned char)0217, TIMESNR_ITALIC_LOWER_CEKVXY,	// egrave
	(unsigned char)0144, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// eth, problem!! becomes "d' as this is what it loks most like?
	(unsigned char)0221, TIMESNR_ITALIC_LOWER_CEKVXY,	// euml
	(unsigned char)0222, TIMESNR_ITALIC_LOWER_FIJLT,	// iacute
	(unsigned char)0224, TIMESNR_ITALIC_LOWER_FIJLT,	// icirc
	(unsigned char)0223, TIMESNR_ITALIC_LOWER_FIJLT,	// igrave
	(unsigned char)0225, TIMESNR_ITALIC_LOWER_FIJLT,	// iuml
	(unsigned char)0370, TIMESNR_ITALIC_LOWER_ABDGHNOPQU /*????*/,	// macr
	(unsigned char)0341, TIMESNR_ITALIC_COMMADOT_SPACE /*????*/,	// middot, using 'period centered'
	(unsigned char)0226, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// ntilde 
	(unsigned char)0227, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// oacute  
	(unsigned char)0231, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// ocirc  
	(unsigned char)0230, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// ograve
	(unsigned char)0273, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// ordf
	(unsigned char)0274, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// ordm
	(unsigned char)0277, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// oslash
	(unsigned char)0233, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// otilde  
	(unsigned char)0232, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// ouml
	(unsigned char)0246, TIMESNR_ITALIC_LOWER_ABDGHNOPQU /*????*/,	// para
	(unsigned char)0053, TIMESNR_ITALIC_LOWER_ABDGHNOPQU /*????*/,	// plusmn
	(unsigned char)0243, TIMESNR_ITALIC_LOWER_ABDGHNOPQU /*????*/,	// pound, bcomes 'sterling'
	(unsigned char)0250, TIMESNR_ITALIC_ATSYMBOL /*????*/,	// reg 
	(unsigned char)0244, TIMESNR_ITALIC_LOWER_ABDGHNOPQU /*????*/,	// sect
	(unsigned char)0234, TIMESNR_ITALIC_LOWER_ABDGHNOPQU /*????*/,  
	(unsigned char)0236, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// ucirc  
	(unsigned char)0235, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// ugrave  
	(unsigned char)0237, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// uml
	(unsigned char)0237, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// uuml
	(unsigned char)0264, TIMESNR_ITALIC_HASH_ANDMORE,	// yen 
	(unsigned char)0330, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// yuml 
	(unsigned char)0235, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,	// code = xxx; break325";
	(unsigned char)0252, TIMESNR_ITALIC_ATSYMBOL,	// trademark

	// Foreign letters and symbols
	(unsigned char)130 /* C cedil		*/, TIMESNR_ITALIC_UPPER_CKN,
	(unsigned char)131 /* E egrave	*/, TIMESNR_ITALIC_UPPER_ABEFPRVX,
	(unsigned char)132 /* N tilde		*/, TIMESNR_ITALIC_UPPER_CKN,
	(unsigned char)133 /* 0 umlaut	*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	(unsigned char)134 /* U umlaut	*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	(unsigned char)135 /* a acute		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)136 /* a grave		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)137 /* a circum	*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)138 /* a umlaut	*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)139 /* a tilde		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)140 /* a ring		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)141 /* c cedil		*/, TIMESNR_ITALIC_LOWER_CEKVXY,
	(unsigned char)142 /* e acute		*/, TIMESNR_ITALIC_LOWER_CEKVXY,
	(unsigned char)143 /* e grave		*/, TIMESNR_ITALIC_LOWER_CEKVXY,
	(unsigned char)144 /* e circum	*/, TIMESNR_ITALIC_LOWER_CEKVXY,
	(unsigned char)145 /* e umlaut	*/, TIMESNR_ITALIC_LOWER_CEKVXY,
	(unsigned char)146 /* l acute		*/, TIMESNR_ITALIC_LOWER_FIJLT /* for foreign i */,
	(unsigned char)147 /* l grave		*/, TIMESNR_ITALIC_LOWER_FIJLT /* for foreign i */,
	(unsigned char)148 /* l circum	*/, TIMESNR_ITALIC_LOWER_FIJLT /* for foreign i */,
	(unsigned char)149 /* l umlaut	*/, TIMESNR_ITALIC_LOWER_FIJLT /* for foreign i */,
	(unsigned char)150 /* n titde		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)151 /* o acute		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)152 /* o grave		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)153 /* o circum	*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)154 /* o umlaut	*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)155 /* o titde		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)156 /* u acute		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)157 /* u grave		*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)158 /* u circum	*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)159 /* u umlaut	*/, TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	(unsigned char)160 /* cross		*/, FONT_SIZE_05000,
	(unsigned char)161 /* ring		*/, FONT_SIZE_04000,
	(unsigned char)162 /* cent		*/, FONT_SIZE_05000,
	(unsigned char)163 /* pound		*/, FONT_SIZE_05000,
	(unsigned char)164 /* double S	*/, FONT_SIZE_05000,
	(unsigned char)165 /* dot			*/, FONT_SIZE_03501,
	(unsigned char)166 /* para end	*/, FONT_SIZE_05236,
	(unsigned char)167 /* german SS	*/, FONT_SIZE_05000,
	(unsigned char)168 /* register	*/, FONT_SIZE_07599,
	(unsigned char)169 /* copyright	*/, FONT_SIZE_07599,
	(unsigned char)170 /* trademark	*/, FONT_SIZE_09804,
	(unsigned char)171 /* apostrophe	*/, FONT_SIZE_03333,
	(unsigned char)172 /* umlaut		*/, FONT_SIZE_03333,
	//(unsigned char)173 /* space		*/, 0,
	(unsigned char)174 /* AE ligature	*/, FONT_SIZE_08897,
	(unsigned char)175 /* O slash		*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	//(unsigned char)176 /* space		*/, 0,
	(unsigned char)177 /* plusminus	*/, FONT_SIZE_06757,
	//(unsigned char)178 /* space		*/, 0,
	//(unsigned char)179 /* space		*/, 0,
	(unsigned char)180 /* yen			*/, FONT_SIZE_05000,
	(unsigned char)181 /* u long stem	*/, FONT_SIZE_05144,
	//(unsigned char)182 /* space		*/, 0,
	//(unsigned char)183 /* space		*/, 0,
	//(unsigned char)184 /* space		*/, 0,
	//(unsigned char)185 /* space		*/, 0,
	//(unsigned char)186 /* space		*/, 0,
	(unsigned char)187 /* a little	*/, FONT_SIZE_02762,
	(unsigned char)188 /* o little	*/, FONT_SIZE_03106,
	//(unsigned char)189 /* space		*/, 0,
	(unsigned char)190 /* ae ligature	*/, FONT_SIZE_06667,
	(unsigned char)191 /* o slash		*/, FONT_SIZE_05000,
	(unsigned char)192 /* ? inverted	*/, FONT_SIZE_05000,
	(unsigned char)193 /* ! inverted	*/, FONT_SIZE_03888,
	(unsigned char)194 /* right angle	*/, FONT_SIZE_06757,
	//(unsigned char)195 /* space		*/, 0,
	(unsigned char)196 /* f fancy		*/, FONT_SIZE_05000,
	//(unsigned char)197 /* space		*/, 0,
	//(unsigned char)198 /* space		*/, 0,
	(unsigned char)199 /* <<			*/, FONT_SIZE_05000,
	(unsigned char)200 /* >>			*/, FONT_SIZE_05000,
	(unsigned char)201 /* ... ellipse	*/, FONT_SIZE_08897,
	//(unsigned char)202 /* space		*/, 0,
	(unsigned char)203 /* A grave		*/, TIMESNR_ITALIC_UPPER_ABEFPRVX,
	(unsigned char)204 /* A tilde		*/, TIMESNR_ITALIC_UPPER_ABEFPRVX,
	(unsigned char)205 /* O tilde		*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	(unsigned char)206 /* OE ligature	*/, FONT_SIZE_09434,
	(unsigned char)207 /* oe ligature	*/, FONT_SIZE_06667,
	(unsigned char)208 /* underscore	*/, FONT_SIZE_05000,
	(unsigned char)209 /* long under	*/, FONT_SIZE_08897,
	(unsigned char)210 /* dquotesopen	*/, FONT_SIZE_05556,
	(unsigned char)211 /* dquotesclose*/, FONT_SIZE_05556,
	(unsigned char)212 /* squotesopen	*/, FONT_SIZE_03333,
	(unsigned char)213 /* squotesclose*/, FONT_SIZE_03333,
	(unsigned char)214 /* divide sign	*/, FONT_SIZE_06757,
	//(unsigned char)215 /* space		*/, 0,
	(unsigned char)216 /* y umlaut	*/, TIMESNR_ITALIC_LOWER_CEKVXY,
	(unsigned char)217 /* Y umlaut	*/, TIMESNR_ITALIC_UPPER_LTYZ,
	(unsigned char)218 /* / 45d slash	*/, FONT_SIZE_01667,
	(unsigned char)219 /* circlestar	*/, FONT_SIZE_05000,
	(unsigned char)220 /* <			*/, FONT_SIZE_03333,
	(unsigned char)221 /* >			*/, FONT_SIZE_03333,
	(unsigned char)222 /* fi ligature */, FONT_SIZE_05000,
	(unsigned char)223 /* fl ligature	*/, FONT_SIZE_05000,
	(unsigned char)224 /* bar 2 slash	*/, FONT_SIZE_05000,
	(unsigned char)225 /* . dot		*/, FONT_SIZE_02500,
	(unsigned char)226 /* , comma		*/, FONT_SIZE_03333,
	(unsigned char)227 /* ,, 2 comma	*/, FONT_SIZE_05556,
	(unsigned char)228 /* % with o	*/, FONT_SIZE_10000,
	(unsigned char)229 /* A accent	*/, TIMESNR_ITALIC_UPPER_ABEFPRVX,
	(unsigned char)230 /* E accent	*/, TIMESNR_ITALIC_UPPER_ABEFPRVX,
	(unsigned char)231 /* A acute		*/, TIMESNR_ITALIC_UPPER_ABEFPRVX,
	(unsigned char)232 /* E umlaut	*/, TIMESNR_ITALIC_UPPER_ABEFPRVX,
	(unsigned char)233 /* E grave		*/, TIMESNR_ITALIC_UPPER_ABEFPRVX,
	(unsigned char)234 /* I acute		*/, FONT_SIZE_03356,
	(unsigned char)235 /* I accent	*/, FONT_SIZE_03356,
	(unsigned char)236 /* I umlaut	*/, FONT_SIZE_03356,
	(unsigned char)237 /* I grave		*/, FONT_SIZE_03356,
	(unsigned char)238 /* O acute		*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	(unsigned char)239 /* O accent	*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	//(unsigned char)240 /* space		*/, 0,
	(unsigned char)241 /* O grave		*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	(unsigned char)242 /* U acute		*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	(unsigned char)243 /* U accent	*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	(unsigned char)244 /* U grave		*/, TIMESNR_ITALIC_UPPER_DGHOQU,
	(unsigned char)245 /* / slash		*/, FONT_SIZE_02778,
	(unsigned char)246 /* ^ accent	*/, FONT_SIZE_03333,
	(unsigned char)247 /* ~ squiggle	*/, FONT_SIZE_03333,
	(unsigned char)248 /* - high dash	*/, FONT_SIZE_03333,
	(unsigned char)249 /* high u		*/, FONT_SIZE_03333,
	(unsigned char)250 /* high dot	*/, FONT_SIZE_03333,
	(unsigned char)251 /* high circle	*/, FONT_SIZE_03333,
	(unsigned char)252 /* curl left	*/, FONT_SIZE_03333,
	(unsigned char)253 /* acute		*/, FONT_SIZE_03333,
	(unsigned char)254 /* curl right	*/, FONT_SIZE_03333,
	(unsigned char)255 /* accent inv	*/, FONT_SIZE_03333,

	// English letters and symbols
	'a', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'b', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'c', TIMESNR_ITALIC_LOWER_CEKVXY,
	'd', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'e', TIMESNR_ITALIC_LOWER_CEKVXY,
	'f', TIMESNR_ITALIC_LOWER_FIJLT,
	'g', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'h', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'i', TIMESNR_ITALIC_LOWER_FIJLT,
	'j', TIMESNR_ITALIC_LOWER_FIJLT,
	'k', TIMESNR_ITALIC_LOWER_CEKVXY,
	'l', TIMESNR_ITALIC_LOWER_FIJLT,
	'm', TIMESNR_ITALIC_LOWER_M,
	'n', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'o', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'p', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'q', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'r', TIMESNR_ITALIC_LOWER_RSZ,
	's', TIMESNR_ITALIC_LOWER_RSZ,
	't', TIMESNR_ITALIC_LOWER_FIJLT,
	'u', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'v', TIMESNR_ITALIC_LOWER_CEKVXY,
	'w', TIMESNR_ITALIC_LOWER_W,
	'x', TIMESNR_ITALIC_LOWER_CEKVXY,
	'y', TIMESNR_ITALIC_LOWER_CEKVXY,
	'z', TIMESNR_ITALIC_LOWER_RSZ,
	'A', TIMESNR_ITALIC_UPPER_ABEFPRVX,
	'B', TIMESNR_ITALIC_UPPER_ABEFPRVX,
	'C', TIMESNR_ITALIC_UPPER_CKN,
	'D', TIMESNR_ITALIC_UPPER_DGHOQU,
	'E', TIMESNR_ITALIC_UPPER_ABEFPRVX,
	'F', TIMESNR_ITALIC_UPPER_ABEFPRVX,
	'G', TIMESNR_ITALIC_UPPER_DGHOQU,
	'H', TIMESNR_ITALIC_UPPER_DGHOQU,
	'I', TIMESNR_ITALIC_UPPER_I,
	'J', TIMESNR_ITALIC_UPPER_J,
	'K', TIMESNR_ITALIC_UPPER_CKN,
	'L', TIMESNR_ITALIC_UPPER_LTYZ,
	'M', TIMESNR_ITALIC_UPPER_MW,
	'N', TIMESNR_ITALIC_UPPER_CKN,
	'O', TIMESNR_ITALIC_UPPER_DGHOQU,
	'P', TIMESNR_ITALIC_UPPER_ABEFPRVX,
	'Q', TIMESNR_ITALIC_UPPER_DGHOQU,
	'R', TIMESNR_ITALIC_UPPER_ABEFPRVX,
	'S', TIMESNR_ITALIC_UPPER_S,
	'T', TIMESNR_ITALIC_UPPER_LTYZ,
	'U', TIMESNR_ITALIC_UPPER_DGHOQU,
	'V', TIMESNR_ITALIC_UPPER_ABEFPRVX,
	'W', TIMESNR_ITALIC_UPPER_MW,
	'X', TIMESNR_ITALIC_UPPER_ABEFPRVX,
	'Y', TIMESNR_ITALIC_UPPER_LTYZ,
	'Z', TIMESNR_ITALIC_UPPER_LTYZ,
	'0', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'1', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'2', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'3', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'4', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'5', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'6', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'7', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'8', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'9', TIMESNR_ITALIC_LOWER_ABDGHNOPQU,
	'|', TIMESNR_ITALIC_BAR,
	',', TIMESNR_ITALIC_COMMADOT_SPACE,
	'.', TIMESNR_ITALIC_COMMADOT_SPACE,
	'\\',TIMESNR_ITALIC_FORWARDSLASH,
	'/', TIMESNR_ITALIC_BACKSLASH,
	':', TIMESNR_ITALIC_ROUNDBRACES_ANDMORE,
	';', TIMESNR_ITALIC_ROUNDBRACES_ANDMORE,
	' ', TIMESNR_ITALIC_COMMADOT_SPACE,
	'[', TIMESNR_ITALIC_SQUAREBRACES,
	']', TIMESNR_ITALIC_SQUAREBRACES,
	'`', TIMESNR_ITALIC_ROUNDBRACES_ANDMORE,
	'-', TIMESNR_ITALIC_ROUNDBRACES_ANDMORE,
	'(', TIMESNR_ITALIC_ROUNDBRACES_ANDMORE,
	')', TIMESNR_ITALIC_ROUNDBRACES_ANDMORE,
	'!', TIMESNR_ITALIC_ROUNDBRACES_ANDMORE,
	'?', TIMESNR_ITALIC_APPIONMARK,
	'"', TIMESNR_ITALIC_QUOTES,
	'\'',TIMESNR_ITALIC_SINGLEQUOTE,
	'^', TIMESNR_ITALIC_CIRCUMFLEX,
	'{', TIMESNR_ITALIC_CURLYBRACES,
	'}', TIMESNR_ITALIC_CURLYBRACES,
	'_', TIMESNR_ITALIC_HASH_ANDMORE,
	'#', TIMESNR_ITALIC_HASH_ANDMORE,
	'$', TIMESNR_ITALIC_HASH_ANDMORE,
	'*', TIMESNR_ITALIC_STAR,
	'~', TIMESNR_ITALIC_SQUIGGLE,
	'=', TIMESNR_ITALIC_EQUALSIGN_ANDMORE,
	'+', TIMESNR_ITALIC_EQUALSIGN_ANDMORE,
	'<', TIMESNR_ITALIC_EQUALSIGN_ANDMORE,
	'>', TIMESNR_ITALIC_EQUALSIGN_ANDMORE,
	'&', TIMESNR_ITALIC_AMPERSAND,
	'%', TIMESNR_ITALIC_PERCENT,
	'@', TIMESNR_ITALIC_ATSYMBOL,
	0, 0
	};

	//SetCharCodeList( charWidths, &list );
	SetItalicCharWidths( charWidths );
}


void PDFTimesNewRomanFont::InitBoldItalicCharWidths()
{
	const double TIMESNR_BOLDITALIC_LOWER_ABDGOPQ = 0.5;
	const double TIMESNR_BOLDITALIC_LOWER_CEVY = 0.444;
	const double TIMESNR_BOLDITALIC_LOWER_F = 0.3333;
	const double TIMESNR_BOLDITALIC_LOWER_HNU = 0.5556;
	const double TIMESNR_BOLDITALIC_LOWER_IJLT = 0.2778;
	const double TIMESNR_BOLDITALIC_LOWER_KX = 0.5;
	const double TIMESNR_BOLDITALIC_LOWER_RZ = 0.3888;
	const double TIMESNR_BOLDITALIC_LOWER_M = 0.7788;
	const double TIMESNR_BOLDITALIC_LOWER_S  = 0.3888;
	const double TIMESNR_BOLDITALIC_LOWER_W = 0.6667;

	const double TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX = 0.6667;
	const double TIMESNR_BOLDITALIC_UPPER_DGNOQU = 0.7225;
	const double TIMESNR_BOLDITALIC_UPPER_H = FONT_SIZE_07788;
	const double TIMESNR_BOLDITALIC_UPPER_I = 0.3894;
	const double TIMESNR_BOLDITALIC_UPPER_J = 0.5;
	const double TIMESNR_BOLDITALIC_UPPER_MW = 0.8897;
	const double TIMESNR_BOLDITALIC_UPPER_LPTYZ = 0.6098;
	const double TIMESNR_BOLDITALIC_UPPER_S = 0.5556;

	const double TIMESNR_BOLDITALIC_NUMBERS = 0.5;

	const double TIMESNR_BOLDITALIC_BAR /* | */ = 0.2203;
	const double TIMESNR_BOLDITALIC_COMMADOT_SPACE /* ,.spacechar */ = 0.25;
	const double TIMESNR_BOLDITALIC_SLASHES /* \/ */ = 0.2778;
	const double TIMESNR_BOLDITALIC_SQUAREBRACES_ANDMORE /* []:;`-() */ = 0.3333;
	const double TIMESNR_BOLDITALIC_EXCLAMATION /* ! */ = FONT_SIZE_03888;
	const double TIMESNR_BOLDITALIC_APPIONMARK /* ? */ = 0.5;
	const double TIMESNR_BOLDITALIC_QUOTES /* " */ = FONT_SIZE_05556;
	const double TIMESNR_BOLDITALIC_SINGLEQUOTE /* ' */ = FONT_SIZE_02784;
	const double TIMESNR_BOLDITALIC_CIRCUMFLEX /* ^ */ = 0.5698;
	const double TIMESNR_BOLDITALIC_CURLYBRACES /* {} */ = FONT_SIZE_03484;
	const double TIMESNR_BOLDITALIC_STAR /* * */ = 0.5;
	const double TIMESNR_BOLDITALIC_HASH_ANDMORE /* _#$ */ = 0.5;
	const double TIMESNR_BOLDITALIC_EQUALSIGN_ANDMORE /* =+<>~ */ = FONT_SIZE_05698;
	const double TIMESNR_BOLDITALIC_AMPERSAND /* & */ = FONT_SIZE_07788;
	const double TIMESNR_BOLDITALIC_PERCENTATSYMBOL /* %@ */ = 0.8333;

	CharOnePointSizeList list;

	CharWidths charWidths[] = {
	(unsigned char)0347, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Aacute
	(unsigned char)0345, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Acirc
	(unsigned char)0313, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Agrave
	(unsigned char)0256, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX /*????*/,	// AElig
	(unsigned char)0201, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Aring
	(unsigned char)0314, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Atilde
	(unsigned char)0200, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Auml
	(unsigned char)0202, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX /*????*/,	// Ccedil
	(unsigned char)0056, TIMESNR_BOLDITALIC_COMMADOT_SPACE /*????*/,	// Dot 
	(unsigned char)0203, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Eacute
	(unsigned char)0346, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Ecirc
	(unsigned char)0351, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Egrave
	(unsigned char)0350, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,	// Euml
	(unsigned char)0352, TIMESNR_BOLDITALIC_UPPER_I,	// Iacute
	(unsigned char)0353, TIMESNR_BOLDITALIC_UPPER_I,	// Icirc
	(unsigned char)0355, TIMESNR_BOLDITALIC_UPPER_I,	// Igrave
	(unsigned char)0354, TIMESNR_BOLDITALIC_UPPER_I,	// Iuml
	(unsigned char)0204, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Ntilde
	(unsigned char)0356, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Oacute  
	(unsigned char)0357, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Ocirc
	(unsigned char)0361, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Ograve
	(unsigned char)0257, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Oslash
	(unsigned char)0315, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Otilde
	(unsigned char)0235, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Ouml
	(unsigned char)0120, TIMESNR_BOLDITALIC_UPPER_LPTYZ /*????*/,	// THORN, becomes 'standard P'
	(unsigned char)0362, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Uacute
	(unsigned char)0363, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Ucirc
	(unsigned char)0364, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Ugrave
	(unsigned char)0206, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Uuml
	(unsigned char)0131, TIMESNR_BOLDITALIC_UPPER_DGNOQU,	// Yacute, becomes 'standard Y'
	(unsigned char)0207, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// aacute
	(unsigned char)0235, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// acute
	(unsigned char)0235, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// acirc
	(unsigned char)0276, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// aelig
	(unsigned char)0210, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// agrave
	(unsigned char)0214, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// aring
	(unsigned char)0235, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// atilde
	(unsigned char)0212, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// auml
	(unsigned char)0174, TIMESNR_BOLDITALIC_BAR,	// brvbar, becomes standard bar '|'
	(unsigned char)0215, TIMESNR_BOLDITALIC_LOWER_CEVY,	// ccedil
	(unsigned char)0374, TIMESNR_BOLDITALIC_LOWER_CEVY,	// cedil
	(unsigned char)0242, TIMESNR_BOLDITALIC_LOWER_CEVY,	// cent
	(unsigned char)0251, TIMESNR_BOLDITALIC_PERCENTATSYMBOL,	// copy
	(unsigned char)0333, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ /*????*/,	// curren
	(unsigned char)0241, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ /*????*/,	// deg 
	(unsigned char)0216, TIMESNR_BOLDITALIC_LOWER_CEVY,	// eacute 
	(unsigned char)0220, TIMESNR_BOLDITALIC_LOWER_CEVY,	// ecirc 
	(unsigned char)0217, TIMESNR_BOLDITALIC_LOWER_CEVY,	// egrave
	(unsigned char)0144, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// eth, problem!! becomes "d' as this is what it loks most like?
	(unsigned char)0221, TIMESNR_BOLDITALIC_LOWER_CEVY,	// euml
	(unsigned char)0222, TIMESNR_BOLDITALIC_LOWER_IJLT,	// iacute
	(unsigned char)0224, TIMESNR_BOLDITALIC_LOWER_IJLT,	// icirc
	(unsigned char)0223, TIMESNR_BOLDITALIC_LOWER_IJLT,	// igrave
	(unsigned char)0225, TIMESNR_BOLDITALIC_LOWER_IJLT,	// iuml
	(unsigned char)0370, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ /*????*/,	// macr
	(unsigned char)0341, TIMESNR_BOLDITALIC_COMMADOT_SPACE /*????*/,	// middot, using 'period centered'
	(unsigned char)0226, TIMESNR_BOLDITALIC_LOWER_HNU,	// ntilde 
	(unsigned char)0227, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// oacute  
	(unsigned char)0231, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// ocirc  
	(unsigned char)0230, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// ograve
	(unsigned char)0273, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// ordf
	(unsigned char)0274, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// ordm
	(unsigned char)0277, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// oslash
	(unsigned char)0233, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// otilde  
	(unsigned char)0232, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// ouml
	(unsigned char)0246, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ /*????*/,	// para
	(unsigned char)0053, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ /*????*/,	// plusmn
	(unsigned char)0243, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ /*????*/,	// pound, bcomes 'sterling'
	(unsigned char)0250, TIMESNR_BOLDITALIC_PERCENTATSYMBOL /*????*/,	// reg 
	(unsigned char)0244, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ /*????*/,	// sect
	(unsigned char)0234, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ /*????*/,  
	(unsigned char)0236, TIMESNR_BOLDITALIC_LOWER_HNU,	// ucirc  
	(unsigned char)0235, TIMESNR_BOLDITALIC_LOWER_HNU,	// ugrave  
	(unsigned char)0237, TIMESNR_BOLDITALIC_LOWER_HNU,	// uml
	(unsigned char)0237, TIMESNR_BOLDITALIC_LOWER_HNU,	// uuml
	(unsigned char)0264, TIMESNR_BOLDITALIC_HASH_ANDMORE,	// yen 
	(unsigned char)0330, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// yuml 
	(unsigned char)0235, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,	// code = xxx; break325";
	(unsigned char)0252, TIMESNR_BOLDITALIC_PERCENTATSYMBOL,	// trademark

	// Foreign letters and symbols
	(unsigned char)130 /* C cedil		*/, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	(unsigned char)131 /* E egrave	*/, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	(unsigned char)132 /* N tilde		*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	(unsigned char)133 /* 0 umlaut	*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	(unsigned char)134 /* U umlaut	*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	(unsigned char)135 /* a acute		*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)136 /* a grave		*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)137 /* a circum	*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)138 /* a umlaut	*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)139 /* a tilde		*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)140 /* a ring		*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)141 /* c cedil		*/, TIMESNR_BOLDITALIC_LOWER_CEVY,
	(unsigned char)142 /* e acute		*/, TIMESNR_BOLDITALIC_LOWER_CEVY,
	(unsigned char)143 /* e grave		*/, TIMESNR_BOLDITALIC_LOWER_CEVY,
	(unsigned char)144 /* e circum	*/, TIMESNR_BOLDITALIC_LOWER_CEVY,
	(unsigned char)145 /* e umlaut	*/, TIMESNR_BOLDITALIC_LOWER_CEVY,
	(unsigned char)146 /* l acute		*/, TIMESNR_BOLDITALIC_LOWER_IJLT /* for foreign i */,
	(unsigned char)147 /* l grave		*/, TIMESNR_BOLDITALIC_LOWER_IJLT /* for foreign i */,
	(unsigned char)148 /* l circum	*/, TIMESNR_BOLDITALIC_LOWER_IJLT /* for foreign i */,
	(unsigned char)149 /* l umlaut	*/, TIMESNR_BOLDITALIC_LOWER_IJLT /* for foreign i */,
	(unsigned char)150 /* n titde		*/, TIMESNR_BOLDITALIC_LOWER_HNU,
	(unsigned char)151 /* o acute		*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)152 /* o grave		*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)153 /* o circum	*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)154 /* o umlaut	*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)155 /* o titde		*/, TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	(unsigned char)156 /* u acute		*/, TIMESNR_BOLDITALIC_LOWER_HNU,
	(unsigned char)157 /* u grave		*/, TIMESNR_BOLDITALIC_LOWER_HNU,
	(unsigned char)158 /* u circum	*/, TIMESNR_BOLDITALIC_LOWER_HNU,
	(unsigned char)159 /* u umlaut	*/, TIMESNR_BOLDITALIC_LOWER_HNU,
	(unsigned char)160 /* cross		*/, FONT_SIZE_05000,
	(unsigned char)161 /* ring		*/, FONT_SIZE_04000,
	(unsigned char)162 /* cent		*/, FONT_SIZE_05000,
	(unsigned char)163 /* pound		*/, FONT_SIZE_05000,
	(unsigned char)164 /* double S	*/, FONT_SIZE_05000,
	(unsigned char)165 /* dot			*/, FONT_SIZE_03501,
	(unsigned char)166 /* para end	*/, FONT_SIZE_05000,
	(unsigned char)167 /* german SS	*/, FONT_SIZE_05000,
	(unsigned char)168 /* register	*/, FONT_SIZE_07463,
	(unsigned char)169 /* copyright	*/, FONT_SIZE_07463,
	(unsigned char)170 /* trademark	*/, FONT_SIZE_10000,
	(unsigned char)171 /* apostrophe	*/, FONT_SIZE_03333,
	(unsigned char)172 /* umlaut		*/, FONT_SIZE_03333,
	//(unsigned char)173 /* space		*/, 0,
	(unsigned char)174 /* AE ligature	*/, FONT_SIZE_09434,
	(unsigned char)175 /* O slash		*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	//(unsigned char)176 /* space		*/, 0,
	(unsigned char)177 /* plusminus	*/, FONT_SIZE_05708,
	//(unsigned char)178 /* space		*/, 0,
	//(unsigned char)179 /* space		*/, 0,
	(unsigned char)180 /* yen			*/, FONT_SIZE_05000,
	(unsigned char)181 /* u long stem	*/, FONT_SIZE_05319,
	//(unsigned char)182 /* space		*/, 0,
	//(unsigned char)183 /* space		*/, 0,
	//(unsigned char)184 /* space		*/, 0,
	//(unsigned char)185 /* space		*/, 0,
	//(unsigned char)186 /* space		*/, 0,
	(unsigned char)187 /* a little	*/, FONT_SIZE_02662,
	(unsigned char)188 /* o little	*/, FONT_SIZE_03005,
	//(unsigned char)189 /* space		*/, 0,
	(unsigned char)190 /* ae ligature	*/, FONT_SIZE_07225,
	(unsigned char)191 /* o slash		*/, FONT_SIZE_05000,
	(unsigned char)192 /* ? inverted	*/, FONT_SIZE_05000,
	(unsigned char)193 /* ! inverted	*/, FONT_SIZE_03888,
	(unsigned char)194 /* right angle	*/, FONT_SIZE_06068,
	//(unsigned char)195 /* space		*/, 0,
	(unsigned char)196 /* f fancy		*/, FONT_SIZE_05000,
	//(unsigned char)197 /* space		*/, 0,
	//(unsigned char)198 /* space		*/, 0,
	(unsigned char)199 /* <<			*/, FONT_SIZE_05000,
	(unsigned char)200 /* >>			*/, FONT_SIZE_05000,
	(unsigned char)201 /* ... ellipse	*/, FONT_SIZE_10000,
	//(unsigned char)202 /* space		*/, 0,
	(unsigned char)203 /* A grave		*/, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	(unsigned char)204 /* A tilde		*/, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	(unsigned char)205 /* O tilde		*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	(unsigned char)206 /* OE ligature	*/, FONT_SIZE_09434,
	(unsigned char)207 /* oe ligature	*/, FONT_SIZE_07225,
	(unsigned char)208 /* underscore	*/, FONT_SIZE_05000,
	(unsigned char)209 /* long under	*/, FONT_SIZE_10000,
	(unsigned char)210 /* dquotesopen	*/, FONT_SIZE_05000,
	(unsigned char)211 /* dquotesclose*/, FONT_SIZE_05000,
	(unsigned char)212 /* squotesopen	*/, FONT_SIZE_03333,
	(unsigned char)213 /* squotesclose*/, FONT_SIZE_03333,
	(unsigned char)214 /* divide sign	*/, FONT_SIZE_05708,
	//(unsigned char)215 /* space		*/, 0,
	(unsigned char)216 /* y umlaut	*/, TIMESNR_BOLDITALIC_LOWER_CEVY,
	(unsigned char)217 /* Y umlaut	*/, TIMESNR_BOLDITALIC_UPPER_LPTYZ,
	(unsigned char)218 /* / 45d slash	*/, FONT_SIZE_01667,
	(unsigned char)219 /* circlestar	*/, FONT_SIZE_05000,
	(unsigned char)220 /* <			*/, FONT_SIZE_03333,
	(unsigned char)221 /* >			*/, FONT_SIZE_03333,
	(unsigned char)222 /* fi ligature */, FONT_SIZE_05556,
	(unsigned char)223 /* fl ligature	*/, FONT_SIZE_05556,
	(unsigned char)224 /* bar 2 slash	*/, FONT_SIZE_05000,
	(unsigned char)225 /* . dot		*/, FONT_SIZE_02500,
	(unsigned char)226 /* , comma		*/, FONT_SIZE_03333,
	(unsigned char)227 /* ,, 2 comma	*/, FONT_SIZE_05000,
	(unsigned char)228 /* % with o	*/, FONT_SIZE_10000,
	(unsigned char)229 /* A accent	*/, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	(unsigned char)230 /* E accent	*/, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	(unsigned char)231 /* A acute		*/, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	(unsigned char)232 /* E umlaut	*/, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	(unsigned char)233 /* E grave		*/, TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	(unsigned char)234 /* I acute		*/, FONT_SIZE_03888,
	(unsigned char)235 /* I accent	*/, FONT_SIZE_03888,
	(unsigned char)236 /* I umlaut	*/, FONT_SIZE_03888,
	(unsigned char)237 /* I grave		*/, FONT_SIZE_03888,
	(unsigned char)238 /* O acute		*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	(unsigned char)239 /* O accent	*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	//(unsigned char)240 /* space		*/, 0,
	(unsigned char)241 /* O grave		*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	(unsigned char)242 /* U acute		*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	(unsigned char)243 /* U accent	*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	(unsigned char)244 /* U grave		*/, TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	(unsigned char)245 /* / slash		*/, FONT_SIZE_02778,
	(unsigned char)246 /* ^ accent	*/, FONT_SIZE_03333,
	(unsigned char)247 /* ~ squiggle	*/, FONT_SIZE_03333,
	(unsigned char)248 /* - high dash	*/, FONT_SIZE_03333,
	(unsigned char)249 /* high u		*/, FONT_SIZE_03333,
	(unsigned char)250 /* high dot	*/, FONT_SIZE_03333,
	(unsigned char)251 /* high circle	*/, FONT_SIZE_03333,
	(unsigned char)252 /* curl left	*/, FONT_SIZE_03333,
	(unsigned char)253 /* acute		*/, FONT_SIZE_03333,
	(unsigned char)254 /* curl right	*/, FONT_SIZE_03333,
	(unsigned char)255 /* accent inv	*/, FONT_SIZE_03333,

	// English letters and symbols
	'a', TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	'b', TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	'c', TIMESNR_BOLDITALIC_LOWER_CEVY,
	'd', TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	'e', TIMESNR_BOLDITALIC_LOWER_CEVY,
	'f', TIMESNR_BOLDITALIC_LOWER_F,
	'g', TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	'h', TIMESNR_BOLDITALIC_LOWER_HNU,
	'i', TIMESNR_BOLDITALIC_LOWER_IJLT,
	'j', TIMESNR_BOLDITALIC_LOWER_IJLT,
	'k', TIMESNR_BOLDITALIC_LOWER_KX,
	'l', TIMESNR_BOLDITALIC_LOWER_IJLT,
	'm', TIMESNR_BOLDITALIC_LOWER_M,
	'n', TIMESNR_BOLDITALIC_LOWER_HNU,
	'o', TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	'p', TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	'q', TIMESNR_BOLDITALIC_LOWER_ABDGOPQ,
	'r', TIMESNR_BOLDITALIC_LOWER_RZ,
	's', TIMESNR_BOLDITALIC_LOWER_S,
	't', TIMESNR_BOLDITALIC_LOWER_IJLT,
	'u', TIMESNR_BOLDITALIC_LOWER_HNU,
	'v', TIMESNR_BOLDITALIC_LOWER_CEVY,
	'w', TIMESNR_BOLDITALIC_LOWER_W,
	'x', TIMESNR_BOLDITALIC_LOWER_KX,
	'y', TIMESNR_BOLDITALIC_LOWER_CEVY,
	'z', TIMESNR_BOLDITALIC_LOWER_RZ,
	'A', TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	'B', TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	'C', TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	'D', TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	'E', TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	'F', TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	'G', TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	'H', TIMESNR_BOLDITALIC_UPPER_H,
	'I', TIMESNR_BOLDITALIC_UPPER_I,
	'J', TIMESNR_BOLDITALIC_UPPER_J,
	'K', TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	'L', TIMESNR_BOLDITALIC_UPPER_LPTYZ,
	'M', TIMESNR_BOLDITALIC_UPPER_MW,
	'N', TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	'O', TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	'P', TIMESNR_BOLDITALIC_UPPER_LPTYZ,
	'Q', TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	'R', TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	'S', TIMESNR_BOLDITALIC_UPPER_S,
	'T', TIMESNR_BOLDITALIC_UPPER_LPTYZ,
	'U', TIMESNR_BOLDITALIC_UPPER_DGNOQU,
	'V', TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	'W', TIMESNR_BOLDITALIC_UPPER_MW,
	'X', TIMESNR_BOLDITALIC_UPPER_ABCEFKRVX,
	'Y', TIMESNR_BOLDITALIC_UPPER_LPTYZ,
	'Z', TIMESNR_BOLDITALIC_UPPER_LPTYZ,
	'0', TIMESNR_BOLDITALIC_NUMBERS,
	'1', TIMESNR_BOLDITALIC_NUMBERS,
	'2', TIMESNR_BOLDITALIC_NUMBERS,
	'3', TIMESNR_BOLDITALIC_NUMBERS,
	'4', TIMESNR_BOLDITALIC_NUMBERS,
	'5', TIMESNR_BOLDITALIC_NUMBERS,
	'6', TIMESNR_BOLDITALIC_NUMBERS,
	'7', TIMESNR_BOLDITALIC_NUMBERS,
	'8', TIMESNR_BOLDITALIC_NUMBERS,
	'9', TIMESNR_BOLDITALIC_NUMBERS,
	'|', TIMESNR_BOLDITALIC_BAR,
	',', TIMESNR_BOLDITALIC_COMMADOT_SPACE,
	'.', TIMESNR_BOLDITALIC_COMMADOT_SPACE,
	'\\',TIMESNR_BOLDITALIC_SLASHES,
	'/', TIMESNR_BOLDITALIC_SLASHES,
	':', TIMESNR_BOLDITALIC_SQUAREBRACES_ANDMORE,
	';', TIMESNR_BOLDITALIC_SQUAREBRACES_ANDMORE,
	' ', TIMESNR_BOLDITALIC_COMMADOT_SPACE,
	'[', TIMESNR_BOLDITALIC_SQUAREBRACES_ANDMORE,
	']', TIMESNR_BOLDITALIC_SQUAREBRACES_ANDMORE,
	'`', TIMESNR_BOLDITALIC_SQUAREBRACES_ANDMORE,
	'-', TIMESNR_BOLDITALIC_SQUAREBRACES_ANDMORE,
	'(', TIMESNR_BOLDITALIC_SQUAREBRACES_ANDMORE,
	')', TIMESNR_BOLDITALIC_SQUAREBRACES_ANDMORE,
	'!', TIMESNR_BOLDITALIC_EXCLAMATION,
	'?', TIMESNR_BOLDITALIC_APPIONMARK,
	'"', TIMESNR_BOLDITALIC_QUOTES,
	'\'',TIMESNR_BOLDITALIC_SINGLEQUOTE,
	'^', TIMESNR_BOLDITALIC_CIRCUMFLEX,
	'{', TIMESNR_BOLDITALIC_CURLYBRACES,
	'}', TIMESNR_BOLDITALIC_CURLYBRACES,
	'_', TIMESNR_BOLDITALIC_HASH_ANDMORE,
	'#', TIMESNR_BOLDITALIC_HASH_ANDMORE,
	'$', TIMESNR_BOLDITALIC_HASH_ANDMORE,
	'*', TIMESNR_BOLDITALIC_STAR,
	'~', TIMESNR_BOLDITALIC_EQUALSIGN_ANDMORE,
	'=', TIMESNR_BOLDITALIC_EQUALSIGN_ANDMORE,
	'+', TIMESNR_BOLDITALIC_EQUALSIGN_ANDMORE,
	'<', TIMESNR_BOLDITALIC_EQUALSIGN_ANDMORE,
	'>', TIMESNR_BOLDITALIC_EQUALSIGN_ANDMORE,
	'&', TIMESNR_BOLDITALIC_AMPERSAND,
	'%', TIMESNR_BOLDITALIC_PERCENTATSYMBOL,
	'@', TIMESNR_BOLDITALIC_PERCENTATSYMBOL,
	0, 0
	};

	//SetCharCodeList( charWidths, &list );
	SetBoldItalicCharWidths( charWidths );
}


PDFHelveticaFont::PDFHelveticaFont( std::string fontNameP, int fontSizeP, int fontSpacingP /*= 4*/ )
	: PDFCharScaleFont( fontNameP, fontSizeP, fontSpacingP )
{
	InitNormalCharWidths();
	InitBoldCharWidths();
	InitItalicCharWidths();
	InitBoldItalicCharWidths();
}

void PDFHelveticaFont::InitNormalCharWidths()
{
	const double HELVETICA_LOWER_ABDEGHNOPQU = 0.5556;
	const double HELVETICA_LOWER_CKSVXYZ = 0.5;
	const double HELVETICA_LOWER_FT = 0.2778;
	const double HELVETICA_LOWER_IJL = 0.2222;
	const double HELVETICA_LOWER_M = 0.8333;
	const double HELVETICA_LOWER_R = 0.3333;
	const double HELVETICA_LOWER_W = 0.7225;

	const double HELVETICA_UPPER_ABEVKPSVXY = 0.6667;
	const double HELVETICA_UPPER_CDHNRU = HELVETICA_LOWER_W;
	const double HELVETICA_UPPER_FTZ = 0.6098;
	const double HELVETICA_UPPER_GOQ = 0.7788;
	const double HELVETICA_UPPER_I = HELVETICA_LOWER_FT;
	const double HELVETICA_UPPER_J = HELVETICA_LOWER_CKSVXYZ;
	const double HELVETICA_UPPER_L = HELVETICA_LOWER_ABDEGHNOPQU;
	const double HELVETICA_UPPER_M = 0.8333;
	const double HELVETICA_UPPER_W = 0.9434;

	const double HELVETICA_NUMBERS = HELVETICA_LOWER_ABDEGHNOPQU;

	const double HELVETICA_BAR /* | */ = 0.2604;
	const double HELVETICA_SLASHES_ANDMORE /* ,.\/[]!:; spacechar */ = HELVETICA_LOWER_FT;
	const double HELVETICA_ROUNDBRACES_ANDMORE /* `-(){} */ = 0.3333;
	const double HELVETICA_QUOTES /* " */ = FONT_SIZE_03546;
	const double HELVETICA_SINGLEQUOTE /* ' */ = FONT_SIZE_01908;
	const double HELVETICA_CIRCUMFLEX /* ^ */ = 0.469;
	const double HELVETICA_STAR /* * */ = 0.3858;
	const double HELVETICA_HASH_ANDMORE /* _#$? */ = HELVETICA_LOWER_ABDEGHNOPQU;
	const double HELVETICA_EQUALSIGN_ANDMORE /* =~+<> */ = 0.5841;
	const double HELVETICA_AMPERSAND /* & */ = HELVETICA_UPPER_ABEVKPSVXY;
	const double HELVETICA_PERCENT /* % */ = 0.8897;
	const double HELVETICA_ATSYMBOL /* @ */ = 1.016;

	CharOnePointSizeList list;

	CharWidths charWidths[] = {
	(unsigned char)0347, HELVETICA_UPPER_ABEVKPSVXY,	// Aacute
	(unsigned char)0345, HELVETICA_UPPER_ABEVKPSVXY,	// Acirc
	(unsigned char)0313, HELVETICA_UPPER_ABEVKPSVXY,	// Agrave
	(unsigned char)0256, HELVETICA_UPPER_ABEVKPSVXY /*????*/,	// AElig
	(unsigned char)0201, HELVETICA_UPPER_ABEVKPSVXY,	// Aring
	(unsigned char)0314, HELVETICA_UPPER_ABEVKPSVXY,	// Atilde
	(unsigned char)0200, HELVETICA_UPPER_ABEVKPSVXY,	// Auml
	(unsigned char)0202, HELVETICA_LOWER_ABDEGHNOPQU /*????*/,	// Ccedil
	(unsigned char)0056, HELVETICA_SLASHES_ANDMORE /*????*/,	// Dot 
	(unsigned char)0203, HELVETICA_UPPER_ABEVKPSVXY,	// Eacute
	(unsigned char)0346, HELVETICA_UPPER_ABEVKPSVXY,	// Ecirc
	(unsigned char)0351, HELVETICA_UPPER_ABEVKPSVXY,	// Egrave
	(unsigned char)0350, HELVETICA_UPPER_ABEVKPSVXY,	// Euml
	(unsigned char)0352, HELVETICA_UPPER_I,	// Iacute
	(unsigned char)0353, HELVETICA_UPPER_I,	// Icirc
	(unsigned char)0355, HELVETICA_UPPER_I,	// Igrave
	(unsigned char)0354, HELVETICA_UPPER_I,	// Iuml
	(unsigned char)0204, HELVETICA_LOWER_ABDEGHNOPQU,	// Ntilde
	(unsigned char)0356, HELVETICA_LOWER_ABDEGHNOPQU,	// Oacute  
	(unsigned char)0357, HELVETICA_LOWER_ABDEGHNOPQU,	// Ocirc
	(unsigned char)0361, HELVETICA_LOWER_ABDEGHNOPQU,	// Ograve
	(unsigned char)0257, HELVETICA_LOWER_ABDEGHNOPQU,	// Oslash
	(unsigned char)0315, HELVETICA_LOWER_ABDEGHNOPQU,	// Otilde
	(unsigned char)0235, HELVETICA_LOWER_ABDEGHNOPQU,	// Ouml
	(unsigned char)0120, HELVETICA_LOWER_ABDEGHNOPQU /*????*/,	// THORN, becomes 'standard P'
	(unsigned char)0362, HELVETICA_LOWER_ABDEGHNOPQU,	// Uacute
	(unsigned char)0363, HELVETICA_LOWER_ABDEGHNOPQU,	// Ucirc
	(unsigned char)0364, HELVETICA_LOWER_ABDEGHNOPQU,	// Ugrave
	(unsigned char)0206, HELVETICA_LOWER_ABDEGHNOPQU,	// Uuml
	(unsigned char)0131, HELVETICA_UPPER_ABEVKPSVXY,	// Yacute, becomes 'standard Y'
	(unsigned char)0207, HELVETICA_LOWER_ABDEGHNOPQU,	// aacute
	(unsigned char)0235, HELVETICA_LOWER_ABDEGHNOPQU,	// acute
	(unsigned char)0235, HELVETICA_LOWER_ABDEGHNOPQU,	// acirc
	(unsigned char)0276, HELVETICA_LOWER_ABDEGHNOPQU,	// aelig
	(unsigned char)0210, HELVETICA_LOWER_ABDEGHNOPQU,	// agrave
	(unsigned char)0214, HELVETICA_LOWER_ABDEGHNOPQU,	// aring
	(unsigned char)0235, HELVETICA_LOWER_ABDEGHNOPQU,	// atilde
	(unsigned char)0212, HELVETICA_LOWER_ABDEGHNOPQU,	// auml
	(unsigned char)0174, HELVETICA_BAR,	// brvbar, becomes standard bar '|'
	(unsigned char)0215, HELVETICA_LOWER_ABDEGHNOPQU,	// ccedil
	(unsigned char)0374, HELVETICA_LOWER_ABDEGHNOPQU,	// cedil
	(unsigned char)0242, HELVETICA_LOWER_ABDEGHNOPQU,	// cent
	(unsigned char)0251, HELVETICA_LOWER_ABDEGHNOPQU,	// copy
	(unsigned char)0333, HELVETICA_LOWER_ABDEGHNOPQU /*????*/,	// curren
	(unsigned char)0241, HELVETICA_LOWER_ABDEGHNOPQU /*????*/,	// deg 
	(unsigned char)0216, HELVETICA_LOWER_ABDEGHNOPQU,	// eacute 
	(unsigned char)0220, HELVETICA_LOWER_ABDEGHNOPQU,	// ecirc 
	(unsigned char)0217, HELVETICA_LOWER_ABDEGHNOPQU,	// egrave
	(unsigned char)0144, HELVETICA_LOWER_ABDEGHNOPQU,	// eth, problem!! becomes "d' as this is what it loks most like?
	(unsigned char)0221, HELVETICA_LOWER_ABDEGHNOPQU,	// euml
	(unsigned char)0222, HELVETICA_LOWER_IJL,	// iacute
	(unsigned char)0224, HELVETICA_LOWER_IJL,	// icirc
	(unsigned char)0223, HELVETICA_LOWER_IJL,	// igrave
	(unsigned char)0225, HELVETICA_LOWER_IJL,	// iuml
	(unsigned char)0370, HELVETICA_SLASHES_ANDMORE /*????*/,	// macr
	(unsigned char)0341, HELVETICA_SLASHES_ANDMORE /*????*/,	// middot, using 'period centered'
	(unsigned char)0226, HELVETICA_LOWER_ABDEGHNOPQU,	// ntilde 
	(unsigned char)0227, HELVETICA_LOWER_ABDEGHNOPQU,	// oacute  
	(unsigned char)0231, HELVETICA_LOWER_ABDEGHNOPQU,	// ocirc  
	(unsigned char)0230, HELVETICA_LOWER_ABDEGHNOPQU,	// ograve
	(unsigned char)0273, HELVETICA_LOWER_ABDEGHNOPQU,	// ordf
	(unsigned char)0274, HELVETICA_LOWER_ABDEGHNOPQU,	// ordm
	(unsigned char)0277, HELVETICA_LOWER_ABDEGHNOPQU,	// oslash
	(unsigned char)0233, HELVETICA_LOWER_ABDEGHNOPQU,	// otilde  
	(unsigned char)0232, HELVETICA_LOWER_ABDEGHNOPQU,	// ouml
	(unsigned char)0246, HELVETICA_LOWER_ABDEGHNOPQU /*????*/,	// para
	(unsigned char)0053, HELVETICA_LOWER_ABDEGHNOPQU /*????*/,	// plusmn
	(unsigned char)0243, HELVETICA_LOWER_ABDEGHNOPQU /*????*/,	// pound, bcomes 'sterling'
	(unsigned char)0250, HELVETICA_ATSYMBOL /*????*/,	// reg 
	(unsigned char)0244, HELVETICA_LOWER_ABDEGHNOPQU /*????*/,	// sect
	(unsigned char)0234, HELVETICA_LOWER_ABDEGHNOPQU /*????*/,  
	(unsigned char)0236, HELVETICA_LOWER_ABDEGHNOPQU,	// ucirc  
	(unsigned char)0235, HELVETICA_LOWER_ABDEGHNOPQU,	// ugrave  
	(unsigned char)0237, HELVETICA_LOWER_ABDEGHNOPQU,	// uml
	(unsigned char)0237, HELVETICA_LOWER_ABDEGHNOPQU,	// uuml
	(unsigned char)0264, HELVETICA_LOWER_CKSVXYZ,	// yen 
	(unsigned char)0330, HELVETICA_LOWER_CKSVXYZ,	// yuml 
	(unsigned char)0235, HELVETICA_LOWER_CKSVXYZ,	// code = xxx; break325";
	(unsigned char)0252, HELVETICA_ATSYMBOL,	// trademark

	// Foreign letters and symbols
	(unsigned char)130 /* C cedil		*/, HELVETICA_UPPER_CDHNRU,
	(unsigned char)131 /* E egrave	*/, HELVETICA_UPPER_ABEVKPSVXY,
	(unsigned char)132 /* N tilde		*/, HELVETICA_UPPER_CDHNRU,
	(unsigned char)133 /* 0 umlaut	*/, HELVETICA_UPPER_GOQ,
	(unsigned char)134 /* U umlaut	*/, HELVETICA_UPPER_CDHNRU,
	(unsigned char)135 /* a acute		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)136 /* a grave		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)137 /* a circum	*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)138 /* a umlaut	*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)139 /* a tilde		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)140 /* a ring		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)141 /* c cedil		*/, HELVETICA_LOWER_CKSVXYZ,
	(unsigned char)142 /* e acute		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)143 /* e grave		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)144 /* e circum	*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)145 /* e umlaut	*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)146 /* l acute		*/, FONT_SIZE_02778 /* for foreign i */,
	(unsigned char)147 /* l grave		*/, FONT_SIZE_02778 /* for foreign i */,
	(unsigned char)148 /* l circum	*/, FONT_SIZE_02778 /* for foreign i */,
	(unsigned char)149 /* l umlaut	*/, FONT_SIZE_02778 /* for foreign i */,
	(unsigned char)150 /* n titde		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)151 /* o acute		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)152 /* o grave		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)153 /* o circum	*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)154 /* o umlaut	*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)155 /* o titde		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)156 /* u acute		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)157 /* u grave		*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)158 /* u circum	*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)159 /* u umlaut	*/, HELVETICA_LOWER_ABDEGHNOPQU,
	(unsigned char)160 /* cross		*/, FONT_SIZE_05556,
	(unsigned char)161 /* ring		*/, FONT_SIZE_04000,
	(unsigned char)162 /* cent		*/, FONT_SIZE_05556,
	(unsigned char)163 /* pound		*/, FONT_SIZE_05556,
	(unsigned char)164 /* double S	*/, FONT_SIZE_05556,
	(unsigned char)165 /* dot			*/, FONT_SIZE_03501,
	(unsigned char)166 /* para end	*/, FONT_SIZE_05376,
	(unsigned char)167 /* german SS	*/, FONT_SIZE_06098,
	(unsigned char)168 /* register	*/, FONT_SIZE_07375,
	(unsigned char)169 /* copyright	*/, FONT_SIZE_07375,
	(unsigned char)170 /* trademark	*/, FONT_SIZE_10000,
	(unsigned char)171 /* apostrophe	*/, FONT_SIZE_03333,
	(unsigned char)172 /* umlaut		*/, FONT_SIZE_03333,
	//(unsigned char)173 /* space		*/, 0,
	(unsigned char)174 /* AE ligature	*/, FONT_SIZE_10000,
	(unsigned char)175 /* O slash		*/, FONT_SIZE_07788,
	//(unsigned char)176 /* space		*/, 0,
	(unsigned char)177 /* plusminus	*/, FONT_SIZE_05841,
	//(unsigned char)178 /* space		*/, 0,
	//(unsigned char)179 /* space		*/, 0,
	(unsigned char)180 /* yen			*/, FONT_SIZE_05556,
	(unsigned char)181 /* u long stem	*/, FONT_SIZE_05787,
	//(unsigned char)182 /* space		*/, 0,
	//(unsigned char)183 /* space		*/, 0,
	//(unsigned char)184 /* space		*/, 0,
	//(unsigned char)185 /* space		*/, 0,
	//(unsigned char)186 /* space		*/, 0,
	(unsigned char)187 /* a little	*/, FONT_SIZE_03704,
	(unsigned char)188 /* o little	*/, FONT_SIZE_03647,
	//(unsigned char)189 /* space		*/, 0,
	(unsigned char)190 /* ae ligature	*/, FONT_SIZE_08897,
	(unsigned char)191 /* o slash		*/, FONT_SIZE_06098,
	(unsigned char)192 /* ? inverted	*/, FONT_SIZE_06098,
	(unsigned char)193 /* ! inverted	*/, FONT_SIZE_03333,
	(unsigned char)194 /* right angle	*/, FONT_SIZE_05841,
	//(unsigned char)195 /* space		*/, 0,
	(unsigned char)196 /* f fancy		*/, FONT_SIZE_05556,
	//(unsigned char)197 /* space		*/, 0,
	//(unsigned char)198 /* space		*/, 0,
	(unsigned char)199 /* <<			*/, FONT_SIZE_05556,
	(unsigned char)200 /* >>			*/, FONT_SIZE_05556,
	(unsigned char)201 /* ... ellipse	*/, FONT_SIZE_10000,
	//(unsigned char)202 /* space		*/, 0,
	(unsigned char)203 /* A grave		*/, HELVETICA_UPPER_ABEVKPSVXY,
	(unsigned char)204 /* A tilde		*/, HELVETICA_UPPER_ABEVKPSVXY,
	(unsigned char)205 /* O tilde		*/, HELVETICA_UPPER_GOQ,
	(unsigned char)206 /* OE ligature	*/, FONT_SIZE_10000,
	(unsigned char)207 /* oe ligature	*/, FONT_SIZE_09434,
	(unsigned char)208 /* underscore	*/, FONT_SIZE_05556,
	(unsigned char)209 /* long under	*/, FONT_SIZE_10000,
	(unsigned char)210 /* dquotesopen	*/, FONT_SIZE_03333,
	(unsigned char)211 /* dquotesclose*/, FONT_SIZE_03333,
	(unsigned char)212 /* squotesopen	*/, FONT_SIZE_02222,
	(unsigned char)213 /* squotesclose*/, FONT_SIZE_02222,
	(unsigned char)214 /* divide sign	*/, FONT_SIZE_05841,
	//(unsigned char)215 /* space		*/, 0,
	(unsigned char)216 /* y umlaut	*/, HELVETICA_LOWER_CKSVXYZ,
	(unsigned char)217 /* Y umlaut	*/, HELVETICA_UPPER_ABEVKPSVXY,
	(unsigned char)218 /* / 45d slash	*/, FONT_SIZE_01667,
	(unsigned char)219 /* circlestar	*/, FONT_SIZE_05556,
	(unsigned char)220 /* <			*/, FONT_SIZE_03333,
	(unsigned char)221 /* >			*/, FONT_SIZE_03333,
	(unsigned char)222 /* fi ligature */, FONT_SIZE_05000,
	(unsigned char)223 /* fl ligature	*/, FONT_SIZE_05000,
	(unsigned char)224 /* bar 2 slash	*/, FONT_SIZE_05556,
	(unsigned char)225 /* . dot		*/, FONT_SIZE_02778,
	(unsigned char)226 /* , comma		*/, FONT_SIZE_02222,
	(unsigned char)227 /* ,, 2 comma	*/, FONT_SIZE_03333,
	(unsigned char)228 /* % with o	*/, FONT_SIZE_10000,
	(unsigned char)229 /* A accent	*/, HELVETICA_UPPER_ABEVKPSVXY,
	(unsigned char)230 /* E accent	*/, HELVETICA_UPPER_ABEVKPSVXY,
	(unsigned char)231 /* A acute		*/, HELVETICA_UPPER_ABEVKPSVXY,
	(unsigned char)232 /* E umlaut	*/, HELVETICA_UPPER_ABEVKPSVXY,
	(unsigned char)233 /* E grave		*/, HELVETICA_UPPER_ABEVKPSVXY,
	(unsigned char)234 /* l acute		*/, FONT_SIZE_02778,
	(unsigned char)235 /* l accent	*/, FONT_SIZE_02778,
	(unsigned char)236 /* l umlaut	*/, FONT_SIZE_02778,
	(unsigned char)237 /* l grave		*/, FONT_SIZE_02778,
	(unsigned char)238 /* O acute		*/, HELVETICA_UPPER_GOQ,
	(unsigned char)239 /* O accent	*/, HELVETICA_UPPER_GOQ,
	//(unsigned char)240 /* space		*/, 0,
	(unsigned char)241 /* O grave		*/, HELVETICA_UPPER_GOQ,
	(unsigned char)242 /* U acute		*/, HELVETICA_UPPER_CDHNRU,
	(unsigned char)243 /* U accent	*/, HELVETICA_UPPER_CDHNRU,
	(unsigned char)244 /* U grave		*/, HELVETICA_UPPER_CDHNRU,
	(unsigned char)245 /* / slash		*/, FONT_SIZE_02778,
	(unsigned char)246 /* ^ accent	*/, FONT_SIZE_03333,
	(unsigned char)247 /* ~ squiggle	*/, FONT_SIZE_03333,
	(unsigned char)248 /* - high dash	*/, FONT_SIZE_03333,
	(unsigned char)249 /* high u		*/, FONT_SIZE_03333,
	(unsigned char)250 /* high dot	*/, FONT_SIZE_03333,
	(unsigned char)251 /* high circle	*/, FONT_SIZE_03333,
	(unsigned char)252 /* curl left	*/, FONT_SIZE_03333,
	(unsigned char)253 /* acute		*/, FONT_SIZE_03333,
	(unsigned char)254 /* curl right	*/, FONT_SIZE_03333,
	(unsigned char)255 /* accent inv	*/, FONT_SIZE_03333,

	// English letters and symbols
	'a', HELVETICA_LOWER_ABDEGHNOPQU,
	'b', HELVETICA_LOWER_ABDEGHNOPQU,
	'c', HELVETICA_LOWER_CKSVXYZ,
	'd', HELVETICA_LOWER_ABDEGHNOPQU,
	'e', HELVETICA_LOWER_ABDEGHNOPQU,
	'f', HELVETICA_LOWER_FT,
	'g', HELVETICA_LOWER_ABDEGHNOPQU,
	'h', HELVETICA_LOWER_ABDEGHNOPQU,
	'i', HELVETICA_LOWER_IJL,
	'j', HELVETICA_LOWER_IJL,
	'k', HELVETICA_LOWER_CKSVXYZ,
	'l', HELVETICA_LOWER_IJL,
	'm', HELVETICA_LOWER_M,
	'n', HELVETICA_LOWER_ABDEGHNOPQU,
	'o', HELVETICA_LOWER_ABDEGHNOPQU,
	'p', HELVETICA_LOWER_ABDEGHNOPQU,
	'q', HELVETICA_LOWER_ABDEGHNOPQU,
	'r', HELVETICA_LOWER_R,
	's', HELVETICA_LOWER_CKSVXYZ,
	't', HELVETICA_LOWER_FT,
	'u', HELVETICA_LOWER_ABDEGHNOPQU,
	'v', HELVETICA_LOWER_CKSVXYZ,
	'x', HELVETICA_LOWER_CKSVXYZ,
	'w', HELVETICA_LOWER_W,
	'y', HELVETICA_LOWER_CKSVXYZ,
	'z', HELVETICA_LOWER_CKSVXYZ,
	'A', HELVETICA_UPPER_ABEVKPSVXY,
	'B', HELVETICA_UPPER_ABEVKPSVXY,
	'C', HELVETICA_UPPER_CDHNRU,
	'D', HELVETICA_UPPER_CDHNRU,
	'E', HELVETICA_UPPER_ABEVKPSVXY,
	'G', HELVETICA_UPPER_GOQ,
	'H', HELVETICA_UPPER_CDHNRU,
	'I', HELVETICA_UPPER_I,
	'J', HELVETICA_UPPER_J,
	'F', HELVETICA_UPPER_FTZ,
	'K', HELVETICA_UPPER_ABEVKPSVXY,
	'L', HELVETICA_UPPER_L,
	'M', HELVETICA_UPPER_M,
	'N', HELVETICA_UPPER_CDHNRU,
	'O', HELVETICA_UPPER_GOQ,
	'P', HELVETICA_UPPER_ABEVKPSVXY,
	'Q', HELVETICA_UPPER_GOQ,
	'R', HELVETICA_UPPER_CDHNRU,
	'S', HELVETICA_UPPER_ABEVKPSVXY,
	'T', HELVETICA_UPPER_FTZ,
	'U', HELVETICA_UPPER_CDHNRU,
	'W', HELVETICA_UPPER_W,
	'V', HELVETICA_UPPER_ABEVKPSVXY,
	'X', HELVETICA_UPPER_ABEVKPSVXY,
	'Y', HELVETICA_UPPER_ABEVKPSVXY,
	'Z', HELVETICA_UPPER_FTZ,
	'0', HELVETICA_NUMBERS,
	'1', HELVETICA_NUMBERS,
	'2', HELVETICA_NUMBERS,
	'3', HELVETICA_NUMBERS,
	'4', HELVETICA_NUMBERS,
	'5', HELVETICA_NUMBERS,
	'6', HELVETICA_NUMBERS,
	'7', HELVETICA_NUMBERS,
	'8', HELVETICA_NUMBERS,
	'9', HELVETICA_NUMBERS,
	'|', HELVETICA_BAR,
	',', HELVETICA_SLASHES_ANDMORE,
	'.', HELVETICA_SLASHES_ANDMORE,
	'\\',HELVETICA_SLASHES_ANDMORE,
	'/', HELVETICA_SLASHES_ANDMORE,
	':', HELVETICA_SLASHES_ANDMORE,
	';', HELVETICA_SLASHES_ANDMORE,
	' ', HELVETICA_SLASHES_ANDMORE,
	'[', HELVETICA_SLASHES_ANDMORE,
	']', HELVETICA_SLASHES_ANDMORE,
	'`', HELVETICA_ROUNDBRACES_ANDMORE,
	'-', HELVETICA_ROUNDBRACES_ANDMORE,
	'(', HELVETICA_ROUNDBRACES_ANDMORE,
	')', HELVETICA_ROUNDBRACES_ANDMORE,
	'!', HELVETICA_SLASHES_ANDMORE,
	'?', HELVETICA_HASH_ANDMORE,
	'"', HELVETICA_QUOTES,
	'\'',HELVETICA_SINGLEQUOTE,
	'^', HELVETICA_CIRCUMFLEX,
	'{', HELVETICA_ROUNDBRACES_ANDMORE,
	'}', HELVETICA_ROUNDBRACES_ANDMORE,
	'*', HELVETICA_STAR,
	'_', HELVETICA_HASH_ANDMORE,
	'#', HELVETICA_HASH_ANDMORE,
	'$', HELVETICA_HASH_ANDMORE,
	'~', HELVETICA_EQUALSIGN_ANDMORE,
	'=', HELVETICA_EQUALSIGN_ANDMORE,
	'+', HELVETICA_EQUALSIGN_ANDMORE,
	'<', HELVETICA_EQUALSIGN_ANDMORE,
	'>', HELVETICA_EQUALSIGN_ANDMORE,
	'&', HELVETICA_AMPERSAND,
	'%', HELVETICA_PERCENT,
	'@', HELVETICA_ATSYMBOL,
	0, 0
	};

	//SetCharCodeList( charWidths, &list );
	SetNormalCharWidths( charWidths ); // Set the list of Scaled 
}

void PDFHelveticaFont::InitBoldCharWidths()
{
	const double HELVETICA_BOLD_LOWER_ACEKSVXY = 0.5556;
	const double HELVETICA_BOLD_LOWER_BDGHNOPQU = 0.6098;
	const double HELVETICA_BOLD_LOWER_FT = 0.3333;
	const double HELVETICA_BOLD_LOWER_IJL = 0.2778;
	const double HELVETICA_BOLD_LOWER_M = 0.8897;//0.8905;
	const double HELVETICA_BOLD_LOWER_R  = 0.39;//0.4219;
	const double HELVETICA_BOLD_LOWER_W = 0.7788;//0.6734;
	const double HELVETICA_BOLD_LOWER_Z = 0.5;

	const double HELVETICA_BOLD_UPPER_ABCDHKNRU = 0.7225;
	const double HELVETICA_BOLD_UPPER_EPSVXY = 0.6667;
	const double HELVETICA_BOLD_UPPER_FLTZ = 0.6098;
	const double HELVETICA_BOLD_UPPER_I = 0.2778;
	const double HELVETICA_BOLD_UPPER_J = 0.5556;
	const double HELVETICA_BOLD_UPPER_GOQ = 0.7788;
	const double HELVETICA_BOLD_UPPER_M = 0.8333;
	const double HELVETICA_BOLD_UPPER_W = 0.9434;

	const double HELVETICA_BOLD_NUMBERS = 0.5556;

	const double HELVETICA_BOLD_BAR /* | */ = 0.28;
	const double HELVETICA_BOLD_COMMADOTSLASHES_SPACE /* ,.\/spacechar */ = 0.2778;
	const double HELVETICA_BOLD_SQUAREBRACES_ANDMORE /* []`'-():; */ = 0.3333;
	//const double HELVETICA_BOLD_ /* ? */ = 0.6098;
	const double HELVETICA_BOLD_EXCLAMATION /* ! */ = 50.0/150.0;
	const double HELVETICA_BOLD_APPIONMARK /* ? */ = 50.0/82.0;//0.6098;
	const double HELVETICA_BOLD_QUOTES /* " */ = FONT_SIZE_04739;
	const double HELVETICA_BOLD_SINGLEQUOTE /* ' */ = FONT_SIZE_02381;
	const double HELVETICA_BOLD_CIRCUMFLEX /* ^ */ = 0.578;
	const double HELVETICA_BOLD_CURLYBRACES /* {} */ = 0.3888;
	const double HELVETICA_BOLD_STAR /* * */ = 0.3888;
	const double HELVETICA_BOLD_HASH_ANDMORE /* _#$ */ = 0.5556;
	const double HELVETICA_BOLD_SQUIGGLE /* ~ */ = 0.5841;
	const double HELVETICA_BOLD_EQUALSIGN_ANDMORE /* =+<> */ = 0.5841;
	const double HELVETICA_BOLD_AMPERSAND /* & */ = 0.7225;
	const double HELVETICA_BOLD_PERCENT /* % */ = 0.8897;
	const double HELVETICA_BOLD_ATSYMBOL /* @ */ = 50.0/51.2;//0.9766;

	CharOnePointSizeList list;

	CharWidths charWidths[] = {
	(unsigned char)0347, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Aacute
	(unsigned char)0345, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Acirc
	(unsigned char)0313, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Agrave
	(unsigned char)0256, HELVETICA_BOLD_UPPER_ABCDHKNRU /*????*/,	// AElig
	(unsigned char)0201, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Aring
	(unsigned char)0314, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Atilde
	(unsigned char)0200, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Auml
	(unsigned char)0202, HELVETICA_BOLD_UPPER_ABCDHKNRU /*????*/,	// Ccedil
	(unsigned char)0056, HELVETICA_BOLD_UPPER_ABCDHKNRU /*????*/,	// Dot 
	(unsigned char)0203, HELVETICA_BOLD_UPPER_EPSVXY,	// Eacute
	(unsigned char)0346, HELVETICA_BOLD_UPPER_EPSVXY,	// Ecirc
	(unsigned char)0351, HELVETICA_BOLD_UPPER_EPSVXY,	// Egrave
	(unsigned char)0350, HELVETICA_BOLD_UPPER_EPSVXY,	// Euml
	(unsigned char)0352, HELVETICA_BOLD_UPPER_I,	// Iacute
	(unsigned char)0353, HELVETICA_BOLD_UPPER_I,	// Icirc
	(unsigned char)0355, HELVETICA_BOLD_UPPER_I,	// Igrave
	(unsigned char)0354, HELVETICA_BOLD_UPPER_I,	// Iuml
	(unsigned char)0204, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Ntilde
	(unsigned char)0356, HELVETICA_BOLD_UPPER_GOQ,	// Oacute  
	(unsigned char)0357, HELVETICA_BOLD_UPPER_GOQ,	// Ocirc
	(unsigned char)0361, HELVETICA_BOLD_UPPER_GOQ,	// Ograve
	(unsigned char)0257, HELVETICA_BOLD_UPPER_GOQ,	// Oslash
	(unsigned char)0315, HELVETICA_BOLD_UPPER_GOQ,	// Otilde
	(unsigned char)0235, HELVETICA_BOLD_UPPER_GOQ,	// Ouml
	(unsigned char)0120, HELVETICA_BOLD_UPPER_EPSVXY /*????*/,	// THORN, becomes 'standard P'
	(unsigned char)0362, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Uacute
	(unsigned char)0363, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Ucirc
	(unsigned char)0364, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Ugrave
	(unsigned char)0206, HELVETICA_BOLD_UPPER_ABCDHKNRU,	// Uuml
	(unsigned char)0131, HELVETICA_BOLD_UPPER_EPSVXY,	// Yacute, becomes 'standard Y'
	(unsigned char)0207, HELVETICA_BOLD_LOWER_ACEKSVXY,	// aacute
	(unsigned char)0235, HELVETICA_BOLD_LOWER_ACEKSVXY,	// acute
	(unsigned char)0235, HELVETICA_BOLD_LOWER_ACEKSVXY,	// acirc
	(unsigned char)0276, HELVETICA_BOLD_LOWER_ACEKSVXY,	// aelig
	(unsigned char)0210, HELVETICA_BOLD_LOWER_ACEKSVXY,	// agrave
	(unsigned char)0214, HELVETICA_BOLD_LOWER_ACEKSVXY,	// aring
	(unsigned char)0235, HELVETICA_BOLD_LOWER_ACEKSVXY,	// atilde
	(unsigned char)0212, HELVETICA_BOLD_LOWER_ACEKSVXY,	// auml
	(unsigned char)0174, HELVETICA_BOLD_BAR,	// brvbar, becomes standard bar '|'
	(unsigned char)0215, HELVETICA_BOLD_LOWER_ACEKSVXY,	// ccedil
	(unsigned char)0374, HELVETICA_BOLD_LOWER_ACEKSVXY,	// cedil
	(unsigned char)0242, HELVETICA_BOLD_LOWER_ACEKSVXY,	// cent
	(unsigned char)0251, HELVETICA_BOLD_ATSYMBOL,	// copy
	(unsigned char)0333, HELVETICA_BOLD_LOWER_ACEKSVXY /*????*/,	// curren
	(unsigned char)0241, HELVETICA_BOLD_LOWER_ACEKSVXY /*????*/,	// deg 
	(unsigned char)0216, HELVETICA_BOLD_LOWER_ACEKSVXY,	// eacute 
	(unsigned char)0220, HELVETICA_BOLD_LOWER_ACEKSVXY,	// ecirc 
	(unsigned char)0217, HELVETICA_BOLD_LOWER_ACEKSVXY,	// egrave
	(unsigned char)0144, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// eth, problem!! becomes "d' as this is what it loks most like?
	(unsigned char)0221, HELVETICA_BOLD_LOWER_ACEKSVXY,	// euml
	(unsigned char)0222, HELVETICA_BOLD_LOWER_IJL,	// iacute
	(unsigned char)0224, HELVETICA_BOLD_LOWER_IJL,	// icirc
	(unsigned char)0223, HELVETICA_BOLD_LOWER_IJL,	// igrave
	(unsigned char)0225, HELVETICA_BOLD_LOWER_IJL,	// iuml
	(unsigned char)0370, HELVETICA_BOLD_LOWER_ACEKSVXY /*????*/,	// macr
	(unsigned char)0341, HELVETICA_BOLD_LOWER_ACEKSVXY /*????*/,	// middot, using 'period centered'
	(unsigned char)0226, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// ntilde 
	(unsigned char)0227, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// oacute  
	(unsigned char)0231, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// ocirc  
	(unsigned char)0230, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// ograve
	(unsigned char)0273, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// ordf
	(unsigned char)0274, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// ordm
	(unsigned char)0277, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// oslash
	(unsigned char)0233, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// otilde  
	(unsigned char)0232, HELVETICA_BOLD_LOWER_ACEKSVXY,	// ouml
	(unsigned char)0246, HELVETICA_BOLD_LOWER_ACEKSVXY /*????*/,	// para
	(unsigned char)0053, HELVETICA_BOLD_LOWER_ACEKSVXY /*????*/,	// plusmn
	(unsigned char)0243, HELVETICA_BOLD_LOWER_ACEKSVXY /*????*/,	// pound, bcomes 'sterling'
	(unsigned char)0250, HELVETICA_BOLD_ATSYMBOL /*????*/,	// reg 
	(unsigned char)0244, HELVETICA_BOLD_LOWER_ACEKSVXY /*????*/,	// sect
	(unsigned char)0234, HELVETICA_BOLD_LOWER_ACEKSVXY /*????*/,  
	(unsigned char)0236, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// ucirc  
	(unsigned char)0235, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// ugrave  
	(unsigned char)0237, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// uml
	(unsigned char)0237, HELVETICA_BOLD_LOWER_BDGHNOPQU,	// uuml
	(unsigned char)0264, HELVETICA_BOLD_LOWER_ACEKSVXY,	// yen 
	(unsigned char)0330, HELVETICA_BOLD_LOWER_ACEKSVXY,	// yuml 
	(unsigned char)0235, HELVETICA_BOLD_LOWER_ACEKSVXY,	// code = xxx; break325";
	(unsigned char)0252, HELVETICA_BOLD_ATSYMBOL,	// trademark

	// Foreign letters and symbols
	(unsigned char)130 /* C cedil		*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)131 /* E egrave	*/, HELVETICA_BOLD_UPPER_EPSVXY,
	(unsigned char)132 /* N tilde		*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)133 /* 0 umlaut	*/, HELVETICA_BOLD_UPPER_GOQ,
	(unsigned char)134 /* U umlaut	*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)135 /* a acute		*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)136 /* a grave		*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)137 /* a circum	*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)138 /* a umlaut	*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)139 /* a tilde		*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)140 /* a ring		*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)141 /* c cedil		*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)142 /* e acute		*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)143 /* e grave		*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)144 /* e circum	*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)145 /* e umlaut	*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)146 /* l acute		*/, HELVETICA_BOLD_LOWER_IJL /* for foreign i */,
	(unsigned char)147 /* l grave		*/, HELVETICA_BOLD_LOWER_IJL /* for foreign i */,
	(unsigned char)148 /* l circum	*/, HELVETICA_BOLD_LOWER_IJL /* for foreign i */,
	(unsigned char)149 /* l umlaut	*/, HELVETICA_BOLD_LOWER_IJL /* for foreign i */,
	(unsigned char)150 /* n titde		*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)151 /* o acute		*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)152 /* o grave		*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)153 /* o circum	*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)154 /* o umlaut	*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)155 /* o titde		*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)156 /* u acute		*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)157 /* u grave		*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)158 /* u circum	*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)159 /* u umlaut	*/, HELVETICA_BOLD_LOWER_BDGHNOPQU,
	(unsigned char)160 /* cross		*/, FONT_SIZE_05556,
	(unsigned char)161 /* ring		*/, FONT_SIZE_04000,
	(unsigned char)162 /* cent		*/, FONT_SIZE_05556,
	(unsigned char)163 /* pound		*/, FONT_SIZE_05556,
	(unsigned char)164 /* double S	*/, FONT_SIZE_05556,
	(unsigned char)165 /* dot			*/, FONT_SIZE_03501,
	(unsigned char)166 /* para end	*/, FONT_SIZE_05376,
	(unsigned char)167 /* german SS	*/, FONT_SIZE_06098,
	(unsigned char)168 /* register	*/, FONT_SIZE_07375,
	(unsigned char)169 /* copyright	*/, FONT_SIZE_07375,
	(unsigned char)170 /* trademark	*/, FONT_SIZE_10000,
	(unsigned char)171 /* apostrophe	*/, FONT_SIZE_03333,
	(unsigned char)172 /* umlaut		*/, FONT_SIZE_03333,
	//(unsigned char)173 /* space		*/, 0,
	(unsigned char)174 /* AE ligature	*/, FONT_SIZE_10000,
	(unsigned char)175 /* O slash		*/, FONT_SIZE_07788,
	//(unsigned char)176 /* space		*/, 0,
	(unsigned char)177 /* plusminus	*/, FONT_SIZE_05841,
	//(unsigned char)178 /* space		*/, 0,
	//(unsigned char)179 /* space		*/, 0,
	(unsigned char)180 /* yen			*/, FONT_SIZE_05556,
	(unsigned char)181 /* u long stem	*/, FONT_SIZE_06098,
	//(unsigned char)182 /* space		*/, 0,
	//(unsigned char)183 /* space		*/, 0,
	//(unsigned char)184 /* space		*/, 0,
	//(unsigned char)185 /* space		*/, 0,
	//(unsigned char)186 /* space		*/, 0,
	(unsigned char)187 /* a little	*/, FONT_SIZE_03704,
	(unsigned char)188 /* o little	*/, FONT_SIZE_03647,
	//(unsigned char)189 /* space		*/, 0,
	(unsigned char)190 /* ae ligature	*/, FONT_SIZE_08897,
	(unsigned char)191 /* o slash		*/, FONT_SIZE_06098,
	(unsigned char)192 /* ? inverted	*/, FONT_SIZE_06098,
	(unsigned char)193 /* ! inverted	*/, FONT_SIZE_03333,
	(unsigned char)194 /* right angle	*/, FONT_SIZE_05841,
	//(unsigned char)195 /* space		*/, 0,
	(unsigned char)196 /* f fancy		*/, FONT_SIZE_05556,
	//(unsigned char)197 /* space		*/, 0,
	//(unsigned char)198 /* space		*/, 0,
	(unsigned char)199 /* <<			*/, FONT_SIZE_05556,
	(unsigned char)200 /* >>			*/, FONT_SIZE_05556,
	(unsigned char)201 /* ... ellipse	*/, FONT_SIZE_10000,
	//(unsigned char)202 /* space		*/, 0,
	(unsigned char)203 /* A grave		*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)204 /* A tilde		*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)205 /* O tilde		*/, HELVETICA_BOLD_UPPER_GOQ,
	(unsigned char)206 /* OE ligature	*/, FONT_SIZE_10000,
	(unsigned char)207 /* oe ligature	*/, FONT_SIZE_09434,
	(unsigned char)208 /* underscore	*/, FONT_SIZE_05556,
	(unsigned char)209 /* long under	*/, FONT_SIZE_10000,
	(unsigned char)210 /* dquotesopen	*/, FONT_SIZE_05000,
	(unsigned char)211 /* dquotesclose*/, FONT_SIZE_05000,
	(unsigned char)212 /* squotesopen	*/, FONT_SIZE_02778,
	(unsigned char)213 /* squotesclose*/, FONT_SIZE_02778,
	(unsigned char)214 /* divide sign	*/, FONT_SIZE_05841,
	//(unsigned char)215 /* space		*/, 0,
	(unsigned char)216 /* y umlaut	*/, HELVETICA_BOLD_LOWER_ACEKSVXY,
	(unsigned char)217 /* Y umlaut	*/, HELVETICA_BOLD_UPPER_EPSVXY,
	(unsigned char)218 /* / 45d slash	*/, FONT_SIZE_01667,
	(unsigned char)219 /* circlestar	*/, FONT_SIZE_05556,
	(unsigned char)220 /* <			*/, FONT_SIZE_03333,
	(unsigned char)221 /* >			*/, FONT_SIZE_03333,
	(unsigned char)222 /* fi ligature */, FONT_SIZE_06098,
	(unsigned char)223 /* fl ligature	*/, FONT_SIZE_06098,
	(unsigned char)224 /* bar 2 slash	*/, FONT_SIZE_05556,
	(unsigned char)225 /* . dot		*/, FONT_SIZE_02778,
	(unsigned char)226 /* , comma		*/, FONT_SIZE_02778,
	(unsigned char)227 /* ,, 2 comma	*/, FONT_SIZE_05000,
	(unsigned char)228 /* % with o	*/, FONT_SIZE_10000,
	(unsigned char)229 /* A accent	*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)230 /* E accent	*/, HELVETICA_BOLD_UPPER_EPSVXY,
	(unsigned char)231 /* A acute		*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)232 /* E umlaut	*/, HELVETICA_BOLD_UPPER_EPSVXY,
	(unsigned char)233 /* E grave		*/, HELVETICA_BOLD_UPPER_EPSVXY,
	(unsigned char)234 /* l acute		*/, FONT_SIZE_02778,
	(unsigned char)235 /* l accent	*/, FONT_SIZE_02778,
	(unsigned char)236 /* l umlaut	*/, FONT_SIZE_02778,
	(unsigned char)237 /* l grave		*/, FONT_SIZE_02778,
	(unsigned char)238 /* O acute		*/, HELVETICA_BOLD_UPPER_GOQ,
	(unsigned char)239 /* O accent	*/, HELVETICA_BOLD_UPPER_GOQ,
	//(unsigned char)240 /* space		*/, 0,
	(unsigned char)241 /* O grave		*/, HELVETICA_BOLD_UPPER_GOQ,
	(unsigned char)242 /* U acute		*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)243 /* U accent	*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)244 /* U grave		*/, HELVETICA_BOLD_UPPER_ABCDHKNRU,
	(unsigned char)245 /* / slash		*/, FONT_SIZE_02778,
	(unsigned char)246 /* ^ accent	*/, FONT_SIZE_03333,
	(unsigned char)247 /* ~ squiggle	*/, FONT_SIZE_03333,
	(unsigned char)248 /* - high dash	*/, FONT_SIZE_03333,
	(unsigned char)249 /* high u		*/, FONT_SIZE_03333,
	(unsigned char)250 /* high dot	*/, FONT_SIZE_03333,
	(unsigned char)251 /* high circle	*/, FONT_SIZE_03333,
	(unsigned char)252 /* curl left	*/, FONT_SIZE_03333,
	(unsigned char)253 /* acute		*/, FONT_SIZE_03333,
	(unsigned char)254 /* curl right	*/, FONT_SIZE_03333,
	(unsigned char)255 /* accent inv	*/, FONT_SIZE_03333,

	// English letters and symbols
	'a', HELVETICA_BOLD_LOWER_ACEKSVXY,
	'b', HELVETICA_BOLD_LOWER_BDGHNOPQU,
	'c', HELVETICA_BOLD_LOWER_ACEKSVXY,
	'd', HELVETICA_BOLD_LOWER_BDGHNOPQU,
	'e', HELVETICA_BOLD_LOWER_ACEKSVXY,
	'f', HELVETICA_BOLD_LOWER_FT,
	'g', HELVETICA_BOLD_LOWER_BDGHNOPQU,
	'h', HELVETICA_BOLD_LOWER_BDGHNOPQU,
	'i', HELVETICA_BOLD_LOWER_IJL,
	'j', HELVETICA_BOLD_LOWER_IJL,
	'k', HELVETICA_BOLD_LOWER_ACEKSVXY,
	'l', HELVETICA_BOLD_LOWER_IJL,
	'm', HELVETICA_BOLD_LOWER_M,
	'n', HELVETICA_BOLD_LOWER_BDGHNOPQU,
	'o', HELVETICA_BOLD_LOWER_BDGHNOPQU,
	'p', HELVETICA_BOLD_LOWER_BDGHNOPQU,
	'q', HELVETICA_BOLD_LOWER_BDGHNOPQU,
	'r', HELVETICA_BOLD_LOWER_R,
	's', HELVETICA_BOLD_LOWER_ACEKSVXY,
	't', HELVETICA_BOLD_LOWER_FT,
	'u', HELVETICA_BOLD_LOWER_BDGHNOPQU,
	'v', HELVETICA_BOLD_LOWER_ACEKSVXY,
	'w', HELVETICA_BOLD_LOWER_W,
	'x', HELVETICA_BOLD_LOWER_ACEKSVXY,
	'y', HELVETICA_BOLD_LOWER_ACEKSVXY,
	'z', HELVETICA_BOLD_LOWER_Z,
	'A', HELVETICA_BOLD_UPPER_ABCDHKNRU,
	'B', HELVETICA_BOLD_UPPER_ABCDHKNRU,
	'C', HELVETICA_BOLD_UPPER_ABCDHKNRU,
	'D', HELVETICA_BOLD_UPPER_ABCDHKNRU,
	'E', HELVETICA_BOLD_UPPER_EPSVXY,
	'F', HELVETICA_BOLD_UPPER_FLTZ,
	'G', HELVETICA_BOLD_UPPER_GOQ,
	'H', HELVETICA_BOLD_UPPER_ABCDHKNRU,
	'I', HELVETICA_BOLD_UPPER_I,
	'J', HELVETICA_BOLD_UPPER_J,
	'K', HELVETICA_BOLD_UPPER_ABCDHKNRU,
	'L', HELVETICA_BOLD_UPPER_FLTZ,
	'M', HELVETICA_BOLD_UPPER_M,
	'N', HELVETICA_BOLD_UPPER_ABCDHKNRU,
	'O', HELVETICA_BOLD_UPPER_GOQ,
	'P', HELVETICA_BOLD_UPPER_EPSVXY,
	'Q', HELVETICA_BOLD_UPPER_GOQ,
	'R', HELVETICA_BOLD_UPPER_ABCDHKNRU,
	'S', HELVETICA_BOLD_UPPER_EPSVXY,
	'T', HELVETICA_BOLD_UPPER_FLTZ,
	'U', HELVETICA_BOLD_UPPER_ABCDHKNRU,
	'V', HELVETICA_BOLD_UPPER_EPSVXY,
	'W', HELVETICA_BOLD_UPPER_W,
	'X', HELVETICA_BOLD_UPPER_EPSVXY,
	'Y', HELVETICA_BOLD_UPPER_EPSVXY,
	'Z', HELVETICA_BOLD_UPPER_FLTZ,
	'0', HELVETICA_BOLD_NUMBERS,
	'1', HELVETICA_BOLD_NUMBERS,
	'2', HELVETICA_BOLD_NUMBERS,
	'3', HELVETICA_BOLD_NUMBERS,
	'4', HELVETICA_BOLD_NUMBERS,
	'5', HELVETICA_BOLD_NUMBERS,
	'6', HELVETICA_BOLD_NUMBERS,
	'7', HELVETICA_BOLD_NUMBERS,
	'8', HELVETICA_BOLD_NUMBERS,
	'9', HELVETICA_BOLD_NUMBERS,
	'|', HELVETICA_BOLD_COMMADOTSLASHES_SPACE,
	',', HELVETICA_BOLD_COMMADOTSLASHES_SPACE,
	'.', HELVETICA_BOLD_COMMADOTSLASHES_SPACE,
	'\\',HELVETICA_BOLD_COMMADOTSLASHES_SPACE,
	'/', HELVETICA_BOLD_COMMADOTSLASHES_SPACE,
	':', HELVETICA_BOLD_SQUAREBRACES_ANDMORE,
	';', HELVETICA_BOLD_SQUAREBRACES_ANDMORE,
	' ', HELVETICA_BOLD_COMMADOTSLASHES_SPACE,
	'[', HELVETICA_BOLD_SQUAREBRACES_ANDMORE,
	']', HELVETICA_BOLD_SQUAREBRACES_ANDMORE,
	'`', HELVETICA_BOLD_SQUAREBRACES_ANDMORE,
	'-', HELVETICA_BOLD_SQUAREBRACES_ANDMORE,
	'(', HELVETICA_BOLD_SQUAREBRACES_ANDMORE,
	')', HELVETICA_BOLD_SQUAREBRACES_ANDMORE,
	'!', HELVETICA_BOLD_SQUAREBRACES_ANDMORE,
	'?', HELVETICA_BOLD_APPIONMARK,
	'"', HELVETICA_BOLD_QUOTES,
	'\'',HELVETICA_BOLD_SINGLEQUOTE,
	'^', HELVETICA_BOLD_CIRCUMFLEX,
	'{', HELVETICA_BOLD_CURLYBRACES,
	'}', HELVETICA_BOLD_CURLYBRACES,
	'*', HELVETICA_BOLD_STAR,
	'_', HELVETICA_BOLD_HASH_ANDMORE,
	'#', HELVETICA_BOLD_HASH_ANDMORE,
	'$', HELVETICA_BOLD_HASH_ANDMORE,
	'~', HELVETICA_BOLD_SQUIGGLE,
	'=', HELVETICA_BOLD_EQUALSIGN_ANDMORE,
	'+', HELVETICA_BOLD_EQUALSIGN_ANDMORE,
	'<', HELVETICA_BOLD_EQUALSIGN_ANDMORE,
	'>', HELVETICA_BOLD_EQUALSIGN_ANDMORE,
	'&', HELVETICA_BOLD_AMPERSAND,
	'%', HELVETICA_BOLD_PERCENT,
	'@', HELVETICA_BOLD_ATSYMBOL,
	0, 0
	};

	//SetCharCodeList( charWidths, &list );
	SetBoldCharWidths( charWidths ); // Set the list of Scaled 
}


void PDFHelveticaFont::InitItalicCharWidths()
{
	const double HELVETICA_ITALIC_LOWER_IJL = 0.2222;
	const double HELVETICA_ITALIC_LOWER_FT = 0.2778;
	const double HELVETICA_ITALIC_LOWER_R  = 0.3333;
	const double HELVETICA_ITALIC_LOWER_CKSVXYZ = 0.5;
	const double HELVETICA_ITALIC_LOWER_ABDEGHNOPQU = 0.5556;
	const double HELVETICA_ITALIC_LOWER_W = 0.7225;
	const double HELVETICA_ITALIC_LOWER_M = 0.8333;

	const double HELVETICA_ITALIC_UPPER_I = 0.2778;
	const double HELVETICA_ITALIC_UPPER_J = 0.5;
	const double HELVETICA_ITALIC_UPPER_L = 0.5556;
	const double HELVETICA_ITALIC_UPPER_CDHNRU = 0.7225;
	const double HELVETICA_ITALIC_UPPER_ABEKPSVXY = 0.6667;
	const double HELVETICA_ITALIC_UPPER_FTZ = 0.6098;
	const double HELVETICA_ITALIC_UPPER_GOQ = 0.7788;
	const double HELVETICA_ITALIC_UPPER_M = 0.8333;
	const double HELVETICA_ITALIC_UPPER_W = 0.9434;

	const double HELVETICA_ITALIC_NUMBERS = 0.5556;

	const double HELVETICA_ITALIC_BAR /* | */ = 0.2604;
	const double HELVETICA_ITALIC_SPACE /* spacechar */ = 0.25;
	const double HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE /* ,.\/[]:; */ = 0.2778;
	const double HELVETICA_ITALIC_EXCLAMATION /* ! */ = 0.2778;
	const double HELVETICA_ITALIC_APPIONMARK /* ? */ = 0.5556;
	const double HELVETICA_ITALIC_QUOTES /* " */ = 0.3556;
	const double HELVETICA_ITALIC_SINGLEQUOTE /* ' */ = 0.1914;
	const double HELVETICA_ITALIC_CIRCUMFLEX /* ^ */ = 0.469;
	const double HELVETICA_ITALIC_CURLYBRACES_ANDMORE /* {}`-() */ = 0.3342;
	const double HELVETICA_ITALIC_STAR /* * */ = 0.3894;
	const double HELVETICA_ITALIC_HASH_ANDMORE /* _#$ */ = 0.5556;
	const double HELVETICA_ITALIC_EQUALSIGN_ANDMORE /* =~+<> */ = 0.5841;
	const double HELVETICA_ITALIC_AMPERSAND /* & */ = 0.6667;
	const double HELVETICA_ITALIC_PERCENT /* % */ = 0.8897;
	const double HELVETICA_ITALIC_ATSYMBOL /* @ */ = 1.016;

	CharOnePointSizeList list;

	CharWidths charWidths[] = {
	(unsigned char)0347, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Aacute
	(unsigned char)0345, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Acirc
	(unsigned char)0313, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Agrave
	(unsigned char)0256, HELVETICA_ITALIC_UPPER_ABEKPSVXY /*????*/,	// AElig
	(unsigned char)0201, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Aring
	(unsigned char)0314, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Atilde
	(unsigned char)0200, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Auml
	(unsigned char)0202, HELVETICA_ITALIC_UPPER_CDHNRU /*????*/,	// Ccedil
	(unsigned char)0056, HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE /*????*/,	// Dot 
	(unsigned char)0203, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Eacute
	(unsigned char)0346, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Ecirc
	(unsigned char)0351, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Egrave
	(unsigned char)0350, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Euml
	(unsigned char)0352, HELVETICA_ITALIC_UPPER_I,	// Iacute
	(unsigned char)0353, HELVETICA_ITALIC_UPPER_I,	// Icirc
	(unsigned char)0355, HELVETICA_ITALIC_UPPER_I,	// Igrave
	(unsigned char)0354, HELVETICA_ITALIC_UPPER_I,	// Iuml
	(unsigned char)0204, HELVETICA_ITALIC_UPPER_CDHNRU,	// Ntilde
	(unsigned char)0356, HELVETICA_ITALIC_UPPER_GOQ,	// Oacute  
	(unsigned char)0357, HELVETICA_ITALIC_UPPER_GOQ,	// Ocirc
	(unsigned char)0361, HELVETICA_ITALIC_UPPER_GOQ,	// Ograve
	(unsigned char)0257, HELVETICA_ITALIC_UPPER_GOQ,	// Oslash
	(unsigned char)0315, HELVETICA_ITALIC_UPPER_GOQ,	// Otilde
	(unsigned char)0235, HELVETICA_ITALIC_UPPER_GOQ,	// Ouml
	(unsigned char)0120, HELVETICA_ITALIC_UPPER_GOQ /*????*/,	// THORN, becomes 'standard P'
	(unsigned char)0362, HELVETICA_ITALIC_UPPER_CDHNRU,	// Uacute
	(unsigned char)0363, HELVETICA_ITALIC_UPPER_CDHNRU,	// Ucirc
	(unsigned char)0364, HELVETICA_ITALIC_UPPER_CDHNRU,	// Ugrave
	(unsigned char)0206, HELVETICA_ITALIC_UPPER_CDHNRU,	// Uuml
	(unsigned char)0131, HELVETICA_ITALIC_UPPER_ABEKPSVXY,	// Yacute, becomes 'standard Y'
	(unsigned char)0207, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// aacute
	(unsigned char)0235, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// acute
	(unsigned char)0235, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// acirc
	(unsigned char)0276, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// aelig
	(unsigned char)0210, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// agrave
	(unsigned char)0214, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// aring
	(unsigned char)0235, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// atilde
	(unsigned char)0212, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// auml
	(unsigned char)0174, HELVETICA_ITALIC_BAR,	// brvbar, becomes standard bar '|'
	(unsigned char)0215, HELVETICA_ITALIC_LOWER_CKSVXYZ,	// ccedil
	(unsigned char)0374, HELVETICA_ITALIC_LOWER_CKSVXYZ,	// cedil
	(unsigned char)0242, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// cent
	(unsigned char)0251, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// copy
	(unsigned char)0333, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU /*????*/,	// curren
	(unsigned char)0241, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU /*????*/,	// deg 
	(unsigned char)0216, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// eacute 
	(unsigned char)0220, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// ecirc 
	(unsigned char)0217, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// egrave
	(unsigned char)0144, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// eth, problem!! becomes "d' as this is what it loks most like?
	(unsigned char)0221, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// euml
	(unsigned char)0222, HELVETICA_ITALIC_LOWER_IJL,	// iacute
	(unsigned char)0224, HELVETICA_ITALIC_LOWER_IJL,	// icirc
	(unsigned char)0223, HELVETICA_ITALIC_LOWER_IJL,	// igrave
	(unsigned char)0225, HELVETICA_ITALIC_LOWER_IJL,	// iuml
	(unsigned char)0370, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU /*????*/,	// macr
	(unsigned char)0341, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU /*????*/,	// middot, using 'period centered'
	(unsigned char)0226, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// ntilde 
	(unsigned char)0227, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// oacute  
	(unsigned char)0231, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// ocirc  
	(unsigned char)0230, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// ograve
	(unsigned char)0273, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// ordf
	(unsigned char)0274, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// ordm
	(unsigned char)0277, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// oslash
	(unsigned char)0233, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// otilde  
	(unsigned char)0232, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// ouml
	(unsigned char)0246, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU /*????*/,	// para
	(unsigned char)0053, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU /*????*/,	// plusmn
	(unsigned char)0243, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU /*????*/,	// pound, bcomes 'sterling'
	(unsigned char)0250, HELVETICA_ITALIC_ATSYMBOL /*????*/,	// reg 
	(unsigned char)0244, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU /*????*/,	// sect
	(unsigned char)0234, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU /*????*/,  
	(unsigned char)0236, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// ucirc  
	(unsigned char)0235, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// ugrave  
	(unsigned char)0237, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// uml
	(unsigned char)0237, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,	// uuml
	(unsigned char)0264, HELVETICA_ITALIC_HASH_ANDMORE,	// yen 
	(unsigned char)0330, HELVETICA_ITALIC_LOWER_CKSVXYZ,	// yuml 
	(unsigned char)0235, HELVETICA_ITALIC_LOWER_CKSVXYZ,	// code = xxx; break325";
	(unsigned char)0252, HELVETICA_ITALIC_ATSYMBOL,	// trademark

	// Foreign letters and symbols
	(unsigned char)130 /* C cedil		*/, HELVETICA_ITALIC_UPPER_CDHNRU,
	(unsigned char)131 /* E egrave	*/, HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	(unsigned char)132 /* N tilde		*/, HELVETICA_ITALIC_UPPER_CDHNRU,
	(unsigned char)133 /* 0 umlaut	*/, HELVETICA_ITALIC_UPPER_GOQ,
	(unsigned char)134 /* U umlaut	*/, HELVETICA_ITALIC_UPPER_CDHNRU,
	(unsigned char)135 /* a acute		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)136 /* a grave		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)137 /* a circum	*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)138 /* a umlaut	*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)139 /* a tilde		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)140 /* a ring		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)141 /* c cedil		*/, HELVETICA_ITALIC_LOWER_CKSVXYZ,
	(unsigned char)142 /* e acute		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)143 /* e grave		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)144 /* e circum	*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)145 /* e umlaut	*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)146 /* l acute		*/, HELVETICA_ITALIC_LOWER_FT /* for foreign i */,
	(unsigned char)147 /* l grave		*/, HELVETICA_ITALIC_LOWER_FT /* for foreign i */,
	(unsigned char)148 /* l circum	*/, HELVETICA_ITALIC_LOWER_FT /* for foreign i */,
	(unsigned char)149 /* l umlaut	*/, HELVETICA_ITALIC_LOWER_FT /* for foreign i */,
	(unsigned char)150 /* n titde		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)151 /* o acute		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)152 /* o grave		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)153 /* o circum	*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)154 /* o umlaut	*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)155 /* o titde		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)156 /* u acute		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)157 /* u grave		*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)158 /* u circum	*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)159 /* u umlaut	*/, HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	(unsigned char)160 /* cross		*/, FONT_SIZE_05556,
	(unsigned char)161 /* ring		*/, FONT_SIZE_04000,
	(unsigned char)162 /* cent		*/, FONT_SIZE_05556,
	(unsigned char)163 /* pound		*/, FONT_SIZE_05556,
	(unsigned char)164 /* double S	*/, FONT_SIZE_05556,
	(unsigned char)165 /* dot			*/, FONT_SIZE_03501,
	(unsigned char)166 /* para end	*/, FONT_SIZE_05376,
	(unsigned char)167 /* german SS	*/, FONT_SIZE_06098,
	(unsigned char)168 /* register	*/, FONT_SIZE_07375,
	(unsigned char)169 /* copyright	*/, FONT_SIZE_07375,
	(unsigned char)170 /* trademark	*/, FONT_SIZE_10000,
	(unsigned char)171 /* apostrophe	*/, FONT_SIZE_03333,
	(unsigned char)172 /* umlaut		*/, FONT_SIZE_03333,
	//(unsigned char)173 /* space		*/, 0,
	(unsigned char)174 /* AE ligature	*/, FONT_SIZE_10000,
	(unsigned char)175 /* O slash		*/, FONT_SIZE_07788,
	//(unsigned char)176 /* space		*/, 0,
	(unsigned char)177 /* plusminus	*/, FONT_SIZE_05841,
	//(unsigned char)178 /* space		*/, 0,
	//(unsigned char)179 /* space		*/, 0,
	(unsigned char)180 /* yen			*/, FONT_SIZE_05556,
	(unsigned char)181 /* u long stem	*/, FONT_SIZE_05787,
	//(unsigned char)182 /* space		*/, 0,
	//(unsigned char)183 /* space		*/, 0,
	//(unsigned char)184 /* space		*/, 0,
	//(unsigned char)185 /* space		*/, 0,
	//(unsigned char)186 /* space		*/, 0,
	(unsigned char)187 /* a little	*/, FONT_SIZE_03704,
	(unsigned char)188 /* o little	*/, FONT_SIZE_03647,
	//(unsigned char)189 /* space		*/, 0,
	(unsigned char)190 /* ae ligature	*/, FONT_SIZE_08897,
	(unsigned char)191 /* o slash		*/, FONT_SIZE_06098,
	(unsigned char)192 /* ? inverted	*/, FONT_SIZE_06098,
	(unsigned char)193 /* ! inverted	*/, FONT_SIZE_03333,
	(unsigned char)194 /* right angle	*/, FONT_SIZE_05841,
	//(unsigned char)195 /* space		*/, 0,
	(unsigned char)196 /* f fancy		*/, FONT_SIZE_05556,
	//(unsigned char)197 /* space		*/, 0,
	//(unsigned char)198 /* space		*/, 0,
	(unsigned char)199 /* <<			*/, FONT_SIZE_05556,
	(unsigned char)200 /* >>			*/, FONT_SIZE_05556,
	(unsigned char)201 /* ... ellipse	*/, FONT_SIZE_10000,
	//(unsigned char)202 /* space		*/, 0,
	(unsigned char)203 /* A grave		*/, HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	(unsigned char)204 /* A tilde		*/, HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	(unsigned char)205 /* O tilde		*/, HELVETICA_ITALIC_UPPER_GOQ,
	(unsigned char)206 /* OE ligature	*/, FONT_SIZE_10000,
	(unsigned char)207 /* oe ligature	*/, FONT_SIZE_09434,
	(unsigned char)208 /* underscore	*/, FONT_SIZE_05556,
	(unsigned char)209 /* long under	*/, FONT_SIZE_10000,
	(unsigned char)210 /* dquotesopen	*/, FONT_SIZE_03333,
	(unsigned char)211 /* dquotesclose*/, FONT_SIZE_03333,
	(unsigned char)212 /* squotesopen	*/, FONT_SIZE_02222,
	(unsigned char)213 /* squotesclose*/, FONT_SIZE_02222,
	(unsigned char)214 /* divide sign	*/, FONT_SIZE_05841,
	//(unsigned char)215 /* space		*/, 0,
	(unsigned char)216 /* y umlaut	*/, HELVETICA_ITALIC_LOWER_CKSVXYZ,
	(unsigned char)217 /* Y umlaut	*/, HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	(unsigned char)218 /* / 45d slash	*/, FONT_SIZE_01667,
	(unsigned char)219 /* circlestar	*/, FONT_SIZE_05556,
	(unsigned char)220 /* <			*/, FONT_SIZE_03333,
	(unsigned char)221 /* >			*/, FONT_SIZE_03333,
	(unsigned char)222 /* fi ligature */, FONT_SIZE_05000,
	(unsigned char)223 /* fl ligature	*/, FONT_SIZE_05000,
	(unsigned char)224 /* bar 2 slash	*/, FONT_SIZE_05556,
	(unsigned char)225 /* . dot		*/, FONT_SIZE_02778,
	(unsigned char)226 /* , comma		*/, FONT_SIZE_02222,
	(unsigned char)227 /* ,, 2 comma	*/, FONT_SIZE_03333,
	(unsigned char)228 /* % with o	*/, FONT_SIZE_10000,
	(unsigned char)229 /* A accent	*/, HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	(unsigned char)230 /* E accent	*/, HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	(unsigned char)231 /* A acute		*/, HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	(unsigned char)232 /* E umlaut	*/, HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	(unsigned char)233 /* E grave		*/, HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	(unsigned char)234 /* l acute		*/, FONT_SIZE_02778,
	(unsigned char)235 /* l accent	*/, FONT_SIZE_02778,
	(unsigned char)236 /* l umlaut	*/, FONT_SIZE_02778,
	(unsigned char)237 /* l grave		*/, FONT_SIZE_02778,
	(unsigned char)238 /* O acute		*/, HELVETICA_ITALIC_UPPER_GOQ,
	(unsigned char)239 /* O accent	*/, HELVETICA_ITALIC_UPPER_GOQ,
	//(unsigned char)240 /* space		*/, 0,
	(unsigned char)241 /* O grave		*/, HELVETICA_ITALIC_UPPER_GOQ,
	(unsigned char)242 /* U acute		*/, HELVETICA_ITALIC_UPPER_CDHNRU,
	(unsigned char)243 /* U accent	*/, HELVETICA_ITALIC_UPPER_CDHNRU,
	(unsigned char)244 /* U grave		*/, HELVETICA_ITALIC_UPPER_CDHNRU,
	(unsigned char)245 /* / slash		*/, FONT_SIZE_02778,
	(unsigned char)246 /* ^ accent	*/, FONT_SIZE_03333,
	(unsigned char)247 /* ~ squiggle	*/, FONT_SIZE_03333,
	(unsigned char)248 /* - high dash	*/, FONT_SIZE_03333,
	(unsigned char)249 /* high u		*/, FONT_SIZE_03333,
	(unsigned char)250 /* high dot	*/, FONT_SIZE_03333,
	(unsigned char)251 /* high circle	*/, FONT_SIZE_03333,
	(unsigned char)252 /* curl left	*/, FONT_SIZE_03333,
	(unsigned char)253 /* acute		*/, FONT_SIZE_03333,
	(unsigned char)254 /* curl right	*/, FONT_SIZE_03333,
	(unsigned char)255 /* accent inv	*/, FONT_SIZE_03333,

	// English letters and symbols
	'a', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'b', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'c', HELVETICA_ITALIC_LOWER_CKSVXYZ,
	'd', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'e', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'f', HELVETICA_ITALIC_LOWER_FT,
	'g', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'h', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'i', HELVETICA_ITALIC_LOWER_IJL,
	'j', HELVETICA_ITALIC_LOWER_IJL,
	'k', HELVETICA_ITALIC_LOWER_CKSVXYZ,
	'l', HELVETICA_ITALIC_LOWER_IJL,
	'm', HELVETICA_ITALIC_LOWER_M,
	'n', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'o', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'p', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'q', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'r', HELVETICA_ITALIC_LOWER_R,
	's', HELVETICA_ITALIC_LOWER_CKSVXYZ,
	't', HELVETICA_ITALIC_LOWER_FT,
	'u', HELVETICA_ITALIC_LOWER_ABDEGHNOPQU,
	'v', HELVETICA_ITALIC_LOWER_CKSVXYZ,
	'w', HELVETICA_ITALIC_LOWER_W,
	'x', HELVETICA_ITALIC_LOWER_CKSVXYZ,
	'y', HELVETICA_ITALIC_LOWER_CKSVXYZ,
	'z', HELVETICA_ITALIC_LOWER_CKSVXYZ,
	'A', HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	'B', HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	'C', HELVETICA_ITALIC_UPPER_CDHNRU,
	'D', HELVETICA_ITALIC_UPPER_CDHNRU,
	'E', HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	'F', HELVETICA_ITALIC_UPPER_FTZ,
	'G', HELVETICA_ITALIC_UPPER_GOQ,
	'H', HELVETICA_ITALIC_UPPER_CDHNRU,
	'I', HELVETICA_ITALIC_UPPER_I,
	'J', HELVETICA_ITALIC_UPPER_J,
	'K', HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	'L', HELVETICA_ITALIC_UPPER_L,
	'M', HELVETICA_ITALIC_UPPER_M,
	'N', HELVETICA_ITALIC_UPPER_CDHNRU,
	'O', HELVETICA_ITALIC_UPPER_GOQ,
	'P', HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	'Q', HELVETICA_ITALIC_UPPER_GOQ,
	'R', HELVETICA_ITALIC_UPPER_CDHNRU,
	'S', HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	'T', HELVETICA_ITALIC_UPPER_FTZ,
	'U', HELVETICA_ITALIC_UPPER_CDHNRU,
	'V', HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	'W', HELVETICA_ITALIC_UPPER_W,
	'X', HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	'Y', HELVETICA_ITALIC_UPPER_ABEKPSVXY,
	'Z', HELVETICA_ITALIC_UPPER_FTZ,
	'0', HELVETICA_ITALIC_NUMBERS,
	'1', HELVETICA_ITALIC_NUMBERS,
	'2', HELVETICA_ITALIC_NUMBERS,
	'3', HELVETICA_ITALIC_NUMBERS,
	'4', HELVETICA_ITALIC_NUMBERS,
	'5', HELVETICA_ITALIC_NUMBERS,
	'6', HELVETICA_ITALIC_NUMBERS,
	'7', HELVETICA_ITALIC_NUMBERS,
	'8', HELVETICA_ITALIC_NUMBERS,
	'9', HELVETICA_ITALIC_NUMBERS,
	'|', HELVETICA_ITALIC_BAR,
	',', HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE,
	'.', HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE,
	'\\',HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE,
	'/', HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE,
	':', HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE,
	';', HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE,
	' ', HELVETICA_ITALIC_SPACE,
	'[', HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE,
	']', HELVETICA_ITALIC_COMMADOTSLASHES_SQUAREBRACES_ANDMORE,
	'`', HELVETICA_ITALIC_CURLYBRACES_ANDMORE,
	'-', HELVETICA_ITALIC_CURLYBRACES_ANDMORE,
	'(', HELVETICA_ITALIC_CURLYBRACES_ANDMORE,
	')', HELVETICA_ITALIC_CURLYBRACES_ANDMORE,
	'!', HELVETICA_ITALIC_EXCLAMATION,
	'?', HELVETICA_ITALIC_APPIONMARK,
	'"', HELVETICA_ITALIC_QUOTES,
	'\'',HELVETICA_ITALIC_SINGLEQUOTE,
	'^', HELVETICA_ITALIC_CIRCUMFLEX,
	'{', HELVETICA_ITALIC_CURLYBRACES_ANDMORE,
	'}', HELVETICA_ITALIC_CURLYBRACES_ANDMORE,
	'*', HELVETICA_ITALIC_STAR,
	'_', HELVETICA_ITALIC_HASH_ANDMORE,
	'#', HELVETICA_ITALIC_HASH_ANDMORE,
	'$', HELVETICA_ITALIC_HASH_ANDMORE,
	'~', HELVETICA_ITALIC_EQUALSIGN_ANDMORE,
	'=', HELVETICA_ITALIC_EQUALSIGN_ANDMORE,
	'+', HELVETICA_ITALIC_EQUALSIGN_ANDMORE,
	'<', HELVETICA_ITALIC_EQUALSIGN_ANDMORE,
	'>', HELVETICA_ITALIC_EQUALSIGN_ANDMORE,
	'&', HELVETICA_ITALIC_AMPERSAND,
	'%', HELVETICA_ITALIC_PERCENT,
	'@', HELVETICA_ITALIC_ATSYMBOL,
	0, 0
	};

	//SetCharCodeList( charWidths, &list );
	SetItalicCharWidths( charWidths );
}

void PDFHelveticaFont::InitBoldItalicCharWidths()
{
	const double HELVETICA_BOLDITALIC_LOWER_ACEKSVXY = 0.5556;
	const double HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU = 0.6098;
	const double HELVETICA_BOLDITALIC_LOWER_FT = 0.3333;
	const double HELVETICA_BOLDITALIC_LOWER_IJL = 0.2778;
	const double HELVETICA_BOLDITALIC_LOWER_M = 0.8897;
	const double HELVETICA_BOLDITALIC_LOWER_R = 0.3888;
	const double HELVETICA_BOLDITALIC_LOWER_Z = 0.5;
	const double HELVETICA_BOLDITALIC_LOWER_W = 0.7788;

	const double HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU = 0.7225;
	const double HELVETICA_BOLDITALIC_UPPER_EPSVXY = 0.6667;
	const double HELVETICA_BOLDITALIC_UPPER_FLTZ = 0.6098;
	const double HELVETICA_BOLDITALIC_UPPER_GOQ = 0.7788;
	const double HELVETICA_BOLDITALIC_UPPER_I = 0.2784;
	const double HELVETICA_BOLDITALIC_UPPER_J = 0.5556;
	const double HELVETICA_BOLDITALIC_UPPER_M = FONT_SIZE_08333;
	const double HELVETICA_BOLDITALIC_UPPER_W = 0.9467;

	const double HELVETICA_BOLDITALIC_NUMBERS = 0.5556;

	const double HELVETICA_BOLDITALIC_BAR /* | */ = 0.2801;
	const double HELVETICA_BOLDITALIC_COMMADOTSLASH_SPACE /* ,.\/spacechar */ = 0.2778;
	const double HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE /* []`-()!:; */ = 0.3333;
	const double HELVETICA_BOLDITALIC_APPIONMARK /* ? */ = 0.6157;
	const double HELVETICA_BOLDITALIC_QUOTES /* " */ = FONT_SIZE_04753;
	const double HELVETICA_BOLDITALIC_SINGLEQUOTE /* ' */ = FONT_SIZE_02444;
	const double HELVETICA_BOLDITALIC_CIRCUMFLEX /* ^ */ = 0.5841;
	const double HELVETICA_BOLDITALIC_CURLYBRACES /* {} */ = 0.3888;
	const double HELVETICA_BOLDITALIC_STAR /* * */ = 0.3894;
	const double HELVETICA_BOLDITALIC_HASH_ANDMORE /* _#$ */ = 0.5556;
	const double HELVETICA_BOLDITALIC_EQUALSIGN_ANDMORE /* =~+<> */ = 0.5841;
	const double HELVETICA_BOLDITALIC_AMPERSAND /* & */ = 0.7225;
	const double HELVETICA_BOLDITALIC_PERCENT /* % */ = 0.8897;
	const double HELVETICA_BOLDITALIC_ATSYMBOL /* @ */ = 0.9756;

	CharOnePointSizeList list;

	CharWidths charWidths[] = {
	(unsigned char)0347, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Aacute
	(unsigned char)0345, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Acirc
	(unsigned char)0313, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Agrave
	(unsigned char)0256, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU /*????*/,	// AElig
	(unsigned char)0201, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Aring
	(unsigned char)0314, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Atilde
	(unsigned char)0200, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Auml
	(unsigned char)0202, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU /*????*/,	// Ccedil
	(unsigned char)0056, HELVETICA_BOLDITALIC_COMMADOTSLASH_SPACE /*????*/,	// Dot 
	(unsigned char)0203, HELVETICA_BOLDITALIC_UPPER_EPSVXY,	// Eacute
	(unsigned char)0346, HELVETICA_BOLDITALIC_UPPER_EPSVXY,	// Ecirc
	(unsigned char)0351, HELVETICA_BOLDITALIC_UPPER_EPSVXY,	// Egrave
	(unsigned char)0350, HELVETICA_BOLDITALIC_UPPER_EPSVXY,	// Euml
	(unsigned char)0352, HELVETICA_BOLDITALIC_UPPER_I,	// Iacute
	(unsigned char)0353, HELVETICA_BOLDITALIC_UPPER_I,	// Icirc
	(unsigned char)0355, HELVETICA_BOLDITALIC_UPPER_I,	// Igrave
	(unsigned char)0354, HELVETICA_BOLDITALIC_UPPER_I,	// Iuml
	(unsigned char)0204, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Ntilde
	(unsigned char)0356, HELVETICA_BOLDITALIC_UPPER_GOQ,	// Oacute  
	(unsigned char)0357, HELVETICA_BOLDITALIC_UPPER_GOQ,	// Ocirc
	(unsigned char)0361, HELVETICA_BOLDITALIC_UPPER_GOQ,	// Ograve
	(unsigned char)0257, HELVETICA_BOLDITALIC_UPPER_GOQ,	// Oslash
	(unsigned char)0315, HELVETICA_BOLDITALIC_UPPER_GOQ,	// Otilde
	(unsigned char)0235, HELVETICA_BOLDITALIC_UPPER_GOQ,	// Ouml
	(unsigned char)0120, HELVETICA_BOLDITALIC_UPPER_EPSVXY /*????*/,	// THORN, becomes 'standard P'
	(unsigned char)0362, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Uacute
	(unsigned char)0363, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Ucirc
	(unsigned char)0364, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Ugrave
	(unsigned char)0206, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,	// Uuml
	(unsigned char)0131, HELVETICA_BOLDITALIC_UPPER_EPSVXY,	// Yacute, becomes 'standard Y'
	(unsigned char)0207, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// aacute
	(unsigned char)0235, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// acute
	(unsigned char)0235, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// acirc
	(unsigned char)0276, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// aelig
	(unsigned char)0210, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// agrave
	(unsigned char)0214, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// aring
	(unsigned char)0235, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// atilde
	(unsigned char)0212, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// auml
	(unsigned char)0174, HELVETICA_BOLDITALIC_BAR,	// brvbar, becomes standard bar '|'
	(unsigned char)0215, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// ccedil
	(unsigned char)0374, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// cedil
	(unsigned char)0242, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// cent
	(unsigned char)0251, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// copy
	(unsigned char)0333, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY /*????*/,	// curren
	(unsigned char)0241, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY /*????*/,	// deg 
	(unsigned char)0216, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// eacute 
	(unsigned char)0220, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// ecirc 
	(unsigned char)0217, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// egrave
	(unsigned char)0144, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// eth, problem!! becomes "d' as this is what it loks most like?
	(unsigned char)0221, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// euml
	(unsigned char)0222, HELVETICA_BOLDITALIC_LOWER_IJL,	// iacute
	(unsigned char)0224, HELVETICA_BOLDITALIC_LOWER_IJL,	// icirc
	(unsigned char)0223, HELVETICA_BOLDITALIC_LOWER_IJL,	// igrave
	(unsigned char)0225, HELVETICA_BOLDITALIC_LOWER_IJL,	// iuml
	(unsigned char)0370, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY /*????*/,	// macr
	(unsigned char)0341, HELVETICA_BOLDITALIC_COMMADOTSLASH_SPACE /*????*/,	// middot, using 'period centered'
	(unsigned char)0226, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// ntilde 
	(unsigned char)0227, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// oacute  
	(unsigned char)0231, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// ocirc  
	(unsigned char)0230, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// ograve
	(unsigned char)0273, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// ordf
	(unsigned char)0274, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// ordm
	(unsigned char)0277, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// oslash
	(unsigned char)0233, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// otilde  
	(unsigned char)0232, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// ouml
	(unsigned char)0246, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY /*????*/,	// para
	(unsigned char)0053, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY /*????*/,	// plusmn
	(unsigned char)0243, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY /*????*/,	// pound, bcomes 'sterling'
	(unsigned char)0250, HELVETICA_BOLDITALIC_ATSYMBOL /*????*/,	// reg 
	(unsigned char)0244, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY /*????*/,	// sect
	(unsigned char)0234, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY /*????*/,  
	(unsigned char)0236, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// ucirc  
	(unsigned char)0235, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// ugrave  
	(unsigned char)0237, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// uml
	(unsigned char)0237, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,	// uuml
	(unsigned char)0264, HELVETICA_BOLDITALIC_HASH_ANDMORE,	// yen 
	(unsigned char)0330, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// yuml 
	(unsigned char)0235, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,	// code = xxx; break325";
	(unsigned char)0252, HELVETICA_BOLDITALIC_ATSYMBOL,	// trademark

	// Foreign letters and symbols
	(unsigned char)130 /* C cedil		*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)131 /* E egrave	*/, HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	(unsigned char)132 /* N tilde		*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)133 /* 0 umlaut	*/, HELVETICA_BOLDITALIC_UPPER_GOQ,
	(unsigned char)134 /* U umlaut	*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)135 /* a acute		*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)136 /* a grave		*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)137 /* a circum	*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)138 /* a umlaut	*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)139 /* a tilde		*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)140 /* a ring		*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)141 /* c cedil		*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)142 /* e acute		*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)143 /* e grave		*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)144 /* e circum	*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)145 /* e umlaut	*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)146 /* l acute		*/, HELVETICA_BOLDITALIC_LOWER_IJL /* for foreign i */,
	(unsigned char)147 /* l grave		*/, HELVETICA_BOLDITALIC_LOWER_IJL /* for foreign i */,
	(unsigned char)148 /* l circum	*/, HELVETICA_BOLDITALIC_LOWER_IJL /* for foreign i */,
	(unsigned char)149 /* l umlaut	*/, HELVETICA_BOLDITALIC_LOWER_IJL /* for foreign i */,
	(unsigned char)150 /* n titde		*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)151 /* o acute		*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)152 /* o grave		*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)153 /* o circum	*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)154 /* o umlaut	*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)155 /* o titde		*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)156 /* u acute		*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)157 /* u grave		*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)158 /* u circum	*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)159 /* u umlaut	*/, HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	(unsigned char)160 /* cross		*/, FONT_SIZE_05556,
	(unsigned char)161 /* ring		*/, FONT_SIZE_04000,
	(unsigned char)162 /* cent		*/, FONT_SIZE_05556,
	(unsigned char)163 /* pound		*/, FONT_SIZE_05556,
	(unsigned char)164 /* double S	*/, FONT_SIZE_05556,
	(unsigned char)165 /* dot			*/, FONT_SIZE_03501,
	(unsigned char)166 /* para end	*/, FONT_SIZE_05556,
	(unsigned char)167 /* german SS	*/, FONT_SIZE_06098,
	(unsigned char)168 /* register	*/, FONT_SIZE_07375,
	(unsigned char)169 /* copyright	*/, FONT_SIZE_07375,
	(unsigned char)170 /* trademark	*/, FONT_SIZE_10000,
	(unsigned char)171 /* apostrophe	*/, FONT_SIZE_03333,
	(unsigned char)172 /* umlaut		*/, FONT_SIZE_03333,
	//(unsigned char)173 /* space		*/, 0,
	(unsigned char)174 /* AE ligature	*/, FONT_SIZE_10000,
	(unsigned char)175 /* O slash		*/, FONT_SIZE_07788,
	//(unsigned char)176 /* space		*/, 0,
	(unsigned char)177 /* plusminus	*/, FONT_SIZE_05841,
	//(unsigned char)178 /* space		*/, 0,
	//(unsigned char)179 /* space		*/, 0,
	(unsigned char)180 /* yen			*/, FONT_SIZE_05556,
	(unsigned char)181 /* u long stem	*/, FONT_SIZE_06098,
	//(unsigned char)182 /* space		*/, 0,
	//(unsigned char)183 /* space		*/, 0,
	//(unsigned char)184 /* space		*/, 0,
	//(unsigned char)185 /* space		*/, 0,
	//(unsigned char)186 /* space		*/, 0,
	(unsigned char)187 /* a little	*/, FONT_SIZE_03704,
	(unsigned char)188 /* o little	*/, FONT_SIZE_03647,
	//(unsigned char)189 /* space		*/, 0,
	(unsigned char)190 /* ae ligature	*/, FONT_SIZE_08897,
	(unsigned char)191 /* o slash		*/, FONT_SIZE_06098,
	(unsigned char)192 /* ? inverted	*/, FONT_SIZE_06098,
	(unsigned char)193 /* ! inverted	*/, FONT_SIZE_03333,
	(unsigned char)194 /* right angle	*/, FONT_SIZE_05841,
	//(unsigned char)195 /* space		*/, 0,
	(unsigned char)196 /* f fancy		*/, FONT_SIZE_05556,
	//(unsigned char)197 /* space		*/, 0,
	//(unsigned char)198 /* space		*/, 0,
	(unsigned char)199 /* <<			*/, FONT_SIZE_05556,
	(unsigned char)200 /* >>			*/, FONT_SIZE_05556,
	(unsigned char)201 /* ... ellipse	*/, FONT_SIZE_10000,
	//(unsigned char)202 /* space		*/, 0,
	(unsigned char)203 /* A grave		*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)204 /* A tilde		*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)205 /* O tilde		*/, HELVETICA_BOLDITALIC_UPPER_GOQ,
	(unsigned char)206 /* OE ligature	*/, FONT_SIZE_10000,
	(unsigned char)207 /* oe ligature	*/, FONT_SIZE_09434,
	(unsigned char)208 /* underscore	*/, FONT_SIZE_05556,
	(unsigned char)209 /* long under	*/, FONT_SIZE_10000,
	(unsigned char)210 /* dquotesopen	*/, FONT_SIZE_05000,
	(unsigned char)211 /* dquotesclose*/, FONT_SIZE_05000,
	(unsigned char)212 /* squotesopen	*/, FONT_SIZE_02857,
	(unsigned char)213 /* squotesclose*/, FONT_SIZE_02857,
	(unsigned char)214 /* divide sign	*/, FONT_SIZE_05841,
	//(unsigned char)215 /* space		*/, 0,
	(unsigned char)216 /* y umlaut	*/, HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	(unsigned char)217 /* Y umlaut	*/, HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	(unsigned char)218 /* / 45d slash	*/, FONT_SIZE_01667,
	(unsigned char)219 /* circlestar	*/, FONT_SIZE_05556,
	(unsigned char)220 /* <			*/, FONT_SIZE_03333,
	(unsigned char)221 /* >			*/, FONT_SIZE_03333,
	(unsigned char)222 /* fi ligature */, FONT_SIZE_06098,
	(unsigned char)223 /* fl ligature	*/, FONT_SIZE_06098,
	(unsigned char)224 /* bar 2 slash	*/, FONT_SIZE_05556,
	(unsigned char)225 /* . dot		*/, FONT_SIZE_02778,
	(unsigned char)226 /* , comma		*/, FONT_SIZE_02778,
	(unsigned char)227 /* ,, 2 comma	*/, FONT_SIZE_05000,
	(unsigned char)228 /* % with o	*/, FONT_SIZE_10000,
	(unsigned char)229 /* A accent	*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)230 /* E accent	*/, HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	(unsigned char)231 /* A acute		*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)232 /* E umlaut	*/, HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	(unsigned char)233 /* E grave		*/, HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	(unsigned char)234 /* l acute		*/, FONT_SIZE_02778,
	(unsigned char)235 /* l accent	*/, FONT_SIZE_02778,
	(unsigned char)236 /* l umlaut	*/, FONT_SIZE_02778,
	(unsigned char)237 /* l grave		*/, FONT_SIZE_02778,
	(unsigned char)238 /* O acute		*/, HELVETICA_BOLDITALIC_UPPER_GOQ,
	(unsigned char)239 /* O accent	*/, HELVETICA_BOLDITALIC_UPPER_GOQ,
	//(unsigned char)240 /* space		*/, 0,
	(unsigned char)241 /* O grave		*/, HELVETICA_BOLDITALIC_UPPER_GOQ,
	(unsigned char)242 /* U acute		*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)243 /* U accent	*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)244 /* U grave		*/, HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	(unsigned char)245 /* / slash		*/, FONT_SIZE_02778,
	(unsigned char)246 /* ^ accent	*/, FONT_SIZE_03333,
	(unsigned char)247 /* ~ squiggle	*/, FONT_SIZE_03333,
	(unsigned char)248 /* - high dash	*/, FONT_SIZE_03333,
	(unsigned char)249 /* high u		*/, FONT_SIZE_03333,
	(unsigned char)250 /* high dot	*/, FONT_SIZE_03333,
	(unsigned char)251 /* high circle	*/, FONT_SIZE_03333,
	(unsigned char)252 /* curl left	*/, FONT_SIZE_03333,
	(unsigned char)253 /* acute		*/, FONT_SIZE_03333,
	(unsigned char)254 /* curl right	*/, FONT_SIZE_03333,
	(unsigned char)255 /* accent inv	*/, FONT_SIZE_03333,

	// English letters and symbols
	'a', HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	'b', HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	'c', HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	'd', HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	'e', HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	'f', HELVETICA_BOLDITALIC_LOWER_FT,
	'g', HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	'h', HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	'i', HELVETICA_BOLDITALIC_LOWER_IJL,
	'j', HELVETICA_BOLDITALIC_LOWER_IJL,
	'k', HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	'l', HELVETICA_BOLDITALIC_LOWER_IJL,
	'm', HELVETICA_BOLDITALIC_LOWER_M,
	'n', HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	'o', HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	'p', HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	'q', HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	'r', HELVETICA_BOLDITALIC_LOWER_R,
	's', HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	't', HELVETICA_BOLDITALIC_LOWER_FT,
	'u', HELVETICA_BOLDITALIC_LOWER_BDGHNOPQU,
	'v', HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	'w', HELVETICA_BOLDITALIC_LOWER_W,
	'x', HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	'y', HELVETICA_BOLDITALIC_LOWER_ACEKSVXY,
	'z', HELVETICA_BOLDITALIC_LOWER_Z,
	'A', HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	'B', HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	'C', HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	'D', HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	'E', HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	'F', HELVETICA_BOLDITALIC_UPPER_FLTZ,
	'G', HELVETICA_BOLDITALIC_UPPER_GOQ,
	'H', HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	'I', HELVETICA_BOLDITALIC_UPPER_I,
	'J', HELVETICA_BOLDITALIC_UPPER_J,
	'K', HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	'L', HELVETICA_BOLDITALIC_UPPER_FLTZ,
	'M', HELVETICA_BOLDITALIC_UPPER_M,
	'N', HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	'O', HELVETICA_BOLDITALIC_UPPER_GOQ,
	'P', HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	'Q', HELVETICA_BOLDITALIC_UPPER_GOQ,
	'R', HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	'S', HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	'T', HELVETICA_BOLDITALIC_UPPER_FLTZ,
	'U', HELVETICA_BOLDITALIC_UPPER_ABCDHKNRU,
	'V', HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	'W', HELVETICA_BOLDITALIC_UPPER_W,
	'X', HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	'Y', HELVETICA_BOLDITALIC_UPPER_EPSVXY,
	'Z', HELVETICA_BOLDITALIC_UPPER_FLTZ,
	'0', HELVETICA_BOLDITALIC_NUMBERS,
	'1', HELVETICA_BOLDITALIC_NUMBERS,
	'2', HELVETICA_BOLDITALIC_NUMBERS,
	'3', HELVETICA_BOLDITALIC_NUMBERS,
	'4', HELVETICA_BOLDITALIC_NUMBERS,
	'5', HELVETICA_BOLDITALIC_NUMBERS,
	'6', HELVETICA_BOLDITALIC_NUMBERS,
	'7', HELVETICA_BOLDITALIC_NUMBERS,
	'8', HELVETICA_BOLDITALIC_NUMBERS,
	'9', HELVETICA_BOLDITALIC_NUMBERS,
	'|', HELVETICA_BOLDITALIC_BAR,
	',', HELVETICA_BOLDITALIC_COMMADOTSLASH_SPACE,
	'.', HELVETICA_BOLDITALIC_COMMADOTSLASH_SPACE,
	'\\',HELVETICA_BOLDITALIC_COMMADOTSLASH_SPACE,
	'/', HELVETICA_BOLDITALIC_COMMADOTSLASH_SPACE,
	':', HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE,
	';', HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE,
	' ', HELVETICA_BOLDITALIC_COMMADOTSLASH_SPACE,
	'[', HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE,
	']', HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE,
	'`', HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE,
	'-', HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE,
	'(', HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE,
	')', HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE,
	'!', HELVETICA_BOLDITALIC_SQUAREBRACES_ANDMORE,
	'?', HELVETICA_BOLDITALIC_APPIONMARK,
	'"', HELVETICA_BOLDITALIC_QUOTES,
	'\'',HELVETICA_BOLDITALIC_SINGLEQUOTE,
	'^', HELVETICA_BOLDITALIC_CIRCUMFLEX,
	'{', HELVETICA_BOLDITALIC_CURLYBRACES,
	'}', HELVETICA_BOLDITALIC_CURLYBRACES,
	'*', HELVETICA_BOLDITALIC_STAR,
	'_', HELVETICA_BOLDITALIC_HASH_ANDMORE,
	'#', HELVETICA_BOLDITALIC_HASH_ANDMORE,
	'$', HELVETICA_BOLDITALIC_HASH_ANDMORE,
	'~', HELVETICA_BOLDITALIC_EQUALSIGN_ANDMORE,
	'=', HELVETICA_BOLDITALIC_EQUALSIGN_ANDMORE,
	'+', HELVETICA_BOLDITALIC_EQUALSIGN_ANDMORE,
	'<', HELVETICA_BOLDITALIC_EQUALSIGN_ANDMORE,
	'>', HELVETICA_BOLDITALIC_EQUALSIGN_ANDMORE,
	'&', HELVETICA_BOLDITALIC_AMPERSAND,
	'%', HELVETICA_BOLDITALIC_PERCENT,
	'@', HELVETICA_BOLDITALIC_ATSYMBOL,
	0, 0
	};

	//SetCharCodeList( charWidths, &list );
	SetBoldItalicCharWidths( charWidths );
}