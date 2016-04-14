#include "FWA.h"
#if DEF_WINDOWS
#include <windows.h>    // includes basic windows functionality
#include <commctrl.h>   // includes the common control header
#include "Winmain.h"
#include "Winutil.h"
#include "prefsgui.h"	// for Init_ComboBoxClearAndSelect()
#include "resource.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Local Header Files
#include "myansi.h"
#include "serialReg.h"
#include "datetime.h"

#include "editpath.h"
#include "config.h"
#include "config_struct.h"
#include "PrefsIO.h"
#include "smtp.h"
#include "FileTypes.h"

//--- Enable emulation of Windows type apis/GUI pieces
#if DEF_UNIX
#include "gtk/gtkwin.h"
#include "gtk/resourceid.h"
#include "util.h"
static HWND		hwndParent;
typedef	char * IDC;
#else
typedef	long IDC;
#endif
// ---------------------------------------------------

#include "ResDefs.h"	// for ReturnString() etc

#ifdef _cplusplus
extern "C" {
#endif

extern	int		gSaved;
extern struct	App_config			*EditPrefPtr;



extern 	 HWND	ghPrefsDlg;

extern 	 HWND    			ghPreProcDlg;
extern 	 HWND    			ghPreProcTabDlg;
extern 	 HWND    			ghPreProc1Dlg;
extern 	 HWND    			ghPreProc2Dlg;
extern 	 HWND    			ghReportDlg;
extern 	 HWND    			ghReportTabDlg;
extern 	 HWND    			ghReport1Dlg;
extern 	 HWND    			ghReport2Dlg;
extern 	 HWND    			ghReportHTMLDlg;
extern 	 HWND    			ghReportPDFDlg;
//				ghReportPDFDlgAdv;
extern 	 HWND    			ghReport4Dlg;
extern 	 HWND    			ghAnalysisDlg;
extern 	 HWND    			ghAnalysisTabDlg;
extern 	 HWND    			ghAnalysis1Dlg;
extern 	 HWND    			ghAnalysis2Dlg;
extern 	 HWND    			ghAnalysis3Dlg;
extern 	 HWND    			ghAnalysis4Dlg;
extern 	 HWND    			ghFiltersDlg;
extern 	 HWND    			ghFiltersTabDlg;
extern 	 HWND    			ghFiltersInDlg;
extern 	 HWND    			ghFiltersOutDlg;
extern 	 HWND    			ghStatsDlg;
extern 	 HWND    			ghStatsTabDlg;
extern 	 HWND    			ghStats1Dlg;
extern 	 HWND    			ghStats2Dlg;
extern 	 HWND    			ghStats3Dlg;
extern 	 HWND    			ghStats4Dlg;
extern 	 HWND    			ghStats5Dlg;
extern 	 HWND    			ghAddsDlg;
extern 	 HWND    			ghAddsTabDlg;
extern 	 HWND    			ghAddsBannersDlg;
extern 	 HWND    			ghAddsCampDlg;
extern 	 HWND    			ghHtmlColorDlg;
extern 	 HWND    			ghPostProcDlg;
extern 	 HWND    			ghPostProcTabDlg;
extern 	 HWND    			ghPostProc1Dlg;
extern 	 HWND    			ghPostProc2Dlg;
extern 	 HWND    			ghPostProc3Dlg;
extern 	 HWND    			ghCustomDlg;
extern 	 HWND    			ghCustomTabDlg;
extern 	 HWND    			ghCustom1Dlg;
extern 	 HWND    			ghCustom2Dlg;
extern 	 HWND    			ghVDomainsDlg;
extern 	 HWND    			ghVDomainsTabDlg;
extern 	 HWND    			ghVDomains1Dlg;
extern 	 HWND    			ghVDomains2Dlg;
extern 	 HWND    			ghVDomains3Dlg;
extern 	 HWND    			ghVDomains4Dlg;
extern 	 HWND    			ghVDomains5Dlg;
extern 	 HWND    			ghProxyDlg;
extern 	 HWND    			ghProxyTabDlg;
extern 	 HWND    			ghProxyIPDlg;
extern 	 HWND    			ghROIDlg;
extern 	 HWND				ghRemoteDlg;

#ifdef _cplusplus
}
#endif



// convert an "blah=value" into two strings, "blah" "value"
void FilterToArray( char *filterstring, char **array, long max )
{
	char *src, *out, c, sep='=';
	long index = 0, ch=0;

	src = filterstring;

	if ( max == 2 )
		sep = '=';
	else
		sep = ',';

	out = array[index];
	while( *src && index<max && out ){
		c=*src;
		if ( c!=sep ) {
			*out++ = c;
		} else {
			*out++ = 0;
			index++;
			out = array[index];
		}
		src++;
	}
	*out = 0;
	//OutDebugs(" Done. index=%d, max=%d, out=%08lx, c=%d", index, max, out, c );
}


void ArrayToFilter( char **array, char *filterstring, long max )
{
	switch( max ){
		case 2:	sprintf( filterstring, "%s=%s", array[0], array[1] ); break;
		case 3:	sprintf( filterstring, "%s,%s,%s", array[0], array[1], array[2] ); break;
	}
}



void Check_OutputSuffix( HWND hDlg, const char *suff )
{
	char	temp[256], *p;

	GetText( IDC_OUTDIR, temp, 256 );
	p = strrchr( temp, '.' );
	if( p ){
		strcpy( p, suff );
		SetText( IDC_OUTDIR, temp );
	}
}


void FixReportOutput( HWND hDlg )
{
	int numSel = GetPopupNum( IDC_PREFGEN_TYPE );
	switch( numSel ){
		case 0:	Check_OutputSuffix( hDlg, HTML_EXT ); Enable( IDC_PREFGEN_GTYPE ); break;
		case 1:	Check_OutputSuffix( hDlg, HTML_EXT ); Enable( IDC_PREFGEN_GTYPE ); break;
		case 2:	Check_OutputSuffix( hDlg, RTF_EXT ); Disable( IDC_PREFGEN_GTYPE );
				SetPopupNum( IDC_PREFGEN_GTYPE, 2 );EditPrefPtr->image_format = 2;  break;
		case 3:	Check_OutputSuffix( hDlg, COMMA_EXT ); Disable( IDC_PREFGEN_GTYPE ); break;
		case 4:	Check_OutputSuffix( hDlg, PDF_EXT ); Disable( IDC_PREFGEN_GTYPE ); break;
		case 5:	Check_OutputSuffix( hDlg, EXCEL_EXT ); Disable( IDC_PREFGEN_GTYPE ); break;
	}
}

void UpdateGraphicsPopup( HWND hDlg )
{
	switch( GetPopupNum( IDC_PREFGEN_TYPE ) ){
		case 0:	Enable( IDC_PREFGEN_GTYPE ); break;
		case 1:	Enable( IDC_PREFGEN_GTYPE ); break;
		case 2:	Disable( IDC_PREFGEN_GTYPE );
				break;
		case 3:	Disable( IDC_PREFGEN_GTYPE ); break;
		case 4:	Disable( IDC_PREFGEN_GTYPE ); break;
		case 5:	Disable( IDC_PREFGEN_GTYPE ); break;
	}
}


//   12/45/7890
void	Check_GUIDates( HWND hDlg )
{
	char	date1[20],date2[20];
	time_t	t1=0, t2=0;
static long showingerror=0;

	GetText( IDC_DATESTART, date1 ,12 );
	GetText( IDC_DATEEND, date2 ,12 );

	TimeStringToDays( date1, (time_t*)&t1 );
	TimeStringToDays( date2, (time_t*)&t2 );

	if ( !showingerror ){
		if ( t1==0 ){
			showingerror = 1;
			MsgBox_Error( IDS_ERR_DATE1 );
			showingerror = 0;
		}

		if ( t2==0 ){
			showingerror = 1;
			MsgBox_Error( IDS_ERR_DATE1 );
			showingerror = 0;
		}

		if ( t1 && t2){
			if ( (t2 < t1) ){
				showingerror = 1;
				MsgBox_Error( IDS_ERR_DATE3, date1, date2 );
				showingerror = 0;
			}
		}
	}
}



long GetTimeValue( HWND hDlg, IDC id )
{
	char txt[256];

	GetText( id, txt, 256 );

	if ( mystrchr( txt , ':' ) ){
		struct tm date;
		long ct;

		ct = StringToDaysTime( txt, &date);
		return ct;
	} else
		return atoi( txt );
}











// set a (BODY color tag)
long SetHTMLColorsGUI( HWND hDlg )
{
	SetHTMLColors( EditPrefPtr );
	SetText( IDC_PREFHTML_HEAD, EditPrefPtr->html_head );
	return 1;
}


// ==================================================================

void	PreProc_DatatoGUI( void )
{
	HWND hDlg;
	char	server[128];
	char	name[128];
	char	passwd[128];
	char	path[256];

	hDlg = ghPreProc1Dlg;
	CheckIt( IDC_PRE_ON, EditPrefPtr->preproc_on );
	if ( EditPrefPtr->preproc_on ){
		Enable( IDC_PRE_HOST );
		Enable( IDC_PRE_USER );
		Enable( IDC_PRE_PASSWD );
		Enable( IDC_PRE_PATH );
		Enable( IDC_PRE_BROWSE );
	} else {
		Disable( IDC_PRE_HOST );
		Disable( IDC_PRE_USER );
		Disable( IDC_PRE_PASSWD );
		Disable( IDC_PRE_PATH );
		Disable( IDC_PRE_BROWSE );
	}

	if ( EditPrefPtr->preproc_location[0] ){
		ExtractUserFromURL( EditPrefPtr->preproc_location, server, name, passwd, path );
		if ( !strcmpd( path, "/" ) ) strcpy( path, "/*.log" );
		SetText( IDC_PRE_HOST, server );
		SetText( IDC_PRE_USER, name );
		SetText( IDC_PRE_PASSWD, passwd );
		SetText( IDC_PRE_PATH, path );
	}

	hDlg = ghPreProc2Dlg;
	SetText( IDC_PREPROC_DEST, EditPrefPtr->preproc_tmp );
	CheckIt( IDC_PREPROC_DEL, EditPrefPtr->preproc_delete );

}
void	PreProc_GUItoData( void )
{
	HWND hDlg = ghPreProc1Dlg;
	char	server[128];
	char	name[128];
	char	passwd[128];
	char	path[256];

	GetText( IDC_PRE_HOST, server, 128 );
	GetText( IDC_PRE_USER, name, 64 );
	GetText( IDC_PRE_PASSWD, passwd, 64 );
	GetText( IDC_PRE_PATH, path, 256 );
	mystrcpy( EditPrefPtr->preproc_location, "ftp://" );
	MakeURLfromDetails( EditPrefPtr->preproc_location, server, name, passwd, path );
	EditPrefPtr->preproc_on = (short)IsChecked( IDC_PRE_ON );

	hDlg = ghPreProc2Dlg;
	{
		int len;
		len = GetText( IDC_PREPROC_DEST, EditPrefPtr->preproc_tmp, 256 );
		if ( len>1 )
		{
			// If not trailing slash is found, add one in to make the path correct.		(QCM:44648)
			if ( EditPrefPtr->preproc_tmp[len-1] != '\\' )
				strcat( EditPrefPtr->preproc_tmp, "\\" );
		}

		EditPrefPtr->preproc_delete = (short)IsChecked( IDC_PREPROC_DEL );
	}

}


void SetPDFPageSize()
{
	HWND hDlg = ghReportPDFDlg;
	size_t pageSize;
	size_t width;
	size_t height;
	char buf[32];
	if ( !hDlg )
		return;
	width = EditPrefPtr->pdfAllSetC.pdfSetC.pageWidth;
	height = EditPrefPtr->pdfAllSetC.pdfSetC.pageHeight;
	if ( width == PDF_USLETTERWIDTH && height == PDF_USLETTERHEIGHT )
	{
		pageSize = PDF_USLETTER;
	}
	else if ( width == PDF_A4WIDTH && height == PDF_A4HEIGHT )
	{
		pageSize = PDF_A4;
	}
	else if ( width == PDF_USLETTERLANDSCAPEWIDTH && height == PDF_USLETTERLANDSCAPEHEIGHT )
	{
		pageSize = PDF_USLETTERLANDSCAPE;
	}
	else if ( width == PDF_A4LANDSCAPEWIDTH && height == PDF_A4LANDSCAPEHEIGHT )
	{
		pageSize = PDF_A4LANDSCAPE;
	}
	else
	{
		pageSize = PDF_CUSTOM;
	}

	SetPopupNum( IDC_PREF_PDF_PAGESIZE, pageSize );
	sprintf( buf, "%d", width );
	SetText( IDC_PREF_PDF_PAGEWIDTH, buf);
	sprintf( buf, "%d", height );
	SetText( IDC_PREF_PDF_PAGEHEIGHT, buf );
	if ( pageSize == PDF_CUSTOM )
	{
		Enable( IDC_PREF_PDF_PAGEWIDTH );
		Enable( IDC_PREF_PDF_PAGEHEIGHT );
	}
	else
	{
		Disable( IDC_PREF_PDF_PAGEWIDTH );
		Disable( IDC_PREF_PDF_PAGEHEIGHT );
	}
}

void ChangePDFPageSize()
{
	HWND hDlg = ghReportPDFDlg;
	size_t width, height;
	size_t numSel;

	if ( !hDlg )
		return;

	numSel = GetPopupNum( IDC_PREF_PDF_PAGESIZE );
	if ( numSel == PDF_USLETTER )
	{
		width = PDF_USLETTERWIDTH;
		height = PDF_USLETTERHEIGHT;
	}
	else if ( numSel == PDF_A4 )
	{
		width = PDF_A4WIDTH;
		height = PDF_A4HEIGHT;
	}
	else if ( numSel == PDF_USLETTERLANDSCAPE )
	{
		width = PDF_USLETTERLANDSCAPEWIDTH;
		height = PDF_USLETTERLANDSCAPEHEIGHT;
	}
	else if ( numSel == PDF_A4LANDSCAPE )
	{
		width = PDF_A4LANDSCAPEWIDTH;
		height = PDF_A4LANDSCAPEHEIGHT;
	}
	if ( numSel != PDF_CUSTOM )
	{
		char buf[32];
		sprintf( buf, "%d", width );
		SetText( IDC_PREF_PDF_PAGEWIDTH, buf);
		sprintf( buf, "%d", height );
		SetText( IDC_PREF_PDF_PAGEHEIGHT, buf );
		Disable( IDC_PREF_PDF_PAGEWIDTH );
		Disable( IDC_PREF_PDF_PAGEHEIGHT );
	}
	else
	{
		Enable( IDC_PREF_PDF_PAGEWIDTH );
		Enable( IDC_PREF_PDF_PAGEHEIGHT );
	}
}


#define PDF_AFTERBANNER	0
#define PDF_AFTERGRAPH	1
#define PDF_AFTERTABLE	2

void ChangeSpacingSizeEdit()
{
	HWND hDlg = ghReportPDFDlg;
	int num;
	int numSel;
	char buf[32];
	int min, max;

	if ( !hDlg )
		return;

	// Get the num item selected (from the combo) and the text (from the edit)
	numSel = GetPopupNum( IDC_PREF_PDF_SPACING );
	GetText( IDC_PREF_PDF_SPACINGSIZE, buf, 32 );
	num = atoi( buf );

	// Check which item we are changing
	switch ( numSel )
	{
	case PDF_AFTERBANNER:
		min = PDF_BANNERTRAILSPACE_MIN;
		max = PDF_BANNERTRAILSPACE_MAX;
		break;
	case PDF_AFTERGRAPH:
		min = PDF_GRAPHTRAILSPACE_MIN;
		max = PDF_GRAPHTRAILSPACE_MAX;
		break;
	case PDF_AFTERTABLE:
		min = PDF_TABLETRAILSPACE_MIN;
		max = PDF_TABLETRAILSPACE_MAX;
		break;
	}

	// Check range
	if ( num < min )
	{
		num = min;
		sprintf( buf, "%d", min );
		SetText( IDC_PREF_PDF_SPACINGSIZE, buf );
	}
	if ( num > max )
	{
		num = max;
		sprintf( buf, "%d", max );
		SetText( IDC_PREF_PDF_SPACINGSIZE, buf );
	}

	// Check which item we are changing
	switch ( numSel )
	{
	case PDF_AFTERBANNER:
		EditPrefPtr->pdfAllSetC.pdfSetC.bannerTrailSpace = num;
		return;
	case PDF_AFTERGRAPH:
		EditPrefPtr->pdfAllSetC.pdfSetC.graphTrailSpace = num;
		return;
	case PDF_AFTERTABLE:
		EditPrefPtr->pdfAllSetC.pdfSetC.tableTrailSpace = num;
		return;
	}
}

void ChangeSpacingSizeCombo()
{
	HWND hDlg = ghReportPDFDlg;
	int spacing;
	int numSel;
	char buf[32];

	if ( !hDlg )
		return;

	// Check which item we are changing in the combo
	numSel = GetPopupNum( IDC_PREF_PDF_SPACING );
	switch ( numSel )
	{
	case PDF_AFTERBANNER:
		spacing = EditPrefPtr->pdfAllSetC.pdfSetC.bannerTrailSpace;
		break;
	case PDF_AFTERGRAPH:
		spacing = EditPrefPtr->pdfAllSetC.pdfSetC.graphTrailSpace;
		break;
	case PDF_AFTERTABLE:
		spacing = EditPrefPtr->pdfAllSetC.pdfSetC.tableTrailSpace;
		break;
	}

	// Set the text in the edit
	sprintf( buf, "%d", spacing );
	SetText( IDC_PREF_PDF_SPACINGSIZE, buf);
}

#define	PDF_TINY_FONT	0
#define	PDF_SMALL_FONT	1
#define	PDF_NORMAL_FONT	2
#define	PDF_LARGE_FONT	3
#define BUFLEN		128

void ChangeGraphFontsNameCombo()
{
	HWND hDlg = ghReportPDFDlg;
	int font;
	int numSel;
	int min, max;
	char buffer[BUFLEN];
	int i;

	if ( !hDlg )
		return;

	// Check which item we are changing in the combo
	numSel = GetPopupNum( IDC_PREF_PDF_GRAPHFONT );
	switch ( numSel )
	{
	case PDF_TINY_FONT:
		font = EditPrefPtr->pdfAllSetC.pdfGraphSetC.tinyFontSize;
		min = PDF_tinyFontSize_Min;
		max = PDF_tinyFontSize_Max;
		break;
	case PDF_SMALL_FONT:
		font = EditPrefPtr->pdfAllSetC.pdfGraphSetC.smallFontSize;
		min = PDF_smallFontSize_Min;
		max = PDF_smallFontSize_Max;
		break;
	case PDF_NORMAL_FONT:
		font = EditPrefPtr->pdfAllSetC.pdfGraphSetC.normalFontSize;
		min = PDF_normalFontSize_Min;
		max = PDF_normalFontSize_Max;
		break;
	case PDF_LARGE_FONT:
		font = EditPrefPtr->pdfAllSetC.pdfGraphSetC.largeFontSize;
		min = PDF_largeFontSize_Min;
		max = PDF_largeFontSize_Max;
		break;
	}

	memset( buffer, 0, BUFLEN );
	for ( i = min; i <= max; i++ )
	{
		strcat( buffer, Long2Str( i, 0 ) );
		if ( i != max )
			strcat( buffer, "," );
	}
	Init_ComboBoxClearAndSelect( ghReportPDFDlg, IDC_PREF_PDF_GRAPHFONTSIZE, buffer, font - min );
}

ComboBoxColors comboBoxColors[] =
{
	PDF_BLACK_STR,		PDF_BLACK,
	PDF_RED_STR,		PDF_RED,
	PDF_GREEN_STR,		PDF_GREEN,
	PDF_LIGHTGREEN_STR,	PDF_LIGHTGREEN,
	PDF_DARKGREEN_STR,	PDF_DARKGREEN,
	PDF_BLUE_STR,		PDF_BLUE,
	PDF_LIGHTBLUE_STR,	PDF_LIGHTBLUE,
	PDF_DARKBLUE_STR,	PDF_DARKBLUE,
	PDF_GREY_STR,		PDF_GREY,
	PDF_LIGHTGREY_STR,	PDF_LIGHTGREY,
	PDF_DARKGREY_STR,	PDF_DARKGREY,
	PDF_PINK_STR,		PDF_PINK,
	PDF_LIGHTPINK_STR,	PDF_LIGHTPINK,
	PDF_DARKPINK_STR,	PDF_DARKPINK,
	PDF_BROWN_STR,		PDF_BROWN, 
	PDF_LIGHTBROWN_STR,	PDF_LIGHTBROWN,
	PDF_DARKBROWN_STR,	PDF_DARKBROWN,
	PDF_YELLOW_STR,		PDF_YELLOW,
	PDF_LIGHTYELLOW_STR,PDF_LIGHTYELLOW,
	PDF_ORANGE_STR,		PDF_ORANGE,
	PDF_LIGHTORANGE_STR,PDF_LIGHTORANGE,
	PDF_PURPLE_STR,		PDF_PURPLE,
	PDF_LIGHTPURPLE_STR,PDF_LIGHTPURPLE,
	PDF_DARKPURPLE_STR,	PDF_DARKPURPLE,
	PDF_KHAKI_STR,		PDF_KHAKI,
	PDF_AQUA_STR,		PDF_AQUA,
	"",					0
};


int SelectPDFColorInCombo( int value )
{
	int i = 0;
	while( comboBoxColors[i].colorName[0] != 0 )
	{
		if ( comboBoxColors[i].colorValue == value )
			return i;
		i++;
	}
	return 0;
}

#define	PDF_TITLE			0
#define	PDF_COLUMNHEADINGS	1
#define	PDF_DATA			2

void ChangeTableTextNameCombo()
{
	HWND hDlg = ghReportPDFDlg;
	int font, style, color;
	int numSel;
	int min, max;
	char buffer[BUFLEN];
	int i;

	if ( !hDlg )
		return;

	// Check which item we are changing in the combo
	numSel = GetPopupNum( IDC_PREF_PDF_TABLETEXTNAME );

	// Text Size - Change the values in the combo and select the current setting
	switch ( numSel )
	{
		case PDF_TITLE:
			font = EditPrefPtr->pdfAllSetC.pdfTableSetC.titleSize;
			min = PDF_titleSize_Min;
			max = PDF_titleSize_Max;
			break;
		case PDF_COLUMNHEADINGS:
			font = EditPrefPtr->pdfAllSetC.pdfTableSetC.colHeadingSize;
			min = PDF_colHeadingSize_Min;
			max = PDF_colHeadingSize_Max;
			break;
		case PDF_DATA:
			font = EditPrefPtr->pdfAllSetC.pdfTableSetC.dataSize;
			min = PDF_dataSize_Min;
			max = PDF_dataSize_Max;
			break;
	}

	memset( buffer, 0, BUFLEN );
	for ( i = min; i <= max; i++ )
	{
		strcat( buffer, Long2Str( i, 0 ) );
		if ( i != max )
			strcat( buffer, "," );
	}
	Init_ComboBoxClearAndSelect( ghReportPDFDlg, IDC_PREF_PDF_FONTSIZE, buffer, font - min );

	// Text Style - Select the current setting
	switch ( numSel )
	{
		case PDF_TITLE:
			style = EditPrefPtr->pdfAllSetC.pdfTableSetC.titleStyle;
			break;
		case PDF_COLUMNHEADINGS:
			style = EditPrefPtr->pdfAllSetC.pdfTableSetC.colHeadingStyle;
			break;
		case PDF_DATA:
			style = EditPrefPtr->pdfAllSetC.pdfTableSetC.dataStyle;
			break;
	}
	SetPopupNum( IDC_PREF_PDF_FONTSTYLE, style );

	// Text Color - Select the current setting
	switch ( numSel )
	{
		case PDF_TITLE:
			color = SelectPDFColorInCombo( EditPrefPtr->pdfAllSetC.pdfTableSetC.titleColor );
			break;
		case PDF_COLUMNHEADINGS:
			color = SelectPDFColorInCombo( EditPrefPtr->pdfAllSetC.pdfTableSetC.colHeadingColor );
			break;
		case PDF_DATA:
			color = SelectPDFColorInCombo( EditPrefPtr->pdfAllSetC.pdfTableSetC.dataColor );
			break;
	}
	SetPopupNum( IDC_PREF_PDF_FONTCOLOR, color );
}


void ChangeTextWrap()
{
	HWND hDlg = ghReportPDFDlg;
	if ( IsChecked( IDC_PREF_PDF_TEXTWRAP ) )
		Disable( IDC_PREF_PDF_ADVANCED );
	else
		Enable( IDC_PREF_PDF_ADVANCED );
}


void Report_PDFPageAdv_DataToGUI( HWND hDlg )
{
	char buf[32];

	// Should we reduce the text font size if the data is too large for a table cell
	CheckIt( IDC_PREF_PDF_REDUCEDATAFONTSIZE, EditPrefPtr->pdfAllSetC.pdfTableSetC.reduceFontSizeOfTooLargeData );
	sprintf( buf, "%d", EditPrefPtr->pdfAllSetC.pdfTableSetC.reduceFontSizeOfTooLargeDataByPercent );
	SetText( IDC_PREF_PDF_REDUCEDATAFONTSIZEPER, buf);
	if ( !IsChecked( IDC_PREF_PDF_REDUCEDATAFONTSIZE ) )
		Disable( IDC_PREF_PDF_REDUCEDATAFONTSIZEPER );

	// If all the data columns won't fit in a table, by how much do we reduce the largest data column
	sprintf( buf, "%d", EditPrefPtr->pdfAllSetC.pdfTableSetC.reduceBiggestDataColWidthByPercent );
	SetText( IDC_PREF_PDF_REDUCELARGESTDATACOL, buf);

	// Minimum number of characters to be displayed in Column Headings
	sprintf( buf, "%d", EditPrefPtr->pdfAllSetC.pdfTableSetC.minimumTableColumnHeadingChars );
	SetText( IDC_PREF_PDF_MINCOLHEADSCHARS, buf);
}

void Report_PDFPage_DataToGUI()
{
	char buf[32];
	HWND hDlg = ghReportPDFDlg;
	int pos;

	if ( !hDlg )
		return;

	// Page Size
	SetPDFPageSize();
	ChangePDFPageSize();

	// Page Margins
	sprintf( buf, "%d", EditPrefPtr->pdfAllSetC.pdfSetC.leftMargin );
	SetText( IDC_PREF_PDF_LEFTMARGIN, buf);
	sprintf( buf, "%d", EditPrefPtr->pdfAllSetC.pdfSetC.rightMargin );
	SetText( IDC_PREF_PDF_RIGHTMARGIN, buf);
	sprintf( buf, "%d", EditPrefPtr->pdfAllSetC.pdfSetC.topMargin );
	SetText( IDC_PREF_PDF_TOPMARGIN, buf);
	sprintf( buf, "%d", EditPrefPtr->pdfAllSetC.pdfSetC.bottomMargin );
	SetText( IDC_PREF_PDF_BOTTOMMARGIN, buf);

	// Spacing
	ChangeSpacingSizeCombo();

	// Page Numbering & Position
	SetPopupNum( IDC_PREF_PDF_PAGENUMBERINGPOS, EditPrefPtr->pdfAllSetC.pdfSetC.pageNumberingPosition );

	// Banner
	CheckIt( IDC_PREF_PDF_SHOWBANNER, EditPrefPtr->pdfAllSetC.pdfSetC.showBanner );
	SetText( IDC_PREF_PDF_BANNERFILE, EditPrefPtr->pdfAllSetC.pdfSetC.bannerFile );

	// Truncate Text
	pos = EditPrefPtr->pdfAllSetC.pdfSetC.truncateTextLeftOrRight;
	SetPopupNum( IDC_PREF_PDF_TRUNCATETEXT, pos );

	// Font
	pos = EditPrefPtr->pdfAllSetC.pdfSetC.font;
	SetPopupNum( IDC_PREF_PDF_FONT, pos );

	// Graph Text Sizes
	ChangeGraphFontsNameCombo();

	// Table Text Attributes
	ChangeTableTextNameCombo();

	// Text Wrap
	CheckIt( IDC_PREF_PDF_TEXTWRAP, EditPrefPtr->pdfAllSetC.pdfTableSetC.textWrap );
	ChangeTextWrap();
}


void ShowDiskFree( HWND hDlg, char *file )
{
	long kb;
	char tmp[128], szText[256];

	kb = (long)(GetDiskFree( file ) / 1024);
	GetString( IDS_LOCATION, tmp, 64 );
	sprintf( szText, tmp, kb );
	SetText( IDC_LOCATION, szText );
}

void SetComboLanguageSel( HWND parent, IDC comboId, char *language )
{
	char	szText[128];
	int lp;
	HWND hDlg = parent;

	lp = GetPopupTot( comboId ) + 1;
	while( lp )
	{
		szText[0]=0;
		GetPopupText( comboId, lp-1, szText );
		if ( !mystrcmpi( szText, language ) ){
			SetPopupNum( comboId, lp-1 );
			lp = 0; continue;
		}
		lp--;
	}
}


void	Report_DatatoGUI( void )
{
	HWND hDlg;

	hDlg = ghReport1Dlg;
	SetText( IDC_OUTDIR, EditPrefPtr->outfile );
	ShowDiskFree( hDlg, EditPrefPtr->outfile );

	SetComboLanguageSel( hDlg, IDC_PREF_LANG, EditPrefPtr->language );
	SetPopupNum( IDC_PREFGEN_GTYPE, EditPrefPtr->image_format );

	switch( EditPrefPtr->report_format ){
		case FORMAT_HTML:
				if ( EditPrefPtr->html_frames )
					SetPopupNum( IDC_PREFGEN_TYPE, 1 );
				else
					SetPopupNum( IDC_PREFGEN_TYPE, 0 );
				Enable( IDC_PREFGEN_GTYPE );break;
		case FORMAT_RTF:	SetPopupNum( IDC_PREFGEN_TYPE, 2 );break;
		case FORMAT_COMMA:	SetPopupNum( IDC_PREFGEN_TYPE, 3 );break;
		case FORMAT_PDF:	SetPopupNum( IDC_PREFGEN_TYPE, 4 );break;
		case FORMAT_EXCEL:	SetPopupNum( IDC_PREFGEN_TYPE, 5 );break;
	}
	UpdateGraphicsPopup( hDlg );

	// Load the PDF Controls from settings data
	Report_PDFPage_DataToGUI();

	SetText( IDC_DB_NAME, EditPrefPtr->database_file );
	CheckIt( IDC_DB_REPORT, !EditPrefPtr->database_no_report );
	CheckIt( IDC_DB_EXCLUDE, EditPrefPtr->database_excluded );

	if ( EditPrefPtr->database_active )
	{
		if( EditPrefPtr->database_extended )
		{
			CheckIt( IDC_DB_SELECT_COMPACT, 0 );
			CheckIt( IDC_DB_SELECT_EXTENDED, 1 );
		}
		else
		{
			CheckIt( IDC_DB_SELECT_COMPACT, 1 );
			CheckIt( IDC_DB_SELECT_EXTENDED, 0 );
		}
		CheckIt( IDC_DB_NONE, 0 );
		Enable( IDC_DB_BROWSE );
		Enable( IDC_DB_DELETE );
		Enable( IDC_DB_NAME );
		Enable( IDC_DB_REPORT );
		
		if( EditPrefPtr->database_no_report )
		{
			Disable( IDC_DB_EXCLUDE );
		}
		else
		{
			Enable( IDC_DB_EXCLUDE );
		}
	}
	else
	{
		CheckIt( IDC_DB_SELECT_COMPACT, 0 );
		CheckIt( IDC_DB_SELECT_EXTENDED, 0 );
		CheckIt( IDC_DB_NONE, 1 );
		Disable( IDC_DB_BROWSE );
		Disable( IDC_DB_NAME );
		Disable( IDC_DB_DELETE );
		Disable( IDC_DB_REPORT );
		Disable( IDC_DB_EXCLUDE );
	}

	hDlg = ghReport2Dlg;
	CheckIt( IDC_PREF_BANDWIDTH, EditPrefPtr->bandwidth );
	CheckIt( IDC_PREF_HEADTITLE, EditPrefPtr->head_title );
	CheckIt( IDC_PREF_TIMESTAT, EditPrefPtr->write_timestat );
	CheckIt( IDC_PREF_LEFTSIDE, EditPrefPtr->headingOnLeft );
	CheckIt( IDC_PREF_QINDEX, EditPrefPtr->html_quickindex );
	CheckIt( IDC_PREF_SHADOW, EditPrefPtr->shadow );
	CheckIt( IDC_PREF_FOOTERLABEL, EditPrefPtr->footer_label );
	CheckIt( IDC_PREF_FOOTERLABELTRAIL, EditPrefPtr->footer_label_trailing );
	SetText( IDC_REPORTTITLE, EditPrefPtr->report_title );

	hDlg = ghReportHTMLDlg;
	SetText( IDC_PREFHTML_HEAD, EditPrefPtr->html_head );
	SetText( IDC_PREFHTML_FOOT, EditPrefPtr->html_foot );

//	hDlg = ghReport4Dlg;
//	SetText( IDC_PREFHTML_FOOT, EditPrefPtr->html_foot );
}


void GetPDFPageSize()
{
	HWND hDlg = ghReportPDFDlg;
	size_t pageSize;

	if ( !hDlg )
		return;
	
	pageSize = GetPopupNum( IDC_PREF_PDF_PAGESIZE ) ;
	if ( pageSize == PDF_USLETTER )
	{
		EditPrefPtr->pdfAllSetC.pdfSetC.pageWidth = PDF_USLETTERWIDTH;
		EditPrefPtr->pdfAllSetC.pdfSetC.pageHeight = PDF_USLETTERHEIGHT;
		EditPrefPtr->pdfAllSetC.pdfSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_PORTRAIT;
		EditPrefPtr->pdfAllSetC.pdfTableSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_PORTRAIT;
	}
	else if ( pageSize == PDF_A4 )
	{
		EditPrefPtr->pdfAllSetC.pdfSetC.pageWidth = PDF_A4WIDTH;
		EditPrefPtr->pdfAllSetC.pdfSetC.pageHeight = PDF_A4HEIGHT;
		EditPrefPtr->pdfAllSetC.pdfSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_PORTRAIT;
		EditPrefPtr->pdfAllSetC.pdfTableSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_PORTRAIT;
	}
	else if ( pageSize == PDF_USLETTERLANDSCAPE )
	{
		EditPrefPtr->pdfAllSetC.pdfSetC.pageWidth = PDF_USLETTERLANDSCAPEWIDTH;
		EditPrefPtr->pdfAllSetC.pdfSetC.pageHeight = PDF_USLETTERLANDSCAPEHEIGHT;
		EditPrefPtr->pdfAllSetC.pdfSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_LANDSCAPE;
		EditPrefPtr->pdfAllSetC.pdfTableSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_LANDSCAPE;
	}
	else if ( pageSize == PDF_A4LANDSCAPE )
	{
		EditPrefPtr->pdfAllSetC.pdfSetC.pageWidth = PDF_A4LANDSCAPEWIDTH;
		EditPrefPtr->pdfAllSetC.pdfSetC.pageHeight = PDF_A4LANDSCAPEHEIGHT;
		EditPrefPtr->pdfAllSetC.pdfSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_LANDSCAPE;
		EditPrefPtr->pdfAllSetC.pdfTableSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_LANDSCAPE;
	}
	else // pageSize = PDF_CUSTOM;
	{
		char buf[32];
		size_t num;
		GetText( IDC_PREF_PDF_PAGEWIDTH, buf, 32 );
		num = atoi( buf );
		if ( num < PDF_PAGEWIDTH_MIN )
			num = PDF_PAGEWIDTH_MIN;
		else if ( num > PDF_PAGEWIDTH_MAX )
			num = PDF_PAGEWIDTH_MAX;
		EditPrefPtr->pdfAllSetC.pdfSetC.pageWidth = num;

		GetText( IDC_PREF_PDF_PAGEHEIGHT, buf, 32 );
		num = atoi( buf );
		if ( num < PDF_PAGEHEIGHT_MIN )
			num = PDF_PAGEHEIGHT_MIN;
		else if ( num > PDF_PAGEHEIGHT_MAX )
			num = PDF_PAGEHEIGHT_MAX;
		EditPrefPtr->pdfAllSetC.pdfSetC.pageHeight = num;
		EditPrefPtr->pdfAllSetC.pdfSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_PORTRAIT;
		EditPrefPtr->pdfAllSetC.pdfTableSetC.subPagesPerPage = PDF_SUBPAGESPERPAGE_PORTRAIT;
	}
}

void GetPDFMargins()
{
	HWND hDlg = ghReportPDFDlg;
	char buf[32];
	size_t num;

	if ( !hDlg )
		return;

	// Left Margin
	GetText( IDC_PREF_PDF_LEFTMARGIN, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_LEFTMARGIN_MIN )
		num = PDF_LEFTMARGIN_MIN;
	else if ( num > PDF_LEFTMARGIN_MAX )
		num = PDF_LEFTMARGIN_MAX;
	EditPrefPtr->pdfAllSetC.pdfSetC.leftMargin = num;

	// Right Margin
	GetText( IDC_PREF_PDF_RIGHTMARGIN, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_RIGHTMARGIN_MIN )
		num = PDF_RIGHTMARGIN_MIN;
	else if ( num > PDF_RIGHTMARGIN_MAX )
		num = PDF_RIGHTMARGIN_MAX;
	EditPrefPtr->pdfAllSetC.pdfSetC.rightMargin = num;

	// Top Margin
	GetText( IDC_PREF_PDF_TOPMARGIN, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_TOPMARGIN_MIN )
		num = PDF_TOPMARGIN_MIN;
	else if ( num > PDF_TOPMARGIN_MAX )
		num = PDF_TOPMARGIN_MAX;
	EditPrefPtr->pdfAllSetC.pdfSetC.topMargin = num;

	// Bottom Margin
	GetText( IDC_PREF_PDF_BOTTOMMARGIN, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_BOTTOMMARGIN_MIN )
		num = PDF_BOTTOMMARGIN_MIN;
	else if ( num > PDF_BOTTOMMARGIN_MAX )
		num = PDF_BOTTOMMARGIN_MAX;
	EditPrefPtr->pdfAllSetC.pdfSetC.bottomMargin = num;
}

void GetPDFSpacing()
{
	HWND hDlg = ghReportPDFDlg;

	if ( !hDlg )
		return;

	// Banner trail spacing
/*
	int pageSize;
	char buf[32];
	int num;

	GetText( IDC_PREF_PDF_BANNERTRAILSPACE, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_BANNERTRAILSPACE_MIN )
		num = PDF_BANNERTRAILSPACE_MIN;
	else if ( num > PDF_BANNERTRAILSPACE_MAX )
		num = PDF_BANNERTRAILSPACE_MAX;
	EditPrefPtr->pdfAllSetC.pdfSetC.bannerTrailSpace = num;

	// Graph trail spacing
	GetText( IDC_PREF_PDF_GRAPHTRAILSPACE, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_GRAPHTRAILSPACE_MIN )
		num = PDF_GRAPHTRAILSPACE_MIN;
	else if ( num > PDF_GRAPHTRAILSPACE_MAX )
		num = PDF_GRAPHTRAILSPACE_MAX;
	EditPrefPtr->pdfAllSetC.pdfSetC.graphTrailSpace = num;

	// Table trail spacing
	GetText( IDC_PREF_PDF_TABLETRAILSPACE, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_TABLETRAILSPACE_MIN )
		num = PDF_TABLETRAILSPACE_MIN;
	else if ( num > PDF_TABLETRAILSPACE_MAX )
		num = PDF_TABLETRAILSPACE_MAX;
	EditPrefPtr->pdfAllSetC.pdfSetC.tableTrailSpace = num;*/
}

void Report_PDFPageAdv_GUIToData( HWND hDlg )
{
	char buf[32];
	int num;

	if ( !hDlg )
		return;

	// Reduce Text Size for too large data
	if ( IsChecked( IDC_PREF_PDF_REDUCEDATAFONTSIZE ) )
		EditPrefPtr->pdfAllSetC.pdfTableSetC.reduceFontSizeOfTooLargeData = 1;
	else
		EditPrefPtr->pdfAllSetC.pdfTableSetC.reduceFontSizeOfTooLargeData = 0;
	GetText( IDC_PREF_PDF_REDUCEDATAFONTSIZEPER, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_reduceFontSizeOfTooLargeDataByPercent_Min )
		num = PDF_reduceFontSizeOfTooLargeDataByPercent_Min;
	else if ( num > PDF_reduceFontSizeOfTooLargeDataByPercent_Max )
		num = PDF_reduceFontSizeOfTooLargeDataByPercent_Max;
	EditPrefPtr->pdfAllSetC.pdfTableSetC.reduceFontSizeOfTooLargeDataByPercent = num;

	GetText( IDC_PREF_PDF_REDUCELARGESTDATACOL, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_reduceBiggestDataColWidthByPercent_Min )
		num = PDF_reduceBiggestDataColWidthByPercent_Min;
	else if ( num > PDF_reduceBiggestDataColWidthByPercent_Max )
		num = PDF_reduceBiggestDataColWidthByPercent_Max;
	EditPrefPtr->pdfAllSetC.pdfTableSetC.reduceBiggestDataColWidthByPercent = num;

	GetText( IDC_PREF_PDF_MINCOLHEADSCHARS, buf, 32 );
	num = atoi( buf );
	if ( num < PDF_minimumTableColumnHeadingChars_Min )
		num = PDF_minimumTableColumnHeadingChars_Min;
	else if ( num > PDF_minimumTableColumnHeadingChars_Max )
		num = PDF_minimumTableColumnHeadingChars_Max;
	EditPrefPtr->pdfAllSetC.pdfTableSetC.minimumTableColumnHeadingChars = num;
}

void Report_PDFPage_GUIToData()
{
	HWND hDlg = ghReportPDFDlg;

	if ( !hDlg )
		return;

	GetPDFPageSize();
	GetPDFMargins();
	EditPrefPtr->pdfAllSetC.pdfSetC.pageNumberingPosition = GetPopupNum( IDC_PREF_PDF_PAGENUMBERINGPOS );

	GetPDFSpacing();

	EditPrefPtr->pdfAllSetC.pdfSetC.showBanner = IsChecked( IDC_PREF_PDF_SHOWBANNER );
	GetText( IDC_PREF_PDF_BANNERFILE, EditPrefPtr->pdfAllSetC.pdfSetC.bannerFile, BANNER_FILE_SIZE );

	// Truncate Text
	EditPrefPtr->pdfAllSetC.pdfSetC.truncateTextLeftOrRight = GetPopupNum( IDC_PREF_PDF_TRUNCATETEXT );

	// Font
	EditPrefPtr->pdfAllSetC.pdfSetC.font = GetPopupNum( IDC_PREF_PDF_FONT );

	// Text Wrap
	if ( IsChecked( IDC_PREF_PDF_TEXTWRAP ) )
		EditPrefPtr->pdfAllSetC.pdfTableSetC.textWrap = 1;
	else
		EditPrefPtr->pdfAllSetC.pdfTableSetC.textWrap = 0;
}

void LanguageString_GUItoData( HWND parent, IDC comboId, char *language )
{
	char buf[256];
	HWND hDlg = parent;
	int i;
	i = GetPopupNum( comboId );
	GetPopupText( comboId, i, buf );
	mystrcpy( language, buf );
}

void ReportFormat_GUItoData()
{
	HWND hDlg = ghReport1Dlg;
	switch( GetPopupNum( IDC_PREFGEN_TYPE ) ){
		case 0 : EditPrefPtr->report_format = FORMAT_HTML; EditPrefPtr->html_frames = 0; break;
		case 1 : EditPrefPtr->report_format = FORMAT_HTML; EditPrefPtr->html_frames = 1; break;
		case 2 : EditPrefPtr->report_format = FORMAT_RTF; break;
		case 3 : EditPrefPtr->report_format = FORMAT_COMMA; break;
		case 4 : EditPrefPtr->report_format = FORMAT_PDF; break;
		case 5 : EditPrefPtr->report_format = FORMAT_EXCEL; break;// excel
	}
}

void	Report_GUItoData( void )
{
	HWND hDlg = ghReport1Dlg;

	GetText( IDC_OUTDIR, EditPrefPtr->outfile, 255 );
	LanguageString_GUItoData( ghReport1Dlg, IDC_PREF_LANG, EditPrefPtr->language );
	ReportFormat_GUItoData();
	EditPrefPtr->image_format = (char)GetPopupNum( IDC_PREFGEN_GTYPE );

	// Save the PDF Controls to settings data
	Report_PDFPage_GUIToData();

	GetText( IDC_DB_NAME, EditPrefPtr->database_file, 255 );
	trimLineWhite(EditPrefPtr->database_file);
	EditPrefPtr->database_active=(short)(!IsChecked( IDC_DB_NONE ));
	EditPrefPtr->database_extended=(short) EditPrefPtr->database_active && IsChecked( IDC_DB_SELECT_EXTENDED );
	if( EditPrefPtr->database_active )
	{
		// ensure that the correct extension is used for both types of database

		if( EditPrefPtr->database_extended )
		{
			char* ptr=strstr( EditPrefPtr->database_file, ".fdb" );

			if( ptr && *(ptr+4)==0 )
			{
				strcpy( ptr+2, "xdb" );
			}
		}
		else
		{
			char* ptr=strstr( EditPrefPtr->database_file, ".fxdb" );

			if( ptr && *(ptr+5)==0 )
			{
				strcpy( ptr+2, "db" );
			}
		}
	}
	EditPrefPtr->database_no_report=(short)!IsChecked( IDC_DB_REPORT );
	EditPrefPtr->database_excluded=(short)IsChecked( IDC_DB_EXCLUDE );

	hDlg = ghReport2Dlg;
	EditPrefPtr->bandwidth = (char)IsChecked( IDC_PREF_BANDWIDTH );
	EditPrefPtr->head_title = (char)IsChecked( IDC_PREF_HEADTITLE );
	EditPrefPtr->write_timestat = (char)IsChecked( IDC_PREF_TIMESTAT );
	EditPrefPtr->headingOnLeft = (char)IsChecked( IDC_PREF_LEFTSIDE );
	EditPrefPtr->html_quickindex = (char)IsChecked( IDC_PREF_QINDEX );
	EditPrefPtr->shadow = (char)IsChecked( IDC_PREF_SHADOW );
	EditPrefPtr->footer_label			= (char)IsChecked( IDC_PREF_FOOTERLABEL );
	EditPrefPtr->footer_label_trailing	= (char)IsChecked( IDC_PREF_FOOTERLABELTRAIL );
	GetText( IDC_REPORTTITLE, EditPrefPtr->report_title, REPORT_TITLE_SIZE );

	hDlg = ghReportHTMLDlg;
	GetText( IDC_PREFHTML_HEAD, EditPrefPtr->html_head, MAX_HTMLSIZE );
	GetText( IDC_PREFHTML_FOOT, EditPrefPtr->html_foot, MAX_HTMLSIZE );

//	hDlg = ghReport4Dlg;
//	GetText( IDC_PREFHTML_FOOT, EditPrefPtr->html_foot, MAX_HTMLSIZE );
}






void	Analysis_DatatoGUI( void )
{
	char	szText[128];
	time_t	t1,t2;
	HWND hDlg;

	hDlg = ghAnalysis1Dlg;
	SetText( IDC_PREFGEN_URL, EditPrefPtr->siteurl );
	SetText( IDC_PREFGEN_INDEXFILE, EditPrefPtr->defaultindex );
	SetPopupNum( IDC_PREFGEN_DATESTYLE, EditPrefPtr->alldates );

	if ( EditPrefPtr->alldates == DATE_SPECIFY ){
		Enable( IDC_DATESTART );
		Enable( IDC_DATEEND );

		DaysDateToString( EditPrefPtr->startTimeT, szText, GetDateStyle(), '/', 1,1);
		SetText( IDC_DATESTART, szText );

		DaysDateToString( EditPrefPtr->endTimeT, szText, GetDateStyle(), '/', 1,1);
		SetText( IDC_DATEEND, szText );
	} else
	// Set Date Ranges
	{
		DatetypeToDate( EditPrefPtr->alldates, &t1, &t2 );

		DaysDateToString( t1, szText, GetDateStyle(), '/', 1,1);
		SetText( IDC_DATESTART, szText );

		DaysDateToString( t2, szText, GetDateStyle(), '/', 1,1);
		SetText( IDC_DATEEND, szText );

		Disable( IDC_DATESTART );
		Disable( IDC_DATEEND );
	}
	SetPopupNum( IDC_DATEOVERIDE, EditPrefPtr->forceddmm );

	DaysTimeToString( EditPrefPtr->time_adjust, szText, TRUE, FALSE, TRUE );
	SetText( IDC_DATEOFFSET, szText );

	if ( EditPrefPtr->multimonths == 1 )
		SetPopupNum( IDC_PREFGEN_SPLITDATE, 2 );
	else
	if ( EditPrefPtr->multimonths == 2 )
		SetPopupNum( IDC_PREFGEN_SPLITDATE, 1 );
	else
		SetPopupNum( IDC_PREFGEN_SPLITDATE, 0 );

	
	//CheckIt( IDC_PREF_STREAM, EditPrefPtr->streaming );
	CheckIt( IDC_PREFGEN_FILTERZERO, EditPrefPtr->filter_zerobyte );
	CheckIt( IDC_PREFGEN_IGNORESELF, EditPrefPtr->ignore_selfreferral );
	CheckIt( IDC_PREFGEN_IGNOREBOOKMARK, EditPrefPtr->ignore_bookmarkreferral );
	CheckIt( IDC_PREFGEN_IGNORECASE, EditPrefPtr->ignorecase );
	CheckIt( IDC_PREFGEN_IGNOREUSERS, EditPrefPtr->ignore_usernames );

	hDlg = ghAnalysis2Dlg;
	SetText( IDC_PREFGEN_DNSNUM, Long2Str( EditPrefPtr->dnsAmount, 1 ) );
	CheckIt( IDC_PREFFTP_PASSIVE, EditPrefPtr->ftp_passive );
	
	switch ( EditPrefPtr->dnslookup ){
		case 0:
				CheckOFF( IDC_PREFGEN_DNSYES);
				CheckOFF( IDC_PREFGEN_DNSCACHE);
				CheckON( IDC_PREFGEN_DNSNO); break;
		case 2:
				CheckOFF( IDC_PREFGEN_DNSYES);
				CheckON( IDC_PREFGEN_DNSCACHE);
				CheckOFF( IDC_PREFGEN_DNSNO); break;
		case 1:
		default:
				CheckON( IDC_PREFGEN_DNSYES);
				CheckOFF( IDC_PREFGEN_DNSCACHE);
				CheckOFF( IDC_PREFGEN_DNSNO); break;
	}
	CheckIt( IDC_PREF_REMOTETITLE, EditPrefPtr->page_remotetitle );
	hDlg = ghAnalysis3Dlg;

	hDlg = ghAnalysis4Dlg;
	CheckIt( IDC_PREFGEN_USECGI, EditPrefPtr->useCGI );
	CheckIt( IDC_PREFGEN_RETAINVAR, EditPrefPtr->retain_variable );
	SetText( IDC_PREFGEN_RETAINVARLIST, EditPrefPtr->retain_variablelist );
	if ( EditPrefPtr->useCGI ){
		Enable( IDC_PREFGEN_RETAINVAR );
	} else {
		Disable( IDC_PREFGEN_RETAINVAR );
	}
	if ( EditPrefPtr->retain_variable ){
		Enable( IDC_PREFGEN_RETAINVARLIST );
	} else {
		Disable( IDC_PREFGEN_RETAINVARLIST );
	}
	if ( IS_ENT ){
		CheckIt( IDC_PREFGEN_FLUSH, EditPrefPtr->streaming );
	} else {
		Disable( IDC_PREFGEN_FLUSH );
		Disable( IDC_FLUSHTXT1 );
	}

}

void	Analysis_GUItoData( void )
{
	HWND hDlg = ghAnalysis1Dlg;
	GetText( IDC_PREFGEN_URL, EditPrefPtr->siteurl , 255 );
	GetText( IDC_PREFGEN_INDEXFILE, EditPrefPtr->defaultindex , 32 );

	switch( GetPopupNum( IDC_PREFGEN_SPLITDATE ) ){
		case 0:	EditPrefPtr->multimonths = 0; break;
		case 1:	EditPrefPtr->multimonths = 2; break;
		case 2:	EditPrefPtr->multimonths = 1; break;
	}

	EditPrefPtr->alldates = (char)GetPopupNum( IDC_PREFGEN_DATESTYLE );

	if ( EditPrefPtr->alldates == DATE_SPECIFY )
	{
		char szText[64];
		//startTimeT
		GetText( IDC_DATESTART, szText ,12 );
		TimeStringToDays( szText, &EditPrefPtr->startTimeT );

		//endTimeT
		GetText( IDC_DATEEND, szText ,12 );
		TimeStringToDays( szText, &EditPrefPtr->endTimeT );
	}
	EditPrefPtr->time_adjust = GetTimeValue( hDlg, IDC_DATEOFFSET );
	EditPrefPtr->forceddmm = (char)GetPopupNum( IDC_DATEOVERIDE );

	//EditPrefPtr->streaming = IsChecked( IDC_PREF_STREAM );
	EditPrefPtr->filter_zerobyte = (char)IsChecked( IDC_PREFGEN_FILTERZERO );
	EditPrefPtr->ignore_selfreferral = (char)IsChecked( IDC_PREFGEN_IGNORESELF );
	EditPrefPtr->ignore_bookmarkreferral = (char)IsChecked( IDC_PREFGEN_IGNOREBOOKMARK );
	EditPrefPtr->ignorecase = (char)IsChecked( IDC_PREFGEN_IGNORECASE );
	EditPrefPtr->ignore_usernames = (char)IsChecked( IDC_PREFGEN_IGNOREUSERS );

	//###############
	hDlg = ghAnalysis2Dlg;
	if ( IsChecked( IDC_PREFGEN_DNSYES ) )
		EditPrefPtr->dnslookup = 1;
	if ( IsChecked( IDC_PREFGEN_DNSCACHE ) )
		EditPrefPtr->dnslookup = 2;
	if ( IsChecked( IDC_PREFGEN_DNSNO ) )
		EditPrefPtr->dnslookup = 0;
	{ char txt[32];
		GetText( IDC_PREFGEN_DNSNUM, txt ,12 );
		EditPrefPtr->dnsAmount = myatoi( txt );
	}
	EditPrefPtr->ftp_passive = (short)IsChecked( IDC_PREFFTP_PASSIVE );
	EditPrefPtr->page_remotetitle = (char)IsChecked( IDC_PREF_REMOTETITLE );	

	hDlg = ghAnalysis3Dlg;
	
	hDlg = ghAnalysis4Dlg;
	EditPrefPtr->useCGI = (char)IsChecked( IDC_PREFGEN_USECGI );
	EditPrefPtr->retain_variable = (short)IsChecked( IDC_PREFGEN_RETAINVAR );
	GetText( IDC_PREFGEN_RETAINVARLIST, EditPrefPtr->retain_variablelist ,255 );
	if ( IS_ENT )
		EditPrefPtr->streaming = (char)IsChecked( IDC_PREFGEN_FLUSH );
}



float GetGUIFloat( HWND hDlg, IDC id )
{
	char numStr[32];
	GetText( id, numStr ,12 );
	return (float)(atof( numStr ));

}


void	Stats_GUItoData( void )
{
	char	numStr[12];
	long	t;
	HWND hDlg = ghStats2Dlg;

	if ( IsChecked( IDC_PREFGRAPH_3DBARS ) )
		EditPrefPtr->graph_style = GRAPH_3DSTYLE;
	else
		EditPrefPtr->graph_style = GRAPH_2DSTYLE;

	EditPrefPtr->graph_wider = (char)IsChecked( IDC_PREFGRAPH_WIDER );
	EditPrefPtr->paletteType = (char)IsChecked( IDC_PREFGRAPH_WEBPAL );
	EditPrefPtr->corporate_look = (char)IsChecked( IDC_PREFGRAPH_CORP );

	GetText( IDC_PREFVD_COST, numStr ,12 );
	EditPrefPtr->dollarpermeg = (short)(atof( numStr ) * 1000);

	GetText( IDC_PREFSTAT_SESSIONTIME, numStr , 4 );
	t = atoi( numStr );
	if ( t > 255 ) t = 255;
	EditPrefPtr->session_timewindow = (char)t;

	GetText( IDC_COOKIEVAR, EditPrefPtr->cookievar , 128 );
	GetText( IDC_URLVAR, EditPrefPtr->urlsessionvar , 128 );
}



typedef	long ControlID;

void	Stats_DatatoGUI( void )
{
	char	numStr[12];
	HWND hDlg = ghStats1Dlg;
#ifndef DEF_DEBUG
	Hide( IDC_STATFILTER );
	Hide( IDC_STATFILTER2 );
	Hide( IDC_STATFILTERTXT );
#endif
	
	hDlg = ghStats2Dlg;
	CheckIt( IDC_PREFGRAPH_3DBARS, EditPrefPtr->graph_style );
	CheckIt( IDC_PREFGRAPH_WIDER, EditPrefPtr->graph_wider );
	CheckIt( IDC_PREFGRAPH_WEBPAL, EditPrefPtr->paletteType );
	CheckIt( IDC_PREFGRAPH_CORP, EditPrefPtr->corporate_look );

	sprintf( numStr, "%.3f", EditPrefPtr->dollarpermeg/1000.0 );
	SetText( IDC_PREFVD_COST, numStr);

	sprintf( numStr, "%d", EditPrefPtr->session_timewindow );
	SetText( IDC_PREFSTAT_SESSIONTIME, numStr);
	SetText( IDC_COOKIEVAR, EditPrefPtr->cookievar);
	SetText( IDC_URLVAR, EditPrefPtr->urlsessionvar);
}








void	VDomains_GUItoData( void )
{
	long	num,i;
	char	numStr[12], *p;
	HWND hDlg = ghVDomains1Dlg;

	EditPrefPtr->multidomains = 0;
	//EditPrefPtr->multimonths = 0;
	EditPrefPtr->multivhosts = 0;
	i = GetPopupNum( IDC_VDREPORTBY );

	switch( i ){
		case 1:	EditPrefPtr->multidomains = MAX_DOMAINS; break;
		case 2:	EditPrefPtr->multivhosts = 1; break;
		case 3:	EditPrefPtr->multivhosts = 2; break;
		case 4:	EditPrefPtr->multivhosts = 3; break;
		case 5:	EditPrefPtr->multivhosts = 4; break;
		case 6:	EditPrefPtr->multivhosts = 5; break;
	}

	i = GetPopupNum( IDC_VDSORTBY );

	switch( i ){
		case 0:	EditPrefPtr->VDsortby = SORT_NAMES; break;
		case 1:	EditPrefPtr->VDsortby = SORT_SIZES; break;
		case 2:	EditPrefPtr->VDsortby = SORT_REQUESTS; break;
		case 3:	EditPrefPtr->VDsortby = SORT_PAGES; break;
		case 4:	EditPrefPtr->VDsortby = SORT_DATE; break;
	}
	EditPrefPtr->vhost_seqdirs = (char)IsChecked( IDC_PREFVD_SEQDIR );
	GetText( IDC_VDMULTIDIR, EditPrefPtr->multireport_path, 256 );
	i = strlen(EditPrefPtr->multireport_path);
	p = &EditPrefPtr->multireport_path[i-1];
	if( *p == '/' || *p == ':' )
		*p = 0;

	hDlg = ghVDomains4Dlg;
	EditPrefPtr->clusteringActive=IsChecked( IDC_PREFVD_CLUSTER );

	GetText( IDC_PREFVD_CLUSTERNUM, numStr, 12 );
	num = atoi( numStr );
	if ( num == 0 )
		num = GetClusterValue();
	EditPrefPtr->numClustersInUse=(char)num;

#ifdef DEF_APP_FIREWALL
	hDlg = ghVDomains5Dlg;
	if( IsChecked( IDC_PREFVD_SLEEP ) ){
		long numtype;

		GetText( IDC_PREFVD_NUM, numStr, 12 );
		num = atoi( numStr );
		numtype = GetPopupNum( IDC_PREFVD_TYPE );
		switch( numtype ){
			case 0 :	num = num * ONEMINUTE;	break;
			case 1 :	num = num * ONEHOUR;	break;
			case 2 :	num = num * ONEDAY;		break;
			case 3 :	num = num * ONEWEEK;	break;
			case 4 :	num = num * ONEMONTH;	break;
		}
		EditPrefPtr->live_sleeptype = (short)num;
	} else
		EditPrefPtr->live_sleeptype = 0;
#endif
}

void	VDomains_DatatoGUI( void )
{
	char	numStr[256];
	HWND hDlg = ghVDomains1Dlg;

	Disable( IDC_VDMULTIDIR );
	if ( EditPrefPtr->multidomains > 0 ){
		SetPopupNum( IDC_VDREPORTBY, 1 );
	} else {
		if ( EditPrefPtr->multivhosts == 1 )
			SetPopupNum( IDC_VDREPORTBY, 2 );
		else
		if ( EditPrefPtr->multivhosts == 2 )
			SetPopupNum( IDC_VDREPORTBY, 3 );
		else
		if ( EditPrefPtr->multivhosts == 3 )
			SetPopupNum( IDC_VDREPORTBY, 4 );
		else
		if ( EditPrefPtr->multivhosts == 4 ){
			SetPopupNum( IDC_VDREPORTBY, 5 );
			Enable( IDC_VDMULTIDIR );
		} else
		if ( EditPrefPtr->multivhosts == 5 ){
			SetPopupNum( IDC_VDREPORTBY, 6 );
		} else
			SetPopupNum( IDC_VDREPORTBY, 0 );
	}

	switch( EditPrefPtr->VDsortby ){
		case SORT_NAMES:	SetPopupNum( IDC_VDSORTBY, 0 ); break;
		case SORT_SIZES:	SetPopupNum( IDC_VDSORTBY, 1 ); break;
		case SORT_REQUESTS: SetPopupNum( IDC_VDSORTBY, 2 ); break;
		case SORT_PAGES:	SetPopupNum( IDC_VDSORTBY, 3 ); break;
		case SORT_DATE:		SetPopupNum( IDC_VDSORTBY, 4 ); break;
	}

	CheckIt( IDC_PREFVD_SEQDIR, EditPrefPtr->vhost_seqdirs );
	SetText( IDC_VDMULTIDIR, EditPrefPtr->multireport_path );
	sprintf( numStr, "Virtual hosts licensed to analyze: %d", MAX_DOMAINS );
	SetText( IDC_VDTOTAL, numStr );
	
	hDlg = ghVDomains4Dlg;
	if ( GetClusterValue() ){
		Enable( IDC_PREFVD_CLUSTER );
		CheckIt( IDC_PREFVD_CLUSTER, EditPrefPtr->clusteringActive );
		sprintf( numStr, "%d", EditPrefPtr->numClustersInUse );
		if( EditPrefPtr->clusteringActive )
		{
			Enable( IDC_PREFVD_CLUSTERNUM );
		}
		else
		{
			Disable( IDC_PREFVD_CLUSTERNUM );
		}
		SetText( IDC_PREFVD_CLUSTERNUM, numStr );
	} else {
		Disable( IDC_PREFVD_CLUSTER );
		Disable( IDC_PREFVD_CLUSTERNUM );
	}
	sprintf( numStr, "%d", GetClusterValue() );
	SetText( IDC_PREFVD_CLUSTERINFO, numStr );

#ifdef DEF_APP_FIREWALL	
	hDlg = ghVDomains5Dlg;
	if ( EditPrefPtr->live_sleeptype ){
		long sleeptime = EditPrefPtr->live_sleeptype, num, numtype;

		CheckON( IDC_PREFVD_SLEEP );
		Enable( IDC_PREFVD_TYPE ); Enable( IDC_PREFVD_NUM ); 
		//Enable( IDC_PREFVD_TYPE2 ); Enable( IDC_PREFVD_NUM2 ); 

		if ( sleeptime >= ONEMINUTE && sleeptime < ONEHOUR ) {
			num = sleeptime/ONEMINUTE;  numtype = 0; } else
		if ( sleeptime >= ONEHOUR && sleeptime < ONEDAY ) {
			num = sleeptime/ONEHOUR;  numtype = 1; } else
		if ( sleeptime >= ONEDAY ) {
			num = sleeptime/ONEDAY;  numtype = 2; }
		SetPopupNum( IDC_PREFVD_TYPE , numtype );
		sprintf( numStr, "%d", num );
		SetText( IDC_PREFVD_NUM, numStr );

	} else {
		CheckOFF( IDC_PREFVD_SLEEP );
		Disable( IDC_PREFVD_TYPE );
		Disable( IDC_PREFVD_NUM ); 
		//Disable( IDC_PREFVD_TYPE2 );Disable( IDC_PREFVD_NUM2 ); 
	}
#endif
}



void	PostProc_GUItoData( void )
{
	char	server[128];
	char	name[128];
	char	passwd[128];
	char	path[256];

	HWND hDlg = ghPostProc1Dlg;
	EditPrefPtr->postproc_rarchive = (char)IsChecked( IDC_PP_ZIPREPORT );
	EditPrefPtr->postproc_larchive = (char)IsChecked( IDC_PP_COMPRESSLOG );
	EditPrefPtr->postproc_deletereport = (char)IsChecked( IDC_PP_DELETEREPORT );
	EditPrefPtr->postproc_deletelog = (char)IsChecked( IDC_PP_DELETELOG );

	hDlg = ghPostProc2Dlg;
	EditPrefPtr->postproc_uploadreport = 0;
	if ( (char)IsChecked( IDC_PP_UPLOADREPORT ) )
	{
		EditPrefPtr->postproc_uploadreport = 1;
		if ( (char)IsChecked( IDC_PP_UPLOADFOLDER ) )
			EditPrefPtr->postproc_uploadreport = 2;
	}

	EditPrefPtr->postproc_uploadlog = (char)IsChecked( IDC_PP_UPLOADLOG );

	GetText( IDC_PP_HOST, server, 64 );
	GetText( IDC_PP_USER, name, 64 );
	GetText( IDC_PP_PASSWD, passwd, 64 );
	GetText( IDC_PP_PATH, path, 256 );
	if ( server[0] ){
		long len = strlen( path );
		if ( path[len-1] != '/' && !strchr( path, '.' ) )
			strcat( path, "/" );
		mystrcpy( EditPrefPtr->postproc_uploadloglocation, "ftp://" );
		MakeURLfromDetails( EditPrefPtr->postproc_uploadloglocation, server, name, passwd, path );
	}

	GetText( IDC_PP_HOST2, server, 64 );
	GetText( IDC_PP_USER2, name, 64 );
	GetText( IDC_PP_PASSWD2, passwd, 64 );
	GetText( IDC_PP_PATH2, path, 256 );
	if ( server[0] ){
		long len = strlen( path );
		if ( path[len-1] != '/' && !strchr( path, '.' ) )
			strcat( path, "/" );
		mystrcpy( EditPrefPtr->postproc_uploadreportlocation, "ftp://" );
		MakeURLfromDetails( EditPrefPtr->postproc_uploadreportlocation, server, name, passwd, path );
	}

	hDlg = ghPostProc3Dlg;
	EditPrefPtr->postproc_emailon = (short)IsChecked( IDC_PP_EMAILON );
	GetText( IDC_PP_EMAIL, EditPrefPtr->postproc_email , 255 );
	GetText( IDC_PP_EMAILFROM, EditPrefPtr->postproc_emailfrom , 128 );
	GetText( IDC_PP_EMAILSUB, EditPrefPtr->postproc_emailsub , 128 );
	GetText( IDC_PP_EMAILSMTP, EditPrefPtr->postproc_emailsmtp , 128 );
	GetText( IDC_PP_MSG, EditPrefPtr->postproc_emailmsg , 4090 );

}

void	Billing_DatatoGUI(void)
{
	SetBillingSetupDetails();
}

void	PostProc_DatatoGUI( void )
{
	char	server[128];
	char	name[128];
	char	passwd[128];
	char	path[256];
	HWND hDlg = ghPostProc1Dlg;
	CheckIt( IDC_PP_ZIPREPORT, EditPrefPtr->postproc_rarchive );
	CheckIt( IDC_PP_COMPRESSLOG, EditPrefPtr->postproc_larchive );
	CheckIt( IDC_PP_DELETEREPORT, EditPrefPtr->postproc_deletereport );
	CheckIt( IDC_PP_DELETELOG, EditPrefPtr->postproc_deletelog );

	hDlg = ghPostProc2Dlg;
	CheckIt( IDC_PP_UPLOADLOG, EditPrefPtr->postproc_uploadlog );
	CheckIt( IDC_PP_UPLOADREPORT, EditPrefPtr->postproc_uploadreport );
	CheckIt( IDC_PP_UPLOADFOLDER, (EditPrefPtr->postproc_uploadreport==2) );

	if ( EditPrefPtr->postproc_uploadloglocation[0] ){
		ExtractUserFromURL( EditPrefPtr->postproc_uploadloglocation, server, name, passwd, path );
		SetText( IDC_PP_HOST, server );
		SetText( IDC_PP_USER, name );
		SetText( IDC_PP_PASSWD, passwd );
		SetText( IDC_PP_PATH, path );
	} else {
		server[0] = 0;
		SetText( IDC_PP_HOST, server );
		SetText( IDC_PP_USER, server );
		SetText( IDC_PP_PASSWD, server );
		SetText( IDC_PP_PATH, server );
	}

	if ( EditPrefPtr->postproc_uploadreportlocation[0] ){
		ExtractUserFromURL( EditPrefPtr->postproc_uploadreportlocation, server, name, passwd, path );
		SetText( IDC_PP_HOST2, server );
		SetText( IDC_PP_USER2, name );
		SetText( IDC_PP_PASSWD2, passwd );
		SetText( IDC_PP_PATH2, path );
	} else {
		server[0] = 0;
		SetText( IDC_PP_HOST2, server );
		SetText( IDC_PP_USER2, server );
		SetText( IDC_PP_PASSWD2, server );
		SetText( IDC_PP_PATH2, server );
	}

	if ( EditPrefPtr->postproc_uploadlog ){
		Enable( IDC_PP_PATH );
		Enable( IDC_PP_HOST );
		Enable( IDC_PP_USER );
		Enable( IDC_PP_PASSWD );
	} else {
		Disable( IDC_PP_PATH );
		Disable( IDC_PP_HOST );
		Disable( IDC_PP_USER );
		Disable( IDC_PP_PASSWD );
	}

	if ( EditPrefPtr->postproc_uploadreport ){
		Enable( IDC_PP_PATH2 );
		Enable( IDC_PP_HOST2 );
		Enable( IDC_PP_USER2 );
		Enable( IDC_PP_PASSWD2 );
	} else {
		Disable( IDC_PP_PATH2 );
		Disable( IDC_PP_HOST2 );
		Disable( IDC_PP_USER2 );
		Disable( IDC_PP_PASSWD2 );
	}
	
	
	hDlg = ghPostProc3Dlg;
	if ( strlen( EditPrefPtr->postproc_emailsmtp ) < 2 )
		mystrcpy( EditPrefPtr->postproc_emailsmtp, GetSMTPServer( EditPrefPtr->postproc_email ) );
	if ( strlen( EditPrefPtr->postproc_emailsub ) < 2 )
		mystrcpy( EditPrefPtr->postproc_emailsub, "Report Notification" );
	SetText( IDC_PP_EMAIL, EditPrefPtr->postproc_email );
	SetText( IDC_PP_MSG, EditPrefPtr->postproc_emailmsg );
	SetText( IDC_PP_EMAILSUB, EditPrefPtr->postproc_emailsub );
	SetText( IDC_PP_EMAILFROM, EditPrefPtr->postproc_emailfrom );
	SetText( IDC_PP_EMAILSMTP, EditPrefPtr->postproc_emailsmtp );
	CheckIt( IDC_PP_EMAILON, EditPrefPtr->postproc_emailon );

	if ( EditPrefPtr->postproc_emailon ){
		Enable( IDC_PP_EMAIL );
		Enable( IDC_PP_EMAILFROM );
		Enable( IDC_PP_EMAILSMTP );
		Enable( IDC_PP_EMAILSUB );
		Enable( IDC_PP_MSG );
		Enable( IDC_EMAIL_TEST );
	} else {
		Disable( IDC_PP_EMAIL );
		Disable( IDC_PP_EMAILFROM );
		Disable( IDC_PP_EMAILSUB );
		Disable( IDC_PP_EMAILSMTP );
		Disable( IDC_PP_MSG );
		Disable( IDC_EMAIL_TEST );
	}
}







void	PrefsCustom_GUItoData( HWND hDlg )
{
	char txt[32];

	EditPrefPtr->custom_format = (short)IsChecked( IDC_CUSTOM_USE );
	EditPrefPtr->custom_dateformat = (short)GetPopupNum( IDC_CUSTOM_DATE );
	EditPrefPtr->custom_timeformat = (short)GetPopupNum( IDC_CUSTOM_TIME );
	GetText( IDC_CUSTOM_SEP, txt, 32 );
	if ( !strcmpd( "comma", txt ) )
		EditPrefPtr->custom_seperator = 44;
	else
	if ( !strcmpd( "space", txt ) )
		EditPrefPtr->custom_seperator = 32;
	else
	if ( !strcmpd( "tab", txt ) )
		EditPrefPtr->custom_seperator = 9;
	else
	if ( txt[1] == 'x' || txt[1] == 'X' )
		EditPrefPtr->custom_seperator = HexToChar( txt );
	else
	if ( isdigit( (txt[0]) ) )
		EditPrefPtr->custom_seperator = (short)myatoi( txt );
	else
		EditPrefPtr->custom_seperator = txt[0];
}



void	PrefsCustom_DatatoGUI( HWND hDlg )
{
	char txt[32];

	CheckIt( IDC_CUSTOM_USE, EditPrefPtr->custom_format );
	SetPopupNum( IDC_CUSTOM_DATE, EditPrefPtr->custom_dateformat );
	SetPopupNum( IDC_CUSTOM_TIME, EditPrefPtr->custom_timeformat );
	if ( EditPrefPtr->custom_seperator <= 32 )
		sprintf( txt, "%d", EditPrefPtr->custom_seperator );
	else
		sprintf( txt, "%c", EditPrefPtr->custom_seperator );
	SetText( IDC_CUSTOM_SEP, txt );
	SetSliderPos( IDC_CUSTOM_POS, EditPrefPtr->custom_dataIndex[ 0 ] );
}






void	PrefsNotify_GUItoData( HWND hDlg )
{
	if ( hDlg ){
		if ( IsChecked( IDC_NOTIFY_ON1 ) )		EditPrefPtr->notify_type = 0; else
		if ( IsChecked( IDC_NOTIFY_ON2 ) )		EditPrefPtr->notify_type = 1; else
		if ( IsChecked( IDC_NOTIFY_ON3 ) )		EditPrefPtr->notify_type = 2;

		GetText( IDC_NOTIFYEMAIL, EditPrefPtr->notify_destination, 255 );
	}
}


void	PrefsNotify_DatatoGUI( HWND hDlg )
{
	if ( hDlg ){
		CheckOFF( IDC_NOTIFY_ON1 );
		CheckOFF( IDC_NOTIFY_ON2 );
		CheckOFF( IDC_NOTIFY_ON3 );

		if ( EditPrefPtr->notify_type == 0 ) CheckON( IDC_NOTIFY_ON1 ); else
		if ( EditPrefPtr->notify_type == 1 ) CheckON( IDC_NOTIFY_ON2 ); else
		if ( EditPrefPtr->notify_type == 2 ) CheckON( IDC_NOTIFY_ON3 );

		SetText( IDC_NOTIFYEMAIL, EditPrefPtr->notify_destination );
	}
}




char	CustomFieldStr[][32] = {
	"Date", "Time", "Status", "Client", "Source", "File", "Agent", "Bytes", "BytesIn",
	"Duration", "Referal", "UserName", "VHost", "Method", "Protocol", "Port",0,0
	};

//------------------------------------------------------------------------------------------------------

void ShowCustomConfigInfo( HWND hDlg )
{
	char	txt[32];
	char	lineTxt[256];
	long	lp=0;

	lineTxt[0] = 0;
	while( lp<16 ){
		if ( EditPrefPtr->custom_dataIndex[lp] > 0 ){
			char t2[128];
#ifdef DEF_UNIX
			lp = GetPopupNum( IDC_CUSTOM_DATA );
			sprintf( txt, "%s[%d]  ", CustomFieldStr[lp], EditPrefPtr->custom_dataIndex[lp] );
#else
			GetPopupText( IDC_CUSTOM_DATA, lp, (char*)t2 );
			sprintf( txt, "%s[%d]  ", t2, EditPrefPtr->custom_dataIndex[lp] );
#endif
			strcat( lineTxt, txt );
		}
		lp++;
	}
	SetText( IDC_CUSTOM_INFO, lineTxt );
}








//PrefsDatatoGUI
void	PrefsFromGUI( void )
{
	gSaved = FALSE;

#if DEF_WINDOWS
	PreProc_GUItoData();
#endif
	Report_GUItoData();
	Analysis_GUItoData();
	Stats_GUItoData();
	PostProc_GUItoData();
	PrefsCustom_GUItoData( ghCustom1Dlg );
	VDomains_GUItoData();
#ifdef	DEF_APP_FIREWALL
	PrefsNotify_GUItoData( ghVDomains4Dlg );
#endif
}


void	PrefsIntoGUI( void )
{

#if DEF_WINDOWS
	OutDebug( "DatatoGUI: PreProc" );
	PreProc_DatatoGUI();
#endif
	OutDebug( "DatatoGUI: Report" );
	Report_DatatoGUI();

	OutDebug( "DatatoGUI: Analysis" );
	Analysis_DatatoGUI();

	OutDebug( "DatatoGUI: Stats" );
	Stats_DatatoGUI();

	OutDebug( "DatatoGUI: PostProc" );
	PostProc_DatatoGUI();

	OutDebug( "DatatoGUI: Billing" );
	Billing_DatatoGUI();

	OutDebug( "DatatoGUI: Custom" );
	PrefsCustom_DatatoGUI( ghCustom1Dlg );

	OutDebug( "DatatoGUI: VDomains" );
	VDomains_DatatoGUI();
#ifdef	DEF_APP_FIREWALL
	OutDebug( "DatatoGUI: Notify" );
	PrefsNotify_DatatoGUI( ghVDomains4Dlg );
#endif
}

