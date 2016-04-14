
#ifndef REPORTEXCEL_H
#define REPORTEXCEL_H

#include "ReportClass.h"


class excelFile : public baseFile 
{
public:
	excelFile();
	~excelFile();
	virtual int Init( const char *fileName = "", FILE *filePtr = NULL );
	virtual void Stat_WriteBold( FILE *fp, const char *txt );
	virtual void Stat_WriteLine( FILE *fp, const char *txt );
	virtual void Stat_WriteRowStart( FILE *fp, long rgb, long col );
	virtual void Stat_WriteRowEnd( FILE *fp );
	virtual void Stat_WriteTableEnd( FILE *fp );
	virtual void Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title );
	virtual void Stat_WriteNumberData( FILE *fp, long num );
	virtual void Stat_WriteFractionData( FILE *fp, long num, long num2 );
	virtual void Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name );
	virtual void WritePageTitle( VDinfoP VDptr, FILE *fh, const char *title );
	virtual void WriteFilterText(FILE* fp, const char* szFilterString);
	virtual void WritePageFooter( VDinfoP VDptr, FILE *fh );
	virtual void WriteDemoBanner( VDinfoP VDptr, FILE *fp );
	virtual void SummaryTableEntryURL( FILE *fp, const char *text, const char *url, long normal );
	virtual void SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal );
	virtual void Stat_WriteText( FILE *fp, short cols, long rgb, const char *name );
	virtual void Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name );
	virtual void Stat_WriteReqData( FILE *fp, long files, double perc );
	virtual void Stat_WriteByteData( FILE *fp, __int64 bytes, double perc );
	virtual void Stat_WriteHeader( FILE *fp, const char *txt, long space );
 	virtual void SummaryTableTitle( FILE *fp, const char *text, long rgb );
	virtual void SummaryTableSubTitle( FILE *fp, const char *text, int x, int y );
	virtual void FinishReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs );
	virtual void writeTextRightWithRLBorder( const char *text, FILE *fp );
	virtual void writeMultiRow( long currentRow, const char *pathName, FILE *fp );
	virtual void writeNumberRightWithRLBorder( double num, FILE *fp );
	virtual void initAreaRow();
	virtual void initColumn();
	virtual void writeNextData();

	virtual int ReportTurnedOn( long id );

protected:
	virtual short SingleFileOutput() { return 1; }
	
private:
	void writeToExcel( const char *text, FILE *fp );
	void writeFooter( FILE *fp );
	void writeBOFExcel( FILE *fp );
	void writeTextGeneral( const char *text, FILE *fp );
	void writeTextRight( const char *text, FILE *fp );
	void writeTextCenter( const char *text, FILE *fp );
	void writeTextGeneralWithRLBorder( const char *text, FILE *fp );
	void writeTextCenterWith4Border( const char *text, FILE *fp );
	void writeTableBottomLine( short num, FILE *fp );
	void writePageBreak( FILE *fp );
	void writeNoFormulaData( FILE *fp );
	void setUpFont( FILE *fp );
	void writeTextFont14( const char* text, FILE *fp );

	void formatToIEEE(double value);
	void convertIntDecPoint(double value);
	void convertExp(double expnt);
	void convertToHex();

	short columth, rowth, areaFirstRow; 
	short tablewidth; 
	unsigned char RGBAttr[3];
	FILE* file;
	
	enum {bit64=64,cnst=2102};

	unsigned char binalValue[cnst];
	unsigned char result[bit64];
	unsigned char IEEEData[8];  // 64 bits data result for IEEE format
	
	
};


class CQExcelSessionWriter : public CQSessionWriter
{
public:
	CQExcelSessionWriter( const char *fileExtensionP, excelFile& myExcelFile )
		: CQSessionWriter( fileExtensionP, myExcelFile ){}
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
	virtual char *WriteSessions( void *vp, StatList *byStat, char *filename, const char *title, const char *thishost, const char *client, short unit, int listdepth ){ return 0; }
};


 
#endif REPORTEXCEL_H

