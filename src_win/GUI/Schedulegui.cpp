/****************************************************************************
*
*
*    PROGRAM: Scheduleing GUI
*
*    PURPOSE: 
*
****************************************************************************/



#include "FWA.h"
#include <windows.h>    // includes basic windows functionality
#include <commctrl.h>   // includes the common control header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Local Header Files

#include "Myansi.h"
#include "config.h"
#include "datetime.h"
#include "PrefsGUI.h"
#include "Schedule.h"

#include "Winmain.h"
#include "Winutil.h"
#include "ScheduleGUI.h"
#include "PrefsGUI.h"
#include "FileViewGUI.h"

#include "resource.h"
#include "ResDefs.h"	// for ReturnString() etc
#include "FileTypes.h"
#include "GlobalPaths.h"
#include "DateFixFileName.h"

extern int IsServiceInstalled( long flag );

/****************************************************************************
* 
*    Global and External variables
*
****************************************************************************/


#define	MYWAIT_EVENT	101


static	long	gItemSel = 1;
static	HWND	hWndSchListView;
static	HWND	hWndScheduleWnd;
static	HWND	hWndScheduleEditWnd = NULL;
struct	App_config SchedulePrefStruct;		// schedule settings

LRESULT ScheduleNotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ShowScheduleEditDialog( ScheduleRecPtr schP );


/* --------------------------------------------------



	Proc dialog stuff here



  ----------------------------------------------------*/







//#    ,Name        ,LogFile     ,Active,Start Date/Time  ,Repeat  ,Next Schedule    ,Settings File              

#define	LISTVIEW_X	9
#define	LISTVIEW_Y	150		//176
#define	LISTVIEW_W	440		//472
#define	LISTVIEW_H	116		//132
#define	SCHNUM_COLUMNS	8
#define	NUM_ITEMS	3


HWND InitSchListView (HWND hWndParent, long itemNum )                                     
{
	LV_COLUMN lvC;      // List View Column structure
	HWND hWndList;      // Handle to the list view window
	char szText[MAX_PATH];    // Place to store some text
	char szIconNames[MAX_PATH], *scheduleColStr[10];
	int index;          // Index used in for loops

	hWndList = GetDlgItem( hWndParent, IDC_SCHED_LISTRECT );

	GetString( IDS_SCH_HEADERS, szIconNames, MAX_PATH-1 );
	Init_StringArray( szIconNames, scheduleColStr, 10 );

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
	for (index = 0; index < SCHNUM_COLUMNS; index++)
	{
		lvC.cx = strlen(scheduleColStr[index])*7;            // width of the column, in pixels
		lvC.iSubItem = index;
		strcpy( szText, scheduleColStr[index] );
		if (ListView_InsertColumn(hWndList, index, &lvC) == -1){
			ErrorMsg( "ListView_InsertColumn() == -1" );
			return NULL;
		}
	}

	index = LVS_EX_FULLROWSELECT;// | LVS_EX_CHECKBOXES ;
	ListView_SetExtendedListViewStyleEx( hWndList, index , index );

	// Finally, let's add the actual items to the control
	// Fill in the LV_ITEM structure for each of the items to add
	// to the list.
	// The mask specifies the the .pszText, .iImage, .lParam and .state
	// members of the LV_ITEM structure are valid.
	for (index = 0; index < itemNum; index++){
		AddItemToListView( hWndList, index, SCHNUM_COLUMNS, 0, LIST_VIEW_SEP );
	}

	return (hWndList);
}








void SetReportFileName( HWND hDlg, char *prefsFilename )
{
	if ( GetFileLength( prefsFilename ) )
	{
		SetText( IDC_SCHED_REPORTFILE, SchedulePrefStruct.outfile );
	}
	else
	{
		SetText( IDC_SCHED_REPORTFILE, ReturnString(IDS_SCH_NOSETTINGS) );
	}
}


void ViewReportFile( HWND hDlg )
{
	char fileName[MAX_PATH];

	GetText( IDC_SCHED_REPORTFILE, fileName, MAX_PATH );
	DateFixFilename( fileName, NULL );

	if ( GetFileLength( fileName ) == 0) {
		OutDebugs( "could not find file %s", fileName );
		MsgBox_Error( IDS_MSG_NOREPORT );
	} else {
		ShowHTMLURL( hDlg, fileName, NULL );
	}
}

void ScheduleToGUI( int number )
{
	ScheduleRecPtr	schP;

	if ( !hWndScheduleEditWnd ) return;

	if ( schP = GetSchedule( number ) )
	{
		char			szText[MAXLOGLENGTH+MAX_PATH];
		HWND hDlg		= hWndScheduleEditWnd;

		sprintf( szText, "Schedule Properties #%d", number );
		SetWindowText( hWndScheduleEditWnd, szText );
		if ( schP->active )
			CheckON( IDC_SCHED_ACTIVE );
		else
			CheckOFF( IDC_SCHED_ACTIVE );

		SetText( IDC_SCHED_NAME, schP->title );
		strcpy( szText, schP->prefsfile );
		SetText( IDC_SCHED_PREFSFILE, szText );

		CommaToMultiline( schP->logfile, szText, MAXLOGLENGTH );
		SetText( IDC_SCHED_LOGFILE, szText );
		SetReportFileName( hDlg, schP->prefsfile );
		SetText( IDC_SCHED_UPLOAD, schP->upload );

		DaysDateToString( schP->startdate, szText, GetDateStyle(), '/', 1,1);
		SetText( IDC_SCHED_DATE, szText );

		CTimetoString( schP->startdate, szText );
		SetText( IDC_SCHED_TIME, szText );

		sprintf( szText, "%d", schP->repeatCount );
		SetText( IDC_SCHED_NUM, szText );
		SetPopupNum( IDC_SCHED_NUMTYPE ,  schP->repeatType );
	}
}




void RedrawListView( HWND w )
{
	ListView_Update( w , gItemSel-1 );
}



void RunonceAllSchedules( void )
{
	long i, tot = GetTotalSchedule();
	ScheduleRecPtr	schP;

	for(i=1; i<=tot ; i++){
		schP = GetSchedule( i );
		schP->runonce = 1;

	}
}



void ScheduleGUIToData( HWND hDlg, long control, int number )
{
	ScheduleRecPtr	schP;

	if ( schP = GetSchedule( number ) ){
		struct tm nowtime;
		char	numStr[MAXLOGLENGTH+100];
		long	now = GetCurrentCTime();

		switch( control ){
			case IDC_SCHED_NAME:
				GetText( IDC_SCHED_NAME, schP->title, SCH_STRINGMAX );
				break;

			case IDC_SCHED_ACTIVE :
				if ( IsChecked( IDC_SCHED_ACTIVE ) )
					schP->active = TRUE;
				else
					schP->active = FALSE;
				break;

			case IDC_SCHED_NUM :
				GetText( IDC_SCHED_NUM, numStr, 12 );
				if ( strlen( numStr ) )
					schP->repeatCount = atoi( numStr );
				if ( schP->repeatCount < 0 )
					schP->repeatCount = 1;
				break;

			case IDC_SCHED_NUMTYPE :
				schP->repeatType = SendMessage( GetDlgItem(hDlg, IDC_SCHED_NUMTYPE ), CB_GETCURSEL , 0, 0 );
				if ( schP->repeatType < 0 || schP->repeatType > 4 )
					schP->repeatType = 1;
				break;

			case IDC_SCHED_DATE :
			case IDC_SCHED_TIME :
				GetText( IDC_SCHED_DATE, numStr ,12 );
				if ( strlen( numStr ) >= 8 )
				{
					schP->startdate = DateStringToCTime( numStr );
					if ( schP->startdate < (now-(ONEDAY*500)) || schP->startdate > (now+(ONEDAY*365)) )
						schP->startdate = now;
				}

				GetText( IDC_SCHED_TIME, numStr ,12 );
				if ( strlen( numStr ) == 8 )
				{
					StringToDaysTime( numStr, &nowtime);
					now = Time2CTime( &nowtime,0 );
					schP->startdate += now;
				}
				break;

			case IDC_SCHED_PREFSFILE :
				GetText( IDC_SCHED_PREFSFILE, schP->prefsfile , SCH_STRINGMAX );
				SetReportFileName( hDlg, schP->prefsfile );
				break;
			case IDC_SCHED_LOGFILE :
				GetText( IDC_SCHED_LOGFILE, numStr ,MAXLOGLENGTH );
				MultilineToComma( numStr, schP->logfile, SCH_STRINGMAX );
				break;
			case IDC_SCHED_UPLOAD :
				GetText( IDC_SCHED_UPLOAD, schP->upload, SCH_STRINGMAX );
				break;
		}		
		ClearSchedule( 0 );
		//RedrawListView( hWndSchListView );
	}
}








void UpdateLiveTime( HWND hDlg )
{
	struct tm nowtime;
	time_t	nowJD;
	char	szText[64];
	char	szoldText[64];

	GetCurrentDate( &nowtime );
	Date2Days( &nowtime, &nowJD );
	//nowJD += (Time2JD( &nowtime,0 ));
	sprintf( szText, "%s: ", ReturnString( IDS_TIME ) );
	DateTimeToString( nowJD, szText+strlen(szText) );
	GetText( IDC_SCHED_CURRENTTIME, szoldText, 64 );
	if ( strcmpd( szText, szoldText ) )
		SetText( IDC_SCHED_CURRENTTIME, szText );
}



static HMENU	hMenu;

#include <commctrl.h>

LRESULT ScheduleNotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
	NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
	int				itemSel = pNm->iItem;
static
	char			szText[MAX_PATH]="(null)\0";    // Place to store some text
	char			*onoffStr[]= { "Off", "On",0 };
	char			szIconNames[MAX_PATH];
	char			*numTypeStr[8] = { NULL };
	ScheduleRecPtr	schP;

	if ( numTypeStr[0] == NULL ){
		GetString( IDS_TIMER, szIconNames, MAX_PATH );
		Init_StringArray( szIconNames, numTypeStr, 8 );
	}

	if (LOWORD(wParam) != IDC_SCHED_LISTRECT)
		return 0L;

	switch(pLvdi->hdr.code)	{
		case LVN_GETDISPINFO:
			itemSel = pLvdi->item.iItem;
			if ( schP = GetSchedule( itemSel + 1 ) ){
				pLvdi->item.pszText = szText;

				ListView_SetCheckState( hWndSchListView, itemSel, schP->active );
				
				//SendMessage( hWndSchListView, LVM_SETITEMSTATE , itemSel, xxx );
				switch (pLvdi->item.iSubItem){
					case 0:		
								sprintf( szText, "%d", itemSel+1 ); break;
					case 1:		strcpy( szText, schP->title ); break;
					case 2:		FileFromPath( schP->logfile, szText ); break;
					case 3:		if ( schP->active ) strcpy( szText, onoffStr[1]); else
													strcpy( szText, onoffStr[0]);
								break;
					case 4:		CTimeToDateTimeStr( schP->startdate, szText ); break;
					case 5:		sprintf( szText, "%d %s", schP->repeatCount, numTypeStr[schP->repeatType] );
								break;
					case 6:		if ( !schP->nextRun )
									ComputeNextSchedule( schP );
								CTimeToDateTimeStr( schP->nextRun, szText );
								break;

					case 7:		FileFromPath( schP->prefsfile, szText ); break;
					default:	break;
				}

			}
			break;

		case NM_DBLCLK:
			gItemSel = itemSel + 1;
			schP = GetSchedule( gItemSel );
			if ( schP )
				ShowScheduleEditDialog( schP );
			break;
			
		case NM_CLICK:
			if ( itemSel == -1 ){
				LVHITTESTINFO pinfo;

				pinfo.pt = pNm->ptAction;
				itemSel = SendMessage( hWnd, LVM_SUBITEMHITTEST , 0, (long)&pinfo );
			}
			if ( itemSel != -1 && hWndScheduleEditWnd && !gProcessingSchedule  ){
				gItemSel = itemSel + 1;
				if ( schP = GetSchedule( gItemSel ) ){
					DoReadPrefsFile( schP->prefsfile,  &SchedulePrefStruct );
					ScheduleToGUI( gItemSel );
				}
			}
            break;

		case NM_RCLICK:
			{
			    POINT	pnt;

				GetCursorPos( &pnt );
				gItemSel = itemSel + 1;
				if (hMenu) {
					TrackPopupMenu (GetSubMenu (hMenu, 0), 0, pnt.x, pnt.y, 0, hWnd, NULL);
				} 
			}
			break;
		case LVN_ITEMCHANGING:
            break;
		default:
			break;
	}
	return 0L;
}


void ViewScheduleLog( HWND hWnd )
{
static	char szText[MAX_PATH];

	sprintf( szText, "%s\\%s", gPath, SCHEDULE_LOGFILENAME );
	if ( GetFileLength( szText ) ){
		//ShowHTMLURL( hWnd, szText, NULL );
		StartFileView( szText );
	} else {
		sprintf( szText, "%s\\%s", gDataPath, SCHEDULE_LOGFILENAME );
		if ( GetFileLength( szText ) ){
			//ShowHTMLURL( hWnd, szText, NULL );
			StartFileView( szText );
		}
	}
}


long DeleteSelected( void )
{
	long count, item, logcount=0;
	count = ListView_GetSelectedCount( hWndSchListView );
	if ( count ){			// copy log files to global list
		item = ListView_GetNextItem( hWndSchListView, -1 , LVNI_SELECTED );
		while( item != -1 ){
			ListView_DeleteItem( hWndSchListView, item );
			DelSchedule( item+1 );
			item = ListView_GetNextItem( hWndSchListView, -1 , LVNI_SELECTED );
		}
		UpdateWindow(hWndSchListView);
	}
	return count;
}


long RunAllSelected( void )
{
	long count, item, logcount=0;

	count = ListView_GetSelectedCount( hWndSchListView );
	if ( count ){			// copy log files to global list
		ScheduleRecPtr schP;
		item = ListView_GetNextItem( hWndSchListView, -1 , LVNI_SELECTED );
		while( item != -1 ){
			schP = GetSchedule( item+1 );
			schP->runonce = 1;
			item = ListView_GetNextItem( hWndSchListView, item , LVNI_SELECTED );
		}
		UpdateWindow(hWndSchListView);
	}
	return count;
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
void	UpdateToolBar(HWND hWnd, long lControlId);

LRESULT CALLBACK ScheduleProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT res;
	ScheduleRecPtr schP;
	HWND hDlg = hWnd;
static
	HWND hPrevWnd;
	schP = GetSchedule( gItemSel );

	switch (message) {

		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_CLOSE:
			KillTimer (hWnd, MYWAIT_EVENT);
			DestroyMenu( hMenu );

			if ( hWndScheduleEditWnd ){
				//CloseWindow( hWndScheduleEditWnd );
				SendMessage( hWndScheduleEditWnd, WM_CLOSE , 0, 0 );
			}
			DestroyWindow( hWndSchListView );
			hWndSchListView = NULL;
			SetFocus( hPrevWnd );
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;
 
		case WM_DESTROY:
            break;

        case WM_INITDIALOG:
			hWndScheduleEditWnd = NULL;
			hWndScheduleWnd = hWnd;
			hPrevWnd = SetFocus( hWnd );

			hMenu = LoadMenu( hInst, "SCHED_MENU" );
			//SendMessage( GetDlgItem(hWnd, IDC_SCHED_OK), WM_SETFONT, (UINT)(hCtrlFont), TRUE);
			hWndSchListView = InitSchListView( hWnd, GetTotalSchedule() );

			if ( hWndSchListView == NULL)
				ErrorMsg( "Schedule Listview not created!" );

			if ( GetTotalSchedule() == 0 ){
				schP = AddSchedule();
				AddItemToListView( hWndSchListView, GetTotalSchedule()-1, SCHNUM_COLUMNS, 0, LIST_VIEW_SEP );
				ListView_SetCheckState( hWndSchListView, GetTotalSchedule()-1, schP->active );
			}
			if ( IsServiceInstalled(1) )
				SetWindowText( hWnd, ReturnString(IDS_SCHEDULEOFF) );

			UpdateWindow( hWndSchListView );
			RedrawListView( hWndSchListView );
			SetTimer (hWnd, MYWAIT_EVENT, 100, NULL);
			UpdateLiveTime( hWnd );
			return (TRUE);
			break;



		case WM_MOVING:
			if ( GetSystemMetrics(SM_CXFULLSCREEN) >= 1024 )
			{
				RECT    rChild;
				long newX, newY;

				if ( hWndScheduleEditWnd ){
					GetWindowRect (hWndScheduleWnd, &rChild);
					newX = rChild.right;
					newY = rChild.top;
					SetWindowPos (hWndScheduleEditWnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			break;

		case WM_TIMER:
			switch (wParam){
				case MYWAIT_EVENT:
					UpdateLiveTime( hWnd );
					break;
			}
			break;

		case WM_NOTIFY:
			//UpdateLiveTime( hWnd );
			res = ScheduleNotifyHandler(hWnd, message, wParam, lParam);
			return res;
			break;

        case WM_RBUTTONDOWN: // RightClick in windows client area...
			{
			    POINT	pnt;	

				pnt.x = LOWORD(lParam);
				pnt.y = HIWORD(lParam);
				ClientToScreen(hWnd, (LPPOINT) &pnt);
				// This is where you would determine the appropriate 'context'
				// menu to bring up. Since this app has no real functionality,
				// we will just bring up the 'Help' menu:
				//hMenu = GetSubMenu (GetMenu (hWnd), 0);
				if (hMenu) {
					TrackPopupMenu (GetSubMenu (hMenu, 0), 0, pnt.x, pnt.y, 0, hWnd, NULL);
				} 
			}
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam) ) {
				case IDM_SCHED_NEW:
				case IDC_SCHED_ADD:
					{
						schP = AddSchedule();

						CopySchedule( schP, gItemSel );

						AddItemToListView( hWndSchListView, GetTotalSchedule()-1, SCHNUM_COLUMNS, 0, LIST_VIEW_SEP );
						UpdateWindow(hWndSchListView);

						gItemSel = GetTotalSchedule();

					}
					break;

				case IDM_SCHED_DELETE:
				case IDC_SCHED_DEL:
					DeleteSelected();
					break;

				case IDM_SCHED_CLEAR:
				case IDC_SCHED_CLEAR:
					if ( MsgBox_yesnocancel( IDS_MSG_CLEARSCH, NULL ) == IDYES ) {
						while( GetTotalSchedule() ){
							ListView_DeleteItem( hWndSchListView, 0 );
							DelSchedule( 1 );
							UpdateWindow(hWndSchListView);
						}
					}
					break;

				case IDM_SCHED_TRANSCRIPT:
				case IDC_SCHED_VIEWLOG:
					ViewScheduleLog( hWnd );
					break;


				case IDM_SCHED_RUNNOW:
				case IDC_SCHED_RUNNOW:
					if( GetTotalSchedule() && !gProcessingSchedule ){
						//RunScheduleEvent( schP );
						RunAllSelected();
					}
					break;
				
				case IDM_SCHED_PROPERTIES:
				case IDC_SCHED_EDIT:
					if( GetTotalSchedule() ){
						if( !gProcessingSchedule ){
							ShowScheduleEditDialog( schP );
						} else
							StatusSetID( IDS_SCH_NOEDIT );
					} else
						StatusSetID( IDS_SCH_NOEDIT2 );
					break;

				case IDM_SCHED_DISABLETHIS:
					if ( schP )
						schP->active = 0;
					ListView_RedrawItems( hWndSchListView, 0 , GetTotalSchedule() );
					break;
				case IDM_SCHED_ENABLETHIS:
					if ( schP )
						schP->active = 1;
					ListView_RedrawItems( hWndSchListView, 0 , GetTotalSchedule() );
					break;

				case IDM_SCHED_DISABLEALL:
					ActivateAllSchedules( 0 );
					ListView_RedrawItems( hWndSchListView, 0 , GetTotalSchedule() );
					break;
				case IDM_SCHED_ENABLEALL:
					ActivateAllSchedules( 1 );
					ListView_RedrawItems( hWndSchListView, 0 , GetTotalSchedule() );
					break;

				case IDC_SCHED_DISABLE:
					if ( IsChecked( IDC_SCHED_DISABLE ) )
						ActivateAllSchedules( 0 );
					else
						ActivateAllSchedules( 1 );

					ListView_RedrawItems( hWndSchListView, 0 , GetTotalSchedule() );
					break;

				case IDC_SCHED_OK:
					SaveSchedule();
					StatusSetID( IDS_SCH_SAVED );
					EndDialog(hWnd, TRUE);
					return (TRUE);
					break;

				case IDC_SCHED_CANCEL:
					InitReadSchedule( 0 );
					StatusSetID( IDS_SCH_CANCEL );
					EndDialog(hWnd, TRUE);
					return (TRUE);
					break;
            }
            break;
	}

    return FALSE;
}






LRESULT CALLBACK ScheduleEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ScheduleRecPtr schP;
	HWND hDlg = hWnd;
static
	HWND hPrevWnd;
static	int	permit_listupdate = 1;

	schP = GetSchedule( gItemSel );

	switch (message) {
		case WM_SETCURSOR:
			UpdateToolBar(hWnd, GetWindowLong((HWND)wParam, GWL_ID ));
			break;

		case WM_CLOSE:
			hWndScheduleEditWnd = NULL;
			SetFocus( hPrevWnd );
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;
 
		case WM_DESTROY:
			SetFocus( hPrevWnd );
            break;

        case WM_INITDIALOG:
			hWndScheduleEditWnd = hWnd;
			Init_ComboBox( hWnd, IDC_SCHED_NUMTYPE, IDS_TIMER );
			permit_listupdate = 0;
			if ( schP )
				DoReadPrefsFile( schP->prefsfile,  &SchedulePrefStruct );
			ScheduleToGUI( gItemSel );
			permit_listupdate = 1;
			if ( GetSystemMetrics(SM_CXFULLSCREEN) >= 1152 )
			{
				RECT    rChild;
				long newX, newY;

				if ( hWndScheduleWnd ){
					GetWindowRect (hWndScheduleWnd, &rChild);
					newX = rChild.right;
					newY = rChild.top;
					SetWindowPos (hWnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			hPrevWnd = SetFocus( hWnd );
			return (TRUE);
			break;

		case WM_NOTIFY: {
				NMUPDOWN  *upndown = (NMUPDOWN  *)lParam;
				char		dateStr[32];
				time_t		ct;

				switch ( upndown->hdr.idFrom ){
					case IDC_SCHED_DATEADJUST:
						switch( upndown->hdr.code )	{
							case UDN_DELTAPOS:
								GetText( IDC_SCHED_DATE, dateStr ,12 );
								//ct = DateStringToCTime( dateStr );
								TimeStringToDays( dateStr, &ct );
								if ( upndown->iDelta > 0 )
									ct -= ONEDAY;
								else
									ct += ONEDAY;
								DaysDateToString( ct, dateStr, GetDateStyle(), '/', 1,1);
								SetText( IDC_SCHED_DATE, dateStr );
								break;
						}
						break;
					case IDC_SCHED_TIMEADJUST:
						switch( upndown->hdr.code )	{
							case UDN_DELTAPOS:
								{
									struct tm nowtime;

								GetText( IDC_SCHED_TIME, dateStr ,12 );
								StringToDaysTime( dateStr, &nowtime);
								ct = Time2CTime( &nowtime,0 );
								if ( upndown->iDelta > 0 )
									ct -= 60;
								else
									ct += 60;
								CTimetoString( ct, dateStr );
								SetText( IDC_SCHED_TIME, dateStr );
								}
								break;
						}
						break;
				}
			}
			break;

		case WM_COMMAND:
			//Parse the menu selections:
            switch (LOWORD(wParam) ) {
				case IDC_SCHED_EDITOK:
					if ( schP ){
						if ( !strcmpd( schP->prefsfile, gPrefsFilename ) && GetFileLength(schP->prefsfile)  ){
							CopyPrefs( &MyPrefStruct, &SchedulePrefStruct );
						}
					}
					EndDialog(hWnd, TRUE);
					return (TRUE);
					break;

				case IDC_SCHED_SELECTLOG:
					{
						char szText[MAXLOGLENGTH+MAX_PATH];
						if ( OpenLog( szText, MAXLOGLENGTH ) && schP ){
							SetForegroundWindow( hWnd );
							mystrncpyNull( schP->logfile, szText, MAXLOGLENGTH-1 );
							CommaToMultiline( schP->logfile, szText, MAXLOGLENGTH );
							SetText( IDC_SCHED_LOGFILE, szText );

							RedrawListView( hWndSchListView );
						}
					}
					break;

				case IDC_SCHED_ADDLOG:
					if ( schP )
					{
						char szText[MAXLOGLENGTH+MAX_PATH];
						if ( OpenLog( szText, MAXLOGLENGTH ) ){
							SetForegroundWindow( hWnd );
							if ( strlen(schP->logfile)<MAXLOGLENGTH ){
								if ( schP->logfile[0] )
									strcat( schP->logfile, "," );
								strcat( schP->logfile, szText );
								CommaToMultiline( schP->logfile, szText, MAXLOGLENGTH );
								SetText( IDC_SCHED_LOGFILE, szText );
							}

							RedrawListView( hWndSchListView );
						}
					}
					break;

				case IDC_SCHED_SELECTPREFS:
					if( GetTotalSchedule() && !gProcessingSchedule )
					{
						char szText[MAX_PATH];
						if ( GetOpenPrefsName( szText ) && schP ){
							SetForegroundWindow( hWnd );
							SetText( IDC_SCHED_PREFSFILE, szText );
							if ( DoReadPrefsFile( szText, &SchedulePrefStruct ) ){
								SetReportFileName( hDlg, szText );//SetText( IDC_SCHED_REPORTFILE, SchedulePrefStruct.outfile );
							}
						}
					}
					break;
				
				case IDC_SCHED_EDITPREFS:
					if( GetTotalSchedule() && schP && !gProcessingSchedule )
					{
						char schedSettingsFilename[MAX_PATH];
						GetText( IDC_SCHED_PREFSFILE, schedSettingsFilename, MAX_PATH );

						if ( GetFileLength( schedSettingsFilename ) == 0 )
							MsgBox_Error( IDS_NOSETTINGSFILE );
						else {
							if ( ShowPrefsDialog2( schedSettingsFilename )){
								StatusSetID( IDS_SCH_SAVED );
							} else
								StatusSetID( IDS_SCH_CANCEL );
							SetForegroundWindow( hWnd );
							SetReportFileName( hDlg, schP->prefsfile );
						}
					}
					if ( gProcessingSchedule )
						StatusSetID( IDS_SCH_NOEDIT );

					break;

				case IDC_SCHED_VIEW:
					ViewReportFile( hDlg );
					break;

				case IDC_SCHED_DATEADJUST:
					break;
				case IDC_SCHED_TIMEADJUST:
					break;

				case IDC_SCHED_REPORTFILE:
				case IDC_SCHED_UPLOAD:
				case IDC_SCHED_LOGFILE:
				case IDC_SCHED_PREFSFILE:
				case IDC_SCHED_TIME:
				case IDC_SCHED_DATE:
				case IDC_SCHED_NUMTYPE:
				case IDC_SCHED_NUM:
				case IDC_SCHED_ACTIVE:
				case IDC_SCHED_NAME:
					if ( GetForegroundWindow() == hWndScheduleEditWnd && permit_listupdate ){
						if( GetTotalSchedule() ){
							//OutDebug( "IDC_SCHED_CHANGE" );
							ScheduleGUIToData( hWnd, LOWORD(wParam), gItemSel );
							RedrawListView( hWndSchListView );
							//UpdateWindow( hWndSchListView );
						}
					}
					break;

            }
            break;
	}

    return FALSE;
}



void ShowScheduleEditDialog( ScheduleRecPtr schP )
{

	if ( hWndScheduleEditWnd && schP )
		ShowWindow ( hWndScheduleEditWnd, SW_SHOW );
	else {
		CreateDialog(hInst, "SCHEDULEEDIT", hWndScheduleWnd, (DLGPROC) ScheduleEditProc);
		//DialogBox(hInst, "SCHEDULEEDIT", hwndParent, (DLGPROC)ScheduleEditProc);
	}
}



void ShowScheduleDialog( void )
{
	DialogBox(hInst, "SCHEDULELIST", hwndParent, (DLGPROC)ScheduleProc);
}
