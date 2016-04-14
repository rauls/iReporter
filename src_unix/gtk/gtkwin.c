/*
 (C) 1999 Raul Sobon  Analysis Software Pty Ltd     www.activeconcepts.com
 		  r.sobon@activeconcepts.com.au

 gtkwin - use Windows resource (.rc) files in gtk (1.1.16+)
 why - gtk is a pain to use and thats being kind to it, and I needed an easy
 		way to port my windows app with out writting 9000 lines of gtk calls.
		Honestly, gtk is a horrible api , too X centric and painfull. It has along
		way to go to being a pleasure to use. Hmpf.. maybe in 2001.

 LOG :
 1999-Feb-15:	Started it, added everything, it works


*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <gtk/gtk.h>
#include "gtkwin.h"







int ConvertCommaListtoArray( char *list, char **stringPtr, int max )
{
	char data[512], *p;
	int c = 0;

	mystrcpy( data, list );
	p = data;
	while( p && c<max){
		stringPtr[c] = p;
		p = mystrchr( p, ',' );
		if ( p ){
			*p = 0;
			p++;
		}
		c++;
		stringPtr[c] = 0;
	}
	return c;
}



// this could be usefull, if the gtk people ever expand...
// simple and no need for 50gig library is it....
#define BMFH_SIZE		(14)
#define BMIH_SIZE		(40)
#define RGB_SIZE		(4*256)
#define	TOTAL_HEADER_SIZE	(BMFH_SIZE + BMIH_SIZE + RGB_SIZE)
GdkPixmap *gdk_pixmap_create_from_bmp_d( GdkWindow *win, unsigned char *bmpd )
{
    GdkPixmap *pixmap; unsigned char *data, *pal; long w, h, o;
	short	bitdepth;

	data = (unsigned	char *) bmpd;
	
	// note: swap the bytes if endian != intel style
	memcpy( &o, data+14-4, 4 );			/*	w = mybmp->bmih.biWidth;  */
	memcpy( &w, data+14+4, 4 );			/*	w = mybmp->bmih.biWidth;  */
	memcpy( &h, data+14+4+4, 4 );		/*	h = mybmp->bmih.biHeight; */
	memcpy( &bitdepth, data+14+4+4+6, 2 );		/*	h = mybmp->bmih.biBitCount; */
	data = (unsigned char *)data + o;
	if ( bitdepth<=8)
		pal = (unsigned char *)data + BMFH_SIZE + BMIH_SIZE;
	else pal = 0;

	pixmap = gdk_pixmap_create_from_data( win, (gchar *)data, (gint)w, (gint)h,
		bitdepth, (GdkColor*)pal, (GdkColor*)pal );
	return pixmap;
}

GdkPixmap *gdk_pixmap_create_from_bmp( GdkWindow *win, char *file )
{
	FILE *fp; long len; char *ram;
	
	if( fp = fopen( file, "r" ) ){
		fseek( fp, 0, SEEK_END );
		len = ftell( fp );
		fseek( fp, 0, SEEK_SET );
		ram = malloc( len );
		fread( ram,1,len,fp );
		gdk_pixmap_create_from_bmp_d( win, ram );
		free( ram );
	}
}


static msgbox_result = 0;

void MessageBox_ok( GtkWidget *widget, GtkFileSelection *fs )
{
	gtk_widget_destroy( GTK_WIDGET(widget) );
	msgbox_result = 1;
}

void MessageBox_cancel( GtkWidget *widget, GtkFileSelection *fs )
{
	gtk_widget_destroy( GTK_WIDGET(widget) );
	msgbox_result = 0;
}


int MessageBox( char *txt, char *title, int number )
{
  GtkWidget *label;
  GtkWidget *button, *button2, *dialog_window;
printf( "%s - %s\n", title, txt );
  if (txt)
  {
      dialog_window = gtk_dialog_new ();

      gtk_signal_connect (GTK_OBJECT (dialog_window), "destroy",
			  GTK_SIGNAL_FUNC(gtk_widget_destroyed),  &dialog_window);

      gtk_window_set_title (GTK_WINDOW (dialog_window), title );
      gtk_container_set_border_width (GTK_CONTAINER (dialog_window), 0);
      gtk_widget_set_usize (dialog_window, 270, 130);

      button = gtk_button_new_with_label ("OK");
      GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area), button, TRUE, TRUE, 0);
      gtk_widget_grab_default (button);
      gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
				 GTK_SIGNAL_FUNC (MessageBox_ok),  (GtkObject*) dialog_window);
      gtk_widget_show (button);

	  if ( number == 2 ){
	      button2 = gtk_button_new_with_label ("Cancel");
	      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area), button2, TRUE, TRUE, 0);
		  gtk_signal_connect_object (GTK_OBJECT (button2), "clicked",
					 GTK_SIGNAL_FUNC (MessageBox_cancel), (GtkObject*) dialog_window);
	  }

      button = gtk_label_new ( txt );
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->vbox),
			  button, TRUE, TRUE, 0);
		gtk_label_set_justify (GTK_LABEL (button), GTK_JUSTIFY_RIGHT);
		gtk_label_set_line_wrap (GTK_LABEL (button), TRUE);
      gtk_widget_show (button);

      gtk_widget_show (dialog_window);

	  gtk_main ();
	  
      label = NULL;
    }
  return msgbox_result;
}


// Similar to mac waitnextevent, needed inside progresses and anything that
// will block the main
void WaitNextEvent( void )
{
	while( gtk_events_pending() ){
		gtk_main_iteration();
	}
	gdk_flush();
}


static long cmw_destroy_cb(GtkWidget *widget)
{
  /* This is needed to get out of gtk_main */
  gtk_main_quit ();

  return FALSE;
}

GdkPixmap *PixMapFromData( GtkWidget *w, char *data_xpm[] )
{
	GdkPixmap *pixmap;
	GdkBitmap *mask = NULL;
	GdkColor	col;
	pixmap = gdk_pixmap_create_from_xpm_d (w->window,
					 &mask, 
					 &w->style->white,
					 data_xpm);
	return pixmap;
}


static double color_sel[4];

void color_selection_ok( GtkWidget *widget, GtkFileSelection *fs )
{
	if ( fs ){
		gtk_color_selection_get_color ( GTK_COLOR_SELECTION_DIALOG(widget)->colorsel, color_sel );
		gtk_widget_destroy( GTK_WIDGET(widget) );
	}
}

long ChooseRGBColor ( char *parentName, long rgb )
{
    GtkWidget *csd;
	GtkWin *window, *parent=NULL;

	color_sel[0] = (rgb>>16) / 256.0;
	color_sel[1] = ((rgb>>8) & 0xff) / 256.0;
	color_sel[2] = (rgb & 0xff) / 256.0;
	color_sel[3] = 0;

	if ( parentName ){
		window = FindWindow(parentName);
		if ( window )	
			parent = window->window;
	}

	if ( parent ){
		csd=gtk_color_selection_dialog_new ("Select your color");

		/* Set as modal */
		gtk_window_set_modal (GTK_WINDOW(csd),TRUE);

		/* And mark it as a transient dialog */
		gtk_window_set_transient_for (GTK_WINDOW (csd), GTK_WINDOW (parent));
    
		gtk_signal_connect (GTK_OBJECT(csd), "destroy",
					GTK_SIGNAL_FUNC(cmw_destroy_cb),NULL);

		gtk_signal_connect_object (GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(csd)->ok_button),
								   "clicked", GTK_SIGNAL_FUNC(color_selection_ok),
								   GTK_OBJECT (csd));
		gtk_signal_connect_object (GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(csd)->cancel_button),
								   "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
								   GTK_OBJECT (csd));
    
		gtk_color_selection_set_color ( GTK_COLOR_SELECTION_DIALOG(csd)->colorsel, color_sel );

		color_sel[0] = color_sel[1] = color_sel[2] = color_sel[3] = -1;
		
		/* wait until destroy calls gtk_main_quit */
		gtk_widget_show (csd);    
		gtk_main ();

		if( color_sel[0] != -1 ){
			rgb =	( ((long)(256*color_sel[0]) << 16) |
					  ((long)(256*color_sel[1]) << 8) |
					  ((long)(256*color_sel[2])) );
			return rgb;
		} else
			return -1;
	}
	return -1;

}

static char gFilename[300];
void file_selection_ok( GtkWidget *widget, GtkFileSelection *fs )
{
	if ( fs ){
		char *p;
		if ( p=gtk_file_dialog_get_filename (GTK_FILE_SELECTION(fs)) )
			strcpy( gFilename , p );
		gtk_widget_destroy( GTK_WIDGET(fs) );
	}
}

void
file_selection_cancel(GtkWidget *w,GtkFileSelection *fs)
{
  gtk_widget_destroy (GTK_WIDGET (fs));
}


#include "gtkfiledialog.h"

char *GetFileName( char *parentName, char *title )
{
    GtkWidget *fs; GtkWidget *parent;

	parent = FindWindow(parentName)->window;

    //fs = gtk_file_selection_new( title );
    fs = gtk_file_dialog_new( title );
	if ( fs ){
		gtk_window_set_position (GTK_WINDOW (fs), GTK_WIN_POS_MOUSE);
		
		/* Set as modal */
		gtk_window_set_modal (GTK_WINDOW(fs),TRUE);

		/* And mark it as a transient dialog */
		gtk_window_set_transient_for (GTK_WINDOW (fs), GTK_WINDOW (parent));

		gtk_signal_connect (GTK_OBJECT(fs), "destroy",
							GTK_SIGNAL_FUNC(cmw_destroy_cb),NULL);

		gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),
								   "clicked",GTK_SIGNAL_FUNC(file_selection_ok),
								   GTK_OBJECT (fs));
		gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),
								   "clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),
								   GTK_OBJECT (fs));
    
		/* wait until destroy calls gtk_main_quit */
		gtk_widget_show (fs);
    
 		gFilename[0]=0;
		gtk_main();
	}


	if( gFilename[0] )
		return gFilename;
	else
		return NULL;
}




// -----------------------


long GetRand( int seed )
{
	srand( seed );
	return rand();
}

unsigned long HashStr(char *string, unsigned short len)
{          
	register unsigned long answer=0,count,idx;
	register unsigned char b0,c0;
	register char *p;

	p=string;
	for (count=0;count<len;count++) {
		c0 = *p++;
		b0 = (unsigned char)(answer >> 24);
		answer = answer << 8;
		answer |= b0;
		idx = (b0^c0)&0xff;
		answer ^= GetRand(idx);
	}

	return(answer);
}


static long	rc_ScaleY = 150,
			rc_ScaleX = 150;		// 150 = ratio 1.5 times 
GtkWin	*gWins,		// top level window
		*gCurrWin;	// current pointer

void SetGlobalScale( long x, long y )
{
	if ( y==0 ) y=x;
	rc_ScaleX = x;
	rc_ScaleY = y;
}

GtkControl *AddControl( GtkWin *w, char *idname, GtkWidget *gtkw , GtkWidget *gtkparam,
 short x, short y, short W, short H )
{
	GtkControl *c = NULL;

	if ( w ){
		if( c = w->widget ){
			// find last control
			while( c->next )
				c = c->next;
			// add control
			if( c->next = malloc( sizeof( GtkControl ) ) ){
				c = c->next;
				memset( c, 0, sizeof( GtkControl ) );
			}
		} else {
			c = w->widget = malloc( sizeof( GtkControl ) );
			memset( c, 0, sizeof( GtkControl ) );
		}

		if( c ){
			c->hashid = HashStr( idname, strlen(idname) );
			c->window = gtkw;
			c->window2 = gtkparam;
			c->x = x;
			c->y = y;
			c->w = W;
			c->h = H;
			c->next = 0;
			//printf( "Add %s=%x\n", idname, c->hashid );
		}
	}	
	return c;
}




GtkWin *AddWindow( GtkWin *top, char *wname,
 GtkWidget *gtkw, GtkWidget *vbox, GtkWidget *area,
 short x, short y, short W, short H )
{
	GtkWin *w=NULL;
	
	if ( top && wname ){
		w = top->next = malloc( GTKWINSIZE );
		if ( w ){
			w->hashid = HashStr( wname, strlen(wname) );
			w->window = gtkw;
			w->vbox = vbox;
			w->area = area;

			w->widget = 0;
			w->x = x;
			w->y = y;
			w->w = W;
			w->h = H;
			
			w->next = 0;
		}
	}
	return w;
}


GtkWin *AddMenu( GtkWin *top, char *menuID, GtkAccelGroup *a, GtkItemFactory *i )
{
	GtkWin *w=gCurrWin;
	
	if ( top && menuID ){
		top->next = malloc( GTKWINSIZE );
		w = top->next;
		if ( w ){
			w->hashid = HashStr( menuID, strlen(menuID) );
			w->window = (GtkWidget*)a;
			w->vbox = (GtkWidget*)i;
			w->area = NULL;
			w->widget = NULL;
			w->next = NULL;
		}
	}
	return w;
}


GtkWidget *FindWindowWidget( GtkWin *w, char *wname )
{
	long x, id;
	GtkControl *c = w->widget;
	
	id = HashStr( wname, strlen(wname) );
	while( c ){
		if( c->hashid == id )
			return c->window;
		c = c->next;
	}
	return NULL;
}

GtkControl *FindWindowControl( GtkWin *w, char *wname )
{
	long x, id;
	GtkControl *c = w->widget;
	
	id = HashStr( wname, strlen(wname) );
	while( c ){
		if( c->hashid == id )
			return c;
		c = c->next;
	}
	return NULL;
}




GtkWidget *FindWidget( char *wname, GtkWin **parent )
{
	long x, id;
	GtkWin *w = gWins;
	GtkControl *c = 0;
	id = HashStr( wname, strlen(wname) );
	
	while( w ){
		c = w->widget;
		while( c ){
			if( c->hashid == id ){
				if ( parent ) *parent = w;
				return c->window;
			}
			c = c->next;
		}
		w = w->next;
	}
	return NULL;
}

GtkControl *FindControl( char *wname, GtkWin **parent )
{
	long x, id;

	if ( wname ) {
		GtkWin *w = gWins;
		GtkControl *c = 0;

		id = HashStr( wname, strlen(wname) );
		
		while( w ){
			c = w->widget;
			while( c ){
				if( c->hashid == id ){
					if ( parent )
						*parent = w;
					return c;
				}
				c = c->next;
			}
			w = w->next;
		}
	}
	printf( "cant find control %s\n", wname );
	return NULL;
}

GtkWin *FindWindow( char *wname )
{
	long x, id;
	GtkWin *w = gWins;
	
	id = HashStr( wname, strlen(wname) );
	while( w ){
		if( w->hashid == id )
			return w;
		w = w->next;
	}
	return NULL;
}

GtkControl *FindControlInWindow( char *window, char *wname )
{
	GtkWin *win;	
	GtkControl *c = NULL;

	win = FindWindow( window );
	if ( win )
		c = FindWindowControl( win, wname );
	return c;
}


GtkWidget *FindWindowPtr( char *wname )
{
	long x, id;
	GtkWin *w = gWins;
	
	id = HashStr( wname, strlen(wname) );
	while( w ){
		if( w->hashid == id )
			return w->window;
		w = w->next;
	}
	return NULL;
}


void RemoveControl( char *nameid )
{
	GtkWin *win=0;
	GtkControl *ctrl = FindControl( nameid, &win );	

	if ( ctrl && win ) {
		gtk_widget_destroy( ctrl->window );
	}
}



int ShowWindow( char *nameid )
{
	long id;
	GtkWin *w = gWins;
	
	id = HashStr( nameid, strlen(nameid) );
	while( w ){
		if( w->hashid == id ){
			//if ( w->area ) gtk_widget_show( w->area );
			if ( w->window ) {
				if ( !GTK_WIDGET_VISIBLE (w->window) )
					gtk_widget_show( w->window );
				return 0;
			} else
				return -2;
		}
		w = w->next;
	}
	return -1;
}



void SetWindowTitle( char *wname, char *t )
{
	GtkWidget *w = FindWindowPtr( wname );
	if ( w  )
	gtk_window_set_title (GTK_WINDOW (w), t );
}




int ShowWidget( char *nameid )
{
	if( nameid ){
		GtkWidget *w = FindWidget( nameid , 0 );
		if ( w ){
			gtk_widget_show( w );
			gtk_widget_grab_focus (w);
		}
	}
}
int HideWidget( char *nameid )
{
	if( nameid ){
		GtkWidget *w = FindWidget( nameid , 0 );
		if( w  ) gtk_widget_hide( w );
	}
}



void AttachProcToButton( GtkWidget *button,  void *func, char *event, void *data )
{
	if ( button )
		gtk_signal_connect( GTK_OBJECT(button), event,GTK_SIGNAL_FUNC(func), data );
}

// Call with AttachProcToButtonName( "windowname", "controlname", myfunc );
long AttachProcToControl( char *win, char *ctrlname,  void *func, char *event, void *data )
{
	GtkWin *window; GtkWidget *button; GtkControl *control;
	
	if ( ctrlname && win && func){
		if ( window = FindWindow( win ) ){
			if ( control = FindWindowControl( window, ctrlname ) ){
				if ( !(button = control->window2) )
					button = control->window;
				AttachProcToButton( button, func, event, data );
				return 0;
			} else	return -3;
		} else	return -2;
	} else return -1;

}

long AttachProcToButtonName( char *win, char *ctrlname,  void *func, void *data )
{
	AttachProcToControl( win, ctrlname, func, "clicked", data );
}

void GetWindowSize( char *windowName, long *w, long *h )
{
	GtkWin   *window;

	window = FindWindow( windowName );
	if ( window && w && h ){
	  *w = window->w;
	  *h = window->h;
	}
}

void GetControlXYWH( char *name, long *x, long *y, long *w, long *h )
{
	GtkWin   *window; GtkControl *wid;

	wid = FindControl( name, &window );
	if ( wid ){
		if( x && y ) { *x = wid->x;	  *y = wid->y; }
		if( w && h ) { *w = wid->w;	  *h = wid->h; }
	}
}


void MoveControl( char *name, long x, long y )
{
	GtkWin   *window; GtkControl *wid;

	wid = FindControl( name, &window );
	if ( wid ){
		gtk_widget_set_uposition( wid->area, x,y );
	}
}



long AttachWindowToWindow( char *win, char *childwin, long x, long y )
{
	GtkWin *window; GtkWin *cwindow; GtkWidget *area;
	
	window = FindWindow( win );
	cwindow = FindWindow( childwin );
	if ( window && cwindow ){
		long w,h;
		GetWindowSize( childwin, &w, &h );
		x = x*SCALEX;
		y = y*SCALEX;
		area = gtk_fixed_new();

		if ( cwindow->area && area ){
			cwindow->area->parent=0;

			gtk_container_add( GTK_CONTAINER(area), cwindow->area );
			gtk_fixed_put( GTK_FIXED(window->area), area, x, y);
			gtk_widget_set_usize( area, w+5,h+8 );
			cwindow->window = area;
			return 0;
		} else
			return -2;
	} else {
		if ( !window ) printf( "cant find tab %d\n", win );
		if ( !cwindow ) printf( "cant find window %d\n", childwin );
		return -1;
	}
}


long AttachWindowToTab( char *tabcontrol, char *childwin, char *title )
{
	GtkWin *window; GtkWin *cwindow; GtkWidget *tab, *child, *label,*label_box,*menu_box;
	
	tab = FindWidget( tabcontrol, 0 );
	cwindow = FindWindow( childwin );

	if ( tab && cwindow ){
		long w,h;
		GetWindowSize( childwin, &w, &h );
		child = gtk_fixed_new ();
		if ( cwindow->area && child ){
			cwindow->area->parent=0;
			gtk_container_add (GTK_CONTAINER (child), cwindow->area);

			label_box = gtk_hbox_new (FALSE, 0);
			if ( label_box ){
				label = gtk_label_new (title);
				gtk_box_pack_start (GTK_BOX (label_box), label, FALSE, TRUE, 0);
				gtk_widget_show_all (label_box);
			}

			menu_box = gtk_hbox_new (FALSE, 0);
			if ( menu_box ){
				label = gtk_label_new (title);
				gtk_box_pack_start (GTK_BOX (menu_box), label, FALSE, TRUE, 0);
				gtk_widget_show_all (menu_box);
			}

			if ( label_box && menu_box ){
				gtk_notebook_append_page_menu( GTK_NOTEBOOK(tab), child, label_box, menu_box );
				gtk_widget_show( child );
			}
			
		} else printf( " no child gtk_fixed_new\n" );
	} else {
		if ( !tab )	printf( " no %s\n", tabcontrol );
		if ( !cwindow )	printf( " no %s\n", childwin );
	}
}

void SetButtonBgColor( char *name, long rgb )
{
	GtkControl	*ctrl;
	GtkWin		*win;
	GtkWidget	*window, *button;

	ctrl = FindControl( name, &win );
	if ( ctrl && win ) {
		button = ctrl->window;
		window = win->window;

		if( button ){
			static GdkColor col1;
			GtkStyle *style1, *style;
			GdkGC *gc;
			
			col1.red = (rgb&0xff0000) >> 8;
			col1.green = (rgb&0xff00);
			col1.blue = (rgb&0xff) << 8;

			if ( ctrl->window2 )
				gtk_style_unref( (GtkStyle*)ctrl->window2 );

			style = gtk_widget_get_default_style();
			if ( style )
				style1 = gtk_style_copy ( style );
				ctrl->window2 = (GtkWidget*)style1;
			if( style1 ){
				style1->bg[GTK_STATE_NORMAL] = col1;
				gtk_widget_set_style( button, style1 );
			}
		} else printf( "nobutton\n");
	} else printf( "noctrl\n");
}


int AddPixmapControl( char *wname, char *name, char *pic_xpm )
{
	GtkWin		*win;
	GtkWidget	*window, *widget;
	
	win = FindWindow( wname );

	if( win ){
		GdkBitmap *mask;
		GdkPixmap *pixmap;
		GtkWidget *wpixmap;
		GdkColor  col; GtkStyle *style;

		win = FindWindow( wname );
		//if ( FindWidget( name, &win ) ) return;
		if ( win ) {
			window = win->window;
			if ( window ){
				style = gtk_widget_get_style( window );
				pixmap = gdk_pixmap_create_from_xpm_d (window->window,
								 &mask, 
								 &col,
								 (gchar**)pic_xpm);
				wpixmap = gtk_pixmap_new (pixmap, mask);
				if ( wpixmap ){
					AddControl( win, name, wpixmap,(GtkWidget*)pixmap ,0,0,0,0 );
					return 0;
				} else
					return -4;
			} else
				return -3;
		} else
			return -2;
	} else
		return -1;
}

GtkWidget *AddPixmapControlFromPixmap( char *sourceControl, char *newname, long x, long y, long w, long h )
{
	GtkWin	*win;
	GtkWidget *src, *wpixmap, *window;
	
	src = FindWidget( sourceControl, &win );
	if( src && win ){
		GdkPixmap *pixmap; GdkBitmap *mask;
		
		window = win->window;
		pixmap = gdk_pixmap_new( window->window, w, h, -1 );
		if ( pixmap ){
			wpixmap = gtk_pixmap_new (pixmap, NULL);
			gdk_draw_pixmap( pixmap,
				  window->style->fg_gc[GTK_WIDGET_STATE (window)],
				  GTK_PIXMAP(src)->pixmap, x,y, 0,0, w,h );
			AddControl( win, newname, wpixmap,0 ,0,0,w,h );
			return wpixmap;
		} else printf( "gdk_pixmap_new( window, w, h, -1 ) == NULL\n" );
	}
	return NULL;
}


// Oh god, not more gtk hassels, why cant we add an image to a button
// thats allready added to a fixed area??? workarounds or what.
//  GRRR is all i can say
int AddImagesToButton( char *image1, char *image2, char *buttonname )
{
	GtkWin		*win; GtkControl *ctrl;
	GtkWidget	*window,
				*button=0, *source, *source2, *newbutton, *box;
	
	source = FindWidget( image1, &win );
	source2 = FindWidget( image2, &win );
	ctrl = FindControl( buttonname, &win );
	if ( ctrl ) {
		button = ctrl->window;
		window = win->window;
	}
	
	if( win && source && ctrl ){
		long x,y,w,h;
		GetControlXYWH( buttonname, &x,&y,&w,&h );
		newbutton = gtk_button_new ();
		if ( newbutton ){
			box = gtk_vbox_new (FALSE, 0);
			if ( box ){
				gtk_container_set_border_width (GTK_CONTAINER (box), 0);
				gtk_container_add (GTK_CONTAINER (box), source);
				gtk_container_add (GTK_CONTAINER (box), source2);
				gtk_container_add (GTK_CONTAINER (newbutton), box);
				gtk_widget_destroy( button );

				gtk_fixed_put (GTK_FIXED (win->area), newbutton, x, y);
				gtk_widget_set_usize( newbutton, w,h );
				gtk_widget_show( source );
				gtk_widget_show( newbutton );
				gtk_widget_show( box );
				ctrl->area = box;
				ctrl->window = newbutton;
				return 0;
			} else
				return -3;
		} else
			return -2;
	} else
		return -1;
}



void AttachPixmapToArea( char *areaName, char *pic_xpm )
{
	GtkWin		*win;
	GtkWidget	*window, *area;
	
	area = FindWidget( areaName, &win );

	if( area ){
		GdkBitmap *mask;
		GdkPixmap *pixmap;
		GtkWidget *wpixmap;
		GdkColor  col; GtkStyle *style;

		window = win->window;
		if ( window ){
			style = gtk_widget_get_style( window );
			if ( style ){
				pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask, 
								 &style->bg[GTK_STATE_NORMAL], (gchar**)pic_xpm);
				if ( pixmap ){
					wpixmap = gtk_pixmap_new (pixmap, mask);
					if ( wpixmap ){
						gtk_widget_show (wpixmap);
						gtk_container_add (GTK_CONTAINER (area), wpixmap );
						gtk_widget_show (area);
					}
				}
			}
		}
	}
}

int AttachPixmapControlToWindow( char *wname, char *pixCtrl )
{
	GtkWin		*win;
	GtkWidget	*window, *widget, *wpixmap;
	
	win = FindWindow( wname );

	if( win ){
		GdkColor  col; GtkStyle *style;

		window = win->window;
		widget = win->area;
		if ( window ){
			style = gtk_widget_get_style( window );
			wpixmap = FindWidget( pixCtrl, 0 );
			if ( wpixmap ){
				gtk_container_add (GTK_CONTAINER (widget), wpixmap );
				gtk_widget_show (wpixmap);
				gtk_widget_show (widget);
				return 0;
			} else
				return -3;
		} else
			return -2;
	} else
		return -1;
}


void AttachPixmapToWindow( char *wname, char *pic_xpm )
{
	GtkWin		*win;
	GtkWidget	*window, *widget;
	
	win = FindWindow( wname );

	if( win ){
		GdkBitmap *mask;
		GdkPixmap *pixmap;
		GtkWidget *wpixmap;
		GdkColor  col; GtkStyle *style;

		window = win->window;
		widget = win->area;
		if ( window ){
			style = gtk_widget_get_style( window );
			if ( style ){
				pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask, 
								 &col,	 (gchar**)pic_xpm);
				if ( pixmap ){
					wpixmap = gtk_pixmap_new (pixmap, mask);
					if ( wpixmap ){
						gtk_container_add (GTK_CONTAINER (widget), wpixmap );
						gtk_widget_show (wpixmap);
						gtk_widget_show (widget);
					}
				}
			}
		}
	}
}




int SetWidgetTimer( long ms, char *wname, void *func )
{
	if (wname && func){
		GtkWidget *label = FindWidget( wname, 0 );
		if ( label ){
			return gtk_timeout_add ( ms, (GtkFunction) func, label);
		} else
			return -1;
	} else
		return 0;
}


void KillWidgetTimer( long timer )
{
	gtk_timeout_remove (timer);
}

void ShowAllWindows( void )
{
	long x, id;
	GtkWin *w = gWins;
	
	while( w ){
		if ( w->area )
			gtk_widget_show( w->area );

		if ( w->window )
			gtk_widget_show( w->window );

		w = w->next;
	}
}


char *SkipSpace( char *txt )
{
	if ( txt ){
		while( *txt == ' ' || *txt == 9 )
			txt++;
	}	
	return txt;
}


long SetDefaultFont( char *font, long size )
{
	GtkStyle *style;
	char	fontspec[120];
	long ret=0;

	if ( size ){
		if ( font )
			sprintf( fontspec, "-adobe-%s-medium-r-normal--*-%d-*-*-*-*-*-*", font, size );
		else
			sprintf( fontspec, "-misc-*-medium-*-normal-*-*-%d-*-*-*-*-*-*", size );
		if( style = gtk_widget_get_default_style() ){
			style->font = gdk_font_load ( fontspec );
			if ( style->font ){
				gtk_widget_set_default_style( style );
				ret = 1;
			}
		}
	}
	return ret;
}

// write text into a image
void WriteXY( char *name, long x, long y, char *txt )
{
	GtkWin *win=NULL;
	GtkControl *ctrl;

	//win = FindWindow( name );
	ctrl = FindControl( name, &win);
	if ( win && txt ){
		GtkStyle	*style;
		GtkWidget	*window = win->window,
					*out = ctrl->window2;		//win->window->window
		if( (style = gtk_widget_get_default_style()) && strlen(txt) ){
			GdkGC *gc;
			// = window->style->fg_gc[GTK_WIDGET_STATE (window)];
			gc = window->style->white_gc;
			//gdk_gc_set_foreground (gc, &window->style->fg[GTK_STATE_SELECTED]);
			gdk_draw_string ( (GdkDrawable*)out, style->font , gc, x, y, txt );
			//gtk_draw_string( style, win->area->window, GTK_STATE_SELECTED, x, y, txt );
			//gdk_gc_set_foreground (gc, &window->style->fg[GTK_STATE_NORMAL]);
		}	
	}
}



//
//	char *text[][2] = { {"Log file", 0 } };
//	CreateListView( text, 1 );
//
GtkWidget * CreateListView( char **textColumns, long ncolumns )
{
	GtkWidget *obj;
	long i;
	
	obj = gtk_clist_new_with_titles( ncolumns, textColumns );
	if ( textColumns[0][0] != ' ' ){
		for( i=0;i<ncolumns;i++)
			gtk_clist_set_column_min_width( GTK_CLIST(obj), i, 80 );
	}
	gtk_clist_set_shadow_type (GTK_CLIST(obj), GTK_SHADOW_OUT);
	return obj;
}


// stupid GTK cant handle NULL textcolumns well, its HBAR becomes real huge
GtkWidget *ReplaceListView( char *nameid, char **textColumns, long columns, long rowheight, long selectionmode )
{
	GtkWidget *lv=0;
	GtkWin *win=0;
	GtkControl *ctrl = FindControl( nameid, &win );	

	if ( ctrl && win ) {
		char *loglistStr[][2] = { {" ", 0 } }; char hide=FALSE;
		
		lv = ctrl->window2 ;
		gtk_widget_destroy( lv );

		if( !textColumns ) {
			textColumns = (char**)loglistStr;
			hide=TRUE;
		}
		lv = ctrl->window2 = CreateListView( textColumns, columns );
		if ( lv ){
			gtk_container_add (GTK_CONTAINER (ctrl->window), lv );
			if( rowheight )
				gtk_clist_set_row_height (GTK_CLIST (lv), rowheight );
			if( selectionmode )
				gtk_clist_set_selection_mode(GTK_CLIST (lv), selectionmode );
			if( hide )
				gtk_clist_column_titles_hide (GTK_CLIST (lv));
			gtk_widget_show( lv );
		}	
	}
	return lv;	
}

void HideListViewTitle( char *nameid )
{
	GtkWidget *lv=0;
	GtkWin *win=0;
	GtkControl *ctrl = FindControl( nameid, &win );	

	if ( ctrl && win ) {
		lv = ctrl->window2;
		gtk_clist_column_titles_hide (GTK_CLIST (lv));
	}
}



// add a 1 column text item to a listview with an icon attached
int AddListViewIconItem( char *listname, char *text, GdkPixmap *pixmap, long width )
{
	GtkControl *ctrl = FindControl( listname, 0 );
	gint	row;

	if ( ctrl && text ){
		char *textPtr[][2] = {{text,0 }};
		if ( GTK_CLIST(ctrl->window2) ){
			GdkColor	col1;
			row = gtk_clist_append( GTK_CLIST(ctrl->window2), textPtr[0] );
			gtk_clist_set_pixtext( GTK_CLIST(ctrl->window2), row, 0, text, 4, pixmap, NULL );
			gtk_clist_set_column_width (GTK_CLIST (ctrl->window2), 0, width - 6 );

			gtk_clist_set_column_justification( GTK_CLIST (ctrl->window2), 0, GTK_JUSTIFY_CENTER );
			col1.red = 0xaa00;
			col1.green = 0xaa00;
			col1.blue = 0xaa00;
			
			//gtk_clist_set_background(GTK_CLIST (ctrl->window2), row, &col1 );
			return 0;
		}
	}
	return -1;
}





// add a 1 column text item to a listview with an icon attached
int AddListViewPixmapItem( char *windowname, char *listname, char *text, char *data_xpm[], long width )
{
	GtkControl *ctrl;
	GtkWidget *w;

	w = FindWindowPtr( windowname );
	ctrl = FindControl( listname, 0 );

	if (  w  ){
		if (  ctrl ){
			if (  text ){
				if ( GTK_CLIST(ctrl->window2) ){
					char *textPtr[][2] = {{text,0 }};
					gint	row;
					GdkColor	col1;
					GdkPixmap *pixmap;
					GdkBitmap *mask = NULL;

					col1.red = 0x8000;
					col1.green = 0x8000;
					col1.blue = 0x8000;
					
					pixmap = gdk_pixmap_create_from_xpm_d (w->window,
									 &mask,  &col1,	 data_xpm);

			
					row = gtk_clist_append( GTK_CLIST(ctrl->window2), textPtr[0] );
					gtk_clist_set_pixtext( GTK_CLIST(ctrl->window2), row, 0, text, 4, pixmap, NULL );
					gtk_clist_set_column_width (GTK_CLIST (ctrl->window2), 0, width - 6 );

					gtk_clist_set_column_justification( GTK_CLIST (ctrl->window2), 0, GTK_JUSTIFY_CENTER );
					
					col1.red = 0x8000;
					col1.green = 0x8000;
					col1.blue = 0x8000;

					gtk_clist_set_background(GTK_CLIST (ctrl->window2), row, &col1 );
					return 0;
				} else
					return -4;
			} else
				return -3;
		} else
			return -2;
	} else
		return -1;
	return -5;
}

// add a 1 column text item to a listview with default look
int ListView_InsertItem( char *name, long row, char **text )
{
	GtkControl *ctrl;	

	if ( name ){
		if ( text ){
			ctrl = FindControl( name, 0 );
			if ( ctrl ){
				if ( ctrl->window2 )
					gtk_clist_insert( GTK_CLIST(ctrl->window2), row, text );
				else
					return -4;
				return 0;
			} else
				return -3;
		} else
			return -2;
	} else
		return -1;
}


// add a 1 column text item to a listview with default look
int ListView_AppendItem( char *name, char **text )
{
	GtkControl *ctrl;	

	if ( name ){
		if ( text ){
			ctrl = FindControl( name, 0 );
			if ( ctrl ){
				if ( ctrl->window2 )
					gtk_clist_append( GTK_CLIST(ctrl->window2), text );
				else
					return -4;
				return 0;
			} else
				return -3;
		} else
			return -2;
	} else
		return -1;
}

// add a multi column text item to a listview with default look
int ListView_AddOneItem( char *name, char *text )
{
	char *p, stext[512], *textarray[] = { 0, 0, 0 };

	textarray[0] = text;

	return ListView_AppendItem( name, textarray );
}

long ListView_GetItem( char *name, long row, long column, char **text )
{
	long tot, col;
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl && text ){
		tot = GTK_CLIST(ctrl->window2)->rows;
		col = GTK_CLIST(ctrl->window2)->columns;

		if ( row < tot ){
			while ( 
				gtk_clist_get_text( GTK_CLIST(ctrl->window2), row, column++, text++ )
				);
		} else
			return 0;
	}
	//printf( "%d-%s\n", row, *text );
	return col;
}

// add a multi column text item to a listview with default look
int ListView_DeleteAll( char *name )
{
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl ){
		gtk_clist_clear( GTK_CLIST(ctrl->window2) );
		return 0;
	} else
		return -1;
}

// add a multi column text item to a listview with default look
int ListView_DeleteItem( char *name, int row )
{
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl ){
		gtk_clist_remove( GTK_CLIST(ctrl->window2), row );
		return 0;
	} else
		return -1;
}

// why cant gtk have a little forsight and have funcs to find out
// which rows are selected, god damn it... when will it mature? 2001?
long ListView_IsItSelectedItem( char *name, long row, long column, char **text )
{
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl && text ){
		GtkCListRow *clist_row; GList *list;
		long tot;
		tot = GTK_CLIST(ctrl->window2)->rows;
		if( tot && row<tot ){
			list = g_list_nth (GTK_CLIST(ctrl->window2)->row_list, row);
			clist_row = list->data;
			while( list && row<tot && (clist_row->state != GTK_STATE_SELECTED) ){
				row++;
				if( row<tot ){
					list = g_list_nth (GTK_CLIST(ctrl->window2)->row_list, row);
					clist_row = list->data;
				}
			}
		}
		if ( row<tot)
			gtk_clist_get_text( GTK_CLIST(ctrl->window2), row++, column, text );
		else row=-1;
	}
	return row;
}

// why cant gtk have a little forsight and have funcs to find out
// which rows are selected, god damn it... when will it mature? 2001?
long ListView_DeleteSelected( char *name )
{
	long deleted=0;
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl  ){
		GtkCListRow *clist_row; GList *list;
		long tot, row = 0;

		tot = GTK_CLIST(ctrl->window2)->rows;
		list = g_list_nth (GTK_CLIST(ctrl->window2)->row_list, row);
		while( list ){
			clist_row = list->data;
			if ( (clist_row->state == GTK_STATE_SELECTED) ){
				gtk_clist_remove( GTK_CLIST(ctrl->window2), row );
				deleted++;
			} else
				row++;
			list = g_list_nth (GTK_CLIST(ctrl->window2)->row_list, row);
		}
	}
	return deleted;
}



// why cant gtk have a little forsight and have funcs to find out
// which rows are selected, god damn it... when will it mature? 2001?
long ListView_GetSelected( char *name )
{
	long deleted=0;
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl  ){
		GtkCListRow *clist_row; GList *list;
		long tot, row = 0;

		tot = GTK_CLIST(ctrl->window2)->rows;
		list = g_list_nth (GTK_CLIST(ctrl->window2)->row_list, row);
		while( list ){
			clist_row = list->data;
			if ( (clist_row->state == GTK_STATE_SELECTED) ){
				return row;
			} else
				row++;
			list = g_list_nth (GTK_CLIST(ctrl->window2)->row_list, row);
		}
	}
	return -1;
}




long ListView_GetItemCount( char *name )
{
	long deleted=0;
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl  ){
		GtkCListRow *clist_row; GList *list;
		long tot, row = 0;

		tot = GTK_CLIST(ctrl->window2)->rows;
		return tot;
	}
	return -1;
}








// ---------- TREE VIEWS
GtkWidget* TreeView_InsertItem( char *name, char *text, long flags, long data, void *func )
{
	GtkControl *ctrl = FindControl( name, 0 );	
static		GtkWidget *subtree, *item, *tree;

	if ( ctrl ){
		tree = ctrl->window2;

		if ( flags == -1 || !tree ){
			subtree = item = tree = NULL;
			return 0;
		}
		// add root level ITEM
		if ( flags == 0 ){
			item = gtk_tree_item_new_with_label( text );
			if ( item ){
				gtk_tree_append( GTK_TREE(tree), item );
				gtk_widget_show( item );
				subtree = gtk_tree_new();
				if ( subtree ){
					gtk_tree_set_selection_mode( GTK_TREE(subtree), GTK_SELECTION_SINGLE );
					gtk_tree_set_view_mode( GTK_TREE(subtree), GTK_TREE_VIEW_ITEM );
					gtk_tree_item_set_subtree( GTK_TREE_ITEM(item), subtree );
					//if ( func )
					//	gtk_signal_connect( GTK_OBJECT(subtree), "select_child" , GTK_SIGNAL_FUNC(func), data );
					return subtree;
				}
			}
		} else
		// add subitem to last ROOT LEVEL ITEM
		if ( flags == 1 && subtree ){
			GtkWidget *subitem;
			subitem = gtk_tree_item_new_with_label( text );
			if ( subitem ){
				gtk_tree_append( GTK_TREE(subtree), subitem );
				gtk_widget_show( subitem );

				if ( func )
					gtk_signal_connect( GTK_OBJECT(subitem), "select" , GTK_SIGNAL_FUNC(func), data );
				return subitem;
			}
		} else
		// subitem with checkbox/tick
		if ( flags >= 2 && subtree ){
			GtkWidget *subitem;
			subitem = gtk_tree_item_new();
			if ( subitem ){
				GtkWidget *cbox, *label;
				cbox = gtk_check_box_new( text );
				if ( flags == 3 )
					gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(cbox), 1 );
				gtk_container_add( GTK_CONTAINER( subitem ), cbox );
				gtk_widget_show( cbox );

				gtk_tree_append( GTK_TREE(subtree), subitem );
				gtk_widget_show( subitem );

				if ( func )
					gtk_signal_connect( GTK_OBJECT(subitem), "select" , GTK_SIGNAL_FUNC(func), data );
				return subitem;
			}
		}
	} else
		subtree = item = tree = NULL;

	return 0;
}



// ---------- CHECK BOXES





void CheckBoxSet( char *name, long flag )
{
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl ){
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl->window), flag );
	}
}
long CheckBoxGet( char *name )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	if ( ctrl )
		return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctrl->window) );
	else return -1;
}


void DisableButton( char *name, long status )
{
	if ( name ){
		GtkControl *ctrl;	
		ctrl = FindControl( name, 0 );
		if ( ctrl ){
			if ( ctrl->window ){
				gtk_widget_set_sensitive( ctrl->window, status );
				gtk_widget_show( ctrl->window );
			}
		}
	}
}



void gtk_text_set_text( GtkText *text, char *txt )
{
	if ( GTK_IS_TEXT(text) ){
		gtk_text_freeze( text );
		gtk_text_set_point( text, 0 );
		gtk_text_forward_delete( text, gtk_text_get_length( text ) );
		gtk_text_insert( text, NULL,NULL,NULL, txt, -1 );
		gtk_text_thaw( text );
	}
}

void SetEntryText( char *name, char *txt )
{
	GtkControl *ctrl = FindControl( name, NULL );	
	if ( ctrl ){
		if( GTK_IS_TEXT (ctrl->window) )
			gtk_text_set_text( GTK_TEXT(ctrl->window), txt);
		if( GTK_IS_ENTRY (ctrl->window) )
			gtk_entry_set_text (GTK_ENTRY (ctrl->window), txt );
		if( GTK_IS_LABEL (ctrl->window) )
			gtk_label_set_text (GTK_LABEL (ctrl->window), txt );
	} 
}

long GetCursorPos( char *name )
{
	GtkWin   *window; GtkControl *ctrl;

	ctrl = FindControl( name, &window );
	if ( ctrl ){
		return gtk_text_get_point ( GTK_TEXT(ctrl->window) );
	} else
		return 0;
}



#define GTK_TEXT_PTR(t, index)        (((t)->use_wchar)  ? ((index) < (t)->gap_position ? &(t)->text.wc[index] :  &(t)->text.wc[(index)+(t)->gap_size]) : ((index) < (t)->gap_position ? &(t)->text.ch[index] :  &(t)->text.ch[(index)+(t)->gap_size]))
static char text_widget[10000];

char *gtk_text_get_text( GtkText *txt )
{
	if ( GTK_IS_TEXT(txt) ){
		char *text;
		long i, l = gtk_text_get_length( txt );
		for(i=0;i<l;i++){
			text_widget[i] = GTK_TEXT_INDEX(txt,i);
		}
		text_widget[l] = 0;
		return text_widget;
	}
	return NULL;
}






static char *temp;
char *GetEntryText( char *name )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	if ( ctrl ){
		if( GTK_IS_TEXT (ctrl->window) )
			return gtk_text_get_text( GTK_TEXT(ctrl->window) );
		if( GTK_IS_ENTRY (ctrl->window) )
			return gtk_entry_get_text (GTK_ENTRY (ctrl->window) );
		if( GTK_IS_LABEL (ctrl->window) ){
			gtk_label_get( GTK_LABEL (ctrl->window), &temp );
			return temp;
		}			
	}
	return NULL;
}

int GetText( char *id, char *out, int n)
{
	if (out){
		char *s = GetEntryText(id);
		if ( s && n ){
			int len;
			strncpy( out, s,  n );
			len = strlen( s );
			if ( len < n )
				out[len] = 0;
		} else
			*out = 0;
	}
	return strlen(out);
}

GtkWidget* gtk_check_box_new(const gchar *label)
{
  GtkWidget *check_button;
  GtkWidget *label_widget;
  
  if ( label ){
		check_button = gtk_check_button_new ();
		label_widget = gtk_label_new (label);
		gtk_misc_set_alignment (GTK_MISC (label_widget), 0.0, 0.5);
		gtk_label_set_line_wrap (GTK_LABEL (label_widget), FALSE );				
		gtk_label_set_justify (GTK_LABEL (label_widget), GTK_JUSTIFY_RIGHT);
		gtk_container_add (GTK_CONTAINER (check_button), label_widget);
		gtk_widget_show (label_widget);
  }
  return check_button;
}


/* turn on/activate the nth item in the popup radio menu */
void gtk_check_menu_setselected( GtkWidget *menu, long n )
{
	GtkWidget *menu_item;
	GList *tmp_list;
	if( tmp_list = g_list_nth (GTK_MENU_SHELL (menu)->children, n) ){
		menu_item = tmp_list->data;
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
	}
}
/* tell us which item is selected, return index # */
long gtk_check_menu_getselected( GtkWidget *menu )
{
	long n = -1;
	if ( menu ){
		GtkWidget *menu_item;	GList *tmp_list;	long i=0;
		
		while( tmp_list = g_list_nth (GTK_MENU_SHELL (menu)->children, i) ){
			menu_item = tmp_list->data;
			if (GTK_CHECK_MENU_ITEM (menu_item)->active ){
				return i;
			} 
			i++;
		}
	}
	return n;
}



void SetPopupNum( char *name, long num )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	if ( ctrl ){
		gtk_check_menu_setselected( ctrl->window2, num );
		gtk_option_menu_set_history (GTK_OPTION_MENU (ctrl->window), num );
	}
}

long GetPopupNum( char *name )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	if ( ctrl ){
		long n;
		n = gtk_check_menu_getselected( ctrl->window2 );
		if ( n <0 ) n = 0;
		return n;
	} else
		return 0;
}


void SetSliderPos( char *name, long pos )
{
	GtkControl *ctrl = FindControl( name, 0 );	
}
long GetSliderPos( char *name )
{
	GtkControl *ctrl = FindControl( name, 0 );	
}
void SetSliderRange( char *name, long i )
{
	GtkControl *ctrl = FindControl( name, 0 );	
}



void SetProgressValue( char *name, long new_val )
{
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl ){
		gtk_progress_set_value( GTK_PROGRESS (ctrl->window), new_val );
	}
}

void SetLabelText( char *name, char *newtext )
{
	GtkControl *ctrl = FindControl( name, 0 );	

	if ( ctrl && newtext )
		gtk_label_set_text( GTK_LABEL (ctrl->window), newtext ); 
}

int WindowPreventResize( char *name )
{
	GtkWin *win = FindWindow( name );
	
	if ( win ){
		gtk_window_set_policy( GTK_WINDOW(win->window), 0, 0, 0 );
		return 0;
		//gtk_window_set_arg( win->window->window, "FALSE" , ARG_ALLOW_GROW );
	} else
		return -1;
}




// ----------- MENU ITEMS

// add a text entry to a popup menu
GSList *AddPopupItem( GtkWidget *menu, GSList *group, char *txt, long on )
{
	if ( menu ){
		GtkWidget *item;
		item = gtk_radio_menu_item_new_with_label (group,txt);
		if ( item ){
			group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (item));
    		gtk_menu_append (GTK_MENU (menu), item);
			if ( on )
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
			gtk_widget_show (item);
			return group;
		}
	}
	return NULL;
}

int AddPopupItems( char *name, char **items )
{
	long	i=0;
	GtkControl *ctrl;	
	
	ctrl = FindControl( name, 0 );

	if ( ctrl ) {
		GtkWidget  *menu;
		menu = ctrl->window2;
		ctrl->items = 0;

		if ( menu && items ){
			GSList *group=NULL;

			while( *items ){
				if ( i==0 )
					group = AddPopupItem( menu, group, *items, 1 );
				else
					group = AddPopupItem( menu, group, *items, 0 );
				items++;
				i++;
				ctrl->items++;
			}
			//if ( ctrl->window )
				gtk_option_menu_set_history (GTK_OPTION_MENU (ctrl->window), 0);
		} else
			return -2;
	} else
		return -1;
	return i;
}


int GetPopupTot( char *name )
{
	long	i=0;
	GtkControl *ctrl;	
	
	ctrl = FindControl( name, 0 );

	if ( ctrl ) {
		return ctrl->items;
	} else
		return 0;
}


int GetPopupText( char *name, int n, char *text )
{
	GtkControl *ctrl;	
	
	ctrl = FindControl( name, 0 );

	if ( ctrl && text ) {
		GtkWidget  *menu, *menu_item;
		menu = ctrl->window2;
		*text = 0;
		if ( menu ){
			GSList *group=NULL;
			if( group = g_list_nth (GTK_MENU_SHELL (menu)->children, n) ){
				char *txt;
				GtkBin *bin;

				bin = GTK_BIN (group->data);
				//menu_item = group->data;
				if ( bin ) {
					if ( bin->child ){
						gtk_label_get (GTK_LABEL (bin->child), &txt );
						strcpy( text, txt );
					} else return -5;
				} else return -4;
			} else return -3;
		} else return -2;
	} else return -1;
	return 0;
}

long Init_ComboBoxString( char *itemname, char *items )
{
	char data[512];
	char *stringPtr[64], *p;
	long	i = 0, c = 0;

//printf( "adding items to %s , %s\n", itemname, items);
	if ( itemname ){
		mystrcpy( data, items );

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
		
		i = AddPopupItems( itemname, stringPtr );
		//if ( i <0 ) printf( "addpopup = %d\n", i );
	}
	return c;
}

long Init_ComboBoxClearAndSelect( void *hDlg, char *comboid, char *data, int sel )
{
	Init_ComboBoxString( comboid, data );
	SetPopupNum( comboid, sel );
}


int AttachProcToPopup( char *name, void *callback )
{
	GtkControl *ctrl;	
	
	ctrl = FindControl( name, 0 );

	if ( ctrl ) {
		GtkWidget  *menu, *menu_item;
		menu = ctrl->window2;

		if ( menu ){
			long n = 0;
			GSList *group=NULL;

			while( group = g_list_nth (GTK_MENU_SHELL(menu)->children, n) ){
				char *txt;
				GtkBin *bin;

				bin = GTK_BIN (group->data);
				//menu_item = group->data;
				if ( bin ) {
					gtk_signal_connect( GTK_OBJECT(bin), "activate",GTK_SIGNAL_FUNC(callback), (long)n );
				}
				n++;
			}
		} else return -2;
	} else return -1;
	return 0;
}




// ----------- LIST STUFF

static void list_getselected_index(GtkWidget *widget, GtkList   *list)
{
	GList *clear_list = NULL;
	GList *sel_row = NULL;
	GList *work = NULL;

	if (list->selection_mode == GTK_SELECTION_EXTENDED) {
		GtkWidget *item;

		item = GTK_CONTAINER (list)->focus_child;

		if (!item && list->selection)
			item = list->selection->data;

		if (item){
			work = g_list_find (list->children, item);
			for (sel_row = work; sel_row; sel_row = sel_row->next)
				if (GTK_WIDGET (sel_row->data)->state != GTK_STATE_SELECTED)
					break;

			if (!sel_row){
				for (sel_row = work; sel_row; sel_row = sel_row->prev)
					if (GTK_WIDGET (sel_row->data)->state != GTK_STATE_SELECTED)
						break;
			}
		}
	}
}


static void list_remove (GtkWidget *widget, GtkList   *list)
{
  GList *clear_list = NULL;
  GList *sel_row = NULL;
  GList *work = NULL;

  if (list->selection_mode == GTK_SELECTION_EXTENDED)
    {
      GtkWidget *item;

      item = GTK_CONTAINER (list)->focus_child;
      if (!item && list->selection)
	item = list->selection->data;

      if (item)
	{
	  work = g_list_find (list->children, item);
	  for (sel_row = work; sel_row; sel_row = sel_row->next)
	    if (GTK_WIDGET (sel_row->data)->state != GTK_STATE_SELECTED)
	      break;

	  if (!sel_row)
	    {
	      for (sel_row = work; sel_row; sel_row = sel_row->prev)
		if (GTK_WIDGET (sel_row->data)->state != GTK_STATE_SELECTED)
		  break;
	    }
	}
    }

  for (work = list->selection; work; work = work->next)
    clear_list = g_list_prepend (clear_list, work->data);

  clear_list = g_list_reverse (clear_list);
  gtk_list_remove_items (GTK_LIST (list), clear_list);
  g_list_free (clear_list);

  if (list->selection_mode == GTK_SELECTION_EXTENDED && sel_row)
    gtk_list_select_child (list, GTK_WIDGET(sel_row->data));
}


static void list_change_txt(GtkList   *list, char *txt)
{
  GList *clear_list = NULL;
  GList *item = NULL;
  GList *work = NULL;

  if (list->selection_mode == GTK_SELECTION_EXTENDED){
      GtkWidget *item;
      item = GTK_CONTAINER (list)->focus_child;
      if (!item && list->selection)
		item = list->selection->data;
		
		if (item){
			GtkBin *bin; char *t;
			work = g_list_find (list->children, item);
			bin = GTK_BIN (work->data);
			gtk_label_get( GTK_LABEL (bin->child), &t ); printf( "t=%s\n", t );
			gtk_label_set_text (GTK_LABEL (bin->child), txt );			
			gtk_widget_show( GTK_WIDGET(bin->child) );
		}
    }
}

static void list_clear (GtkWidget *widget, GtkList *list)
{
	gtk_list_clear_items (GTK_LIST (list), 0, -1);
}

void ChangeListItem( char *name, char *text )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	GtkWidget  *list;
	
	if ( ctrl )
		list = ctrl->window2;
	if ( list ){
		list_change_txt( GTK_LIST(list), text );
	}
}

void list_add_item( GtkWidget *gtklist, char *buffer )
{
	GtkWidget *label, *list_item; char *string;

	if ( gtklist ){
		list_item = gtk_list_item_new_with_label(buffer);
		gtk_container_add(GTK_CONTAINER(gtklist), list_item);
		gtk_object_set_data( GTK_OBJECT(list_item), "name", buffer );
		gtk_widget_show(list_item);
	}
}

void AddItemtoList( char *name, char *text )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	GtkWidget  *list;
	
	if ( ctrl )	list = ctrl->window2;
	if ( list ){
		list_add_item( list,text );
	}
}

void DeleteSelectedListItems( char *name )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	GtkWidget  *list;
	
	if ( ctrl )	list = ctrl->window2;
	if ( list ){
		 list_remove ( ctrl->window, GTK_LIST(list) );
	}
}


void ClearListItems( char *name )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	GtkWidget  *list;
	
	if ( ctrl )	{
		if ( ctrl->window2 )
			 list_clear ( (GtkWidget*)ctrl->window, GTK_LIST(ctrl->window2) );
	}
}


long GetListItemTotal( char *name )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	GtkWidget  *list;
	
	if ( ctrl )	list = ctrl->window2;
	if ( list ){
		return g_list_length( GTK_LIST(list)->children );
	} else return 0;
}

char *GetListItemText( char *name, long n, char *out )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	GtkWidget  *list, *items, *label;  GList *item;
	
	if ( ctrl )
		list = ctrl->window2;

	if ( list && out ){
		char *temp=0;
		GtkBin *bin;
		item = g_list_nth ( GTK_LIST(list)->children, n );
		bin = GTK_BIN (item->data);
		if ( bin ) {
			label = bin->child;
			if ( label ){
				gtk_label_get( GTK_LABEL (label), &temp );
				strcpy( out, temp );
			}
		}
	}
	return NULL;
}


char *GetSelectedListItemText( char *name, char *out )
{
	GtkControl *ctrl = FindControl( name, 0 );	
	GtkWidget  *list, *items, *label;  GList *item;
	
	if ( ctrl )
		list = ctrl->window2;

	if ( list ){
		char *tmp=0;
		GtkBin *bin;
		GList *dlist;

		dlist = GTK_LIST(list)->selection;
		if ( dlist ){
			GtkObject *list_item;
			list_item = GTK_OBJECT( dlist->data );
			tmp = gtk_object_get_data( list_item, "name" );
			if ( out )
				mystrcpy( out, tmp );
			return tmp;
		}
	}
	return NULL;
}



/*
		{	GtkWidget *win = FindWindowPtr( "ABOUT" );
			GtkWidget *x = AddBitMap( win->window, "applicationpro24.BMP" );
		}

*/
//NOTE: EXPERIMENTAL , STILL DOESNT WORK
GtkWidget *AddBitMap( GdkWindow *win, char *file )
{
	GdkPixmap *pixmap; GtkWidget *pixmapwid = NULL;  GdkBitmap *mask;
	
	pixmap = gdk_pixmap_create_from_bmp( win, file );
	if ( pixmap ) {
		/* a pixmap widget to contain the pixmap */
		pixmapwid = gtk_pixmap_new( pixmap, mask );
		if ( pixmapwid )
			gtk_widget_show( pixmapwid );
	}
	return pixmapwid;
}







int ViewFile( char *filename )
{
  GtkWidget *window;
  GtkWidget *box1;
  GtkWidget *box2;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *check;
  GtkWidget *separator;
  GtkWidget *table;
  GtkWidget *vscrollbar;
  GtkWidget *text;
  GdkColormap *cmap;
  GdkColor colour;
  GdkFont *fixed_font;

  FILE *infile;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_usize (window, 640, 500);
  gtk_window_set_policy (GTK_WINDOW(window), TRUE, TRUE, FALSE);  
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
		      GTK_SIGNAL_FUNC(gtk_widget_destroy),
		      NULL);
  gtk_window_set_title (GTK_WINDOW (window), "View");
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);
  
  
  box1 = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), box1);
  gtk_widget_show (box1);
  
  
  box2 = gtk_vbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (box2), 2);
  gtk_box_pack_start (GTK_BOX (box1), box2, TRUE, TRUE, 0);
  gtk_widget_show (box2);
  
  
  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacing (GTK_TABLE (table), 0, 2);
  gtk_table_set_col_spacing (GTK_TABLE (table), 0, 2);
  gtk_box_pack_start (GTK_BOX (box2), table, TRUE, TRUE, 0);
  gtk_widget_show (table);
  
  /* Create the GtkText widget */
  text = gtk_text_new (NULL, NULL);
  gtk_text_set_editable (GTK_TEXT (text), TRUE);
  gtk_table_attach (GTK_TABLE (table), text, 0, 1, 0, 1,
		    GTK_EXPAND | GTK_SHRINK | GTK_FILL,
		    GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (text);

  /* Add a vertical scrollbar to the GtkText widget */
  vscrollbar = gtk_vscrollbar_new (GTK_TEXT (text)->vadj);
  gtk_table_attach (GTK_TABLE (table), vscrollbar, 1, 2, 0, 1,
		    GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (vscrollbar);

  /* Get the system colour map and allocate the colour red */
  cmap = gdk_colormap_get_system();
  colour.red = 0xffff;
  colour.green = 0;
  colour.blue = 0;
  if (!gdk_color_alloc(cmap, &colour)) {
    g_error("couldn't allocate colour");
  }

  /* Load a fixed font */
  fixed_font = gdk_font_load ("-misc-fixed-medium-r-*-*-*-100-*-*-*-*-*-*");

  /* Realizing a widget creates a window for it, ready for us to insert some text */
  gtk_widget_realize (text);

  /* Freeze the text widget, ready for multiple updates */
  gtk_text_freeze (GTK_TEXT (text));

  /* Load the file text.c into the text window */

  infile = fopen( filename, "r");
  
  if (infile) {
    char buffer[1024];
    int nchars;
    
    while (1)
      {
	nchars = fread(buffer, 1, 1024, infile);
	gtk_text_insert (GTK_TEXT (text), fixed_font, NULL,
			 NULL, buffer, nchars);
	
	if (nchars < 1024)
	  break;
      }
    
    fclose (infile);
  }

  /* Thaw the text widget, allowing the updates to become visible */  
  gtk_text_thaw (GTK_TEXT (text));

  gtk_widget_show (window);

//  gtk_main ();
  
  return(0);       
}
/* example-end */





void (*handleMenuFunc)(long i);

int AddMenuToWindow( char *menuID, char *windowID, void *func )
{
	GtkWin *win = FindWindow( windowID );
	GtkWin *menu = FindWindow( menuID );

	if ( win ){
		if (  menu ){
			GtkWidget *menubar;
			handleMenuFunc = func;
			gtk_accel_group_attach (menu->window, GTK_OBJECT (win->window)); 
			menubar = gtk_item_factory_get_widget(menu->vbox, "<main>" ); 
			if ( menubar ){
				gtk_box_pack_start(GTK_BOX(win->vbox), menubar, FALSE, TRUE, 0);
				gtk_widget_show(menubar); 
				menu->area = menubar;
				gtk_widget_set_usize (win->window, win->w, win->h+28 );
				return 0;
			} else return -3;
		} else return -2;
	} else return -1;
}


static void handleMenu( GtkWidget *w, gpointer data ) {
	//g_message("DEBUG: handleMenu() date=%08lx\n", data);      
	if( handleMenuFunc ){
		handleMenuFunc( (long)data );
	}
}

static int debug_rc = 0;

GtkWin *DecodeMenu( char *line, GtkWin *win, long mode )
{
	char	*p, *widget,*id;
	char *argv[256]; int argc=0, i;
static	char menutitle[256], tmp[256];
static	long menu_num=0;
static GtkAccelGroup *accel_group; 
static GtkItemFactory *item_factory=NULL; 
	
	if ( line && win ){
		GtkItemFactoryEntry menu_item = { "", NULL, NULL, 0 , NULL };
		
		widget = SkipSpace( line );
		if( p = (char*)strchr( widget, ' ' ) ){
			*p=0;
			p = SkipSpace( p+1 );
			memset( argv, 0, 63*sizeof(void*) );
			while( (p=(char*)strtok( p, "," )) && (argc<64) ){
		 		argv[ argc++ ] = p;
		 		p=NULL;
			}
		}
//printf( "\t menu=%s\n\t'%s' '%s' '%s'\n", widget,argv[0], argv[1], argv[1] );
		if ( argv[0]  ){
			id = NULL;
			if ( strstr( argv[0], "MENU" ) ){
				menu_num=0;
				accel_group = gtk_accel_group_new(); 
				item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
				win = AddMenu( win , widget, accel_group, item_factory );
				return win;
			} else
			if ( strstr( widget, "POPUP" ) && mode == 4){
				id = argv[0]+1;
				if ( *id == '&' ) *id = '_';
				if ( p=(char*)strchr(id, '\"') ) *p =0;
				sprintf( menutitle, "/%s", id );

				menu_item.path = menutitle;
				menu_item.item_type = "<Branch>";
				menu_num += 0x0100;
				menu_num &= 0xff00;
			} else
			if ( strstr( widget, "MENUITEM" ) && mode == 5){
				id = argv[0];
				if ( *id == '\"' ) id++;
				if ( p=(char*)strstr(id, "\\t") ) strncpy( p, "\09\09", 2 );
				if ( p=(char*)strchr(id, '&') ) *p = '_';
				if ( p=(char*)strchr(id, '\"') ) *p =0; else p=id;
				if ( !strcmp( p, "SEPARATOR" ) ){
					sprintf( tmp, "%s/%s", menutitle, "<Seperator>" );
					menu_item.item_type = "<Separator>";
				} else {
					sprintf( tmp, "%s/%s", menutitle, id );
					menu_item.item_type = NULL;
				}
				menu_item.path = tmp;
				menu_num ++;
				menu_item.callback = handleMenu;
				menu_item.callback_action = menu_num;
			} 
			if ( id ){
				//printf( "\tadd menuitem '%s' '%s' '%lx'\n", menu_item.path, menu_item.item_type, item_factory );
				gtk_item_factory_create_item (item_factory, &menu_item, NULL, 1 ); 
			}
		}
		return win;
	}
	return NULL;
}

static char tmpChar[300];
char *forceMultiLine( char *src, long len )
{
	long i=0; char *d = tmpChar, *space, c;
	while( c=*src++ ){
		if ( c == ' ' ) space = d;
		*d++ = c;
		i++;

		if ( i > len ){
			*space = '\n';
			i = 0;
		}
	}
	*d++ = 0;
	return tmpChar;
}


//-------------------
// decode control strings from RC file and add gtk widgets that
//  emulate them

void DecodeCtrl( char *line, GtkWin *win )
{
	char	*p, *widget, *label, *id, *control, *flags;
	char	*argv[256]; int argc=0, i;
	long	x,y,w,h, visible, keepgroup;
	static GSList *group = NULL;
	
	if ( line && win ){
		GtkControl *ctrl; GtkWidget *obj=0, *obj2;

if ( debug_rc >1 && strstr( line, "Tree" ) ) printf( "control='%s'\n\n", line );

		widget = SkipSpace( line );
		if( p = (char*)strchr( widget, ' ' ) )
		{
			*p=0;
			p = SkipSpace( p+1 );
			memset( argv, 0, 63*sizeof(void*) );
			//p = (char*)strtok( p, "," );
			while( p && (argc<64) )
			{
				p = SkipSpace(p);
		 		argv[ argc ] = p;
				if ( *p == '\"' )
					p = strchr( p+1, '\"' );

				if ( p )
					p = strchr( p, ',' );
				if ( p )
					*p++ = 0;
		 		//p = (char*)strtok( NULL, "," );
				//printf( "argv[%d] = '%s' \n", argc, argv[ argc ] );
				argc++;
			}
		}
		if ( argc>4 && widget )
		{
			label = control = flags = 0; x=y=w=h=0;
			i = 0; obj = obj2 = 0;

			if ( *argv[i] == '\"' || isdigit(*argv[i]) ) {
				label = argv[i++]+1;
				if ( p=(char*)strchr(label, '\"' ) )
					*p=0;
				if ( p=(char*)strchr(label, '&' ) )
					mystrcpy( p, p+1 );
			}

			id = argv[i++];
			if ( *argv[i] == '\"' ) { control = argv[i++]; flags = argv[i++]; }
			if ( argv[i] ) x = atoi( argv[i++] )*SCALEX;
			if ( argv[i] ) y = atoi( argv[i++] )*SCALEY;
			if ( argv[i] ) w = atoi( argv[i++] )*SCALEX;
			if ( argv[i] ) h = (atoi( argv[i++] )+2) *SCALEY;
			if ( !flags && i<argc ) { flags = argv[i++]; }
			
			visible = TRUE;
			if ( flags ){
				if ( strstr( flags, "NOT WS_VISIBLE" ) || strstr( flags, "WS_DISABLED" ) ){
					visible = FALSE;
					//printf( "flags=%s, %s\n",  id,flags );
				}
			}
if ( debug_rc >1 )	printf( "'%s' %d,%d,%d,%d, lab='%s' c='%s' id='%s' f='%s'\n", widget, x,y,w,h,label, control, id, flags );

			keepgroup = FALSE;

			if ( control && !strcmp( widget, "CONTROL" )  ){
				if( flags && label && strstr( control+1, "Button" ) ){
					if ( strstr( flags, "BS_OWNERDRAW" ) )
					    obj = gtk_button_new_with_label ( label );
					else
					if ( strstr( flags, "BS_AUTOCHECKBOX" ) )
					    obj = gtk_check_box_new ( label );
					else
					if ( strstr( flags, "BS_AUTORADIOBUTTON" ) ){
					    obj = gtk_radio_button_new_with_label ( group, label );
						group = gtk_radio_button_group (GTK_RADIO_BUTTON (obj));
						keepgroup = TRUE;
					}
				} else
				if ( strstr( control+1, "SysTreeView32" ) ){
					obj = gtk_scrolled_window_new(NULL, NULL);
					gtk_container_set_border_width (GTK_CONTAINER (obj), 0);
					gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (obj),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

					obj2 = gtk_tree_new();
					gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (obj), obj2);
					gtk_tree_set_selection_mode( GTK_TREE(obj2), GTK_SELECTION_SINGLE );
					gtk_widget_show( obj2 );
				} else
				if ( strstr( control+1, "SysTabControl32" ) ){
					obj = gtk_notebook_new( );
					gtk_notebook_set_tab_pos (GTK_NOTEBOOK (obj), GTK_POS_TOP);
					//gtk_table_attach_defaults(GTK_TABLE(table), notebook, 0,6,0,1);
					y+=8; h+=6;
				} else
				if ( strstr( control+1, "SysListView32" ) ){		// TABLED LISTVIEW
					obj = gtk_scrolled_window_new(NULL, NULL);
					gtk_container_set_border_width (GTK_CONTAINER (obj), 0);
					gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (obj),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
					
					obj2 = gtk_clist_new_with_titles( 1, NULL );
					gtk_clist_set_shadow_type (GTK_CLIST(obj2), GTK_SHADOW_OUT);
					gtk_container_add (GTK_CONTAINER (obj), obj2);
					gtk_widget_show( obj2 );
				} else
				if ( strstr( control+1, "msctls_trackbar32" ) ){		// SLIDER
					obj2 = (GtkWidget *)gtk_adjustment_new (1.0, 0.0, 16.0, 1.0, 1.0, 0.0 );
					obj = gtk_hscale_new( GTK_ADJUSTMENT ( obj2 ) );
					gtk_range_set_update_policy (GTK_RANGE (obj), GTK_UPDATE_CONTINUOUS );
					gtk_scale_set_digits (GTK_SCALE (obj), 0);
				} else
				if ( strstr( control+1, "msctls_progress32" ) ){		// PROGRESS BAR
					obj2 = (GtkWidget *)gtk_adjustment_new (0, 1, 100, 0, 0, 0); 
					obj = gtk_progress_bar_new_with_adjustment ((GtkAdjustment *)obj2);
					gtk_progress_set_format_string (GTK_PROGRESS (obj),"%p%%"); 
				} else
				if ( strstr( control+1, "Static" ) ){		// BITMAP
					if ( flags && strstr( flags, "SS_BITMAP" ) ){
						obj = gtk_fixed_new(); flags=0;
					}
					if ( flags && strstr( flags, "SS_SUNKEN" ) ){
						obj = gtk_label_new( label );
						if ( !strstr( flags, "NOWORDWRAP" ) )
							gtk_label_set_line_wrap (GTK_LABEL (obj), TRUE);
						gtk_misc_set_alignment( GTK_MISC (obj), 0.0 , 0.0 );
						if ( strstr( flags, "SS_LEFT" ) )
							gtk_label_set_justify (GTK_LABEL (obj), GTK_JUSTIFY_LEFT );
						else
							gtk_label_set_justify (GTK_LABEL (obj), GTK_JUSTIFY_RIGHT );
					}
				}
			} else
			if ( !strcmp( widget, "PUSHBUTTON" ) && label ){
				if( flags ){
					if ( strstr( flags, "WS_DISABLED" ) || strstr( flags, "NOT WS_VISIBLE" ) )
						obj = NULL;
					else
						obj = gtk_button_new_with_label ( label );
				} else
					obj = gtk_button_new_with_label ( label );
			} else
			if ( !strcmp( widget, "DEFPUSHBUTTON" ) && label ){
				obj = gtk_button_new_with_label ( label );
			} else
			if ( !strcmp( widget, "GROUPBOX" ) && label ){
				obj = gtk_frame_new( label );
				gtk_container_set_border_width(GTK_CONTAINER(obj), 3);
			} else
			if ( !strcmp( widget, "EDITTEXT" ) ){
				h-=2;
				if( flags && strstr( flags, "ES_MULTILINE" ) && flags ){
					obj = gtk_text_new (NULL, NULL);
					gtk_text_set_editable (GTK_TEXT (obj), TRUE);
				} else {
					obj = gtk_text_new (NULL, NULL);
					gtk_text_set_editable (GTK_TEXT (obj), TRUE);
					gtk_text_set_line_wrap (GTK_TEXT (obj), FALSE);
					//obj = gtk_entry_new_with_max_length( 255 );
				}
			} else
			if ( !strcmp( widget, "LTEXT" ) && label ){
				if( !strstri( flags, "WS_DISABLED" ) ){
					long max = w/5;
					if ( strchr(label,' ') && strlen(label)>max ){
						obj = gtk_label_new( forceMultiLine( label, max ) );
						h += 8;
					} else {
						obj = gtk_label_new( label );
						gtk_label_set_line_wrap (GTK_LABEL (obj), TRUE);
					}
					gtk_misc_set_alignment( GTK_MISC (obj), 0.0 , 0.0 );
					gtk_label_set_justify (GTK_LABEL (obj), GTK_JUSTIFY_LEFT);
				}
			} else
			if ( !strcmp( widget, "RTEXT" ) && label ){
				long max = w/5;
				if ( strchr(label,' ') && strlen(label)>max ){
					obj = gtk_label_new( forceMultiLine( label, max ) );
					h += 8;
				} else {
					obj = gtk_label_new( label );
				}
				gtk_label_set_justify (GTK_LABEL (obj), GTK_JUSTIFY_RIGHT);
				gtk_misc_set_alignment( GTK_MISC (obj), 1.0 , 0.0 );
				//gtk_label_set_line_wrap (GTK_LABEL (obj), TRUE);				
				h += 8;
			} else
			if ( !strcmp( widget, "LISTBOX" ) ){
				obj = gtk_scrolled_window_new(NULL, NULL);
				if ( obj ){
					gtk_container_set_border_width (GTK_CONTAINER (obj), 0);
					gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (obj),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
					if( obj2 = gtk_list_new( ) ){
						if ( strstr( flags, "LBS_EXTENDEDSEL" ) )
							gtk_list_set_selection_mode (GTK_LIST (obj2), GTK_SELECTION_EXTENDED);
						else
						if ( strstr( flags, "LBS_MULTIPLESEL" ) )
							gtk_list_set_selection_mode (GTK_LIST (obj2), GTK_SELECTION_MULTIPLE);
						else
							gtk_list_set_selection_mode (GTK_LIST (obj2), GTK_SELECTION_SINGLE);

						gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(obj),obj2);
						gtk_widget_show( obj2 );
					}
				}
			} else
			if ( !strcmp( widget, "COMBOBOX" ) ){
				obj = gtk_option_menu_new();
				obj2 = gtk_menu_new();
				gtk_option_menu_set_menu (GTK_OPTION_MENU (obj), obj2);
				gtk_option_menu_set_history (GTK_OPTION_MENU (obj), 0);
				h = 16*SCALEY;		//default height
				//AddPopupItem( obj2, "combo test...1" );
			}

			if ( obj ){
				gtk_fixed_put (GTK_FIXED (win->area), obj, x, y);
				if ( w && h ) gtk_widget_set_usize( obj, w,h );
				if ( visible ) gtk_widget_show( obj );
				ctrl = AddControl( win, id, obj, obj2, x, y, w, h );
				
				if ( !keepgroup ) group = NULL;
			}
		}
	}
}


int AddLabelToWindow( char *labelID, char *windowID, char *label, long x, long y )
{
	GtkWin *win;
	if ( win=FindWindow( windowID ) ){
		GtkWidget *obj;
		long w=strlen(label)*8, h = 12;

		x=x*SCALEX; y=y*SCALEY;
		w=w*SCALEX; h=h*SCALEY;
		obj = gtk_label_new( label );
		if ( obj ){
			gtk_label_set_justify (GTK_LABEL (obj), GTK_JUSTIFY_LEFT);
			gtk_label_set_line_wrap (GTK_LABEL (obj), TRUE);
			gtk_fixed_put (GTK_FIXED (win->area), obj, x, y);
			if ( w && h )
				gtk_widget_set_usize( obj, w,h );
			gtk_widget_show( obj );
			AddControl( win, labelID, obj, 0, x, y, w, h );
			return 0;
		} else
			return -1;
	} else
		return -2;
}

void CloseWindowProc( void *w, void *d )
{
	if ( GTK_WIDGET_VISIBLE (w) )
		gtk_widget_hide( w );
}

void CloseWindow( char *wname )
{
	GtkWidget *w;
	
	if ( w = FindWindowPtr( wname ) )
		gtk_widget_hide( w );
}

void SetWindowExitProc( char *windowName, void *proc )
{
	GtkWidget *win;
	
	if ( win = FindWindowPtr( windowName ) )
		gtk_signal_connect (GTK_OBJECT (win), "delete_event",GTK_SIGNAL_FUNC (proc), &win);
}


GtkWin *DecodeDialog( char **argv, int argc, char *buf )
{
static	long x,y,w,h;
static	char wname[256], wtitle[256], style[256];
		long i=1;
	
	if ( argc>5 && strstr(argv[i], "DIALOG" ) ){
		i++;
		if ( !isdigit( *argv[i] ) ) i++;
		x = atoi( argv[i++] )*SCALEX;
		y = atoi( argv[i++] )*SCALEY;
		w = atoi( argv[i++] )*SCALEX;
		h = atoi( argv[i++] )*SCALEY;
		strcpy( wname, argv[0] );
	} else
	if( !strcmp( argv[0], "STYLE" ) ){
		*buf = 0;
		strcpy( style, buf );
	} else
	if( !strcmp( argv[0], "CAPTION" ) ){
		char *p = (char*)strchr( buf, '\"'  );
		if ( p ){
			strcpy( wtitle, p );
			p = (char*)strchr( wtitle, '\"' );
			if( p ) *p = 0;
		}
	} else
	if( !strcmp( argv[0], "BEGIN" ) ){
		GtkWidget *win, *area, *vbox, *menubox;

if ( debug_rc >0 )
printf( "decoding window...%s(%s), at (%d,%d), size(%d,%d)\n" , wname,wtitle, x,y,w,h );

		if ( strstr( wname, "SPLASH" ) || strstr( wname, "ABOUT" ) )
			h+=35;

		win = gtk_window_new (GTK_WINDOW_TOPLEVEL);		//win = gtk_dialog_new();

		if ( win ){
    		gtk_signal_connect (GTK_OBJECT (win), "delete_event",GTK_SIGNAL_FUNC (CloseWindowProc), &win);

			if ( *wtitle )
	    		gtk_window_set_title (GTK_WINDOW (win), wtitle );
			else
	    		gtk_window_set_title (GTK_WINDOW (win), wname );

			gtk_window_set_policy (GTK_WINDOW (win), TRUE, TRUE, FALSE);	

			gtk_container_set_border_width (GTK_CONTAINER (win), 1);
			gtk_widget_set_uposition( win, x,y );
			gtk_widget_set_usize( win, w,h );

			area = gtk_fixed_new ();
			gtk_object_set_data (GTK_OBJECT (win), "fixed1", area);
			gtk_widget_show (area);
			//gtk_container_add (GTK_CONTAINER (win), area);

			vbox = gtk_vbox_new(FALSE, 1);
			gtk_container_border_width(GTK_CONTAINER(vbox), 0);
			gtk_widget_show(vbox);

			menubox = gtk_vbox_new(FALSE, 1);
			gtk_widget_show(menubox);
			gtk_box_pack_start (GTK_BOX(vbox), menubox, FALSE, TRUE, 0);
			gtk_box_pack_start (GTK_BOX(vbox), area, FALSE, TRUE, 0);
			gtk_container_add(GTK_CONTAINER(win), vbox);

			
			gCurrWin = AddWindow( gCurrWin, wname, win,menubox,area,  x,y,w,h );
			
			wname[0] = 0;
			wtitle[0] = 0;
			x=y=w=h=0;
			return gCurrWin;
		}
	}
	return NULL;
}


char *Fgets( char *buffer, long len, char *ptr )
{
	unsigned char *p = buffer, copy=TRUE, c;
	while( copy ){
		c = *(ptr++);
		if ( c==13 ) c = *(ptr++);
		*(p++) = c;
		if ( c==13 | (c==10) || (c==0xff) )
			copy = FALSE;
	}
	*p++=0;
	return ptr;
}

#define	CHECKEOF \
		if ( ramFile ) { if(*ramfp==0xffffffff) eof=TRUE; else eof=FALSE; } \
		else eof = feof( fp );

#define	READLINE( b ) \
		if ( ramFile ) ramfp = Fgets( b, 255, ramfp ); \
		else fgets( b, 255, fp );
		



// now this is tricky, you can use a file name, or pointer to the file
// in memory

GtkWidget **ParseWindowsRC( char *file, char *depend, int debuglevel )
{
	long mode=0, ignoredialog=0;		// 0=none, 1=dialog header, 2=control
	FILE *fp;
	char 	buf[1024], line[1024];
	char	*argv[256] , *p, *ramfp;
	int 	argc, count = 0, linenum=0, len, ramFile = FALSE, eof = FALSE, mtries;
	GtkWin *win;
	
	gWins = malloc( GTKWINSIZE );
	memset( gWins, 0, GTKWINSIZE );
	gCurrWin = gWins;

	debug_rc = debuglevel;

	if ( !file) return NULL;

	if ( strstr( file, "resource script" ) ) ramFile = TRUE;

	// open file
	if ( ramFile ) ramfp = file;
	else
	if( !(fp = fopen( file, "r" )) ) return NULL;
	
	// loop for each line, but if last char has , or | merge the linse to one
	while( !eof ){
		READLINE( buf );
		CHECKEOF
		
		len = strlen(buf); if( len<3 ) continue;
		//printf( ">%s<\n", buf );

		mtries = 4;

		while( mtries>0 ){
			if ( buf[len-3] == '|' || buf[len-2] == '|' || buf[len-2] == ',' ){
				READLINE( &buf[len-1] );
				len = strlen(buf);
			} else
				break;
			mtries--;
		}

		CHECKEOF
		buf[len-1]=0;
		strcpy( line,buf );

		// tokenize line
		p = line; argc=0;
		memset( argv, 0, 63*sizeof(void*) );
		while( (p=(char*)strtok( p, " " )) && (argc<64) )
		{
		 	argv[ argc++ ] = p;
		 	p=NULL;
		}

		// handle DEPENDANT dialogs that are to be used/ignored
		if( strstr( buf, "#if" ) ){
			if( strstr( buf, depend ) || strstr( buf, "AFX_TARG_EN" ) )
				ignoredialog=0;
			else
				ignoredialog++;
		}		
		if( (strstr( buf, "#endif" ) || strstr( buf, "#else" )) && ignoredialog>0 )
			ignoredialog--;
		//if ( ignoredialog ) printf( "IGNORE %d: %s\n", count, buf );
		
		// check for dialog definition
		if ( ignoredialog == 0 ){
			if ( argc>4 ){
				if( strstr( argv[1], "DIALOG" ) && mode==0){
					mode = 1;
					if( strchr( argv[0], '$' ) ) mode = 0;
				}
			} else
			if ( argc>2 ){
				if( strstr( argv[1], "MENU" ) && mode==0)
					mode = 3;
			}
//printf( "valid %d: argc=%d, mode=%d, arg0='%s'\n", count, argc, mode, argv[0] );
			// find start of control definitions
			if( !strcmp( argv[0], "BEGIN" ) ) {
//printf( "being found\n" );
				switch ( mode ){
					case 1:	win = DecodeDialog( argv, argc, buf ); mode=2; continue;
					case 3: mode = 4; continue;
					case 4: mode = 5; continue;
					case 5: mode = 6; continue;
				}
			} else
			if( !strcmp( argv[0], "END" ) ) {
				switch( mode ){
					case 2:	mode = 0;	continue;
					case 3:	mode = 0;	continue;
					case 4:	mode = 3;	continue;
					case 5:	mode = 4;	continue;
					case 6:	mode = 5;	continue;
				}
			}
			// if mode1, decode dialog params, add window
			// if mode2, decode control
			switch( mode ){
				case 1:
					win = DecodeDialog( argv, argc, buf );
					break;
				case 2:
					DecodeCtrl( buf, win );
					break;
				case 3:
				case 4:
				case 5: case 6:
					win = DecodeMenu( buf, win, mode );
					break;
				case 0:
					//printf( "\n" );
					break;
			}	
		}
		count++;	
	}
	if ( !ramFile )
		fclose(fp);
}

// -------------- EXAMPLE OF HOW TO USE THE ABOVE LIBRARY
/*
int main( int argc, char *argv[] )`
{
    gtk_init( &argc, &argv );
	
	ParseWindowsRC( "win32.rc", "_PRO" );
	
	if ( argc>1 ){
		if ( !ShowWindow( argv[1] ) )
			printf( "\n%s not found\n", argv[1] );
	} else
		ShowAllWindows();
   
    gtk_main ();
} 
*/
