#include "Compiler.h"

#include <string>
#include <list>
#include <stdio.h>
#include "PDFSettings.h"
#include "myansi.h"

#ifdef DEF_MAC
#include <string.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
const size_t PDF_PAGEWIDTH_MIN	= 500;
const size_t PDF_PAGEHEIGHT_MIN = 500;
const size_t PDF_PAGEWIDTH_MAX = 1000;
const size_t PDF_PAGEHEIGHT_MAX = 2000;
const size_t PDF_USLETTERWIDTH = 612;
const size_t PDF_USLETTERHEIGHT = 792;
const size_t PDF_A4WIDTH = 595;
const size_t PDF_A4HEIGHT = 842;
const size_t PDF_USLETTERLANDSCAPEWIDTH = PDF_USLETTERHEIGHT;
const size_t PDF_USLETTERLANDSCAPEHEIGHT = PDF_USLETTERWIDTH;
const size_t PDF_A4LANDSCAPEWIDTH = PDF_A4HEIGHT;
const size_t PDF_A4LANDSCAPEHEIGHT = PDF_A4WIDTH;
const size_t PDF_USLETTER = 0;
const size_t PDF_USLETTERLANDSCAPE = 1;
const size_t PDF_A4 = 2;
const size_t PDF_A4LANDSCAPE =	3;
const size_t PDF_CUSTOM = 4;
const size_t PDF_SUBPAGESPERPAGE_MIN = 1;
const size_t PDF_SUBPAGESPERPAGE_MAX = 2;
const size_t PDF_SUBPAGESPERPAGE_PORTRAIT = PDF_SUBPAGESPERPAGE_MIN;
const size_t PDF_SUBPAGESPERPAGE_LANDSCAPE = PDF_SUBPAGESPERPAGE_MAX;
const size_t PDF_LEFTMARGIN_MIN = 0;
const size_t PDF_RIGHTMARGIN_MIN = 0;
const size_t PDF_TOPMARGIN_MIN = 20;
const size_t PDF_BOTTOMMARGIN_MIN = 20;
const size_t PDF_LEFTMARGIN_MAX = 100;
const size_t PDF_RIGHTMARGIN_MAX = 80;
const size_t PDF_TOPMARGIN_MAX = 40;
const size_t PDF_BOTTOMMARGIN_MAX = 40;
const size_t PDF_BANNERTRAILSPACE_MIN = 0;
const size_t PDF_BANNERTRAILSPACE_MAX = 40;
const size_t PDF_GRAPHTRAILSPACE_MIN = 0;
const size_t PDF_GRAPHTRAILSPACE_MAX = 40;
const size_t PDF_TABLETRAILSPACE_MIN = 0;
const size_t PDF_TABLETRAILSPACE_MAX = 40;
const size_t PDF_TRAILSPACE_DEFAULT = 20;
#ifdef __cplusplus
}
#endif

class ValidSettingPair
{
public:
	ValidSettingPair( std::string firstP, size_t firstValueP, std::string secondP, size_t secondValueP )
	{
		first = firstP;
		firstValue = firstValueP;
		second = secondP;
		secondValue = secondValueP;
	}
	~ValidSettingPair()
	{
		first += "Text";
		second += "Text";
	}
	std::string first;
	size_t firstValue;
	std::string second;
	size_t secondValue;
};

typedef std::list<ValidSettingPair> ValidSettingList;

static const char* PDF_SET_YES = "Yes";
static const char* PDF_SET_NO = "No";
static const char* PDF_SET_ON = "On";
static const char* PDF_SET_OFF = "Off";
static const char* PDF_SET_LEFT_ELLIPSE = "LeftEllipse";
static const char* PDF_SET_RIGHT_ELLIPSE = "RightEllipse";
static const char* PDF_SET_LEFT_NOELLIPSE = "LeftNoEllipse";
static const char* PDF_SET_RIGHT_NOELLIPSE = "RightNoEllipse";
static const char* PDF_SET_NORMAL = "Normal";
static const char* PDF_SET_BOLD = "Bold";
static const char* PDF_SET_ITALIC = "Italic";
static const char* PDF_SET_BOLDITALIC = "BoldItalic";
static const char* PDF_SET_USLETTER = "USLetter";
static const char* PDF_SET_A4 = "A4";
static const char* PDF_SET_USLETTERLANDSCAPE = "USLetterLandscape";
static const char* PDF_SET_A4LANDSCAPE = "A4Landscape";
static const char* PDF_SET_PAGENUMBER_BOTTOMCENTER = "BottomCenter";
static const char* PDF_SET_PAGENUMBER_BOTTOMRIGHT = "BottomRight";
static const char* PDF_SET_PAGENUMBER_TOPCENTER = "TopCenter";
static const char* PDF_SET_PAGENUMBER_TOPRIGHT = "TopRight";
static const char* PDF_SET_PAGENUMBER_TOPANDBOTTOMCENTER = "TopAndBottomCenter";
static const char* PDF_SET_PAGENUMBER_TOPANDBOTTOMRIGHT = "TopAndBottomRight";
static const char* PDF_SET_FONT_TIMESNEWROMAN = "TimesNewRoman";
static const char* PDF_SET_FONT_HELVETICA = "Helvetica";

static ValidSettingPair PDF_YesNo( PDF_SET_YES, 1, PDF_SET_NO, 0 );
static ValidSettingPair PDF_OnOff( PDF_SET_ON, 1, PDF_SET_OFF, 0 );
static ValidSettingPair PDF_LeftRightEllipse( PDF_SET_LEFT_ELLIPSE, 0, PDF_SET_RIGHT_ELLIPSE, 1 );
static ValidSettingPair PDF_LeftRightNoEllipse( PDF_SET_LEFT_NOELLIPSE, 2, PDF_SET_RIGHT_NOELLIPSE, 3 );
static ValidSettingPair PDF_NormalBold( PDF_SET_NORMAL, PDF_NORMAL, PDF_SET_BOLD, PDF_BOLD );
static ValidSettingPair PDF_ItalicBoldItalic( PDF_SET_ITALIC, PDF_ITALIC, PDF_SET_BOLDITALIC, PDF_BOLDITALIC );
static ValidSettingPair PDF_USLetterA4Width( PDF_SET_A4, PDF_A4WIDTH, PDF_SET_USLETTER, PDF_USLETTERWIDTH );
static ValidSettingPair PDF_USLetterA4WidthLandscape( PDF_SET_A4LANDSCAPE, PDF_A4LANDSCAPEWIDTH, PDF_SET_USLETTERLANDSCAPE, PDF_USLETTERLANDSCAPEWIDTH );
static ValidSettingPair PDF_USLetterA4Height( PDF_SET_USLETTER, PDF_USLETTERHEIGHT, PDF_SET_A4, PDF_A4HEIGHT );
static ValidSettingPair PDF_USLetterA4HeightLandscape( PDF_SET_USLETTERLANDSCAPE, PDF_USLETTERLANDSCAPEHEIGHT, PDF_SET_A4LANDSCAPE, PDF_A4LANDSCAPEHEIGHT );
static ValidSettingPair PDF_Top( PDF_SET_PAGENUMBER_TOPCENTER, PDF_PAGENUMBER_TOPCENTER, PDF_SET_PAGENUMBER_BOTTOMRIGHT, PDF_PAGENUMBER_BOTTOMRIGHT );
static ValidSettingPair PDF_Bottom( PDF_SET_PAGENUMBER_TOPCENTER, PDF_PAGENUMBER_TOPCENTER, PDF_SET_PAGENUMBER_TOPRIGHT, PDF_PAGENUMBER_TOPRIGHT );
static ValidSettingPair PDF_TopBottom( PDF_SET_PAGENUMBER_TOPANDBOTTOMCENTER, PDF_PAGENUMBER_TOPANDBOTTOMCENTER, PDF_SET_PAGENUMBER_TOPANDBOTTOMRIGHT, PDF_PAGENUMBER_TOPANDBOTTOMRIGHT );
static ValidSettingPair PDF_Font1( PDF_SET_FONT_TIMESNEWROMAN, 0, PDF_SET_FONT_HELVETICA, 1 );

// Given a "text" setting "set", we "look" in the "setList" to see if we can find a matching string,
// if so then we set the the data to the corresponding value, otherwise we use the default value.
void SetValueUsingText( size_t *dataPos, std::string set, size_t defaultValue, ValidSettingList *setList )
{
	short validSetting = 0;
	for ( ValidSettingList::iterator iter = setList->begin(); iter != setList->end(); iter++ )
	{
		if ( set == (*iter).first )
		{
			validSetting = 1;
			*dataPos = (*iter).firstValue;
		}
		if ( set == (*iter).second )
		{
			validSetting = 1;
			*dataPos = (*iter).secondValue;
		}
	}
	if ( !validSetting )
		*dataPos = defaultValue;
}

// Given a setting "value", we first compare the setting names "prefName" & "optionName", if it is the right one,
// then we set the the data and check that it is in the valid range "min" & "max"
size_t CompareAndSetWithRangeCheck( const char *prefName, const char *optionName, size_t *dataPos, size_t value, size_t defaultValue , size_t min, size_t max )
{
	if ( !mystrcmpi( prefName, optionName ) )
	{
		*dataPos = value;
		if ( *dataPos < min || *dataPos > max )
			*dataPos = defaultValue;
		return 1;
	}
	return 0;
}

void SettingValidColorList( ValidSettingList& setList )
{
	setList.push_back( ValidSettingPair( PDF_BLACK_STR, PDF_BLACK, PDF_RED_STR, PDF_RED ) );
	setList.push_back( ValidSettingPair( PDF_GREEN_STR, PDF_GREEN, PDF_LIGHTGREEN_STR, PDF_LIGHTGREEN ) );
	setList.push_back( ValidSettingPair( PDF_DARKGREEN_STR, PDF_DARKGREEN, PDF_BLUE_STR, PDF_BLUE ) );
	setList.push_back( ValidSettingPair( PDF_LIGHTBLUE_STR, PDF_LIGHTBLUE, PDF_DARKBLUE_STR, PDF_DARKBLUE ) );
	setList.push_back( ValidSettingPair( PDF_GREY_STR, PDF_GREY, PDF_LIGHTGREY_STR, PDF_LIGHTGREY ) );
	setList.push_back( ValidSettingPair( PDF_DARKGREY_STR, PDF_DARKGREY, PDF_PINK_STR, PDF_PINK ) );
	setList.push_back( ValidSettingPair( PDF_LIGHTPINK_STR, PDF_LIGHTPINK, PDF_DARKPINK_STR, PDF_DARKPINK ) );
	setList.push_back( ValidSettingPair( PDF_BROWN_STR, PDF_BROWN, PDF_LIGHTBROWN_STR, PDF_LIGHTBROWN ) );
	setList.push_back( ValidSettingPair( PDF_DARKBROWN_STR, PDF_DARKBROWN, PDF_YELLOW_STR, PDF_YELLOW ) );
	setList.push_back( ValidSettingPair( PDF_LIGHTYELLOW_STR, PDF_LIGHTYELLOW, PDF_ORANGE_STR, PDF_ORANGE ) );
	setList.push_back( ValidSettingPair( PDF_LIGHTORANGE_STR, PDF_LIGHTORANGE, PDF_PURPLE_STR, PDF_PURPLE ) );
	setList.push_back( ValidSettingPair( PDF_LIGHTPURPLE_STR, PDF_LIGHTPURPLE, PDF_DARKPURPLE_STR, PDF_DARKPURPLE ) );
	setList.push_back( ValidSettingPair( PDF_KHAKI_STR, PDF_KHAKI, PDF_AQUA_STR, PDF_AQUA ) );
}

std::string GetSettingValueName( ValidSettingList *setList, size_t value )
{
	for ( ValidSettingList::iterator iter = setList->begin(); iter != setList->end(); iter++ )
	{
		if ( value == (*iter).firstValue )
			return (*iter).first;
		if ( value == (*iter).secondValue )
			return (*iter).second;
	}
	return "";
}

std::string GetColorList( ValidSettingList *setList )
{
	std::string colors = "\n#\t";
	size_t totalLen = 0;
	size_t currTextLen = 0;
	size_t lineLen = 0;
	for ( ValidSettingList::iterator iter = setList->begin(); iter != setList->end(); iter++ )
	{
		colors += (*iter).first;
		colors += ", ";
		colors += (*iter).second;
		colors += ", ";
		
		currTextLen = colors.length() - totalLen;
		totalLen += currTextLen;
		lineLen += currTextLen;
		if ( lineLen > 80 )
		{
			lineLen = 0;
			colors += "\n#\t";
		}
	}
	colors.erase( colors.length()-2, colors.length() );
	colors += "\n";

	return colors;
}

std::string GetNameList( ValidSettingList *setList )
{
	std::string names = "";
	for ( ValidSettingList::iterator iter = setList->begin(); iter != setList->end(); iter++ )
	{
		names += (*iter).first;
		names += ", ";
		names += (*iter).second;
		names += ", ";
	}
	names.erase( names.length()-2, names.length() );
	return names;
}

//
// Graph
//

// Graph Settings - names used in Funnel Web Settings file
const char* PDF_tinyFontSize = "PDFTinyFontSize";
const char* PDF_smallFontSize = "PDFSmallFontSize";
const char* PDF_normalFontSize = "PDFNormalFontSize";
const char* PDF_largeFontSize = "PDFLargeFontSize";
const char* PDF_dottedLineStokeSize = "PDFDottedLineStokeSize";
const char* PDF_dottedLineNonStokeSize = "PDFDottedLineNonStokeSize";

// Graph Settings - Default Settings
const size_t PDF_tinyFontSize_Default = 6;
const size_t PDF_smallFontSize_Default = 8;
const size_t PDF_normalFontSize_Default = 11;
const size_t PDF_largeFontSize_Default = 14;
const size_t PDF_dottedLineStokeSize_Default = 1;
const size_t PDF_dottedLineNonStokeSize_Default = 2;

void PDFGraphSettingsCInit( PDFGraphSettingsC *obj )
{
	obj->tinyFontSize = PDF_tinyFontSize_Default;
	obj->smallFontSize = PDF_smallFontSize_Default;
	obj->normalFontSize = PDF_normalFontSize_Default;
	obj->largeFontSize = PDF_largeFontSize_Default;
	obj->dottedLineStokeSize = PDF_dottedLineStokeSize_Default;
	obj->dottedLineNonStokeSize = PDF_dottedLineNonStokeSize_Default;
}

void PDFGraphSettingsCSave( FILE *fp, PDFGraphSettingsC *obj )
{
	fprintf( fp, "\n" );
	std::string validRange = "Valid Range ";
	fprintf( fp, "# PDF Graph Settings\n" );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_tinyFontSize, obj->tinyFontSize, validRange.c_str(), PDF_tinyFontSize_Min, PDF_tinyFontSize_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_smallFontSize, obj->smallFontSize, validRange.c_str(), PDF_smallFontSize_Min, PDF_smallFontSize_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_normalFontSize, obj->normalFontSize, validRange.c_str(), PDF_smallFontSize_Min, PDF_smallFontSize_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_largeFontSize, obj->largeFontSize, validRange.c_str(), PDF_largeFontSize_Min, PDF_largeFontSize_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_dottedLineStokeSize, obj->dottedLineStokeSize, validRange.c_str(), PDF_dottedLineStokeSize_Min, PDF_dottedLineStokeSize_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_dottedLineNonStokeSize, obj->dottedLineNonStokeSize, validRange.c_str(), PDF_dottedLineNonStokeSize_Min, PDF_dottedLineNonStokeSize_Max );
	fprintf( fp, "\n" );
}

size_t PDFGraphSettingsCIntRead( const char *PDFSettingName, PDFGraphSettingsC *obj, size_t value )
{
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_tinyFontSize, (size_t*)(&obj->tinyFontSize), value, PDF_tinyFontSize_Default, PDF_tinyFontSize_Min, PDF_tinyFontSize_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_smallFontSize, (size_t*)(&obj->smallFontSize), value, PDF_smallFontSize_Default, PDF_smallFontSize_Min, PDF_smallFontSize_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_normalFontSize, (size_t*)(&obj->normalFontSize), value, PDF_normalFontSize_Default, PDF_normalFontSize_Min, PDF_normalFontSize_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_largeFontSize, (size_t*)(&obj->largeFontSize), value, PDF_largeFontSize_Default, PDF_largeFontSize_Min, PDF_largeFontSize_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_dottedLineStokeSize, (size_t*)(&obj->dottedLineStokeSize), value, PDF_dottedLineStokeSize_Default, PDF_dottedLineStokeSize_Min, PDF_dottedLineStokeSize_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_dottedLineNonStokeSize, (size_t*)(&obj->dottedLineNonStokeSize), value, PDF_dottedLineNonStokeSize_Default, PDF_dottedLineNonStokeSize_Min, PDF_dottedLineNonStokeSize_Max ) )
		return 1;
	return 0;
}

//
// Table
//

// Table Settings - names used in Funnel Web Settings file
const char* PDF_titleSize = "PDFTitleSize";
const char* PDF_titleStyle = "PDFTitleStyle";
const char* PDF_titleColor = "PDFTitleColor";
const char* PDF_colHeadingSize = "PDFColHeadingSize";
const char* PDF_colHeadingStyle = "PDFColHeadingStyle";
const char* PDF_colHeadingColor = "PDFColHeadingColor";
const char* PDF_dataSize = "PDFDataSize";
const char* PDF_dataStyle = "PDFDataStyle";
const char* PDF_dataColor = "PDFDataColor";
const char* PDF_reduceFontSizeOfTooLargeData = "PDFReduceFontSizeOfTooLargeData";
const char* PDF_reduceFontSizeOfTooLargeDataByPercent = "PDFReduceFontSizeOfTooLargeDataByPercent";
const char* PDF_minimumTableColumnHeadingChars = "PDFMinimumTableColumnHeadingChars";
const char* PDF_reduceBiggestDataColWidthByPercent = "PDFReduceBiggestDataColWidthByPercent";
const char* PDF_textWrap = "PDFTextWrap";

// Table Settings - Default Settings
const size_t PDF_subPagesPerPage_Default = PDF_SUBPAGESPERPAGE_PORTRAIT;
const size_t PDF_subPagesPerPage_Landscape = PDF_SUBPAGESPERPAGE_LANDSCAPE;
const size_t PDF_titleSize_Default = 14;
const size_t PDF_titleStyle_Default = PDF_BOLD;
const size_t PDF_titleColor_Default = PDF_BLACK;
const size_t PDF_colHeadingSize_Default = 12;
const size_t PDF_colHeadingStyle_Default = PDF_BOLD;
const size_t PDF_colHeadingColor_Default = PDF_BLACK;
const size_t PDF_dataSize_Default = 9;
const size_t PDF_dataStyle_Default = PDF_NORMAL;
const size_t PDF_dataColor_Default = PDF_BLACK;
const size_t PDF_reduceFontSizeOfTooLargeData_Default = 1;
const size_t PDF_reduceFontSizeOfTooLargeDataByPercent_Default = 60;
const size_t PDF_minimumTableColumnHeadingChars_Default = 3;
const size_t PDF_reduceBiggestDataColWidthByPercent_Default = 75;
const size_t PDF_textWrap_Default = 0;

size_t PDF_FONTSIZE_ADJ = 2;


void PDFSaveColorSetting( FILE *fp, const char *settingName, size_t value, size_t min, size_t max )
{
	std::string validRange = "Valid Range ";
	std::string validNames = " as RGB values, also valid color names may be used - see color list above";

	ValidSettingList setList;
	SettingValidColorList( setList );
	std::string name = GetSettingValueName( &setList, value );
	if ( name.length() )
		fprintf( fp, "%s %s # %s(0x%06X - 0x%06X%s)\n", settingName, name.c_str(), validRange.c_str(), min, max, validNames.c_str() );
	else
		fprintf( fp, "%s %d # %s(0x%06X - 0x%06X%s)\n", settingName, value, validRange.c_str(), min, max, validNames.c_str() );
}

void PDFSaveStyleSetting( FILE *fp, const char *settingName, size_t value, size_t min, size_t max )
{
	std::string validRange = "Valid Range ";
	ValidSettingList setList;
	setList.push_back( PDF_NormalBold );
	setList.push_back( PDF_ItalicBoldItalic );
	std::string name = GetSettingValueName( &setList, value );
	std::string validNames = " also valid style names may be used " + GetNameList( &setList );
	if ( name.length() )
		fprintf( fp, "%s %s # %s(%d - %d%s)\n", settingName, name.c_str(), validRange.c_str(), min, max, validNames.c_str() );
	else
		fprintf( fp, "%s %d # %s(%d - %d%s)\n", settingName, value, validRange.c_str(), min, max, validNames.c_str() );
}

void PDFSaveOnOffSetting( FILE *fp, const char *settingName, size_t value, size_t min, size_t max )
{
	std::string validRange = "Valid Range ";
	ValidSettingList setList;
	setList.push_back( PDF_OnOff );
	setList.push_back( PDF_YesNo );
	std::string name = GetSettingValueName( &setList, value );
	std::string validNames = " also valid settings are " + GetNameList( &setList );
	if ( name.length() )
		fprintf( fp, "%s %s # %s(%d - %d%s)\n", settingName, name.c_str(), validRange.c_str(), min, max, validNames.c_str() );
	else
		fprintf( fp, "%s %d # %s(%d - %d%s)\n", settingName, value, validRange.c_str(), min, max, validNames.c_str() );
}

void PDFSaveFontSetting( FILE *fp, const char *settingName, size_t value, size_t min, size_t max )
{
	std::string validRange = "Valid Range ";
	ValidSettingList setList;
	setList.push_back( PDF_Font1 );
	std::string name = GetSettingValueName( &setList, value );
	std::string validNames = " also valid settings are " + GetNameList( &setList );
	if ( name.length() )
		fprintf( fp, "%s %s # %s(%d - %d%s)\n", settingName, name.c_str(), validRange.c_str(), min, max, validNames.c_str() );
	else
		fprintf( fp, "%s %d # %s(%d - %d%s)\n", settingName, value, validRange.c_str(), min, max, validNames.c_str() );
}

void PDFTableSettingsCSave( FILE *fp, PDFTableSettingsC *obj )
{
	ValidSettingList setList;
	SettingValidColorList( setList );
	std::string colors = GetColorList( &setList );
	fprintf( fp, "# PDF Settings which use a RGB color can specify a colors by name, valid names are: %s", colors.c_str() ); 
	fprintf( fp, "\n" );
	std::string validRange = "Valid Range ";
	fprintf( fp, "# PDF Table Settings\n" );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_titleSize, obj->titleSize, validRange.c_str(), PDF_titleSize_Min, PDF_titleSize_Max );
	PDFSaveStyleSetting( fp, PDF_titleStyle, obj->titleStyle, PDF_titleStyle_Min, PDF_titleStyle_Max );
	PDFSaveColorSetting( fp, PDF_titleColor, obj->titleColor, PDF_titleColor_Min, PDF_titleColor_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_colHeadingSize, obj->colHeadingSize, validRange.c_str(), PDF_colHeadingSize_Min, PDF_colHeadingSize_Max );
	PDFSaveStyleSetting( fp, PDF_colHeadingStyle, obj->colHeadingStyle, PDF_colHeadingStyle_Min, PDF_colHeadingStyle_Max );
	PDFSaveColorSetting( fp, PDF_colHeadingColor, obj->colHeadingColor, PDF_colHeadingColor_Min, PDF_colHeadingColor_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_dataSize, obj->dataSize, validRange.c_str(), PDF_dataSize_Min, PDF_dataSize_Max );
	PDFSaveStyleSetting( fp, PDF_dataStyle, obj->dataStyle, PDF_dataStyle_Min, PDF_dataStyle_Max );
	PDFSaveColorSetting( fp, PDF_dataColor, obj->dataColor, PDF_dataColor_Min, PDF_dataColor_Max );
	PDFSaveOnOffSetting( fp, PDF_reduceFontSizeOfTooLargeData, obj->reduceFontSizeOfTooLargeData, PDF_reduceFontSizeOfTooLargeData_Min, PDF_reduceFontSizeOfTooLargeData_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_reduceFontSizeOfTooLargeDataByPercent, obj->reduceFontSizeOfTooLargeDataByPercent, validRange.c_str(), PDF_reduceFontSizeOfTooLargeDataByPercent_Min, PDF_reduceFontSizeOfTooLargeDataByPercent_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_minimumTableColumnHeadingChars, obj->minimumTableColumnHeadingChars, validRange.c_str(), PDF_minimumTableColumnHeadingChars_Min, PDF_minimumTableColumnHeadingChars_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_reduceBiggestDataColWidthByPercent, obj->reduceBiggestDataColWidthByPercent, validRange.c_str(), PDF_reduceBiggestDataColWidthByPercent_Min, PDF_reduceBiggestDataColWidthByPercent_Max );
	PDFSaveOnOffSetting( fp, PDF_textWrap, obj->textWrap, PDF_textWrap_Min, PDF_textWrap_Max );
	fprintf( fp, "\n" );
}

void PDFTableSettingsCInit( PDFTableSettingsC *obj )
{
	obj->titleSize = PDF_titleSize_Default;
	obj->titleStyle = PDF_titleStyle_Default;
	obj->titleColor = PDF_titleColor_Default;
	obj->colHeadingSize = PDF_colHeadingSize_Default;
	obj->colHeadingStyle = PDF_colHeadingStyle_Default;
	obj->colHeadingColor = PDF_colHeadingColor_Default;
	obj->dataSize = PDF_dataSize_Default;
	obj->dataStyle = PDF_dataStyle_Default;
	obj->dataColor = PDF_dataColor_Default;
	obj->reduceFontSizeOfTooLargeData = PDF_reduceFontSizeOfTooLargeData_Default;
	obj->reduceFontSizeOfTooLargeDataByPercent = PDF_reduceFontSizeOfTooLargeDataByPercent_Default;
	obj->minimumTableColumnHeadingChars = PDF_minimumTableColumnHeadingChars_Default;
	obj->reduceBiggestDataColWidthByPercent = PDF_reduceBiggestDataColWidthByPercent_Default;
	obj->textWrap = PDF_textWrap_Default;
	obj->subPagesPerPage = PDF_subPagesPerPage_Default;
}


size_t PDFTableSettingsCIntRead( const char *PDFSettingName, PDFTableSettingsC *obj, size_t value )
{
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_titleSize, (size_t*)(&obj->titleSize), value, PDF_titleSize_Default, PDF_titleSize_Min, PDF_titleSize_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_titleStyle, (size_t*)(&obj->titleStyle), value, PDF_titleStyle_Default, PDF_titleStyle_Min, PDF_titleStyle_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_titleColor, (size_t*)(&obj->titleColor), value, PDF_titleColor_Default, PDF_titleColor_Min, PDF_titleColor_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_colHeadingSize, (size_t*)(&obj->colHeadingSize), value, PDF_colHeadingSize_Default, PDF_colHeadingSize_Min, PDF_colHeadingSize_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_colHeadingStyle, (size_t*)(&obj->colHeadingStyle), value, PDF_colHeadingStyle_Default, PDF_colHeadingStyle_Min, PDF_colHeadingStyle_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_colHeadingColor, (size_t*)(&obj->colHeadingColor), value, PDF_colHeadingColor_Default, PDF_colHeadingColor_Min, PDF_colHeadingColor_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_dataSize, (size_t*)(&obj->dataSize), value, PDF_dataSize_Default, PDF_dataSize_Min, PDF_dataSize_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_dataStyle, (size_t*)(&obj->dataStyle), value, PDF_dataStyle_Default, PDF_dataStyle_Min, PDF_dataStyle_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_dataColor, (size_t*)(&obj->dataColor), value, PDF_dataColor_Default, PDF_dataColor_Min, PDF_dataColor_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_reduceFontSizeOfTooLargeData, (size_t*)(&obj->reduceFontSizeOfTooLargeData), value, PDF_reduceFontSizeOfTooLargeData_Default, PDF_reduceFontSizeOfTooLargeData_Min, PDF_reduceFontSizeOfTooLargeData_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_reduceFontSizeOfTooLargeDataByPercent, (size_t*)(&obj->reduceFontSizeOfTooLargeDataByPercent), value, PDF_reduceFontSizeOfTooLargeDataByPercent_Default, PDF_reduceFontSizeOfTooLargeDataByPercent_Min, PDF_reduceFontSizeOfTooLargeDataByPercent_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_minimumTableColumnHeadingChars, (size_t*)(&obj->minimumTableColumnHeadingChars), value, PDF_minimumTableColumnHeadingChars_Default, PDF_minimumTableColumnHeadingChars_Min, PDF_minimumTableColumnHeadingChars_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_reduceBiggestDataColWidthByPercent, (size_t*)(&obj->reduceBiggestDataColWidthByPercent), value, PDF_reduceBiggestDataColWidthByPercent_Default, PDF_reduceBiggestDataColWidthByPercent_Min, PDF_reduceBiggestDataColWidthByPercent_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_textWrap, (size_t*)(&obj->textWrap), value, PDF_textWrap_Default, PDF_textWrap_Min, PDF_textWrap_Max ) )
		return 1;
	return 0;
}

size_t SetTitleStyle( const char *prefName, PDFTableSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_titleStyle ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_NormalBold );
		setList.push_back( PDF_ItalicBoldItalic );
		SetValueUsingText( (size_t*)(&obj->titleStyle), set, PDF_titleStyle_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetTitleColor( const char *prefName, PDFTableSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_titleColor ) )
	{
		ValidSettingList setList;
		SettingValidColorList( setList );
		SetValueUsingText( (size_t*)(&obj->titleColor), set, PDF_titleColor_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetColHeadingStyle( const char *prefName, PDFTableSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_colHeadingStyle ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_NormalBold );
		setList.push_back( PDF_ItalicBoldItalic );
		SetValueUsingText( (size_t*)(&obj->colHeadingStyle), set, PDF_colHeadingStyle_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetColHeadingColor( const char *prefName, PDFTableSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_colHeadingColor ) )
	{
		ValidSettingList setList;
		SettingValidColorList( setList );
		SetValueUsingText( (size_t*)(&obj->colHeadingColor), set, PDF_colHeadingColor_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetDataStyle( const char *prefName, PDFTableSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_dataStyle ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_NormalBold );
		setList.push_back( PDF_ItalicBoldItalic );
		SetValueUsingText( (size_t*)(&obj->dataStyle), set, PDF_dataStyle_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetDataColor( const char *prefName, PDFTableSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_dataColor ) )
	{
		ValidSettingList setList;
		SettingValidColorList( setList );
		SetValueUsingText( (size_t*)(&obj->dataColor), set, PDF_dataColor_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetReduceFontSizeOfTooLargeData( const char *prefName, PDFTableSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_reduceFontSizeOfTooLargeData ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_YesNo );
		setList.push_back( PDF_OnOff );
		SetValueUsingText( (size_t*)(&obj->reduceFontSizeOfTooLargeData), set, PDF_reduceFontSizeOfTooLargeData_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetTextWrap( const char *prefName, PDFTableSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_textWrap ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_YesNo );
		setList.push_back( PDF_OnOff );
		SetValueUsingText( (size_t*)(&obj->textWrap), set, PDF_textWrap_Default, &setList );
		return 1;
	}
	return 0;
}

size_t PDFTableSettingsCTextRead( const char *PDFSettingName, PDFTableSettingsC *obj, const char *set )
{
	if ( SetTitleStyle( PDFSettingName, obj, set ) )
		return 1;
	if ( SetTitleColor( PDFSettingName, obj, set ) )
		return 1;
	if ( SetColHeadingStyle( PDFSettingName, obj, set ) )
		return 1;
	if ( SetColHeadingColor( PDFSettingName, obj, set ) )
		return 1;
	if ( SetDataStyle( PDFSettingName, obj, set ) )
		return 1;
	if ( SetDataColor( PDFSettingName, obj, set ) )
		return 1;
	if ( SetReduceFontSizeOfTooLargeData( PDFSettingName, obj, set ) )
		return 1;
	if ( SetTextWrap( PDFSettingName, obj, set ) )
		return 1;
	return 0;
}

//
// Page
//

// Page Settings - names used in Funnel Web Settings file
static const char* PDF_pageWidth = "PDFPageWidth";
static const char* PDF_pageHeight = "PDFPageHeight";
static const char* PDF_leftMargin = "PDFLeftMargin";
static const char* PDF_rightMargin = "PDFRightMargin";
static const char* PDF_topMargin = "PDFTopMargin";
static const char* PDF_bottomMargin = "PDFBottomMargin";
static const char* PDF_bannerTrailSpace = "PDFBannerTrailSpace";
static const char* PDF_graphTrailSpace = "PDFGraphTrailSpace";
static const char* PDF_tableTrailSpace = "PDFTableTrailSpace";
static const char* PDF_showBanner = "PDFShowBanner";
static const char* PDF_bannerFile = "PDFBannerFile";
static const char* PDF_minimumTableSizeAtPageEnd = "PDFMinimumTableSizeAtPageEnd";
static const char* PDF_commentsOn = "PDFCommentsOn";
static const char* PDF_truncateTextLeftOrRight = "PDFTruncateTextLeftOrRight";
static const char* PDF_pageNumberingPosition = "PDFPageNumberingPosition";
static const char* PDF_links = "PDFLinks";
static const char* PDF_font = "PDFFont";
static const char* PDF_curvedTables = "PDFCurvedTables";

// Page Settings - Default Settings
static const size_t PDF_pageWidth_Default = PDF_USLETTERWIDTH;
static const size_t PDF_pageHeight_Default = PDF_USLETTERHEIGHT;
static const size_t PDF_leftMargin_Default = 60;
static const size_t PDF_rightMargin_Default = 40;
static const size_t PDF_topMargin_Default = 40;
static const size_t PDF_bottomMargin_Default = 20;
static const size_t PDF_bannerTrailSpace_Default = PDF_TRAILSPACE_DEFAULT;
static const size_t PDF_graphTrailSpace_Default = PDF_TRAILSPACE_DEFAULT;
static const size_t PDF_tableTrailSpace_Default = PDF_TRAILSPACE_DEFAULT;
static const size_t PDF_showBanner_Default = 1;
static const size_t PDF_minimumTableSizeAtPageEnd_Default = 5;
static const size_t PDF_commentsOn_Default = 0;
static const size_t PDF_truncateTextLeftOrRight_Default = PDF_LEFT_ELLIPSE;
static const size_t PDF_pageNumberingPosition_Default = PDF_PAGENUMBER_TOPANDBOTTOMCENTER;
static const size_t PDF_links_Default = 0;
static const size_t PDF_font_Default = PDF_FONT_TIMESNEWROMAN;
static const size_t PDF_curvedTables_Default = 0;

// Page Settings - Minimum Settings
static const size_t PDF_pageWidth_Min = PDF_PAGEWIDTH_MIN;
static const size_t PDF_pageHeight_Min = PDF_PAGEHEIGHT_MIN;
static const size_t PDF_subPagePerPage_Min = PDF_SUBPAGESPERPAGE_MIN;
static const size_t PDF_leftMargin_Min = PDF_LEFTMARGIN_MIN;
static const size_t PDF_rightMargin_Min = PDF_RIGHTMARGIN_MIN;
static const size_t PDF_topMargin_Min = PDF_TOPMARGIN_MIN;
static const size_t PDF_bottomMargin_Min = PDF_BOTTOMMARGIN_MIN;
static const size_t PDF_bannerTrailSpace_Min = PDF_BANNERTRAILSPACE_MIN;
static const size_t PDF_graphTrailSpace_Min = PDF_GRAPHTRAILSPACE_MIN;
static const size_t PDF_tableTrailSpace_Min = PDF_TABLETRAILSPACE_MIN;
static const size_t PDF_showBanner_Min = 0;
static const size_t PDF_minimumTableSizeAtPageEnd_Min = 3;
static const size_t PDF_commentsOn_Min = 0;
static const size_t PDF_truncateTextLeftOrRight_Min = PDF_LEFT;
static const size_t PDF_pageNumberingPosition_Min = PDF_PAGENUMBER_OFF;
static const size_t PDF_links_Min = 0;
static const size_t PDF_font_Min = PDF_FONT_TIMESNEWROMAN;
static const size_t PDF_yesNo_Min = 0;

// Page Settings - Maximum Settings
static const size_t PDF_pageWidth_Max = PDF_PAGEWIDTH_MAX;
static const size_t PDF_pageHeight_Max = PDF_PAGEHEIGHT_MAX;
static const size_t PDF_subPagePerPage_Max = PDF_SUBPAGESPERPAGE_MAX;
static const size_t PDF_leftMargin_Max = PDF_LEFTMARGIN_MAX;
static const size_t PDF_rightMargin_Max = PDF_RIGHTMARGIN_MAX;
static const size_t PDF_topMargin_Max = PDF_TOPMARGIN_MAX;
static const size_t PDF_bottomMargin_Max = PDF_BOTTOMMARGIN_MAX;
static const size_t PDF_bannerTrailSpace_Max = PDF_BANNERTRAILSPACE_MAX;
static const size_t PDF_graphTrailSpace_Max = PDF_GRAPHTRAILSPACE_MAX;
static const size_t PDF_tableTrailSpace_Max = PDF_TABLETRAILSPACE_MAX;
static const size_t PDF_showBanner_Max = 1;
static const size_t PDF_minimumTableSizeAtPageEnd_Max = 10;
static const size_t PDF_commentsOn_Max = 1;
static const size_t PDF_truncateTextLeftOrRight_Max = PDF_RIGHT_ELLIPSE;
static const size_t PDF_pageNumberingPosition_Max = PDF_PAGENUMBER_TOPANDBOTTOMRIGHT;
static const size_t PDF_links_Max = 1;
static const size_t PDF_font_Max = PDF_FONT_HELVETICA;
static const size_t PDF_yesNo_Max = 1;

void PDFPageSettingsCInit( PDFPageSettingsC *obj )
{
	obj->pageWidth = PDF_pageWidth_Default;
	obj->pageHeight = PDF_pageHeight_Default;
	obj->subPagesPerPage = PDF_subPagesPerPage_Default;
	obj->leftMargin = PDF_leftMargin_Default;
	obj->rightMargin = PDF_rightMargin_Default;
	obj->topMargin = PDF_topMargin_Default;
	obj->bottomMargin = PDF_bottomMargin_Default,
	obj->bannerTrailSpace = PDF_bannerTrailSpace_Default;
	obj->graphTrailSpace = PDF_graphTrailSpace_Default;
	obj->tableTrailSpace = PDF_tableTrailSpace_Default;
	obj->showBanner = PDF_showBanner_Default;
	obj->bannerFile[0] = 0;
	obj->minimumTableSizeAtPageEnd = PDF_minimumTableSizeAtPageEnd_Default;
	obj->commentsOn = PDF_commentsOn_Default;
	obj->truncateTextLeftOrRight = PDF_truncateTextLeftOrRight_Default;
	obj->links = PDF_links_Default;
	obj->font = PDF_font_Default;
	obj->curvedTables = PDF_curvedTables_Default;
}


void PDFSaveLeftRightSetting( FILE *fp, const char *settingName, size_t value, size_t min, size_t max )
{
	std::string validRange = "Valid Range ";
	ValidSettingList setList;
	setList.push_back( PDF_LeftRightEllipse );
	setList.push_back( PDF_LeftRightNoEllipse );
	std::string name = GetSettingValueName( &setList, value );
	std::string validNames = " also valid settings are " + GetNameList( &setList );
	if ( name.length() )
		fprintf( fp, "%s %s # %s(%d - %d%s)\n", settingName, name.c_str(), validRange.c_str(), min, max, validNames.c_str() );
	else
		fprintf( fp, "%s %d # %s(%d - %d%s)\n", settingName, value, validRange.c_str(), min, max, validNames.c_str() );
}


void PDFSavePageDimSetting( FILE *fp, const char *settingName, size_t value, size_t min, size_t max, size_t widthOrHeight )
{
	std::string validRange = "Valid Range ";
	ValidSettingList setList;
	if ( widthOrHeight )
	{
		setList.push_back( PDF_USLetterA4Width );
		setList.push_back( PDF_USLetterA4WidthLandscape );
	}
	else
	{
		setList.push_back( PDF_USLetterA4Height );
		setList.push_back( PDF_USLetterA4HeightLandscape );
	}
	std::string name = GetSettingValueName( &setList, value );
	std::string validNames = " also valid settings are " + GetNameList( &setList );
	if ( name.length() )
		fprintf( fp, "%s %s # %s(%d - %d%s)\n", settingName, name.c_str(), validRange.c_str(), min, max, validNames.c_str() );
	else
		fprintf( fp, "%s %d # %s(%d - %d%s)\n", settingName, value, validRange.c_str(), min, max, validNames.c_str() );
}

void PDFSavePageNumberingPosition( FILE *fp, const char *settingName, size_t value, size_t min, size_t max )
{
	std::string validRange = "Valid Range ";
	ValidSettingList setList;
	setList.push_back( PDF_Top );
	setList.push_back( PDF_Bottom );
	setList.push_back( PDF_TopBottom );
	std::string name = GetSettingValueName( &setList, value );
	std::string validNames = " also valid settings are " + GetNameList( &setList );
	if ( name.length() )
		fprintf( fp, "%s %s # %s(%d - %d%s)\n", settingName, name.c_str(), validRange.c_str(), min, max, validNames.c_str() );
	else
		fprintf( fp, "%s %d # %s(%d - %d%s)\n", settingName, value, validRange.c_str(), min, max, validNames.c_str() );
}

void PDFPageSettingsCSave( FILE *fp, PDFPageSettingsC *obj )
{
	std::string validRange = "Valid Range ";
	fprintf( fp, "# PDF Page Settings\n" );
	PDFSavePageDimSetting( fp, PDF_pageWidth, obj->pageWidth, PDF_pageWidth_Min, PDF_pageWidth_Max, 1 );
	PDFSavePageDimSetting( fp, PDF_pageHeight, obj->pageHeight, PDF_pageHeight_Min, PDF_pageHeight_Max, 0 );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_leftMargin, obj->leftMargin, validRange.c_str(), PDF_leftMargin_Min, PDF_leftMargin_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_rightMargin, obj->rightMargin, validRange.c_str(), PDF_rightMargin_Min, PDF_rightMargin_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_topMargin, obj->topMargin, validRange.c_str(), PDF_topMargin_Min, PDF_topMargin_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_bottomMargin, obj->bottomMargin, validRange.c_str(), PDF_bottomMargin_Min, PDF_bottomMargin_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_bannerTrailSpace, obj->bannerTrailSpace, validRange.c_str(), PDF_bannerTrailSpace_Min, PDF_bannerTrailSpace_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_graphTrailSpace, obj->graphTrailSpace, validRange.c_str(), PDF_graphTrailSpace_Min, PDF_graphTrailSpace_Max );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_tableTrailSpace, obj->tableTrailSpace, validRange.c_str(), PDF_tableTrailSpace_Min, PDF_tableTrailSpace_Max );
	PDFSaveOnOffSetting( fp, PDF_showBanner, obj->showBanner, PDF_showBanner_Min, PDF_showBanner_Max );
	fprintf( fp, "%s %s\n", PDF_bannerFile, obj->bannerFile );
	fprintf( fp, "%s %d # %s(%d - %d)\n", PDF_minimumTableSizeAtPageEnd, obj->minimumTableSizeAtPageEnd, validRange.c_str(), PDF_minimumTableSizeAtPageEnd_Min, PDF_minimumTableSizeAtPageEnd_Max );
	PDFSaveOnOffSetting( fp, PDF_commentsOn, obj->commentsOn, PDF_commentsOn_Min, PDF_commentsOn_Max );
	PDFSaveLeftRightSetting( fp, PDF_truncateTextLeftOrRight, obj->truncateTextLeftOrRight, PDF_truncateTextLeftOrRight_Min, PDF_truncateTextLeftOrRight_Max );
	PDFSavePageNumberingPosition( fp, PDF_pageNumberingPosition, obj->pageNumberingPosition, PDF_pageNumberingPosition_Min, PDF_pageNumberingPosition_Max );
	PDFSaveOnOffSetting( fp, PDF_links, obj->links, PDF_links_Min, PDF_links_Max );
	PDFSaveFontSetting( fp, PDF_font, obj->font, PDF_font_Min, PDF_font_Max );
	PDFSaveOnOffSetting( fp, PDF_curvedTables, obj->curvedTables, PDF_yesNo_Min, PDF_yesNo_Max );
	fprintf( fp, "\n" );
}


size_t SetPageWidth( const char *prefName, PDFPageSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_pageWidth ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_USLetterA4Width ); 
		setList.push_back( PDF_USLetterA4WidthLandscape ); 
		SetValueUsingText( (size_t*)(&obj->pageWidth), set, PDF_pageWidth_Default, &setList );
		obj->subPagesPerPage = 1;
		if ( obj->pageWidth == PDF_USLETTERLANDSCAPEWIDTH || obj->pageWidth == PDF_A4LANDSCAPEWIDTH )
			obj->subPagesPerPage = 2;
		return 1;
	}
	return 0;
}

size_t SetPageHeight( const char *prefName, PDFPageSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_pageHeight ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_USLetterA4Height ); 
		setList.push_back( PDF_USLetterA4HeightLandscape ); 
		SetValueUsingText( (size_t*)(&obj->pageHeight), set, PDF_pageHeight_Default, &setList );
		obj->subPagesPerPage = 1;
		if ( obj->pageHeight == PDF_USLETTERLANDSCAPEHEIGHT || obj->pageHeight == PDF_A4LANDSCAPEHEIGHT )
			obj->subPagesPerPage = 2;
		return 1;
	}
	return 0;
}

size_t SetShowBanner( const char *prefName, PDFPageSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_showBanner ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_YesNo );
		setList.push_back( PDF_OnOff );
		SetValueUsingText( (size_t*)(&obj->showBanner), set, PDF_showBanner_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetCommentsOn( const char *prefName, PDFPageSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_commentsOn ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_YesNo );
		setList.push_back( PDF_OnOff );
		SetValueUsingText( (size_t*)(&obj->commentsOn), set, PDF_commentsOn_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetTruncateTextLeftOrRight( const char *prefName, PDFPageSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_truncateTextLeftOrRight ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_LeftRightEllipse );
		setList.push_back( PDF_LeftRightNoEllipse );
		SetValueUsingText( (size_t*)(&obj->truncateTextLeftOrRight), set, PDF_truncateTextLeftOrRight_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetPageNumberingPosition( const char *prefName, PDFPageSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_pageNumberingPosition ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_Top );
		setList.push_back( PDF_Bottom );
		setList.push_back( PDF_TopBottom );
		SetValueUsingText( (size_t*)(&obj->pageNumberingPosition), set, PDF_pageNumberingPosition_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetLinks( const char *prefName, PDFPageSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_links ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_YesNo );
		setList.push_back( PDF_OnOff );
		SetValueUsingText( (size_t*)(&obj->links), set, PDF_links_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetFont( const char *prefName, PDFPageSettingsC *obj, const char *set )
{
	if ( !mystrcmpi( prefName, PDF_font ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_Font1 );
		SetValueUsingText( (size_t*)(&obj->font), set, PDF_font_Default, &setList );
		return 1;
	}
	return 0;
}

size_t SetYesNoSetting( const char *prefName, const char *settingName, size_t& settingVar, size_t defaultSet, const char *set )
{
	if ( !mystrcmpi( prefName, settingName ) )
	{
		ValidSettingList setList;
		setList.push_back( PDF_YesNo );
		setList.push_back( PDF_OnOff );
		SetValueUsingText( (size_t*)(&settingVar), set, defaultSet, &setList );
		return 1;
	}
	return 0;
}

size_t PDFPageSettingsCTextRead( const char *PDFSettingName, PDFPageSettingsC *obj, const char *set )
{
	if ( SetPageWidth( PDFSettingName, obj, set ) )
		return 1;
	else
	if ( SetPageHeight( PDFSettingName, obj, set ) )
		return 1;
	else
	if ( SetShowBanner( PDFSettingName, obj, set ) )
		return 1;
	else
	if ( !mystrcmpi( PDFSettingName, PDF_bannerFile ) )
	{
		mystrncpyNull( obj->bannerFile, set, BANNER_FILE_SIZE );
		return 1;
	}
	else
	if ( SetCommentsOn( PDFSettingName, obj, set ) )
		return 1;
	else
	if ( SetTruncateTextLeftOrRight( PDFSettingName, obj, set ) )
		return 1;
	else
	if ( SetPageNumberingPosition( PDFSettingName, obj, set ) )
		return 1;
	else
	if ( SetLinks( PDFSettingName, obj, set ) )
		return 1;
	else
	if ( SetFont( PDFSettingName, obj, set ) )
		return 1;
	else
	if ( SetYesNoSetting( PDFSettingName, PDF_curvedTables, obj->curvedTables, PDF_curvedTables_Default, set ) )
		return 1;
	
	return 0;
}

size_t PDFPageSettingsCIntRead( const char *PDFSettingName, PDFPageSettingsC *obj, size_t value )
{
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_pageWidth, (size_t*)(&obj->pageWidth), value, PDF_pageWidth_Default, PDF_pageWidth_Min, PDF_pageWidth_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_pageHeight, (size_t*)(&obj->pageHeight), value, PDF_pageHeight_Default, PDF_pageHeight_Min, PDF_pageHeight_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_leftMargin, (size_t*)(&obj->leftMargin), value, PDF_leftMargin_Default, PDF_leftMargin_Min, PDF_leftMargin_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_rightMargin, (size_t*)(&obj->rightMargin), value, PDF_rightMargin_Default, PDF_rightMargin_Min, PDF_rightMargin_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_topMargin, (size_t*)(&obj->topMargin), value, PDF_topMargin_Default, PDF_topMargin_Min, PDF_topMargin_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_bottomMargin, (size_t*)(&obj->bottomMargin), value, PDF_bottomMargin_Default, PDF_bottomMargin_Min, PDF_bottomMargin_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_bannerTrailSpace, (size_t*)(&obj->bannerTrailSpace), value, PDF_bannerTrailSpace_Default, PDF_bannerTrailSpace_Min, PDF_bannerTrailSpace_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_graphTrailSpace, (size_t*)(&obj->graphTrailSpace), value, PDF_graphTrailSpace_Default, PDF_graphTrailSpace_Min, PDF_graphTrailSpace_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_tableTrailSpace, (size_t*)(&obj->tableTrailSpace), value, PDF_tableTrailSpace_Default, PDF_tableTrailSpace_Min, PDF_tableTrailSpace_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_showBanner, (size_t*)(&obj->showBanner), value, PDF_showBanner_Default, PDF_showBanner_Min, PDF_showBanner_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_minimumTableSizeAtPageEnd, (size_t*)(&obj->minimumTableSizeAtPageEnd), value, PDF_minimumTableSizeAtPageEnd_Default, PDF_minimumTableSizeAtPageEnd_Min, PDF_minimumTableSizeAtPageEnd_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_commentsOn, (size_t*)(&obj->commentsOn), value, PDF_commentsOn_Default, PDF_commentsOn_Min, PDF_commentsOn_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_truncateTextLeftOrRight, (size_t*)(&obj->truncateTextLeftOrRight), value, PDF_truncateTextLeftOrRight_Default, PDF_truncateTextLeftOrRight_Min, PDF_truncateTextLeftOrRight_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_pageNumberingPosition, (size_t*)(&obj->pageNumberingPosition), value, PDF_pageNumberingPosition_Default, PDF_pageNumberingPosition_Min, PDF_pageNumberingPosition_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_links, (size_t*)(&obj->links), value, PDF_links_Default, PDF_links_Min, PDF_links_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_font, (size_t*)(&obj->font), value, PDF_font_Default, PDF_font_Min, PDF_font_Max ) )
		return 1;
	else
	if ( CompareAndSetWithRangeCheck( PDFSettingName, PDF_curvedTables, (size_t*)(&obj->curvedTables), value, PDF_curvedTables_Default, PDF_yesNo_Min, PDF_yesNo_Max ) )
		return 1;

	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif


//
// All Settings
//
void PDFAllSettingsCInit( PDFAllSettingsC *obj )
{
	PDFGraphSettingsCInit( &obj->pdfGraphSetC );
	PDFTableSettingsCInit( &obj->pdfTableSetC );
	PDFPageSettingsCInit( &obj->pdfSetC );
}

void PDFAllSettingsCSave( FILE *fp, PDFAllSettingsC *obj )
{
	PDFGraphSettingsCSave( fp, &obj->pdfGraphSetC );
	PDFTableSettingsCSave( fp, &obj->pdfTableSetC );
	PDFPageSettingsCSave( fp, &obj->pdfSetC );
}

void PDFAllSettingsCRead( const char *PDFSettingName, PDFAllSettingsC *obj, const char *set )
{
	short textSetting = 0;
	if (!set)
		return;
	if (set[0] == 0)
		return;

	size_t value = 0;

	if (set[0] == '0' && (set[1] == 'x' || set[1] == 'X') )
	{
		if ( set[2]	!= 0 )
		{
			if ( (set[2] >= '0' && set[2] <= '9') || (set[2] >= 'a' && set[2] <= 'f') || (set[2] >= 'A' && set[2] <= 'F') )
			{
				sscanf( &set[2], "%x", &value );
			}
		}
		else
			return;
	}
	else
		value = atoi( set );

	// Check to see if we are setting a TEXT setting or an INTERGER setting
	if ( value == 0 && mystrcmpi( set, "0" ) )
		textSetting = 1;

	if ( !textSetting )
	{
		if ( PDFGraphSettingsCIntRead( PDFSettingName, &obj->pdfGraphSetC, value ) )
			return;
		else 
		if ( PDFTableSettingsCIntRead( PDFSettingName, &obj->pdfTableSetC, value ) )
			return;
		else
		if ( PDFPageSettingsCIntRead( PDFSettingName, &obj->pdfSetC, value ) )
			return;
		return;
	}
	else
	{
		if ( PDFTableSettingsCTextRead( PDFSettingName, &obj->pdfTableSetC, set ) )
			return;
		else 
		if ( PDFPageSettingsCTextRead( PDFSettingName, &obj->pdfSetC, set ) )
			return;
		return;
	}
}

#ifdef __cplusplus
}
#endif


PDFGraphSettings::PDFGraphSettings( size_t tinyFontSizeP, size_t smallFontSizeP, size_t normalFontSizeP, size_t largeFontSizeP,
	size_t	dottedLineStokeSizeP, size_t dottedLineNonStokeSizeP )
{
	tinyFontSize = tinyFontSizeP;
	smallFontSize = smallFontSizeP;
	normalFontSize = normalFontSizeP;
	largeFontSize = largeFontSizeP;
	dottedLineStokeSize = dottedLineStokeSizeP;
	dottedLineNonStokeSize = dottedLineNonStokeSizeP;
}
PDFGraphSettings::PDFGraphSettings( PDFGraphSettingsC *obj )
{
	tinyFontSize = obj->tinyFontSize;
	smallFontSize = obj->smallFontSize;
	normalFontSize = obj->normalFontSize;
	largeFontSize = obj->largeFontSize;
	dottedLineStokeSize = obj->dottedLineStokeSize;
	dottedLineNonStokeSize = obj->dottedLineNonStokeSize;
}
PDFGraphSettings& PDFGraphSettings::operator=( PDFGraphSettings orig )
{
	if ( &orig != this )
	{
		tinyFontSize = orig.tinyFontSize;
		smallFontSize = orig.smallFontSize;
		normalFontSize = orig.normalFontSize;
		largeFontSize = orig.largeFontSize;
		dottedLineStokeSize = orig.dottedLineStokeSize;
		dottedLineNonStokeSize = orig.dottedLineNonStokeSize;
	}
	return (*this);
}

PDFTableSettings::PDFTableSettings( size_t titleSizeP, size_t titleStyleP, size_t titleColorP,
	size_t colHeadingSizeP, size_t colHeadingStyleP, size_t colHeadingColorP,
	size_t dataSizeP, size_t dataStyleP, size_t dataColorP )
{
	subPagesPerPage = PDF_subPagesPerPage_Default;
	titleSize = titleSizeP;
	titleStyle = titleStyleP;
	titleColor = titleColorP;
	colHeadingSize = colHeadingSizeP;		
	colHeadingStyle = colHeadingStyleP;
	colHeadingColor = colHeadingColorP;
	dataSize = dataSizeP;
	dataStyle = dataStyleP;
	dataColor = dataColorP;
	textCellXIndent = 2;
	reduceFontSizeOfTooLargeData = PDF_reduceFontSizeOfTooLargeData_Default;
	reduceFontSizeOfTooLargeDataByPercent = PDF_reduceFontSizeOfTooLargeDataByPercent_Default;
	minimumTableColumnHeadingChars = PDF_minimumTableColumnHeadingChars_Default;
	reduceBiggestDataColWidthByPercent = PDF_reduceBiggestDataColWidthByPercent_Default;
	textWrap = PDF_textWrap_Default;
}
PDFTableSettings::PDFTableSettings( PDFTableSettingsC *obj )
{
	subPagesPerPage = obj->subPagesPerPage;
	titleSize = obj->titleSize;
	titleStyle = obj->titleStyle;
	titleColor = obj->titleColor;
	colHeadingSize = obj->colHeadingSize;
	colHeadingStyle = obj->colHeadingStyle;
	colHeadingColor = obj->colHeadingColor;
	dataSize = obj->dataSize;
	dataStyle = obj->dataStyle; 
	dataColor = obj->dataColor;
	textCellXIndent = 2;
	reduceFontSizeOfTooLargeData = obj->reduceFontSizeOfTooLargeData;
	reduceFontSizeOfTooLargeDataByPercent = obj->reduceFontSizeOfTooLargeDataByPercent;
	minimumTableColumnHeadingChars = obj->minimumTableColumnHeadingChars;
	reduceBiggestDataColWidthByPercent = obj->reduceBiggestDataColWidthByPercent;
	textWrap = obj->textWrap;
}
PDFTableSettings& PDFTableSettings::operator=( PDFTableSettings orig )
{
	if ( &orig != this )
	{
		subPagesPerPage = orig.subPagesPerPage;
		titleSize = orig.titleSize;
		titleStyle = orig.titleStyle;
		titleColor = orig.titleColor;
		colHeadingSize = orig.colHeadingSize;
		colHeadingStyle = orig.colHeadingStyle;
		colHeadingColor = orig.colHeadingColor;
		dataSize = orig.dataSize;
		dataStyle = orig.dataStyle; 
		dataColor = orig.dataColor;
		textCellXIndent = orig.textCellXIndent;
		reduceFontSizeOfTooLargeData = orig.reduceFontSizeOfTooLargeData;
		reduceFontSizeOfTooLargeDataByPercent = orig.reduceFontSizeOfTooLargeDataByPercent;
		minimumTableColumnHeadingChars = orig.minimumTableColumnHeadingChars;
		reduceBiggestDataColWidthByPercent = orig.reduceBiggestDataColWidthByPercent;
		textWrap = orig.textWrap;
	}
	return (*this);
}

PDFSettings::PDFSettings( size_t pageWidthP, size_t pageHeightP, size_t leftMarginP,
		     size_t rightMarginP, size_t topMarginP, size_t bottomMarginP,
             size_t bannerTrailSpaceP, size_t graphTrailSpaceP, size_t tableTrailSpaceP,
			 size_t showBannerP, char *bannerFileP, size_t minimumTableSizeAtPageEndP, size_t commentsOnP, size_t curvedTablesP )
{
	page.pageWidth = pageWidthP;
	page.pageHeight = pageHeightP;
	if ( page.pageWidth == PDF_USLETTERLANDSCAPEWIDTH && page.pageHeight == PDF_USLETTERLANDSCAPEHEIGHT )
		page.subPagesPerPage = PDF_subPagesPerPage_Landscape;
	else if ( page.pageWidth == PDF_A4LANDSCAPEWIDTH && page.pageHeight == PDF_A4LANDSCAPEWIDTH )
		page.subPagesPerPage = PDF_subPagesPerPage_Landscape;
	else
		page.subPagesPerPage = PDF_subPagesPerPage_Default;
	page.leftMargin = leftMarginP;
	page.rightMargin = rightMarginP;
	page.topMargin = topMarginP;
	page.bottomMargin = bottomMarginP;
	page.bannerTrailSpace = bannerTrailSpaceP;
	page.graphTrailSpace = graphTrailSpaceP; 
	page.tableTrailSpace = tableTrailSpaceP;
	page.showBanner = showBannerP;
	mystrncpyNull( page.bannerFile, bannerFileP, BANNER_FILE_SIZE );
	page.minimumTableSizeAtPageEnd = minimumTableSizeAtPageEndP;
	page.commentsOn = commentsOnP;
	page.truncateTextLeftOrRight = 0;
	page.pageNumberingPosition = PDF_pageNumberingPosition_Default;
	page.links = PDF_links_Default;
	page.font = PDF_font_Default;
	page.curvedTables = curvedTablesP;
}

PDFSettings::PDFSettings( PDFPageSettingsC *obj )
{
	page.subPagesPerPage = obj->subPagesPerPage;
	page.pageWidth = obj->pageWidth;
	page.pageHeight = obj->pageHeight;
	page.leftMargin = obj->leftMargin;
	page.rightMargin = obj->rightMargin;
	page.topMargin = obj->topMargin;
	page.bottomMargin = obj->bottomMargin;
	page.bannerTrailSpace = obj->bannerTrailSpace;
	page.graphTrailSpace = obj->graphTrailSpace; 
	page.tableTrailSpace = obj->tableTrailSpace;
	page.showBanner = obj->showBanner;
	mystrncpyNull( page.bannerFile, obj->bannerFile, BANNER_FILE_SIZE );
	page.minimumTableSizeAtPageEnd = obj->minimumTableSizeAtPageEnd;
	page.commentsOn = obj->commentsOn;
	page.truncateTextLeftOrRight = obj->truncateTextLeftOrRight;
	page.pageNumberingPosition = obj->pageNumberingPosition;
	page.links = obj->links;
	page.font = obj->font;
	page.curvedTables = obj->curvedTables;
}
PDFSettings& PDFSettings::operator=( PDFSettings orig )
{
	if ( &orig != this )
	{
		page.pageWidth = orig.page.pageWidth;
		page.pageHeight = orig.page.pageHeight;
		page.subPagesPerPage = orig.page.subPagesPerPage;
		page.leftMargin = orig.page.leftMargin;
		page.rightMargin = orig.page.rightMargin;
		page.topMargin = orig.page.topMargin;
		page.bottomMargin = orig.page.bottomMargin;
		page.bannerTrailSpace = orig.page.bannerTrailSpace;
		page.graphTrailSpace = orig.page.graphTrailSpace; 
		page.tableTrailSpace = orig.page.tableTrailSpace;
		page.showBanner = orig.page.showBanner;
		mystrncpyNull( page.bannerFile, orig.page.bannerFile, BANNER_FILE_SIZE );
		page.minimumTableSizeAtPageEnd = orig.page.minimumTableSizeAtPageEnd;
		page.commentsOn = orig.page.commentsOn;
		page.truncateTextLeftOrRight = orig.page.truncateTextLeftOrRight;
		page.pageNumberingPosition = orig.page.pageNumberingPosition;
		page.links = orig.page.links;
		page.font = orig.page.font;
		page.curvedTables = orig.page.curvedTables;
	}
	return (*this);
}


size_t PDFSettings::SubPageDrawWidth()
{
	if ( SubPagesPerPage() > 1 )
	{
		size_t subPageMiddleMarginWidth, subPageWidth;
		subPageMiddleMarginWidth = RightMargin() * (SubPagesPerPage()-1);
		subPageWidth = (DrawWidth() - subPageMiddleMarginWidth) / SubPagesPerPage();
		return subPageWidth;
	}
	else
		return DrawWidth();
}
