
#ifndef REPORTCOMMA_H
#define REPORTCOMMA_H

#include "ReportClass.h"

class commaFile : public baseFile
{
public:

	commaFile();
	~commaFile();

private:

	// table elements
	virtual void Stat_WriteRowEnd( FILE *fp );
	virtual void Stat_WriteTableEnd( FILE *fp );
	virtual void Stat_WriteHeader( FILE *fp, const char *txt, long space );
	
	// filters.
	virtual void WriteFilterText( FILE *fp, const char *szFilterString);

	// text formatting
	virtual void Stat_WriteText( FILE *fp, short cols, long rgb, const char *name );
	virtual void Stat_WriteLink( FILE *fp, const char *url, const char *name, long num );
	virtual void Stat_WriteItal( FILE *fp, const char *txt );
	virtual void Stat_WriteBold( FILE *fp, const char *txt );
	virtual void Stat_WriteLine( FILE *fp, const char *txt);
	virtual void Stat_WriteNumberData( FILE *fp, long num );
	virtual void Stat_WriteFractionData( FILE *fp, long num, long num2 );
	virtual void Stat_WriteFloatData( FILE *fp, double num );
	virtual void Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteTextTooltip( FILE *fp, short cols, const char *name, const char *tooltip );
	virtual void Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name );

//	char *GetCIACountryURL( char *cname, char *outname );
//	char *GetEngineLogo( FILE *fp, char *engine, char *p );
//	void Stat_WriteSpace( FILE *fp, long size );
//	void Stat_WriteAnchor( FILE *fp, long a );

//	virtual void WriteGraphStat( VDinfoP VDptr, StatList *byStat , long id, long typeID, char *filename, char *title, char *header, short sort, short depth );

//	virtual void WriteReportSummary( VDinfoP VDptr, FILE *fp, char *outfile, long logNum ){} // Don't do a summary report for Comma Delimited
	virtual void Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title );
	virtual void WriteText( FILE *fp, const char *textString );
	virtual void WriteTextLine( FILE *fp, const char *textString );
	virtual void Stat_WriteDualText( FILE *fp, const char *name, const char *name2 );
	virtual void Stat_WriteBottomHeader( FILE *fp, const char *txt, long space );
	virtual void SummaryTableTitle( FILE *fp, const char *text, long rgb );
	virtual void SummaryTableStart( FILE *fp, long width, long centre );
	virtual void SummaryTableFinish( FILE *fp );
	virtual void SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal );
	virtual void SummaryTableSubTitle( FILE *fp, const char *text, int x, int y );
	virtual void ProduceSummaryTitle( FILE *fp, int logNum, const char *domainName );
	virtual void WritePageFooter( VDinfoP VDptr, FILE *fh );
protected:
	virtual short SingleFileOutput() { return 1; }
//	virtual void Stat_WriteCentreHeading( FILE *fp, const char *txt );

protected:
	int numOfCols;
	int dataColNum;
	short writingSummaryTable;
};


class CQCommaSessionWriter : public CQSessionWriter
{
public:
	CQCommaSessionWriter( const char *fileExtensionP, commaFile& myCommaFile )
		: CQSessionWriter( fileExtensionP, myCommaFile ){}

protected:
/*	virtual void Write_SessHeader( FILE *fp , const char *client, const char *timeStr, const char *b, const char *o ){}
	virtual void Write_SessItal( FILE *fp, const char *txt );
	virtual void Write_SessSummary( FILE *fp , const char *txt, __int64 b1, int grandTotal = 0 ){}
	virtual void Write_SessTotalBand( FILE *fp , __int64 b1, __int64 b2, long days );
	virtual void Write_SessTotalTime( FILE *fp , long time, long days );
	virtual void Write_SessLength( FILE *fp , const char *timeStr );
	virtual void Write_SessName( FILE *fp , long rgb, long sitem, const char *dateStr ){}
	virtual void Write_SessNewDayMarker( FILE *fp );
	virtual void Stat_WriteSessTableEnd( FILE *fp );
//	virtual void Write_SessPage( FILE *fp , long item, const char *dateStr, const char *thishost, const char *url, long proto );
//	virtual void SessionTextSameXPosBold( FILE *fp, std::string text );
//	virtual void SessionTextBlankLine(){}
//	virtual void CreateSessionFilename( FILE *hout, char *newfile, char *tempSpec, char *filename, VDinfoP VDptr, short unit );
//	virtual void WriteSessionsList(){}
	virtual char *WriteSessions( void *vp, StatList *byStat, char *filename, const char *title, const char *thishost, const char *client, short unit, int listdepth );
*/
/*	virtual void Write_SessHeader( FILE *fp, const char *client, const char *timeStr, const char *b, const char *o );
	virtual void Write_SessSummary( FILE *fp, const char *txt, __int64 b1, int grandTotal = 0 );
	virtual void Write_SessTotalTime( FILE *fp, long time, long days );
	virtual void Write_SessLength( FILE *fp, const char *timeStr );
	virtual void Write_SessName( FILE *fp , long rgb, long sitem, const char *dateStr );*/
	virtual char *WriteSessions( void *vp, StatList *byStat, char *filename, const char *title, const char *thishost, const char *client, short unit, int listdepth ){ return 0; }

};


#endif // REPORTCOMMA_H

