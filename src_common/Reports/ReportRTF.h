
#ifndef REPORTRTF_H
#define REPORTRTF_H

#include "ReportClass.h"


class rtfFile : public baseFile
{
public:
	rtfFile();
	~rtfFile();
	virtual void FinishReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs );
	virtual void RTF_WriteRowDef( FILE *fp , long rgb, long num );
	virtual void WriteCellStartLeft( FILE *fp );
	virtual void WriteCellEnd( FILE *fp );
	virtual void RTF_WriteCell( FILE *fp, const char *txt );
	virtual void Stat_WriteSpace( FILE *fp, long );
	virtual void Stat_WriteItal( FILE *fp, const char *txt );
	virtual void Stat_WriteBold( FILE *fp, const char *txt );
	virtual void Stat_WriteBoldDual( FILE *fp, const char *txt, const char *txt2 );
	virtual void Stat_WriteLine( FILE *fp, const char *txt );
	virtual void Stat_WriteCenterSmall( FILE *fp, const char *txt );
	virtual void Stat_WriteCentreHeading( FILE *fp, const char *txt );
	virtual void Stat_WriteRowStart( FILE *fp, long rgb, long col );
	virtual void Stat_WriteRowEnd( FILE *fp );
	virtual void Stat_WriteComment( FILE *fp, const char *name  );

	virtual void Stat_WriteTableStart( FILE *fp, long tabwidth);
	virtual void Stat_WriteTableStart( FILE *fp, long tabwidth, int iCellSpacing);
	virtual void Stat_WriteTableEnd( FILE *fp );

	virtual void Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title );
	virtual void Stat_WriteHeader( FILE *fp, const char *txt, long space );
	virtual void Stat_WriteNumberData( FILE *fp, long num );
	virtual void Stat_WriteFractionData( FILE *fp, long num, long num2 );
	virtual void Stat_WriteFloatData( FILE *fp, double num );
	virtual void Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteLink( FILE *fp, const char *url, const char *name, long num );
	virtual void Stat_WriteAnotherLinkInMultiLineCell( FILE *fp, const char *url, const char *name, long num );
	virtual void Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteText( FILE *fp, short cols, long rgb, const char *name );
	virtual void Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name );
	virtual void Stat_WriteLocalDiffFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name );// { Stat_WriteLink( fp, url, name, 1 ); }
	virtual void Stat_WriteLocalSameFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name );// { Stat_WriteLink( fp, url, name, 1 ); }

	virtual void WriteFilterText(FILE* fp, const char* szFilterString);

	virtual void WritePageTitle( VDinfoP VDptr, FILE *fh, const char *title );
	virtual void WritePageImageLink( VDinfoP VDptr, FILE *fh, const char *imagename, const char *title, int graph, long sort );
	virtual void WritePageFooter( VDinfoP VDptr, FILE *fh );
	virtual void WriteDemoBanner( VDinfoP VDptr, FILE *fp );
	virtual void SummaryTableEntryURL( FILE *fp, const char *text, const char *url, long normal );
	virtual void SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal );
	virtual void SummaryTableSubTitle( FILE *fp, const char *text, int x, int y );
	virtual void RTF_IncludePicture( FILE *fp, const char *path, const char *file );
	virtual void MakeOneRTF( FILE *fp, const char *path );
	virtual void Stat_WriteTextTooltip( FILE *fp, short cols, const char *name, const char *tooltip );
protected:
	short multiLinedCell;
};


class CQRTFSessionWriter : public CQSessionWriter
{
public:
	CQRTFSessionWriter( const char *fileExtensionP, rtfFile& myRtfFile )
		: CQSessionWriter( fileExtensionP, myRtfFile ){}
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

	virtual void WritePageTitle( VDinfoP VDptr, FILE *fh, const char *title );
	virtual void WritePageFooter( VDinfoP VDptr, FILE *fh );

	virtual void Write_SessBold( FILE *fp, const char *txt );
	virtual void Write_SessColumnBold( FILE *fp, const char *txt );

	virtual void Write_SessHeader( FILE *fp, const char *client, const char *timeStr, const char *b, const char *o );
	virtual void Write_SessTotalTime( FILE *fp, long time, long days );
	virtual void Write_SessLength( FILE *fp, const char *timeStr );
	virtual void Write_SessName( FILE *fp, long rgb, long sitem, const char *dateStr );
	virtual void Write_SessPage( FILE *fp, long item, const char *dateStr, const char *thishost, const char *url, long proto );
};

#endif // REPORTRTF_H
