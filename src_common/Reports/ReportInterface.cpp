
#include "FWA.h"

#include <string>
#include <list>
#include <map>
#include "ReportHTML.h"
#include "ReportPDF.h"
#include "ReportRTF.h"
#include "ReportExcel.h"
#include "ReportComma.h"
#include "config_struct.h"
#include "ReportInterface.h"
#include "translate.h"



extern long outgoing_socket;
extern VDinfoP	VD[MAX_DOMAINSPLUS];			/* 16/32 virtual domain pointers */

long LiveViewReport( long which, long socket )
{
	FILE *fp = (FILE*)3;
	outgoing_socket = socket;

	baseFile *baseFilePtr;
	switch (MyPrefStruct.report_format)
	{
		case FORMAT_RTF:
			baseFilePtr = new rtfFile();
			break;
		case FORMAT_COMMA:
			baseFilePtr = new commaFile();
			break;
		case FORMAT_PDF:
			return 0;
			break;
		case FORMAT_EXCEL:
			baseFilePtr = new excelFile();
			break;
		case FORMAT_HTML:
		default:
			baseFilePtr = new htmlFile();
	}
	baseFilePtr->WriteReportSummary( VD[ 0 ], fp, (char*)0, 1 );
	delete 	baseFilePtr;

	return 0;
}

void WriteReportToDisk( long logstyle, long logNum )
{
	baseFile			*baseFilePtr;
	long ret = InitLanguage( MyPrefStruct.language, 0 );

	switch (MyPrefStruct.report_format)
	{
		case FORMAT_RTF:
			//StatusSet( "Doing RTF" );
			baseFilePtr = new rtfFile();
			break;
		case FORMAT_COMMA:
			baseFilePtr = new commaFile();
			break;
		case FORMAT_PDF:
			//StatusSet( "Doing PDF" );
			{
				// Copy the duplicated setting which determines whether to display put 2 pages to a sheet of paper (for Landscape only)
				MyPrefStruct.pdfAllSetC.pdfTableSetC.subPagesPerPage = MyPrefStruct.pdfAllSetC.pdfSetC.subPagesPerPage;
				
				PDFSettings thePDFSettings( &MyPrefStruct.pdfAllSetC.pdfSetC );
				PDFTableSettings thePDFTableSettings( &MyPrefStruct.pdfAllSetC.pdfTableSetC );
				baseFilePtr = new pdfFile( thePDFSettings, thePDFTableSettings );
			}
			break;
		case FORMAT_EXCEL:
			baseFilePtr = new excelFile();
			break;
		case FORMAT_HTML:
		default:
			//StatusSet( "Doing HTML" );
			baseFilePtr = new htmlFile();
	}

//OutDebug( "WriteClassReport" );
	baseFilePtr->WriteClassReport (logstyle, logNum);

	delete 	baseFilePtr;
}

