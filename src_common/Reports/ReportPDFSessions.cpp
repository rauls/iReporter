
#include "FWA.h"

#include "ReportPDFSessions.h"
#include "ReportPDF.h"
#include "ReportFuncs.h"
#include "translate.h"
#include "config.h"
#include "EngineParse.h"
#include "engine_drill.h"
#include "engine_proc.h"
#include "FileTypes.h"
#include "StatDefs.h"

#if DEF_MAC
	#include "post_comp.h"
	#include "progress.h"
#endif
#if DEF_WINDOWS 
	#include "postproc.h"
	#include "httpinterface.h"
#endif
#if DEF_UNIX
	#include "postproc.h"
	#include "httpinterface.h"
#endif

extern void ValuetoString( long scaler_type, __int64 hvalue, __int64 current_value, char *name );
extern char * protocolStrings[];
extern struct App_config MyPrefStruct;
extern long			VDnum;
extern "C" long strcmpExtensions( char *string, char exts[256][8] );



CQPDFSessionWriter::CQPDFSessionWriter( const char *fileExtensionP, pdfFile& myPdfFile, PDFTableSettings thePDFTableSettingsP, FILE *theFPP, int subPageDrawWidthP )
:	CQSessionWriter( fileExtensionP, myPdfFile )
{
	thePDFTableSettings = thePDFTableSettingsP;
	clickStreamTextLists.clear();
	theFP = theFPP;
	subPageDrawWidth = subPageDrawWidthP;
}

void CQPDFSessionWriter::Write_SessHeader( FILE *fp , const char *client, const char *timeStr, const char *browser, const char *opersys )
{
	std::string text = TranslateID( SESS_SESSLISTINGFOR );
	text += " ";
	text +=	client; 
	text += ", ";
	text += TranslateID( SESS_AVGSESSLENGTH );
	text += " ";
	text += timeStr;

	// This will be an "outline" entry linking to a page, it will not appear on the page as text
	SessionOutline( client );

	SessionHeading( text );
	if ( browser && opersys )
	{
		text = TranslateID(SESS_BROWSERUSED);
		text += " ";
		text +=	browser;
		text +=	", ";
		text +=	TranslateID(SESS_OPERSYSUSED);
		text +=	" ";
		text +=	opersys;
		SessionText( text, ColHeadingSize(), PDF_NON_COLUMN, PDF_BOLD );
	}
	SessionText( PDF_BLANKLINE, TitleSize(), PDF_NON_COLUMN );
	SessionText( TranslateID( SESS_SESSLIST ), TitleSize(), PDF_TEXT_CENTERED, PDF_BOLDITALIC );
	SessionText( PDF_BLANKLINE, TitleSize(), PDF_NON_COLUMN );
}

void CQPDFSessionWriter::Write_SessItal( FILE *fp, const char *txt )
{
	SessionText( txt, DataSize(), PDF_TEXT_COLUMN, PDF_ITALIC );
}

/*void CQPDFSessionWriter::Write_SessSummary( FILE *fp , const char *txt, __int64 b1, int grandTotal )
{
	char	numStr[32];

	if (b1)
	{
		ValuetoString( TYPE_BYTES, b1, b1, numStr );
		std::string temp;
		temp = txt;
		temp += " ";
		temp += numStr;

		if ( !grandTotal )
			SessionText( temp, DataSize(), PDF_TEXT_COLUMN, PDF_BOLD );
		else
			SessionText( temp, DataSize(), PDF_TEXT_COLUMN, PDF_BOLD );
	}
}*/

/*void CQPDFSessionWriter::Write_SessSummary( FILE *fp , const char *txt, const char *txt2, int grandTotal = 0 )
{

}*/

void CQPDFSessionWriter::Write_SessTotalBand( FILE *fp , __int64 b1, __int64 b2, long days )
{
	SessionText( PDF_BLANKLINE, ColHeadingSize(), PDF_NON_COLUMN );
	Write_SessSummary( fp, TranslateID( SESS_TOTALBYTESUSED ), b1, PDF_BOLD );
	Write_SessSummary( fp, TranslateID( SESS_TOTALBYTESSENT ), b2, PDF_BOLD );
	Write_SessSummary( fp, TranslateID( SESS_AVGBYTESPERDAY ), b1/days, PDF_BOLD );
}

void CQPDFSessionWriter::Write_SessTotalTime( FILE *fp , long time, long days )
{
	char	numStr[32];
	CTimetoString( time, numStr );
	std::string temp = TranslateID( SESS_TOTALONLINETIME );
	temp += " ";
	temp += numStr;
	SessionText( temp, DataSize(), PDF_NON_COLUMN, PDF_BOLD );
	CTimetoString( time/days, numStr );
	temp = TranslateID( SESS_AVGONLINETIME );
	temp += " ";
	temp += numStr;
	SessionText( temp, DataSize(), PDF_NON_COLUMN, PDF_BOLD );
}



void CQPDFSessionWriter::Write_SessLength( FILE *fp , const char *timeStr )
{
	std::string str;
	str += TranslateID( SESS_ESTSESSLENGTH );
	str += " ";
	str += timeStr;
	SessionText( str, DataSize(), PDF_TEXT_COLUMN, PDF_BOLD );
}

void CQPDFSessionWriter::Write_SessName( FILE *fp , long rgb, long sitem, const char *dateStr )
{
	std::string str;
	str += TranslateID( SESS_SESSION );
	str += "#";
	str += intToStr( sitem );
	str += " - ";
	str += dateStr;
	SessionText( str, DataSize(), PDF_TEXT_COLUMN, PDF_BOLD );
}

//long strcmpExtensions( char *string, char exts[256][8] );
#include "Report.h"
extern char *protocolStrings[];

void CQPDFSessionWriter::Write_SessPage( FILE *fp , long item, const char *dateStr, const char *thishost, const char *url, long proto )
{
	char *p, c = ' ';

	if ( proto == -1 )
		c = '*';

	if ( strstr( url, "://" ) )
		mystrcpy( tempBuff2, url );
	else
		MakeURL( tempBuff2, thishost, url );

	if ( proto>0 )
		p = protocolStrings[proto];
	else 
		p = " ";

	std::string str;

	if ( proto>0 )
	{
		str = intToStr( item );
		str += ". ";
		str += dateStr;
		str += p;
		str += tempBuff2;
		SessionText( str, DataSize() );
	}
	else if ( item == 0 )
	{
		str = TranslateID(LABEL_REFERRAL);
		str += ":  ";
		str += url;
		SessionText( str, DataSize(), PDF_TEXT_COLUMN, PDF_BOLD );
	}
	else
	{
		str = intToStr( item );
		str += ". ";
		str += dateStr;
		str += p;
		str += tempBuff2;
		SessionText( str, DataSize() );
	}
}

void CQPDFSessionWriter::SessionTextSameXPosBold( FILE *fp, std::string text )
{
	SessionText( text, DataSize(), PDF_NON_COLUMN, PDF_BOLD );
}

void CQPDFSessionWriter::SessionTextColumnBlankLine()
{
	SessionText( PDF_BLANKLINE, DataSize() );
}

char *CQPDFSessionWriter::WriteSessions( void *vp, StatList *byStat, char *filename, const char *title, const char *thishost, const char *client, short unit, int listdepth )
{
	static char newfile[MAXURLSIZE];
	char		tempSpec[MAXURLSIZE];
	long		depth, sitem;
	FILE		*hout = 0;
	VDinfoP	VDptr = (VDinfoP)vp;
	register Statistic	*p;

	p = byStat->GetStat(unit);
	
	if ( !p->sessionStat->Session )
		return 0;

	char *browser, *opersys, timeStr[256];
		char	dateStr[256], *url;
		long	firstDate=0, dur, time, ptime=0;
		long	hashid, phashid=SESS_START_PAGE;
		long	rgbcolor[]={ -1, 0xe0e0e0, 0, 0 };
		long	days=0, numPagesFound = 0, totalDownloads = 0, sessionDownloads=0;
		long	totaldownloadbytes = 0, sessionBytes = 0;
		Statistic* pStat;

	if ( VDptr->byBrowser )	browser = VDptr->byBrowser->GetName( p->counter2 ); else browser = NULL;
	if ( VDptr->byOperSys )	opersys = VDptr->byOperSys->GetName( p->counter3 ); else opersys = NULL;

	CTimetoString( p->visitTot / p->visits, timeStr );
	Write_SessHeader( hout , client, timeStr, browser, opersys );

	depth = 1;
	sitem = 1;

	PROGRESS_IDLE

	long numSessionEntries(p->sessionStat->GetNum());

	if( numSessionEntries )
	{
		long numPagesToDisplay( numSessionEntries );

		// limit the session output so its not 50meg html file.
		if (numPagesToDisplay > MyPrefStruct.sessionLimit && MyPrefStruct.sessionLimit )
			numPagesToDisplay = MyPrefStruct.sessionLimit;

		days = 0;
		for( long i(0); i<numSessionEntries; ++i )
		{
			if ( IsDebugMode() )
			{
				sprintf( dateStr, "index=%ld, pagehash=0x%08lx", i, hashid );
				Write_SessComment( hout, dateStr );
			}

			hashid = p->sessionStat->GetSessionPage( i );
			time = p->sessionStat->GetSessionTime( i );
			if ( hashid == SESS_START_PAGE )
			{				// START OF SESSION (1,time)
				if ( ((time/ONEDAY) - (firstDate/ONEDAY)) >= 1 )
				{
					Write_SessNewDayMarker( hout );
					days++;
				}
				DateTimeToString( time, dateStr );
				Write_SessName( hout , rgbcolor[(sitem)%2], sitem, dateStr );
				firstDate = time;
			}
			else if ( hashid == SESS_BYTES )
			{				// END OF SESSION (2,bytes)
				if ( !ptime )
					dur = 30;
				else
					dur = (ptime-firstDate) + 30;
				if ( phashid == SESS_START_PAGE && dur<0 )
					SessionTextSameXPosBold( hout, TranslateID(COMM_NOPAGESVIEWED) );
				else
				{
					if ( dur > 0 )
					{
						CTimetoString( dur, timeStr );
						Write_SessLength( hout , timeStr );
					}
					else
					{
						sprintf( timeStr, "Duration < 0 = %ld, (%08lx)", dur, dur );
						Write_SessComment( hout, timeStr );
					}
				}
			}
			else if ( hashid == SESS_TIME )
			{				// END OF SESSION (3,time)
				Write_SessColumnSummary( hout , TranslateID(SESS_BYTESUSED), (unsigned long)ptime );
				if ( sessionDownloads )
				{
					ValuetoString( TYPE_BYTES, sessionBytes, sessionBytes, timeStr );
					sprintf( tempSpec, "Attempted Downloads %ld (%s)", sessionDownloads, timeStr );
					SessionText( tempSpec, DataSize() );
					sessionDownloads = sessionBytes = 0;
				}
				Stat_WriteSessTableEnd( hout );
				SessionTextColumnBlankLine();
				depth = 1;
				sitem++;
			}
			else if ( hashid == SESS_START_REFERAL )
			{				// REFERAL AT SESSION (4,referal)
				if ( VDptr->byRefer )
				{
					long	bytes = 0;
					url = VDptr->byRefer->GetStatDetails( time, &pStat, &bytes );
					if ( url )
						Write_SessPage( hout , 0, dateStr, thishost, url, 0 );
				}
			}
			else if ( hashid<SESS_START_PAGE || hashid>SESS_ABOVE_IS_PAGE )
			{	// display page used within session (hash,time)
				long	bytes = 0;

				url = VDptr->byFile->GetStatDetails( hashid, &pStat, &bytes );

				if ( url )
				{
					char *urlext;
					int dopage = 0;
					DateTimeToString( time, dateStr );

					if ( urlext = strrchr( url, '.' ) )
					{
						if ( !strcmpExtensions( urlext, MyPrefStruct.downloadStr ) )
						{
							totalDownloads++;
							totaldownloadbytes += bytes;
							sessionDownloads++;
							sessionBytes += bytes;
							dopage = -1;
						}
					}
					
					// only display the set amount of pages
					if( numPagesFound++<numPagesToDisplay )
					{
						if ( VDptr->logStyle == LOGFORMAT_FIREWALLS )
							Write_SessPage( hout , depth, dateStr, thishost, url, pStat->GetProtocolType() );
						else
							Write_SessPage( hout , depth, dateStr, thishost, url, dopage );
					}

					depth++;
				}
				else
				{
					if (IsDebugMode())
					{
						char	sz[1024];
						sprintf(sz, "Can not find HashId[%d]", hashid);
						Write_SessPage( hout , depth, dateStr, thishost, sz, 0 );
					}
				}
			}
			else
			{
				DEF_ASSERT(false);	// shouldn't get to here
			}
			ptime = time;
			phashid = hashid;
		} //for loop

		// plot last sessions summary if its incompleted
		if ( hashid!=SESS_TIME )
		{
			dur = (time-firstDate) + 30;
			CTimetoString( dur, timeStr );
			Write_SessLength( hout , timeStr );
			Write_SessColumnSummary( hout , TranslateID(SESS_BYTESUSED), (unsigned long)p->sessionStat->SessionBytes );	//
			if ( sessionDownloads ){
				ValuetoString( TYPE_BYTES, sessionBytes, sessionBytes, timeStr );
				sprintf( tempSpec, "Attempted Downloads %ld (%s)", sessionDownloads, timeStr );
				SessionText( tempSpec, DataSize() );
				sessionDownloads = sessionBytes = 0;
			}
			Write_SessNewDayMarker( hout );
			SessionTextColumnBlankLine();
		}

		SessionText( PDF_BLANKLINE, DataSize(), PDF_NON_COLUMN );

		// show all the sessions summary for the client
		if ( days )
		{
			Write_SessTotalBand( hout , (unsigned long)byStat->GetBytes(unit), (unsigned long)byStat->GetBytesIn(unit), days );
			Write_SessTotalTime( hout , p->visitTot, days );
		}
		if ( numPagesFound )
		{
			sprintf( tempSpec, "Average pages per session is %0.2f", numPagesFound / (float)p->visits );
			std::string temp = tempSpec;
			SessionTextSameXPosBold( hout, temp );
		}
		if ( totalDownloads )
		{
			ValuetoString( TYPE_BYTES, totaldownloadbytes, totaldownloadbytes, timeStr );
			sprintf( tempSpec, "Total Attempted Downloads %ld (%s)", totalDownloads, timeStr );
			std::string temp = tempSpec;
			SessionTextSameXPosBold( hout, temp );
		}
	}

	WritePageFooter( VDptr, hout );
	if( !SingleFileOutput() )
		Fclose( hout );

	return filename;
}


void CQPDFSessionWriter::SessionOutline( const char *text )
{
	PDFTextList textList;
	textList.push_back( PDFTextStr( text, 0, PDF_TEXT_OUTLINE, PDF_BOLD, TitleSize() ) );
	clickStreamTextLists.push_back( textList );
}

void CQPDFSessionWriter::SessionHeading( std::string text )
{
	//PDFTextList textList;
	//textList;
	//SessionText( text, thePDFTableSettings.TitleSize(), PDF_NEW_SECTION, PDF_BOLD );
	//textList.push_back( PDFTextStr( text, 0, PDF_NEW_SECTION, PDF_BOLD, thePDFTableSettings.TitleSize() ) );
	//clickStreamTextLists.push_back( textList );
	if ( clickStreamTextLists.size() )
	{
		PDFTextList& textList = clickStreamTextLists.back();
		textList.push_back( PDFTextStr( text, 0, PDF_NEW_SECTION, PDF_BOLD, TitleSize() ) );
	}
}

/*void CQPDFSessionWriter::SessionTextNormal( std::string text )
{
	SessionTextNormal( text, DataSize(), PDF_TEXT_COLUMN, PDF_NORMAL );
}

void CQPDFSessionWriter::SessionTextBold( std::string text )
{
	SessionTextNormal( text, DataSize(), PDF_TEXT_COLUMN, PDF_BOLD );
}*/

void CQPDFSessionWriter::SessionText( std::string text, int size /*= 6*/, int xPos /*= PDF_TEXT_COLUMN*/, int state /*= PDF_NORMAL*/ )
{
	if ( clickStreamTextLists.size() )
	{
		PDFTextList& textList = clickStreamTextLists.back();
		/*int maxTextSize;
		if ( xPos == PDF_NON_COLUMN )
			maxTextSize = SubPageDrawWidth();
		else if ( xPos == PDF_TEXT_COLUMN )
			maxTextSize = SubPageDrawWidth()/2 - 10;
		else // For safety... but shouldn't really take this path... 
			maxTextSize = SubPageDrawWidth();
		while ( timesNRFont->GetStringLen( text.c_str(), size, state ) >= maxTextSize )
		{
			std::string splitText;
			splitText = timesNRFont->SetStringLen( text.c_str(), maxTextSize, size, PDF_LEFTJUSTIFY, state );
			textList.push_back( PDFTextStr( splitText, xPos, size, state, size ) );
			text.erase( 0, splitText.length() );
		}*/
		if ( text.length() == 0 )
			textList.push_back( PDFTextStr( text, xPos, size, state, size ) );
		else
			textList.push_back( PDFTextStr( text, xPos, PDF_TEXT_ON_NEXT_LINE, state, size ) );
	}
}


void CQPDFSessionWriter::Write_SessBold( FILE *fp, const char *txt )
{
	SessionText( txt, DataSize(), PDF_NON_COLUMN, PDF_BOLD );
}

void CQPDFSessionWriter::Write_SessColumnBold( FILE *fp, const char *txt )
{
	SessionText( txt, DataSize(), PDF_TEXT_COLUMN, PDF_BOLD );
}