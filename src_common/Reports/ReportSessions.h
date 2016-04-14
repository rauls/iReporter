
#ifndef REPORT_SESSIONS_H
#define REPORT_SESSIONS_H

#include <string>
#include <stdio.h>

class StatList;
#include "VirtDomInfo.h"
#include "myansi.h"

#define	MAXURLSIZE	16000

class baseFile;

class CQSessionWriter
{
public:
	CQSessionWriter( const char *fileExtensionP, baseFile& myBaseFile );
	virtual ~CQSessionWriter();
	void SessionStart( FILE *hout, long &time, long &firstDate, long &days, long sitem, long rgbcolor[], char *dateStr );
	void SessionLength( FILE *hout, long ptime, long firstDate, long phashid, char *timeStr );
	void SessionBandwidth( FILE *hout, long ptime, long firstDate, long sessionDownloads, long sessionBytes, char* timeStr, char *tempSpec, long &depth, long &sitem );
	void SessionURL( FILE *hout, VDinfoP VDptr, long time, long item, const char* dateStr, const char *thishost );
	virtual char *WriteSessions( void *vp, StatList *byStat, char *filename, const char *title, const char *thishost, const char *client, short unit, int listdepth );

protected:
	virtual void Write_SessHeader( FILE *fp , const char *client, const char *timeStr, const char *b, const char *o ){}
	virtual void Write_SessItal( FILE *fp, const char *txt ){}
	virtual void Write_SessSummary( FILE *fp , const char *txt, __int64 b1, int grandTotal = 0 );
	virtual void Write_SessSummary( FILE *fp , const char *txt, const char *txt2, int grandTotal = 0 );
	virtual void Write_SessColumnSummary( FILE *fp , const char *txt, __int64 b1, int grandTotal = 0 );
	virtual void Write_SessColumnSummary( FILE *fp , const char *txt, const char *txt2, int grandTotal = 0 );
	virtual void Write_SessTotalBand( FILE *fp , __int64 b1, __int64 b2, long days );
	virtual void Write_SessTotalTime( FILE *fp , long time, long days );
	virtual void Write_SessLength( FILE *fp , const char *timeStr );
	virtual void Write_SessName( FILE *fp , long rgb, long sitem, const char *dateStr ){}
	virtual void Write_SessNewDayMarker( FILE *fp ){}
	virtual void Stat_WriteSessTableEnd( FILE *fp ){}
	virtual void SessionTextSameXPosBold( FILE *fp, std::string text );
	virtual void SessionTextColumnBlankLine(){}
	virtual void Write_SessBold( FILE *fp, const char *txt ){}
	virtual void Write_SessColumnBold( FILE *fp, const char *txt ){}
	virtual void Write_SessComment( FILE *fp, const char *name ){}
//	virtual void SessionTextNormal( std::string text );
//	virtual void SessionTextBold( std::string text );

	// Must look at all these functions
	virtual void Write_SessPage( FILE *fp , long item, const char *dateStr, const char *thishost, const char *url, long proto );

	// Don't implement in derived classes
	virtual void SetHTMLFont( FILE *fp ){}

	// Must be implemented in derived classes, later
	virtual void WritePageTitle( VDinfoP VDptr, FILE *fh, const char *title ){} //= 0
	virtual void WritePageFooter( VDinfoP VDptr, FILE *fh ) {} //= 0
	virtual short SingleFileOutput() { return 0; }

protected:
	virtual int Fprintf( FILE *fp, const char *fmt, ... );
/*	int WriteLine( FILE *fp, const char *txt );
	long Fwrite( FILE *fp, const char *source );
	long WriteToStorage( const char *source, void *fp );*/
protected:
	char tempBuff2[10240];
	const char *fileExtension;
	baseFile& m_baseFile;
};

#endif // REPORT_SESSIONS_H
