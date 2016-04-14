/****************************************************************************
*
*
*    PROGRAM: Scheduleing GUI
*
*    PURPOSE: 
*
****************************************************************************/



#include <windows.h>    // includes basic windows functionality
#include <commctrl.h>   // includes the common control header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "zlib.h"

// Local Header Files
#include "Winmain.h"
#include "Winutil.h"
#include "Myansi.h"
#include "config.h"
#include "log_io.h"

#include "resource.h"
#include "FileTypes.h"

/****************************************************************************
* 
*    Global and External variables
*
****************************************************************************/

#define	LINESTOREAD		32
#define	LINELEN			1024
#define	SCALE			30000
#define	NUMFILES		10
#define FILENAME_SIZE	128

static HWND hViewWindows[NUMFILES] = { 0,0,0,0,0, 0,0,0,0,0 };
static HMENU hMenu[NUMFILES];

static long	last_seek[NUMFILES] = { 0,0,0,0,0, 0,0,0,0,0 };
static char	fileview_name[NUMFILES][FILENAME_SIZE];

char * lineTooLongMsg = "  **** Msg: Line too long - wrapping line ****  ";
long lineTooLongMsgLen = 24; // Initialise to 24 for safety, ie less than the string 'lineTooLongMsg'.


typedef struct FileWindowDisplayStatsTag
{
	long totalLines;
	long totalLinesLen;
	long averageLineLen;
	long scrollScale;
	long averageCharsPerScreen;
	long resetScroll;
	long linesInWin;
	long fileLen;
	long readInterval;
	long prevLineFilePos;
	long nextLineFilePos;
	long prevPageFilePos;
	long nextPageFilePos;
} FileWindowDisplayStats;
static FileWindowDisplayStats winStats[NUMFILES];

long GetLastPage( char *filename, long fpos, char *data, long numlines )
{
	long fp;
	long length, len;
	long lines = 0;

	length = (long)GetFileLength( filename );

	if ( (fp = (long)gzopen( filename, "r" )) ){
		char c, *p, *start;
		char *mem;
		long count=0;
		lines = numlines;

		mem = (char*)malloc( (2+numlines)*LINELEN );
		memset( mem, 0, (2+numlines)*LINELEN );

		if ( fpos < 0 )
			fpos = 0;

		gzseek( (gzFile)fp, fpos, SEEK_SET );
		
		//OutDebugs( "In GetLastPage() at fpos %d trying to read %d bytes from %s", fpos, numlines*LINELEN, filename );
		if( (len = gzread( (gzFile)fp, mem, numlines*LINELEN ) ) )
		{
			//OutDebugs( "Read %d bytes", len );
			mem[len] = 0;
			if ( fpos == 0 ) // Last Page is the first page too, only 1 page...
				start = mem;
			else
			{
				p = mem + len -1;
				start = p;
				lines++;
				while( lines>0 && p>mem && (c=*p) ){
					if( c=='\r' || c=='\n' ){
						lines--;
						start = p+1;
						p--;
						if( *p=='\r' ) p--;
					} else
						p--;
				}
			}
			len = (mem+len) - start;
			memcpy( data, start, len+1 );
		}
		gzclose( (gzFile)fp );
		free( mem );
	}
	return length;
}

void InitWindowStats( FileWindowDisplayStats *winStatsPtr )
{
	memset( winStatsPtr, 0, sizeof(FileWindowDisplayStats) );
}

long GetPartialFileData( char *filename, long fpos, char *data, long linesWanted, short win )
{
	long fp;
	long len = 0, charCount = 0, linesGot;

	if ( linesWanted == 0 )
		linesWanted = LINESTOREAD;

	linesGot = 0;

	if ( fp = (long)gzopen( filename, "r" ) ){
		char *mem = (char*)malloc( (2+linesWanted)*LINELEN );
		char c, *p;

		memset( mem, 0, (2+linesWanted)*LINELEN );
		gzseek( (gzFile)fp, fpos, SEEK_SET );
		//OutDebugs( "In GetPartialFileData() at file pos (fpos) %d trying to read %d bytes from %s", fpos, linesWanted*LINELEN, filename );

		if( (len = gzread( (gzFile)fp, mem, linesWanted*LINELEN )) )
		{
			//OutDebugs( "Read %d bytes", len );
			mem[len] = 0;
			if ( fpos != 0 ) // If this is the first line, then we don't search for the first "return" ie, new line as we are at the begining
			{
				p = mystrchr( mem, '\r' );
				if ( !p )
					p = mystrchr( mem, '\n' );
				if ( !p )
					p = mem;
			}
			else
				p = mem;

			while( linesGot < linesWanted && (c = *p++) )
			{
				*data++ = c;
				if( c=='\r' || c=='\n' )
				{
					linesGot++;
					charCount++;
					if( *p=='\n' )
					{
						charCount++;
						*data++ = *p++;
					}
				}
				else if ( linesGot > 0 )
					charCount++;
			}
			*data++ = 0;
		}
		free( mem );
		gzclose( (gzFile)fp );
	}

	// Update the window's stats
	winStats[win].totalLines += linesGot;
	winStats[win].totalLinesLen += charCount;

	return charCount;
}



void AddToListView( HWND hlst, int index, char *text )
{
	LV_ITEM lvI; 
	char szText[LINELEN];

	lvI.mask = LVIF_TEXT | LVIF_PARAM;
	lvI.state = 0;      //
	lvI.stateMask = 0;  //LVIS_SELECTED 

	lvI.iItem = index;
	lvI.iSubItem = 0;

	lvI.pszText = szText; 
	lvI.cchTextMax = LINELEN;
	lvI.iImage = 0;
	mystrcpy( szText, text );

	ListView_InsertItem( hlst, &lvI);
	//ListView_RedrawItems( hlst,  index , index );
	//ListView_Update( hlst , index );
}


void AddLinetoFileView( HWND hDlg , char *text, int deletefirst )
{
	if ( strlen( text ) > 0 ){
		//ListDelString( IDC_VIEWLINES, 0 );
		//ListAddString( IDC_VIEWLINES, text );
		HWND hlst = GetDlgItem(hDlg, IDC_FILEVIEW);
		if ( deletefirst )
			ListView_DeleteItem( hlst, 1 );
		AddToListView( hlst, ListView_GetItemCount(hlst), text );
	}
}


long GetLinestoFit( HWND hDlg, short win )
{
	long bh,h,t;
	RECT	rc;
	long numlines;
	HWND	hctrl = GetDlgItem(hDlg, IDC_FILEVIEW);

	AddToListView( hctrl, 0, "x" );
	ListView_GetItemRect( hctrl, 0, &rc, LVIR_BOUNDS );
	ListView_DeleteItem( hctrl, 0 );

	t = ListView_GetItemCount(hctrl);
	bh = rc.bottom - rc.top;

	GetControlRect( hDlg, hctrl, &rc );
	h = rc.bottom - rc.top;

	numlines = (h/bh)-1;
	if ( numlines != winStats[win].linesInWin )
	{
		// Changed the size of the window (vertically)
		winStats[win].resetScroll = 1;
		winStats[win].linesInWin = numlines;
	}

	return numlines;
}

BOOL WINAPI EditCopy( HWND hDlg ) 
{ 
    LPTSTR  lptstrCopy; 
    HGLOBAL hglbCopy; 

    // Open the clipboard, and empty it. 
 
    if (!OpenClipboard(hDlg)) 
        return FALSE; 
    EmptyClipboard(); 
 
    if (hDlg) 
    { 
		long count, item;

		count = ListView_GetSelectedCount( GetDlgItem(hDlg, IDC_FILEVIEW) );

		if ( count ){
			char string[LINELEN];
			HWND hlist = GetDlgItem(hDlg, IDC_FILEVIEW);

	        hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (count*LINELEN) * sizeof(TCHAR));
	        lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
			if ( lptstrCopy ){
				lptstrCopy[0] = 0;
				item = ListView_GetNextItem( hlist, -1 , LVNI_SELECTED );
				while( item != -1 ){
					ListView_GetItemText( hlist, item, 0, string, LINELEN );
					strcat( lptstrCopy, string );
					strcat( lptstrCopy, "\r\n" );
					item = ListView_GetNextItem( hlist, item , LVNI_SELECTED );
				}
			}
	        GlobalUnlock(hglbCopy); 
	        SetClipboardData(CF_TEXT, hglbCopy); 
		}
    } 
    // Close the clipboard. 
 
    CloseClipboard();
 
    return TRUE; 
} 

long UpdateFileView( HWND hDlg, short win, long pos );

LRESULT KeyPressed( HWND hDlg, int key, short win )
{
	WORD listScrollCmd;
	WORD listPos;
	LPARAM listInfo;
	WPARAM viewCtrlhWnd;
	static int lastKey = 0;

	viewCtrlhWnd = (WPARAM)GetDlgItem(hDlg, IDC_VIEWSCROLL);

	switch ( key )
	{
	case VK_PRIOR:
		listScrollCmd = SB_PAGEUP;
		break;
	case VK_NEXT:
		listScrollCmd = SB_PAGEDOWN;
		break;
	case VK_UP:
		if ( lastKey == VK_SHIFT )
			return 0;
		else
			listScrollCmd = SB_LINEUP;
		break;
	case VK_DOWN:
		if ( lastKey == VK_SHIFT )
			return 0;
		else
			listScrollCmd = SB_LINEDOWN;
		break;
	case 0x43: // C Key - for copy
		if ( lastKey == VK_CONTROL )
		{
			lastKey = key;
			EditCopy( hDlg );
			return 0;
		}


	case VK_END:
		lastKey = key;
		UpdateFileView( hDlg, win, winStats[win].scrollScale );
		SetScrollPos( GetDlgItem(hDlg, IDC_VIEWSCROLL), SB_CTL, winStats[win].scrollScale, 1 );
		return 0;
	case VK_HOME:
		lastKey = key;
		UpdateFileView( hDlg, win, 0 );
		SetScrollPos( GetDlgItem(hDlg, IDC_VIEWSCROLL), SB_CTL, 0, 1 );
		return 0;
	default:
		lastKey = key;
		return 0;
	};

	lastKey = key;
	listPos = GetScrollPos( GetDlgItem(hDlg, IDC_VIEWSCROLL), SB_CTL );
	listInfo = MAKEWPARAM( listScrollCmd, listPos );

	return SendMessage( hDlg, WM_VSCROLL, listInfo, viewCtrlhWnd );
}


LRESULT FileViewNotifyHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, short win )
{
	LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
	NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
	int				itemSel = pNm->iItem;
	
	if (LOWORD(wParam) != IDC_FILEVIEW)
		return 0L;

	switch(pLvdi->hdr.code)	{
		case LVN_GETDISPINFO:
			itemSel = pLvdi->item.iItem;
			break;
		case NM_RCLICK:
			{
				POINT	pnt;
				GetCursorPos( &pnt );
				if (hMenu[win]) {
					TrackPopupMenu (GetSubMenu (hMenu[win], 0), 0, pnt.x, pnt.y, 0, hWnd, NULL);
				} 
			}
			break;
		case -155: // Dunno why this is -155, but it catches keys pressed, trying to catch messages like WM_KEYDOWN
			//OutDebugs( "Key: %d", pLvdi->item.cchTextMax );
			return KeyPressed( hWnd, pLvdi->item.cchTextMax, win );
		default:
			break;
	}
	return 0L;
}


void SetScale( char *filename, long filelen, short win, HWND hDlg )
{
	long len;
	FileWindowDisplayStats *stat;
	stat = &winStats[win];

	if ( filelen > 0 )
		len = filelen;
	else
	{
		if ( stat->fileLen )
			len = stat->fileLen;
		else
			len = (long)GetFileLength( filename );
	}


	if ( !stat->totalLines || !stat->totalLinesLen || !stat->linesInWin )
	{
		char data[50*LINELEN];
		int val = 0;
		val = GetPartialFileData( filename, 0, data, 50, win ); // Read some test the data from the file to calculate scroll stats etc.
		stat->linesInWin = GetLinestoFit( hDlg, win );
		stat->resetScroll = 1;
	}
	if ( stat->resetScroll && stat->totalLines && stat->totalLinesLen )
	{
		stat->resetScroll = 0;
		stat->averageLineLen = stat->totalLinesLen / stat->totalLines;
		stat->averageCharsPerScreen = stat->averageLineLen * stat->linesInWin;
		stat->readInterval = stat->averageLineLen;
		stat->fileLen = len;
		stat->scrollScale = (stat->fileLen / (long)stat->averageLineLen);
		if ( stat->scrollScale > SCALE )
		{
			stat->readInterval = (stat->averageLineLen * stat->scrollScale) / SCALE;
			stat->scrollScale = SCALE;
		}
		
		SetScrollRange(  GetDlgItem(hDlg, IDC_VIEWSCROLL), SB_CTL, 0, stat->scrollScale, 1 );

		//OutDebugs( "In SetScale(): totLines %d, totLinesLen %d, aver %d, Filelen %d, averageCharsPerScreen %d, scale %d", stat->totalLines, stat->totalLinesLen, stat->averageLineLen, stat->fileLen, stat->averageCharsPerScreen, stat->scrollScale );
	}

	//OutDebugs( "In SetScale(): scale set to %d for window %d", stat->scrollScale, win );
}


long UpdateFileView( HWND hDlg, short win, long pos )
{
	char	*data, *origData, *p, *d;
	char	*filename = fileview_name[win];
	long	i, len, fpos, linesdone=0;
	long totalCharCount = 0;
	long lineCharCount = 0;
	char	line[LINELEN];
	long fileLenPos = 0;
	long percentDiff;

	if ( hDlg )
	{
		long numlines = GetLinestoFit( hDlg, win );

		data = (char*)malloc( (2+numlines)*LINELEN );
		if ( !data )
			return 0;

		SetScale( filename, 0, win, hDlg );
		len = winStats[win].fileLen;

		fpos = pos * winStats[win].readInterval;
		fileLenPos = (long)( (double)len * ((double)pos / (double)winStats[win].scrollScale) );
		
		if ( fpos > 0 )
		{
			percentDiff = (long)(fileLenPos/(double)fpos) * 100;
			if ( percentDiff > 110 || percentDiff < 90 )
			{
				winStats[win].resetScroll = 1;
				SetScale( filename, 0, win, hDlg );
				fpos = pos * winStats[win].readInterval;
			}
		}
		
		if ( fpos >= len - winStats[win].averageCharsPerScreen ) // Leave a quarter of a page of text
			fpos = len - winStats[win].averageCharsPerScreen;
		//OutDebugs( "len %d, scale: %d, pos %d, fpos %d", len, scale, pos, fpos );

		if ( pos < 0 || pos*winStats[win].readInterval > len || pos >= winStats[win].scrollScale )
		{
			last_seek[win] = GetLastPage( filename, fpos, data, numlines );
		}
		else
			GetPartialFileData( filename, fpos, data, numlines, win );

		p = data;
		origData = data;

		//OutDebugs( "p ptrAddr: %0xd , p char: '%c', totalCharCount: %d", p, *p, totalCharCount );

		ListView_DeleteAllItems( GetDlgItem(hDlg, IDC_FILEVIEW) );
		for(i=0; i< numlines; i++ )
		{
			d = line;
			lineCharCount = 0;
			memset( line, 0, LINELEN );
			while( *p != '\r' && *p != '\n' && *p )
			{
				//OutDebugs( "p ptrAddr: %0xd , p char: '%c', totalCharCount: %d", p, *p, totalCharCount );
				*d++ = *p++;
				totalCharCount++;
				lineCharCount++;
				if ( lineCharCount > LINELEN-lineTooLongMsgLen-24 )
				{
					*d = 0;
					mystrcpy( d-1, lineTooLongMsg );
					d += lineTooLongMsgLen;
					break;
				}
				
			}
			*d++ = 0;

			while( *p == '\r' || *p == '\n' )
			{
				//OutDebugs( "p ptrAddr: %0xd , p char: '%c', totalCharCount: %d", p, *p, totalCharCount );
				p++;
				totalCharCount++;
			}

			if ( *line && lineCharCount )
			{
				AddLinetoFileView( hDlg, line, FALSE );
				linesdone++;
				memset( line, 0, LINELEN );
				lineCharCount = 0;
			}
		}
		free( data );
	}
	return linesdone;
}


// returns length added.
long AddNewFileView( HWND hDlg, char *filename, long pos, short win )
{
	char	line[LINELEN], *data, *p, *d;
	long	i, len=0, count=0;

	if ( hDlg ){
		long numlines = GetLinestoFit( hDlg, win );
		winStats[win].linesInWin = numlines;
		data = (char*)malloc( (2+numlines)*LINELEN );
		if ( !data )
			return 0;

		len = GetPartialFileData( filename, pos, data, 0, win );
		if ( !len )
			return 0;
		p = data;

		len = 0;
		for(i=0; i<numlines,*p; i++ ){
			d = line;
			count=0;
			while( *p != '\r' && *p != '\n' && *p && count<LINELEN ){
				*d++ = *p++;
				count++;
			}
			*d++ = 0;
			while( *p == '\r' || *p == '\n' ){
				p++;
				count++;
			}
			if ( *line ){
				AddLinetoFileView( hDlg, line, TRUE );
				len += count;
			}
		}
		free( data );
	}
	return len;
}


// get length
// if length > last_seek then
//		read new data
//		update lines with new data
void CheckFileViewStatus( HWND hWnd, short win )
{
	long len;

	if ( last_seek[win] >= 0 )
	{
		len = (long)GetFileLength( fileview_name[win] );
		if ( len > last_seek[win] )
		{
			//OutDebugs( "CheckFileViewStatus() %d", last_seek[win] );
			last_seek[win] += AddNewFileView( hWnd, fileview_name[win], last_seek[win], win );
		}
	} else
		UpdateFileView( hWnd, win, -1 );
}

#define	FILEVIEW_TIMER		1003

static short SCROLL_BAR_WIDTH = 18;
static short TITLE_BAR_HEIGHT = 20;
static short BORDER_SPACE = 11;
static short SPACING = 4;

// Dunno why I have to use these factors to make the window position correctly, but I do...
static short widthFactor = 7; 
static short heightFactor = 29; // Probably to do with the Main Window Title Height...

void SetWinSize( HWND hWnd, long w, long h )
{
	RECT obj;
	long width;
	long height;
	long x, y, wd, ht;

	// Get the new width of the main window
	GetWindowRect( hWnd, &obj );
	width = obj.right-obj.left - BORDER_SPACE - SPACING - SCROLL_BAR_WIDTH - BORDER_SPACE - widthFactor;
	height = obj.bottom-obj.top - SPACING - TITLE_BAR_HEIGHT - SPACING - BORDER_SPACE;

	// Positon the Title Bar Window - this is another title window, besides the Main Window's title... dunno why we have it really
	x = BORDER_SPACE;
	y = SPACING;
	wd = width;
	ht = TITLE_BAR_HEIGHT;
	MoveWindow( GetDlgItem(hWnd, IDC_FILEVIEWNAME), x, y, wd, ht, TRUE );

	// Position the "File View" Window (where the text is displayed)
	x = BORDER_SPACE;
	y = SPACING+TITLE_BAR_HEIGHT+SPACING;
	wd = width;
	ht = height-heightFactor;
	MoveWindow( GetDlgItem(hWnd, IDC_FILEVIEW), x, y, wd, ht, TRUE );

	// Position the Scroll Bar Window
	x = BORDER_SPACE+width+SPACING;
	y = SPACING;
	wd = SCROLL_BAR_WIDTH;
	ht = TITLE_BAR_HEIGHT + SPACING + height - heightFactor;
	MoveWindow( GetDlgItem(hWnd, IDC_VIEWSCROLL), x, y, wd, ht, TRUE );

}

			
static long	initWindowIndex = -1;

LRESULT CALLBACK FileViewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hDlg = hWnd;
	long	i=0;
	short	win=0;

	while( i < NUMFILES ){
		if ( hViewWindows[i] == hWnd ){
			win = (short)i;
			break;
		}
		i++;
	}
	i = 0;
	//filename = fileview_name[win];
//	OutDebugs( "Window %d, filename %s", win, filename );

	switch (message) {
		case WM_PAINT:{
				HDC hdc;
                PAINTSTRUCT ps;
                RECT rc;

                GetClientRect(hDlg, &rc);
				hdc = BeginPaint(hDlg, &ps);
				FillRect( hdc, &rc, GetSysColorBrush(COLOR_3DFACE) );
                EndPaint(hDlg, &ps);

				UpdateWindow( GetDlgItem(hWnd, IDC_VIEWSCROLL) );
            }
			SetText( IDC_FILEVIEWNAME, fileview_name[win] );
			break; 

		
		case WM_INITDIALOG:
			lineTooLongMsgLen = mystrlen( lineTooLongMsg );
			if ( initWindowIndex >= 0 ){
				hViewWindows[ initWindowIndex ] = hWnd;
				initWindowIndex = -1;
			}
			hMenu[win] = LoadMenu( hInst, "FILEVIEW_MENU" );
			SetTimer (hWnd, FILEVIEW_TIMER, 300, NULL);
//			OutDebugs( "Init Dialog: Window %d, filename %s", win, fileview_name[win] );
			SetText( IDC_FILEVIEWNAME, fileview_name[win] );

			EnableScrollBar( GetDlgItem(hWnd, IDC_VIEWSCROLL), SB_CTL,ESB_ENABLE_BOTH );
			OutDebug( fileview_name[win] );
			InitWindowStats( &winStats[win] );
			SetScale( fileview_name[win], 0, win, hDlg );
			SetScrollRange(  GetDlgItem(hWnd, IDC_VIEWSCROLL), SB_CTL, 0, winStats[win].scrollScale, 1 );
			ShowScrollBar( GetDlgItem(hWnd, IDC_VIEWSCROLL), SB_CTL, 1 );

			ListView_SetExtendedListViewStyleEx( GetDlgItem(hWnd, IDC_FILEVIEW), LVS_EX_FULLROWSELECT , LVS_EX_FULLROWSELECT );

			{
				long numlines, pos = winStats[win].scrollScale+1;
				numlines = GetLinestoFit( hWnd, win );
				winStats[win].linesInWin = numlines;
				i = 0;
				while( i < numlines-1 ){
					i += UpdateFileView( hDlg, win, pos-- );
					if ( pos < 0 )
						break;
				}
				SetScrollPos( GetDlgItem(hDlg, IDC_VIEWSCROLL), SB_CTL, pos, 1 );
			}
			SetFocus( GetDlgItem(hDlg, IDC_FILEVIEW) );
			break;

	    case WM_TIMER:
			if (wParam == FILEVIEW_TIMER ){
				char	title[350];
/*				long x = GetScrollPos( GetDlgItem(hWnd, IDC_VIEWSCROLL), SB_CTL );
				long startPos, endPos;
				startPos = (long)((winStats[win].fileLen/(double)winStats[win].averageLineLen) * (x/(double)winStats[win].scrollScale));
				endPos = startPos + winStats[win].linesInWin;
				sprintf( title, "Viewing - %s  (at approx. lines %d - %d)", fileview_name[win], startPos, endPos );*/
				sprintf( title, "View - %s", fileview_name[win] );
				SetWindowText( hWnd, title );
				CheckFileViewStatus( hWnd, win );
			}
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
				if (hMenu[win]) {
					TrackPopupMenu (GetSubMenu (hMenu[win], 0), 0, pnt.x, pnt.y, 0, hWnd, NULL);
				} 
			}
            break;

		case WM_CLOSE:
			if ( hMenu[win] ) DestroyMenu( hMenu[win] );
			hViewWindows[win] = 0;
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;
 
		case WM_DESTROY:
			KillTimer (hWnd, FILEVIEW_TIMER);
			break;

		case WM_NOTIFY:
			switch( wParam )
			{
				case IDC_FILEVIEW:
					return FileViewNotifyHandler(hWnd, message, wParam, lParam, win );
			}
			break;

		case WM_VSCROLL:
			{
				long x;
				long nScrollCode = (int) LOWORD(wParam); // scroll bar value 
				long nPos = (short int) HIWORD(wParam);  // scroll box position 
				HWND hwndScrollBar = (HWND) lParam;      // handle to scroll bar 

				x = GetScrollPos( GetDlgItem(hWnd, IDC_VIEWSCROLL), SB_CTL );

				//OutDebugs( "nPos: %d, x: %d, linesInWin: %d, scale: %d", nPos, x, winStats[win].linesInWin, winStats[win].scrollScale );

				switch( nScrollCode ){
					case SB_LINEDOWN:		nPos = x+1;		break;
					case SB_PAGEDOWN:		nPos = x+winStats[win].linesInWin/2;		break;
					case SB_LINEUP:			nPos = x-1;		break;
					case SB_PAGEUP:			nPos = x-winStats[win].linesInWin/2;		break;
					case SB_ENDSCROLL:
						return 1;
				}

				//SetScale( filename, 0, win, hDlg );
				if( nPos > winStats[win].scrollScale )
					nPos = winStats[win].scrollScale;
				if( nPos < 0 )
					nPos = 0;

				//OutDebugs( "pos=%d", nPos );
				UpdateFileView( hWnd, win,nPos );

				//if ( nScrollCode == SB_THUMBTRACK )
					SetScrollPos( GetDlgItem(hWnd, IDC_VIEWSCROLL), SB_CTL, nPos, 1 );
				return 0;
			}
			break;


		case WM_SIZE:
			SetWinSize( hWnd, LOWORD(lParam)-40, HIWORD(lParam)-37 );
			{
				long nPos = GetScrollPos( GetDlgItem(hWnd, IDC_VIEWSCROLL), SB_CTL );
				UpdateFileView( hWnd, win, nPos );
			}
			break;

		case WM_SIZING:
			{
				RECT *lprc = (LPRECT) lParam;
				long w,h, x,y;

				w = lprc->right - lprc->left;
				h = lprc->bottom - lprc->top;
				x = lprc->left;
				y = lprc->top;

				w = w -48;
				h = h -64;

				SetWinSize( hWnd, w, h );

			}
			break;

		case WM_COPY: 
			EditCopy(hWnd); 
			break; 

		case WM_COMMAND:
			switch (LOWORD(wParam) ) {
				case IDM_FM_COPY: 
					EditCopy(hWnd); 
					break; 
			}
			break;
		//default:
		//	return (DefWindowProc(hWnd, message, wParam, lParam));
	}

    return FALSE;
}



/* --------------------------------------------------

	Proc dialog stuff here

  ----------------------------------------------------*/


void InitFileViewDialog( char *filename )
{
	long	win = 0;

	while( hViewWindows[win] && win < NUMFILES )
		win++;

	if( win<NUMFILES ){
		mystrncpyNull( fileview_name[win], filename, FILENAME_SIZE );
		last_seek[win] = -1;

		initWindowIndex = win;
		//hWnd = CreateDialog( hInst, (LPCSTR)"FILEVIEW", 0, (DLGPROC) FileViewProc);
		DialogBox( hInst, (LPCSTR)"FILEVIEW", 0, (DLGPROC)FileViewProc );
	}
}


DWORD WINAPI StartFileViewProc( PVOID lpParam )
{
	InitFileViewDialog( (char*)lpParam );
	return 0;
}


HANDLE 		ghThread = NULL;

long StartFileView( char *filename )
{
	DWORD dwThreadId, dwThrdParam = 1;

	if ( IsGZIP( filename ) ){
		SpawnShowShellFile( filename );
		//ShellExecute( NULL, "OPEN", "winzip32.exe", filename, NULL, SW_SHOW );
		return 1;
	}

	if ( !filename ) return 0;

	ghThread = CreateThread( 
		NULL,                        // no security attributes 
		0,                           // use default stack size  
		StartFileViewProc,		    // thread function 
		filename,                // argument to thread function 
		0,                           // use default creation flags 
		&dwThreadId);                // returns the thread identifier  
    
	if (ghThread) {
		SetThreadPriority( ghThread, THREAD_PRIORITY_BELOW_NORMAL );
	}
	return(1);
}

