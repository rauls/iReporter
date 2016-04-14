/*
Default font would be good to have, <basefont ="arial, helvetica" size="2">
if possible in all table cells.
*/

#include "FWA.h"

#include <string>
#define NO_INLINE
#include <list>
#include <map>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <iostream>
#include "datetime.h"
#include "OSInfo.h"
#include "Registration.h"

#ifdef DEF_MAC
	#include <OpenTransport.h>
	#include <OpenTptInternet.h>
	#include <memory.h>
	#include "config.h"
	#include "MacStatus.h"
	#include "MacUtil.h"
	#include "progress.h"
	#include "post_comp.h"
	#include "Processing.h"
#endif

#include "myansi.h"
#include "zlib.h"
#include "gd.h"
#include "config_struct.h"
#include "engine_proc.h"
#include "StandardPlot.h"
#include "engine_drill.h"
#include "editpath.h"
#include "translate.h"
#include "weburls.h"
#include "EngineParse.h"
#include "EngineStatus.h"	// for Getsleeptypesecs()
#include "Utilities.h"

#if DEF_WINDOWS
	#include <sys/stat.h>
	#include "Windnr.h"
	#include "Winmain.h"
	#include "postproc.h"
	#include "httpinterface.h"
	#include "resource.h"
#endif				

#if DEF_UNIX		// UNIX include
	#include <sys/stat.h>
	#include <errno.h>
	#include "unistd.h"
	#include "main.h"
	#include "postproc.h"
	#include "httpinterface.h"
	#include "resource_strings.h"
#endif

#ifdef	DEF_FULLVERSION
//	#include "virtualE.h"
	#include "fwprogif3.h"
#else
//	#include "virtual4.h"
	#include "fwstdgif3.h"
#endif

#include "blackline.h"		// blackline GIF , a HR tag replacement option.

#include "Report.h"
#include "ReportFuncs.h"
#include "report_keypages.h"
#include "ReportHTML.h"
#include "OSInfo.h"
#include "FileTypes.h"
#include "DateFixFileName.h"
#include "HelpCard.h"
#include "Icons.h"

#include "html_decodetemplate.h"


// Stuff from Engine.cpp
#include "EngineVirtualDomain.h"	// for extern of ...
//extern VDinfoP		VD[MAX_DOMAINSPLUS];
//extern long			VDnum;
extern long			totalDomains;



#define	SUMMARY_POPUP		2
#define	SUMMARY_DUALCOLUMN	1
#define	SUMMARY_FRAMED		0
#define	SUMMARY_NONFRAMED	-1











htmlFile::htmlFile() : baseFile()
{
	m_style = FORMAT_HTML;
	fileExtension = HTML_EXT;
	sessionWriter = new CQHTMLSessionWriter( fileExtension, *this );
	tableLevel = 0;
}

htmlFile::~htmlFile()
{
}


int htmlFile::Fprintf( FILE *fp, const char *fmt, ... )
{
	va_list		args;
static	char lineout[10240];

	va_start( args, fmt );
	vsprintf( lineout, fmt, args );
	va_end( args );

	return WriteLine( fp, lineout );
}

void htmlFile::Stat_WriteSessTableEnd( FILE *fp )
{
	Fprintf(fp,"</td></tr>\n" );
}

#ifdef DEF_MAC
#pragma mark --- Pages ---
#endif


void htmlFile::WriteCenterOn( FILE *fp )
{
	Fprintf (fp, "<CENTER>");
}
void htmlFile::WriteCenterOff( FILE *fp )
{
	Fprintf (fp, "</CENTER>");
}


void htmlFile::WritePageHeader( FILE *fp, const char *title )
{
	Fprintf (fp, "<html>\n");
	Fprintf (fp, "<head>\n");
	Fprintf (fp, "%s\n", pageOptions.metaGenerator);	
	Fprintf (fp, "%s\n", pageOptions.metaContentType);	
	Fprintf (fp, "%s\n", pageOptions.metaExpires);
	if ( title )
		Fprintf (fp, "<title>%s</title>\n", title);
	Fprintf (fp, "</head>\n\n");

	WriteCenterOn( fp );
}

void htmlFile::WritePageBodyOpen (FILE *fp)
{
	Fprintf (fp, "<body bgcolor=\"#%s\" text=\"#%s\" link=\"#%s\" vlink=\"#%s\" alink=\"#%s\">\n\n",
				pageOptions.bgColour, pageOptions.textColour, 
				pageOptions.linkColour, pageOptions.vLinkColour, pageOptions.aLinkColour);
}

void htmlFile::WritePageBodyClose (FILE *fp)
{
	Fprintf (fp, "</body>\n</html>");
}

// *********************************************************************************
// JMC: 
// *********************************************************************************
void htmlFile::WriteFilterText(FILE* fp, const char* szFilterString)
{
	WriteLine( fp, "<table width=\"600\"><tr>\n");
	int	iStrLen;
	for (const char*	sz = szFilterString; iStrLen=mystrlen(sz); sz += (iStrLen+1))
	{
		WriteLine( fp, sz);
		WriteLine( fp, ".<br>\n" );
	}
	WriteLine( fp, "</tr></table>\n");
	WriteLine( fp, "<br>\n" );
}
// *********************************************************************************

void htmlFile::WritePageTitle( VDinfoP VDptr, FILE *fp, const char *title )
{
	WritePageHeader( fp, title );

	if ( title )
	{
		WriteLine( fp, MyPrefStruct.html_head );				// write out processing information
	} else 
	{
		WritePageBodyOpen( fp );
	}
}

void htmlFile::WriteDemoBanner( VDinfoP VDptr, FILE *fp )
{
	if (IsDemoReg())
	{
		Fprintf (fp, "<center><i><font size=\"6\" color=\"red\">iReporter Demonstration</font></i><br>\n");
		Fprintf (fp, "<a href=\"%s\">Order iReporter Here</a>\n<p></center>\n", URL_STORE );
	}
}

void htmlFile::WritePageShadowInit( FILE *fp )
{
	if ( !MyPrefStruct.corporate_look )
	{
		IncrementTableCount( fp );
		Fprintf( fp, "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n<tr>\n\t<td>\n");
	}
}

void htmlFile::WritePageImageLink( VDinfoP VDptr, FILE *fp , const char *imagename, const char *title, int graph, long sort )
{
	if ( graph )
	{
		char	*prefixStr=NULL, *fileprefix=NULL;
		char	*suff;

		if( !imagename || !title || !graph ) return;
		suff = imagesuffixStrings[ MyPrefStruct.image_format ];

		if ( sort == -2 ) {
			fileprefix = "Sessions";
		} else {
			fileprefix = "";
		}

		if ( MyPrefStruct.shadow )
			WritePageShadowInit( fp );

		Fprintf( fp, "<img src=\"%s%s%s\" alt=\"%s Graph\">", fileprefix, imagename, suff, title);

		if ( MyPrefStruct.shadow ){
			if ( strstr( imagename, "World" ) )
				WriteImageShadow( fp, 620, 310 );
			else
				WriteImageShadow( fp, GraphWidth(), GraphHeight() );
		}
	}
}

void htmlFile::WritePageShadow( FILE *fp )
{
	char *suff;

	suff = imagesuffixStrings[ MyPrefStruct.image_format ];

	if ( !MyPrefStruct.corporate_look )
	{
		Fprintf( fp, "</td>\n\t<td height=\"100%%\"><img src=\"rshad%s\" width=\"%d\" height=\"100%%\"></td>\n</tr>\n", suff, ShadowSize() );
		Fprintf( fp, "<tr>\n\t<td><img src=\"bshad%s\" width=\"100%%\" height=\"%d\"></td>\n", suff, ShadowSize() );
		Fprintf( fp, "\t<td><img src=\"cshad%s\" height=\"%d\" width=\"%d\"></td>\n</tr></table>\n\n", suff, ShadowSize(), ShadowSize());
		DecrementTableCount( fp );
	}
}

void htmlFile::WriteImageShadow( FILE *fp, long w, long h )
{
	char *suff;

	suff = imagesuffixStrings[ MyPrefStruct.image_format ];

	if ( !MyPrefStruct.corporate_look )
	{
		Fprintf( fp, "</td>\n\t<td><img src=\"rshad%s\" width=\"%d\" height=\"%d\"></td>\n</tr>\n", suff, ShadowSize(), h );
		Fprintf( fp, "<tr>\n\t<td><img src=\"bshad%s\" width=\"%d\" height=\"%d\"></td>\n", suff, w, ShadowSize() );
		Fprintf( fp, "\t<td><img src=\"cshad%s\" height=\"%d\" width=\"%d\"></td>\n</tr></table>\n\n", suff, ShadowSize(), ShadowSize());
		DecrementTableCount( fp );
	}
}



void htmlFile::Stat_WriteTitle( FILE *fp, long colspan, long list, const char *title )
{
	Fprintf( fp, "<th" );
	if ( colspan>1 )
		Fprintf( fp, " colspan=\"%d\"", colspan );
	Fprintf( fp," align=left>" );
	SetHTMLFont( fp );
	Fprintf( fp,"<font size=\"3\">%s</font></th>\n", title );
}

void htmlFile::Stat_WriteHeader( FILE *fp, const char *txt, long space )
{
	Fprintf( fp,"<th");
	if ( space>1 )
		Fprintf( fp, " colspan=\"%d\"", space );
	Fprintf( fp,">" );

	if ( txt ){
		if ( txt[0] == ' ' && txt[1] == 0  ) // a space character
			Fprintf( fp,"&nbsp; ", txt ); // change space char to html code for netscape browser
		else {
			SetHTMLFont( fp );
			WriteLine( fp, txt );
		}
	}
	Fprintf( fp,"</th>");
}

// write out main frame and secondary list frame

/* writes out the parent frame for the Virtual Hosts */
void htmlFile::WriteVirtualHostFrame (FILE *fi)
{
	//Fprintf (fi, "<!-- Virtual List HEAD -->\n");
	Fprintf (fi, "<html>\n<head>\n<title>" );
	Fprintf (fi, TranslateID(REPORT_VIRTUALHOSTSTITLE) );		// needs to be translated here !!! RHF
	Fprintf (fi, "</title>\n</head>\n\n" );
	Fprintf (fi, "<frameset cols=\"205,*\" frameborder=\"1\" framespacing=\"3\" border=\"3\">\n" );
	Fprintf (fi, "\t<frame src=\"frm_index%s\" name=\"list\" marginwidth=\"0\" marginheight=\"0\" frameborder=\"0\" alt=\"Analysis Software (C) 1997-2002\">\n", fileExtension );
	Fprintf (fi, "\t<frame src=\"%s%s\" name=\"report\" resize>\n", FindReportFilenameStr( VHOST_PAGE ), fileExtension );

	//Fprintf (fi, "<!-- Virtual List TAIL -->\n");
	Fprintf (fi, "</frameset>\n</html>\n");
}

void htmlFile::WriteVirtualHostListBegin (FILE *fi)
{
	//Fprintf (fi, "<!-- Virtual Host head -->\n");
	Fprintf (fi, "<html>\n<head>\n<title>");
	Fprintf (fi, "Domains");								// needs to be translated here !!! RHF
	Fprintf (fi, "</title>\n</head>\n\n");
	WritePageBodyOpen (fi);								
	Fprintf (fi, "<a href=\"%s%s\" target=\"report\"><img src=\"VirtualBanner.gif\" border=\"0\"></a><hr>\n", FindReportFilenameStr( VHOST_PAGE ), fileExtension);
	Fprintf (fi, "<ol>\n");									// open ordered list
															// need to add size tags to graphic when we add new one !!! RHF
}

void htmlFile::WriteVirtualHostListItem (FILE *fi, long realVDnum, char *newVDname, char* fileName, VDinfoP VDPtr, short URLType)
{
	if (URLType == kMappedLink)
		WriteOUT (fi, "\t<li><a href=\"file:///%s/%s\" target=\"report\">%s</a><br>\n", newVDname, fileName, VDPtr->domainName);
	else	// URLType = kNormalLink
		WriteOUT (fi, "\t<li><a href=\"%s/%s\" target=\"report\">%s</a><br>\n", newVDname, fileName, VDPtr->domainName);
}

void htmlFile::WriteVirtualHostListEnd (FILE *fi)
{
	Fprintf (fi, "</ol>\n");								// close ordered list
	WritePageBodyClose (fi);
}

void htmlFile::WritePageFooter( VDinfoP VDptr, FILE *fp )
{
	WriteLine( fp, MyPrefStruct.html_foot ); // write html footer
}



#ifdef DEF_MAC
#pragma mark --- Framed Reports ---
#endif

void htmlFile::Write_FramedMain( FILE *fp, char *indexfile, char *framefile )
{
	char framelink[100], *p;

	Fprintf (fp, "<html>\n<head>\n<title>");
	Fprintf (fp, "iReporter Report");						// needs to be translated here !!! RHF
	Fprintf (fp, "</title>\n</head>\n\n");
	Fprintf (fp, "<frameset cols=\"205,*\" frameborder=\"1\" framespacing=\"3\" border=\"3\">\n");
	Fprintf (fp, "\t<frame src=\"%s%s\" name=\"list\" marginwidth=\"0\" marginheight=\"0\" frameborder=\"0\" alt=\"Analysis Software (C) 1997-2002\">\n", indexfile, fileExtension);

	mystrcpy (framelink, framefile);
	if (p = mystrchr (framelink, '.'))
		*p = 0;
	Fprintf (fp, "\t<frame src=\"%s%s\" name=\"reportstat\" resize>\n", framelink, fileExtension);
	
	Fprintf (fp, "</frameset>\n</html>\n");
}

#ifdef DEF_MAC
#pragma mark --- Sessions ---
#endif

#define	NEWBLACKHR		"<img src=\"blackdot.gif\" width=\"100%\" height=\"5\">"
#define	NEWBLACKHR2		"<img src=\"blackdot.gif\" width=\"100%\" height=\"10\">"
#define	BLACKHR			"<hr>"
#define	BLACKHR2		"<hr>"


#define	FPRINTF(fp,source) fwrite( source, 1, strlen(source), (FILE*)fp )

void WriteHRTag( FILE *fp )
{
	if ( IsDebugMode() )
		FPRINTF( fp, NEWBLACKHR ); 
	else
		FPRINTF( fp, BLACKHR ); 
}

void WriteHR2Tag( FILE *fp )
{
	if ( IsDebugMode() )
		FPRINTF( fp, NEWBLACKHR2 ); 
	else
		FPRINTF( fp, BLACKHR2 ); 
}


static char html_index[8000];
static long html_indexpos = 0;

void htmlFile::WriteRefreshMetaOption( FILE *fp )
{
	// Write REFRESH meta option
	if ( (MyPrefStruct.live_sleeptype > 0) ) {// && m_style == FORMAT_HTML ) {
		char	fileN[512];
		FileFromPath( MyPrefStruct.outfile, fileN);
		WriteOUT( fp, GetHTMLFormat( OUT_REFRESHLINK ), GetSleeptypeSecs( MyPrefStruct.live_sleeptype ), fileN );
	}
}



void htmlFile::SummaryTableStatEntry( FILE *fp, char *text, char *file, long rgb, long normal )
{

	if ( normal == SUMMARY_POPUP )
	{
		html_indexpos = 0;
		IncrementTableCount( fp );
		Fprintf(fp, "<table border=\"0\" width=\"%d\"><tr><td>\n", GraphWidth() );
		Fprintf(fp, "<form name=\"mjump\">\n");
		Fprintf(fp, "<center><font face=\"%s\" size=\"-5\">", MyPrefStruct.html_font );
		Fprintf(fp, "Quick Link Index <select name=\"go\" onchange=\"location.href=document.mjump.go.options[document.mjump.go.selectedIndex]\">" );
		Fprintf(fp, "<option value=\"%s\">Main Page Index</option>\n", text );
		WriteLine(fp, html_index );
		Fprintf(fp, "</select></form></font></center></td></tr></table>\n" );
		DecrementTableCount( fp );
	} else
	{
		Fprintf(fp,"\n<!-- SUMMARY TABLE ENTRY -->\n" );
		if ( normal == SUMMARY_DUALCOLUMN )
		{
			if ( html_count%2 == 0 )	Fprintf(fp,"<tr>" );
			Fprintf(fp,"\n\t<td width=\"50%%\"><p align=\"left\"><font face=\"%s\" size=\"2\">", MyPrefStruct.html_font );
			Fprintf(fp,"<a href=\"%s%s\">%s</a></font>", file, fileExtension, text );
			Stat_WriteSpace( fp, 1 );
			WriteHRTag( fp );
			Fprintf(fp,"</td>\n" );
			if ( html_count%2 == 1 )	Fprintf(fp,"</tr>" );
			html_count++;
		} else
		if ( normal == SUMMARY_FRAMED )
		{
			Fprintf(fp,"<tr>\n\t<td width=\"100%%\"><p align=\"left\"><font face=\"%s\" size=\"2\">", MyPrefStruct.html_font );
			Fprintf(fp,"<a href=\"%s%s\" target=\"reportstat\">%s</a></font></td>\n</tr>\n", file, fileExtension, text );
		} else
		if ( normal == SUMMARY_NONFRAMED )
		{
			Fprintf(fp,"<tr>\n\t<td width=\"100%%\"><p align=\"left\"><font face=\"%s\" size=\"2\">", MyPrefStruct.html_font );
			Fprintf(fp,"<a href=\"%s%s\">%s</a></font></td>\n</tr>\n", file, fileExtension, text );
		}
		//<option value=\"%s%s\">%s</option>
		//html_indexpos += sprintf( html_index+html_indexpos,"<option value=\"%s%s\">%s</option>\n", file, fileExtension, text );
		//html_indexpos += sprintf( html_index+html_indexpos," | <a href=\"%s%s\">%s</a>", file, fileExtension, text );
	}
}

/*
// This is better suited for use for links for all types since it uses names from the table.
// RS,29May
void htmlFile::SummaryTableStatName( FILE *fp, char *id, long rgb, long normal )
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

void htmlFile::IncrementTableCount( FILE *fp )
{
	tableLevel++;
#ifdef DEF_DEBUG
	Stat_WriteCommentArgs( fp,"<!-- TABLE START (level %d) -->\n", tableLevel );
#endif
}

void htmlFile::DecrementTableCount( FILE *fp )
{
#ifdef DEF_DEBUG
	Stat_WriteCommentArgs( fp,"<!-- TABLE FINISH (level %d) -->\n\n", tableLevel );
#endif
	tableLevel--;
}

void htmlFile::SummaryTableStart( FILE *fp, long width, long centre )
{
	if ( centre )
		Fprintf(fp,"\n\n");

	IncrementTableCount( fp );
	if ( width )
		Fprintf(fp,"<table width=\"%d\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n", width );
	else
		Fprintf(fp,"<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"1\">\n" );
}

void htmlFile::SummaryTableFinish( FILE *fp )
{
	Fprintf( fp, "</table>\n" );
	DecrementTableCount( fp );
}

void htmlFile::SummaryTableTitle( FILE *fp, const char *text, long rgb )
{
	if ( rgb != -1 )
		Fprintf(fp,"<tr>\n\t<td></td>\n\t<td colspan=\"2\" bgcolor=\"#%06x\"><a name=\"%s\"><b><font face=\"%s\" color=\"#ffffff\" size=\"3\">%s</font></b></a></td>\n</tr>\n", rgb, text, MyPrefStruct.html_font, text );
	else
		Fprintf(fp,"<tr>\n\t<td></td>\n\t<td colspan=\"2\"><b><font face=\"%s\" color=\"#000000\" size=\"3\"><a name=\"%s\">%s</a></font></b></td>\n</tr>\n", MyPrefStruct.html_font, text, text );
	Fprintf(fp,"<tr>\n\t<td colspan=\"3\">" );
	WriteHR2Tag( fp );
	Fprintf(fp,"</td>\n</tr>\n" );
}

void htmlFile::SummaryTableSubTitle( FILE *fp, const char *text, int x, int y )
{
	Fprintf(fp,"<tr>\n\t<td height=\"17\" colspan=\"2\"><p align=\"left\"><br><b>" );
	Fprintf(fp,"<font face=\"%s\" color=\"#000000\">%s</font></b><hr></td>\n</tr>\n", MyPrefStruct.html_font, text );
}

void htmlFile::SummaryTableRow( FILE *fp, long size )
{
	if ( size )
		Fprintf(fp,"<tr>\n\t<td width=\"%d%%\" valign=\"top\">\n", size);
	else
		Fprintf(fp,"\t<td valign=\"top\">\n");

	SetHTMLFont( fp );
}

void htmlFile::SummaryTableRowEnd( FILE *fp )
{
	Fprintf(fp,"</tr>\n<!-- ROW END -->\n");
}

void htmlFile::SummaryTableData( FILE *fp, long width )
{
	if ( width )
		Fprintf(fp,"\t<td width=\"%d%%\" valign=\"top\">\n<!-- DATA START -->\n", width );
	else
		Fprintf(fp,"\t<td valign=\"top\">\n<!-- DATA START -->\n");

	SetHTMLFont( fp );
}

void htmlFile::SummaryTableDataEnd( FILE *fp )
{
	Fprintf(fp,"</td>\n<!-- DATA END -->\n\n");
}

void htmlFile::SummaryTableHeader( FILE *fp, long width, long height )
{
	Fprintf(fp,"\n\n");
	IncrementTableCount( fp );
	Fprintf(fp,"<table width=\"%d\" border=\"0\" cellspacing=\"8\" cellpadding=\"0\">\n", width );
}

/* WriteReportBanner - print summary of total day statistics */
void htmlFile::WriteReportBanner( VDinfoP VDptr )
{
	char	number[512];
	char	newFile[512];

	// save the blackdot gif, which is used for HR replacement with more control.
	if ( IsDebugMode() ){
		if ( totalDomains > 0 && VDptr ) {
			sprintf( number, "%s%cblackdot.gif", GetDomainPath(VDptr), PATHSEP );
			CopyFilenameUsingPath( newFile, MyPrefStruct.outfile, number );
			ReplacePathFromHost( GetDomainPath(VDptr), newFile );
		} else
			CopyFilenameUsingPath( newFile, MyPrefStruct.outfile, "blackdot.gif" );
		MemToFile( newFile, black_line_data, sizeof( black_line_data ) );
		AddFileHistory(newFile);
	}

	if ( strstr( MyPrefStruct.html_head, "internal_logo.gif" )) {
		if ( totalDomains > 0 && VDptr ) {
			sprintf( number, "%s%cinternal_logo.gif", GetDomainPath(VDptr), PATHSEP );
			CopyFilenameUsingPath( newFile, MyPrefStruct.outfile, number );
			ReplacePathFromHost( GetDomainPath(VDptr), newFile );
		} else
			CopyFilenameUsingPath( newFile, MyPrefStruct.outfile, "internal_logo.gif" );
			
		if ( GetFileLength( newFile ) == 0 ) 
		{	// if file is not there, save it there...
			CopyFile( "logo.gif", newFile, true );
		} else {
			//add it to the list of files anyway so we can upload it if needed
			AddFileHistory(newFile);
		}	
	}
}


void htmlFile::QuickLinkIndexStart( VDinfoP VDptr, FILE *fp )
{
	if ( fp ){
		long num, c=0;
        char *ext,indexfile[64];

		//calculate days elapsed
		VDptr->totalDays = num = (long)(VDptr->lastTime/ONEDAY)-(VDptr->firstTime/ONEDAY)+1;
		SummaryTableHeader( fp, 0, 0 );
		SummaryTableRow( fp, 20 );

		BeginSummaryTable( fp, 180, /*"Statistics"*/ TranslateID(RCAT_STATISTICS), c );

        FileFromPath(MyPrefStruct.outfile, indexfile );
		DateFixFilename( indexfile, NULL );		// fixup indexfile tokens

        ext = strrchr(indexfile,'.');
        if (ext)
            *ext = 0;
		
		SummaryTableStatEntry( fp, /*"Summary"*/ TranslateID(REPORT_SUMMARY), indexfile, 0, SUMMARY_NONFRAMED );
		EndSummaryTable( fp );
		WriteSummaryHTMLLinks( VDptr, fp, num, 180, SUMMARY_NONFRAMED );
		SummaryTableDataEnd( fp );
		SummaryTableData( fp, 80 );
	}
}

void htmlFile::QuickLinkIndexEnd( FILE *fp )
{
	if ( fp ){
		SummaryTableRowEnd( fp );
		SummaryTableFinish( fp );
	}
}


void htmlFile::ProduceHTMLInternalPageLinks( VDinfoP VDptr, FILE *fp )
{
	SummaryTableEntry( fp, TranslateID(SUMM_DETAILEDREPORTS), NULL, -1 , 2 );
	if ( AnyTrafficStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_TRAFFIC), NULL, 2 );
	if ( AnyDiagnosticStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_DIAGNOSTIC), NULL, 2 );
	if ( AnyServerStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_SERVER), NULL, 2 );
	if ( AnyDemographicStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_DEMOGRAPH), NULL, 2 );
	if ( AnyReferralStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_REFERRALS), NULL, 2 );
	if ( AnyStreamingMediaStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_STREAMING), NULL, 2 );
	if ( AnySystemsStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_SYSTEMS), NULL, 2 );
#ifdef DEF_FULLVERSION
	if ( AnyAdvertisingStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_ADVERT), NULL, 2 );
#endif
	if ( AnyMarketingStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_MARKETING), NULL, 2 );
	if ( AnyClustersStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_CLUSTERS), NULL, 2 );
#ifdef DEF_APP_FIREWALL
	if ( AnyFirewallStatsTurnedOn( VDptr, &MyPrefStruct ) )
		SummaryTableEntryURL( fp, TranslateID(RCAT_FIREWALL), NULL, 2 );
#endif // DEF_APP_FIREWALL
}

#ifdef DEF_MAC
#pragma mark --- text formatting ---
#endif

void htmlFile::Stat_WriteText( FILE *fp, short cols, long rgb, const char *name )
{
	char	temp[MAXURLSIZE], *p = temp;

	p += sprintf( p, "\t<td align=\"left\" " );
	if ( rgb != -1 )
		p+= sprintf( p, "bgcolor=\"#%06lx\" ", rgb  );
	if( cols>1 )
		p += sprintf( p, "colspan=\"%d\" ", cols );
	p += sprintf( p,">" );
	WriteLine( fp, temp );
	SetHTMLFont( fp );
	Fprintf(fp,"%s</td>\n", name );
}

void htmlFile::Stat_WriteSpace( FILE *fp, long size )
{
	while( size-- )
		Fprintf( fp, "<br>\n", size );
}

void htmlFile::Stat_WriteAnchor( FILE *fp, const char* szAnchor )
{
	Fprintf( fp, "<a name=\"%s\"></a>\n", szAnchor );
}

void htmlFile::Stat_WriteAnchor( FILE *fp, long a )
{
	Fprintf( fp, "<a name=\"%d\"></a>\n", a );
}

void htmlFile::Stat_WriteItal( FILE *fp, const char *txt )
{
	Fprintf( fp, "<i>%s</i><br>\n", txt );
}

void htmlFile::Stat_WriteBold( FILE *fp, const char *txt )
{
	Fprintf( fp, "<b>%s</b><br>\n", txt );
}

void htmlFile::Stat_WriteBoldDual( FILE *fp, const char *txt, const char *txt2 )
{
	Fprintf( fp, "<tr><td>" );
	SetHTMLFont( fp );
	Fprintf( fp, "<b>%s</b></td>\n", txt );

	WriteCellStartRight( fp );
	Fprintf( fp, "<b>%s</b><br></td>\n</tr>\n", txt2 );
}

void htmlFile::Stat_WriteCenterSmall( FILE *fp, const char *txt )
{
	Fprintf( fp, "<center><font size=\"-2\">%s</font></center></p>\n", txt );
}

void htmlFile::Stat_WriteNumberData( FILE *fp, long num )
{
	char	numStr[32];
	FormatLongNum( num, numStr );
	WriteCellStartRight( fp );
	SetHTMLFont( fp );
	Fprintf( fp,"%s</td>\n", numStr );
}

void htmlFile::Stat_WriteFractionData( FILE *fp, long num, long num2 )
{
	char	numStr[32];
	char	numStr2[32];

	FormatLongNum( num, numStr );
	FormatLongNum( num2, numStr2 );

	WriteCellStartRight( fp );
	SetHTMLFont( fp );

	Fprintf( fp,"%s", numStr );

	if ( num != num2 )
		Fprintf( fp," %s %s", TranslateID(LABEL_OF), numStr2 );

	Fprintf( fp,"</td>\n" );
}

void htmlFile::Stat_WriteFloatData( FILE *fp, double num )
{
	char	numStr[32];

	Double2CStr( num, numStr, 1 );
	FormatNum( numStr, 0 );

	WriteCellStartRight( fp );
	SetHTMLFont( fp );
	Fprintf( fp,"%s</td>\n", numStr );
}







void htmlFile::Stat_WriteCentreHeading( FILE *fp, const char *txt )
{
	Fprintf( fp, "<h3 align=\"center\">%s</h3>\n", txt );
}

void htmlFile::Stat_WriteRowStart( FILE *fp, long rgb, long col )
{
	Fprintf( fp,"<tr bgcolor=\"#%06x\">\n", rgb );
}

void htmlFile::Stat_WriteRowEnd( FILE *fp )
{
	Fprintf( fp,"</tr>\n" );
	Fprintf( fp,"\n" );
}

void htmlFile::Stat_WriteTableStart( FILE *fp, long tabwidth)
{
	IncrementTableCount( fp );
	Fprintf( fp, "<table border=\"0\" width=\"%d\" cellspacing=\"0\" cellpadding=\"0\">\n<tr>\n\t<td bgcolor=\"#000000\">\n", tabwidth);
	IncrementTableCount( fp );
	Fprintf( fp, "<table border=\"0\" cellspacing=\"%d\" cellpadding=\"2\" width=\"100%%\">\n", 1);	
}

void htmlFile::Stat_WriteTableStart( FILE *fp, long tabwidth, int iCellSpacing)
{
	IncrementTableCount( fp );
	Fprintf( fp, "<table border=\"0\" width=\"%d\" cellspacing=\"0\" cellpadding=\"0\">\n<tr>\n\t<td bgcolor=\"#000000\">\n", tabwidth);
	IncrementTableCount( fp );
	Fprintf( fp, "<table border=\"0\" cellspacing=\"%d\" cellpadding=\"2\" width=\"100%%\">\n", iCellSpacing);	
}

void htmlFile::Stat_WriteTableEnd( FILE *fp )
{
	Fprintf(fp,"</table>\n");
	DecrementTableCount( fp );
	Fprintf(fp,"</table>\n");
	DecrementTableCount( fp );
}


void htmlFile::Stat_WriteIconLink( FILE* fp, const char* szIcon, const char* szLinkReportName, const char* szLinkExtension, const char* szPopHelp, bool bLeftAlign)
{
	DEF_ASSERT(fp);
	DEF_ASSERT(szIcon);
	DEF_ASSERT(szLinkReportName);
	DEF_ASSERT(szPopHelp);

	Fprintf(fp, "<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" width=%d>\n", GraphWidth());

	Fprintf(fp, "<TD colspan=1 align=%s valign=top>\n", (bLeftAlign?"left":"right"));

	if (szLinkReportName)
	{
		if (szLinkExtension)
			Fprintf(fp, "	<a href=\"%s.html#%s\">\n", szLinkReportName, szLinkExtension);
		else
			Fprintf(fp, "	<a href=\"%s\">\n", szLinkReportName);
	}	
	
	Fprintf(fp, "	<img src=\"%s\" alt=\"%s\" border=0>\n", szIcon, szPopHelp);
	
	if (szLinkReportName)
		Fprintf(fp, "	</a>\n");
	
	Fprintf(fp, "</TD>\n</table>\n");
}

void htmlFile::Stat_WriteIconAndText(  FILE* fp, const char* szText,  const char* szIcon, const char* szPopHelp)
{
	Fprintf(fp, "	<TD colspan=1>\n");
	Fprintf(fp, "	<table width=100%% cellspacing=0 cellpadding=0><tr>\n");
	Fprintf(fp, "		<TD colspan=1 align=left valign=top width=5%%><img src=\"%s\" alt=\"%s\"></TD>\n", szIcon, szPopHelp);
	Fprintf(fp, "		<TD colspan=1 align=left width=95%%><font face=\"%s\" size=\"%d\">%s</font></TD>\n", MyPrefStruct.html_font, MyPrefStruct.html_fontsize, szText );
	Fprintf(fp, "	</tr></table>\n");
	Fprintf(fp, "	</TD>\n");
}


void htmlFile::BuildHelpCardText(const class CQHelpCardText& rCardText, std::string& rstr)
{
	CQHelpCardText::const_iterator	it;

	rstr = "";

	for (it = rCardText.begin(); it!=rCardText.end(); ++it)
	{
		if (it->first & HELPTEXT_FORMATMASK_BOLD)		rstr += STARTTAG_BOLD;
		if (it->first & HELPTEXT_FORMATMASK_ITALICS)	rstr += STARTTAG_ITALICS;
		if (it->first & HELPTEXT_FORMATMASK_UNDERLINE)	rstr += STARTTAG_UNDERLINE;
		rstr += (*it).second;
		if (it->first & HELPTEXT_FORMATMASK_BOLD)		rstr += ENDTAG_BOLD;
		if (it->first & HELPTEXT_FORMATMASK_ITALICS)	rstr += ENDTAG_ITALICS;
		if (it->first & HELPTEXT_FORMATMASK_UNDERLINE)	rstr += ENDTAG_UNDERLINE;

		if (it->first & HELPTEXT_FORMATMASK_NEWLINE)
		{
			rstr += "\n";
			rstr += TAG_NEWLINE;
		}
	}
}

void htmlFile::WriteHelpCardIcons(VDinfoP VDptr)
{
	std::string	strFileName;
	FILE*		fIcon;
	char		szPath[MAXFILENAMESSIZE+1];
	
	szPath[0] = szPath[MAXFILENAMESSIZE] = 0;

	GetReportPath(VDptr, szPath, MAXFILENAMESSIZE);


	strFileName =	szPath;
	strFileName +=	"Top.gif";
	fIcon = Fopen( (char*)strFileName.c_str(), "wb");
	if (fIcon)
	{
		::fwrite(QS::g_ucIcon_Top,  sizeof(unsigned char), QS::g_ulSize_Top, fIcon);
	}
	(void)fclose(fIcon);
	(void)fflush(fIcon);


	strFileName =	szPath;
	strFileName +=	"Help.gif";
	fIcon = Fopen((char*)strFileName.c_str(), "wb");
	if (fIcon)
	{
		::fwrite(QS::g_ucIcon_HelpCard,  sizeof(unsigned char), QS::g_ulSize_HelpCard, fIcon);
	}
	(void)fclose(fIcon);
	(void)fflush(fIcon);


	strFileName =	szPath;
	strFileName +=	"OverviewIcon.gif";
	fIcon = Fopen((char*)strFileName.c_str(), "wb");
	if (fIcon)
	{
		::fwrite(QS::g_ucIcon_Overview,  sizeof(unsigned char), QS::g_ulSize_Overview, fIcon);
	}
	(void)fclose(fIcon);
	(void)fflush(fIcon);


	strFileName =	szPath;
	strFileName +=	"GraphIcon.gif";
	fIcon = Fopen((char*)strFileName.c_str(), "wb");
	if (fIcon)
	{
		::fwrite(QS::g_ucIcon_Graph,  sizeof(unsigned char), QS::g_ulSize_Graph, fIcon);
	}
	(void)fclose(fIcon);
	(void)fflush(fIcon);


	strFileName =	szPath;
	strFileName +=	"TableIcon.gif";
	fIcon = Fopen((char*)strFileName.c_str(), "wb");
	if (fIcon)
	{
		::fwrite(QS::g_ucIcon_Table,  sizeof(unsigned char), QS::g_ulSize_Table, fIcon);
	}
	(void)fclose(fIcon);
	(void)fflush(fIcon);

}

void htmlFile::Stat_WriteCenterStart( FILE* fp )
{
	Fprintf(fp, "<center>\n");
}

void htmlFile::Stat_WriteCenterEnd( FILE* fp )
{
	Fprintf(fp, "</center>\n");
}

void htmlFile::Stat_WriteHelpCard(FILE *fp, const class CQHelpCard& rCQHelpCard, bool bTableOn, bool bGraphOn)
{
	// ****************************************************************************
	// We have loaded the CQHelpCard object with the contents from the file and 
	// we are okay to proceed.
	// ****************************************************************************
	size_t		nTabWidth	= GraphWidth();


	// Load it into a CQHelpCardText object so that we can get a 'generic' version.
	std::string		str;
	CQHelpCardText	text;
	text.load(rCQHelpCard.GetTitle());

	// ******************************************************************************
	// If there is no title string then do NOT produce a HelpCard.
	// ******************************************************************************
	if (!text.size())
	{
		return;
	}
	
	// ******************************************************************************
	// If there is not other cell that is going to be produced then do NOT produce a HelpCard.
	// ******************************************************************************
	if (!rCQHelpCard.GetOverview().size()	&&
		!rCQHelpCard.GetGraph().size()		&&
		!rCQHelpCard.GetTable().size()		)
	{
		return;
	}
	
	// Convert it to a string. (clobbers the previous content of str).
	BuildHelpCardText(		text, str);
	
	// Title
	size_t rows = 2; // one for the heading, and one for the "overall Report description"
	if (bGraphOn)
		rows++;
	if (bTableOn)
		rows++;


	// ******************************************************************************
	// Centre the HelpText.
	// ******************************************************************************
	Stat_WriteCenterStart( fp );

	// ******************************************************************************
	// Start the HelpCard
	// ******************************************************************************
	Stat_WriteTableStart(	fp, nTabWidth );

	// ******************************************************************************
	// Write the Title section
	// ******************************************************************************
	Stat_WriteRowStart(		fp, RGBtableTitle, 1 );
	Stat_WriteTitle(		fp, 1, rows, str.c_str() );
	Stat_WriteRowEnd(		fp);
	
	
	// ******************************************************************************
	// Write the Overview section
	// ******************************************************************************
	if (rCQHelpCard.GetOverview().size())
	{
		BuildHelpCardText(		rCQHelpCard.GetOverview(), str);
		Stat_WriteRowStart(	fp, RGBHelpCardOverview, 1);
		Stat_WriteIconAndText( fp, str.c_str(), "OverviewIcon.gif", "Report Overview");
		Stat_WriteRowEnd(		fp);
	}

	if (bGraphOn && rCQHelpCard.GetGraph().size())
	{
		// ******************************************************************************
		// Write the Graph section
		// ******************************************************************************
		BuildHelpCardText(		rCQHelpCard.GetGraph(), str);
		Stat_WriteRowStart(	fp, RGBHelpCardGraph, 1);
		Stat_WriteIconAndText( fp, str.c_str(), "GraphIcon.gif", "Graph Description");
		Stat_WriteRowEnd(		fp);
	}

	if (bTableOn && rCQHelpCard.GetTable().size())
	{
		// ******************************************************************************
		// Write the Table section
		// ******************************************************************************
		BuildHelpCardText(		rCQHelpCard.GetTable(), str);
		Stat_WriteRowStart(	fp, RGBHelpCardTable, 1);
		Stat_WriteIconAndText( fp, str.c_str(), "TableIcon.gif", "Table Description");
		Stat_WriteRowEnd(		fp);
	}


	// ******************************************************************************
	// We are done writing this HelpCard (table).
	// ******************************************************************************
	Stat_WriteTableEnd( fp );
	Stat_InsertTable();


	// ******************************************************************************
	// This is for the icon-link back to the refering table.
	// As we do not want a border around this row we need to make it a separate table
	// ******************************************************************************
	Stat_WriteIconLink(fp, "Top.gif", HTMLLinkFilename.c_str(), NULL, "Go back to the top.", false);
	Stat_InsertTable();

	// ******************************************************************************
	// End Centre the HelpText.
	// ******************************************************************************
	Stat_WriteCenterEnd( fp );

}

static char html_fonttag[256];
#define	DEFINEHTMLFONT 1

void htmlFile::Stat_WriteColURLjump( FILE *fp, long cols, long rgb, const char *url, const char *name  )
{
	char temp[MAXURLSIZE], *p = temp;

	p += sprintf( p,"\t<td valign=\"left\"" );
	if ( rgb != -1 )
		p+= sprintf( p," bgcolor=\"%06lx\"", rgb  );
	if( cols>1 )
		p += sprintf( p," colspan=\"%d\"", cols );
	p += sprintf( p,">" );

#ifdef DEFINEHTMLFONT
	p += sprintf( p, html_fonttag );
#endif

	if( url )
		p += sprintf( p,"<a href=\"%s\" target=\"_new\">", url );
	p += sprintf( p,"<font color=\"#000000\">%s</font>", CleanURLString(name) );
	if( url )
		p += sprintf( p,"</a>" );
	p += sprintf( p,"</td>\n" );
	WriteLine( fp, temp );
}

void htmlFile::Stat_WriteLink( FILE *fp, const char *url, const char *name, long num  )
{
	char temp[MAXURLSIZE], *p = temp;

	p += sprintf( p,"%d.  ", num );
	p += sprintf( p,"<a href=\"%s\">", url );
	p += sprintf( p,"%s", CleanURLString(name) );
	p += sprintf( p,"</a><br>" );
	WriteLine( fp, temp );
}

void htmlFile::Stat_WriteURL( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	char temp[MAXURLSIZE], *p = temp;

	if ( url ) {
		if ( !*url ) url = NULL;
	}
	p += sprintf( p,"\t<td align=\"left\"" );
	if ( rgb != -1 )
		p+= sprintf( p," bgcolor=\"%06lx\"", rgb  );
	if( cols>1 )
		p += sprintf( p," colspan=\"%d\"", cols );
	p += sprintf( p,">" );
#ifdef DEFINEHTMLFONT
	p += sprintf( p, html_fonttag );
#endif
	WriteLine( fp, temp );

	if( url ){
		WriteLine( fp, "<a href=\"" );
		WriteLine( fp, url );
		WriteLine( fp, "\">" );
	}

	p = temp;
	p += sprintf( p,"<font color=\"#000000\">%s</font>", CleanURLString(name) );
	if( url )
		p += sprintf( p,"</a>" );
	p += sprintf( p,"</td>\n" );
	WriteLine( fp, temp );
}

void htmlFile::Stat_WriteURLRight( FILE *fp, short cols, long rgb, const char *url, const char *name  )
{
	Fprintf(fp,"\t<td align=\"right\"" );
	if ( rgb != -1 )
		Fprintf(fp," bgcolor=\"#%06lx\"", rgb  );
	if( cols>1 )
		Fprintf(fp," colspan=\"%d\"", cols );
	Fprintf(fp,">" );
	SetHTMLFont( fp );
	Fprintf(fp,"<a href=\"%s\">%s</a></td>\n", url, CleanURLString(name) );
}

void htmlFile::Stat_WriteTextTooltip( FILE *fp, short cols, const char *name, const char *tooltip )
{
	if ( tooltip )
		Fprintf(fp,"\t<td align=\"left\" colspan=\"%d\" title=\"%s\">%s</td>\n",cols, tooltip, name );
	else
		Fprintf(fp,"\t<td align=\"left\" colspan=\"%d\">%s</td>\n",cols, name );
}

void htmlFile::Stat_WriteRight( FILE *fp, short cols, long rgb, const char *name )
{
	char temp[MAXURLSIZE];
	char *p = temp;
	p += sprintf( p, "\t<td align=\"right\" " );
	if ( rgb != -1 )
		p+= sprintf( p, "bgcolor=\"#%06lx\" ", rgb  );
	if( cols>1 )
		p += sprintf( p, "colspan=\"%d\" ", cols );
	p += sprintf( p,">" );
	WriteLine( fp, temp );
	SetHTMLFont( fp );
	Fprintf( fp,"%s</td>\n", name );
}

void htmlFile::Stat_WriteDualText( FILE *fp, char *name, char *name2 )
{
	Fprintf(fp,"\t<td align=\"right\" colspan=\"%d\">", 2 );
	SetHTMLFont( fp );
	// OLD
	//Fprintf(fp,"%s <font size=\"-4\">%s</font>", name, name2 );

	Fprintf(fp,"<TABLE width=100%% cellspacing=0 cellpadding=0><TR><TD align=right width=50%%>" );
	SetHTMLFont( fp );
	Fprintf(fp,"%s</TD><TD align=right width=50%%>", name );

	SetHTMLFont( fp );
	Fprintf(fp,"<font size=\"-4\">&nbsp;&nbsp;&nbsp;%s</font></TD></TR></TABLE>", name2 );


	Fprintf(fp,"</td>\n" );
}

void htmlFile::SetHTMLFont( FILE *fp )
{
#ifdef DEFINEHTMLFONT
	sprintf( html_fonttag, "<font face=\"%s\" size=\"%d\">\n", MyPrefStruct.html_font, MyPrefStruct.html_fontsize );
	if ( fp )
		Fwrite( fp, html_fonttag );
#endif
}

void htmlFile::WriteCellStartRight( FILE *fp )
{
	Fwrite( fp, "\t<td align=\"right\">" );
}

void htmlFile::WriteCellStartLeft( FILE *fp )
{
	Fwrite( fp, "\t<td align=\"left\">" );
	SetHTMLFont( fp );
}

void htmlFile::WriteCellEnd( FILE *fp )
{
	Fwrite( fp, "</td>\n" );
}
		
void htmlFile::Stat_WriteLine( FILE *fp, const char *txt )
{
	if ( txt[0] == ' ' && txt[1] == 0  ) // a space character
		Fprintf( fp,"<th>&nbsp;</th>", txt ); // change space char to html code for netscape browser
	else
		Fprintf( fp, "%s<br>\n", txt );
}

void htmlFile::Stat_WriteComment( FILE *fp, const char *name  )
{
	if ( 1 ) // showComments
	{
		WriteLine( fp,"\n<!-- ");
		WriteLine( fp, name );
		WriteLine( fp," -->\n\n");
	}
}

int htmlFile::Stat_WriteCommentArgs( FILE *fp, const char *fmt, ... )
{
	va_list		args;
static	char lineout[1024];

	va_start( args, fmt );
	vsprintf( lineout, fmt, args );
	va_end( args );

	return WriteLine( fp, lineout );
}

extern short logType;
char *GetLogTypeName( long logType, char *txt );

void htmlFile::WriteDebugComments( FILE *fp, VDinfoP VDptr )
{
	char name[512];
	char outBuffer[512];

	// secret HTML internal comments
	WriteLine( fp, "<!-- \n" );
	sprintf( outBuffer, "\tTotal Requests = %ld\n\tTotal Data In = %ld\n\ttime taken = %.02f\n\tstartTime = %d\n\tlastTime = %d\n\n",
		VDptr->totalRequests, VDptr->totalInDataSize, VDptr->time2, VDptr->firstTime, VDptr->lastTime );
	WriteLine( fp, outBuffer );
	sprintf( outBuffer, "\tLogformat = %s\n", GetLogTypeName( logType, name ) );
	WriteLine( fp, outBuffer );
	sprintf( outBuffer, "\tHost = %s\n", VDptr->domainName );
	WriteLine( fp, outBuffer );
	WriteLine( fp, "\n-->\n" );
}

char *htmlFile::GetEngineLogo( FILE *fp, char *engine, char *p )
{
	char *file = 0;
	if ( strstr( engine, "altavista" ) ) file = "http://altavista.com/av/gifs/new/front_hdr.gif"; else
	if ( strstr( engine, "google" ) ) file = "http://www.google.com/intl/en_extra/images/Title_Left.gif"; else
	if ( strstr( engine, "yahoo" ) ) file = "http://yahoo.com/images/yahoo.gif"; else
	if ( strstr( engine, "lycos" ) ) file = "http://www.lycos.com/assist/graphics/lycoslogo.gif"; else
	if ( strstr( engine, "webcrawler" ) ) file = "http://www.webcrawler.com/img/art4/head/wcdirect.gif"; else
	if ( strstr( engine, "infoseek" ) ) file = "http://infoseek.go.com/images/tmpl/Infoseekw_82x82.gif"; else
	if ( strstr( engine, "excite" ) ) file = "http://excite.com/img/head/logo.gif"; else
	if ( strstr( engine, "hotbot" ) ) file = "http://s.hotbot.com/images/hblogo7.gif"; else
	if ( strstr( engine, "anzwers" ) ) file = "http://www.anzwers.com.au/images/header_logo.gif"; else
	if ( strstr( engine, "aolsearch" ) ) file = "http://aolsearch.aol.com/gr/logo_service_yellow.gif"; else
	if ( strstr( engine, "netfind" ) || strstr( engine, "aol" ) ) file = "http://www.aol.com/gr/nflrglavtxt.gif"; else
	if ( strstr( engine, "freeserve" ) ) file = "http://www.freeserve.net/images/cserve_logo.gif"; else
	if ( strstr( engine, "search.cnet" ) ) file = "http://www.cnet.com/i/se/hd_lg.gif"; else
	if ( strstr( engine, "metacrawler" ) ) file = "http://images.go2net.com/go2net/images/metacrawler_new.gif"; else
	if ( strstr( engine, "cyber411" ) ) file = "http://www.cyber411.com/images/2_logos.gif"; else
	if ( strstr( engine, "northernlight" ) ) file = "http://northernlight.com/docs/gif/logo.gif"; else
	if ( strstr( engine, "alltheweb" ) ) file = "http://www.ussc.alltheweb.com/graphics/logo.gif"; else
	if ( strstr( engine, "looksmart" ) ) file = "http://www.looksmart.com/i/ls_logo_title.gif"; else
	if ( strstr( engine, "evreka" ) ) file = "http://evreka.com/q/img/logo_index.gif"; else
	if ( strstr( engine, "infind" ) ) file = "http://infind.com/images/infind.gif"; else
	if ( strstr( engine, "goto" ) ) file = "www.goto.com/images/home/logo6.gif"; else
	if ( strstr( engine, "overture" ) ) file = "www.overture.com/images/home/logo6.gif"; else
	if ( strstr( engine, "thunderstone" ) ) file = "http://www.thunderstone.com/webinator/swdrmlog.gif"; else
	if ( strstr( engine, "mamma" ) ) file = "http://www.mamma.com/mamma.gif"; else
	if ( strstr( engine, "smallbizsearch" ) ) file = "http://www.smallbizsearch.com/graphics/hdr_generic.gif";
			
	if ( file )
		sprintf( p, "<center><img src=\"%s\"></center>\n", file );

	return file;
}


/***********************************************************
		SERVER SUMMARY FUNCTIONS
  *********************************************************/
void htmlFile::SummaryTableEntryURL( FILE *fp, const char *text, const char *url, long normal )
{
	long fs=-2;
	if ( normal )
		fs = normal;

	Fprintf(fp,"<tr>\n\t<td width=\"50%%\"><font face=\"%s\" size=\"%d\">\n", MyPrefStruct.html_font, fs );

	if ( url )
		Fprintf(fp,"<a href=\"%s\">%s</a>", url, text );
	else
		Fprintf(fp,"<a href=\"#%s\">%s</a>", text, text );
	Fprintf(fp,"</font><br></td>\n</tr>\n" );
}

void htmlFile::SummaryTableEntry( FILE *fp, const char *text, const char *number, long rgb, long normal )
{
	long fs=-2;
	if ( normal ) fs = normal;

	Fprintf(fp,"<tr>\n\t<td width=\"50%%\" valign=top><font face=\"%s\" size=\"%d\">%s</font></td>\n", MyPrefStruct.html_font, fs, text );

	if ( number )
		Fprintf(fp,"\t<td valign=\"top\"><p align=\"right\"><font face=\"%s\" size=\"%d\">%s</font></td>\n", MyPrefStruct.html_font, fs, number );

	Fprintf(fp,"</tr>\n" );
}

void htmlFile::SummaryTableSmallTitle( FILE *fp, char *text )
{
	switch( m_style ){
		case FORMAT_HTML:
			Fprintf(fp,"<tr>\n\t<td colspan=\"3\"><br></td>\n</tr>\n" );
			Fprintf(fp,"<tr>\n\t<td colspan=\"3\"><b><font face=\"%s\" color=\"#000000\" size=\"2\">\n", MyPrefStruct.html_font );
			Fprintf(fp,"<a name=\"%s\">%s</a></font></b><br><hr></td>\n</tr>\n", text,text );
			break;
	}
}

// added RS 15/6/00
#define	SUMMARY_LINK( id )	SummaryTableStatEntry( fp, FindReportTitleStr(id), FindReportFilenameStr(id), -1, c );

/* WriteReportServerSummary - print summary of total day statistics */
void htmlFile::WriteReportServerSummary( VDinfoP VDptr, FILE *fp, long num, long tw, long c )
{
	long		line=0, rgbcolor[2]={ 0xf0f0f0, 0xe0e0e0 };

	//server statistics
	line = 0;
	if ( AnyTrafficStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, /*"Traffic"*/ TranslateID(RCAT_TRAFFIC), c );
		if (VDptr->Done.Hour && ANYSTAT(MyPrefStruct.stat_hourly) )
			SUMMARY_LINK( HOUR_PAGE );
		if (VDptr->Done.Hour && ANYSTAT(MyPrefStruct.stat_hourlyHistory) )
			SUMMARY_LINK( HOURHIST_PAGE );
		if (VDptr->byDate->GetStatListNum()>1 && ANYSTAT(MyPrefStruct.stat_daily) ) {
			SUMMARY_LINK( DATE_PAGE );
			if (VDptr->byDate->GetStatListNum()>31)
				SUMMARY_LINK( RECENTDATE_PAGE );
		}
		if (VDptr->Done.Week && ANYSTAT( MyPrefStruct.stat_weekly) )
			SUMMARY_LINK( WEEK_PAGE );
		if (VDptr->Done.Month>1 && ANYSTAT( MyPrefStruct.stat_monthly) )
			SUMMARY_LINK( MONTH_PAGE );
		EndSummaryTable( fp );
	}

	if ( AnyDiagnosticStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, /*"Diagnostics"*/ TranslateID(RCAT_DIAGNOSTIC), c );
		if (VDptr->Done.Errors && ANYSTAT(MyPrefStruct.stat_errors) ) {
			SUMMARY_LINK( ERRORS_PAGE );
			if (ANYSTAT(MyPrefStruct.stat_errorsHistory))
				SUMMARY_LINK( ERRORSHIST_PAGE );
		}
		if (VDptr->Done.ErrorURL && ANYSTAT(MyPrefStruct.stat_errorurl) ){
			SUMMARY_LINK( ERRORURL_PAGE );
			if (ANYSTAT(MyPrefStruct.stat_errorurlHistory))
				SUMMARY_LINK( ERRORURLHIST_PAGE );
		}
		if( ANYSTAT(MyPrefStruct.stat_brokenLinks) )
		{
			if( VDptr->byBrokenLinkReferal->GetStatListNum() )
			{
				SUMMARY_LINK( VDptr->GetSiteURL() ? EXTBROKENLINKS_PAGE : BROKENLINKS_PAGE );
			}
			if( VDptr->byIntBrokenLinkReferal && VDptr->byIntBrokenLinkReferal->GetStatListNum() )
			{
				SUMMARY_LINK( INTBROKENLINKS_PAGE );
			}
		}
		EndSummaryTable( fp );
	}
}


// ******************** TO DO IDEA *****************
// Raul: add byStats into the report structures to reduce all this stuff
// below into one small loop that does a Check() and MakeLink() call.


/* WriteReportClientSummary - print summary of total day statistics */
void htmlFile::WriteReportClientSummary( VDinfoP VDptr, FILE *fp, long num, long tw, long c )
{
	long		line=0, rgbcolor[2]={ 0xf0f0f0, 0xe0e0e0 };

	//client statistics
	line = 0;

	if ( AnyServerStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, /*"Server"*/ TranslateID(RCAT_SERVER), c );
		if ( ANYSTAT(MyPrefStruct.stat_pages) )
			SUMMARY_LINK( PAGES_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_pagesHistory) )
			SUMMARY_LINK( PAGEHIST_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_pagesLeastVisited) )
			SUMMARY_LINK( PAGESLEAST_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_pagesfirst) )
			SUMMARY_LINK( PAGESFIRST_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_pagesfirstHistory) )
			SUMMARY_LINK( PAGESFIRSTHIST_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_pageslast) )
			SUMMARY_LINK( PAGESLAST_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_pageslastHistory) )
			SUMMARY_LINK( PAGESLASTHIST_PAGE );
		if (VDptr->Done.Dir && ANYSTAT( MyPrefStruct.stat_dir) )	
			SUMMARY_LINK( DIRS_PAGE );

		if (VDptr->Done.TopDir && ANYSTAT(MyPrefStruct.stat_topdir) ){
			SUMMARY_LINK( TOPDIRS_PAGE );
			if (ANYSTAT(MyPrefStruct.stat_topdirHistory))		
				SUMMARY_LINK( TOPDIRSHIST_PAGE );
		}
		if (VDptr->Done.File && ANYSTAT(MyPrefStruct.stat_files) )
			SUMMARY_LINK( FILE_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_meanpath) && VDptr->Done.MeanPath )
			SUMMARY_LINK( MEANPATH_PAGE );
		if (VDptr->Done.Groups && ANYSTAT(MyPrefStruct.stat_groups) ){
			SUMMARY_LINK( GROUPS_PAGE );
			if (ANYSTAT(MyPrefStruct.stat_groupsHistory) )
				SUMMARY_LINK( GROUPSHIST_PAGE );
		}
		if (VDptr->Done.Download && ANYSTAT(MyPrefStruct.stat_download) ){
			SUMMARY_LINK( DOWNLOAD_PAGE );
			if ( ANYSTAT(MyPrefStruct.stat_downloadHistory))
				SUMMARY_LINK( DOWNLOADHIST_PAGE );
		}
		if (VDptr->Done.Type && ANYSTAT(MyPrefStruct.stat_type) )
			SUMMARY_LINK( FILETYPE_PAGE );

		if (VDptr->Done.Pages 
			&& ANYSTAT(MyPrefStruct.stat_keypageroute)		
			&& VDptr->byPages->TestAnyMatches(FilterNonKeyPagesTo)
			)
		{
			SUMMARY_LINK( KEYPAGEROUTE_PAGE );
		}

		if (VDptr->Done.Pages
			&& ANYSTAT(MyPrefStruct.stat_keypageroutefrom)
			&& VDptr->byPages->TestAnyMatches(FilterNonKeyPagesFrom)
			)
		{
			SUMMARY_LINK( KEYPAGEROUTEFROM_PAGE );
		}
		
		EndSummaryTable( fp );
	}

	if ( AnyDemographicStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, /*"Demographics"*/ TranslateID(RCAT_DEMOGRAPH), c );
		if ( ANYSTAT(MyPrefStruct.stat_client) )
			SUMMARY_LINK( CLIENT_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_clientStream) )
			SUMMARY_LINK( CLIENTSTREAM_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_clientHistory) )
			SUMMARY_LINK( CLIENTHIST_PAGE );
		if ( VDptr->Done.User && ANYSTAT(MyPrefStruct.stat_user))
			SUMMARY_LINK( USER_PAGE );
		if ( VDptr->Done.User && ANYSTAT(MyPrefStruct.stat_userStream))
			SUMMARY_LINK( USERSTREAM_PAGE );
		if ( VDptr->Done.User && ANYSTAT(MyPrefStruct.stat_userHistory) )
			SUMMARY_LINK( USERHIST_PAGE );
		if ( VDptr->Done.SecondDomain && ANYSTAT(MyPrefStruct.stat_seconddomain) )
			SUMMARY_LINK( SECONDDOMAIN_PAGE );
		if (VDptr->Done.Domain && ANYSTAT(MyPrefStruct.stat_country) )
			SUMMARY_LINK( DOMAIN_PAGE );
		if (VDptr->Done.Regions && ANYSTAT(MyPrefStruct.stat_regions) )
			SUMMARY_LINK( REGION_PAGE );
		if ( MyPrefStruct.filterdata.orgTot>0 && VDptr->Done.Orgs && ANYSTAT(MyPrefStruct.stat_orgs ) )
			SUMMARY_LINK( ORGNAMES_PAGE );
		if ( ANYSTAT(MyPrefStruct.stat_sessionScatter) )
			SUMMARY_LINK( SESSIONS_PAGE );
		if (VDptr->Done.Client
			&& ANYSTAT(MyPrefStruct.stat_keyvisitor)
			&& VDptr->byClient->TestAnyMatches(LookupKeyVisitor)
			)
		{
			SUMMARY_LINK( KEYVISITORS_PAGE );
		}

		EndSummaryTable( fp );
	}

	if ( AnyReferralStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, TranslateID(RCAT_REFERRALS), c );
		if (VDptr->Done.Refer && ANYSTAT(MyPrefStruct.stat_refer) )
			SUMMARY_LINK( REFERURL_PAGE );
		if (VDptr->Done.ReferSite && ANYSTAT(MyPrefStruct.stat_refersite) ) {
			SUMMARY_LINK( REFERSITE_PAGE );
		}
		if (VDptr->Done.ReferSite && ANYSTAT(MyPrefStruct.stat_refersiteHistory) ) {
			if (logType<30){
				SUMMARY_LINK( REFERSITEHIST_PAGE );
			} else {
				SUMMARY_LINK( REFERSITEHIST_PAGE );
			}
		}
		if (VDptr->Done.SearchSite && ANYSTAT(MyPrefStruct.stat_searchsite) )
			SUMMARY_LINK( SEARCHSITE_PAGE );
		if (VDptr->Done.SearchSite && ANYSTAT(MyPrefStruct.stat_searchsiteHistory) )
			SUMMARY_LINK( SEARCHSITEHIST_PAGE );

		if (VDptr->Done.SearchStr && ANYSTAT(MyPrefStruct.stat_searchstr) )
			SUMMARY_LINK( SEARCHSTR_PAGE );
		if (VDptr->Done.SearchStr && ANYSTAT(MyPrefStruct.stat_searchstrHistory) )
			SUMMARY_LINK( SEARCHSTRHIST_PAGE );
		EndSummaryTable( fp );
	}

	if ( AnyStreamingMediaStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, TranslateID(RCAT_STREAMING), c );
		if (ANYSTAT(MyPrefStruct.stat_mplayers) && VDptr->byMediaPlayers->GetStatListNum() )
			SUMMARY_LINK( MPLAYERS_PAGE );
		if (ANYSTAT(MyPrefStruct.stat_mplayersHistory) && VDptr->byMediaPlayers->GetStatListNum() )
			SUMMARY_LINK( MPLAYERSHIST_PAGE );

		if (ANYSTAT(MyPrefStruct.stat_audio) && VDptr->byAudio->GetStatListNum())
			SUMMARY_LINK( AUDIO_PAGE );
		if (ANYSTAT(MyPrefStruct.stat_audioHistory) && VDptr->byAudio->GetStatListNum())
			SUMMARY_LINK( AUDIOHIST_PAGE );
		if (ANYSTAT(MyPrefStruct.stat_video) && VDptr->byVideo->GetStatListNum())
			SUMMARY_LINK( VIDEO_PAGE );
		if (ANYSTAT(MyPrefStruct.stat_videoHistory) && VDptr->byVideo->GetStatListNum())
			SUMMARY_LINK( VIDEOHIST_PAGE );
		if (ANYSTAT(MyPrefStruct.stat_mediatypes) && VDptr->byMediaTypes->GetStatListNum())
			SUMMARY_LINK( MEDIATYPES_PAGE );
		EndSummaryTable( fp );
	}

	if ( AnySystemsStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, TranslateID(RCAT_SYSTEMS), c );
		if (VDptr->Done.Robot && ANYSTAT(MyPrefStruct.stat_robot) )
			SUMMARY_LINK( ROBOT_PAGE );
		if (VDptr->Done.Robot && ANYSTAT(MyPrefStruct.stat_robotHistory) )
			SUMMARY_LINK( ROBOTHIST_PAGE );
		if ( VDptr->Done.Browser && ANYSTAT(MyPrefStruct.stat_browsers) )
			SUMMARY_LINK( BROWSER_PAGE );
		if ( VDptr->Done.Browser && ANYSTAT(MyPrefStruct.stat_browsersHistory) )
			SUMMARY_LINK( BROWSERHIST_PAGE );

		if ( VDptr->byUnrecognizedAgents->GetStatListNum() && ANYSTAT(MyPrefStruct.stat_unrecognizedagents) )
			SUMMARY_LINK( UNRECOGNIZEDAGENTS_PAGE );

		if ( VDptr->Done.Browser && ANYSTAT(MyPrefStruct.stat_browserVSos) )
			SUMMARY_LINK( BROWSEROS_PAGE );

		if ( VDptr->Done.OperSys && ANYSTAT(MyPrefStruct.stat_opersys) )
			SUMMARY_LINK( OPERSYS_PAGE );
		if ( VDptr->Done.OperSys && ANYSTAT(MyPrefStruct.stat_opersysHistory) )
			SUMMARY_LINK( OPERSYSHIST_PAGE );
		EndSummaryTable( fp );
	}
		
	if ( AnyAdvertisingStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, TranslateID(RCAT_ADVERT), c );
		if (ANYSTAT(MyPrefStruct.stat_advert) && VDptr->byAdvert->GetStatListNum() )
			SUMMARY_LINK( ADVERT_PAGE );
		if (ANYSTAT(MyPrefStruct.stat_advertHistory) && VDptr->byAdvert->GetStatListNum() )
			SUMMARY_LINK( ADVERTHIST_PAGE );
		if (ANYSTAT(MyPrefStruct.stat_advertcamp) && VDptr->byAdCamp->GetStatListNum() )
			SUMMARY_LINK( ADVERTCAMP_PAGE );
		if (ANYSTAT(MyPrefStruct.stat_advertcampHistory) && VDptr->byAdCamp->GetStatListNum() )
			SUMMARY_LINK( ADVERTCAMPHIST_PAGE );
		EndSummaryTable( fp );
	}

	if ( AnyMarketingStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, TranslateID(RCAT_MARKETING), c );
		if ( VDptr->Done.Circulation  && ANYSTAT(MyPrefStruct.stat_circulation) )
			SUMMARY_LINK( CIRC_PAGE );
		if ( VDptr->Done.Loyalty  && ANYSTAT(MyPrefStruct.stat_loyalty) )
			SUMMARY_LINK( LOYALTY_PAGE );
		if ( VDptr->Done.Timeon  && ANYSTAT(MyPrefStruct.stat_timeon) )
			SUMMARY_LINK( TIMEON_PAGE );
		EndSummaryTable( fp );
	}
		
	if ( AnyClustersStatsTurnedOn( VDptr, &MyPrefStruct ) ) {
		BeginSummaryTable( fp, tw, TranslateID(RCAT_CLUSTERS), c );
		if( ANYSTAT(MyPrefStruct.stat_clusterServers) )
		{
			SUMMARY_LINK( CLUSTER_PAGE );
		}
		if( ANYSTAT(MyPrefStruct.stat_clusterServersHistory) )
		{
			SUMMARY_LINK( CLUSTERHIST_PAGE );
		}
		EndSummaryTable( fp );
	}



#ifdef DEF_APP_FIREWALL
	if ( AnyFirewallStatsTurnedOn( VDptr, &MyPrefStruct ) ){
		BeginSummaryTable( fp, tw, TranslateID(RCAT_FIREWALL), c );
		if (VDptr->Done.SourceAddr && ANYSTAT(MyPrefStruct.stat_sourceaddress) )
			SUMMARY_LINK( SRCADDR_PAGE );
		if (VDptr->Done.Protocol && ANYSTAT(MyPrefStruct.stat_protSummary) )
			SUMMARY_LINK( PROTSUMMARY_PAGE );
		if (VDptr->Done.HTTP && ANYSTAT(MyPrefStruct.stat_protHTTP) )
			SUMMARY_LINK( PROTHTTP_PAGE );
		if (VDptr->Done.HTTPS && ANYSTAT(MyPrefStruct.stat_protHTTPS) )
			SUMMARY_LINK( PROTHTTPS_PAGE );
		if (VDptr->Done.Mail && ANYSTAT(MyPrefStruct.stat_protMail) )
			SUMMARY_LINK( PROTMAIL_PAGE );
		if (VDptr->Done.FTP && ANYSTAT(MyPrefStruct.stat_protFTP) )
			SUMMARY_LINK( PROTFTP_PAGE );
		if (VDptr->Done.Telnet && ANYSTAT(MyPrefStruct.stat_protTelnet) )
			SUMMARY_LINK( PROTTELNET_PAGE );
		if (VDptr->Done.DNS && ANYSTAT(MyPrefStruct.stat_protDNS) )
			SUMMARY_LINK( PROTDNS_PAGE );
		if (VDptr->Done.POP3 && ANYSTAT(MyPrefStruct.stat_protPOP3) )
			SUMMARY_LINK( PROTPOP3_PAGE );
		if (VDptr->Done.RealAudio && ANYSTAT(MyPrefStruct.stat_protReal) )
			SUMMARY_LINK( PROTREAL_PAGE );
		if (VDptr->Done.Others && ANYSTAT(MyPrefStruct.stat_protOthers) )
			SUMMARY_LINK( PROTOTHERS_PAGE );
		EndSummaryTable( fp );
	}
#endif // DEF_APP_FIREWALL
}

void htmlFile::WriteSummaryHTMLLinks( VDinfoP VDptr, FILE *fp, long num, long tw, long c )
{
	WriteReportServerSummary( VDptr, fp, num, tw, c );
	WriteReportClientSummary( VDptr, fp, num, tw, c );
}





void htmlFile::WriteReportSummary_Framed( VDinfoP VDptr, FILE *fout, char *filename, long logNum )
{
	long	tw=TABLE_WIDTH, th=TABLE_HEIGHT;
	char	newname[512];

	if( fout )
	{
		Write_FramedMain( fout, "findex", "fstart" );
		fflush( fout );
		if( !SingleFileOutput() )
			Fclose( fout );
	}

	PathFromFullPath( filename, newname );
	strcat( newname, "findex" );
	strcat( newname, fileExtension );

	FILE	*fp;
	fp = Fopen( newname, "w" );
	if ( fp )
	{
		char c = 0;
#define	FRAMED_WIDTH		(220)
		WritePageTitle( VDptr, fp, 0 );			//write custom HTML header

		// Insert SUMMARY link into the LEFT SIDE LIST....
		if ( STAT(MyPrefStruct.stat_summary) )
		{
			BeginSummaryTable( fp, FRAMED_WIDTH, TranslateID(RCAT_STATISTICS), 0 );
			SummaryTableStatEntry( fp, TranslateID(REPORT_SUMMARY), "fstart", 0, SUMMARY_FRAMED );
			EndSummaryTable( fp );
		}

		WriteSummaryHTMLLinks( VDptr, fp, VDptr->totalDays, FRAMED_WIDTH, SUMMARY_FRAMED );
		WritePageFooter( VDptr, fp );
		Stat_WriteSpace( fp, 2 );
		Fclose( fp );
	}

	PathFromFullPath( filename, newname );
	strcat( newname, "fstart" );
	strcat( newname, fileExtension );

	fp = Fopen( newname, "w" );
	if ( fp )
	{
		if ( CreateSummaryPage( VDptr, fp, filename, logNum ) == FALSE )
		{
			WriteSummaryDetails( VDptr, fp, filename, logNum );
			WritePageFooter( VDptr, fp );
		}
		Fclose( fp );
	}
}


void htmlFile::WriteReportSummary( VDinfoP VDptr, FILE *fp, char *filename, long logNum )
{
	if ( MyPrefStruct.html_frames )
	{
		WriteReportSummary_Framed( VDptr, fp, filename, logNum );
	}
	else
	{
		WriteReportBanner( VDptr );

		if ( CreateSummaryPage( VDptr, fp, filename, logNum ) == FALSE )
		{
			WriteSummaryDetails( VDptr, fp, filename, logNum );
			WriteSummaryHTMLLinks( VDptr, fp, VDptr->totalDays, TABLE_WIDTH, 1 );
			WritePageFooter( VDptr, fp );
		}
	}
}

void htmlFile::StartReport( VDinfoP VDptr, FILE *fp, const char *filename, long logNum )
{
	if ( MyPrefStruct.corporate_look == FALSE )
	{
		StandardPlot aPlot;
		aPlot.WriteShadowImages( VDptr );
	}
	// Write a dummy summary which will say, PLEASE WAIT...
	Fprintf( fp, "<html><head><title>Summary Report</title>" );
	Fprintf( fp, "<head><meta http-equiv=\"refresh\" content=\"%ld\"></head>", 2 );
	Fprintf( fp, "<br><br><br><br><center><i><h2>Generating reports<br><br>Please wait....</h2></i></center></html>" );
	fflush( fp );
	fseek( fp, 0, SEEK_SET );

	SwitchonAllThumbnails( VDptr );
	return;
}

void htmlFile::FinishReport( VDinfoP VDptr, FILE *fp, const char *filename, long numberOfLogs )
{
	WriteReportSummary( VDptr, fp, (char*)filename, numberOfLogs );
	fflush( fp );
	Fclose( fp );		// close the first output file.
}





