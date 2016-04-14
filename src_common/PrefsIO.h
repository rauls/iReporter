
#ifndef	PREFSIO_H
#define	PREFSIO_H


#if DEF_UNIX
typedef void * HWND;
#endif

void	Report_GUItoData( void );
void	Report_DatatoGUI( void );

void PrefsFromGUI( void );
void PrefsIntoGUI( void );
void LanguageString_GUItoData( HWND parent, long comboId, char *language );
void ReportFormat_GUItoData();

void ShowCustomConfigInfo( HWND hDlg );
long SetHTMLColors( HWND hDlg );

void Report_PDFPageAdv_DataToGUI( HWND hDlg );
void Report_PDFPageAdv_GUIToData( HWND hDlg );

long SetHTMLColorsGUI( HWND hDlg );

void FixReportOutput( HWND hDlg );
void ShowDiskFree( HWND hDlg, char *file );
void ChangePDFPageSize();
void ChangeTableTextNameCombo();
void ChangeSpacingSizeCombo();
void ChangeGraphFontsNameCombo();
void ChangeSpacingSizeEdit();
void ChangeTextWrap();

void	PreProc_GUItoData(void);
void	PreProc_DatatoGUI(void);
void	Analysis_GUItoData(void);
void	Analysis_DatatoGUI(void);

void	PostProc_GUItoData( void );
void	PostProc_DatatoGUI( void );

#endif
