
#ifndef REPORT_CLASS_H
#define REPORT_CLASS_H

#include <string>
#include <stdio.h>
#include "config.h"
#include "myansi.h"
class StatList;
#include "VirtDomInfo.h"
#include "PDFTableData.h"
#include "ReportSessions.h"

// flags for Virtual Host link writing, URLType parameter for WriteVirtualHostListItem (...)
enum
{
	kNormalLink = 0,
	kMappedLink
};

typedef struct {
	char	pageTitle[256];
	char	metaGenerator[256];
	char	metaContentType[256];
	char	metaExpires[256];
	char	pageBanner[32];
	char	linkColour[7];
	char	vLinkColour[7];
	char	aLinkColour[7];
	char	textColour[7];
	char	bgColour[7];
} HTMLOptions;



class baseFile
{
public:
	baseFile();
	virtual ~baseFile();
	virtual void WriteClassReport (long logstyle, long logNum);
	virtual int Init( const char *filename = "", FILE *filePtr = NULL ){return true;}
	virtual void WriteOutline( std::string outlineTitle, int level = 0, int forceLink = 0 ) {}
	virtual const char* Translate( const char *text ) { return text; }
	virtual int ReportTurnedOn( long id ){ return 1; }
	virtual int DontWriteReports( long id ){ return 0; }			// temp added back RHF
	virtual long WriteReport( StatList *statListObj, long id );
	virtual long WriteReport( StatList *statListObj, long id, FilterFunc m_FilterIndex );
	virtual char *FindHeadFormat( long type );
	virtual void InitExtensions( void );
//	virtual long WriteToStorage( const char *source,void *fp );
	virtual int WriteLine( FILE *fp, const char *txt );
protected:
	virtual int Fprintf( FILE *fp, const char *fmt, ... );
	virtual int WriteOUT( FILE *fp, char *fmt, ... );
	int WriteDirectlyToFile( FILE *fp, char *fmt, ... );
//	virtual void HTMLprintf( FILE *fp, char *fmt, ... );
	virtual void WriteFilterText(FILE* fp, const char* szFilterString)	{ }

public:
	virtual void StartReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs );
	virtual void FinishReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs );
	virtual void SummaryTableSmallTitle( FILE *fp, char *text ){}

	virtual void Stat_WriteComment( FILE *fp, const char *name  ){}
	virtual void Stat_WriteCentreHeading( FILE *fp, const char *txt ) { Stat_WriteHeader( fp, txt, 0 ); }
	virtual void Stat_WriteRowStart( FILE *fp, long rgb, long col ){}
	virtual void Stat_WriteRowEnd( FILE *fp ){}
	virtual void Stat_WriteTableStart( FILE *fp, long tabwidth){}
	virtual void Stat_WriteTableStart( FILE *fp, long tabwidth, int iCellSpacing){}
	virtual void Stat_WriteTableEnd( FILE *fp ){}


	virtual void Stat_WriteIconAndText(  FILE* fp, const char* szText,  const char* szIcon, const char* szPopHelp) {}
	virtual void Stat_WriteIconLink( FILE* fp, const char* szIcon, const char* szLink, const char* szLinkExtension, const char* szPopHelp, bool bLeftAlign=true) {}
	virtual bool SupportHelpCards(void) { return false; }
	virtual void BuildHelpCardText(const class CQHelpCardText& rCard, std::string& rstr) {}
	virtual void WriteHelpCardIcons(VDinfoP VDptr) {}
	virtual void CleanUpHelpCardIcons(VDinfoP VDptr) {}
	virtual void Stat_WriteCenterStart( FILE* fp ) {}
	virtual void Stat_WriteCenterEnd( FILE* fp ) {}
	virtual void Stat_WriteHelpCard(FILE *fp, const class CQHelpCard& rCQHelpCard, bool bTableOn, bool bGraphOn) {}

	virtual void Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title ){}
	virtual void Stat_WriteBottomHeader( FILE *fp, const char *txt, long space );
	virtual void Stat_WriteHeader( FILE *fp, const char *txt, long space ) = 0;
	virtual void Stat_WriteDualHeader( FILE *fp, const char *head1, const char *head2 );
	virtual void Stat_WriteCostData( FILE *fp, __int64 bytes );
	virtual void Stat_WriteTimeDuration( FILE *fp, long totalTime , long totalVisits );
	virtual void Stat_WritePercent( FILE *fp, double perc );
	virtual void Stat_WriteReqData( FILE *fp, long files, double perc );
	virtual void Stat_WriteHitData( FILE *fp, long files, __int64 totalFiles );
	virtual void Stat_WriteBytes( FILE *fp, __int64 bytes );
	virtual void Stat_WriteByteData( FILE *fp, __int64 bytes, double perc );
	virtual void Stat_WriteMainData( FILE *fp, long files, double p1, __int64 bytes,  double p2 );
	virtual void Stat_WriteHitsAndBytes( FILE *fp, long files, __int64 totalFiles, __int64 bytes,  __int64 totalBytes );
	virtual void Stat_WriteReqHeader( FILE *fp );
	virtual void Stat_WriteByteHeader( FILE *fp );
	virtual void Stat_WriteMainHeader( FILE *fp );
	virtual void Stat_WritePercent(		FILE* fp, double dNumerator, double dDenominator);

	virtual void WriteMeanPathList( FILE *fp, VDinfoP VDptr, long clienthash, long sindex, long count, long  hits, long rowNum );
	virtual void Stat_WriteMeanPath( VDinfoP VDptr, FILE *fp, int depth );
	virtual void Stat_WriteSearchEngine( VDinfoP VDptr, StatList *byStat, FILE *fp, long searchEngine, int depth, long sort );
	virtual void Stat_WriteKeyVisitors(  VDinfoP VDptr, StatList *byStat, FILE *fp, long nIndex, int depth, long sort, long nTableNumber );

	virtual void DoSearchEngines( VDinfoP VDptr, FILE *fp, StatList *byStat, long typeID, long sort );
	virtual void DoKeyVisitorsTables( VDinfoP VDptr, FILE *fp, StatList *byStat, long typeID, long sort );
	virtual void DoFailedRequestSubTables( VDinfo* VDptr, FILE* fp, const char*	filename, StatList* pMainTableStats,
		                                   size_t startIndex, size_t numSubTables, StatList* pFailedRequestURLStats,
										   const char* firstColHeader, const char* subTableNamePrefix=0,
										   const char* subTableNameSuffix=0, size_t	maxNumSubTableRows=0
										   );
	
	virtual void VHosts_Write( void *vp, FILE *fp, char *title, char *heading );
	virtual long CheckStrQ( char *s1, char *s2 );
	virtual long Stat_WriteReportHeaders( FILE *fp, long type, long typeID, long e, char *dh );
	virtual void WriteTable( void *vp, StatList *byStat, FILE *fp,
		char *filename, char *title, char *heading,
		short id, long typeID, short liststart, short list, short graph );
	virtual void WriteTableData( void *vp, StatList *byStat, FILE *fp,
		char *filename, char *title, char *heading,
		long typeID, short liststart, short list, short graph,
		short id, int cols, int colspan, long columnIDs[/*16*/], char *columnStrPtr[/*16*/],
		char thishost[/*MAXURLSIZE*/] );
	virtual void GetTableColumnHeadings( char *columnStrPtr[/*16*/],
		long columnIDs[/*16*/],	int	&colspan, int &cols, long &hashID, long typeID );
	virtual void WriteTableHeadings( long tabwidth, FILE *fp, int colspan, short list, char *title, char *heading, int cols, long columnIDs[], long typeID, int extras  );

	virtual void WritePageTitle( VDinfoP VDptr, FILE *fh, const char *title ){}
	virtual void WritePageShadowInit( FILE *fh ){}
	virtual void WritePageShadow( FILE *fh ){}
	virtual void WriteImageShadow( FILE *fh, long, long ){}
	virtual void WritePageImageLink( VDinfoP VDptr, FILE *fh , const char *imagename, const char *title, int graph, long sort );
	virtual void WriteLogProcessingTimeInfo( VDinfoP VDptr, FILE *fp );
	virtual void WritePageFooter( VDinfoP VDptr, FILE *fh ) {}
	virtual void WriteDemoBanner( VDinfoP VDptr, FILE *fp ) {}
	virtual void WriteDebugComments( FILE *fp, VDinfoP VDptr ){}
	virtual void BeginSummaryTable( FILE *fp, long tw, char *txt, int c );
	virtual void EndSummaryTable( FILE *fp );
	virtual void SummaryTableEntryURL( FILE *fp, const char *text, const char *url, long normal ){}
	virtual void SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal ){}
	virtual void SummaryTableStatEntry( FILE *fp, char *text, char *file, long rgb, long normal ){}
	virtual void SummaryTableStart( FILE *fp, long width, long centre ){}
	virtual void SummaryTableFinish( FILE *fp ){}
	virtual void SummaryTableTitle( FILE *fp, const char *text, long rgb ){}
	virtual void SummaryTableSubTitle( FILE *fp, const char *text, int x = 0, int y = 0 ){}
	virtual void SummaryTableRow( FILE *fp, long size ){}
	virtual void SummaryTableRowEnd( FILE *fp ){}
	virtual void SummaryTableData( FILE *fp, long width ){}
	virtual void SummaryTableDataEnd( FILE *fp ){}
	virtual void SummaryTableHeader( FILE *fp, long width, long height ){}
	virtual void SummaryBandwidth( FILE *fp, char *title, __int64 bandwidth, double num, long c );
	virtual void QuickLinkIndexStart( VDinfoP VDptr, FILE *fp ){}
	virtual void QuickLinkIndexEnd( FILE *fp ){}
	virtual void Write_FramedMain( FILE *fp, char *indexfile, char *framefile ) {}
	virtual void WriteReportLoadSummary( VDinfoP VDptr, FILE *fp, long num, long tw, long c );
	virtual void WriteReportLoad2Summary( VDinfoP VDptr, FILE *fp, long num, long tw, long c );
	virtual void WriteReportBanner( VDinfoP VDptr ){}
	virtual void WriteSummaryDetails( VDinfoP VDptr, FILE *fp, char *outfile, long logNum );
	virtual void WriteReportSummary( VDinfoP VDptr, FILE *fp, char *outfile, long logNum );
	virtual void ProduceSummaryTitle( FILE *fp, int logNum, const char *domainName );
	virtual void WriteRefreshMetaOption( FILE *fp ){}
	virtual void WriteReportStat( VDinfoP VDptr, StatList *byStat, FILE *fp, char *filename, long id, long typeID, char *title, char *header, char *imagename, short depth, short sort, short graph, short table );
	virtual void WriteGraphStat( VDinfoP VDptr, StatList *byStat , long id, long typeID, char *filename, char *title, char *header, short sort, short depth, long labelTransId=0 );
	virtual void outputStats( VDinfoP VDptr, StatList *byStat, long id );
	virtual void OutlineStatus( int status ){}
	virtual long WriteReportVDList (long n);
	virtual void RTF_IncludePicture( FILE *fp, const char *path, const char *file ){}
	virtual long WriteAllPages( VDinfoP VDptr, long logstyle );
	virtual void AddPDFGraph( std::string graphDataP, int graphHeightP ){}
	virtual void AddPageBanner(){}
	virtual void MakeOneRTF( FILE *fp, const char *path ){}
	virtual void writeTextRightWithRLBorder( const char *text, FILE *fp ){}
	virtual void writeNumberRightWithRLBorder( double num, FILE *fp ){}
	virtual void writeMultiRow( long currentRow, char * pathName, FILE *fp ){}
	virtual void WriteCellStartLeft( FILE *fp ){}
	virtual void WriteCellStartRight( FILE *fp ){}
	virtual void WriteCellEnd( FILE *fp ){}
	virtual void initAreaRow(){}
	virtual void initColumn(){}
	virtual void writeNextData(){}
	virtual void ProduceHTMLInternalPageLinks( VDinfoP VDptr, FILE *fp ){}
	inline void SERVER_TABLEFLOAT( FILE *fp, double value, char *text, int color, long &line, long c );
	inline void SERVER_TABLENUM( FILE *fp, long value, char *text, int color, long &line, long c );
	inline void SERVER_TABLETEXT( FILE *fp, char *text, char *number, int color, long &line, long c );

	// text formatting
	virtual void Stat_WriteText( FILE *fp, short cols, long rgb, const char *name ) {}
	virtual void Stat_WriteSpace( FILE *fp, long size ) {}
	virtual void Stat_WriteAnchor( FILE *fp, long a ) {}
	virtual void Stat_WriteAnchor( FILE *fp, const char* szAnchor ) {}
	virtual void Stat_WriteItal( FILE *fp, const char *txt ) { Stat_WriteLine( fp, txt ); }
	virtual void Stat_WriteBold( FILE *fp, const char *txt ) = 0;
	virtual void Stat_WriteBoldDual( FILE *fp, const char *txt, const char *txt2 );
	virtual void Stat_WriteLine( FILE *fp, const char *txt ) {}
	virtual void Stat_WriteCenterSmall( FILE *fp, const char *txt );
	virtual void Stat_WriteNumberData( FILE *fp, long num ) {}

	virtual void Stat_WriteFractionData( FILE *fp, long num, long num2 ) {}
	virtual void Stat_WriteFloatData( FILE *fp, double num ) {}
	virtual void Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name ) {}
	virtual void Stat_WriteLink( FILE *fp, const char *url, const char *name, long num ) {}
	virtual void Stat_WriteAnotherLinkInMultiLineCell( FILE *fp, const char *url, const char *name, long num ) { Stat_WriteLink( fp, url, name, num ); }
	virtual void Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name ) {}
	virtual void Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name ) {}
	virtual void Stat_WriteLocalDiffFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name ) { Stat_WriteURL( fp, cols, rgb, url, name ); }
	virtual void Stat_WriteLocalSameFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name ) { Stat_WriteURL( fp, cols, rgb, url, name ); }
	virtual void Stat_WriteTextTooltip( FILE *fp, short cols, const char *name, const char *tooltip ) {}
	virtual void Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name ) {}
	virtual void Stat_WriteTextRight( FILE *fp, short cols, long rgb, char *name ){
		Stat_WriteRight( fp, cols, rgb, name );	}
	virtual void Stat_WriteNumRight( FILE *fp, short cols, long rgb, char *name ){
		Stat_WriteRight( fp, cols, rgb, name );	}
	virtual void Stat_WriteDualText( FILE *fp, char *name, char *name2 );
	virtual short SingleFileOutput() { return 0; }
	virtual double GetStringLen( std::string str ) { return str.length(); }
	virtual void AddTableCellMultilineText( const PDFMultiLineCellTextStrList& cellStrList, int justify ){}
	virtual void Stat_InsertTable(void) {}
	virtual void Stat_WritingHelpCard( bool flag ) {}

protected:
	short AnyTrafficStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyDiagnosticStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyServerStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyDemographicStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyReferralStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyStreamingMediaStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnySystemsStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyAdvertisingStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyBillingStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyMarketingStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyClustersStatsTurnedOn( VDinfo *VD, App_config* prefs );
	short AnyFirewallStatsTurnedOn( VDinfo *VD, App_config* prefs );

protected:
	virtual char *GetEngineLogo( FILE *fp, char *engine, char *p ){ return 0; }
	virtual char *GetEngineCGI( char *engine, char *p );
//	long Fwrite( FILE *fp, const char *source );
	virtual void PutReportOnNextPage( bool state ) {}
	virtual void ShowingGraph( bool state ) {}
	virtual void WriteVirtualHostFrame( FILE *fp );
	virtual void WriteVirtualHostListBegin( FILE *fp );
	virtual void WriteVirtualHostListItem( FILE *fi, long realVDnum, char *newVDname, char* fileName, VDinfoP VDPtr, short URLType);
	virtual void WriteVirtualHostListEnd( FILE *fp );
	virtual void WritePageBodyOpen( FILE *fp );
	virtual void WritePageBodyClose( FILE *fp );
	virtual int NormalLink(){ return kNormalLink; }
	virtual int MappedLink(){ return kMappedLink; }
	void InitPageOptions();
	HTMLOptions	pageOptions;

protected:
	const char* GetReportTitle() { return reportTitle.c_str(); }
	void SetReportTitle( std::string txt ) { reportTitle = txt; }
private:
	std::string reportTitle;

public:
	long	m_style;
	long	html_count;
	int		rownum;
	int		one_file_output;
	int		dont_do_subtotals;
	int		dont_do_others;
	int		dont_do_average;
	int		dont_do_totals;
	std::string HTMLLinkFilename;

	typedef enum{DATA,SUM,AVERAGE} operateType;
	operateType cellType;

protected:
	virtual void WriteSessionsList(){}
	const char *fileExtension;
	char tempBuff[10240];
	char tempBuff2[10240];
	CQSessionWriter *sessionWriter;
};


#endif // REPORT_CLASS_H
