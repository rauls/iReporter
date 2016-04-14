
#ifndef REPORTHTML_SESSIONS_H
#define REPORTHTML_SESSIONS_H

#include "ReportSessions.h"

class htmlFile;

class CQHTMLSessionWriter : public CQSessionWriter
{
public:
	CQHTMLSessionWriter( const char *fileExtensionP, htmlFile& myHtmlFile );

protected:
	virtual void SetHTMLFont( FILE *fp );
	virtual void Write_Header( FILE *fp, const char *title );
	virtual void Write_SessHeader( FILE *fp , const char *client, const char *timeStr, const char *b, const char *o );
	virtual void Write_SessItal( FILE *fp, const char *txt );
	virtual void Write_SessTotalBand( FILE *fp , __int64 b1, __int64 b2, long days );
	virtual void Write_SessTotalTime( FILE *fp , long time, long days );
	virtual void Write_SessLength( FILE *fp , const char *timeStr );
	virtual void Write_SessName( FILE *fp , long rgb, long sitem, const char *dateStr );
	virtual void Stat_WriteSessTableEnd( FILE *fp );
	virtual void Write_SessNewDayMarker( FILE *fp );
	virtual char *WriteSessions( void *vp, StatList *byStat, char *filename, const char *title, const char *thishost, const char *client, short unit, int listdepth );
	virtual void Write_SessBold( FILE *fp, const char *txt );
	virtual void Write_SessColumnBold( FILE *fp, const char *txt );
	virtual void Write_SessComment( FILE *fp, const char *name );
	virtual void WritePageFooter( VDinfoP VDptr, FILE *fh );
	virtual int Fprintf( FILE *fp, const char *fmt, ... );
};


#endif // REPORTHTML_SESSIONS_H

