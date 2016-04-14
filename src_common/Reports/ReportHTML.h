
#ifndef REPORTHTML_H
#define REPORTHTML_H

#include "ReportClass.h"
#include "HTMLFormat.h"
#include "ReportHTMLSessions.h"

enum {
	kReportPage = 1,
	kFrameIndex,
	kVirtualIndex,
	kMultipleIndex
};

class htmlFile : public baseFile
{
public:

	htmlFile();
	~htmlFile();

	virtual void Stat_WriteSessTableEnd( FILE *fp );

	virtual int Fprintf( FILE *fp, const char *fmt, ... );
	char *GetCIACountryURL( char *cname, char *outname );
	virtual char *GetEngineLogo( FILE *fp, char *engine, char *p );
//	virtual char *GetEngineCGI( char *engine, char *p );

	virtual void WritePageShadowInit( FILE *fh );
	virtual void WritePageShadow( FILE *fh );
	virtual void WriteImageShadow( FILE *fh, long, long );
	virtual void WritePageImageLink( VDinfoP VDptr, FILE *fh , const char *imagename, const char *title, int graph, long sort );
	virtual void WriteRefreshMetaOption( FILE *fp );

	// summary page
	virtual void SummaryTableStatEntry( FILE *fp, char *text, char *file, long rgb, long normal );
	virtual void SummaryTableStart( FILE *fp, long width, long centre );
	virtual void SummaryTableFinish( FILE *fp );
	virtual void SummaryTableTitle( FILE *fp, const char *text, long rgb );
	virtual void SummaryTableSubTitle( FILE *fp, const char *text, int x = 0, int y = 0  );

	virtual void SummaryTableRow( FILE *fp, long size );
	virtual void SummaryTableRowEnd( FILE *fp );
	virtual void SummaryTableData( FILE *fp, long width );
	virtual void SummaryTableDataEnd( FILE *fp );
	virtual void SummaryTableHeader( FILE *fp, long width, long height );
	virtual void StartReport( VDinfoP VDptr, FILE *fout, const char *outfile, long logNum );
	virtual void FinishReport( VDinfoP VDptr, FILE *fout, const char *outfile, long logNum );

private:
	virtual void WriteFilterText(FILE* fp, const char* szFilterString);
	virtual void Stat_WriteIconLink( FILE* fp, const char* szIcon, const char* szLink, const char* szLinkExtension, const char* szPopHelp, bool bLeftAlign=true);

	// pages
	virtual void WritePageTitle (VDinfoP VDptr, FILE *fh, const char *title);
	virtual void WriteDemoBanner (VDinfoP VDptr, FILE *fp);
	
	virtual void Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title );
	virtual void Stat_WriteHeader( FILE *fp, const char *txt, long space );

	// framed reports
	virtual void Write_FramedMain( FILE *fp, char *indexfile, char *framefile );
	
	virtual void QuickLinkIndexStart( VDinfoP VDptr, FILE *fp );
	virtual void QuickLinkIndexEnd( FILE *fp );
	virtual void WriteReportBanner( VDinfoP VDptr );
	virtual void ProduceHTMLInternalPageLinks( VDinfoP VDptr, FILE *fp );
	virtual void SummaryTableEntryURL( FILE *fp, const char *text, const char *url, long normal );
	virtual void SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal );
	virtual void SummaryTableSmallTitle( FILE *fp, char *text );
	virtual void WriteReportSummary_Framed( VDinfoP VDptr, FILE *fp, char *outfile, long logNum );
	virtual void WriteCenterOn( FILE *fp );
	virtual void WriteCenterOff( FILE *fp );

private:
	virtual void Stat_WriteComment( FILE *fp, const char *name );
	// text formatting
	virtual void Stat_WriteText( FILE *fp, short cols, long rgb, const char *name );
	virtual void Stat_WriteSpace( FILE *fp, long size );
	virtual void Stat_WriteAnchor( FILE *fp, long a );
	virtual void Stat_WriteAnchor( FILE *fp, const char* szAnchor );
	virtual void Stat_WriteItal( FILE *fp, const char *txt );
	virtual void Stat_WriteBold( FILE *fp, const char *txt );
	virtual void Stat_WriteBoldDual( FILE *fp, const char *txt, const char *txt2 );
	virtual void Stat_WriteLine( FILE *fp, const char *txt );
	virtual void Stat_WriteCentreHeading( FILE *fp, const char *txt );
	virtual void Stat_WriteCenterSmall( FILE *fp, const char *txt );
	virtual void Stat_WriteNumberData( FILE *fp, long num );
	virtual void Stat_WriteFractionData( FILE *fp, long num, long num2 );
	virtual void Stat_WriteFloatData( FILE *fp, double num );
	virtual void WriteDebugComments( FILE *fp, VDinfoP VDptr );
	virtual void Stat_WriteRowStart( FILE *fp, long rgb, long col );
	virtual void Stat_WriteRowEnd( FILE *fp );
	virtual void Stat_WriteTableStart( FILE *fp, long tabwidth);
	virtual void Stat_WriteTableStart( FILE *fp, long tabwidth, int iCellSpacing);
	virtual void Stat_WriteTableEnd( FILE *fp );

	virtual void Stat_WriteIconAndText(  FILE* fp, const char* szText,  const char* szIcon, const char* szPopHelp);
	virtual bool SupportHelpCards(void) { return true; }
	virtual void BuildHelpCardText(const class CQHelpCardText& rCard, std::string& rstr);
	virtual void WriteHelpCardIcons(VDinfoP VDptr);
	virtual void Stat_WriteCenterStart( FILE* fp );
	virtual void Stat_WriteCenterEnd( FILE* fp );
	virtual void Stat_WriteHelpCard(FILE *fp, const class CQHelpCard& rCQHelpCard, bool bTableOn, bool bGraphOn);


	virtual void Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteLink( FILE *fp, const char *url, const char *name, long num );
	virtual void Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteTextTooltip( FILE *fp, short cols, const char *name, const char *tooltip );
	virtual void Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name );
	virtual void Stat_WriteDualText( FILE *fp, char *name, char *name2 );
	virtual void WriteCellStartLeft( FILE *fp );
	virtual void WriteCellStartRight( FILE *fp );
	virtual void WriteCellEnd( FILE *fp );

protected:
	virtual void WriteVirtualHostFrame( FILE *fp );
	virtual void WriteVirtualHostListBegin( FILE *fp );
	virtual void WriteVirtualHostListItem (FILE *fi, long realVDnum, char *newVDname, char* fileName, VDinfoP VDPtr, short URLType);
	virtual void WriteVirtualHostListEnd( FILE *fp );
	virtual void WritePageBodyOpen (FILE *fp);
	virtual void WritePageBodyClose (FILE *fp);
	virtual void WriteSummaryHTMLLinks( VDinfoP VDptr, FILE *fp, long num, long tw, long c );
	void WriteReportServerSummary( VDinfoP VDptr, FILE *fp, long num, long tw, long c );
	void WriteReportClientSummary( VDinfoP VDptr, FILE *fp, long num, long tw, long c );
	void WriteReportSummary( VDinfoP VDptr, FILE *fp, char *outfile, long logNum );
	void WritePageHeader( FILE *fp, const char *title );
	virtual void WritePageFooter( VDinfoP VDptr, FILE *fh );
	void SetHTMLFont( FILE *fp );
	int Stat_WriteCommentArgs( FILE *fp, const char *fmt, ... );
	void IncrementTableCount( FILE *fp );
	void DecrementTableCount( FILE *fp );
	short tableLevel;
};

#endif // REPORTHTML_H
