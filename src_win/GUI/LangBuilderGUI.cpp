
#include "LangBuilderGUI.h"
#include "ResDefs.h"
#include "myansi.h"
#include "config.h"
#include "GlobalPaths.h"
#include "translate.h"
#include "winutil.h"
#include "Prefsgui.h"

HWND hWndLangBuilderListView = NULL;
long currItemSel = 0;
short alreadySaving = 0;
#define LANG_LINE_SIZE 256
extern "C" HINSTANCE hInst;
HWND hDlg;

#define CATEGORY_NAME_LISTVIEW_POS 0
#define DEFAULT_ENGLISH_LISTVIEW_POS 1
#define EDIT_LANGUAGE_LISTVIEW_POS 2


void ReadNewLangStringsFromLangList()
{
	int nonRealTokenOffset = 0;
	char lineStr[LANG_LINE_SIZE];
	for( int i = SUMM_BEGIN; i < END_OF_STRINGS; i++ )
	{
		if ( !RealLangToken( i ) )
		{
			AddNewLangString( 0, i );
			nonRealTokenOffset++;
		}
		else
		{
			ListView_GetItemText( hWndLangBuilderListView, i-nonRealTokenOffset-1, EDIT_LANGUAGE_LISTVIEW_POS, lineStr, LANG_LINE_SIZE );
			AddNewLangString( lineStr, i );
		}
	}
}

short SaveNewLangStringsToLangFile( char *filename )
{
	FILE *fp;
	fp = CreateNewLang( filename );
	fp = fopen( filename, "wb" );

	if ( !fp )
	{
		MessageBox( GetFocus(), "Unable to save language file!", "File Save Error!", MB_OK|MB_ICONERROR );
		return 0;
	}

	char szFile[256], *p;
	FileFromPath( filename, szFile );
	p = strstr( szFile, "." );
	if ( p )
		*p = 0;
	time_t jd;
	char dateStr[64];
	TimeStringToDays( "now", &jd );
	CTimeToDateTimeStr( jd, dateStr );
	fprintf( fp, "// A couple of lines for top of lang file\n// %s File\n// Date %s\n\n", szFile, dateStr );

	for( int i = SUMM_BEGIN; i < END_OF_STRINGS; i++ )
	{
		WriteNewLangString( fp, i );
		ClearNewLangString( i );
	}
	fclose( fp );
	return 1;
}



BOOL GetLangSaveFolderName( char *fileWithPath, char *filename )
{
    char szPath[256];
	char *szFilter;

    szFilter = "Language Files (*.lang)\0*.lang\0\0";
	PathFromFullPath( fileWithPath, szPath );
 	return GetSaveDialog( fileWithPath, filename, szPath, szFilter, ReturnString(IDS_LANG_SAVEAS), "lang", 1);
}

extern const char *LANG_EXT;
extern short ValidatePath( char *pathtocheck );

short SaveLangFileAs( char *filename )
{
	char fileWithPath[1024];
	// Check the exe directory first - as this is where users should have their lang directory
	GetLangV41File( fileWithPath, gPath, PATHSEPSTR, 0 ); 
	if ( ValidatePath( fileWithPath ) )
	{
		// Now check for a lang directory which is located back a directory - this where we have our lang directory
		GetLangV41File( fileWithPath, gPath, PATHBACKDIR, 0 ); 
	}
	if ( ValidatePath( fileWithPath ) )
	{
		// Try the current directory - basically, a last resort...
		GetLangV41File( fileWithPath, 0, 0, 0 ); 
	}
	if ( GetLangSaveFolderName( fileWithPath, filename ) )
	{
		ReadNewLangStringsFromLangList();
		char *pos;
		pos = strstri( fileWithPath, LANG_EXT );
		if ( !pos )
			mystrcat( fileWithPath, LANG_EXT );
		if ( SaveNewLangStringsToLangFile( fileWithPath ) )
		{
			FileFromPath( fileWithPath, MyPrefStruct.language );
			strcpyuntil( MyPrefStruct.language, MyPrefStruct.language, '.' );
			return 1;
		}
	}
	return 0;
}

extern long Init_LangComboBox( HWND parent, long comboId );
extern void SetComboLanguageSel( HWND parent, long comboId, char *language );
extern void LanguageString_GUItoData( HWND parent, long comboId, char *language );

static short changedSomeStrings;

HWND hWndEdit = 0;
#define LANG_LIST_SEP ";*," // Need to have a seperator for the lang list that would not be used as a language string (or part thereof)

void InitLangList()
{
	char colTitles[LANG_LINE_SIZE];
	char *catStr;
	char lineStr[LANG_LINE_SIZE];
	short i;
	short category = -1;
	changedSomeStrings = 0;
	alreadySaving = 0;

	// If we have already displayed (created) the list,
	// then week need to delete the column headings... this seems to do it...
	if ( hWndLangBuilderListView )
	{
		ListView_DeleteColumn( hWndLangBuilderListView, EDIT_LANGUAGE_LISTVIEW_POS );
		ListView_DeleteColumn( hWndLangBuilderListView, DEFAULT_ENGLISH_LISTVIEW_POS );
		ListView_DeleteColumn( hWndLangBuilderListView, CATEGORY_NAME_LISTVIEW_POS );
	}
	
	// Initialise the Language list, means creating the column titles and adding them,
	// then making sure there is no data in the list
	mystrncpyNull( colTitles, ReturnString( IDS_LANGBUILDER_LISTTITLES ), LANG_LINE_SIZE );
	mystrncatNull( colTitles, MyPrefStruct.language, LANG_LINE_SIZE );
	hWndLangBuilderListView = InitGenericListView( hDlg, IDC_LANGTOKEN_LIST, 0, 0, colTitles );
	ListView_DeleteAllItems( hWndLangBuilderListView );		

	ListView_SetColumnWidth( hWndLangBuilderListView, CATEGORY_NAME_LISTVIEW_POS, 100 );
	ListView_SetColumnWidth( hWndLangBuilderListView, DEFAULT_ENGLISH_LISTVIEW_POS, 200 );
	ListView_SetColumnWidth( hWndLangBuilderListView, EDIT_LANGUAGE_LISTVIEW_POS, 200 );

	// Add the Language strings to the list
	for ( i = SUMM_BEGIN; i < END_OF_STRINGS; i++ )
	{
		catStr = GetLangSectionName(i);
		if ( !catStr )
			continue;
		mystrncpyNull( lineStr, catStr, LANG_LINE_SIZE ); 
		mystrncatNull( lineStr, LANG_LIST_SEP, LANG_LINE_SIZE ); 
		mystrncatNull( lineStr, DefaultEnglishStr(i), LANG_LINE_SIZE ); 
		mystrncatNull( lineStr, LANG_LIST_SEP, LANG_LINE_SIZE ); 
		mystrncatNull( lineStr, TranslateID(i), LANG_LINE_SIZE ); 
		AddItemToListView( hWndLangBuilderListView, ListView_GetItemCount(hWndLangBuilderListView), 3, lineStr, LANG_LIST_SEP );
	}
	ListView_GetItemText( hWndLangBuilderListView, 0, EDIT_LANGUAGE_LISTVIEW_POS, lineStr, LANG_LINE_SIZE );
	SetWindowText( GetDlgItem( hDlg, IDC_LANGTOKEN_TEXT), lineStr );
	SetFocus( GetDlgItem( hDlg, IDC_LANGTOKEN_LIST ) );
	int state = LVIS_SELECTED|LVIS_FOCUSED;
	if ( hWndLangBuilderListView )
		ListView_SetItemState( hWndLangBuilderListView, 0, state, state );
}

void DisplayCurrListItemInTextBox( int sel )
{
	char lineStr[LANG_LINE_SIZE];
	ListView_GetItemText( hWndLangBuilderListView, sel, EDIT_LANGUAGE_LISTVIEW_POS, lineStr, LANG_LINE_SIZE );
	SetWindowText( GetDlgItem( hDlg, IDC_LANGTOKEN_TEXT), lineStr );
}

void SelectFoundTextInList( long item, long pos, long scrollOffset )
{
	int state = LVIS_SELECTED|LVIS_FOCUSED;
	ListView_SetItemState( hWndLangBuilderListView, pos, state, state );
	currItemSel = pos;
	RECT rc;
	ListView_GetItemRect( hWndLangBuilderListView, pos, &rc, LVIR_LABEL );
	int rowHeight = rc.bottom-rc.top;
	int scrollTo;
	int count = ListView_GetItemCount( hWndLangBuilderListView );
	if ( item > 0 )
		scrollTo = (pos-item+scrollOffset+1) * rowHeight;
	else 
		scrollTo = (pos-item-count+scrollOffset+1) * rowHeight;
	ListView_Scroll( hWndLangBuilderListView, 0, scrollTo );
	DisplayCurrListItemInTextBox( pos );
}

short CheckLangStringForTextPrev( char *searchText, char *lineStr1, char *lineStr2, short i, int scrollOffset, int &nonRealTokenOffset, int &item ) 
{
	// See if the token number is a real language token or just a category seperator token
	if ( !RealLangToken( i ) )
		nonRealTokenOffset++;
	else // We have a real token
	{
		// Select a position in the list view to check for the search text
		int nonRealTokenOffsetPrev = 0;
		int realTokens = 0;
		for ( short j = SUMM_BEGIN; j < item-nonRealTokenOffsetPrev; j++ )
		{
			if ( !RealLangToken( j ) )
				nonRealTokenOffsetPrev++;
			else
				realTokens++;

		}
		int pos = i-nonRealTokenOffsetPrev-1+item; 
		int count = ListView_GetItemCount( hWndLangBuilderListView );
		if ( pos >= count )
		{
			item -= count;
			pos = i-nonRealTokenOffsetPrev-1+item;
		}
		else if ( pos < 0 )
		{
			item += count;
			pos = i-nonRealTokenOffsetPrev-1+item;
		}
		// Get both the text in the Default English, and the current language we are searching
		ListView_GetItemText( hWndLangBuilderListView, pos, DEFAULT_ENGLISH_LISTVIEW_POS, lineStr1, LANG_LINE_SIZE );
		ListView_GetItemText( hWndLangBuilderListView, pos, EDIT_LANGUAGE_LISTVIEW_POS, lineStr2, LANG_LINE_SIZE );
		if ( strstri( lineStr1, searchText ) || strstri( lineStr2, searchText ) )
		{
			SelectFoundTextInList( item, pos, scrollOffset );
			return 1;
		}
	}
	return 0;
}

short CheckLangStringForText( char *searchText, char *lineStr1, char *lineStr2, short i, int scrollOffset, int &nonRealTokenOffset, int &item ) 
{
	// See if the token number is a real language token or just a category seperator token
	if ( !RealLangToken( i ) )
		nonRealTokenOffset++;
	else // We have a real token
	{
		// Select a position in the list view to check for the search text
		int pos = i-nonRealTokenOffset-1+item; 
		int count = ListView_GetItemCount( hWndLangBuilderListView );
		if ( pos >= count )
		{
			item -= count;
			pos = i-nonRealTokenOffset-1+item;
		}
		// Get both the text in the Default English, and the current language we are searching
		ListView_GetItemText( hWndLangBuilderListView, pos, DEFAULT_ENGLISH_LISTVIEW_POS, lineStr1, LANG_LINE_SIZE );
		ListView_GetItemText( hWndLangBuilderListView, pos, EDIT_LANGUAGE_LISTVIEW_POS, lineStr2, LANG_LINE_SIZE );
		if ( strstri( lineStr1, searchText ) || strstri( lineStr2, searchText ) )
		{
			SelectFoundTextInList( item, pos, scrollOffset );
			return 1;
		}
	}
	return 0;
}

void SearchForText( short prev )
{
	char searchText[64];
	GetWindowText( GetDlgItem( hDlg, IDC_LANGTOKEN_FINDTEXT), searchText, 64 );
	if ( searchText[0] == 0 )
		return;
	int nonRealTokenOffset = 0;
	char lineStr1[LANG_LINE_SIZE];
	char lineStr2[LANG_LINE_SIZE];

	int top = ListView_GetTopIndex( hWndLangBuilderListView ) + 1;
	int listviewsize = ListView_GetCountPerPage( hWndLangBuilderListView );
	int bottom = top + listviewsize;
	int scrollOffset = 0;
	int item;
	/*if ( currItemSel+1 < top || currItemSel > bottom ) // Selected item is out of the list view (off the screen)
	{
		if ( currItemSel < top ) // Selected item is above the view
			scrollOffset = currItemSel - top - (listviewsize/2); // We can't see the selected item, so use the top of the view as the starting point for the search
		else // if ( currItemSel > bottom )
			scrollOffset = currItemSel - top - (listviewsize/2); // 
	}
	else*/
	scrollOffset = currItemSel - top - (listviewsize/2);

	if ( prev )
	{
		short i;
		item = currItemSel-1;
		// Since we can see the selected item in the list view, we start seaching right after it...
		for ( i = END_OF_STRINGS; i > SUMM_BEGIN; i-- )
		{
			if ( CheckLangStringForTextPrev( searchText, lineStr1, lineStr2, i, scrollOffset, nonRealTokenOffset, item ) )
				break;
		}
		if ( i == SUMM_BEGIN )
		{
			sprintf( lineStr1, "The string '%s' is not in the language list", searchText );                
			MessageBox( GetFocus(), lineStr1, "String not found", MB_OK );
		}
	}
	else
	{
		short i;
		item = currItemSel+1;
		// Since we can see the selected item in the list view, we start seaching right after it...
		for ( i = SUMM_BEGIN; i < END_OF_STRINGS; i++ )
		{
			if ( CheckLangStringForText( searchText, lineStr1, lineStr2, i, scrollOffset, nonRealTokenOffset, item ) )
				break;
		}
		if ( i == END_OF_STRINGS )
		{
			sprintf( lineStr1, "The string '%s' is not in the language list", searchText );                
			MessageBox( GetFocus(), lineStr1, "String not found", MB_OK );
		}
	}
}


void InitLanguageDialog()
{
	int langRet;

	langRet = InitLanguage( MyPrefStruct.language, 0 );
	if ( langRet ) // Have defaulted to loading English
		mystrcpy( MyPrefStruct.language, ENGLISH_LANG );
	Init_LangComboBox( hDlg, IDC_LANGTOKEN_LANG );
	SetComboLanguageSel( hDlg, IDC_LANGTOKEN_LANG, MyPrefStruct.language ); 
	InitLangList();
}


void UpdateALanguageString()
{
	if ( alreadySaving )
	{
		MessageBox( GetFocus(), "You currently have the 'Save as' dialog open, please close it before updating a string.", "Warning!"/*ReturnString(IDS_MAKEDIR)*/, MB_OK|MB_ICONWARNING );
		return;
	}
	char lineStr[LANG_LINE_SIZE];
	char currlineStr[LANG_LINE_SIZE];
	GetWindowText( GetDlgItem( hDlg, IDC_LANGTOKEN_TEXT), lineStr, LANG_LINE_SIZE );
	ListView_GetItemText( hWndLangBuilderListView, currItemSel, EDIT_LANGUAGE_LISTVIEW_POS, currlineStr, LANG_LINE_SIZE );
	if ( strcmp( currlineStr, lineStr ) )
	{
		changedSomeStrings = 1;
		ListView_SetItemText( hWndLangBuilderListView, currItemSel, EDIT_LANGUAGE_LISTVIEW_POS, lineStr );
	}
}

extern "C" int gSaved;

void ChangeLanguageStringInComboBox()
{
	int langRet;
	if ( changedSomeStrings )
	{
		if ( MessageBox( GetFocus(), "Some strings have been changed!\nAre you sure you want to load another language without saving the current language?", "Unsaved language strings"/*ReturnString(IDS_MAKEDIR)*/, MB_YESNO|MB_ICONWARNING ) == IDNO ){
			return;
		}
	}
	LanguageString_GUItoData( hDlg, IDC_LANGTOKEN_LANG, MyPrefStruct.language );
	langRet = InitLanguage( MyPrefStruct.language, 1 );
	if ( langRet ) // Have defaulted to loading English
		mystrcpy( MyPrefStruct.language, ENGLISH_LANG );
	InitLangList();
	SetComboLanguageSel( hDlg, IDC_LANGTOKEN_LANG, MyPrefStruct.language );
	gSaved = FALSE;
}

void ExitLanguageDialog()
{
	if ( alreadySaving )
	{
		MessageBox( GetFocus(), "You currently have the 'Save as' dialog open, please close it before exiting.", "Warning!"/*ReturnString(IDS_MAKEDIR)*/, MB_OK|MB_ICONWARNING );
		return;
	}

	if ( changedSomeStrings )
	{
		if ( MessageBox( GetFocus(), "Some strings have been changed!\nAre you sure you want to exit without saving?", "Continue Exiting?"/*ReturnString(IDS_MAKEDIR)*/, MB_YESNO|MB_ICONWARNING ) == IDYES ){
			EndDialog(hDlg, TRUE);
		}
	}
	else
		EndDialog(hDlg, TRUE);
}

void SaveLanguageToFile( char *filename )
{
	if ( alreadySaving )
		return;

	alreadySaving = 1;
	if ( SaveLangFileAs( filename ) )
	{
		changedSomeStrings = 0;
		Init_LangComboBox( hDlg, IDC_LANGTOKEN_LANG );
		SetComboLanguageSel( hDlg, IDC_LANGTOKEN_LANG, MyPrefStruct.language ); 
		ChangeLanguageStringInComboBox();
	}
	alreadySaving = 0;
}

#define LANG_NEXT 0
#define LANG_PREV 1

extern "C" void DeleteLocalEditClass();
WNDPROC lpfnEditClassProc = NULL;

short pressedReturnInEdit = 0;
LRESULT CALLBACK LocalEdit_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	static int msgs[10] = { 0,0,0,0,0,0,0,0,0,0 };
//	static int wParams[10] = { 0,0,0,0,0,0,0,0,0,0 };
	short ret = 0;
	static int lastCharMsg = 0;

	if ( msg == 135 ) // spose to be WM_CHAR type of message, but for "return" chars etc...
	{
		if ( (TCHAR) wParam == '\r' ) // Return key pressed 
		{
			//OutDebug( "Return!!!" );
			//OutDebugs( "Msg = %d, Wparam = %d", msg, wParam );
			//for ( int i = 0; i < 10; i++ )
			//	OutDebugs( "Msg%d = %d, Wparam%d = %d", i, msgs[i], i, wParams[i] );
			pressedReturnInEdit = 1;
			// if we are currently in the "Find" edit box, then
			if ( hwnd == GetDlgItem( hDlg, IDC_LANGTOKEN_FINDTEXT ) )
			{
				SearchForText( LANG_NEXT );
				ret = 1;
			}
			else if ( hwnd == GetDlgItem( hDlg, IDC_LANGTOKEN_TEXT ) )
			{
				UpdateALanguageString();
				ret = 1;
			}
		}
		else if ( (TCHAR) wParam == VK_F3 && lastCharMsg != 82 )
		{
			//OutDebug( "F3" );
			//OutDebugs( "Msg = %d, Wparam = %d", msg, wParam );
			//for ( int i = 0; i < 10; i++ )
			//	OutDebugs( "Msg%d = %d, Wparam%d = %d", i, msgs[i], i, wParams[i] );
			SearchForText( LANG_NEXT );
			ret = 1;
		}
		else if ( (TCHAR) wParam == VK_F3 && lastCharMsg == 82 )
		{
			//OutDebug( "R!!!" );
			//OutDebugs( "Msg = %d, Wparam = %d", msg, wParam );
			//for ( int i = 0; i < 10; i++ )
			//	OutDebugs( "Msg%d = %d, Wparam%d = %d", i, msgs[i], i, wParams[i] );
			ret = 0;
		}
//		OutDebug( "" );
		lastCharMsg = wParam;
	}

/*	msgs[0] = msg;
	for ( int i = 8; i >= 0; i-- )
		msgs[i+1] = msgs[i];
	wParams[0] = wParam;
	for ( i = 8; i >= 0; i-- )
		wParams[i+1] = wParams[i];*/
	if ( ret )
		return ret;
	return CallWindowProc(lpfnEditClassProc, hwnd, msg, wParam, lParam);
}

BOOL CreateLangGuiLocalEditClass()
{
   WNDCLASS wc;

   if ( lpfnEditClassProc == NULL ) {
      GetClassInfo(NULL, "Edit", &wc);
      lpfnEditClassProc = (WNDPROC) wc.lpfnWndProc;
      wc.lpfnWndProc = LocalEdit_WndProc;
      wc.lpszClassName   = "Edit";
      wc.hInstance = hInst;
      if (!RegisterClass(&wc))
         return FALSE;

      return TRUE;
   }
   return FALSE;
}

void DeleteLangGuiLocalEditClass()
{
   if ( lpfnEditClassProc != NULL ) {
      UnregisterClass("Edit", hInst);
      lpfnEditClassProc = NULL;
   }
}


LRESULT CALLBACK LangBuilderProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	hDlg = hWnd;

	static int show = 0;
	if ( show )
		OutDebugs( "LangBuilderProc: Message = %d, Wparam = %d, Lparam = %d", message, wParam, lParam );
	switch (message)
	{
        case WM_DESTROY:
			DestroyWindow( hWndLangBuilderListView );
            break;
        case WM_INITDIALOG:
			InitLanguageDialog();
			return 0;
		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;
		case WM_COMMAND:
			switch( LOWORD(wParam) ){
					break;
				case IDC_LANGTOKEN_UPDATE:
					UpdateALanguageString();
					break;
				case IDC_LANGTOKEN_FIND:
					SearchForText( LANG_NEXT );
					break;
				// Not working correctly, too hard to do... case IDC_LANGTOKEN_FINDPREV: 
				//	SearchForText( LANG_PREV );
				//	break;
				case IDC_LANGTOKEN_LANG:
					if ( HIWORD(wParam) == CBN_SELCHANGE )
						ChangeLanguageStringInComboBox();
					break;
				case IDC_LANGTOKEN_EXIT:
					ExitLanguageDialog();
					break;
				case IDC_LANGTOKEN_SAVEAS:
					char currLangName[64];
					if ( pressedReturnInEdit ) {
						pressedReturnInEdit = 0;
						return 1;
					}
					GetWindowText( GetDlgItem( hDlg, IDC_LANGTOKEN_LANG), currLangName, 64 );
					SaveLanguageToFile( currLangName );
					break;
			}
			break;

		case WM_NOTIFY:
			if ( LOWORD(wParam) == IDC_LANGTOKEN_LIST && ((LV_DISPINFO *)lParam)->hdr.code == LVN_ITEMCHANGING )
			{
				currItemSel = ((LV_DISPINFO *)lParam)->item.mask;
				DisplayCurrListItemInTextBox( currItemSel );
			}
			else if ( LOWORD(wParam) == IDC_LANGTOKEN_LIST && ((LV_DISPINFO *)lParam)->item.cchTextMax == VK_F3 )
				SearchForText( LANG_NEXT );
			break;
	}
    return 0;
}

void ShowEditLangGuiDialog( HWND hWnd )
{
	CreateLangGuiLocalEditClass();
	DialogBox( hInst, "IDD_LANGBUILDER", hWnd, (DLGPROC)LangBuilderProc );
	DeleteLangGuiLocalEditClass();
}
