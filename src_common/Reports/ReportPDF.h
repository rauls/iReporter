
#ifndef REPORTPDF_H
#define REPORTPDF_H

#include "ReportClass.h"
#include "PDFFile.h"
#include "ReportPDFSessions.h"

#define TEMP_RESULT 1

class pdfFile : public baseFile, PDFFile
{
public:
	pdfFile( PDFSettings thePDFSettings, PDFTableSettings thePDFTableSettings );
	~pdfFile();
	virtual int Init( const char *fileName = PDF_BLANK, FILE *filePtr = NULL );
	virtual void FinishReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs );
	virtual void WriteOutline( std::string outlineTitle, int level = 0, int forceLink = 0 );
	virtual const char* Translate( const char *text );
	std::string TranslateHTMLSpecialTokens( std::string str );
	std::string CheckForHTMLCharCode( char *str );
//	virtual long WriteToStorage( const char *source, void *fp ){ return 0; }
	int WriteLine( FILE *fp, const char *txt );
	virtual void Stat_WriteSpace( FILE *fp, long size );
	virtual void Stat_WriteItal( FILE *fp, const char *txt );
	virtual void Stat_WriteBold( FILE *fp, const char *txt );
	virtual void Stat_WriteLine( FILE *fp, const char *txt );
	virtual void Stat_WriteCenterSmall( FILE *fp, const char *txt );

	virtual void WriteFilterText(FILE* fp, const char* szFilterString);

// *********************************************************************************
// These are the HelpCard methods.
// *********************************************************************************
	virtual bool SupportHelpCards(void) { return true; }
	virtual void Stat_WriteIconAndText(  FILE* fp,const char* szText,  const char* szIcon, const char* szPopHelp);
	virtual void BuildHelpCardText(const class CQHelpCardText& rCard, std::string& rstr);
	virtual void WriteHelpCardIcons(VDinfoP VDptr);
	virtual void CleanUpHelpCardIcons(VDinfoP VDptr);
	virtual void Stat_WriteCenterStart( FILE* fp );
	virtual void Stat_WriteCenterEnd( FILE* fp );
	virtual void Stat_WriteHelpCard(FILE *fp, const class CQHelpCard& rCQHelpCard, bool bTableOn, bool bGraphOn);


	virtual void Stat_WriteCentreHeading( FILE *fp, char *txt );
	virtual void Stat_WriteRowStart( FILE *fp, long rgb, long col );
	virtual void Stat_WriteRowEnd( FILE *fp );

	virtual void Stat_WriteTableStart( FILE *fp, long tabwidth ){ numOfRows = 0; }

	virtual void Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title );
	virtual void Stat_WriteBottomHeader( FILE *fp, const char *txt, long space );
	virtual void Stat_WriteHeader( FILE *fp, const char *txt, long space );
	virtual void Stat_WriteDualHeader( FILE *fp, const char *head1, const char *head2 );

	virtual void Stat_WriteNumberData( FILE *fp, long num );
	virtual void Stat_WriteFractionData( FILE *fp, long num, long num2 );
	virtual void Stat_WriteFloatData( FILE *fp, double num );
	virtual void Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteLink( FILE *fp, const char *url, const char *name, long num );
	virtual void Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name );

	virtual void Stat_WriteLocalDiffFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name );
	virtual void Stat_WriteLocalSameFileURL( FILE *fp, short cols, long rgb, const char *url, const char *name );

	virtual void Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name  );
	virtual void Stat_WriteTextTooltip( FILE *fp, short cols, const char *name, const char *tooltip );
	virtual void Stat_WriteText( FILE *fp, short cols, long rgb, const char *name );
	virtual void Stat_WriteDualText( FILE *fp, char *text1, char *text2 );
	virtual void Stat_WriteBoldDual( FILE *fp, const char *txt, const char *txt2 );

	virtual void Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name );
	virtual void Stat_WriteTextRight( FILE *fp, short cols, long rgb, char *name ){
		Stat_WriteRight( fp, cols, rgb, Translate(name) );}
	virtual void Stat_WriteNumRight( FILE *fp, short cols, long rgb, char *name ){
		Stat_WriteRight( fp, cols, rgb, name );	}
//	virtual void WriteMeanPathList( FILE *fp, VDinfoP VDptr, long clienthash, long sindex, long count, long  hits, long rowNum );
	virtual void Stat_WriteMeanPath( VDinfoP VDptr, FILE *fp, int depth );
	virtual void Stat_InsertTable(void);
	virtual void Stat_WritingHelpCard( bool flag );

	virtual void DoSearchEngines( VDinfoP VDptr, FILE *fp, StatList *byStat, long typeID, long sort );

	virtual void WriteTable( void *vp, StatList *byStat, FILE *fp,
		char *filename, char *title, char *heading,
		short id, long typeID, short liststart, short list, short graph );

	virtual void WritePageTitle( VDinfoP VDptr, FILE *fh, const char *title ){}
	virtual void WritePageShadowInit( FILE *fh ){}
	virtual void WritePageShadow( FILE *fh ){}
	virtual void WriteImageShadow( FILE *fh, long , long ){}
	virtual void WritePageImageLink( VDinfoP VDptr, FILE *fh , const char *imagename, const char *title, int graph, long sort ){}
	virtual void WriteLogProcessingTimeInfo( VDinfoP VDptr, FILE *fp );
	virtual void WritePageFooter( VDinfoP VDptr, FILE *fh ){}
	virtual void WriteDemoBanner( VDinfoP VDptr = (VDinfoP)0, FILE *fp = 0 );
	virtual void SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal );
	virtual void SummaryTableStatEntry( FILE *fp, char *text, char *file, long rgb, long normal ){}
	virtual void SummaryTableTitle( FILE *fp, const char *text, long rgb );
	virtual void SummaryTableSubTitle( FILE *fp, const char *text, int x = 0, int y = 0 );
	virtual void SummaryTableHeader( FILE *fp, long width, long height ){}
	virtual void WriteReportServerSummary( VDinfoP VDptr, FILE *fp, long num, long tw, long c ){}
	virtual void WriteReportClientSummary( VDinfoP VDptr, FILE *fp, long num, long tw, long c ){}
	virtual void WriteReportSummary( VDinfoP VDptr, FILE *fp, char *outfile, long logNum );
	virtual void ProduceSummaryTitle( FILE *fp, int logNum, const char *domainName );
	virtual void WriteReportFramedSummary( VDinfoP VDptr, FILE *fp, char *outfile, long logNum ){}
	void AddPDFTable( int outlineLevel = 1 );
	void AddDualTableText( std::string str, int justify, std::string str2, int justify2  );
	void AddTableText( std::string str, int justify );
	virtual void AddTableCellMultilineText( const PDFMultiLineCellTextStrList& cellStrList, int justify );
	virtual void WriteGraphStat( VDinfoP VDptr, StatList *byStat , long id, long typeID, char *filename, char *title, char *header, short sort, short depth, long labelTransId=0 );
	virtual void OutlineStatus( int status ) { outlineDone = status; }
	virtual void AddPDFGraph( std::string graphDataP, int graphHeightP );
	virtual void AddPageBanner();
	void ResetTable();
	virtual double GetStringLen( std::string str );
protected:
	virtual void PutReportOnNextPage( bool state );
	virtual void ShowingGraph( bool state );
	virtual short SingleFileOutput() { return 1; }
	virtual void WriteSessionsList();

protected:
	char transBuf[1024];
};

#endif // REPORTPDF_H
