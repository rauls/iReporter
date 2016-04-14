#ifndef REPORT_H
#define REPORT_H

#include "VirtDomInfo.h"	// for VDinfoP

int	LookupKeyVisitor(Statistic *stat);

#ifdef __cplusplus
extern "C" {
#endif

#define	ONEMEGFLOAT	1048576.0

#define	RGBtableTitle		(MyPrefStruct.RGBtable[1]&0xffffff)
#define	RGBtableHeaders 	(MyPrefStruct.RGBtable[2]&0xffffff)
#define	RGBtableItems		(MyPrefStruct.RGBtable[3]&0xffffff)
#define	RGBtableOthers  	(MyPrefStruct.RGBtable[4]&0xffffff)
#define	RGBtableAvg			(MyPrefStruct.RGBtable[5]&0xffffff)
#define	RGBtableTotals		(MyPrefStruct.RGBtable[6]&0xffffff)

#define	TABLE_WIDTH			(620)
#define	TABLE_HEIGHT		(480)


// ----------------------------------
typedef struct {
	short	type;					// Index to data		
	long	typeID;					// Type of Report - determines how stats are interpreted and what column headings are used
	long	titleStringID;			// Report Title ID
	long	columnStringID;			// Column Text ID
	//long	*flags;					// Ptr to Flags indicating whether to show the report etc.
} ReportTypes, *ReportTypesP;

extern ReportTypesP FindReportTypeData( long id );
long *FindReportFlags( long id );
char *FindReportTitleStr( long id );
char *FindReportFilenameStr( long id );
// ----------------------------------



extern char *imagesuffixStrings[];



extern long	multi_rgb[32];

char *ConvertParamtoURL(char *param, char *newParam);
void VHosts_Write( void *vp, FILE *fp, char *filename, char *title, char *heading, short type );
void GetRealtimeSummary( VDinfoP VDptr, char *out );
void WriteHtmlGifLink( VDinfoP VDptr, FILE *fh , char *gifname, char *title, int graph );
//long Fwrite( FILE *fp, const char *source );
long CompSortVDomain (void *p1, void *p2, long *result);
long CompSortReqVDomain (void *p1, void *p2, long *result);
long CompSortBytesVDomain (void *p1, void *p2, long *result);
long CompSortVisitsVDomain (void *p1, void *p2, long *result);
long CompSortVisitorsVDomain (void *p1, void *p2, long *result);
long CompSortPagesVDomain (void *p1, void *p2, long *result);
long CompSortErrorsVDomain (void *p1, void *p2, long *result);
long CompSortDateVDomain (void *p1, void *p2, long *result);
long strcmpExtensions (char *string, char exts[256][8]);
char *GetTimeonStrings( short lp );
char *GetTimeonStringsTrans( short lp );
char *GetLoyaltyStrings( short lp );
char *GetLoyaltyStringsTrans( short lp );

extern char *HeaderFormat[];
void GetCorrectSiteURL( VDinfoP VDptr, char *host );
#define		NAME_LENGTH		1023
char *GetCIACountryURL( char *cname, char *outname );
long IsThisSortValid( const long sorttype, const long reporttype );
long CountValidFirst( StatList *byStat );
long CountValidLast( StatList *byStat );
long GetTotalReportTypes( void );
long CountRepeatVisits( StatList *byStat );


#ifdef __cplusplus
}
#endif

#endif // REPORT_H



