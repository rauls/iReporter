#include "FWA.h"

#include <stdarg.h>

#include "PDFTableData.h"
#include "ReportSessions.h"
#include "ReportFuncs.h"
#include "translate.h"
#include "config.h"
#include "EngineParse.h"
#include "engine_proc.h"
#include "engine_drill.h"
#include "StatDefs.h"
#include "FileTypes.h"

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

CQSessionWriter::CQSessionWriter( const char *fileExtensionP, baseFile& myBaseFile )
:	fileExtension(fileExtensionP),
	m_baseFile(myBaseFile)
{
}

CQSessionWriter::~CQSessionWriter()
{
}

int CQSessionWriter::Fprintf( FILE *fp, const char *fmt, ... )
{
	va_list		args;
static	char lineout[10240];

	va_start( args, fmt );
	vsprintf( lineout, fmt, args );
	va_end( args );

	return WriteLine( fp, lineout );
}

void CQSessionWriter::Write_SessTotalBand( FILE *fp, __int64 b1, __int64 b2, long days )
{
	Write_SessSummary( fp, TranslateID(SESS_TOTALDAYS), days );
	Write_SessSummary( fp, TranslateID(SESS_TOTALBYTESUSED), b1 );
	Write_SessSummary( fp, TranslateID(SESS_TOTALBYTESSENT), b2 );
	if ( days > 1 ){
		Write_SessSummary( fp, TranslateID(SESS_AVGBYTESPERDAY), b1/days );
	}
}

void CQSessionWriter::Write_SessTotalTime( FILE *fp, long time, long days )
{
	char	numStr[32];

	CTimetoString( time, numStr );
	Write_SessSummary( fp, TranslateID(SESS_TOTALONLINETIME), numStr );

	if ( days > 1 ){
		CTimetoString( time/days, numStr );
		Write_SessSummary( fp, TranslateID(SESS_AVGONLINETIME), numStr );
	}
}

void CQSessionWriter::Write_SessSummary( FILE *fp , const char *txt, __int64 b1, int grandTotal )
{
	char numStr[32], out[256];
	if ( b1)
	{
		ValuetoString( TYPE_BYTES, b1, b1, numStr );
		sprintf( out, "%s %s", txt, numStr );
		Write_SessBold( fp, out );
	}
}

void CQSessionWriter::Write_SessSummary( FILE *fp , const char *txt, const char *txt2, int grandTotal )
{
	char out[256];
	sprintf( out, "%s %s", txt, txt2 );
	Write_SessBold( fp, out );
}

void CQSessionWriter::Write_SessColumnSummary( FILE *fp , const char *txt, __int64 b1, int grandTotal )
{
	char numStr[32], out[256];
	if ( b1)
	{
		ValuetoString( TYPE_BYTES, b1, b1, numStr );
		sprintf( out, "%s %s", txt, numStr );
		Write_SessColumnBold( fp, out );
	}
}

void CQSessionWriter::Write_SessColumnSummary( FILE *fp , const char *txt, const char *txt2, int grandTotal )
{
	char out[256];
	sprintf( out, "%s %s", txt, txt2 );
	Write_SessColumnBold( fp, out );
}


void CQSessionWriter::Write_SessLength( FILE *fp , const char *timeStr )
{
	std::string str;
	str += TranslateID( SESS_ESTSESSLENGTH );
	str += timeStr;
	str += " ";
	if ( timeStr ){
	str += TranslateID( SESS_ESTSESSLENGTH );
		Write_SessSummary( fp, TranslateID(SESS_ESTSESSLENGTH), timeStr );
	}
}

void CQSessionWriter::Write_SessPage( FILE *fp , long item, const char *dateStr, const char *thishost, const char *url, long proto )
{

	char *p, c = ' ';

	if ( proto == -1 )
		c = '*';

	if ( strstr( url, "://" ) )		// if the link already has a fully defined root URL
		mystrcpy( tempBuff2, url );
	else
		MakeURL( tempBuff2, thishost, url );

	if ( proto>0 )
		p = protocolStrings[proto];
	else 
		p = " ";

	//switch( m_style ){
		//case FORMAT_HTML:
				if ( proto>0 )
					Fprintf( fp, "%d. %s <b>%s</b> <a href=\"%s\">%s</a>%c<br>\n", item, dateStr, p, tempBuff2, url, c );
				else
				if ( item == 0 )
					Fprintf( fp, "<b> %s:  <a href=\"%s\">%s</a>%c </b> <br>\n", TranslateID(LABEL_REFERRAL), url, url, c );
				else
					Fprintf( fp, "%d. %s <a href=\"%s\">%s</a>%c<br>\n", item, dateStr, tempBuff2, url, c );
				//break;
		//case FORMAT_COMMA:	Fprintf( fp, "%d,%s,%s,%s %c\n", item, dateStr, p, tempBuff2, c ); break;
	//}
}

void CQSessionWriter::SessionTextSameXPosBold( FILE *fp, std::string text )
{
	Write_SessItal( fp, text.c_str() ); 
	//Stat_WriteItal( fp, text.c_str() );
}

void CQSessionWriter::SessionStart( FILE *hout, long &time, long &firstDate, long &days, long sitem, long rgbcolor[], char *dateStr )
{
	if ( ((time/ONEDAY) - (firstDate/ONEDAY)) >= 1 )
	{
		Write_SessNewDayMarker( hout );
		days++;
	}
	DateTimeToString( time, dateStr );
	Write_SessName( hout , rgbcolor[(sitem%2)], sitem, dateStr );
	firstDate = time;
}

void CQSessionWriter::SessionLength( FILE *hout, long ptime, long firstDate, long phashid, char *timeStr )
{
	long dur;
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
			Write_SessLength( hout, timeStr );
		}
		else
		{
			sprintf( timeStr, "Duration < 0 = %ld, (%08lx)", dur, dur );
			Write_SessComment( hout, timeStr );
		}
	}
}


void CQSessionWriter::SessionBandwidth( FILE *hout, long ptime, long firstDate, long sessionDownloads, long sessionBytes, char* timeStr, char *tempSpec, long &depth, long &sitem )
{
	Write_SessColumnSummary( hout, TranslateID(SESS_BYTESUSED), (unsigned long)ptime );
	if ( sessionDownloads )
	{
		ValuetoString( TYPE_BYTES, sessionBytes, sessionBytes, timeStr );
		sprintf( tempSpec, "%ld (%s)", sessionDownloads, timeStr );
		Write_SessSummary( hout, TranslateID(SESS_ATTEPTDOWNLOADS), tempSpec );
		sessionDownloads = sessionBytes = 0;
	}
	Stat_WriteSessTableEnd( hout );
	SessionTextColumnBlankLine();
	depth = 1;
	sitem++;
}

void CQSessionWriter::SessionURL( FILE *hout, VDinfoP VDptr, long time, long item, const char* dateStr, const char *thishost )
{
	if ( VDptr->byRefer )
	{
		long	bytes = 0;
		char *url = VDptr->byRefer->GetStatDetails( time, NULL, &bytes );
		if ( url )
			Write_SessPage( hout, 0, dateStr, thishost, url, 0 );
	}
}

char *CQSessionWriter::WriteSessions( void *vp, StatList *byStat, char *filename, const char *title, const char *thishost, const char *client, short unit, int listdepth )
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
	// Append the filename extension e.g. for html we add "-#.html"
	char* end = newfile + strlen( newfile );
	sprintf( end, "-%ld%s", unit, fileExtension );
	mystrcpy( tempSpec, newfile);
	hout = Fopen( tempSpec, "w+" );

	if ( hout )
	{
		char *browser, *opersys, timeStr[256];

		if ( VDptr->byBrowser )
			browser = VDptr->byBrowser->GetName( p->counter2 );
		else
			browser = NULL;
		if ( VDptr->byOperSys )
			opersys = VDptr->byOperSys->GetName( p->counter3 );
		else
			opersys = NULL;
		WritePageTitle( VDptr, hout, title );
		CTimetoString( p->visitTot / p->visits, timeStr );
		Write_SessHeader( hout , client, timeStr, browser, opersys );

		depth = 1;
		sitem = 1;

		PROGRESS_IDLE

		long numSessionEntries(p->sessionStat->GetNum());
		
		if ( numSessionEntries )
		{
			char	dateStr[256], *url;
			long	firstDate=0, dur, time, ptime=0;
			long	hashid, phashid=SESS_START_PAGE, page;
			long	rgbcolor[]={ -1, 0xe0e0e0, 0, 0 };
			long	days=0, numPagesFound = 0, totalDownloads = 0, sessionDownloads=0;
			long	totaldownloadbytes = 0, sessionBytes = 0;

			long numPagesToDisplay( numSessionEntries );

			// limit the session output so its not 50meg html file.
			if (numPagesToDisplay > MyPrefStruct.sessionLimit && MyPrefStruct.sessionLimit )
				numPagesToDisplay = MyPrefStruct.sessionLimit;

			dateStr[0] = 0;
			days = 0;
			for ( page=0; page < numSessionEntries; page++ )
			{
				if ( IsDebugMode() )
				{
					sprintf( dateStr, "index=%ld, pagehash=0x%08lx", page, hashid );
					Write_SessComment( hout, dateStr );
				}

				hashid = p->sessionStat->GetSessionPage( page );
				time = p->sessionStat->GetSessionTime( page );
				switch( hashid )
				{
					case SESS_START_PAGE:// START OF SESSION (1,time)
						SessionStart( hout, time, firstDate, days, sitem, rgbcolor, dateStr );
						break;
					case SESS_BYTES:// END OF SESSION (2,bytes)
						SessionLength( hout, ptime, firstDate, phashid, timeStr );
						break;
					case SESS_TIME:// END OF SESSION (3,time)
						SessionBandwidth( hout, ptime, firstDate, sessionDownloads, sessionBytes, timeStr, tempSpec, depth, sitem );
						break;
					case SESS_START_REFERAL:// REFERAL AT SESSION (4,referal)
						SessionURL( hout, VDptr, time, 0, dateStr, thishost );	// the 0 arguement is ignored.
						break;
				}
				if ( hashid<SESS_START_PAGE || hashid>SESS_ABOVE_IS_PAGE )
				{	// display page used within session (hash,time)
					long	bytes = 0;
					Statistic*	pStat = NULL;

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
				ptime = time;
				phashid = hashid;
			} //for loop

			// plot last sessions summary if its incompleted
			if ( hashid!=SESS_TIME )
			{
				dur = (time-firstDate) + 30;
				CTimetoString( dur, timeStr );
				Write_SessLength( hout , timeStr );
				Write_SessColumnSummary( hout, TranslateID(SESS_BYTESUSED), (unsigned long)p->sessionStat->SessionBytes );	//
				if ( sessionDownloads ){
					ValuetoString( TYPE_BYTES, sessionBytes, sessionBytes, timeStr );
					sprintf( tempSpec, "%ld (%s)", sessionDownloads, timeStr );

					Write_SessSummary( hout, TranslateID(SESS_ATTEPTDOWNLOADS), tempSpec );
					sessionDownloads = sessionBytes = 0;
				}
				Write_SessNewDayMarker( hout );
			}

			// show all the sessions summary for the client
			if ( days )
			{
				Write_SessTotalBand( hout , byStat->GetBytes(unit), byStat->GetBytesIn(unit), days );
				Write_SessTotalTime( hout , p->visitTot, days );
			}
			if ( numPagesFound ) {
				sprintf( tempSpec, "%0.2f", (float)(numPagesFound / (float)p->visits) );
				Write_SessSummary( hout, TranslateID(SESS_AVGPAGESPERSESSION), tempSpec );
			}
			if ( totalDownloads ) {
				ValuetoString( TYPE_BYTES, totaldownloadbytes, totaldownloadbytes, timeStr );
				sprintf( tempSpec, "%ld (%s)", totalDownloads, timeStr );

				Write_SessSummary( hout, TranslateID(SESS_TOTALATTEPTDOWNLOADS), tempSpec );
			}
		} //if ( numSessionEntries ) {


		WritePageFooter( VDptr, hout );
		if( !SingleFileOutput() )
			Fclose( hout );
	}//if ( hout = Fopen( tempSpec, "w+" ) ){
	return filename;
}

