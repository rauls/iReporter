#include "FWA.h"

#include <windows.h>    // includes basic windows functionality
#include <commctrl.h>   // includes the common control header
#include <stdio.h>

#include "StringLoader.h"

// Local Header Files
#include "PrefsGUI.h"
#include "Winmain.h"
#include "Winutil.h"
#include "Myansi.h"
#include "resource.h"
#include "serialReg.h"
#include "datetime.h"
#include "myansi.h"

#include "Editpath.h"
#include "config.h"
#include "config_struct.h"
//extern "C"
//{
	#include "PrefsIO.h"
//};

#include "translate.h"
#include "GlobalPaths.h"
#include "ResDefs.h"	// for GetString() etc

#include "HelpTextMap.h"
#include "engine_process.h"	// for ClearLookupCacheFile()

#include "HelpCard.h"
#include "V5Database.h"	// for isV5Database()

extern BOOL GetLoadDialog( char *putNamehere, char *initDir, char *initfile, char *filter, char *title );

/****************************************************************************
* 
*    Global and External variables
*
****************************************************************************/

extern ComboBoxColors comboBoxColors[];
extern	LONG            iDepth;

#define	EnableID( id,x )		EnableWindow( GetDlgItem( hDlg, id), x )

/****************************************************************************
* 
*    Global and External variables
*
****************************************************************************/
//extern "C" {
HWND			ghPrefsDlg = NULL,
    			ghPreProcDlg = NULL,
    			ghPreProcTabDlg = NULL,
    			ghPreProc1Dlg = NULL,
    			ghPreProc2Dlg = NULL,
    			ghReportDlg = NULL,
    			ghReportTabDlg = NULL,
    			ghReport1Dlg = NULL,
    			ghReport2Dlg = NULL,
    			ghReportHTMLDlg = NULL,
    			ghReportPDFDlg = NULL,
//				ghReportPDFDlgAdv = NULL,
    			ghReport4Dlg = NULL,
    			ghAnalysisDlg = NULL,
    			ghAnalysisTabDlg = NULL,
    			ghAnalysis1Dlg = NULL,
    			ghAnalysis2Dlg = NULL,
    			ghAnalysis3Dlg = NULL,
    			ghAnalysis4Dlg = NULL,
    			ghFiltersDlg = NULL,
    			ghFiltersTabDlg = NULL,
    			ghFiltersInDlg = NULL,
    			ghFiltersOutDlg = NULL,
    			ghStatsDlg = NULL,
    			ghStatsTabDlg = NULL,
    			ghStats1Dlg = NULL,
    			ghStats2Dlg = NULL,
    			ghStats3Dlg = NULL,
				ghRoutesKeyVisitorDlg = NULL,
				ghRoutesRouteToKeyPagesDlg = NULL,
				ghRoutesRouteFromKeyPagesDlg = NULL,
    			ghStats4Dlg = NULL,
    			ghStats5Dlg = NULL,

				ghRoutesDlg = NULL,
    			ghRoutesTabDlg = NULL,

				ghBillingDlg		= NULL,
				ghBillingTabDlg		= NULL,
				ghBillingSetupDlg	= NULL,
				ghBillingChargesDlg	= NULL,

    			ghAddsDlg = NULL,
    			ghAddsTabDlg = NULL,
    			ghAddsBannersDlg = NULL,
    			ghAddsCampDlg = NULL,
    			ghHtmlColorDlg = NULL,
    			ghPostProcDlg = NULL,
    			ghPostProcTabDlg = NULL,
    			ghPostProc1Dlg = NULL,
    			ghPostProc2Dlg = NULL,
    			ghPostProc3Dlg = NULL,
    			ghCustomDlg = NULL,
    			ghCustomTabDlg = NULL,
    			ghCustom1Dlg = NULL,
				ghCustom2Dlg = NULL,
    			ghVDomainsDlg = NULL,
    			ghVDomainsTabDlg = NULL,
    			ghVDomains1Dlg = NULL,
    			ghVDomains2Dlg = NULL,
    			ghVDomains3Dlg = NULL,
    			ghVDomains4Dlg = NULL,
    			ghVDomains5Dlg = NULL,
    			ghProxyDlg = NULL,
    			ghProxyTabDlg = NULL,
    			ghProxyIPDlg = NULL,
    			ghROIDlg = NULL,
    			ghRemoteDlg = NULL,
    			ghDebugDlg = NULL,
    			ghDebugTabDlg = NULL,
    			ghDebug1Dlg = NULL,
				ghEnd;

struct	App_config			*EditPrefPtr,	*OrigPrefPtr;

//};

static HWND		hWndListView, hWndEditSettingsView, hWndExtSettingsView, hWndPathListView, hWndPathWnd;
static int		prefsReturn = FALSE;
static char		*prefsTitle = NULL;
static long		gPaneNum = 'repo';
static long		gPathItemSel = 0;

static struct	App_config	SavedPrefStruct;			// backed up ones, for revert

static	HWND	sg_hwndToolTip = NULL;

extern struct	App_config	SchedulePrefStruct;		// schedule settings
extern struct	App_config	MyPrefStruct;		// global settings

LRESULT EditpathNotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


/****************************************************************************
* 
*    Currently unused ids that may be reused in the future
*
****************************************************************************/
#define IDC_PREFREF_CLR		1
#define IDC_PREFADD_CLR		2			
#define IDC_EDITPATH_CLR	3
#define IDC_FRAMEDHTML		4
#define IDC_PREFVD_WEEKLY	5
#define IDC_PREFVD_MONTHLY	6
#define IDC_GLOB_PAGECLR	7
#define IDC_PP_SENDTO		8
#define IDC_PP_LOCATION		9
#define IDC_PP_LOGBROWSE	10
#define IDC_PP_BROWSE		11
#define IDC_PP_FTP			12
#define IDC_PREFVD_CLR		13
#define IDC_PREFS_HELP		14
#define IDC_PREFVD_TYPE2	15
#define IDC_PREFVD_NUM2		16

HFONT	hfixedFont;
// ****************************************************************************
// Method:		UpdateToolBar
//
// Abstract:	This method updates the Toolbar with the text that appears in
//				the tool tip.
//
// Declaration: UpdateToolBar(HWND hWnd, long lControlId)
//
// Arguments:	long lControlId		: The id of the resource which is the control
//									  within the dialog.
//				TOOLTIPDATA **tips	: The set of ControlId/ToolTipString pairs.
//
// Returns:		void
// ****************************************************************************
void
UpdateToolBar(HWND hWnd, long lControlId)
{
	// ****************************************************************************
	// If this is our first time through then lets load the XML and create the
	// ToolTip window.
	// ****************************************************************************
	if (sg_hwndToolTip == 0)
	{
		// ****************************************************************************
		// Load the XML and create the mapping between ControlID and HelpText.
		// ****************************************************************************
		(void)HelpText_Load();

		// ****************************************************************************
		// Create the actual tooltip window.
		// ****************************************************************************
		sg_hwndToolTip = CreateWindowEx( 0, 
			TOOLTIPS_CLASS, 
			NULL, 
			WS_POPUP | TTS_ALWAYSTIP, 
			CW_USEDEFAULT, 
			CW_USEDEFAULT, 
			12, 
			12, 
			NULL,
			NULL, 
			0,	
			NULL);
	}

	// ****************************************************************************
	// Prevent multiple updates to the tooltip & helpbar.
	// ****************************************************************************
	if (::GetWindowLong(sg_hwndToolTip,GWL_USERDATA) == lControlId)
		return;
	(void)::SetWindowLong(sg_hwndToolTip,GWL_USERDATA,lControlId);


	// ****************************************************************************
	// Lookup the text values needed
	// ****************************************************************************
	const char* szHelpText = HelpText_Lookup(lControlId);
	// *********************************************************
	// And send the text to the helpbar.
	// *********************************************************
	if( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
	{
		SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  szHelpText);
	}

	// *********************************************************
	// Do the Same for the Location-description in Begin mode.
	// *********************************************************
#ifdef	DEF_DEBUG
	if( ghPrefsDlg )
	{
		const char* szDescText = HelpText_LookupDesc(lControlId);
		if (szDescText != NULL)
			SetDlgItemText(ghPrefsDlg, IDC_EDIT_CONTROLID,  szDescText);
	}
#endif


	// *************************************************************
	// If we do NOT have a TOOLBAR to populate with the help string
	// then we us the TOOLTIP window.
	// At this stage I don't believe there are any windows which
	// will be not be opened the call this function to do Tooltips.
	// *************************************************************
	if( !ghPrefsDlg || !IsWindowEnabled(ghPrefsDlg))
	{
		// *********************************************************
		// Create the tooltip message.
		// *********************************************************
		TOOLINFO			ti;
		ZeroMemory(&ti, sizeof(ti));
		ti.cbSize	= sizeof(ti);
		ti.uFlags	= TTF_IDISHWND | TTF_SUBCLASS;  //TTF_SUBCLASS causes the tooltip to automatically subclass the window and look for the messages it is interested in.
		ti.hwnd		= hWnd;
		ti.uId		= (UINT)GetDlgItem( hWnd, lControlId );
		ti.lpszText	= (char*)szHelpText;

		// *********************************************************
		// If we moveit and then add it we will not infinitly
		// increase the number of tooltip in this global tooltip
		// window.
		// *********************************************************
		long l1 = SendMessage(sg_hwndToolTip, TTM_DELTOOL, 0, (LPARAM)&ti);	// remove
		long l2 = SendMessage(sg_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);	// add
	}
}
// *******************************************************************************


#define	NUM_EDITCOLUMNS	2

HWND InitGenericListView ( HWND hWndParent, long controlid, long stringid, long columns, char* string )
{
	LV_COLUMN lvC;      // List View Column structure
	HWND hWndList;      // Handle to the list view window
	int index;          // Index used in for loops
	char szText[512];
	char *stringPtr[64], *p;
	long	i = 0, c = 0, num_columns;

	if ( stringid != 0 )
		LoadString( hInst, stringid, szText, 512 );
	else
		mystrcpy( szText, string );

	p = szText;
	while( p ){
		stringPtr[c] = p;
		p = mystrstr( p, LIST_VIEW_SEP );
		if ( p ){
			*p = 0;
			p += mystrlen( LIST_VIEW_SEP );
		}
		c++;
		stringPtr[c] = 0;
	}
	num_columns = c;

	hWndList = GetDlgItem( hWndParent, controlid );

	// Now initialize the columns we will need
	// Initialize the LV_COLUMN structure
	// the mask specifies that the .fmt, .ex, width, and .subitem members 
	// of the structure are valid,
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	//lvC.fmt = LVCFMT_CENTER;  // centre align the column 
	lvC.cx = 80;            // width of the column, in pixels
	lvC.pszText = szText;
	// Add the columns.
	for (index = 0; index < num_columns; index++)
	{
		if ( index < num_columns-1 )
			lvC.cx = strlen(stringPtr[index])*15;            // width of the column, in pixels
		else
			lvC.cx = 320;
		lvC.iSubItem = index;
		strcpy( szText, stringPtr[index] );
		if (ListView_InsertColumn(hWndList, index, &lvC) == -1){
			UserMsg( "ListView_InsertColumn() == -1" );
			return NULL;
		}
	}

	if ( columns )
	{
		for (index = 0; index < columns; index++){
			AddItemToListView( hWndList, index, columns, 0, LIST_VIEW_SEP );
		}
	}
	
	ListView_SetExtendedListViewStyleEx( hWndList, LVS_EX_FULLROWSELECT , LVS_EX_FULLROWSELECT );


	return (hWndList);
}







 


void EditPathToGUI( HWND hDlg, int number )
{
	long			num=0,numtype=0;
	EditPathRecPtr	dataP;

	if ( dataP = GetEditPath( EditPrefPtr->EditPaths, number ) ){
		SetText( IDC_EDITPATH_HOST, dataP->vhost );
		SetText( IDC_EDITPATH_PATH, dataP->path );
	}
}


LRESULT EditPathNotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
	NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
	int				itemSel = pNm->iItem;
static
	char			szText[MAX_PATH]="(null)\0";    // Place to store some text
	EditPathRecPtr	dataP;

	if (wParam == IDC_PREFS_EDITPATH){
		switch(pLvdi->hdr.code)	{
			case LVN_GETDISPINFO:
				itemSel = pLvdi->item.iItem;
				if ( dataP = GetEditPath( EditPrefPtr->EditPaths, itemSel + 1 ) ){
					pLvdi->item.pszText = szText;
					switch (pLvdi->item.iSubItem){
						case 0:		strcpy( szText, dataP->vhost ); break;
						case 1:		strcpy( szText, dataP->path ); break;
						default:	break;
					}

				}
				break;
			case LVN_BEGINLABELEDIT:
				return TRUE;
				break;
			case LVN_ITEMCHANGING:
				gPathItemSel = itemSel + 1;
				if ( dataP = GetEditPath( EditPrefPtr->EditPaths, gPathItemSel ) )
					EditPathToGUI( hWnd, gPathItemSel );
				break;
		}
	}

	return 0L;
}

LRESULT CALLBACK EditPathProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hDlg = hWnd;
static
	HWND hPrevWnd;
	EditPathRecPtr dataP;

	switch (msg) {
		case WM_INITDIALOG:
			hWndPathListView = InitGenericListView( hDlg, IDC_PREFS_EDITPATH, IDS_EDITPATH, GetTotalEditPath( EditPrefPtr->EditPaths ), 0 );
			if ( hWndPathListView == NULL)
				UserMsg ( "Editpath Listview not created!" );
			SetText( IDC_EDITPATH_HOST, "your.virtual.host" );
			SetText( IDC_EDITPATH_PATH, "your/custom/path/" );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hWndPathListView );
			hWndPathListView = NULL;
            break;

		case WM_NOTIFY:
			return( EditPathNotifyHandler(hWnd, msg, wParam, lParam));
			break;

        case WM_COMMAND:
            switch (LOWORD(wParam) ) {
				case IDC_EDITPATH_ADD:
					{
						char	host[MAXEDITPATH_SIZE],path[MAXEDITPATH_SIZE];
						GetText( IDC_EDITPATH_HOST, host, MAXEDITPATH_SIZE );
						GetText( IDC_EDITPATH_PATH, path, MAXEDITPATH_SIZE );
						AddEditPath( &EditPrefPtr->EditPaths, host, path );
						AddItemToListView( hWndPathListView, GetTotalEditPath(EditPrefPtr->EditPaths)-1, NUM_EDITCOLUMNS, 0, LIST_VIEW_SEP );
						EditPathToGUI( hWnd, GetTotalEditPath(EditPrefPtr->EditPaths) );
						SelectListViewItem( hWndPathListView, GetTotalEditPath(EditPrefPtr->EditPaths)-1 );
					}
					break;

				case IDC_EDITPATH_DEL:
					ListView_DeleteItem( hWndPathListView, gPathItemSel-1 );
					DelEditPath( &EditPrefPtr->EditPaths, gPathItemSel );
					EditPathToGUI( hWnd, GetTotalEditPath(EditPrefPtr->EditPaths) );
					UpdateWindow( hWndPathListView );
					break;

				case IDC_EDITPATH_CLR:
					{
						long i;
						while( i=GetTotalEditPath(EditPrefPtr->EditPaths) ){
							ListView_DeleteItem( hWndPathListView, i-1 );
							DelEditPath( &EditPrefPtr->EditPaths, i );
						}
						EditPathToGUI( hWnd, GetTotalEditPath(EditPrefPtr->EditPaths) );
						UpdateWindow( hWndPathListView );
					}
					break;
				
				case IDC_EDITPATH_CHANGE:
					if ( dataP = GetEditPath( EditPrefPtr->EditPaths, gPathItemSel ) ){
						GetText( IDC_EDITPATH_HOST, dataP->vhost, MAXEDITPATH_SIZE );
						GetText( IDC_EDITPATH_PATH, dataP->path, MAXEDITPATH_SIZE );
						ListView_DeleteItem( hWndPathListView, gPathItemSel-1 );
						AddItemToListView( hWndPathListView, gPathItemSel-1, NUM_EDITCOLUMNS, 0, LIST_VIEW_SEP );
						ListView_Update( hWndPathListView , gPathItemSel-1 );
						UpdateWindow( hWndPathListView );
					}
					break;

				case IDC_EDITPATH_BROWSE:
					{
						char szText[MAXFILENAMESSIZE];
						if ( GetSaveDomainName( szText ) ){
							SetText( IDC_EDITPATH_PATH, szText );
						}
					}
					break;
            }
            break;
	}

    return FALSE;
}





















/*-------------------------------------------------------




				 Create custom controls



-------------------------------------------------------*/

WNDPROC wpOrigTabProc; 
 
// Subclass procedure 
LRESULT APIENTRY TabSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
//    if (uMsg == WM_ERASEBKGND) {
//		GradientBK( hwnd, wParam );
  //      return 1; 
//	}
    if( ForceBackgroundGradient( hwnd, uMsg, wParam, lParam ) )
		return 1;

    return CallWindowProc(wpOrigTabProc, hwnd, uMsg, 
        wParam, lParam); 
} 

void OverideTabControl( HWND w )
{
	// Subclass the edit control. 
	wpOrigTabProc = (WNDPROC) SetWindowLong(w, GWL_WNDPROC, (LONG) TabSubclassProc); 
            
}


HWND WINAPI DoCreateTabControl( HWND hwndParent, char **text, long count, long id ) {
    HWND hwndTab;
	TC_ITEM tie;
	int i;  

    // Get the dimensions of the parent window's client area, and 
    // create a tab control child window of that size. 
	hwndTab = GetDlgItem( hwndParent, id );
	tie.mask = TCIF_TEXT;
	tie.iImage = -1; 
    tie.pszText = 0;
	for (i = 0; i < count,text[i] ; i++) { 
	    tie.pszText = text[i];
		tie.cchTextMax = mystrlen(text[i]);
        if (TabCtrl_InsertItem( hwndTab, i, &tie) == -1) {
            DestroyWindow(hwndTab); 
			return NULL;         
		}     
	}
	//OverideTabControl( hwndTab );
	//SendMessage( hwndTab, WM_SETFONT, (UINT)(hCtrlFont), TRUE);
//SetWindowLong( hwndTab, GWL_STYLE, GetWindowLong( hwndTab, GWL_STYLE) | TCS_MULTILINE );
    return hwndTab;
} 







long Init_StringArray( char *commalist, char **array, int arraySize )
{
	char *p; int i=0, c=0;
	if ( commalist && array ){
		p = commalist;
		while( p ){
			array[c] = p;
			p = mystrchr( p, ',' );
			if ( p ){
				*p = 0;
				p++;
			}
			c++;
			if ( c >= arraySize )
				return c-1;
			array[c] = 0;
		}
	}
	return c;
}

#define ICON_NAMES_ARRAY_SIZE 12

/****************************************************************************
* 
*    FUNCTION: CreateListView(HWND)
*
*    PURPOSE:  Creates the list view window and initializes it
*
****************************************************************************/
HWND InitSettingsListView (HWND hWndParent )                                     
{
	HIMAGELIST hLarge;  // Handles to image lists for large icons
	HICON hIcon;        // Handle to an icon
	LV_COLUMN lvC;      // List View Column structure
	LV_ITEM lvI;        // List view item structure
	HWND hWndList;      // Handle to the list view window
	RECT rcl, rclist;           // Rectangle for setting the size of the window
	char szText[MAX_PATH];    // Place to store some text
	char szIconNames[MAXFILENAMESSIZE], *lpIconNames[ICON_NAMES_ARRAY_SIZE];
	int iSubItem;       // Index for inserting sub items
	int index;          // Index used in for loops
	long	bgcolor = 0xffffff, i;
	char	*iconList[] = {
		"IDI_PREPROC",
		"IDI_GENERALICON", 
		"IDI_ANALYSISICON",
		"IDI_FILTERSICON", 
		"IDI_STATSICON",
#ifdef DEF_FULLVERSION
		"IDI_ROUTESICON",
#endif
		"IDI_BILLINGICON",
#ifdef DEF_FULLVERSION
		"IDI_ADDSICON",
#endif
		"IDI_VDOMAINSICON",
		"IDI_POSTPROC",
#ifdef DEF_FULLVERSION
		"IDI_CUSTOMICON",
#endif
#ifdef DEF_FIREWALL
		"IDI_ROUTERICON",
#endif
#ifdef DEF_DEBUG
		"IDI_AAA_APPICON",
#endif
		0 };


	long numItems = 0;

#ifdef DEF_FIREWALL
	GetString( IDS_ICON_NAMESFIREWALL, szIconNames, MAXFILENAMESSIZE );
#elif DEF_FULLVERSION
	GetString( IDS_ICON_NAMESENT, szIconNames, MAXFILENAMESSIZE );
#else
	GetString( IDS_ICON_NAMES, szIconNames, MAXFILENAMESSIZE );
#endif

	Init_StringArray( szIconNames, lpIconNames, ICON_NAMES_ARRAY_SIZE );

	// Get the size and position of the parent window
	GetClientRect(hWndParent, &rcl);
	GetObjectRect( hWndParent, GetDlgItem( hWndParent, IDC_PREFS_LISTRECT ), &rclist );

	hWndList = GetDlgItem( hWndParent, IDC_PREFS_LISTRECT );

	// initialize the list view window
	// First, initialize the image lists we will need
	// create an image list for the small and large icons
	// FALSE specifies large icons - TRUE specifies small

	while( iconList[numItems] ){
		numItems++;
	}
	ListView_SetBkColor( hWndList, bgcolor );
	ListView_SetTextBkColor( hWndList, bgcolor );
	ListView_SetTextColor( hWndList, 0000000 );

	if( hLarge = ImageList_Create( LG_BITMAP_WIDTH, LG_BITMAP_HEIGHT, ILC_MASK|ILC_COLOR24 , numItems, 0 ) )
	{
		ImageList_SetBkColor( hLarge, CLR_NONE  );

		numItems = i = 0;

		while( iconList[numItems] ){
			hIcon = LoadIcon ( hInst, iconList[numItems] );		//hInst
			ImageList_AddIcon( hLarge, hIcon );
			numItems++;
		}


		// Make sure that all of the icons were added
		if (ImageList_GetImageCount(hLarge))
		{
			ListView_SetImageList(hWndList, hLarge, LVSIL_NORMAL);
			ListView_SetIconSpacing( hWndList, 0, LG_BITMAP_WIDTH );
		}
	}


	// Now initialize the columns we will need
	// Initialize the LV_COLUMN structure
	// the mask specifies that the .fmt, .ex, width, and .subitem members 
	// of the structure are valid,
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	//lvC.fmt = LVCFMT_CENTER;  // centre align the column 
	lvC.cx = 0;            // width of the column, in pixels
	lvC.pszText = szText;
	// Add the columns.
	for (index = 0; index <= NUM_COLUMNS; index++)
	{
		lvC.iSubItem = index;
		strcpy( szText, "Item" );
		if (ListView_InsertColumn(hWndList, index, &lvC) == -1){
			UserMsg( "ListView_InsertColumn() == -1" );
			return NULL;
		}
	}

	// Finally, let's add the actual items to the control
	// Fill in the LV_ITEM structure for each of the items to add
	// to the list.
	// The mask specifies the the .pszText, .iImage, .lParam and .state
	// members of the LV_ITEM structure are valid.
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvI.state = 0;      //
	lvI.stateMask = 0;  //
	for (index = 0; index < numItems; index++){
		lvI.iItem = index;
		lvI.iSubItem = 0;
		// The parent window is responsible for storing the text. The List view
		// window will send a LVN_GETDISPINFO when it needs the text to display/
		lvI.pszText = LPSTR_TEXTCALLBACK; 
		lvI.cchTextMax = MAX_ITEMLEN;
		lvI.iImage = index;
		lvI.lParam = (LPARAM)index;
		lvI.pszText = (LPSTR)lpIconNames[index];
		if (ListView_InsertItem(hWndList, &lvI) == -1){
			return NULL;
		}
		for (iSubItem = 1; iSubItem < NUM_COLUMNS; iSubItem++){
			ListView_SetItemText( hWndList,
				index,
				iSubItem,
				LPSTR_TEXTCALLBACK);
		}
	}

	return (hWndList);
}


long TreeView_GetItemParam(HWND hwndTreeView, HTREEITEM hItem)
{
    TVITEM tvItem;

    // Prepare to receive the desired information.
    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
    tvItem.hItem = hItem;
	tvItem.lParam = (long)NULL;

    // Request the information.
    TreeView_GetItem(hwndTreeView, &tvItem);

    // Return zero if it's not checked, or nonzero otherwise.
    return tvItem.lParam;
}

long TreeView_GetItemText(HWND hwndTreeView, HTREEITEM hItem, char *text )
{
    TVITEM tvItem;

    // Prepare to receive the desired information.
    tvItem.mask = TVIF_HANDLE | TVIF_TEXT;
    tvItem.hItem = hItem;
	tvItem.pszText = text;
	tvItem.cchTextMax = 32;
    // Request the information.
    TreeView_GetItem(hwndTreeView, &tvItem);

    // Return zero if it's not checked, or nonzero otherwise.
   return 1;
}

BOOL MyTreeView_GetCheckState(HWND hwndTreeView, HTREEITEM hItem)
{
    TVITEM tvItem;

    // Prepare to receive the desired information.
    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    // Request the information.
    TreeView_GetItem(hwndTreeView, &tvItem);

    // Return zero if it's not checked, or nonzero otherwise.
    return ((BOOL)(tvItem.state >> 12) -1);
}

BOOL MyTreeView_SetCheckState(HWND hwndTreeView, HTREEITEM hItem, BOOL fCheck)
{
    TVITEM tvItem;

    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    /*
    Since state images are one-based, 1 in this macro turns the check off, and 
    2 turns it on.
    */
    tvItem.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));

    return TreeView_SetItem(hwndTreeView, &tvItem);
}

BOOL TreeView_SetItemText(HWND hwndTreeView, HTREEITEM hItem, char *text )
{
    TVITEM tvItem;

    tvItem.mask = TVIF_HANDLE | TVIF_TEXT;
    tvItem.hItem = hItem;
    tvItem.pszText = text;

    return TreeView_SetItem(hwndTreeView, &tvItem);
}
// ---------------------------------------------------

 

void ShowSettingsInfoOptions( HWND hWnd, long data )
{
	HWND hDlg = hWnd;
	if ( TABLE( data ) )	CheckON( IDC_STAT_TABLE ); else CheckOFF( IDC_STAT_TABLE );
	if ( GRAPH( data ) )	CheckON( IDC_STAT_GRAPH ); else CheckOFF( IDC_STAT_GRAPH );
	if ( COMMENT(data) )	CheckON( IDC_USEDESC );	   else CheckOFF( IDC_USEDESC );
	SetPopupNum( IDC_TABSIZE, TABSIZE(data)-1 );
	SetPopupNum( IDC_SORTING, TABSORT(data) );
}


void TreeView_SetParentNodeStates( /*HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam*/ )
{
	HTREEITEM hti, htp; long parentstate = 0, parentdata = 0, parentindex = 0, mstate = 0;
	long data, index, state;
	char *pstr;

	htp = hti = TreeView_GetRoot( hWndEditSettingsView );
	index = 0;
	while ( hti ){
		state = MyTreeView_GetCheckState( hWndEditSettingsView, hti );
		index = TreeView_GetItemParam( hWndEditSettingsView, hti );
		pstr = ConfigFindSettings( EditPrefPtr, index, &data );
		if ( pstr ){
			if ( *pstr == '>' ){	// actual stats, not headers
				mstate = 0;
				parentstate = state;
				parentdata = data;
				parentindex = index;
				if ( !state )
					state = 0;
				ConfigSetSettings( EditPrefPtr, index, state );
			}
			if ( *pstr != '>' ){	// actual stats, not headers
				if ( !parentstate && !parentdata && state ){
					parentstate = parentdata = state = 1;
					MyTreeView_SetCheckState(hWndEditSettingsView, hti, 1 );
					MyTreeView_SetCheckState(hWndEditSettingsView, htp, 1 );
					ConfigSetSettings( EditPrefPtr, parentindex, 1 );
				}
				// handle auto turning of all childs depending on parent
				if ( !parentstate && parentdata ){
					state = 0;
					MyTreeView_SetCheckState(hWndEditSettingsView, hti, 0 );
				}
				if ( parentstate && !parentdata ){
					state = 1;
					MyTreeView_SetCheckState(hWndEditSettingsView, hti, 1 );
				}
				STATSET(data,state);
				ConfigSetSettings( EditPrefPtr, index, data );
				mstate += state;
			}

			if ( *pstr == '>' ){
				hti = TreeView_GetChild( hWndEditSettingsView, hti );
			} else
				hti = TreeView_GetNextSibling( hWndEditSettingsView, hti );
			if ( !hti ){
				if ( !mstate ){
					MyTreeView_SetCheckState(hWndEditSettingsView, htp, 0 );
					ConfigSetSettings( EditPrefPtr, parentindex, 0 );
				}
				htp = hti = TreeView_GetNextSibling( hWndEditSettingsView, htp );
			}
		}
	}
}


HWND InitSettingsTreeView (HWND hWndParent )                                     
{
	HWND hWndList;      // Handle to the list view window
	int index;          // Index used in for loops

	hWndList = GetDlgItem( hWndParent, IDC_STATEDIT );

	if ( hWndList ){
		long data, mstate = 0; char *p;//TVS_HASBUTTONS 
		HTREEITEM parent = NULL, child;
		TVINSERTSTRUCT item;
		HIMAGELIST hLarge;  // Handles to image lists for large icons
		HICON hIcon;        // Handle to an icon

		hLarge = ImageList_Create( 14, 14, ILC_COLOR16 , 3, 0 );
		hIcon = LoadIcon ( hInst, MAKEINTRESOURCE( IDI_STATITEM ) );	ImageList_AddIcon(hLarge, hIcon);
		hIcon = LoadIcon ( hInst, MAKEINTRESOURCE( IDI_CHECKOFF ) );	ImageList_AddIcon(hLarge, hIcon);
		hIcon = LoadIcon ( hInst, MAKEINTRESOURCE( IDI_CHECKON ) );		ImageList_AddIcon(hLarge, hIcon);
		TreeView_SetImageList(hWndList, hLarge, TVSIL_STATE );

		memset( &item, 0, sizeof( TVINSERTSTRUCT ) ); 
		item.hInsertAfter  = TVI_LAST;
		item.item.hItem  = 0;

		index = 0;
		while( p = ConfigFindSettings( EditPrefPtr, index, &data ) )
		{
			if ( ConfigInStyle( EditPrefPtr, index ) )
			{
				if ( *p == '>' ) {
					if ( index && !mstate ){
						MyTreeView_SetCheckState(hWndList, parent, 0 );
					}

					item.item.pszText = p+1;
					item.hParent = NULL;
					item.item.mask  = TVIF_STATE |TVIF_TEXT | TVIF_PARAM;
					item.item.lParam = index;
					item.item.stateMask = TVIS_STATEIMAGEMASK;
					if( data )
						item.item.state = INDEXTOSTATEIMAGEMASK(2);
					else
						item.item.state = INDEXTOSTATEIMAGEMASK(1);
					parent = TreeView_InsertItem( hWndList, &item );
					mstate = 0;
				} else {
					item.hParent = parent;
					item.item.pszText = p;
					item.item.mask  = TVIF_STATE | TVIF_TEXT | TVIF_PARAM;
					item.item.lParam = index;
					item.item.stateMask = TVIS_STATEIMAGEMASK;
					if( STAT(data) ){
						item.item.state = INDEXTOSTATEIMAGEMASK(2);
						mstate ++;
					} else
						item.item.state = INDEXTOSTATEIMAGEMASK(1);
					child = TreeView_InsertItem( hWndList, &item );
				}
			}
			index++;
		}
	}
	hWndEditSettingsView = hWndList;
	TreeView_SetParentNodeStates();
	return (hWndList);
}



LRESULT HandleStatEditProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hDlg = hWnd;

	switch( uMsg ){
		case WM_CLOSE:
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
            switch (LOWORD(wParam)) {
				case IDC_DESCOK:
					EndDialog(hWnd, TRUE);
					break;
				case IDC_DESCCANCEL:
					EndDialog(hWnd, TRUE);
					break;
			}
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
 	}
	return 0L;
}


static long gLastSettings = 0,
			gLastSettings2 = 0;

static void
TreeView_ExpandRecursive(HWND hwndTV, HTREEITEM hti, long lExpandType = TVE_EXPAND)
{
	for (; hti; hti = TreeView_GetNextSibling(hwndTV, hti))
	{
		BOOL b = TreeView_Expand(hwndTV, hti, lExpandType);

		HTREEITEM hti_clild = TreeView_GetChild(hwndTV, hti);
		if (hti_clild)
		{
			// the recursive call.		
			TreeView_ExpandRecursive(hwndTV, hti_clild, lExpandType);
		}
	}
}

void EnableStatsSettings( HWND hWnd )
{
	HWND hDlg = hWnd;
	Enable( IDC_STAT_TABLE );
	Enable( IDC_STAT_GRAPH );
	Enable( IDC_TABSIZE );
	Enable( IDC_SORTING );
	//Enable( IDC_APPLYALL );
	Enable( IDC_STATFILTER);
	Enable( IDC_STATFILTER2);
	Enable( IDC_STATFILTERTXT);
	Enable( IDC_STATEDITDESC);
	Enable( IDC_USEDESC);
}

void DisableStatsSettings( HWND hWnd )
{
	HWND hDlg = hWnd;
	Disable( IDC_STAT_TABLE );
	Disable( IDC_STAT_GRAPH );
	Disable( IDC_TABSIZE );
	Disable( IDC_SORTING );
	//Disable( IDC_APPLYALL );
	Disable( IDC_STATFILTER);
	Disable( IDC_STATFILTER2);
	Disable( IDC_STATFILTERTXT);
	Disable( IDC_STATEDITDESC);
	Disable( IDC_USEDESC);
}

char *REPORT_STAT_PROCESSING_STATS = "Processing Statistics";
//extern char *REPORT_STAT_PROCESSING_STATS;

LRESULT HandleSettingsTreeView( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pheader = (NMHDR *)lParam;
	char szText[MAX_PATH]="(null)\0";    // Place to store some text
	HWND hDlg = hWnd;
	long data, index, state;
	char *pstr;

	switch( uMsg ){
		case WM_COMMAND:
            switch (LOWORD(wParam))
			{
				case IDC_USEDESC:
					{
						pstr =	ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
						if ( IsChecked( IDC_USEDESC ) )
							STATSETC(data, 1);
						else
							STATSETC(data, 0);
						pstr =	ConfigSetSettings( EditPrefPtr, gLastSettings, data );
					}
					break;
				case IDC_STATEDITDESC:
					{
						const char* szFilename = ConfigLookupFilename(gLastSettings);
						if (!szFilename)
							break;

						std::string	strFileName;

						BuildHelpcardFilename(strFileName, MyPrefStruct.language, szFilename);
						FILE* f = fopen(strFileName.c_str(),"r");
						if (f != NULL)
						{
							(void)fclose(f);
							(void)ShellExecute( hWnd, "open", strFileName.c_str(), NULL, NULL, SW_SHOWNORMAL );
							break;
						}

						BuildHelpcardFilename(strFileName, ENGLISH_LANG, szFilename);
						f = fopen(strFileName.c_str(),"r");
						if (f != NULL)
						{
							(void)fclose(f);
							(void)ShellExecute( hWnd, "open", strFileName.c_str(), NULL, NULL, SW_SHOWNORMAL );
							break;
						}

						strFileName += " file not found.";
						::SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS, strFileName.c_str());
					}
					break;

				case IDC_STATDEFAULT:
					FillDefaults( EditPrefPtr );
					TreeView_DeleteAllItems( hWndEditSettingsView );
					hWndEditSettingsView = InitSettingsTreeView( hWnd );
					SendMessage( hWnd, WM_COMMAND, IDC_BUTTON_EXPANDALL, lParam );
					break;

				case IDC_APPLYALL:	
					pstr = ConfigFindSettings( EditPrefPtr, gLastSettings2, &data );
					if ( pstr ) {
						ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
						if ( *pstr == '>' ){
							SetGroup_Data( EditPrefPtr, gLastSettings2, data );
						} else {
							SetAll_Data( EditPrefPtr, data );
						}
					}
					//TreeView_DeleteAllItems( hWndEditSettingsView );
					//hWndEditSettingsView = InitSettingsTreeView( hWnd );
					break;

				case IDC_BUTTON_EXPANDALL:
					{
						static long	sl_LastExpand = TVE_COLLAPSE;	// So we start with Expand.
						HTREEITEM hti;

						// Toggle the Expand/Collapse.
						if (sl_LastExpand == TVE_COLLAPSE)
						{
							sl_LastExpand = TVE_EXPAND;
							SetText( IDC_BUTTON_EXPANDALL, "Collapse All" );
						}
						else
						{
							sl_LastExpand = TVE_COLLAPSE;
							SetText( IDC_BUTTON_EXPANDALL, "Expand All" );
						}

						// Expand all or collapse all.
						hti = TreeView_GetRoot(hWndEditSettingsView);
						TreeView_ExpandRecursive(hWndEditSettingsView, hti, sl_LastExpand);
						// note: TreeView_ExpandRecursive is Implemented locally - above.

					}
					break;
				
				case IDC_STAT_TABLE:
					ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
					if ( IsChecked( IDC_STAT_TABLE ) )
						STATSETT( data, 1 );
					else
						STATSETT( data, 0 );
					ConfigSetSettings( EditPrefPtr, gLastSettings, data );
					break;
				case IDC_STAT_GRAPH:
					ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
					if ( IsChecked( IDC_STAT_GRAPH ) )
						STATSETG( data, 1 );
					else
						STATSETG( data, 0 );
					ConfigSetSettings( EditPrefPtr, gLastSettings, data );
					break;

				//------------------------------------------------------------
				case IDC_TABSIZE:
					if (HIWORD(wParam) == CBN_SELENDOK ) 
					{
						int numSel = GetPopupNum( IDC_TABSIZE );
						ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
						STATSETTAB(data, (numSel+1) );
						ConfigSetSettings( EditPrefPtr, gLastSettings, data );
					}
					break;
				case IDC_SORTING:
					if (HIWORD(wParam) == CBN_SELENDOK ) 
					{
						int numSel = GetPopupNum( IDC_SORTING );
						ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
						STATSETSORT(data,numSel);
						ConfigSetSettings( EditPrefPtr, gLastSettings, data );
					}
					break;
				// CHANGE STYLES........
				case IDC_STATSTYLES:
					if (HIWORD(wParam) == CBN_SELENDOK ) 
					{
						int numSel = GetPopupNum( IDC_STATSTYLES );
						EditPrefPtr->stat_style = numSel+1;
						TreeView_DeleteAllItems( hWndEditSettingsView );
						FillDefaults( EditPrefPtr );

						hWndEditSettingsView = InitSettingsTreeView( hWnd );

						UpdateWindow( ghStatsDlg );
						UpdateWindow( hWnd );
					}
					break;

			}
			break;
		case WM_NOTIFY:
			if( LOWORD(wParam) == IDC_STATEDIT ){
				LPNMTREEVIEW pnmtv;

				switch(pheader->code){
					case NM_SETCURSOR:
						TreeView_SetParentNodeStates();
						break;

					case TVN_SELCHANGING :
						pnmtv = (LPNMTREEVIEW)lParam;
						gLastSettings2 =
						index = pnmtv->itemNew.lParam;
						pstr = ConfigFindSettings( EditPrefPtr, index, &data );
						state = MyTreeView_GetCheckState( hWndEditSettingsView, pnmtv->itemNew.hItem );

						if ( pstr )
						{
							if ( *pstr != '>' ) {		//&& STAT(data) 
								if ( !(strcmp( pstr, REPORT_STAT_PROCESSING_STATS )) ) // Special case for summary stats, which is a non report stat
									DisableStatsSettings( hDlg );
								else {

									gLastSettings = index;
									SetText( IDC_APPLYALL, ReturnString( IDS_APPLY_ALL ) );
									if ( state == 1 )
										sprintf( szText, "%s %s", pstr, ReturnString( IDS_OPTIONS_ON ) );
									else
										sprintf( szText, "%s %s", pstr, ReturnString( IDS_OPTIONS_OFF ) );
#ifdef	IDS_ICON_NAMES
									//SetText( IDC_STATEDIT_NAME, szText );
#endif
									EnableStatsSettings( hDlg );
									ShowSettingsInfoOptions( hWnd, data );
								}
							} else {
								SetText( IDC_APPLYALL, ReturnString( IDS_APPLY_GROUP ) );
								pstr = NULL;
							}
						}
/*
						if ( !pstr ) {
							DisableStatsSettings( hDlg );
							SetText( IDC_STATEDIT_NAME, "Options" );
						}
*/
						break;
				}
			}
			break;
	} 
	return 0L;
}






HWND InitExtensionsTreeView (HWND hWndParent )                                     
{
	HWND hWndList;      // Handle to the list view window
	RECT rcl, rclist;           // Rectangle for setting the size of the window
	int index;          // Index used in for loops
	TVINSERTSTRUCT item;

	// Get the size and position of the parent window
	GetClientRect( hWndParent, &rcl );
	GetObjectRect( hWndParent, GetDlgItem( hWndParent, IDC_EXTEDIT ), &rclist );

	hWndList = GetDlgItem( hWndParent, IDC_EXTEDIT );

	if ( hWndList ){
		HTREEITEM parent = NULL;

		item.hInsertAfter  = TVI_LAST;
		item.item.mask  = TVIF_TEXT|TVIF_PARAM ;
		item.hParent = NULL;
		item.item.pszText = ReturnString( IDS_EXT_PAGES );
		item.item.lParam = 1;
		parent = TreeView_InsertItem( hWndList, &item );
		index = 0;
		while( EditPrefPtr->pageStr[index][0] ){
			item.hParent = parent;
			item.item.pszText = EditPrefPtr->pageStr[index];
			TreeView_InsertItem( hWndList, &item );
			index++;
		}

		item.hParent = NULL;
		item.item.pszText = ReturnString( IDS_EXT_DOWN );
		item.item.lParam = 2;
		parent = TreeView_InsertItem( hWndList, &item );
		index = 0;
		while( EditPrefPtr->downloadStr[index][0] ){
			item.hParent = parent;
			item.item.pszText = EditPrefPtr->downloadStr[index];
			TreeView_InsertItem( hWndList, &item );
			index++;
		}
	
		item.hParent = NULL;
		item.item.pszText = ReturnString( IDS_EXT_AUDIO );
		item.item.lParam = 3;
		parent = TreeView_InsertItem( hWndList, &item );
		index = 0;
		while( EditPrefPtr->audioStr[index][0] ){
			item.hParent = parent;
			item.item.pszText = EditPrefPtr->audioStr[index];
			TreeView_InsertItem( hWndList, &item );
			index++;
		}

		item.hParent = NULL;
		item.item.pszText = ReturnString( IDS_EXT_VIDEO );
		item.item.lParam = 4;
		parent = TreeView_InsertItem( hWndList, &item );
		index = 0;
		while( EditPrefPtr->videoStr[index][0] ){
			item.hParent = parent;
			item.item.pszText = EditPrefPtr->videoStr[index];
			TreeView_InsertItem( hWndList, &item );
			index++;
		}
	
	}
	return (hWndList);
}


void GetDataExtensionsTreeView( void )
{
	char szText[MAX_PATH];    // Place to store some text
	int index;          // Index used in for loops
	HTREEITEM hparent, hcurritem;
	char *ext;
	long parents = 0;

	if ( hparent = TreeView_GetRoot( hWndExtSettingsView ) ){
		while( hparent ){
			hcurritem = TreeView_GetChild( hWndExtSettingsView, hparent );
			switch ( parents ){
				case 0:	ext = EditPrefPtr->pageStr[0]; break;
				case 1:	ext = EditPrefPtr->downloadStr[0]; break;
				case 2:	ext = EditPrefPtr->audioStr[0]; break;
				case 3:	ext = EditPrefPtr->videoStr[0]; break;
			}
			memset( ext, 0, 8*256 );
			index = 0;
			while( hcurritem ){
				TreeView_GetItemText( hWndExtSettingsView, hcurritem, szText );
				strncpy( ext, szText, 7 );
				ext[7] = 0;
				ext+=8;
				ext[0] = 0;
				hcurritem = TreeView_GetNextSibling( hWndExtSettingsView, hcurritem );
				index++;
			}
			parents++;

			hparent = TreeView_GetNextSibling( hWndExtSettingsView, hparent );
		}
	}
}

static long gLastExtension = 0;

LRESULT HandleExtensionsTreeView( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NMHDR *	pheader = (NMHDR *)lParam;
	char	szText[MAX_PATH]="(null)\0";    // Place to store some text
	HWND	hDlg = hWnd;
	long	index;
	LPNMTREEVIEW pnmtv;
	LPNMTVDISPINFO lptvdi;

	switch( uMsg ){
		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;
		case WM_COMMAND:
            switch (LOWORD(wParam)) {
				case IDC_GLOB_PAGEEDIT:
					{
						HTREEITEM hcurritem;
						hcurritem = TreeView_GetSelection( hWndExtSettingsView );
						TreeView_EditLabel( hWndExtSettingsView, hcurritem );
						UpdateWindow( hWndExtSettingsView );
					}
					break;
				case IDC_GLOB_PAGEADD:
					{
						HTREEITEM hcurritem;
						TVINSERTSTRUCT item;
						pnmtv = (LPNMTREEVIEW)lParam;

						hcurritem = TreeView_GetSelection( hWndExtSettingsView );
						item.hInsertAfter  = TVI_LAST;
						item.item.mask  = TVIF_TEXT|TVIF_PARAM ;
						item.hParent = TreeView_GetParent( hWndExtSettingsView, hcurritem );
						item.item.pszText = ".newext";
						if ( item.hParent ){
							item.item.lParam = TreeView_GetItemParam( hWndExtSettingsView, item.hParent );
							hcurritem = TreeView_InsertItem( hWndExtSettingsView, &item );
							TreeView_EditLabel( hWndExtSettingsView, hcurritem );
							UpdateWindow( hWndExtSettingsView );
						}
					}
					break;
				case IDC_GLOB_PAGEREM:
					{
						HTREEITEM hcurritem;
						pnmtv = (LPNMTREEVIEW)lParam;
						hcurritem = TreeView_GetSelection( hWndExtSettingsView );
						if ( TreeView_GetParent( hWndExtSettingsView, hcurritem ) ){
							TreeView_DeleteItem( hWndExtSettingsView, hcurritem );
							UpdateWindow( hWndExtSettingsView );
						}
					}
					break;
				case IDC_GLOB_PAGEDEFAULT:
					TreeView_DeleteAllItems( hWndExtSettingsView );
					DefaultExtensions( EditPrefPtr );
					hWndExtSettingsView = InitExtensionsTreeView( hWnd );
					break;
				case IDOK:
					if ( !HIWORD(wParam) )
						TreeView_EndEditLabelNow( hWndExtSettingsView, FALSE );
					return 1;
					break;
			}
			break;
		case WM_NOTIFY:
			if( LOWORD(wParam) == IDC_EXTEDIT ){
				char ext[64];

				switch (pheader->code) {
					case TVN_ENDLABELEDIT :
						lptvdi = (LPNMTVDISPINFO) lParam ;
						if ( lptvdi ){
							if ( lptvdi->item.pszText ){
								mystrcpy( ext, lptvdi->item.pszText );
								//SysBeep(1);
								TreeView_SetItemText( hWndExtSettingsView, lptvdi->item.hItem, lptvdi->item.pszText );
								return TRUE;
							}
						}
						//TreeView_EndEditLabelNow( hWndExtSettingsView, FALSE );
						break;
					case TVN_SELCHANGING :
						pnmtv = (LPNMTREEVIEW)lParam;
						gLastExtension =
						index = pnmtv->itemNew.lParam;
						TreeView_GetItemText( hWndExtSettingsView, pnmtv->itemNew.hItem, ext );
#ifdef	DEF_DEBUG
						sprintf( szText, "Editing Extension '%s'", ext );
						SetText( IDC_GLOB_TITLE, szText );
#endif
						break;

				}
			}
			break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0L;
}











int CheckWarning( void )
{
	if ( EditPrefPtr->stat_sessionHistory ){
		if ( !gNoGUI ){
			if ( MsgBox_yesno( IDS_WARN_MANYFILES ) == IDYES ){ 
				return FALSE;
			} else
				return TRUE;
		}
	}
	return FALSE;
}







// ------------------- TAB CONTROL



void ShowOneWindow( HWND *htmlWnds, long iPage, long iMax )
{
	if ( iMax >0 ){
		long i;
		for( i=0; i<iMax; i++ ){
			if ( iPage == i )
				ShowWindow ( htmlWnds[i], SW_SHOW );
			else
				ShowWindow ( htmlWnds[i], SW_HIDE);
		}
	}
}


void DoPanelTabs( HWND hWnd, short tab )
{
	long iMax=0;


	switch ( tab ) 
	{
		case IDC_TAB_PREPROC: { 
				HWND	htmlWnds[] = { ghPreProc1Dlg,ghPreProc2Dlg,0,0 };
				int		iPage = TabCtrl_GetCurSel( ghPreProcTabDlg );
				ShowOneWindow( htmlWnds, iPage, 2 );
			}
			break;
		case IDC_TAB_REPORT: {
				if ( EditPrefPtr->report_format/*GetPopupNum( IDC_PREFGEN_TYPE )*/ == FORMAT_PDF /* PDF */ )
				{
					HWND	htmlWnds[] = { ghReport1Dlg,ghReport2Dlg,ghReportPDFDlg,0,0,0 };
					int		iPage = TabCtrl_GetCurSel( ghReportTabDlg );
					ShowOneWindow( htmlWnds, iPage, 3 );
				}
				else
				{
					HWND	htmlWnds[] = { ghReport1Dlg,ghReport2Dlg,ghReportHTMLDlg,0,0,0,0 };
					int		iPage = TabCtrl_GetCurSel( ghReportTabDlg );
					ShowOneWindow( htmlWnds, iPage, 3 );
				}
			}
			break;
		case IDC_TAB_ANALYSIS: { 
				HWND	htmlWnds[] = { ghAnalysis1Dlg, ghAnalysis2Dlg, ghAnalysis3Dlg, ghAnalysis4Dlg,0 };
				int		iPage = TabCtrl_GetCurSel( ghAnalysisTabDlg );
				ShowOneWindow( htmlWnds, iPage, 4 );
			}
			break;
		case IDC_TAB_FILTER: { 
				HWND	htmlWnds[] = { ghFiltersInDlg, ghFiltersOutDlg,0 };
				int		iPage = TabCtrl_GetCurSel( ghFiltersTabDlg );
				ShowOneWindow( htmlWnds, iPage, 2 );
			}
			break;
		case IDC_TAB_STATS: { 
				HWND	htmlWnds[] = { ghStats1Dlg, ghStats2Dlg,  ghStats3Dlg, ghStats4Dlg, ghStats5Dlg,0 };
				int		iPage = TabCtrl_GetCurSel( ghStatsTabDlg ), iTotal = 3;
#ifdef DEF_FULLVERSION	
				iTotal = 5;
#endif
				ShowOneWindow( htmlWnds, iPage, iTotal );
			}
			break;
		case IDC_TAB_ROUTES:
			{
				HWND	htmlWnds[] = { ghRoutesKeyVisitorDlg, ghRoutesRouteToKeyPagesDlg, ghRoutesRouteFromKeyPagesDlg, 0 };
				int		iPage = TabCtrl_GetCurSel( ghRoutesTabDlg );
				ShowOneWindow( htmlWnds, iPage, 3 );
			}
			break;
		case IDC_TAB_BILLING: { 
				HWND	htmlWnds[] = { ghBillingSetupDlg,ghBillingChargesDlg,0 };
				int		iPage = TabCtrl_GetCurSel( ghBillingTabDlg );
				ShowOneWindow( htmlWnds, iPage, 2 );
			}
			break;
		case IDC_TAB_ADDS: { 
				HWND	htmlWnds[] = { ghAddsBannersDlg,ghAddsCampDlg,0 };
				int		iPage = TabCtrl_GetCurSel( ghAddsTabDlg );
				ShowOneWindow( htmlWnds, iPage, 2 );
			}
			break;
		case IDC_TAB_VDOMAINS: { 
				HWND	htmlWnds[] = { ghVDomains1Dlg, ghVDomains2Dlg, ghVDomains3Dlg, ghVDomains4Dlg,0 };
				int		iPage = TabCtrl_GetCurSel(ghVDomainsTabDlg);
				ShowOneWindow( htmlWnds, iPage, 4 );
			}
			break;
		case IDC_TAB_POSTPROC: { 
				HWND	htmlWnds[] = { ghPostProc1Dlg, ghPostProc2Dlg, ghPostProc3Dlg, 0,0 };
				int		iPage = TabCtrl_GetCurSel(ghPostProcTabDlg);
				ShowOneWindow( htmlWnds, iPage, 3 );
			}
			break;

		case IDC_TAB_CUSTOM: { 

#if DEF_WINDOWS && DEF_DEBUG
				HWND	htmlWnds[] = { ghCustom1Dlg,ghCustom2Dlg, 0,0 };
				int		iPage = TabCtrl_GetCurSel(ghCustomTabDlg), i=0;
				ShowOneWindow( htmlWnds, iPage, 2 );
#else
				HWND	htmlWnds[] = { ghCustom1Dlg, 0,0 };
				int		iPage = TabCtrl_GetCurSel(ghCustomTabDlg), i=0;
				ShowWindow ( htmlWnds[i], SW_SHOW );
#endif
			}
			break;

		case IDC_TAB_PROXY: { 
				HWND	htmlWnds[] = { ghProxyIPDlg, 0,0 };
				int		iPage = TabCtrl_GetCurSel(ghProxyTabDlg), i=0;
				ShowWindow ( htmlWnds[i], SW_SHOW );
			}
			break;

#if DEF_WINDOWS && DEF_DEBUG
		case IDC_TAB_DEBUG: { 
				HWND	htmlWnds[] = { ghDebug1Dlg, 0,0 };
				int		iPage = TabCtrl_GetCurSel(ghDebugTabDlg), i=0;
				ShowWindow ( htmlWnds[i], SW_SHOW );
			}
			break;
#endif
	
	}
}


void ShowPrefsPaneX( long paneNum )	
{
	// Hide all windows panes
	ShowWindow (ghPreProcDlg, SW_HIDE);
	ShowWindow (ghReportDlg, SW_HIDE);
	ShowWindow (ghAnalysisDlg, SW_HIDE);
	ShowWindow (ghFiltersDlg, SW_HIDE);
	ShowWindow (ghFiltersInDlg, SW_HIDE);
	ShowWindow (ghFiltersOutDlg, SW_HIDE);
	ShowWindow (ghStatsDlg, SW_HIDE);
	ShowWindow (ghRoutesDlg, SW_HIDE);
	ShowWindow (ghBillingDlg, SW_HIDE);
	ShowWindow (ghAddsDlg, SW_HIDE);
	ShowWindow (ghVDomainsDlg, SW_HIDE);
	ShowWindow (ghCustomDlg, SW_HIDE);
	ShowWindow (ghPostProcDlg, SW_HIDE);
#if DEF_APP_FIREWALL
	ShowWindow (ghProxyDlg, SW_HIDE);
	ShowWindow (ghProxyIPDlg, SW_HIDE);
#endif
#if DEF_WINDOWS && DEF_DEBUG			// Windows??? its always windows here.
	ShowWindow (ghDebugDlg, SW_HIDE);
	ShowWindow (ghRemoteDlg, SW_HIDE);
#endif

	gPaneNum = paneNum;

	// show the window pane specified (X)
	switch ( paneNum ){
		case 'prep':	ShowWindow (ghPreProcDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_PREPROC );
						break;
		case 'repo':	ShowWindow (ghReportDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_REPORT );
						break;
		case 'anal':	ShowWindow (ghAnalysisDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_ANALYSIS );
						break;
		case 'filt':	ShowWindow (ghFiltersDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_FILTER );
						break;
		case 'stat':	ShowWindow (ghStatsDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_STATS );
						break;
		case 'rout':	ShowWindow (ghRoutesDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_ROUTES );
						break;
		case 'bill':	ShowWindow (ghBillingDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_BILLING );
						break;
		case 'adds':	ShowWindow (ghAddsDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_ADDS );
						break;
		case 'post':	ShowWindow (ghPostProcDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_POSTPROC );
						break;
		case 'cust':	ShowWindow (ghCustomDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_CUSTOM );
						break;
		case 'virt':	ShowWindow (ghVDomainsDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_VDOMAINS );
						break;
		case 'remo':	ShowWindow (ghRemoteDlg, SW_RESTORE);
						break;
		case 'prox':	ShowWindow (ghProxyDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_PROXY );
						break;
		case 'DBUG':	ShowWindow (ghDebugDlg, SW_RESTORE);
						DoPanelTabs(ghPrefsDlg, IDC_TAB_DEBUG );
						break;
	}
}






void DumpHex( char *txt, void *ptr, long len )
{
	char *str;	int lp,idx=0; long *p;
	
	str = (char*)malloc( 8000 );
	if ( str ){
		p = (long*)ptr;
		idx += sprintf( str+idx, "%s ", txt );
		for ( lp=0; lp<len/4 ; lp++)
			idx += sprintf( str+idx, "$%08lx ", *(p+lp) );
		LogMessage( "application.log", str, 0);
		free( str );
	}
}








typedef struct {
	long nameId;
	long rgb[16];
} SchemeData, *SchemeDataP;

SchemeData default_schemes[] = {
	// Name,    Title,    Header,   Line Even, Line Odd, Others,  Total
	{ IDS_COLORSCHEME_RANDOM,	-1, -1, -1, -1, -1, -1 } ,
	{ IDS_COLORSCHEME_DEFAULT,	0x999999, 0xcccccc, 0xf4f4f4, 0xcccccc, 0x99cccc, 0x99cccc } ,
	{ IDS_COLORSCHEME_BASIC,	0x99AACC, 0x888888, 0xffffff, 0xdddddd, 0xd0c0b0, 0xa0b0c0 } ,
	{ IDS_COLORSCHEME_GREY,		0xa0a0a0, 0x808080, 0xffffff, 0xdddddd, 0xd0d0d0, 0xb0b0b0 } ,
	{ IDS_COLORSCHEME_BLUE,		0x3080c0, 0x90d0f0, 0xdfefff, 0xb0c0d0, 0xd0c0ff, 0x50a0f0 } ,
	{ IDS_COLORSCHEME_WILD,		0xff0030, 0xffc030, 0xffffef, 0xddddad, 0x80f030, 0xccffcc } ,
	{ IDS_COLORSCHEME_BLAND,	0xfafafa, 0xffcfff, 0xffffff, 0xffffff, 0xffcfff, 0xfafafa } ,
	{ IDS_COLORSCHEME_ICE,		0xfafafa, 0x99ccff, 0xccccff, 0x99ccff, 0x99ccff, 0x3399cc } ,
	{ 0,0,0,0,0,0,0 } };

void InitColorSchemes( void )
{
	long i = 0;
	HWND hDlg = ghReport2Dlg;

	srand( GetCurrentCTime() );

	while( default_schemes[i].nameId ){
		ListBox_AddString( IDC_SCHEMES, ReturnString(default_schemes[i].nameId ) );
		i++;
	}
}
void UseColorSchemes( long item )
{
	long i = 0, rgb = 0;


	for( i=0;i<6;i++){
		rgb = default_schemes[item].rgb[i];
		if ( rgb == -1 )
			rgb = (rand() * 255)<<16 | (rand() * 255)<<8 | (rand() * 255);
		EditPrefPtr->RGBtable[i+1] = rgb;
	}
}


/*
** Remove this hardCoded string list in preference to the string loaded from the .rc file.
** As it turns out it was ALREADY in the .rc file.  However the string userName was entered
** here as DestAddress!
**
** static char *f1[] = { "File/URL", "Client","Agent","Referral","Errors","VirtualHost","Cookie","DestAddress", "Method", "1st Referral in Session",  0 };
**
** The class CStringLoader in the namepsace QS implements a very helpful string container.
*/
const char*	GetFilter2String(int iIndex)
{
	static	QS::CStringLoader	s_strlist(IDS_FILTER2);

	if (iIndex < 0 || iIndex >= s_strlist.size())
		return NULL;
	return s_strlist[iIndex].c_str();
}


// convert the text in the gui from, "Include,Client,Contains,203.31", to "1*203.31"
long SetFilterListData( long num, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE], char *ftxt )
{
	long i;
	char *p, *src, c;

	if ( num < MAX_FILTERUNITS ){
		filter[num][0]=0;
		p = &filter[num][0];
		src = ftxt;
		// if disabled then insert - character
		if ( *src == '-' ){
			*p++ = '-';
			src++;
		}
		p++;
		if( !strcmpd( "Exclude", src ) )
			*p++ = '!';
		if( src = mystrchr( src, ',' ) ) src++;
		i = 0;
		while (GetFilter2String(i))	{
			if ( !strcmpd( GetFilter2String(i), src ) )
				break;
			i++;
		}
		filter[num][0] = i+'0';
		if( src = mystrchr( src, ',' ) ) src++;
		c = *src;
		if( src = mystrchr( src, ',' ) ) src++;
		switch( c ){
			default:
			case 'c' :
			case 'C' : sprintf( p, "*%s*", src ); break;
			case 'e' :
			case 'E' : sprintf( p, "*%s", src ); break;
			case 's' :
			case 'S' : sprintf( p, "%s*", src ); break;
			case 'i' :
			case 'I' : sprintf( p, "%s", src ); break;
		}
		//filter[num+1][0]=0;
	}
	return num;
}


long GetFilterListData( long num, long item,  char filter[MAX_FILTERUNITS][MAX_FILTERSIZE], char *out )
{
	int ret = 0;

	if ( filter ){
		int		exc, active;
		char	ftxt[MAX_FILTERSIZE], c, *p, *dest;

		exc = 0;
		active = 1;
		if( num<MAX_FILTERUNITS ){
			p = filter[num];
			dest = ftxt;
			c = *p++;
			if ( c ){
				// if disabled, show properly
				if ( c == '-' ) {
					c = *p++;
					active = 0;
				}

				c -= '0';
				if ( *p == '!' ){
					p++;
					exc = 1;
				}
				switch ( item ){
					case 0 :	if( exc )
									sprintf( out, "Exclude" );
								else
									sprintf( out, "Include" );
								ret = exc;
								break;

					case 1 :	sprintf( out, "%s", GetFilter2String(c) ); ret = c;
								break;

					case 2 :	c = strlen( p );
								if ( *p == '*' && p[c-1] == '*' )	{ sprintf( out, "Contains" );	ret = 0; }
								if ( *p != '*' && p[c-1] == '*' )	{ sprintf( out, "Starts with" );ret = 1; }
								if ( *p == '*' && p[c-1] != '*' )	{ sprintf( out, "Ends with" );	ret = 2; }
								if ( *p != '*' && p[c-1] != '*' )	{ sprintf( out, "Is equal to" );ret = 3; }
								break;

					case 3 :	if ( *p == '*' ) p++;
								sprintf( out, p );
								c = strlen( out );
								if ( out[c-1] == '*' ) out[c-1] = 0;
								ret = 0;
								break;
					case 4 :	ret = active;
				}
			}
		}
	}
	return ret;
}


// ---------- realtime listview handlers

HWND hWndFilterListView = NULL;

LRESULT FilterInListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hDlg = hWnd, hList = hWndFilterListView;
	long count, item, state;
	static int gFilterItemSel;

    switch (msg) {
		case WM_INITDIALOG: 
			hList = hWndFilterListView = InitGenericListView( hDlg, IDC_FILTERIN_LIST, IDS_FILTERS, EditPrefPtr->filterdata.filterInTot, 0 );
			item = LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;
			ListView_SetExtendedListViewStyleEx( hWndFilterListView, item , item );
			for( item = 0; item< EditPrefPtr->filterdata.filterInTot; item ++ ){
				state = GetFilterListData( item, 4, EditPrefPtr->filterdata.filterIn, NULL );
				ListView_SetCheckState( hList, item, state );
			}
			break;
		case WM_DESTROY:
			DestroyWindow( hWndFilterListView );
			break;

		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				NMHDR *pheader = (NMHDR *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[MAX_PATH]="(null)\0";    // Place to store some text

				if ( LOWORD(wParam) == IDC_FILTERIN_LIST ){
					switch(pLvdi->hdr.code)
					{
						case LVN_KEYDOWN:
							{
								LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
								if ( pnkd->wVKey == VK_DELETE )
								{
									SendMessage( hWnd, WM_COMMAND, IDC_FILTERIN_REM, lParam );
								}
							}
							break;
						// draw the items
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;

							GetFilterListData( itemSel, pLvdi->item.iSubItem, EditPrefPtr->filterdata.filterIn, szText );
							break;

						// different ROW has bit CLICKED on
						case LVN_ITEMCHANGING:
							gFilterItemSel = itemSel;
							if ( itemSel >= 0 ){
								long i;
								//state = GetFilterListData( itemSel, 4, EditPrefPtr->filterdata.filterIn, NULL );
								//ListView_SetCheckState( hList, itemSel, state );

								i = GetFilterListData( itemSel, 0, EditPrefPtr->filterdata.filterIn, szText );
								SetPopupNum( IDC_FILTERIN1, i );

								i = GetFilterListData( itemSel, 1, EditPrefPtr->filterdata.filterIn, szText );
								SetPopupNum( IDC_FILTERIN2, i );

								i = GetFilterListData( itemSel, 2, EditPrefPtr->filterdata.filterIn, szText );
								SetPopupNum( IDC_FILTERIN3, i );

								i = GetFilterListData( itemSel, 3, EditPrefPtr->filterdata.filterIn, szText );
								SetText( IDC_FILTERIN_DATA, szText );
							} else
								SetPopupNum( IDC_FILTERIN1, 1 );
							break;
					} // switch
				}
			}
			break;

		case WM_SETCURSOR:
			{
				UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
				char szText[MAXFILENAMESSIZE];
					for(item=0;item<EditPrefPtr->filterdata.filterInTot; item++ )
					{
						int itemState = ListView_GetCheckState( hList, item );

						if ( EditPrefPtr->filterdata.filterIn[item][0] == '-' && itemState )
						{
							mystrcpy( szText, EditPrefPtr->filterdata.filterIn[item] );
							mystrcpy( EditPrefPtr->filterdata.filterIn[item], szText+1 );
						}
						if ( EditPrefPtr->filterdata.filterIn[item][0] != '-' && !itemState )
						{
							mystrcpy( szText, "-" );
							strcat( szText, EditPrefPtr->filterdata.filterIn[item] );
							mystrcpy( EditPrefPtr->filterdata.filterIn[item], szText );
						}

					}
					break;
			}
			break;

		case WM_COMMAND:
            switch (LOWORD(wParam)) {
				case IDC_FILTERIN2:
					{
						if (HIWORD(wParam) != CBN_SELCHANGE)
							break;

						int		iSelection = (int)::SendMessage(::GetDlgItem(hWnd,IDC_FILTERIN2), CB_GETCURSEL, 0, 0L);

						if (iSelection == 9)		// 1st referals in report.
						{
							// Force the other filters to be 'Include' and 'Is equal to'.
							::SendDlgItemMessage(hWnd,IDC_FILTERIN1,CB_SETCURSEL, 0, 0L);
							::SendDlgItemMessage(hWnd,IDC_FILTERIN3,CB_SETCURSEL, 3, 0L);

							// Disable the other control.
							::EnableWindow(::GetDlgItem(hWnd,IDC_FILTERIN1), FALSE);
							::EnableWindow(::GetDlgItem(hWnd,IDC_FILTERIN3), FALSE);
						}
						else
						{
							// Disable the other control.
							::EnableWindow(::GetDlgItem(hWnd,IDC_FILTERIN1), TRUE);
							::EnableWindow(::GetDlgItem(hWnd,IDC_FILTERIN3), TRUE);
						}
					}
					break;
				case IDC_FILTERIN_ADD:
					{
						if (EditPrefPtr->filterdata.filterInTot >= MAX_FILTERUNITS)
						{
							if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
							{
								SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
							}
							break;
						}

					}
				case IDC_FILTERIN_CHANGE:
					{
						long var[3]; char str[356], name[200];

						GetText( IDC_FILTERIN_DATA, str, 200 );
						if ( str[0] ) {
							var[0] = GetPopupNum( IDC_FILTERIN1 );
							var[1] = GetPopupNum( IDC_FILTERIN2 );
							var[2] = GetPopupNum( IDC_FILTERIN3 );

							GetPopupText( IDC_FILTERIN1, var[0], (LPCTSTR)name );
							sprintf( str, "%s,", name );
							GetPopupText( IDC_FILTERIN2, var[1], (LPCTSTR)name );
							strcat( str, name ); strcat( str, LIST_VIEW_SEP );
							GetPopupText( IDC_FILTERIN3, var[2], (LPCTSTR)name );
							strcat( str, name ); strcat( str, LIST_VIEW_SEP );
							GetText( IDC_FILTERIN_DATA, name, 200 );
							strcat( str, name );

							if ( wParam == IDC_FILTERIN_ADD ) {
								SetFilterListData( EditPrefPtr->filterdata.filterInTot, EditPrefPtr->filterdata.filterIn, str );
								AddItemToListView( hList, EditPrefPtr->filterdata.filterInTot, 4, 0, LIST_VIEW_SEP );
								SelectListViewItem( hList, EditPrefPtr->filterdata.filterInTot );
								ListView_SetCheckState( hList, EditPrefPtr->filterdata.filterInTot, 1 );
								EditPrefPtr->filterdata.filterInTot++;
							} else
							if ( wParam == IDC_FILTERIN_CHANGE ) {
								SetFilterListData( gFilterItemSel, EditPrefPtr->filterdata.filterIn, str );
								ListView_Update( hList , gFilterItemSel-1 );
							}
						}
					}
					break;
				case IDC_FILTERIN_REM:
					count = ListView_GetSelectedCount( hList );
					if ( count ){			// copy log files to global list
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->filterdata.filterInTot ){
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->filterdata.filterIn[item], EditPrefPtr->filterdata.filterIn[item+1], (EditPrefPtr->filterdata.filterInTot-item) * MAX_FILTERSIZE );
							EditPrefPtr->filterdata.filterInTot--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
			}
		break;
	}
	return 0;

}


// ****************************************************************************
// Method:		RouteToKeyPagesHandler
//
// Abstract:	This method is used to hander the windows messages for the
//				RouteToKeyPages dialog window.  It is a copied version of
//				KeyVisitorListHandler with the variables changed to KeyPages.
//
// Declaration: LRESULT RouteToKeyPagesHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
//
// Returns:		LRESULT
//
// ****************************************************************************
HWND hWndRouteToKeyPagesListView = NULL;

LRESULT RouteToKeyPagesHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char	sz[10];
	long count,item;
	HWND hDlg = hWnd, hList = hWndRouteToKeyPagesListView;
static int gRouteToKeyPagesItemSel;

    switch (msg) {
		case WM_INITDIALOG: 
			(void)sprintf(sz,"%d", EditPrefPtr->nToKeyPageMaxRows);
			SetText( IDC_ROUTE2KEYPAGE_NUMBERTOREPORT, sz );

			(void)sprintf(sz,"%d", EditPrefPtr->nToKeyPageRouteDepth);
			SetText( IDC_ROUTE2KEYPAGE_TRACEBACK, sz );


			// Needs fixing.
			hList = hWndRouteToKeyPagesListView = InitGenericListView( hDlg, IDC_LIST_KEYPAGES, IDS_ROUTETOKEYPAGE, EditPrefPtr->nToKeyPages, 0 );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hWndRouteToKeyPagesListView );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[KEYPAGE_LENGTH+1]="(null)\0";    // Place to store some text

				{
					switch(pLvdi->hdr.code)	{
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							if ( (itemSel >= 0) && ( itemSel < EditPrefPtr->nToKeyPages ))
							{
								mystrcpy( szText, EditPrefPtr->szToKeyPages[itemSel] );
							}
							break;
						case LVN_ITEMCHANGING:
							gRouteToKeyPagesItemSel = itemSel;
							if ( ( itemSel >= 0 ) && (itemSel < NUMBER_OF_KEYPAGES) )
							{
								SetText( IDC_ROUTE2KEYPAGE_KEYPAGE, EditPrefPtr->szToKeyPages[itemSel] );
							}
							break;
					}
				}
			}
			break;
		case WM_COMMAND:
            switch (LOWORD(wParam)) {
				case IDC_ROUTE2KEYPAGE_TRACEBACK:
					{
						char txt[10];
						if (HIWORD(wParam) == EN_UPDATE)
						{
							GetText( IDC_ROUTE2KEYPAGE_TRACEBACK, txt ,9 );
							EditPrefPtr->nToKeyPageRouteDepth = atoi(txt);
						} 
						else if (HIWORD(wParam) == EN_KILLFOCUS)
						{
							if (EditPrefPtr->nToKeyPageRouteDepth < 1)
							{
								EditPrefPtr->nToKeyPageRouteDepth = 1;
								(void)sprintf(txt, "%d", EditPrefPtr->nToKeyPageRouteDepth);
								SetText( IDC_ROUTE2KEYPAGE_TRACEBACK, txt );
							}
							if (EditPrefPtr->nToKeyPageRouteDepth > MAX_KEYPAGE_DEPTH)
							{
								EditPrefPtr->nToKeyPageRouteDepth = MAX_KEYPAGE_DEPTH;
								(void)sprintf(txt, "%d", EditPrefPtr->nToKeyPageRouteDepth);
								SetText( IDC_ROUTE2KEYPAGE_TRACEBACK, txt );
							}
						}
					}
					break;
				case IDC_ROUTE2KEYPAGE_NUMBERTOREPORT:
					{
						char txt[10];
						if (HIWORD(wParam) == EN_UPDATE)
						{
							GetText( IDC_ROUTE2KEYPAGE_NUMBERTOREPORT, txt ,9 );
							EditPrefPtr->nToKeyPageMaxRows = atoi(txt);
						}
						else if (HIWORD(wParam) == EN_KILLFOCUS)
						{
							if (EditPrefPtr->nToKeyPageMaxRows < 1)
							{
								EditPrefPtr->nToKeyPageMaxRows = 1;
								(void)sprintf(txt, "%d", EditPrefPtr->nToKeyPageMaxRows);
								SetText( IDC_ROUTE2KEYPAGE_NUMBERTOREPORT, txt );
							}
							if (EditPrefPtr->nToKeyPageMaxRows > MAX_KEYPAGE_ROWS)
							{
								EditPrefPtr->nToKeyPageMaxRows = MAX_KEYPAGE_ROWS;
								(void)sprintf(txt, "%d", EditPrefPtr->nToKeyPageMaxRows);
								SetText( IDC_ROUTE2KEYPAGE_NUMBERTOREPORT, txt );
							}
						}
					}
					break;
				case IDC_ROUTE2KEYPAGE_CHANGE:
					{
						char txt[KEYPAGE_LENGTH];

						GetText( IDC_ROUTE2KEYPAGE_KEYPAGE, txt ,KEYPAGE_LENGTH );
						if( txt[0] && (gRouteToKeyPagesItemSel < EditPrefPtr->nToKeyPages) )
						{
							strcpy( EditPrefPtr->szToKeyPages[gRouteToKeyPagesItemSel], txt );
							ListView_Update( hList , gRouteToKeyPagesItemSel-1 );
						}
					}
					break;
				case IDC_ROUTE2KEYPAGE_ADD:
					{
						char txt[MAX_FILTERSIZE];

						GetText( IDC_ROUTE2KEYPAGE_KEYPAGE, txt ,KEYPAGE_LENGTH );
						if ( ( txt[0] ) && (EditPrefPtr->nToKeyPages < NUMBER_OF_KEYPAGES) )
						{
							strcpy( EditPrefPtr->szToKeyPages[EditPrefPtr->nToKeyPages], txt );
							AddItemToListView( hList, EditPrefPtr->nToKeyPages, 1, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->nToKeyPages );
							EditPrefPtr->nToKeyPages++;
						}
					}
					break;
				case IDC_ROUTE2KEYPAGE_REM:
					count = ListView_GetSelectedCount( hList );
					if ( count )		
					{
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->nToKeyPages )
						{
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->szToKeyPages[item], EditPrefPtr->szToKeyPages[item+1], (EditPrefPtr->nToKeyPages-item) * (KEYPAGE_LENGTH+1) );
							EditPrefPtr->nToKeyPages--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
				case IDC_ROUTE2KEYPAGE_CLEAR:
					{
						item = ListView_GetNextItem( hList, -1 , LVNI_ALL );
						while( item != -1 && EditPrefPtr->nToKeyPages )
						{
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->szToKeyPages[item], EditPrefPtr->szToKeyPages[item+1], (EditPrefPtr->nToKeyPages-item) * (KEYPAGE_LENGTH+1) );
							EditPrefPtr->nToKeyPages--;
							item = ListView_GetNextItem( hList, -1 , LVNI_ALL );
						}

						SetText( IDC_ROUTE2KEYPAGE_KEYPAGE, "" );
					}
					break;
			}
		break;
	}
	return 0;

}



void AddListItem( HWND hWnd, int iRow, long iColumn, const char* szText)
{
	LV_ITEM lvI; 

	lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	lvI.state = 0;      //
	lvI.stateMask = 0;  //LVIS_SELECTED 

	lvI.iItem = iRow;
	lvI.iSubItem = 0;
	lvI.pszText = (char*)szText;
	
	lvI.cchTextMax = 256;
	lvI.iImage = 0;

	if (iColumn == 0)	// 0 based.
	{
		ListView_InsertItem(hWnd, &lvI);
	}
	else
	{
		ListView_SetItemText( hWnd,	iRow, iColumn, lvI.pszText );
	}
}


HWND	g_hwndBillingChargesList = NULL;

void
SetBillingCustomerDetails(BillingCustomerStruct* pCustomer)
{
	DEF_ASSERT(pCustomer);
	if (!pCustomer)
		return;

	SetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLING_CUSTOMERNAME,	pCustomer->szCustomer);
	SetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMERID,		pCustomer->szCustomerID);
	SetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMERURL,	pCustomer->szURL);
	
	SetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMERFIXEDCHARGE,				pCustomer->szFixedCharge);
	SetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMERFIXEDNUMBER,				pCustomer->szFixedPeriod);
	SendMessage( GetDlgItem( ghBillingChargesDlg, IDC_COMBO_BILLINGCUSTOMERFIXEDTYPE), CB_SETCURSEL,	pCustomer->ucFixedGroup, 0 );

	SendMessage( GetDlgItem( ghBillingChargesDlg, IDC_CHECK_BILLINGCUSTOMEREXCESSCHARGE), BM_SETCHECK,	pCustomer->bExcessCharge?BST_CHECKED:BST_UNCHECKED, 0);
	SetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMEREXCESSCHARGE,				pCustomer->szExcessCharge);
	SetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMEREXCESSNUMBER,				pCustomer->szExcessPeriod);
	SendMessage( GetDlgItem( ghBillingChargesDlg, IDC_COMBO_BILLINGCUSTOMEREXCESSTYPE), CB_SETCURSEL,	pCustomer->ucExcessGroup, 0 );
}

void
GetBillingCustomerDetails(BillingCustomerStruct* pCustomer)
{
	DEF_ASSERT(pCustomer);
	if (!pCustomer)
		return;

	GetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLING_CUSTOMERNAME,	pCustomer->szCustomer, BILLINGCUSTOMER_NAME_WIDTH);
	GetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMERID,		pCustomer->szCustomerID, BILLINGCUSTOMER_ID_WIDTH);
	GetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMERURL,	pCustomer->szURL, BILLINGCUSTOMER_URL_WIDTH);
	
	GetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMERFIXEDCHARGE, pCustomer->szFixedCharge, BILLINGCUSTOMER_CHARGE_WIDTH);
	GetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMERFIXEDNUMBER, pCustomer->szFixedPeriod, BILLINGCUSTOMER_PERIOD_WIDTH);
	pCustomer->ucFixedGroup = (int)SendMessage( GetDlgItem( ghBillingChargesDlg, IDC_COMBO_BILLINGCUSTOMERFIXEDTYPE), CB_GETCURSEL,	0, 0 );

	pCustomer->bExcessCharge = (int)SendMessage( GetDlgItem( ghBillingChargesDlg, IDC_CHECK_BILLINGCUSTOMEREXCESSCHARGE), BM_GETCHECK,	0, 0);
	GetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMEREXCESSCHARGE, pCustomer->szExcessCharge, BILLINGCUSTOMER_CHARGE_WIDTH);
	GetDlgItemText(ghBillingChargesDlg, IDC_EDIT_BILLINGCUSTOMEREXCESSNUMBER, pCustomer->szExcessPeriod, BILLINGCUSTOMER_PERIOD_WIDTH);
	pCustomer->ucExcessGroup = (int)SendMessage( GetDlgItem( ghBillingChargesDlg, IDC_COMBO_BILLINGCUSTOMEREXCESSTYPE), CB_GETCURSEL,	0, 0 );
}


BillingCustomerStruct*	g_pCustomerCurrent = NULL;

LRESULT BillingChargesHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG: 
			{
				EditPrefPtr->billingCharges;	// All the Billing Charges stuff.

				g_hwndBillingChargesList = InitGenericListView( hWnd, IDC_LIST_BILLINGCUSTOMER, IDS_BILLINGCHARGESLIST, 0, 0 );

				BillingCustomerStruct*	pCustomer = EditPrefPtr->billingCharges.pFirstCustomer;
				for (int i=0;i<EditPrefPtr->billingCharges.nCustomers && pCustomer!=NULL; ++i)
				{
					AddListItem( g_hwndBillingChargesList, i, 0, pCustomer->szCustomerID);
					AddListItem( g_hwndBillingChargesList, i, 1, pCustomer->szCustomer);
					AddListItem( g_hwndBillingChargesList, i, 2, pCustomer->szURL);

					pCustomer = pCustomer->pNext;
				}


				g_pCustomerCurrent = EditPrefPtr->billingCharges.pFirstCustomer;

				// ******************************************************
				// Sometimes you can not do things in INIT_DIALOG.
				// So lets Queue it to occur directly AFTERwards.
				// ******************************************************
				if (g_pCustomerCurrent)
				{
					ListView_SetItemState(g_hwndBillingChargesList, 0, LVIS_SELECTED|LVIS_FOCUSED , LVIS_SELECTED|LVIS_FOCUSED);
					::PostMessage(hWnd, WM_USER+1, 0, 0);
				}

			}
			break;
		case WM_DESTROY:
			DestroyWindow( g_hwndBillingChargesList );
			break;

		// ******************************************************
		// This is just the typical POST_INIT_DIALOG message
		// that is so often needed.
		// ******************************************************
		case WM_USER+1:
			SetBillingCustomerDetails(g_pCustomerCurrent);
			break;

		// HelpCard stuff.
		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		// Changes to the listctrl (ie: The selection!)
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;

				if (pLvdi->hdr.code == LVN_ITEMCHANGING && pLvdi->item.state == 0) 
				{
					int	itemSel = pNm->iItem;

					// Linear search, but hey - even with 200 customers its fine.
					BillingCustomerStruct*	pCustomer = EditPrefPtr->billingCharges.pFirstCustomer;
					for (int i=0; i<itemSel && pCustomer!=NULL; ++i)
					{
						pCustomer = pCustomer->pNext;
					}

					if (pCustomer)
						SetBillingCustomerDetails(pCustomer);

					g_pCustomerCurrent = pCustomer;
				}
			}
			break;

		// All the GUI actions to populate the settings.
		case WM_COMMAND:
            switch (LOWORD(wParam)) 
			{

			case	IDC_BUTTON_BILLINGCUSTOMERADD:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					BillingCustomerStruct*	pCustomer = new BillingCustomerStruct;
					memset(pCustomer, 0, sizeof(BillingCustomerStruct));

					// Gather all data and add to list.
					GetBillingCustomerDetails(pCustomer);

					// Add to the top of the list.
					pCustomer->pNext = EditPrefPtr->billingCharges.pFirstCustomer;
					EditPrefPtr->billingCharges.pFirstCustomer = pCustomer;
					EditPrefPtr->billingCharges.nCustomers++;

					AddListItem( g_hwndBillingChargesList, 0, 0, pCustomer->szCustomerID);
					AddListItem( g_hwndBillingChargesList, 0, 1, pCustomer->szCustomer);
					AddListItem( g_hwndBillingChargesList, 0, 2, pCustomer->szURL);
				}
				break;

			case	IDC_BUTTON_BILLINGCUSTOMERDELETE:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					int iCount = ListView_GetSelectedCount( g_hwndBillingChargesList );
					if ( iCount )		
					{
						long lItem = -1;
						while( (lItem=ListView_GetNextItem( g_hwndBillingChargesList, -1 , LVNI_SELECTED )) != -1 )
						{
							ListView_DeleteItem( g_hwndBillingChargesList, lItem );

							BillingCustomerStruct*	pCustomer	= EditPrefPtr->billingCharges.pFirstCustomer;
							if (!pCustomer)
								break;

							BillingCustomerStruct*	pPrev = NULL;
							if (lItem == 0)
							{
								EditPrefPtr->billingCharges.pFirstCustomer = pCustomer->pNext;
							}
							else
							{
								while (lItem--)
								{
									pPrev = pCustomer;
									pCustomer=pCustomer->pNext;
								}
								pPrev->pNext = pCustomer->pNext;
							}

							if (g_pCustomerCurrent == pCustomer)
								g_pCustomerCurrent = pPrev;

							delete pCustomer;
							EditPrefPtr->billingCharges.nCustomers--;
						}
					}
					break;
				}
				break;

			case	IDC_BUTTON_BILLINGCUSTOMERCHANGE:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					long lItem=ListView_GetNextItem( g_hwndBillingChargesList, -1 , LVNI_SELECTED);

					if (lItem == -1)
						break;

					if (!g_pCustomerCurrent)
						g_pCustomerCurrent = EditPrefPtr->billingCharges.pFirstCustomer;

					if (g_pCustomerCurrent)
					{
						GetBillingCustomerDetails(g_pCustomerCurrent);
						
						ListView_SetItemText( g_hwndBillingChargesList, lItem, 0, g_pCustomerCurrent->szCustomerID);
						ListView_SetItemText( g_hwndBillingChargesList, lItem, 1, g_pCustomerCurrent->szCustomer);
						ListView_SetItemText( g_hwndBillingChargesList, lItem, 2, g_pCustomerCurrent->szURL);
					}
				}
				break;

			}
			break;
	}

	return 0;
}

void	SetBillingSetupDetails(void)
{
	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_CHECK_ENABLEBILLING), BM_SETCHECK,	EditPrefPtr->billingSetup.bEnabled?BST_CHECKED:BST_UNCHECKED, 0);
	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_COMBO_BILLINGPERIOD), CB_SETCURSEL,	EditPrefPtr->billingSetup.ucBillingPeriod, 0 );
	SetDlgItemText(ghBillingSetupDlg, IDC_EDIT_BILLINGSTART,							EditPrefPtr->billingSetup.szStartDate);
	SetDlgItemText(ghBillingSetupDlg, IDC_EDIT_BILLINGEND,								EditPrefPtr->billingSetup.szEndDate);
	SetDlgItemText(ghBillingSetupDlg, IDC_EDIT_BILLINGREPORTTITLE,						EditPrefPtr->billingSetup.szTitle);
	SetDlgItemText(ghBillingSetupDlg, IDC_EDIT_BILLINGCURRENCY,							EditPrefPtr->billingSetup.szCurrencySymbol);

	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_COMBO_BILLINGFORMAT), CB_SETCURSEL,	EditPrefPtr->billingSetup.ucOutputFormat, 0 );

	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_CHECK_BILLINGREPORT_NAME),			BM_SETCHECK, EditPrefPtr->billingSetup.bHeaderCustomerName?BST_CHECKED:BST_UNCHECKED,	0);
	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_CHECK_BILLINGREPORT_ID),			BM_SETCHECK, EditPrefPtr->billingSetup.bHeaderCustomerID?BST_CHECKED:BST_UNCHECKED,		0);
	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_CHECK_BILLINGREPORT_MBTRANSFERED),	BM_SETCHECK, EditPrefPtr->billingSetup.bHeaderMBTransfered?BST_CHECKED:BST_UNCHECKED,	0);
	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_CHECK_BILLINGREPORT_MBALLOW),		BM_SETCHECK, EditPrefPtr->billingSetup.bHeaderMBAllowance?BST_CHECKED:BST_UNCHECKED,	0);
	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_CHECK_BILLINGREPORT_MAXCONN),		BM_SETCHECK, EditPrefPtr->billingSetup.bHeaderMaxConnections?BST_CHECKED:BST_UNCHECKED,	0);
	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_CHECK_BILLINGREPORT_EXCESSCHARGES),	BM_SETCHECK, EditPrefPtr->billingSetup.bHeaderExcessCharges?BST_CHECKED:BST_UNCHECKED,	0);
	SendMessage( GetDlgItem( ghBillingSetupDlg, IDC_CHECK_BILLINGREPORT_TOTALCHARGES),	BM_SETCHECK, EditPrefPtr->billingSetup.bHeaderTotalCharges?BST_CHECKED:BST_UNCHECKED,	0);
}

LRESULT BillingSetupHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG: 
			{
				EditPrefPtr->billingSetup;	// Contains all the BillingSetup data.

				// Usual PostInitDialog initizations.
				::PostMessage(hWnd, WM_USER+1, 0, 0);
			}
			break;

		case WM_USER+1:
			SetBillingSetupDetails();
			break;

		// HelpCard stuff.
		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		// All the GUI actions to populate the settings.
		case WM_COMMAND:
            switch (LOWORD(wParam)) {
				case IDC_CHECK_ENABLEBILLING:
					EditPrefPtr->billingSetup.bEnabled = (int)SendMessage( GetDlgItem( hWnd, IDC_CHECK_ENABLEBILLING), BM_GETCHECK,	0, 0);
					break;

				case IDC_COMBO_BILLINGPERIOD:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int		iSelection = (int)::SendMessage(::GetDlgItem(hWnd,IDC_COMBO_BILLINGPERIOD), CB_GETCURSEL, 0, 0L);
						EditPrefPtr->billingSetup.ucBillingPeriod = iSelection;
					}
					break;

				case IDC_EDIT_BILLINGSTART:
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						GetDlgItemText( hWnd, IDC_EDIT_BILLINGSTART, EditPrefPtr->billingSetup.szStartDate ,DATE_WIDTH+1 );
						// Validate Date.
					}
					break;

				case IDC_EDIT_BILLINGEND:
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						GetDlgItemText( hWnd, IDC_EDIT_BILLINGEND, EditPrefPtr->billingSetup.szEndDate ,DATE_WIDTH+1 );
						// Validate Date.
					}
					break;

				case IDC_EDIT_BILLINGREPORTTITLE:
					if (HIWORD(wParam) == EN_KILLFOCUS)
						GetDlgItemText( hWnd, IDC_EDIT_BILLINGREPORTTITLE, EditPrefPtr->billingSetup.szTitle ,BILLINGTITLE_WIDTH );
					break;

				case IDC_EDIT_BILLINGCURRENCY:
					if (HIWORD(wParam) == EN_KILLFOCUS)
						GetDlgItemText( hWnd, IDC_EDIT_BILLINGCURRENCY, EditPrefPtr->billingSetup.szCurrencySymbol ,CURRENCY_WIDTH );
					break;

				case IDC_COMBO_BILLINGFORMAT:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int		iSelection = (int)::SendMessage(::GetDlgItem(hWnd,IDC_COMBO_BILLINGFORMAT), CB_GETCURSEL, 0, 0L);
						EditPrefPtr->billingSetup.ucOutputFormat = iSelection;
					}
					break;

				case IDC_CHECK_BILLINGREPORT_NAME:
					EditPrefPtr->billingSetup.bHeaderCustomerName = (int)SendMessage( GetDlgItem( hWnd, IDC_CHECK_BILLINGREPORT_NAME), BM_GETCHECK, 0, 0);
					break;

				case IDC_CHECK_BILLINGREPORT_ID:
					EditPrefPtr->billingSetup.bHeaderCustomerID = (int)SendMessage( GetDlgItem( hWnd, IDC_CHECK_BILLINGREPORT_ID), BM_GETCHECK, 0, 0);
					break;

				case IDC_CHECK_BILLINGREPORT_MBTRANSFERED:
					EditPrefPtr->billingSetup.bHeaderMBTransfered = (int)SendMessage( GetDlgItem( hWnd, IDC_CHECK_BILLINGREPORT_MBTRANSFERED), BM_GETCHECK, 0, 0);
					break;
				
				case IDC_CHECK_BILLINGREPORT_MBALLOW:
					EditPrefPtr->billingSetup.bHeaderMBAllowance = (int)SendMessage( GetDlgItem( hWnd, IDC_CHECK_BILLINGREPORT_MBALLOW), BM_GETCHECK, 0, 0);
					break;
				
				case IDC_CHECK_BILLINGREPORT_MAXCONN:
					EditPrefPtr->billingSetup.bHeaderMaxConnections = (int)SendMessage( GetDlgItem( hWnd, IDC_CHECK_BILLINGREPORT_MAXCONN), BM_GETCHECK, 0, 0);
					break;
				
				case IDC_CHECK_BILLINGREPORT_EXCESSCHARGES:
					EditPrefPtr->billingSetup.bHeaderExcessCharges = (int)SendMessage( GetDlgItem( hWnd, IDC_CHECK_BILLINGREPORT_EXCESSCHARGES), BM_GETCHECK, 0, 0);
					break;
				
				case IDC_CHECK_BILLINGREPORT_TOTALCHARGES:
					EditPrefPtr->billingSetup.bHeaderTotalCharges = (int)SendMessage( GetDlgItem( hWnd, IDC_CHECK_BILLINGREPORT_TOTALCHARGES), BM_GETCHECK, 0, 0);
					break;
			}
			break;


	}

	return 0;
}

// ****************************************************************************
// Method:		RouteFromKeyPagesHandler
//
// Abstract:	This method is used to hander the windows messages for the
//				RouteFromKeyPages dialog window.  It is a copied version of
//				KeyVisitorListHandler with the variables changed to KeyPages.
//
// Declaration: LRESULT RouteFromKeyPagesHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
//
// Returns:		LRESULT
//
// ****************************************************************************
HWND hWndRouteFromKeyPagesListView = NULL;

LRESULT RouteFromKeyPagesHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char	sz[10];
	long count,item;
	HWND hDlg = hWnd, hList = hWndRouteFromKeyPagesListView;
static int gRouteFromKeyPagesItemSel;

    switch (msg) {
		case WM_INITDIALOG: 
			(void)sprintf(sz,"%d", EditPrefPtr->nFromKeyPageMaxRows);
			SetText( IDC_ROUTEFROMKEYPAGE_NUMBERTOREPORT, sz );

			(void)sprintf(sz,"%d", EditPrefPtr->nFromKeyPageRouteDepth);
			SetText( IDC_ROUTEFROMKEYPAGE_TRACEBACK, sz );


			// Needs fixing.
			hList = hWndRouteFromKeyPagesListView = InitGenericListView( hDlg, IDC_ROUTEFROMKEYPAGE_LIST, IDS_ROUTEFROMKEYPAGE, EditPrefPtr->nFromKeyPages, 0 );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hWndRouteFromKeyPagesListView );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[KEYPAGE_LENGTH+1]="(null)\0";    // Place to store some text

				{
					switch(pLvdi->hdr.code)	{
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							if ( (itemSel >= 0) && ( itemSel < EditPrefPtr->nFromKeyPages ))
							{
								mystrcpy( szText, EditPrefPtr->szFromKeyPages[itemSel] );
							}
							break;
						case LVN_ITEMCHANGING:
							gRouteFromKeyPagesItemSel = itemSel;
							if ( ( itemSel >= 0 ) && (itemSel < NUMBER_OF_KEYPAGES) )
							{
								SetText( IDC_ROUTEFROMKEYPAGE_KEYPAGE, EditPrefPtr->szFromKeyPages[itemSel] );
							}
							break;
					}
				}
			}
			break;
		case WM_COMMAND:
            switch (LOWORD(wParam)) {
				case IDC_ROUTEFROMKEYPAGE_TRACEBACK:
					{
						char txt[10];
						if (HIWORD(wParam) == EN_UPDATE)
						{
							GetText( IDC_ROUTEFROMKEYPAGE_TRACEBACK, txt ,9 );
							EditPrefPtr->nFromKeyPageRouteDepth = atoi(txt);
						} 
						else if (HIWORD(wParam) == EN_KILLFOCUS)
						{
							if (EditPrefPtr->nFromKeyPageRouteDepth < 1)
							{
								EditPrefPtr->nFromKeyPageRouteDepth = 1;
								(void)sprintf(txt, "%d", EditPrefPtr->nFromKeyPageRouteDepth);
								SetText( IDC_ROUTEFROMKEYPAGE_TRACEBACK, txt );
							}
							if (EditPrefPtr->nFromKeyPageRouteDepth > MAX_KEYPAGE_DEPTH)
							{
								EditPrefPtr->nFromKeyPageRouteDepth = MAX_KEYPAGE_DEPTH;
								(void)sprintf(txt, "%d", EditPrefPtr->nFromKeyPageRouteDepth);
								SetText( IDC_ROUTEFROMKEYPAGE_TRACEBACK, txt );
							}
						}
					}
					break;
				case IDC_ROUTEFROMKEYPAGE_NUMBERTOREPORT:
					{
						char txt[10];
						if (HIWORD(wParam) == EN_UPDATE)
						{
							GetText( IDC_ROUTEFROMKEYPAGE_NUMBERTOREPORT, txt ,9 );
							EditPrefPtr->nFromKeyPageMaxRows = atoi(txt);
						}
						else if (HIWORD(wParam) == EN_KILLFOCUS)
						{
							if (EditPrefPtr->nFromKeyPageMaxRows < 1)
							{
								EditPrefPtr->nFromKeyPageMaxRows = 1;
								(void)sprintf(txt, "%d", EditPrefPtr->nFromKeyPageMaxRows);
								SetText( IDC_ROUTEFROMKEYPAGE_NUMBERTOREPORT, txt );
							}
							if (EditPrefPtr->nFromKeyPageMaxRows > MAX_KEYPAGE_ROWS)
							{
								EditPrefPtr->nFromKeyPageMaxRows = MAX_KEYPAGE_ROWS;
								(void)sprintf(txt, "%d", EditPrefPtr->nFromKeyPageMaxRows);
								SetText( IDC_ROUTEFROMKEYPAGE_NUMBERTOREPORT, txt );
							}
						}
					}
					break;
				case IDC_ROUTEFROMKEYPAGE_CHANGE:
					{
						char txt[KEYPAGE_LENGTH];

						GetText( IDC_ROUTEFROMKEYPAGE_KEYPAGE, txt ,KEYPAGE_LENGTH );
						if( txt[0] && (gRouteFromKeyPagesItemSel < EditPrefPtr->nFromKeyPages) )
						{
							strcpy( EditPrefPtr->szFromKeyPages[gRouteFromKeyPagesItemSel], txt );
							ListView_Update( hList , gRouteFromKeyPagesItemSel-1 );
						}
					}
					break;
				case IDC_ROUTEFROMKEYPAGE_ADD:
					{
						char txt[MAX_FILTERSIZE];

						GetText( IDC_ROUTEFROMKEYPAGE_KEYPAGE, txt ,KEYPAGE_LENGTH );
						if ( ( txt[0] ) && (EditPrefPtr->nFromKeyPages < NUMBER_OF_KEYPAGES) )
						{
							strcpy( EditPrefPtr->szFromKeyPages[EditPrefPtr->nFromKeyPages], txt );
							AddItemToListView( hList, EditPrefPtr->nFromKeyPages, 1, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->nFromKeyPages );
							EditPrefPtr->nFromKeyPages++;
						}
					}
					break;
				case IDC_ROUTEFROMKEYPAGE_REM:
					count = ListView_GetSelectedCount( hList );
					if ( count )			// copy log files to global list
					{
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->nFromKeyPages )
						{
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->szFromKeyPages[item], EditPrefPtr->szFromKeyPages[item+1], (EditPrefPtr->nFromKeyPages-item) * (KEYPAGE_LENGTH+1) );
							EditPrefPtr->nFromKeyPages--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
				case IDC_ROUTEFROMKEYPAGE_CLEAR:
					{
						item = ListView_GetNextItem( hList, -1 , LVNI_ALL );
						while( item != -1 && EditPrefPtr->nFromKeyPages )
						{
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->szFromKeyPages[item], EditPrefPtr->szFromKeyPages[item+1], (EditPrefPtr->nFromKeyPages-item) * (KEYPAGE_LENGTH+1) );
							EditPrefPtr->nFromKeyPages--;
							item = ListView_GetNextItem( hList, -1 , LVNI_ALL );
						}

						SetText( IDC_ROUTEFROMKEYPAGE_KEYPAGE, "" );
					}
					break;
			}
		break;
	}
	return 0;

}


// ****************************************************************************
// KeyVisitors message handler
//	IDC_PREFKEYVISITOR_ADD,		IDS_PREFKEYVISITOR_ADD,
//	IDC_PREFKEYVISITOR_CHANGE,	IDS_STRING_REMOVETHIS,	
//	IDC_PREFKEYVISITOR_NAME,	IDS_PREFKEYVISITOR_NAME,
//	IDC_PREFKEYVISITOR_DEL,		IDS_PREFKEYVISITOR_REM,	
//	IDC_PREFKEYVISITOR_DEFAULT,	IDS_PREFKEYVISITOR_DEFAULT,
// ****************************************************************************
HWND hWndKeyVisitorListView = NULL;

LRESULT KeyVisitorListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	long count,item;
	HWND hDlg = hWnd, hList = hWndKeyVisitorListView;
static int gKeyVisitorItemSel;

    switch (msg) {
		case WM_INITDIALOG: 
			hList = hWndKeyVisitorListView = InitGenericListView( hDlg, IDC_PREFKEYVISITOR_LIST, IDS_KEYVISITORHEADER, EditPrefPtr->filterdata.keyvisitorsTot, 0 );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hWndKeyVisitorListView );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[MAX_PATH]="(null)\0";    // Place to store some text

				{
					switch(pLvdi->hdr.code)	{
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							if ( itemSel < EditPrefPtr->filterdata.keyvisitorsTot )
							{
								mystrcpy( szText, EditPrefPtr->filterdata.keyvisitors[itemSel] );
							}
							break;
						case LVN_ITEMCHANGING:
							gKeyVisitorItemSel = itemSel;
							if ( itemSel >= 0 )
							{
								SetText( IDC_PREFKEYVISITOR_NAME, EditPrefPtr->filterdata.keyvisitors[itemSel] );
							}
							break;
					}
				}
			}
			break;
		case WM_COMMAND:
            switch (wParam) {
				case IDC_PREFKEYVISITOR_CHANGE:
					{
						char txt[MAX_FILTERSIZE];

						GetText( IDC_PREFKEYVISITOR_NAME, txt ,MAX_FILTERSIZE );
						if( txt[0] && (gKeyVisitorItemSel < EditPrefPtr->filterdata.keyvisitorsTot) )
						{
							strcpy( EditPrefPtr->filterdata.keyvisitors[gKeyVisitorItemSel], txt );
							ListView_Update( hList , gKeyVisitorItemSel-1 );
						}
					}
					break;
				case IDC_PREFKEYVISITOR_NAME:
				case IDC_PREFKEYVISITOR_ADD:
					{
						if (EditPrefPtr->filterdata.keyvisitorsTot >= MAX_FILTERUNITS)
						{
							if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
							{
								SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
							}
							break;
						}

						char txt[MAX_FILTERSIZE];

						GetText( IDC_PREFKEYVISITOR_NAME, txt ,MAX_FILTERSIZE );
						if ( ( txt[0] ) && (EditPrefPtr->filterdata.keyvisitorsTot < MAX_FILTERUNITS) )
						{
							strcpy( EditPrefPtr->filterdata.keyvisitors[EditPrefPtr->filterdata.keyvisitorsTot], txt );
							AddItemToListView( hList, EditPrefPtr->filterdata.keyvisitorsTot, 4, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->filterdata.keyvisitorsTot );
							EditPrefPtr->filterdata.keyvisitorsTot++;
						}
					}
					break;
				case IDC_PREFKEYVISITOR_DEL:
					count = ListView_GetSelectedCount( hList );
					if ( count )			// copy log files to global list
					{
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->filterdata.keyvisitorsTot )
						{
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->filterdata.keyvisitors[item], EditPrefPtr->filterdata.keyvisitors[item+1], (EditPrefPtr->filterdata.keyvisitorsTot-item) * MAX_FILTERSIZE );
							EditPrefPtr->filterdata.keyvisitorsTot--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
				case IDC_PREFKEYVISITOR_DEFAULT:
					{
						while( 	(item = ListView_GetNextItem( hList, -1 , LVNI_ALL  )) != -1 ){
							ListView_DeleteItem( hList, item );
						}

						SetText( IDC_PREFKEYVISITOR_NAME, "" );
					}
					break;
			}
		break;
	}
	return 0;

}
// ****************************************************************************
// End - KeyVisitors list.
// ****************************************************************************


HWND hWndStatsOrgListView = NULL;

LRESULT StatsOrgListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	long count,item;
	HWND hDlg = hWnd, hList = hWndStatsOrgListView;
static int gOrgItemSel;

    switch (msg) {
		case WM_INITDIALOG: 
			hList = hWndStatsOrgListView = InitGenericListView( hDlg, IDC_PREFORG_LIST, IDS_ORGHEADER, EditPrefPtr->filterdata.orgTot, 0 );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hWndStatsOrgListView );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[MAX_PATH]="(null)\0";    // Place to store some text

				{
					switch(pLvdi->hdr.code)	{
						case LVN_KEYDOWN:
							{
								LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
								if ( pnkd->wVKey == VK_DELETE )
								{
									SendMessage( hWnd, WM_COMMAND, IDC_PREFORG_DEL, lParam );
								}
							}
							break;
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							if ( itemSel < EditPrefPtr->filterdata.orgTot ){
								if ( pLvdi->item.iSubItem == 0 ){
									strcpyuntil( szText, EditPrefPtr->filterdata.org[itemSel], '=' );
								} else {
									char *p = mystrchr( EditPrefPtr->filterdata.org[itemSel], '=' );
									if ( p )
										mystrcpy( szText, p+1 );
								}
							}
							break;
						case LVN_ITEMCHANGING:
							gOrgItemSel = itemSel;
							if ( itemSel >= 0 ){
								char *p;
								strcpyuntil( szText, EditPrefPtr->filterdata.org[itemSel], '=' );
								SetText( IDC_PREFORG_PATTERN, szText );
								p = mystrchr( EditPrefPtr->filterdata.org[itemSel], '=' );
								SetText( IDC_PREFORG_NAME, p+1 );
							}
							break;
					}
				}
			}
			break;
		case WM_COMMAND:
            switch (wParam) {
				case IDC_PREFORG_CHANGE:
					{
						char pat[MAX_FILTERSIZE],txt[MAX_FILTERSIZE];

						GetText( IDC_PREFORG_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFORG_NAME, txt ,MAX_FILTERSIZE );
						if( txt[0] && pat[0] && (gOrgItemSel < EditPrefPtr->filterdata.orgTot) ){
							sprintf( EditPrefPtr->filterdata.org[gOrgItemSel], "%s=%s", pat, txt );
							ListView_Update( hList , gOrgItemSel-1 );
						}
					}
					break;
				case IDC_PREFORG_NAME:
				case IDC_PREFORG_ADD:
					{
						if (EditPrefPtr->filterdata.orgTot >= MAX_FILTERUNITS)
						{
							if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
							{
								SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
							}
							break;
						}

						char pat[MAX_FILTERSIZE],txt[MAX_FILTERSIZE];

						GetText( IDC_PREFORG_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFORG_NAME, txt ,MAX_FILTERSIZE );
						if ( (txt[0]) && (pat[0]) && (EditPrefPtr->filterdata.orgTot < MAX_FILTERUNITS) )
						{
							sprintf( EditPrefPtr->filterdata.org[EditPrefPtr->filterdata.orgTot], "%s=%s", pat, txt );
							AddItemToListView( hList, EditPrefPtr->filterdata.orgTot, 4, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->filterdata.orgTot );
							EditPrefPtr->filterdata.orgTot++;
						}
					}
					break;
				case IDC_PREFORG_DEL:
					count = ListView_GetSelectedCount( hList );
					if ( count ){			// copy log files to global list
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->filterdata.orgTot ){
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->filterdata.org[item], EditPrefPtr->filterdata.org[item+1], (EditPrefPtr->filterdata.orgTot-item) * MAX_FILTERSIZE );
							EditPrefPtr->filterdata.orgTot--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
				case IDC_PREFORG_DEFAULT:
					{
						short i;
						while( 	(item = ListView_GetNextItem( hList, -1 , LVNI_ALL  )) != -1 ){
							ListView_DeleteItem( hList, item );
						}

						InitLanguage( EditPrefPtr->language, 0 );
						TopLevelDomainStrings( EditPrefPtr );
						for( i=0; i < EditPrefPtr->filterdata.orgTot; i++ )
						{
							AddItemToListView( hList, i, 4, 0, LIST_VIEW_SEP );
						}
					}
					break;
			}
		break;
	}
	return 0;

}


// ******************   GROUP LIST **********************/


HWND hWndStatsGroupListView = NULL;
LRESULT StatsGroupListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	long count,item;
	HWND hDlg = hWnd, hList = hWndStatsGroupListView;
static int gGrpItemSel;

    switch (msg) {
		case WM_INITDIALOG:
			hList = hWndStatsGroupListView = InitGenericListView( hDlg, IDC_PREFGRP_LIST, IDS_GROUPHEADER, EditPrefPtr->filterdata.groupTot, 0 );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hWndStatsGroupListView );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[MAX_PATH]="(null)\0";    // Place to store some text

				{
					switch(pLvdi->hdr.code)	{
						case LVN_KEYDOWN:
							{
								LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
								if ( pnkd->wVKey == VK_DELETE )
								{
									SendMessage( hWnd, WM_COMMAND, IDC_PREFGRP_REM, lParam );
								}
							}
							break;
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							if ( itemSel < EditPrefPtr->filterdata.groupTot ){
								char *p;
								if ( pLvdi->item.iSubItem == 0 ){
									mystrcpy( szText, EditPrefPtr->filterdata.group[itemSel] );
									p = strrchr( szText, '=' );
									if ( p ) *p = 0;
								} else {
									p = strrchr( EditPrefPtr->filterdata.group[itemSel], '=' );
									if ( p )
										mystrcpy( szText, p+1 );
								}
							}
							break;
						case LVN_ITEMCHANGING:
							gGrpItemSel = itemSel;
							if ( itemSel >= 0 ){
								char *p;
								mystrcpy( szText, EditPrefPtr->filterdata.group[itemSel] );
								if ( p=strrchr(szText,'=') ) *p = 0;
								SetText( IDC_PREFGRP_PATTERN, szText );

								p = strrchr( EditPrefPtr->filterdata.group[itemSel], '=' );
								SetText( IDC_PREFGRP_NAME, p+1 );
							}
							break;
					}
				}
			}
			break;
		case WM_COMMAND:
            switch (wParam) {
				case IDC_PREFGRP_CHANGE:
					{
						char pat[MAX_FILTERSIZE],txt[MAX_FILTERSIZE];

						GetText( IDC_PREFGRP_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFGRP_NAME, txt ,MAX_FILTERSIZE );
						if( txt[0] && pat[0] && (gGrpItemSel < EditPrefPtr->filterdata.groupTot) ){
							sprintf( EditPrefPtr->filterdata.group[gGrpItemSel], "%s=%s", pat, txt );
							ListView_Update( hList , gGrpItemSel-1 );
						}
					}
					break;
				case IDC_PREFGRP_NAME:
				case IDC_PREFGRP_ADD:
					{
						if (EditPrefPtr->filterdata.groupTot >= MAX_FILTERUNITS)
						{
							if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
							{
								SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
							}
							break;
						}
						char pat[MAX_FILTERSIZE],txt[MAX_FILTERSIZE];

						GetText( IDC_PREFGRP_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFGRP_NAME, txt ,MAX_FILTERSIZE );
						if ( (txt[0]) && (pat[0]) && (EditPrefPtr->filterdata.groupTot < MAX_FILTERUNITS) )
						{
							sprintf( EditPrefPtr->filterdata.group[EditPrefPtr->filterdata.groupTot], "%s=%s", pat, txt );
							AddItemToListView( hList, EditPrefPtr->filterdata.groupTot, 4, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->filterdata.groupTot );
							EditPrefPtr->filterdata.groupTot++;
						}
					}
					break;
				case IDC_PREFGRP_REM:
					count = ListView_GetSelectedCount( hList );
					if ( count ){			// copy log files to global list
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->filterdata.groupTot ){
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->filterdata.group[item], EditPrefPtr->filterdata.group[item+1], (EditPrefPtr->filterdata.groupTot-item) * MAX_FILTERSIZE );
							EditPrefPtr->filterdata.groupTot--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
			}
		break;
	}
	return 0;

}








HWND hWndStatsReferralListView = NULL;
LRESULT StatsReferralListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	long	count,item;
	HWND	hDlg = hWnd,
			hList = hWndStatsReferralListView;
static int gItemSel;

#define dataMap		EditPrefPtr->filterdata.referralMap

    switch (msg) {
		case WM_INITDIALOG: 
			hList = hWndStatsReferralListView = InitGenericListView( hDlg, IDC_PREFREF_LIST, IDS_REFERHEADER, EditPrefPtr->filterdata.referralTot, 0 );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hWndStatsReferralListView );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[MAX_PATH]="(null)\0";    // Place to store some text

				{
					switch(pLvdi->hdr.code)	
					{
						case LVN_KEYDOWN:
							{
								LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
								if ( pnkd->wVKey == VK_DELETE )
								{
									SendMessage( hWnd, WM_COMMAND, IDC_PREFREF_REM, lParam );
								}
							}
							break;
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							if ( itemSel < EditPrefPtr->filterdata.referralTot )
							{
								char *p;
								if ( pLvdi->item.iSubItem == 0 )
								{
									mystrcpy( szText, dataMap[itemSel] );
									p = strrchr( szText, '=' );
									if ( p ) *p = 0;
									//strcpyuntil( szText, dataMap[itemSel], '=' );
								} else {
									p = strrchr( dataMap[itemSel], '=' );
									if ( p )
										mystrcpy( szText, p+1 );
								}
							}
							break;
						case LVN_ITEMCHANGING:
							gItemSel = itemSel;
							if ( itemSel >= 0 )
							{
								char *p;

								mystrcpy( szText, dataMap[itemSel] );
								if ( p=strrchr(szText,'=') ) *p = 0;
								SetText( IDC_PREFREF_PATTERN, szText );

								p = strrchr( dataMap[itemSel], '=' );
								SetText( IDC_PREFREF_NAME, p+1 );
							}
							break;
					}
				}
			}
			break;
		case WM_COMMAND:
            switch (wParam) {
				case IDC_PREFREF_CHANGE:
					{
						char pat[MAX_FILTERSIZE],txt[MAX_FILTERSIZE];

						GetText( IDC_PREFREF_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFREF_NAME, txt ,MAX_FILTERSIZE );
						if( txt[0] && pat[0] && (gItemSel < EditPrefPtr->filterdata.referralTot) ){
							sprintf( dataMap[gItemSel], "%s=%s", pat, txt );
							ListView_Update( hList , gItemSel-1 );
						}
					}
					break;
				case IDC_PREFREF_NAME:
				case IDC_PREFREF_ADD:
					{
						if (EditPrefPtr->filterdata.referralTot >= MAX_FILTERUNITS)
						{
							if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
							{
								SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
							}
							break;
						}
						char pat[MAX_FILTERSIZE],txt[MAX_FILTERSIZE];

						GetText( IDC_PREFREF_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFREF_NAME, txt ,MAX_FILTERSIZE );
						if ( (txt[0]) && (pat[0]) && (EditPrefPtr->filterdata.referralTot < MAX_FILTERUNITS) )
						{
							sprintf( dataMap[EditPrefPtr->filterdata.referralTot], "%s=%s", pat, txt );
							AddItemToListView( hList, EditPrefPtr->filterdata.referralTot, 4, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->filterdata.referralTot );
							EditPrefPtr->filterdata.referralTot++;
						}
					}
					break;
				case IDC_PREFREF_REM:
					count = ListView_GetSelectedCount( hList );
					if ( count ){			// copy log files to global list
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->filterdata.referralTot ){
							ListView_DeleteItem( hList, item );
							memcpy( dataMap[item], dataMap[item+1], (EditPrefPtr->filterdata.referralTot-item) * MAX_FILTERSIZE );
							EditPrefPtr->filterdata.referralTot--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
			}
		break;
	}
	return 0;

}


/*************************************************************************

   CLUSTERS:

  test is , "ip,path=Cluster Name

**************************************************************************/
void UpdateClusterWidgets( HWND hDlg, int enabled )
{
	if( enabled )
	{
		Enable( IDC_CLUSTER_NAME );
		Enable( IDC_CLUSTER_IP );
		Enable( IDC_CLUSTER_PATTERN );
		Enable( IDC_CLUSTER_LIST );
		Enable( IDC_CLUSTER_ADD );
		Enable( IDC_CLUSTER_DEL );
		Enable( IDC_CLUSTER_CHANGE );
		Enable( IDC_PREFVD_CLUSTERNUM );
	} else
	{
		Disable( IDC_CLUSTER_NAME );
		Disable( IDC_CLUSTER_IP );
		Disable( IDC_CLUSTER_PATTERN );
		Disable( IDC_CLUSTER_LIST );
		Disable( IDC_CLUSTER_ADD );
		Disable( IDC_CLUSTER_DEL );
		Disable( IDC_CLUSTER_CHANGE );
		Disable( IDC_PREFVD_CLUSTERNUM );
	}

}

void ClearSeperatorsFromString( char *string )
{
	DEF_ASSERT( string );

	char *pt = string;
	while( *pt )
	{
		if ( *pt == '=' ) *pt = 0;
		if ( *pt == ',' ) *pt = 0;
		pt++;
	}
	pt++;
	*pt=0;
}


HWND hWndClusterListView = NULL;
LRESULT ClusterListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	long	count,item;
	HWND	hDlg = hWnd, 
			hList = hWndClusterListView;
static int	gGrpItemSel;
	char	nameStr[MAX_FILTERSIZE],
			ipStr[MAX_FILTERSIZE],
			pathStr[MAX_FILTERSIZE];

    switch (msg) {
		case WM_INITDIALOG:
			hList = hWndClusterListView = InitGenericListView( hDlg, IDC_CLUSTER_LIST, IDS_CLUSTERHEADER, EditPrefPtr->clusterNamesTot, 0 );
			UpdateClusterWidgets( hWnd, EditPrefPtr->clusteringActive );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hWndClusterListView );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[MAX_PATH]="(null)\0";    // Place to store some text

				if ( itemSel>=0 && itemSel <= EditPrefPtr->clusterNamesTot )
				{
					switch(pLvdi->hdr.code)	
					{
						case LVN_KEYDOWN:
							{
								LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
								if ( pnkd->wVKey == VK_DELETE )
								{
									SendMessage( hWnd, WM_COMMAND, IDC_CLUSTER_DEL, lParam );
								}
							}
							break;
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							{
								char *p;

								// Date is of the type, "IP,PATH=NAME"
								switch( pLvdi->item.iSubItem )
								{
									// NAME
									case 0 :
										mystrcpy( szText, EditPrefPtr->clusterNames[itemSel] );
										break;
									// IP
									case 1 :
										p = strchr( EditPrefPtr->clusterNames[itemSel], 0 );
										mystrcpy( szText, p+1 );
										break;
									// PATTERN
									case 2 :
										p = strchr( EditPrefPtr->clusterNames[itemSel], 0 );
										p = strchr( p+1, 0 );
										mystrcpy( szText, p+1 );
										break;
								}
							}
							break;
						case LVN_ITEMCHANGING:
							gGrpItemSel = itemSel;
							if ( itemSel >= 0 )
							{
								char *p;

								mystrcpy( szText, EditPrefPtr->clusterNames[itemSel] );
								SetText( IDC_CLUSTER_NAME, szText );

								p = strchr( EditPrefPtr->clusterNames[itemSel], 0 );
								mystrcpy( szText, p+1 );
								SetText( IDC_CLUSTER_IP, szText );
								
								p = strchr( EditPrefPtr->clusterNames[itemSel], 0 );
								p = strchr( p+1, 0 );
								mystrcpy( szText, p+1 );
								SetText( IDC_CLUSTER_PATTERN, szText );

							}
							break;
					}
				}
			}
			break;

		case WM_COMMAND:
            switch (wParam) 
			{
				case IDC_PREFVD_CLUSTER:
					UpdateClusterWidgets( hWnd, IsChecked( IDC_PREFVD_CLUSTER ) );
					break;

				case IDC_CLUSTER_CHANGE:
					GetText( IDC_CLUSTER_NAME, nameStr ,MAX_FILTERSIZE );
					GetText( IDC_CLUSTER_IP, ipStr ,MAX_FILTERSIZE );
					GetText( IDC_CLUSTER_PATTERN, pathStr ,MAX_FILTERSIZE );

					if( nameStr[0] && pathStr[0] && (gGrpItemSel < EditPrefPtr->clusterNamesTot) )
					{
						sprintf( EditPrefPtr->clusterNames[gGrpItemSel], "%s,%s,%s", nameStr, ipStr, pathStr );
						ClearSeperatorsFromString( EditPrefPtr->clusterNames[gGrpItemSel] );
						ListView_Update( hList , gGrpItemSel-1 );
					}
					break;

				case IDC_CLUSTER_NAME:
				case IDC_CLUSTER_ADD:
					GetText( IDC_CLUSTER_NAME, nameStr ,MAX_FILTERSIZE );
					GetText( IDC_CLUSTER_IP, ipStr ,MAX_FILTERSIZE );
					GetText( IDC_CLUSTER_PATTERN, pathStr ,MAX_FILTERSIZE );

					if ( nameStr[0] && ipStr[0] && !pathStr[0] )
						strcpy( pathStr, "-" );

					if (EditPrefPtr->clusterNamesTot >= MAX_FILTERUNITS)
					{
						if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
						{
							SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
						}
						break;
					}

					if( nameStr[0] )
					{
						if ( ( ipStr[0] || pathStr[0] ) && (EditPrefPtr->clusterNamesTot < MAX_FILTERUNITS) )
						{
							sprintf( EditPrefPtr->clusterNames[EditPrefPtr->clusterNamesTot], "%s,%s,%s", nameStr, ipStr, pathStr );
							ClearSeperatorsFromString( EditPrefPtr->clusterNames[EditPrefPtr->clusterNamesTot] );
							AddItemToListView( hList, EditPrefPtr->clusterNamesTot, 4, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->clusterNamesTot );
							EditPrefPtr->clusterNamesTot++;
						}
					}
					break;

				case IDC_CLUSTER_DEL:
					count = ListView_GetSelectedCount( hList );
					if ( count )
					{			// copy log files to global list
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->clusterNamesTot ){
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->clusterNames[item], EditPrefPtr->clusterNames[item+1], (EditPrefPtr->clusterNamesTot-item) * MAX_FILTERSIZE );
							EditPrefPtr->clusterNamesTot--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
			}
		break;
	}
	return 0;

}







HWND hWndAddsListView = NULL;
LRESULT StatsAddListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
static int gAddItemSel;
	long count,item;
	HWND hDlg = hWnd, hList = hWndAddsListView;

    switch (msg) {
		case WM_INITDIALOG:
			hList = hWndAddsListView = InitGenericListView( hDlg, IDC_PREFADD_LIST, IDS_ADDHEADER, EditPrefPtr->filterdata.advertTot, 0 );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hList );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[MAX_PATH]="(null)\0";    // Place to store some text

				{
					switch(pLvdi->hdr.code)	{
						case LVN_KEYDOWN:
							{
								LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
								if ( pnkd->wVKey == VK_DELETE )
								{
									SendMessage( hWnd, WM_COMMAND, IDC_PREFADD_REM, lParam );
								}
							}
							break;
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							if ( itemSel < EditPrefPtr->filterdata.advertTot ){
								char *p = EditPrefPtr->filterdata.advert[itemSel];
								switch( pLvdi->item.iSubItem ){
									case 0:	strcpyuntil( szText, p, ',' ); break;
									case 1:	p = mystrchr( p, ',' );
											strcpyuntil( szText, p+1, ',' ); break;
									case 2:	p = strrchr( p, ',' );
											strcpyuntil( szText, p+1, ',' ); break;
								}
							}
							break;
						case LVN_ITEMCHANGING:
							gAddItemSel = itemSel;
							if ( itemSel >= 0 ){
								char *p = EditPrefPtr->filterdata.advert[itemSel];
								strcpyuntil( szText, p, ',' );		SetText( IDC_PREFADD_NAME, szText );
								p = mystrchr( p, ',' );
								strcpyuntil( szText, p+1, ',' );	SetText( IDC_PREFADD_PATTERN1, szText );
								p = strrchr( p, ',' );
								strcpyuntil( szText, p+1, ',' );	SetText( IDC_PREFADD_PATTERN2, szText );
							}
							break;
					}
				}
			}
			break;
		case WM_COMMAND:
            switch (wParam) {
				case IDC_PREFADD_CHANGE:
					{
						char	pat[MAX_FILTERSIZE],
								pat2[MAX_FILTERSIZE],
								txt[MAX_FILTERSIZE];

						GetText( IDC_PREFADD_NAME, txt ,MAX_FILTERSIZE );
						GetText( IDC_PREFADD_PATTERN1, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFADD_PATTERN2, pat2 ,MAX_FILTERSIZE );
						if( txt[0] && pat[0] && (gAddItemSel < EditPrefPtr->filterdata.advertTot) ){
							sprintf( EditPrefPtr->filterdata.advert[gAddItemSel], "%s,%s,%s", txt, pat, pat2 );
							ListView_Update( hList , gAddItemSel-1 );
						}
					}
					break;
				case IDC_PREFADD_NAME:
				case IDC_PREFADD_ADD:
					{
						if (EditPrefPtr->filterdata.advertTot >= MAX_FILTERUNITS)
						{
							if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
							{
								SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
							}
							break;
						}

						char	pat[MAX_FILTERSIZE],
								pat2[MAX_FILTERSIZE],
								txt[MAX_FILTERSIZE];

						GetText( IDC_PREFADD_NAME, txt ,MAX_FILTERSIZE );
						GetText( IDC_PREFADD_PATTERN1, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFADD_PATTERN2, pat2 ,MAX_FILTERSIZE );
						if( txt[0] && pat[0] && pat2[0] && (EditPrefPtr->filterdata.advertTot < MAX_FILTERUNITS))
						{
							sprintf( EditPrefPtr->filterdata.advert[EditPrefPtr->filterdata.advertTot], "%s,%s,%s", txt, pat, pat2 );
							AddItemToListView( hList, EditPrefPtr->filterdata.advertTot, 4, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->filterdata.advertTot );
							EditPrefPtr->filterdata.advertTot++;
						}
					}
					break;
				case IDC_PREFADD_REM:
					count = ListView_GetSelectedCount( hList );
					if ( count ){			// copy log files to global list
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->filterdata.advertTot ){
							ListView_DeleteItem( hList, item );
							if ( item < EditPrefPtr->filterdata.advertTot )
								memcpy( EditPrefPtr->filterdata.advert[item], EditPrefPtr->filterdata.advert[item+1], (EditPrefPtr->filterdata.advertTot-item) * MAX_FILTERSIZE );
							EditPrefPtr->filterdata.advertTot--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
			}
		break;
	}
	return 0;

}






HWND hWndAddCampListView = NULL;

LRESULT StatsAddCampListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	long count,item;
	HWND hDlg = hWnd, hList = hWndAddCampListView;
	static int gAdd2ItemSel;

    switch (msg) {
		case WM_INITDIALOG: 
			hList = hWndAddCampListView = InitGenericListView( hDlg, IDC_ADDCAMP_LIST, IDS_ADDHEADER2, EditPrefPtr->filterdata.advert2Tot, 0 );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hList );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[MAX_PATH]="(null)\0";    // Place to store some text

				{
					switch(pLvdi->hdr.code)	{
						case LVN_KEYDOWN:
							{
								LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
								if ( pnkd->wVKey == VK_DELETE )
								{
									SendMessage( hWnd, WM_COMMAND, IDC_ADDCAMP_REM, lParam );
								}
							}
							break;
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							if ( itemSel < EditPrefPtr->filterdata.advert2Tot ){
								char *p = EditPrefPtr->filterdata.advert2[itemSel];
								switch( pLvdi->item.iSubItem ){
									case 0:	strcpyuntil( szText, p, ',' ); break;
									case 1:	p = mystrchr( p, ',' );
											strcpyuntil( szText, p+1, ',' ); break;
									case 2:	p = strrchr( p, ',' );
											strcpyuntil( szText, p+1, ',' ); break;
								}
							}
							break;
						case LVN_ITEMCHANGING:
							gAdd2ItemSel = itemSel;
							if ( itemSel >= 0 ){
								char *p = EditPrefPtr->filterdata.advert2[itemSel];
								strcpyuntil( szText, p, ',' );		SetText( IDC_ADDCAMP_NAME, szText );
								p = mystrchr( p, ',' );
								strcpyuntil( szText, p+1, ',' );	SetText( IDC_ADDCAMP_PATTERN, szText );
								p = strrchr( p, ',' );
								strcpyuntil( szText, p+1, ',' );	SetText( IDC_ADDCAMP_COST, szText );
							}
							break;
					}
				}
			}
			break;
		case WM_COMMAND:
            switch (wParam) {
				case IDC_ADDCAMP_CHANGE:
					{
						char	pat[MAX_FILTERSIZE],
								pat2[MAX_FILTERSIZE],
								txt[MAX_FILTERSIZE];

						GetText( IDC_ADDCAMP_NAME, txt ,MAX_FILTERSIZE );
						GetText( IDC_ADDCAMP_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_ADDCAMP_COST, pat2 ,MAX_FILTERSIZE );
						if( txt[0] && pat[0] && (gAdd2ItemSel < EditPrefPtr->filterdata.advert2Tot) ){
							sprintf( EditPrefPtr->filterdata.advert2[gAdd2ItemSel], "%s,%s,%s", txt, pat, pat2 );
							ListView_Update( hList , gAdd2ItemSel-1 );
						}
					}
					break;
				case IDC_ADDCAMP_NAME:
				case IDC_ADDCAMP_ADD:
					{
						if (EditPrefPtr->filterdata.advert2Tot >= MAX_FILTERUNITS)
						{
							if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
							{
								SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
							}
							break;
						}
						char	pat[MAX_FILTERSIZE],
								pat2[MAX_FILTERSIZE],
								txt[MAX_FILTERSIZE];

						GetText( IDC_ADDCAMP_NAME, txt ,MAX_FILTERSIZE );
						GetText( IDC_ADDCAMP_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_ADDCAMP_COST, pat2 ,MAX_FILTERSIZE );
						if( txt[0] && pat[0] && pat2[0] && (EditPrefPtr->filterdata.advert2Tot < MAX_FILTERUNITS) ){
							sprintf( EditPrefPtr->filterdata.advert2[EditPrefPtr->filterdata.advert2Tot], "%s%s%s%s%s", txt, LIST_VIEW_SEP, pat, LIST_VIEW_SEP, pat2 );
							AddItemToListView( hList, EditPrefPtr->filterdata.advert2Tot, 4, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->filterdata.advert2Tot );
							EditPrefPtr->filterdata.advert2Tot++;
						}
					}
					break;
				case IDC_ADDCAMP_REM:
					count = ListView_GetSelectedCount( hList );
					if ( count ){			// copy log files to global list
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->filterdata.advert2Tot ){
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->filterdata.advert2[item], EditPrefPtr->filterdata.advert2[item+1], (EditPrefPtr->filterdata.advert2Tot-item) * MAX_FILTERSIZE );
							EditPrefPtr->filterdata.advert2Tot--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
			}
		break;
	}
	return 0;

}




HWND hWndVhostMapListView = NULL;
static long gVhostItemSel = 0;
LRESULT VhostMapListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	long count,item;
	HWND hDlg = hWnd, hList = hWndVhostMapListView;

    switch (msg) {
		case WM_INITDIALOG: 
			hList = hWndVhostMapListView = InitGenericListView( hDlg, IDC_PREFVD_LIST, IDS_VMAPHEADER, EditPrefPtr->filterdata.vhostmapTot, 0 );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			DestroyWindow( hWndVhostMapListView );
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				int				itemSel = pNm->iItem;
static			char			szText[MAX_PATH]="(null)\0";    // Place to store some text

				{
					switch(pLvdi->hdr.code)	{
						case LVN_KEYDOWN:
							{
								LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
								if ( pnkd->wVKey == VK_DELETE )
								{
									SendMessage( hWnd, WM_COMMAND, IDC_PREFVD_REM, lParam );
								}
							}
							break;
						case LVN_GETDISPINFO:
							pLvdi->item.pszText = szText;
							szText[0] = 0;
							itemSel = pLvdi->item.iItem;
							if ( itemSel < EditPrefPtr->filterdata.vhostmapTot ){
								if ( pLvdi->item.iSubItem == 0 ){
									strcpyuntil( szText, EditPrefPtr->filterdata.vhostmap[itemSel], '=' );
								} else {
									char *p = mystrchr( EditPrefPtr->filterdata.vhostmap[itemSel], '=' );
									if ( p )
										mystrcpy( szText, p+1 );
								}
							}
							break;
						case LVN_ITEMCHANGING:
							gVhostItemSel = itemSel;
							if ( itemSel >= 0 ){
								char *p;
								strcpyuntil( szText, EditPrefPtr->filterdata.vhostmap[itemSel], '=' );
								SetText( IDC_PREFVD_PATTERN, szText );
								p = mystrchr( EditPrefPtr->filterdata.vhostmap[itemSel], '=' );
								SetText( IDC_PREFVD_NAME, p+1 );
							}
							break;
					}
				}
			}
			break;
		case WM_COMMAND:
            switch (wParam) {
				case IDC_PREFVD_CHANGE:
					{
						char pat[MAX_FILTERSIZE],txt[MAX_FILTERSIZE];

						GetText( IDC_PREFVD_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFVD_NAME, txt ,MAX_FILTERSIZE );
						if( txt[0] && pat[0] && (gVhostItemSel < EditPrefPtr->filterdata.vhostmapTot) ){
							sprintf( EditPrefPtr->filterdata.vhostmap[gVhostItemSel], "%s=%s", pat, txt );
							ListView_Update( hList , gVhostItemSel-1 );
						}
					}
					break;
				case IDC_PREFVD_NAME:
				case IDC_PREFVD_ADD:
					{
						if (EditPrefPtr->filterdata.vhostmapTot >= MAX_FILTERUNITS)
						{
							if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
							{
								SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
							}
							break;
						}
						char pat[MAX_FILTERSIZE],txt[MAX_FILTERSIZE];

						GetText( IDC_PREFVD_PATTERN, pat ,MAX_FILTERSIZE );
						GetText( IDC_PREFVD_NAME, txt ,MAX_FILTERSIZE );
						if( txt[0] && pat[0] && (EditPrefPtr->filterdata.vhostmapTot < MAX_FILTERUNITS) ){
							sprintf( EditPrefPtr->filterdata.vhostmap[EditPrefPtr->filterdata.vhostmapTot], "%s=%s", pat, txt );
							AddItemToListView( hList, EditPrefPtr->filterdata.vhostmapTot, 4, 0, LIST_VIEW_SEP );
							SelectListViewItem( hList, EditPrefPtr->filterdata.vhostmapTot );
							EditPrefPtr->filterdata.vhostmapTot++;
						}
					}
					break;
				case IDC_PREFVD_REM:
					count = ListView_GetSelectedCount( hList );
					if ( count ){			// copy log files to global list
						item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						while( item != -1 && EditPrefPtr->filterdata.vhostmapTot ){
							ListView_DeleteItem( hList, item );
							memcpy( EditPrefPtr->filterdata.vhostmap[item], EditPrefPtr->filterdata.vhostmap[item+1], (EditPrefPtr->filterdata.vhostmapTot-item) * MAX_FILTERSIZE );
							EditPrefPtr->filterdata.vhostmapTot--;
							item = ListView_GetNextItem( hList, -1 , LVNI_SELECTED );
						}
					}
					break;
			}
		break;
	}
	return 0;

}












void List_DelCurrent( HWND hDlg, long filterID  )
{
	long index;

	index = ListBox_GetSelected( filterID );
	ListBox_DelString( filterID, index );
}


void List_ChangeItem( HWND hDlg, long nameID, long listID )
{
	char txt[MAX_FILTERSIZE];
	long index;

	index = ListBox_GetSelected( listID );

	GetText( nameID, txt ,MAX_FILTERSIZE );
	ListBox_ChangeString( listID, index, (long)txt );
}

long DataListGUItoData( HWND hDlg, long id, long total, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE] )
{
	long index=0,tot, num=0;

	filter[index][0]=0;
	tot = ListBox_GetTotal( id );
	if ( tot>MAX_FILTERUNITS ) tot=MAX_FILTERUNITS;
	for ( index=0; index < tot; index++ ){
		ListBox_GetText( id, index,  (long)filter[index] );
		filter[index+1][0]=0;
		num++;
	}
	return num;
}


long DataListDatatoGUI( HWND hDlg,long id, long total, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE] )
{
	int num=0;
	
	if ( total>0 ){
		while( total>0 ){
			ListBox_AddString( id, (long)filter[num] );
			num++;
			total--;
		}
	}
	return num;
}




LRESULT ProxyListHandler( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char txt[MAX_FILTERSIZE];

    switch (msg) {
		case WM_INITDIALOG: 
			EditPrefPtr->filterdata.routerTot = DataListDatatoGUI( hDlg,IDC_PROXY_LIST, EditPrefPtr->filterdata.routerTot, EditPrefPtr->filterdata.router );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hDlg, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			if ( prefsReturn )
				EditPrefPtr->filterdata.routerTot = DataListGUItoData( hDlg, IDC_PROXY_LIST, ListBox_GetTotal( IDC_PROXY_LIST ), EditPrefPtr->filterdata.router );
			break;
		case WM_COMMAND:
            switch (wParam) {
				case IDC_PROXY_LIST:
					break;
				case IDC_PROXY_NAME:
				case IDC_PROXY_ADD:
					if (EditPrefPtr->filterdata.routerTot < MAX_FILTERUNITS)
					{
						GetText( IDC_PROXY_NAME, txt ,MAX_FILTERSIZE );
						ListBox_AddString( IDC_PROXY_LIST, (long)txt );
						EditPrefPtr->filterdata.routerTot++;
					}
					else
					{
						if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
						{
							SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
						}
					}
					break;
				case IDC_PROXY_CHANGE:
					List_ChangeItem( hDlg, IDC_PROXY_NAME, IDC_PROXY_LIST );
					break;
				case IDC_PROXY_REM:
					List_DelCurrent( hDlg, IDC_PROXY_LIST );
					EditPrefPtr->filterdata.routerTot--;
					break;
				case IDC_PROXY_CLR:
					while( ListBox_GetTotal( IDC_PROXY_LIST ) >0 )
						ListBox_DelFirst( IDC_PROXY_LIST );
					break;
			}
		break;
	}
	return 0;

}


#include "engine_notify.h"
/**
char *StatListStr[] = {
	"Hour", "Weekday", "Month", "Date", "User",
	"Client", "SourceAddress", "SecondDomain", "Country",
	"Organization", "Region", "File", "Browser",
	"OperatingSystem", "Referral", "ReferralSite", "Directory",
	"TopDirectory", "FileType", "Error", "ErrorURL",
	"Page", "Search String", "SearchEngine", "Download",
	"AddHit", "AddClick", 0
};
char *StatFieldStr[] = {
	"Name", "URLs/Files", "Bytes", "Bytes-In", "Visits", "Errors", "VisitsTotal", 0
};
char *LogicStr[]= { ">=", "<=", "==", "!=", 0 };
/**/

LRESULT NotifyListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hDlg = hWnd;


    switch (msg) {
		case WM_INITDIALOG: 
			DataListDatatoGUI( hDlg,IDC_PREFNOTIFY_LIST, EditPrefPtr->filterdata.notifyTot, EditPrefPtr->filterdata.notify );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_DESTROY:
			if ( prefsReturn )
				EditPrefPtr->filterdata.notifyTot = DataListGUItoData( hDlg, IDC_PREFNOTIFY_LIST, EditPrefPtr->filterdata.notifyTot, EditPrefPtr->filterdata.notify );
			break;
		case WM_COMMAND:
            switch (wParam) {
				case IDC_PREFNOTIFY_LIST:
					break;
				case IDC_PREFNOTIFY_NAME:
				case IDC_PREFNOTIFY_ADD:
					
					if (EditPrefPtr->filterdata.notifyTot < MAX_FILTERUNITS)
					{
						char txt[MAX_FILTERSIZE], name[100];
						long db,fld,log;

						db = GetPopupNum( IDC_NOTIFY_DB );
						fld = GetPopupNum( IDC_NOTIFY_FIELD );
						log = GetPopupNum( IDC_NOTIFY_LOGIC );

						GetText( IDC_PREFNOTIFY_NAME, name , 100 );
						sprintf( txt, "%s %s %s %s", StatListStr[ db ], StatFieldStr[ fld ], LogicStr[ log ], name );
						ListBox_AddString( IDC_PREFNOTIFY_LIST, (long)txt );

						EditPrefPtr->filterdata.notifyTot++;
					}
					else
					{
						if ( ghPrefsDlg && IsWindowEnabled(ghPrefsDlg))
						{
							SetDlgItemText(ghPrefsDlg, IDC_EDIT_TIPS,  "No more rows can be added.");
						}
					}
					break;
				case IDC_PREFNOTIFY_REM:
					List_DelCurrent( hWnd, IDC_PREFNOTIFY_LIST );
					EditPrefPtr->filterdata.notifyTot--;
					if ( EditPrefPtr->filterdata.notifyTot < 0 ) EditPrefPtr->filterdata.notifyTot = 0;
					break;
				case IDC_PREFNOTIFY_CLR:
					while( EditPrefPtr->filterdata.notifyTot > 0 ){
						ListBox_DelFirst( IDC_PREFNOTIFY_LIST );
						EditPrefPtr->filterdata.notifyTot--;
					}
					break;
			}
			break;
	}
	return 0;
}




//------------------------------------------------------------------------------------------------------


void InitComboBox( HWND hDlg, char	mycombotxt[][32], long id, long num )
{

	int		loop;

	if ( hDlg ){
		for( loop=0; loop< num; loop++ ){
			AddPopupItem( id, mycombotxt[loop] );
		}
		SetPopupNum( id, 0 );
	}
}

void InitDropBox( HWND hDlg, char	*mycombotxt[], long id )
{
	int		loop=0;

	if ( hDlg ){
		while(  mycombotxt[loop] ){
			AddPopupItem( id, mycombotxt[loop] );
			loop++;
		}
		SetPopupNum( id, 0 );
	}
}



long Init_ComboBox( HWND hDlg, long comboid, long stringid )
{
	char data[512];
	char *stringPtr[64], *p;
	long	i = 0, c = 0;

	if ( hDlg ){
		LoadString( hInst, stringid, data, 512 );

		p = data;
		while( p ){
			stringPtr[c] = p;
			p = mystrchr( p, ',' );
			if ( p ){
				*p = 0;
				p++;
			}
			c++;
			stringPtr[c] = 0;
		}
		InitDropBox( hDlg, stringPtr, comboid );
	}
	return c;
}






long Init_ColorComboBox( HWND hDlg, long comboid )
{
	long	i = 0;

	if ( hDlg )
	{
		while( comboBoxColors[i].colorName[0] != 0 )
		{
			AddPopupItem( comboid, comboBoxColors[i].colorName );
			i++;
		}
	}
	return i;
}

long Init_ComboBoxClearAndSelect( HWND hDlg, long comboid, char *data, int sel )
{
	char *stringPtr[64], *p;
	long	i = 0, c = 0;
	int count;
	HWND ctrlHWND;

	// Clear the combo box
	ctrlHWND = GetDlgItem(hDlg, comboid);
	count = SendMessage( ctrlHWND, CB_GETCOUNT, 0, 0 );
	while( count > 0 )
	{
		SendMessage( ctrlHWND, CB_DELETESTRING, count-1, 0 );
		count--;
	}

	if ( hDlg )
	{
		p = data;
		while( p )
		{
			stringPtr[c] = p;
			p = mystrchr( p, ',' );
			if ( p )
			{
				*p = 0;
				p++;
			}
			c++;
			stringPtr[c] = 0;
		}
		InitDropBox( hDlg, stringPtr, comboid );
	}
	SetPopupNum( comboid, sel );
	return c;
}


HWND Init_TabControl( HWND hDlg, long tabid, long stringid )
{
	char data[512];
	char *stringPtr[64], *p;
	long	i = 0, c = 0;

	if ( hDlg ){
		LoadString( hInst, stringid, data, 512 );

		p = data;
		while( p ){
			stringPtr[c] = p;
			p = mystrchr( p, ',' );
			if ( p ){
				*p = 0;
				p++;
			}
			c++;
			stringPtr[c] = 0;
		}
		return DoCreateTabControl( hDlg, stringPtr, c, tabid );
	}
	return NULL;
}



/*****************************************************************************

    FUNCTION: HandleFocusState

    PURPOSE:  If button has focus, draw dashed rectangle around text in button

*****************************************************************************/
void RedrawColoredBlock( HWND hWnd, long id, LPDRAWITEMSTRUCT lpdis , long rgb)
{
	RECT		rectHi;         // rectangle around up button's "Hi!"
	long		RGB;

	RGB = SwapEndian( rgb );
	GetObjectRect( hWnd, GetDlgItem( hWnd, id ), &rectHi );
	rectHi.bottom -= rectHi.top;
	rectHi.right -= rectHi.left;
	rectHi.top = rectHi.left = 0;


	FrameRect( lpdis->hDC, (LPRECT)&rectHi, CreateSolidBrush( 0xffffff ) );
	rectHi.top++;
	rectHi.left++;
	FrameRect( lpdis->hDC, (LPRECT)&rectHi, CreateSolidBrush( 0x000000 ) );
	rectHi.bottom--;
	rectHi.right--;
	FillRect( lpdis->hDC, (LPRECT)&rectHi, CreateSolidBrush( RGB ) );

	//Rectange( lpdis->hDC, rectHi.left, rectHi.topCreateSolidBrush( 0xffffff ) );
    return;
}




LONG APIENTRY PrefsDialogsProcPDFAdvanced( HWND hWnd, UINT msg, DWORD dwParam, LONG lParam )
{
	HWND 	hDlg = hWnd;
    switch (msg) {
        case WM_INITDIALOG:
			Report_PDFPageAdv_DataToGUI( hDlg );
			break;

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)dwParam, GWL_ID ));
			break;

		case WM_COMMAND:
            switch ( LOWORD(dwParam) ) {
				case IDC_PREF_PDF_ADV_OK:
					if (HIWORD(dwParam) == BN_CLICKED)
					{
						Report_PDFPageAdv_GUIToData( hDlg );
						EndDialog(hWnd, TRUE);
						return TRUE;
					}
					break;
				case IDC_PREF_PDF_ADV_CANCEL:
				case IDCANCEL:
					if (HIWORD(dwParam) == BN_CLICKED)
					{
						EndDialog(hWnd, TRUE);
						return TRUE;
					}
					break;
				case IDC_PREF_PDF_REDUCEDATAFONTSIZE:
						if (HIWORD(dwParam) == BN_CLICKED )
						{
							if ( IsChecked( IDC_PREF_PDF_REDUCEDATAFONTSIZE ) )
								Enable( IDC_PREF_PDF_REDUCEDATAFONTSIZEPER );
							else
								Disable( IDC_PREF_PDF_REDUCEDATAFONTSIZEPER );
						}
						break;
				case IDC_PREF_PDF_REDUCEDATAFONTSIZEPER:
				case IDC_PREF_PDF_REDUCELARGESTDATACOL:
				case IDC_PREF_PDF_MINCOLHEADSCHARS:
					{
						if (HIWORD(dwParam) == EN_KILLFOCUS )
						{
							int ctrlId = LOWORD(dwParam);
							int bufSize = 32;
							char *buf = (char*)malloc( bufSize+1 );
							int num, min, max;
							GetText( ctrlId, buf, bufSize );
							num = atoi( buf );
							switch ( ctrlId )
							{
								case IDC_PREF_PDF_REDUCEDATAFONTSIZEPER:
									{
										min = PDF_reduceFontSizeOfTooLargeDataByPercent_Min;
										max = PDF_reduceFontSizeOfTooLargeDataByPercent_Max;
									}
									break;
								case IDC_PREF_PDF_REDUCELARGESTDATACOL:
									{
										min = PDF_reduceBiggestDataColWidthByPercent_Min;
										max = PDF_reduceBiggestDataColWidthByPercent_Max;
									}
									break;
								case IDC_PREF_PDF_MINCOLHEADSCHARS:
									{
										min = PDF_minimumTableColumnHeadingChars_Min;
										max = PDF_minimumTableColumnHeadingChars_Max;
									}
									break;
							}
							if ( num < min )
							{
								sprintf( buf, "%d", min );
								SetText( ctrlId, buf );
							}
							if ( num > max )
							{
								sprintf( buf, "%d", max );
								SetText( ctrlId, buf );
							}
							free( buf );
						}
					}
					break;
			break;
			}
        default:
            return FALSE;
    }

    return TRUE;
}


#define	PDF_TITLE			0
#define	PDF_COLUMNHEADINGS	1
#define	PDF_DATA			2

void ChangeATableTextAttributeCombo( WORD ctrlId )
{
	HWND hDlg = ghReportPDFDlg;
	int numSel;
	char buf[32];
	int i;

	if ( !hDlg )
		return;

	// Check which item we are changing in the combo
	numSel = GetPopupNum( IDC_PREF_PDF_TABLETEXTNAME );

	switch ( ctrlId )
	{
		case IDC_PREF_PDF_FONTSIZE:
			{
				GetText( IDC_PREF_PDF_FONTSIZE, buf, 32 );
				i = atoi( buf );
				switch ( numSel )
				{
				case PDF_TITLE:
					EditPrefPtr->pdfAllSetC.pdfTableSetC.titleSize = i;
					break;
				case PDF_COLUMNHEADINGS:
					EditPrefPtr->pdfAllSetC.pdfTableSetC.colHeadingSize = i;
					break;
				case PDF_DATA:
					EditPrefPtr->pdfAllSetC.pdfTableSetC.dataSize = i;
					break;
				}
			}
		case IDC_PREF_PDF_FONTSTYLE:
			{
				int i = GetPopupNum( IDC_PREF_PDF_FONTSTYLE );
				switch( numSel )
				{
				case PDF_TITLE:
					EditPrefPtr->pdfAllSetC.pdfTableSetC.titleStyle = i;
					break;
				case PDF_COLUMNHEADINGS:
					EditPrefPtr->pdfAllSetC.pdfTableSetC.colHeadingStyle = i;
					break;
				case PDF_DATA:
					EditPrefPtr->pdfAllSetC.pdfTableSetC.dataStyle = i;
					break;
				}
			}
			break;
		case IDC_PREF_PDF_FONTCOLOR:
			{
				int i = GetPopupNum( IDC_PREF_PDF_FONTCOLOR );
				switch( numSel )
				{
				case PDF_TITLE:
					EditPrefPtr->pdfAllSetC.pdfTableSetC.titleColor = comboBoxColors[i].colorValue;
					break;
				case PDF_COLUMNHEADINGS:
					EditPrefPtr->pdfAllSetC.pdfTableSetC.colHeadingColor = comboBoxColors[i].colorValue;
					break;
				case PDF_DATA:
					EditPrefPtr->pdfAllSetC.pdfTableSetC.dataColor = comboBoxColors[i].colorValue;
					break;
				}
			}
			break;
	}
}


#define	PDF_TINY_FONT	0
#define	PDF_SMALL_FONT	1
#define	PDF_NORMAL_FONT	2
#define	PDF_LARGE_FONT	3

void ChangeGraphFontsSizeCombo()
{
	HWND hDlg = ghReportPDFDlg;
	int numSel;
	char buf[32];
	int i;

	if ( !hDlg )
		return;

	// Check which item we are changing in the combo
	numSel = GetPopupNum( IDC_PREF_PDF_GRAPHFONT );
	GetText( IDC_PREF_PDF_GRAPHFONTSIZE, buf, 32 );
	i = atoi( buf );
	switch ( numSel )
	{
		case PDF_TINY_FONT:
			EditPrefPtr->pdfAllSetC.pdfGraphSetC.tinyFontSize = i;
			break;
		case PDF_SMALL_FONT:
			EditPrefPtr->pdfAllSetC.pdfGraphSetC.smallFontSize = i;
			break;
		case PDF_NORMAL_FONT:
			EditPrefPtr->pdfAllSetC.pdfGraphSetC.normalFontSize = i;
			break;
		case PDF_LARGE_FONT:
			EditPrefPtr->pdfAllSetC.pdfGraphSetC.largeFontSize = i;
			break;
	}
}


BOOL GetPPLocationName( char *putNamehere ) 
{
	char     *szFilter;
    szFilter = "Path (*.*)\0*.*\0\0\0";
	return GetSaveDialog( putNamehere, "here\0", 0, szFilter, ReturnString(IDS_SELECTOUTPUT), "", 1  );
}




LONG APIENTRY PrefsDialogsProc( HWND hWnd, UINT msg, DWORD dwParam, LONG lParam )
{
	HWND 	hDlg = hWnd;
	DWORD	wParam = dwParam;
static	HBITMAP	hbmBackGround;
static	BITMAP	bmBackGround;
static	HPALETTE	hPal=NULL, hPalOld;
	char szText[512], tmp[128];

    switch (msg) {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc;
                hdc = BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            break;

		case WM_PALETTECHANGED:
			if ( iDepth <=8 ){
				if( (HWND)wParam != hDlg )
					if( RealizeMyPalette(hDlg, hPal) )
						InvalidateRect(hDlg,NULL,TRUE);
				return 0;
			}
			break;

		case WM_QUERYNEWPALETTE:
			if ( iDepth <=8 ){
				if(RealizeMyPalette(hDlg, hPal)) {
					InvalidateRect(hDlg,NULL,TRUE);
					return TRUE;
				}
				return 0;
			}
			break;

		case WM_DESTROY:
            DeleteObject (hPal);
			break;

        case WM_INITDIALOG:
#ifndef DEF_FULLVERSION
			// For the standard version, disable the select extended database setting
			{
				HWND hWndSelectExtDB=GetDlgItem( hDlg, IDC_DB_SELECT_EXTENDED );
				if( hWndSelectExtDB )
				{
					char text[255];
					if( GetWindowText( hWndSelectExtDB, text, sizeof(text)-1 ) )
					{
						strcat( text, " (not available)" );
						SetWindowText( hWndSelectExtDB, text );
					}
					EnableWindow( hWndSelectExtDB, FALSE );
				}
			}
#endif // DEF_FULLVERSION
			switch( gPaneNum ){
				case 'repo':
					EnableWindow(GetDlgItem( hDlg, IDC_PREF_FOOTERLABELTRAIL), EditPrefPtr->footer_label);
					break;
			}
			break;

		case WM_SETCURSOR:
			// *******************************************************************
			// Set the tips-window to any matching window-ids in the TOOLTIPDATA
			// *******************************************************************
			{
				UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			}
			break;

        case WM_DROPFILES: {
	        	char	filename[MAXFILENAMESSIZE], *tempbuff;
		        HDROP	hDrop = (HDROP) dwParam;  // handle of internal drop structure  
		        long	len= CONFIG_SIZE;

				DragQueryFile( hDrop, 0, filename, MAXFILENAMESSIZE );
				tempbuff = (char*)malloc( CONFIG_SIZE );
				DoReadFile( tempbuff, filename, CONFIG_SIZE );
				switch( gPaneNum ){
					case 'repo':	if ( hWnd == ghReportHTMLDlg ) SetText( IDC_PREFHTML_HEAD, tempbuff );
									break;
				}
				free( tempbuff );
				DragFinish( hDrop );
			}
			break;
			
		case WM_DRAWITEM:
			{	LPDRAWITEMSTRUCT    lpdis;
				static long			rgb = 0;

				lpdis = (LPDRAWITEMSTRUCT)lParam;    // for convenience

				switch (lpdis->itemAction)
				{
					// handle normal drawing of button, but check if its
					// selected or focus
					case ODA_SELECT:
						if ( lpdis->itemState & ODS_SELECTED ){
							rgb = ChooseRGBColor( hWnd, rgb );

							if ( rgb != -1 ){
								switch ( LOWORD(dwParam) ) {
									case IDC_PREFHTML_RGB1:		EditPrefPtr->RGBtable[1] = rgb; break;
									case IDC_PREFHTML_RGB2:		EditPrefPtr->RGBtable[2] = rgb; break;
									case IDC_PREFHTML_RGB3:		EditPrefPtr->RGBtable[3] = rgb; break;
									case IDC_PREFHTML_RGB4:		EditPrefPtr->RGBtable[4] = rgb; break;
									case IDC_PREFHTML_RGB5:		EditPrefPtr->RGBtable[5] = rgb; break;
									case IDC_PREFHTML_RGB6:		EditPrefPtr->RGBtable[6] = rgb; break;

									case IDC_PREFHTML_RGB7:		EditPrefPtr->RGBtable[7] = rgb; SetHTMLColorsGUI(ghReportHTMLDlg);break;
									case IDC_PREFHTML_RGB8:		EditPrefPtr->RGBtable[8] = rgb; SetHTMLColorsGUI(ghReportHTMLDlg);break;
									case IDC_PREFHTML_RGB9:		EditPrefPtr->RGBtable[9] = rgb; SetHTMLColorsGUI(ghReportHTMLDlg);break;
									case IDC_PREFHTML_RGB10:	EditPrefPtr->RGBtable[10] = rgb; SetHTMLColorsGUI(ghReportHTMLDlg);break;
									case IDC_PREFHTML_RGB11:	EditPrefPtr->RGBtable[11] = rgb; SetHTMLColorsGUI(ghReportHTMLDlg);break;

									case IDC_PREFIMG_RGB1:		EditPrefPtr->multi_rgb[1] = rgb; break;
									case IDC_PREFIMG_RGB2:		EditPrefPtr->multi_rgb[2] = rgb; break;
									case IDC_PREFIMG_RGB3:		EditPrefPtr->multi_rgb[3] = rgb; break;
									case IDC_PREFIMG_RGB4:		EditPrefPtr->multi_rgb[4] = rgb; break;
									case IDC_PREFIMG_RGB5:		EditPrefPtr->multi_rgb[5] = rgb; break;
									case IDC_PREFIMG_RGB6:		EditPrefPtr->multi_rgb[6] = rgb; break;
									case IDC_PREFIMG_RGB7:		EditPrefPtr->multi_rgb[1] = rgb; break;
									case IDC_PREFIMG_RGB8:		EditPrefPtr->multi_rgb[2] = rgb; break;
									case IDC_PREFIMG_RGB9:		EditPrefPtr->multi_rgb[3] = rgb; break;
									case IDC_PREFIMG_RGB10:		EditPrefPtr->multi_rgb[4] = rgb; break;
								}
								SendDlgItemMessage( hDlg, LOWORD(dwParam), WM_ENABLE, TRUE, 0 );

							}
						}
						break;

					case ODA_FOCUS:
					case ODA_DRAWENTIRE:
						switch ( LOWORD(dwParam) ) {
							case IDC_PREFHTML_RGB1:		rgb = EditPrefPtr->RGBtable[1]; break;
							case IDC_PREFHTML_RGB2:		rgb = EditPrefPtr->RGBtable[2]; break;
							case IDC_PREFHTML_RGB3:		rgb = EditPrefPtr->RGBtable[3]; break;
							case IDC_PREFHTML_RGB4:		rgb = EditPrefPtr->RGBtable[4]; break;
							case IDC_PREFHTML_RGB5:		rgb = EditPrefPtr->RGBtable[5]; break;
							case IDC_PREFHTML_RGB6:		rgb = EditPrefPtr->RGBtable[6]; break;

							case IDC_PREFHTML_RGB7:		rgb = EditPrefPtr->RGBtable[7]; break;
							case IDC_PREFHTML_RGB8:		rgb = EditPrefPtr->RGBtable[8]; break;
							case IDC_PREFHTML_RGB9:		rgb = EditPrefPtr->RGBtable[9]; break;
							case IDC_PREFHTML_RGB10:	rgb = EditPrefPtr->RGBtable[10];break;
							case IDC_PREFHTML_RGB11:	rgb = EditPrefPtr->RGBtable[11];break;

							case IDC_PREFIMG_RGB1:		rgb = EditPrefPtr->multi_rgb[0]; break;
							case IDC_PREFIMG_RGB2:		rgb = EditPrefPtr->multi_rgb[1]; break;
							case IDC_PREFIMG_RGB3:		rgb = EditPrefPtr->multi_rgb[2]; break;
							case IDC_PREFIMG_RGB4:		rgb = EditPrefPtr->multi_rgb[3]; break;
							case IDC_PREFIMG_RGB5:		rgb = EditPrefPtr->multi_rgb[4]; break;
							case IDC_PREFIMG_RGB6:		rgb = EditPrefPtr->multi_rgb[5]; break;
							case IDC_PREFIMG_RGB7:		rgb = EditPrefPtr->multi_rgb[6]; break;
							case IDC_PREFIMG_RGB8:		rgb = EditPrefPtr->multi_rgb[7]; break;
							case IDC_PREFIMG_RGB9:		rgb = EditPrefPtr->multi_rgb[8]; break;
							case IDC_PREFIMG_RGB10:		rgb = EditPrefPtr->multi_rgb[9]; break;
						}
						RedrawColoredBlock( hWnd, LOWORD(dwParam), lpdis , rgb );
						return TRUE;
				}  //itemAction
			}
			break;

		case WM_NOTIFY:
			// slider control bar code
			switch(LOWORD(wParam)){
				case IDC_STATEDIT:
					HandleSettingsTreeView( ghStats1Dlg, msg, wParam, lParam );
					break;
				case IDC_CUSTOM_POS:
					{
						NMHDR *pnmhdr = (NMHDR *)lParam;
						long pos, dataNum;

						dataNum = GetPopupNum( IDC_CUSTOM_DATA );
						pos = GetSliderPos( IDC_CUSTOM_POS );
						if ( pos >=0 )
						{
							EditPrefPtr->custom_dataIndex[ dataNum ] = pos;
							if( pos )
							{
								sprintf( szText, "Field position: %d", pos );
							}
							else
							{
								sprintf( szText, "Field position: None" );
							}
							SetText( IDC_CUSTOM_POSTXT, szText );
							ShowCustomConfigInfo( hWnd );
						}
					}
					break;
				default:
					DoPanelTabs( hWnd, LOWORD(wParam) );
					break;
			}
			break;


		case WM_COMMAND:
            switch ( LOWORD(dwParam) ) {
				case IDC_PREFGEN_SPLITDATE:
					{
						if ((HIWORD(dwParam) == CBN_SELCHANGE) &&
							(GetPopupNum( IDC_PREFGEN_SPLITDATE )) )
						{
							// 4 == SORT_DATE!
							SendMessage( GetDlgItem(ghVDomains1Dlg, IDC_VDSORTBY), CB_SETCURSEL , 4 , 0 );
							// Force to use byDate!
							// The user can change this afterwards if they really desire.
						}
					}
					break;
				case IDC_PREF_FOOTERLABEL:
					{
						if ((int)::SendMessage(GetDlgItem( hDlg, IDC_PREF_FOOTERLABEL), BM_GETCHECK, 0, 0L))
						{
							EnableWindow(GetDlgItem( hDlg, IDC_PREF_FOOTERLABELTRAIL), TRUE);
						}
						else
						{
							EnableWindow(GetDlgItem( hDlg, IDC_PREF_FOOTERLABELTRAIL), FALSE);
						}
					}
					break;

				case IDC_PREFGEN_INSERT: {		//IDC_TOKENS
						long i; char txt[MAXFILENAMESSIZE], *p;
						//SendMessage( GetDlgItem( hDlg, IDC_OUTDIR),  EM_SETSEL, 0, 0 );
						i = GetPopupNum( IDC_TOKENS );
						GetPopupText( IDC_TOKENS, i, (LPCTSTR)txt );
						p = mystrchr( txt, '{' );
						if ( p ){
							SendMessage( GetDlgItem( hDlg, IDC_OUTDIR),  EM_REPLACESEL, 0, (long)p );
						}
					}
					break;

				case IDC_SCHEMES:
                    if (HIWORD(wParam) == LBN_SELCHANGE ) {
						long item;
						hDlg = ghReport2Dlg;
						item = SendMessage( GetDlgItem(hDlg, IDC_SCHEMES), LB_GETCURSEL, 0, 0 );
						if ( item != LB_ERR){
							UseColorSchemes( item );
							SendMessage( GetDlgItem( hDlg, IDC_PREFHTML_RGB1),  WM_ENABLE, TRUE, 0 );
							SendMessage( GetDlgItem( hDlg, IDC_PREFHTML_RGB2),  WM_ENABLE, TRUE, 0 );
							SendMessage( GetDlgItem( hDlg, IDC_PREFHTML_RGB3),  WM_ENABLE, TRUE, 0 );
							SendMessage( GetDlgItem( hDlg, IDC_PREFHTML_RGB4),  WM_ENABLE, TRUE, 0 );
							SendMessage( GetDlgItem( hDlg, IDC_PREFHTML_RGB5),  WM_ENABLE, TRUE, 0 );
							SendMessage( GetDlgItem( hDlg, IDC_PREFHTML_RGB6),  WM_ENABLE, TRUE, 0 );
						}
						//SendDlgItemMessage( hWnd, GetDlgItem(hDlg, IDC_PREFHTML_RGB1), BN_PAINT, 0, 0 );
						//OutDebug( "IDC_SCHEMES" );
					}
					break;
				case IDC_CUSTOM_DATA:
                    if (HIWORD(wParam) == CBN_SELENDOK ) {
						long dataNum,pos;

						dataNum = GetPopupNum( IDC_CUSTOM_DATA );
						pos = EditPrefPtr->custom_dataIndex[ dataNum ];
						SetSliderPos( IDC_CUSTOM_POS, pos );
						if( pos )
						{
							sprintf( szText, "Field position: %d", pos );
						}
						else
						{
							sprintf( szText, "Field position: None" );
						}
						SetText( IDC_CUSTOM_POSTXT, szText );
						ShowCustomConfigInfo( hWnd );
					}
					break;

				case IDC_STATDEFAULT:
				case IDC_APPLYALL:
				case IDC_BUTTON_EXPANDALL:
				case IDC_STAT_TABLE:
				case IDC_STAT_GRAPH:
				case IDC_SORTING:
				case IDC_TABSIZE:
				case IDC_STATSTYLES:
				case IDC_STATEDITDESC:
				case IDC_USEDESC:
					HandleSettingsTreeView( hWnd, msg, wParam, lParam );
					break;

				case IDC_VDREPORTBY:
                    if (HIWORD(wParam) == CBN_SELENDOK ) {
						long i;
						i = (short)GetPopupNum( IDC_VDREPORTBY );
						if ( i == 5 )
							Enable( IDC_VDMULTIDIR );
						else
							Disable( IDC_VDMULTIDIR );

					}
					break;
				case IDC_PREFGEN_DATESTYLE:
                    if (HIWORD(wParam) == CBN_SELENDOK ) {
						short	datestyle;

						datestyle = (short)GetPopupNum( IDC_PREFGEN_DATESTYLE );

						if( datestyle ){
							time_t	jd1,jd2;

							DatetypeToDate( datestyle, &jd1, &jd2 );

							DaysDateToString( jd1, szText, GetDateStyle(), '/', 1,1);
							SetText( IDC_DATESTART, szText );

							DaysDateToString( jd2, szText, GetDateStyle(), '/', 1,1);
							SetText( IDC_DATEEND, szText );

							/* disable the date fields */
							Disable( IDC_DATESTART );
							Disable( IDC_DATEEND );
						} else {
							/* enable the date fields */
							Enable( IDC_DATESTART );
							Enable( IDC_DATEEND );
						}
					}
					break;


				case IDC_DATESTART:
				case IDC_DATEEND:
					//if ( !GetPopupNum( IDC_PREFGEN_DATESTYLE ) )
					//	Check_GUIDates( hWnd );
					break;

				case IDC_PREFGEN_OUT: {
						trimLine( EditPrefPtr->outfile );
						strcpy( szText, EditPrefPtr->outfile );

						if ( GetSaveFolderName( szText ) ){
							char *p;
							strcpy( EditPrefPtr->outfile, szText );
							SetText( IDC_OUTDIR, szText );

							if ( p = strrchr( szText, '/' ) ){
								if ( !strchr( p, '.' ) ){
									strcat( szText, "/index.html" );
									FixReportOutput( hWnd );
								}
							}

							SetSaveFolder( szText );
							ShowDiskFree( hDlg, EditPrefPtr->outfile );
						}
					}
					break;

				case IDC_PREFGEN_TYPE:
                    if (HIWORD(wParam) == CBN_SELENDOK ) {
						FixReportOutput( hWnd );
						GetHTMLColors( EditPrefPtr );
						Report_GUItoData();
						Report_DatatoGUI();
					}
					break;
				case IDC_PREF_LANG:
					if ( HIWORD(wParam) == CBN_SELCHANGE )
						LanguageString_GUItoData( ghReport1Dlg, IDC_PREF_LANG, EditPrefPtr->language );
					break;
				case IDC_PREF_PDF_PAGESIZE:
                    if (HIWORD(wParam) == CBN_SELENDOK )
						ChangePDFPageSize();
					break;
				case IDC_PREF_PDF_TABLETEXTNAME:
                    if (HIWORD(wParam) == CBN_SELENDOK )
						ChangeTableTextNameCombo();
					break;
				case IDC_PREF_PDF_SPACING:
                    if (HIWORD(dwParam) == CBN_SELENDOK )
						ChangeSpacingSizeCombo();
					break;

				case IDC_PREF_PDF_ADVANCED:
					if (HIWORD(wParam) == BN_CLICKED)
						DialogBox(hInst, "PREFS_REPORTPDFADVANCED", ghReportPDFDlg, (DLGPROC) PrefsDialogsProcPDFAdvanced);
					break;
				case IDC_PREF_PDF_FONTSIZE:
				case IDC_PREF_PDF_FONTCOLOR:
				case IDC_PREF_PDF_FONTSTYLE:
                    if (HIWORD(wParam) == CBN_SELENDOK )
						ChangeATableTextAttributeCombo( LOWORD(dwParam) );
					break;
				case IDC_PREF_PDF_GRAPHFONT:
                    if (HIWORD(dwParam) == CBN_SELENDOK )
						ChangeGraphFontsNameCombo();
					break;
				case IDC_PREF_PDF_GRAPHFONTSIZE:
                    if (HIWORD(dwParam) == CBN_SELENDOK )
						ChangeGraphFontsSizeCombo();
					break;

				case IDC_PREF_PDF_PAGEWIDTH:
				case IDC_PREF_PDF_PAGEHEIGHT:
				case IDC_PREF_PDF_LEFTMARGIN:
				case IDC_PREF_PDF_RIGHTMARGIN:
				case IDC_PREF_PDF_TOPMARGIN:
				case IDC_PREF_PDF_BOTTOMMARGIN:
				case IDC_PREF_PDF_SPACINGSIZE:
					{
						if (HIWORD(wParam) == EN_KILLFOCUS )
						{
							int ctrlId = LOWORD(dwParam);
							int bufSize = 32;
							char *buf = (char*)malloc( bufSize+1 );
							int num, min, max;
							GetText( ctrlId, buf, bufSize );
							num = atoi( buf );
							switch ( ctrlId )
							{
								case IDC_PREF_PDF_PAGEWIDTH:
									{
										min = PDF_PAGEWIDTH_MIN;
										max = PDF_PAGEWIDTH_MAX;
									}
									break;
								case IDC_PREF_PDF_PAGEHEIGHT:
									{
										min = PDF_PAGEHEIGHT_MIN;
										max = PDF_PAGEHEIGHT_MAX;
									}
									break;
								case IDC_PREF_PDF_LEFTMARGIN:
									{
										min = PDF_LEFTMARGIN_MIN;
										max = PDF_LEFTMARGIN_MAX;
									}
									break;
								case IDC_PREF_PDF_RIGHTMARGIN:
									{
										min = PDF_RIGHTMARGIN_MIN;
										max = PDF_RIGHTMARGIN_MAX;
									}
									break;
								case IDC_PREF_PDF_TOPMARGIN:
									{
										min = PDF_TOPMARGIN_MIN;
										max = PDF_TOPMARGIN_MAX;
									}
									break;
								case IDC_PREF_PDF_BOTTOMMARGIN:
									{
										min = PDF_BOTTOMMARGIN_MIN;
										max = PDF_BOTTOMMARGIN_MAX;
									}
									break;
								case IDC_PREF_PDF_SPACINGSIZE:
									ChangeSpacingSizeEdit();
									return 0;
							}
							if ( num < min )
							{
								sprintf( buf, "%d", min );
								SetText( ctrlId, buf );
							}
							if ( num > max )
							{
								sprintf( buf, "%d", max );
								SetText( ctrlId, buf );
							}
							free( buf );
						}
					}
					break;
				case IDC_PREF_PDF_BROWSEBANNERFILE:
						if (HIWORD(wParam) == BN_CLICKED )
						{
							char initDir[BANNER_FILE_SIZE];
							char filename[BANNER_FILE_SIZE];
							PathFromFullPath( EditPrefPtr->pdfAllSetC.pdfSetC.bannerFile, initDir );
							mystrrchrs( filename, EditPrefPtr->pdfAllSetC.pdfSetC.bannerFile, "\\/" );
							if ( GetLoadDialog( filename, initDir, "*.jpg; *.jpeg", "JPEG File Interchange Format (*.jpg;*.jpeg)\0", "Select an image (Jpeg only) for PDF output" ) )
							{
								mystrcpy( EditPrefPtr->pdfAllSetC.pdfSetC.bannerFile, filename );
								SetText( IDC_PREF_PDF_BANNERFILE, EditPrefPtr->pdfAllSetC.pdfSetC.bannerFile );
							}
						}
						break;
				case IDC_PREF_PDF_TEXTWRAP:
                    if (HIWORD(wParam) == BN_CLICKED )
						ChangeTextWrap();
					break;
				case IDC_CLEARCACHE:
					ClearLookupCacheFile();
					MsgBox_Error( IDS_DNSCLEARED );
					break;

	    		case IDC_PREFVD_SLEEP:
					if ( IsChecked( IDC_PREFVD_SLEEP ) ){
						Enable( IDC_PREFVD_TYPE ); Enable( IDC_PREFVD_NUM ); 
						Enable( IDC_PREFVD_TYPE2 ); Enable( IDC_PREFVD_NUM2 );
					} else {
						Disable( IDC_PREFVD_TYPE ); Disable( IDC_PREFVD_NUM ); 
						Disable( IDC_PREFVD_TYPE2 ); Disable( IDC_PREFVD_NUM2 ); 
					}
					break;

				case IDC_PRE_ON:
					PreProc_GUItoData();
					PreProc_DatatoGUI();
					break;

				case IDC_PREFGEN_USECGI:
				case IDC_PREFGEN_RETAINVAR:
					Analysis_GUItoData();
					Analysis_DatatoGUI();
					break;
				case IDC_PREPROC_BROWSE:
					GetText( IDC_PREPROC_DEST, szText, MAXFILENAMESSIZE );
					GetString( IDS_SELECTFOLDER, tmp, 128 );
					if ( SelectFolder( szText, NULL, tmp ) ){
						SetText( IDC_PREPROC_DEST, szText );
					}
					break;

				case IDC_PP_UPLOADLOG:
				case IDC_PP_UPLOADREPORT:
				case IDC_PP_UPLOADFOLDER:
				case IDC_PP_EMAILON:
					PostProc_GUItoData();
					PostProc_DatatoGUI();
					break;
				case IDC_PP_LOCATION:
					GetText( IDC_PP_LOCATION, szText, MAXFILENAMESSIZE );
					if ( !mystrcmpi( szText, EditPrefPtr->outfile ) ){
						MsgBox_Error( IDS_ERR_POSTPROC );
					}
					break;

				case IDC_PP_BROWSE:
					GetText( IDC_PP_LOCATION, szText, MAXFILENAMESSIZE );
					if ( GetPPLocationName( szText ) ){
						if ( !mystrcmpi( szText, EditPrefPtr->outfile ) ){
							MsgBox_Error( IDS_ERR_POSTPROC );
						} else
							SetText( IDC_PP_LOCATION, szText );
					}
					break;


				case IDC_DB_NONE:
				case IDC_DB_SELECT_COMPACT:
				case IDC_DB_SELECT_EXTENDED:
				case IDC_DB_REPORT:
				case IDC_DB_EXCLUDE:
					Report_GUItoData();
					Report_DatatoGUI();
					break;

				case IDC_DB_BROWSE:
					{
						char szPath[MAXFILENAMESSIZE];
						GetText( IDC_DB_NAME, szText, MAXFILENAMESSIZE-1 );
						PathFromFullPath( szText, szPath );
						
						if( IsChecked( IDC_DB_SELECT_EXTENDED ) &&
							GetSaveDialog( szText, "mydb.fxdb\0", szPath, "Path (*.fxdb)\0*.fxdb\0\0\0", ReturnString( IDS_SELECT_EXTENDED_DATABASE ), "fxdb", 1 )
							||
							IsChecked( IDC_DB_SELECT_COMPACT ) &&
							GetSaveDialog( szText, "mydb.fdb\0", szPath, "Path (*.fdb)\0*.fdb\0\0\0", ReturnString( IDS_SELECT_COMPACT_DATABASE ), "fdb", 1 )
							)
						{
							if ( strlen( szText ) >= MAXFILENAMESSIZE )
								ErrorMsg( "Path is too long, >%s. Choose a different path", MAXFILENAMESSIZE-1 );
							else
							{
								static const char s_message[]="Unable to load database file:\n\n%s\n\n%s";
								CQV5Database::Type v5DBType;

								if( CQV5Database::isV5Database( szText, 0, 0, &v5DBType ) )
								{
									if( v5DBType==CQV5Database::Web && MyPrefStruct.streaming )
									{
										ErrorMsg( s_message, szText, "A database created in web mode cannot be processed in streaming mode!" );
										break;
									}
									
									if( v5DBType==CQV5Database::Streaming && !MyPrefStruct.streaming )
									{
										ErrorMsg( s_message, szText, "A database created in streaming mode cannot be processed in web mode!" );
										break;
									}
								}

								SetText( IDC_DB_NAME, szText );
							}
						}
					}
					break;

				case IDC_DB_DELETE:
					GetText( IDC_DB_NAME, szText, MAXFILENAMESSIZE-1 );
					trimLineWhite( szText );
					if( *szText )
					{
						if( GetFileLength( szText ) )	// ie if file exists
						{
							if( CautionMsg( ReturnString(IDS_CONFIRM_DB_DELETE), szText )==IDYES )
							{
								if( DeleteFile( szText ) )
								{
									NotifyMsg( ReturnString(IDS_DB_DELETED_OK), szText );
								}
								else
								{
									ErrorMsg( ReturnString(IDS_DB_DELETE_FAILED), szText );
								}
							}
						}
						else
						{
							NotifyMsg( ReturnString(IDS_DB_DOES_NOT_EXIST), szText );
						}
					}
					break;

			}
			break;
		
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}


long prefs_names[] = {
	'prep',
	'repo',
	'anal',
	'filt',
	'stat',
#ifdef DEF_FULLVERSION
	'rout',
#endif
	'bill',
#ifdef DEF_FULLVERSION
	'adds',
#endif
	'virt',
	'post',
#ifdef DEF_FULLVERSION
	'cust',
#endif
#ifdef DEF_APP_FIREWALL
	'prox',
#endif
	'DBUG',
	0,0,0 };

/****************************************************************************
* 
*    FUNCTION: NotifyHandler(HWND, UINT, UINT, LONG)
*
*    PURPOSE: This function is the handler for the WM_NOTIFY that is 
*    sent to the parent of the list view window.
*
****************************************************************************/
LRESULT NotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hDlg = hWnd;
	LRESULT pResult=0;

	if (wParam == IDC_PREFS_LISTRECT) {
		LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
		NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
		int itemSel = pNm->iItem;

		switch(pLvdi->hdr.code)
		{
			case LVN_BEGINLABELEDIT:
				return TRUE;
				break;
			case LVN_ITEMCHANGING:
				ShowPrefsPaneX( prefs_names[itemSel] );
				break;
		}
	} else
		pResult = 1;
	
	return pResult;
}

long Init_LangComboBox( HWND hDlg, long langComboId )
{
	WIN32_FIND_DATA fd; 
	HANDLE	hFind;
	int		nNext = 0;
	int 	fFound=FALSE, lastn, count=0;
	char	newPath[512];
	char	*p;
	char langsPath[1024];

	SendMessage( GetDlgItem( hDlg, langComboId), CB_RESETCONTENT, 0, 0);
	AddPopupItem( langComboId, "English" );

	// Check the exe directory first - as this is where users should have their v4.1 "Languages" directory
	GetLangV41File( langsPath, gPath, PATHSEPSTR, "*" ); 
	hFind = FindFirstFile( langsPath, &fd);
	if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;

	if ( !fFound ) // Now check for the v4.1 "Languages" directory which is located back a directory - this where we have our Languages directory
	{
		GetLangV41File( langsPath, gPath, PATHBACKDIR, "*" ); 
		hFind = FindFirstFile( langsPath, &fd); 
		if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;
	}
		
	// Check the exe directory first - as this is where users should have their v3.7 "lang" directory
	if ( !fFound ) 
	{
		GetLangV37File( langsPath, gPath, PATHSEPSTR, "*" ); 
		hFind = FindFirstFile( langsPath, &fd);
		if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;
	}

	if ( !fFound ) // Now check for a lang directory which is located back a directory - this where we have our v3.7 "lang" directory
	{
		GetLangV37File( langsPath, gPath, PATHBACKDIR, "*" ); 
		hFind = FindFirstFile( langsPath, &fd); 
		if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;
	}
		
	if ( !fFound ) // Try the current directory - basically, a last resort...
	{
		GetLangV41File( langsPath, 0, 0, "*" ); 
		hFind = FindFirstFile( langsPath, &fd); 
		if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;
	}

	while ( fFound )  {
		if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ){
			FileFromPath( fd.cFileName, newPath );
			p = mystrchr( newPath, '.' );
			*p = 0;
			if ( mystrcmpi( newPath, "English" ) )	// only add if its not english
				AddPopupItem( langComboId, newPath );
			lastn = nNext;
		}
		fFound = FindNextFile( (HANDLE)hFind, &fd );
	}
	SetPopupNum( langComboId, 0 );
	FindClose(hFind);
	return nNext;
} 



void InitNotifyCombos( HWND hWnd )
{
	if ( hWnd ){
		InitDropBox( hWnd, StatListStr, IDC_NOTIFY_DB );
		InitDropBox( hWnd, StatFieldStr, IDC_NOTIFY_FIELD );
		InitDropBox( hWnd, LogicStr, IDC_NOTIFY_LOGIC );
	}
}


void InitAllCombos( HWND hWnd )
{
	InitNotifyCombos( hWnd );
	Init_LangComboBox( ghReport1Dlg, IDC_PREF_LANG );

	Init_ComboBox( ghReport1Dlg, IDC_PREFGEN_TYPE, IDS_REPORTFORMAT );
	Init_ComboBox( ghReport1Dlg, IDC_PREFGEN_GTYPE, IDS_IMAGETYPE );
	Init_ComboBox( ghReport1Dlg, IDC_TOKENS, IDS_DATETOKENS );

	Init_ComboBox( ghReportPDFDlg, IDC_PREF_PDF_PAGESIZE, IDS_REPORT_PDFPAGESIZES );
	Init_ComboBox( ghReportPDFDlg, IDC_PREF_PDF_PAGENUMBERINGPOS, IDS_REPORT_PDFPAGENUMBERINGPOS );
	Init_ComboBox( ghReportPDFDlg, IDC_PREF_PDF_SPACING, IDS_REPORT_PDFSPACING );
	Init_ComboBox( ghReportPDFDlg, IDC_PREF_PDF_TABLETEXTNAME, IDS_REPORT_PDFTABLETEXTNAME );
	Init_ComboBox( ghReportPDFDlg, IDC_PREF_PDF_TRUNCATETEXT, IDS_REPORT_PDFTRUNCATETEXT );
	Init_ComboBox( ghReportPDFDlg, IDC_PREF_PDF_FONT, IDS_REPORT_PDFFONT );
	Init_ComboBox( ghReportPDFDlg, IDC_PREF_PDF_FONTSTYLE, IDS_REPORT_PDFFONTSTYLES );
	Init_ColorComboBox( ghReportPDFDlg, IDC_PREF_PDF_FONTCOLOR );
	Init_ComboBox( ghReportPDFDlg, IDC_PREF_PDF_GRAPHFONT, IDS_REPORT_PDF_GRAPHFONT );

	Init_ComboBox( ghAnalysis1Dlg, IDC_PREFGEN_DATESTYLE, IDS_DATES );
	Init_ComboBox( ghAnalysis1Dlg, IDC_DATEOVERIDE, IDS_DATEFORMAT );
	Init_ComboBox( ghAnalysis1Dlg, IDC_PREFGEN_SPLITDATE, IDS_PREFGEN_SPLITDATE );
	Init_ComboBox( ghStats1Dlg, IDC_TABSIZE, IDS_TABSIZE );

	Init_ComboBox( ghBillingChargesDlg, IDC_COMBO_BILLINGCUSTOMERFIXEDTYPE,		IDS_BILLINGCHARGES_FIXED);
	Init_ComboBox( ghBillingChargesDlg, IDC_COMBO_BILLINGCUSTOMEREXCESSTYPE,	IDS_BILLINGCHARGES_EXCESS);
	Init_ComboBox( ghBillingSetupDlg,	IDC_COMBO_BILLINGPERIOD,				IDS_BILLINGSETUP_PERIOD);
	Init_ComboBox( ghBillingSetupDlg,	IDC_COMBO_BILLINGFORMAT,				IDS_BILLINGSETUP_OUTPUTFORMAT);
	
	Init_ComboBox( ghVDomains1Dlg, IDC_VDREPORTBY, IDS_VDREPORTBY );
	Init_ComboBox( ghVDomains1Dlg, IDC_VDSORTBY, IDS_VDSORTBY );
	Init_ComboBox( ghVDomains3Dlg, IDC_PREFVD_TYPE, IDS_TIMER );
	Init_ComboBox( ghCustom1Dlg, IDC_CUSTOM_DATE, IDS_DATEFMT );
	Init_ComboBox( ghCustom1Dlg, IDC_CUSTOM_TIME, IDS_TIMEFMT );
	Init_ComboBox( ghCustom1Dlg, IDC_CUSTOM_DATA, IDS_FIELDFMT );
	Init_ComboBox( ghStats1Dlg, IDC_SORTING, IDS_SORTING );
	
	Init_ComboBox( ghFiltersInDlg, IDC_FILTERIN1, IDS_FILTER1 );
	Init_ComboBox( ghFiltersInDlg, IDC_FILTERIN2, IDS_FILTER2 );
	Init_ComboBox( ghFiltersInDlg, IDC_FILTERIN3, IDS_FILTER3 );

	Init_ComboBox( ghStats1Dlg, IDC_STATFILTER, IDS_FILTER1 );
	Init_ComboBox( ghStats1Dlg, IDC_STATFILTER2, IDS_FILTER3 );
	Init_ComboBox( ghStats1Dlg, IDC_STATSTYLES, IDS_STATSTYLES );
	HWND hDlg = ghStats1Dlg;
	SetPopupNum( IDC_STATSTYLES, EditPrefPtr->stat_style-1 );
	
}



void InitAllListViews( HWND hWnd )
{
	if ( (hWndListView = InitSettingsListView( hWnd )) == NULL )
		UserMsg( "Icon List not created!" );

	if ( (hWndEditSettingsView = InitSettingsTreeView( ghStats1Dlg )) == NULL )
		UserMsg( "Settings view not created!" );

	if ( (hWndExtSettingsView = InitExtensionsTreeView( ghAnalysis3Dlg )) == NULL )
		UserMsg( "Ext Settings view not created!" );
}




void InitAllTabControls( HWND hWnd )
{
	ghPreProcTabDlg = Init_TabControl( ghPreProcDlg, IDC_TAB_PREPROC, IDS_TABPREPROC );
	ghReportTabDlg = Init_TabControl( ghReportDlg, IDC_TAB_REPORT, IDS_TABREPORT );
	ghAnalysisTabDlg = Init_TabControl( ghAnalysisDlg, IDC_TAB_ANALYSIS, IDS_TABANALYSIS );
	ghFiltersTabDlg = Init_TabControl( ghFiltersDlg, IDC_TAB_FILTER, IDS_TABFILTERS );

	if ( IsDebugMode() )
		ghStatsTabDlg = Init_TabControl( ghStatsDlg, IDC_TAB_STATS, IDS_TABSTATSDEBUG );
	else {
#ifdef DEF_FULLVERSION
		ghStatsTabDlg = Init_TabControl( ghStatsDlg, IDC_TAB_STATS, IDS_TABSTATSENT );
#else
		ghStatsTabDlg = Init_TabControl( ghStatsDlg, IDC_TAB_STATS, IDS_TABSTATS );
#endif
	}

#ifdef DEF_FULLVERSION
	ghRoutesTabDlg = Init_TabControl( ghRoutesDlg, IDC_TAB_ROUTES, IDS_TABROUTES );
#endif

#ifdef DEF_FULLVERSION
	ghVDomainsTabDlg = Init_TabControl( ghVDomainsDlg, IDC_TAB_VDOMAINS, IDS_TABVHOSTS );
#else
	ghVDomainsTabDlg = Init_TabControl( ghVDomainsDlg, IDC_TAB_VDOMAINS, IDS_TABVHOSTS2 );
#endif

	ghPostProcTabDlg = Init_TabControl( ghPostProcDlg, IDC_TAB_POSTPROC, IDS_TABPOSTPROC );
	ghBillingTabDlg	 = Init_TabControl( ghBillingDlg, IDC_TAB_BILLING, IDS_TABBILLING );
	ghAddsTabDlg = Init_TabControl( ghAddsDlg, IDC_TAB_ADDS, IDS_TABADDS );

#ifdef DEF_APP_FIREWALL
	ghProxyTabDlg = Init_TabControl( ghProxyDlg, IDC_TAB_PROXY, IDS_TABPROXY );
#endif

#if DEF_DEBUG 
	ghCustomTabDlg = Init_TabControl( ghCustomDlg, IDC_TAB_CUSTOM, IDS_TABCUSTOM2 );
	ghDebugTabDlg = Init_TabControl( ghDebugDlg, IDC_TAB_DEBUG, IDS_SET );
#else
	ghCustomTabDlg = Init_TabControl( ghCustomDlg, IDC_TAB_CUSTOM, IDS_TABCUSTOM );
#endif
}














//
//  FUNCTION: PrefsDlgProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for "PrefsDlgProc" dialog box
// 		This version allows greater flexibility over the contents of the 'PrefsDlgProc' box,
// 		by pulling out values from the 'Version' resource.
//
//  MESSAGES:
//
//	WM_INITDIALOG - initialize dialog box
//	WM_COMMAND    - Input received
//
//

static long	first_pane = 'repo';

LRESULT CALLBACK PrefsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hDlg = hWnd;
	long	pane;

	switch (message) {
		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_ERASEBKGND:
			if ( ForceBackgroundGradient( hWnd, message, wParam, lParam ) )
				return 1;
			break;

        case WM_PAINT: {
				HDC hdc;
                PAINTSTRUCT ps;
                RECT rc;

                GetClientRect(hWnd, &rc);
				hdc = BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
			break;

		case WM_CLOSE:
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;

        case WM_DESTROY:
			if ( ghFiltersDlg )
			{
				DestroyWindow(ghPreProcDlg);
				DestroyWindow(ghPreProcTabDlg);
				DestroyWindow(ghPreProc1Dlg);
				DestroyWindow(ghPreProc2Dlg);
				DestroyWindow(ghReportDlg);
				DestroyWindow(ghReport1Dlg);
				DestroyWindow(ghReport2Dlg);
				DestroyWindow(ghReportHTMLDlg);
				DestroyWindow(ghReportPDFDlg);
				//DestroyWindow(ghReport4Dlg);
				DestroyWindow(ghAnalysisDlg);
				DestroyWindow(ghAnalysis1Dlg);
				DestroyWindow(ghAnalysis2Dlg);
				DestroyWindow(ghAnalysis3Dlg);
				DestroyWindow(ghAnalysis4Dlg);
				DestroyWindow(ghFiltersDlg);
				DestroyWindow(ghFiltersInDlg);
				//DestroyWindow(ghFiltersOutDlg);
				DestroyWindow(ghStatsDlg);
				DestroyWindow(ghStatsTabDlg);
				DestroyWindow(ghStats1Dlg);
				DestroyWindow(ghStats2Dlg);
				DestroyWindow(ghStats3Dlg);
				DestroyWindow(ghStats4Dlg);
				DestroyWindow(ghStats5Dlg);

				DestroyWindow(ghRoutesDlg);
				DestroyWindow(ghRoutesTabDlg);
				DestroyWindow(ghRoutesKeyVisitorDlg);
				DestroyWindow(ghRoutesRouteToKeyPagesDlg);
				DestroyWindow(ghRoutesRouteFromKeyPagesDlg);

				DestroyWindow(ghBillingDlg);
				DestroyWindow(ghBillingTabDlg);
				DestroyWindow(ghBillingSetupDlg);
				DestroyWindow(ghBillingChargesDlg);

				DestroyWindow(ghAddsDlg);
				DestroyWindow(ghAddsTabDlg);
				DestroyWindow(ghAddsBannersDlg);
				DestroyWindow(ghAddsCampDlg);
				DestroyWindow(ghPostProcDlg);
				DestroyWindow(ghPostProc1Dlg);
				DestroyWindow(ghPostProc2Dlg);
				DestroyWindow(ghPostProc3Dlg);
				DestroyWindow(ghCustomDlg);
				DestroyWindow(ghVDomainsDlg);
				DestroyWindow(ghVDomainsTabDlg);
				DestroyWindow(ghVDomains1Dlg);
				DestroyWindow(ghVDomains2Dlg);
				DestroyWindow(ghVDomains3Dlg);
				DestroyWindow(ghVDomains4Dlg);

				DestroyWindow(ghProxyDlg);
				DestroyWindow(ghProxyTabDlg);
				DestroyWindow(ghProxyIPDlg);

#if DEF_DEBUG		
				DestroyWindow(ghDebugDlg);
				DestroyWindow(ghDebugTabDlg);
				DestroyWindow(ghDebug1Dlg);
				DestroyWindow(ghRemoteDlg);
#endif
			}
            break;

        case WM_INITDIALOG:
			ghPrefsDlg = hWnd;
			pane = gPaneNum;
			// open prefs panes
		    gPaneNum = 'prep';
			ghPreProcDlg = CreateDialog(hInst, "PREFS_PREPROC", hWnd, (DLGPROC) PrefsDialogsProc);
			ghPreProc1Dlg = CreateDialog(hInst, "PREFS_PREDOWNLOAD", ghPreProcDlg, (DLGPROC) PrefsDialogsProc);
			ghPreProc2Dlg = CreateDialog(hInst, "PREFS_PREOPTIONS", ghPreProcDlg, (DLGPROC) PrefsDialogsProc);
			
			gPaneNum = 'repo';
			ghReportDlg = CreateDialog(hInst, "PREFS_REPORT", hWnd, (DLGPROC) PrefsDialogsProc);
			ghReport1Dlg = CreateDialog(hInst, "PREFS_REPORTOUTPUT", ghReportDlg, (DLGPROC) PrefsDialogsProc);
			ghReport2Dlg = CreateDialog(hInst, "PREFS_REPORTPAGE", ghReportDlg, (DLGPROC) PrefsDialogsProc);
			ghReportHTMLDlg = CreateDialog(hInst, "PREFS_REPORTHTMLHEAD", ghReportDlg, (DLGPROC) PrefsDialogsProc);
			ghReportPDFDlg = CreateDialog(hInst, "PREFS_REPORTPDF2", ghReportDlg, (DLGPROC) PrefsDialogsProc);
			//ghReport4Dlg = CreateDialog(hInst, "PREFS_REPORTHTMLFOOT", ghReportDlg, (DLGPROC) PrefsDialogsProc);
			//hfixedFont = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,FIXED_PITCH, "Terminal");
			//SendMessage( GetDlgItem(ghReport1Dlg, IDC_TOKENS), WM_SETFONT, (UINT)(hfixedFont), TRUE);


		    gPaneNum = 'anal';
			ghAnalysisDlg = CreateDialog(hInst, "PREFS_ANALYSIS", hWnd, (DLGPROC) PrefsDialogsProc);
			ghAnalysis1Dlg = CreateDialog(hInst, "PREFS_ANALYSISOPT", ghAnalysisDlg, (DLGPROC) PrefsDialogsProc);
			ghAnalysis2Dlg = CreateDialog(hInst, "PREFS_ANALYSISNET", ghAnalysisDlg, (DLGPROC) PrefsDialogsProc);
			ghAnalysis3Dlg = CreateDialog(hInst, "PREFS_ANALYSISEXT", ghAnalysisDlg, (DLGPROC) HandleExtensionsTreeView);
			ghAnalysis4Dlg = CreateDialog(hInst, "PREFS_ANALYSISDYN", ghAnalysisDlg, (DLGPROC) PrefsDialogsProc);

							
		    gPaneNum = 'filt';
			ghFiltersDlg = CreateDialog(hInst, "PREFS_FILTERS", hWnd, (DLGPROC) PrefsDialogsProc);
			ghFiltersInDlg = CreateDialog(hInst, "PREFS_FILTERINPUT", ghFiltersDlg, (DLGPROC) FilterInListHandler);
//			ghFiltersOutDlg = CreateDialog(hInst, "PREFS_FILTEROUTPUT", ghFiltersDlg, (DLGPROC) FilterOutListHandler);

			gPaneNum = 'stat';	
			ghStatsDlg = CreateDialog(hInst, "PREFS_STATS", hWnd, (DLGPROC) PrefsDialogsProc);
			ghStats1Dlg = CreateDialog(hInst, "PREFS_STATSEDIT", ghStatsDlg, (DLGPROC) PrefsDialogsProc);
			ghStats2Dlg = CreateDialog(hInst, "PREFS_STATSGLOBAL", ghStatsDlg, (DLGPROC) PrefsDialogsProc);
			ghStats3Dlg = CreateDialog(hInst, "PREFS_STATSDOMAINS", ghStatsDlg, (DLGPROC) StatsOrgListHandler);

			
#ifdef	DEF_FULLVERSION
			ghStats4Dlg = CreateDialog(hInst, "PREFS_STATSDIRMAP", ghStatsDlg, (DLGPROC) StatsGroupListHandler);
			ghStats5Dlg = CreateDialog(hInst, "PREFS_STATSREFMAP", ghStatsDlg, (DLGPROC) StatsReferralListHandler);
#endif

#ifdef DEF_FULLVERSION
			gPaneNum = 'rout';
			ghRoutesDlg = CreateDialog(hInst, "PREFS_ROUTES", hWnd, (DLGPROC) PrefsDialogsProc);
			ghRoutesKeyVisitorDlg = CreateDialog(hInst, "PREFS_STATSKEYVISITORS", ghRoutesDlg, (DLGPROC) KeyVisitorListHandler);
			ghRoutesRouteToKeyPagesDlg	= CreateDialog(hInst, "PREFS_REPORTROUTE2KEYPAGES",		ghRoutesDlg,	(DLGPROC) RouteToKeyPagesHandler);
			ghRoutesRouteFromKeyPagesDlg = CreateDialog(hInst, "PREFS_REPORTROUTEFROMKEYPAGES",	ghRoutesDlg,	(DLGPROC) RouteFromKeyPagesHandler);
#endif

			// Billing Dialogs.
			gPaneNum = 'bill';	
			ghBillingDlg		= CreateDialog(hInst, "PREFS_BILLING",			hWnd,			(DLGPROC)PrefsDialogsProc);
			ghBillingChargesDlg	= CreateDialog(hInst, "PREFS_BILLINGCHARGES",	ghBillingDlg,	(DLGPROC)BillingChargesHandler);
			ghBillingSetupDlg	= CreateDialog(hInst, "PREFS_BILLINGSETUP",		ghBillingDlg,	(DLGPROC)BillingSetupHandler);


#ifdef	DEF_FULLVERSION
			gPaneNum = 'adds';	
			ghAddsDlg = CreateDialog(hInst, "PREFS_ADDS", hWnd, (DLGPROC) PrefsDialogsProc);
			ghAddsBannersDlg = CreateDialog(hInst, "PREFS_ADDSBANNERS", ghAddsDlg, (DLGPROC) StatsAddListHandler);
			ghAddsCampDlg = CreateDialog(hInst, "PREFS_ADDSCAMPAIGNS", ghAddsDlg, (DLGPROC) StatsAddCampListHandler);
#endif

			gPaneNum = 'virt';
			ghVDomainsDlg = CreateDialog(hInst, "PREFS_VDOMAINS", hWnd, (DLGPROC) PrefsDialogsProc);
			ghVDomains1Dlg = CreateDialog(hInst, "PREFS_VDOMAINSOPT", ghVDomainsDlg, (DLGPROC) PrefsDialogsProc);
			ghVDomains2Dlg = CreateDialog(hInst, "PREFS_VDOMAINSMAP", ghVDomainsDlg, (DLGPROC) VhostMapListHandler);
			ghVDomains3Dlg = CreateDialog(hInst, "PREFS_VDOMAINSPATH", ghVDomainsDlg, (DLGPROC) EditPathProc);
			ghVDomains4Dlg = CreateDialog(hInst, "PREFS_VDOMAINSCLUSTER", ghVDomainsDlg, (DLGPROC) ClusterListHandler);

			gPaneNum = 'post';
			ghPostProcDlg = CreateDialog(hInst, "PREFS_POSTPROC", hWnd, (DLGPROC) PrefsDialogsProc);
			ghPostProc1Dlg = CreateDialog(hInst, "PREFS_POSTCOMPRESS", ghPostProcDlg, (DLGPROC) PrefsDialogsProc);
			ghPostProc2Dlg = CreateDialog(hInst, "PREFS_POSTUPLOAD", ghPostProcDlg, (DLGPROC) PrefsDialogsProc);
			ghPostProc3Dlg = CreateDialog(hInst, "PREFS_POSTEMAIL", ghPostProcDlg, (DLGPROC) PrefsDialogsProc);

#ifdef	DEF_FULLVERSION
			gPaneNum = 'cust';
			ghCustomDlg = CreateDialog(hInst, "PREFS_CUSTOM", hWnd, (DLGPROC) PrefsDialogsProc);
			ghCustom1Dlg = CreateDialog(hInst, "PREFS_CUSTOMFORMAT", ghCustomDlg, (DLGPROC) PrefsDialogsProc);

			gPaneNum = 'prox';
			ghProxyDlg = CreateDialog(hInst, "PREFS_PROXY", hWnd, (DLGPROC) PrefsDialogsProc);
			ghProxyIPDlg = CreateDialog(hInst, "PREFS_PROXY_IP", ghProxyDlg, (DLGPROC) ProxyListHandler);

#endif

		    gPaneNum = pane;

			InitAllTabControls( hWnd );
			InitAllCombos( hWnd );
			SetPopupNum( IDC_FILTERIN1, 1 );
			InitAllListViews( hWnd );
			InitColorSchemes();
			DisableStatsSettings( ghStats1Dlg );

			PrefsIntoGUI();			// set the GUI to values from data structs

			ShowPrefsPaneX( gPaneNum );

			if ( prefsTitle )
				SetWindowText( hWnd, prefsTitle );

			Disable( IDC_PREFGEN_FLUSH );
			Disable( IDC_STATSTYLES );
			//Enable( IDC_STATSTYLES );

#ifdef	DEF_DEBUG
			long lWindowLong;
			lWindowLong =	::GetWindowLong(::GetDlgItem(hWnd, IDC_EDIT_CONTROLID), GWL_STYLE);
			lWindowLong |=	WS_VISIBLE;
			::SetWindowLong(::GetDlgItem(hWnd, IDC_EDIT_CONTROLID), GWL_STYLE, lWindowLong);

			lWindowLong =	::GetWindowLong(::GetDlgItem(hWnd, IDC_BUTTON_DEBUG_RELOADXML), GWL_STYLE);
			lWindowLong |=	WS_VISIBLE;
			::SetWindowLong(::GetDlgItem(hWnd, IDC_BUTTON_DEBUG_RELOADXML), GWL_STYLE, lWindowLong);
#endif

			return (TRUE);
			break;

		case WM_NOTIFY:
			return( NotifyHandler(hWnd, message, wParam, lParam));
			break;

        case WM_COMMAND:
            switch (wParam) {
				case IDC_BUTTON_DEBUG_RELOADXML:
					{
						if (sg_hwndToolTip)
						{
							::DestroyWindow(sg_hwndToolTip);
							sg_hwndToolTip = NULL;
							UpdateToolBar(hWnd, IDC_BUTTON_DEBUG_RELOADXML);
						}
					}
					break;

				case IDC_PREFDEFAULT:
					{
						char *lang = (char*)malloc( mystrlen( EditPrefPtr->language ) + 1 );
						mystrcpy( lang, EditPrefPtr->language );
						CreateDefaults( EditPrefPtr, 1 );
						mystrcpy( EditPrefPtr->language, lang );
						free( lang );
						InitLanguage( EditPrefPtr->language, 0 );
						TopLevelDomainStrings( EditPrefPtr );
						PrefsIntoGUI();
					}
					break;

				case IDC_PREFREVERT:
					CopyPrefs( EditPrefPtr, OrigPrefPtr );
					PrefsIntoGUI();
					break;

				case IDC_PREFOK:
					gSaved = FALSE;
					ChangeMainWindowTitle();
					prefsReturn = TRUE;

					PrefsFromGUI();
					GetDataExtensionsTreeView();

					EndDialog(hWnd, TRUE);
					return (TRUE);
					break;

				case IDC_PREFCANCEL:
					prefsReturn = FALSE;
					RemoveEditPath( &EditPrefPtr->EditPaths );
					EndDialog(hWnd, TRUE);
					return (TRUE);
					break;
            }
            break;
	}

    return FALSE;
}


int ShowPrefsDialog( long pane )
{
	if ( pane ){
		gPaneNum = prefs_names[pane-1];
	}
	if ( !gProcessingSchedule ){
		EditPrefPtr = &SavedPrefStruct;
		OrigPrefPtr = &MyPrefStruct;
		CopyPrefs( EditPrefPtr, OrigPrefPtr );

		DialogBox(hInst, "CONFIGPREFS", hwndParent, (DLGPROC)PrefsProc);
		if ( prefsReturn )
		{
			CopyPrefs( OrigPrefPtr, EditPrefPtr );
			RemoveEditPath( &EditPrefPtr->EditPaths );
			InitLanguage( EditPrefPtr->language, 0 );
			TopLevelDomainStrings( EditPrefPtr );
		}
		prefsTitle = NULL;
	} else
		StatusSetID( IDS_CANNOTEDIT );

	return prefsReturn;
}

// NOTE: do not use 'true', but use 'TRUE'

int ShowPrefsDialog2( char *title )
{
	char szText[MAXFILENAMESSIZE];
	sprintf( szText, "Settings - %s", title );
	StatusSet( szText );
	prefsTitle = title;

	if ( !gProcessingSchedule ){
		if ( DoReadPrefsFile( title, &SchedulePrefStruct ) ){		// returns TRUE for read file

			EditPrefPtr = &SavedPrefStruct;
			OrigPrefPtr = &SchedulePrefStruct;
			CopyPrefs( EditPrefPtr, OrigPrefPtr );

			DialogBox(hInst, "CONFIGPREFS", hwndParent, (DLGPROC)PrefsProc);
			if ( prefsReturn )
			{
				CopyPrefs( OrigPrefPtr, EditPrefPtr );
				RemoveEditPath( &EditPrefPtr->EditPaths );
			}

			prefsTitle = NULL;
			if (prefsReturn)
			{
				if ( DoSavePrefsFile( title, &SchedulePrefStruct ) ){	// returns TRUE for error in saving or canceled
					MsgBox_Error( IDS_ERR_CANTSAVE, title );
				} else {
					StatusSetID( IDS_SETTINGSSAVED, title );
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
