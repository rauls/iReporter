
#ifndef PDFSETTINGS_H
#define PDFSETTINGS_H

#include "PDFSettingsC.h"
#include "PDFColors.h"

extern const size_t PDF_tinyFontSize_Default;
extern const size_t PDF_smallFontSize_Default;
extern const size_t PDF_normalFontSize_Default;
extern const size_t PDF_largeFontSize_Default;
extern const size_t PDF_dottedLineStokeSize_Default;
extern const size_t PDF_dottedLineNonStokeSize_Default;

class PDFGraphSettings 
{
public:
	PDFGraphSettings( size_t tinyFontSizeP = 6, size_t smallFontSizeP = 8, size_t normalFontSizeP = 11, size_t largeFontSizeP = 14,
		size_t	dottedLineStokeSizeP = 1, size_t dottedLineNonStokeSizeP = 2 );
	PDFGraphSettings( PDFGraphSettingsC *obj );
	PDFGraphSettings& operator=( PDFGraphSettings orig );
	size_t tinyFontSize;
	size_t smallFontSize;
	size_t normalFontSize;
	size_t largeFontSize;
	size_t	dottedLineStokeSize;
	size_t dottedLineNonStokeSize;
};

extern const size_t PDF_titleSize_Default;
extern const size_t PDF_titleStyle_Default;
extern const size_t PDF_titleColor_Default;
extern const size_t PDF_colHeadingSize_Default;
extern const size_t PDF_colHeadingStyle_Default;
extern const size_t PDF_colHeadingColor_Default;
extern const size_t PDF_dataSize_Default;
extern const size_t PDF_dataStyle_Default;
extern const size_t PDF_dataColor_Default;
extern const size_t PDF_reduceFontSizeOfTooLargeData_Default;
extern const size_t PDF_reduceFontSizeOfTooLargeDataByPercent_Default;
extern const size_t PDF_minimumTableColumnHeadingChars_Default;
extern const size_t PDF_reduceBiggestDataColWidthByPercent_Default;
extern const size_t PDF_textWrap_Default;

extern size_t PDF_FONTSIZE_ADJ;

class PDFTableSettings 
{
public:
	PDFTableSettings( size_t titleSizeP = 14, size_t titleStyleP = PDF_BOLD, size_t titleColorP = PDF_BLACK,
		size_t colHeadingSizeP = 12, size_t colHeadingStyleP = PDF_BOLD, size_t colHeadingColorP = PDF_BLACK,
		size_t dataSizeP = 9, size_t dataStyleP = PDF_NORMAL, size_t dataColorP = PDF_BLACK );
	PDFTableSettings( PDFTableSettingsC *obj );
	PDFTableSettings& operator=( PDFTableSettings orig );

	size_t SubPagesPerPage() { return subPagesPerPage; }
	size_t TitleSize() { return (titleSize-PDF_FONTSIZE_ADJ)/SubPagesPerPage()+PDF_FONTSIZE_ADJ; }
	size_t TitleStyle() { return titleStyle; }
	size_t TitleColor() { return titleColor; }
	size_t ColHeadingSize() { return (colHeadingSize-PDF_FONTSIZE_ADJ)/SubPagesPerPage()+PDF_FONTSIZE_ADJ; }
	size_t ColHeadingStyle() { return colHeadingStyle; }
	size_t ColHeadingColor() { return colHeadingColor; }
	size_t DataSize() { return (dataSize-PDF_FONTSIZE_ADJ)/SubPagesPerPage()+PDF_FONTSIZE_ADJ; }
	size_t DataStyle() { return dataStyle; }
	size_t DataColor() { return dataColor; }

protected:
	size_t titleSize;
	size_t titleStyle;
	size_t titleColor;
	size_t colHeadingSize;
	size_t colHeadingStyle;
	size_t colHeadingColor;
	size_t dataSize;
	size_t dataStyle;
	size_t dataColor;
public:
	float textCellXIndent;
	size_t reduceFontSizeOfTooLargeData;
	size_t reduceFontSizeOfTooLargeDataByPercent;
	size_t minimumTableColumnHeadingChars;
	size_t reduceBiggestDataColWidthByPercent;
	size_t textWrap;
protected:
	size_t subPagesPerPage;
};

extern const size_t PDF_pageWidth_Default;
extern const size_t PDF_pageHeight_Default;
extern const size_t PDF_subPagesPerPage_Default;
extern const size_t PDF_leftMargin_Default;
extern const size_t PDF_rightMargin_Default;
extern const size_t PDF_topMargin_Default;
extern const size_t PDF_bottomMargin_Default;
extern const size_t PDF_bannerTrailSpace_Default;
extern const size_t PDF_graphTrailSpace_Default;
extern const size_t PDF_tableTrailSpace_Default;
extern const size_t PDF_showBanner_Default;
extern const size_t PDF_minimumTableSizeAtPageEnd_Default;
extern const size_t PDF_commentsOn_Default;
extern const size_t PDF_truncateTextLeftOrRight_Default;
extern const size_t PDF_pageNumberingPosition_Default;
extern const size_t PDF_links_Default;
extern const size_t PDF_font_Default;
extern const size_t PDF_curvedTables_Default;


class PDFSettings
{
public:
	PDFSettings( size_t pageWidthP = 612, size_t pageHeightP = 792, size_t leftMarginP = 60,
		         size_t rightMarginP = 40, size_t topMarginP = 40, size_t bottomMarginP = 20,
                 size_t bannerTrailSpaceP = 20, size_t graphTrailSpaceP = 20, size_t tableTrailSpaceP = 20,
				 size_t showBannerP = 1, char *bannerFileP = "",  size_t minimumTableSizeAtPageEndP = 5, size_t commentsOnP = 0, size_t curvedTablesP = 0 );
	PDFSettings( PDFPageSettingsC *obj );
	PDFSettings& operator=( PDFSettings orig );

	// Functions
	size_t PageWidth() { return page.pageWidth; } // the width of each page
	size_t PageHeight() { return page.pageHeight; } // the length of each page
	size_t SubPagesPerPage() { return page.subPagesPerPage; }
	size_t LeftMargin() { return page.leftMargin; }
	size_t RightMargin() { return page.rightMargin; }
	size_t TopMargin() { return page.topMargin; }
	size_t BottomMargin() { return page.bottomMargin; }
	size_t PageTopDrawPos() { return (PageHeight() - TopMargin()); }
	size_t DrawWidth() { return PageWidth() - LeftMargin() - RightMargin(); }
	size_t SubPageDrawWidth();
	size_t DrawHeight() { return PageHeight() - TopMargin() - BottomMargin(); }
	size_t BannerTrailSpace() { return page.bannerTrailSpace; }
	size_t GraphTrailSpace() { return page.graphTrailSpace; }
	size_t TableTrailSpace() { return page.tableTrailSpace; }
	size_t ShowBanner() { return page.showBanner; }
	char* BannerFile() { return page.bannerFile; }
	size_t MinimumTableSizeAtPageEnd() { return page.minimumTableSizeAtPageEnd; }
	size_t CommentsOn() { return page.commentsOn; }
	size_t TruncateTextLeftOrRight() { return page.truncateTextLeftOrRight; }
	size_t PageNumberingPosition() { return page.pageNumberingPosition; }
	size_t Links() { return page.links; }
	size_t Font() { return page.font; }
	size_t CurvedTables() { return page.curvedTables; }

protected:
	// Data
	PDFPageSettingsC page;
};

#endif // PDFSETTINGS_H
