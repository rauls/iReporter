/*
	File:		util.c

	Contains:	Utilities for reading preferences and custom open dialogs

	Written by:	Darren Williams

	Copyright:	© 1996 by Interactive Concepts, all rights reserved.

	Change History (most recent first):

	To Do:      Its all in my head for safe keeping under lock and key



*/

#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <windows.h>
#include <shlobj.h>
#include <commctrl.h>   // includes the common control header
#include <io.h>			// for write()

#include "myansi.h"

#include "Winmain.h"
#include "WinSerialReg.h"
#include "Winutil.h"
#include "Winnet_io.h"

#include "config.h"
#include "log_io.h"
#include "net_io.h"
#include "schedule.h"
#include "resource.h"
#include "version.h"
#include "postproc.h"
#include "GlobalPaths.h"
#include "FileTypes.h"
#include "ResDefs.h"	// for ReturnString() etc
#include "DateFixFileName.h"

extern long PerformWebStatistics( char **filenames, int num );


////////////////////////////////////////////////////////////////////////////
//
// File Open/Save handling routines
//
////////////////////////////////////////////////////////////////////////////



static    OPENFILENAME    gFileNameStruct;








void AddItemToListView( HWND hwin, int index, long columns, char* sepData, char *sep )
{
	LV_ITEM lvI; 
	int iSubItem;       // Index for inserting sub items
	char cpyOfData[256], *p;
	short sepLen;

	lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	lvI.state = 0;      //
	lvI.stateMask = 0;  //LVIS_SELECTED 

	lvI.iItem = index;
	lvI.iSubItem = 0;
	lvI.pszText = LPSTR_TEXTCALLBACK;
	
	lvI.cchTextMax = 256;
	lvI.iImage = 0;
	ListView_InsertItem(hwin, &lvI);

	if( sepData )
	{
		mystrncpyNull( cpyOfData, sepData, 256 );
	}

	p = cpyOfData;
	sepLen = mystrlen( sep );
	for (iSubItem = 0; iSubItem <= columns; iSubItem++)
	{
		if ( sepData ){
			lvI.pszText = p;
			p = mystrstr( p, sep );
			if ( p ){
				if ( *p ){ 
					*p = 0;
					p += sepLen;
				}
			}
		}
		ListView_SetItemText( hwin,	index, iSubItem, lvI.pszText );
	}
	ListView_RedrawItems( hwin, index, index );
}

void SelectListViewItem( HWND hwin, long index )
{
		ListView_SetItemState( hwin, index,	LVIS_SELECTED|LVIS_FOCUSED  , 0xf  );
		ListView_RedrawItems( hwin,  index , index );
		ListView_Scroll( hwin, 0, 9999 );
}





static COLORREF acrCustClr[16]; // array of custom colors 
long ChooseRGBColor( HWND hWnd, DWORD rgbCurrent )
{
	CHOOSECOLOR cc;                 // common dialog box structure 
	long		RGB;

	RGB = SwapEndian( rgbCurrent );

	// Initialize CHOOSECOLOR
	ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hWnd;
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = RGB;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	if (ChooseColor(&cc)==TRUE) {
		//hbrush = CreateSolidBrush(cc.rgbResult);
		rgbCurrent = SwapEndian( cc.rgbResult );
		return rgbCurrent;
	} else return -1;
}


/****************************************************************************
*
*    FUNCTION: FileOpenNotify( HWND hDlg, LPOFNOTIFY pofn)
*
*    PURPOSE: This function processes the WM_NOTIFY message that is sent
*		to the hook dialog procedure for the File Open common dialog.
*
*    COMMENTS:
*
*
****************************************************************************/
BOOL NEAR PASCAL FileOpenNotify(HWND hDlg, LPOFNOTIFY pofn)
{
	static char lpszNotification[128];

	switch (pofn->hdr.code)
	{
		// The selection has changed. 
		case CDN_SELCHANGE:
		{
			char szFile[MAX_PATH];

			// Get the file specification from the common dialog.
			CommDlg_OpenSave_GetSpec(GetParent(hDlg),
				szFile, sizeof(szFile));
			wsprintf(lpszNotification, "File Open Notification: %s. File: %s",
				"CDN_SELCHANGE", szFile);
		}
		break;

		// A new folder has been opened.
		case CDN_FOLDERCHANGE:
		{
			char szFile[MAX_PATH];

			if (CommDlg_OpenSave_GetFolderPath(GetParent(hDlg),
				szFile, sizeof(szFile)) <= sizeof(szFile))
			{
				wsprintf(lpszNotification, "File Open Notification: %s. File: %s",
					"CDN_FOLDERCHANGE", szFile);
			}
		}
		break;

		// The "Help" pushbutton has been pressed.
		case CDN_HELP:
			wsprintf(lpszNotification, "File Open Notification: %s.",
				"CDN_HELP");
			break;

		// The 'OK' pushbutton has been pressed.
		case CDN_FILEOK:
			SetWindowLong(hDlg, DWL_MSGRESULT, 1L);
			wsprintf(lpszNotification, "File Open Notification: %s. File: %s",
				"CDN_FILEOK", pofn->lpOFN->lpstrFile);
			break;

		// Received a sharing violation.
		case CDN_SHAREVIOLATION:
			wsprintf(lpszNotification, "File Open Notification: %s.",
				"CDN_SHAREVIOLATION");
			break;

		case CDN_INITDONE:
			wsprintf(lpszNotification, "File Open Notification: %s.",
				"CDN_INITDONE");
			break;

		case CDN_TYPECHANGE:
			wsprintf(lpszNotification, "File Open Notification: %s.",
				"CDN_TYPECHANGE");
			break;
	}

	// write the notification out the the status window.
	StatusSet( lpszNotification );

	return(TRUE);
}

/****************************************************************************
*
*    FUNCTION: FileOpenHookProc(HWND, UINT, UINT, LONG)
*
*    PURPOSE:  Processes messages for GetFileNameOpen() common dialog box
*
*    COMMENTS:
*
*        This function will prompt the user if they are sure they want
*        to open the file if the OFN_ENABLEHOOK flag is set.
*
*        If the current option mode is CUSTOM, the user is allowed to check
*        a box in the dialog prompting them whether or not they would like
*        the file created.  If they check this box, the file is created and
*        the string 'Empty' is written to it.
*
*    RETURN VALUES:
*        TRUE - User chose 'Yes' from the "Are you sure message box".
*        FALSE - User chose 'No'; return to the dialog box.
*
****************************************************************************/

BOOL APIENTRY FileOpenHookProc(
        HWND hDlg,                /* window handle of the dialog box */
        UINT message,             /* type of message                 */
        UINT wParam,            /* message-specific information    */
        LONG lParam)
{

    TCHAR szTempText[256];
    TCHAR szString[256];
	
	switch (message) {
		case WM_CREATE:
        case WM_INITDIALOG:
			CenterWindowOnScreen( hDlg );
			break;
		case WM_NOTIFY:
			FileOpenNotify(hDlg, (LPOFNOTIFY)lParam);
			break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
				//UpdateWindow( hwndParent );
				UpdateWindow( ghMainDlg );

                GetDlgItemText( hDlg, edt1, szTempText,sizeof( szTempText ) - 1);

                if ( gFileNameStruct.Flags & OFN_PATHMUSTEXIST ){
                    sprintf( szString, ReturnString(IDS_MSG_OPENYESNO),szTempText);
                    if ( MessageBox( hDlg, szString, ReturnString(IDS_INFO), MB_YESNO ) == IDYES )
                        break;
                    return (TRUE);
                }
            }
            break;
    }
    return (FALSE);
}

void GetPersonalDataPath( char *out )
{
	char	szAppData[MAX_PATH];
	int		err;
	LPITEMIDLIST ppidl;
	LPMALLOC ppMalloc;

	err = SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &ppidl );
	if ( err==NOERROR && out ){
		SHGetMalloc( &ppMalloc );
		if( SHGetPathFromIDList( ppidl, szAppData ) ){
			mystrcpy( out, szAppData );
		}
		//ErrorMsg( out);
	} else
		mystrcpy( out, gPath );
}



void GetApplicationDataPath( char *out )
{
	char	szAppData[MAX_PATH];
	int		err=0;
	LPITEMIDLIST ppidl=NULL;
	LPMALLOC ppMalloc;

	err = SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &ppidl );
	if ( err != NOERROR )
	err = SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &ppidl );

	if ( err==NOERROR && out ){
		SHGetMalloc( &ppMalloc );
		if( SHGetPathFromIDList( ppidl, szAppData ) ){
			sprintf( out, "%s\\iReporter Software", szAppData );
			if( GetFileAttributes( out ) == (unsigned long)0xffffffff ){
				CreateDirectory( out, NULL );
			}
		}
	} else {
		mystrcpy( out, gPath );
	}
}


BOOL SelectFolder( char *out, char *initDir, char *title )
{
	BROWSEINFO bi;
	LPITEMIDLIST rs;

	memset( &bi, 0, sizeof( bi ) );
	bi.pszDisplayName = out;
	bi.lpszTitle = title;
	rs = SHBrowseForFolder( &bi );
	if ( rs ){
		SHGetPathFromIDList( rs, out );
		strcat( out, "\\" );
		return TRUE;
	} else
		return FALSE;
}



/*
With Windows 2000, the OPENFILENAME structure has increased to include some additional members.
 However, this causes problems for applications on previous operation systems. To use the current
 header files for applications on Windows 95/98 and Windows NT 4.0, either use the #define 
 "/D_WIN32_WINNT=0x0400" or use OPENFILENAME_SIZE_VERSION_400 for the lStructSize member of OPENFILENAME.

*/
BOOL GetLoadDialog( char *putNamehere, char *initDir, char *initfile, char *filter, char *title )
{
    char            szFile[256], szFileTitle[256], path[256], *cptr;
    BOOL			bSuccess;

    bSuccess = TRUE;

	if ( !initDir ){
		initDir = gPersonalPath;
	}

	ZeroMemory( &gFileNameStruct, sizeof(OPENFILENAME));
    mystrcpy( szFile, initfile );
    gFileNameStruct.lStructSize = sizeof(OPENFILENAME);
    gFileNameStruct.hwndOwner = hwndParent;
    gFileNameStruct.lpstrFilter = filter;
    gFileNameStruct.lpstrCustomFilter = (LPSTR) NULL;
    gFileNameStruct.nMaxCustFilter = 0L;
    gFileNameStruct.nFilterIndex = 1;
    gFileNameStruct.lpstrFile = szFile;
    gFileNameStruct.nMaxFile = sizeof(szFile);
    gFileNameStruct.lpstrFileTitle = szFileTitle;
    gFileNameStruct.nMaxFileTitle = sizeof(szFileTitle);
    gFileNameStruct.lpstrInitialDir = initDir;
    gFileNameStruct.lpstrTitle = title;
    gFileNameStruct.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOVALIDATE;
    gFileNameStruct.nFileOffset = 0;
    gFileNameStruct.nFileExtension = 0;
    gFileNameStruct.lpstrDefExt = "LOG";

	GetCurrentDirectory( 256, path );
    if (!GetOpenFileName(&gFileNameStruct)){
        return 0L;
	}
	SetCurrentDirectory( path );

	if ( cptr = strstr( szFile, "ftp://" ) )
		mystrcpy( putNamehere, cptr );
	else
	if ( cptr = strstr( szFile, "http://" ) )
		mystrcpy( putNamehere, cptr );
	else
		mystrcpy( putNamehere, szFile );

	return bSuccess;
}












int ExpandMultiFiles( char *src, char *out, long max )
{
	char	*dir, *file, *outp;
	int		flen, fileNum=0, totalsize=0;

	dir = src;
	flen = strlen( dir );
	file = src + flen + 1;
	flen = strlen( file );

	// one entry in the dialog
	if ( flen == 0 ){
		fileNum = 0;
		if ( file=strstr( dir,"ftp://" ) )
			dir = file;

		if ( (file = IsURL( dir )) ){
			mystrcpy( out, dir );
		} else {
			if ( file=strstr( dir,"http://" ) )
				dir = file;
			mystrcpy( out, dir );
		}
		return 1;
	} else
	// multiple files in the dialog
	{
		char	*newfilename;
		*out = 0;

		newfilename = (char*)malloc( 1024 );
		outp = out;

		while( flen ){
			fileNum++;
			totalsize += sprintf( newfilename, "%s\\%s", dir, file );
			totalsize++;

			if ( totalsize+10>max )
				break;
			outp += sprintf( outp, newfilename );
			file = file + flen + 1;
			flen = strlen( file );
			if ( flen )
				outp += sprintf( outp, "," );
		}
		free( newfilename );
	}

	return fileNum;
}


int GetMultiLoadDialog( char *putNamehere, char *initDir, char *initfile, char *filter, char *title, long max )
{
    char        *szText, szFileTitle[256], path[256];
    int			bSuccess;

    bSuccess = TRUE;

	if ( !initDir ){
		initDir = gPersonalPath;
	}
	szText = (char*)malloc( max+4 );
	if ( szText && max ){
		*szText = 0;
		ZeroMemory( &gFileNameStruct, sizeof(OPENFILENAME));
		mystrcpy( putNamehere, initfile );
		gFileNameStruct.lStructSize = sizeof(OPENFILENAME);
		gFileNameStruct.hwndOwner = hwndParent;
		gFileNameStruct.lpstrFilter = filter;
		gFileNameStruct.lpstrCustomFilter = (LPSTR) NULL;
		gFileNameStruct.nMaxCustFilter = 0L;
		gFileNameStruct.nFilterIndex = 1;
		gFileNameStruct.lpstrFile = szText;
		gFileNameStruct.nMaxFile = max;
		gFileNameStruct.lpstrFileTitle = szFileTitle;
		gFileNameStruct.nMaxFileTitle = sizeof(szFileTitle);
		gFileNameStruct.lpstrInitialDir = initDir;
		gFileNameStruct.lpstrTitle = title;
		gFileNameStruct.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOVALIDATE;
		gFileNameStruct.nFileOffset = 0;
		gFileNameStruct.nFileExtension = 0;
		gFileNameStruct.lpstrDefExt = "LOG";

		GetCurrentDirectory( 256, path );
		if (!GetOpenFileName(&gFileNameStruct)){
			return 0L;
		}
		SetCurrentDirectory( path );

		bSuccess = ExpandMultiFiles( szText, putNamehere, max );

		free( szText );
	}
	return bSuccess;
}









#define	MAXRETURNSIZE		10000

/* MultiFilename LOG selection dialog */
long GetLogFileNames( char *initDir, char *szFile )
{
    char            szFileTitle[256], path[256];
    static char     *szFilter;
    BOOL			bSuccess;

	memset( szFile, 0, MAXRETURNSIZE );
    bSuccess = TRUE;

	if ( !initDir ){
		initDir = gPersonalPath;
	}

	GetString( IDS_SELECTLOG, szFileTitle, 255 );
	szFilter =
	  "Log files (*log*)\0*log*\0WWW Log files (*.lg)\0*.lg\0Compact database files (*.fdb)\0*.fdb\0Extended database files (*.fxdb)\0*.fxdb\0Text files (*.txt)\0*.txt\0Gz files (*.gz)\0*.gz\0Bzip2 files (*.bz2)\0*.bz2\0All files (*.*)\0*.*\0\0";

	ZeroMemory( &gFileNameStruct, sizeof(OPENFILENAME));
	mystrcpy(szFile, "*log*\0\0");
	gFileNameStruct.lStructSize = sizeof(OPENFILENAME);
	gFileNameStruct.hwndOwner = hwndParent;
	gFileNameStruct.lpstrFilter = szFilter;
	gFileNameStruct.lpstrCustomFilter = (LPSTR) NULL;
	gFileNameStruct.nMaxCustFilter = 0L;
	gFileNameStruct.nFilterIndex = 1;
	gFileNameStruct.lpstrFile = szFile;
	gFileNameStruct.nMaxFile = MAXRETURNSIZE;
	gFileNameStruct.lpstrFileTitle = szFileTitle;
	gFileNameStruct.nMaxFileTitle = sizeof(szFileTitle);
	gFileNameStruct.lpstrInitialDir = initDir;
	gFileNameStruct.lpstrTitle = szFileTitle;
	gFileNameStruct.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOVALIDATE;
	gFileNameStruct.nFileOffset = 0;
	gFileNameStruct.nFileExtension = 0;
	gFileNameStruct.lpstrDefExt = "LOG";

	GetCurrentDirectory( 256, path );
	if (!GetOpenFileName(&gFileNameStruct)){
			//wsprintf( szFileTitle, "err=%d", CommDlgExtendedError() );
			//ErrorMsg( szFileTitle );
		return 0L;
	}
	SetCurrentDirectory( path );
	return bSuccess;
}



BOOL GetSaveDialog( char *putNamehere, char *defaultfile, char *dir, char *filter, char *title, char *defaultExt, long defaultFilterIndex )
{
    char            szFile[512], szFileTitle[256], path[256];
    static char     *szFilter;

	if ( !dir )
	{
		dir = gPersonalPath;
	}

    mystrcpy(szFile, defaultfile);
    gFileNameStruct.lStructSize = sizeof(OPENFILENAME);
    gFileNameStruct.hwndOwner = hwndParent;
    gFileNameStruct.lpstrFilter = filter;
    gFileNameStruct.lpstrCustomFilter = (LPSTR) NULL;
    gFileNameStruct.nMaxCustFilter = 0L;
    gFileNameStruct.nFilterIndex = defaultFilterIndex;
    gFileNameStruct.lpstrFile = szFile;
    gFileNameStruct.nMaxFile = 511;
    gFileNameStruct.lpstrFileTitle = szFileTitle;
    gFileNameStruct.nMaxFileTitle = sizeof(szFileTitle);
    gFileNameStruct.lpstrInitialDir = dir;
    gFileNameStruct.lpstrTitle = title;
    gFileNameStruct.Flags = 0L;
    gFileNameStruct.nFileOffset = 0;
    gFileNameStruct.nFileExtension = 0;
    gFileNameStruct.lpstrDefExt = defaultExt;

	GetCurrentDirectory( 256, path );

    BOOL bSuccess( GetSaveFileName(&gFileNameStruct) );

	// if we didn't succeed in choosing a filename
    if( !bSuccess )
	{
		long err( CommDlgExtendedError() );
		
		// if a fair dinkum error occured and user didn't just hit cancel
		if( err )
		{
			gFileNameStruct.lpstrInitialDir=NULL;	// reset start dir and try again

			// if we didn't succeed (again) and a fair dinkum error occured and user didn't just hit cancel
			if( !(bSuccess=GetSaveFileName(&gFileNameStruct)) && (err=CommDlgExtendedError())!=0 )
			{
				switch ( err )
				{
					case FNERR_BUFFERTOOSMALL :
						ErrorMsg( "The buffer pointed to by the lpstrFile member of the OPENFILENAME structure is too small" );
						break;

					case FNERR_INVALIDFILENAME :
						ErrorMsg( "A filename is invalid" );
						break;

					case FNERR_SUBCLASSFAILURE :
						ErrorMsg( "An attempt to subclass a list box failed because sufficient memory was not available." );
						break;

					default:
						sprintf( szFile, "Can't open file dialog, error = %08lx", err );
						ErrorMsg( szFile );
						break;
				}
			}
		}
	}

    if( bSuccess )
	{
		SetCurrentDirectory( path );
		mystrcpy( putNamehere, szFile );
	}

	return bSuccess;
}




BOOL GetSaveLogName( char *putNamehere ) 
{
	char     *szFilter;

    szFilter =
      "Path (*.*)\0*.*\0\0\0";

	return GetSaveDialog( putNamehere, "temp.log\0", 0, szFilter, ReturnString(IDS_SELECTSAVELOG), "log", 1 );
}

BOOL GetSaveDomainName( char *putNamehere ) 
{
	char     *szFilter;

    szFilter =
      "Path (*.*)\0*.*\0\0\0";

	return GetSaveDialog( putNamehere, "index.html\0", 0, szFilter, ReturnString(IDS_SELECTDOMAIN), "html", 1 );
}



BOOL GetSavePrefsName( char *putNamehere ) 
{
    char     szFile[256], szPath[256];
    char     *szFilter="Settings files (*.conf)\0*.conf\0Settings files (*.cfg)\0*.cfg\0\0";

	FileFromPath( putNamehere, szFile );
	PathFromFullPath( putNamehere, szPath );
 	return GetSaveDialog( putNamehere, szFile, szPath, szFilter, ReturnString(IDS_SELECTSAVE), "conf", 1 );
}

BOOL GetSaveFolderName( char *putNamehere ) 
{
    char	szFile[256], szPath[256];
	char	*szFilter, *szExt;
	long	iFilterIndex = 1;

    szFilter =
      "HTML Files (*.html)\0*.html\0RTF Files (*.rtf)\0*.rtf\0CSV Files (*.csv)\0*.csv\0PDF Files (*.pdf)\0*.pdf\0TXT Files (*.txt)\0*.txt\0Excel Files (*.xls)\0*.xls\0\0";

	FileFromPath( putNamehere, szFile );
	PathFromFullPath( putNamehere, szPath );

	szExt = strrchr( putNamehere, '.' );
	if ( szExt )
		szExt++;
	else
		szExt = "html";

	if ( !strcmpd( "html",szExt ) )		iFilterIndex = 1;
	if ( !strcmpd( "rtf", szExt ) )		iFilterIndex = 2;
	if ( !strcmpd( "csv", szExt ) )		iFilterIndex = 3;
	if ( !strcmpd( "pdf", szExt ) )		iFilterIndex = 4;
	if ( !strcmpd( "txt", szExt ) )		iFilterIndex = 5;
	if ( !strcmpd( "xls", szExt ) )		iFilterIndex = 6;

 	return GetSaveDialog( putNamehere, szFile, szPath, szFilter, ReturnString(IDS_SELECTOUTPUT), szExt, iFilterIndex );
    //ofn.lpstrDefExt = "HTML";
}


BOOL GetSaveLogListName( char *putNamehere ) 
{
    char     szFile[256], szPath[256];
	char     *szFilter;

    szFilter =
      "Log List Files (*.lst)\0*.lst\0TXT Files (*.txt)\0*.txt\0\0";

	FileFromPath( putNamehere, szFile );
	PathFromFullPath( putNamehere, szPath );
 	return GetSaveDialog( putNamehere, szFile, szPath, szFilter, ReturnString(IDS_SELECTSAVELOGLIST), "lst", 1 );
    //ofn.lpstrDefExt = "HTML";
}

BOOL GetOpenLogListName( char *putNamehere )
{
	char fil[] = "Log List Files (*.lst)\0*.lst\0TXT Files (*.txt)\0*.txt\0All files (*.*)\0*.*\0\0";
	char szPath[256];

	PathFromFullPath( gPrefsFilename , szPath );

	return GetLoadDialog( putNamehere, szPath, "*.lst\0\0" , fil, ReturnString(IDS_SELECTLOADLOGLIST) );
}

BOOL GetOpenSkinsName( char *putNamehere )
{
	char fil[] = "Skin files (*.skin)\0*.skin\0All files (*.*)\0*.*\0\0";
	char szPath[256];

	PathFromFullPath( gPrefsFilename , szPath );

	return GetLoadDialog( putNamehere, szPath, "*.skin\0\0" , fil, ReturnString(IDS_SELECTSKIN) );
}


BOOL GetOpenPrefsName( char *putNamehere )
{
	char fil[] = "Settings files (*.conf)\0*.conf\0Settings files (*.cfg)\0*.cfg\0All files (*.*)\0*.*\0\0";
	char szPath[256];

	PathFromFullPath( gPrefsFilename , szPath );

	return GetLoadDialog( putNamehere, szPath, "*.conf\0\0" , fil, ReturnString(IDS_SELECTLOAD) );
}


int GetLogFileName( char *putNamehere, char *initDir, long max )
{
	char fil[] = "Log files (*.log)\0*.log\0WWW Log files (*.lg)\0*.lg\0Compact database files (*.db)\0*.db\0Extended database files (*.xdb)\0*.xdb\0Text files (*.txt)\0*.txt\0Gz files (*.gz)\0*.gz\0Bzip2 files (*.bz2)\0*.bz2\0All files (*.*)\0*.*\0\0";

	return GetMultiLoadDialog( putNamehere, initDir, "*.log" , fil, ReturnString(IDS_SELECTLOG), max );
}



void SetSaveFolder( char *fullName )
{
	char	path[255];

	PathFromFullPath( fullName, path );
	SetCurrentDirectory( path );
}








////////////////////////////////////////////////////////////////////////////
//
// Preferences handling routines
//
////////////////////////////////////////////////////////////////////////////


// read x file into y data of n size






int GetFile( char *fileName, char *initDir, long max )
{
	FILE 	*fp;
	long total;

	if ( total = GetLogFileName( fileName, initDir, max ) ){
		if ( IsURL( fileName ) ){
			return TRUE;
		}
		if ( total == 1 )
		{
			if ( (fp = fopen( fileName, "rb" )) ) {
				fclose( fp );
				return TRUE;
			} else {
				ErrorMsg( "GetFile() Cannot open log file" );
				return FALSE;
			}
		}
		return TRUE;
	} else {
		return FALSE;
	}
}




short OpenLog( char *outfilename, long max )
{
	char logsPath[256], *lpath=NULL;
	if ( GetReg_Prefs( "logspath", logsPath ) )
		lpath = logsPath;

	if ( !outfilename ) outfilename = glogFilename;
	if ( GetFile( outfilename,lpath, max ) ) {
		if ( !IsURL( outfilename ) ){
			strcpyuntil( logsPath, outfilename, ',' );
			PathFromFullPath( logsPath, NULL );
			SetReg_Prefs( "logspath", logsPath );
		}
		UpdateProgressBar(0);
		return(1);
	} else {
		UpdateProgressBar(0);
		return(0);
	}
}







// -------------------------------------------------------
// add a bunch of log files to the internal Queue
// -------------------------------------------------------
long AddWildCardFilenames( char *szTempFile, long start )
{
	WIN32_FIND_DATA fd; 
	HANDLE hFind; 
	int nNext = start;
	static int level=0;

	if ( szTempFile )
	{
		int 	fFound=FALSE,lastn;
		char	newPath[512], firstFile[512];

		level++;
		DateFixFilename( szTempFile, firstFile );
		hFind = FindFirstFile(firstFile,&fd); 
		if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;

		while ( fFound )  {
			if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ){
				CopyFilenameUsingPath( newPath, firstFile, fd.cFileName );
				lastn = nNext;

				if( !IsPrefsFile( newPath ) ){
					nNext = AddFileToLogQ( newPath, nNext );
					if ( lastn == nNext ){
						fFound = FALSE;
						continue;
					}
				}
			} else {
				// Ignore the . dir and use the other dirs.
				if ( level < 3 && strcmpd( ".", fd.cFileName ) ){
					CopyFilenameUsingPath( newPath, firstFile, fd.cFileName );
					strcat( newPath, "\\*" );
					nNext = start = AddWildCardFilenames( newPath, start );
				}
			}
			fFound = FindNextFile( (HANDLE)hFind, &fd );
		}
		FindClose(hFind);
	}
	level--;
	return nNext;  
} /* end AddWildCardFilenames() */






/*---------------------------------------------------------------
  Add all files in X directory to the internal file report list, thats
  used to decide which files get uploaded
 */
long AddWildCardReportFiles( const char *szTempFile )
{
	WIN32_FIND_DATA fd; 
	HANDLE hFind; 
	int nFilesDone = 0;
	static int level=0;

	if ( szTempFile )
	{
		int 	fFound=FALSE, iLength;
		char	newPath[512], firstFile[512];

		StopFopenHistory();
		StartFopenHistory();

		level++;
		DateFixFilename( (char *)szTempFile, firstFile );

		iLength = strlen( firstFile );
		if ( firstFile[iLength-1] == '\\' )
			strcat( firstFile, "*.*" );

		hFind = FindFirstFile(firstFile,&fd); 
		if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;

		while ( fFound )  
		{
			if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				CopyFilenameUsingPath( newPath, firstFile, fd.cFileName );

				if( !IsPrefsFile( newPath ) )
				{
					AddFileHistory( newPath );
					nFilesDone++;				
				}
			}
			fFound = FindNextFile( (HANDLE)hFind, &fd );
		}
		FindClose(hFind);
	}
	level--;
	return nFilesDone;  
}






long AddWildCardFilenamesToHistory( char *szTempFile, long start )
{
	WIN32_FIND_DATA fd; 
	HANDLE hFind; 
	int nNext = start;
	static int level=0;

	if ( szTempFile )
	{
		int 	fFound=FALSE, lastn;
		char	newPath[512], firstFile[512];

		level++;
		DateFixFilename( szTempFile, firstFile );
		hFind = FindFirstFile(firstFile,&fd); 
		if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;

		while ( fFound )  {
			if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ){
				CopyFilenameUsingPath( newPath, firstFile, fd.cFileName );
				lastn = nNext;

				if( !IsPrefsFile( newPath ) ){
					nNext++;
					AddLogToHistory( newPath );
					if ( lastn == nNext ){
						fFound = FALSE;
						continue;
					}
				}
			} else {
				if ( level < 3 && strcmpd( ".", fd.cFileName ) ){
					CopyFilenameUsingPath( newPath, firstFile, fd.cFileName );
					strcat( newPath, "\\*" );
					start = AddWildCardFilenamesToHistory( newPath, start );
				}
			}
			fFound = FindNextFile( (HANDLE)hFind, &fd );
		}
		FindClose(hFind);
	}
	level--;
	return nNext;  
} /* end AddWildCardFilenamesToHistory() */







// convert a list of files 
// "C:\this\dir\file file2 file3 file4"

int AddMultiFilesToHistory( char *src )
{
	char	*dir, *file;
	int		flen, fileNum=0;

	dir = src;
	flen = strlen( dir );
	file = src + flen + 1;
	flen = strlen( file );

	// one entry in the dialog
	if ( flen == 0 ){
		fileNum = 0;
		if ( file=strstr( dir,"ftp://" ) )
			dir = file;

		if ( (file = IsURL( dir )) ){
			fileNum = AddWildcardFtpDirToHistory( dir, fileNum );
		} else {
			if ( file=strstr( dir,"http://" ) )
				dir = file;
			if ( mystrchr( dir, '*' ) ){
				fileNum = AddWildCardFilenamesToHistory( dir, 0 );
			} else {
				fileNum++;
				AddLogToHistory( dir );
			}
		}
		return fileNum;
	} else
	// multiple files in the dialog
	{
		char	newfilename[256];
		while( flen ){
			wsprintf( newfilename, "%s\\%s", dir, file );
			fileNum++;
			AddLogToHistory( newfilename );

			file = file + flen + 1;
			flen = strlen( file );
		}
	}
	return fileNum;
}




short Dialog_SelectLogs( void )
{
	char logsPath[256], *lpath=NULL, *szFile;
	//char szFile[8048];
	int total;

	if ( szFile = (char*)malloc( MAXRETURNSIZE+8 ) ){
		if ( GetReg_Prefs( "logspath", logsPath ) )
			lpath = logsPath;

		//glogFilenames[0]=NULL;
		if ( GetLogFileNames( lpath,szFile ) ){
			DeSelectLogHistory();
			total = AddMultiFilesToHistory( szFile );
		}

		// remember the output path
		if ( total  ){
			if ( !IsURL( szFile ) ){
				strcpyuntil( logsPath, szFile, ',' );
				PathFromFullPath( logsPath, NULL );
				SetReg_Prefs( "logspath", logsPath );
			}
		}
		free( szFile );
		UpdateProgressBar(0);
	}

	return total;
}



void GetControlRect( HWND hwnd, HWND objectWnd, RECT *rc )
{
	RECT winrc;

	winrc.left = 0;
	winrc.top = 0;
	// Dunno if this does anything important??? ClientToScreen( hwnd, &winrc );

	GetWindowRect( objectWnd, rc ); 

	//GetWindowRect( hwnd, rc ); 

	rc->top -= (winrc.top);
	rc->bottom -= (winrc.top);
	rc->right -= winrc.left;
	rc->left -= winrc.left;
}



void GetObjectRect( HWND hwnd, HWND objectWnd, RECT *rc )
{
	RECT winrc;

	GetWindowRect( hwnd, &winrc );
	GetWindowRect( objectWnd, rc );

	rc->top -= (winrc.top + GetSystemMetrics( SM_CYSIZE ));
	rc->bottom -= (winrc.top + GetSystemMetrics( SM_CYSIZE ));
	rc->right -= winrc.left;
	rc->left -= winrc.left;
}



void GetObjectCoord( HWND hwnd, HWND objectWnd, RECT *rc )
{
	RECT winrc;

	GetWindowRect( hwnd, &winrc );
	GetWindowRect( objectWnd, rc );

	rc->top -= winrc.top;
	rc->bottom -= winrc.top;
	rc->right -= winrc.left;
	rc->left -= winrc.left;
}

extern int	processing_http_request;
extern void HttpOutputError( const char* errorString );


long WinErrorMsgEx( const char *txt, long type )
{
	long ret=0;

	if ( txt ) {
		if ( strlen( txt ) ){
			if( processing_http_request ){
				HttpOutputError(txt);
			} else {
				if ( !gNoGUI && !gProcessingSchedule ){
					switch( type ){
						case 0: ret = MessageBox ( GetFocus(), txt, szAppName, MB_OK|MB_ICONINFORMATION); break;
						case 1: ret = MessageBox ( GetFocus(), txt, szAppName, MB_YESNO|MB_ICONWARNING); break;
						case 2: ret = MessageBox ( GetFocus(), txt, szAppName, MB_OK|MB_ICONERROR); break;
					}
				} else {
					if ( gNoGUI )
						write( 1, txt, strlen(txt) );
				}
				if ( gProcessingSchedule )
					LogScheduleTxt( txt, 0l );
				OutDebug( txt );
			}
		}
	}
	return ret;
}                             


long WinNotifyMsg( const char *txt )
{
	return WinErrorMsgEx( txt, 0 );
}                             


long WinCautionMsg( const char *txt )
{
	return WinErrorMsgEx( txt, 1 );
}                             

long WinErrorMsg( const char *txt )
{
	return WinErrorMsgEx( txt, 2 );
}                             


long CheckReportFreeSpace( void )
{
	long kbfree;
	kbfree = (long)(GetDiskFree( MyPrefStruct.outfile ) / 1024);
	if ( kbfree < 200 && kbfree>= 0 )
		return 1;
	else
		return 0;
}


extern "C" long serialID;
#include "winnet_io.h"
#include "version.h"

void _tempy() {
	calloc(NULL,0);;
	rewind(NULL);
}

DWORD WINAPI CheckforAppUpdate( PVOID lpParam )
{
	void	*hnd;
	char	ver[320];
	long	length;
	__int64	len;
	double	version, current;

	memset( ver, 0, sizeof(ver));
	length = sprintf( ver, "http://www.ac3dec.com/winver.html?" );

	strncpy( ver+length, (char*)&serialID, 4 );

	hnd = (void*)INetOpen( ver, &len );
	if ( hnd ){
		NetRead( hnd, ver, 32 );
		INetClose( hnd );

		version = atof( ver );
		current = CURRENT_PREF_VERSION;
	}
	return 0;
}



HANDLE 		ghCheckThread = NULL;

long CheckforUpdate( void )
{
	DWORD dwThreadId, dwThrdParam = 1;

	if ( ghCheckThread )
		CloseHandle( ghCheckThread );

	ghCheckThread = CreateThread( 
		NULL,                        // no security attributes 
		0,                           // use default stack size  
		CheckforAppUpdate,     // thread function 
		&dwThrdParam,                // argument to thread function 
		0,                           // use default creation flags 
		&dwThreadId);                // returns the thread identifier  
    
	if (ghCheckThread) {
		SetThreadPriority( ghCheckThread, THREAD_PRIORITY_BELOW_NORMAL );
	}
	return(1);
}




void DownloadAndUpdateEXE( void )
{
}

void InstallUpdate( void )
{
}

/*
#include <ole2.h>
#include <shlobj.h>
#include <objidl.h>
HRESULT ResolveShortCut(HWND hwnd, LPCSTR lpszLinkFile, LPSTR lpszPath)
{ 
    HRESULT hres;
	IShellLink* psl;
	char szGotPath[MAX_PATH]; 
    char szDescription[MAX_PATH];
	WIN32_FIND_DATA wfd; 

    *lpszPath = 0; // assume failure  
	CoInitialize(NULL);
    // Get a pointer to the IShellLink interface. 
    hres = CoCreateInstance( &CLSID_ShellLink, NULL, 
            CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl); 
    if (SUCCEEDED(hres)) {
		IPersistFile* ppf;  
        // Get a pointer to the IPersistFile interface. 
        hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);
		if (SUCCEEDED(hres)) { 
            WORD wsz[MAX_PATH];  
            // Ensure that the string is Unicode. 
            MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz,  MAX_PATH);              // Load the shortcut. 
            hres = ppf->lpVtbl->Load(ppf, wsz, STGM_READ); 
            if (SUCCEEDED(hres)) {                  // Resolve the link. 
                hres = psl->lpVtbl->Resolve(psl, hwnd, SLR_ANY_MATCH); 
                if (SUCCEEDED(hres)) {  
                    // Get the path to the link target. 
                    hres = psl->lpVtbl->GetPath(psl, szGotPath, 
                        MAX_PATH, (WIN32_FIND_DATA *)&wfd, 
                        SLGP_SHORTPATH ); 
                    if (!SUCCEEDED(hres) )
                        HandleErr(hres); // application-defined function  
                    // Get the description of the target. 
                    hres = psl->lpVtbl->GetDescription(psl, 
                        szDescription, MAX_PATH); 
                    if (!SUCCEEDED(hres)) 
                        HandleErr(hres); 
                    lstrcpy(lpszPath, szGotPath);
				} 
            }         // Release the pointer to the IPersistFile interface. 
        ppf->lpVtbl->Release(ppf);
		} 
    // Release the pointer to the IShellLink interface. 
    psl->lpVtbl->Release(psl);
	}
	CoUninitialize();
	return hres;
} 

*/
/* -=------------------------ TEMP STUFF ---------------------------
SortCompVDomain(const void *p1, const void *p2)
{
	char *domain1 = ((VDptr)p1)->domainName;
	char *domain2 = ((VDptr)p2)->domainName;

	return( strcmpd(domain1,domain2) );
}


				WriteToWebreportFile( out2, html_frm_list );
				realVDnum=myVDnum=0;
				VDptr	*VDptrs[MAXDOMAINS];

				for( VDcount=0; VDcount <= VDnum; VDcount++) {
					VDptr = VD[ VDcount ];
					VDptrs[ VDcount ] = VDptr;
				}
				qsort( VDptrs, VDnum, sizeof(void*), SortCompVDomain );
  
				for( VDcount=0; VDcount <= VDnum; VDcount++) {
					VDptr = VDptrs[ VDcount ];
					if ( VDptr->totalRequests ) {
						realVDnum++;
						sprintf( buf2, "<A HREF=\"%s/%s\" target=\"report\">%s</A><BR>\n", VDptr->domainName, filename1, VDptr->domainName );
						WriteToWebreportFile( out2, buf2 );
					}
				}
*/


#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")


#include "gd.h"

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

// Converts the temp.bmp image to final format image
int Convert_Temp_Image(char *lpszFilename, char type)
{
    WCHAR wString[MAX_PATH];  
    // Ensure that the string is Unicode. 
    MultiByteToWideChar(CP_ACP, 0, lpszFilename, -1, wString,  MAX_PATH);              // Load the shortcut. 

   // Initialize GDI+.
   GdiplusStartupInput gdiplusStartupInput;
   ULONG_PTR gdiplusToken;
   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
   CLSID   encoderClsid;
   Status  stat;
   Image*   image = new Image(L".temp.bmp");
   // Get the CLSID of the PNG encoder.
   GetEncoderClsid(L"image/png", &encoderClsid);
   stat = image->Save(wString, &encoderClsid, NULL);
   delete image;
   GdiplusShutdown(gdiplusToken);

   if(stat == Ok)
      return 0;
   else
      return stat;
}
