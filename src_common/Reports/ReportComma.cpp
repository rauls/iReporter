
#include "compiler.h"

#include <string>

#include "ReportComma.h"
#include "ReportFuncs.h"

commaFile::commaFile() : baseFile()
{
	m_style = FORMAT_COMMA;
	numOfCols = 0;
	dataColNum = 0;
	fileExtension = COMMA_EXT;
	writingSummaryTable = 0;
	sessionWriter = new CQCommaSessionWriter( fileExtension, *this );
}

commaFile::~commaFile()
{
}

#ifdef DEF_MAC
#pragma mark --- table elements ---
#endif

void commaFile::Stat_WriteRowEnd( FILE *fp )
{
	Fprintf( fp,"\n" );
}

void commaFile::Stat_WriteTableEnd( FILE *fp )
{
	Fprintf( fp,"\n" );
	numOfCols = 0;
	dataColNum = 0;
}

void commaFile::Stat_WriteHeader( FILE *fp, const char *txt, long space )
{
	if ( numOfCols == 0 )
		Fprintf( fp,"%s", txt ); // No comma for first column
	else
		Fprintf( fp,",%s", txt );
	numOfCols++;
}

#ifdef DEF_MAC
#pragma mark --- text formatting ---
#endif

void commaFile::WriteText(FILE *fp, const char *textString)
{
	if ( dataColNum == numOfCols )
		dataColNum = 0;

	if ( dataColNum == 0 )
		Fprintf( fp, "%s", textString ); // No comma for first column
	else
		Fprintf( fp, ",%s", textString );
	dataColNum++;
}

void commaFile::WriteTextLine( FILE *fp, const char *textString)
{
	Fprintf( fp, "%s\n", textString );
}

// **********************************************************************************
void commaFile::WriteFilterText( FILE *fp, const char *szFilterString)
{
	// Is already newline separated.
	int	iStrLen;
	for (const char*	sz = szFilterString; iStrLen=mystrlen(sz); sz += (iStrLen+1))
	{
		Fprintf( fp, "%s\n", sz );
	}
	Fprintf( fp, "\n");
}

void commaFile::Stat_WriteText( FILE *fp, short cols, long rgb, const char *name )
{
	WriteText( fp, name );
}

void commaFile::Stat_WriteLink( FILE *fp, const char *url, const char *name, long num )
{
	if ( num == 1 )
	{
		if ( dataColNum == numOfCols )
			dataColNum = 0;
		if ( dataColNum == 0 )
			Fprintf( fp, "%d.  %s", num, name );
		else
			Fprintf( fp, ",%d.  %s", num, name );
		dataColNum++;
	}
	else
	{
		Fprintf( fp, "  %d.  %s", num, name );
	}
}

void commaFile::Stat_WriteItal( FILE *fp, const char *txt )
{
	WriteText( fp, txt );
}

void commaFile::Stat_WriteBold( FILE *fp, const char *txt )
{
	if ( writingSummaryTable )
		WriteTextLine( fp, txt );
	else
		WriteText( fp, txt );
}

void commaFile::Stat_WriteLine( FILE *fp, const char *txt )
{
	WriteTextLine( fp, txt );
};

void commaFile::Stat_WriteNumberData( FILE *fp, long num )
{
	if ( dataColNum == numOfCols )
		dataColNum = 0;

	if ( dataColNum == 0 )
		Fprintf( fp,"%d", num ); // No comma for first column 
	else
		Fprintf( fp,",%d", num );
	dataColNum++;
}

void commaFile::Stat_WriteFractionData( FILE *fp, long num, long num2 )
{
	if ( dataColNum == numOfCols )
		dataColNum = 0;

	if ( dataColNum == 0 )
		Fprintf( fp,"%d/%d", num, num2 ); // No comma for first column 
	else
		Fprintf( fp,",%d/%d", num, num2 );
	dataColNum++;
}

void commaFile::Stat_WriteFloatData( FILE *fp, double num )
{
	if ( dataColNum == numOfCols )
		dataColNum = 0;

	if ( dataColNum == 0 )
		Fprintf( fp,"%.2f", num ); // No comma for first column 
	else
		Fprintf( fp,",%.2f", num );
	dataColNum++;
}

void commaFile::Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name  )
{
	WriteText( fp, name );
	//Fprintf( fp, ",%s", name );
}

void commaFile::Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	WriteText( fp, name );
	//Fprintf( fp,",%s",name );
}

void commaFile::Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	WriteText( fp, name );
	//Fprintf( fp,",%s",name );
}

void commaFile::Stat_WriteTextTooltip( FILE *fp, short cols, const char *name, const char *tooltip )
{
	//Fprintf( fp,",%s", name );
}

void commaFile::Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name )
{
	WriteText( fp, name );
	//Fprintf( fp,",%s", name );
}

/*

//static char *imagesuffixStrings[] = { ".gif", ".jpg", ".png", ".bmp", 0,0,0,0,0 };

/*
// This is better suited for use for links for all types since it uses names from the table.
// RS,29May
void commaFile::SummaryTableStatName( FILE *fp, char *id, long rgb, long normal )
{
	char *text, *file;
	ReportTypesP	report_data;

	if( report_data = FindReportTypeData( id ) ){
		text = report_data->title;
		file = report_data->filename;
		SummaryTableStatEntry( fp, text, file , rgb, normal );
	}
}
*/
/*

void commaFile::WriteGraphStat( VDinfoP VDptr, StatList *byStat , long id, long typeID, char *filename, char *title, char *header, short sort, short depth )
{
	StandardPlot *thePlot = new StandardPlot();
//	thePlot->DrawbyDataStyle( VDptr, byStat , id, typeID, filename, title, header, sort, depth/*, thePlot );
/*	delete thePlot;
}



*/


#ifdef DEF_MAC
#pragma mark --- Sessions ---
#endif

/*void CQCommaSessionWriter::Write_SessHeader( FILE *fp, const char *client, const char *timeStr, const char *browser, const char *opersys )
{
	Fprintf( fp, "Session listing for %s, Average length %s\n", client, timeStr );
	Fprintf( fp, "     Sessions list\n\n" );
}*/

//extern void ValuetoString( long scaler_type, __int64 hvalue, __int64 current_value, char *name );
/*void CQCommaSessionWriter::Write_SessSummary( FILE *fp, const char *txt, __int64 b1, int grandTotal )
{
	char	numStr[32], out[256];

	if( b1 )
	{
		ValuetoString( TYPE_BYTES, b1, b1, numStr );
		sprintf( out, "%s %s", txt, numStr );
		Stat_WriteBold( fp, out );
	}
}

void CQCommaSessionWriter::Write_SessTotalTime( FILE *fp , long time, long days )
{
	char	numStr[32];

	CTimetoString( time, numStr );
	Fprintf( fp, "\nTotal online time %s\n", numStr );
	CTimetoString( time/days, numStr );
	Fprintf( fp, "\nAverage online time per day %s\n", numStr );
}

void CQCommaSessionWriter::Write_SessLength( FILE *fp, const char *timeStr )
{
	Fprintf( fp, "\nEstimated Session length %s\n\n", timeStr );
}

void CQCommaSessionWriter::Write_SessName( FILE *fp , long rgb, long sitem, const char *dateStr )
{
	Fprintf( fp, "Session #%d %s\n", sitem, dateStr );
}*/

void commaFile::Stat_WriteDualText( FILE *fp, const char *name, const char *name2 )
{
	if ( dataColNum == numOfCols )
		dataColNum = 0;

	if ( dataColNum == 0 )
		Fprintf( fp,"%s %s", name, name2 ); // No comma for first column 
	else
		Fprintf( fp,",%s %s", name, name2 );
	dataColNum++;
}

void commaFile::Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title )
{
	Fprintf( fp,"%s\n", title );
}

void commaFile::Stat_WriteBottomHeader( FILE *fp, const char *txt, long space )
{
	WriteText( fp, txt );
}

void commaFile::SummaryTableStart( FILE *fp, long width, long centre )
{
	Fprintf(fp,"\n");
//	writingSummaryTable++;
}

void commaFile::SummaryTableFinish( FILE *fp )
{
	Fprintf(fp,"\n");
//	writingSummaryTable--;
}

void commaFile::SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal )
{
	Fprintf(fp,"\n%s", text );
	if ( number )
		Fprintf(fp,"\t%s", number );
}

void commaFile::SummaryTableSubTitle( FILE *fp, const char *text, int x, int y )
{
	Fprintf(fp,"\n\n%s\n", text );
}

void commaFile::SummaryTableTitle( FILE *fp, const char *text, long rgb )
{
	Fprintf(fp,"\n%s\n", text );
}

void commaFile::ProduceSummaryTitle( FILE *fp, int logNum, const char *domainName )
{
	baseFile::ProduceSummaryTitle( fp, logNum, domainName );
	writingSummaryTable = 1;
}

void commaFile::WritePageFooter( VDinfoP VDptr, FILE *fp )
{
	writingSummaryTable = 0;
}

/*void commaFile::Stat_WriteCentreHeading( FILE *fp, const char *txt )
{
	Stat_WriteHeader( fp, txt, 0 );
}*/
