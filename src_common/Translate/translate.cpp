/*

  Lang Ver 2 : 

  PROS
  * easy to code
  * easy to remember tokens or the original english words
  * easy to use lang file that isnt convoluted in number id orders etc
  * its already working, but needs completed language files

  CONS
  * slight perfomance hit, but with todays 500mip+ cpus its not an issue unless your using a 286 or 68030 (pre 1990 hardware)
  * if english words change, then they must reflect the otherlang files, solution: do global search/replace



  Lang Ver 3 : (Richards {XXnn} tokens built to multi tables of 100 each for easy reference

  PROS
  * very fast to find text, but not noticeable
  * allows english defaults
  * no miss intepretations

  CONS
  * requires all lang files to be redone
  * requires all source files to use tokens (a lot of changes)
  * requires ENUM (#define style) definitions to represent the #s that must match (very fragile)
  * requires all lang files to match in #s, ie {DD34} has to match to the enum which is really something like IDS_DAILY
  * limited to 100 entries per block, but that can be changed later... by dynamic code..
  * makes coding with tokens a pain since you have to manualy lookup the tokento see what it really is.


  */

#include "FWA.h"

#include <map>
#include <string>
#include <string.h>				// need this for ANSI functions we haven't got in myansi.c
#include "myansi.h"
#include "ResDefs.h"
#include "translate.h"
#include "GlobalPaths.h"

#ifdef DEF_MAC
	#include "main.h"
	#include "MacStatus.h"
	#include "MacUtil.h"
#endif

#include "Hash.h"	// for HashIt

extern "C" long OutDebug (const char *txt);
extern "C" long OutDebugs( const char *txt, ... );
char *EMPTY_STR = 0;

#include "English.h"

const char *LANGDIR_V41 = "Languages";
const char *LANGDIR_V37 = "lang";
const char *LANG_EXT = ".lang";
const char *HELP_CARDS = "HelpCards";

// Unix paths
const char *USER_LOCAL_APP	= "/usr/local/Analyzer/";
const char *LOCAL_APP		= "/local/Analyzer";
const char *APP			= "/opt/Analyzer";


bool TestToSeeIfLangHelpCardPathExists( std::string& helpCardsPath, const char *appPath, const char* relPath, const char *language, const char *testFileName )
{
	FILE *testfp;

	if ( appPath && relPath )
	{
		helpCardsPath += appPath;
#ifndef DEF_MAC
		helpCardsPath += relPath;		// Mac gPath already has separator at end
#endif
	}
	if ( language )
	{
		helpCardsPath += LANGDIR_V41;
		helpCardsPath += PATHSEPSTR;
		helpCardsPath += language;
		helpCardsPath += PATHSEPSTR;
		helpCardsPath += HELP_CARDS;
		helpCardsPath += PATHSEPSTR;
	}
	if ( testFileName )
	{
		std::string testFile = helpCardsPath + testFileName;
		testfp = fopen( testFile.c_str(), "rb" );
		if ( testfp )
		{
			fclose( testfp );
			return true;
		}
	}
	helpCardsPath.resize( 0 );
	return false;
}

bool GetLangHelpCardsPath( std::string& helpCardsPath, const char *language, const char *testFileName )
{
	// Check the exe directory first - as this is where users should have their languages directory
	if ( TestToSeeIfLangHelpCardPathExists( helpCardsPath, gPath, PATHSEPSTR, language, testFileName ) )
		return true;
	
#if DEF_WINDOWS
	// Now check for a lang directory which is located back a directory - this where we have our languages directory
	if ( TestToSeeIfLangHelpCardPathExists( helpCardsPath, gPath, PATHBACKDIR, language, testFileName ) )
		return true;
#endif

#if DEF_UNIX
	if ( TestToSeeIfLangHelpCardPathExists( helpCardsPath, USER_LOCAL_APP, PATHBACKDIR, language, testFileName ) )
		return true;
	if ( TestToSeeIfLangHelpCardPathExists( helpCardsPath, LOCAL_APP, PATHBACKDIR, language, testFileName ) )
		return true;
	if ( TestToSeeIfLangHelpCardPathExists( helpCardsPath, APP, PATHBACKDIR, language, testFileName ) )
		return true;
	if ( TestToSeeIfLangHelpCardPathExists( helpCardsPath, "", "", language, testFileName ) )
		return true;
	if ( TestToSeeIfLangHelpCardPathExists( helpCardsPath, 0, 0, language, testFileName ) )
		return true;
#endif
	return false;
}


void GetLangV41File( char *langsPath, char *appPath, char *relPath, char *file )
{
	if ( appPath && relPath )
	{
		mystrcpy( langsPath, appPath );
		mystrcat( langsPath, relPath );
		mystrcat( langsPath, LANGDIR_V41 );
		mystrcat( langsPath, PATHSEPSTR );
	}
	if ( file )
	{
		mystrcat( langsPath, file );
		mystrcat( langsPath, LANG_EXT );
	}
}

void GetLangV37File( char *langsPath, char *appPath, char *relPath, char *file )
{
	// Use path information if given it
	if ( appPath && relPath )
	{
		mystrcpy( langsPath, appPath );
		mystrcat( langsPath, relPath );
		mystrcat( langsPath, LANGDIR_V37 );
		mystrcat( langsPath, PATHSEPSTR );
	}
	if ( file )
	{
		mystrcat( langsPath, file );
		mystrcat( langsPath, LANG_EXT );
	}
}

FILE *OpenLang( char *file )
{
	char fullname[512];
	FILE *fp=0;
#if DEF_WINDOWS
		//
		// Searching "languages" directories first, to be consistent with the mac, and for version 4 files
		//
		// Check the exe directory first - as this is where users should have their lang directory
		GetLangV41File( fullname, gPath, PATHSEPSTR, file ); 
		if( fp = fopen( fullname, "rb" ))  return fp;
		
		// Now check for a lang directory which is located back a directory - this where we have our lang directory
		GetLangV41File( fullname, gPath, PATHBACKDIR, file ); 
		if( fp = fopen( fullname, "rb" ))  return fp;
		
		//
		// Searching the old "lang" directories second, just in case the new "languages" directory doesn't exist
		//
		// Check the exe directory first - as this is where users should have their lang directory
		GetLangV37File( fullname, gPath, PATHSEPSTR, file ); 
		if( fp = fopen( fullname, "rb" ))  return fp;
		
		// Now check for a lang directory which is located back a directory - this where we have our lang directory
		GetLangV37File( fullname, gPath, PATHBACKDIR, file ); 
		if( fp = fopen( fullname, "rb" ))  return fp;
		
		// Try the current directory - basically, a last resort...
		GetLangV41File( fullname, 0, 0, file ); 
		if( fp = fopen( fullname, "rb" ))  return fp;

#elif DEF_MAC
		mystrcpy( fullname, gPath) ;
		mystrcat( fullname, LANGDIR_V41 );
		mystrcat( fullname, PATHSEPSTR );
		mystrcat( fullname, file );
		mystrcat( fullname, LANG_EXT );

		if (fp = fopen (fullname, "r"))
			return fp;
		else
		{
			char msg[256];
			sprintf( msg, "\"%s\" folder not found", LANGDIR_V41 );
			// REQUIRE RESOURCE STRING
			UserMsg( msg );
			
			mystrcpy( fullname, gPath );
			mystrcat( fullname, LANGDIR_V37 );
			mystrcat( fullname, PATHSEPSTR );
			mystrcat( fullname, file );
			mystrcat( fullname, LANG_EXT );

			if (fp = fopen( fullname, "r" ))
				return fp;
			else
			{
				UserMsg ("No language file found");
				// REQUIRE RESOURCE STRING
				return 0;
			}
		}
#else
		char name[256];
		mystrcpylower( name,file );
		GetLangV41File( fullname, (char*)USER_LOCAL_APP, PATHSEPSTR, file ); 
		if( fp = fopen( fullname, "rb" ) ) return fp;

		GetLangV41File( fullname, (char*)LOCAL_APP, PATHSEPSTR, file ); 
		if( fp = fopen( fullname, "rb" ) ) return fp;

		GetLangV41File( fullname, (char*)APP, PATHSEPSTR, file ); 
		if( fp = fopen( fullname, "rb" ) ) return fp;

		GetLangV41File( fullname, "", "", file ); 
		if( fp = fopen( fullname, "rb" ) ) return fp;

		GetLangV41File( fullname, 0, 0, file ); 
		if( fp = fopen( fullname, "rb" ) ) return fp;
#endif
	return fp;
}

FILE *CreateNewLang( char *file )
{
	char fullname[512];
	FILE *fp=0;
#if DEF_WINDOWS
		//
		// Searching "languages" directories first, to be consistent with the mac, and for version 4 files
		//
		// Check the exe directory first - as this is where users should have their lang directory
		GetLangV41File( fullname, gPath, PATHSEPSTR, file ); 
		if( fp = fopen( fullname, "wb" ))  return fp;
		
		// Now check for a lang directory which is located back a directory - this where we have our lang directory
		GetLangV41File( fullname, gPath, PATHBACKDIR, file ); 
		if( fp = fopen( fullname, "wb" ))  return fp;

		// Try the current directory - basically, a last resort...
		GetLangV41File( fullname, 0, 0, file ); 
		if( fp = fopen( fullname, "wb" ))  return fp;
#endif
/* When implementing for Mac & Unix, please check the following code... as directories need to be specified...
#elif DEF_MAC
		mystrcpy( fullname, gPath) ;
		mystrcat( fullname, LANGDIR_V41 );
		mystrcat( fullname, PATHSEPSTR );
		mystrcat( fullname, file );
		mystrcat( fullname, LANG_EXT );

		if (fp = fopen (fullname, "w"))
			return fp;
#else
		mystrcpylower( name,file );
		
		GetLangV41File( fullname, "/usr/local/application4", PATHSEPSTR, name ); 
		if( fp = fopen( fullname, "wb" ) ) return fp;

		GetLangV41File( fullname, "/local/application4", PATHSEPSTR, name ); 
		if( fp = fopen( fullname, "wb" ) ) return fp;

		GetLangV41File( fullname, "/application4", PATHSEPSTR, name ); 
		if( fp = fopen( fullname, "wb" ) ) return fp;

		GetLangV41File( fullname, "", "", name ); 
		if( fp = fopen( fullname, "wb" ) ) return fp;

		GetLangV41File( fullname, 0, 0, name ); 
		if( fp = fopen( fullname, "wb" ) ) return fp;
#endif*/
	return fp;
}




#define MAXTOKENS		END_OF_STRINGS

static char *default_lang = ENGLISH_LANG;
static char *LanguageFile = NULL;
static char *DefaultLangFile = NULL;
typedef struct TokenlookupStruct
{
	char *langFilelinePtr;
	long key;
	char *string;
	char *defaultEnglish;
} Tokenlookup;
static Tokenlookup tokenlookup[MAXTOKENS];
static short readLangFile = 0; // Indicates that the language file has been initialised
static long	LangVersion = 0, totalLookups = 0;

short HaveReadLangFile()
{
	return readLangFile;
}

#define NEWLINE	10
#define RETURN	13
#define TAB		9

// Eats up "normal" characters until one of the delimiting chars is found (this is after a comment)
char *EatUpNormalCharsLine( char *p )
{
	while ( *p != NEWLINE && *p != RETURN && *p != TAB && *p != 0 )
		p++;
	return p;
}

short ReadGroup( long& lasttk, char **groupTokenPtr, long& group )
{
	if ( groupTokenPtr ){
		char *p, c;
		long groupend;

		p = *groupTokenPtr; 
		c = *p;
		// Get the token to see if it is the start of a category...
		long tk = ReadLong( (unsigned char *)++p );
		if ( (tk&0xffff0000) != lasttk )
		{
			// We have a new category, so find out which one...
			switch( tk>>16 )
			{
				case 'ss': group = SUMM_BEGIN; groupend = SUMM_END; break;
				case 'rc': group = RCAT_BEGIN; groupend = RCAT_END;  break;
				case 'rl': group = REPORT_BEGIN; groupend = REPORT_END;  break;
				case 'gl': group = LABEL_BEGIN; groupend = LABEL_END;  break;
				case 'ti': group = TIME_BEGIN; groupend = TIME_END;  break;
				case 'se': group = SESS_BEGIN; groupend = SESS_END;  break;
				case 'cm': group = COMM_BEGIN; groupend = COMM_END;  break;
				case 'er': group = SERR_BEGIN; groupend = SERR_END;  break;
				case 'sy': group = SYSI_BEGIN; groupend = SYSI_END;  break;
				case 'co': group = COUN_BEGIN; groupend = COUN_END;  break;
				case 'or': group = ORGS_BEGIN; groupend = ORGS_END;  break;
				default:
					OutDebugs( "Language File - group '%c%c' found, this group is not being translated.\nThe language file is incorrect, please rectify.", tk>>16, tk>>24 );
					p = EatUpNormalCharsLine( p ); // Eat up the line
					*groupTokenPtr = p;
					return 0;
			}

			lasttk = tk&0xffff0000;
			p = EatUpNormalCharsLine( p ); // Eat up the line
			*groupTokenPtr = p;
			return 1;
		}
	}
	return 0;
}

// Checks to see if we are starting a comment line
short CommentLine( char *p )
{
	if ( *p == '#' || *p == ';' || *p == '/' )
		return 1;
	return 0;
}

// Eats up crappy chars which delimit a line, 
char *EatUpCrappyChars( char *p )
{
	while ( *p == NEWLINE || *p == RETURN || *p == TAB || *p == 0 )
	{
		if ( *p == 0 )
			return p;
		p++;
	}
	return p;
}

char *FindGroupStartToken( char *p )
{
	while ( *p != NEWLINE && *p != RETURN && *p != TAB && *p != 0 && *p != '{' )
		p++;
	return p;
}

void SetLangVersion( char *p )
{
	// Check for the version string in the language file
	if ( LangVersion == 0 )
	{
		if ( strstr( p, "Version 3") )
			LangVersion = 3;
		else if ( strstr( p, "Version 4") )
			LangVersion = 4;
		else
			LangVersion = 2;
	}
}

char *AddTheStringToArray( char *p, long i, long tk )
{
	tokenlookup[i].langFilelinePtr = p; // Keep track of the entire line, dunno why?
	tokenlookup[i].key = tk; // Keep track of the key for some reaon??
	p = mystrchr( p, ',' );
	if ( p )
	{
		tokenlookup[i].string = p+1;
		while( (unsigned char)*p >=32 )
		{
			// See if there are any comments in the line, if so, then mark the end of the string
			if ( *p == '/' && *(p+1) == '/' )
			{
				*p = 0;
				p++;
				p = EatUpNormalCharsLine( p );
			}
			else
				p++;
		}
		// We've hit a 'return' or 'nexline' char so mark the end of the string
		*p = 0;
		p++;
	}
	return p;
}

void InitFastLookup( char *table )
{
	char *p;
	long i = 0, tk = 0, lasttk = 0, groupLabelNum, token = 0, group = 0, tokenCount = 0, fastCount = 0;

	LangVersion = 0;
	if ( !table ){
		mymemset( tokenlookup, 0, sizeof( Tokenlookup ) * MAXTOKENS ); 
		return;
	}
		
	p = table;
	while( *p )
	{
		// Skip crappy characters and comments 
		p = EatUpCrappyChars( p );
		if ( !p ) {
			readLangFile = 1;
			return;
		}

		// Comment lines
		if ( CommentLine( p ) )
		{
			SetLangVersion( p );
			p = FindGroupStartToken( p );
			if ( !p ) {
				readLangFile = 1;
				return;
			}
			// If we have found an open brace, then we assume it is the start of a group
			if ( *p == '{' )
				ReadGroup( lasttk, &p, group );
			continue;
		}

		// Check for start of a string token
		if ( *p == '{' ) 
		{
			// Lets make sure we are looking at a string in the correct group
			long tk = ReadLong( (unsigned char *)p+1 );
			if ( (tk&0xffff0000) != lasttk )
			{
				OutDebugs( "Language File - Ignoring line staring with '{%c%c%c%c%c' as '%c%c' is not a valid group.", *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+1), *(p+2) );
				p = EatUpNormalCharsLine( p ); // Eat up the line
				continue;
			}

			// We have a token so load it... e.g. {ss20},Bandwidth In
			if ( myisdigit( p[3] ) )
				groupLabelNum = myatoi( p+3 );
			else
				groupLabelNum++;

			i = group + groupLabelNum;

#ifdef DEF_DEBUG
			//OutDebugs( "adding token '%08lx' at pos#%d, group#%d,location#%d", tk, i, group, groupLabelNum );
#endif
			// Do a sanity check, just in case a language file 
			if ( i > END_OF_STRINGS || i > MAXTOKENS )
			{
				OutDebug( "\n\n\nTrying to access string greater than END_OF_STRINGS or we have to update the size of MAXTOKENS - please fix...\n\n\n" );
				readLangFile = 1;
				return;
			}

			p = AddTheStringToArray( p, i, tk );
			tokenCount++;
			continue;
		}
		else
			p++;
	}
	readLangFile = 1;
}

/* Old code - redundant

char *FindToken( char *tokenStr )
{
	unsigned long tk, tk2;
	long idx;
	char c1;

	if ( (LangVersion == 3 || LangVersion == 4)  && tokenStr ){
		long group, groupend;
		tk = ReadLong( (unsigned char *)tokenStr+1 );

		switch( (tk>>16)&0xffff ) {
			case 'ss': group = SUMM_BEGIN; groupend = SUMM_END; break;
			case 'rc': group = RCAT_BEGIN; groupend = RCAT_END;  break;
			case 'rl': group = REPORT_BEGIN; groupend = REPORT_END;  break;
			case 'gl': group = LABEL_BEGIN; groupend = LABEL_END;  break;
			case 'ti': group = TIME_BEGIN; groupend = TIME_END;  break;
			case 'er': group = SERR_BEGIN; groupend = SERR_END;  break;
			case 'sy': group = SYSI_BEGIN; groupend = SYSI_END;  break;
			case 'co': group = COUN_BEGIN; groupend = COUN_END;  break;
			case 'or': group = ORGS_BEGIN; groupend = ORGS_END;  break;
			default: return 0;
		}
		c1 = myatoi( tokenStr+3 );

		idx = group + c1;

		tk2 = tokenlookup[idx].key;
		if ( (tk2&0xffff0000) == (tk&0xffff0000) ){
			if ( tk2 == tk ){
				return tokenlookup[idx].string;
			}
		}
	}
	return 0;
}

*/

long CompFastLookup( void *p1, void *p2, long *result)
{
	char	*s1, *s2;

	s1=*((char**)p1); 
	s2=*((char**)p2);

	if ( s1 && s2 ){
		long c1,c2;

		c1 = countchar( s1, 32 );
		c2 = countchar( s2, 32 );

		return c1-c2;
	}
	return 0;
}


void LanguageError( long level )
{
	SysBeep(1);
	//FileError( LangFiles[level] );
}

char *LoadLanguageFile( char *langname )
{
	char *p;
	long filelen;
	FILE *fp=0;
	char *langStr = NULL;

	if ( langname ){
		fp = OpenLang( langname );
		if ( fp ){
			filelen = GetFPLength( fp ) ;
			//printf( "f=%d leve=%d\n", filelen, level );
			langStr = (char*)malloc( filelen+256 );
			if ( p = langStr ){
				mymemset( p , 0 ,filelen+2 );
				fread( p, filelen, 1, fp );
				fclose( fp );
			}
		}
	}
	else // load lang from english data header file
	{
		if ( DefaultLangFile )
		{
			free( DefaultLangFile );
			DefaultLangFile = 0;
		}
		DefaultLangFile = (char*)malloc( strlen( (char*)english_data ) + 256 );
		memcpy( DefaultLangFile, english_data, strlen( (char*)english_data )+1 );
	}
	return langStr;
}

void InitDefaultEnglishStrings()
{
	long i;
	for	( i = SUMM_BEGIN; i < END_OF_STRINGS; i++ )
	{
		tokenlookup[i].defaultEnglish = tokenlookup[i].string;
	}
}



void InitHTMLTokenToAsciiCharMaps();
static char *prev_lang = 0;

long InitLanguage( char *langIn, short forceLoad )
{
	static char langFilename[256];
	char *lang; 

	InitHTMLTokenToAsciiCharMaps();

	// Make sure there is a language file specified
	if ( !langIn )
		lang = ENGLISH_LANG;
	else
		lang = langIn;

	// Really make sure... we have lang file
	if ( !*lang )
		lang = default_lang;

	// If we've been using a previous language and we change to use another language...
	// then reset the lookup table, as we will need to re-read the default English from the .rc file
	// language text and then also re-read the newly specified language strings from the .lang file
	//
	// NOTE: We read to the "English Resource" first as we cannot depend upon a .lang file being complete, ie.
	// having all the strings in the .lang which correspond to the ids used in the reports etc.
	//
	if ( prev_lang )
	{
		if ( strcmp( lang, prev_lang ) || forceLoad )
		{
			InitFastLookup( NULL );
			delete[] prev_lang;
			prev_lang = 0;
		}
		else
			return 0; // Using the same language file as last time, so do nothing...
	}

	if ( !prev_lang )
	{
		prev_lang = new char[mystrlen( lang )+1];
		mystrcpy( prev_lang, lang );
	}

	// If we are using the same language then we don't have to do anything,
	// as we have already initialised the language lookup table...
	if ( strcmp( lang, langFilename ) || forceLoad )
	{
		// Initialise the language lookup table, and load the internal English language file from the resources file
		InitFastLookup( 0 );
		LoadLanguageFile( 0 );

		// Make sure we have an got some text from the resource file, if not, then look for the default .lang file (English)
		if ( !DefaultLangFile )
		{
			InitFastLookup( 0 );
			DefaultLangFile = LoadLanguageFile( default_lang );
			if ( !DefaultLangFile )
				return 1;
		}

		// Initialise the language lookup table to the default English (either loaded by Resource or by English.lang file)
		InitFastLookup( DefaultLangFile );
		InitDefaultEnglishStrings();

		// Free the previously allocated memory for the .lang file
		mystrcpy( langFilename, lang );
		if ( LanguageFile )
		{
			free( LanguageFile );
			LanguageFile = 0;
		}

		// Re-load the newly selected .lang file, and set the lookup table string entries to point to these new strings
		if ( LanguageFile = LoadLanguageFile( lang ) ) {
			InitFastLookup( LanguageFile );
		} else
			return 2;

		// Produce a warning message for the user if they are using a version 2 .lang file, which will have no effect anymore...
		if ( LangVersion == 2 ) {
			char *msg = "The selected language file, '%s', is a version 2 file.\nYou need a version 4 language file.\n\nThis file will be ignored and the default language, English, will be used.";
			char *msgBuf = new char[ mystrlen( msg ) + mystrlen( lang ) ];
			sprintf( msgBuf, msg, lang ); 
			ErrorMsg( msgBuf );
			return 1;
			//MessageBox( NULL, , "Warning", MB_YESNO );
		}
	}
	//printf( "%08lx, %08lx, %s\n", DefaultLangFile, LanguageFile, lang );
	return 0;
}	

long GetLangVersion( void )
{
	return LangVersion;

}

#define	GET_TOKEN( number )		tokenlookup[number].string	
char *TranslateID( long id )
{
	if ( id >= 0 && id < MAXTOKENS )
		return GET_TOKEN( id );
	else
		return 0;
}

char *DefaultEnglishStr( long id )
{
	if ( id >= 0 && id < MAXTOKENS )
		return tokenlookup[id].defaultEnglish;
	else
		return 0;
}

typedef struct htmlTokenToAsciiCharMap
{
	int htmlIndexStrNameIndex;
	int htmlIndexHashNumIndex;	// this field appears to be unused, what is it for?		RHF	11/09/01
	char *htmlTokenName;
	int AsciiCharNum;
} HTMLTokenToPDFCharMap;

static htmlTokenToAsciiCharMap HTMLTokenToAsciiCharMapArray[] = 
{
'quot',	'#36',  "quot",		36,		// " quote
'Dot\0',-1,		"Dot",		56,		// Dot 
'iexc',	'#161', "iexcl",	161,	// inverted exclamation
'cent', '#162', "cent",		162,	// cent
'curr', '#164', "curren",	163,	// curren
'yen\0','#165', "yen",		164,	// yen 
'brvb', '#166', "brvbar",	165,	// brvbar, becomes standard bar '|'
'poun', '#163', "pound,",	166,	// pound, bcomes 'sterling'
'sect', '#167', "sect",		167,	// section
'uml\0','#168', "uml",		168,	// uml
'copy', '#169', "copy",		169,	// copyright
'ordf', '#170', "ordf",		170,	// ordfeminine
'laqu', '#171', "laquo",	171,	// Left angle quote, guillemot left 
'not\0','#172', "not",		172,	// Not sign  
'shy\0','#173', "shy",		173,	// Soft hyphen  - make it a normal hyphen
'reg\0','#174', "reg",		174,	// Registered trademark  
'macr', '#175', "macr",		175,	// Macron accent 
'deg\0','#176', "deg",		176,	// Degree sign  
'plus', '#177', "plusmn",	177,	// Plus or minus
'sup2', '#178', "sup2",		178,	// Superscript two 
'sup3', '#179', "sup3",		179,	// Superscript three
'acut', '#180', "acute",	180,	// acute
'micr', '#181', "micro",	181,	// Micro sign 
'para', '#182', "para",		182,	// Paragraph sign 
'midd', '#183', "middot",	183,	// Middle dot, using 'period centered'
'cedi', '#184', "cedil",	184,	// cedilla
'sup1', '#185', "sup1",		185,	// Superscript one
'ordm', '#186', "ordm",		186,	// Masculine ordinal 
'raqu', '#187', "raquo",	187,	// Right angle quote, guillemot right 
'fr14', '#188', "frac14",	188,	// frac14
'fr12', '#189', "frac12",	189,	// frac12
'fr34', '#190', "frac34",	190,	// frac34
'ique', '#191', "iquest",	191,	// Inverted question mark
'Agra', '#192', "Agrave",	192,	// Agrave
'Aacu', '#193', "Aacute",	193,	// Aacute
'Acir', '#194', "Acirc",	194,	// Acirc
'Atil', '#195', "Atilde",	195,	// Atilde
'Auml', '#196', "Auml",		196,	// Auml
'Arin', '#197', "Aring",	197,	// Aring
'AEli', '#198', "AElig",	198,	// AElig
'Cced', '#199', "Ccedil",	199,	// Ccedil
'Egra', '#200', "Egrave",	200,	// Egrave
'Eacu', '#201', "Eacute",	201,	// Eacute
'Ecir', '#202', "Ecirc",	202,	// Ecirc
'Euml', '#203', "Euml",		203,	// Euml
'Igra', '#204', "Igrave",	204,	// Igrave
'Iacu', '#205', "Iacute",	205,	// Iacute
'Icir', '#206', "Icirc",	206,	// Icirc
'Iuml', '#207', "Iuml",		207,	// Iuml
//'ETH\0','#208': return "Fi"; // ETH - special case ETH = 'Fi'
'Ntil', '#209', "Ntilde",	209,	// Ntilde
'Ogra', '#210', "Ograve",	210,	// Ograve
'Oacu', '#211', "Oacute",	211,	// Oacute
'Ocir', '#212', "Ocirc",	212,	// Ocirc
'Otil', '#213', "Otilde",	213,	// Otilde
'Ouml', '#214', "Ouml",		215,	// Ouml
'time', '#215', "times",	214,	// Multiply sign 
'Osla', '#216', "Oslash",	216,	// Oslash
'Ugra', '#217', "Ugrave",	217,	// Ugrave
'Uacu', '#218', "Uacute",	218,	// Uacute
'Ucir', '#219', "Ucirc",	219,	// Ucirc
'Uuml', '#220', "Uuml",		220,	// Uuml
'Yacu', '#221', "Yacute",	221,	// Yacute, becomes 'standard Y'
'THOR', '#222', "THORN,",	222,	// THORN, becomes 'standard P'
'szli', '#223', "szlig",	223,	// szlig
'agra', '#224', "agrave",	224,	// agrave
'aacu', '#225', "aacute",	225,	// aacute
'acir', '#226', "acirc",	226,	// acirc
'atil', '#227', "atilde",	227,	// atilde
'auml', '#228', "auml",		228,	// auml
'arin', '#229', "aring",	229,	// aring
'aeli', '#230', "aelig",	230,	// aelig
'cced', '#231', "ccedil",	231,	// ccedil
'egra', '#232', "egrave",	232,	// egrave
'eacu', '#233', "eacute",	233,	// eacute 
'ecir', '#234', "ecirc",	234,	// ecirc 
'euml', '#235', "euml",		235,	// euml
'igra', '#236', "igrave",	236,	// igrave
'iacu', '#237', "iacute",	237,	// iacute
'icir', '#238', "icirc",	238,	// icirc
'iuml', '#239', "iuml",		239,	// iuml
'eth\0','#240', "eth",		240,	// eth, Icelandic
'ntil', '#241', "ntilde",	241,	// ntilde 
'ogra', '#242', "ograve",	243,	// ograve
'oacu', '#243', "oacute",	242,	// oacute 
'ocir', '#244', "ocirc",	244,	// ocirc  
'otil', '#245', "otilde",	245,	// otilde  
'ouml', '#246', "ouml",		246,	// ouml

'osla', '#248', "oslash",	248,	// oslash
'ugra', '#249', "ugrave",	249,	// ugrave  
'uacu', '#250', "uacute",	250,	// uacute 
'ucir', '#251', "ucirc",	251,	// ucirc  
'uuml', '#252', "uuml",		252,	// uuml
'yacu', '#253', "yacute",	253,	// yacute 
'thor', '#254', "thorn",	254,	// thorn
'yuml', '#255', "yuml",		255,	// yuml 
-1,		'#848', "#8482",	153,	// trademark
0,		0,		"",			0		// Denotes end of table :o))))
};

//http://www.bbsinc.com/iso8859.html is a good source for HTML symbols

/*
&copy; © &#33; ! &#95; _ &#157; ù &#219; € 
&reg; Æ &#34; " &#96; ` &#158; û &#220; ‹ 
&nbsp;  &#35; ` &#97; a &#159; ü &#221; › 
&quot; " &#36; $ &#98; b &#160;   &#222; ﬁ 
&amp; & &#37; % &#99; c &#161; ° &#223; ﬂ 
&lt; < &#38; & &#100; d &#162; ¢ &#224; ‡ 
&gt; > &#39; ' &#101; e &#163; £ &#227; „ 
&Agrave; ¿ &#40; ( &#102; f &#164; § &#226; ‚ 
&Aacute; ¡ &#41; ) &#103; g &#165; • &#227; „ 
&Acirc; ¬ &#42; * &#104; h &#166; ¶ &#228; ‰ 
&Atilde; √ &#43; + &#105; i &#167; ß &#229; Â 
&Auml; ƒ &#44; , &#106; j &#168; ® &#230; Ê 
&Aring; ≈ &#45; - &#107; k &#169; © &#231; Á 
&AElig; ∆ &#46; . &#108; l &#170; ™ &#232; Ë 
&Ccedil; « &#47; / &#109; m &#171; ´ &#233; È 
&Egrave; » &#48; 0 &#110; n &#172; ¨ &#234; Í 
&Eacute; … &#49; 1 &#111; o &#173; ≠ &#235; Î 
&Ecirc;   &#50; 2 &#112; p &#174; Æ &#236; Ï 
&Euml; À &#51; 3 &#113; q &#175; Ø &#237; Ì 
&Igrave; Ã &#52; 4 &#114; r &#176; ∞ &#238; Ó 
&Iacute; Õ &#53; 5 &#115; s &#177; ± &#239; Ô 
&Icirc; Œ &#54; 6 &#116; t &#178; ≤ &#240;  
&Iuml; œ &#55; 7 &#117; u &#179; ≥ &#241; Ò 
&ETH; – &#56; 8 &#118; v &#180; ¥ &#242; Ú 
&Ntilde; — &#57; 9 &#119; w &#181; µ &#243; Û 
&Otilde; ’ &#58; : &#120; x &#182; ∂ &#244; Ù 
&Ouml; ÷ &#59; ; &#121; y &#183; ∑ &#245; ı 
&Oslash; ÿ &#60; < &#122; z &#184; ∏ &#246; ˆ 
&Ugrave; Ÿ &#61; = &#123; { &#185; π &#247; ˜ 
&Uacute; ⁄ &#62; > &#124; | &#186; ∫ &#248; ¯ 
&Ucirc; € &#63; ? &#125; } &#187; ª &#249; ˘ 
&Uuml; ‹ &#64; @ &#126; ~ &#188; º &#250; ˙ 
&Yacute; › &#65; A &#127; ? &#189; Ω &#251; ˚ 
&THORN; ﬁ &#66; B &#128; Ä &#190; æ &#252 ¸ 
&szlig; ﬂ &#67; C &#129; Å &#191; ø &#253; ˝ 
&agrave; ‡ &#68; D &#130; Ç &#192; ¿ &#254; ˛ 
&aacute; · &#69; E &#131; É &#193; ¡ &#255; ˇ 
&aring; Â &#70; F &#132; Ñ &#194; ¬   
&aelig; Ê &#71; G &#133; Ö &#195; √   
&ccedil; Á &#72; H &#134; Ü &#196; ƒ   
&egrave; Ë &#73; I &#135; á &#197; ≈   
&eacute; È &#74; J &#136; à &#198; ∆   
&ecirc; Í &#75; K &#137; â &#199; «   
&euml; Î &#76; L &#138; ä &#200; »   
&igrave; Ï &#77; M &#139; ã &#201; …   
&iacute; Ì &#78; N &#140; å &#202; ?   
&icirc; Ó &#79; O &#141; ç &#203; À   
&iuml; Ô &#80; P &#142; é &#204; Ã   
&eth;  &#81; Q &#143; è &#205; Õ   
&ntilde; Ò &#82; R &#144; ê &#206; Œ   
&ograve; Ú &#83; S &#145; ë &#207; œ   
&oacute; Û &#84; T &#146; í &#208; –   
&ocirc; Ù &#85; U &#147; ì &#209; —   
&otilde; ı &#86; V &#148; î &#210; “   
&ouml; ˆ &#87; W &#149; ï &#211; ”   
&oslash; ¯ &#88; X &#150; ñ &#212; ‘   
&ugrave; ˘ &#89; Y &#151; ó &#213; ’   
&uacute; ˙ &#90; Z &#152; ò &#214; ÷   
&ucirc; ˚ &#91; [ &#153; ô &#215; ◊   
&yacute; ˝ &#92; \ &#154; ö &#216; ÿ   
&thorn; ˛ &#93; ] &#155; õ &#217; Ÿ   
&yuml; ˇ &#94; ^ &#156; ú &#218; ⁄ 
*/



typedef std::map<int,int> ArrayIndex;
static ArrayIndex htmlShortStrNameIntIndex;
static ArrayIndex htmlHashNumIntIndex;

void InitHTMLTokenToAsciiCharMaps()
{
	int i = 0;
	if ( htmlShortStrNameIntIndex.size() != 0 )
		return;

	while ( HTMLTokenToAsciiCharMapArray[i].htmlIndexStrNameIndex )
	{
		ArrayIndex::value_type indexName(HTMLTokenToAsciiCharMapArray[i].htmlIndexStrNameIndex, i);
		htmlShortStrNameIntIndex.insert( htmlShortStrNameIntIndex.end(), indexName );
		ArrayIndex::value_type indexHash(HTMLTokenToAsciiCharMapArray[i].htmlIndexStrNameIndex, i);
		htmlHashNumIntIndex.insert( htmlHashNumIntIndex.end(), indexHash );
		i++;
	}
}

#define HTMLTOKEN_NOTFOUND 0
#define HTMLTOKEN_INSERTSTRING 1

unsigned char ConvertHTMLStrToAsciiCharCode( char *str )
{
	int code = 0;
	std::string charCodeStr = "";
	charCodeStr.reserve(16);
	int count = 0;
	while( *str != 0 && count < 10 )
	{
		if (*str == ';')
		{
			// Found a possible HTML special character
			char charCodeIntStr[6];
			mystrncpy(charCodeIntStr, charCodeStr.c_str(), 6);
			int charCodeInt = 0;
			for(int i=0; i<3; i++)
			{
				charCodeInt = ((charCodeInt + (int)charCodeIntStr[i]) * 256);
			}
			charCodeInt += (int)charCodeIntStr[3];

			if ( charCodeInt == 'frac' )
			{
				charCodeInt = 0;
				for(int i=0; i<2; i++)
				{
					charCodeInt = ((charCodeInt + (int)charCodeIntStr[i]) * 256);
				}
				charCodeInt = ((charCodeInt + (int)charCodeIntStr[4]) * 256);
				charCodeInt += (int)charCodeIntStr[5];
			}

			ArrayIndex::iterator posIter;;
			int pos;

			if ( charCodeIntStr[0] != '#' )
			{
				posIter = htmlShortStrNameIntIndex.find( charCodeInt );
				if ( posIter == htmlShortStrNameIntIndex.end() )
					return 0;
				pos = (*posIter).second;
				if ( pos > htmlShortStrNameIntIndex.size() || pos < 0 )
					return 0;
				code = HTMLTokenToAsciiCharMapArray[pos].AsciiCharNum;
			}
			else
			{
				posIter = htmlHashNumIntIndex.find( charCodeInt );
				if ( posIter == htmlHashNumIntIndex.end() )
					return 0;
				pos = (*posIter).second;
				if ( pos >= htmlHashNumIntIndex.size() )
					return 0;
				// Maybe check the text here... too...
				code = HTMLTokenToAsciiCharMapArray[pos].AsciiCharNum;
			}
			return (unsigned char)code;
		}
		charCodeStr += *str;
		str++;
		count++;
	}
	return HTMLTOKEN_NOTFOUND;
}

std::string ConvertHTMLStrToAsciiString( char *str )
{
	int code = 0;
	std::string charCodeStr = "";
	charCodeStr.reserve(16);
	int count = 0;
	while( *str != 0 && count < 10 )
	{
		if (*str == ';')
		{
			// Found a possible HTML special character
			char charCodeIntStr[4];
			mystrncpy(charCodeIntStr, charCodeStr.c_str(), 4);
			int charCodeInt = 0;
			for(int i=0; i<3; i++)
			{
				charCodeInt = ((charCodeInt + (int)charCodeIntStr[i]) * 256);
			}
			charCodeInt += (int)charCodeIntStr[3];
			switch( charCodeInt )
			{
			case '#848'/*#8482*/: if (charCodeStr == "#8482") return "TM"; // trademark (a clumsy attempt to produce one)
			default: return "";
			}
		}
		charCodeStr += *str;
		str++;
		count++;
	}
	return "";
}



void ConvertHTMLCharacterTokens( std::string &str )
{

	if ( str.find( "&" ) == std::string::npos )
		return;

	unsigned char c;
	std::string replaceHTMLTokensStr;
	replaceHTMLTokensStr = "";

	replaceHTMLTokensStr.reserve(str.length());

	for( int i=0; i < str.length(); i++ )
	{
		c = str[i];
		if (c == '&')
		{
			// Check for the special HTML characters which are shown as tokens,
			// We need to change these back into their ascii character code 
			unsigned char charCode = ConvertHTMLStrToAsciiCharCode( &str[i+1] );
			if ( charCode != HTMLTOKEN_NOTFOUND ) // That is, if a token was
			{
				if ( charCode == HTMLTOKEN_INSERTSTRING )
				{
					std::string asciiStr = ConvertHTMLStrToAsciiString( &str[i+1] );
					replaceHTMLTokensStr += asciiStr;
				}
				else
					replaceHTMLTokensStr += charCode;
				while (str[i] != ';')
					i++;
				continue;
			}
		}
		replaceHTMLTokensStr += c; // Add the current character
	}
	str = replaceHTMLTokensStr;
	return;
}

void SwapHTMLCharacterTokens( char* str, char* replaceHTMLTokensStr )
{
	unsigned char c;
	char* src, * dest;
	int len;

	if ( !(src=mystrchr( str, '&' )) )
		return;

	if ( replaceHTMLTokensStr )
		dest = replaceHTMLTokensStr;
	else
		dest = src;

	len = mystrlen(str);

	while( *src )
	{
		c = *src++;
		if (c == '&')
		{
			// Check for the special HTML characters which are shown as tokens,
			// We need to change these back into their ascii character code 
			unsigned char charCode = ConvertHTMLStrToAsciiCharCode( src );
			if ( charCode != HTMLTOKEN_NOTFOUND ) // That is, if a token was
			{
				if ( charCode != HTMLTOKEN_INSERTSTRING )
					*dest++ = charCode;

				while ( *src++ != ';' )
					;
				continue;
			}
		}
		*dest++ = c; // Add the current character
	}
	*dest++ = 0;
	return;
}

const char* LoadLangStringConvertFromHTML( long stringId )
{
	static std::string transText;
	char *s = TranslateID( stringId );
	if (!s)
		return "";
	transText = s;
	ConvertHTMLCharacterTokens( transText );
	return transText.c_str();
}

void LoadWeekdaysAndMonths( char weekdaysT[WEEKDAYS+1][BUF16], char monthsT[MONTHS][BUF16], char MonthsT[MONTHS][BUF16] )
{
	int i;

	// Initialise the weekday names
	for( i = 0; i < WEEKDAYS; i++ )
	{
		// Copy each of the seven translated weekdays
		mystrncpy( weekdaysT[i], LoadLangStringConvertFromHTML( TIME_SUNDAY+i ), BUF16 );
		weekdaysT[i][BUF16-1] = 0;
	}
	// Make an eightd position be empty... may be needed by existing code...
	weekdaysT[WEEKDAYS][0] = 0;

	// Initialise the month abbreviations
	for( i = 0; i < MONTHS; i++ )
	{
		mymemset( monthsT[i], 0, BUF16 );
		// Copy each of the seven translated weekdays
		mystrncpy( monthsT[i], LoadLangStringConvertFromHTML( TIME_JAN+i ), BUF16 );
		monthsT[i][BUF16-1] = 0;
		mystrncpy( MonthsT[i], monthsT[i], BUF16 );
	}
}

typedef struct _LangSectionCategoryName
{ 
	long startTokenId;
	long endTokenId;
	long categoryName;
	char *categoryAbrev;
} LangSectionCategoryName;

LangSectionCategoryName langSectionCategoryName[] =
{
	{ SUMM_BEGIN, SUMM_END, IDS_SERVER_STATS, "ss" },
	{ RCAT_BEGIN, RCAT_END, IDS_REPORT_CAT, "rc" },
	{ REPORT_BEGIN, REPORT_END, IDS_REPORT_TITLES, "rl" },
	{ LABEL_BEGIN, LABEL_END, IDS_GENERAL_LABELS, "gl" },
	{ TIME_BEGIN, TIME_END, IDS_TIME_RELATED, "ti" },
	{ SESS_BEGIN, SESS_END, IDS_SESSION_LIST, "se" },
	{ COMM_BEGIN, COMM_END, IDS_COMMENTS, "cm" },
	{ SERR_BEGIN, SERR_END, IDS_SERVER_ERRORS, "er" },
	{ SYSI_BEGIN, SYSI_END, IDS_SYSTEM_INFO, "sy" },
	{ COUN_BEGIN, COUN_END, IDS_COUNTRIES, "co" },
	{ ORGS_BEGIN, ORGS_END, IDS_ORGANIZATIONS, "or" },
	//{ xx, xx, xx, "xx" },
	{ 0, 0, 0, 0 }
};

short GetLangSectionBegin( short index )
{
	int i = 0;
	while( langSectionCategoryName[i].startTokenId )
	{
		if ( index >= langSectionCategoryName[i].startTokenId && index < langSectionCategoryName[i].endTokenId )
			return langSectionCategoryName[i].startTokenId;
		i++;
	}
	return 0;
}

char *GetLangSectionName( short index )
{
	int i = 0;
	while( langSectionCategoryName[i].startTokenId )
	{
		if ( index > langSectionCategoryName[i].startTokenId && index < langSectionCategoryName[i].endTokenId )
			return ReturnString( langSectionCategoryName[i].categoryName );
		i++;
	}
	return 0;
}

char *GetLangSectionAbrev( short index )
{
	int i = 0;
	while( langSectionCategoryName[i].startTokenId )
	{
		if ( index > langSectionCategoryName[i].startTokenId && index < langSectionCategoryName[i].endTokenId )
			return langSectionCategoryName[i].categoryAbrev;
		i++;
	}
	return 0;
}

short RealLangToken( short index )
{
	// Range check
	if ( index < SUMM_BEGIN || index > END_OF_STRINGS )
		return 0;

	int i = 0;
	while( langSectionCategoryName[i].startTokenId )
	{
		if ( index == langSectionCategoryName[i].startTokenId || index == langSectionCategoryName[i].endTokenId )
			return 0;
		i++;
	}
	if ( index == END_OF_STRINGS )
		return 0;
	return 1;
}

typedef struct _LangEditedStrings
{
	int index;
	char *string;
} LangEditedStrings;
LangEditedStrings langEditedStrings[END_OF_STRINGS];

void AddNewLangString( char *str, short index )
{
	if ( index < SUMM_BEGIN || index >= END_OF_STRINGS )
		return;

	langEditedStrings[index].index = index;
	if ( str )
	{
		langEditedStrings[index].string = new char[mystrlen(str)+1];
		mystrcpy( langEditedStrings[index].string, str );
	}
	else
		langEditedStrings[index].string = str;
}

#define LangFormatLine "{%s%02d},%s\n"
#define LangSectionLines "\n// %s = {%sNN}\n\n"

void WriteNewLangString( FILE *fp, short index )
{
	static char buf[1024];
	if ( index < SUMM_BEGIN || index >= END_OF_STRINGS )
		return;

	if ( langEditedStrings[index].string )
		sprintf( buf, LangFormatLine, GetLangSectionAbrev(index), index-GetLangSectionBegin(index), langEditedStrings[index].string );
	else if ( GetLangSectionBegin(index) )
		sprintf( buf, LangSectionLines, GetLangSectionName(index+1), GetLangSectionAbrev(index+1) );
	else
		return;
	fwrite( buf, mystrlen( buf ), 1, fp ); 
}

void ClearNewLangString( short index )
{
	if ( index < SUMM_BEGIN || index >= END_OF_STRINGS )
		return;

	if ( langEditedStrings[index].string )
		delete[] langEditedStrings[index].string;
}

