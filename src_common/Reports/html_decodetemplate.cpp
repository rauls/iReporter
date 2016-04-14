/*=================================================================================================
 Copyright © Software 2001, 2002

 File:            html_decodetemplate.cpp
 Subsystem:       Report
 Created By:      RSOBON, 12/Nov/2002
 Description:
 This segment loads and decodes a summary page template file for html only, it will
 fill in all variables in [$VAR] format, and also decode which segments to keep or ignore.

 $Archive: /iReporter/Dev/src_common/Reports/html_decodetemplate.cpp $
 $Revision: 59 $
 $Author: Raul.sobon $
 $Date: 30/07/02 3:51p $



 5.0 Commands/Variables
 multilinks_		-	Show the rest of this reports links from starting report/stat name.
 multithumblink_	-	Show the rest of this reports thumbnails/links from starting report/stat name.
 addhostheader		-	#include a html file of the [virtual host name].html
 htmlheader			-	add the settings file html header at this position
 htmlfooter			-	add the settings file html footer at this position

*/ 



//================================================================================================= 
#include "FWA.h"

#include "stdio.h"
#include "stdlib.h"

#ifdef DEF_MAC
#include <alloca.h>
#else
#include <malloc.h>				// for alloca
#endif
	
#include "myansi.h"
#include "config.h"
#include "VirtDomInfo.h"		// for VDinfoP
#include "Stats.h"				// for StatList
#include "ReportClass.h"		// for baseFile.
#include "Report.h"				// for RGBtableTitle and the like.
#include "Translate.h"			// for TranslateID()

#include "engine_proc.h"
#include "log_io.h"

#include "version.h"
#include "Utilities.h"
#include "serialReg.h"
#include "WebURLS.h"

#include "GlobalPaths.h"
#include <string>

// ---------------------- get variable funcs
#define	ONEMEGFLOAT	1048576.0




char SummaryTemplatename[256] = "\0";

#define	DEFAULT_SUMMARYTEMPLATE			"default_summary.html"
#define	DEFAULT_OLDSUMMARYTEMPLATE		"summary_template.html"





StatList *GetStatListFromName( VDinfoP VDptr, const char *statName )
{
	// -------------------------- WEB STUFF ---------------------------------
	// Traffic
	if ( !strcmpd( "hourly",statName ) )			return VDptr->byHour;
	if ( !strcmpd( "daily",statName ) )				return VDptr->byDate;
	if ( !strcmpd( "recentdaily",statName ) )		return VDptr->byDate;
	if ( !strcmpd( "weekly",statName ) )			return VDptr->byWeekday;
	if ( !strcmpd( "monthly",statName ) )			return VDptr->byMonth;

	// Diag
	if ( !strcmpd( "errors",statName ) )			return VDptr->byErrors;
	if ( !strcmpd( "errorurl",statName ) )			return VDptr->byErrorURL;
	if ( !strcmpd( "brokenlinks",statName ) )		return VDptr->byBrokenLinkReferal;
	if ( !strcmpd( "intbrokenlinks",statName ) )	return VDptr->byIntBrokenLinkReferal;
	if ( !strcmpd( "extbrokenlinks",statName ) )	return VDptr->byBrokenLinkReferal;

	//Server
	if ( !strcmpd( "pages",statName ) )				return VDptr->byPages;
	if ( !strcmpd( "dir",statName ) )				return VDptr->byDir;
	if ( !strcmpd( "topdir",statName ) )			return VDptr->byTopDir;
	if ( !strcmpd( "file",statName ) )				return VDptr->byFile;
	if ( !strcmpd( "groups",statName ) )			return VDptr->byGroups;
	if ( !strcmpd( "download",statName ) )			return VDptr->byDownload;
	if ( !strcmpd( "upload",statName ) )			return VDptr->byUpload;
	if ( !strcmpd( "redirect",statName ) )			return VDptr->byPages;
	if ( !strcmpd( "filetype",statName ) )			return VDptr->byType;

	// Demog
	if ( !strcmpd( "client",statName ) )			return VDptr->byClient;
	if ( !strcmpd( "users",statName ) )				return VDptr->byUser;
	if ( !strcmpd( "seconddomain",statName ) )		return VDptr->bySecondDomain;
	if ( !strcmpd( "country",statName ) )			return VDptr->byDomain;
	if ( !strcmpd( "region",statName ) )			return VDptr->byRegions;
	if ( !strcmpd( "orgs",statName ) )				return VDptr->byOrgs;

	// Referral
	if ( !strcmpd( "refersite",statName ) )			return VDptr->byReferSite;
	if ( !strcmpd( "refer",statName ) )				return VDptr->byRefer;
	if ( !strcmpd( "searchengine",statName ) )		return VDptr->bySearchSite;
	if ( !strcmpd( "searchterms",statName ) )		return VDptr->bySearchStr;
	if ( !strcmpd( "search",statName ) )			return VDptr->bySearchStr;

	// Media
	if ( !strcmpd( "mplayers",statName ) )			return VDptr->byMediaPlayers;
	if ( !strcmpd( "players",statName ) )			return VDptr->byMediaPlayers;
	if ( !strcmpd( "mediatypes",statName ) )		return VDptr->byMediaTypes;
	if ( !strcmpd( "audiostreams",statName ) )		return VDptr->byAudio;
	if ( !strcmpd( "videostreams",statName ) )		return VDptr->byVideo;

	//  systems
	if ( !strcmpd( "robots",statName ) )			return VDptr->byRobot;
	if ( !strcmpd( "browser",statName ) )			return VDptr->byBrowser;
	if ( !strcmpd( "unrecognizedagents",statName ))	return VDptr->byUnrecognizedAgents;
	if ( !strcmpd( "opersys",statName ) )			return VDptr->byOperSys;

	// adds
	if ( !strcmpd( "advert",statName ) )			return VDptr->byAdvert;
	if ( !strcmpd( "campaigns",statName ) )			return VDptr->byAdCamp;

	// circ
	if ( !strcmpd( "circulation",statName ) )		return VDptr->byCirculation;
	if ( !strcmpd( "loyalty",statName ) )			return VDptr->byLoyalty;
	if ( !strcmpd( "timeonline",statName ) )		return VDptr->byTimeon;

	if ( !strcmpd( "clusters",statName ) )			return VDptr->byClusters;

	
	// ---------------------- STREAMING STUFF ----------------------------	
	if ( !strcmpd( "clipsfailed",statName ) )		return VDptr->byErrorURL;
	if ( !strcmpd( "brokenclips",statName ) )		return VDptr->byBrokenLinkReferal;

	//Server
	if ( !strcmpd( "debugclips",statName ) )		return VDptr->byClips;
	if ( !strcmpd( "clips",statName ) )				return VDptr->byClips;
	if ( !strcmpd( "live",statName ) )				return VDptr->byClips;
	if ( !strcmpd( "videocodecs",statName ) )		return VDptr->byVideoCodecs;
	if ( !strcmpd( "audiocodecs",statName ) )		return VDptr->byAudioCodecs;
	if ( !strcmpd( "billing",statName ) )			return VDptr->byBilling;
	if ( !strcmpd( "cpus",statName ) )				return VDptr->byCPU;
	if ( !strcmpd( "langs",statName ) )				return VDptr->byLang;

	if ( !strcmpd( "servers",statName ) )			return VDptr->byServers;

	if ( !strcmpd( "protocols",statName ) )			return VDptr->byProtocol;
	
	return NULL;
}





// load a file from HD into ram and return pointer
static char *LoadFile( char *filename, long *length )
{
	FILE *fp;
	char *ram=NULL;
	long dataread, fileSize;
	std::string	szFile;

	if ( filename[0] != '/' )
	{
		szFile	+= gPath;
#ifndef DEF_MAC
		szFile	+= PATHSEPSTR;					// QCM #50036 - gPath is a folder, so has a terminal ':' already!
#endif
	}
	szFile	+= filename;

	if ( fp = fopen( szFile.c_str(), "rb" ) )
	{
		fileSize = GetFPLength(fp);
		ram = (char *)malloc( fileSize+1 );
		dataread = fread( ram, 1, fileSize, fp );
		ram[dataread] = 0;
		fclose(fp);
		*length = fileSize;
	}
	return ram;
}

// load a file from HD into ram and return pointer
char *LoadSummaryFile( char *filename, long *length )
{
	FILE *fp;
	char *ram=NULL;
	long dataread, fileSize;
	std::string	szFile;

	if ( filename[0] != '/' )
	{
		szFile	+= gPath;
#ifndef DEF_MAC
		szFile	+= PATHSEPSTR;					// QCM #50036 - gPath is a folder, so has a terminal ':' already!
#endif
		szFile	+= "Templates";
		szFile	+= PATHSEPSTR;

		switch( MyPrefStruct.stat_style )
		{
			case STREAM:	szFile	+= "streaming"; break;
			default:
			case WEB:		szFile	+= "web"; break;
		}

		szFile	+= PATHSEPSTR;
	}
	szFile	+= filename;

	if ( fp = fopen( szFile.c_str(), "rb" ) )
	{
		fileSize = GetFPLength(fp);
		ram = (char *)malloc( fileSize+1 );
		dataread = fread( ram, 1, fileSize, fp );
		ram[dataread] = 0;
		fclose(fp);
		*length = fileSize;
	} else {
	// Try old path just in case...
		ram = LoadFile( filename, length );
	}
	return ram;
}


//VDptr->byClusters->GetStatListNum()




double GetBandwidthName( double bps, char *type )
{
	int kilo = 1000;
	double perc;

	if ( bps < 64*kilo ){
		sprintf (type, "64 %s", TranslateID(LABEL_KBPS)); perc = bps/(64*kilo);
	} else
	if ( bps < 2*64*kilo ){
		sprintf (type, "128 %s", TranslateID(LABEL_KBPS)); perc = bps/(2*64*kilo);
	} else
	if ( bps < 4*64*kilo ){
		sprintf (type, "256 %s", TranslateID(LABEL_KBPS)); perc = bps/(4*64*kilo);
	} else
	if ( bps < 6*64*kilo ){
		sprintf (type, "384 %s", TranslateID(LABEL_KBPS)); perc = bps/(6*64*kilo);
	} else
	if ( bps < 8*64*kilo ){
		sprintf (type, "512 %s", TranslateID(LABEL_KBPS)); perc = bps/(8*64*kilo);
	} else
	if ( bps < 12*64*kilo ){
		sprintf (type, "768 %s", TranslateID(LABEL_KBPS)); perc = bps/(12*64*kilo);
	} else
	if ( bps < 24*64*kilo ){
		mystrcpy( type, "T1" ); perc = bps/(24*64*kilo);
	} else
	if ( bps < 24*24*64*kilo ){
		mystrcpy( type, "T3" ); perc = bps/(24*24*64*kilo);
	} else
	if ( bps < 155 * 1000000 ){
		mystrcpy( type, "OC3" ); perc = bps/(155 * 1000000);
	} else
	if ( bps < 655 * 1000000 ){
		mystrcpy( type, "OC12" ); perc = bps/(655 * 1000000);
	} //else you are AT&T/WorldCom with 8GIG/sec fibre.
	return perc;
}


int ActivateThumbImage( char *statName )
{
	long data, i;
	i = ConfigNameFindSettings( &MyPrefStruct, statName, &data );
	if ( i >0 )
	{
		if ( GRAPH(data) )
		{
			STATSETTHUMB(data,1);
			ConfigSetSettings( &MyPrefStruct, i, data );
			return i;
		}
	}
	return 0;
}

int IsThumbImageOn( char *statName )
{
	long data, i;
	i = ConfigNameFindSettings( &MyPrefStruct, statName, &data );
	if ( i >0 )
	{
		if ( THUMB_ON(data) && GRAPH(data) )
			return i;
	}
	return 0;
}

#include "engineparse.h"

char *GetClientName( Statistic	*p )
{
static char szName[1024];
	if ( p->length < 0 )
		return p->GetName();
	else {
		ReverseAddress( p->GetName(), szName );
		return szName;
	}
}


const char *string_na		= "N/A";
const char *string_noentry	= "Unknown Entry";


#define	FIND_HIGHEST		1
#define	FIND_LOWEST			0

char *FindTopStat_byValue( VDinfo *VDptr, const char *statName, char *pattern, long type, long highest, __int64 *result )
// type = use the same value as the SORT_ defines
{
	if ( !VDptr ) return (char*)string_noentry;

	StatList *byStat = GetStatListFromName( VDptr,statName );
	if ( !byStat ) return (char*)string_noentry;

	long		lp;
	__int64		current_value,key_value=-1;
	Statistic	*p, 
				*foundItem=NULL;
	char		*name = (char*)string_na;

	if ( highest )
		key_value = 0;

	for ( lp=0; lp<byStat->GetStatListNum(); lp++ )
	{
		p = byStat->GetStat(lp);
		if ( pattern )		// use this as a filter..., if valid, only get TOP value for the MATCH of the NAME
		{
			if ( !strcmpd( "client", statName ) || !strcmpd( "seconddomain", statName ) )
				name = GetClientName(p);
			else
				name = p->GetName();

			if ( match( name, pattern, 1 ) == FALSE )
				continue;
		}

		switch( type )
		{
			case TYPE_REQUESTS:		current_value = p->files; break;
			case TYPE_BYTES:		current_value = p->bytes; break;
			case TYPE_COUNTER:		current_value = p->counter; break;		//visitors
			case TYPE_PAGES:		current_value = p->counter4; break;
			case TYPE_SESSIONS:		current_value = p->visits; break;
			case TYPE_ERRORS:		current_value = p->errors; break;
		}

		if ( key_value == -1 )
			key_value = current_value;

		if ( (highest && current_value > key_value) || (highest==0 && current_value < key_value) )
		{
			key_value = current_value;
			foundItem = p;
		}
	}

	*result = key_value;

	if ( foundItem )
	{
		if ( !strcmpd( "client", statName ) || !strcmpd( "seconddomain", statName ) )
			name = GetClientName(foundItem);
		else
			name = foundItem->GetName();
	}

	return name;
}


long FindTotalStat_WithErrors( VDinfo *VDptr, StatList *byStat )
// type = use the same value as the SORT_ defines
{
	long lp, count=0;
	Statistic *p;

	if ( !VDptr || !byStat ) return 0;

	for ( lp=0; lp<byStat->GetStatListNum(); lp++ )
	{
		p = byStat->GetStat(lp);
		if ( p->errors )
			count++;
	}
	return count;
}

#ifdef DEF_FULLVERSION
#define	RGB_Common	0x0099CC
#else
#define	RGB_Common	0xEC3A3E
#endif

#define	MAX_STYLES		10
#define	STYLE_BUF		1024

static char *link_default = "<li><a href=\"%s.html\"> %s </a></li>";
static char link_templates[MAX_STYLES][STYLE_BUF];
#define	STORE_LINKTEMPLATE( style, string )	strcpy( link_templates[ style ], string )

static char *thumblink_default = "<img src=\"%s_t.jpg\"><br><center><font size=\"1\" face=\"Arial, Helvetica, sans-serif\">%s Report</font></center><br>";
static char thumblink_templates[MAX_STYLES][STYLE_BUF];
#define	STORE_THUMBLINKTEMPLATE( style, string )	strcpy( thumblink_templates[ style ], string )


long Decode_Variables( VDinfoP VDptr, const char *variable, char *output, long logNum )
{
	char		param[256],
				*topStr;
	const char	*p,
				*p2,
				*var;
	__int64		result;

	if ( !strcmpd( "[$", variable ) )
		var = variable+2;
	else
		return 0;

	if ( p=strchr( var, '_' ) )
	{
		strcpyuntil( param, (char*)p+1, ']' );
	}


	// ------------------- Decode Dynamic vars ----------------
	if ( !strcmpd( "reportfile_", var ) ){								// name of report
		p = ConfigNameFindFilename( &MyPrefStruct, param );
		if ( p )
		{
			return sprintf( output, "%s", p );
		} else
			return mystrcpy( output, " " );
	} else
	if ( !strcmpd( "reportname_", var ) ){								// name of report
		long i;
		if ( i = IsReportNameCompleted( &MyPrefStruct, param ) )	// returns index loc of the settings...
		{
			p2 = ConfigLookupTitle( i );
			return mystrcpy( output, p2 );
		} else
			return mystrcpy( output, " " );
	} else
	if ( !strcmpd( "report_", var ) ){								// name of report
		p = ConfigNameFindFilename( &MyPrefStruct, param );
		if ( p )
		{
			if( IsReportNameCompleted( &MyPrefStruct, param ) )
				return sprintf( output, "%s.html", p );
			else
				return mystrcpy( output, "#" );
		}
	} else
	// perform a complete LINK to a REPORT
	if ( !strcmpd( "link", var ) )
	{							// link to report AND test for reportdone logic
		char *format;

		long style = atoi( var+4 );		// 4th char is before the _
		if ( style >= MAX_STYLES || style == 0 )
			format = link_default;
		else
			format = link_templates[ style ];

		if ( *format == 0 )	format = link_default;


		// Special CASE for any broken links reports....
		if ( strstri(param, "BrokenLinks") )
		{
			// We are expecting internal & external reports.
			if(	(VDptr->GetSiteURL()) )
			{
				if( !strcmpd( "IntBrokenLinks",param ) && VDptr->byIntBrokenLinkReferal && VDptr->byIntBrokenLinkReferal->GetStatListNum() )
					return sprintf( output, format, FindReportFilenameStr(INTBROKENLINKS_PAGE), TranslateID(REPORT_INTBROKENLINKS) );
				else
				if( !strcmpd( "ExtBrokenLinks",param ) && VDptr->byBrokenLinkReferal && VDptr->byBrokenLinkReferal->GetStatListNum() )
					return sprintf( output, format, FindReportFilenameStr(EXTBROKENLINKS_PAGE), TranslateID(REPORT_EXTBROKENLINKS) );
				else
				if( !strcmpd( "BrokenLinks",param ) )
					return -1;
			}
		}

		// Normal LINKS
		if ( long i = IsReportNameCompleted( &MyPrefStruct, param ) )	// returns index loc of the settings...
		{

			p = ConfigLookupFilename( i );
			p2 = ConfigLookupTitle( i );
			return sprintf( output, format, p, p2 );
		} else
			return -1;
			//return sprintf( output, "</IGNOREBR " );		// This is a massive cute hack, it basicly will KILL/IGNORE the next HTMLTAG which will be a <BR> hopefully.
	} else
	// Show the rest of this groups links from starting report/stat name.
	if ( !strcmpd( "multilinks", var ) )					// link to report AND test for reportdone logic
	{	
		long totlen=0;

		long i = ConfigNameFindSettings( &MyPrefStruct, param, NULL );
		while( i>=0 )
		{
			p = ConfigLookupName( i );
			if ( p ){
				char tmp[100] = "[$";
				strcpyuntil( tmp, (char*)var+5, '_' );
				strcat( tmp, "_" );
				strcat( tmp, p );

				long len = Decode_Variables( VDptr, (const char *)tmp, output, logNum );
				output+=len;
				totlen+=len;
			} else
				break;
			i++;
		}
		return totlen;
	} else
	// perform a complete THUMB IMAGE and TEXT TITLE
	if ( !strcmpd( "thumblink", var ) )
	{								// name of thumb nail image, and actived thumbnail flag for report.
		long i, style;
		if ( i = IsThumbImageOn( param ) && IsReportNameCompleted( &MyPrefStruct, param ) )
		{
			char *format;
			style = atoi( var+9 );		// 4th char is before the _
			if ( style >= MAX_STYLES || style == 0 )
				format = thumblink_default;
			else
				format = thumblink_templates[ style ];
	
			if ( *format == 0 )
				format = thumblink_default;

			p = ConfigNameFindFilename( &MyPrefStruct, param );
			p2 = ConfigLookupTitle( i );
			if ( p )
				return sprintf( output, format, p, p2 );			// thumbs always are jpeg.
		} else
			return mystrcpy( output, " " );
	} else
	// Show the rest of this groups links from starting report/stat name.
	//              012345678901234
	if ( !strcmpd( "multithumblink", var ) )
	{							// link to report AND test for reportdone logic
		long totlen=0;

		long i = ConfigNameFindSettings( &MyPrefStruct, param, NULL );
		while( i )
		{
			p = ConfigLookupName( i );
			if ( p )
			{
				char tmp[100] = "[$";
				strcpyuntil( tmp+2, (char*)var+5, '_' );
				strcat( tmp, "_" );
				strcat( tmp, p );

				long len = Decode_Variables( VDptr, (const char *)tmp, output, logNum );
				output+=len;
				totlen+=len;
			} else
				break;
			i++;
		}
		return totlen;
	} else
	if ( !strcmpd( "thumb_", var ) )
	{								// name of thumb nail image, and actived thumbnail flag for report.
		if ( IsThumbImageOn( param ) )
		{
			p = ConfigNameFindFilename( &MyPrefStruct, param );
			if ( p ) return sprintf( output, "%s_t.jpg", p );			// thumbs always are jpeg.
		}
	} else
	if ( !strcmpd( "image_", var ) )
	{								// name of image file for <IMG> tags
		p = ConfigNameFindFilename( &MyPrefStruct, param );
		if ( p ) return sprintf( output, "%s.gif", p );
	} else
	if ( !strcmpd( "include_", var ) )
	{								// #include type of command
		return FileToMem( param, output, 64000 );
	} else
	if ( !strcmpd( "addhostheader", var ) )
	{								// #include type of command based on the virtual hosts name (V5)
		char filename[256];
		sprintf( filename, "%s.html", VDptr->domainName );
		return FileToMem( filename, output, 64000 );
	} else
	if ( !strcmpd( "htmlheader", var ) )
	{								// #include html header from settings (V5)
		return sprintf( output, MyPrefStruct.html_head );
	} else
	if ( !strcmpd( "htmlfooter", var ) )
	{								// #include html footer from settings (V5)
		return sprintf( output, MyPrefStruct.html_foot );
	}




	// ------------- STATUS VARIABLE RESULTS -----------------------------------------------------------
	if ( !strcmpd( "statusreport_", var ) ){								// name of report
		p = ConfigNameFindFilename( &MyPrefStruct, param );
		if ( p )
		{
			if( IsReportNameCompleted( &MyPrefStruct, param ) )
				return mystrcpy( output, "true" );
			else
				return mystrcpy( output, "false" );
		}
	} else
	if ( !strcmpd( "statusthumb_", var ) ){								// name of report
		if ( IsThumbImageOn( param ) )
			return mystrcpy( output, "true" );
		else
			return mystrcpy( output, "false" );
	}
	// ------------- STATUS VARIABLE RESULTS -----------------------------------------------------------




	// ------------------  DYNAMIC results from each stat : return ITEM with the highest X value -----------------
	if ( !strcmpd( "TOPHITS_", var ) )
	{
		if ( topStr = FindTopStat_byValue( VDptr, param, NULL, TYPE_REQUESTS, FIND_HIGHEST, &result ) )
			return mystrcpy( output, topStr );
	} else
	if ( !strcmpd( "TOPBYTES_", var ) )
	{
		if ( topStr = FindTopStat_byValue( VDptr, param, NULL, TYPE_BYTES, FIND_HIGHEST, &result ) )
			return mystrcpy( output, topStr );
	} else
	if ( !strcmpd( "TOPSESSIONS_", var ) )
	{
		if ( topStr = FindTopStat_byValue( VDptr, param, NULL, TYPE_SESSIONS, FIND_HIGHEST, &result ) )
			return mystrcpy( output, topStr );
	} else
	if ( !strcmpd( "TOPVISITORS_", var ) )
	{
		if ( topStr = FindTopStat_byValue( VDptr, param, NULL, TYPE_COUNTER, FIND_HIGHEST, &result ) )
			return mystrcpy( output, topStr );
	} else
	if ( !strcmpd( "TOPPAGES_", var ) || !strcmpd( "TOPCLIPS_", var ) )
	{
		if ( topStr = FindTopStat_byValue( VDptr, param, NULL, TYPE_PAGES, FIND_HIGHEST, &result ) )
			return mystrcpy( output, topStr );
	} else
	if ( !strcmpd( "TOPERRORS_", var ) )
	{
		if ( topStr = FindTopStat_byValue( VDptr, param, NULL, TYPE_ERRORS, FIND_HIGHEST, &result ) )
			return mystrcpy( output, topStr );
	} else

		
	if ( !strcmpd( "CLIENTPERCENT", var ) )
	{
		if ( topStr = FindTopStat_byValue( VDptr, "client", param, TYPE_REQUESTS, FIND_HIGHEST, &result ) )
			return sprintf( output, "%.2f%%", 100*result/(double)VDptr->byClient->GetStatListTotalRequests() );
	} else

		
	if ( !strcmpd( "LOWEST_", var ) )
	{
		if ( topStr = FindTopStat_byValue( VDptr, param, NULL, TYPE_REQUESTS, FIND_LOWEST, &result ) )
			return mystrcpy( output, topStr );
	} else

	// ------------------  DYNAMIC results from each stat : return the TOTAL values per STAT -----------------
	// All these return the TOTAL about of each VAr, for instance, the total bytes for all downloads for instance.
	if ( !strcmpd( "TOTALHITS_", var ) )
	{
		StatList *stat = GetStatListFromName( VDptr,param );
		if (!stat)
			return mystrcpy(output, string_na );
		return FormatLongNum( stat->GetStatListTotalRequests(), output );
	} else
	if ( !strcmpd( "TOTALBYTES_", var ) )
	{
		StatList *stat = GetStatListFromName( VDptr,param );
		if (!stat)
			return mystrcpy(output, string_na );
		return FormatLongNum( stat->GetStatListTotalBytes(), output );
	} else
	if ( !strcmpd( "TOTALSESSIONS_", var ) )
	{
		StatList *stat = GetStatListFromName( VDptr,param );
		if (!stat)
			return mystrcpy(output, string_na );
		return FormatLongNum( stat->GetStatListTotalVisits(), output );
	} else
	if ( !strcmpd( "TOTALVISITORS_", var ) )
	{
		StatList *stat = GetStatListFromName( VDptr,param );
		if (!stat)
			return mystrcpy(output, string_na );
		return FormatLongNum( stat->GetStatListTotalCounters(), output );
	} else
	if ( !strcmpd( "TOTALPAGES_", var ) )
	{
		StatList *stat = GetStatListFromName( VDptr,param );
		if (!stat)
			return mystrcpy(output, string_na );
		return FormatLongNum( stat->GetStatListTotalCounters4(), output );
	} else
	if ( !strcmpd( "TOTALERRORS_", var ) )
	{
		StatList *stat = GetStatListFromName( VDptr,param );
		if (!stat)
			return mystrcpy(output, string_na );
		return FormatLongNum( stat->GetStatListTotalErrors(), output );
	} else
	if ( !strcmpd( "TOTAL_", var ) )
	{
		StatList *stat = GetStatListFromName( VDptr,param );
		if (!stat)
			return mystrcpy(output, string_na );
		return FormatLongNum( stat->GetStatListNum(), output );
	} else


		// -------- This returns the about of items with errors, ie if its CLIENTS, its the amount of clients with errors.
	if ( !strcmpd( "ERRORITEMS_", var ) )
	{
		StatList *stat = GetStatListFromName( VDptr,param );
		if (!stat)
			return mystrcpy(output, string_na );
		long x = FindTotalStat_WithErrors( VDptr, stat );
		return FormatLongNum( x, output );
	} else


	// ------------- Various fixed non paramatized variables concerning Vd wide totals ----------------------
	if ( !mystrcmpi( "TITLE", var ) )
	{
		if ( MyPrefStruct.report_title[0] )
			return mystrcpy(output, MyPrefStruct.report_title);
		else
			return mystrcpy(output, TranslateID(SUMM_ACCESSSTATS));
	} else
	if ( !mystrcmpi( "CURRENTTIME", var ) )
	{
		time_t nowDays = GetCurrentCTime();
		DateTimeToStringTranslated( nowDays, param );
		return mystrcpy(output, param);
	} else
	if ( !mystrcmpi( "PROCESSTIME", var ) )
	{
		if ( VDptr->time2 > 0 ){
			return sprintf( output, "%s: %02d:%02d:%02d.%d, %.0f %s, %.2f %s/%s",
				TranslateID( SUMM_LOGPROCESSTIME ),
				(int)VDptr->time2/(60*60),(int)(VDptr->time2/60)%60,
				(int)(VDptr->time2)%60,(int)(VDptr->time2*10)%10,
				60*(VDptr->totalRequests/VDptr->time2),
				TranslateID( SUMM_LOGLINESPERMIN ),
				60*((VDptr->totalInDataSize/VDptr->time2)/1048576.0),
				TranslateID( LABEL_MB ),
				TranslateID( TIME_MIN ) );
		}
	} else
	if ( !mystrcmpi( "TOTALTIME", var ) )
	{
		time_t totaltime = (VDptr->lastTime-VDptr->firstTime);
		if ( !totaltime )	totaltime = 1;
		CTimetoString( totaltime, param );
		return mystrcpy(output, param);
	} else
	if ( !mystrcmpi( "TOTALDAYS", var ) )
	{
		if ( VDptr->totalDays < 2 ) 
			return sprintf( output, "(%ld %s)", VDptr->totalDays, TranslateID( TIME_DAY ) );
		else
			return sprintf( output, "(%ld %s)", VDptr->totalDays, TranslateID( TIME_DAYS ) );
	} else
	if ( !mystrcmpi( "STARTDATE", var ) )
	{
		DateTimeToStringTranslated( VDptr->firstTime, param );
		return mystrcpy(output, param);
	} else
	if ( !mystrcmpi( "ENDDATE", var ) )
	{
		DateTimeToStringTranslated( VDptr->lastTime, param );
		return mystrcpy(output, param);
	} else
	if ( !mystrcmpi( "VERSION", var ) )
		return sprintf( output, "%s %s", PRODUCT_TITLE, VERSION_STRING );
	else
	if ( !mystrcmpi( "BUILDDETAILS", var ) )
		return GetAppBuildDetails( output ) -1;

	// -----------------------------------
	// Basic top hits
	if ( !mystrcmpi( "DOMAINNAME", var ) )	
	{
		if (!MyPrefStruct.head_title)
			return mystrcpy( output, " ");	// Need a space to ensure that the token is removed.

		if ( VDptr->domainTotal==0 && logNum>1 && MyPrefStruct.multivhosts==0 && MyPrefStruct.siteurl[0]==0 )
			return sprintf( output, "(%d Log files)", logNum );
		else
		if ( VDptr->domainTotal==0 && logNum>1 && MyPrefStruct.multivhosts==0 )
			return sprintf( output, "%s (%d Log files)", VDptr->domainName, logNum );
		else
			return sprintf( output, "%s", VDptr->domainName );
	}
	if ( !mystrcmpi( "STATUS_NUMBEROFLOGS", var ) )		return sprintf( output, "%d", logNum );

	time_t totaltime = (VDptr->lastTime-VDptr->firstTime);
	if ( !totaltime )	totaltime = 1;

	if ( !mystrcmpi( "TOTALHITS", var ) )				return FormatLongNum( VDptr->totalRequests, output );
	if ( !mystrcmpi( "TOTALCACHE", var ) )				return FormatLongNum( VDptr->totalCachedHits, output );
	if ( !mystrcmpi( "TOTALFAILED", var ) )				return FormatLongNum( VDptr->totalFailedRequests, output );
	if ( !mystrcmpi( "TOTALINVALID", var ) )			return FormatLongNum( VDptr->badones, output );
	if ( !mystrcmpi( "AVGDAILYHITS", var ) )			return FormatLongNum( VDptr->totalRequests/(totaltime/ONEDAY), output );
	if ( !mystrcmpi( "AVGHOURLYHITS", var ) )			return FormatLongNum( VDptr->totalRequests/(totaltime/3600.0), output );

	// Client based top values
	if ( VDptr->byClient )
	{
		long totalVisits = VDptr->byClient->GetStatListTotalVisits();

		if ( !mystrcmpi( "TOTALSESSIONS", var ) )		return FormatLongNum( totalVisits, output );
		if ( !mystrcmpi( "TOTALVISITORS", var ) )		return FormatLongNum( VDptr->byClient->GetStatListNum(), output );
		if ( !mystrcmpi( "VISITORSPERHOUR", var ) )		return FormatDoubleNum( VDptr->byClient->GetStatListNum()/(totaltime/3600.0), output );
		if ( !mystrcmpi( "REPEATVISITORS", var ) )		return FormatLongNum( CountRepeatVisits( VDptr->byClient ), output );
		if ( !mystrcmpi( "ONCEVISITORS", var ) )		return FormatLongNum( VDptr->byClient->GetStatListNum() - CountRepeatVisits( VDptr->byClient ), output );
		if ( !mystrcmpi( "DAILYSESSIONS", var ) )		return FormatDoubleNum( totalVisits/((totaltime)/ONEDAY), output );

		if ( !mystrcmpi( "AVGHITSVISIT", var ) ){
			if ( totalVisits )
				return FormatDoubleNum( VDptr->totalRequests/(double)totalVisits, output  );
			else
				return mystrcpy(output, string_na);
		}

		if ( !mystrcmpi( "AVGPAGESVISIT", var ) ){
			if ( totalVisits )
				return FormatDoubleNum( VDptr->byClient->GetStatListTotalCounters4()/(double)totalVisits, output  );
			else
				return mystrcpy(output, string_na);
		}

		if ( !mystrcmpi( "AVGVISITLEN", var ) ){
			if ( totalVisits )
				return FormatDoubleNum( VDptr->byClient->GetStatListTotalTime()/(double)totalVisits, output );
			else
				return mystrcpy(output, string_na);
		}
	}
	else
	{
		if	(	( !mystrcmpi( "TOTALSESSIONS",	var ) )	
			||	( !mystrcmpi( "TOTALVISITORS",	var ) )	
			||	( !mystrcmpi( "VISITORSPERHOUR",var ) )	
			||	( !mystrcmpi( "REPEATVISITORS",	var ) )	
			||	( !mystrcmpi( "ONCEVISITORS",	var ) )	
			||	( !mystrcmpi( "DAILYSESSIONS",	var ) )	
			||	( !mystrcmpi( "AVGVISITLEN",	var ) )	
			||	( !mystrcmpi( "AVGPAGESVISIT",	var ) )	
			||	( !mystrcmpi( "AVGHITSVISIT",	var ) )	
			)
			{
				return mystrcpy(output, string_na);
			}
	}

	// Pages based top values
	if ( VDptr->byPages )
	{
		if ( !mystrcmpi( "TOTALPAGES", var ) )			return FormatLongNum( VDptr->byPages->GetStatListNum(), output );
		if ( !mystrcmpi( "TOTALPAGEHITSPERDAY", var ) )	return FormatDoubleNum( VDptr->byPages->GetStatListTotalRequests()/VDptr->totalDays, output );
		if ( !mystrcmpi( "TOTALPAGEBYTES", var ) )		return FormatLongNum( VDptr->byPages->GetStatListTotalBytes(), output );
		if ( !mystrcmpi( "TOTALPAGEMB", var ) )			return FormatDoubleNum( VDptr->byPages->GetStatListTotalBytes()/(1024*1024), output  );
		if ( !mystrcmpi( "TOTALPAGEHITS", var ) )		return FormatLongNum( VDptr->byPages->GetStatListTotalRequests(), output );
		if ( !mystrcmpi( "TOTALPAGEERRORS", var ) )		return FormatLongNum( VDptr->byPages->GetStatListTotalErrors(), output );
	}
	else
	{
		if	(	( !mystrcmpi( "TOTALPAGES",			var ) )	
			||	( !mystrcmpi( "TOTALPAGEHITSPERDAY",var ) )	
			||	( !mystrcmpi( "TOTALPAGEBYTES",		var ) )	
			||	( !mystrcmpi( "TOTALPAGEMB",		var ) )	
			||	( !mystrcmpi( "TOTALPAGEHITS",		var ) )	
			||	( !mystrcmpi( "TOTALPAGEERRORS",	var ) )	
			)
			{
				return mystrcpy(output, string_na);
			}
	}

	// Downloads based top values
	if ( VDptr->byDownload )
	{
		if ( !mystrcmpi( "TOTALDLOADS", var ) )			return FormatLongNum( VDptr->byDownload->GetStatListNum(), output );
		if ( !mystrcmpi( "TOTALDLOADHITSPERDAY", var ) )return FormatDoubleNum( VDptr->byDownload->GetStatListTotalRequests()/VDptr->totalDays, output );
		if ( !mystrcmpi( "TOTALDLOADBYTES", var ) )		return FormatLongNum( VDptr->byDownload->GetStatListTotalBytes(), output );
		if ( !mystrcmpi( "TOTALDLOADMB", var ) )		return FormatDoubleNum( VDptr->byDownload->GetStatListTotalBytes()/(1024*1024), output );
		if ( !mystrcmpi( "TOTALDLOADHITS", var ) )		return FormatLongNum( VDptr->byDownload->GetStatListTotalRequests(), output );
		if ( !mystrcmpi( "TOTALDLOADERRORS", var ) )	return FormatLongNum( VDptr->byDownload->GetStatListTotalErrors(), output );
	}
	else
	{
		if	(	( !mystrcmpi( "TOTALDLOADS",		var ) )	
			||	( !mystrcmpi( "TOTALDLOADHITSPERDAY",var ) )	
			||	( !mystrcmpi( "TOTALDLOADBYTES",	var ) )	
			||	( !mystrcmpi( "TOTALDLOADMB",		var ) )	
			||	( !mystrcmpi( "TOTALDLOADHITS",		var ) )	
			||	( !mystrcmpi( "TOTALDLOADERRORS",	var ) )	
			)
			{
				return mystrcpy(output, string_na);
			}
	}

	// Downloads based top values
	if ( VDptr->byClips )
	{
		if ( !mystrcmpi( "TOTALCLIPS", var ) )			return FormatLongNum( VDptr->byClips->GetStatListNum(), output );
		if ( !mystrcmpi( "TOTALCLIPSHITSPERDAY", var ) )return FormatDoubleNum( VDptr->byClips->GetStatListTotalRequests()/VDptr->totalDays, output );
		if ( !mystrcmpi( "TOTALCLIPSBYTES", var ) )		return FormatLongNum( VDptr->byClips->GetStatListTotalBytes(), output );
		if ( !mystrcmpi( "TOTALCLIPSMB", var ) )		return FormatDoubleNum( VDptr->byClips->GetStatListTotalBytes()/(1024*1024), output );
		if ( !mystrcmpi( "TOTALCLIPSHITS", var ) )		return FormatLongNum( VDptr->byClips->GetStatListTotalRequests(), output );
		if ( !mystrcmpi( "TOTALCLIPSERRORS", var ) )	return FormatLongNum( VDptr->byClips->GetStatListTotalErrors(), output );
	}
	else
	{
		if	(	( !mystrcmpi( "TOTALCLIPS",		var ) )	
			||	( !mystrcmpi( "TOTALCLIPSHITSPERDAY",var ) )	
			||	( !mystrcmpi( "TOTALCLIPSBYTES",	var ) )	
			||	( !mystrcmpi( "TOTALCLIPSMB",		var ) )	
			||	( !mystrcmpi( "TOTALCLIPSHITS",		var ) )	
			||	( !mystrcmpi( "TOTALCLIPSERRORS",	var ) )	
			)
			{
				return mystrcpy(output, string_na);
			}
	}
	
	// Bandwidth Details.
	{
		double bps = (VDptr->totalBytes*8.0)/(totaltime);
		if ( !mystrcmpi( "BYTES_TOTAL", var ) )				return FormatDoubleNum( VDptr->totalBytes, output );
		if ( !mystrcmpi( "BYTES_TOTALKB", var ) )			return FormatDoubleNum( VDptr->totalBytes/1024.0, output );
		if ( !mystrcmpi( "BYTES_TOTALMB", var ) )			return FormatDoubleNum( VDptr->totalBytes/(ONEMEGFLOAT), output );
		if ( !mystrcmpi( "BYTES_TOTALCOST", var ) )			return FormatLongNum( (MyPrefStruct.dollarpermeg*(VDptr->totalBytes/1000.0))/(ONEMEGFLOAT), output );
		if ( !mystrcmpi( "BYTES_DAILY", var ) )
		{
			if ( ((VDptr->totalBytes/(ONEMEGFLOAT))/totaltime) < 1 )
				return sprintf( output, "%.2f Kb", ( (VDptr->totalBytes/1024.0)/(double)totaltime/ONEDAY) );
			else
				return sprintf( output, "%.2f Mb", ( (VDptr->totalBytes/ONEMEGFLOAT)/(double)totaltime/ONEDAY) );
		}
		if ( !mystrcmpi( "BYTES_BPS", var ) )				return sprintf( output, "%.2f", bps );		//, TranslateID(SUMM_AVGBITSPERSEC)
		if ( !mystrcmpi( "BYTES_PERCENT", var ) )
		{	
			double perc = GetBandwidthName( bps, param );
			return sprintf( output, "%.2f%%", perc*100 );
		}
		if ( !mystrcmpi( "BYTES_SPEED", var ) )
		{	
			double perc = GetBandwidthName( bps, param );
			return sprintf( output, "%s", param );
		}
	}
	// ------------------ IN bound
	{
		if ( !mystrcmpi( "BYTESIN_TOTAL", var ) )			return FormatDoubleNum( VDptr->totalBytesIn, output );
		if ( !mystrcmpi( "BYTESIN_TOTALKB", var ) )			return FormatDoubleNum( VDptr->totalBytesIn/1024.0, output );
		if ( !mystrcmpi( "BYTESIN_TOTALMB", var ) )			return FormatDoubleNum( VDptr->totalBytesIn/(ONEMEGFLOAT), output );
		if ( !mystrcmpi( "BYTESIN_TOTALCOST", var ) )		return FormatLongNum( (MyPrefStruct.dollarpermeg*(VDptr->totalBytesIn/1000.0))/(ONEMEGFLOAT), output );
		if ( !mystrcmpi( "BYTESIN_DAILY", var ) )
		{
			if ( ((VDptr->totalBytesIn/(ONEMEGFLOAT))/totaltime) < 1 )
				return sprintf( output, "%.2f Kb", ( (VDptr->totalBytesIn/1024.0)/(double)totaltime/ONEDAY) );
			else
				return sprintf( output, "%.2f Mb", ( (VDptr->totalBytesIn/ONEMEGFLOAT)/(double)totaltime/ONEDAY) );
		}
		double bps = (VDptr->totalBytesIn*8.0)/(totaltime);
		if ( !mystrcmpi( "BYTESIN_BPS", var ) )				return sprintf( output, "%.2f", bps );
		if ( !mystrcmpi( "BYTESIN_PERCENT", var ) )
		{	
			double perc = GetBandwidthName( bps, param );
			return sprintf( output, " %.2f%%", perc*100 );
		}
		if ( !mystrcmpi( "BYTESIN_SPEED", var ) )
		{	
			double perc = GetBandwidthName( bps, param );
			return sprintf( output, "%s", param );
		}
	}

	// ------------------ SUPPORT HTML VARIABLES FROM SETTINGS FILE
	if ( !mystrcmpi( "HTMLFONTSIZE", var ) )	return sprintf( output, "%d", MyPrefStruct.html_fontsize );
	if ( !mystrcmpi( "HTMLFONT", var ) )		return mystrcpy( output, MyPrefStruct.html_font );
	if ( !mystrcmpi( "HTMLHEADER", var ) )		return mystrcpy( output, MyPrefStruct.html_head );
	if ( !mystrcmpi( "HTMLFOOTER", var ) )		return mystrcpy( output, MyPrefStruct.html_foot );

	if ( !strcmpd( "RGB_", var ) )
	{
		if ( !mystrcmpi( "RGB_DEFAULT", var ) )	return sprintf( output, "%06lx", RGB_Common );
		if ( !mystrcmpi( "RGB_TITLE", var ) )	return sprintf( output, "%06lx", RGBtableTitle );
		if ( !mystrcmpi( "RGB_HEADERS", var ) )	return sprintf( output, "%06lx", RGBtableHeaders );
		if ( !mystrcmpi( "RGB_ITEMS", var ) )	return sprintf( output, "%06lx", RGBtableItems );
		if ( !mystrcmpi( "RGB_OTHERS", var ) )	return sprintf( output, "%06lx", RGBtableOthers );
		if ( !mystrcmpi( "RGB_AVG", var ) )		return sprintf( output, "%06lx", RGBtableAvg );
		if ( !mystrcmpi( "RGB_TOTALS", var ) )	return sprintf( output, "%06lx", RGBtableTotals );
	}
	return 0;
}



// saving config into output format.
// for a given template file with [keywords] in form objects, replace them
// with actual values/numbers whatever to fill in the data.
long ExpandVariable( VDinfoP VDptr, const char *htmltext, char *out, char **data, long logNum )
{
	long		len = 0;
	char		*newPtr;

	if( VDptr )
	{
		char		var[256];

		if ( !strcmpd( "[$", htmltext ) )
		{
			strcpyuntil( var, (char*)htmltext, ']' );
			len = Decode_Variables( VDptr, var, out, logNum );
			newPtr = (char*)strchr( htmltext, ']' );
			newPtr++;
		} else
		if ( !strcmpd( "%5B$", (char*)htmltext ) )
		{
			long copied;
			copied = strcpyuntil( var, (char*)htmltext+2, '%' );
			var[0] = '[';

			len = Decode_Variables( VDptr, var, out, logNum );

			newPtr = (char*)strstr( htmltext, "%5D" );
			if ( newPtr )
				newPtr+=3;
		}
	}
	if ( len ) 
		*data = newPtr;
	return len;
}





char *ResolveAllVariables( VDinfoP VDptr, char *source_html, long datalen, long logNum )
{
	char	*out, 
			*new_html, 
			*data = source_html,
			c;

	new_html = (char *)malloc( datalen*10 );
	out = new_html;

	while( c=*data )
	{
		if ( c=='[' || c=='%' )
		{
			long len = ExpandVariable( VDptr, data, out, &data, logNum );

			if( len > 0 )
			{
				out += len;
				continue;
			}
			// Special case to seek out next <BR> and kill it.
			if( len == -1 )
			{
				char *p;

				p = strchr( data, '<' );

				if ( p && !strcmpd( "<br>", p ) )
					strncpy( p, "    ", 4 );
				continue;
			}
		}
		*out++ = c;
		data++;
	}
	*out++ = 0;

	return new_html;
}



// travers the whole html text, and determine all the IMG tags which files they are and copy them to the outpath.
int CopySourceImages( VDinfoP VDptr, char *htmldata, char *outpath )
{
	char	*data = htmldata;
	char	c;
	int		images = 0;

	while( c=*data )
	{
		if ( c == '<' )
		{
			if ( !strcmpd( "<IMG SRC=\"", data ) )
			{
				char imageName[256];
				char *file = strchr( data, '\"' );
				if ( file )
				{
					file++;
					if ( *file != '%' && *file != '[' )
					{
						long length;
						strcpyuntil( imageName, file, 34 );
						//Load File into RAM
						char *ram = LoadSummaryFile( imageName, &length );
						if ( ram )
						{
							char newFilename[512];
							//Save file to report path...
							sprintf( newFilename, "%s%s", outpath, imageName );
							MemToFile( newFilename, ram, length );
							images++;
							//free ram.
							free( ram );
						}
					}
				}
			}
		}
		data++;
	}
	return images;
}


// determin which areas to remove/keep based on <!-- group XYZ --> comments....
/*
Declare all regions, like demographics in comments 
<!-- groupstart demographics -->
<!-- groupend -->


Indiviual stats can be 

<!-- statstart daily -->
<!-- statend -->

*/

char *SkipAllUntil( char *source, char *until )
{
	char *p = NULL;
	if ( p = strstr( source, until ) )
	{
		if ( p = strchr( p, '>' ) )
		{
			p++;
			mystrcpy( source, p );
		}
	}
	return p;
}


char *SkipStat( char *source )
{
	return SkipAllUntil( source, "#endstat" );
}

char *SkipGroup( char *source )
{
	return SkipAllUntil( source, "#endgroup" );
}



// ------ LOGIC FILTER -----
// this acts like an #IFDEF STATISON #ENDIF pair.
long FilterSections( VDinfoP VDptr, char *source_html, long dovalidreports )
{
	char *groups[] = {	"Traffic", "Diagnostic", "Server", "Demographic", "Referrals",
						"Multimedia", "Multi Media", "Clip Access Information", "Live Clip Access Information"
						"Systems", "Advertising", "Billing", "Marketing", "Clustering", 0 };
	char *p = source_html;

	while ( *p )
	{
		if ( *p == '<' )
		{
			char Name[100], *ptr;

			if ( !strcmpd( "<!--", p ) )
			{
				char *comment = strchr( p, '#' );

			
				// if these statistic group is on....
				if ( !strcmpd( "#if groupOn=", comment ) )
				{
					long i = 0;

					ptr = strchr( p, '=' );
					if ( ptr )
					{
						ptr++;
						strcpyuntil( Name, ptr, '-' );

						while( groups[i] )
						{
							if ( !strcmpd( groups[i], Name ) ) {
								if( IsGroupReportNameOn( &MyPrefStruct, groups[i] ) == FALSE )
									SkipGroup( p );
							}
							i++;
						}
					}
				}
				else
				// if this indivdual statistic is turned on....
				if ( !strcmpd( "#if statOn=", comment ) )
				{
					long i = 0;

					ptr = strchr( p, '=' );
					if ( ptr )
					{
						ptr++;
						strcpyuntil( Name, ptr, '-' );

						if ( !IsReportNameOn( &MyPrefStruct, Name ) )
							SkipStat( p );
					}
				}
				else
				// if logs processed are in style format ...
				if ( !strcmpd( "#if logstyle", comment ) )
				{
					long i = 0;

					ptr = strchr( comment, '=' );
					if ( ptr )
					{
						ptr++;
						strcpyuntil( Name, ptr, '-' );

						if ( !strcmpd( "proxy", Name ) && ISLOG_PROXY(logStyle) )
							SkipAllUntil( p, "#endstyle" );

						if ( !strcmpd( "web", Name ) && ISLOG_WEB(logStyle) )
							SkipAllUntil( p, "#endstyle" );
					}
				}

				if ( dovalidreports )
				{
					// perform #define macros for default command templates
					// eg.
					// <!-- #define link1=<li><a href=\"%s.html\"> %s </a></li> -->
					if ( !strcmpd( "#define", comment ) )
					{
						long style; 
						char *end;

						ptr = strchr( comment, '=' );

						if ( ptr )
							style = atoi( ptr-1 );

						if ( style >= MAX_STYLES )
							style = MAX_STYLES-1;
		
						ptr = strchr( comment, '=' );
						end = strstr( comment, "-->" );

						*end = 0;

						if ( !strcmpd( "#define link", comment ) )
							STORE_LINKTEMPLATE( style , ptr+1 );
						else
						if ( !strcmpd( "#define thumblink", comment ) )
							STORE_THUMBLINKTEMPLATE( style , ptr+1 );

						*end = '-';
					} else


					// if this X statistic has data in it....
					if ( !strcmpd( "#if statReported=", comment ) )
					{
						long i = 0;

						ptr = strchr( comment, '=' );
						if ( ptr )
						{
							ptr++;
							strcpyuntil( Name, ptr, '-' );

							// *********************************************************************
							// As the Broken Links report has different behaviour to the other reports
							// we need to code in a 'special' to create this behaviour.
							// If the user has specified a SiteURL for their website then the Brokenlinks
							// report is split into both Internal and External broken links.
							// To reflect this in the summary template we need to create a label to
							// specify the report. This is "ExtBrokenLinks" and "IntBrokenLinks".
							// *********************************************************************
							if ( (mystrcmpi("IntBrokenLinks", Name) == 0) || (mystrcmpi("ExtBrokenLinks", Name) == 0) )
							{
								if ( VDptr->GetSiteURL() == NULL)	// we are NOT SUPPOSED to have generated int/ext report.
									SkipStat(p);
								else if ( !IsReportNameCompleted( &MyPrefStruct, "BrokenLinks" ) ) // if it was NOT generated!
									SkipStat(p);
							}
							else if ( mystrcmpi("BrokenLinks", Name) == 0)
							{
								if ( VDptr->GetSiteURL() != NULL)	// we ARE SUPPOSED to have generated int/ext report.
									SkipStat(p);
								else if ( !IsReportNameCompleted( &MyPrefStruct, "BrokenLinks" ) ) // if it was NOT generated!
									SkipStat(p);
							}
							else if ( !IsReportNameCompleted( &MyPrefStruct, Name ) )
							{
								SkipStat(p);
							}
						}
					} else
					if ( !strcmpd( "#if groupReported=", comment ) )
					{
						ptr = strchr( comment, '=' );

						if ( ptr )
						{
							ptr++;
							strcpyuntil( Name, ptr, '-' );

							long i = 0;
							while( groups[i] && *groups[i] )
							{
								if ( !strcmpd( groups[i], Name ) ) 
								{
									if( IsGroupReportNameCompleted( &MyPrefStruct, groups[i] ) == FALSE )
										SkipGroup( p );
									break;
								}
								i++;
							}
						}
					}
				}
			}
		}
		p++;
	}
	return strlen(source_html);
}


long SwitchonAllThumbnails( VDinfoP VDptr )
{
	long	length = 0, 
			total = 0;
	char	*sourceHTML;

	if ( SummaryTemplatename[0] )
		sourceHTML = LoadSummaryFile( SummaryTemplatename, &length );
	else
		sourceHTML = LoadSummaryFile( DEFAULT_SUMMARYTEMPLATE, &length );

	if ( sourceHTML  )
	{
		length = FilterSections( VDptr, sourceHTML, FALSE /* do all valid #ifs too */  );

		char	*data = sourceHTML;


		while( char c=*data )
		{
			if ( c=='[' || c=='%' )
			{
				if ( !strcmpd( "[$multithumblink", data ) )
				{
					char	var[256];
					strcpyuntil( var, (char*)data, ']' );
					if ( char *p=strchr( var, '_' ) )
					{
						long i = ConfigNameFindSettings( &MyPrefStruct, p+1, NULL );
						while( i ){
							p = (char*)ConfigLookupName( i );
							if ( p ){
								ActivateThumbImage( p );
							} else
								break;
							i++;
						}
					}
				} else
				if ( !strcmpd( "[$thumb_", data ) || !strcmpd( "[$thumblink", data ) )
				{
					char	var[256];
					strcpyuntil( var, (char*)data, ']' );
					if ( char *p=strchr( var, '_' ) )
					{
						char statName[100];
						strcpyuntil( statName, (char*)p+1, ']' );
						ActivateThumbImage( statName );
						total++;
					}
				}
			}
			data++;
		}

		free( sourceHTML );
	}
	return total;
}


// ------------------------------------------------ MAIN ENTRY POINT ------------------------------------------------
long CreateSummaryPage( VDinfoP VDptr, FILE *outfp, char *fileLocation, long logNum )
{
	long length = 0;
	char *newHTML;
	char outpath[512];
	char *sourceHTML;

	if ( SummaryTemplatename[0] ){
		sourceHTML = LoadSummaryFile( SummaryTemplatename, &length );
		OutDebugs( "Loaded alternative template %s", SummaryTemplatename );
	} else {
		sourceHTML = LoadSummaryFile( DEFAULT_SUMMARYTEMPLATE, &length );
		OutDebugs( "Loaded default template %s", DEFAULT_SUMMARYTEMPLATE );
		if ( !sourceHTML )
		{
			OutDebugs( "Loadeding old default template %s", DEFAULT_OLDSUMMARYTEMPLATE );
			sourceHTML = LoadSummaryFile( DEFAULT_OLDSUMMARYTEMPLATE, &length );
		}
	}

	if ( sourceHTML && outfp )
	{

		// Write DEMO notice in demo mode.
		if ( IsDemoReg() )
		{
			fprintf (outfp, "<center><i><font size=\"6\" color=\"red\">iReporter Demonstration</font></i><br>\n");
			fprintf (outfp, "<a href=\"%s\">Order iReporter Here</a>\n<p></center>\n", URL_STORE );
			fflush( outfp );
		}

		PathFromFullPath( fileLocation, outpath );
		// this will reduce the length if any segments are not to be used.
		length = FilterSections( VDptr, sourceHTML, TRUE /* do all valid #ifs too */  );

		// Copy all image files in the html from the template path to the report path.
		CopySourceImages( VDptr, sourceHTML, outpath );

		// Convert all {$VARS} to their respective proper values.
		newHTML = ResolveAllVariables( VDptr, sourceHTML, length, logNum );

		// save the new html
		if ( newHTML )
		{
			size_t len = mystrlen( newHTML );
			fwrite( newHTML, len, 1, outfp );
			fflush( outfp );
			free( newHTML );
		}
		//fclose( outfp );		// DO NOT CLOSE, as the caller does it. , doing it twice screws up linux.
		free( sourceHTML );
	}
	return length;
}


































