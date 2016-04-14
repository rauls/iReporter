#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "gtk/gtkwin.h"
#include "gtk/xapplication.h"

#include "main.h"
#include "config.h"
#include "resourceid.h"

#include "ResDefs.h"	// for ReturnString() etc



#include "datetime.h"
#include "config.h"
#include "schedule.h"
#include "editpath.h"


extern int		gSaved;
extern int		gProcessingSchedule;

static struct	App_config	SchedulePrefStruct;
static struct	App_config	*EditPrefPtr,
									*OrigPrefPtr;

typedef	char *	ControlID;
typedef	char *	HWND;



// ----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------- schedulegui  ----------------------------------

int InitScheduleListView( void );


static	char	*onoffStr[]= { "Off", "On",0,0 };
static	char	*numTypeStr[]= { "mins", "hrs", "days", "months", "years" ,0,0};

void ScheduleUpdateCron( void )
{
	long	i=0,status=0,t;
	ScheduleRecPtr	schP;
	
	t = GetTotalSchedule();
	for(i=0; i<t; i++){
		schP = GetSchedule( i+1 );
		if( schP->active )
			status = 1;
	}
	EnableCron( status );
}


extern void ShowHTMLURL( char *url );;

void ViewReportFile( void )
{
	char fileName[256];
	GetText( IDC_SCHED_REPORTFILE, fileName, 256 );
	if ( GetFileLength( fileName ) == 0)
		MsgBox_Error( IDS_MSG_NOREPORT );
	else
		ShowHTMLURL( SchedulePrefStruct.outfile );
}

void SetReportFileName( char *prefsFilename )
{
	if ( GetFileLength( prefsFilename ) )
	{
		SetText( IDC_SCHED_REPORTFILE, SchedulePrefStruct.outfile );
	}
	else
	{
		SetText( IDC_SCHED_REPORTFILE, "No settings found" );
	}
}


void ScheduleToGUI( long i )
{
	ScheduleRecPtr	schP;
	
	schP = GetSchedule( i );
	if( schP ){
		char	szText[4000];

		if ( schP->active )
			CheckON( IDC_SCHED_ACTIVE );
		else
			CheckOFF( IDC_SCHED_ACTIVE );

		SetText( IDC_SCHED_NAME, schP->title );
		SetText( IDC_SCHED_PREFSFILE, schP->prefsfile );

		CommaToMultiline( schP->logfile, szText, 4000 );
		SetText( IDC_SCHED_LOGFILE, szText );
		SetReportFileName( schP->prefsfile );
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


void ScheduleGUIToData( int number )
{
	ScheduleRecPtr	schP;

	if ( schP = GetSchedule( number ) ){
		struct tm nowtime;
		char	numStr[4000];
		long	now = GetCurrentCTime(), ct;

		if ( IsChecked( IDC_SCHED_ACTIVE ) )
			schP->active = TRUE;
		else
			schP->active = FALSE;

		GetText( IDC_SCHED_NAME, schP->title, 255 );

		GetText( IDC_SCHED_NUM, numStr, 12 );
		if ( strlen( numStr ) )
			schP->repeatCount = atoi( numStr );
		if ( schP->repeatCount < 0 )
			schP->repeatCount = 1;

		schP->repeatType = GetPopupNum( IDC_SCHED_NUMTYPE );
		if ( schP->repeatType < 0 || schP->repeatType > 4 )
			schP->repeatType = 1;

		GetText( IDC_SCHED_DATE, numStr ,12 );
		schP->startdate = DateStringToCTime( numStr );
		if ( schP->startdate < (now-(ONEDAY*500)) || schP->startdate > (now+(ONEDAY*365)) )
			schP->startdate = now;

		GetText( IDC_SCHED_TIME, numStr ,12 );
		StringToDaysTime( numStr, &nowtime);
		now = Time2CTime( &nowtime,0 );
		schP->startdate += now;

		GetText( IDC_SCHED_PREFSFILE, schP->prefsfile , 255 );

		GetText( IDC_SCHED_LOGFILE, numStr ,4000 );
		MultilineToComma( numStr, schP->logfile, 4000 );

		GetText( IDC_SCHED_UPLOAD, schP->upload, 252 );

		schP->nextRun = ComputeNextSchedule( schP );

	}
}






// ------------------------------------



void ScheduleGUIAddToList( long i )
{
	ScheduleRecPtr	schP;
	long	x=0;
	char	items[9][256];
	char	*itemP[9];
	char	szText[256];
	
	schP = GetSchedule( i );
	if( schP ){
		mystrcpy( items[0], schP->title );
		FileFromPath( schP->logfile, items[1] );
		strcpy( items[2], onoffStr[ schP->active ]);
		CTimeToDateTimeStr( schP->startdate, items[3] );
		sprintf( items[4] , "%d %s", schP->repeatCount, numTypeStr[schP->repeatType] );

		if( ! schP->nextRun ){
			long	currentTime = GetCurrentCTime();
			schP->nextRun = schP->startdate;
			while( currentTime > schP->nextRun )
				schP->nextRun = CalcNextSchedule( schP->nextRun, schP->repeatCount, schP->repeatType );
		}		
		CTimeToDateTimeStr( schP->nextRun, items[5] );
		FileFromPath( schP->prefsfile, items[6] );
		//items[6] = 0;

		for(x=0;x<7;x++)
			itemP[x] = items[x];
		ListView_AppendItem( "IDC_SCHED_LISTRECT", itemP );
	}
}

void ScheduleDatatoGUI( void )
{
	long	i=0,x=0;
	
	for(i=0; i<GetTotalSchedule(); i++){
		ScheduleGUIAddToList( i+1 );
		//printf( "change - %d\n", i );
	}
	ScheduleToGUI(1);
}


static int scheduleTimer=0;
void UpdateLiveTime( void *d )
{
	struct tm nowtime;
	long	nowJD;
	char	szText[64];

	GetCurrentDate( &nowtime );
	Date2Days( &nowtime, &nowJD );
	sprintf( szText, "Time: " );
	DateTimeToString( nowJD, szText+strlen(szText) );
	SetText( IDC_SCHED_CURRENTTIME, szText );
}

// save schedule to file etc...
long ScheduleGetSelected( long row )
{
	while( row>=0 ){
		char *txt;
		row = ListView_IsItSelectedItem( "IDC_SCHED_LISTRECT", row, 0, &txt );
		if ( row >= 0 ){
			return row;
		}
	}
	return -1;
}


// ----------------------------------------------







void RefreshScheduleGUI( void )
{
	InitScheduleListView();
	ScheduleDatatoGUI();
}



void ScheduleDialogAdd( void *w, void *d)
{
	ScheduleRecPtr	schP;
	schP = AddSchedule();
	ScheduleGUIToData( GetTotalSchedule() );
	ScheduleGUIAddToList( GetTotalSchedule() );
}

void ScheduleDialogDelete( void *w, void *d)
{
	long row=0;
	row = ScheduleGetSelected(0);
	if ( row >= 0 ){
		DelSchedule( row );
		RefreshScheduleGUI();
	}
}

void ScheduleDialogClear( void *w, void *d)
{
	while( GetTotalSchedule() ){
		DelSchedule( 1 );
	}
	InitScheduleListView();
}

//obselete
void ScheduleDialogChange( void *w, void *d)
{
	long row=0;
	row = ScheduleGetSelected(0);
	if ( row >= 0 ){
		ScheduleGUIToData( row );
		RefreshScheduleGUI();
		ScheduleToGUI( row + 1 );
	}
}

void ScheduleDialogTranscript(  void *w, void *d)
{
	char szText[256];
	char *home = getenv( "HOME" );
	sprintf( szText, "%s/.%s", home, SCHEDULE_LOGFILENAME );

	ViewFile( szText );
}

void ScheduleDialogRunNow( void *w, void *d)
{
	long row=0;
	ScheduleRecPtr	schP;
	while( (row = ScheduleGetSelected(row)) >= 0 ){
		schP = GetSchedule( row );
		schP->runonce = 1;
	}
}

void ScheduleDialogProperties( void *w, void *d)
{
	if( GetTotalSchedule() ){
		if( !gProcessingSchedule ){
			ShowWindow( "SCHEDULEEDIT" ); 
		} else
			StatusSetID( IDS_SCH_NOEDIT );
	} else
		StatusSetID( IDS_SCH_NOEDIT2 );
}

void ScheduleDialogDisable( void *w, void *d)
{
	if ( IsChecked( IDC_SCHED_DISABLE ) )
		ActivateAllSchedules( FALSE );
	else
		ActivateAllSchedules( TRUE );
}



// save schedule to file etc...
void ScheduleDialogOk( void *w, void *d)
{
	//memcpy( EditPrefPtr, &BackupPrefStruct, CONFIG_SIZE );
	KillWidgetTimer( scheduleTimer );
	CloseWindow( "SCHEDULELIST" );

	SaveSchedule();

	ScheduleUpdateCron();
}

// save schedule to file etc...
void ScheduleDialogCancel( void *w, void *d)
{
	KillWidgetTimer( scheduleTimer );

	// GUI to DATA
	CloseWindow( "SCHEDULELIST" );
}

// save schedule to file etc...
void ScheduleDialogPress( void *w, void *d)
{
	long row=0;
	while( row>=0 ){
		char *txt;
		row = ListView_IsItSelectedItem( "IDC_SCHED_LISTRECT", row, 0, &txt );
		if ( row >= 0 ){
			ScheduleToGUI( row );
			row = -1;
		}
	}
}









void ScheduleEditOk( void *w, void *d )
{
/*	if ( schP ){
		if ( !strcmpd( schP->prefsfile, gPrefsFilename ) && GetFileLength(schP->prefsfile)  ){
			CopyPrefs( &MyPrefStruct, &SchedulePrefStruct );
		}
	}
*/
	long row=0;

	row = ScheduleGetSelected(0);
	if ( row >= 0 ){
		// GUI to DATA
		ScheduleGUIToData( row );
	}
	RefreshScheduleGUI();

	CloseWindow( "SCHEDULEEDIT" );
}



void ScheduleSelectLog( void *w, void *d )
{
	char *file;

	file = GetFileName( "SCHEDULEEDIT", "Open log file..." );
	if ( file )
		SetText( IDC_SCHED_LOGFILE, file );
}

void ScheduleAddLog( void *w, void *d )
{
	char *file;

	file = GetFileName( "SCHEDULEEDIT", "Add log file..." );
	if ( file ) {
		char orig[4000];
		long len;

		GetText( IDC_SCHED_LOGFILE, orig, 4000 );
		len = strlen(orig);
		if ( len > 2 ){
			strcat( orig, "\r\n" );
			strcat( orig, file );
		} else
			strcpy( orig, file );

		SetText( IDC_SCHED_LOGFILE, orig );
	}
}



void ScheduleSelectPrefs( void *w, void *d )
{
	char *file;

	if( GetTotalSchedule() ) {
		file = GetFileName( "SCHEDULEEDIT", "Select settings file..." );
		if ( file ){
			SetText( IDC_SCHED_PREFSFILE, file );
			if ( DoReadAsciiPrefs( file, &SchedulePrefStruct ) ){
				SetReportFileName( file );
			} else
				ErrorMsg( "file is not a settings file" );
		}
	}
}


extern int ShowSchedulePrefsDialog( Prefs *schedulePrefs );
// save schedule to file etc...
void ScheduleEditPrefsProc( void *w, void *d)
{
	long row=0;

	row = ScheduleGetSelected(0);
	if ( row >= 0 ){
		ScheduleRecPtr	schP;

		if ( schP = GetSchedule( row ) ){
			if ( DoReadAsciiPrefs( schP->prefsfile, &SchedulePrefStruct ) ){
				StatusWindowSetText( "Edit Schedules Settings" );

				if ( ShowSchedulePrefsDialog( &SchedulePrefStruct ) ){
					StatusWindowSetText( "Schedule Edit Done." );
					DoSavePrefsFile( schP->prefsfile, &SchedulePrefStruct );
					ScheduleToGUI( row );
				} else
					StatusWindowSetText( "Schedule Edit Cancelled" );
			}
		}
	}
}


void ScheduleViewReport( void *w, void *d)
{
	ViewReportFile();
}






// ------------------------




int InitScheduleListView( void )
{
	char *listline[] = { "Name      ", "WebServer Log file", "Active", "Start Date / Time         ", "Repeat time", "Next Schedule           ", "Settings File", 0 };

	if ( ReplaceListView( "IDC_SCHED_LISTRECT", listline, 7, 0, GTK_SELECTION_EXTENDED ) ){
		AttachProcToControl( "SCHEDULELIST", IDC_SCHED_LISTRECT,  (void*)ScheduleDialogPress, "select_row", (void*)0 );
		return 0;
	} else
		return -1;
}

int InitSchedDialog( void )
{
	//return 0;
	InitScheduleListView();

	// Main List window
	AttachProcToButtonName( "SCHEDULELIST", "IDC_SCHED_ADD", (void*)ScheduleDialogAdd, 0 );
	AttachProcToButtonName( "SCHEDULELIST", "IDC_SCHED_DEL", (void*)ScheduleDialogDelete, 0 );
	AttachProcToButtonName( "SCHEDULELIST", "IDC_SCHED_RUNNOW", (void*)ScheduleDialogRunNow, 0 );
	AttachProcToButtonName( "SCHEDULELIST", "IDC_SCHED_EDIT", (void*)ScheduleDialogProperties, 0 );
	AttachProcToButtonName( "SCHEDULELIST", "IDC_SCHED_VIEWLOG", (void*)ScheduleDialogTranscript, 0 );
	AttachProcToButtonName( "SCHEDULELIST", "IDC_SCHED_DISABLE", (void*)ScheduleDialogDisable, 0 );
	AttachProcToButtonName( "SCHEDULELIST", "IDC_SCHED_OK", (void*)ScheduleDialogOk, 0 );
	AttachProcToButtonName( "SCHEDULELIST", "IDC_SCHED_CANCEL", (void*)ScheduleDialogCancel, 0 );

	// Properties Window
	AddPopupItems( IDC_SCHED_NUMTYPE, numTypeStr );
	AttachProcToButtonName( "SCHEDULEEDIT", "IDC_SCHED_SELECTLOG", (void*)ScheduleSelectLog, 0 );
	AttachProcToButtonName( "SCHEDULEEDIT", "IDC_SCHED_ADDLOG", (void*)ScheduleAddLog, 0 );
	AttachProcToButtonName( "SCHEDULEEDIT", "IDC_SCHED_SELECTPREFS", (void*)ScheduleSelectPrefs, 0 );
	AttachProcToButtonName( "SCHEDULEEDIT", "IDC_SCHED_EDITPREFS", (void*)ScheduleEditPrefsProc, 0 );
	AttachProcToButtonName( "SCHEDULEEDIT", "IDC_SCHED_VIEW", (void*)ScheduleViewReport, 0 );
	AttachProcToButtonName( "SCHEDULEEDIT", "IDC_SCHED_EDITOK", (void*)ScheduleEditOk, 0 );
	return 0;	
}

int ShowScheduleDialog( void *w, void *d)
{
	static int init=0;

	
	if ( !ShowWindow( "SCHEDULELIST" ) ) {
		EditPrefPtr = &SchedulePrefStruct;
		OrigPrefPtr = &MyPrefStruct;

		OutDebug( "copy prefs" );
		CopyPrefs( EditPrefPtr, OrigPrefPtr );

		WindowPreventResize( "SCHEDULELIST" );
		SetWindowTitle( "SCHEDULELIST", "Schedule Editor" );

		if ( !init ) {
			OutDebug( "Init schedule gui" );
			InitSchedDialog();
			ScheduleDatatoGUI();
			init = 1;
		}
		OutDebug( "SetWidgetTimer Sched" );
		scheduleTimer = SetWidgetTimer( 200, "IDC_SCHED_CURRENTTIME", (void*)UpdateLiveTime );

	} else
		printf( "Cant open schedule window \n" );
	return init;
}

