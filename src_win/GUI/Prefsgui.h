// PREFSGUI.H

#ifndef PREFSGUI_H
#define PREFSGUI_H

#ifdef __cplusplus
extern "C" {
#endif

// constants                            
#define ID_LISTVIEW     1000


#define NUM_COLUMNS			1
#define LG_BITMAP_WIDTH		32
#define LG_BITMAP_HEIGHT	32
#define MAX_ITEMLEN			64


// Function prototypes

HWND CreateListView(HWND,long);
LRESULT NotifyHandler(HWND, UINT, WPARAM, LPARAM);
void InitTimerComboBox( HWND hWnd, long id );
int ShowPrefsDialog( long );

void ShowPrefsPaneX( long paneNum );
LRESULT ServerHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LONG APIENTRY PrefsDialogsProc(HWND hWnd, UINT msg, DWORD dwParam, LONG lParam);
LRESULT CALLBACK PrefsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

long Init_StringArray( char *commalist, char **array, int arraySize );


void FilterDelCurrent( HWND hWnd, long filterID  );

LRESULT FilterInListHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void FilterFileAdd( HWND hWnd );

LRESULT ClientFilterListHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void FilterClientAdd( HWND hWnd );

LRESULT SourceFilterListHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void FilterSourceAdd( HWND hWnd );


long GetStatListStrIdx( char *txt );
long GetStatFieldtStrIdx( char *txt );
long GetLogicStrIdx( char *txt );

int ShowPrefsDialog2( char *title );

void Init_lReportTIP();
void Init_lAnalysisTIP();
void Init_lStats1TIP();
void Init_lVDomainsTIP();
void Init_lPostProcTIP();
void Init_lCustomTIP();

long Init_ComboBox( HWND hDlg, long comboid, long stringid );
long Init_ComboBoxClearAndSelect( HWND hDlg, long comboid, char *data, int sel );

HWND InitGenericListView ( HWND hWndParent, long controlid, long stringid, long itemNum, char* string );




#ifdef __cplusplus
}

#include "config_struct.h"

void	UpdateToolBar(HWND hWnd, long lControlId);
void	SetBillingSetupDetails(void);


/*class ACSettingsDialog
{
public:
	App_config *SchedulePrefStruct;
	ACSettingsDialog( App_config *SchedulePrefStructP ) { SchedulePrefStruct = SchedulePrefStruct; }
	LRESULT EditpathNotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);;
	HWND InitGenericListView ( HWND hWndParent, long controlid, long stringid, long itemNum );
	void EditPathToGUI( HWND hDlg, int number );
	LRESULT CALLBACK EditPathNotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK EditPathProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT APIENTRY TabSubclassProc(    HWND hwnd,     UINT uMsg,     WPARAM wParam,     LPARAM lParam); 
	void OverideTabControl( HWND w );
	HWND WINAPI DoCreateTabControl( HWND hwndParent, char **text, long count, long id );
	HWND InitSettingsListView (HWND hWndParent );
	long TreeView_GetItemParam(HWND hwndTreeView, HTREEITEM hItem);
	long TreeView_GetItemText(HWND hwndTreeView, HTREEITEM hItem, char *text );
	BOOL MyTreeView_GetCheckState(HWND hwndTreeView, HTREEITEM hItem);
	BOOL MyTreeView_SetCheckState(HWND hwndTreeView, HTREEITEM hItem, BOOL fCheck);
	BOOL TreeView_SetItemText(HWND hwndTreeView, HTREEITEM hItem, char *text );
	void SettingsDatatoGUI( HWND hWnd, long data );
	HWND InitSettingsTreeView (HWND hWndParent );                                  
	LRESULT HandleStatEditProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleSettingsTreeView( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND InitExtensionsTreeView (HWND hWndParent );
	void GetDataExtensionsTreeView( void );
	LRESULT HandleExtensionsTreeView( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Check_OutputSuffix( HWND hDlg, char *suff );
	void	Check_GUIDates( HWND hDlg );
	int CheckWarning( void );
	void	PreProc_DatatoGUI( void );
	void	PreProc_GUItoData( void );
	void	Report_DatatoGUI( void );
	void	Report_GUItoData( void );
	void	Analysis_DatatoGUI( void );
	void	Analysis_GUItoData( void );
	float GetGUIFloat( HWND hDlg, long id );
	void	Stats_GUItoData( void );
	void	Stats_DatatoGUI( void );
	void	VDomains_GUItoData( void );
	void	VDomains_DatatoGUI( void );
	void	PostProc_GUItoData( void );
	void	PostProc_DatatoGUI( void );
	void	PrefsCustom_GUItoData( HWND hDlg );
	void	PrefsCustom_DatatoGUI( HWND hDlg );
	void	PrefsNotify_GUItoData( HWND hDlg );
	void	PrefsNotify_DatatoGUI( HWND hDlg );
	void	RemoteServer_GUItoData( HWND hDlg );
	void	RemoteServer_DatatoGUI( HWND hDlg );
	void ShowOneWindow( HWND *htmlWnds, long iPage, long iMax );
	void DoPanelTabs( HWND hWnd, short tab );
	void ShowPrefsPaneX( long paneNum );
	void DumpHex( char *txt, void *ptr, long len );
	void FilterDelCurrent( HWND hDlg, long filterID  );
	void FilterAddItem( HWND hDlg, long nameID, long listID );
	void FilterChangeItem( HWND hDlg, long nameID, long listID );
	void InitColorSchemes( void );
	void UseColorSchemes( long item );
	void RouterAddrAdd( HWND hDlg );
	void FilterDataAdd( long *total, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE], char *data );
	long DataListGUItoData( HWND hDlg, long id, long total, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE] );
	long DataListDatatoGUI( HWND hDlg,long id, long total, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE] );
	long SetFilterListData( long num, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE], char *ftxt );
	long GetFilterListData( long num, long item,  char filter[MAX_FILTERUNITS][MAX_FILTERSIZE], char *out );
	LRESULT FilterInListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT RouterListHandler( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT StatsOrgListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT StatsGroupListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT StatsAddListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT StatsAddCampListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT VhostMapListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT NotifyListHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void InitComboBox( HWND hDlg, char	mycombotxt[][32], long id, long num );
	void InitDropBox( HWND hDlg, char	*mycombotxt[], long id );
	HWND Init_TabControl( HWND hDlg, long tabid, long stringid );
	void InitNotifyCombos( HWND hWnd );
	void ShowCustomConfigInfo( HWND hDlg );
	void RedrawColoredBlock( HWND hWnd, long id, LPDRAWITEMSTRUCT lpdis , long rgb);
	long SetHTMLColors( void );
	long GetHTMLColor( char *p );
	void GetHTMLColors( void );
	LONG APIENTRY PrefsDialogsProc( HWND hWnd, UINT msg, DWORD dwParam, LONG lParam );
	BOOL GetPPLocationName( char *putNamehere );
	LRESULT NotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void PrefsDatatoGUI ( void );
	LRESULT CALLBACK PrefsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};*/

#endif

#endif // PREFSGUI_H
