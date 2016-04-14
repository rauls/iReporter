#ifndef	__XLATE
#define	__XLATE

#include <stdio.h>

#include "translateids.h"

#define MONTHS			12
#define WEEKDAYS		7
#define BUF16			16

extern char *EMPTY_STR;
#define ENGLISH_LANG "English"

#ifdef __cplusplus
// Tuff luck, you only get this function if you're a cpp!
#include <string>
void ConvertHTMLCharacterTokens( std::string &str );
bool GetLangHelpCardsPath( std::string& helpCardsPath, const char *language, const char *testFileName );
extern "C" {
#endif

long InitLanguage( char *lang, short forceLoad );
char *GetLangName( long n );
void LanguageError( long level );
long GetLangVersion( void );
char * TranslateID( long id );
char * DefaultEnglishStr( long id );
unsigned char ConvertHTMLStrToAsciiCharCode( char *str );
void SwapHTMLCharacterTokens( char* str, char* replaceHTMLTokensStr );
const char* LoadLangStringConvertFromHTML( long stringId );
void LoadWeekdaysAndMonths( char weekdaysT[WEEKDAYS+1][BUF16], char monthsT[MONTHS][BUF16], char MonthsT[MONTHS][BUF16] );
short HaveReadLangFile();
void GetLangV41File( char *langsPath, char *appPath, char *relPath, char *file );
void GetLangV37File( char *langsPath, char *appPath, char *relPath, char *file );
char *GetLangSectionName( short index );
FILE *CreateNewLang( char *file );
void AddNewLangString( char *str, short index );
void WriteNewLangString(  FILE *fp, short index );
void ClearNewLangString( short index );
short RealLangToken( short index );

#define TranslateString	TranslateID

#ifdef __cplusplus
}
#endif



#endif

