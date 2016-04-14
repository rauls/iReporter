
#ifndef REPORTPDF_SESSIONS_H
#define REPORTPDF_SESSIONS_H

#include <list>
#include "ReportSessions.h"
#include "PDFFont.h"
#include "PDFSettings.h"

typedef std::list<PDFTextList> ClickStreamTextLists;

class pdfFile;

class CQPDFSessionWriter : public CQSessionWriter
{
public:
	CQPDFSessionWriter( const char *fileExtensionP, pdfFile& myPdfFile, PDFTableSettings thePDFTableSettingsP, FILE *theFPP, int subPageDrawWidthP );
protected:
	virtual void Write_SessHeader( FILE *fp , const char *client, const char *timeStr, const char *b, const char *o );
	virtual void Write_SessItal( FILE *fp, const char *txt );
	virtual void Write_SessTotalBand( FILE *fp , __int64 b1, __int64 b2, long days );
	virtual void Write_SessTotalTime( FILE *fp , long time, long days );
	virtual void Write_SessLength( FILE *fp , const char *timeStr );
	virtual void Write_SessName( FILE *fp , long rgb, long sitem, const char *dateStr );
	virtual void Write_SessPage( FILE *fp , long item, const char *dateStr, const char *thishost, const char *url, long proto );

	virtual void SessionTextSameXPosBold( FILE *fp, std::string text );
	virtual void SessionTextColumnBlankLine();
	virtual char *WriteSessions( void *vp, StatList *byStat, char *filename, const char *title, const char *thishost, const char *client, short unit, int listdepth );//{ return "PDF Sess"; }
	virtual void Write_SessBold( FILE *fp, const char *txt );
	virtual void Write_SessColumnBold( FILE *fp, const char *txt );
	virtual short SingleFileOutput() { return 1; }

protected:
	void SessionOutline( const char *text );
	void SessionHeading( std::string text );
	void SessionText( std::string text, int size = 6, int xPos = PDF_TEXT_COLUMN, int state = PDF_NORMAL );
	int ColHeadingSize() { return thePDFTableSettings.ColHeadingSize(); }
	int TitleSize() { return thePDFTableSettings.TitleSize(); }
	int DataSize() { return thePDFTableSettings.DataSize(); }
	int SubPageDrawWidth() { return subPageDrawWidth; }
	char * intToStr( int num )
	{
		static char buf[32];
		sprintf( buf, "%d", num );
		return buf;
	}
public:
	ClickStreamTextLists clickStreamTextLists;
private:
	PDFTableSettings thePDFTableSettings;
	FILE *theFP;
	int subPageDrawWidth;
};

#endif // REPORTPDF_SESSIONS_H

