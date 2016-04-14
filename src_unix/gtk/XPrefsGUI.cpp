#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "gtkwin.h"

#include "XSchedule.h"
#include "images/images.c"
#include "resourceid.h"
#include "resource_strings.h"

#include "datetime.h"
#include "schedule.h"
#include "editpath.h"

#include "config.h"
#include "schedule.h"
#include "LogFileHistory.h"
#include "PrefsIO.h"
#include "serialReg.h"
#include "engine_process.h"
#include "EngineStatus.h"
#include "Utilities.h"



typedef void * HWND;

struct	App_config			*EditPrefPtr,
									*OrigPrefPtr;
static struct	App_config	SavedPrefStruct;			// backed up ones, for revert
static GdkPixmap	*gIconPixmaps[10];


long InitGenericListView( char *itemname, char *items );
void RedrawRGBColors( void );
int InitPrefsDialog( void );

long Init_ComboBox( char *id, int ids )
{
	return Init_ComboBoxString( id, ReturnString(ids) );
}




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
    			ghStats4Dlg = NULL,
    			ghStats5Dlg = NULL,
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
    			ghVDomainsDlg = NULL,
    			ghVDomainsTabDlg = NULL,
    			ghVDomains1Dlg = NULL,
    			ghVDomains2Dlg = NULL,
    			ghVDomains3Dlg = NULL,
    			ghVDomains4Dlg = NULL,
    			ghVDomains5Dlg = NULL,
    			ghRouterDlg = NULL,
    			ghRouterTabDlg = NULL,
    			ghRouterIPDlg = NULL,
    			ghROIDlg = NULL,
    			ghRemoteDlg = NULL;
				
void PrefsOk( void *w, void *d );
void PrefsCancel( void *w, void *d );
				



static char *prefsWindows[][3] = {
//	{ "PREFS_PREPROC",		"PreProcess" ,	(char*)preproc_xpm }, 
	{ "PREFS_REPORT",		"Report" ,		(char*)report_xpm }, 
	{ "PREFS_ANALYSIS",		"Analysis",		(char*)analysis_xpm }, 
	{ "PREFS_FILTERS",		"Filters",		(char*)filters_xpm }, 
	{ "PREFS_STATS",		"Statistics",	(char*)stats_xpm }, 
	{ "PREFS_ADDS",			"Advertizing",	(char*)advert_xpm }, 
	{ "PREFS_VDOMAINS",		"Virtual Domains",	(char*)vdomains_xpm }, 
	{ "PREFS_POSTPROC",		"Post Proc",	(char*)postproc_xpm }, 
	{ "PREFS_CUSTOM",		"Custom",		(char*)custom_xpm },
//	{ "PREFS_ROUTER",		"Router/Firewall",	(char*)router_xpm },
//	{ "PREFS_SERVER",		"Remote Control",	(char*)remote_xpm }, 
	{ 0,0,0 },
	};


void HideAllPrefs( void )
{
	long i=0;
	
	while( prefsWindows[i][0] ){
		CloseWindow( prefsWindows[i++][0] );
	}
}

void SelectPrefsOption( void *clist, gint row, gint col, void *event, void *d )
{
	long i=0;
	
	while( prefsWindows[i][0] ){
		if ( row == i )
			ShowWindow( prefsWindows[i++][0] );
		else
			CloseWindow( prefsWindows[i++][0] );
	}
}




//--------- FILTER IO FUNCTIONS

// convert an "blah=value" into two strings, "blah" "value"
static void FilterToArray( char *filterstring, char **array, long max )
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


static void ArrayToFilter( char **array, char *filterstring, long max )
{
	switch( max ){
		case 2:	sprintf( filterstring, "%s=%s", array[0], array[1] ); break;
		case 3:	sprintf( filterstring, "%s,%s,%s", array[0], array[1], array[2] ); break;
	}
}


long DataListFromGUI( char* id, long total, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE] )
{
	long index=0, tot=total, num=0, cols;

	filter[index][0]=0;

	if ( tot>MAX_FILTERUNITS ) tot=MAX_FILTERUNITS;

	for ( index=0; index < tot; index++ ){
		char texts[3][256];
		char *textarray[] = { texts[0], texts[1], texts[2], 0 };

		cols = ListView_GetItem( id, index, 0, textarray );
		if ( cols ){
			ArrayToFilter( textarray, filter[index], cols );
			filter[index+1][0]=0;
		}
		num++;
	}
	return num;
}

long DataListToGUI( char* id, long total, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE] )
{
	int num=0;
	
	if ( total>0 ){
		char texts[2][256];
		char *textarray[3];

		textarray[0] = texts[0];
		textarray[1] = texts[1];
		textarray[2] = NULL;

		OutDebugs( "Settings up %s", id );
		ClearListItems( id );
		OutDebug( "Clear List Done" );
		while( total>0 ){

			FilterToArray( filter[num], textarray, 2 );
		OutDebugs( "ListView_AppendItem 1=%s, 2=%s", textarray[0], textarray[1] );
			ListView_AppendItem( id, textarray );
			num++;
			total--;
		}
	}
	return num;
}

long FilterListFromGUI( char* id, long total, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE] )
{
	long index=0, tot=total, num=0, cols;

	filter[index][0]=0;

	if ( tot>MAX_FILTERUNITS ) tot=MAX_FILTERUNITS;

	for ( index=0; index < tot; index++ ){
		char texts[3][256];
		char *textarray[5];

		textarray[0] = texts[0];
		textarray[1] = texts[1];
		textarray[2] = texts[2];
		textarray[3] = NULL;

		cols = ListView_GetItem( id, index, 0, textarray );
		if ( cols ){
			ArrayToFilter( textarray, filter[index], cols );
			filter[index+1][0]=0;
		}
		num++;
	}
	return num;
}

long FilterListToGUI( char* id, long total, char filter[MAX_FILTERUNITS][MAX_FILTERSIZE] )
{
	int num=0;
	
	if ( total>0 ){
		char texts[3][256];
		char *textarray[5];

		textarray[0] = texts[0];
		textarray[1] = texts[1];
		textarray[2] = texts[2];
		textarray[3] = NULL;

		ClearListItems( id );
		while( total>0 ){
			FilterToArray( filter[num], textarray, 3 );
			ListView_AppendItem( id, textarray );
			num++;
			total--;
		}
	}
	return num;
}



void DataListsToGUI( void )
{
	FilterListToGUI( "IDC_FILTERIN_LIST",	EditPrefPtr->filterdata.filterInTot,	EditPrefPtr->filterdata.filterIn );

	DataListToGUI( IDC_PREFADD_LIST,		EditPrefPtr->filterdata.advertTot,		EditPrefPtr->filterdata.advert );
	DataListToGUI( IDC_ADDCAMP_LIST,		EditPrefPtr->filterdata.advert2Tot,		EditPrefPtr->filterdata.advert2 );
	DataListToGUI( IDC_PREFORG_LIST,		EditPrefPtr->filterdata.orgTot,			EditPrefPtr->filterdata.org );
	DataListToGUI( IDC_PREFVD_LIST,			EditPrefPtr->filterdata.vhostmapTot,	EditPrefPtr->filterdata.vhostmap );
	DataListToGUI( IDC_PREFROUTER_LIST,		EditPrefPtr->filterdata.routerTot,		EditPrefPtr->filterdata.router );
}

void DataListsFromGUI( void )
{
	FilterListFromGUI( "IDC_FILTERIN_LIST",	EditPrefPtr->filterdata.filterInTot,	EditPrefPtr->filterdata.filterIn );

	DataListFromGUI( IDC_PREFADD_LIST,		EditPrefPtr->filterdata.advertTot,		EditPrefPtr->filterdata.advert );
	DataListFromGUI( IDC_ADDCAMP_LIST,		EditPrefPtr->filterdata.advert2Tot,		EditPrefPtr->filterdata.advert2 );
	DataListFromGUI( IDC_PREFORG_LIST,		EditPrefPtr->filterdata.orgTot,			EditPrefPtr->filterdata.org );
	DataListFromGUI( IDC_PREFVD_LIST,		EditPrefPtr->filterdata.vhostmapTot,	EditPrefPtr->filterdata.vhostmap );
	DataListFromGUI( IDC_PREFROUTER_LIST,	EditPrefPtr->filterdata.routerTot,		EditPrefPtr->filterdata.router );
}


long InitGenericListView( char *itemname, char *items )
{
	char data[512];
	char *stringPtr[64] = {0}, *p;
	long	i = 0, c = 0, count=0;

	if ( itemname ){
		mystrcpy( data, items );

		p = data;
		while( p ){
			count++;
			stringPtr[c] = p;
			p = mystrchr( p, ',' );
			if ( p ){
				*p = 0;
				p++;
			}
			c++;
			stringPtr[c] = 0;
		}
		ReplaceListView( itemname, stringPtr, count, 14, 0 );
	}
	return c;
}



#include <dirent.h>
/*---------------------------------------------------------------
 * Function: GetLclDir()
 *
 * Description: Get the local file directory and write to
 *   temporary file for later display.
 */
long FindLangFilenames( char *source , char *out )
{
	DIR *dp;
	char	newPath[256], firstFile[256], pattern[256], *p;
	int 	fFound=FALSE,len, lastn;
	int		nNext = 0;

	mystrcpy( firstFile, source );
	FileFromPath( firstFile, pattern );
	
	PathFromFullPath( firstFile, NULL );

	if ( !firstFile[0] )
		sprintf( firstFile, "./" );

	dp = opendir ( firstFile );
	if (dp){
		struct dirent *ep;

		while (ep = readdir (dp)){
			FileFromPath( ep->d_name, newPath );
			if ( match( newPath,pattern, 1 ) ){
				//sprintf( newPath, "%s/%s", firstFile, ep->d_name );

				p = strchr( newPath, '.' );
				if ( p ) *p = 0;

				if ( !nNext )
					strcpy( out, "English" );
				strcat( out, "," );
				strcat( out, newPath );

				nNext++;
			}
		}
		(void) closedir (dp);
	} else
		OutDebugs ("Couldn't open the directory (%s)", firstFile);
	return nNext;
}





long Init_LangComboBox( char *idname )
{
	int 	fFound=0,len, lastn, count=0;
	char	list[1024];
	char	langsPath[256];
	char	*p, hPath[256];


	strcpy( hPath, getenv("HOME") );
	// Check the exe directory first - as this is where users should have their lang directory
	if ( !fFound ){
		mystrcpy( langsPath, "/usr/local/Analyzer/Languages/*.lang" );
		fFound = FindLangFilenames( langsPath, list );
	}

	if ( !fFound ){
		mystrcpy( langsPath, "/local/Analyzer/Languages/*.lang" );
		fFound = FindLangFilenames( langsPath, list );
	}

	if ( !fFound ){
		mystrcpy( langsPath, "/opt/Analyzer/Languages/*.lang" );
		fFound = FindLangFilenames( langsPath, list );
	}

	// Try the current directory - basically, a last resort...
	if ( !fFound ){
		mystrcpy( langsPath, "Languages/*.lang" );
		fFound = FindLangFilenames( langsPath, list );
	}

	// Now check for a lang directory which is located back a directory - this where we have our lang directory
	if ( !fFound ) {
		sprintf( langsPath, "%s/Languages/*.lang", hPath );
		fFound = FindLangFilenames( langsPath, list );
	}

		
	// Try the current directory - basically, a last resort...
	if ( !fFound ){
		mystrcpy( langsPath, "*.lang" );
		fFound = FindLangFilenames( langsPath, list );
	}

	if ( fFound )  {
		//printf( "list = %s\n", list);
		Init_ComboBoxString( idname, list );
		SetPopupNum( idname, 0 );
	} else {
		Init_ComboBoxString( idname, "English" );
		SetPopupNum( idname, 0 );
	}
	return fFound;
} 

// -----------------------------------------------------------------------




void SelectChildSettingsTreeProc(GtkTree *tree, GtkWidget *widget, gpointer user_data )
{
	char *name;
	long data;

	name = ConfigFindSettings( EditPrefPtr, (long)user_data, &data );
	OutDebugs( "tree child selected - %s", name );
}



void ShowSettingsInfoOptions( long data )
{
	HWND hDlg;
	if ( TABLE( data ) )	CheckON( IDC_STAT_TABLE ); else CheckOFF( IDC_STAT_TABLE );
	if ( GRAPH( data ) )	CheckON( IDC_STAT_GRAPH ); else CheckOFF( IDC_STAT_GRAPH );
	SetPopupNum( IDC_TABSIZE, TABSIZE(data)-1 );
	SetPopupNum( IDC_SORTING, TABSORT(data) );
}


static long gLastSettings;


// Handle the Settings options for each Report page.
void StatsSettingTableProc( GtkWidget *widget, gpointer user_data )
{
	long data;
	ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
	if ( IsChecked( IDC_STAT_TABLE ) )
		STATSETT( data, 1 );
	else
		STATSETT( data, 0 );
	ConfigSetSettings( EditPrefPtr, gLastSettings, data );
	//OutDebugs( "tableoptions %d = %08lx", gLastSettings, data );
}

void StatsSettingRowsProc( GtkWidget *widget, gpointer user_data )
{
	long data;
	int numSel = GetPopupNum( IDC_TABSIZE );
	ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
	STATSETTAB(data,numSel+1);
	ConfigSetSettings( EditPrefPtr, gLastSettings, data );
	//OutDebugs( "rowsoptions %d = %08lx", gLastSettings, data );
}
void StatsSettingGraphProc( GtkWidget *widget, gpointer user_data )
{
	long data;
	ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
	if ( IsChecked( IDC_STAT_GRAPH ) )
		STATSETG( data, 1 );
	else
		STATSETG( data, 0 );
	ConfigSetSettings( EditPrefPtr, gLastSettings, data );
}
void StatsSettingRequestsProc( GtkWidget *widget, gpointer user_data )
{
	long data;
	int numSel = GetPopupNum( IDC_SORTING );
	ConfigFindSettings( EditPrefPtr, gLastSettings, &data );
	STATSETSORT(data,numSel);
	ConfigSetSettings( EditPrefPtr, gLastSettings, data );
}



// Init all the settings tree details, ie which ones have ticks or not
void SelectSettingsTreeProc( GtkWidget *widget, gpointer user_data )
{
	char *pstr;
	long data;

	pstr = ConfigFindSettings( EditPrefPtr, (long)user_data, &data );
	gLastSettings = (long)user_data;
	//OutDebugs( "tree selected - %d , %s = %08lx", gLastSettings, pstr, data );

	if ( pstr )
	{
		if ( *pstr != '>' ){		//&& STAT(data) 
			Enable( IDC_STAT_TABLE );
			Enable( IDC_STAT_GRAPH );
			Enable( IDC_TABSIZE );
			Enable( IDC_SORTING );
			ShowSettingsInfoOptions( data );
		} else {
			SetText( IDC_APPLYALL, ReturnString(IDS_APPLY_GROUP) );
			pstr = NULL;
		}
	}

	if ( !pstr )
	{
		//ShowSettingsInfoOptions( hWnd, 0 );
		Disable( IDC_STAT_TABLE );
		Disable( IDC_STAT_GRAPH );
		Disable( IDC_TABSIZE );
		Disable( IDC_SORTING );
		SetText( IDC_STATEDIT_NAME, "Options" );
	}
}
 


void InitSettingsTreeView ( void )                                     
{
	int index;          // Index used in for loops

	{
		long data, mstate = 0; char *p;//TVS_HASBUTTONS 
		GtkWidget *item;

		index = 0;
		while( p = ConfigFindSettings( EditPrefPtr, index, &data ) ){
			if ( *p == '>' ) {
				item = TreeView_InsertItem( IDC_STATEDIT, p+1, 0, index, (void*)SelectSettingsTreeProc );
			} else {
				int state;

				if ( STAT(data) )
					state = 3;
				else 
					state = 2;

				item = TreeView_InsertItem( IDC_STATEDIT, p, state, index, (void*)SelectSettingsTreeProc );
			}
			index++;
		}

		AttachProcToControl( "PREFS_STATSEDIT", "IDC_STAT_TABLE",	(void*)StatsSettingTableProc, "toggled", (void*)0 );
		AttachProcToControl( "PREFS_STATSEDIT", "IDC_STAT_GRAPH",	(void*)StatsSettingGraphProc, "toggled", (void*)0 );

		AttachProcToPopup( "IDC_TABSIZE", (void*)StatsSettingRowsProc );
		AttachProcToPopup( "IDC_SORTING", (void*)StatsSettingRequestsProc );
	}
}



void InitExtensionsTreeView ( void ) 
{
	int index;          // Index used in for loops
	GtkWidget *item;

	TreeView_InsertItem( IDC_EXTEDIT, ReturnString(IDS_EXT_PAGES), 0, 0, NULL );

	index = 0;
	while( EditPrefPtr->pageStr[index][0] ){
		item = TreeView_InsertItem( IDC_EXTEDIT, EditPrefPtr->pageStr[index], 1, 0, NULL );
		index++;
	}

	TreeView_InsertItem( IDC_EXTEDIT, ReturnString(IDS_EXT_DOWN), 0 , 0, NULL);
	index = 0;
	while( EditPrefPtr->downloadStr[index][0] ){
		item = TreeView_InsertItem( IDC_EXTEDIT, EditPrefPtr->downloadStr[index], 1, 0,  NULL );
		index++;
	}

	TreeView_InsertItem( IDC_EXTEDIT, ReturnString(IDS_EXT_AUDIO), 0, 0, NULL );
	index = 0;
	while( EditPrefPtr->audioStr[index][0] ){
		item = TreeView_InsertItem( IDC_EXTEDIT, EditPrefPtr->audioStr[index], 1, 0, NULL );
		index++;
	}

	TreeView_InsertItem( IDC_EXTEDIT, ReturnString(IDS_EXT_VIDEO), 0, 0, NULL );
	index = 0;
	while( EditPrefPtr->videoStr[index][0] ){
		item = TreeView_InsertItem( IDC_EXTEDIT, EditPrefPtr->videoStr[index], 1, 0, NULL );
		index++;
	}
}






void EditPathClear( void *w, void *d)
{
	while( GetTotalEditPath( EditPrefPtr->EditPaths ) ){
		DelEditPath( &EditPrefPtr->EditPaths, 1 );
	}
}
void EditPathDelete( void *w, void *d)
{
	long row=0;
	char *txt;
	
	while( row>=0 ){
		row = ListView_IsItSelectedItem( IDC_PREFS_EDITPATH, row, 0, &txt );
		if ( row >= 0 ){
			DelEditPath( &EditPrefPtr->EditPaths, row );
		}
	}
}
void EditPathGUIToData( int number )
{
	EditPathRecPtr	dataP;
	if ( dataP = GetEditPath( EditPrefPtr->EditPaths, number ) ){
		char			items[2][256];

		GetText( IDC_EDITPATH_HOST, dataP->vhost, 256 );
		GetText( IDC_EDITPATH_PATH, dataP->path, 256 );
	}
}
void EditPathChange( void *w, void *d)
{
	long row=0;
	char *txt;
	
	while( row>=0 ){
		row = ListView_IsItSelectedItem( IDC_PREFS_EDITPATH, row, 0, &txt );
		if ( row >= 0 ){
			EditPathGUIToData( row );
			row = -1;
		}
	}
}






// -------------------------

void UpdateEditPathNames( int number )
{
	EditPathRecPtr	dataP;

	if ( dataP = GetEditPath( EditPrefPtr->EditPaths, number ) ){
		SetText( IDC_EDITPATH_HOST, dataP->vhost );
		SetText( IDC_EDITPATH_PATH, dataP->path );
	}
}


void EditPathSelectedProc( GtkCList *clist, gint row,
                                            gint column,
                                            GdkEventButton *event,
                                            gpointer user_data)
{
	char *txt;
	row = ListView_IsItSelectedItem( IDC_PREFS_EDITPATH, row, 0, &txt );
	if ( row >= 0 ){
		UpdateEditPathNames( row );
		row = -1;
	}
}

// Copy the whole list to the GUI
void EditPathListToGUI( void )
{
	char			items[2][256];
	char			*itemP[1][7];
	long			num=1,numtype=0;
	EditPathRecPtr	dataP;
	
	itemP[0][0] = items[0];
	itemP[0][1] = items[1];
	
	while( dataP = GetEditPath( EditPrefPtr->EditPaths, num ) ){
		strcpy( items[0], dataP->vhost );
		strcpy( items[1], dataP->path );
		ListView_AppendItem( IDC_PREFS_EDITPATH, itemP[0] );
		num++;
	}
}

void EditPathBrowseProc( void *w, void *d)
{
	char *file;
	file = GetFileName( "IDC_XMAINTOOLBAR", "Select output location/file..." );
	if ( file ){
		SetText( IDC_EDITPATH_PATH, file );
	}
}











void UpdateSliderProc( GtkAdjustment *adj , gpointer user_data)
{
	long pos,dataNum;
	if ( adj && EditPrefPtr ){
		pos = (gint) adj->value;
		if ( pos >=0 ){
			char	szText[255];
			dataNum = GetPopupNum( IDC_CUSTOM_DATA );
			EditPrefPtr->custom_dataIndex[ dataNum ] = pos;
			sprintf( szText, "Index: %d", pos );
			SetText( IDC_CUSTOM_POSTXT, szText );
			ShowCustomConfigInfo( NULL );
		}
	}
}





void GetSelectedPopText( char *id, char *array, char *text )
{
	int i;
	char	*p;
	char	*list[32];

	i = GetPopupNum( id );
	ConvertCommaListtoArray( array, list, 30 );
	mystrcpy( text, list[i] );
//	OutDebugs( "popitem=%d", i );
//	OutDebugs( "popitemtext=%s, i=%d", list[i], i );

}





void DefaultOrgsProc( void *w, void *d )
{
	char *defaultOrgs[][3] = {
		{ "*.org",		"Organisations",0 },
		{ "*.org.*",	"Organisations",0 },
		{ "*.edu",		"US Educational",0 },
		{ "*.edu.*",	"Educational",0 },
		{ "*.mil",		"US Military",0 },
		{ "*.mil.*",	"Military",0 },
		{ "*.gov",		"US Government",0 },
		{ "*.gov.*",	"Government",0 },
		{ "*.com",		"US Commercial",0 },
		{ "*.com.*",	"Commercial",0 },
		{ "*.co.*",		"Commercial",0 },
		{ "*.net",		"US Network",0 },
		{ "*.net.*",	"Network",0 },
		{ "*proxy*",	"Proxy Server",0 },
		{ "*robot*",	"Search Bots",0 },
		{ 0,0,0 }
	};

	int i;
	ListView_DeleteAll( "IDC_PREFORG_LIST" );
	EditPrefPtr->filterdata.orgTot = 0;
	*EditPrefPtr->filterdata.org[0] = 0;
	i=0;
	while( *defaultOrgs[i] ){
		ListView_AppendItem( "IDC_PREFORG_LIST" , defaultOrgs[i] );
		EditPrefPtr->filterdata.orgTot++;
		i++;
	}
}


void ClearCacheProc( void *w, void *d )
{
	ClearLookupCacheFile();
	ErrorMsg( ReturnString(IDS_DNSCLEARED) );
}





// ------------------------------------------------------------------------------------------
// These are all the LISTVIEW functions, that add/del/clr/change/default all the listviews


// WINDOW, NAME WIDGET, LIST WIDGET, ADD, REM, CLR, CHANGE, DEFAULTBUTTOPN
static char *ListViewsButtons[][7] = {
	{ "PREFS_FILTERINPUT",		"IDC_FILTERIN_DATA",	"IDC_FILTERIN_LIST","IDC_FILTERIN_ADD",	"IDC_FILTERIN_REM",	"IDC_FILTERIN_CHANGE", NULL },
	{ "PREFS_STATSDOMAINS",		"IDC_PREFORG_PATTERN",	"IDC_PREFORG_LIST", "IDC_PREFORG_ADD",	"IDC_PREFORG_REM",	NULL, NULL },
	{ "PREFS_STATSDIRMAP",		"IDC_PREFGRP_PATTERN",	"IDC_PREFGRP_LIST", "IDC_PREFGRP_ADD",	"IDC_PREFGRP_REM",	"IDC_PREFGRP_CHANGE", NULL },
	{ "PREFS_STATSREFMAP",		"IDC_PREFREF_PATTERN",	"IDC_PREFREF_LIST", "IDC_PREFREF_ADD",	"IDC_PREFREF_REM",	"IDC_PREFREF_CHANGE", NULL },
	{ "PREFS_ADDSBANNERS",		"IDC_PREFADD_NAME",		"IDC_PREFADD_LIST", "IDC_PREFADD_ADD"	"IDC_PREFADD_REM",	"IDC_PREFADD_CHANGE", NULL },
	{ "PREFS_ADDSCAMPAIGNS",	"IDC_ADDCAMP_NAME",		"IDC_ADDCAMP_LIST", "IDC_ADDCAMP_ADD",	"IDC_ADDCAMP_REM",	"IDC_ADDCAMP_CHANGE", NULL },
	{ "PREFS_VDOMAINSPATH",		"IDC_EDITPATH_HOST",	"IDC_PREFS_EDITPATH",	"IDC_EDITPATH_ADD",		"IDC_EDITPATH_DEL",		"IDC_EDITPATH_CHANGE", NULL },
	{ "PREFS_VDOMAINSMAP",		"IDC_PREFVD_PATTERN",	"IDC_PREFVD_LIST",	"IDC_PREFVD_ADD",	"IDC_PREFVD_REM",	NULL, NULL },
	{ "PREFS_ROUTER_IP",		"IDC_PREFROUTER_NAME",	"IDC_PREFROUTER_LIST",	"IDC_PREFROUTER_ADD",	"IDC_PREFROUTER_REM",	"IDC_PREFROUTER_CHANGE", "IDC_PREFROUTER_CLR" },
	{ NULL,NULL,NULL,NULL,NULL,NULL,NULL }
};

void AddItemToListView( long i, int insertrow )
{
	char text[256];
	char text2[256];
	char text3[256];
	char text4[256];
	char *textPtr[] = {0,0,0,0,0,0};

	textPtr[0] = text;
	textPtr[1] = text2;

	GetText( ListViewsButtons[i][1], text, 255 );

//OutDebugs( "addlist %s\n", ListViewsButtons[i][0] );	
	if ( !strcmpd( "PREFS_FILTERINPUT", ListViewsButtons[i][0] ) ){
		GetSelectedPopText( "IDC_FILTERIN1", ReturnString(IDS_FILTER1), text );
		GetSelectedPopText( "IDC_FILTERIN2", ReturnString(IDS_FILTER2), text2 );
		GetSelectedPopText( "IDC_FILTERIN3", ReturnString(IDS_FILTER3), text3 );
		GetText( ListViewsButtons[i][1], text4, 255 );
		textPtr[2] = text3;
		textPtr[3] = text4;
		textPtr[4] = 0;
	}

	if ( !strcmpd( "PREFS_ADDSBANNERS", ListViewsButtons[i][0] ) ){ 
		GetText( "IDC_PREFADD_PATTERN1", text2, 255 );
		GetText( "IDC_PREFADD_PATTERN2", text3, 255 );
		textPtr[2] = text3;
	}
	if ( !strcmpd( "PREFS_ADDSCAMPAIGNS", ListViewsButtons[i][0] ) ){ 
		GetText( "IDC_ADDCAMP_PATTERN", text2, 255 );
		GetText( "IDC_ADDCAMP_COST", text3, 255 );
		textPtr[2] = text3;
	}

	if ( !strcmpd( "PREFS_STATSDOMAINS", ListViewsButtons[i][0] ) ){ 
		GetText( "IDC_PREFORG_NAME", text2, 255 );
	}
	if ( !strcmpd( "PREFS_STATSDIRMAP", ListViewsButtons[i][0] ) ){ 
		GetText( "IDC_PREFGRP_NAME", text2, 255 );
	}
	if ( !strcmpd( "PREFS_STATSREFMAP", ListViewsButtons[i][0] ) ){ 
		GetText( "IDC_PREFREF_NAME", text2, 255 );
	}

	if ( !strcmpd( "PREFS_VDOMAINSMAP", ListViewsButtons[i][0] ) ){ 
		GetText( "IDC_PREFVD_NAME", text2, 255 );
	}
		
	if ( !strcmpd( "PREFS_VDOMAINSPATH", ListViewsButtons[i][0] ) ){ 
		GetText( "EDITPATH_PATH", text2, 255 );
	}

	if ( insertrow>=0 )
		ListView_InsertItem( ListViewsButtons[i][2] , insertrow, textPtr );
	else
		ListView_AppendItem( ListViewsButtons[i][2] , textPtr );
}



void AddStringToListViewProc( void *w, void *d )
{
	AddItemToListView( (long)d, -1 );
}


// This one is tricky, it will do these in order
// 1. Add after selected first entry
// 2. Delete selected entry (1)
void ChangeListViewItemProc( void *w, void *d )
{
	char text[256]; long i = (long)d, row;

	GetText( ListViewsButtons[i][1], text, 255 );
	row = ListView_GetSelected( ListViewsButtons[i][2] );
	if ( row >= 0 ){
		AddItemToListView( i, row );
		ListView_DeleteSelected( ListViewsButtons[i][2] );
	}
}

// Delete item(s) selected
void DeleteIteminListViewProc( void *w, void *d )
{
	ListView_DeleteSelected( ListViewsButtons[(long)d][2] );
}

// Delete all items, ie CLEAR
void DeleteWholeListViewProc( void *w, void *d )
{
	ListView_DeleteAll( ListViewsButtons[(long)d][2] );
}


void InsertTokenProc( void *w, void *d )
{
	long pos, pitem, l;
	char tmp[256], tmp2[256], *p;
	char	*list[32];

	pos = GetCursorPos( IDC_OUTDIR );	

	pitem = GetPopupNum( IDC_TOKENS );

	ConvertCommaListtoArray( ReturnString(IDS_DATETOKENS), list, 30 );

//	OutDebugs( "1=%s,2=%s", list[0], list[1] );

	p = (char*)mystrchr( list[ pitem ], '{' );
	if( p ){
		//pos--;
		l = strlen( p );
		memset( tmp, 0, 255 );
		GetText( IDC_OUTDIR, tmp, 255 );
		sprintf( tmp2, "%s\0", &tmp[pos] );
		mystrncpy( &tmp[pos], p, l );
		strcat( tmp, tmp2 );
		SetText( IDC_OUTDIR, tmp );
	}
}


void SelectDatabaseONProc( void *w, void *d )
{
	Enable( IDC_DB_NAME );
	Enable( IDC_DB_BROWSE );
}

void SelectDatabaseOFFProc( void *w, void *d )
{
	Disable( IDC_DB_NAME );
	Disable( IDC_DB_BROWSE );
}


void DateSelectedProc( void *h, void *d )
{
	int datestyle;
	
	datestyle = (short)GetPopupNum( IDC_PREFGEN_DATESTYLE );

	if ( datestyle >0 ){
		long	jd1,jd2;
		char szText[64];

		DatetypeToDate( datestyle, &jd1, &jd2 );

		DaysDateToString( jd1, szText, GetDateStyle(), '/', 1,1);
		SetText( IDC_DATESTART, szText );

		DaysDateToString( jd2, szText, GetDateStyle(), '/', 1,1);
		SetText( IDC_DATEEND, szText );

		/* disable the date fields */
		Disable( IDC_DATESTART );
		Disable( IDC_DATEEND );
	} else {
		Enable( IDC_DATESTART );
		Enable( IDC_DATEEND );
	}
}

void OutputFormatSelectedProc( void *w, void *d )
{
	FixReportOutput( w );
}



void SelectReportLocationProc( void *w, void *d )
{
	char *file;
	file = GetFileName( "IDC_XMAINTOOLBAR", "Select output location/file..." );
	if ( file ){
		strcpy( EditPrefPtr->outfile, file );
		SetText( IDC_OUTDIR, file );
		FixReportOutput( w );
	}
}

void SelectDBLocationProc( void *w, void *d )
{
	char *file;
	file = GetFileName( "IDC_XMAINTOOLBAR", "Select database location/file..." );
	if ( file ){
		strcpy( EditPrefPtr->database_file, file );
		SetText( IDC_DB_NAME, file );
	}
}


typedef struct {
	char *name;
	long rgb[16];
} SchemeData, *SchemeDataP;

SchemeData default_schemes[] = {
	// Name,    Title,    Header,   Line Even, Line Odd, Others,  Total
	{ "Random",	-1, -1, -1, -1, -1, -1 } ,
	{ "Default",0x999999, 0xcccccc, 0xf4f4f4, 0xcccccc, 0x99cccc, 0x99cccc } ,
	{ "Basic",	0x99AACC, 0x888888, 0xffffff, 0xdddddd, 0xd0c0b0, 0xa0b0c0 } ,
	{ "Grey",	0xa0a0a0, 0x808080, 0xffffff, 0xdddddd, 0xd0d0d0, 0xb0b0b0 } ,
	{ "Blue",	0x3080c0, 0x90d0f0, 0xdfefff, 0xb0c0d0, 0xd0c0ff, 0x50a0f0 } ,
	{ "Wild",	0xff0030, 0xffc030, 0xffffef, 0xddddad, 0x80f030, 0xccffcc } ,
	{ "Bland",	0xfafafa, 0xffcfff, 0xffffff, 0xffffff, 0xffcfff, 0xfafafa } ,
	{ "Ice",	0xfafafa, 0x99ccff, 0xccccff, 0x99ccff, 0x99ccff, 0x3399cc } ,

	{ 0,0,0,0,0,0,0 } };

void UseColorSchemeX( long item )
{
	long i = 0, rgb = 0;

	srand( GetCurrentCTime() );

	for( i=0;i<6;i++){
		rgb = default_schemes[item].rgb[i];
		if ( rgb == -1 )
			rgb = (rand() * 255)<<16 | (rand() * 255)<<8 | (rand() * 255);
		EditPrefPtr->RGBtable[i+1] = rgb;
	}
	RedrawRGBColors();
}
void UseColorSchemeName( char *name )
{
	long i = 0;

	if ( name )
	{
		while ( default_schemes[i].name )
		{
			if ( !strcmpd( name, default_schemes[i].name ) )
			{
				UseColorSchemeX( i );	
			}
			i++;
		}
	}
}


void SelectColorSchemeProc( GtkList *list,  gpointer user_data )
{
	char *name;

	name = (char*)GetSelectedListItemText( "IDC_SCHEMES", (char*)0 );
	UseColorSchemeName( name );
}

void InitColorSchemes( void )
{
	long i = 0;

	OutDebug( "InitColorSchemes" );

	srand( GetCurrentCTime() );

	while( default_schemes[i].name ){
		AddItemtoList( IDC_SCHEMES, default_schemes[i].name );
		i++;
	}


	i = AttachProcToControl( "PREFS_REPORTPAGE", "IDC_SCHEMES",  (void*)SelectColorSchemeProc, "selection_changed", NULL );
	if ( i <0 ) OutDebugs( "error attachproc %d", i );
}



void RedrawRGBColor( void *w, void *d, void *num)
{
	long i = (long)num;
	if ( i >= 0 && i < 16 ){
		long col = EditPrefPtr->RGBtable[i];
		char name[64];
		sprintf( name, "IDC_PREFHTML_RGB%d", i );
		SetButtonBgColor( name, col );
	}
}

void RedrawRGBColors( void )
{
	long i=0;

	// set the html colors of the tables
	for(i=1;i<7;i++){
		char name[64];
		sprintf( name, "IDC_PREFHTML_RGB%d", i );
		SetButtonBgColor( name, EditPrefPtr->RGBtable[i] );
	}
}



void RedrawHTMLRGBColors( void )
{
	long i=0;

	GetHTMLColors();
	// set the html colors of the HTML headers
	for(i=7;i<12;i++){
		char name[64];
		sprintf( name, "IDC_PREFHTML_RGB%d", i );
		SetButtonBgColor( name, EditPrefPtr->RGBtable[i] );
	}

}

void SelectRGBColor( void *w, void *d )
{
	long	num = (long)d;
	long	rgb;

	if ( num >= 0 && num < 16 ){
		rgb = ChooseRGBColor ( "IDC_XMAINTOOLBAR", EditPrefPtr->RGBtable[num] );
		if ( rgb != -1 ){
			char name[64];
			sprintf( name, "IDC_PREFHTML_RGB%d", num );
			EditPrefPtr->RGBtable[num] = rgb;
			SetButtonBgColor( name, rgb );
		}
	}
}




void SelectHTMLRGBColor( void *w, void *d )
{
	long	num = (long)d;
	long	rgb;

	if ( num >= 0 && num < 16 ){
		rgb = ChooseRGBColor ( "IDC_XMAINTOOLBAR", EditPrefPtr->RGBtable[num] );
		if ( rgb != -1 ){
			char name[64];
			sprintf( name, "IDC_PREFHTML_RGB%d", num );
			SetButtonBgColor( name, rgb );
			EditPrefPtr->RGBtable[num] = rgb;
			SetHTMLColors( w );
		}
	}
}










void InitColorButtons( void )
{
	long i = 0;
	i = 0;
	OutDebug( "Init Color Buttons" );

	for(i=1;i<7;i++){
		char id[64];
		sprintf( id, "IDC_PREFHTML_RGB%d", i );
		AttachProcToButtonName( "PREFS_REPORTPAGE", id, (void*)SelectRGBColor, (void*)i );		
		//AttachProcToControl( "PREFS_REPORTPAGE", id,  RedrawRGBColor, "leave-notify-event", (void*)i );
		//AttachProcToControl( "PREFS_REPORTPAGE", id,  RedrawRGBColor, "enter-notify-event", (void*)i );
	}

	for(i=7;i<12;i++){
		char id[64];
		sprintf( id, "IDC_PREFHTML_RGB%d", i );
		AttachProcToButtonName( "PREFS_REPORTHTMLHEAD", id,	(void*)SelectHTMLRGBColor, (void*)i );
	}

}



// ------------------------------- INIT

void InitAllListViews( void )
{

	InitGenericListView( IDC_PREFS_EDITPATH , ReturnString(IDS_EDITPATH) );
	InitGenericListView( IDC_FILTERIN_LIST, ReturnString(IDS_FILTERS) );
	InitGenericListView( IDC_PREFORG_LIST, ReturnString(IDS_ORGHEADER) );
	InitGenericListView( IDC_PREFGRP_LIST, ReturnString(IDS_GROUPHEADER) );
	InitGenericListView( IDC_PREFREF_LIST, ReturnString(IDS_REFERHEADER) );
	InitGenericListView( IDC_PREFADD_LIST, ReturnString(IDS_ADDHEADER) );
	InitGenericListView( IDC_ADDCAMP_LIST, ReturnString(IDS_ADDHEADER2) );
	InitGenericListView( IDC_PREFVD_LIST, ReturnString(IDS_VMAPHEADER) );

}


void InitAllTreeViews( void )
{
	OutDebug( "Init ExtensionsTreeView" );
	InitExtensionsTreeView();

	OutDebug( "Init SettingsTreeView" );
	InitSettingsTreeView();
}


void InitAllComboboxes( void )
{
	Init_LangComboBox( IDC_PREF_LANG );

	Init_ComboBox( IDC_FILTERIN1, IDS_FILTER1 );
	Init_ComboBox( IDC_FILTERIN2, IDS_FILTER2 );
	Init_ComboBox( IDC_FILTERIN3, IDS_FILTER3 );

	Init_ComboBox( IDC_PREFGEN_TYPE, IDS_REPORTFORMAT );
	Init_ComboBox( IDC_PREFGEN_GTYPE, IDS_IMAGETYPE );
	Init_ComboBox( IDC_TOKENS, IDS_DATETOKENS );

	Init_ComboBox( IDC_PREF_PDF_PAGESIZE, IDS_REPORT_PDFPAGESIZES );
	Init_ComboBox( IDC_PREF_PDF_PAGENUMBERINGPOS, IDS_REPORT_PDFPAGENUMBERINGPOS );
	Init_ComboBox( IDC_PREF_PDF_SPACING, IDS_REPORT_PDFSPACING );
	Init_ComboBox( IDC_PREF_PDF_TABLETEXTNAME, IDS_REPORT_PDFTABLETEXTNAME );
	Init_ComboBox( IDC_PREF_PDF_TRUNCATETEXT, IDS_REPORT_PDFTRUNCATETEXT );
	Init_ComboBox( IDC_PREF_PDF_FONT, IDS_REPORT_PDFFONT );
	Init_ComboBox( IDC_PREF_PDF_FONTSTYLE, IDS_REPORT_PDFFONTSTYLES );
	//Init_ColorComboBox( ghReportPDFDlg, IDC_PREF_PDF_FONTCOLOR );
	Init_ComboBox( IDC_PREF_PDF_GRAPHFONT, IDS_REPORT_PDF_GRAPHFONT );

	Init_ComboBox( IDC_CUSTOM_DATE, IDS_DATEFMT );
	Init_ComboBox( IDC_CUSTOM_TIME, IDS_TIMEFMT );
	Init_ComboBox( IDC_CUSTOM_DATA, IDS_FIELDFMT );
	
	Init_ComboBox( IDC_VDSORTBY, IDS_VDSORTBY );
	Init_ComboBox( IDC_PREFVD_TYPE, IDS_TIMER );
	Init_ComboBox( IDC_SORTING, IDS_SORTING );
	SetSliderRange( "IDC_CUSTOM_POS", 32 );

	Init_ComboBox( IDC_PREFGEN_DATESTYLE, IDS_DATES );
	Init_ComboBox( IDC_DATEOVERIDE, IDS_DATEFORMAT );
	Init_ComboBox( IDC_PREFGEN_SPLITDATE, IDS_PREFGEN_SPLITDATE );
	Init_ComboBox( IDC_TABSIZE, IDS_TABSIZE );
	Init_ComboBox( IDC_VDREPORTBY, IDS_VDREPORTBY );

}





void SelectPostProc( void *w, void *d )
{
	PostProc_GUItoData();
	PostProc_DatatoGUI();
}






void InitCallbackProcs( void )
{
	long i=0;

	OutDebug( "Init Call Backs" );
	AttachProcToButtonName( "PREFS_REPORTOUTPUT", "IDC_DB_NONE", (void*)SelectDatabaseOFFProc, 0 );
	AttachProcToButtonName( "PREFS_REPORTOUTPUT", "IDC_DB_SELECT", (void*)SelectDatabaseONProc, 0 );

	AttachProcToButtonName( "PREFS_REPORTOUTPUT", "IDC_PREFGEN_OUT", (void*)SelectReportLocationProc, 0 );
	AttachProcToButtonName( "PREFS_REPORTOUTPUT", "IDC_DB_BROWSE",(void*) SelectDBLocationProc, 0 );
	AttachProcToButtonName( "PREFS_REPORTOUTPUT", "IDC_PREFGEN_INSERT", (void*)InsertTokenProc, 0 );

	AttachProcToButtonName( "PREFS_ANALYSISNET", "IDC_CLEARCACHE", (void*)ClearCacheProc, 0 );

	AttachProcToControl( "PREFS_CUSTOMFORMAT", "IDC_CUSTOM_POS", (void*)UpdateSliderProc, "value_changed", (void*)0 );

	AttachProcToPopup( IDC_PREFGEN_DATESTYLE, (void*)DateSelectedProc );
	AttachProcToPopup( IDC_PREFGEN_TYPE, (void*)OutputFormatSelectedProc );

	//AttachProcToControl( "PREFS_VDOMAINS5", "PREFS_EDITPATH",  EditPathSelectedProc, "select_row", (void*)0 );

	while( ListViewsButtons[i][0]  ){
		//OutDebugs( "Attach Listview Buttons to %s (add/del/change)" , ListViewsButtons[i][0] );
		AttachProcToButtonName( ListViewsButtons[i][0], ListViewsButtons[i][3], (void*)AddStringToListViewProc, (void*)i );
		AttachProcToButtonName( ListViewsButtons[i][0], ListViewsButtons[i][4], (void*)DeleteIteminListViewProc, (void*)i );
		AttachProcToButtonName( ListViewsButtons[i][0], ListViewsButtons[i][5], (void*)ChangeListViewItemProc, (void*)i );
		AttachProcToButtonName( ListViewsButtons[i][0], ListViewsButtons[i][6], (void*)DeleteWholeListViewProc, (void*)i );
		i++;
	}
	OutDebug( "Attach Default Procs" );
	AttachProcToButtonName( "PREFS_STATSDOMAINS", "IDC_PREFORG_DEFAULT", (void*)DefaultOrgsProc, (void*)6 );
	AttachProcToButtonName( "PREFS_VDOMAINSPATH", "EDITPATH_BROWSE", (void*)EditPathBrowseProc, (void*)8 );

	// Attache procs to checkboxes in PostProcess area
	AttachProcToButtonName( "PREFS_POSTUPLOAD", "IDC_PP_UPLOADLOG", (void*)SelectPostProc, (void*)0 );
	AttachProcToButtonName( "PREFS_POSTUPLOAD", "IDC_PP_UPLOADREPORT", (void*)SelectPostProc, (void*)1 );
	AttachProcToButtonName( "PREFS_POSTEMAIL", "IDC_PP_EMAILON", (void*)SelectPostProc, (void*)2 );



}

void InitPrefsControls( void )
{

	InitAllComboboxes();
	InitAllListViews();
	InitAllTreeViews();
	InitCallbackProcs();
	InitColorButtons();
	InitColorSchemes();

	return;
}




int InitPrefsDialog( void )
{
	long	i=0, ret=0;
	GtkWidget *lv, *win;


	lv = ReplaceListView( "IDC_PREFS_LISTRECT", NULL, 1, 66, 0 );
	if ( !lv ) return -1;

	win = FindWindowPtr( "CONFIGPREFS" );
	if ( !win ) return -2;
	
	while( prefsWindows[i][1] ){
		ret = AddListViewPixmapItem( "CONFIGPREFS", "IDC_PREFS_LISTRECT", "", (char**)prefsWindows[i][2], 66 );
		if ( ret<0 )
			printf( "AddListViewPixmapItem = %d\n", ret);

/*		gIconPixmaps[i] = PixMapFromData(  win, (char**)prefsWindows[i][2] );
		if ( gIconPixmaps[i] ){
			AddListViewIconItem( "IDC_PREFS_LISTRECT", prefsWindows[i][1], gIconPixmaps[i], 64 );
			//AddListViewItem( "IDC_PREFS_LISTRECT", prefsWindows[i][1] );
		}
*/
		i++;
	}

	AttachProcToButtonName( "CONFIGPREFS", "IDC_PREFCANCEL", (void*)PrefsCancel, 0 );
	AttachProcToButtonName( "CONFIGPREFS", "IDC_PREFOK", (void*)PrefsOk, 0 );
	AttachProcToControl( "CONFIGPREFS", "IDC_PREFS_LISTRECT", (void*)SelectPrefsOption, "select_row", 0 );

	i=0;
	while( prefsWindows[i][0] ){
		ret = AttachWindowToWindow( "CONFIGPREFS", prefsWindows[i++][0], 72, +4 );
		if ( ret <0 )
			printf( "AttachWindowToWindow failed =%d\n", ret );
	}

	AttachWindowToTab( "IDC_TAB_PREPROC", "PREFS_PREDOWNLOAD", "Download" );
	AttachWindowToTab( "IDC_TAB_PREPROC", "PREFS_PREOPTIONS", "Options" );

	AttachWindowToTab( "IDC_TAB_REPORT", "PREFS_REPORTOUTPUT", "Report" );
	AttachWindowToTab( "IDC_TAB_REPORT", "PREFS_REPORTPAGE", "Options" );
	AttachWindowToTab( "IDC_TAB_REPORT", "PREFS_REPORTHTMLHEAD", "Html Style" );
	//AttachWindowToTab( "IDC_TAB_REPORT", "PREFS_REPORTPDF2", "PDF Style" );

	AttachWindowToTab( "IDC_TAB_ANALYSIS", "PREFS_ANALYSISOPT", "Options" );
	AttachWindowToTab( "IDC_TAB_ANALYSIS", "PREFS_ANALYSISNET", "Network" );
	AttachWindowToTab( "IDC_TAB_ANALYSIS", "PREFS_ANALYSISEXT", "Extensions" );
	AttachWindowToTab( "IDC_TAB_ANALYSIS", "PREFS_ANALYSISDYN", "Dynamic" );
	
	AttachWindowToTab( "IDC_TAB_FILTER", "PREFS_FILTERINPUT", "Input Filter" );

	AttachWindowToTab( "IDC_TAB_STATS", "PREFS_STATSEDIT", "Statistics" );
	AttachWindowToTab( "IDC_TAB_STATS", "PREFS_STATSGLOBAL", "Global" );
	AttachWindowToTab( "IDC_TAB_STATS", "PREFS_STATSDOMAINS", "Orgs" );
	AttachWindowToTab( "IDC_TAB_STATS", "PREFS_STATSDIRMAP", "Content" );
	AttachWindowToTab( "IDC_TAB_STATS", "PREFS_STATSREFMAP", "Referral" );

	AttachWindowToTab( "IDC_TAB_ADDS", "PREFS_ADDSBANNERS", "Impressions" );
	AttachWindowToTab( "IDC_TAB_ADDS", "PREFS_ADDSCAMPAIGNS", "Campaigns" );

	AttachWindowToTab( "IDC_TAB_VDOMAINS", "PREFS_VDOMAINSOPT", "Domains" );
	AttachWindowToTab( "IDC_TAB_VDOMAINS", "PREFS_VDOMAINSPATH", "Aggregate" );
	AttachWindowToTab( "IDC_TAB_VDOMAINS", "PREFS_VDOMAINSMAP", "Mapping" );
	AttachWindowToTab( "IDC_TAB_VDOMAINS", "PREFS_VDOMAINSCLUSTER", "Cluster" );

	AttachWindowToTab( "IDC_TAB_POSTPROC", "PREFS_POSTCOMPRESS", "Compress" );
	AttachWindowToTab( "IDC_TAB_POSTPROC", "PREFS_POSTUPLOAD", "Upload" );
	AttachWindowToTab( "IDC_TAB_POSTPROC", "PREFS_POSTEMAIL", "Email" );

	AttachWindowToTab( "IDC_TAB_CUSTOM", "PREFS_CUSTOMFORMAT", "Custom" );
	//AttachWindowToTab( "IDC_TAB_ROUTER", "PREFS_ROUTER_IP", "Router Firewall" );


	OutDebug( "Init Prefs Controls" );
	InitPrefsControls();

	OutDebug( "Init Prefs default" );
	SelectPrefsOption( NULL, 0, 0, NULL, NULL );

	return 0;
}


static int returnstatus = 0;

void PrefsCancel( void *w, void *d )
{
	returnstatus = 0;
	CloseWindow( "CONFIGPREFS" );
}

void PrefsOk( void *w, void *d )
{
	EditPrefPtr = &MyPrefStruct;
	PrefsFromGUI();
	returnstatus = 1;
	CloseWindow( "CONFIGPREFS" );
}


int ShowPrefsDialog( void *w, void *d )
{
	static int init=0;

	EditPrefPtr = &MyPrefStruct;

	if ( ShowWindow( "CONFIGPREFS" ) ){
		perror( "cant open prefs\n" );
	} else {
		int ret;
		WindowPreventResize( "CONFIGPREFS" );
		if ( !init ) {
			ret = InitPrefsDialog();
			init = 1;
		}
		OutDebug( "Setting Prefs PrefsIntoGUI" );
		PrefsIntoGUI();
		OutDebug( "Setting Prefs DataListsToGUI" );
		DataListsToGUI();
		OutDebug( "Setting Prefs RedrawRGBColors" );
		RedrawRGBColors();
		OutDebug( "Setting Prefs RedrawHTMLRGBColors" );
		RedrawHTMLRGBColors();
	}
	return 	returnstatus;
}



int ShowSchedulePrefsDialog( Prefs *schedulePrefs )
{
	static int init=0;

	EditPrefPtr = schedulePrefs;

	if ( ShowWindow( "CONFIGPREFS" ) ){
		perror( "cant open prefs\n" );
	} else {
		int ret;
		WindowPreventResize( "CONFIGPREFS" );
		if ( !init ) {
			ret = InitPrefsDialog();
			init = 1;
		}
		OutDebug( "Setting Prefs data" );
		PrefsIntoGUI();
		DataListsToGUI();
		RedrawRGBColors();
		RedrawHTMLRGBColors();
	}
	return 	returnstatus;
}
