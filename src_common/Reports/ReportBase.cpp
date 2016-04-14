/*
##
##      ##
##      ##
##      ##  ##
##      #########
##          ##
##          ##
##          ##
##
##		V4 Analyzer -	Write module, this outputs all html/rtf/, and spits out calls to make
##						Doc/Excel/PDF to other modules, but basically handles all the reports.
##
##
##
####################################################*/



// ************************************************************************
// Includes
// ************************************************************************    
#include "FWA.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <iostream>
#include "datetime.h"
#include "version.h"
#include "ReportHTML.h"
#include "FilterString.h"		// for QS::FormatFilterString().
#include "report_keypages.h"	// for DoRouteToKeyPages && FilterNonKeyPages
#include "ReportFuncs.h"

#include "Hash.h"
#include "EngineBuff.h"
#include "EngineMeanPath.h"
#include "EngineVirtualDomain.h"
#include "EngineParse.h"
#include "EngineStatus.h"
#include "ResDefs.h"

#ifdef DEF_MAC
	#include <OpenTransport.h>
	#include <OpenTptInternet.h>
	#include "main.h"
	#include "config.h"
	#include "MacStatus.h"
	#include "MacUtil.h"
	#include "progress.h"
	#include "post_comp.h"
	#include "Processing.h"
	#include "stdfile.h"
	#define	RemoveFile(x)	remove(x)
#endif

#include "myansi.h"
#include "zlib.h"
#include "gd.h"
#include "config_struct.h"
#include "Stats.h"
#include "VirtDomInfo.h"
#include "StandardPlot.h"
#include "engine_drill.h"
#include "engine_proc.h"
#include "editpath.h"
#include "translate.h"
#include "net_io.h"
#include "Registration.h"
#include "FileTypes.h"
#include "DateFixFileName.h"


#if DEF_WINDOWS 
	#include <sys/stat.h>
	#include "Winmain.h"
	#include "Winutil.h"
	#include "resource.h"
	#include "postproc.h"
	#include "httpinterface.h"
#endif				

#if DEF_UNIX		// UNIX include
	#include <sys/stat.h>
	#include <errno.h>
	#include "unistd.h"
	#include "main.h"
	#include "postproc.h"
	#include "httpinterface.h"
#endif

// ------ Include images for multireports
#include "monthly.h"
#include "quarter.h"
#include "smedia.h"
#include "vhost.h"
#include "weekly.h"
// --------------------------------------


//#include "country_data.c"

#include "Report.h"
#include "ReportClass.h"
#include "OSInfo.h"
#include "StatDefs.h"	// for SESS_BYTES, SESS_TIME, etc.


#include <string>
#define NO_INLINE
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>

#include "Utilities.h"	// for match(...)
#include "SettingsAppWide.h"

#include "HelpCard.h"	// for WriteHelpCard(...);


#if DEF_WINDOWS
#include "GraphDrawStr.h"
NonISO88591String *nonISO88591String = 0;
#endif // DEF_WINDOWS


// ************************************************************************
// Extern functions
// ************************************************************************
extern int IsDebugMode( void );
//extern "C" short	IsDemoReg( void );		// should be defined in serialReg.h


// ************************************************************************
// MACROs
// ************************************************************************
#undef ShowProgress
#define ShowProgress( percent, forceShow, msg )		ShowProgressDetail( percent*10,  forceShow, msg, 0 )


// ************************************************************************
// Defines
// ************************************************************************
#define	FILTER_STRING_LENGTH			2046


// ************************************************************************
// Globals & Externs.
// ************************************************************************
std::string			pdfFilename;
extern std::string	pdf_graph;
extern int			pdf_currHeight;

extern struct App_config MyPrefStruct;
extern long			allTotalRequests;
extern long			allTotalFailedRequests;
extern long			allTotalCachedHits;
extern __int64		allTotalBytes;
extern long			allfirstTime, allendTime;
extern long			badones;
extern long			totalDomains;
extern int	 		endall,stopall;
extern short		logStyle;
extern long			VDnum;
extern VDinfoP		VD[MAX_DOMAINSPLUS];			/* 16/32 virtual domain pointers */
extern "C" long		serialID;
extern VDinfoP		VDptrs[MAX_DOMAINS+1];
extern FILE			*out;

// *********************************************************************************
// baseFile:: methods.
// *********************************************************************************
void baseFile::Stat_WritePercent(FILE* fp, double dNumerator, double dDenominator)
{
	char	szNumber[16];	// 3:number, 1:'.', 2:precision, 1:'%', 1:null
							// now double the size for safety.

	if( dDenominator == 0)
		(void)sprintf(szNumber, "%3.2f%%", 0.0 );
	else if( (100.0 * dNumerator / dDenominator) < 1.0)
		(void)strcpy(szNumber, "<1%");
	else if( dNumerator == dDenominator)
		(void)strcpy(szNumber, "100%");
	else
		(void)sprintf(szNumber, "%3.2f%%", 100.0 * dNumerator / dDenominator );

	Stat_WriteRight(fp,	1, -1, szNumber);
}

// write out a line of text to file and translate its language at the same time
int baseFile::WriteLine( FILE *fp, const char *txt )
{
	if( fp ) {
		WriteToStorage( txt, fp );
		return 1;
	}
	return 0;
}

baseFile::baseFile()
{
	InitPageOptions();
	sessionWriter = 0;
}

baseFile::~baseFile()
{
	if( sessionWriter )
		delete sessionWriter;
}


int baseFile::WriteOUT( FILE *fp, char *fmt, ... )
{
	va_list		args;

	va_start( args, fmt );
	vsprintf( tempBuff, fmt, args );
	va_end( args );

	return WriteLine( fp, tempBuff );
}


int baseFile::Fprintf( FILE *fp, const char *fmt, ... )
{
	va_list		args;
static	char lineout[10240];

	va_start( args, fmt );
	vsprintf( lineout, fmt, args );
	va_end( args );

	SwapHTMLCharacterTokens( lineout, NULL );

	return WriteLine( fp, lineout );
}

int baseFile::WriteDirectlyToFile( FILE *fp, char *fmt, ... )
{
	va_list		args;

	va_start( args, fmt );
	vsprintf( tempBuff, fmt, args );
	va_end( args );

	return WriteToStorage( tempBuff, fp );
}

char *baseFile::FindHeadFormat( long type )
{
	long i = 0;
	char *p;

	while( p=HeaderFormat[i] ){
		if( ReadLong( (unsigned char *)p ) == type )
			return p;
		i++;
	}
	return HeaderFormat[0];
}


void baseFile::InitExtensions( void )
{
	dont_do_subtotals = 0;
	dont_do_average = 0;
	dont_do_totals = 0;
	dont_do_others = 0;
	one_file_output = 0;
}


void baseFile::Stat_WriteBoldDual( FILE *fp, const char *txt, const char *txt2 )
{
	char tstr[512];
	sprintf( tstr, "%s %s", txt, txt2 );
	Stat_WriteBold( fp, tstr );
}

void baseFile::Stat_WriteCenterSmall( FILE *fp, const char *txt )
{
	Stat_WriteLine( fp, txt );
}

void baseFile::Stat_WriteBottomHeader( FILE *fp, const char *txt, long space )
{
	Stat_WriteHeader( fp, txt, space );
}

void baseFile::Stat_WriteCostData( FILE *fp, __int64 bytes )
{
	if( MyPrefStruct.dollarpermeg ) {
		char dollarStr[64];
		sprintf( dollarStr, "%s%.2f", TranslateID(LABEL_CURRENCYUNIT), (((bytes>>10) * MyPrefStruct.dollarpermeg)>>10)/1000.0 ) ;
		Stat_WriteRight( fp, 1, -1, dollarStr );
	}
}

void baseFile::Stat_WriteTimeDuration( FILE *fp, long totalTime , long totalVisits )
{
	char durat[16];

	if( totalVisits )
		CTimetoString( totalTime/totalVisits, durat );
	else
		CTimetoString( 0, durat );
	Stat_WriteNumRight( fp, 1, -1, durat );
}


void baseFile::Stat_WriteReqData( FILE *fp, long files, double perc )
{
	char	numStr[64];
	char	num2Str[64];

	if( m_style == FORMAT_COMMA )
		sprintf( numStr, "%ld", files );
	else
		FormatLongNum( files, numStr );

	if( perc >= 100.0 )
		sprintf( num2Str, "100%%" );
	else
	if( perc == 0.0 )
		sprintf( num2Str, "0%%" );
	else
	if( perc >= 10.0 )
		sprintf( num2Str, "%3.1f%%", perc );
	else
	if( perc >= 1.0 )
		sprintf( num2Str, "%3.2f%%", perc );
	else {
		if( m_style == FORMAT_HTML )
			sprintf( num2Str, "&lt;1%%" );
		else
			sprintf( num2Str, "<1%%" );		// 
	}
	Stat_WriteDualText( fp, numStr, num2Str );
}


void baseFile::Stat_WriteHitData( FILE *fp, long files, __int64 totalFiles )
{
	double p1;
	if ( totalFiles )
		p1 = files / (double)(totalFiles/100.0);
	else
		p1 = 0;

	Stat_WriteReqData( fp, files, p1 );
}



void baseFile::Stat_WritePercent( FILE *fp, double perc )
{
	char	num2Str[64];

	if( perc >= 100.0 )
		sprintf( num2Str, "100%%" );
	else
	if( perc == 0.0 )
		sprintf( num2Str, "0%%" );
	else
	if( perc >= 10.0 )
		sprintf( num2Str, "%3.1f%%", perc );
	else
	if( perc >= 1.0 )
		sprintf( num2Str, "%3.2f%%", perc );
	else {
		if( m_style == FORMAT_HTML )
			sprintf( num2Str, "&lt;1%%" );
		else
			sprintf( num2Str, "<1%%" );		// 
	}
	
	Stat_WriteNumRight( fp, 1, -1, num2Str );
}


void baseFile::Stat_WriteBytes( FILE *fp, __int64 bytes )
{
	char	numStr[64];

	if( m_style == FORMAT_COMMA )
		sprintf( numStr, "%lld", bytes );
	else
		ValuetoString( TYPE_BYTES, bytes, bytes, numStr );
	Stat_WriteNumRight( fp, 1, -1, numStr );
}

void baseFile::Stat_WriteByteData( FILE *fp, __int64 bytes, double perc )
{
	char	numStr[64];
	char	num2Str[64];

	if( m_style == FORMAT_COMMA )
		sprintf( numStr, "%lld", bytes );
	else
		ValuetoString( TYPE_BYTES, bytes, bytes, numStr );

	
	if( perc >= 100.0 )
		sprintf( num2Str, "100%%" );
	else
	if( perc == 0.0 )
		sprintf( num2Str, "0%%" );
	else
	if( perc >= 10.0 )
		sprintf( num2Str, "%3.1f%%", perc );
	else
	if( perc >= 1.0 )
		sprintf( num2Str, "%3.2f%%", perc );
	else {
		if( m_style == FORMAT_HTML )
			sprintf( num2Str, "&lt;1%%" );
		else
			sprintf( num2Str, "<1%%" );		// 
	}
	
	Stat_WriteDualText( fp, numStr, num2Str );
}

void baseFile::Stat_WriteDualText( FILE *fp, char *name, char *name2 )
{
	Stat_WriteNumRight( fp, 1, -1, name );
	Stat_WriteNumRight( fp, 1, -1, name2 );
}

void baseFile::Stat_WriteMainData( FILE *fp, long files, double p1, __int64 bytes,  double p2 )
{
	Stat_WriteReqData( fp, files, p1 );
	Stat_WriteByteData( fp, bytes, p2 );
}


// write Hits,Hits %, and Bytes,Bytes%
void baseFile::Stat_WriteHitsAndBytes( FILE *fp, long files, __int64 totalFiles, __int64 bytes,  __int64 totalBytes )
{
	double p1=0, p2=0;

	if ( totalFiles )
		p1 = files / (double)(totalFiles/100.0);

	if ( totalBytes )
		p2 = bytes / (double)(totalBytes/100.0);

	Stat_WriteMainData( fp, files, p1, bytes, p2 );
}




// ----------------------------------------------------------------



void baseFile::Stat_WriteReqHeader( FILE *fp )
{
	if( MyPrefStruct.stat_style == STREAM )
		Stat_WriteDualHeader( fp, TranslateID(LABEL_PLAYS), TranslateID(LABEL_PERCENT) );
	else
		Stat_WriteDualHeader( fp, TranslateID(LABEL_REQUESTS), TranslateID(LABEL_PERCENT) );
}

void baseFile::Stat_WriteByteHeader( FILE *fp )
{
	Stat_WriteDualHeader( fp, TranslateID(LABEL_BYTES), TranslateID(LABEL_PERCENT) );
}

void baseFile::Stat_WriteMainHeader( FILE *fp )
{
	Stat_WriteReqHeader( fp );
	Stat_WriteByteHeader( fp );
}

void baseFile::Stat_WriteDualHeader( FILE *fp, const char *head1, const char *head2 )
{
	Stat_WriteHeader( fp, head1, 1 );
	Stat_WriteHeader( fp, head2, 1 );
}

void baseFile::WriteMeanPathList( FILE *fp, VDinfoP VDptr, long clienthash, long sindex, long count, long  hits, long rowNum )
{
	long lp, client,pagehash;
	char *name, thishost[MAXURLSIZE];
	bool firstEnter=true;
	static char buf[64];

	Statistic *p, *page;

	GetCorrectSiteURL( VDptr, thishost );

	client = VDptr->byClient->FindHash( clienthash );
	p = VDptr->byClient->GetStat( client );

	if( m_style != FORMAT_EXCEL )
		Stat_WriteNumberData( fp, rowNum );

	if( !MyPrefStruct.headingOnLeft )	
		Stat_WriteHitData( fp, hits, VDptr->byClient->GetStatListTotalVisits() );

	std::string displayName;

	if( p->sessionStat )
	{
		PDFMultiLineCellTextStrList cellStrList;
		long number = 1; //avoid bug in number order

		if( client != -1 )
		{
			time_t pageTime;
			lp = 0;
			WriteCellStartLeft( fp );

			if( IsDebugMode() ){
				sprintf( tempBuff2, "Client = %s #%ld (%08lx)", p->GetName(), client, clienthash );
				Stat_WriteComment( fp, tempBuff2 );
			}

			while( lp < count )
			{
				char urlname[MAXURLSIZE];
				pagehash = p->sessionStat->GetSessionPage( sindex+lp );
				pageTime = p->sessionStat->GetSessionTime( sindex+lp );

				page = VDptr->byPages->FindHashStat( pagehash );
				if( !page )
					page = VDptr->byDownload->FindHashStat( pagehash );

				if( m_style == FORMAT_PDF ) {
					sprintf( buf, "%d", lp+1 );
					displayName = buf;
					displayName += ".  ";
				}

				if( (long)page != -1 && page ) // make sure its a valid page, if its not then err, ignore it.
				{		
					name = page->GetName();
					MakeURL( urlname, thishost, name );
					if( m_style == FORMAT_EXCEL)
					{
						if( firstEnter ){
							firstEnter = false;
						} else
							Stat_WriteRowEnd( fp );  //rowth++;  write multi lines in the same colum
						writeMultiRow( number, name, fp );
						rownum++;
					} else
					if( m_style == FORMAT_PDF )
						displayName += name;
					else {
						Stat_WriteAnotherLinkInMultiLineCell( fp, urlname, name, number );
						DateTimeToString( pageTime, buf );
						sprintf( tempBuff2, "Date of Page : %s", buf );
						Stat_WriteComment( fp, tempBuff2 );
					}

					number++;
					if( m_style == FORMAT_PDF )
					{
						PDFMultiLineCellTextStr temp( displayName, GetStringLen( displayName ) );//, thePDFTableSettings.dataSize, thePDFTableSettings.dataStyle ) );
						cellStrList.push_back( temp );
					}
				}
				else 
				{
					if( m_style == FORMAT_PDF )
					{
						displayName = "Error: ";
						displayName += p->GetName();
						PDFMultiLineCellTextStr temp( displayName, GetStringLen( displayName ) );//, thePDFTableSettings.dataSize, thePDFTableSettings.dataStyle ) );
						cellStrList.push_back( temp );
					}
					sprintf( tempBuff2, "Invalid page hash (%08lx) at pos %ld , pages=%ld, clientname=%s", pagehash, sindex+lp, count, p->GetName() );
					Stat_WriteComment( fp, tempBuff2 );
				}
				lp++;
			}

			if( m_style == FORMAT_PDF ) {
 				AddTableCellMultilineText( cellStrList, PDF_LEFTJUSTIFY ); }
			WriteCellEnd( fp );
		}
		else
		{ 
			if( m_style == FORMAT_PDF )
			{
				displayName = "1. Error: ";
				displayName += p->GetName();
				PDFMultiLineCellTextStr temp( displayName, GetStringLen( displayName ) );//, thePDFTableSettings.dataSize, thePDFTableSettings.dataStyle ) );
				cellStrList.push_back( temp );
			}
			sprintf( tempBuff2, "invalid client = %ld (%08lx)", client, clienthash );
			Stat_WriteComment( fp, tempBuff2 );
		}
		writeNextData();
	}

	if( MyPrefStruct.headingOnLeft )	
		Stat_WriteHitData( fp, hits, VDptr->byClient->GetStatListTotalVisits() );
}


//#define	SHADOW_ON

/* Write - write list to file */

void baseFile::Stat_WriteMeanPath( VDinfoP VDptr, FILE *fp, int depth )
{
	int			list, cols;
	long		n, num;
	__int64		sumBytes = 0, totalBytes = 0;
	long		sumRequests = 0, totalRequests = 0;		// total hits
	//char		*name, /* *heading*/;
	long		tabwidth=GraphWidth();

	//heading = "Mean Path";
	
	//limit depth if necessary
	list = num = MP_GetMeanPathTotal( VDptr );
	//OutDebugs( "MPnum = %d", num );
	if( num <= 0 )
		return;

	if( depth>1 )
		if( list > depth) list = depth;

	sprintf( tempBuff2, "Total Units = %ld", num );
	Stat_WriteComment( fp, tempBuff2 );
		
	if( MyPrefStruct.dollarpermeg )
		cols = 5; else	cols = 4;

	if( m_style == FORMAT_EXCEL )
		cols--;

	//write heading out
	if( MyPrefStruct.shadow )
		WritePageShadowInit( fp );

	Stat_WriteTableStart( fp, tabwidth );
	Stat_WriteRowStart( fp, RGBtableTitle, 1 );
	Stat_WriteTitle( fp, cols, list, TranslateID(LABEL_MEANPATH) );

	Stat_WriteRowEnd( fp );
	Stat_WriteRowStart( fp, RGBtableHeaders, cols );
	if( m_style != FORMAT_EXCEL )
		Stat_WriteHeader( fp, " ", 1 );
	if( MyPrefStruct.headingOnLeft )
	{
		Stat_WriteHeader( fp, TranslateID(LABEL_MEANPATH), 1 );
		Stat_WriteDualHeader( fp, TranslateID(LABEL_SESSIONS), TranslateID(LABEL_PERCENT) );
	}
	else
	{
		Stat_WriteDualHeader( fp, TranslateID(LABEL_SESSIONS), TranslateID(LABEL_PERCENT) );
		Stat_WriteHeader( fp, TranslateID(LABEL_MEANPATH), 1 );
	}
	Stat_WriteRowEnd( fp );

	initAreaRow();
	rownum=0;
	//now output the stats
	for (n=0; n<list; n++) {	
		long	hashid, index, r;
		short	count;

		if( MP_GetMeanPath( VDptr, n, &hashid, &r, &index, &count ) ){
			Stat_WriteRowStart( fp, RGBtableItems, cols );
			WriteMeanPathList( fp, VDptr, hashid, index, count, r, n+1 );
			Stat_WriteRowEnd( fp );
			sumRequests += r;
		}
	
	}

	totalRequests = sumRequests;

	// ********************************************************
	// Do we want the Headers BEFORE the Average & Total?
	// ********************************************************
	if( MyPrefStruct.footer_label && !MyPrefStruct.footer_label_trailing)
	{
		// Copied & Pasted directly from above!
		Stat_WriteRowStart( fp, RGBtableHeaders, cols );
		if( m_style != FORMAT_EXCEL )
			Stat_WriteHeader( fp, " ", 1 );
		if( MyPrefStruct.headingOnLeft )
		{
			Stat_WriteHeader( fp, TranslateID(LABEL_MEANPATH), 1 );
			Stat_WriteDualHeader( fp, TranslateID(LABEL_SESSIONS), TranslateID(LABEL_PERCENT) );
		}
		else
		{
			Stat_WriteDualHeader( fp, TranslateID(LABEL_SESSIONS), TranslateID(LABEL_PERCENT) );
			Stat_WriteHeader( fp, TranslateID(LABEL_MEANPATH), 1 );
		}
		Stat_WriteRowEnd( fp );
	}

	/* show Averages */

	if( m_style == FORMAT_EXCEL )
		cellType = AVERAGE;
	Stat_WriteRowStart( fp, RGBtableAvg, cols );
	if( m_style != FORMAT_EXCEL )
		Stat_WriteBottomHeader( fp, " ", 1 );
	if( MyPrefStruct.headingOnLeft )	
		Stat_WriteBottomHeader( fp, TranslateID(LABEL_AVERAGE), 1 );
	Stat_WriteHitData( fp, (long)totalRequests/num,totalRequests );
	if( !MyPrefStruct.headingOnLeft )	
		Stat_WriteBottomHeader( fp, TranslateID(LABEL_AVERAGE), 1 );
	Stat_WriteRowEnd( fp );
	
	/* show Totals */
	if( m_style == FORMAT_EXCEL )
		cellType = SUM;
	Stat_WriteRowStart( fp, RGBtableTotals, cols );
	Stat_WriteBottomHeader( fp, " ", 1 );
	if( MyPrefStruct.headingOnLeft )	
		Stat_WriteBottomHeader( fp, TranslateID(LABEL_TOTALSESSIONS), 1 );
	Stat_WriteHitData( fp, (long)VDptr->byClient->GetStatListTotalVisits(),VDptr->byClient->GetStatListTotalVisits() );
	if( !MyPrefStruct.headingOnLeft )	
		Stat_WriteBottomHeader( fp, TranslateID(LABEL_TOTALSESSIONS), 1 );
	Stat_WriteRowEnd( fp );

	// ********************************************************
	// Do we want the Headers AFTER the Average & Total?
	// ********************************************************
	if( MyPrefStruct.footer_label && MyPrefStruct.footer_label_trailing)
	{
		// Copied & Pasted directly from above!
		Stat_WriteRowStart( fp, RGBtableHeaders, cols );
		if( m_style != FORMAT_EXCEL )
			Stat_WriteHeader( fp, " ", 1 );
		if( MyPrefStruct.headingOnLeft )
		{
			Stat_WriteHeader( fp, TranslateID(LABEL_MEANPATH), 1 );
			Stat_WriteDualHeader( fp, TranslateID(LABEL_SESSIONS), TranslateID(LABEL_PERCENT) );
		}
		else
		{
			Stat_WriteDualHeader( fp, TranslateID(LABEL_SESSIONS), TranslateID(LABEL_PERCENT) );
			Stat_WriteHeader( fp, TranslateID(LABEL_MEANPATH), 1 );
		}
		Stat_WriteRowEnd( fp );
	}


	Stat_WriteTableEnd( fp );
	if( m_style == FORMAT_EXCEL )
		cellType = DATA;

	if( MyPrefStruct.shadow )
		WritePageShadow( fp );

	// ******************************************************************************
	// If this report type suports HelpCards...
	// ******************************************************************************
	if	(	SupportHelpCards()
		&&	ReportCommentOn( MEANPATH_PAGE )
		)
	{
		// ******************************************************************************
		// Add a table DIRECTly following the previous one, however this one has no
		// border - producing the look of footnote-like data.
		// In this case we want it to be just the linking icon with no text.
		// ******************************************************************************
		Stat_WriteIconLink(	fp, "Help.gif", FindReportFilenameStr(MEANPATH_PAGE),	HELPCARD_EXTENSION, "Go the Help Card.", false);
		Stat_InsertTable();
	}
	else
	{
		Stat_WriteSpace( fp, 2 );
	}
	Stat_WriteSpace( fp, 1 );

}

// ****************************************************************************
// Method:		GetNextPage
//
// Abstract:	This method Skips the SESS_... states that are internal and
//				returns the index to the actual next value.  Additionally
//				it populates the pHashId pointer, the pTime pointer and the
//				sum of new-session detected is copied to plNewSessions.
//
// Declaration: GetNextPage(Statistic* pStat, int iStart, long* pHashID, long* pTime, long* plNewSessions)
//
// Arguments:	
//		Statistic*	pStat			: 
//		int			iStart			: 
//		long*		pHashID			: 
//		long*		pTime			: 
//		long*		plNewSessions	: 
//
// Returns:		
//		The index to the next valid state.
//
// ****************************************************************************
int	GetNextPage(Statistic* pStat, int iStart, long* pHashID, long* pTime, long* plNewSessions)
{
	*plNewSessions = 0;
	int i;
	for (i=iStart; i<pStat->sessionStat->SessionNum; ++i)
	{
		*pHashID	= pStat->sessionStat->GetSessionPage(i);
		*pTime		= pStat->sessionStat->GetSessionTime(i);

		if( (*pHashID) == SESS_START_PAGE)
		{
			++(*plNewSessions);
			continue;
		}

		if( (*pHashID) < 0 || (*pHashID) > SESS_ABOVE_IS_PAGE)
		{
			break;
		}
	}		

	return i;
}

// *********************************************************************************
// Implementation
// *********************************************************************************

long CountRepeatVisits( StatList *byStat )
{
	long repeatVisits=0;

	for (size_t i = 0; i<static_cast<size_t>(byStat->num); ++i)
	{
		if( byStat->stat[i].visits > 1 )
		{
			repeatVisits++;
		}
	}

	return repeatVisits;
}


int
LookupKeyVisitor( Statistic *stat )
{
	char	szName[NAME_LENGTH+1];

	szName[0] = szName[NAME_LENGTH] = 0;	// 1st & last to NULL.


	// ******************************************************************
	// The name is reversed, so we need to reverse it again to print it.
	// ******************************************************************
	strncpy(szName, stat->GetName(), NAME_LENGTH);
	if( stat->length > 0)
		ReverseAddress(szName, szName );	// For string length safety reasons.
	
	// ******************************************************************
	// Loop through all the KeyVisitor patterns and look for a match.
	// ******************************************************************
	for (int i=0; i<MyPrefStruct.filterdata.keyvisitorsTot; ++i)
	{
		// ******************************************************************
		// Do we have a match for this pattern?
		// ******************************************************************
		if( match(szName, MyPrefStruct.filterdata.keyvisitors[i], TRUE))
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ****************************************************************************
// struct:		KeyVisitorPageData
// class:		KVPage_less		(like sdt::less<>)
//
// Abstract:	This structure/class is used in a std::map to gather data for
//				the processing of KeyVisitors.
//				It is also used in a std::vector as a pointer and is passed to
//				std::sort where a compare function is called.  The function 
//				compares depending upon the sort value passed to the constructor
//
// Members:		m_eSortType
//
// ****************************************************************************
struct KeyVisitorPageData
{
	KeyVisitorPageData()
			: nCount(0)
			, nSessions(0)
			, lLastAccessSessionNumber(-1)
			, nDuration(0)
			, szURL(0)
	{		
	}

	// The count of the requests
	size_t	nCount;

	// Session count
	size_t	nSessions;
	long	lLastAccessSessionNumber;

	// Duration count.
	size_t	nDuration;
	
	char*	szURL;
};

class KVPage_less : std::binary_function<KeyVisitorPageData*,KeyVisitorPageData*,bool>
{
public:
	KVPage_less(int	eSortType);
	bool operator()(const KeyVisitorPageData* x, const KeyVisitorPageData* y) const;

protected:
	int		m_eSortType;

};

KVPage_less::KVPage_less(int eSortType)
{
	if	(	
		(eSortType == SORT_NAMES)	||
		(eSortType == SORT_PAGES)	||
		(eSortType == SORT_VISITS)
		)
	{
		m_eSortType = eSortType;
	}
	else
	{
		m_eSortType = SORT_REQUESTS;
	}
}

bool
KVPage_less::operator()(const KeyVisitorPageData* x, const KeyVisitorPageData* y) const
{
	switch (m_eSortType)
	{
	case SORT_PAGES:
	case SORT_NAMES:	return (strcmp(x->szURL, y->szURL) < 0);

	case SORT_VISITS:	return (x->nSessions < y->nSessions);

	case SORT_DATE:		return (x->nDuration < y->nDuration);

	default:
	case SORT_REQUESTS:	return (x->nCount < y->nCount);
	}
}

// ****************************************************************************
// Method:		baseFile::Stat_WriteKeyVisitors
//
// Abstract:	This method accumulates and generates a single table for a particular
//				clients pages-report.
//
// Declaration: void baseFile::Stat_WriteKeyVisitors( VDinfoP VDptr, StatList *byStat, FILE *fp, long nIndex, int iDepth, long sort, long nTableNumber )
//
// Arguments:	
//		VDinfoP		VDptr	: 
//		StatList*	byStat	: 
//		FILE*		fp		: 
//		long		nIndex	: 
//		int			iDepth	: 
//		long		sort	: 
//
// Returns:		void
//
// ****************************************************************************
void baseFile::Stat_WriteKeyVisitors( VDinfoP VDptr, StatList *byStat, FILE *fp, long nIndex, int iDepth, long sort, long nTableNumber )
{
	size_t		nColumns(0);
	size_t		nTabWidth(0);
	size_t		nRows(0);
	char		szName[NAME_LENGTH+1];
	Statistic*	pStat;


	nColumns	= 6;
	nTabWidth	= GraphWidth();


	// ********************************************************
	// The name is reversed, so we need to reverse it again to print it.
	// ********************************************************
	szName[0]	= szName[NAME_LENGTH] = 0;	// set 1st & last to null.
	strncpy(szName, byStat->GetName( nIndex ), NAME_LENGTH);
	if( byStat->GetLength(nIndex) > 0)
		ReverseAddress(szName, szName );	// For string length safety reasons.


	// ********************************************************
	// We need the Statistic that is the Client/Visitor.
	// ********************************************************
	pStat = byStat->GetStat( nIndex );


	// ********************************************************
	// Loop through the pages for this Keyvisitor and contruct
	// a map of pages (via KeyVisitorPageData).
	// ********************************************************
	std::map<long,KeyVisitorPageData>	mapHashCount;
	size_t		nTotalRequests(0);
	size_t		nCurrentSessionNumber(1);
	long		lLastPage_HashID(-1);
	size_t		tLastPage_Time(0);
	long		lPageHashID;
	long		lPageTime;
	long		lNewSessions;
	KeyVisitorPageData*	pKV = NULL;

	int	i = GetNextPage(pStat,0,&lPageHashID,&lPageTime,&lNewSessions);
	while (i < pStat->sessionStat->SessionNum)
	{
		lLastPage_HashID = lPageHashID;
		tLastPage_Time	 = lPageTime;
		i = GetNextPage(pStat,i+1,&lPageHashID,&lPageTime,&lNewSessions);

		char*		szURL		= 0;
		Statistic*	pPageStat	= 0;

		// *************************************************************************
		// Verify the page.
		// *************************************************************************
		pPageStat	= VDptr->byPages->FindHashStat(lLastPage_HashID);
		if( !pPageStat)	
		{
			pPageStat	= VDptr->byDownload->FindHashStat(lLastPage_HashID);
			if( !pPageStat)
			{
				// This is a problem!
				DEF_ASSERT(FALSE);
				continue;
			}
		}
		szURL = pPageStat->GetName();


		// *************************************************************************
		// Locate the PageData for this Page that the KeyVisitor has requested.
		// *************************************************************************
		pKV = &mapHashCount[lLastPage_HashID];


		// *************************************************************************
		// Calculate the increase to the duration.
		// *************************************************************************
		if	(lNewSessions)
		{
			// pKV is the last page of the previous session.
			pKV->nDuration += 30;	// default for last page of session.
		}
		else
		{
			if( (lPageTime - tLastPage_Time) < 0)
			{
				DEF_ASSERT(FALSE);
			}
			if( (lPageTime - tLastPage_Time) > 60*10)	// standard timeout.
			{
				// This is unusual, but not an error.
			}

			pKV->nDuration += (lPageTime - tLastPage_Time);
		}


		// *************************************************************************
		// If this the first time this page has been accessed in this session
		// then increment the session count for this page and update the 
		// 'lLastAccessSessionNumber' variable.
		// *************************************************************************
		if	(pKV->lLastAccessSessionNumber != nCurrentSessionNumber)
		{
			pKV->lLastAccessSessionNumber = nCurrentSessionNumber;

			// Increment the unique_session counter.
			++(pKV->nSessions);
		}


		// *************************************************************************
		// Increment the request count.
		// *************************************************************************
		++(pKV->nCount);
		// *************************************************************************
		// Increment the total number of requests.
		// *************************************************************************
		++nTotalRequests;


		// *************************************************************************
		// Assign & test the URL of this page (hashID prblem might show up!)
		// *************************************************************************
		if( pKV->szURL != NULL)
		{
			if( strcmp(pKV->szURL, szURL) != 0)
			{
				// This is really bad!
				// We have two strings which hash to the same hashid.
				DEF_ASSERT(FALSE);
			}
		}
		pKV->szURL = szURL;


		// *************************************************************************
		// Increase the session number by the number of new sessions (often zero!)
		// *************************************************************************
		if( lNewSessions > 0)
			nCurrentSessionNumber += lNewSessions;
	}


	// ********************************************************
	// get the number of rows to the user-define value.
	// ********************************************************
	nRows = mapHashCount.size();


	// ********************************************************
	// If we have No rows then lets not even bother writing the
	// table for this Key Visitor.
	// ********************************************************
	if( (nRows <= 0) || (iDepth <= 0) )
	{
		return;
	}


	// ********************************************************
	// We have committed to writing the table now, so lets write
	// the anchor.
	// ********************************************************
	Stat_WriteAnchor( fp, nTableNumber );
	Stat_WriteSpace( fp, 0 );


	// ********************************************************
	// Now that we have the map fo data we need create a sorted
	// version of it, 'by request count'.
	// To achieve this we need to construct a vector form the map
	// and then sort it.
	// ********************************************************
	std::vector<KeyVisitorPageData*>	vecByRequest;
	vecByRequest.reserve(mapHashCount.size());

	std::map<long,KeyVisitorPageData>::iterator	it;
	for (it=mapHashCount.begin(); it!=mapHashCount.end(); ++it)
	{
		vecByRequest.push_back(&(it->second));
	}


	// ********************************************************
	// Sort the list by SORT_REQUESTS on the sub tables.
	// ********************************************************
	std::sort(vecByRequest.begin(), vecByRequest.end(), KVPage_less(SORT_REQUESTS));


	// ********************************************************
	// Create the Top row, containing the KeyVisitor.
	// ********************************************************
	Stat_WriteTableStart(	fp, nTabWidth );
	Stat_WriteRowStart(		fp, RGBtableTitle, 1 );
	Stat_WriteTitle(		fp, nColumns, nRows, szName );
	Stat_WriteRowEnd(		fp );
		
	// ********************************************************
	// Create the Column headers/titles.
	// - # (number)
	// - Page
	// - Requests
	// - % Requests
	// - Sessions
	// - meantime
	// ********************************************************
	Stat_WriteRowStart( fp, RGBtableHeaders, nColumns );
	Stat_WriteHeader( fp, "", 1 );
	Stat_WriteHeader( fp, TranslateID(LABEL_PAGE),	1 );
	Stat_WriteReqHeader( fp);
	Stat_WriteHeader( fp, TranslateID(LABEL_SESSIONS),	1 );
	Stat_WriteHeader( fp, "Mean Time",	1 );
	Stat_WriteRowEnd( fp );


	char	szTitle[NAME_LENGTH+1];
	std::vector<KeyVisitorPageData*>::reverse_iterator	rit;
	for (i=1,rit=vecByRequest.rbegin(); rit!=vecByRequest.rend() ; ++rit,++i)
	{
		if( i > iDepth)
			break;

		KeyVisitorPageData& refKeyVisitorPageData = *(*rit);

		// Row Start. (Alternating colours)
		Stat_WriteRowStart( fp, ScaleRGB( 100-((i&1)*5), RGBtableItems ), nColumns );
		// # Column
		Stat_WriteNumberData( fp, i );
		// Page Column
		if( MyPrefStruct.page_remotetitle )
		{
			szTitle[0]	= szTitle[NAME_LENGTH]	= 0;
			HTTPURLGetTitle( refKeyVisitorPageData.szURL, szTitle );
			Stat_WriteURL( fp, 1, -1, refKeyVisitorPageData.szURL, szTitle);
		}
		else
		{
			Stat_WriteText(	fp,	1, -1, refKeyVisitorPageData.szURL);
		}

		// Requests Column & %
		Stat_WriteHitData(fp, refKeyVisitorPageData.nCount, nTotalRequests);
		// Sessions Column
		Stat_WriteNumberData( fp, refKeyVisitorPageData.nSessions );
		// MeanTime Column
		Stat_WriteTimeDuration( fp, refKeyVisitorPageData.nDuration, refKeyVisitorPageData.nCount);
		// End of Row
		Stat_WriteRowEnd( fp );
	}

	// ********************************************************
	// Do we want the Headers BEFORE the Average & Total?
	// ********************************************************
	if( MyPrefStruct.footer_label && !MyPrefStruct.footer_label_trailing)
	{
		Stat_WriteRowStart( fp, RGBtableHeaders, nColumns );
		Stat_WriteHeader( fp, "", 1 );
		Stat_WriteHeader( fp, TranslateID(LABEL_PAGE),	1 );
		Stat_WriteReqHeader( fp);
		Stat_WriteHeader( fp, TranslateID(LABEL_SESSIONS),	1 );
		Stat_WriteHeader( fp, "Mean Time",	1 );
		Stat_WriteRowEnd( fp );
	}


	// ********************************************************
	// Add a row detailing the Average,
	// ********************************************************
	Stat_WriteRowStart( fp, RGBtableAvg, nColumns );
	Stat_WriteText(	fp,	1, -1, "");
	Stat_WriteBottomHeader( fp, TranslateID(LABEL_AVERAGE), 1 );
	// Average Requests Column & %
	Stat_WriteReqData( fp, (long)nTotalRequests/nRows,(double)((100.0*nTotalRequests/nRows)/nRows) );
	// Average Sessions Column
	Stat_WriteRight(	fp,	1, -1, "-");	// NA
	// Average MeanTime Column
	Stat_WriteRight(	fp,	1, -1, "-");	// NA
	// End of Row
	Stat_WriteRowEnd( fp );


	// ********************************************************
	// Add a row detailing the Totals.
	// ********************************************************
	Stat_WriteRowStart( fp, RGBtableTotals, nColumns );
	// # Column
	Stat_WriteNumberData( fp, nRows );
	// Page Column
	Stat_WriteBottomHeader( fp, TranslateID(LABEL_TOTALS), 1 );
	// Total Requests Column & %
	Stat_WriteHitData( fp, (long)nTotalRequests, nTotalRequests );
	// Total Sessions Column
	Stat_WriteNumberData( fp, pStat->visits );
	// Total MeanTime Column
	Stat_WriteTimeDuration( fp, pStat->visitTot, pStat->visits);
	// End of Row
	Stat_WriteRowEnd( fp );


	// ********************************************************
	// Do we want the Headers AFTER the Average & Total?
	// ********************************************************
	if( MyPrefStruct.footer_label && MyPrefStruct.footer_label_trailing)
	{
		Stat_WriteRowStart( fp, RGBtableHeaders, nColumns );
		Stat_WriteHeader( fp, "", 1 );
		Stat_WriteHeader( fp, TranslateID(LABEL_PAGE),	1 );
		Stat_WriteReqHeader( fp);
		Stat_WriteHeader( fp, TranslateID(LABEL_SESSIONS),	1 );
		Stat_WriteHeader( fp, "Mean Time",	1 );
		Stat_WriteRowEnd( fp );
	}


	// ********************************************************
	// Close the table.
	// ********************************************************
	Stat_WriteTableEnd( fp );
	Stat_InsertTable();

	// ******************************************************************************
	// If this report type suports HelpCards...
	// ******************************************************************************
	if( SupportHelpCards() )
	{
		// ******************************************************************************
		// If this report has HelpCards turned on...
		// ******************************************************************************
		if( ReportCommentOn( KEYVISITORS_PAGE ) ) 
		{
			// ******************************************************************************
			// Add a table DIRECTly following the previous one, however this one has no
			// border - producing the look of footnote-like data.
			// In this case we want it to be just the linking icon with no text.
			// ******************************************************************************
			Stat_WriteIconLink(	fp, "Help.gif", FindReportFilenameStr(KEYVISITORS_PAGE),	HELPCARD_EXTENSION, "Go the Help Card.", false);
			Stat_InsertTable();
		}
	}

	// ********************************************************
	// Space separate the tables.
	// ********************************************************
	Stat_WriteSpace( fp, 1 );
}

void baseFile::DoKeyVisitorsTables( VDinfoP VDptr, FILE *fp, StatList *byStat, long list, long sort )
{
	long count;
	int	 iTablesGenerated(0);
	char		szName[NAME_LENGTH+1];

	szName[0]	= szName[NAME_LENGTH] = 0;	// Set 1st & Last to NULL.

	Stat_WriteSpace( fp, 1 );
	for( count=0; count<byStat->num && iTablesGenerated<list; count++)
	{
		// ******************************************************************
		// The name is reversed, so we need to reverse it again to print it.
		// ******************************************************************
		strncpy(szName, byStat->GetName( count ), NAME_LENGTH);
		if( byStat->GetLength(count) > 0)
			ReverseAddress(szName, szName );	// For string length safety reasons.

		
		// ******************************************************************
		// Loop through all the KeyVisitor patterns and look for a match.
		// ******************************************************************
		for (int i=0; i<MyPrefStruct.filterdata.keyvisitorsTot; ++i)
		{
			// ******************************************************************
			// Do we have a match for this pattern?
			// ******************************************************************
			if( match(szName, MyPrefStruct.filterdata.keyvisitors[i], TRUE))
			{
				// ******************************************************************
				// Write the sub-table and break out for the next KeyVisitor.
				// ******************************************************************
				Stat_WriteKeyVisitors( VDptr, byStat, fp, count, list, sort, iTablesGenerated);

				// ******************************************************************
				// Increment the generated Table count, so we are displaying only
				// the sub-table which appear in the Graph & the major table.
				// ******************************************************************
				++iTablesGenerated;
				break;
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
// Creates failed request sub-tables that are used for the pages containing broken links report as
// well as for the top referals on errors report.  A sub-table is created for each specified stat
// in <pMainTableStats>.  For each of these stats, its corresponding FailedRequestInfo is obtained
// and used to populate a temp StatList.  This temp StatList is then passed on to WriteTable(),
// which actually does the work of producing the sub-table.  The name of the sub-table will be 
// the same as the name of its corresponding main-table stat with a possible prefix and suffix as
// specified by <subTableNamePrefix> and <subTableNameSuffix>.
//-------------------------------------------------------------------------------------------------
void baseFile::
DoFailedRequestSubTables( VDinfo*		VDptr,						// context in which it all happens
						  FILE*			fp,							// output file pointer
						  const char*	filename,					// output file name
						  StatList*		pMainTableStats,			// creates a sub table for each specified stat
						  size_t		startIndex,					// index of first stat in <pMainTableStats) to start from
						  size_t		numSubTables,				// number of sub-tables to create
						  StatList*		pFailedRequestURLStats,		// source of sub-table stats for temp StatList
						  const char*   firstColHeader,				// header of first column in sub-tables - cannot be 0!!
						  const char*	subTableNamePrefix/*=0*/,	// prepended to main table stat name
						  const char*	subTableNameSuffix/*=0*/,	// appended to main table stat name
						  size_t		maxNumSubTableRows/*=0*/	// if not 0, limits number of sub-table rows produced
						  )
{
	DEF_PRECONDITION( VDptr );
	DEF_PRECONDITION( fp );
	DEF_PRECONDITION( filename );
	DEF_PRECONDITION( pMainTableStats );
	DEF_PRECONDITION( pFailedRequestURLStats );
	DEF_PRECONDITION( firstColHeader );

	// determine the index (plus one) of the last main table stat that we'll be
	// creating a sub table for
	size_t numMainTableStats( static_cast<size_t>(pMainTableStats->GetStatListNum()) );
	size_t stopIndex( startIndex+numSubTables );  // one after the last index that we want
	if( stopIndex > numMainTableStats )
	{
		stopIndex=numMainTableStats;
	}

	// for each sub-table that we need to create...
	for( size_t i(startIndex); i<stopIndex; i++ )
	{
		// ...ref the corresponsing main table stat and...
		Statistic* pMainTableStat=pMainTableStats->GetStat(i);					
		DEF_ASSERT(pMainTableStat);

		// ...ref relevant FailedRequestInfo for that stat
		const CQFailedRequestInfo& failReqInfo=VDptr->GetFailedRequestInfo( pMainTableStat );

		// if the current main table stat has any failed request sub-stats...
		size_t numSubTableRows( failReqInfo.getNumFailedRequests() );
		if( numSubTableRows )
		{
			// ...init a temp StatList to store the failed request sub-stats
			StatList subTableStats( VDptr, 10 );
			subTableStats.useOtherNames = NAMEIS_COPY;

			// get the FailedRequestInfo to populate the temp StatList with those sub-stats
			failReqInfo.addFailedRequestStats( subTableStats, *pFailedRequestURLStats );
			DEF_ASSERT( subTableStats.GetStatListNum()==numSubTableRows );

			// perusual limiting of number of rows
			if( maxNumSubTableRows && numSubTableRows>maxNumSubTableRows )
			{
				numSubTableRows=maxNumSubTableRows;
			}

			// sorting is hard coded (sort by failed requests referred) for the moment
			subTableStats.DoSort( SORT_ERRORS, numSubTableRows );

			// prepare sub-table title
			std::string title;
			title.reserve(80);	// not critical
			if( subTableNamePrefix )
			{
				title+=subTableNamePrefix;	
				title+=' ';
			}
			title+=pMainTableStat->GetName();
			if( subTableNameSuffix )
			{
				title+=' ';
				title+=subTableNameSuffix;	
			}
			
			// write out the sub table for the current server error stat
			Stat_WriteAnchor( fp, i );		// linking
			WriteTable( VDptr,
						&subTableStats,
						fp,
						const_cast<char*>(filename),
						const_cast<char*>(title.c_str()),	// title
						const_cast<char*>(firstColHeader),	// first column header
						-1,									// report id
						'flrq',								// typeID
						0,									// liststart
						numSubTableRows,					// list
						0 );								// graph
		}
	}
}


char *baseFile::GetEngineCGI( char *engine, char *p )
{
	char *file=NULL;

	if( strstr( engine, "altavista" ) )	file = "http://altavista.com/cgi-bin/query?q="; else
	if( strstr( engine, "google" ) )		file = "http://www.google.com/search?q="; else
	if( strstr( engine, "yahoo" ) )		file = "http://search.yahoo.com/bin/search?p="; else
	if( strstr( engine, "lycos" ) )		file = "http://www.lycos.com/srch/?lpv=1&loc=searchhp&query="; else
	if( strstr( engine, "webcrawler" ) )	file = "http://www.webcrawler.com/cgi-bin/WebQuery?searchText="; else
	if( strstr( engine, "infoseek" ) )		file = "http://www.go.com/Titles?col=WW&qt="; else
	if( strstr( engine, "excite" ) )		file = "http://search.excite.com/search.gw?search="; else
	if( strstr( engine, "hotbot" ) )		file = "http://hotbot.lycos.com/?MT="; else
	if( strstr( engine, "anzwers" ) )		file = "http://www.anzwers.com.au/cgi-bin/process_search.pl?pageid=search&firstresult=0&query="; else
	if( strstr( engine, "aolsearch" ) )	file = "http://aolsearch.aol.com/dirsearch.adp?query="; else
	if( strstr( engine, "netfind" ) || strstr( engine, "aol" ) )
											file = "http://search.aol.com/dirsearch.adp?query="; else
	if( strstr( engine, "freeserve" ) )	file = "http://www.ifind.freeserve.com/servlet/search/?qt=b&q="; else
	if( strstr( engine, "search.cnet" ) )	file = "http://www.search.com/search?channel=1&tag=st.se.fd..sch&q="; else
	if( strstr( engine, "metacrawler" ) )	file = "http://search.metacrawler.com/crawler?general="; else
	if( strstr( engine, "cyber411" ) )		file = "http://www.c4.com/cgi-bin/c4-search.cgi?SO=FASTER&SearchText="; else
	if( strstr( engine, "northernlight"))	file = "http://northernlight.com/nlquery.fcg?cb=0&qr="; else
	if( strstr( engine, "alltheweb" ) )	file = "http://www.ussc.alltheweb.com/cgi-bin/search?exec=FAST+Search&type=all&query="; else
	if( strstr( engine, "looksmart" ) )	file = "http://www.looksmart.com/r_search?key="; else
	if( strstr( engine, "evreka" ) )		file = "http://evreka.com/query?kl=&what=web&q="; else
	if( strstr( engine, "infind" ) )		file = "http://infind.com/infind/infind.exe?query="; else
	if( strstr( engine, "goto" ) )			file = "http://goto.com/d/search/;$sessionid$VPH0AYIABQNO1QFIEFOAPUQ?type=home&Keywords="; else
	if( strstr( engine, "mamma" ) )		file = "http://www.mamma.com/Mamma?p1=1&timeout=4&query="; else
	if( strstr( engine, "smallbizsearch" ) ) file = "http://websearch.entrepreneur.com/cgi-bin/texis/webinator/newsearch/?query=";
			
	if( file )
		sprintf( p, "%s", file );	
	else
		*p = 0;

	return file;
}

void baseFile::Stat_WriteSearchEngine( VDinfoP VDptr, StatList *byStat, FILE *fp, long searchEngine, int depth, long sort )
{
	int			list, cols;
	long		n, num;
	__int64		sumBytes = 0, totalBytes = 0;
	long		sumRequests = 0, totalRequests = 0;		// total hits
	char		*name, *heading, engineparam[128];
	long		tabwidth=GraphWidth();
	double		totFile;
	double		totByte;
	std::string	searchEngineName;

	heading = byStat->GetName( searchEngine );
	GetEngineCGI( heading, engineparam );

	searchEngineName = CQSettings::TheSettings().SearchEngines().GetSearchEngineCombinedName( heading );
	heading = (char *)searchEngineName.c_str();

	byStat->SortTimeStat2( searchEngine, sort );
	
	//limit depth if necessary
	list = num = byStat->GetHistoryNum2( searchEngine );
	if( depth>1)
		if( list > depth) list=depth;

	if( MyPrefStruct.dollarpermeg )
		cols = 7; else cols = 6;

	//write heading out
	Stat_WriteTableStart( fp, tabwidth );
	if( IsDebugMode() ){
		if( GetEngineLogo( fp, heading, tempBuff2 ) ){			// write a search engine Logo
			Stat_WriteRowStart( fp, 0xFFFFFF, 1 );
			Stat_WriteTitle( fp, cols, list, tempBuff2 );
		} else
		{
			Stat_WriteRowStart( fp, RGBtableTitle, 1 );
			Stat_WriteTitle( fp, cols, list, heading );
		}
	} else
	{
		Stat_WriteRowStart( fp, RGBtableTitle, 1 );
		Stat_WriteTitle( fp, cols, list, heading );
	}

	Stat_WriteRowEnd( fp );
	Stat_WriteRowStart( fp, RGBtableHeaders, cols );
	Stat_WriteHeader( fp, " ", 1 );
	if( MyPrefStruct.headingOnLeft )	Stat_WriteHeader( fp, TranslateID(LABEL_SEARCHTERM) ,1 );
	Stat_WriteHeader( fp, TranslateID(LABEL_REQUESTS), 1 );
	Stat_WriteHeader( fp, "%" ,1 );
	Stat_WriteHeader( fp, TranslateID(LABEL_BYTESSENT), 1 );
	Stat_WriteHeader( fp, "%", 1 );
	if( MyPrefStruct.dollarpermeg )
	{
		std::string tempStr;
		tempStr = TranslateID(LABEL_CURRENCYUNIT);
		tempStr += " ";
		tempStr += TranslateID(LABEL_COST);
		Stat_WriteHeader( fp, tempStr.c_str(), 1 );
	}
	if( !MyPrefStruct.headingOnLeft )	Stat_WriteHeader( fp, TranslateID(LABEL_SEARCHTERM) ,1 );
	Stat_WriteRowEnd( fp );

	for (n=0; n<num; n++) {	
		long	r,b;
		//hashid = byStat->GetDaysAt2( searchEngine, n );
		r = byStat->GetFilesAt2( searchEngine, n );
		b = byStat->GetBytesAt2( searchEngine, n );
		totalRequests += r;
		totalBytes	+= b;
	}

	totFile = totalRequests / 100.0;
	if( !totFile ) totFile = 1;
	totByte = totalBytes / 100.0;
	if( !totByte )  totByte = 1;

	initAreaRow();
	//now output the stats
	for (n=0; n<list; n++) {	
		long	hashid, item;
		long	r,b;
		hashid = byStat->GetDaysAt2( searchEngine, n );
		r = byStat->GetFilesAt2( searchEngine, n );
		b = byStat->GetBytesAt2( searchEngine, n );

		item = VDptr->bySearchStr->FindHash( hashid );
		if( item >= 0 ){
			char link[2560];
			name = VDptr->bySearchStr->GetName( item );
			sumRequests	+= r;
			sumBytes	+= b;
			Stat_WriteRowStart( fp, RGBtableItems, cols );
			Stat_WriteNumberData( fp, n+1 );
			if( engineparam[0] )
				sprintf( link, "%s%s", engineparam, name );
			else
				link[0] = 0;

			if( MyPrefStruct.headingOnLeft )
				Stat_WriteURL( fp, 1, -1, link, name );

			Stat_WriteHitsAndBytes( fp, r,totalRequests, b, totalBytes );
			Stat_WriteCostData( fp, b );

			if( !MyPrefStruct.headingOnLeft )
				Stat_WriteURL( fp, 1, -1, link, name );
				//Stat_WriteText( fp, 1, -1, name );

			Stat_WriteRowEnd( fp );
		}
	}
	if( m_style != FORMAT_EXCEL )
	{

		if( num > list ){
			/* show OTHERS stat as well */
			Stat_WriteRowStart( fp, RGBtableOthers, cols );
			Stat_WriteNumberData ( fp, num - list);
			if( MyPrefStruct.headingOnLeft ) Stat_WriteBottomHeader( fp, TranslateID(LABEL_OTHERS), 1 );
			Stat_WriteHitsAndBytes( fp, (long)(totalRequests-sumRequests), totalRequests,  totalBytes-sumBytes, totalBytes );
			if( MyPrefStruct.dollarpermeg ) Stat_WriteCostData( fp, totalBytes-sumBytes );
			if( !MyPrefStruct.headingOnLeft ) Stat_WriteBottomHeader( fp, TranslateID(LABEL_OTHERS), 1 );
			Stat_WriteRowEnd( fp );
		}
	}
	
	/* show Averages */
	if( num > 1 ){
		if( m_style == FORMAT_EXCEL )
		{
			rownum=n;
			cellType = AVERAGE;
		}
		Stat_WriteRowStart( fp, RGBtableAvg, cols );
		Stat_WriteBottomHeader( fp, " ", 1 );
		if( MyPrefStruct.headingOnLeft ) Stat_WriteBottomHeader( fp, TranslateID(LABEL_AVERAGE), 1 );
		Stat_WriteHitsAndBytes( fp, (long)totalRequests/num,totalRequests, totalBytes/num,totalBytes );
		if( MyPrefStruct.dollarpermeg ) Stat_WriteCostData( fp, totalBytes/num );
		if( !MyPrefStruct.headingOnLeft ) Stat_WriteBottomHeader( fp, TranslateID(LABEL_AVERAGE), 1 );
		Stat_WriteRowEnd( fp );

		/* show Totals */
		if( m_style == FORMAT_EXCEL )
			cellType = SUM;
		Stat_WriteRowStart( fp, RGBtableTotals, cols );
		Stat_WriteNumberData ( fp, num );
		if( MyPrefStruct.headingOnLeft ) Stat_WriteBottomHeader( fp, TranslateID(LABEL_TOTALS), 1 );
		Stat_WriteHitsAndBytes( fp, (long)totalRequests,totalRequests, totalBytes,totalBytes );
		if( MyPrefStruct.dollarpermeg ) Stat_WriteCostData( fp, totalBytes );
		if( !MyPrefStruct.headingOnLeft ) Stat_WriteBottomHeader( fp, TranslateID(LABEL_TOTALS), 1 );
		Stat_WriteRowEnd( fp );
		if( m_style == FORMAT_EXCEL )
			cellType = DATA;
	}

	Stat_WriteTableEnd( fp );
}


void baseFile::DoSearchEngines( VDinfoP VDptr, FILE *fp, StatList *byStat, long list, long sort )
{
	long count;

	Stat_WriteSpace( fp, 2 );
	for( count=0; count< list; count++){
		Stat_WriteAnchor( fp, count );
		Stat_WriteSearchEngine( VDptr, byStat, fp, count, list, sort );
		Stat_WriteSpace( fp, 2 );
	}
}

/* Write - write list to file */
void baseFile::VHosts_Write( void *vp, FILE *fp, char *title, char *heading )
{
	int			list, cols;
	long		n, num;
	double		totFile;
	double		totByte;
	double		percFiles=0, percBytes=0;
	long		sumVisits=0, sumRequests=0, sumCounters=0, sumTime = 0;
	__int64		sumBytes=0;
	char		*name,
				thishost[MAXURLSIZE];
	long		tabwidth=GraphWidth();
	long		totalCounters;		// total custom counter
	long		totalVisits, totalVisitors = 0;		// total sessions, clients
	long		totalErrors = 0;
	__int64		totalBytes;			// total bytes served
	long		totalTime;			// total time of all sessions
	VDinfoP		VDptr = (VDinfoP)vp;

	if( title == NULL ) title = heading;
	if( heading == NULL ) heading = title;

	num = totalDomains+1;

	totalCounters	= 0;
	totalVisits		= 0;
	totalBytes		= VDptr->totalBytes;
	totalTime		= 0;//VDptr->totalTime;

	GetCorrectSiteURL( VDptr, thishost );

	if( !allTotalRequests)
		totFile = 1;				// avoid divide by zero
	else
		totFile = allTotalRequests / 100.0;
	
	if( !allTotalBytes)
		totByte = 1;
	else
		totByte = allTotalBytes / 100.0;

	//limit depth if necessary
	list = num;

	if( MyPrefStruct.dollarpermeg )
		cols = 11; else cols = 10;

	if( MyPrefStruct.shadow )
		WritePageShadowInit( fp );

	//write heading out
	Stat_WriteTableStart( fp, tabwidth );

	Stat_WriteRowStart( fp, RGBtableTitle, 1 );
	Stat_WriteTitle( fp, cols, list-1, title );		// dont include first vhost which isnt used
	Stat_WriteRowEnd( fp );

	Stat_WriteRowStart( fp, RGBtableHeaders, cols );

	Stat_WriteHeader( fp, " ", 1 );
	Stat_WriteHeader( fp, TranslateID(LABEL_REQUESTS), 1 );
	Stat_WriteHeader( fp, "%",1 );
	Stat_WriteHeader( fp, TranslateID(LABEL_BYTESSENT), 1 );
	Stat_WriteHeader( fp, "%",1 );
	Stat_WriteHeader( fp, TranslateID(LABEL_SESSIONS), 1 );
	Stat_WriteHeader( fp, TranslateID(LABEL_VISITORS), 1 );
	Stat_WriteHeader( fp, TranslateID(LABEL_PAGES), 1 );
	Stat_WriteHeader( fp, TranslateID(LABEL_ERRORS), 1 );
	if( MyPrefStruct.dollarpermeg )
	{
		std::string tempStr;
		tempStr = TranslateID(LABEL_CURRENCYUNIT);
		tempStr += " ";
		tempStr += TranslateID(LABEL_COST);
		Stat_WriteHeader( fp, tempStr.c_str(), 1 );
	}
	Stat_WriteHeader( fp, heading ,1 );
	Stat_WriteRowEnd( fp );

	int hostn = 0;
	//now output the stats
	for (n=0; n<list; n++) 
	{
		long	tdrgb=-1;
		name = VDptr->domainName;
		if( !name ) name = " ";
		sumRequests	+= VDptr->totalRequests;
		sumBytes	+= VDptr->totalBytes;
		sumVisits	+= VDptr->byClient->GetStatListTotalVisits();
		sumCounters	+= VDptr->byClient->GetStatListTotalCounters4();
		percFiles	+= (VDptr->totalRequests / totFile);
		percBytes	+= (VDptr->totalBytes / totByte);
	
		if( VDptr->totalRequests ){
			if( n < 50 )
				tdrgb = multi_rgb[ hostn % 10 ];

			Stat_WriteRowStart( fp, RGBtableItems, cols );

			Stat_WriteNumberData( fp, hostn+1 );
			Stat_WriteHitsAndBytes( fp, VDptr->totalRequests,allTotalRequests, VDptr->totalBytes, allTotalBytes );
			Stat_WriteNumberData( fp, VDptr->byClient->GetStatListTotalVisits() );
			Stat_WriteNumberData( fp, VDptr->byClient->GetStatListNum() );
			Stat_WriteNumberData( fp, VDptr->byClient->GetStatListTotalCounters4() );
			Stat_WriteNumberData( fp, VDptr->totalFailedRequests );
			totalErrors += VDptr->totalFailedRequests;
			totalVisitors += VDptr->byClient->GetStatListNum();

			Stat_WriteCostData( fp, VDptr->totalBytes );
			Stat_WriteText( fp, 1, tdrgb, name );
			Stat_WriteRowEnd( fp );
			hostn++;
		}
		VDptr = (VDinfoP)VDptr->next;
		if( !VDptr ) n=list;
	}

	/* show Averages */
	Stat_WriteRowStart( fp, RGBtableAvg, cols );
	Stat_WriteHeader( fp, " ", 1 );
	Stat_WriteHitsAndBytes( fp, (long)allTotalRequests/num,(allTotalRequests), allTotalBytes/num,allTotalBytes );
	
	Stat_WriteNumberData( fp, sumVisits/num );
	Stat_WriteNumberData( fp, totalVisitors/num );
	Stat_WriteNumberData( fp, sumCounters/num );
	Stat_WriteNumberData( fp, totalErrors/num );
	if( MyPrefStruct.dollarpermeg ) Stat_WriteCostData( fp, allTotalBytes/num );
	Stat_WriteBottomHeader( fp, TranslateID(LABEL_AVERAGE), 1 );
	Stat_WriteRowEnd( fp );


	/* show Totals */
	Stat_WriteRowStart( fp, RGBtableTotals, cols );
	Stat_WriteHeader( fp, " ", 1 );
	Stat_WriteHitsAndBytes( fp, (long)allTotalRequests,(allTotalRequests), allTotalBytes,allTotalBytes );
	Stat_WriteNumberData( fp, sumVisits );
	Stat_WriteNumberData( fp, totalVisitors );
	Stat_WriteNumberData( fp, sumCounters );
	Stat_WriteNumberData( fp, totalErrors );
	if( MyPrefStruct.dollarpermeg ) Stat_WriteCostData( fp, allTotalBytes );
	Stat_WriteBottomHeader( fp, TranslateID(LABEL_TOTALS), 1 );

	Stat_WriteRowEnd( fp );
	Stat_WriteTableEnd( fp );

	if( MyPrefStruct.shadow )
		WritePageShadow( fp );

}



// quick compare by doing a check of first 4 bytes only
long baseFile::CheckStrQ( char *s1, char *s2 )
{
	long x1, x2, xd;

	x1 = *(long*)(s1);
	x2 = *(long*)(s2);
	xd = x1 - x2;

	if( xd == 0 || xd == 0x20 ) return 0;
	return 1;
}



// the better way to do headers based on table information
long baseFile::Stat_WriteReportHeaders( FILE *fp, long type, long reportTypeId, long extras, char *defaultname )
{
	long done = 0;

	switch( type )
	{
		case 'clen':	Stat_WriteHeader( fp, TranslateID(LABEL_CLIPLENGTH), 1 );		done+=1;		break;
		case 'csiz':	Stat_WriteHeader( fp, TranslateID(LABEL_BYTES), 1 );			done+=1;		break;
		case 'catv':	Stat_WriteHeader( fp, TranslateID(LABEL_AVGTIMEVIEW),1 );		done+=1;		break;
		case 'capv':	Stat_WriteHeader( fp, TranslateID(LABEL_PERCPLAYED),1 );		done+=1;		break;
		case 'caph':	Stat_WriteHeader( fp, TranslateID(LABEL_AVGTIMELISTENING), 1 ); done+=1;		break;
		case 'cptx':	Stat_WriteDualHeader( fp, TranslateID(LABEL_TOTALPACKETS_TX), TranslateID(LABEL_PERCENT) );	done+=2;		break;
		case 'cprx':	Stat_WriteDualHeader( fp, TranslateID(LABEL_TOTALPACKETS_RX), TranslateID(LABEL_PERCENT) );	done+=2;		break;
		case 'capl':	Stat_WriteHeader( fp, TranslateID(LABEL_AVGPACKETLOSS), 1 );	done+=1;		break;
		case 'catr':	Stat_WriteHeader( fp, TranslateID(LABEL_AVGBPSRATE), 1 );		done+=1;		break;
		case 'caps':	Stat_WriteHeader( fp, TranslateID(LABEL_PERCSTREAMED),1 );		done+=1;		break;
		case 'cett':	Stat_WriteHeader( fp, TranslateID(LABEL_TOTALMBEXPECTED), 1 );	done+=1;		break;
		case 'catt':	Stat_WriteHeader( fp, TranslateID(LABEL_TOTALMBSTREAMED), 1 );	done+=1;		break;
		case 'catq':	Stat_WriteHeader( fp, TranslateID(LABEL_TRANSFERQUALITY), 1 );	done+=1;		break;

		case 'cbuf':	Stat_WriteHeader( fp, TranslateID(LABEL_AVGSECSBUFFERED), 1 );	done+=1;		break;
		case 'caff':	Stat_WriteHeader( fp, TranslateID(LABEL_AVG_FF), 1 );			done+=1;		break;
		case 'carw':	Stat_WriteHeader( fp, TranslateID(LABEL_AVG_RW), 1 );			done+=1;		break;
		case 'ctff':	Stat_WriteHeader( fp, TranslateID(LABEL_NUMOF_FF), 1 );			done+=1;		break;
		case 'ctrw':	Stat_WriteHeader( fp, TranslateID(LABEL_NUMOF_RW), 1 );			done+=1;		break;

		case 'cmce':	Stat_WriteHeader( fp, TranslateID(LABEL_MOSTCOMMONERROR), 1 );	done+=1;		break;
		case 'cunp':	Stat_WriteDualHeader( fp, TranslateID(LABEL_UNINTERRUPTEDPLAYS), TranslateID(LABEL_PERCENT) );	done+=2;	break;
		case 'cmax':	Stat_WriteHeader( fp, TranslateID(LABEL_MAXCONNECTIONS), 1 );done+=1;		break;
		case 'time':	Stat_WriteHeader( fp, TranslateID(LABEL_TIMESTAMP), 1 );		done+=1;		break;

		case 'reqb':	Stat_WriteReqHeader(  fp ); done+=2;
						Stat_WriteByteHeader( fp ); done+=2;
						break;
		case 'req1':	Stat_WriteHeader( fp, "Win", 1 ); done+=1;			break;
		case 'req2':	Stat_WriteHeader( fp, "Mac", 1 ); done+=1;			break;
		case 'req3':	Stat_WriteHeader( fp, "Unix", 1 ); done+=1;			break;
		case 'Requ':	if( extras )
		case 'requ':	
						if( reportTypeId=='pcbl' )
						{
							Stat_WriteDualHeader( fp, TranslateID(SUMM_TOTALREQS), TranslateID(LABEL_PERCENT) );
						}
						else
						{
							Stat_WriteReqHeader( fp );
						}
						done+=2;
						break;
		case 'Byte':	if( extras )
		case 'byte':	Stat_WriteByteHeader( fp ); done+=2;			break;
		case 'Sess':	if( extras )
		case 'sess':	Stat_WriteHeader( fp, TranslateID(LABEL_SESSIONS), 1 ); done+=1;			break;
		case 'Visi':	if( extras )
		case 'visi':	Stat_WriteHeader( fp, TranslateID(LABEL_VISITORS), 1 ); done+=1;			break;
		case 'Page':	if( extras )
		case 'page':
						if( MyPrefStruct.stat_style == STREAM )
							Stat_WriteHeader( fp, TranslateID(LABEL_CLIPS), 1 );
						else
							Stat_WriteHeader( fp, TranslateID(LABEL_PAGES), 1 );
						done+=1;
						break;
		case '%pag':	
						Stat_WriteHeader( fp, TranslateID(LABEL_PAGES), 1 ); done+=1;
						Stat_WriteHeader( fp, TranslateID(LABEL_PERCENT), 1 ); done+=1;
						break;
		case '%ses':	
						Stat_WriteHeader( fp, TranslateID(LABEL_SESSIONS), 1 ); done+=1;
						Stat_WriteHeader( fp, TranslateID(LABEL_PERCENT), 1 ); done+=1;
						break;
		case 'Pagl':	if( extras )
		case 'pagl':	Stat_WriteHeader( fp, TranslateID(LABEL_PAGESLEAST), 1 ); done+=1;			break;

		case 'Dura':	if( extras )
		case 'dura':	Stat_WriteHeader( fp, TranslateID(TIME_MEANTIME), 1 ); done+=1;		break;
		case 'Durv':	if( extras )
		case 'durv':	Stat_WriteHeader( fp, TranslateID(TIME_MEANTIME), 1 ); done+=1;		break;
		case 'Durp':	if( extras )
		case 'durp':	Stat_WriteHeader( fp, TranslateID(TIME_MEANTIME), 1 ); done+=1;		break;

		case 'tota':	Stat_WriteHeader( fp, TranslateID(TIME_TOTALTIME), 1 ); done+=1;	break;
		case 'Erro':	if( extras )
		case 'erro':	Stat_WriteHeader( fp, TranslateID(LABEL_ERRORS), 1 ); done+=1;	break;
		case 'cnt4':
						switch( reportTypeId )
						{
							case 'adds' :
							case 'addh' :	Stat_WriteHeader( fp, TranslateID(LABEL_CLICKTHROUGHS), 1 );	done+=1; break;
							case 'adcs' :
							case 'adch' :	Stat_WriteHeader( fp, TranslateID(LABEL_COSTPERMONTH), 1 ); done+=1; break;
						}
						break;
		case 'addp':	
						{
							std::string tempStr;
							tempStr = TranslateID(LABEL_PERCENT);
							tempStr += " ";
							tempStr += TranslateID(LABEL_CLICKS);
							Stat_WriteHeader( fp, tempStr.c_str(), 1 );
							done++;
							break;
						}
		case 'addc':	Stat_WriteHeader( fp, TranslateID(LABEL_COSTPERCLICK), 1 ); done+=1;	break;
		case 'Fses':	Stat_WriteHeader( fp, TranslateID(LABEL_FIRSTSESSIONS), 1 ); done+=1;	break;
		case 'Lses':	Stat_WriteHeader( fp, TranslateID(LABEL_LASTSESSIONS), 1 ); done+=1;	break;
		case 'cost':
						if( MyPrefStruct.dollarpermeg )
						{
							std::string tempStr;
							tempStr = TranslateID(LABEL_CURRENCYUNIT);
							tempStr += " ";
							tempStr += TranslateID(LABEL_COST);
							Stat_WriteHeader( fp, tempStr.c_str(), 1 );
							done++;
						}
						break;
		case 'cach':
						Stat_WriteDualHeader( fp, TranslateID(LABEL_CACHEDHITS), TranslateID(LABEL_PERCENT) );
						done+=2;
						break;
		case 'brok':
						Stat_WriteDualHeader( fp, TranslateID(LABEL_BROKENLINKS), TranslateID(LABEL_PERCENT) );
						done+=2;
						break;
		case 'reqf':
						{
							int headerID( reportTypeId=='pcbl' ? LABEL_BROKENLINKREQUESTS : LABEL_REQUESTSREFERRED );
							Stat_WriteDualHeader( fp, TranslateID(headerID), TranslateID(LABEL_PERCENT) );
							done+=2;
						}
						break;
		case 'indx':	Stat_WriteHeader( fp, " ", 1 ); done+=1;		break;

		// *************************************************
		// Billing Columns.
		// *************************************************
		case 'bCus':	// Customer Name
			if (MyPrefStruct.billingSetup.bHeaderCustomerName)
			{
				Stat_WriteHeader( fp, TranslateID(LABEL_BILLING_CUSTOMER_NAME), 1 );
				done+=1;
			}
			break;
		case 'bCID':	// Customer ID
			if (MyPrefStruct.billingSetup.bHeaderCustomerID)
			{
				Stat_WriteHeader( fp, TranslateID(LABEL_BILLING_CUSTOMER_ID), 1 );
				done+=1;
			}
			break;
		case 'bMBt':	// MB Transfered
			if (MyPrefStruct.billingSetup.bHeaderMBTransfered)
			{
				Stat_WriteHeader( fp, TranslateID(LABEL_BILLING_BYTES_TRANSFERRED), 1 );
				done+=1;
			}
			break;
		case 'bMBa':	// MB Allowance
			if (MyPrefStruct.billingSetup.bHeaderMBAllowance)
			{
				Stat_WriteHeader( fp, TranslateID(LABEL_BILLING_BYTE_ALLOWANCE), 1 );
				done+=1;
			}
			break;
		case 'bExc':	// Excess Charge
			if (MyPrefStruct.billingSetup.bHeaderExcessCharges)
			{
				Stat_WriteHeader( fp, TranslateID(LABEL_BILLING_EXCESS_CHARGE), 1 );
				done+=1;
			}
			break;
		case 'bTot':	// Total Charge
			if (MyPrefStruct.billingSetup.bHeaderTotalCharges)
			{
				Stat_WriteHeader( fp, TranslateID(LABEL_BILLING_TOTAL_CHARGE), 1 );
				done+=1;
			}
			break;
		// *************************************************
		// End Billing Columns.
		// *************************************************

		default:		Stat_WriteHeader( fp, defaultname, 1 ); done+=1;		break;
	}
	return done;
}

/* Write - write list to file */
void baseFile::WriteTable( void *vp, StatList *byStat, FILE *fp,
							   char *filename, char *title, char *heading,
							   short id, long typeID, short liststart, short list, short graph )
{
	rownum = 0;

	// Check if we have a "Stat" object to output
	if( !byStat )
		return;

	if( title == NULL ) title = heading;
	if( heading == NULL ) heading = title;

	// Determine the width of the table
	long tabwidth=GraphWidth();
	//if( MyPrefStruct.graph_wider )		tabwidth = (long)(tabwidth*1.5);

	// Get the Domain address 
	char thishost[MAXURLSIZE];

	VDinfoP	VDptr = (VDinfoP)vp;
	GetCorrectSiteURL( VDptr, thishost );

	long num = byStat->GetStatListNum();

	// Write unit totals in comments
	sprintf( tempBuff2, "Total Units = %ld (bytes=%ld) / %ld (%ld)", num, sizeof(Statistic)*num, byStat->maxNum, byStat->maxNum*sizeof(Statistic) );
	Stat_WriteComment( fp, tempBuff2 );

	//header information
	if( m_style == FORMAT_HTML )
	{
#ifdef	DEF_FULLVERSION
		if( MyPrefStruct.stat_clientStream && MyPrefStruct.stat_sessionHistory && typeID == 'clis')
			//Stat_WriteBold (fp, "NOTE: Click on the clients to show individual session history");
			Stat_WriteBold (fp, TranslateID(COMM_CLICKSESSIONHIST));
#endif
	}
	
	if( MyPrefStruct.shadow )
		WritePageShadowInit( fp );

	int extras = 0; // = 0 added by TREV as was not being initialised...
	if( MyPrefStruct.stat_sessionHistory )		// uppercase ABD sessionhistory
		extras = 1;

	char *columnStrPtr[16];
	long columnIDs[16];
	int	colspan=10, cols;
	long hashID;

	GetTableColumnHeadings( columnStrPtr, columnIDs, colspan, cols, hashID, typeID );

	// We really not not want these to be NULL at this stage.
	DEF_ASSERT(title != NULL);
	DEF_ASSERT(heading != NULL);

	if( title == NULL)
		title = "";
	if( heading == NULL)
		heading = "";
	//colspan here is really the headingSpan in a way.

	switch (typeID)
	{
		case 'kvis':	title = TranslateID(REPORT_KEYVISITORS_TABLE);	break;
		case 'kpag':	title = TranslateID(REPORT_ROUTESTO_TABLE);		break;
		case 'kpaf':	title = TranslateID(REPORT_ROUTESFROM_TABLE);	break;
	}

	WriteTableHeadings( tabwidth, fp, colspan, list, title, heading, cols, columnIDs, typeID, extras );

	WriteTableData( vp, byStat, fp, filename, title, heading, typeID, liststart,list, graph,
					id, cols, colspan, columnIDs, columnStrPtr, thishost	);
}

void baseFile::GetTableColumnHeadings( char *columnStrPtr[/*16*/],
		long columnIDs[/*16*/],	int	&colspan, int &cols, long &hashID, long typeID )
{
	// new funky fast way to do table headers, based on tables.
	char *columnStr;
	char *formatStr = FindHeadFormat( typeID );
	char *type = mystrchr( formatStr, ',' );
	long i = 0, extras = 0;

	if( type ){
		type++;
		while( *type == ' ' ) type++;			// skip spaces
		columnStr = type;
		columnIDs[i++] = 'indx';
		while( type ){
			columnIDs[i] = ReadLong( (unsigned char *)type );
			if( columnIDs[i] == 'reqb'  ) extras+=3;
			if( columnIDs[i] == 'requ' ) extras++;
			if( columnIDs[i] == '%pag' ) extras++;
			if( columnIDs[i] == '%ses' ) extras++;
			if( columnIDs[i] == 'brok' ) extras++;
			if( columnIDs[i] == 'reqf' ) extras++;
			if( columnIDs[i] == 'cach' ) extras++;
			if( columnIDs[i] == 'cptx' ) extras++;
			if( columnIDs[i] == 'cprx' ) extras++;
			if( columnIDs[i] == 'cunp' ) extras++;
			if( columnIDs[i] == 'cost' && !MyPrefStruct.dollarpermeg )	i--;
			type = mystrchr( type, ',' );
			if( type ){
				type++;
				while( *type == ' ' ) type++;			// skip spaces
			}
			i++;
		}
		columnIDs[i] = 0;
		colspan = cols = i;

		//swap order
		if( MyPrefStruct.headingOnLeft ){
			hashID = columnIDs[i-1];
			for( long n=i-1; n>1; n-- ){
				columnIDs[n] = columnIDs[n-1];
			}
			columnIDs[1] = hashID;
		}
		/////////////

		colspan += extras;
	}
}

void baseFile::WriteTableHeadings( long tabwidth, FILE *fp, int colspan, short list, char *title, char *heading, int totalCols, long columnIDs[], long typeID, int extras )
{
	Stat_WriteTableStart( fp, tabwidth );
	Stat_WriteRowStart( fp, RGBtableTitle, 1 );
	Stat_WriteTitle( fp, colspan, list, title );
	Stat_WriteRowEnd( fp );

	Stat_WriteRowStart( fp, RGBtableHeaders, colspan );

	//if( MyPrefStruct.headingOnLeft )
	//	Stat_WriteHeader( fp, heading, 1 );
	for( int n=0; n<totalCols; n++){
		Stat_WriteReportHeaders( fp, columnIDs[n], typeID, extras, heading );
	}
	//if( !MyPrefStruct.headingOnLeft )
	//	Stat_WriteHeader( fp, heading, 1 );

	Stat_WriteRowEnd( fp );
}


// ********************************************************************************
// This function adds extra data to the statistics based upon the billing settings.
// It is required to be called After Processing, but before Report writing.
// ********************************************************************************
void
CalculateBillingData(StatList* pStatList, BillingSetupStruct* pBillingSetup, BillingChargesStruct* pBillingCharges)
{	
	BillingCustomerStruct*	pCustomer;
	Statistic*				pStatistic;

	pStatList->totalCounters	= 0;
	pStatList->totalCounters4	= 0;
	pStatList->totalVisits		= 0;


	for (int i=0; pStatistic=pStatList->GetStat(i); ++i)
	{
		for (pCustomer=pBillingCharges->pFirstCustomer; pCustomer; pCustomer=pCustomer->pNext)
		{
			if (mystrcmpi(pStatistic->name, pCustomer->szURL) == 0)
				break;
		}

		if (!pCustomer)	// If we have no settings for this customer then ignore it and continue.
			continue;

		// *************************************************************
		// Populate the map with the bytes for each BillingPeriod Chunk
		// *************************************************************
		std::map<size_t, __int64>		mapTotals;
		{
			TimeRecPtr	pTime = pStatistic->timeStat->byTimerec;
			for (int i=0;i<pStatistic->timeStat->byTimerecNum; ++i)
			{
				long		lDay	= pTime[i].day;		// Days since 1970.
				long		lBytes	= pTime[i].bytes;		// Total bytes on this days.
				long		nKey(0);

				{
					time_t		t		= lDay*86400;
					struct tm*	ptm		= localtime(&t);
					int			iYear	= ptm->tm_year+1900;

					if (pBillingSetup->ucBillingPeriod == 0)		// Day(s)
					{
						nKey = (iYear<<16) + ptm->tm_yday;
					}
					else if (pBillingSetup->ucBillingPeriod == 1)	// Week(s)		Sunday -> Saturday
					{
						int		iWeek(0);
						size_t	nDayAdd(0);

						if ((ptm->tm_yday - ptm->tm_wday) <= 0)
						{
							--iYear;
							if (iYear%4 == 0)
								nDayAdd = 366;
							else
								nDayAdd = 365;
						}
						iWeek = (ptm->tm_yday + nDayAdd - ptm->tm_wday + 6) / 7;
						DEF_ASSERT(iWeek != 0);

						nKey = (iYear<<16) + iWeek;
					}
					else if (pBillingSetup->ucBillingPeriod == 2)	// Month(s)		1st -> 31st.
					{
						nKey = (iYear<<16) + ptm->tm_mon;
					}
					else if (pBillingSetup->ucBillingPeriod == 3)	// Quarter(s)	1st (Jan,Apr,Jul,Oct) -> 31st (Mar,Jun,Sep,Dec).
					{
						nKey = (iYear<<16) + (ptm->tm_mon/3);
					}
				}

				// nKey contains the Year and the (Days/Weeks/Months/Quarters) within the year.
				mapTotals[nKey]	+= lBytes;
			}
		}

		
		// *************************************************************
		// Fixed Charge, #Units and Unit size.
		// *************************************************************
		int		iFixedUnits	= atoi(pCustomer->szFixedPeriod);
		__int64	iFixedUnitSize;
		double	dFixedCharge;
		(void)sscanf(pCustomer->szFixedCharge, "%lf", &dFixedCharge);
		if (pCustomer->ucFixedGroup == 0)		// Kilobytes
			iFixedUnitSize = 1000;
		else if (pCustomer->ucFixedGroup == 1)	// Megabytes
			iFixedUnitSize = 1000000;
		else if (pCustomer->ucFixedGroup == 2)	// Gigabytes
			iFixedUnitSize = 1000000000;
		else if (pCustomer->ucFixedGroup == 3)	// Terabytes
			iFixedUnitSize = 1000000000000;


		// *************************************************************
		// Excess Charge, #Units and Unit size.
		// *************************************************************
		int		iExcessUnits	= atoi(pCustomer->szExcessPeriod);
		__int64	iExcessUnitSize;
		double	dExcessCharge;
		(void)sscanf(pCustomer->szExcessCharge, "%lf", &dExcessCharge);
		if (pCustomer->ucExcessGroup == 0)		// Kilobytes
			iExcessUnitSize = 1000;
		else if (pCustomer->ucExcessGroup == 1)	// Megabytes
			iExcessUnitSize = 1000000;
		else if (pCustomer->ucExcessGroup == 2)	// Gigabytes
			iExcessUnitSize = 1000000000;
		else if (pCustomer->ucExcessGroup == 3)	// Terabytes
			iExcessUnitSize = 1000000000000;


		// *************************************************************
		// Now we have the total-bytes per BillingPeriod.
		// We need to Calculate the Total Cost and Total Excess Cost.
		// *************************************************************
		double	dTotalFixedCost(0);
		double	dTotalExcessCost(0);
		__int64	nTotalBytes(0);
		std::map<size_t, __int64>::iterator it;
		if (pCustomer->bExcessCharge)
		{
			for (it=mapTotals.begin(); it!=mapTotals.end(); ++it)
			{
				__int64	iBytesThisPeriod = it->second;

				nTotalBytes		+= iBytesThisPeriod;
				dTotalFixedCost	+= dFixedCharge;

				if (iBytesThisPeriod > (iFixedUnits*iFixedUnitSize) )
				{
					int	iExcessBytes	= iBytesThisPeriod - (iFixedUnits*iFixedUnitSize);

					dTotalExcessCost += (iExcessBytes*dExcessCharge)/iExcessUnitSize;
				}
			}
		}


		// *************************************************************
		// Done with this Client.(Statistic).
		// *************************************************************
		pStatistic->bytes;													// MBTransfered
		pStatistic->counter4	= dTotalExcessCost*100;						// Excess Charges (in cents)
		pStatistic->counter		= (dTotalFixedCost+dTotalExcessCost)*100;	// Total Charges (in cents)
		pStatistic->visits		= iFixedUnits*iFixedUnitSize;				// MB Allowance. (in bytes!!!!)

		pStatistic->name;									// url	(pCustomer->szURL)
		// url leads to the pCustomer.
		pCustomer->szCustomerID;
		pCustomer->szCustomer;


		// *************************************************************
		// Need to accumulate totalVisitors, totalCounter4, totalCounter
		// *************************************************************
		pStatList->totalCounters	+= pStatistic->counter;
		pStatList->totalCounters4	+= pStatistic->counter4;
		pStatList->totalVisits		+= pStatistic->visits;
	}
}

BillingCustomerStruct*
LocateCustomer(const char* szURL)
{
	BillingCustomerStruct*	p;

	for (p=MyPrefStruct.billingCharges.pFirstCustomer; p; p=p->pNext)
	{
		if (strcmp(szURL, p->szURL) == 0)
			break;
	}

	return p;
}


// ------------------------ WRITE TABLE DATA
#define	MAX_COLUMNS		16

void baseFile::WriteTableData( void *vp, StatList *byStat, FILE *fp,
	char *filename, char *title, char *heading,long typeID, short liststart, short list, short graph,
	short id, int cols, int colspan, long columnIDs[/*16*/], char *columnStrPtr[/*16*/],	char thishost[/*MAXURLSIZE*/] )
{
	VDinfoP	VDptr		= (VDinfoP)vp;
	char *formatStr		= FindHeadFormat( typeID );
	char *type			= mystrchr( formatStr, ',' );
	long len			= mystrlen(heading)+1;
	long num			= byStat->GetStatListNum();
	long totalRequests	= byStat->totalRequests;
	long totalCounters	= byStat->totalCounters;
	long totalCounters4	= byStat->totalCounters4;
	long totalVisits	= byStat->totalVisits;
	long totalTime		= byStat->totalTime;
	long totalErrors	= byStat->totalErrors;
	__int64 totalBytes	= byStat->totalBytes;

	__int64 sumColumns[MAX_COLUMNS];		// This contains the sub totals for each column type, (used only for streaming at the moment)


	for( long i=0; i<MAX_COLUMNS; i++){
		sumColumns[i] = 0;
	}


	if( byStat->m_FilterIndex )
	{
		num				= 
		totalRequests	= 
		totalCounters	= 
		totalCounters4	= 
		totalVisits		= 
		totalTime		= 
		totalErrors		= 0;
		totalBytes		= 0;

		Statistic* pStat;
		for( pStat = byStat->GetFirstStat(); pStat; )
		{
			++num;
			totalRequests	+= pStat->files;
			totalCounters	+= pStat->counter;
			totalCounters4	+= pStat->counter4;
			totalVisits		+= pStat->visits;
			totalBytes		+= pStat->bytes;
			totalTime		+= pStat->visitTot;
			totalErrors		+= pStat->errors;

			pStat = byStat->GetNextStat( pStat );
		}
	}

	double totFile = totalRequests / 100.0;
	if( !totFile)
		totFile = 1;				// avoid divide by zero
	double totByte = totalBytes / 100.0;
	if( !totByte)
		totByte = 1;
	double totError = totalErrors / 100.0;
	if( !totError)
		totError = 1;
	double totCounters4 = totalCounters4 / 100.0;
	if( !totCounters4)
		totCounters4 = 1;


	long sumVisits=0, sumRequests=0, sumCounters=0, sumCounters4=0, sumErrors=0, sumVisitIn = 0;
	long sumReq1=0, sumReq2=0, sumReq3=0;
	char *name;
	char rname[MAXURLSIZE];
	__int64	sumBytes=0;
	double percFiles=0,percBytes=0,percErrors=0,percCounters4=0;
	long sumTime = 0;			// total time of all sessions

	// Start Added by Trev
	int extras = 0; // = 0 added by TREV 
	if( MyPrefStruct.stat_sessionHistory )		// uppercase ABD sessionhistory
		extras = 1;
	// End Added by Trev
	
	// only for Browser/OS add up indiviual totals
	if( typeID == 'bros' ){
		for (long n=0; n<num; n++) {
			register Statistic	*p;
			p = byStat->GetStat(n);
			if( p ){
				sumReq1 += p->GetFiles2(1);
				sumReq2 += p->GetFiles2(2);
				sumReq3 += p->GetFiles2(3);
			}
		}
	}
	//now output the stats
	rownum = 0;
	initAreaRow();
	std::string searchEngineName;
	for (long n=liststart; rownum<list && n<byStat->GetStatListNum(); n++) {
		char		lastTimeStr[42];
		register	Statistic	*p;
		long		tdrgb;
		__int64		bytes; 

		p = byStat->GetStat(n);
		if( !p )
			continue;

		if( typeID == 'seng' )
		{
			name = p->GetName();
			searchEngineName = CQSettings::TheSettings().SearchEngines().GetSearchEngineCombinedName( name );
			name = (char *)searchEngineName.c_str();
		}
		else if( byStat->GetNameTrans )
			name = byStat->GetNameTrans( (short)n );
		else
			name = p->GetName();
		if( !name ) continue;
		bytes = p->bytes;

		if( byStat->m_FilterIndex)
		{
			// *****************************************************************************************
			// If this in to be filtered out then continue from here as the following code does the
			// table generation.
			// We do the 'sum'ing calculations below as they are not desired in the totals.
			// *****************************************************************************************
			if( !byStat->m_FilterIndex(p))
				continue;
		}

		sumRequests	+= p->files;
		sumBytes	+= bytes;
		sumVisits	+= p->visits;
		sumCounters += p->counter;
		sumCounters4 += p->counter4;
		sumTime		+= p->visitTot;
		sumErrors	+= p->errors;
		sumVisitIn	+= p->GetVisitIn();
		percFiles	+= (p->files / totFile);
		percBytes	+= (bytes / totByte);
		percErrors	+= ( p->errors / totError);
		percCounters4+= (p->counter4 / totCounters4 ); 


		CTimetoString( p->lastday, lastTimeStr );
	
		if( n < 50 && strstri( filename, "Hist" ) && !MyPrefStruct.corporate_look )
			tdrgb = multi_rgb[ n % 10 ]; else tdrgb = -1;

		if( typeID == 'tope' && p->counter4 == 0 )	break;		// stop if no more top entry pages
		if( typeID == 'topx' && p->GetVisitIn() == 0 )	break;		// stop if no more top exit pages

		// Make every 2nd row a darker grey.
		Stat_WriteRowStart( fp, ScaleRGB( 100-((rownum&1)*5), RGBtableItems ), colspan );
	
		if( IsDebugMode() )
		{
			char tstr[256];
			sprintf( tstr, "index=%ld, hash=0x%08lX, %ld, Hashit()=%08lx", n, p->hash, p->hash, HashIt( p->GetName(), 0xffff ) );
			Stat_WriteComment( fp, tstr );
		}

		long i = 0;
		BillingCustomerStruct* pCustomer;

		while( columnIDs[i] ){
			switch( columnIDs[i]){
				case 'indx':
								if( m_style == FORMAT_EXCEL) {
									initColumn();        
									writeNumberRightWithRLBorder( rownum+1, fp );
								} else		
									Stat_WriteNumberData( fp, rownum+1 );
								break;
					
				case 'reqb':	Stat_WriteHitsAndBytes( fp, p->files, totalRequests,  bytes, totalBytes ); break;
				case 'requ':	Stat_WriteHitData( fp, p->files, totalRequests ); break;
				case 'req1':	Stat_WriteNumberData( fp, p->GetFiles2(1) ); break;
				case 'req2':	Stat_WriteNumberData( fp, p->GetFiles2(2) ); break;
				case 'req3':	Stat_WriteNumberData( fp, p->GetFiles2(3) ); break;
				case 'Sess':	if( extras )
				case 'sess':	Stat_WriteNumberData( fp, p->visits ); break;
				case 'Visi':	if( extras )
				case 'visi':	Stat_WriteNumberData( fp, p->counter ); break;
				// Cached Requests, (for CLUSTERS)
				case 'cach':	Stat_WriteHitData( fp, p->counter, p->files ); break;
				case 'Page':	if( extras ) 
				case 'page':	Stat_WriteNumberData( fp, p->counter4 ); break;
		
				case '%pag':	// The percentage of the total pages
								Stat_WriteHitData( fp, p->counter4, totalCounters4 );
								break;
				case '%ses':	// The percentage of the total sesisons
								Stat_WriteHitData( fp, p->visits, totalVisits );
								break;

				case 'Pagl':	if( extras ) 
				case 'pagl':	Stat_WriteNumberData( fp, p->counter4 ); break;
				case 'Cnt4':	if( extras ) 
				case 'cnt4':	Stat_WriteNumberData( fp, p->counter4 ); break;
				// Averate Time per Session
				case 'Dura':	if( extras )
				case 'dura':	Stat_WriteTimeDuration( fp, p->visitTot, p->visits ); break;

				// Average Time per Hit/Play
				case 'Durv':	if( extras )
				case 'durv':	Stat_WriteTimeDuration( fp, p->visitTot, p->counter ); break;

				// Average Time per Visitor
				case 'Durp':	if( extras )
				case 'durp':	Stat_WriteTimeDuration( fp, p->visitTot, p->files ); break;

				case 'tota':	Stat_WriteTimeDuration( fp, p->visitTot, 1 ); break;
				case 'erro':	Stat_WriteNumberData( fp, p->errors ); break;
				case 'Fses':	Stat_WriteNumberData( fp, p->counter4 ); break;
				case 'Lses':	Stat_WriteNumberData( fp, p->GetVisitIn() ); break;
				case 'cost':	Stat_WriteCostData( fp, bytes ); break;
				case 'name':	Stat_WriteText( fp, 1, tdrgb, name ); break;
				case 'link':	Stat_WriteURL( fp, 1, tdrgb, name, name  ); break;
				case 'urls':	MakeURL( tempBuff2, thishost, name );
								Stat_WriteURL( fp, 1, tdrgb, tempBuff2, name );
								break;
				case 'sver':	// server error
								if( !VDptr->GetNumFailedRequests(p) )	// if there are no failed requests for current stat
								{
									Stat_WriteText( fp, 1, tdrgb, name );
									break;
								} // else fall through
				case 'blrf':	// broken link referal page
				case 'engi':	sprintf( tempBuff, "#%ld", n );
								ConvertParamtoURL( tempBuff, tempBuff2 );
								Stat_WriteLocalSameFileURL( fp, 1, -1, tempBuff2, name  );
								break;

				case 'urlp':	// Make direct call to 
								/*namelen = */mystrcpy( rname, name );				// name
								MakeURL( tempBuff2, thishost, name );
								if( MyPrefStruct.page_remotetitle )	HTTPURLGetTitle( tempBuff2, rname );
								Stat_WriteURL( fp, 1, tdrgb, tempBuff2, rname  );
								break;

				case 'prot':	if( *name == 'U' )		// unknown protocol has no link
									Stat_WriteText( fp, 1, -1, name );
								else {
									sprintf( tempBuff2, "%s%s", name, fileExtension );
									Stat_WriteURL( fp, 1, -1, tempBuff2, name  );
								}
								break;

				case 'clie':	if( p->length>0 ) {
									mystrcpy( rname, name );
									ReverseAddress( rname,rname );
								} else mystrcpy( rname, name );
								if( (MyPrefStruct.stat_clientStream && MyPrefStruct.stat_sessionHistory) ){
									if( p->sessionStat )
									{
										char *htmlname = sessionWriter->WriteSessions( vp, byStat, filename, title, thishost, rname, (short)n, list );
										Stat_WriteColURLjump( fp, 1, tdrgb, htmlname, rname );
									} else
										Stat_WriteText( fp, 1, tdrgb, rname  );
								}
								break;

				case 'user':	if( (MyPrefStruct.stat_userStream && MyPrefStruct.stat_sessionHistory) )
								{
									if( p->sessionStat )
									{
										char *htmlname = sessionWriter->WriteSessions( vp, byStat, filename, title, thishost, name, (short)n, list );
										Stat_WriteColURLjump( fp, 1, tdrgb, htmlname, name );
									} else
										Stat_WriteText( fp, 1, tdrgb, name  );
								}
								break;

				case 'naml':
								{
									if( !isdigit( *name ))
									{
										mystrcpy( rname, name );
										ReverseAddress( rname,rname );
									} 
									else
									{
										mystrcpy( rname, name );
									}

									sprintf( tempBuff, "#%ld", rownum );
									ConvertParamtoURL( tempBuff, tempBuff2 );
									Stat_WriteLocalSameFileURL( fp, 1, -1, tempBuff2, rname  );
								}
								break;

				case 'doma':	if( !isdigit( *name )) {
									mystrcpy( rname, name );
									ReverseAddress( rname,rname );
								} else mystrcpy( rname, name );
								Stat_WriteText( fp, 1, tdrgb, rname );
								break;

				case 'coun':	if( GetCIACountryURL( name, rname ) )
									Stat_WriteURL( fp, 1, -1, rname, name );
								else
									Stat_WriteText( fp, 1, -1, name );
								break;

				case 'week':	{
									// offset the current day index according to first day of week setting, so that
									// the specified day of week appears first within the weekly traffic reports
									size_t dayIndex=(n+MyPrefStruct.firstDayOfWeek)%7;
									long weekIDs[] = { SUNDAY_PAGE,MONDAY_PAGE,TUEDAY_PAGE,WEDDAY_PAGE,THUDAY_PAGE,FRIDAY_PAGE,SATDAY_PAGE,0 };

									if( VDptr->byWeekdays[dayIndex] )
									{
										DEF_ASSERT( VDptr->byWeekdays[dayIndex]->GetStatListNum()==24 );		// setup in InitVirtualDomain()
										sprintf( tempBuff2, "%s%s", FindReportFilenameStr(weekIDs[dayIndex]), fileExtension );
										name = GetWeekdayTranslatedByDayNum(dayIndex);
										Stat_WriteLocalDiffFileURL( fp, 1, -1, tempBuff2, name );
									}
									else
									{
										name = GetWeekdayTranslatedByDayNum(dayIndex);
										Stat_WriteText( fp, 1, -1, name );
									}
								}
								break;

				case 'hour':	sprintf( tempBuff2,"%-2s:00 - %-2s:59", name, name );
								Stat_WriteText( fp, 1, -1, tempBuff2 );
								break;

				case 'date':	strncpy( tempBuff2, GetWeekdayTranslated( p->lastday ), 3 );
								tempBuff2[3] = ' ';
								DaysDateToStringTranslated( p->lastday, tempBuff2+4, 16, -1, '/', TRUE, TRUE );
								//sprintf( tempBuff2+3, " %s", name );
								//strncpy( tempBuff2, GetWeekdayTranslated( p->lastday ), 3 );
								//sprintf( tempBuff2+3, " %s", name );
								Stat_WriteText( fp, 1, -1, tempBuff2 );
								break;

				case 'time':	strncpy( tempBuff2, GetWeekdayTranslated( p->lastday ), 3 );
								tempBuff2[3] = ' ';
								DaysDateToStringTranslated( p->lastday, tempBuff2+4, 16, -1, '/', TRUE, TRUE );
								strcat( tempBuff2, ", " );
								strcat( tempBuff2,lastTimeStr );
								Stat_WriteText( fp, 1, -1, tempBuff2 );
								break;

				case 'mont':	strncpy( tempBuff2, GetMonthTranslated( p->lastday ), 4 );
								sprintf( tempBuff2+4,"%s", name+4 );
								Stat_WriteText( fp, 1, -1, tempBuff2 );
								break;

				case 'addp':	sprintf( tempBuff2,"%ld%%", p->counter4*100/p->files );
								Stat_WriteTextRight( fp, 1, -1, tempBuff2 );
								break;

				case 'addc':	sprintf( tempBuff2,"$%.02f", p->counter4/(double)p->files );
								Stat_WriteTextRight( fp, 1, -1, tempBuff2 );
								break;
				case 'brok':	Stat_WriteHitData( fp, p->counter4, totalCounters4);
								break;
				case 'reqf':	Stat_WriteHitData( fp, p->errors,totalErrors);
								break;

				// ------------ Clip Column Details ---------------------------------------------------------------------------

				// Clip total Length on server (seconds)
				case 'clen':	Stat_WriteTimeDuration( fp, p->clipData->m_file_length, 1 );	break;
				// Clip Size (bytes)
				case 'csiz':	Stat_WriteBytes( fp, p->clipData->m_file_size );			break;
				// Clip Length Played/Sent(seconds)
				case 'catv':	Stat_WriteTimeDuration( fp,(p->clipData->m_played_length), p->files );
								sumColumns[i] += p->clipData->m_played_length;
								break;
				// Average Percentage Viewed
				case 'caph':	// Heard
				case 'capv':	if ( p->clipData->m_file_length )
									Stat_WritePercent( fp, (100*((p->clipData->m_played_length/(double)p->files))/p->clipData->m_file_length) );
								else
									Stat_WritePercent( fp, 0 /*(p->bytes/p->files)/p->clipData->file_size*/ );
								break;
				// Total Packets Sent
				case 'cptx':	Stat_WriteHitData( fp, p->GetPacketsSent(), byStat->GetPacketsSent() );		break;
				// Total Packets Recieved
				case 'cprx':	Stat_WriteHitData( fp, p->GetPacketsRec(), byStat->GetPacketsRec() );		break;

				// Average Packets lost & percent
				case 'capl':	Stat_WritePercent( fp, p->GetPacketLossPerc() );
								sumColumns[i] += p->GetPacketsLost();
								break;
				// Average Transfer Rate		(TranslateID(LABEL_KBPS))
				case 'catr':	char	numStr[32];
								FormatLongNum( (p->clipData->m_meanbps/1000.0), numStr );
								sprintf( tempBuff2, "%s%s", numStr, TranslateID(LABEL_KBPS) );
								Stat_WriteTextRight( fp, 1, -1, tempBuff2 );
								break;
				// Average Percent of clip streamed based on bytes transfered
				case 'caps':	Stat_WritePercent( fp, 100 * (p->bytes / (double)(p->clipData->m_file_size * p->files) ) );		break;
				// Expected Total MB to be transfered
				case 'cett':	Stat_WriteBytes( fp, (p->clipData->m_file_size * p->files) );	break;
				// Actual Total MB streamed (that was transfered)
				case 'catt':	Stat_WriteBytes( fp, (p->bytes) );					break;
				// Clip Access Transfer Quality
				case 'catq':	Stat_WritePercent( fp, p->clipData->m_meanquality );		break;
				// Clip Ave Seconds Buffered
				case 'cbuf':	Stat_WriteNumberData( fp, p->clipData->m_meanbufcount );		break;
				// Clip Average Forwards 
				case 'caff':	Stat_WriteNumberData( fp, p->clipData->m_ff_count/p->visits );	break;
				// Clip Average Rewinds
				case 'carw':	Stat_WriteNumberData( fp, p->clipData->m_rw_count/p->visits );	break;
				// Clip Total Forwards 
				case 'ctff':	Stat_WriteNumberData( fp, p->clipData->m_ff_count );	break;
				// Clip Total Rewinds
				case 'ctrw':	Stat_WriteNumberData( fp, p->clipData->m_rw_count );	break;
				// Clip Most Common Error
				case 'cmce':	Stat_WriteNumberData( fp, p->GetMostCommonError() );	// ;
								break;
				// Clip Uninterrupted Plays
				case 'cunp':	Stat_WriteHitData( fp, p->clipData->m_uninterrupted_count, p->files );
								sumColumns[i] += p->clipData->m_uninterrupted_count;
								break;
				// Clip Max Concurrent Connections
				case 'cmax':	Stat_WriteNumberData( fp, p->clipData->m_max_concurrent );
								break;
				// Clip Most Common Error Codes ....
				case 'err1':	Stat_WriteNumberData( fp, p->GetMostErrorcodeHits(0) );	break;
				case 'err2':	Stat_WriteNumberData( fp, p->GetMostErrorcodeHits(1) );	break;
				case 'err3':	Stat_WriteNumberData( fp, p->GetMostErrorcodeHits(2) );	break;
				case 'err4':	Stat_WriteNumberData( fp, p->GetMostErrorcodeHits(3) );	break;
				case 'err5':	Stat_WriteNumberData( fp, p->GetMostErrorcodeHits(4) );	break;

				// *************************************************
				// Billing Columns.
				// *************************************************
				case 'bCus':	// Customer Name
					if (MyPrefStruct.billingSetup.bHeaderCustomerName)
					{
						// This field is ALWAYS called before the other billing columns.
						pCustomer = LocateCustomer(p->name);
						Stat_WriteText(fp, 1, -1, pCustomer->szCustomer);
					}
					break;
				case 'bCID':	// Customer ID
					if (MyPrefStruct.billingSetup.bHeaderCustomerID)
					{
						if (!pCustomer)
							pCustomer = LocateCustomer(p->name);
						Stat_WriteText(fp, 1, -1, pCustomer->szCustomerID);
						pCustomer = NULL;	// Just to ensure its re-searched next time through.
					}
					break;
				case 'bMBt':	// MB Transfered
					if (MyPrefStruct.billingSetup.bHeaderMBTransfered)
					{
						Stat_WriteNumberData(fp, p->bytes);
					}
					break;
				case 'bMBa':	// MB Allowance
					if (MyPrefStruct.billingSetup.bHeaderMBAllowance)
					{
						Stat_WriteNumberData(fp, p->visits);
					}
					break;
				case 'bExc':	// Excess Charge
					if (MyPrefStruct.billingSetup.bHeaderExcessCharges)
					{
						char	sz[32];
						sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)p->counter4)/100.0);
						Stat_WriteTextRight(fp, 1, -1, sz);
					}
					break;
				case 'bTot':	// Total Charge
					if (MyPrefStruct.billingSetup.bHeaderTotalCharges)
					{
						char	sz[32];
						sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)p->counter)/100.0);
						Stat_WriteTextRight(fp, 1, -1, sz);
					}
					break;
				// *************************************************
				// End Billing Columns.
				// *************************************************
			}
			i++;
		}
		rownum++;
		Stat_WriteRowEnd( fp );
	} // end for
	

	// *****************************************************************
	// Do we need to show the Headers again before the Substotal, others,
	// Average, Total?
	// *****************************************************************
	if( MyPrefStruct.footer_label && !MyPrefStruct.footer_label_trailing)
	{
		Stat_WriteRowStart( fp, RGBtableHeaders, colspan );
		for( int n=0; n<cols; ++n )
		{
			Stat_WriteReportHeaders( fp, columnIDs[n], typeID, extras, heading );
		}
		Stat_WriteRowEnd( fp );
	}


	if( typeID == 'tope' || typeID == 'topx' || typeID == 'kvis' || typeID == 'kpag' || typeID == 'kpaf' )
	{
		//totalRequests = sumRequests;
		//totalErrors = sumErrors;
		dont_do_subtotals = TRUE;
		dont_do_others = TRUE;
	}
	else
	{
		dont_do_subtotals = FALSE;
		dont_do_others = FALSE;
	}


	if( m_style != FORMAT_EXCEL )
	{
		/* show SUBTOTALS stat as well */
		if( num > list && !dont_do_subtotals ){
			Stat_WriteRowStart( fp, RGBtableOthers, colspan );

			long i = 0;
			while( columnIDs[i] ){
				switch( columnIDs[i]){
					case '%pag':	
									Stat_WriteHitData( fp, sumCounters4/num, totalCounters4 );
									break;
					case '%ses':	
									Stat_WriteReqData( fp, sumVisits/num, ((double)sumVisits/(double)num)*100.0/(double)totalVisits);
									break;

					case 'indx':	Stat_WriteBottomHeader( fp, " ", 1 ); break;
					case 'reqb':	Stat_WriteHitsAndBytes( fp, (long)(sumRequests), totalRequests, sumBytes, totalBytes ); break;
					case 'requ':	Stat_WriteHitData( fp, (long)(sumRequests), totalRequests ); break;
					case 'req1':	Stat_WriteNumberData( fp, (long)(sumReq1) );	break;
					case 'req2':	Stat_WriteNumberData( fp, (long)(sumReq2) );	break;
					case 'req3':	Stat_WriteNumberData( fp, (long)(sumReq3) );	break;
					case 'Sess':	if( extras)
					case 'sess':	Stat_WriteNumberData( fp, (long)(sumVisits) );	break;
					case 'Visi':	if( extras)
					case 'visi':	Stat_WriteNumberData( fp, (long)(sumCounters) );	break;
					case 'cach':	Stat_WriteReqData( fp, (long)(sumCounters), sumCounters*100/(double)sumRequests );	break;
					case 'Page':	if( extras)
					case 'page':	Stat_WriteNumberData( fp, sumCounters4 );	break;
					case 'Pagl':	if( extras)
					case 'pagl':	Stat_WriteNumberData( fp, sumCounters4 );	break;
					case 'Cnt4':	if( extras)
					case 'cnt4':	Stat_WriteNumberData( fp, sumCounters4 );	break;
					case 'Dura':	if( extras)
					case 'dura':	Stat_WriteTimeDuration( fp, sumTime, 1 );	break;
					case 'tota':	Stat_WriteTimeDuration( fp, sumTime, 1 );	break;
					case 'erro':	Stat_WriteNumberData( fp, sumErrors );	break;
					case 'addp':	Stat_WriteFloatData( fp, sumCounters4*100/(double)sumRequests );	break;
					case 'addc':	sprintf( tempBuff2, "$%.2f", sumCounters4/(double)sumRequests ); Stat_WriteTextRight( fp, 1, -1, tempBuff2 );	break;
					case 'cost':	Stat_WriteCostData( fp, sumBytes );	break;
					case 'Fses':	Stat_WriteNumberData( fp, sumCounters4 ); break;
					case 'Lses':	Stat_WriteNumberData( fp, sumVisitIn ); break;
					case 'brok':	Stat_WriteHitData( fp, (long)(sumCounters4), totalCounters4 ); break;
					case 'reqf':	Stat_WriteHitData( fp, (long)(sumErrors), totalErrors ); break;
					case 'catt':	Stat_WriteBytes( fp, sumBytes );			break;
					case 'capl':	Stat_WritePercent( fp, (100.0*sumColumns[i])/(double)byStat->GetPacketsSent() );		break;
					case 'catv':	Stat_WriteTimeDuration( fp, sumColumns[i], sumRequests );		break;
					case 'catq':	Stat_WriteBottomHeader( fp, TranslateID(LABEL_NA), 1 );		break;
					case 'cunp':	Stat_WriteHitData( fp, sumColumns[i], sumRequests );	break;
					default:		Stat_WriteBottomHeader( fp, TranslateID(LABEL_SUBTOTALS), 1 );break;

					// *************************************************
					// Billing Columns.
					// *************************************************
					case 'bCus':	// Customer Name
						if (MyPrefStruct.billingSetup.bHeaderCustomerName)
						{
							Stat_WriteBottomHeader(fp, TranslateID(LABEL_SUBTOTALS), 1 );
						}
						break;
					case 'bCID':	// Customer ID
						if (MyPrefStruct.billingSetup.bHeaderCustomerID)
						{
							Stat_WriteText(fp, 1, -1, "-");
						}
						break;
					case 'bMBt':	// MB Transfered
						if (MyPrefStruct.billingSetup.bHeaderMBTransfered)
						{
							Stat_WriteNumberData(fp, sumBytes);
						}
						break;
					case 'bMBa':	// MB Allowance
						if (MyPrefStruct.billingSetup.bHeaderMBAllowance)
						{
							Stat_WriteNumberData(fp, sumVisits);
						}
						break;
					case 'bExc':	// Excess Charge
						if (MyPrefStruct.billingSetup.bHeaderExcessCharges)
						{
							char	sz[32];
							sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)sumCounters4)/100.0);
							Stat_WriteTextRight(fp, 1, -1, sz);
						}
						break;
					case 'bTot':	// Total Charge
						if (MyPrefStruct.billingSetup.bHeaderTotalCharges)
						{
							char	sz[32];
							sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)sumCounters)/100.0);
							Stat_WriteTextRight(fp, 1, -1, sz);
						}
						break;
					// *************************************************
					// End Billing Columns.
					// *************************************************
				}
				i++;
			}
			Stat_WriteRowEnd( fp );
		}

		/* show OTHERS stat as well */
		if( num > list && !dont_do_others ){
			Stat_WriteRowStart( fp, RGBtableOthers, colspan );
			long i = 0;
			while( columnIDs[i] ){
				switch( columnIDs[i]){
					case '%ses':
									Stat_WriteReqData( fp, totalVisits, 100.0);
									break;
					case '%pag':	
									Stat_WriteReqData( fp, totalCounters4, 100.0);
									break;

					case 'indx':	Stat_WriteNumberData( fp, num-list );		break;
					case 'reqb':	Stat_WriteHitsAndBytes( fp, (long)(totalRequests-sumRequests), totalRequests, totalBytes-sumBytes, totalBytes ); break;
					case 'requ':	Stat_WriteHitData( fp, (long)(totalRequests-sumRequests), totalRequests );break;
					case 'Sess':	if( extras)
					case 'sess':	Stat_WriteNumberData( fp, (long)(totalVisits-sumVisits) );break;
					case 'Visi':	if( extras)
					case 'visi':	Stat_WriteNumberData( fp, (long)(totalCounters-sumCounters) );break;
					//case 'cach':	Stat_WriteReqData( fp, (long)(totalCounters-sumCounters)*100/sumRequests, ( ) );break;
					case 'Page':	if( extras)
					case 'page':	Stat_WriteNumberData( fp, totalCounters4-sumCounters4 );	break;
					case 'Pagl':	if( extras)
					case 'pagl':	Stat_WriteNumberData( fp, totalCounters4-sumCounters4 );	break;
					case 'Cnt4':	if( extras)
					case 'cnt4':	Stat_WriteNumberData( fp, totalCounters4-sumCounters4 );	break;
					case 'Dura':	if( extras)
					case 'dura':	Stat_WriteTimeDuration( fp, totalTime-sumTime, 1 );	break;
					case 'tota':	Stat_WriteTimeDuration( fp, totalTime-sumTime, 1 );	break;
					case 'erro':	Stat_WriteNumberData( fp, totalErrors-sumErrors );	break;
					case 'cost':	Stat_WriteCostData( fp, totalBytes-sumBytes ); break;
					case 'brok':	Stat_WriteHitData( fp, (long)(totalCounters4-sumCounters4), totalCounters4 );break;
					case 'reqf':	Stat_WriteHitData( fp, (long)(totalErrors-sumErrors), totalErrors );break;
					case 'catt':	Stat_WriteBytes( fp, totalBytes-sumBytes );			break;
					case 'capl':	Stat_WritePercent( fp, 100.0*(byStat->GetPacketsLost()-sumColumns[i])/(double)byStat->GetPacketsSent() );		break;
					default:		Stat_WriteBottomHeader( fp, TranslateID(LABEL_OTHERS), 1 ); break;

					// *************************************************
					// Billing Columns.
					// *************************************************
					case 'bCus':	// Customer Name
						if (MyPrefStruct.billingSetup.bHeaderCustomerName)
						{
							Stat_WriteBottomHeader( fp, TranslateID(LABEL_OTHERS), 1 );
						}
						break;
					case 'bCID':	// Customer ID
						if (MyPrefStruct.billingSetup.bHeaderCustomerID)
						{
							Stat_WriteText(fp, 1, -1, "-");
						}
						break;
					case 'bMBt':	// MB Transfered
						if (MyPrefStruct.billingSetup.bHeaderMBTransfered)
						{
							Stat_WriteNumberData(fp, totalBytes-sumBytes);
						}
						break;
					case 'bMBa':	// MB Allowance
						if (MyPrefStruct.billingSetup.bHeaderMBAllowance)
						{
							Stat_WriteNumberData(fp, totalVisits-sumVisits);
						}
						break;
					case 'bExc':	// Excess Charge
						if (MyPrefStruct.billingSetup.bHeaderExcessCharges)
						{
							char	sz[32];
							sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)totalCounters4-sumCounters4)/100.0);
							Stat_WriteTextRight(fp, 1, -1, sz);
						}
						break;
					case 'bTot':	// Total Charge
						if (MyPrefStruct.billingSetup.bHeaderTotalCharges)
						{
							char	sz[32];
							sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)totalCounters-sumCounters)/100.0);
							Stat_WriteTextRight(fp, 1, -1, sz);
						}
						break;
					// *************************************************
					// End Billing Columns.
					// *************************************************
				}
				i++;
			}
			Stat_WriteRowEnd( fp );
		}
	}

	if( list>1 && !dont_do_average ) {
		/* show Averages */
		Stat_WriteRowStart( fp, RGBtableAvg, colspan );
		if( m_style == FORMAT_EXCEL )
			cellType = AVERAGE;
		long i = 0;
		while( columnIDs[i] ){
			switch( columnIDs[i]){
				case 'indx':
								if( m_style == FORMAT_EXCEL)
								{
									initColumn(); 
									writeTextRightWithRLBorder( "", fp );
								}
								else
									Stat_WriteBottomHeader( fp, " ", 1 );
								break;
				case 'reqb':	Stat_WriteHitsAndBytes( fp, (long)totalRequests/num, totalRequests, totalBytes/num, totalBytes );	break;
				case 'requ':	Stat_WriteHitData( fp, (long)totalRequests/num, totalRequests );	break;
				case 'req1':	Stat_WriteNumberData( fp, (long)(sumReq1/num) );	break;
				case 'req2':	Stat_WriteNumberData( fp, (long)(sumReq2/num) );	break;
				case 'req3':	Stat_WriteNumberData( fp, (long)(sumReq3/num) );	break;
				case 'Sess':	if( extras)
				case 'sess':	Stat_WriteNumberData( fp, (long)totalVisits/num );break;
				case 'Visi':	if( extras)
				case 'visi':	Stat_WriteNumberData(fp,(long)totalCounters/num);break;
				// Cache Hits
				case 'cach':	Stat_WriteReqData(fp,(long)totalCounters/num, (totalCounters/totFile) );break;
				case 'Page':	if( extras)
				case 'page':	Stat_WriteNumberData( fp, (long)totalCounters4/num );break;

				case '%pag':	// The overall average.
								Stat_WriteReqData( fp, sumCounters4/num, ((double)sumCounters4/(double)num)*100.0/(double)totalCounters4);
								break;
				case '%ses':	// The overall average.
								Stat_WriteReqData( fp, sumVisits/num, ((double)sumVisits/(double)num)*100.0/(double)totalVisits);
								break;
				
				case 'Pagl':	if( extras)
				case 'pagl':	Stat_WriteNumberData( fp, (long)totalCounters4/num );break;
				case 'Cnt4':	if( extras)
				case 'cnt4':	Stat_WriteNumberData( fp, (long)sumCounters4/num );break;
				case 'Dura':	if( extras)
				case 'dura':	if( m_style == FORMAT_EXCEL)
									Stat_WriteTimeDuration( fp, sumTime, totalVisits ); //not include others
								else 
									Stat_WriteTimeDuration( fp, totalTime, totalVisits );
								break;
				case 'tota':
								if( m_style == FORMAT_EXCEL)
									Stat_WriteTimeDuration( fp, sumTime, totalVisits ); //not include others
								else 
									Stat_WriteTimeDuration( fp, totalTime, totalVisits );
								break;
				case 'erro':	Stat_WriteNumberData( fp, (long)totalErrors/num );break;
				case 'addp':	Stat_WriteFloatData( fp, (sumCounters4*100/sumRequests)/num );	break;
				case 'addc':	sprintf( tempBuff2, "$%.2f", (sumCounters4/(double)sumRequests)/num ); Stat_WriteTextRight( fp, 1, -1, tempBuff2 );	break;
				case 'cost':	Stat_WriteCostData( fp, (totalBytes/num) );break;
				case 'Fses':	Stat_WriteNumberData( fp, (long)sumCounters4/num ); break;
				case 'Lses':	Stat_WriteNumberData( fp, sumVisitIn/num ); break;
				case 'brok':	Stat_WriteReqData( fp, (long)totalCounters4/num,(double)(totalCounters4/totCounters4)/num  );	break;
				case 'reqf':	Stat_WriteReqData( fp, (long)totalErrors/num,(double)(totalErrors/totError)/num  );	break;
				case 'catt':	Stat_WriteBytes( fp, totalBytes/num );			break;
				case 'capl':	Stat_WritePercent( fp, 100.0*(sumColumns[i]/num)/(double)byStat->GetPacketsSent() );		break;
				case 'cunp':	Stat_WriteHitData( fp, sumColumns[i]/num, totalRequests );	break;
				default:		Stat_WriteBottomHeader( fp, TranslateID(LABEL_AVERAGE), 1 ); break;

				// *************************************************
				// Billing Columns.
				// *************************************************
				case 'bCus':	// Customer Name
					if (MyPrefStruct.billingSetup.bHeaderCustomerName)
					{
						Stat_WriteBottomHeader( fp, TranslateID(LABEL_AVERAGE), 1 );
					}
					break;
				case 'bCID':	// Customer ID
					if (MyPrefStruct.billingSetup.bHeaderCustomerID)
					{
						Stat_WriteText(fp, 1, -1, "-");
					}
					break;
				case 'bMBt':	// MB Transfered
					if (MyPrefStruct.billingSetup.bHeaderMBTransfered)
					{
						Stat_WriteNumberData(fp, totalBytes/num);
					}
					break;
				case 'bMBa':	// MB Allowance
					if (MyPrefStruct.billingSetup.bHeaderMBAllowance)
					{
						Stat_WriteNumberData(fp, totalVisits/num);
					}
					break;
				case 'bExc':	// Excess Charge
					if (MyPrefStruct.billingSetup.bHeaderExcessCharges)
					{
						char	sz[32];
						sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)totalCounters4/num)/100.0);
						Stat_WriteTextRight(fp, 1, -1, sz);
					}
					break;
				case 'bTot':	// Total Charge
					if (MyPrefStruct.billingSetup.bHeaderTotalCharges)
					{
						char	sz[32];
						sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)totalCounters/num)/100.0);
						Stat_WriteTextRight(fp, 1, -1, sz);
					}
					break;
				// *************************************************
				// End Billing Columns.
				// *************************************************
			}

			i++;
		}
		if( m_style == FORMAT_EXCEL)
			cellType = DATA;
		Stat_WriteRowEnd( fp );
		
	}	//if( typeID != 17 ) {
	
	/* show recent Totals */
	if( strstr( title, TranslateID(LABEL_RECENTTOTALS) ) ){
		Stat_WriteRowStart( fp, RGBtableTotals, colspan );
		Stat_WriteHitsAndBytes( fp, (long)sumRequests,totalRequests, sumBytes,totalBytes );
		if( MyPrefStruct.stat_sessionHistory ){
			Stat_WriteNumberData(fp, sumVisits );
			Stat_WriteNumberData(fp, sumCounters );
		}
		Stat_WriteNumberData(fp, sumCounters4 );
		Stat_WriteNumberData(fp, totalErrors );
		if( MyPrefStruct.dollarpermeg ) Stat_WriteCostData( fp, sumRequests );
		Stat_WriteHeader( fp, TranslateID(LABEL_RECENT), 1 );
		Stat_WriteRowEnd( fp );
	}

	/* show Totals */
	if( !dont_do_totals )
	{
		Stat_WriteRowStart( fp, RGBtableTotals, colspan );
		if( m_style == FORMAT_EXCEL)
			cellType = SUM;
		long i = 0;
		while( columnIDs[i] ){
			switch( columnIDs[i]){
				case 'indx':	if( m_style == FORMAT_EXCEL){
									initColumn(); 
									writeTextRightWithRLBorder( "", fp );
								} else 
								{
									Stat_WriteNumberData( fp, num );
								}
								break;
				case 'reqb':	Stat_WriteHitsAndBytes( fp, (long)totalRequests,totalRequests, totalBytes,totalBytes ); break;
				case 'requ':	Stat_WriteReqData( fp, (long)totalRequests,(double)(totalRequests/totFile) ); break;
				case 'req1':	Stat_WriteNumberData( fp, (long)(sumReq1) );	break;
				case 'req2':	Stat_WriteNumberData( fp, (long)(sumReq2) );	break;
				case 'req3':	Stat_WriteNumberData( fp, (long)(sumReq3) );	break;
				case 'Sess':	if( extras)
				case 'sess':	Stat_WriteNumberData( fp, totalVisits ); break;

				case '%ses':
								// The overall total
								Stat_WriteReqData( fp, totalVisits, 100.0);
								break;
				case '%pag':	
								// The overall total
								Stat_WriteReqData( fp, totalCounters4, 100.0);
								break;

				case 'Visi':	if( extras)
				case 'visi':	Stat_WriteNumberData(fp, totalCounters ); break;
				case 'cach':	Stat_WriteReqData(fp, totalCounters, (totalCounters/totFile) ); break;
				case 'Page':	if( extras)
				case 'page':	Stat_WriteNumberData( fp, totalCounters4 );	 break;
				case 'Pagl':	if( extras)
				case 'pagl':	Stat_WriteNumberData( fp, totalCounters4 );	 break;
				case 'Cnt4':	if( extras)
				case 'cnt4':	Stat_WriteNumberData( fp, sumCounters4 ); break;
				case 'Dura':	if( extras)
				case 'dura':
								if( m_style == FORMAT_EXCEL)
									Stat_WriteTimeDuration( fp, sumTime, 1); //not include others
								else 
									Stat_WriteTimeDuration( fp, totalTime, 1 ); 
								break;
				case 'tota':
								if( m_style == FORMAT_EXCEL)
									Stat_WriteTimeDuration( fp, sumTime, 1); //not include others
								else 
									Stat_WriteTimeDuration( fp, totalTime, 1 );
								break;
				case 'erro':	Stat_WriteFractionData( fp, totalErrors, VDptr->totalFailedRequests ); break;
				case 'addp':	Stat_WriteFloatData( fp, sumCounters4*100/sumRequests );	break;
				case 'addc':	sprintf( tempBuff2, "$%.2f", sumCounters4/(double)sumRequests ); Stat_WriteTextRight( fp, 1, -1, tempBuff2 );	break;
				case 'cost':	Stat_WriteCostData( fp, totalBytes ); break;
				case 'Fses':	Stat_WriteNumberData( fp, sumCounters4 ); break;
				case 'Lses':	Stat_WriteNumberData( fp, sumVisitIn ); break;
				case 'brok':	Stat_WriteHitData( fp, (long)totalCounters4,totalCounters4 ); break;
				case 'reqf':	Stat_WriteHitData( fp, (long)totalErrors,totalErrors ); break;
				case 'catt':	Stat_WriteBytes( fp, totalBytes );			break;
				case 'capl':	Stat_WritePercent( fp, (double)byStat->GetPacketLossPerc() );		break;
				case 'cunp':	Stat_WriteHitData( fp, sumColumns[i], totalRequests );	break;
				default: Stat_WriteBottomHeader( fp, TranslateID(LABEL_TOTALS), 1 ); break;

				// *************************************************
				// Billing Columns.
				// *************************************************
				case 'bCus':	// Customer Name
					if (MyPrefStruct.billingSetup.bHeaderCustomerName)
					{
						Stat_WriteBottomHeader( fp, TranslateID(LABEL_TOTALS), 1 );
					}
					break;
				case 'bCID':	// Customer ID
					if (MyPrefStruct.billingSetup.bHeaderCustomerID)
					{
						Stat_WriteText(fp, 1, -1, "-");
					}
					break;
				case 'bMBt':	// MB Transfered
					if (MyPrefStruct.billingSetup.bHeaderMBTransfered)
					{
						Stat_WriteNumberData(fp, totalBytes);
					}
					break;
				case 'bMBa':	// MB Allowance
					if (MyPrefStruct.billingSetup.bHeaderMBAllowance)
					{
						Stat_WriteNumberData(fp, totalVisits);
					}
					break;
				case 'bExc':	// Excess Charge
					if (MyPrefStruct.billingSetup.bHeaderExcessCharges)
					{
						char	sz[32];
						sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)totalCounters4)/100.0);
						Stat_WriteTextRight(fp, 1, -1, sz);
					}
					break;
				case 'bTot':	// Total Charge
					if (MyPrefStruct.billingSetup.bHeaderTotalCharges)
					{
						char	sz[32];
						sprintf(sz,"%s%-.2f", MyPrefStruct.billingSetup.szCurrencySymbol, ((double)totalCounters)/100.0);
						Stat_WriteTextRight(fp, 1, -1, sz);
					}
					break;
				// *************************************************
				// End Billing Columns.
				// *************************************************
			}
			i++;
		}
		if( m_style == FORMAT_EXCEL)
			cellType = DATA;
		Stat_WriteRowEnd( fp );
	}

	// *****************************************************************
	// Do we need to show the Headers again after the Average & Total?
	// *****************************************************************
	if( MyPrefStruct.footer_label && MyPrefStruct.footer_label_trailing)
	{
		Stat_WriteRowStart( fp, RGBtableHeaders, colspan );
		for( int n=0; n<cols; ++n )
		{
			Stat_WriteReportHeaders( fp, columnIDs[n], typeID, extras, heading );
		}
		Stat_WriteRowEnd( fp );
	}

	Stat_WriteTableEnd( fp );
	Stat_InsertTable();

	if( MyPrefStruct.shadow )
		WritePageShadow( fp );

	// ******************************************************************************
	// If this report type suports HelpCards...
	// ******************************************************************************
	if( SupportHelpCards() )
	{
		// ******************************************************************************
		// If this report has HelpCards turned on...
		// ******************************************************************************
		if( ReportCommentOn( id ) )
		{
			// ******************************************************************************
			// Add a table DIRECTly following the previous one, however this one has no
			// border - producing the look of footnote-like data.
			// In this case we want it to be just the linking icon with no text.
			// ******************************************************************************
			Stat_WriteIconLink(fp, "Help.gif", filename, HELPCARD_EXTENSION, "Go the Help Card.", false);
			Stat_InsertTable();
		}
	}


	Stat_WriteSpace( fp, 1 );
	
}

























/***********************************************************

		SERVER SUMMARY FUNCTIONS

  *********************************************************/

void baseFile::SummaryBandwidth( FILE *fp, char *title, __int64 bandwidth, double num, long c )
{
	char		number[128], number2[128];
	double		bps;
	long		line=0, rgbcolor[2]={ 0xf0f0f0, 0xe0e0e0 };


	// bandwidth...
	SummaryTableSubTitle( fp, title );
	if( (bandwidth/(ONEMEGFLOAT)) < 10 ){
		SERVER_TABLEFLOAT( fp, bandwidth/1024.0, TranslateID(SUMM_TOTALKB), rgbcolor[(line)%2], line, c );
	} else {
		SERVER_TABLEFLOAT( fp, bandwidth/(ONEMEGFLOAT), TranslateID(SUMM_TOTALMB), rgbcolor[(line)%2], line, c );
	}
	if( MyPrefStruct.dollarpermeg ){
		sprintf( number, "$%.2f", (MyPrefStruct.dollarpermeg*(bandwidth/1000.0))/(ONEMEGFLOAT) );
		SERVER_TABLETEXT( fp, TranslateID(LABEL_TOTALCOST), number, rgbcolor[(line)%2], line, c );
	}
	if( ((bandwidth/(ONEMEGFLOAT))/num) < 1 ){
		SERVER_TABLEFLOAT( fp, (bandwidth/1024.0)/(double)num, TranslateID(SUMM_AVGDAILYKB), rgbcolor[(line)%2], line, c );
	} else {
		SERVER_TABLEFLOAT( fp, (bandwidth/(ONEMEGFLOAT))/(double)num, TranslateID(SUMM_AVGDAILYMB), rgbcolor[(line)%2], line, c );

	}

	bps = (bandwidth)/(num*3600*24/8);
	SERVER_TABLEFLOAT( fp, bps,  TranslateID(SUMM_AVGBITSPERSEC), rgbcolor[(line)%2], line, c );
	{
		char type[24]; double perc; long kilo = 1000;

		// Show the static bandwdith level where specified by MyPrefStruct.server_bandwidth
		if( MyPrefStruct.server_bandwidth ){
			sprintf (type, "%d %s", MyPrefStruct.server_bandwidth, TranslateID(LABEL_KBPS)); perc = bps/(MyPrefStruct.server_bandwidth*kilo);
		} else
		if( bps < 64*kilo ){
			sprintf (type, "64 %s", TranslateID(LABEL_KBPS)); perc = bps/(64*kilo);
			//mystrcpy( type, "64Kbps" ); perc = bps/(64*kilo);
		} else
		if( bps < 2*64*kilo ){
			sprintf (type, "128 %s", TranslateID(LABEL_KBPS)); perc = bps/(2*64*kilo);		// NOW dont fuck it up!! (Raul Says)
			//mystrcpy( type, "128Kbps" ); perc = bps/(2*64*kilo);
		} else
		if( bps < 4*64*kilo ){
			sprintf (type, "256 %s", TranslateID(LABEL_KBPS)); perc = bps/(4*64*kilo);
			//mystrcpy( type, "256Kbps" ); perc = bps/(4*64*kilo);
		} else
		if( bps < 6*64*kilo ){
			sprintf (type, "384 %s", TranslateID(LABEL_KBPS)); perc = bps/(6*64*kilo);
			//mystrcpy( type, "384Kbps" ); perc = bps/(6*64*kilo);
		} else
		if( bps < 8*64*kilo ){
			sprintf (type, "512 %s", TranslateID(LABEL_KBPS)); perc = bps/(8*64*kilo);
			//mystrcpy( type, "512Kbps" ); perc = bps/(8*64*kilo);
		} else
		if( bps < 12*64*kilo ){
			sprintf (type, "768 %s", TranslateID(LABEL_KBPS)); perc = bps/(12*64*kilo);
			//mystrcpy( type, "768Kbps" ); perc = bps/(12*64*kilo);
		} else
		if( bps < 24*64*kilo ){
			mystrcpy( type, "T1" ); perc = bps/(24*64*kilo);
		} else
		if( bps < 24*24*64*kilo ){
			mystrcpy( type, "T3" ); perc = bps/(24*24*64*kilo);
		} else
		if( bps < 155 * 1000000 ){
			mystrcpy( type, "OC3" ); perc = bps/(155 * 1000000);
		} else
		if( bps < 655 * 1000000 ){
			mystrcpy( type, "OC12" ); perc = bps/(655 * 1000000);
		}
		sprintf( number2, "%s %s", TranslateID(SUMM_PERCENTOF), type );		// was SUMM_PERCENT_OF but SUMM_PERCENTOF already in language files as {ss31}
		sprintf( number, "%.2f%%", perc*100 );
		SERVER_TABLETEXT( fp, number2, number, rgbcolor[(line)%2], line, c );
	}
}


#ifdef	DEF_FULLVERSION
#define	BASECOLOR 0x0A60a0		//10,80,160
#else
#define	BASECOLOR 0xd0003c		//228,0,57
#endif

inline void baseFile::SERVER_TABLEFLOAT( FILE *fp, double value, char *text, int color, long &line, long c )
{
	static char buf[32];
	FormatDoubleNum( value, buf );
	SummaryTableEntry( fp, text, buf, color, c );
	line++;
}

inline void baseFile::SERVER_TABLENUM( FILE *fp, long value, char *text, int color, long &line, long c ) // #define SERVER_TABLENUM( fp, value, text )
{
	static char buf[32];
	FormatLongNum( value, buf );
	SummaryTableEntry( fp, text, buf, color, c );
	line++;
}

inline void baseFile::SERVER_TABLETEXT( FILE *fp, char *text, char *number, int color, long &line, long c ) // #define SERVER_TABLETEXT( fp, text )
{
	SummaryTableEntry( fp, text, number, color, c );
	line++;
}

/* WriteReportLoadSummary - print summary of total day statistics */
void baseFile::WriteReportLoadSummary( VDinfoP VDptr, FILE *fp, long num, long tw, long c )
{
	char		number[256];
	long		tv=0, line=0, rgbcolor[2]={ 0xf0f0f0, 0xe0e0e0 }, totaltime, color, rv;

	//server load
	SummaryTableStart( fp, tw, c );
	color = BASECOLOR;

	totaltime = (VDptr->lastTime-VDptr->firstTime);
	if( !totaltime )
		totaltime = 1;

	SummaryTableSubTitle( fp, TranslateID( SUMM_TIMEPERIOD ) );
	{
		char timeStr[32];

		CTimetoString( totaltime, timeStr );
		sprintf( number, "%s %s", timeStr, TranslateID( TIME_MINS ) );
		SERVER_TABLETEXT( fp, TranslateID(SUMM_DURATION), number, rgbcolor[(line)%2], line, c );

		
		if( VDptr->totalDays < 2 ) 
			sprintf( number, "(%ld %s)", VDptr->totalDays, TranslateID( TIME_DAY ) );
		else
			sprintf( number, "(%ld %s)", VDptr->totalDays, TranslateID( TIME_DAYS ) );
		SERVER_TABLETEXT( fp, "", number, rgbcolor[(line)%2], line, c );
	}

	{
		char	firstStr[64],lastStr[64];
		if( VDptr ){
			DateTimeToStringTranslated( VDptr->firstTime, firstStr );
			DateTimeToStringTranslated( VDptr->lastTime, lastStr );
		} else {
			DateTimeToStringTranslated( allfirstTime, firstStr );
			DateTimeToStringTranslated( allendTime, lastStr );
		}

		mystrcpy(number,firstStr);
		SERVER_TABLETEXT( fp, TranslateID(SUMM_DATERANGEFROM), number, rgbcolor[(line)%2], line, c );
		mystrcpy(number,lastStr);
		SERVER_TABLETEXT( fp, TranslateID(SUMM_DATERANGETO), number, rgbcolor[(line)%2], line, c );
	}
	
	// Requests stuff...
	SummaryTableSubTitle( fp, TranslateID(SUMM_REQUESTS) );
	SERVER_TABLENUM( fp, VDptr->totalRequests, TranslateID(SUMM_TOTALREQS), rgbcolor[(line)%2], line, c );
	SERVER_TABLENUM( fp, VDptr->totalCachedHits, TranslateID(SUMM_TOTALCACHEDREQS), rgbcolor[(line)%2], line, c );
	SERVER_TABLENUM( fp, VDptr->totalFailedRequests, TranslateID(SUMM_TOTALFAILREQS), rgbcolor[(line)%2], line, c );
	SERVER_TABLENUM( fp, VDptr->badones, TranslateID(SUMM_INVALLOGENTRIES), rgbcolor[(line)%2], line, c );
	SERVER_TABLENUM( fp, VDptr->totalRequests/(totaltime/86400.0), TranslateID(SUMM_AVGDAILYREQS), rgbcolor[(line)%2], line, c );
	SERVER_TABLENUM( fp, VDptr->totalRequests/(totaltime/3600.0), TranslateID(SUMM_AVGREQSPERHR), rgbcolor[(line)%2], line, c );

	line = 0;
	// Visits stuff...
	SummaryTableSubTitle( fp, TranslateID(SUMM_SESSIONSINFO) );

	if( VDptr->byClient ){
		SERVER_TABLENUM( fp, VDptr->byClient->GetStatListTotalVisits(), TranslateID(SUMM_TOTALSESSIONS), rgbcolor[(line)%2], line, c );
		rv = CountRepeatVisits( VDptr->byClient );
		SERVER_TABLENUM( fp, VDptr->byClient->GetStatListNum(), TranslateID(SUMM_TOTALUNIQUEVISITORS), rgbcolor[(line)%2], line, c );
		SERVER_TABLENUM( fp, rv, TranslateID(SUMM_TOTALRPTVISITORS), rgbcolor[(line)%2], line, c );
		SERVER_TABLENUM( fp, VDptr->byClient->GetStatListNum() - rv, TranslateID(SUMM_TOTALONETIMEVISITORS), rgbcolor[(line)%2], line, c );

		tv = VDptr->byClient->GetStatListTotalVisits();
		if( !tv ) tv = 1;
		SERVER_TABLENUM( fp, tv/((totaltime)/86400.0), TranslateID(SUMM_AVGDAILYSESSIONS), rgbcolor[(line)%2], line, c );
		CTimetoString( VDptr->byClient->GetStatListTotalTime()/tv, number );
		SERVER_TABLETEXT( fp, TranslateID(SUMM_AVGSESSIONLENGTH), number, rgbcolor[(line)%2], line, c );

		sprintf( number, "%.2f", VDptr->byClient->GetStatListTotalCounters4()/(double)tv );
		SERVER_TABLETEXT( fp, TranslateID(SUMM_AVGPAGESPERSESSION), number, rgbcolor[(line)%2], line, c );

		sprintf( number, "%.2f", VDptr->totalRequests/(double)tv );
		SERVER_TABLETEXT( fp, TranslateID(SUMM_AVGREQSPERSESSION), number, rgbcolor[(line)%2], line, c );
	}
	
	SummaryTableFinish( fp );
}


/* WriteReportLoad2Summary - print summary of total day statistics */
void baseFile::WriteReportLoad2Summary( VDinfoP VDptr, FILE *fp, long num, long tw, long c )
{
	long		line=0, rgbcolor[2]={ 0xf0f0f0, 0xe0e0e0 }, totaltime;

	totaltime = (VDptr->lastTime-VDptr->firstTime);
	if( !totaltime ) totaltime = 1;

		//server load
	SummaryTableStart( fp, tw, c );

	SummaryTableSubTitle( fp, TranslateID(SUMM_PAGESINFO) );
	
	if( VDptr->byPages ){
		SERVER_TABLENUM( fp, VDptr->byPages->GetStatListTotalRequests(), TranslateID(SUMM_TOTALPAGES), rgbcolor[(line)%2], line, c );		// includes wrong urls/non existent pages attepted
		SERVER_TABLENUM( fp, VDptr->byPages->GetStatListTotalRequests()/VDptr->totalDays, TranslateID(SUMM_AVGPAGESPERDAY), rgbcolor[(line)%2], line, c );
	}

	if( VDptr->byDownload ){
		if( VDptr->byDownload->GetStatListTotalRequests() ){
			SERVER_TABLENUM( fp, VDptr->byDownload->GetStatListTotalRequests()-VDptr->byDownload->GetStatListTotalErrors(), TranslateID(SUMM_TOTALDLOADFILES), rgbcolor[(line)%2], line, c );
			SERVER_TABLEFLOAT( fp, (double)VDptr->byDownload->GetStatListTotalBytes()/(ONEMEGFLOAT), TranslateID(SUMM_TOTALDOWNLOADMB), rgbcolor[(line)%2], line, c );
		}
	}

	line = 0;

	if( MyPrefStruct.bandwidth ){
		// bandwidth...
		SummaryBandwidth( fp, TranslateID(SUMM_BANDWIDTHOUT), VDptr->totalBytes, (totaltime/86400.0), c );
		SummaryBandwidth( fp, TranslateID(SUMM_BANDWIDTHIN), VDptr->totalBytesIn, (totaltime/86400.0), c );
	}

	SummaryTableFinish( fp );
}


void baseFile::BeginSummaryTable( FILE *fp, long tw, char *txt, int c )
{
	html_count = 0;
	SummaryTableStart( fp, tw, c );
	if( c == 1 ) {
		SummaryTableTitle( fp, txt, BASECOLOR );
		SummaryTableRow( fp, 20 );
	}
	else {
		SummaryTableSmallTitle( fp, txt );
		SummaryTableRow( fp, 10 );
	}
	SummaryTableDataEnd( fp );
	SummaryTableRow( fp, 0 );
	SummaryTableStart( fp, tw*.80, c );
}

void baseFile::EndSummaryTable( FILE *fp )
{
	if( html_count%2==1)
		SummaryTableRowEnd( fp );
	SummaryTableFinish( fp );
	SummaryTableRowEnd( fp );
	SummaryTableFinish( fp );
}


short baseFile::AnyTrafficStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	return (( VDptr->Done.Hour && ANYSTAT(prefs->stat_hourly) ) ||
		( VDptr->Done.Hour && ANYSTAT(prefs->stat_hourlyHistory) ) ||
		( VDptr->Done.Date && ANYSTAT(prefs->stat_daily) ) ||
		( VDptr->Done.Date && ANYSTAT(prefs->stat_recentdaily) ) ||
		( VDptr->Done.Weekdays[0] && ANYSTAT(prefs->stat_weekly) ) ||
		( VDptr->Done.Weekdays[1] && ANYSTAT(prefs->stat_weekdays) ) ||
		( VDptr->Done.Weekdays[2] && ANYSTAT(prefs->stat_weekdays) ) ||
		( VDptr->Done.Weekdays[3] && ANYSTAT(prefs->stat_weekdays) ) ||
		( VDptr->Done.Weekdays[4] && ANYSTAT(prefs->stat_weekdays) ) ||
		( VDptr->Done.Weekdays[5] && ANYSTAT(prefs->stat_weekdays) ) ||
		( VDptr->Done.Weekdays[6] && ANYSTAT(prefs->stat_weekdays) ) ||
		( VDptr->Done.Weekdays[7] && ANYSTAT(prefs->stat_weekdays) ) ||
		( VDptr->Done.Month && ANYSTAT(prefs->stat_monthly) ));
}
short baseFile::AnyDiagnosticStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	return VDptr->Done.Errors   && ANYSTAT(prefs->stat_errors) ||
		   VDptr->Done.Errors   && ANYSTAT(prefs->stat_errorsHistory) ||
		   VDptr->Done.ErrorURL && ANYSTAT(prefs->stat_errorurl) ||
		   VDptr->Done.ErrorURL && ANYSTAT(prefs->stat_errorurlHistory) ||
		   (ANYSTAT(prefs->stat_brokenLinks) ) &&
		   VDptr->byBrokenLinkReferal->GetStatListNum();
}
short baseFile::AnyServerStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	if( VDptr->Done.File ){
		return (( VDptr->Done.Pages && ANYSTAT(prefs->stat_pages) ) ||
			( VDptr->Done.Pages && ANYSTAT(prefs->stat_pagesLeastVisited) ) ||
			( VDptr->Done.Pages && ANYSTAT(prefs->stat_pagesHistory) ) ||
			( VDptr->Done.Pages && ANYSTAT(prefs->stat_pagesfirst) ) ||
			( VDptr->Done.Pages && ANYSTAT(prefs->stat_pagesfirstHistory) ) ||
			( VDptr->Done.Pages && ANYSTAT(prefs->stat_pageslast) ) ||
			( VDptr->Done.Pages && ANYSTAT(prefs->stat_pageslastHistory) ) ||
			( VDptr->Done.Dir && ANYSTAT(prefs->stat_dir) ) ||
			( VDptr->Done.TopDir && ANYSTAT(prefs->stat_topdir) ) ||
			( VDptr->Done.TopDir && ANYSTAT(prefs->stat_topdirHistory) ) ||
			( VDptr->Done.File && ANYSTAT(prefs->stat_files) ) ||
			( VDptr->Done.MeanPath && ANYSTAT(prefs->stat_meanpath) ) ||
			( VDptr->Done.Groups && ANYSTAT(prefs->stat_groups) ) ||
			( VDptr->Done.Groups && ANYSTAT(prefs->stat_groupsHistory) ) ||
			( VDptr->Done.Download && ANYSTAT(prefs->stat_download) ) ||
			( VDptr->Done.Download && ANYSTAT(prefs->stat_downloadHistory) ) ||
			( VDptr->Done.Type && ANYSTAT(prefs->stat_type) )	||
			( VDptr->Done.Pages && ANYSTAT(prefs->stat_keypageroute)	
					&& VDptr->byPages->TestAnyMatches(FilterNonKeyPagesTo) ) ||
			( VDptr->Done.Pages && ANYSTAT(prefs->stat_keypageroutefrom)
					&& VDptr->byPages->TestAnyMatches(FilterNonKeyPagesFrom) )
			);
	}
	else
		return 0;
}
short baseFile::AnyDemographicStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	if( VDptr->Done.Client ) {
		return (( VDptr->Done.Client && ANYSTAT(prefs->stat_client) ) ||
			( VDptr->Done.Client && ANYSTAT(prefs->stat_clientStream) ) ||
			( VDptr->Done.Client && ANYSTAT(prefs->stat_clientHistory) ) ||
			( VDptr->Done.User && ANYSTAT(prefs->stat_user) ) ||
			( VDptr->Done.User && ANYSTAT(prefs->stat_userHistory) ) ||
			( VDptr->Done.SecondDomain && ANYSTAT(prefs->stat_seconddomain) ) ||

			( VDptr->Done.Domain && ANYSTAT(prefs->stat_country) ) ||
			( VDptr->Done.Regions && ANYSTAT(prefs->stat_regions) ) ||
			( VDptr->Done.Orgs && ANYSTAT(prefs->stat_orgs) ) ||
			( VDptr->Done.Client && ANYSTAT(prefs->stat_sessionScatter) ) ||
			( VDptr->Done.Client && ANYSTAT(prefs->stat_keyvisitor) )
			);
	}
	else
		return 0;
}

short baseFile::AnyReferralStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	if( VDptr->Done.Refer || VDptr->Done.SearchSite || VDptr->Done.SearchStr ){
		return (( VDptr->Done.Refer && ANYSTAT(prefs->stat_refer) ) ||
			( VDptr->Done.ReferSite && ANYSTAT(prefs->stat_refersite) ) ||
			( VDptr->Done.ReferSite && ANYSTAT(prefs->stat_refersiteHistory) ) ||
			( VDptr->Done.SearchSite && ANYSTAT(prefs->stat_searchsite) ) ||
			( VDptr->Done.SearchStr && ANYSTAT(prefs->stat_searchstr) ));
	}
	else
		return 0;
}
short baseFile::AnyStreamingMediaStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	if( VDptr->byMediaTypes->GetStatListNum() ||
		 VDptr->byMediaPlayers->GetStatListNum() ){
		return (( /*VDptr->Done.xxx &&*/ ANYSTAT(prefs->stat_mediatypes) ) ||
			( /*VDptr->Done.xxx &&*/ ANYSTAT(prefs->stat_video) ) ||
			( /*VDptr->Done.xxx &&*/ ANYSTAT(prefs->stat_audio) ) ||
			( /*VDptr->Done.xxx &&*/ ANYSTAT(prefs->stat_mplayers) ));
	}
	else 
		return 0;
}
short baseFile::AnySystemsStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	return (( VDptr->Done.Browser && ANYSTAT(prefs->stat_browsers) ) ||
		( VDptr->Done.Browser && ANYSTAT(prefs->stat_browserVSos) ) ||
		( 
			ANYSTAT(prefs->stat_unrecognizedagents) && 
			VDptr->byUnrecognizedAgents &&
			VDptr->byUnrecognizedAgents->TestAnyMatches()
		) ||
		( VDptr->Done.Robot && ANYSTAT(prefs->stat_robot) ) ||
		( VDptr->Done.Robot && ANYSTAT(prefs->stat_robotHistory) ) ||
		( VDptr->Done.OperSys && ANYSTAT(prefs->stat_opersys) ));
}
short baseFile::AnyAdvertisingStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	return VDptr->byAdvert->GetStatListNum() || VDptr->byAdCamp->GetStatListNum();
/*
	if( VDptr->byAdvert->GetStatListNum() || VDptr->byAdCamp->GetStatListNum() ){
	return (( ANYSTAT(prefs->stat_advert) ) ||
		( ANYSTAT(prefs->stat_advertHistory) ) ||
		( ANYSTAT(prefs->stat_advertcamp) ) ||
		( ANYSTAT(prefs->stat_advertcampHistory) ));
	}
	else
		return 0;
*/
}
short baseFile::AnyMarketingStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	if( VDptr->Done.Circulation || VDptr->Done.Loyalty || VDptr->Done.Timeon ){
		return (( VDptr->Done.Circulation && ANYSTAT(prefs->stat_circulation) ) ||
			( VDptr->Done.Loyalty && ANYSTAT(prefs->stat_loyalty) ) ||
			( VDptr->Done.Timeon && ANYSTAT(prefs->stat_timeon) ));
	}
	else
		return 0;
}

short baseFile::AnyClustersStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
	return ( ANYSTAT(prefs->stat_clusterServers) || ANYSTAT(prefs->stat_clusterServersHistory) ) &&
		   VDptr->byClusters->GetStatListNum();
}

short baseFile::AnyFirewallStatsTurnedOn( VDinfo *VDptr, App_config* prefs )
{
#ifdef DEF_APP_FIREWALL
	return (( VDptr->Done.SourceAddr && ANYSTAT(prefs->stat_sourceaddress) ) ||
		( VDptr->Done.Protocol && ANYSTAT(prefs->stat_protSummary) ) ||
		( VDptr->Done.HTTP && ANYSTAT(prefs->stat_protHTTP) ) ||
		( VDptr->Done.HTTPS && ANYSTAT(prefs->stat_protHTTPS) ) ||
		( VDptr->Done.Mail && ANYSTAT(prefs->stat_protMail) ) ||
		( VDptr->Done.FTP && ANYSTAT(prefs->stat_protFTP) ) ||
		( VDptr->Done.Telnet && ANYSTAT(prefs->stat_protTelnet) ) ||
		( VDptr->Done.DNS && ANYSTAT(prefs->stat_protDNS) ) ||
		( VDptr->Done.POP3 && ANYSTAT(prefs->stat_protPOP3) ) ||
		( VDptr->Done.RealAudio && ANYSTAT(prefs->stat_protReal) ) ||
		( VDptr->Done.Others && ANYSTAT(prefs->stat_protOthers) ));
#endif // DEF_APP_FIREWALL
	return 0;
}


void baseFile::WriteLogProcessingTimeInfo( VDinfoP VDptr, FILE *fp )
{
	char	name[1200];

	if( VDptr && fp ){
		long nowDays;

//		SummaryTableStart( fp, 0, 0 );
		SummaryTableHeader( fp, 0, 0 );
		SummaryTableRow( fp, 40 );
		//get current date
		nowDays = GetCurrentCTime();
		DateTimeToStringTranslated( nowDays, name);
		if( MyPrefStruct.write_timestat )
			Stat_WriteBold( fp, name );
		else
			Stat_WriteComment( fp, name );
		SummaryTableDataEnd( fp );

		SummaryTableRow( fp, 0 );
		if( VDptr->time2 > 0 ){
			sprintf( name, "%s: %02d:%02d:%02d.%d, %.0f %s, %.2f %s/%s",
				TranslateID( SUMM_LOGPROCESSTIME ),
				(int)VDptr->time2/(60*60),(int)(VDptr->time2/60)%60,
				(int)(VDptr->time2)%60,(int)(VDptr->time2*10)%10,
				60*(VDptr->totalRequests/VDptr->time2),
				TranslateID( SUMM_LOGLINESPERMIN ),
				60*((VDptr->totalInDataSize/VDptr->time2)/1048576.0),
				TranslateID( LABEL_MB ),
				TranslateID( TIME_MIN ) );
			if( MyPrefStruct.write_timestat )
				Stat_WriteLine( fp, name );
			else
				Stat_WriteComment( fp, name );
		}
		SummaryTableDataEnd( fp );
		SummaryTableRowEnd( fp );
		SummaryTableFinish( fp );

		WriteDebugComments( fp, VDptr );
	}
}


void baseFile::ProduceSummaryTitle( FILE *fp, int numberOfLogs, const char *domainName )
{
	char		txt[256];

	if( MyPrefStruct.head_title) 
	{
		if( MyPrefStruct.multivhosts == 0 && numberOfLogs >1 && VDnum == 0 ) 
		{
			if( strlen(MyPrefStruct.siteurl)>7 )
				sprintf( txt, "%s: %s", TranslateID( SUMM_SERVERLOADSTATS ), MyPrefStruct.siteurl );
			else
			if( *domainName )
				sprintf( txt, "%s: %s (%d log files)", TranslateID( SUMM_SERVERLOADSTATS ), domainName, numberOfLogs );
			else
				sprintf( txt, "%s: (%d log files)", TranslateID( SUMM_SERVERLOADSTATS ), numberOfLogs );
		} else
			sprintf( txt, "%s: %s", TranslateID( SUMM_SERVERLOADSTATS ), domainName );
	} else {
		strcpy( txt, TranslateID( SUMM_SERVERLOADSTATS ) );
	}

	DateFixFilename( txt, NULL );

	SetReportTitle( txt );
}




void baseFile::WriteGraphStat( VDinfoP VDptr, StatList *byStat , long id, long typeID, char *filename, char *title, char *header, short sort, short depth, long labelTransId/*=0*/ )
{
	StandardPlot aPlot;
	aPlot.DrawbyDataStyle( VDptr, byStat, id, typeID, filename, title, header, sort, depth, labelTransId );
}

void baseFile::WriteReportStat( VDinfoP VDptr, StatList *byStat, FILE *fp, char *filename, long id,
							    long typeID, char *title, char *header, char *imagename,
								short depth, short sort, short graph, short table )
{
	char	tempname[512];
	long	liststart = 0, list;
	long	graphLabelTransId(0);		// translate id of optional & overriding graph label 

	if( typeID && byStat )
		list = byStat->num;

	// ---------------------------------------------
	// This checks if the selected SORT option is valid or crap
	// If its crap, then default to requests.......... if in doubt, dont call then.
	if( ! IsThisSortValid( sort, id ) )
		sort = SORT_REQUESTS;

	// default sorting
	switch( typeID ){
		case 'mont':
		case 'days':	byStat->DoSort( SORT_LASTDAY, list );
						break;
		case 'week':
		case 'norm':
		case 'hr3d':
		case 'hour':
		case 'loya':	break;

		case 'tope':	list = CountValidFirst( byStat );
						if( list > depth && depth >1 )
							list = depth;
						byStat->DoSort( sort=SORT_COUNTER4, list );
						break;
		case 'topx':	list = CountValidLast( byStat );
						if( list > depth && depth >1 )
							list = depth;
						byStat->DoSort( sort=SORT_VISITIN, list );
						break;
		case 'mean':	graph = FALSE; break;
		case 'virt':	graph = TRUE; break;

		case 'rday':	if( list > 31 ){
							liststart = list-31;
							if( liststart <0 ) liststart = 0;
							list = 31;
						}
						break;

		case 'kpaf':	// "Routes from Key Pages"	-
		case 'kpag':	// "Routes to Key Pages"	-- All these three reports use post-filters.
		case 'kvis':	// "Key Visitors"			-
						// Restrict the number of items to report on.
						if( list > depth && depth >1 )
							list = depth;
						// *************************************************
						// We need to sort the whole list as we do not know
						// where the top 'list' items will be due to the 
						// post-build filtering.
						// *************************************************
						byStat->DoSort(sort, -1, TOP_SORT);
						break;
		
		case 'pagl':	if( list > depth && depth >1 ) list = depth;
						if( sort != SORT_NONE )
							byStat->DoSort( sort, list, BOTTOM_SORT );
						break;

		case 'pcbl':	// Restrict the number of items to report on - as usual
						if( list > depth && depth >1 ) list = depth;
						// Sorting is hard coded (sort by broken links then by num broken link 
						// requests) due to poor sorting implementation that doesn't allow 
						// sorting on any arbitrary table column.  The default of sorting by
						// requests is not suitable for this report.
						byStat->DoSort( SORT_COUNTER4THENERRORS, list );
						sort=SORT_COUNTER4;	// for graphing below
						graphLabelTransId=LABEL_BROKENLINKS;	// override default graph label
						break;

		case 'bill':
						// Do the PostCalculations on the byBilling StatList.
						CalculateBillingData(byStat, &MyPrefStruct.billingSetup, &MyPrefStruct.billingCharges);

						if( list > depth && depth >1 ) list = depth;
						if( sort != SORT_NONE )
							byStat->DoSort( sort, list );
						break;

		default:		if( list > depth && depth >1 ) list = depth;
						if( sort != SORT_NONE )
							byStat->DoSort( sort, list );
						break;
	}


	sprintf( tempname, "%s  %s", VDptr->domainName, title );
	WritePageTitle( VDptr, fp, tempname );				//write out custom header

	WriteDebugComments( fp, VDptr );

	WriteDemoBanner( VDptr, fp );

	ShowingGraph( graph!=0 ); // For PDF

	if( graph ) 
	{
		WriteGraphStat( VDptr, byStat, id, typeID, filename, title, header, sort, depth, graphLabelTransId );
		if( m_style != FORMAT_HTML && id == VHOST_PAGE ) 
		{
			htmlFile format;
			format.WritePageImageLink( VDptr, fp, imagename, header, graph, sort );
		}
		else
		{
			WritePageImageLink( VDptr, fp, imagename, header, graph, sort );
		}

		// ******************************************************************************
		// If this report type suports HelpCards...
		// ******************************************************************************
		if	(	SupportHelpCards()
			&&	ReportCommentOn( id ) 
			)
		{
			// ******************************************************************************
			// Add a table DIRECTly following the previous one, however this one has no
			// border - producing the look of footnote-like data.
			// In this case we want it to be just the linking icon with no text.
			// ******************************************************************************
			Stat_WriteIconLink(fp, "Help.gif", filename, HELPCARD_EXTENSION, "Go the Help Card.", false);
			Stat_InsertTable();
		}
		else
		{
			Stat_WriteSpace( fp, 2 );
		}
		Stat_WriteSpace( fp, 2 );

		if( m_style == FORMAT_HTML && MyPrefStruct.graph_ascii && typeID != 'virt' && byStat )
			DrawAsciiGraph( VDptr, fp, byStat, typeID, list );
	}


	switch( typeID )
	{
		case 'virt':
			if( m_style != FORMAT_HTML )
			{
				htmlFile format;
				format.VHosts_Write( VDptr, fp, title, header );
			}
			else
				VHosts_Write( VDptr, fp, title, header );
			break;

		case 'mean':
			Stat_WriteMeanPath( VDptr, fp, depth );
			break;

		case 'none':
			break;

		default:
			if( table )
			{
				// main report table
				WriteTable( VDptr, byStat, fp, filename, title, header, id, typeID, liststart,list, graph );

				// misc/sub-table stuff				
				switch( typeID )
				{
					case 'clis':
						if( id == CLIENTSTREAM_PAGE )
						{
							WriteSessionsList();
						}
						break;

					case 'usrs':
						if( id == USERSTREAM_PAGE)
						{
							WriteSessionsList();
						}
						break;

					case 'seng':
						//990415: At the end of a page, special write each of the search engines search strings tables.
						DoSearchEngines( VDptr, fp, byStat, list, sort );
						break;

					case 'kpag':
						DoRouteToKeyPages(VDptr, fp, this, sort);
						break;
	
					case 'kpaf':
						DoRouteFromKeyPages(VDptr, fp, this, sort);
						break;
	
					case 'kvis':
						// Key Visitors a have sub-table each.
						DoKeyVisitorsTables(VDptr, fp, byStat, list, sort);
						break;

					case 'pcbl':
						// each page containing broken links has its own sub-table of the broken links
						DoFailedRequestSubTables( VDptr, fp, filename, byStat, liststart, list,
												  VDptr->byFile, TranslateID(LABEL_BROKENLINK)
												  );
						break;

					case 'ertr':
						// each server error has its own sub-table of its top referals
						DoFailedRequestSubTables( VDptr, fp, filename, byStat, liststart, list,
												  VDptr->byRefer, TranslateID(LABEL_PAGE),
												  TranslateID(LABEL_TOPREFERRALSOF),
												  TranslateID(LABEL_ERRORS), depth > 1 ? depth : 0
												  );
						break;

					default:
						break;
				};
			}
			break;
	}

	// *************************************************************
	// Generate the filter text for this report type and write it
	// to the report.
	// *************************************************************
#ifdef	FILTERS_ENABLED
	{
		// Destination buffer for our filter string.
		char	szFilterString[FILTER_STRING_LENGTH+2];

		// Set the 1st and last 2 characters to NULL.
		// (The last 2 need to be null as that is how we terminate the string list).
		szFilterString[0]						= 
		szFilterString[FILTER_STRING_LENGTH]	= 
		szFilterString[FILTER_STRING_LENGTH+1]	= 0;

		// Construct the filter(s) description.
		int iChars = QS::FormatFilterString(&MyPrefStruct.filterdata,szFilterString, FILTER_STRING_LENGTH, id);

		// Write to report.
		WriteFilterText(fp, szFilterString);

		if( iChars == -1)
			WriteFilterText(fp, "Filter listing truncated, too many to list\0");	// Double null terminated!

	}
#endif

	WriteHelpCard(filename, VDptr, fp, this, id);
}

void baseFile::WritePageImageLink( VDinfoP VDptr, FILE *fp , const char *imagename, const char *title, int graph, long sort )
{
}


// if byStat == 0 and type == -1 , then its a virtualhost main graph
// if byStat == 0 and type == -1 , then its a virtualhost main graph
void baseFile::outputStats(	VDinfoP VDptr, StatList *byStat, long reportPageID )
{
	FILE		*fp = NULL;
	char		reportFilename[512];
	char		logname[512];
	long		sort;
	long		table,graph;
	long		depth;
	long		*flagsPtr, flags;
	long		typeID; 
	char		*filename, *title, *header, *imagename;
	ReportTypesP	report_data;

	if( report_data = FindReportTypeData( reportPageID ) )
	{
		typeID = report_data->typeID;
		title = TranslateID(report_data->titleStringID);
		header = TranslateID(report_data->columnStringID);
		filename = FindReportFilenameStr(report_data->type);
		imagename = FindReportFilenameStr(report_data->type);
		flagsPtr = FindReportFlags( reportPageID );
		flags = flagsPtr ? *flagsPtr : 0;
	} else
		return;

	if( REPORT_TOBE_GENERATED(flags) )
	{
		table = TABLE(flags);
		graph = GRAPH(flags);
		sort =  TABSORT(flags);
		depth = TABSIZE(flags);
		if( depth < 6 ) depth *= 10;
		else
		if( depth < 11 ) depth = (depth-5)*100;
		else depth = 0;
		STATSETDONE( flags, 1 );
		*flagsPtr = flags;
	} else
		return;

	if( !title ) title = header;
	if( !header ) header = title;

	// default sorting
	switch( typeID )
	{
		case 'rday':	if( VDptr->totalDays < 31 ) table=graph=FALSE; break;
	}

	PROGRESS_IDLE

	{
		strcpy( reportFilename,MyPrefStruct.outfile );
		FileFromPath( MyPrefStruct.outfile, logname); 
		CopyFilenameUsingPath( reportFilename, MyPrefStruct.outfile, filename);

		// If any kind of virtual host on , do it in the host directory
		if( MyPrefStruct.ShowVirtualHostGraph() )
		{
			sprintf( tempBuff, "%s%c", GetDomainPath(VDptr), PATHSEP );
			if( typeID != 'norm' && typeID != 'virt' ){
				SetFilePrefix( reportFilename, tempBuff );

				if( typeID != 'virt' )
					ReplacePathFromHost( GetDomainPath(VDptr), reportFilename);
			}
		}

		if( reportPageID == VHOST_PAGE )
		{
			// All formats produce a virtual hosts page in HTML format,
			// this is used to link to each of virtual hosts
			strcat( reportFilename, HTML_EXT );
			HTMLLinkFilename = filename;
			HTMLLinkFilename += HTML_EXT;
		}
		else
		{
			strcat( reportFilename, fileExtension );
			HTMLLinkFilename = filename;
			HTMLLinkFilename += fileExtension;
		}

		if( SingleFileOutput() )
			fp = out;
		else
		{
			if( (fp = Fopen(reportFilename,"w")) == 0) 
			{
				StatusSetID( IDS_ERR_FILEFAILED, reportFilename );
				MsgBox_Error( IDS_ERR_FILEFAILED, reportFilename );
			}
		}
	}


	if( fp )
	{
		// quick link index
		if( MyPrefStruct.html_quickindex && !MyPrefStruct.html_frames && typeID != 'virt' )
		{
			QuickLinkIndexStart( VDptr, fp );
		}

		WriteReportStat( VDptr, byStat, fp, filename, reportPageID, typeID, title,header, imagename, depth, sort, graph, table );
		pdf_currHeight = 0;

		// quick link index
		if( MyPrefStruct.html_quickindex && !MyPrefStruct.html_frames && typeID != 'virt' )
		{
			QuickLinkIndexEnd( fp );
		}

		WritePageFooter( VDptr, fp );

		if( !SingleFileOutput() || reportPageID == VHOST_PAGE )
			Fclose( fp );
	}
}



void baseFile::InitPageOptions()
{
	// once-only initialisation for things that won't change

	char	appBuildDetails[128];

#ifndef DEF_MAC
	long	cid = 0;
	char	idStr[8];

	cid = GetRegisteredCustomerID();

	idStr[0] = serialID >> 24;
	idStr[1] = serialID >> 16 & 0xff;
	idStr[2] = serialID >> 8 & 0xff;
	idStr[3] = serialID & 0xff;
	idStr[4] = 0;
#endif

	GetAppBuildDetails( appBuildDetails );

#ifndef DEF_MAC
	sprintf (pageOptions.metaGenerator, "<meta name=\"Generator\" content=\"%s %s, %s%ld\">", appBuildDetails, GetOSType(), idStr, cid );
#else
	sprintf (pageOptions.metaGenerator, "<meta name=\"Generator\" content=\"%s %s\">", appBuildDetails, GetOSType());
#endif

	sprintf (pageOptions.metaContentType, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">", TranslateID(SYSI_CHARSET));
	sprintf (pageOptions.metaExpires, "<meta http-equiv=\"Expires\" content=\"0\">");
	sprintf (pageOptions.bgColour, "%06lx", MyPrefStruct.RGBtable[7]);
	sprintf (pageOptions.textColour, "%06lx", MyPrefStruct.RGBtable[8]);
	sprintf (pageOptions.linkColour, "%06lx", MyPrefStruct.RGBtable[9]);
	sprintf (pageOptions.vLinkColour, "%06lx", MyPrefStruct.RGBtable[10]);
	sprintf (pageOptions.aLinkColour, "%06lx", MyPrefStruct.RGBtable[11]);
}

void baseFile::WriteVirtualHostListBegin( FILE *fp )
{
	//WriteDirectlyToFile (fi, "<!-- Virtual Host head -->\n");
	WriteDirectlyToFile( fp, "<html>\n<head>\n<title>" );
	WriteDirectlyToFile( fp, "Domains" ); // needs to be translated here !!! RHF
	WriteDirectlyToFile( fp, "</title>\n</head>\n\n" );
	WritePageBodyOpen( fp );								
	WriteDirectlyToFile( fp, "<a href=\"%s%s\" target=\"report\"><img src=\"VirtualBanner.gif\" border=\"0\"></a><hr>\n", FindReportFilenameStr( VHOST_PAGE ), fileExtension );
	WriteDirectlyToFile( fp, "<font face=\"%s\"><ol>\n", MyPrefStruct.html_font ); // open ordered list
}
void baseFile::WritePageBodyOpen( FILE *fp )
{
	WriteDirectlyToFile( fp, "<body bgcolor=\"#%s\" text=\"#%s\" link=\"#%s\" vlink=\"#%s\" alink=\"#%s\">\n\n",
				pageOptions.bgColour, pageOptions.textColour, 
				pageOptions.linkColour, pageOptions.vLinkColour, pageOptions.aLinkColour);
}

void baseFile::WritePageBodyClose( FILE *fp )
{
	WriteDirectlyToFile( fp, "</body>\n</html>");
}

void baseFile::WriteVirtualHostListEnd( FILE *fp )
{
	WriteDirectlyToFile( fp, "</ol>\n");								// close ordered list
	WritePageBodyClose( fp );
}

void baseFile::WriteVirtualHostListItem( FILE *fp, long realVDnum, char *newVDname, char* fileName, VDinfoP VDPtr, short URLType )
{
	if( URLType == kMappedLink)
		WriteDirectlyToFile( fp, "\t<li><a href=\"file:///%s/%s\" target=\"report\">%s</a><br>\n", newVDname, fileName, VDPtr->domainName);
	else	// URLType = kNormalLink
		WriteDirectlyToFile( fp, "\t<li><a href=\"%s/%s\" target=\"report\">%s</a><br>\n", newVDname, fileName, VDPtr->domainName);
}

void baseFile::WriteVirtualHostFrame( FILE *fp )
{
	//WriteDirectlyToFile( fp, "<!-- Virtual List HEAD -->\n");
	WriteDirectlyToFile( fp, "<html>\n<head>\n<title>" );
	WriteDirectlyToFile( fp, TranslateID(REPORT_VIRTUALHOSTSTITLE) );		// needs to be translated here !!! RHF
	WriteDirectlyToFile( fp, "</title>\n</head>\n\n" );
	WriteDirectlyToFile( fp, "<frameset cols=\"205,*\" frameborder=\"1\" framespacing=\"3\" border=\"3\">\n" );
	WriteDirectlyToFile( fp, "\t<frame src=\"frm_index.html\" name=\"list\" marginwidth=\"0\" marginheight=\"0\" frameborder=\"0\" alt=\"Software (C) 1997-2002\">\n" );
	WriteDirectlyToFile( fp, "\t<frame src=\"%s%s\" name=\"report\" resize>\n", FindReportFilenameStr( VHOST_PAGE ), fileExtension );

	//WriteDirectlyToFile( fp, "<!-- Virtual List TAIL -->\n");
	WriteDirectlyToFile( fp, "</frameset>\n</html>\n" );
}

// write out main frame and secondary list frame
long baseFile::WriteReportVDList( long n )
{
	char		newVDname[256];
	FILE		*out2;
	char		indexFilename[512];
	long		myVDnum, totalDomainsDone, VDcount;
	register	VDinfoP	VDptr;			/* current virt domain pointer */
	
	StatusSetID (IDS_WRITINGHOST);

	myVDnum = 0;

	// Create the "right" file for virtual hosts, for all output formats,
	// it will be a HTML file which links to the correct format file (extension) in its directory 
	mystrcpy (indexFilename , MyPrefStruct.outfile);
	SetFileExtension( indexFilename, HTML_EXT ); // Link file will always be HTML

	out2 = Fopen( indexFilename, "w" );
	if( !out2 )
	{
		ErrorMsg ("WriteReportVDList(): Can't open %s\nbecause ", indexFilename, strerror(errno) );
	} else {
		for (VDcount=0; VDcount <= VDnum; VDcount++) {
			VDptr = VD[ VDcount ];
			if( VDptr->totalRequests ) {
				myVDnum = VDptr->domainNum;
				break;
			}
		}
		WriteVirtualHostFrame (out2);
		Fclose( out2 );
		VDptr = VD[ 1 ];
		
		CopyFilenameUsingPath( tempBuff, MyPrefStruct.outfile, "VirtualBanner.gif" );

		if( MyPrefStruct.multivhosts >0 )
			MemToFile( tempBuff,vhost_data,sizeof(vhost_data) );
		else
		if( MyPrefStruct.multimonths == 1 )
			MemToFile( tempBuff,monthly_data,sizeof(monthly_data) );
		else
		if( MyPrefStruct.multimonths == 2 )
			MemToFile( tempBuff,weekly_data,sizeof(weekly_data) );
		else
		if( MyPrefStruct.multimonths == 3 )
			MemToFile( tempBuff,quarter_data,sizeof(quarter_data) );

	}

	/* this code was to create an index frame using the name for the top frame, not important and error-prone, use frm_index instead
	strcpy( indexFilename , MyPrefStruct.outfile );
	SetFilePrefix( indexFilename, "frm_");
	*/
	
	PathFromFullPath( MyPrefStruct.outfile, indexFilename );
	mystrcat( indexFilename, "frm_index" );
	mystrcat( indexFilename, HTML_EXT );
	
	out2 = Fopen( indexFilename,"w" );
	if( !out2 )
	{
		ErrorMsg ("WriteReportVDList(): Can't open frame index %s\nbecause ", indexFilename, strerror(errno) );
	} else
	{
		WriteVirtualHostListBegin (out2);		

		for( VDcount=0; VDcount <= VDnum; VDcount++) {
			VDptr = VD[ VDcount ];
			VDptrs[ VDcount ] = VDptr;
			VDptrs[ VDcount+1 ] = 0;
		}

		// Sort host 1 to n, (ignoring sorting host 0 because thats the FILENAME host which is empty)
		switch (MyPrefStruct.VDsortby) {
			case SORT_SIZES:
				FastQSort( &VDptrs[1], VDnum, 4, CompSortBytesVDomain, TOP_SORT );
				break;
			case SORT_REQUESTS:
				FastQSort( &VDptrs[1], VDnum, 4, CompSortReqVDomain, TOP_SORT );
				break;
			case SORT_PAGES:
				FastQSort( &VDptrs[1], VDnum, 4, CompSortPagesVDomain, TOP_SORT );
				break;
			case SORT_DATE:
				FastQSort( &VDptrs[1], VDnum, 4, CompSortDateVDomain, TOP_SORT );
				break;
			default:
			case 0:
				FastQSort( &VDptrs[1], VDnum, 4, CompSortVDomain, TOP_SORT );
				break;
		}			
		// relcalc next pointers so Vhosts are sorted nicely
		for( VDcount=0; VDcount <= VDnum; VDcount++) {
			VDptr = VDptrs[ VDcount ];
			VDptr->next = VDptrs[ VDcount+1 ];
			VDptr->domainTotal = VDnum;
		}

		if( !MyPrefStruct.corporate_look )
		{
			StandardPlot aPlot;
			aPlot.WriteShadowImages( 0 );
		}

		WriteReportBanner( 0 );

		totalDomainsDone = 0;

		for (VDcount = 0; VDcount <= VDnum; VDcount++)
		{
			if( VDcount > 0)
				VDptr->next = VDptrs[VDcount];

			VDptr = VDptrs[VDcount];
			if( VDptr->totalRequests)
			{
				char szFileName[256];
				if( MyPrefStruct.vhost_seqdirs )
					sprintf (VDptr->domainPath, "%04d", totalDomainsDone );

				totalDomainsDone++;

				if( GetNewOutpath (VDptr, MyPrefStruct.outfile, newVDname))
				{
#ifdef DEF_MAC
					// Bug Fix by Richard - QCM 46759 (duplicate 45394, possibly 46865 also!)
					// trim name while still a Mac colon-separated path as doesn't work on URL-style paths after Path2FilePath() call
					// - may be side effects for other Virtual Host options, this code is far too complex!
					
					TrimPathForClassicMacOS (newVDname);								// trim to 31 characters with ~ at end if longer
#endif				

					// Bug fix by raul - added by JMC QCM43772.
					//mystrcpy (newVDname, filename1 );

					if( newVDname[1] != ':' && newVDname[2] != '/' )
						Path2FilePath (newVDname);

					FileFromPath (MyPrefStruct.outfile, szFileName );
					// ------
					{	// Fix Direct URL paths to work relatively if they match the output path
						char	outpath[256];

						PathFromFullPath (MyPrefStruct.outfile, outpath);
						if( !strcmpd( outpath, newVDname ))
						{
							long	i = mystrlen (outpath);

							mystrcpy (newVDname, newVDname + i);
						}
					}// ------

					DateFixFilename( szFileName, NULL );
					WriteVirtualHostListItem( out2, totalDomainsDone, newVDname, szFileName, VDptr, MappedLink() );

				} else {
					FileFromPath( MyPrefStruct.outfile, szFileName );
					DateFixFilename( szFileName, NULL );
					mystrcpy (newVDname, GetDomainPath (VDptr));
#ifdef DEF_MAC
					TrimPathForClassicMacOS (newVDname);								// trim to 31 characters with ~ at end if longer
#endif				
					WriteVirtualHostListItem( out2, totalDomainsDone, newVDname, szFileName, VDptr, NormalLink() );
				}
			}
		}//for loop

		WriteVirtualHostListEnd (out2);		
		Fclose( out2 );

		long *flagsPtr = FindReportFlags( VHOST_PAGE );
		STATSET( *flagsPtr, 1 );
		STATSETT( *flagsPtr, 1 );
		STATSETG( *flagsPtr, 1 );
		outputStats( VDptrs[0], NULL, VHOST_PAGE );
	}
	return totalDomainsDone;
}

long			maxreports, report_count;


// Same as below but accept filter too.
long baseFile::WriteReport( StatList *statListObj, long id, FilterFunc p_filter )
{
	long ret;
	statListObj->m_FilterIndex = p_filter;
	ret = WriteReport( statListObj, id );
	statListObj = NULL;
	return ret;
}

long baseFile::WriteReport( StatList *statListObj, long id )
{
	ReportTypesP	report_data;

	if( IsStopped())				// if we have already stopped, just return
		return (0);					// return value is ignored by calling function
		
	if( CheckReportFreeSpace())		// less than 200 kB left, let's bail
	{
		StopAll (OUTOFDISK);		// here's the reason
		return (0);					// again, return value is ignored by caller
	}

	if( statListObj ){

		if( ReportTurnedOn( id ) )
		{
			report_data = FindReportTypeData( id );

			if( statListObj->num && !stopall && report_data )
			{
				char	txt[256], *ptr;
				long	perc;

				VDinfoP v = (VDinfoP)statListObj->GetVD();

				PutReportOnNextPage( true ); // For PDF, so that each Section starts on a new Page

				ptr = txt;
#ifdef DEF_MAC
				if( v->domainTotal > 0 ){
					ptr += sprintf( ptr, "%s: ", v->domainName );		// already show the domain number in separate item
				}
				
				// Show which report we are current writing on the status bar
				// Don't use translated string on Mac as high ASCII character conversion is wrong for MacRoman!
				sprintf (ptr, (const char*)ReturnString (IDS_WRITEREPORT), DefaultEnglishStr (report_data->titleStringID));
#else
				if( v->domainTotal > 0 ){
					ptr += sprintf( ptr, "%d/%d. %s: ", v->domainNum, v->domainTotal, v->domainName );
				}

				// Show which report we are current writing on the status bar
				std::string noHTMLTokensReportName = LoadLangStringConvertFromHTML( report_data->titleStringID );
				sprintf( ptr, (const char*)ReturnString( IDS_WRITEREPORT ), noHTMLTokensReportName.c_str() );
#endif
				perc = ((report_count++)*100L)/maxreports;
				ShowProgress( perc, FALSE, txt );

				// ***************************************************************************************
				// Check if the filtering is going to remove ALL the items in the table.
				// If there are no rows then we don't want to produce the table.
				// ***************************************************************************************
				bool bDoWriteReport = true; // If there is no filtering function then we can assume that there is no filtering.
				
				if( statListObj->m_FilterIndex )
				{
					int i;
					for (i=0; i<statListObj->num; ++i)
					{
						if( statListObj->m_FilterIndex(statListObj->GetStat(i)) )
							break;
					}

					// ***************************************************************************************
					// If we iterated through the whole list then we didn't find
					// any index values that should be in the table.
					// Therefore we should NOT generate the table in the report.
					// ***************************************************************************************
					bDoWriteReport = (i != statListObj->num);
				}

				if( bDoWriteReport)
				{
					// Produce the particular report
					outputStats( v, statListObj, id );
					OutlineStatus( 0 );
				}

				// Show we have finished the current report on the status bar
#ifndef DEF_MAC
				sprintf( ptr, ReturnString( IDS_WRITEREPORT_DONE ), noHTMLTokensReportName.c_str() );
				ShowProgress( perc, FALSE, txt );
#endif
				return 1;
			}
		}
	}

	maxreports--;

	return 0;

}




long baseFile::WriteAllPages( VDinfoP VDptr, long logstyle )
{
	struct App_config *c = &MyPrefStruct;

	maxreports = GetTotalReportTypes() - 1;
	report_count = 0;

	// ****************************************************
	// The HelpCards need icons to be written.
	// ****************************************************
	WriteHelpCardIcons(VDptr);


	if( AnyTrafficStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		WriteOutline( TranslateID(RCAT_TRAFFIC));
		WriteReport( VDptr->byHour, HOUR_PAGE );
		WriteReport( VDptr->byHour, HOURHIST_PAGE );
		WriteReport( VDptr->byDate, DATE_PAGE );
		if( VDptr->byDate->GetStatListNum() > 31 )
			WriteReport( VDptr->byDate, RECENTDATE_PAGE );	
		else maxreports--;
		WriteReport( VDptr->byWeekday, WEEK_PAGE );	
		WriteReport( VDptr->byWeekdays[0], SUNDAY_PAGE );	
		WriteReport( VDptr->byWeekdays[1], MONDAY_PAGE );	
		WriteReport( VDptr->byWeekdays[2], TUEDAY_PAGE );	
		WriteReport( VDptr->byWeekdays[3], WEDDAY_PAGE );	
		WriteReport( VDptr->byWeekdays[4], THUDAY_PAGE );	
		WriteReport( VDptr->byWeekdays[5], FRIDAY_PAGE );	
		WriteReport( VDptr->byWeekdays[6], SATDAY_PAGE );	
		if( VDptr->byMonth->GetStatListNum() > 1 )
			WriteReport( VDptr->byMonth, MONTH_PAGE );	
		else maxreports--;
	}


	if( AnyDiagnosticStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		WriteOutline( TranslateID(RCAT_DIAGNOSTIC));
		WriteReport( VDptr->byErrors, ERRORS_PAGE );
		WriteReport( VDptr->byErrors, ERRORSHIST_PAGE );
		// ------------ STREAMING REPORTS
		if( MyPrefStruct.stat_style == STREAM ) 
		{
			WriteReport( VDptr->byBrokenLinkReferal, CLIPSBROKENLINKS_PAGE );
			WriteReport( VDptr->byPages, CLIPSERRORS_PAGE );		// Most Common Top 5 Errors
			WriteReport( VDptr->byPages, CLIPSFAILED_PAGE );		// only 404+ clips
			WriteReport( VDptr->byPages, CLIPSFAILEDHIST_PAGE );
			WriteReport( VDptr->byPages, CLIPSPACKETLOSS_PAGE );
			WriteReport( VDptr->byPages, CLIPSLOSSRATE_PAGE );
			WriteReport( VDptr->byPages, CLIPSSECSBUF_PAGE );
			WriteReport( VDptr->byPages, CLIPSHIGH_PAGE );
			WriteReport( VDptr->byPages, CLIPSLOW_PAGE );
		} else {
			WriteReport( VDptr->byBrokenLinkReferal, VDptr->GetSiteURL() ? EXTBROKENLINKS_PAGE : BROKENLINKS_PAGE );
			if( VDptr->byIntBrokenLinkReferal )
			{ 
				WriteReport( VDptr->byIntBrokenLinkReferal, INTBROKENLINKS_PAGE );
			}
			WriteReport( VDptr->byErrorURL, ERRORURL_PAGE );
			WriteReport( VDptr->byErrorURL, ERRORURLHIST_PAGE );
		}
	}
	
	if( AnyServerStatsTurnedOn( VDptr, &MyPrefStruct ) ||
		GroupReportDone( &MyPrefStruct, STREAMINGCONTENT_GROUP ) ) {		
		WriteOutline( TranslateID(RCAT_SERVER));

		// ------------ STREAMING REPORTS
		if( MyPrefStruct.stat_style == STREAM && VDptr->byPages ) 
		{
			WriteReport( VDptr->byPages, CLIPSVID_PAGE,		Filter_IsVideoClip );
			WriteReport( VDptr->byPages, CLIPSVIDHIST_PAGE, Filter_IsVideoClip );
			WriteReport( VDptr->byPages, CLIPSVIDVIEW_PAGE, Filter_IsVideoClip );
			WriteReport( VDptr->byPages, CLIPSVIDRATES_PAGE,Filter_IsVideoClip );
			WriteReport( VDptr->byPages, CLIPSAUD_PAGE,		Filter_IsAudioClip );
			WriteReport( VDptr->byPages, CLIPSAUDHIST_PAGE,	Filter_IsAudioClip );
			WriteReport( VDptr->byPages, CLIPSAUDVIEW_PAGE,	Filter_IsAudioClip );
			WriteReport( VDptr->byPages, CLIPSAUDRATES_PAGE,Filter_IsAudioClip );

			WriteReport( VDptr->byPages, LIVEVID_PAGE,		Filter_IsLiveVideoClip );
			WriteReport( VDptr->byPages, LIVEVIDHIST_PAGE,	Filter_IsLiveVideoClip );
			WriteReport( VDptr->byPages, LIVEVIDVIEW_PAGE,	Filter_IsLiveVideoClip );
			WriteReport( VDptr->byPages, LIVEVIDRATES_PAGE,	Filter_IsLiveVideoClip );
			WriteReport( VDptr->byPages, LIVEAUD_PAGE,		Filter_IsLiveAudioClip );
			WriteReport( VDptr->byPages, LIVEAUDHIST_PAGE,	Filter_IsLiveAudioClip );
			WriteReport( VDptr->byPages, LIVEAUDVIEW_PAGE,	Filter_IsLiveAudioClip );
			WriteReport( VDptr->byPages, LIVEAUDRATES_PAGE,	Filter_IsLiveAudioClip );

			WriteReport( VDptr->byPages, CLIPS_PAGE,		Filter_IsAnyClip );
			WriteReport( VDptr->byPages, CLIPSHIST_PAGE,	Filter_IsAnyClip );
			WriteReport( VDptr->byPages, CLIPSLEAST_PAGE,	Filter_IsAnyClip );
			WriteReport( VDptr->byPages, CLIPSMAXCON_PAGE,	Filter_IsAnyClip );
			WriteReport( VDptr->byPages, CLIPS_FF_PAGE,		Filter_IsAnyClip );
			WriteReport( VDptr->byPages, CLIPS_RW_PAGE,		Filter_IsAnyClip );
			WriteReport( VDptr->byPages, CLIPSPERCENT_PAGE,	Filter_IsClip );
			WriteReport( VDptr->byPages, CLIPSCOMPLETED_PAGE,Filter_IsClip );

			WriteReport( VDptr->byProtocol, CLIPSPROTOCOLS_PAGE );
			WriteReport( VDptr->byVideoCodecs, VIDCODECS_PAGE );
			WriteReport( VDptr->byAudioCodecs, AUDCODECS_PAGE );
			
			WriteReport( VDptr->byMediaTypes, MEDIATYPES_PAGE );
		}
		if( VDptr->byPages )
		{
			WriteReport( VDptr->byPages, PAGES_PAGE, Filter_IsPage );
			WriteReport( VDptr->byPages, PAGEHIST_PAGE, Filter_IsPage );
			WriteReport( VDptr->byPages, PAGESLEAST_PAGE, Filter_IsPage );
			WriteReport( VDptr->byPages, PAGESFIRST_PAGE, Filter_IsPage );
			WriteReport( VDptr->byPages, PAGESFIRSTHIST_PAGE, Filter_IsPage );
			WriteReport( VDptr->byPages, PAGESLAST_PAGE, Filter_IsPage );
			WriteReport( VDptr->byPages, PAGESLASTHIST_PAGE, Filter_IsPage );

			WriteReport( VDptr->byPages, KEYPAGEROUTE_PAGE, FilterNonKeyPagesTo );			// Route Taken to KeyPages.
			WriteReport( VDptr->byPages, KEYPAGEROUTEFROM_PAGE, FilterNonKeyPagesFrom );		// Route Taken from KeyPages.
		}

		WriteReport( VDptr->byServers, SRCADDR_PAGE );
		WriteReport( VDptr->byDir, DIRS_PAGE );
		WriteReport( VDptr->byTopDir, TOPDIRS_PAGE );
		WriteReport( VDptr->byTopDir, TOPDIRSHIST_PAGE );
		WriteReport( VDptr->byFile, FILE_PAGE ); // URLs
		WriteReport( VDptr->byClient, MEANPATH_PAGE );
		WriteReport( VDptr->byGroups, GROUPS_PAGE );
		WriteReport( VDptr->byGroups, GROUPSHIST_PAGE );
		WriteReport( VDptr->byDownload, DOWNLOAD_PAGE );
		WriteReport( VDptr->byDownload, DOWNLOADHIST_PAGE );
		WriteReport( VDptr->byType, FILETYPE_PAGE );

	}


	if( AnyStreamingMediaStatsTurnedOn( VDptr, &MyPrefStruct ) && 
		 MyPrefStruct.stat_style != STREAM )
	{
		WriteOutline( TranslateID(RCAT_STREAMING));
		WriteReport( VDptr->byAudio, AUDIO_PAGE );
		WriteReport( VDptr->byAudio, AUDIOHIST_PAGE );
		WriteReport( VDptr->byVideo, VIDEO_PAGE );
		WriteReport( VDptr->byVideo, VIDEOHIST_PAGE );
		WriteReport( VDptr->byMediaTypes, MEDIATYPES_PAGE );
		WriteReport( VDptr->byMediaPlayers, MPLAYERS_PAGE );
		WriteReport( VDptr->byMediaPlayers, MPLAYERSHIST_PAGE );
	}


	if( ReportOn( BILLING_PAGE ) && VDptr->byBilling )
	{
		WriteReport( VDptr->byBilling, BILLING_PAGE );
	}

	if( AnyDemographicStatsTurnedOn( VDptr, &MyPrefStruct ) ) {		
		WriteOutline( TranslateID(RCAT_DEMOGRAPH));
		WriteReport( VDptr->byClient, CLIENT_PAGE );
		WriteReport( VDptr->byClient, CLIENTSTREAM_PAGE );
		WriteReport( VDptr->byClient, CLIENTHIST_PAGE );
		WriteReport( VDptr->byUser, USER_PAGE );
		WriteReport( VDptr->byUser, USERSTREAM_PAGE );
		WriteReport( VDptr->byUser, USERHIST_PAGE );
		WriteReport( VDptr->bySecondDomain, SECONDDOMAIN_PAGE );
		WriteReport( VDptr->byDomain, DOMAIN_PAGE );
		WriteReport( VDptr->byRegions, REGION_PAGE );
		WriteReport( VDptr->byOrgs, ORGNAMES_PAGE );
		WriteReport( VDptr->byClient, SESSIONS_PAGE );
		// *************************************************************
		// The KeyVisitor report has a filtering function.
		// *************************************************************
		WriteReport( VDptr->byClient, KEYVISITORS_PAGE, LookupKeyVisitor );
	}

	if( AnyReferralStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		WriteOutline( TranslateID(RCAT_REFERRALS));
		WriteReport( VDptr->byRefer, REFERURL_PAGE );
		WriteReport( VDptr->byReferSite, REFERSITE_PAGE );
		WriteReport( VDptr->byReferSite, REFERSITEHIST_PAGE );
		WriteReport( VDptr->bySearchSite, SEARCHSITE_PAGE );
		WriteReport( VDptr->bySearchSite, SEARCHSITEHIST_PAGE );
		WriteReport( VDptr->bySearchStr, SEARCHSTR_PAGE );
		WriteReport( VDptr->bySearchStr, SEARCHSTRHIST_PAGE );
	}



	if( AnySystemsStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		WriteOutline( TranslateID(RCAT_SYSTEMS));
		if( MyPrefStruct.stat_style == STREAM ) 
		{
			WriteReport( VDptr->byMediaPlayers, MPLAYERS_PAGE );
			WriteReport( VDptr->byMediaPlayers, MPLAYERSHIST_PAGE );
			WriteReport( VDptr->byMediaPlayers, MPLAYERSOS_PAGE );
			WriteReport( VDptr->byCPU, CPU_PAGE );
			WriteReport( VDptr->byLang, LANG_PAGE );
		}
		WriteReport( VDptr->byRobot, ROBOT_PAGE );
		WriteReport( VDptr->byRobot, ROBOTHIST_PAGE );
		WriteReport( VDptr->byBrowser, BROWSER_PAGE );
		WriteReport( VDptr->byBrowser, BROWSERHIST_PAGE );
		WriteReport( VDptr->byBrowser, BROWSEROS_PAGE );
		WriteReport( VDptr->byOperSys, OPERSYS_PAGE );
		WriteReport( VDptr->byOperSys, OPERSYSHIST_PAGE );
		WriteReport( VDptr->byUnrecognizedAgents, UNRECOGNIZEDAGENTS_PAGE );
	}

	if( AnyAdvertisingStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		WriteOutline( TranslateID(RCAT_ADVERT));
		WriteReport( VDptr->byAdvert, ADVERT_PAGE );
		WriteReport( VDptr->byAdvert, ADVERTHIST_PAGE );
		WriteReport( VDptr->byAdCamp, ADVERTCAMP_PAGE );
		WriteReport( VDptr->byAdCamp, ADVERTCAMPHIST_PAGE );
	}

	if( AnyMarketingStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		WriteOutline( TranslateID(RCAT_MARKETING));
		WriteReport( VDptr->byCirculation,  CIRC_PAGE );
		WriteReport( VDptr->byLoyalty, LOYALTY_PAGE );
		WriteReport( VDptr->byTimeon, TIMEON_PAGE );
	}

	if( AnyClustersStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		WriteOutline( TranslateID(RCAT_CLUSTERS) );
		WriteReport( VDptr->byClusters, CLUSTER_PAGE );
		WriteReport( VDptr->byClusters, CLUSTERHIST_PAGE );
	}


#ifdef DEF_APP_FIREWALL
	if( logstyle == LOGFORMAT_FIREWALLS && AnyFirewallStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		WriteOutline( TranslateID(RCAT_FIREWALL));
		WriteReport( VDptr->byServers, SRCADDR_PAGE );
		WriteReport( VDptr->byProtocol, PROTSUMMARY_PAGE );
		WriteReport( VDptr->byHTTP, PROTHTTP_PAGE );
		WriteReport( VDptr->byHTTPS,  PROTHTTPS_PAGE );
		WriteReport( VDptr->byMail,  PROTMAIL_PAGE );
		WriteReport( VDptr->byFTP, PROTFTP_PAGE );
		WriteReport( VDptr->byTelnet, PROTTELNET_PAGE );
		WriteReport( VDptr->byDNS, PROTDNS_PAGE );
		WriteReport( VDptr->byPOP3, PROTPOP3_PAGE );
		WriteReport( VDptr->byReal, PROTREAL_PAGE );
		WriteReport( VDptr->byOthers, PROTOTHERS_PAGE );
	} else
		maxreports -= 7;
#endif // DEF_APP_FIREWALL

	WriteOutline( "" );

#if DEF_WINDOWS
	{	long	kbfree = GetDiskFree( MyPrefStruct.outfile ) / 1024;
		if( kbfree < 200 && kbfree>=0 ){
			MsgBox_Error( IDS_NODISKSPACE, kbfree );
		}
	}
#endif

	CleanUpHelpCardIcons(VDptr);

	return 1;
}




/* WriteSummaryDetails - print summary of total day statistics */
void baseFile::WriteSummaryDetails( VDinfoP VDptr, FILE *fp, char *outfile, long numberOfLogs )
{
	long		tw = TABLE_WIDTH, th = TABLE_HEIGHT, num;

	//calculate days elapsed
	num = VDptr->totalDays;
	//num = (long)(VDptr->lastTime/ONEDAY) - (VDptr->firstTime/ONEDAY)+1;

	ProduceSummaryTitle( fp, numberOfLogs, VDptr->domainName );

	if( MyPrefStruct.report_title[0] )
		WritePageTitle( VDptr, fp, MyPrefStruct.report_title );
	else
		WritePageTitle( VDptr, fp, TranslateID(SUMM_ACCESSSTATS) );

/*	This call is redundant, as htmlFile::WriteReportSummary calls WriteReportBanner() 
	before calling WriteSummaryDetails, so the banner gets written twice. This is a fix 
	for QCM #49882 ... broke StuffIt compress reports w/. delete original !!!

	// WriteReportBanner( VDptr );
*/
	WriteDemoBanner( VDptr, fp );

	WriteRefreshMetaOption( fp );

	if( STAT(MyPrefStruct.stat_summary) )
	{
		WriteLogProcessingTimeInfo( VDptr, fp );
		Stat_WriteSpace( fp, 2 );

		SummaryTableHeader( fp, tw, th );
		SummaryTableRow( fp, 20 );
		// use the siteurl as the header when more than one log file added to one report

		SummaryTableTitle( fp, (const char*)GetReportTitle(), -1 );
	}

	SummaryTableRow(fp, 20);
	SummaryTableStart( fp, 0, 0 );

	// ********* HTML AREA ***********
	ProduceHTMLInternalPageLinks( VDptr, fp );
	// ********* HTML AREA ***********

	SummaryTableFinish( fp );
	SummaryTableDataEnd( fp );

	if( STAT(MyPrefStruct.stat_summary) )
	{
		SummaryTableData( fp, 40 );
		WriteReportLoadSummary(  VDptr, fp, num, 0, 2 );
		SummaryTableDataEnd( fp );

		SummaryTableData( fp, 40 );
		WriteReportLoad2Summary( VDptr, fp, num, 0, 2 );
		SummaryTableDataEnd( fp );
		SummaryTableRowEnd( fp );
		SummaryTableFinish( fp );

		Stat_WriteSpace( fp, 2 );
	}
}



/* WriteReportSummary - print summary of total day statistics */
void baseFile::WriteReportSummary( VDinfoP VDptr, FILE *fp, char *outfile, long numberOfLogs )
{
	WriteSummaryDetails( VDptr, fp, outfile, numberOfLogs );
	WritePageFooter( VDptr, fp );
	fflush( fp );
}

void baseFile::StartReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs )
{
	WriteReportSummary( VDptr, fp, (char*)filename, numberOfLogs );
}

void baseFile::FinishReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs )
{
	fflush( fp );
	Fclose( fp );		// close the first output file.
}

// --------------------------------------------------------------------------------------------------------------
void baseFile::WriteClassReport ( long logstyle, long numberOfLogs )
{
	VDinfoP				VDptr;			/* current virt domain pointer */
	long				currentDomainCount, totalDomainsDone, VDcount;
	double				gtime1;
	short				doSummary = 1;
	char				summaryreportFilename[512];

	InitExtensions();
	ClearLoadWeekdaysAndMonths();

	totalDomains = VDnum;
	totalDomainsDone = 1;

	// Only for MULTI_REPORTS....
	if( MyPrefStruct.ShowVirtualHostGraph() )
	{
		if( m_style == FORMAT_HTML )
			totalDomainsDone = WriteReportVDList( 0 );
		else
		{
			// Have to change format to HTML, coz for Virtual Hosts, the link to each of the "other" formats is done using HTML
			htmlFile format;
			totalDomainsDone = format.WriteReportVDList( 0 );
		}
	}

	// Sort all HOSTS
	for( VDcount=0; (VDcount<=VDnum); VDcount++)
	{
		VDptr = VD[ VDcount ];
		SortRecords( VDptr, logstyle );
	}

#if DEF_WINDOWS
	if( mystrcmpi( TranslateID( SYSI_CHARSET ), "ISO-8859-1" ) != 0 )
	{
		nonISO88591String = new NonISO88591String();
		if( !nonISO88591String->CreateObjects( TranslateID( SYSI_GRAPHFONTNAME ), TranslateID( SYSI_CHARSET ) ) )
		{
			delete nonISO88591String;
			nonISO88591String = 0;
		}
	}
#endif // DEF_WINDOWS	

	currentDomainCount = 1;		// works far better if this is initialised before the loop, not inside the loop !!!
	
	// LOOP FOR EACH VIRTUAL DOMAIN.... or just 1
	for (VDcount = 0; (VDcount <= VDnum && !stopall); VDcount++)
	{
		VDptr = VD[ VDcount ];

		if( VDptr->time2 == 0 )
			VDptr->time2 = timems() - VDptr->time1;

		if( VDptr->byMonth )
			VDptr->byMonth->Substitute(ConvDate);		/* convert date formats */

		if( VDptr->byDate )
			VDptr->byDate->Substitute(ConvDate);		/* convert date formats */

		VDptr->totalDays = (long)(VDptr->lastTime/ONEDAY) - (VDptr->firstTime/ONEDAY)+1;		// total inclusive days it exists in

		if( VDptr->totalRequests>0 )
		{
			char dirName[512];
			if( MyPrefStruct.ShowVirtualHostGraph() ) 
			{
				if( GetNewOutpath( VDptr, MyPrefStruct.outfile, dirName ) ) 
				{
					MakeDir( dirName );
					sprintf( summaryreportFilename, "%s%c%s", dirName, PATHSEP, FileFromPath( MyPrefStruct.outfile,0 ) );
				} else {
					MakeDir( dirName );

					// need to check that directory path isn't too long and truncate, as for "MakeDirPath" in myansi.c"
					sprintf( summaryreportFilename, "%s%s", dirName, FileFromPath( MyPrefStruct.outfile,0 ) );
				}
			} else 
			{
				mystrcpy( summaryreportFilename, MyPrefStruct.outfile );
				if( mystrchr( summaryreportFilename, '{' ) )
				{
					PathFromFullPath( summaryreportFilename, 0 );

					DateFixFilename( summaryreportFilename, dirName );
					MakeDir( dirName );

					// need to check that directory path isn't too long and truncate, as for "MakeDirPath" in myansi.c"
					sprintf( summaryreportFilename, "%s%s", dirName, FileFromPath( MyPrefStruct.outfile,0 ) );
				}
			}

			// Again this should have been in its own OpenSummaryReport();
			if( m_style == FORMAT_EXCEL )
				out = Fopen( summaryreportFilename,"wb" );  // for excel BIFF
			else
				out = Fopen( summaryreportFilename,"w" );

			/* open output html file */
			if( out == 0 ) 
			{
				StatusSetID( IDS_ERR_OUTPUT, summaryreportFilename, strerror(errno) );
				MsgBox_Error( IDS_ERR_OUTPUT, summaryreportFilename, strerror(errno) );
			} else {
#ifdef DEF_MAC
				char	status[64];
				sprintf (status, "Domain %i of %i", currentDomainCount, totalDomainsDone );
				StatusWindowSetFile (status);
#else
				char	status[256];
				sprintf (status, "Writing Domain %d of %d : '%s'...", currentDomainCount, totalDomainsDone, VDptr->domainName );
				StatusSet( status );

#endif
				currentDomainCount++;

				gtime1 = timems();

				// errrrrrrrrrrrr god damn, why ??????????? , PDF only stuff (RS)
				// this should be in StartReport() in PDFClass.
				Init( pdfFilename.c_str(), out );

				// clear the Report Completed status BITs in the config structure settings (config_struct.h)
				// these results can be used by the report summaryies or other things...
				ConfigSettings_ClearReportStatus( &MyPrefStruct );	

				// NOTE: depending on the format, either StartReport or FinishReport writes out the summary page....
				// by default for now, only html writes the summary at the end.
				StartReport( VDptr, out, summaryreportFilename, numberOfLogs );
				WriteAllPages( VDptr, logstyle );
				FinishReport( VDptr, out, summaryreportFilename, numberOfLogs );

				StatusSetID( IDS_DONE );
			}
		} //if( VDptr->totalRequests>0 ) {
		else
			StatusSet( "Zero Requests" );
	} //for( VDcount=0; VDcount <= VDnum; VDcount++){

#if DEF_WINDOWS
	if( nonISO88591String )
	{
		delete nonISO88591String;
		nonISO88591String = 0;
	}
#endif	
}

