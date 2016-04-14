#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

// gadget/widget control button
typedef struct gtkButtons {
	GtkWidget	*window;
	GtkWidget	*window2;
	GtkWidget	*area;
	short		x;
	short		y;
	short		w;
	short		h;
	long		hashid;
	long		items;
	struct gtkButtons	*next;
} GtkControl;

// window/dialog structure
typedef struct GtkWinSt {
	GtkWidget	*window;			// pointer to gtk win widget
	GtkWidget	*vbox;				// pointer to gtk area widget
	GtkWidget	*area;				// pointer to gtk area widget
	GtkControl 	*widget;			// pointer to child controls/widgets
	short		x;
	short		y;
	short		w;
	short		h;
	long		hashid;
	struct GtkWinSt	*next;
} GtkWin;

#define	GTKWINSIZE	sizeof( struct GtkWinSt )
#define	SCALEX		rc_ScaleX/100.0
#define	SCALEY		rc_ScaleY/100.0



int ConvertCommaListtoArray( char *list, char **stringPtr, int max );

void ChangeListItem( char *name, char *text );

int ViewFile( char *filename );


void SetGlobalScale( long ,long );

int AddPopupItems( char *name, char **items );
void AddListItem( GtkWidget *gtklist, char *buffer );
void AddItemtoList( char *name, char *text );

long GetPopupNum( char *name );
void SetPopupNum( char *name, long num );
int GetPopupTot( char *name );
int GetPopupText( char *name, int size, char *text );

void DeleteSelectedListItems( char *name );
void ClearListItems( char *name );
long GetListItemTotal( char *name );
char *GetListItemText( char *name, long n, char *out );


GdkPixmap *gdk_pixmap_create_from_bmp_d( GdkWindow *win, unsigned char *bmpd );
GdkPixmap *gdk_pixmap_create_from_bmp( GdkWindow *win, char *file );

int MessageBox( char *txt, char *title, int );

// similar to macOS's function, while in a long loop or anything that blocks the gui
// call this to make the gui work, why the gtk people didnt give us a nice function like
// this for us to use, GOD  KNOWS!!!  is it so much to ask?
void WaitNextEvent( void );

long ChooseRGBColor ( char *parentName, long rgb );
char *GetFileName( char *parentName, char *title );

GtkWidget *FindWindowWidget( GtkWin *w, char *wname );
GtkControl *FindWindowControl( GtkWin *w, char *wname );
GtkWidget *FindWidget( char *wname, GtkWin **parent );
GtkControl *FindControl( char *wname, GtkWin ** );
GtkWin *FindWindow( char *wname );
GtkWidget *FindWindowPtr( char *wname );

//overide default font for all windows/objects
long SetDefaultFont( char *font, long size );

void AttachProcToButton( GtkWidget *button,  void *func, char *event, void *data );
// if someone presses the button ctrl , it will call func (event is clicked)
long AttachProcToControl( char *win, char *ctrlname,  void *func, char *event, void *data );
long AttachProcToButtonName( char *win, char *ctrlname,  void *func, void *data );

void CloseWindow( char *wname );		// what else close A window (really hides it), its still in ram
int ShowWindow( char *nameid );			// bingo, show it
void ShowAllWindows( void );			// popup all windows that are loaded with .rc
int ShowWidget( char *nameid );
int HideWidget( char *nameid );

void SetButtonBgColor( char *name, long rgb );

void MoveControl( char *name, long x, long y );
long GetCursorPos( char *name );


// specify a windows exit proc, nice and easy, just pass the NAME,FUNC()
void SetWindowExitProc( char *windowName, void *proc );
void SetWindowTitle( char *wname, char *t );

// since windows limits the info in the rc file for listviews, we have to at runtime
// replace a listview with its same coords/size with out own column/headers info.
GtkWidget *ReplaceListView( char *nameid, char **textColumns, long columns, long rowheight, long selectionmode );

// gota read the data from the list view somehow, so here you are, a simple
// one liner, no crap , just simple to use, without passing 50 args or 4 calls...
long ListView_IsItSelectedItem( char *name, long row, long column, char **text );
int AddListViewIconItem( char *listname, char *text, GdkPixmap *pixmap, long width );
int AddListViewPixmapItem( char *windowname, char *listname, char *text,  char *data_xpm[], long width );
void HideListViewTitle( char *nameid );
int ListView_InsertItem( char *name, long row, char **text );
int ListView_AppendItem( char *name, char **text );
int ListView_AddOneItem( char *name, char * );
int ListView_DeleteAll( char *name );
long ListView_GetItem( char *name, long row, long column, char **text );
int ListView_DeleteAll( char *name );
int ListView_DeleteItem( char *name, int row );
long ListView_DeleteSelected( char *name );
long ListView_GetSelected( char *name );

char *GetSelectedListItemText( char *name, char *out );

GtkWidget* TreeView_InsertItem( char *name, char *text, long flags, long data, void *func );

void WriteXY( char *windowname, long x, long y, char *txt );
int AddLabelToWindow( char *labelID, char *windowID, char *label, long x, long y );


long Init_ComboBoxString( char *itemname, char *items );
long Init_ComboBoxClearAndSelect( void *hDlg, char *comboid, char *data, int sel );

// you can 'glue' a smaller window into the contents of another window, usefull
// for prefernces options where regions apear/disapear, just show/hide the child region
// this is similar to Win32 child windows, ie add X window to Y window at x,y coord
// ie a window with in a window, very usefull
long AttachWindowToWindow( char *win, char *childwin, long x, long y );

int AttachProcToPopup( char *name, void *callback );

// attach a window definition to a predefined TABBAR (notebook) control
long AttachWindowToTab( char *win, char *childwin, char *title );

int AttachPixmapControlToWindow( char *wname, char *pixCtrl );
void AttachPixmapToWindow( char *bname, char *pic_xpm );  //YEP
void AttachPixmapToArea( char *area, char *pic_xpm );

int AddPixmapControl( char *wname, char *name, char *pic_xpm );
GtkWidget *AddPixmapControlFromPixmap( char *sourceControl, char *newname, long x, long y, long w, long h );

int AddImagesToButton( char *image1, char *image2, char *buttonname );


// return window's width/height by given window name
void GetWindowSize( char *windowName, long *w, long *h );

int AddMenuToWindow( char *menuID, char *windowID, void *func );

void SetSliderPos( char *name, long pos );
long GetSliderPos( char *name );
void SetSliderRange( char *name, long i );


int WindowPreventResize( char *name );

// simple... 0...100
void SetProgressValue( char *name, long new_val );
// duhh.
void SetLabelText( char *name, char *newtext );


void DisableButton( char *name, long status );


// oh man, do i make it easy, common, you cant have it simpler than this
// this is way cool to set timers without complex crap
int SetWidgetTimer( long count, char *wname, void *func );
void KillWidgetTimer( long timer );

// The BIG MOTHER that does it all
// it loads up the windos .rc file, taking #ifdef into account
// and allocates a window for each dialog or dialogex and its sub controls
// all are converted (tried) to gtk format data, each CONTROL and WINDOW are
// defined in the rc file with an ID name that in windows is a #define for an unique ID
// but here I will only use that name as a string and hash it to reference find it like that.
// All you do is pass the NAMEID and it finds the window or button by searching the linklist
// until it finds that hash in the struct and it then can talk directly to its gtk GtkWidget ptr.
// Simple? piece of cake , now no more stupid 500 lines of gtk code for a simple window with
// buttons, just a windows .rc file and call this and its defined, then just call ShowWindow( "name" );
// and its visible in the same layout as possible as windows, you then can call AttachProcToButton
// to trigger events if someone presses a button etc...
// Emulating windows message events is no good, this is better and easier to implement.

GtkWidget **ParseWindowsRC( char *file, char *depend, int debuglevel );

char *GetEntryText( char *name );
void SetEntryText( char *name, char *txt );
int GetText( char *id, char *out, int n);

void CheckBoxSet( char *name, long flag );
long CheckBoxGet( char *name );


#define IfCheckIt( id, check )  		CheckBoxGet( id ) == check
#define IfCheckOFF( id )                CheckBoxGet( id ) == 0

#define IsChecked( id )                 CheckBoxGet( id )
#define CheckIt( id, check )    		CheckBoxSet( id, check )
#define CheckON( id )                   CheckBoxSet( id, 1 )
#define CheckOFF( id )                  CheckBoxSet( id, 0 )
 
#define SetText( id, text )             SetEntryText( id, text )
 
#define Enable( id )                    DisableButton(id,1)
#define Disable( id )					DisableButton(id,0)
#define	SetEnabled( id, status )		DisableButton(id,status)
 
#define Hide( id )						HideWidget(id)
#define Show( id )						ShowWidget(id)

/*
NOTE: There is no reason a custom format spec cant be made to replace the windows .rc format
and also gtk's rc format with a blend in between with the ease of window rc files and the
compatibility of gtk functionality.








*/





#ifdef __cplusplus
}
#endif






