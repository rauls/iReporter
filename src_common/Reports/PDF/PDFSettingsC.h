
#ifndef PDFSETTINGSC_H
#define PDFSETTINGSC_H

#include <stdio.h>
#include "PDFColors.h"

#ifdef __cplusplus
extern "C" {
#endif
extern const size_t PDF_PAGEWIDTH_MIN;
extern const size_t PDF_PAGEHEIGHT_MIN;
extern const size_t PDF_PAGEWIDTH_MAX;
extern const size_t PDF_PAGEHEIGHT_MAX;
extern const size_t PDF_USLETTERWIDTH;
extern const size_t PDF_USLETTERHEIGHT;
extern const size_t PDF_A4WIDTH;
extern const size_t PDF_A4HEIGHT;
extern const size_t PDF_USLETTERLANDSCAPEWIDTH;
extern const size_t PDF_USLETTERLANDSCAPEHEIGHT;
extern const size_t PDF_A4LANDSCAPEWIDTH;
extern const size_t PDF_A4LANDSCAPEHEIGHT;
extern const size_t PDF_USLETTER;
extern const size_t PDF_USLETTERLANDSCAPE;
extern const size_t PDF_A4;
extern const size_t PDF_A4LANDSCAPE;
extern const size_t PDF_CUSTOM;
extern const size_t PDF_SUBPAGESPERPAGE_MIN;
extern const size_t PDF_SUBPAGESPERPAGE_MAX;
extern const size_t PDF_SUBPAGESPERPAGE_PORTRAIT;
extern const size_t PDF_SUBPAGESPERPAGE_LANDSCAPE;
extern const size_t PDF_LEFTMARGIN_MIN;
extern const size_t PDF_RIGHTMARGIN_MIN;
extern const size_t PDF_TOPMARGIN_MIN;
extern const size_t PDF_BOTTOMMARGIN_MIN;
extern const size_t PDF_LEFTMARGIN_MAX;
extern const size_t PDF_RIGHTMARGIN_MAX;
extern const size_t PDF_TOPMARGIN_MAX;
extern const size_t PDF_BOTTOMMARGIN_MAX;
extern const size_t PDF_BANNERTRAILSPACE_MIN;
extern const size_t PDF_BANNERTRAILSPACE_MAX;
extern const size_t PDF_GRAPHTRAILSPACE_MIN; 
extern const size_t PDF_GRAPHTRAILSPACE_MAX;
extern const size_t PDF_TABLETRAILSPACE_MIN;
extern const size_t PDF_TABLETRAILSPACE_MAX;
extern const size_t PDF_TRAILSPACE_DEFAULT;
#ifdef __cplusplus
}
#endif

#define PDF_FONT_TIMESNEWROMAN 0x0000
#define PDF_FONT_HELVETICA 0x0001

#define PDF_PAGENUMBER_OFF					0x0000
#define PDF_PAGENUMBER_TOPANDBOTTOMCENTER	0x0001
#define PDF_PAGENUMBER_TOPANDBOTTOMRIGHT	0x0002
#define PDF_PAGENUMBER_TOPCENTER			0x0003
#define PDF_PAGENUMBER_TOPRIGHT				0x0004
#define PDF_PAGENUMBER_BOTTOMCENTER			0x0005
#define PDF_PAGENUMBER_BOTTOMRIGHT			0x0006

// Graph Settings - Minimum Settings
#define PDF_tinyFontSize_Min 3
#define PDF_smallFontSize_Min 6
#define PDF_normalFontSize_Min 8
#define PDF_largeFontSize_Min 12
#define PDF_dottedLineStokeSize_Min 1
#define PDF_dottedLineNonStokeSize_Min 1

// Graph Settings - Maximum Settings
#define PDF_tinyFontSize_Max 8
#define PDF_smallFontSize_Max 11
#define PDF_normalFontSize_Max 14
#define PDF_largeFontSize_Max 16
#define PDF_dottedLineStokeSize_Max 10
#define PDF_dottedLineNonStokeSize_Max 10


// Table Settings - Minimum Settings
#define PDF_titleSize_Min 10
#define PDF_titleStyle_Min PDF_NORMAL
#define PDF_titleColor_Min PDF_BLACK
#define PDF_colHeadingSize_Min 8
#define PDF_colHeadingStyle_Min PDF_NORMAL
#define PDF_colHeadingColor_Min PDF_BLACK
#define PDF_dataSize_Min 4
#define PDF_dataStyle_Min PDF_NORMAL
#define PDF_dataColor_Min PDF_BLACK
#define PDF_reduceFontSizeOfTooLargeData_Min 0
#define PDF_reduceFontSizeOfTooLargeDataByPercent_Min 40
#define PDF_minimumTableColumnHeadingChars_Min 3
#define PDF_reduceBiggestDataColWidthByPercent_Min 40
#define PDF_textWrap_Min 0

// Table Settings - Maximum Settings
#define PDF_titleSize_Max 18
#define PDF_titleStyle_Max PDF_BOLDITALIC
#define PDF_titleColor_Max PDF_WHITE
#define PDF_colHeadingSize_Max 16
#define PDF_colHeadingStyle_Max PDF_BOLDITALIC
#define PDF_colHeadingColor_Max PDF_WHITE
#define PDF_dataSize_Max 13
#define PDF_dataStyle_Max PDF_BOLDITALIC
#define PDF_dataColor_Max PDF_WHITE
#define PDF_reduceFontSizeOfTooLargeData_Max 1
#define PDF_reduceFontSizeOfTooLargeDataByPercent_Max 100
#define PDF_minimumTableColumnHeadingChars_Max 10
#define PDF_reduceBiggestDataColWidthByPercent_Max 100
#define PDF_textWrap_Max 1




typedef struct PDFGraphSettingsStruct 
{
	size_t tinyFontSize;
	size_t smallFontSize;
	size_t normalFontSize;
	size_t largeFontSize;
	size_t	dottedLineStokeSize;
	size_t dottedLineNonStokeSize;
} PDFGraphSettingsC;


typedef struct PDFTableSettingsStruct
{
	size_t subPagesPerPage;
	size_t titleSize;
	size_t titleStyle;
	size_t titleColor;
	size_t colHeadingSize;
	size_t colHeadingStyle;
	size_t colHeadingColor;
	size_t dataSize;
	size_t dataStyle;
	size_t dataColor;
	size_t reduceFontSizeOfTooLargeData;
	size_t reduceFontSizeOfTooLargeDataByPercent;
	size_t minimumTableColumnHeadingChars;
	size_t reduceBiggestDataColWidthByPercent;
	size_t textWrap;
} PDFTableSettingsC;

#define BANNER_FILE_SIZE 128
#define REPORT_TITLE_SIZE 128

typedef struct PDFPageSettingsStruct
{
	size_t pageWidth; // the width of each page
	size_t pageHeight; // the length of each page
	size_t subPagesPerPage;
	size_t leftMargin;
	size_t rightMargin;
	size_t topMargin;
	size_t bottomMargin;
	size_t bannerTrailSpace;
	size_t graphTrailSpace;
	size_t tableTrailSpace;
	size_t showBanner;
	char bannerFile[BANNER_FILE_SIZE];
	size_t minimumTableSizeAtPageEnd;
	size_t commentsOn;
	size_t truncateTextLeftOrRight;
	size_t pageNumberingPosition;
	size_t links;
	size_t font;
	size_t curvedTables;
} PDFPageSettingsC;


typedef struct PDFAllSettingsStruct
{
	PDFTableSettingsC pdfTableSetC;
	PDFGraphSettingsC pdfGraphSetC;
	PDFPageSettingsC pdfSetC;
} PDFAllSettingsC;

#ifdef __cplusplus
extern "C" {
#endif

void PDFAllSettingsCRead( const char *PDFSettingName, PDFAllSettingsC *obj, const char *arg );
void PDFAllSettingsCInit( PDFAllSettingsC *obj );
void PDFAllSettingsCSave( FILE *fp, PDFAllSettingsC *obj );


#ifdef __cplusplus
}
#endif

#endif // PDFSETTINGSC_H
