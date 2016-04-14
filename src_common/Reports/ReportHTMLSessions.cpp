
#include "FWA.h"

#include <stdarg.h>
#include <string.h>

#include "ReportHTMLSessions.h"
#include "ReportHTML.h"
#include "ReportFuncs.h"
#include "HTMLFormat.h"
#include "translate.h"
#include "config.h"
#include "EngineParse.h"
#include "engine_proc.h"
#include "engine_drill.h"
#include "StatDefs.h"
#include "Report.h"

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
extern long ScaleRGB( double perc,long rgb );


CQHTMLSessionWriter::CQHTMLSessionWriter( const char *fileExtensionP, htmlFile& myHtmlFile )
:	CQSessionWriter( fileExtensionP, myHtmlFile )
{
}


void CQHTMLSessionWriter::Stat_WriteSessTableEnd( FILE *fp )
{
	Fprintf(fp,"</td></tr>\n" );
}

void CQHTMLSessionWriter::Write_SessItal( FILE *fp, const char *txt )
{
	Fprintf( fp, "<i>%s</i><br>\n", txt );
}


void CQHTMLSessionWriter::Write_Header( FILE *fp, const char *title )
{
	Fprintf (fp, "<html>\n<head>\n<title>%s</title>\n</head>\n\n<CENTER>", title );
}

char *CQHTMLSessionWriter::WriteSessions( void *vp, StatList *byStat, char *filename, const char *title, const char *thishost, const char *client, short unit, int listdepth )
{
	static char newfile[MAXURLSIZE];
	char		tempSpec[MAXURLSIZE];
	char 		*end;
	long		depth, sitem, n;
	FILE		*hout;
	VDinfoP	VDptr = (VDinfoP)vp;
	register Statistic	*p;

	p = byStat->GetStat(unit);
	
	if ( !p->sessionStat->Session )
		return 0;

	strcpy(tempSpec,MyPrefStruct.outfile);
	if ( MyPrefStruct.ShowVirtualHostGraph() )
	{
		if (GetNewOutpath( VDptr, MyPrefStruct.outfile, newfile ))
			strcat( newfile, PATHSEPSTR );
		strcat( newfile, filename );
		filename = FileFromPath( newfile,0 );
	}
	else
	{
		CopyFilenameUsingPath( newfile, MyPrefStruct.outfile, filename);
		filename = FileFromPath( newfile,0 );
	}
	// append the -#.html
	end = newfile + strlen( newfile );
	sprintf( end, "-%d%s", unit, fileExtension );
	mystrcpy( tempSpec, newfile);

	n = unit;
	
	hout = Fopen( tempSpec, "w+" );

	if ( hout)
	{
		char *browser, *opersys, timeStr[256];

		if ( VDptr->byBrowser )	browser = VDptr->byBrowser->GetName( p->counter2 ); else browser = NULL;
		if ( VDptr->byOperSys )	opersys = VDptr->byOperSys->GetName( p->counter3 ); else opersys = NULL;

		Write_Header( hout, title );
		if( title )
		{
			WriteLine( hout, MyPrefStruct.html_head );
		}

		CTimetoString( p->visitTot / p->visits, timeStr );
		Write_SessHeader( hout , client, timeStr, browser, opersys );

		depth = 1;
		sitem = 1;

		PROGRESS_IDLE

		long numSessionEntries( p->sessionStat->GetNum() );

		if( numSessionEntries )
		{
			char	dateStr[256], *url;
			long	firstDate=0, dur, time, ptime=0;
			long	hashid, phashid=SESS_START_PAGE;
			long	rgbcolor[]={ -1, 0xe0e0e0, 0, 0 };
			long	days=0, numPagesFound = 0, totalDownloads = 0, sessionDownloads=0;
			long	totaldownloadbytes = 0, sessionBytes = 0, statCode = 0;
			Statistic* pStat = NULL;

			// make the background 90% darker
			rgbcolor[1] = ScaleRGB( 90, MyPrefStruct.RGBtable[7] );

			long numPagesToDisplay( numSessionEntries );

			// limit the session output so its not 50meg html file.
			if (numPagesToDisplay > MyPrefStruct.sessionLimit && MyPrefStruct.sessionLimit )
				numPagesToDisplay = MyPrefStruct.sessionLimit;

			days = 0;
			for( long i(0); i<numSessionEntries; ++i )
			{
				hashid = p->sessionStat->GetSessionPage( i );
				time = p->sessionStat->GetSessionTime( i );

				if ( IsDebugMode() )
				{
					sprintf( dateStr, "%ld. pagehash=0x%08lx, param=0x%08lx", i, hashid, time );
					Write_SessComment( hout, dateStr );
				}

				if ( hashid == SESS_START_PAGE )
				{				// START OF SESSION (1,time)
					if ( ((time/ONEDAY) - (firstDate/ONEDAY)) >= 1 )
					{
						Write_SessNewDayMarker( hout );
						days++;
					}
					DateTimeToString( time, dateStr );
					Write_SessName( hout , rgbcolor[(sitem%2)], sitem, dateStr );
					firstDate = time;
				}
				else if ( hashid == SESS_BYTES )
				{				// END OF SESSION (2,bytes)
					if ( !ptime )
						dur = 30;
					else
						dur = (ptime-firstDate) + 30;
					if ( phashid == SESS_START_PAGE && dur<0 )
						Write_SessItal( hout, /*"No pages viewed during this session"*/ TranslateID(COMM_NOPAGESVIEWED) );
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
					Write_SessSummary( hout , TranslateID(SESS_BYTESUSED), (unsigned long)ptime );
					if ( sessionDownloads )
					{
						ValuetoString( TYPE_BYTES, sessionBytes, sessionBytes, timeStr );
						sprintf( tempSpec, "%d (%s)", sessionDownloads, timeStr );
						Write_SessSummary( hout, TranslateID(SESS_ATTEPTDOWNLOADS), tempSpec );
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
				else if ( hashid == SESS_ERROR_CODE )
				{
					statCode = time;
				}
				else if ( hashid<SESS_START_PAGE || hashid>SESS_ABOVE_IS_PAGE )
				{	// display page used within session (hash,time)
					long	bytes = 0;

					url = VDptr->byFile->GetStatDetails( hashid, &pStat, &bytes );

					if ( url )
					{
						int dopage = 0;
						DateTimeToString( time, dateStr );

						if ( pStat->counter2 == URLID_DOWNLOAD )
						{
							totalDownloads++;
							totaldownloadbytes += bytes;
							sessionDownloads++;
							sessionBytes += bytes;
							dopage = -1;
						}
						
						// only display the set amount of pages
						if( numPagesFound++<numPagesToDisplay )
						{
							if ( statCode )
								strcat( dateStr, " failed " );
							if ( VDptr->logStyle == LOGFORMAT_FIREWALLS )
								Write_SessPage( hout , depth, dateStr, thishost, url, pStat->GetProtocolType() );
							else
								Write_SessPage( hout , depth, dateStr, thishost, url, dopage );
						}
						depth++;
						statCode = 0;
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
			if ( hashid!=SESS_TIME ){
				dur = (time-firstDate) + 30;
				CTimetoString( dur, timeStr );
				Write_SessLength( hout , timeStr );
				Write_SessSummary( hout , TranslateID(SESS_BYTESUSED), (unsigned long)p->sessionStat->SessionBytes );	//
				if ( sessionDownloads ){
					ValuetoString( TYPE_BYTES, sessionBytes, sessionBytes, timeStr );
					sprintf( tempSpec, "%d (%s)", sessionDownloads, timeStr );

					Write_SessSummary( hout, TranslateID(SESS_ATTEPTDOWNLOADS), tempSpec );
					sessionDownloads = sessionBytes = 0;
				}
				Write_SessNewDayMarker( hout );
			}

			// show all the sessions summary for the client
			if ( days ){
				Write_SessTotalBand( hout , (unsigned long)byStat->GetBytes(unit), (unsigned long)byStat->GetBytesIn(unit), days );
				Write_SessTotalTime( hout , p->visitTot, days );
			}
			if ( numPagesFound ) {
				sprintf( tempSpec, "%0.2f", numPagesFound / (float)p->visits );
				Write_SessSummary( hout, TranslateID(SESS_AVGPAGESPERSESSION), tempSpec );
			}
			if ( totalDownloads ) {
				ValuetoString( TYPE_BYTES, totaldownloadbytes, totaldownloadbytes, timeStr );
				sprintf( tempSpec, "%d (%s)", totalDownloads, timeStr );

				Write_SessSummary( hout, TranslateID(SESS_TOTALATTEPTDOWNLOADS), tempSpec );
			}

			Fprintf( hout, GetHTMLFormat( OUT_VLISTTAIL ) );
		} //if ( numSessionEntries )

		WritePageFooter( VDptr, hout );
		Fclose( hout );
	}//if ( hout = Fopen( tempSpec, "w+" ) ){
	return filename;
}

void CQHTMLSessionWriter::Write_SessHeader( FILE *fp , const char *client, const char *timeStr, const char *browser, const char *opersys )
{
	Fprintf( fp, "%s <b>%s</b>, %s %s<br>", TranslateID( SESS_SESSLISTINGFOR ), client, TranslateID( SESS_AVGSESSLENGTH ), timeStr );
	if ( browser && opersys )
		Fprintf( fp, "%s <b>%s</b>, %s <b>%s</b><br>", TranslateID(SESS_BROWSERUSED), browser, TranslateID(SESS_OPERSYSUSED), opersys );
	Fprintf( fp, "<ul><br><center><font size=\"6\"><b><i>%s</i></b></font></center><br>\n\n", TranslateID( SESS_SESSLIST ) );
//	IncrementTableCount( fp );
	Fprintf( fp, "<table border=\"0\" width=\"90%%\">\n" );
}

void CQHTMLSessionWriter::Write_SessTotalBand( FILE *fp , __int64 b1, __int64 b2, long days )
{
	//char txt[256];
	//sprintf( txt, "%s %d", TranslateID( SESS_TOTALDAYS ), days );
	//Stat_WriteBold( fp, txt );
	Write_SessSummary( fp, TranslateID(SESS_TOTALDAYS), days );
	Write_SessSummary( fp, TranslateID(SESS_TOTALBYTESUSED), b1 );
	Write_SessSummary( fp, TranslateID(SESS_TOTALBYTESSENT), b2 );
	Write_SessSummary( fp, TranslateID(SESS_AVGBYTESPERDAY), b1/days );
}


void CQHTMLSessionWriter::Write_SessTotalTime( FILE *fp , long time, long days )
{
	char	numStr[32];
	//switch( m_style ){
	//	case FORMAT_HTML:
				CTimetoString( time, numStr );
				Fprintf( fp, "\n<b>%s %s</b><br>\n", TranslateID(SESS_TOTALONLINETIME), numStr );
				CTimetoString( time/days, numStr );
				Fprintf( fp, "\n<b>%s %s</b><br>\n", TranslateID(SESS_AVGONLINETIME),  numStr );
	/*			break;
		case FORMAT_COMMA:
				CTimetoString( time, numStr );
				Fprintf( fp, "\n%s %s\n", TranslateID(SESS_TOTALONLINETIME), numStr );
				CTimetoString( time/days, numStr );
				Fprintf( fp, "\n%s %s\n", TranslateID(SESS_AVGONLINETIME), numStr );
				break;
	}*/
}

void CQHTMLSessionWriter::Write_SessLength( FILE *fp , const char *timeStr )
{
	//switch( m_style ){
		//case FORMAT_HTML:
	Fprintf( fp, "\n<b>%s %s</b><br>\n", TranslateID(SESS_ESTSESSLENGTH), timeStr );
	//break;
	//	case FORMAT_COMMA:	Fprintf( fp, "\n%s %s\n\n", TranslateID(SESS_ESTSESSLENGTH), timeStr ); break;
	//}
}

void CQHTMLSessionWriter::Write_SessName( FILE *fp , long rgb, long sitem, const char *dateStr )
{
//	this code ...
	//Fprintf( fp, "<tr><td bgcolor=\"#%06lx\">",rgb );
	//Fprintf( fp, "<br>\n<b>Session #%d %s</b><br>\n", sitem, dateStr );
// replaced by ...
	Fprintf( fp, "<tr>\n\t<td><br><br></td>\n</tr>\n" );
	if ( rgb != -1 )
		Fprintf( fp, "<tr>\n\t<td bgcolor=\"#%06lx\" colspan=\"2\">", rgb );
	else
		Fprintf( fp, "<tr>\n\t<td colspan=\"2\">", rgb );

	SetHTMLFont( fp );
	Fprintf( fp, "<b>%s #%d</b> . <b>%s</b><br>\n", TranslateID(SESS_SESSION), sitem, dateStr );

}

void CQHTMLSessionWriter::Write_SessNewDayMarker( FILE *fp )
{
	Fprintf(fp,"<tr>\n\t<td colspan=\"2\"><hr><br></td>\n</tr>\n" );
}

void CQHTMLSessionWriter::Write_SessBold( FILE *fp, const char *txt )
{
	Fprintf( fp, "<b>%s</b><br>\n", txt );
}

void CQHTMLSessionWriter::Write_SessColumnBold( FILE *fp, const char *txt )
{
	Fprintf( fp, "<b>%s</b><br>\n", txt );
}

void CQHTMLSessionWriter::Write_SessComment( FILE *fp, const char *name  )
{
	if ( 1 ) // showComments
	{
		WriteLine( fp,"\n<!-- ");
		WriteLine( fp, name );
		WriteLine( fp," -->\n\n");
	}
}

void CQHTMLSessionWriter::SetHTMLFont( FILE *fp )
{
#ifdef DEFINEHTMLFONT
	sprintf( html_fonttag, "<font face=\"%s\" size=\"%d\">\n", MyPrefStruct.html_font, MyPrefStruct.html_fontsize );
	if ( fp )
		Fwrite( fp, html_fonttag );
#endif
}

void CQHTMLSessionWriter::WritePageFooter( VDinfoP, FILE* fp )
{
	WriteLine( fp, MyPrefStruct.html_foot );
}

int CQHTMLSessionWriter::Fprintf( FILE *fp, const char *fmt, ... )
{
	va_list		args;
static	char lineout[10240];

	va_start( args, fmt );
	vsprintf( lineout, fmt, args );
	va_end( args );

	return WriteLine( fp, lineout );
}

