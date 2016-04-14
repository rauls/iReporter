#ifndef	__XAPP_H
#define __XAPP_H


#ifdef __cplusplus
extern "C" {
#endif


void SetWindowText( const char *txt );
void UpdateTitleBar( void );
void SaveLogListAsText( void );
void SetStatusBar( char *txt, long val );
long FindLogInGUIHistory( char *logtofind );
long XAddLogToGUIHistory( char *filename, long item );
void ReDrawListView( );
void ReadLogListAsText( void );
long CopySelectedHistory_ToLOGQ( void );
void OnProcessLog( void *w, void *d );
void ShowHTMLURL( char *url );
void ViewLocalFile( char *filepath );

void XStatusWindowSetText( char *txt );
long XAskForRegistration( char *name, char *comp, char *serial );

int BeginGUI( int argc, char *argv[] );

#ifdef __cplusplus
}
#endif
  

#endif