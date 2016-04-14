/*

This controls the full settings saving/loading and argument handeling.

*/

#include "FWA.h"

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <string>


// COMMON INCLUDES
#include "myansi.h"
#include "datetime.h"
#include "editpath.h"
#include "schedule.h"
#include "postproc.h"
#include "net_io.h"
#include "log_io.h"
#include "schedule.h"
#include "config.h"
#include "Registration.h"
#include "translate.h"
#include "OSInfo.h"
#include "version.h"
#include "Report.h"
#include "FileTypes.h"
#include "GlobalPaths.h"
#include "IceStr.h"

// UNIX SPECIFICS
#if DEF_UNIX
	#include "main.h"
	#include "util.h"
#endif

// Mac SPECIFICS
#if DEF_MAC
	#include "main.h"
	#include "MacStatus.h"
	#include "MacDebug.h"
	#include "MacUtil.h"
#endif

// WINDOWS BITS
#if DEF_WINDOWS
	#include <windows.h>
	#include "winmain.h"
	#include "createshortcuts.h"
#endif

#include <vector>
using namespace std;

extern "C" long AddWildCardFilenames( char *szTempFile, long start );


// NOTE Addlog has been moved here since its more of a universal function.
Prefs	MyPrefStruct;
Prefs	TempPrefStruct;

int		gVerbose = FALSE, gDebugPrint = FALSE;

const char *HTML_EXT = ".html";
const char *PDF_EXT = ".pdf";
const char *RTF_EXT = ".rtf";
const char *EXCEL_EXT = ".xls";
const char *COMMA_EXT = ".csv";

static char *helptext[] = {
"--------------------- iReporter  ---------------------------\n",
"-h                   Help\n",
"-l file              Use logfile(s) , you can use wildcards but enclose in quotes. ie \"*.log\" \n",
"-l FTPURL            You can also source your logs from an ftp server using a standard URL (ftp://username:password@site/file.log)\n",
"-l HTTPURL           You can also source your logs from an http server using a standard URL (http://myserver.com/logs/access_log)\n",
"-r file              Use report config [file] or\n",
"-settingsfile file    \n",
"-includesettings X   Append (or include) a subset of a settings file\n"
"-saveprefs X         Save internal settings to a prefs file (for testing) where X is the file\n"
"-q                   Quiet no output at all\n",
"-q1                  Normal status on\n",
"-ver                 Display Current Version Information\n",
#ifndef DEF_FREEVERSION
"-reg                 Perform registration procedure\n"
#endif
"-convert_to_w3c X    Convert processed log files into one W3C format log file X\n"
"                     eg. '–l /test.log –convert_to_w3c new.log'   creates converted file called new.log\n"
"-debugarg            Debug arguments passed\n"
"-debug               Log debug messages to log file '~/.debug_log.txt'\n"
"-debugprint          Log debug messages and print to console\n"
"-summaryfile X       Use file X as the new summary template\n"
"-searchfor X         Search for pattern in URL and...\n",
"-replacewith X       Replace it with this value/string (both these have to be used together)\n",
"-checkschedule       Check the internal schedule list to decide processing times (schedule file is ~/.schedule.txt)\n",
"-runschedule X       Run schedule X in the schedule config settings\n",
"NOTE: when using a * or other wildcards, supply it inside a quotes to prevent the shell from interpreting it.\n",

"\n--------- General Report Style Options\n",
"-d or -o or -out     Destination directory/filename.html\n",
"-reportformat X      Choose a new output format, (html,pdf,rtf,comma,excel) or (1..5)\n",
"-imageformat X       Define the image format, (gif,jpeg,png,bmp,none or 0,1,2,3 )\n",
"-language X          Language output type (1=German,2=Italian,3=French,4=Spanish,5=Swedish,6=Norw,7=Danish,8=Dutch,9=Japan)\n,t\t(or you can pass the name of the language)\n",
"-footer_label        Enable header to also appear at bottom of the table\n",
"-report_title        Define the title of the report\n",
"-theme X             Use color theme X (0=random,1=default,2=basic,3=grey,4=blue,5=wild,6=blan,7=ice)\n",
"-shadow              Use shadows on graphs or '-noshadow' to switch off shadows\n",
"-headingonleft       Place main column name on left side\n",
"-headingonright      Place main column name on right side\n",
"-style [2d/3d]       Use graph style [3d/2d] on bar graphs\n",
"-webpalette          Use 256 netscape colors in all images\n",
"-wider               Draw wider format graphs\n",
"-corporate_look      Make all graphs look corporate style\n",
"-allimagesoff        Turn off ALL Images for all reports\n",

"\n--------- Database Options\n",
"-database_active     Activate saving reports to a binary database\n",
"-database_extended   Database is an extended format (V5) database\n",
"-database_file       File to use or create\n",
"-database_no_report  Do not produce a report once data has been added to the database\n",
"-database_excluded   Exclude database from report (report on new logs only)\n",

"\n--------- HTML Options\n",
"-html_font X         Specify html fonts to use in reports\n",
"-html_fontsize n     Specify html font size to use with above\n",
"-html_frames         Enable html reports to be reported in framed mode\n",
"-html_quickindex     Enable report index in every report page\n",
"-head                File to use for HTML header\n",
"-foot                File to use for HTML footer\n",

"\n--------- Analysis Options\n",
"-sdate X             Set start date [MM/DD/YY]\n",
"-edate X             Set end date [MM/DD/YY] (if start date not entered, defaults to current date)\n",
"-start_date X        Set start date YYYY/MM/DD absolute dates\n",
"-end_date X          Set end date YYYY/MM/DD absolute dates\n",
"-alldates            process all dates\n",
"-time_adjust X       Adjust a time offset to all times (seconds or hh:mm:ss)\n",
"-forcedateformat X   Overide default dateformat and use specified (DD/YY, MM/DD, YY/MM)\n",
"-ignorezerobyte      Filter out zero byte files\n",
"-ignorerobots        Ignore all hits comming from robots\n",
"-ignoreself          Ignore referrals pointing back to your server (must specify –siteurl and –indexfile)\n",
"-ignorecase          Ignore case for all urls/files\n",
"-ignoreusernames     Dont use authenticated usernames for visitors, but only the ip or domain name.\n",
"-ignorebookmark      Ignore bookmark referrals or file:// referrals\n",
"-ignoreipvhosts      Ignore virtualhosts IPs\n",
"-siteurl             Specify the name of the host that is localhost to the log file  eg: http://www.home.com\n",
"-indexfile X         Specify the default server index file (ie index.html)\n",
"-page.ext X          Supply a list of extensions in the format  .abc,.zxc  to define pages\n",
"-download.ext X      Supply a list of extensions in the format  .abc,.zxc  to define downloads\n",
"-audio.ext X         Supply a list of extensions in the format  .abc,.zxc  to define audio files\n",
"-video.ext X         Supply a list of extensions in the format  .abc,.zxc  to define video files\n",
"-usecgi              Accept cgi parameters in all urls\n",
"-retain_variable     Retain the group variables in all cgi parameters listed with -retain_varlist X\n",
"-retain_varlist X    Retain these groups of variables (comma seperated)\n",
"-flushclients        Active stream flushing mode where least common clients are flushed (only use if short on ram)\n",
"-filterIn TYPE KEY   Add an include data input filter;\n",
"                     TYPE can be any of ... URL, Visitor, Agent, Referral, Stat, VHost, Cookie, Username, Method, SessionReferral\n",
"                     KEY  is your data that your are filtering, wildcards accepted\n",
"-filterexIn TYPE KEY Add an exclude data input filter\n",
"-sort X              Sort all by [name | byte | hits | page]\n",
"-sessiontime [mins]  Specify the length in minutes of non activity that defines a new session\n",
"-sessionLimit X      Limit the amount of sessions listed in the visitor sessions reports to X (default is 1000)\n",
"-dollar X            Define the cost per MB in dollars, ie 0.05\n",
"-cookievar X         Accept cookie parameters for user tracking\n",
"-urlsessionvar X     Accept URL session variable for user tracking\n",
"-allhistoriesoff     Turn OFF all History reports to save ram and speed up processing on large logs\n",
"-sessionhistoryoff   Turn OFF all Click Streams Session to speed up processing, (you will loose dependant reports)\n",

"\n--------- Group Mapping Options\n",
"-orgmap X=Y          Add an organization map (*.com.*=Commercial)\n",
"-groups X=Y          Add a url/content group map (/images/*=ServerImages)\n",
"-referralmap X=Y     Add a referral url group map (*.yahoo.com=Yahoo Corp)\n",
"-advert X            Define an advertising url based on adds served served locally\n",
"                     where X= Description,Filename,Click String \n",
"                     eg. MyProduct,/images/prodx.gif,/prodxpage.html\n",
"-advertcampaign X    Define an advertising campaign based referral hits\n",
"                     where X= Description,Referral,Cost/Month ($)\n",
"                     eg. ProdX campaign,http://www.adhost.com/ProdXAd.html,1.50\n",
"-internalnetwork X   Add an internet network IP filter (router only) where X=your local ip wildcard\n",

"\n--------- Tracking Options\n",
"-kvisitormap X       Add a Key Visitor where X=visitor ip or name\n",
"-kpagetomap X        Add a Key Pages To: where X=page url\n",
"-kpageto_depth X     Define the number of pages to trace back\n",
"-kpageto_maxrows X   Defines the maximum number of alternate routes to key pages\n",
"-kpagefrommap X      Add a Key Pages From: where X=page url\n",
"-kpagefrom_depth X   Define the depth of pages to trace forward\n",
"-kpagefrom_maxrows X Defines the maximum number of alternate routes from a key page\n",

"\n--------- Virtual Host Options\n",
"-multimonths         Process a log file and generate a report for every month\n",
"-multiweeks          Process a log file and generate a report for every week\n",
"-multivhosts         Process a log file and extact virtual host info to create a host-by-host report\n",
"-multidomains        Process multiple log files as multiple domains and not a combined report\n",
"-multitopdirs        Process a log file and generate a report for every top level directory\n",
"-multitop2dirs       Process a log file and generate a report for every second level directory\n",
"-multiclients        Process a log file and generate a report for every visitor\n",
"-multicustomdirs     Process a log file with a custom directory root path as the virtual host root for all next dirs\n",
"                     Where X = Custom directory eg. /images/,/about/,/etc../ \n",
"-multireport_path    Use this as the root path\n",
"-sequencedir         For all virtual hosts, use a squential numbered directory instead of its name\n",
"-vdsortby            Sort the virtual host list in order of [name | byte | hits | page | date] or (0..5)\n",
"-editpath {site}={dir}        : add a virtual host mapping dir, so {site} goes into {dir}\n",
"-vhostmap {pattern}={newhostname}        : remap a patterned vhost into a new vhost name or identity\n",
"-realtimeinterval    Number of seconds between new reports (quassi real time mode)\n",
"-weekstartday X      Define which day the week starts on (0 ... 6) (sun/mon/tue/wed/thu/fri/sat)\n",

"\n--------- Cluster Options\n",
"-clustering_enabled  Clustering is enabled\n",
"-clusters X          Define the number of clusters in use\n",
"-defineCluster X     Define a cluster member where X = 'name,ip,path'\n",
"\n",
"\n--------- Network Options\n",
"+dns or -dnson       Perform dns on visitor IPs \n",
"-dnsCache            Perform dns on visitor IPs from cache \n",
"-dnsAmount X         Number of max lookups  (2...1000)\n",
"-dnr_ttl X           Number of seconds for unresolved IPs to timeout\n",
"-dnr_expire X        Number of days the resolved IP will be kept in cache\n",
"-dnrfile X           Use file X as the cache for DNR queries\n",
#if DEF_UNIX
"-dns_server X        Specify a different dns server to use for lookups\n",
"-dns_timeout X       Timeout for lookups\n",
"-dns_retries X       Amount of retries for lookups\n",
"-ftp_passive         Enable all ftp operations to be passive (best for firewalls)\n",
"-remotetitle         Enable remote page titles in pages reports, ie to resolve urls \n",
#endif

"\n--------- Old Report Summary Options\n",
"-timestat            Include time processing information in report\n",
"-notimestat          Don't include time processing information in report\n",
"-bandwidth           Include bandwidth information in report\n",
"-nobandwidth         Don't include bandwidth information in report\n",
"-headtitle           Include log name in report header\n",
"-noheadtitle         Do not place the log name in the report header\n",
"-server_bandwidth X  Define the server bandwidth for report reference (x=bps)\n",

"\n--------- Pre and Post Processing Options\n",
#ifdef DEF_DEBUG
"-preproc_on          Turn on Pre Processing\n",
"-preproc_location X  Source of preprocess log files (eg. X = ftp://username::password@ftpserver/logs/*.log )\n",
"-preproc_tmp X       Temp location for download of preprocess logs\n",
"-preproc_delete      Delete downloaded files after processing\n",
#endif
"-post_zipreport      Archive the report into a zip file\n",
"-postproc_deletereport    Delete report after zipping\n",
"-post_compresslog    Compress the processed log into a gziped log\n",
"-postproc_deletelog  Delete above original log after compressing\n",

#ifdef DEF_DEBUG
"\n--------- Misc Options\n",
"-remotelogin_port    Remote login port for ServerMode (911 = default)\n",
"-remotelogin_pass    Remote login password (test = default)\n",
"-remoteloginon       Activate remote login\n",
"-daemon              Run as a server using above details\n",
#endif

NULL };


char **GetHelpText( void )
{
	return helptext;
}


int Fprintf( FILE *fp, const char *fmt, ... )
{
	va_list		args;
	long ret;
	char lineout[10240];

	va_start( args, fmt );
	vsprintf( lineout, fmt, args );
	va_end( args );

	ret = fwrite( lineout, 1, strlen(lineout), (FILE*)fp );

	fflush( fp );
	return ret;
}



long OutDebug( const char *txt )
{
	if ( gDebugPrint && txt ){

#if DEF_WINDOWS
		OutputDebugString( txt );
		OutputDebugString( "\n" );
#elif DEF_UNIX
		LogMessage( NULL, (char*)txt, 0 );
		if ( gDebugPrint == 2 )
			printf( "%s\n", txt );
#elif DEF_MAC
		AppendDebugFile (txt);		
#endif
	}
	return gDebugPrint;
}


int IsDebugMode( void )
{
	return gDebugPrint;
}



long OutDebugs( const char *txt, ... )
{
	if ( gDebugPrint && txt ){
		char lineout[4000];

		if ( txt ){
			va_list		args;
			va_start( args, txt);
			vsprintf( lineout, txt, args );
			va_end( args );

			OutDebug( lineout );
			//OutDebug( "\n" );
		}
	}
	return gDebugPrint;
}

long LogMessage( char *filename, char *str, long err )
{ 
	char		logname[]="debug_log.txt";
	FILE		*fp;
	long		ferr=0;
	struct		tm	tr;

	
	if ( str ){
		char 	log_str[300];
		long	len;
static	long last_err = 0;

		GetCurrentDate( &tr );
		tr.tm_year+=1900;

		if( err == -1 ) last_err = -1;

		if ( err == -1 || last_err == -1 )
			len = sprintf( log_str, "%02d:%02d:%02d %d/%d/%d - NTSERVICE:    %s\n", tr.tm_hour,tr.tm_min,tr.tm_sec, tr.tm_mday,tr.tm_mon+1,tr.tm_year,  str );
		else
		if ( err )
			len = sprintf( log_str, "%02d:%02d:%02d %d/%d/%d - err=(%ld):    %s\n", tr.tm_hour,tr.tm_min,tr.tm_sec, tr.tm_mday,tr.tm_mon+1,tr.tm_year,  err, str );
		else
			len = sprintf( log_str, "%02d:%02d:%02d %d/%d/%d :    %s\n", tr.tm_hour,tr.tm_min,tr.tm_sec, tr.tm_mday,tr.tm_mon+1,tr.tm_year, str );

		if ( !filename ) filename = logname;
		fp = fopen( filename, "a+" );
		if ( fp==NULL ) {
			sprintf( log_str, "cannot open logger %s", filename );
			perror( log_str );
			return ferr;
		}
		if ( fp ){
			fwrite( log_str, len, 1, fp );
			fclose( fp );
		}
	} 
	return 0;
}

#define EXTSIZE		8
#define	EXTTOTAL	256

char defaultpageStr[EXTTOTAL][EXTSIZE]=			{ ".html", ".htm", ".shtml", ".lhtml", ".htmls", ".phtml", ".dbm", ".ssi", ".asp", ".txt", ".cgi", ".pl", ".exe", ".dll", ".tmpl", ".tpl", ".cfm", ".php3", ".php",".fcgi", ".lasso", ".srch", ".nclk", ".t",  ".taf", ".qry", ".flx", ".acgi", 0 };
char defaultdownloadStr[EXTTOTAL][EXTSIZE]=		{ ".zip", ".bin", ".sit", ".hqx", ".tar", ".exe", ".pdf", ".wrl", ".tgz", ".gz", ".z", ".rpm", ".arj", ".lha", ".rar", ".ps", ".bz2", 0 };
char defaultaudioStr[EXTTOTAL][EXTSIZE]=		{ ".mp3", ".ram", ".ra", ".wma", ".aiff", ".wav", ".mp2", ".ac3", ".vqf", 0 };
char defaultvideoStr[EXTTOTAL][EXTSIZE]=		{ ".mov", ".asf", ".asx", ".wmv", ".rm", ".qt", ".mpg", ".avi", ".m2v", ".m1v", ".vob", 0 };

void DefaultExtensions( Prefs *pPrefs )
{
	memcpy( pPrefs->pageStr, defaultpageStr, EXTTOTAL*EXTSIZE );
	memcpy( pPrefs->downloadStr, defaultdownloadStr, EXTTOTAL*EXTSIZE );
	memcpy( pPrefs->audioStr, defaultaudioStr, EXTTOTAL*EXTSIZE );
	memcpy( pPrefs->videoStr, defaultvideoStr, EXTTOTAL*EXTSIZE );
}




void SaveExtStrings( FILE *fp, char strings[EXTTOTAL][EXTSIZE], char *name )
{
	long lp = 0;
	Fprintf( fp, "%s ",name );
	fflush( fp );

	while( strings[lp][0] ){
		if ( lp )	Fprintf( fp, "," );
		Fprintf( fp, "%s", strings[lp] ); lp++;
	}
	Fprintf( fp, "\n" );
}


void LoadExtStrings( char strings[EXTTOTAL][EXTSIZE], char *data )
{
	long lp = 0, i = 0;

	while( data && lp<EXTTOTAL ){
		if ( *data == ',' || !*data ){
			strings[lp][i] = 0;
			lp++; i=0;
			if ( !*data ) {
				data = 0;
				continue;
			}
		} else
			strings[lp][i++] = *data;
		data++;
	}
}
void AddExtStrings( char strings[EXTTOTAL][EXTSIZE], char *data )
{
	long lp = 0, i = 0;

	while( data && lp<EXTTOTAL ){
		if ( strings[lp][0] == 0 ){
			if ( *data == ',' || !*data ){
				strings[lp][i] = 0;
				lp++; i=0;
				if ( !*data ) {
					data = 0;
					continue;
				}
			} else
				strings[lp][i++] = *data;
			data++;
		} else lp++;
	}
}

// ---------------------------------------------------------------------------------------------------------------





#define	DEFAULT_CLUSTEROPTIONS		((SORT_REQUESTS<<26) | (4<<16) | 7)


// ---------------------------------------------------------------------------------------------------------------
// Maybe make this stuff below into one Class called "ReportSettings::"
//
//	type	Display_Title			Settings_Name			address where the data is stored
ConfigNames stat_confignames[] = {
	HIDDEN,	"VirtualHost",			"vhost",					offsetof(Prefs, stat_vhost),						VHOST_PAGE,

	WEB,	">Summary",				0,							offsetof(Prefs, stat_summary),						0,
	WEB,	"Processing Statistics","summary",					offsetof(Prefs, stat_summary),						0,
																													
	// ------------------------
	WEB,	">Traffic",				0,							offsetof(Prefs, stat_traffic),						TRAFFIC_GROUP,
	WEB,	"Hourly",				"hourly",					offsetof(Prefs, stat_hourly),						HOUR_PAGE,
	WEB,	"Hourly History",		"hourlyhistory",			offsetof(Prefs, stat_hourlyHistory),				HOURHIST_PAGE,
	WEB,	"Daily",				"daily",					offsetof(Prefs, stat_daily),						DATE_PAGE,
	WEB,	"Recent Daily",			"recentdaily",				offsetof(Prefs, stat_recentdaily),					RECENTDATE_PAGE,
	WEB,	"Weekly",				"weekly",					offsetof(Prefs, stat_weekly),						WEEK_PAGE,
	WEB,	"Weekdays",				"weekdays",					offsetof(Prefs, stat_weekdays),						WEEKDAYS_PAGE,
	HIDDEN,	"Monday",				"monday",					offsetof(Prefs, stat_hourly),						MONDAY_PAGE,
	HIDDEN,	"Tuesday",				"tueday",					offsetof(Prefs, stat_hourly),						TUEDAY_PAGE,
	HIDDEN,	"Wednesday",			"wedday",					offsetof(Prefs, stat_hourly),						WEDDAY_PAGE,
	HIDDEN,	"Thursday",				"thuday",					offsetof(Prefs, stat_hourly),						THUDAY_PAGE,
	HIDDEN,	"Friday",				"friday",					offsetof(Prefs, stat_hourly),						FRIDAY_PAGE,
	HIDDEN,	"Saturday",				"satday",					offsetof(Prefs, stat_hourly),						SATDAY_PAGE,
	HIDDEN,	"Sunday",				"sunday",					offsetof(Prefs, stat_hourly),						SUNDAY_PAGE,
	WEB,	"Monthly",				"monthly",					offsetof(Prefs, stat_monthly),						MONTH_PAGE,
																													
	// ------------------------									
	WEB,	">Diagnostic",			0,							offsetof(Prefs, stat_diagnostics),					DIAGNOSTIC_GROUP,
	WEB,	"Server Errors",		"errors",					offsetof(Prefs, stat_errors),						ERRORS_PAGE,
	WEB,	"Server Errors History","errorshistory",			offsetof(Prefs, stat_errorsHistory),				ERRORSHIST_PAGE,
	WEB,	"Failed URLs",			"errorurl",					offsetof(Prefs, stat_errorurl),						ERRORURL_PAGE,
	WEB,	"Failed URLs History",	"errorurlhistory",			offsetof(Prefs, stat_errorurlHistory),				ERRORURLHIST_PAGE,
#ifdef DEF_FULLVERSION										
	HIDDEN,	"Int Links",			"brokenlinks",				offsetof(Prefs, stat_brokenLinks),					EXTBROKENLINKS_PAGE,
	HIDDEN,	"Ext Links",			"brokenlinks",				offsetof(Prefs, stat_brokenLinks),					INTBROKENLINKS_PAGE,
	WEB,	"Pages Containing "																						
			"Broken Links",			"brokenlinks",				offsetof(Prefs, stat_brokenLinks),					BROKENLINKS_PAGE,
#endif
	// ------------------------
	WEB,	">Server",				0,							offsetof(Prefs, stat_server),						SERVER_GROUP,
	WEB,	"Pages",				"pages",					offsetof(Prefs, stat_pages),						PAGES_PAGE,
	WEB,	"Pages History",		"pageshistory",				offsetof(Prefs, stat_pagesHistory),					PAGEHIST_PAGE,
	WEB,	"Pages (Least Visited)","pagesleast",				offsetof(Prefs, stat_pagesLeastVisited),			PAGESLEAST_PAGE,
	WEB,	"Entry Pages",			"pagesfirst",				offsetof(Prefs, stat_pagesfirst),					PAGESFIRST_PAGE,
	WEB,	"Entry Pages History",	"pagesfirsthistory",		offsetof(Prefs, stat_pagesfirstHistory),			PAGESFIRSTHIST_PAGE,
	WEB,	"Exit Pages",			"pageslast",				offsetof(Prefs, stat_pageslast),					PAGESLAST_PAGE,
	WEB,	"Exit Pages History",	"pageslasthistory",			offsetof(Prefs, stat_pageslastHistory),				PAGESLASTHIST_PAGE,
	WEB,	"Directories",			"dir",						offsetof(Prefs, stat_dir),							DIRS_PAGE,
	WEB,	"Top Level Directories",		"topdir",			offsetof(Prefs, stat_topdir),						TOPDIRS_PAGE,
	WEB,	"Top Level Directories History","topdirhistory",	offsetof(Prefs, stat_topdirHistory),				TOPDIRSHIST_PAGE,
	WEB,	"URLs",					"file",						offsetof(Prefs, stat_files),						FILE_PAGE,
	WEB,	"Mean Path",			"meanpath",					offsetof(Prefs, stat_meanpath),						MEANPATH_PAGE,
#ifdef DEF_FULLVERSION										
	WEB,	"Routes to KeyPages",	"keypageroute",				offsetof(Prefs, stat_keypageroute),					KEYPAGEROUTE_PAGE,
	WEB,	"Routes from KeyPages",	"keypageroutefrom",			offsetof(Prefs, stat_keypageroutefrom),				KEYPAGEROUTEFROM_PAGE,
	WEB,	"Content Groups",		"groups",					offsetof(Prefs, stat_groups),						GROUPS_PAGE,
	WEB,	"Content Groups History","groupshistory",			offsetof(Prefs, stat_groupsHistory),				GROUPSHIST_PAGE,
#endif
	WEB,	"Downloads",			"download",					offsetof(Prefs, stat_download),						DOWNLOAD_PAGE,
	WEB,	"Downloads History",	"downloadhistory",			offsetof(Prefs, stat_downloadHistory),				DOWNLOADHIST_PAGE,
	WEB,	"Uploads",				"upload",					offsetof(Prefs, stat_upload),						UPLOAD_PAGE,
	WEB,	"Uploads History",		"uploadhistory",			offsetof(Prefs, stat_uploadHistory),				UPLOADHIST_PAGE,
	WEB,	"Page Redirects",		"redirects",				offsetof(Prefs, stat_redirects),					REDIRECTS_PAGE,
	WEB,	"File Types",			"filetype",					offsetof(Prefs, stat_type),							FILETYPE_PAGE,
																
	// ------------------------									
	WEB,	">Demographic",			0,							offsetof(Prefs, stat_demographics),					DEMOGRAPHIC_GROUP,
	WEB,	"Visitors",				"client",					offsetof(Prefs, stat_client),						CLIENT_PAGE,
	WEB,	"Visitors History",		"clienthistory",			offsetof(Prefs, stat_clientHistory),				CLIENTHIST_PAGE,
#ifdef DEF_FULLVERSION										
	WEB,	"Visitors Click Stream","clientstream",				offsetof(Prefs, stat_clientStream),					CLIENTSTREAM_PAGE,
#endif
	WEB,	"Authenticated Users",	"users",					offsetof(Prefs, stat_user),							USER_PAGE,
	WEB,	"Authenticated Users History","usershistory",		offsetof(Prefs, stat_userHistory),					USERHIST_PAGE,
#ifdef DEF_FULLVERSION										
	WEB,	"Authenticated Users Click Stream","usersstream",	offsetof(Prefs, stat_userStream),					USERSTREAM_PAGE,
#endif
	WEB,	"Domains",				"seconddomain",				offsetof(Prefs, stat_seconddomain),					SECONDDOMAIN_PAGE,
	WEB,	"Countries",			"country",					offsetof(Prefs, stat_country),						DOMAIN_PAGE,
	WEB,	"World Region",			"region",					offsetof(Prefs, stat_regions),						REGION_PAGE,
	WEB,	"Organizations",		"orgs",						offsetof(Prefs, stat_orgs),							ORGNAMES_PAGE,
#ifdef DEF_FULLVERSION																							
	WEB,	"Session Distribution",	"sessiondist",				offsetof(Prefs, stat_sessionScatter),				SESSIONS_PAGE,
#endif
#ifdef DEF_FULLVERSION
	WEB,	"Key Visitors",			"keyvisitors",				offsetof(Prefs, stat_keyvisitor),					KEYVISITORS_PAGE,
#endif
																												
	// ------------------------
	WEB,	">Referrals",			0,							offsetof(Prefs, stat_referrals),					REFERRALS_GROUP,
	WEB,	"Referral URLs",		"refer",					offsetof(Prefs, stat_refer),						REFERURL_PAGE,
	WEB,	"Referral Sites",		"refersite",				offsetof(Prefs, stat_refersite),					REFERSITE_PAGE,
	WEB,	"Referral Sites History","refersitehistory",		offsetof(Prefs, stat_refersiteHistory),				REFERSITEHIST_PAGE,
	WEB,	"Search Engines",		 "searchengine",			offsetof(Prefs, stat_searchsite),					SEARCHSITE_PAGE,
	WEB,	"Search Engines History","searchenginehistory",		offsetof(Prefs, stat_searchsiteHistory),			SEARCHSITEHIST_PAGE,
	WEB,	"Search Terms",			"search",					offsetof(Prefs, stat_searchstr),					SEARCHSTR_PAGE,
	WEB,	"Search Terms History",	"searchhistory",			offsetof(Prefs, stat_searchstrHistory),				SEARCHSTRHIST_PAGE,
																
	// ------------------------									
	WEB,	">Multi Media",			0,							offsetof(Prefs, stat_multimedia),					MULTIMEDIA_GROUP,
	WEB,	"Media Players",		"mplayers",					offsetof(Prefs, stat_mplayers),						MPLAYERS_PAGE,
	WEB,	"Media Players History","mplayershistory",			offsetof(Prefs, stat_mplayersHistory),				MPLAYERSHIST_PAGE,
	WEB,	"Audio Content",		"audiostreams",				offsetof(Prefs, stat_audio),						AUDIO_PAGE,
	WEB,	"Audio Content History","audiostreamshistory",		offsetof(Prefs, stat_audioHistory),					AUDIOHIST_PAGE,
	WEB,	"Video Content",		"videostreams",				offsetof(Prefs, stat_video),						VIDEO_PAGE,
	WEB,	"Video Content History","videostreamshistory",		offsetof(Prefs, stat_videoHistory),					VIDEOHIST_PAGE,
	WEB,	"Media Types",			"mediatypes",				offsetof(Prefs, stat_mediatypes),					MEDIATYPES_PAGE,
																
	// ------------------------									
	WEB,	">Systems",				0,							offsetof(Prefs, stat_systems),						SYSTEMS_GROUP,
	WEB,	"Spiders/Robots",		"robots",					offsetof(Prefs, stat_robot),						ROBOT_PAGE,
	WEB,	"Spiders/Robots History","robotshistory",			offsetof(Prefs, stat_robotHistory),					ROBOTHIST_PAGE,
	WEB,	"Browsers",				"browser",					offsetof(Prefs, stat_browsers),						BROWSER_PAGE,
	WEB,	"Browsers History",		"browserhistory",			offsetof(Prefs, stat_browsersHistory),				BROWSERHIST_PAGE,
	WEB,	"Browsers/Operating Systems","browsersvsos",		offsetof(Prefs, stat_browserVSos),					BROWSEROS_PAGE,
	WEB,	"Operating Systems",	"opersys",					offsetof(Prefs, stat_opersys),						OPERSYS_PAGE,
	WEB,	"Operating Systems History","opersyshistory",		offsetof(Prefs, stat_opersysHistory),				OPERSYSHIST_PAGE,
#ifdef DEF_FULLVERSION
	WEB,	"Unrecognized Agents",	"unrecognizedagents",		offsetof(Prefs, stat_unrecognizedagents),			UNRECOGNIZEDAGENTS_PAGE,
#endif
																													
	// ------------------------									
#ifdef DEF_FULLVERSION																							
	WEB,	">Advertising",			0,							offsetof(Prefs, stat_advertizing),					ADVERTISING_GROUP, 
	WEB,	"Impressions",			"advert",					offsetof(Prefs, stat_advert),						ADVERT_PAGE,
	WEB,	"Impressions History",	"adverthistory",			offsetof(Prefs, stat_advertHistory),				ADVERTHIST_PAGE,
	WEB,	"Campaigns",			"campaigns",				offsetof(Prefs, stat_advertcamp),					ADVERTCAMP_PAGE,
	WEB,	"Campaigns History",	"campaignshistory",			offsetof(Prefs, stat_advertcampHistory),			ADVERTCAMPHIST_PAGE,
#endif
																													
	// *****************************
	// Billing
	// *****************************
	WEB,	">Billing",				0,							offsetof(Prefs, stat_billing),						BILLING_GROUP, 
	WEB,	"Billing Customers",	"billing",					offsetof(Prefs, stat_billingCustomers),				BILLING_PAGE,

	// ------------------------									
	WEB,	">Marketing",			0,							offsetof(Prefs, stat_marketing),					MARKETING_GROUP,
#ifdef DEF_FULLVERSION										
	WEB,	"Circulation",			"circulation",				offsetof(Prefs, stat_circulation),					CIRC_PAGE,
#endif
	WEB,	"Loyalty",				"loyalty",					offsetof(Prefs, stat_loyalty),						LOYALTY_PAGE,
	WEB,	"Time Online",			"timeonline",				offsetof(Prefs, stat_timeon),						TIMEON_PAGE,
#ifdef DEF_FULLVERSION										
	WEB,	">Clustering",			0,							offsetof(Prefs, stat_cluster),						CLUSTER_GROUP,
	WEB,	"Load Balancing",			"clusters",				offsetof(Prefs, stat_clusterServers),				CLUSTER_PAGE,
	WEB,	"Load Balancing History",	"clustershistory",		offsetof(Prefs, stat_clusterServersHistory),		CLUSTERHIST_PAGE,
#endif


	// ########################## STREAMING STATS/TOKENS/CONFIG/ID
	
	STREAM,	">Streaming Summary",			0,							offsetof(Prefs, stat_summary),						0,
	STREAM,	"Processing Statistics",		"summary",					offsetof(Prefs, stat_summary),						0,
																															
	// #############################		
	STREAM,	">Traffic",						0,							offsetof(Prefs, stat_traffic),						TRAFFIC_GROUP,
	STREAM,	"Hourly",						"hourly",					offsetof(Prefs, stat_hourly),						HOUR_PAGE,
	STREAM,	"Hourly History",				"hourlyhistory",			offsetof(Prefs, stat_hourlyHistory),				HOURHIST_PAGE,
	STREAM,	"Daily",						"daily",					offsetof(Prefs, stat_daily),						DATE_PAGE,
	STREAM,	"Recent Daily",					"recentdaily",				offsetof(Prefs, stat_recentdaily),					RECENTDATE_PAGE,
	STREAM,	"Weekly",						"weekly",					offsetof(Prefs, stat_weekly),						WEEK_PAGE,
	STREAM,	"Weekdays",						"weekdays",					offsetof(Prefs, stat_weekdays),						WEEKDAYS_PAGE,
	STREAM,	"Monthly",						"monthly",					offsetof(Prefs, stat_monthly),						MONTH_PAGE,
																															
	// #############################		
	STREAM,	">Diagnostic",					0,							offsetof(Prefs, stat_diagnostics),					DIAGNOSTIC_GROUP,
	STREAM,	"Server Errors",				"errors",					offsetof(Prefs, stat_errors),						ERRORS_PAGE,
	STREAM,	"Server Errors History",		"errorshistory",			offsetof(Prefs, stat_errorsHistory),				ERRORSHIST_PAGE,
	STREAM,	"Failed Hits",					"errorurl",					offsetof(Prefs, stat_errorurl),						ERRORURL_PAGE,
	STREAM,	"Failed Hits History",			"errorurlhistory",			offsetof(Prefs, stat_errorurlHistory),				ERRORURLHIST_PAGE,
	STREAM,	"Top Error Codes Per Clip",		"clipserrors",				offsetof(Prefs, stat_clipserrors),					CLIPSERRORS_PAGE,
	STREAM,	"Clips With Errors",			"clipsfailed",				offsetof(Prefs, stat_clipsfailed),					CLIPSFAILED_PAGE,
	STREAM,	"Clips With Errors History",	"clipsfailedhistory",		offsetof(Prefs, stat_clipsfailedHistory),			CLIPSFAILEDHIST_PAGE,
#ifdef DEF_FULLVERSION												
	STREAM,	"Web Pages Referencing"																								
			"Invalid Clips",				"brokenclips",				offsetof(Prefs, stat_brokenLinks),					CLIPSBROKENLINKS_PAGE,
	STREAM,	"Average Packet Loss",			"clipspacketloss",			offsetof(Prefs, stat_clipspacketloss),				CLIPSPACKETLOSS_PAGE,
	STREAM,	"Packet Loss vs TransferRate",	"clipslossvsrate",			offsetof(Prefs, stat_clipslossvsrate),				CLIPSLOSSRATE_PAGE,
	STREAM,	"Average Seconds Buffered",		"clipssecsbuffered",		offsetof(Prefs, stat_clipssecsbuffered),			CLIPSSECSBUF_PAGE,
	STREAM,	"Highest Quality Clips",		"clipshighquality",			offsetof(Prefs, stat_clipshighiquality),			CLIPSHIGH_PAGE,
	STREAM,	"Lowest Quality Clips",			"clipslowquality",			offsetof(Prefs, stat_clipslowquality),				CLIPSLOW_PAGE,

	
	// #############################		
	STREAM,	">Clip Access Information",			0,							offsetof(Prefs, stat_streaming),				STREAMINGCONTENT_GROUP,
	STREAM,	"Most Popular Video Clips",			"clipsvideo",				offsetof(Prefs, stat_clipsvideo),				CLIPSVID_PAGE,
	STREAM,	"Most Popular Video Clips History",	"clipsvideohistory",		offsetof(Prefs, stat_clipsvideoHistory),		CLIPSVIDHIST_PAGE,
	STREAM,	"Video Clip Viewing Times",			"clipsvideoviewing",		offsetof(Prefs, stat_clipsvideoviews),			CLIPSVIDVIEW_PAGE,
	STREAM,	"Video Clip Transfer Rates",		"clipsvideorates",			offsetof(Prefs, stat_clipsvideorates),			CLIPSVIDRATES_PAGE,
	STREAM,	"Most Popular Audio Clips",			"clipsaudio",				offsetof(Prefs, stat_clipsaudio),				CLIPSAUD_PAGE,	
	STREAM,	"Most Popular Audio Clips History",	"clipsaudiohistory",		offsetof(Prefs, stat_clipsaudioHistory),		CLIPSAUDHIST_PAGE,
	STREAM,	"Audio Clip Listening Times",		"clipsaudioviewing",		offsetof(Prefs, stat_clipsaudioviews),			CLIPSAUDVIEW_PAGE,
	STREAM,	"Audio Clip Transfer Rates",		"clipsaudiorates",			offsetof(Prefs, stat_clipsaudiorates),			CLIPSAUDRATES_PAGE,
												
	STREAM,	"Most Popular Clips",				"clips",					offsetof(Prefs, stat_clips),					CLIPS_PAGE,
	STREAM,	"Most Popular Clips History",		"clipshistory",				offsetof(Prefs, stat_clipsHistory),				CLIPSHIST_PAGE,	
	STREAM,	"Least Played Clips",				"clipsleast",				offsetof(Prefs, stat_clipsleast),				CLIPSLEAST_PAGE,
	STREAM,	"Percentage Streamed",				"clipspercent",				offsetof(Prefs, stat_clipspercent),				CLIPSPERCENT_PAGE,
	STREAM,	"Max Concurrent Connections",		"clipsmaxconcurrent",		offsetof(Prefs, stat_clipsmaxconcurrent),		CLIPSMAXCON_PAGE,
	STREAM,	"Average Forwards per Clip",		"clipsforwards",			offsetof(Prefs, stat_clipsforwards),			CLIPS_FF_PAGE,
	STREAM,	"Average Rewinds per Clip",			"clipsrewinds",				offsetof(Prefs, stat_clipsrewinds),				CLIPS_RW_PAGE,
	STREAM,	"Uninterrupted Played Clips",		"clipscompleted",			offsetof(Prefs, stat_clipscomplete),			CLIPSCOMPLETED_PAGE,
												
	STREAM,	"Protocols Used by Clips",			"protocols",				offsetof(Prefs, stat_protSummary),				CLIPSPROTOCOLS_PAGE,
	STREAM,	"Most Utilized Video Codecs",		"videocodecs",				offsetof(Prefs, stat_clipsvideocodecs),			VIDCODECS_PAGE,
	STREAM,	"Most Utilized Audio Codecs",		"audiocodecs",				offsetof(Prefs, stat_clipsaudiocodecs),			AUDCODECS_PAGE,
	STREAM,	"All File Types",					"filetype",					offsetof(Prefs, stat_type),						FILETYPE_PAGE,
	STREAM,	"Media Types",						"mediatypes",				offsetof(Prefs, stat_mediatypes),				MEDIATYPES_PAGE,
	STREAM,	"Pages",							"pages",					offsetof(Prefs, stat_pages),					PAGES_PAGE,
	STREAM,	"Pages History",					"pageshistory",				offsetof(Prefs, stat_pagesHistory),				PAGEHIST_PAGE,
	STREAM,	"Pages (Least Visited)",			"pagesleast",				offsetof(Prefs, stat_pagesLeastVisited),		PAGESLEAST_PAGE,
	STREAM,	"Directories",						"dir",						offsetof(Prefs, stat_dir),						DIRS_PAGE,
	STREAM,	"Top Level Directories",			"topdir",					offsetof(Prefs, stat_topdir),					TOPDIRS_PAGE,
	STREAM,	"Top Level Directories History",	"topdirhistory",			offsetof(Prefs, stat_topdirHistory),			TOPDIRSHIST_PAGE,
	STREAM,	"URLs",								"file",						offsetof(Prefs, stat_files),					FILE_PAGE,
	STREAM,	"Content Groups",					"groups",					offsetof(Prefs, stat_groups),					GROUPS_PAGE,
	STREAM,	"Content Groups History",			"groupshistory",			offsetof(Prefs, stat_groupsHistory),			GROUPSHIST_PAGE,
#ifdef DEF_DEBUG									
//	STREAM,	"DEBUG - All Clips Details",		"debugclips",				offsetof(Prefs, stat_clips),					DEBUGPAGES_PAGE,
#endif
																
	// #############################  LIVE		
	STREAM,	">Live Clip Access Information",	0,							offsetof(Prefs, stat_livestreaming),			STREAMINGCONTENT_GROUP,
	STREAM,	"Most Popular Live Video",			"livevideo",				offsetof(Prefs, stat_livevideo),				LIVEVID_PAGE,
	STREAM,	"Most Popular Live Video History",	"livevideohistory",			offsetof(Prefs, stat_livevideoHistory),			LIVEVIDHIST_PAGE,
	STREAM,	"Live Video Viewing Times",			"livevideoviewing",			offsetof(Prefs, stat_livevideoviews),			LIVEVIDVIEW_PAGE,
	STREAM,	"Live Video Transfer Rates",		"livevideorates",			offsetof(Prefs, stat_livevideorates),			LIVEVIDRATES_PAGE,
	STREAM,	"Most Popular Live Audio Clips",		"liveaudio",			offsetof(Prefs, stat_liveaudio),				LIVEAUD_PAGE,	
	STREAM,	"Most Popular Live Audio Clips History","liveaudiohistory",		offsetof(Prefs, stat_liveaudioHistory),			LIVEAUDHIST_PAGE,
	STREAM,	"Live Audio Clip Listening Times",	"liveaudioviewing",			offsetof(Prefs, stat_liveaudioviews),			LIVEAUDVIEW_PAGE,
	STREAM,	"Live Audio Clip Transfer Rates",	"liveaudiorates",			offsetof(Prefs, stat_liveaudiorates),			LIVEAUDRATES_PAGE,
#endif											

	
	// #############################
	STREAM,	">Demographic",			0,									offsetof(Prefs, stat_demographics),					DEMOGRAPHIC_GROUP,
	STREAM,	"Visitors",						"client",					offsetof(Prefs, stat_client),						CLIENT_PAGE,
	STREAM,	"Visitors History",				"clienthistory",			offsetof(Prefs, stat_clientHistory),				CLIENTHIST_PAGE,
#ifdef DEF_FULLVERSION											
	STREAM,	"Visitors Click Stream",		"clientstream",				offsetof(Prefs, stat_clientStream),					CLIENTSTREAM_PAGE,
#endif										
	STREAM,	"Authenticated Users",			"users",					offsetof(Prefs, stat_user),							USER_PAGE,
	STREAM,	"Authenticated Users History",	"usershistory",				offsetof(Prefs, stat_userHistory),					USERHIST_PAGE,
#ifdef DEF_FULLVERSION										
	STREAM,	"Authenticated Users Click Stream","usersstream",			offsetof(Prefs, stat_userStream),					USERSTREAM_PAGE,
#endif
	STREAM,	"Domains",						"seconddomain",				offsetof(Prefs, stat_seconddomain),					SECONDDOMAIN_PAGE,
	STREAM,	"Countries",					"country",					offsetof(Prefs, stat_country),						DOMAIN_PAGE,
	STREAM,	"World Region",					"region",					offsetof(Prefs, stat_regions),						REGION_PAGE,
	STREAM,	"Organizations",				"orgs",						offsetof(Prefs, stat_orgs),							ORGNAMES_PAGE,
#ifdef DEF_FULLVERSION																									
	STREAM,	"Session Distribution",			"sessiondist",				offsetof(Prefs, stat_sessionScatter),				SESSIONS_PAGE,
#endif										
#ifdef DEF_FULLVERSION					
	STREAM,	"Key Visitors",					"keyvisitors",				offsetof(Prefs, stat_keyvisitor),					KEYVISITORS_PAGE,
#endif
																												
	// #############################
	STREAM,	">Referrals",					0,							offsetof(Prefs, stat_referrals),					REFERRALS_GROUP,
	STREAM,	"Referral URLs",				"refer",					offsetof(Prefs, stat_refer),						REFERURL_PAGE,
	STREAM,	"Referral Sites",				"refersite",				offsetof(Prefs, stat_refersite),					REFERSITE_PAGE,
	STREAM,	"Referral Sites History"		,"refersitehistory",		offsetof(Prefs, stat_refersiteHistory),				REFERSITEHIST_PAGE,
																		
	// #############################		
	STREAM,	">Systems",						0,							offsetof(Prefs, stat_systems),						SYSTEMS_GROUP,
	STREAM,	"Media Players Used",			"mplayers",					offsetof(Prefs, stat_mplayers),						MPLAYERS_PAGE,
	STREAM,	"Media Players Used History",	"mplayershistory",			offsetof(Prefs, stat_mplayersHistory),				MPLAYERSHIST_PAGE,
	STREAM,	"Players/Operating Systems",	"mplayersvsos",				offsetof(Prefs, stat_mplayersVSos),					MPLAYERSOS_PAGE,
	STREAM,	"Most Popular CPU Types",		"cpus",						offsetof(Prefs, stat_cpu),							CPU_PAGE,
	STREAM,	"Most Popular Languages",		"langs",					offsetof(Prefs, stat_lang),							LANG_PAGE,
											
	STREAM,	"Browsers",						"browser",					offsetof(Prefs, stat_browsers),						BROWSER_PAGE,
	STREAM,	"Browsers History",				"browserhistory",			offsetof(Prefs, stat_browsersHistory),				BROWSERHIST_PAGE,
	STREAM,	"Browsers/Operating Systems",	"browsersvsos",				offsetof(Prefs, stat_browserVSos),					BROWSEROS_PAGE,
	STREAM,	"Operating Systems",			"opersys",					offsetof(Prefs, stat_opersys),						OPERSYS_PAGE,
	STREAM,	"Operating Systems History",	"opersyshistory",			offsetof(Prefs, stat_opersysHistory),				OPERSYSHIST_PAGE,
	STREAM,	"Spiders/Robots",				"robots",					offsetof(Prefs, stat_robot),						ROBOT_PAGE,
	STREAM,	"Spiders/Robots History",		"robotshistory",			offsetof(Prefs, stat_robotHistory),					ROBOTHIST_PAGE,
#ifdef DEF_FULLVERSION																							
	STREAM,	"Unrecognized Agents",			"unrecognizedagents",		offsetof(Prefs, stat_unrecognizedagents),			UNRECOGNIZEDAGENTS_PAGE,
#endif										
																															
	// ------------------------											
#ifdef DEF_FULLVERSION																									
	STREAM,	">Advertising",					0,							offsetof(Prefs, stat_advertizing),					ADVERTISING_GROUP, 
	STREAM,	"Impressions",					"advert",					offsetof(Prefs, stat_advert),						ADVERT_PAGE,
	STREAM,	"Impressions History",			"adverthistory",			offsetof(Prefs, stat_advertHistory),				ADVERTHIST_PAGE,
	STREAM,	"Campaigns",					"campaigns",				offsetof(Prefs, stat_advertcamp),					ADVERTCAMP_PAGE,
	STREAM,	"Campaigns History",			"campaignshistory",			offsetof(Prefs, stat_advertcampHistory),			ADVERTCAMPHIST_PAGE,
#endif										
																															
	// ------------------------											
	STREAM,	">Marketing",					0,							offsetof(Prefs, stat_marketing),					MARKETING_GROUP,
#ifdef DEF_FULLVERSION												
	STREAM,	"Circulation",					"circulation",				offsetof(Prefs, stat_circulation),					CIRC_PAGE,
#endif										
	STREAM,	"Loyalty",						"loyalty",					offsetof(Prefs, stat_loyalty),						LOYALTY_PAGE,
	STREAM,	"Time Online",					"timeonline",				offsetof(Prefs, stat_timeon),						TIMEON_PAGE,	
	
	
	
	
	
	
// ----------- FWA FIREWALL -------------																			
#ifdef DEF_APP_FIREWALL																								
	FIRE,	">Firewall",			0,							offsetof(Prefs, stat_firewall),						FIREWALL_GROUP,
	FIRE,	"Source Address",		"sourceaddresses",			offsetof(Prefs, stat_sourceaddress),				SRCADDR_PAGE,
	FIRE,	"Protocol Summary",		"protocols",				offsetof(Prefs, stat_protSummary),					PROTSUMMARY_PAGE,
	FIRE,	"HTTP Protocol",		"prot-http",				offsetof(Prefs, stat_protHTTP),						PROTHTTP_PAGE,
	FIRE,	"HTTPS Protocol",		"prot-https",				offsetof(Prefs, stat_protHTTPS),					PROTHTTPS_PAGE,
	FIRE,	"Mail Protocol",		"prot-mail",				offsetof(Prefs, stat_protMail),						PROTMAIL_PAGE,
	FIRE,	"FTP Protocol",			"prot-ftp",					offsetof(Prefs, stat_protFTP),						PROTFTP_PAGE,
	FIRE,	"Telnet Protocol",		"prot-telnet",				offsetof(Prefs, stat_protTelnet),					PROTTELNET_PAGE,
	FIRE,	"DNS Protocol",			"prot-dns",					offsetof(Prefs, stat_protDNS),						PROTDNS_PAGE,
	FIRE,	"POP3 Protocol",		"prot-pop3",				offsetof(Prefs, stat_protPOP3),						PROTPOP3_PAGE,
	FIRE,	"Real Protocol",		"prot-real",				offsetof(Prefs, stat_protReal),						PROTREAL_PAGE,
	FIRE,	"Other Protocols",		"prot-other",				offsetof(Prefs, stat_protOthers),					PROTOTHERS_PAGE,
#endif

	
	0,0,0,0,																										0
};																												











#define GETDATAPTR(x)	( ((char*)pPrefs) + (long)(stat_confignames[x].data) );


// Lots of GUI code depends HIGHLY on the specs of these.
// What goes in/out is specific, it cannot be changed.

void SaveStat_Settings( Prefs *pPrefs, FILE *fp )
{
	long i = 0, *data, def=0;
	char *name;

	Fprintf( fp , "\n# +stat {name} {tableon} {graphon} {sorting} {tablesize}\n#\n" );

	while( (name = stat_confignames[i].title) )
	{
		// Only save the settings for which STYLE is active at the moment.
		if ( ConfigInStyle( pPrefs, i ) )
		{
			if ( *name != '>' )
			{
				data = (long*)GETDATAPTR(i);
				def = *data;
				if ( REPORT_CHECK_ON(def) )
					Fprintf( fp , "+stat " );
				else
					Fprintf( fp , "-stat " );
				Fprintf( fp , "%s ", stat_confignames[i].name );
				if ( TABLE(def) )
					Fprintf( fp , "tableon " );
				else
					Fprintf( fp , "tableoff " );
				if ( GRAPH(def) )
					Fprintf( fp , "graphon " );
				else
					Fprintf( fp , "graphoff " );
				Fprintf( fp , "%d %d ", TABSORT(def), TABSIZE(def) );

				if (COMMENT (def))						// help text on?
					Fprintf (fp, "helpon");
				else
					Fprintf (fp, "helpoff");
				Fprintf (fp, "\n");
			} else
				Fprintf( fp, "# %s\n", name );
		}
		i++;
	}
}

int CheckConfigStyle( long option, long stat_style )
{
	int rc = FALSE;

	switch( stat_style )
	{
		case ALL:		rc=TRUE; break;
		case WEB:		if ( IS_WEB(option) ) rc=TRUE; break;
		case PROXY:		if ( IS_PROXY(option) ) rc=TRUE; break;
		case STREAM:	if ( IS_STREAM(option) ) rc=TRUE; break;
		case FIRE:		if ( IS_FIRE(option) ) rc=TRUE; break;
	}
	return rc;
}

int ConfigInStyle( Prefs *pPrefs, long item )
{
	return CheckConfigStyle( stat_confignames[item].style, pPrefs->stat_style );
}



// Switch ON the visible style options, while turning off invisible ones
void SetAll_DataInStyle( Prefs *pPrefs, long def )
{
	long i = 0, *data;

	while( stat_confignames[i].title )
	{
		data = (long*)GETDATAPTR(i);

		if ( data && stat_confignames[i].name )
		{
			if ( ConfigInStyle( pPrefs, i ) ){
				*data = def;
#ifdef DEF_DEBUG
				OutDebugs( "%s reseting to %08lx", stat_confignames[i].name,def );
#endif
			}
		}
		i++;
	}
}

void SetAll_Sorting( Prefs *pPrefs, short sort )
{
	long i = 0, *data, def=0;

	while( stat_confignames[i].title ){
		data = (long*)GETDATAPTR(i);

		if ( data && stat_confignames[i].name ){
			def = *data;
			STATSETSORT(def,sort);
			*data = def;
		}
		i++;
	}
}


void SetAllHistories_OnOff( Prefs *pPrefs, long status )
{
	long i = 0, *data, def=0;

	while( stat_confignames[i].title && stat_confignames[i].name )
	{
		if ( mystrstr( stat_confignames[i].name, "history" ) )
		{
			data = (long*)GETDATAPTR(i);

			if ( data ){
				def = *data;
				STATSET(def,status);
				*data = def;
			}
		}
		i++;
	}
}

void SetAll_Tablesize( Prefs *pPrefs, short ts )
{
	long i = 0, *data, def=0;

	while( stat_confignames[i].title ){
		data = (long*)GETDATAPTR(i);
		if ( data && stat_confignames[i].name ){
			def = *data;
			STATSETTAB(def,ts);
			*data = def;
		}
		i++;
	}
}

void SetAll_GraphMode( Prefs *pPrefs, short gm )
{
	long i = 0, *data, def=0;

	while( stat_confignames[i].title )
	{
		data = (long*)GETDATAPTR(i);
		if ( data && stat_confignames[i].name )
		{
			def = *data;
			STATSETG( def, gm );
			*data = def;
		}
		i++;
	}
}


void SetAll_Data( Prefs *pPrefs, long def )
{
	long i = 0, *data;

	while( stat_confignames[i].title ){
		data = (long*)GETDATAPTR(i);
		if ( data && stat_confignames[i].name )
			*data = def;
		i++;
	}
}

void SetGroup_Data( Prefs *pPrefs, long item, long def )
{
	long i = item, *data;

	i++;

	while( stat_confignames[i].title && stat_confignames[i].name ){
		data = (long*)GETDATAPTR(i);
		if ( data && stat_confignames[i].name )
			*data = def;
		i++;
	}
}


char *ConfigSetSettings( Prefs *pPrefs, long item, long newdata )
{
	long i = item, *data;

	if ( stat_confignames[i].title )
	{
		data = (long*)GETDATAPTR(i);

		if ( data )
			*data = newdata;
		return stat_confignames[i].title;
	}
	return NULL;
}

void ConfigSettings_ClearReportStatus( Prefs *pPrefs )
{
	long i = 0, *data, def=0;

	while( stat_confignames[i].title )
	{
		data = (long*)GETDATAPTR(i);
		if ( data && stat_confignames[i].name )
		{
			def = *data;		// read it
			STATSETDONE(def,0);
			*data = def;		// store it
		}
		i++;
	}
}

void ConfigSetStatOff( Prefs *pPrefs, char *name )
{
	long index;
	long data;

	index = ConfigNameFindSettings( pPrefs, name, &data );
	if ( index >= 0){
		STATSET(data,0);
		ConfigSetSettings( pPrefs, index, data );
	}
}


void ConfigSetSort( Prefs *pPrefs, char *name, long sorting )
{
	long index;
	long data;

	index = ConfigNameFindSettings( pPrefs, name, &data );
	if ( index >= 0){
		STATSETSORT(data,sorting);
		ConfigSetSettings( pPrefs, index, data );
	}
}



void FillDefaults( Prefs *pPrefs )
{
	long i = 0, def=7;

	STATSETSORT(def,SORT_REQUESTS);
	STATSETTAB(def,4);
	STATSETC(def, 1);		// Turn on the HelpCard for ALL reports as the default.

	SetAll_Data( pPrefs, 0 );
	SetAll_DataInStyle( pPrefs, def );

	ConfigSetSort( pPrefs, "keyvisitors", SORT_PAGES );

	ConfigSetSort( pPrefs, "circulation", SORT_COUNTER );
	ConfigSetSort( pPrefs, "loyalty", SORT_COUNTER );
	ConfigSetSort( pPrefs, "timeonline", SORT_COUNTER );

	pPrefs->stat_clusterServers = DEFAULT_CLUSTEROPTIONS;
	pPrefs->stat_clusterServersHistory = DEFAULT_CLUSTEROPTIONS;
}







// ------------- GET values routines ---------------

const char*
ConfigLookupFilename(unsigned long item)
{
	return FindReportFilenameStr( stat_confignames[item].id );
	//return stat_confignames[item].szFileName;
}
const char*
ConfigLookupTitle(unsigned long item)
{
	return stat_confignames[item].title;
}
const char*
ConfigLookupName(unsigned long item)
{
	return stat_confignames[item].name;
}

// Return stat settings options...
char *ConfigFindSettings( Prefs *pPrefs, long item, long *dataout )
{
	long *data;

	if ( stat_confignames[item].title ){
		data = (long*)GETDATAPTR(item);

		if (stat_confignames[item].data)
			*dataout = *data;
		else
			*dataout = 0;
		return stat_confignames[item].title;
	}
	return NULL;
}


long ConfigFindIndex( long id, long style_mask )
{
static	long i = 0;

	// check last index.
	if ( stat_confignames[i].id == id )
		return i;
	else
		i = 0;

	while( stat_confignames[i].title )
	{
		if ( stat_confignames[i].title[0] != '>'  && CheckConfigStyle( stat_confignames[i].style, style_mask ) )
		{
			if ( stat_confignames[i].id == id )
			{
				return i;
			}
		}
		i++;
	}
	i = 0; // reset offset
	return -1;
}


long ConfigNameFindIndex( char *name, long style_mask )
{
	long i = 0;

	while( stat_confignames[i].title )
	{
		if ( stat_confignames[i].title[0] != '>' && CheckConfigStyle( stat_confignames[i].style, style_mask ) )
		{
			if ( stat_confignames[i].name && !mystrcmpi( stat_confignames[i].name, name ) )
			{
				return i;
			}
		}
		i++;
	}
	return -1;
}

long ConfigGroupNameFindIndex( Prefs *pPrefs, char *groupName )
{
	long i = 0;

	while( stat_confignames[i].title ){
		// Only for TITLE objects that are in current mode (web/streaming/firewall)
		if ( stat_confignames[i].title[0] == '>' && ConfigInStyle( pPrefs, i ) )
		{
			if ( !mystrcmpi( stat_confignames[i].title+1, groupName ) )
			{
				return i;
			}
		}
		i++;
	}
	return -1;
}

// ########



long ConfigNameFindSettings( Prefs *pPrefs, char *name, long *dataout )
{
	long i, *data;

	i = ConfigNameFindIndex( name, pPrefs->stat_style );

	if ( i>= 0 )
	{
		data = (long*)GETDATAPTR(i);

		if ( data && dataout )
			*dataout = *data;
		return i;
	}
	return -1;
}


long ConfigGetSettingsbyStyle( Prefs *pPrefs, long id, long style )
{
	long i, *data;

	i = ConfigFindIndex( id, style );

	if ( i>= 0 )
	{
		data = (long*)GETDATAPTR(i);
		if ( data )
			return *data;
	}
	return NULL;
}



// Return settings flags pointer not based on any style....
long *ConfigFindSettingsPtrbyStyle( Prefs *pPrefs, long id, long style  )
{
	long i, *data;

	i = ConfigFindIndex( id, style );

	if ( i>= 0 )
	{
		data = (long*)GETDATAPTR(i);
		return data;
	}
	return NULL;
}
long *ConfigFindSettingsPtr( Prefs *pPrefs, long id )
{
	return ConfigFindSettingsPtrbyStyle( pPrefs, id, pPrefs->stat_style );
}




const char *ConfigFindTitle( Prefs *pPrefs, long id )
{
	long i;

	i = ConfigFindIndex( id, pPrefs->stat_style );

	if ( i>= 0 )
	{
		return ConfigLookupTitle( i );
	}
	return NULL;
}

char *ConfigFindName( Prefs *pPrefs, long id )
{
	long i = 0;

	i = ConfigFindIndex( id, pPrefs->stat_style );

	if ( i>= 0 && ConfigInStyle( pPrefs, i ) )
	{
		return stat_confignames[i].name;
	}
	return NULL;
}

const char *ConfigNameFindFilename( Prefs *pPrefs, char *name )
{
	long i = 0;

	i = ConfigNameFindIndex( name, pPrefs->stat_style );

	if ( i>= 0 )
	{
		return ConfigLookupFilename( i );
		//return stat_confignames[i].szFileName;
	}
	return NULL;
}

long ConfigNameFindID( char *name )
{
	long i = 0;

	i = ConfigNameFindIndex( name, ALL );

	if ( i>= 0 )
	{
		return stat_confignames[i].id;
	}
	return -1;
}






// ---------------- determine if what group or specific set of options are on or off ----------------











// find out if a group report is turned on using its name
int IsGroupReportNameOn( Prefs *pPrefs, char *name )
{
	long i;
	long *data;
	int status=0;

	i = ConfigGroupNameFindIndex( pPrefs, name );

	if ( i>=0 )
	{
		i++;
		while( stat_confignames[i].title && stat_confignames[i].name )
		{
			data = (long*)GETDATAPTR(i);
			if ( REPORT_TOBE_GENERATED(*data) )
				status++;
			i++;
		}
	}
	return status;
}

// find out if a group report was written using its string name
int IsGroupReportNameCompleted( Prefs *pPrefs, char *name )
{
	long i;
	long *data;
	int status=0;


	i = ConfigGroupNameFindIndex( pPrefs, name );

	if ( i>=0 )
	{
		i++;
		while( stat_confignames[i].title && stat_confignames[i].name )
		{
			data = (long*)GETDATAPTR(i);
			if ( REPORT_WAS_GENERATED(*data) && REPORT_TOBE_GENERATED(*data))
				status++;
			i++;
		}
	}
	return status;
}





// find out if a group report is turned on using its _PAGE id
int GroupReportOn( Prefs *pPrefs, long id )
{
	long i;
	long *data;
	int status=0;

	i = ConfigFindIndex( id, pPrefs->stat_style );

	if ( i>=0 )
	{
		i++;
		while( stat_confignames[i].title && stat_confignames[i].name )
		{
			data = (long*)GETDATAPTR(i);
			if ( REPORT_TOBE_GENERATED(*data) )
				status++;
			i++;
		}
	}
	return status;
}

// find out if a group report was written using its _PAGE id
int GroupReportDone( Prefs *pPrefs, long id )
{
	long i;
	long *data;
	int status=0;

	i = ConfigFindIndex( id, pPrefs->stat_style );

	if ( i>=0 )
	{
		i++;
		while( stat_confignames[i].title && stat_confignames[i].name )
		{
			data = (long*)GETDATAPTR(i);
			if ( REPORT_WAS_GENERATED(*data) && REPORT_TOBE_GENERATED(*data))
				status++;
			i++;
		}
	}
	return status;
}


int ReportCommentOn( long id )
{
	long *dataP;
	dataP = ConfigFindSettingsPtr( &MyPrefStruct, id );
	return ( dataP && COMMENT(*dataP) );
}

// Use this instead of the old direct way of  -  if ( STAT(MyPrefStruct.stat_seconddomain) ) )
int ReportDone( long id )
{
	long *dataP;
	dataP = ConfigFindSettingsPtr( &MyPrefStruct, id );
	return ( dataP && REPORT_WAS_GENERATED(*dataP) && REPORT_TOBE_GENERATED(*dataP) );
}

// Use this instead of the old direct way of  -  if ( STAT(MyPrefStruct.stat_seconddomain) ) )
int ReportOn( long id )
{
	long *dataP;
	dataP = ConfigFindSettingsPtr( &MyPrefStruct, id );
	return ( dataP && REPORT_CHECK_ON(*dataP) );
}

int ReportsOn( long *ids )
{
	long ret = 0;
	while ( ids && *ids )
	{
		ret += ReportOn( *ids );
		ids++;
	}
	return ret;
}


// find out if a report is turned on using its name
int IsReportNameOn( Prefs *pPrefs, char *name )
{
	long i, data;
	i = ConfigNameFindSettings( pPrefs, name, &data );
	if ( (i!=-1) && REPORT_TOBE_GENERATED(data) )
		return i;
	else
		return 0;
}

// find out if a report was written using its name
int IsReportNameCompleted( Prefs *pPrefs, char *name )
{
	long i,data;
	i = ConfigNameFindSettings( pPrefs, name, &data );
	if ( (i!=-1) && REPORT_WAS_GENERATED(data) && REPORT_TOBE_GENERATED(data) )
		return i;
	else
		return 0;
}












// A nicer cleaner way to figure out if each group is on.
int AnyTrafficStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Traffic" );
}
int AnyDiagnosticStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Diagnostic" );
}
int AnyServerStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Server" );
}
int AnyDemographicStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Demographic" );
}
int AnyReferralStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Referrals" );
}
int AnyMultiMediaStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Multi Media" );
}
int AnyStreamingMediaStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Streaming Content" );
}
int AnySystemsStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Systems" );
}
int AnyAdvertisingStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Advertising" );
}
int AnyMarketingStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Marketing" );
}
int AnyClustersStatsOn( Prefs * config )
{
	if ( config->clusteringActive ){
		return 1;
	}
	else
		return 0;
}
int AnyFirewallStatsOn( Prefs * config )
{
	return IsGroupReportNameOn( config, "Firewall" );
}





// ----------------------------------- GRAPH DEFINITIONS -----------------------------------------


#define	MAKERGB(r,g,b)			((r<<16) | (g<<8) | (b))

#define	DEFAULT_3D_BG		MAKERGB( 198,198,204 )
#define	DEFAULT_3D_H		MAKERGB( 155, 155, 155)
#define	DEFAULT_3D_M		MAKERGB( 255, 255, 255)
#define	DEFAULT_3D_S		(0)

#define	DEFAULT_SCAT_BG		MAKERGB( 0,0,95 )
#define	DEFAULT_SCAT_H		MAKERGB( 0,255,153)
#define	DEFAULT_SCAT_M		MAKERGB( 0,0,102)
#define	DEFAULT_SCAT_S		MAKERGB( 0,142,81)

#define	DEFAULT_PIE_BG		MAKERGB( 0,0,119 )
#define	DEFAULT_PIE_H		MAKERGB( 204,204,255 )
#define	DEFAULT_PIE_M		MAKERGB( 153,153,255 )
#define	DEFAULT_PIE_S		MAKERGB( 51,51,102 )

#define	DEFAULT_LINE_BG		MAKERGB( 102,153,255)
#define	DEFAULT_LINE_H		0x000000
#define	DEFAULT_LINE_M		0xFF0000
#define	DEFAULT_LINE_S		0xFFFF00

#define	DEFAULT_VBAR_BG		MAKERGB( 119,0,0 )
#define	DEFAULT_VBAR_H		MAKERGB( 255,153,153 )
#define	DEFAULT_VBAR_M		MAKERGB( 204,102,102 )
#define	DEFAULT_VBAR_S		MAKERGB( 153,0,0 )

#define	DEFAULT_HBAR_BG		MAKERGB( 10,125,110 )
#define	DEFAULT_HBAR_H		MAKERGB( 203,214,231 )
#define	DEFAULT_HBAR_M		MAKERGB( 131,174,226 ) 
#define	DEFAULT_HBAR_S		MAKERGB( 13,45,155 )   

//	ID						DTYLE,				bar highlight, bar main, bar shadow,		background color
GraphDataStyles graph_data_default[] = {
	0,						GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG
};

GraphDataStyles graph_data_defaults[] = {
	VHOST_PAGE,				GRAPH_MULTIVHOST3D,	DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	HOUR_PAGE,				GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 140,140,240 ),
	HOURHIST_PAGE,			GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	DATE_PAGE,				GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 90,140,240 ),
	RECENTDATE_PAGE,		GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 90,140,240 ),
	WEEK_PAGE,				GRAPH_VBAR,			DEFAULT_VBAR_H,	DEFAULT_VBAR_M,	DEFAULT_VBAR_S,	MAKERGB( 119,0,0 ),
	SUNDAY_PAGE,			GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 153,153,255 ),
	MONDAY_PAGE,			GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 153,153,255 ),
	TUEDAY_PAGE,			GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 153,153,255 ),
	WEDDAY_PAGE,			GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 153,153,255 ),
	THUDAY_PAGE,			GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 153,153,255 ),
	FRIDAY_PAGE,			GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 153,153,255 ),
	SATDAY_PAGE,			GRAPH_LINE,			DEFAULT_LINE_H,	DEFAULT_LINE_M,	DEFAULT_LINE_S,	MAKERGB( 153,153,255 ),
	MONTH_PAGE,				GRAPH_VBAR,			DEFAULT_VBAR_H,	DEFAULT_VBAR_M,	DEFAULT_VBAR_S,	MAKERGB( 119,0,0 ),
							
	ERRORS_PAGE,			GRAPH_PIE,			DEFAULT_PIE_H,	DEFAULT_PIE_M,	DEFAULT_PIE_S,	DEFAULT_PIE_BG,
	ERRORSHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	ERRORURL_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	ERRORURLHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	BROKENLINKS_PAGE,		GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	EXTBROKENLINKS_PAGE,	GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	INTBROKENLINKS_PAGE,	GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	CLIPSFAILED_PAGE,		GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	CLIPSFAILEDHIST_PAGE,	GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,

	CLIPSVID_PAGE,			GRAPH_HBAR,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	CLIPSVIDHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	CLIPSAUD_PAGE,			GRAPH_HBAR,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	CLIPSAUDHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	CLIPS_PAGE,				GRAPH_HBAR,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	CLIPSHIST_PAGE,			GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	CLIPSLEAST_PAGE,		GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	CLIPSPROTOCOLS_PAGE,	GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	VIDCODECS_PAGE,			GRAPH_PIE,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	AUDCODECS_PAGE,			GRAPH_PIE,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
						
							
	PAGES_PAGE,				GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PAGEHIST_PAGE,			GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	PAGESLEAST_PAGE,		GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PAGESFIRST_PAGE,		GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PAGESFIRSTHIST_PAGE,	GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	PAGESLAST_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PAGESLASTHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	REDIRECTS_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	DIRS_PAGE,				GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	TOPDIRS_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	TOPDIRSHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	FILE_PAGE,				GRAPH_HBAR,			MAKERGB( 204,255,255), MAKERGB( 102,204,204),MAKERGB( 0,51,51), MAKERGB( 0,102,102 ),
	MEANPATH_PAGE,			GRAPH_HBAR,			MAKERGB( 204,204,255), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	KEYPAGEROUTE_PAGE,		GRAPH_HBAR,			MAKERGB( 204,204,255), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	KEYPAGEROUTEFROM_PAGE,	GRAPH_HBAR,			MAKERGB( 204,204,255), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	GROUPS_PAGE,			GRAPH_HBAR,			MAKERGB( 51,245,193), MAKERGB( 0,184,122), MAKERGB( 51,132,91), MAKERGB( 0,100,81 ),
	GROUPSHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	UPLOAD_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	UPLOADHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	DOWNLOAD_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	DOWNLOADHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	FILETYPE_PAGE,			GRAPH_PIE,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	MEDIATYPES_PAGE,		GRAPH_PIE,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
						
	AUDIO_PAGE,				GRAPH_HBAR,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	AUDIOHIST_PAGE,			GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	VIDEO_PAGE,				GRAPH_HBAR,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	VIDEOHIST_PAGE,			GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,

	REFERURL_PAGE,			GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	REFERSITE_PAGE,			GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	REFERSITEHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	SEARCHSITE_PAGE,		GRAPH_HBAR,			MAKERGB( 51,153,255), MAKERGB( 0,102,204), MAKERGB( 51,81,142), MAKERGB( 0,51,102 ),
	SEARCHSITEHIST_PAGE,	GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	SEARCHSTR_PAGE,			GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	SEARCHSTRHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
							
	ADVERT_PAGE,			GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	ADVERTHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	ADVERTCAMP_PAGE,		GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	ADVERTCAMPHIST_PAGE,	GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
						
	BILLING_PAGE,			GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),

	CLIENT_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	CLIENTSTREAM_PAGE,		GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	CLIENTHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	USER_PAGE,				GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	USERHIST_PAGE,			GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	USERSTREAM_PAGE,		GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	SECONDDOMAIN_PAGE,		GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	DOMAIN_PAGE,			GRAPH_HBAR,			MAKERGB( 255,255,102), MAKERGB( 204,204,51), MAKERGB( 153,153,0), MAKERGB( 102,102,0 ),
	REGION_PAGE,			GRAPH_WORLD,		MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
	ORGNAMES_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	SESSIONS_PAGE,			GRAPH_SCATTER,		DEFAULT_SCAT_H, DEFAULT_SCAT_M, DEFAULT_SCAT_S, DEFAULT_SCAT_BG,
	KEYVISITORS_PAGE,		GRAPH_HBAR,			MAKERGB( 51,153,255), MAKERGB( 0,102,204), MAKERGB( 51,81,142), MAKERGB( 0,51,102 ),
							
	CPU_PAGE,				GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	LANG_PAGE,				GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	ROBOT_PAGE,				GRAPH_HBAR,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	ROBOTHIST_PAGE,			GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	MPLAYERS_PAGE,			GRAPH_HBAR,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	MPLAYERSHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	MPLAYERSOS_PAGE,		GRAPH_MULTIVBAR4	,MAKERGB( 204,204,255), MAKERGB( 153,153,255), MAKERGB( 51,51,102), MAKERGB( 0,0,119 ),
	BROWSER_PAGE,			GRAPH_HBAR,			MAKERGB( 204,204,255), MAKERGB( 153,153,255), MAKERGB( 51,51,102), MAKERGB( 0,0,119 ),
	BROWSERHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	BROWSEROS_PAGE,			GRAPH_MULTIVBAR4,	MAKERGB( 204,204,255), MAKERGB( 153,153,255), MAKERGB( 51,51,102), MAKERGB( 0,0,119 ),
	OPERSYS_PAGE,			GRAPH_HBAR,			MAKERGB( 204,204,255), MAKERGB( 153,153,255), MAKERGB( 51,51,102), MAKERGB( 0,0,119 ),
	OPERSYSHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	UNRECOGNIZEDAGENTS_PAGE,GRAPH_HBAR,			MAKERGB( 204,204,255), MAKERGB( 153,153,255), MAKERGB( 51,51,102), MAKERGB( 0,0,119 ),

	CIRC_PAGE,				GRAPH_HBAR,			MAKERGB( 255,153,102), MAKERGB( 204,102,51), MAKERGB( 153,51,0), MAKERGB( 0,102,153 ),
	LOYALTY_PAGE,			GRAPH_HBAR,			MAKERGB( 102,204,102), MAKERGB( 0,153,102), MAKERGB( 0,102,51), MAKERGB( 0,102,153 ),
	TIMEON_PAGE,			GRAPH_HBAR,			MAKERGB( 255,255,153), MAKERGB( 255,204,51), MAKERGB( 204,153,0), MAKERGB( 153,153,51 ),
							
	SRCADDR_PAGE,			GRAPH_HBAR,			MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	PROTHTTP_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PROTHTTPS_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PROTMAIL_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PROTFTP_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PROTTELNET_PAGE,		GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PROTDNS_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PROTPOP3_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PROTREAL_PAGE,			GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
	PROTOTHERS_PAGE,		GRAPH_HBAR,			DEFAULT_HBAR_H,	DEFAULT_HBAR_M,	DEFAULT_HBAR_S,	DEFAULT_HBAR_BG,
							
	// Pie graph cluster	, OFF at the moment.
	//CLUSTER_PAGE,			GRAPH_PIE, MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	CLUSTER_PAGE,			GRAPH_VBAR, MAKERGB( 170,200,230), MAKERGB( 60,95,190), MAKERGB( 20,30,60), MAKERGB( 30,60,90 ),
	CLUSTERHIST_PAGE,		GRAPH_MULTI3D,		DEFAULT_3D_H,	DEFAULT_3D_M,	DEFAULT_3D_S,	DEFAULT_3D_BG,
	-1,0,0,0,0,0,
 };


 
GraphDataStyles graph_data_settings[TOTALREPORTS_PAGE];

// Fill in graph_data_settings[] with default colours for all graphs to be used
// and populate with the specified defaults in graph_data_defaults[]
int GraphData_InitDefaults( void )
{
	long i, id;

	i = 0;
	for ( i=0; i<TOTALREPORTS_PAGE; i++)
	{
		graph_data_settings[i].id = i;
		graph_data_settings[i].style = GRAPH_HBAR;
		graph_data_settings[i].col1 = DEFAULT_HBAR_H;
		graph_data_settings[i].col2 = DEFAULT_HBAR_M;
		graph_data_settings[i].col3 = DEFAULT_HBAR_S;
		graph_data_settings[i].bcol = DEFAULT_HBAR_BG;
	}

	i = 0;
	while ( graph_data_defaults[i].id != -1 )
	{
		id = graph_data_defaults[i].id;
		graph_data_settings[id] = graph_data_defaults[i];
		i++;
	}
	return i;
}



int SaveGraphsSettings( FILE *fp )
{
	long i;
	GraphDataStylesP item;
	char *name;

	Fprintf( fp, "\n# These are the colors for each reports graph.\n" );
	Fprintf( fp, "#graphcolor NAME BarColors-light,mid,shadow, background, graph-style(V-bars,H-Bars,Line,Pie-Charts,3D-History,3D-VBar)\n" );

	i = 0;
	while ( graph_data_defaults[i].id != -1 ) 
	{
		item = &graph_data_defaults[i];
		if ( item->col1 != -1 ) 
		{
			name = ConfigFindName( &MyPrefStruct, item->id );
			if ( name )
				Fprintf( fp, "graphcolor %s %06x %06x %06x %06x %d\n", name, item->col1, item->col2, item->col3, item->bcol, item->style );
		}
		i++;
	}

	Fprintf( fp, "\n# These are the colors for each graphs type in this order.\n" );
	Fprintf( fp, "# V-bars,H-Bars,Line,Pie-Charts,3D-History,3D-VBar,Scatter-Chart\n" );
	Fprintf( fp, "# graphtypecolor num label grid base borderhi borderlo border\n" );
	for(i=0; i<GRAPHCOLORS_MAX; i++)
	{
		Fprintf( fp, "graphtypecolor %d %06x %06x %06x %06x %06x %06x\n", i+1,
			MyPrefStruct.graphcolors[i].label,
			MyPrefStruct.graphcolors[i].grid,
			MyPrefStruct.graphcolors[i].base,
			MyPrefStruct.graphcolors[i].border_hilight,
			MyPrefStruct.graphcolors[i].border_lolight,
			MyPrefStruct.graphcolors[i].border
			);
	}
	
	
	return i;
}


// Return the graph item colors, if not found, return a default set.
GraphDataStylesP FindGraphTypeData( long id )
{

/*	if ( id < TOTALREPORTS_PAGE )
	{
		if ( graph_data_settings[id].id == id )
			return &graph_data_settings[id];
	}
*/
	long i = 0;
	while ( graph_data_settings[i].id != -1)
	{
		if ( graph_data_settings[i].id == id )
			return &graph_data_settings[i];
		i++;
	}
	graph_data_default[0].id = id;
	return graph_data_default;
}


//graphcolor vhost 9b9b9b ffffff 000000 c6c6cc
int ReadGraphColors( char **argv, long count )
{
	long id = 1;
	GraphDataStylesP item;

	id = ConfigNameFindID( argv[1] );

	if ( id >= 0 )
	{
		item = FindGraphTypeData( id );
		if ( item ){
			item->col1 = HexStr2int( argv[2] );
			item->col2 = HexStr2int( argv[3] );
			item->col3 = HexStr2int( argv[4] );
			item->bcol = HexStr2int( argv[5] );
			if ( count > 6 )
				item->style = HexStr2int( argv[6] );
		}
	}
	return id;
}



int ReadGraphTypeColors( char **argv, long count )
{
	long id = 0;

	id = myatoi( argv[1] );

	if ( id ){
		id--;
		MyPrefStruct.graphcolors[id].label= HexStr2int( argv[2] );
		MyPrefStruct.graphcolors[id].grid= HexStr2int( argv[3] );
		MyPrefStruct.graphcolors[id].base= HexStr2int( argv[4] );
		MyPrefStruct.graphcolors[id].border_hilight= HexStr2int( argv[5] );
		MyPrefStruct.graphcolors[id].border_lolight= HexStr2int( argv[6] );
		MyPrefStruct.graphcolors[id].border= HexStr2int( argv[7] );
	}
	return id;
}
// ---------------------------------------------------------------------------------------


 

void CommaToMultiline( char *string, char *dest, long max )
{
	char *p, *d;
	long len = 0,size=0;

	d = dest;
	while( string && size<max){
		p = mystrchr( string, ',' );
		if ( len>1 )
			d += sprintf( d, "\r\n" );

		if( p ){
			*p = 0;
			len = mystrlen( string );
			if ( len>1 )
				d += sprintf( d, "%s", string );
			*p++ = ',';
		} else {
			len = mystrlen( string );
			mystrcpy( d, string );
		}
		size += (len+3);
		string = p;
	}
}

void MultilineToComma( char *string, char *dest, long max )
{
	char *p;
	long len = 0,size=0;
	while( string ){
		p = mystrchr( string, '\r' );
		if ( len > 2 )
			dest += sprintf( dest, "," );

		if( p ){
			*p++ = 0;
			*p++ = 0;
			len = mystrlen( string );
			if ( len > 2 )
				dest += sprintf( dest, "%s", string );
		} else {
			len = mystrlen( string );
			mystrcpy( dest, string );
		}
		string = p;
	}
}



int myfgets( char *str, long max, FILE *fp )
{
	int c; int count = 0;

	while( count<max && (c=fgetc( fp )) != -1){
		if ( c == 13 || c == 10 )
			c = 0;
		str[count] = c;
		count++;
		if ( !c ) break;
	}
	return count;
}

long IsPrefsFile(char *prefsname)
{
	FILE *fp;
	long	result = 0;

	if( strstr( prefsname, ".fwp" ) )
	{
		result = 1;
	} else
	if ( (fp = fopen( prefsname, "r" ))) 
	{
		char	tempdata[128];
        result = fread(tempdata, 1, 127, fp);              // get info
		if ( !strncmp( tempdata, "APP", 4 ) || (tempdata[0] == '#' && strstr( tempdata, "settings" )) )
		{
			result = 1;
		} else
			result = 0;
        fclose(fp);
	}
    return result;  // let them know how long the file is (can only be shorter, of course)
}


static size_t
find_next(const char* sz, bool (*cmp)(const char))
{
	const char* s;
	for (s = sz; *s; ++s)
	{
		if (*s == '\\')
			++s;
		else if (cmp(*s))
			break;
	}

	return s-sz;
}

static size_t
strip_escapes(char* sz)
{
	char* szTo			= sz;
	const char* szFrom	= sz;

	while (*szFrom)
	{
		if (*szFrom == '\\')
			++szFrom;
		*szTo++ = *szFrom++;
	}
	*szTo = 0;

	return szFrom - sz;
}

static bool	isspacebar(const char c)	{ return c == ' '; }
static bool	isquotes(const char c)		{ return c == '"'; }

void
MakeArgs(char *line, char **argv, int &argc)
{
	size_t	n(0);
	
	argc = 0;

	while(line[n])
	{
		if (isquotes(line[n]))
		{
			++line;
			argv[argc++] = line+n;
			n = find_next(line+n, isquotes);
		}
		else
		{
			argv[argc++] = line+n;
			n = find_next(line+n, isspacebar);
		}

		if (!line[n])
			break;

		line[n++] = 0;
		line += n;
		while(isspacebar(line[0]))
			++line;

		n = 0;
//		(void)strip_escapes(argv[argc-1]);
	}

	// Need to do this on last arg also!
//	(void)strip_escapes(argv[argc-1]);
}


static	int doing_htmlhead = 0,
			doing_htmlfoot = 0,
			doing_editpath = 0;

long DoReadAsciiPrefs( char *filename, Prefs *pPrefs )
{
 	FILE *fp;
 	char	line[4096], line2[4096];
 	char	*argv[256] , *p;
 	int 	argc, count = 0, linenum=0;
 
 	if ( fp = fopen( filename, "r" ) )
 	{
		// assume we may get an old settings file
		pPrefs->prefVER = OBSOLETE_PREF_VERSION;
		
		doing_htmlhead = doing_htmlfoot = 0;

		pPrefs->filterdata.orgTot = 0;
		pPrefs->filterdata.org[0][0] = 0;

		while( !feof( fp ) )
		{
		 	myfgets( line, 4095, fp );
			trimLineWhite( line );		// trims trailing spaces, tabs, returns & newlines
			if( *line!=0 )				// allow for empty lines
			{
				if( linenum == 0 && line[0] != '#' )
				{
					sprintf( line, "The Report Settings file (%s) is not valid\n", filename );
	 				ErrorMsg( line );
	 				return FALSE;
				}

				mystrcpy( line2, line );

				linenum++;

		 		p = line; argc=0; count=0;
		 		mymemset( argv, 0, 255*sizeof(void*) );
				MakeArgs(line, argv, argc);
//		 		while( (p=strtok( p, " " )) && argc<256 )
//				{
//		 			argv[ argc++ ] = p;
//		 			p=NULL;
//		 		}
		 		ProcessPrefsLine( argc, argv, line2, 0, pPrefs );
			}
		}
		if (pPrefs->prefVER < CURRENT_PREF_VERSION)
		{
			// do cleanup of old settings file here
			// NotifyMsg ("Old settings file!");
			// fix dates
		}
		doing_htmlhead = doing_htmlfoot = 0;
		GetHTMLColors( pPrefs );
	}
	return linenum;
}





long DoAddPrefsFile( char *filename, Prefs *pPrefs )
{
 	FILE *fp;
 	char	line[4096], line2[4096];
 	char	*argv[256] , *p;
 	int 	argc, count = 0, linenum=0;
 
 	if ( fp = fopen( filename, "r" ) )
 	{
		doing_htmlhead = doing_htmlfoot = 0;
		while( !feof( fp ) )
		{
		 	myfgets( line, 4095, fp );
			trimLineWhite( line );		// trims trailing spaces, tabs, returns & newlines
			if( *line!=0 )				// allow for empty lines
			{
				if( linenum == 0 && strncmp( line, "#Fun", 4 ) )
				{
					sprintf( line, "report prefs %s is not valid\n", filename );
	 				ErrorMsg( line );
	 				return FALSE;
				}

				mystrcpy( line2, line );

				linenum++;

		 		p = line; argc=0; count=0;
		 		mymemset( argv, 0, 255*sizeof(void*) );
				MakeArgs(line, argv, argc);
//		 		while( (p=strtok( p, " " )) && argc<256 )
//				{
//		 			argv[ argc++ ] = p;
//		 			p=NULL;
//		 		}
		 		ProcessPrefsLine( argc, argv, line2, 0, pPrefs );
			}
		}
		doing_htmlhead = doing_htmlfoot = 0;
	}
	return linenum;
}





static long ScaleRGB( double perc,long rgb )
{
	long r,g,b;

	r = (rgb & 0xff0000) >> 16;
	g = (rgb & 0xff00) >> 8;
	b = (rgb & 0xff);

	r = (long)(r * perc / 100.0);
	if ( r < 0 ) r = 0;
	if ( r > 255 ) r = 255;

	g = (long)(g * perc / 100.0);
	if ( g < 0 ) g = 0;
	if ( g > 255 ) g = 255;

	b = (long)(b * perc / 100.0);
	if ( b < 0 ) b = 0;
	if ( b > 255 ) b = 255;

	return ( r<<16 | g<<8 | b );
}


void CreateDefaultRGBTable( Prefs *pPrefs )
{
	long	defaultRGBtable[] = { '-RGB', 0x999999, 0xcccccc, 0xf4f4f4, 0xcccccc, 0x99cccc, 0x99cccc,
									0xffffff, 0x000000, 0x0000ff, 0xff0000, 0xff0088, -1};
	long	defaultmulti_rgb[] = { 0xff0033, 0xff8C00, 0xffcc33, 0x00cc33, 0x339999, 0x00ffff, 0x008Cff, 0x9999ff, 0xcc66cc,0xff99cc };

	long	i = 0;

	while( defaultRGBtable[i] != -1 ){
		pPrefs->RGBtable[i] = defaultRGBtable[i];
		i++;
	}

	for( i=0; i<10; i++){
		pPrefs->multi_rgb[i] = defaultmulti_rgb[i];
		pPrefs->multi_rgb[i+10] = ScaleRGB( 50,defaultmulti_rgb[i] );
	}
}

#define	DEFAULT_TEXTCOLOR		0x0
#define	DEFAULT_GRIDCOLOR		0xDDDDDD
#define	DEFAULT_BASECOLOR		0xF0F0F0
#define	DEFAULT_BORDERHI		0xFFFFFF
#define	DEFAULT_BORDERLO		0x555555
#define	DEFAULT_BORDER			0x000000

void CreateDefaultGraphColorSet( GraphColorsPtr g )
{
	g->label = DEFAULT_TEXTCOLOR;
	g->grid = DEFAULT_GRIDCOLOR;
	g->base = DEFAULT_BASECOLOR;
	g->border_hilight = DEFAULT_BORDERHI;
	g->border_lolight = DEFAULT_BORDERLO;
	g->border = DEFAULT_BORDER;
}

void CreateDefaultGraphcolors( Prefs *pPrefs )
{
	for (long i=0; i<GRAPHCOLORS_MAX; i++){
		CreateDefaultGraphColorSet( &pPrefs->graphcolors[i] );
	}

	MyPrefStruct.graphcolors[GRAPHCOLORS_MULTI3D].grid = 0x6E6E6E;
	MyPrefStruct.graphcolors[GRAPHCOLORS_SCATTER].grid = 0x141478;
}

#ifdef DEF_MAC
char html_head[] =
{
"<body bgcolor=\"#ffffff\" text=\"#000000\" link=\"#0000ff\" vlink=\"#ff0000\" alink=\"#ff0088\">\r"
"<p align=\"center\">\r"
"<img src=\"logo.gif\" align=\"bottom\" border=\"0\" alt=\"banner\">\r"
"</p>\r\r"
};

char html_foot[] =
{
	"<p align=\"center\">\r\r"
	"</p>\r</body>\r</html>\r"
};
#else

// IMPORTANT!!! - For windows & Unix, we need a \n after every \r so that the GUI edit control puts the text on another line!
char html_head[]={
"<BODY bgcolor=#ffffff text=#000000 link=#0000ff alink=#ff0088 vlink=#ff0000>\r\n"
"<P ALIGN=CENTER>\r\n"
"\r\n"
"<IMG SRC=\"logo.gif\" ALIGN=\"BOTTOM\" border=0></A>\r\n<BR>\r\n"
"</P>\r\n"
};
char html_foot[]={
	"<P ALIGN=CENTER>\r\n\r\n"
	"</BODY></HTML>\r\n"
};

#endif



static TopLevelDomains topLevelDomains[MAX_FILTERSIZE] =
{
	"*.org",	ORGS_US_ORGANISATIONS,
	"*.org.*",	ORGS_ORGANISATIONS,
	"*.edu",	ORGS_US_EDUCATION,
	"*.edu.*",	ORGS_EDUCATION,
	"*.mil",	ORGS_US_MILITARY,
	"*.mil.*",	ORGS_MILITARY,
	"*.gov",	ORGS_US_GOVERNMENT,
	"*.gov.*",	ORGS_GOVERNMENT,
	"*.com",	ORGS_US_COMMERCIAL,
	"*.com.*",	ORGS_COMMERCIAL,
	"*.co.*",	ORGS_COMMERCIAL,
	"*.net",	ORGS_US_NETWORK,
	"*.net.*",	ORGS_NETWORK,
	"*proxy*",	ORGS_PROXY_SERVER,
	"*robot*",	ORGS_SEARCH_ROBOTS,

/*  New TLDS to be added */
	"*.biz",	COUN_BUSINESSES,	
	"*.name",	COUN_INDIVIDUALS,	
	"*.museum",	COUN_MUSEUMS,		
	"*.pro",	COUN_PROFESSIONSALS,
	"*.aero",	COUN_AVIATION,		
	"*.coop",	COUN_COOPERATIVES,	
	"*.info",	COUN_GNERALINFO,	
	0,			0
};

const char *TopLevelDomainName( long i )
{
	return topLevelDomains[i].tldName;
}

const char *TopLevelDomainDesc( long i )
{
	return TranslateID( topLevelDomains[i].descId );
}

const char *TopLevelDomainString( long i )
{
	static char buf[128];
	mystrcpy( buf, TopLevelDomainName(i) );
	mystrcat( buf, "=" );
	mystrcat( buf, TopLevelDomainDesc(i) );
	return buf;
}

void TopLevelDomainStrings( Prefs *pPrefs )
{
	int i = 0;

	pPrefs->filterdata.orgTot = 0;
	*pPrefs->filterdata.org[0] = 0;
	while( TopLevelDomainName(i) )
	{
		if (i >= MAX_FILTERUNITS)
			break;
		mystrcpy( pPrefs->filterdata.org[i], TopLevelDomainString(i) );
		pPrefs->filterdata.orgTot++;
		i++;
	}
}


void CryptPasswd( char *ps )
{
	long n;

	while( *ps ){
		n = *ps;
		n = 127 - n;
		n = 32 + n;
		*ps = (char)n;
		ps++;
	}
}


// This character is prepended to ftp passwords when they written
// out in an ecrypted state - which now is always.  This is required to be able
// to differentiate between older non-encrypted passwords and newer encrypted ones.
static const char encryptedIndicator(':');

//------------------------------------------------------------------------------------------
// Encrypt the specified password and convert to a printable state suitable for storage
// within a settings file.  Returns the result.
//------------------------------------------------------------------------------------------
static std::string EncryptPassword( const char* passWord )
{
	// encrypt the actual password
	std::string encrPasswd( IceEncryptStr( passWord ) );
	
	// convert bytes in encrypted password to a format that's printable and suitable for
	// storage within a text file - each character represented as a 2 digit hex number
	std::string printEncrPasswd;
	printEncrPasswd.reserve( encrPasswd.size()*2);	// hex representation requires twice space
	
	printEncrPasswd+=encryptedIndicator;		// prepend a colon to indicate that password is encrypted
	
	// for each encrypted password character...
	for( std::string::const_iterator it( encrPasswd.begin() ); it!=encrPasswd.end(); ++it )
	{
		// ...determine the hex version of the character
		char hexbuf[3];  // ie "FF"
		sprintf( hexbuf, "%02x", static_cast<unsigned char>(*it) );  // don't remove the cast!

		// append to our final string
		printEncrPasswd+=hexbuf;
	}

	return printEncrPasswd;
}


//------------------------------------------------------------------------------------------
// Dencrypt the specified password after converting it from its existing printable state.
// Returns the result. 
//------------------------------------------------------------------------------------------
static std::string DecryptPassword( const char* passWord )
{
	std::string origPasswd;

	// if password has been encrypted
	if( *passWord==encryptedIndicator )
	{
		// the encrypted password is currently in a printable state (where each
		// encrypted character is represented as a 2 digit hex number) and needs to be
		// converted back to its encrypted state
		size_t length( strlen(passWord)-1 );	// don't count encryptedIndicator

		std::string encrPasswd;
		encrPasswd.reserve( length/2 );

		// for each 2 digit hex number (starting after encryptedIndicator!)...
		for( size_t pos(1); pos<length; pos+=2 )
		{
			// ...convert back to original encrypted character
			unsigned char c; unsigned long l;
			sscanf( passWord+pos, "%02x", &l );

			c = l;

			// append original enccrypted character to string
			encrPasswd+=c;
		}

		// decrypt the password
		// Note: Must pass in length as encrypted password may actually contain embedded nulls.
		origPasswd=IceDecryptStr( encrPasswd.c_str(), encrPasswd.length() );
	}
	// else password hasn't been encrypted so just return passWord intact
	else
	{
		origPasswd=passWord;
	}

	return origPasswd;
}


//------------------------------------------------------------------------------------------
// Encrypt the password portion of the specified <ftpLocation>.  The resulting encrypted
// password is in a (printable) state that may be written out to a text file.  The function
// returns the resulting ftp location containing the encrypted password.
//------------------------------------------------------------------------------------------
static std::string EncryptFTPLocationPassword( const char* ftpLocation )
{
	// Sorry, these magic numbers for array sizes are used all over the place
	// in conjunction with the ExtractUserFromURL() function!  The function
	// doesn't check for overflow anyway....
	char server[128];
	char name[128];
	char passwd[128];
	char path[256];

	// break ftp location url into constituents
	ExtractUserFromURL( (char*)ftpLocation, server, name, passwd, path );

	// encrypt password component
	std::string printableEncrPasswd( EncryptPassword(passwd) );

	// reconstitute the ftp location url
	char encryptedFTPLocation[ 512 ]="ftp://";
	MakeURLfromDetails( encryptedFTPLocation, server, name, const_cast<char*>(printableEncrPasswd.c_str()), path );

	return std::string( encryptedFTPLocation );
}


//------------------------------------------------------------------------------------------
// Decrypt the password portion of the specified <ftpLocation>.  The function returns the
// resulting ftp location containing the decrypted password.
//------------------------------------------------------------------------------------------
static std::string DecryptFTPLocationPassword( const char* ftpLocation )
{
	// Sorry, these magic numbers for array sizes are used all over the place
	// in conjunction with the ExtractUserFromURL() function!  The function
	// doesn't check for overflow anyway....
	char server[128];
	char name[128];
	char passwd[128];
	char path[256];

	// break ftp location url into constituents
	ExtractUserFromURL( (char*)ftpLocation, server, name, passwd, path );

	// decrypt password component
	std::string origPasswd( DecryptPassword(passwd) );

	// reconstitute the ftp location url
	char decryptedFTPLocation[ 512 ]="ftp://";
	MakeURLfromDetails( decryptedFTPLocation, server, name, const_cast<char*>(origPasswd.c_str()), path );

	return std::string( decryptedFTPLocation );
}


void ClearConfig( Prefs *pPrefs )
{
	// Clean up the linked list.
	BillingCustomerStruct*	p = pPrefs->billingCharges.pFirstCustomer;
	if (p)
	{
		while (p=p->pNext)
		{
			delete pPrefs->billingCharges.pFirstCustomer;
			pPrefs->billingCharges.pFirstCustomer = p;
		}

		delete pPrefs->billingCharges.pFirstCustomer;
	}

	ClearEditPath( &pPrefs->EditPaths );
	mymemset( &pPrefs->filterdata, 0, sizeof( struct FilterData ) );
}

#define	DEFAULT_FONT		"tahoma,verdana,helvetica"


//
// Create the default settings
//
//
void CreateDefaults( Prefs *pPrefs, long deforgs, int stat_style )
{
	struct tm nowtime;
	time_t ct;

	mymemset( pPrefs, 0, CONFIG_SIZE );

	// This initializes the Fast10000 lookups
	//TimeStringToDays( "01/Jan/1990", &ct );  //oldway
	InitFast10000();

	GetCurrentDate( &nowtime );

	Date2Days( &nowtime, &ct);

/*
	long i;
	char str[MAXFILENAMESSIZE];
#ifdef DEF_MAC
	i = 1;
#else
	i = GetDateStyle();
#endif

	DaysDateToString( ct, str, i, '/', 1,1);
	mystrcpy( pPrefs->date1, str );

	DaysDateToString( ct, str, i, '/', 1,1);
	mystrcpy( pPrefs->date2, str );
*/
	pPrefs->startTimeT = pPrefs->endTimeT = ct;

	ClearEditPath( &pPrefs->EditPaths );

	mystrcpy( pPrefs->prefID, "APP" );		// make sure our Settings have an ID so we can be sure it is the PREF

	pPrefs->prefVER = CURRENT_PREF_VERSION;	// this is 5, as we can't put "4.5" into a short 
	pPrefs->alldates = DATE_ALL;
	pPrefs->sortby = SORT_REQUESTS;			// SORT_NAMES = 0,or SORT_SIZES = 1,or SORT_REQUESTS = 2
	pPrefs->dnslookup = 0;
	pPrefs->dnsAmount = 50;					// was 100, changed to 50 for 4.0.3 release RHF 31/05/01
	pPrefs->dnr_ttl = 45;
	pPrefs->dnr_expire = 90;
	pPrefs->graph_type = TYPE_REQUESTS;		// BYTES = 0, REQUESTS = 1
	pPrefs->graph_style = GRAPH_3DSTYLE;
	pPrefs->filter_zerobyte = FALSE;
	pPrefs->dollarpermeg = 0;		// Dollar * 1000 scaled
	pPrefs->session_timewindow = 10;		// 5 mins
	pPrefs->write_timestat = 1;
	pPrefs->footer_label = 0;			// Default to no-label at footer
	pPrefs->footer_label_trailing = 1;	// This is only relevant if footer_label os is true.
	pPrefs->head_title = 1;
	pPrefs->bandwidth = 1;
	pPrefs->useCGI = 0;
	pPrefs->stat_end = -1;
	pPrefs->headingOnLeft = 1;
	pPrefs->sessionLimit = 1000;
	pPrefs->theme = 0;				// initialise to Default theme
	pPrefs->stat_style = stat_style;

	PDFAllSettingsCInit( &pPrefs->pdfAllSetC );

	FillDefaults( pPrefs );


#if DEF_UNIX
	pPrefs->graph_ascii = TRUE;
#endif

	pPrefs->stat_sessionHistory = TRUE;

	CreateDefaultRGBTable( pPrefs );
	CreateDefaultGraphcolors( pPrefs );
	GraphData_InitDefaults();

	mystrcpy( pPrefs->html_head, html_head );
	mystrcpy( pPrefs->html_foot, html_foot );

#ifdef DEF_MAC
	mystrcpy (pPrefs->preproc_downloc, gPath);
	mystrcat (pPrefs->preproc_downloc, "Logs:");

	pPrefs->postproc_lcompressor = kGzip;
	pPrefs->postproc_rcompressor = kStuffIt;
#elif DEF_WINDOWS
	pPrefs->postproc_lcompressor = kGzip;
	pPrefs->postproc_rcompressor = kZip;
#else
	pPrefs->postproc_lcompressor = kUnknown;	// what are defaults for Unix?
	pPrefs->postproc_rcompressor = kUnknown;
#endif
	
	mystrcpy( pPrefs->remotelogin_passwd, "test" );
	mystrcpy( pPrefs->html_font, "default" );
	pPrefs->html_fontsize = 2;

	
	pPrefs->remotelogin_port = 911;
	pPrefs->remotelogin_enable = 0;

	pPrefs->stat_summary = 0xFFFFFFFF;
#if DEF_UNIX
	sprintf( pPrefs->outfile, "report/index.html" );
	//sprintf( pPrefs->dnr_cache_file, "%s/dnr.cache", gPath );
	char *home = getenv( "HOME" );
	sprintf( pPrefs->dnr_cache_file, "%s/.fwa45.dnr.cache", home );
#endif
#if DEF_WINDOWS
	sprintf( pPrefs->outfile, "%s\\Report\\index.html", gPath );
	sprintf( pPrefs->dnr_cache_file, "%s\\dnr.cache", gPath );			// default dns cache is EXE relative now
#endif
#ifdef DEF_MAC
	InitReportPath (pPrefs);
	mystrcpy( pPrefs->dnr_cache_file, "dnr.cache" );
#endif

	sprintf (pPrefs->language, ENGLISH_LANG );
	if ( deforgs )
	{
		InitLanguage( pPrefs->language, 0 );
		TopLevelDomainStrings( pPrefs );
	}
	
	DefaultExtensions( pPrefs );

	pPrefs->nToKeyPages				= 0;
	pPrefs->nToKeyPageMaxRows		= 10;
	pPrefs->nToKeyPageRouteDepth	= 6;

	pPrefs->nFromKeyPages			= 0;
	pPrefs->nFromKeyPageMaxRows		= 10;
	pPrefs->nFromKeyPageRouteDepth	= 6;

	pPrefs->billingCharges.nCustomers = 0;
	pPrefs->billingCharges.pFirstCustomer = NULL;
	memset(&pPrefs->billingSetup, 0, sizeof(pPrefs->billingSetup) );
	pPrefs->billingSetup.ucBillingPeriod = 2;	// Month(s)
	pPrefs->billingSetup.bHeaderCustomerName	= true;
	pPrefs->billingSetup.bHeaderCustomerID		= true;
	pPrefs->billingSetup.bHeaderMBTransfered	= true;
	pPrefs->billingSetup.bHeaderTotalCharges	= true;
}


void WizardDefaults( Prefs *pPrefs, long type )
{

}


void CopyPrefs( Prefs *dest, Prefs *source )
{
	RemoveEditPath( &dest->EditPaths );
	memcpy( dest , source,  CONFIG_SIZE );
	dest->EditPaths = NULL;
	CopyEditPath( &dest->EditPaths, &source->EditPaths );
}



/* 
	read the prefs in from the file

	This is the full routine that reads in the preferences file no matter what version and
	what items are missing from it, it will try load as best as it can....


  */
long DoReadPrefsFile(char *prefsname, Prefs *pPrefs )
{
	FILE	*fp;
	long	result=0, readin = FALSE;
	char	tempdata[MAXFILENAMESSIZE];

	if ( (fp = fopen( prefsname, "rb" ))) {
        fread(tempdata, 1, 8, fp);              // get info
		if ( !strncmp( tempdata, "#Fun", 4 ) || tempdata[0]=='#'  ){
			fclose( fp );
			CreateDefaults( pPrefs, 1 );
			result = DoReadAsciiPrefs( prefsname, pPrefs );
			//TopLevelDomainStrings( pPrefs );
		}
		// No more BINARY reads supported, only ascii reading

		if ( result ){
			if ( pPrefs->RGBtable[0] != '-RGB' )
				CreateDefaultRGBTable( pPrefs );					// make a default RGB palette if one doesnt exist

			if ( !strcmpd( "tp://",pPrefs->siteurl ) ){
				mystrcpy( tempdata, pPrefs->siteurl );
				sprintf( pPrefs->siteurl, "ht%s", tempdata );
			}

			if ( pPrefs->session_timewindow == 0 )
				pPrefs->session_timewindow = 10;

		}

        fclose(fp);
	}
    return result;  // let them know how long the file is (can only be shorter, of course)
}

char *JDtoStr( double jd )
{
static	char aStr[32];

	DaysDateToString( (long)jd, aStr,-1,'/',TRUE,TRUE);
	return aStr;

}

void WriteMultiLine( FILE *fp , char *text )
{
	char *p,c;

	p = text;

	while( (c=*p) ){
		if ( c=='\r' ) Fprintf( fp, "\\n" );
		else
		if ( c=='\n' ) ;//Fprintf( fp, "\\n" );
		else Fprintf( fp, "%c", c );
		p++;
	}
}

void ReadMultiLineFormat( char *dest , char *src )
{
	char *p,c;

	p = src;

	while( (c=*p++) ){
		if ( c=='\\' ) {
			c = *p++;
			if ( c=='n' )
				c = '\n';
		}
		if( c=='\n' ) *dest++ = '\r';
		*dest++ = c;
	}
}

void WriteFlag( FILE *fp, int flag, char *name )
{
	if ( flag )
		Fprintf( fp, "%s\n", name );
	else
		Fprintf( fp, "# Option Off: %s\n", name );
}

void WriteValue( FILE *fp, int value, char *name )
{
	if ( value )
		Fprintf( fp, "%s %d\n", name, value );
}

void WriteString( FILE *fp, const char *value, const char *name )
{
	if ( value )
	{
		// Save a string with in quotes, if it has spaces in it so that it is preserved properly and not lost.
		//if ( strchr( value, ' ' ) )
		//Fprintf( fp, "%s \"%s\"\n", name, value );
		Fprintf( fp, "%s %s\n", name, value );
	}
}






// write a string of "value\0value\0value" into "value,value,value"
void WriteGroupSet( FILE *fp, char *name, char *data )
{
	char *p;
	Fprintf( fp, "%s %s", name, data );

	p = strchr( data, 0 );
	if ( p )
		p++;
	else
		return;

	Fprintf( fp, "," );
	if ( *p )	Fprintf( fp, "%s", p );

	p = strchr( p, 0 );
	if ( p )
		p++;
	else
		return;

	Fprintf( fp, "," );
	if ( *p )	Fprintf( fp, "%s", p );

	Fprintf( fp, "\n" );
}

// input is a string, "VAR,VAR,VAR" or "VAR=VAR"
// all = and , get NULLed, extra 0 added at the end.
void ReadGroupSet( char *line, char *arg, char *dest )
{
	char *pt;

	if ( line )
	{
		pt = mystrchr( line, ' ' );
		pt++;
		trimLine( pt );
	} else
		pt = arg;

	if ( pt )
	{
		mystrcpy( dest, pt );
		pt = dest;
		while( *pt )
		{
			if ( *pt == '=' ) *pt = 0;
			if ( *pt == ',' ) *pt = 0;
			pt++;
		}
		pt++;
		*pt = 0;
	}
}

void
SaveSettingsBillingSetup(FILE* fp, const BillingSetupStruct* pSetup)
{
	Fprintf( fp, "billingsetup_enabled %d\n",			pSetup->bEnabled?1:0);
	Fprintf( fp, "billingsetup_billingperiod %d\n",		pSetup->ucBillingPeriod);
	Fprintf( fp, "billingsetup_outputformat %d\n",		pSetup->ucOutputFormat);
	Fprintf( fp, "billingsetup_startdate %s\n",			pSetup->szStartDate);
	Fprintf( fp, "billingsetup_enddate %s\n",			pSetup->szEndDate);
	Fprintf( fp, "billingsetup_title %s\n",				pSetup->szTitle);
	Fprintf( fp, "billingsetup_currency %s\n",			pSetup->szCurrencySymbol);
	Fprintf( fp, "billingsetup_header_name %d\n",		pSetup->bHeaderCustomerName?1:0);
	Fprintf( fp, "billingsetup_header_id %d\n",			pSetup->bHeaderCustomerID?1:0);
	Fprintf( fp, "billingsetup_header_mbtrans %d\n",	pSetup->bHeaderMBTransfered?1:0);
	Fprintf( fp, "billingsetup_header_mballow %d\n",	pSetup->bHeaderMBAllowance?1:0);
	Fprintf( fp, "billingsetup_header_maxconn %d\n",	pSetup->bHeaderMaxConnections?1:0);
	Fprintf( fp, "billingsetup_header_xscharge %d\n",	pSetup->bHeaderExcessCharges?1:0);
	Fprintf( fp, "billingsetup_header_totalcharge %d\n",pSetup->bHeaderTotalCharges?1:0);
}

void
SaveSettingsBillingCharges(FILE* fp, const BillingChargesStruct* pCharge)
{
	// Need to reverse the list prior to saving. (So it loads back in the same order)
	vector<const BillingCustomerStruct*> vec;

	vec.reserve(pCharge->nCustomers);

	const BillingCustomerStruct*	p;
	for (p = pCharge->pFirstCustomer; p; p=p->pNext)
		vec.push_back(p);

	for (int i=pCharge->nCustomers-1;i+1;--i)
	{
		p = vec[i];
		Fprintf( fp, "billingcharges \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" %d %d \"%s\" \"%s\" %d\n"
			, p->szCustomer
			, p->szCustomerID
			, p->szURL
			, p->szFixedCharge
			, p->szFixedPeriod
			, p->ucFixedGroup
			, p->bExcessCharge?1:0
			, p->szExcessCharge
			, p->szExcessPeriod
			, p->ucExcessGroup
			);
	}
}

void DoSaveAsciiPrefs( FILE *fp, Prefs *pPrefs )
{
	char	txt[MAX_FILTERSIZE];
	long	lp;
	
	OutDebug( "saving settings" );
	Fprintf( fp, "# settings file (%s)\n", VERSION_STRING );

	Fprintf (fp, "\nSettingsVersion: %s\n", DEF_APP_SETTINGS_VERSION_4_5);
	
	Fprintf( fp , "stat_style %d\n", pPrefs->stat_style  );

	// Pre Process
	OutDebug( "Saving preprocess details" );

	Fprintf (fp, "\n# Pre Processing: Download\n");

#if DEF_MAC
	// password is in separate field, so don't mess with location
	if (pPrefs->preproc_on)
		Fprintf (fp, "preproc_on\n");
	if (pPrefs->preproc_location[0])
		Fprintf (fp, "preproc_location %s\n", pPrefs->preproc_location);
#else	
	if ( pPrefs->preproc_on ) Fprintf( fp, "preproc_on   #PreProcess options, or you could just use a ftp:// url any where\n" );
	if ( pPrefs->preproc_location[0] )
	{
		std::string encryptedFTPLoc( EncryptFTPLocationPassword( pPrefs->preproc_location ) );
		Fprintf( fp, "preproc_location %s\n", encryptedFTPLoc.c_str() );
	}
#endif
	if ( pPrefs->preproc_tmp[0] ) Fprintf( fp, "preproc_tmp %s\n", pPrefs->preproc_tmp );

	if (mystrlen (pPrefs->preproc_luser))	Fprintf (fp, "preproc_luser %s\n", pPrefs->preproc_luser);
	if (mystrlen (pPrefs->preproc_lpass)) 
	{
		std::string encryptedPassword( EncryptPassword(pPrefs->preproc_lpass) );
		Fprintf (fp, "preproc_lpass %s\n", encryptedPassword.c_str() );
	}
	if (mystrlen (pPrefs->preproc_lsite))	Fprintf (fp, "preproc_lsite %s\n", pPrefs->preproc_lsite);

	Fprintf (fp, "\n# Pre Processing: Options\n");
	if (mystrlen (pPrefs->preproc_downloc))	Fprintf (fp, "preproc_downloc %s\n", pPrefs->preproc_downloc);
	if ( pPrefs->preproc_delete )			Fprintf (fp, "preproc_delete   #delete remote file after download\n");

	if ( pPrefs->searchfor[0] ) {
		OutDebug( "Saving search/replace details" );
		Fprintf( fp, "# Search for pattern in URL and replace with new string\n" );
		Fprintf( fp, "searchfor %s\n", pPrefs->searchfor );
		if ( pPrefs->replacewith[0] ) Fprintf( fp, "replacewith %s\n", pPrefs->replacewith );
	}

	// Report
	
	Fprintf (fp, "\n# Report: Output\n");
	Fprintf( fp, "out %s\n", pPrefs->outfile );
	Fprintf( fp, "language %s\n", pPrefs->language );

	if ( pPrefs->html_fontsize ) Fprintf( fp, "html_fontsize %d\n", pPrefs->html_fontsize );
	if ( !mystrcmpi( DEFAULT_FONT, pPrefs->html_font )  )
		Fprintf( fp, "html_font default\n" );
	else
	if ( pPrefs->html_font[0] )
		Fprintf( fp, "html_font %s\n", pPrefs->html_font );

	if ( pPrefs->report_format ) Fprintf( fp, "reportformat %d\n", pPrefs->report_format );
	if ( pPrefs->html_frames ) Fprintf( fp, "html_frames\n" );
	if ( pPrefs->html_css ) Fprintf( fp, "html_css\n" );
	if ( pPrefs->image_format ) Fprintf( fp, "imageformat %d\n", pPrefs->image_format );
	
	OutDebug( "Saving report options" );
	Fprintf (fp, "\n# Report: Page\n");
	if ( pPrefs->bandwidth ) Fprintf( fp, "bandwidth\n"); else Fprintf( fp, "nobandwidth\n");
	if ( pPrefs->head_title ) Fprintf( fp, "headtitle\n");	else Fprintf( fp, "noheadtitle\n" );
	if ( pPrefs->write_timestat ) Fprintf( fp, "timestat\n"); else Fprintf( fp, "notimestat\n");
	if ( pPrefs->headingOnLeft ) Fprintf( fp, "headingonleft\n"); else Fprintf( fp, "headingonright\n");
	if ( pPrefs->html_quickindex ) Fprintf( fp, "html_quickindex\n"); else Fprintf( fp, "nohtml_quickindex\n");
	if ( pPrefs->footer_label )			 Fprintf( fp, "footer_label\n");		  else Fprintf( fp, "nofooter_label\n");
	if ( pPrefs->footer_label_trailing ) Fprintf( fp, "footer_label_trailing\n"); else Fprintf( fp, "nofooter_label_trailing\n");
	Fprintf( fp, "report_title %s\n", pPrefs->report_title );
	WriteFlag( fp, pPrefs->shadow, "shadow" );

	Fprintf (fp, "theme %d\n", pPrefs->theme);

	Fprintf (fp, "\n# Report: Style\n");

	OutDebug( "Saving table info" );
	Fprintf( fp, "# Table cell colors\n" );
	for( lp=1; lp < 7; lp++ )
		Fprintf( fp, "rgb %d %06lx\n", lp, pPrefs->RGBtable[lp] );

	OutDebug ("Saving html page colours");
	Fprintf (fp, "# HTML page colors\n");
	for (lp = 7; lp <= 11; lp++)
		Fprintf (fp, "rgb %d %06lx\n", lp, pPrefs->RGBtable[lp]);

	Fprintf( fp, "# These are the 3D multi colors and pie colors\n" );
	for( lp=1; lp < 10; lp++ )
		Fprintf( fp, "graphrgb %d %06lx\n", lp, pPrefs->multi_rgb[lp] );
	pPrefs->RGBtable[0] = '-RGB';

	OutDebug( "Saving Graph info" );
	SaveGraphsSettings( fp );

	// Analysis
	Fprintf (fp, "\n# Analysis: Options\n");

	if( pPrefs->firstDayOfWeek ) Fprintf( fp, "weekstartday %d\n", pPrefs->firstDayOfWeek );
	
	if ( pPrefs->alldates == DATE_ALL )						// "All dates" menu item
		Fprintf( fp, "date all\n" );
	else if ( pPrefs->alldates == DATE_SPECIFY )			// "Custom" date range item
	{
		char	dateString[32];
		
/*		unixtime strings are WRONG here, both print out start time */

//		TimeStringToDays( pPrefs->date1, &days );
//		Fprintf( fp, "sdate %s    #unixtime=%ld\n", pPrefs->date1, days );
//		TimeStringToDays( pPrefs->date1, &days );
//		Fprintf( fp, "edate %s    #unixtime=%ld\n", pPrefs->date2, days );

		// print out in absolute format, YYYY/MM/DD

		// convert time_t to dateString
		ConvertTimeTToFWDateString (pPrefs->startTimeT, dateString);
		Fprintf (fp, "start_date %s      # YYYY/MM/DD\n", dateString);
		// convert time_t to dateString
		ConvertTimeTToFWDateString (pPrefs->endTimeT, dateString);
		Fprintf (fp, "end_date %s        # YYYY/MM/DD\n", dateString);
	}
	else													// named menu item for specific period
		Fprintf( fp, "date %s\n", DecodeDateRangeID( pPrefs->alldates, NULL, NULL) );

	if ( pPrefs->multimonths == 1 ) Fprintf( fp, "multimonths\n" ); else		// mutiple reports
	if ( pPrefs->multimonths == 2 ) Fprintf( fp, "multiweeks\n" );

	if ( pPrefs->filter_zerobyte )
		Fprintf( fp, "ignorezerobyte\n" );
	else
		Fprintf( fp, "acceptzerobyte\n" );
	if ( pPrefs->filter_robots ) Fprintf( fp, "ignorerobots\n" );
	if ( pPrefs->ignore_selfreferral ) Fprintf( fp, "ignoreself\n" );
	if ( pPrefs->ignorecase ) Fprintf( fp, "ignorecase\n" );
	if ( pPrefs->ignore_usernames ) Fprintf( fp, "ignoreusernames\n" );
	if ( pPrefs->ignore_bookmarkreferral ) Fprintf( fp, "ignorebookmark\n" );

	OutDebug( "Saving siteurl" );
	if ( mystrlen( pPrefs->siteurl ) )											// default host
		Fprintf( fp, "siteurl %s\n", pPrefs->siteurl );

	if ( mystrlen( pPrefs->defaultindex ) )	{									// default page
		Fprintf (fp, "\n# the default file(s) served, can be comma delimited ie 'indexfile index.html,index.php'\n");
		Fprintf( fp, "indexfile %s\n", pPrefs->defaultindex );	
	}

	if ( pPrefs->ignore_ipvhosts ) Fprintf( fp, "ignoreipvhosts\n" );			// GUI?

	Fprintf (fp, "\n# Analysis: Network\n");
	switch ( pPrefs->dnslookup ){
		case 1:	Fprintf( fp, "dnson\n" ); break;
		case 0:	Fprintf( fp, "dnsoff\n" ); break;
		case 2:	Fprintf( fp, "dnscache\n" ); break;
	}

	OutDebug( "Saving flags" );
	Fprintf( fp, "dnsAmount %d\n", pPrefs->dnsAmount );
	Fprintf( fp, "dnr_ttl %d\n", pPrefs->dnr_ttl );
	Fprintf( fp, "dnr_expire %d\n", pPrefs->dnr_expire );
	Fprintf( fp, "dnrfile %s\n", pPrefs->dnr_cache_file );
	if( pPrefs->ftp_passive )
		Fprintf( fp, "ftp_passive\n" );
	if ( pPrefs->page_remotetitle ) Fprintf( fp, "remotetitle\n");	else Fprintf( fp, "noremotetitle\n" );

	Fprintf (fp, "\n# Analysis: Extensions\n");
	OutDebug( "Saving EXT info" );
	SaveExtStrings( fp , pPrefs->pageStr, "page.ext" );
	SaveExtStrings( fp , pPrefs->downloadStr, "download.ext" );
	SaveExtStrings( fp , pPrefs->audioStr, "audio.ext" );
	SaveExtStrings( fp , pPrefs->videoStr, "video.ext" );

	Fprintf (fp, "\n# Analysis: Dynamic\n");
	if ( pPrefs->useCGI ) Fprintf( fp, "usecgi\n" );							// Retain URL parameters
	if ( pPrefs->retain_variable ) Fprintf( fp, "retain_variable\n" );
	if ( mystrlen( pPrefs->retain_variablelist ) )
		Fprintf( fp, "retain_varlist %s\n", pPrefs->retain_variablelist );

	WriteFlag( fp, pPrefs->streaming, "flushclients" );
	WriteValue( fp, pPrefs->server_bandwidth, "server_bandwidth" );

	// Filters
	
	OutDebug( "Saving filters" );
	Fprintf (fp, "\n# Filters: Input Data\n");
	if ( pPrefs->filterdata.filterInTot ){
		Fprintf( fp, "\n# FilterIn Format, 1 Digit Followed by the string, without space\n" );
		Fprintf( fp, "# 0=File/URL, 1=Visitor, 2=Agent, 3=Referral, 4=Errors, 5=VirtualHost, 6=Cookie, 7=Username, 8=Method, 9=1stReferralInSessionReferral\n" );
		Fprintf( fp, "# eg. FilterIn 3yahoo.com   or FilterIn 2!Mozilla\n" );
	}

	for( lp=0; lp<pPrefs->filterdata.filterInTot; lp++)
		Fprintf( fp, "filterIn %s\n", pPrefs->filterdata.filterIn[lp] );

	// Statistics

	Fprintf (fp, "\n# Statistics: Statistics\n");
	OutDebug( "Saving STAT info" );
	SaveStat_Settings( pPrefs, fp );

	Fprintf (fp, "\n# Statistics: Global\n");
	Fprintf( fp, "style %d\n", pPrefs->graph_style );
	WriteFlag( fp, pPrefs->graph_wider, "wider" );
	WriteFlag( fp, pPrefs->paletteType, "webpalette" );
	WriteFlag( fp, pPrefs->corporate_look, "corporate_look" );

	sprintf( txt, "dollar %.2lf   # Dollar value per Meg (1000000 bytes)\n", (float)(pPrefs->dollarpermeg/1000.0) );
	Fprintf( fp, txt );
	Fprintf( fp, "# number of minutes between hits that defines a new session\n" );
	Fprintf( fp, "sessiontime %d\n", pPrefs->session_timewindow );

	if ( mystrlen( pPrefs->cookievar ) )
		Fprintf( fp, "cookievar %s\n", pPrefs->cookievar );
	if ( mystrlen( pPrefs->urlsessionvar ) )
		Fprintf( fp, "urlsessionvar %s\n", pPrefs->urlsessionvar );

	Fprintf( fp, "# This is the limit of pages in the ClickStream/Session report\n" );		// GUI?
	Fprintf( fp, "sessionLimit %d\n", pPrefs->sessionLimit );
	Fprintf( fp, "# Amount of seconds to offset all times from the log file\n" );			// GUI?
	Fprintf( fp, "time_adjust %d\n", pPrefs->time_adjust );
	
	Fprintf (fp, "\n# Statistics: Domain Groups\n");
	OutDebug( "Saving org/groups/mappings" );
	Fprintf( fp, "# Organization mapping 'orgmap pattern=name'\n" );
	for( lp=0; lp<pPrefs->filterdata.orgTot; lp++)
		Fprintf( fp, "orgmap %s\n", pPrefs->filterdata.org[lp] );

	Fprintf (fp, "\n# Statistics: Content Groups\n");
	Fprintf( fp, "# Content Group mapping 'groups pattern=name'\n" );
	for( lp=0; lp<pPrefs->filterdata.groupTot; lp++)
		Fprintf( fp, "groups %s\n", pPrefs->filterdata.group[lp] );

	// Tracking

	Fprintf (fp, "\n# Statistics: Key Visitors\n");
	OutDebug( "Saving key Visitors/mappings" );
	Fprintf( fp, "# key Visitors mapping 'kvismap pattern=name'\n" );
	for( lp=0; lp<pPrefs->filterdata.keyvisitorsTot; lp++)
		Fprintf( fp, "kvismap %s\n", pPrefs->filterdata.keyvisitors[lp] );

	Fprintf (fp, "\n# Statistics: Key Pages\n");
	OutDebug( "Saving key Pages/mappings" );
	Fprintf( fp, "# key Pages mapping 'kpagmap pattern=name'\n" );
	for( lp=0; lp<pPrefs->nToKeyPages; lp++)
		Fprintf( fp, "kpagmap %s\n", pPrefs->szToKeyPages[lp] );
	Fprintf( fp, "kpag_depth %d\n",		pPrefs->nToKeyPageRouteDepth );
	Fprintf( fp, "kpag_maxrows %d\n",	pPrefs->nToKeyPageMaxRows );

	Fprintf (fp, "\n# Statistics: Key Pages From\n");
	OutDebug( "Saving key Pages From/mappings" );
	Fprintf( fp, "# key Pages mapping 'kpafmap pattern=name'\n" );
	for( lp=0; lp<pPrefs->nFromKeyPages; lp++)
		Fprintf( fp, "kpafmap %s\n",	pPrefs->szFromKeyPages[lp] );
	Fprintf( fp, "kpaf_depth %d\n",		pPrefs->nFromKeyPageRouteDepth );
	Fprintf( fp, "kpaf_maxrows %d\n",	pPrefs->nFromKeyPageMaxRows );

	// Advertising

	OutDebug( "Saving adverts" );
	Fprintf (fp, "\n# Advertising: Impressions\n");
	for( lp=0; lp<pPrefs->filterdata.advertTot; lp++)
		Fprintf( fp, "advert %s\n", pPrefs->filterdata.advert[lp] );

	Fprintf (fp, "\n# Advertising: Campaigns\n");
	for( lp=0; lp<pPrefs->filterdata.advert2Tot; lp++)
		Fprintf( fp, "advertcampaign %s\n", pPrefs->filterdata.advert2[lp] );

	if ( pPrefs->filterdata.routerTot ){													// GUI?
		OutDebug( "Saving router" );
		Fprintf( fp, "# Router/Network mapping 'internalnetwork pattern=name'\n" );
		for( lp=0; lp<pPrefs->filterdata.routerTot; lp++)
			Fprintf( fp, "internalnetwork %s\n", pPrefs->filterdata.router[lp] );
	}

	Fprintf( fp, "# Referral mapping 'referralmap pattern=name'\n" );
	for( lp=0; lp<pPrefs->filterdata.referralTot; lp++)
		Fprintf( fp, "referralmap %s\n", pPrefs->filterdata.referralMap[lp] );

	for( lp=0; lp<pPrefs->filterdata.notifyTot; lp++)										// GUI?
		Fprintf( fp, "notifyrules %s\n", pPrefs->filterdata.notify[lp] );
	

	OutDebug( "Saving database flags" );
	if ( pPrefs->forceddmm ) Fprintf( fp, "forcedateformat %d\t# 0 = Auto, 1 = DD/MM/YY,  2 = MM/DD/YY, 3 = YY/MM/DD\n", pPrefs->forceddmm );
	if ( pPrefs->database_active ) Fprintf( fp, "database_active\n" );
	if ( pPrefs->database_extended ) Fprintf( fp, "database_extended\n" );
	if ( pPrefs->database_file[0] ) Fprintf( fp, "database_file %s\n", pPrefs->database_file );
	if ( pPrefs->database_no_report ) Fprintf( fp, "database_no_report\n" );
	if ( pPrefs->database_excluded ) Fprintf( fp, "database_excluded\n" );

	// Virtual
	
	OutDebug( "Saving vhost info" );
	Fprintf (fp, "\n# Virtual: Domains\n");
	if ( pPrefs->multidomains > 0 ) Fprintf (fp, "multidomains\n");
	if ( pPrefs->multivhosts == 1 ) Fprintf( fp, "multivhosts\n" ); else
	if ( pPrefs->multivhosts == 2 ) Fprintf( fp, "multitopdirs\n" ); else
	if ( pPrefs->multivhosts == 3 ) Fprintf( fp, "multitop2dirs\n" ); else
	if ( pPrefs->multivhosts == 4 ) Fprintf( fp, "multicustomdirs\n" ); else
	if ( pPrefs->multivhosts == 5 ) Fprintf( fp, "multiclients\n" );
	if ( pPrefs->multireport_path[0] ) Fprintf( fp, "multireport_path %s\n", pPrefs->multireport_path );
	if ( pPrefs->vhost_seqdirs ) Fprintf( fp, "sequencedir\n" );

	{
		char *sortStr[] = { "name", "size", "reqs", "page", "visits", "error", "date",0 };
		long x;
		x = pPrefs->VDsortby;
		if ( x > 6 ) x = 6;
		Fprintf( fp, "vdsortby %s\n", sortStr[ x ] );
	}

	Fprintf (fp, "\n# Virtual: Aggregation\n");
	Fprintf( fp, "# Virtual Host mapping 'vhostmap pattern=name'\n" );
	for( lp=0; lp<pPrefs->filterdata.vhostmapTot; lp++)
		Fprintf( fp, "vhostmap %s\n", pPrefs->filterdata.vhostmap[lp] );

	Fprintf (fp, "\n# Virtual: Mapping\n");
	// "vhostpath" saved in editpath.c

	OutDebug( "Saving cluster info" );
	Fprintf (fp, "\n# Virtual: Clustering\n");
	Fprintf (fp, "clustering_enabled %d\n", pPrefs->clusteringActive);		// for 4.5, just a flag
	WriteValue (fp, pPrefs->numClustersInUse, "clusters");					// for 4.5, this is the count

	// ************* CLUSTER GROUPS ************/
	Fprintf (fp, "\n# Statistics: Definitions/Mapping of Clusters\n");
	Fprintf( fp, "# Cluster mapping 'defineCluster ip,path,name'\n" );
	for( lp=0; lp<pPrefs->clusterNamesTot; lp++)
		WriteGroupSet( fp, "defineCluster", pPrefs->clusterNames[lp] );
		//Fprintf( fp, "clusterNames %s\n", pPrefs->clusterNames[lp] );

	
	// Post Process
	
	OutDebug( "Saving postproc details" );
	Fprintf (fp, "\n# Post Processing: Compression\n");
	if ( pPrefs->postproc_larchive ) {
		Fprintf( fp, "postproc_larchive\n" );
		Fprintf( fp, "post_compresslog\n" );		// for Windoze compatibility
	}
	Fprintf( fp, "postproc_lcompressor %d\n", pPrefs->postproc_lcompressor);
	if ( pPrefs->postproc_lsea )			Fprintf( fp, "postproc_lsea\n" );
	if ( pPrefs->postproc_lbin )			Fprintf( fp, "postproc_lbin\n" );
	if ( pPrefs->postproc_ldel )			Fprintf( fp, "postproc_ldel\n" );

	if ( pPrefs->postproc_rarchive ) {
		Fprintf( fp, "postproc_rarchive\n" );
		Fprintf( fp, "post_zipreport\n" );			// for Windoze compatibility
	}
	Fprintf( fp, "postproc_rcompressor %d\n", pPrefs->postproc_rcompressor);
	if ( pPrefs->postproc_rsea )			Fprintf( fp, "postproc_rsea\n" );
	if ( pPrefs->postproc_rbin )			Fprintf( fp, "postproc_rbin\n" );
	if ( pPrefs->postproc_rdel )			Fprintf( fp, "postproc_rdel\n" );

	Fprintf (fp, "\n# Post Processing: Upload\n");
	// NON mac section
	if ( pPrefs->postproc_uploadreport == 2 )
		Fprintf( fp, "post_uploadreportdir\n" );
	else
	if ( pPrefs->postproc_uploadreport )
		Fprintf( fp, "post_upreport\n" );

	if ( pPrefs->postproc_deletereport ) Fprintf( fp, "post_delreport\n" );
#if DEF_MAC
	// password is in separate field, so don't mess with location								QCM #50440
	if (pPrefs->postproc_uploadreportlocation[0])
		Fprintf (fp, "post_reportloc %s\n", pPrefs->postproc_uploadreportlocation);
#else
	// location has all the other data in too, so need to encrypt password in situ
	if ( pPrefs->postproc_uploadreportlocation[0] )
	{
		std::string encryptedFTPLoc( EncryptFTPLocationPassword( pPrefs->postproc_uploadreportlocation ) );
		WriteString( fp, encryptedFTPLoc.c_str(), "post_reportloc" );
	}
#endif
	if ( pPrefs->postproc_uploadlog ) Fprintf( fp, "post_uplog\n" );
#if DEF_MAC
	// password is in separate field, so don't mess with location								QCM #50440
	if (pPrefs->postproc_uploadloglocation[0])
		Fprintf (fp, "post_logloc %s\n", pPrefs->postproc_uploadloglocation);
#else
	// location has all the other data in too, so need to encrypt password in situ
	if ( pPrefs->postproc_uploadloglocation[0] )
	{
		std::string encryptedFTPLoc( EncryptFTPLocationPassword( pPrefs->postproc_uploadloglocation ) );
		WriteString( fp, encryptedFTPLoc.c_str(), "post_logloc" );
	}
#endif
	//------------------

	// Mac only section
	if (mystrlen (pPrefs->postproc_luser)) Fprintf (fp, "postproc_luser %s\n", pPrefs->postproc_luser);
	if (mystrlen (pPrefs->postproc_lpass))
	{
		std::string encryptedPassword( EncryptPassword(pPrefs->postproc_lpass) );
		Fprintf (fp, "postproc_lpass %s\n", encryptedPassword.c_str());
	}
	if (mystrlen (pPrefs->postproc_lsite)) Fprintf (fp, "postproc_lsite %s\n", pPrefs->postproc_lsite);
	if (pPrefs->postproc_deletelog) Fprintf (fp, "postproc_deletelog\n");

	if (mystrlen (pPrefs->postproc_ruser)) Fprintf (fp, "postproc_ruser %s\n", pPrefs->postproc_ruser);
	if (mystrlen (pPrefs->postproc_rpass))
	{
		std::string encryptedPassword( EncryptPassword(pPrefs->postproc_rpass) );
		Fprintf (fp, "postproc_rpass %s\n", encryptedPassword.c_str());
	}
	if (mystrlen (pPrefs->postproc_rsite)) Fprintf (fp, "postproc_rsite %s\n", pPrefs->postproc_rsite);
	if (pPrefs->postproc_deletereport) Fprintf (fp, "postproc_deletereport\n");
	//------------------

	Fprintf (fp, "\n# Post Processing: Email\n");
	if ( pPrefs->postproc_emailon )  Fprintf( fp, "post_emailon\n" );
	if ( mystrlen(pPrefs->postproc_email)>1 )  Fprintf( fp, "post_email %s\n", pPrefs->postproc_email );
	if ( mystrlen(pPrefs->postproc_emailfrom)>1 )  Fprintf( fp, "post_emailfrom %s\n", pPrefs->postproc_emailfrom );
	if ( mystrlen(pPrefs->postproc_emailsub)>1 )  Fprintf( fp, "post_emailsub %s\n", pPrefs->postproc_emailsub );
	if ( mystrlen(pPrefs->postproc_emailsmtp)>1 )  Fprintf( fp, "post_emailsmtp %s\n", pPrefs->postproc_emailsmtp );
	if ( mystrlen(pPrefs->postproc_emailmsg)>1 ) {
		Fprintf( fp, "post_emailmsg " );
		WriteMultiLine( fp , pPrefs->postproc_emailmsg );
		Fprintf( fp, "\n" );
	}

	// Custom Format

	OutDebug( "Saving custom info" );
	Fprintf (fp, "\n# Custom log file: Format\n");
	if ( pPrefs->custom_format ) Fprintf( fp, "usecustomformat\n" );
	if ( pPrefs->custom_seperator ) Fprintf( fp, "custom_seperator %d	#0x%08x (%c)\n", pPrefs->custom_seperator, pPrefs->custom_seperator, pPrefs->custom_seperator );
	if ( pPrefs->custom_dateformat ) Fprintf( fp, "custom_dateformat %d\n", pPrefs->custom_dateformat );
	if ( pPrefs->custom_timeformat ) Fprintf( fp, "custom_timeformat %d\n", pPrefs->custom_timeformat );
	for ( lp=0; lp<25; lp++){
		if ( pPrefs->custom_dataIndex[lp] )
			Fprintf( fp, "custom_dataIndex %d %d\n", lp, pPrefs->custom_dataIndex[lp] );
	}
	
	Fprintf (fp, "\n");

	// Other options
	
	if ( pPrefs->live_sleeptype ) Fprintf( fp, "realtimeinterval %d\n", pPrefs->live_sleeptype );
	//if ( pPrefs->stat_sessionHistory ) Fprintf( fp, "sessionhistory\n" );

	if ( pPrefs->notify_type ){
		OutDebug( "Saving notify info" );
		Fprintf( fp, "notify_type %d\n", pPrefs->notify_type );
		if ( mystrlen(pPrefs->notify_destination)>1 ) {
			Fprintf( fp, "notify_dest %s\n", pPrefs->notify_destination );
		}
	}

	OutDebug( "Saving remote info" );
	if ( pPrefs->remotelogin_enable ) Fprintf( fp, "remoteloginon\n" );
	if ( pPrefs->remotelogin_port ) Fprintf( fp, "remotelogin_port %ld\n", pPrefs->remotelogin_port );
	if ( pPrefs->remotelogin_passwd[0] ) {
		CryptPasswd( pPrefs->remotelogin_passwd );
		Fprintf( fp, "remotelogin_passwd %s\n", pPrefs->remotelogin_passwd );
		CryptPasswd( pPrefs->remotelogin_passwd );
	}

	// Save all the PDF Settings
	OutDebug( "Saving PDF info" );
	PDFAllSettingsCSave( fp, &pPrefs->pdfAllSetC );


	// Save the Billing Settings
	OutDebug("Saving the Billing Setup info");
	SaveSettingsBillingSetup(fp, &pPrefs->billingSetup);
	OutDebug("Saving the Billing Charges info");
	SaveSettingsBillingCharges(fp, &pPrefs->billingCharges);

	OutDebug( "Saving editpath info" );
	SaveEditPathNew( fp, pPrefs->EditPaths );

	OutDebug( "Saving htmlheaders info" );
	Fprintf( fp, "htmlhead\n" ); fflush( fp );

#ifdef DEF_MAC
	fwrite( pPrefs->html_head, mystrlen(pPrefs->html_head), 1, fp ); fflush(fp);
#else
	char *p = pPrefs->html_head;
	char *p2 = p;
	long len = 0;
	while ( *p != 0 )
	{
		len++;
		if ( *p == '\r' )
		{
			//*p = 0;
			fwrite( p2, len-1, 1, fp );
			//*p = '\r';
			len = 0;
			p2 = p+1;
		}
		p++;
	}
	fflush(fp);
#endif
	Fprintf( fp, "\n<HTMLEND>\n\n" ); fflush( fp );

	Fprintf( fp, "htmlfoot\n" ); fflush( fp );
#ifdef DEF_MAC
	fwrite( pPrefs->html_foot, mystrlen(pPrefs->html_foot), 1, fp ); fflush(fp);
#else
	p = pPrefs->html_foot;
	p2 = p;
	len = 0;
	while ( *p != 0 )
	{
		len++;
		if ( *p == '\r' )
		{
			//*p = 0;
			fwrite( p2, len-1/*mystrlen(p2)*/, 1, fp );
			//*p = '\r';
			len = 0;
			p2 = p+1;
		}
		p++;
	}
	fflush(fp);
#endif
	Fprintf( fp, "\n<HTMLEND>\n\n" ); fflush(fp);
	Fprintf( fp, "#END\n" );

	OutDebug( "Saving Complete." );
}






//////////////////////////////////////////////////////////




char *InitLogQ( long total )
{
	if ( glogFilenamesData == NULL )
		glogFilenamesData = (char*)malloc( total );
	return glogFilenamesData;
}


int AddFileToLogQ( char *filename, long index )
{
	char	*lastfilename, *newfilename;
	char	linkdir[MAXFILENAMESSIZE];
	long	len;

#ifndef DEF_MAC
#if DEF_WINDOWS
	if ( IsShortCut( filename ) ){
		GetShortCut( filename, linkdir );
		if ( IsFileaFolder( linkdir ) )
			filename = linkdir;
	}
	if ( IsURLShortCut( filename ) ){
		int len;
		GetURLShortCut( filename, linkdir );
		filename = linkdir;
		len = strlen( filename );
		if ( filename[len-1] == '/' )
			strcat( filename, "*.log" );
	}
#endif
	
	if ( IsURL( filename ) && strchr( filename, '*' ) ){
		AddWildcardFtpDirList( filename, index );				//add to Q
		return glogFilenamesNum;
	}

	if ( IsFileaFolder( filename ) ){
		char	dir[MAXFILENAMESSIZE];
		sprintf( dir, "%s\\*", filename );
		AddWildCardFilenames( dir, index );				//add to Q
		return glogFilenamesNum;
	}

	len = mystrlen( filename );
	if ( index == 0 ){
		newfilename = InitLogQ( MAXFILEDATASIZE );
	} else {
		lastfilename = glogFilenames[glogFilenamesNum-1];
		newfilename = lastfilename + strlen( lastfilename ) + 1;
	}


	if ( ((newfilename+len) - glogFilenamesData) < MAXFILEDATASIZE && len>1 ){
		strcpy( newfilename, filename );
		newfilename[ len ] = 0;
		glogFilenames[index] = newfilename;
		glogFilenamesNum = index+1;
	}
	
#else
	// Old Crappy way for the mac, remove soon
	mystrcpy (glogFilenamesStr[index],filename);
	glogFilenames[index]=glogFilenamesStr[index];
	glogFilenamesNum = index+1;
#endif // #ifdef DEF_MAC
	return glogFilenamesNum;
}


// ------------------------ SORTING OF LOG Queue --------------------------------

typedef struct {
	char *m_fileName;
	long m_date;
} SortedLogQ;


long CompSortLogQ( void *p1, void *p2, long *result )
{
	SortedLogQ *d1,*d2;

	d1 = (SortedLogQ*)p1;
	d2 = (SortedLogQ*)p2;

	*result = d1->m_date - d2->m_date;
	return 0;
}

long SortLogQ( void )
{
	long	i;
	SortedLogQ	*logQtoSort;

	logQtoSort = new SortedLogQ[ glogFilenamesNum ];

	OutDebugs( "Getting log file start times...." );
	// Construct the data and get all the logs start dates if possible.
	for (i=0; i<glogFilenamesNum; i++){
		long type, date1, date2;
		logQtoSort[i].m_fileName = glogFilenames[i];
		GetLogFileType( logQtoSort[i].m_fileName, &type, &date1, &date2 );
		logQtoSort[i].m_date = date1;
	}

	OutDebugs( "Sorting logs...." );
	// sort our structure of dates...
	FastQSort( logQtoSort, glogFilenamesNum, sizeof(SortedLogQ), CompSortLogQ, TOP_SORT );

	// put back the filename pointers in the sorted order.
	for (i=0; i<glogFilenamesNum; i++){
		glogFilenames[i] = logQtoSort[i].m_fileName;
	}

	delete logQtoSort;

	return glogFilenamesNum;
}
// ------------- END


int AddLogFile( char *filename, long index )
{
	return AddFileToLogQ( filename, index );
}



void AddLogFromArg( char * dir )
{
	if ( dir ){
		char *file;
		if ( (file = IsURL( dir )) ){
			AddFileToLogQ( file, glogFilenamesNum );		//glogFilenamesNum is incremented internaly
		} else {
			if ( mystrchr( dir, '*' ) ){
				glogFilenamesNum = AddWildCardFilenames( dir, glogFilenamesNum );
			} else
				AddFileToLogQ( dir, glogFilenamesNum );
		}
	}
}


static void Check_OutputSuffix( const char *output, const char *suff )
{
	const char *p;
	p = strrchr( output, '.' );
	if ( p )
	{
		if ( strcmpd( suff, p ) )		// if extension is NOT valid, replace it.
			strcpy( (char*)p, suff );
	}
}

void FixReportOutputExtension( Prefs *pPrefs )
{
	switch( pPrefs->report_format )
	{
		case 0:	Check_OutputSuffix( pPrefs->outfile, HTML_EXT ); break;
		case 1:	Check_OutputSuffix( pPrefs->outfile, HTML_EXT ); break;
		case 2:	Check_OutputSuffix( pPrefs->outfile, PDF_EXT ); break;
		case 3:	Check_OutputSuffix( pPrefs->outfile, RTF_EXT ); pPrefs->image_format = 2;  break;
		case 4:	Check_OutputSuffix( pPrefs->outfile, COMMA_EXT ); break;
		case 5:	Check_OutputSuffix( pPrefs->outfile, EXCEL_EXT ); break;
	}
}





static char *LangFiles[]= {
	"English",
	"German",
	"Italian",
	"French",
	"Spanish",
	"Swedish",
	"Norwegian",	
	"Danish",
	"Dutch",
	"Japanese",
	0
};

char *GetLangName( long n )
{
	long i = 0;
	return LangFiles[n];
}



long GetStringArg2( char *line, char *argv2, char *dest )
{
	char	*file;

	if( line )
	{
		file = mystrchr( line, ' ' );
		if( file )
		{
			file++;
			trimLine( file );
			mystrcpy( dest, file );
		} else
			mystrcpy( dest, argv2 );
	} else {
		mystrcpy( dest, argv2 );
	}
	return 2;
}





// This is used by the summary template , but only if its set, other wise its ignored.
extern char SummaryTemplatename[256];

/*
-postproclocation	set the post processing location path\n\
-archive	archive the output report into a zip file at the same location\n\
-compresslog	compress the log file being processed after processing\n\
-sendto		send the Zipped report or the report files themselves to ...\n\
*/
static int debugarg = 0;


long ProcessPrefsLine( int argc, char **argv, char *line, long count, Prefs *pPrefs )
{
	static int LFcount=0;
	char	*arg, *oarg;
	int		minus=0,plus=0, ret = 1,
			unknown = FALSE, unknown2 = FALSE;


	if ( line ){
		if ( doing_htmlhead ){
			long len = 0;
			if ( *line == '\n' ) LFcount++;
			if ( !mystrcmpi( "<HTMLEND>", argv[count] ) ) LFcount=3;
			if ( LFcount < 2 ){
				trimLine( line );
				if ( *line )
					len = mystrncatNull( pPrefs->html_head, line, MAX_HTMLSIZE );
#ifndef DEF_MAC
				pPrefs->html_head[len] = '\r';len++;pPrefs->html_head[len] = 0;
				//mystrncatNull( pPrefs->html_head, "\r", MAX_HTMLSIZE ); // Need to put in extra return for the GUI control
#endif

// not sure what you guys are trying to do here but all I need is this for the header and below for the footer

#ifdef DEF_MAC
				mystrncatNull( pPrefs->html_head, "\r", MAX_HTMLSIZE );
#else
				pPrefs->html_head[len] = '\n';len++;pPrefs->html_head[len] = 0;
#endif
			} else {
				LFcount = doing_htmlhead = 0;
			}
			return 1;
		} else
		if ( doing_htmlfoot ){
			if ( *line == '\n' ) LFcount++;
			if ( !mystrcmpi( "<HTMLEND>", argv[count] ) ) LFcount=3;
			if ( LFcount < 2 ){
				trimLine( line );
				if ( *line )
					mystrncatNull( pPrefs->html_foot, line, MAX_HTMLSIZE );
#ifndef DEF_MAC
				mystrncatNull( pPrefs->html_foot, "\r", MAX_HTMLSIZE ); // Need to put in extra return for the GUI control
#endif

#ifdef DEF_MAC
				mystrncatNull( pPrefs->html_foot, "\r", MAX_HTMLSIZE );
#else
				mystrncatNull( pPrefs->html_foot, "\n", MAX_HTMLSIZE );
#endif

			} else {
				LFcount = doing_htmlfoot = 0;
			}
			return 1;
		} else
		if ( doing_editpath ){		// for multi line PATH statements
			static char tempLine[128];
			switch( LFcount ){
				case 0 :	mystrcpy( tempLine, line ); LFcount++; break;
				case 1 :	AddEditPath( &pPrefs->EditPaths, tempLine, line ); LFcount=0; doing_editpath=0; break;
			}
		}
	}


	oarg = arg = argv[count];
	if ( !arg ) return ret;
	if ( !*arg ) return ret;
	if ( *arg == '-' ) { minus++; arg++; } else
	if ( *arg == '+' ) { plus++; arg++; }
	if ( myisdigit(*arg) ) return ret;

	if ( debugarg ) {
		OutDebugs( "Checking arg (%s)\n", argv[0] );
	}

	if(		!mystrcmpi( "h", arg ) ||
			!mystrcmpi( "-help", arg ) ||
			!mystrcmpi( "help", arg ) ||
			!mystrcmpi( "?", arg )  ) {
		long lineindex = 0;

		while( helptext[lineindex] ){
			fwrite( helptext[lineindex], mystrlen( helptext[lineindex] ), 1, stdout );
			lineindex++;
		}
		//write( 1, helptext, mystrlen( helptext ) );
		//exit(0);
	} else
	if ( !mystrcmpi( "debugarg", arg )  ) {
		gDebugPrint = 1;
		debugarg = 1;
	} else
	if ( !mystrcmpi( "checkschedule", arg )  ) {
		InitReadSchedule(0);
		ForceCheckSchedule();
	} else
	// this is a special case, it will run a schedule, just 1 from the list if there, then quit, thats all
	if ( !mystrcmpi( "runschedule", arg )  ) {
		long sched;

		if ( count+1 < argc )
			sched = atoi( argv[count+1] );
		InitReadSchedule(2);
		RunScheduleX( sched );
		exit(0);
	}
	else 
	if (!mystrcmpi ("SettingsVersion:", arg))
	{
		if (count + 1 < argc)
		{
			if (!mystrcmpi (DEF_APP_SETTINGS_VERSION_4_5, argv[count + 1]))
				pPrefs->prefVER = CURRENT_PREF_VERSION;
			ret = 2;
		}
	}
	else if (mystrncmpi("billingsetup_", arg , 13) == 0)
	{
		// *************************************************************************
		// Added Billing values.
		// *************************************************************************
		if (mystrcmpi("billingsetup_enabled", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.bEnabled = (atoi( argv[count+1] ) == 1);
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_billingperiod", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.ucBillingPeriod = atoi( argv[count+1] );
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_outputformat", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.ucOutputFormat = atoi( argv[count+1] );
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_startdate", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				mystrncpy(pPrefs->billingSetup.szStartDate, argv[count+1], DATE_WIDTH);
				pPrefs->billingSetup.szStartDate[DATE_WIDTH] = 0;
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_enddate", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				mystrncpy(pPrefs->billingSetup.szEndDate, argv[count+1], DATE_WIDTH);
				pPrefs->billingSetup.szEndDate[DATE_WIDTH] = 0;
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_title", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				mystrncpy(pPrefs->billingSetup.szTitle, argv[count+1], BILLINGTITLE_WIDTH);
				pPrefs->billingSetup.szTitle[BILLINGTITLE_WIDTH] = 0;
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_currency", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				mystrncpy(pPrefs->billingSetup.szCurrencySymbol, argv[count+1], CURRENCY_WIDTH);
				pPrefs->billingSetup.szCurrencySymbol[CURRENCY_WIDTH] = 0;
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_header_name", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.bHeaderCustomerName = (atoi( argv[count+1] ) == 1);
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_header_id", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.bHeaderCustomerID = (atoi( argv[count+1] ) == 1);
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_header_mbtrans", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.bHeaderMBTransfered = (atoi( argv[count+1] ) == 1);
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_header_mballow", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.bHeaderMBAllowance = (atoi( argv[count+1] ) == 1);
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_header_maxconn", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.bHeaderMaxConnections = (atoi( argv[count+1] ) == 1);
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_header_xscharge", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.bHeaderExcessCharges = (atoi( argv[count+1] ) == 1);
				ret = 2;
			}
		}
		else if (mystrcmpi("billingsetup_header_totalcharge", arg ) == 0)
		{
			if ( count+1 < argc )
			{
				pPrefs->billingSetup.bHeaderTotalCharges = (atoi( argv[count+1] ) == 1);
				ret = 2;
			}
		}
	}

	// This one is the billing charges
	else if (mystrcmpi("billingcharges", arg ) == 0)
	{
		if ( count+10 < argc )
		{
			BillingCustomerStruct* p = new BillingCustomerStruct;

			// Name
			mystrncpy(p->szCustomer, argv[count+1], BILLINGCUSTOMER_NAME_WIDTH);
			p->szCustomer[BILLINGCUSTOMER_NAME_WIDTH] = 0;

			// id
			mystrncpy(p->szCustomerID, argv[count+2], BILLINGCUSTOMER_ID_WIDTH);
			p->szCustomerID[BILLINGCUSTOMER_ID_WIDTH] = 0;

			// URL
			mystrncpy(p->szURL, argv[count+3], BILLINGCUSTOMER_URL_WIDTH);
			p->szURL[BILLINGCUSTOMER_URL_WIDTH] = 0;

			// FixedCharge
			mystrncpy(p->szFixedCharge, argv[count+4], BILLINGCUSTOMER_CHARGE_WIDTH);
			p->szFixedCharge[BILLINGCUSTOMER_CHARGE_WIDTH] = 0;

			// FixedPeriod
			mystrncpy(p->szFixedPeriod, argv[count+5], BILLINGCUSTOMER_PERIOD_WIDTH);
			p->szFixedPeriod[BILLINGCUSTOMER_PERIOD_WIDTH] = 0;

			// FixedGroup
			p->ucFixedGroup = atoi(argv[count+6]);

			// ExcessEnabled.
			p->bExcessCharge = (atoi(argv[count+7])==1);

			// ExcessCharge
			mystrncpy(p->szExcessCharge, argv[count+8], BILLINGCUSTOMER_CHARGE_WIDTH);
			p->szExcessCharge[BILLINGCUSTOMER_CHARGE_WIDTH] = 0;

			// ExcessPeriod
			mystrncpy(p->szExcessPeriod, argv[count+9], BILLINGCUSTOMER_PERIOD_WIDTH);
			p->szExcessPeriod[BILLINGCUSTOMER_PERIOD_WIDTH] = 0;

			// ExcessGroup
			p->ucExcessGroup = atoi(argv[count+10]);


			pPrefs->billingCharges.nCustomers++;
			p->pNext = pPrefs->billingCharges.pFirstCustomer;
			pPrefs->billingCharges.pFirstCustomer = p;

			ret = 11;	// ??
		}
	}
	// *************************************************************************
	// End of Billing values.
	// *************************************************************************

	else 
	if ( !mystrcmpi( "saveprefs", arg ) ) {										// save whatever prefs are in memory now.
		if ( count+1 < argc ) DoSavePrefsFile( argv[count+1], pPrefs );
		ret = 2;
	} else
	if ( !mystrcmpi( "prefsfile", arg ) || !mystrcmpi( "p", arg ) ) {			// read prefs with out a default first
		if ( count+1 < argc ) DoReadAsciiPrefs( argv[count+1], pPrefs );
		ret = 2;
	} else
	if ( !mystrcmpi( "settingsfile", arg ) || !mystrcmpi( "r", arg ) ) {		// clears prefs and reads new ones.
		if ( count+1 < argc ) DoReadPrefsFile( argv[count+1], pPrefs );
		ret = 2;
	} else
	if ( !mystrcmpi( "includesettings", arg ) || !mystrcmpi( "r", arg ) ) {		// 100% clean read/attach prefs command.
		if ( count+1 < argc ) DoAddPrefsFile( argv[count+1], pPrefs );
		ret = 2;
	} else
	if ( !mystrcmpi( "convert_to_w3c", arg ) ) {
		gConvertLogToW3C_Flag = TRUE;
		if ( count+1 < argc )
		{
			ret = GetStringArg2( line, argv[count+1], pPrefs->outfile );
		}
		ret = 2;
	} else
	if ( !mystrcmpi( "log", arg ) || !mystrcmpi( "l", arg ) || !mystrcmpi( "input", arg ) ) {
		if ( count+1 < argc ) {
			char *dir = argv[count+1];

			if ( line ){		// needed to support unix comandlines (line=0 then)
				trimLine( line );
				if( dir = mystrchr( line, ' ' ) )
					dir++;
			}
			AddLogFromArg( dir );

			ret = 2;
		}
	}
	else if( !mystrcmpi( "weekstartday", arg )  )
	{
		if( count+1 < argc )
		{
			if( myisdigit( *argv[count+1] ) )
			{
				pPrefs->firstDayOfWeek=atoi( argv[count+1] );

				if( pPrefs->firstDayOfWeek>6 )
				{
					pPrefs->firstDayOfWeek=0;
				}
			}
		}
	}
	else if (!mystrcmpi ("sdate", arg))				// these are here for backwards compatibility
	{
		char	tempDate[20];
		
		if (count + 1 < argc)
		{
			mystrcpy (tempDate, argv[count + 1]);
			// convert tempDate to time_t, date string may be in any international format !!!
			ConvertOldDateFormatToTimeT (tempDate, &(pPrefs->startTimeT));
			pPrefs->alldates = DATE_SPECIFY;
			ret = 2;
		}
	}
	else if (!mystrcmpi ("edate", arg))
	{
		char	tempDate[20];

		if (count + 1 < argc)
		{
			mystrcpy (tempDate, argv[count + 1]);
			// convert tempDate to time_t, date string may be in any international format !!!
			ConvertOldDateFormatToTimeT (tempDate, &(pPrefs->endTimeT));
			pPrefs->alldates = DATE_SPECIFY;
			ret = 2;
		}
	}
	else if (!mystrcmpi ("start_date", arg))			// new tags for YYYY/MM/DD "absolute" dates
	{
		char	tempDate[20];

		if (count + 1 < argc)
		{
			mystrcpy (tempDate, argv[count + 1]);
			// convert tempDate to time_t, always in YYYY/MM/DD format
			ConvertFWDateStringToTimeT (tempDate, &(pPrefs->startTimeT));
			pPrefs->alldates = DATE_SPECIFY;
			ret = 2;
		}
	}
	else if (!mystrcmpi ("end_date", arg))
	{
		char	tempDate[20];

		if (count + 1 < argc)
		{
			mystrcpy (tempDate, argv[count + 1]);
			// convert tempDate to time_t, always in YYYY/MM/DD format
			ConvertFWDateStringToTimeT (tempDate, &(pPrefs->endTimeT));
			pPrefs->alldates = DATE_SPECIFY;
			ret = 2;
		}
	}
	else if ( !mystrcmpi( "alldates", arg ) || !mystrcmpi( "ad", arg )  ) {
		pPrefs->alldates = DATE_ALL;
	} else
	if ( !mystrcmpi( "date", arg ) || !mystrcmpi( "daterange", arg ) ) {
		if ( count+1 < argc ) {
			if ( !mystrcmpi( "all", argv[count+1] ) )
				pPrefs->alldates = DATE_ALL;
			else
			// get the old format style for date range.
			if ( isdigit( *argv[count+1] ) )
				pPrefs->alldates = atoi( argv[count+1] );
			else
			{
				// new style date range using ascii Ids
				int id = ReturnIDFromDateRange( argv[count+1] );
				// if we hav eno idea what date range it is, then just ignore it.
				if ( id != -1 )
				{
					pPrefs->alldates = id;
					pPrefs->startTimeT = 0;										// pPrefs->date1[0] = 0;
					pPrefs->endTimeT = 0;										// pPrefs->date2[0] = 0;
				}
			}
			ret = 2;
		}
	} else
	if ( !strncmp( "PDF", arg, 3 ) ) {
		if( (argc > 2) && (argv[2][0] == '#') )
		{
			PDFAllSettingsCRead( arg, &pPrefs->pdfAllSetC, argv[count+1] );
		}
		else
		{
			static std::string strParam;
			// use stuff after the first space in the line directly.
			if ( line ) 
			{
				char *arg2Str = strchr( line, ' ' );
				if ( arg2Str )
					strParam = (arg2Str+1);
			} else
			// join the argv commands to one string garbage (Thanks Trev)
			if ( argv[count+1] )
			{
				int pos = 2;
				strParam = argv[count+1];
				while ( pos < argc )
				{
					if ( argv[count+pos][0] == '#' )
						break;
					strParam += " ";
					strParam += argv[count+pos];
					pos++;
				}

			}
			PDFAllSettingsCRead( arg, &pPrefs->pdfAllSetC, strParam.c_str() );
		}
		return 1;
	} else
	if ( !mystrcmpi( "dnsbackground", arg )  ) {
		pPrefs->dnslookup |= DNR_BACKGROUNDMODE;
	} else
	if ( !mystrcmpi( "dnsdynamic", arg )  ) {
		pPrefs->dnslookup |= DNR_DYNAMICMODE;
	} else
	if ( !mystrcmpi( "dnscache", arg )  ) {
		pPrefs->dnslookup = DNR_CACHEONLY;
	} else
	if ( !mystrcmpi( "dnson", arg ) || !mystrcmpi( "+dns", oarg ) ) {
		pPrefs->dnslookup = DNR_LOOKUPSON;
	} else
	if ( !mystrcmpi( "dnsoff", arg ) || !mystrcmpi( "dns", arg ) ) {
		pPrefs->dnslookup = 0;
	} else
	if ( !mystrcmpi( "dnr", arg )  ) {
		if ( count+1 < argc ){
			pPrefs->dnslookup = atoi( argv[count+1] );
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "dnsAmount", arg )  ) {
		if ( count+1 < argc ){
			pPrefs->dnsAmount = atoi( argv[count+1] );
			ret = 2;
		}
		if ( pPrefs->dnsAmount > 10000 )
			pPrefs->dnsAmount = 50;
	} else
	if ( !mystrcmpi( "dnr_ttl", arg )  ) {
		if ( count+1 < argc ){
			pPrefs->dnr_ttl = atoi( argv[count+1] );
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "dnr_expire", arg )  ) {
		if ( count+1 < argc ){
			pPrefs->dnr_expire = atoi( argv[count+1] );
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "dnrfile", arg )  ) {
		if ( count+1 < argc ) {
			char path[512];
			ret = GetStringArg2( line, argv[count+1], path );
#ifndef DEF_MAC
			if ( !strchr( path, PATHSEP ) )	// if theres no path component, add one
				sprintf( pPrefs->dnr_cache_file, "%s%c%s", gPath, PATHSEP, path );
			else
				sprintf( pPrefs->dnr_cache_file, "%s", path );
#endif
			OutDebugs( "Using new DNRCache File %s", pPrefs->dnr_cache_file );
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "ignorerobots", arg )  ) {
		pPrefs->filter_robots = TRUE;
	} else
	if ( !mystrcmpi( "ignoreself", arg )  ) {
		pPrefs->ignore_selfreferral = TRUE;
	} else
	if ( !mystrcmpi( "ignorebookmark", arg )  ) {
		pPrefs->ignore_bookmarkreferral = TRUE;
	} else
	if ( !mystrcmpi( "ignorecase", arg )  ) {
		pPrefs->ignorecase = TRUE;
	} else
	if ( !mystrcmpi( "ignoreipvhosts", arg )  ) {
		pPrefs->ignore_ipvhosts = TRUE;
	} else
	if ( !mystrcmpi( "ignoreusernames", arg )  ) {
		pPrefs->ignore_usernames = TRUE;
	} else
	if ( !mystrcmpi( "usecgi", arg ) ) {
		pPrefs->useCGI = TRUE;
	} else
	if ( !mystrcmpi( "retain_variable", arg ) ) {
		pPrefs->retain_variable = TRUE;
	} else
	if ( !mystrcmpi( "retain_varlist", arg ) ) {
		if ( count+1 < argc )
			ret = GetStringArg2( line, argv[count+1], pPrefs->retain_variablelist );
	} else
	if ( !mystrcmpi( "ignorezerobyte", arg ) || !mystrcmpi( "+f0", oarg ) ) {
		pPrefs->filter_zerobyte = TRUE;
	} else
	if ( !mystrcmpi( "includezero", arg ) || !mystrcmpi( "-f0", oarg ) ) {
		pPrefs->filter_zerobyte = FALSE;
	} else
	if ( !mystrcmpi( "sort", arg )  ) {
		if ( count+1 < argc && !myisdigit( *argv[count+1]) ) {
			if ( !mystrcmpi( "name", argv[count+1] ) )	pPrefs->sortby = SORT_NAMES;
			if ( !mystrcmpi( "none", argv[count+1] ) )	pPrefs->sortby = SORT_NAMES;
			if ( !mystrcmpi( "byte", argv[count+1] ) )	pPrefs->sortby = SORT_SIZES;
			if ( !mystrcmpi( "reqs", argv[count+1] ) )	pPrefs->sortby = SORT_REQUESTS;
			if ( !mystrcmpi( "hits", argv[count+1] ) )	pPrefs->sortby = SORT_REQUESTS;
			if ( !mystrcmpi( "page", argv[count+1] ) )	pPrefs->sortby = SORT_PAGES;
			if ( !mystrcmpi( "clips", argv[count+1] ) )	pPrefs->sortby = SORT_PAGES;
		} else {
			pPrefs->sortby = myatoi( argv[count+1] );
		}
		SetAll_Sorting( pPrefs, pPrefs->sortby );
	} else
	if ( !mystrcmpi( "out", oarg ) ||
		 !mystrcmpi( "out", arg ) ||
		 !mystrcmpi( "d", arg ) ||
		 !mystrcmpi( "o", arg ) ) {	// handle spaces in OUTPUT path from config file
		if ( count+1 < argc ){
			ret = GetStringArg2( line, argv[count+1], pPrefs->outfile );
			char *p = pPrefs->outfile;

			if ( pPrefs->outfile[1] != ':' ){		// if its a "C:" style path, ie drive path, dont transcode it.
				// fix the stupid mac path insanity...
				while( *p ){
					if ( *p == ':' ) *p = '/';
					p++;
				}
			}
		}
	} else
	if ( !mystrcmpi( "language", arg )  ) {
		if ( count+1 < argc ){
			if ( myisdigit( *argv[count+1]) ){
				long id;
				id = atoi( argv[count+1] );
				mystrcpy( pPrefs->language, GetLangName(id) );
			} else
				mystrcpy( pPrefs->language, argv[count+1] );
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "siteurl", arg )  ) {
		if ( count+1 < argc ){
			mystrcpy( pPrefs->siteurl, argv[count+1] );
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "indexfile", arg )  ) {
		if ( count+1 < argc ){
			mystrcpy( pPrefs->defaultindex, argv[count+1] );
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "searchfor", arg )  ) {
		if ( count+1 < argc ){
			mystrcpy( pPrefs->searchfor, argv[count+1] );
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "replacewith", arg )  ) {
		if ( count+1 < argc ){
			mystrcpy( pPrefs->replacewith, argv[count+1] );
			ret = 2;
		}
	} else
	//------------------------------
	if ( !mystrcmpi( "filterIn", arg ) || !mystrcmpi( "filterexIn", arg ) ) {
		char type = '0';
		// PREFS decode
		if ( ( count+1 < argc ) && (pPrefs->filterdata.filterInTot < MAX_FILTERUNITS) )
		{
			char *str = NULL;
			char *filterStr = NULL;

			if( line )
			{
				str = mystrchr( line, ' ' );
				if ( str ) str++;
				filterStr = mystrchr( str, ' ' );
				if ( filterStr ) filterStr++;
			} else {
				str = argv[count+1];
				if( (count+2 < argc) )
					filterStr = argv[count+2];
			}

			if( str && ( *str >= '0' && *str <= '9' || *str == '-' ) )
			{
				trimLine( str );
				mystrcpy( pPrefs->filterdata.filterIn[pPrefs->filterdata.filterInTot++], str );
			} else
			if( filterStr ) 
			{
				if( !mystrcmpi( "url", str ) ) type = '0';
				if( !mystrcmpi( "visitor", str ) ) type = '1';
				if( !mystrcmpi( "client", str ) ) type = '1';
				if( !mystrcmpi( "agent", str ) ) type = '2';
				if( !mystrcmpi( "referral", str ) ) type = '3';
				if( !mystrcmpi( "stat", str ) ) type = '4';
				if( !mystrcmpi( "vhost", str ) ) type = '5';
				if( !mystrcmpi( "cookie", str ) ) type = '6';
				if( !mystrcmpi( "username", str ) ) type = '7';
				if( !mystrcmpi( "method", str ) ) type = '8';
				if( !mystrcmpi( "sessionreferral", str ) ) type = '9';

				if ( !mystrcmpi( "filterexIn", arg ) )
					sprintf( pPrefs->filterdata.filterIn[pPrefs->filterdata.filterInTot++], "%c!%s" , type, filterStr );
				else {
					if( *filterStr == '-' )
						*filterStr = '!';
					sprintf( pPrefs->filterdata.filterIn[pPrefs->filterdata.filterInTot++], "%c%s" , type, filterStr );
				}
				OutDebugs( "added filter %c%s", type, filterStr );
			}
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "advert", arg ) ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.advertTot < MAX_FILTERUNITS) ){
			char *dest;

			dest = pPrefs->filterdata.advert[pPrefs->filterdata.advertTot];
			ret = GetStringArg2( line, argv[count+1], dest );
			pPrefs->filterdata.advertTot++;
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "advertcampaign", arg ) ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.advert2Tot < MAX_FILTERUNITS) ){
			char *dest;

			dest = pPrefs->filterdata.advert2[pPrefs->filterdata.advert2Tot];
			ret = GetStringArg2( line, argv[count+1], dest );
			pPrefs->filterdata.advert2Tot++;
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "filterurl", arg ) ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.filterInTot < MAX_FILTERUNITS) ){
			char *p = pPrefs->filterdata.filterIn[pPrefs->filterdata.filterInTot];
			*p++ = '0';
			mystrcpy( p, argv[count+1] );
			pPrefs->filterdata.filterInTot++;
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "filtersite", arg ) || !mystrcmpi( "filterclient", arg ) || !mystrcmpi( "filterhost", arg ) ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.filterInTot < MAX_FILTERUNITS) ){
			char *p = pPrefs->filterdata.filterIn[pPrefs->filterdata.filterInTot];
			*p++ = '1';
			mystrcpy( p, argv[count+1] );
			pPrefs->filterdata.filterInTot++;
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "filtersource", arg )  ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.filterInTot < MAX_FILTERUNITS) ){
			char *p = pPrefs->filterdata.filterIn[pPrefs->filterdata.filterInTot];
			*p++ = '2';
			mystrcpy( p, argv[count+1] );
			pPrefs->filterdata.filterInTot++;
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "filterbrowser", arg )  ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.filterInTot < MAX_FILTERUNITS) ){
			char *p = pPrefs->filterdata.filterIn[pPrefs->filterdata.filterInTot];
			*p++ = '3';
			mystrcpy( p, argv[count+1] );
			pPrefs->filterdata.filterInTot++;
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "filterreferal", arg ) || !mystrcmpi( "filterreferral", arg ) ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.filterInTot < MAX_FILTERUNITS) ){
			char *p = pPrefs->filterdata.filterIn[pPrefs->filterdata.filterInTot];
			*p++ = '4';
			mystrcpy( p, argv[count+1] );
			pPrefs->filterdata.filterInTot++;
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "internalnetwork", arg )  ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.routerTot < MAX_FILTERUNITS) ) {
			ret = GetStringArg2( line, argv[count+1], pPrefs->filterdata.router[pPrefs->filterdata.routerTot++] );
		}
	} else
	if ( !mystrcmpi( "orgmap", arg )  ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.orgTot < MAX_FILTERUNITS) ){
			ret = GetStringArg2( line, argv[count+1], pPrefs->filterdata.org[pPrefs->filterdata.orgTot++] );
		}
	} else
	if ( !mystrcmpi( "kvismap", arg )  ||  !mystrcmpi( "kvisitormap", arg ) ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.keyvisitorsTot < MAX_FILTERUNITS) )
		{
			ret = GetStringArg2( line, argv[count+1], pPrefs->filterdata.keyvisitors[pPrefs->filterdata.keyvisitorsTot++] );
		}
	} else
	if ( !mystrcmpi( "kpagmap", arg ) || !mystrcmpi( "kpagetomap", arg )  ) {
		if ( ( count+1 < argc ) && (pPrefs->nToKeyPages < NUMBER_OF_KEYPAGES) )
		{
			ret = GetStringArg2( line, argv[count+1], pPrefs->szToKeyPages[pPrefs->nToKeyPages++] );
		}
	} else
	if ( !mystrcmpi( "kpag_depth", arg ) || !mystrcmpi( "kpageto_depth", arg ) ) {
		if ( count+1 < argc ) {
			char *pt;
			if ( (pt = mystrchr( line, ' ' )) == NULL )
				pt = argv[count+1];
			pPrefs->nToKeyPageRouteDepth = atoi(pt);
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "kpag_maxrows", arg ) || !mystrcmpi( "kpageto_maxrows", arg ) ) {
		if ( count+1 < argc ) {
			char *pt;
			if ( (pt = mystrchr( line, ' ' )) == NULL )
				pt = argv[count+1];
			pPrefs->nToKeyPageMaxRows = atoi(pt);
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "kpafmap", arg ) || !mystrcmpi( "kpagefrommap", arg ) ) {
		if ( ( count+1 < argc ) && (pPrefs->nFromKeyPages < NUMBER_OF_KEYPAGES) ){
			ret = GetStringArg2( line, argv[count+1], pPrefs->szFromKeyPages[pPrefs->nFromKeyPages++] );
		}
	} else
	if ( !mystrcmpi( "kpaf_depth", arg ) || !mystrcmpi( "kpagefrom_depth", arg ) ) {
		if ( count+1 < argc ) {
			char *pt;
			if ( (pt = mystrchr( line, ' ' )) == NULL )
				pt = argv[count+1];
			pPrefs->nFromKeyPageRouteDepth = atoi(pt);
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "kpaf_maxrows", arg ) || !mystrcmpi( "kpagefrom_maxrows", arg ) ) {
		if ( count+1 < argc ) {
			char *pt;
			if ( (pt = mystrchr( line, ' ' )) == NULL )
				pt = argv[count+1];
			pPrefs->nFromKeyPageMaxRows = atoi(pt);
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "groupdir", arg ) || !mystrcmpi( "groups", arg ) || !mystrcmpi( "groupmap", arg )   ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.groupTot < MAX_FILTERUNITS) ){
			ret = GetStringArg2( line, argv[count+1], pPrefs->filterdata.group[pPrefs->filterdata.groupTot++] );
		}
	} else
	if ( !mystrcmpi( "defineCluster", arg ) ) {
		if ( ( count+1 < argc ) && (pPrefs->clusterNamesTot < MAX_FILTERUNITS) ){
			ReadGroupSet( line, argv[count+1], pPrefs->clusterNames[pPrefs->clusterNamesTot] );
			pPrefs->clusterNamesTot++;
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "vhostmap", arg )   ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.vhostmapTot < MAX_FILTERUNITS) ){
			ret = GetStringArg2( line, argv[count+1], pPrefs->filterdata.vhostmap[pPrefs->filterdata.vhostmapTot++] );
		}
	} else
	if ( !mystrcmpi( "referralmap", arg )   ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.referralTot < MAX_FILTERUNITS) ){
			ret = GetStringArg2( line, argv[count+1], pPrefs->filterdata.referralMap[pPrefs->filterdata.referralTot++] );
		}
	} else
	if ( !mystrcmpi( "notifyrules", arg )  ) {
		if ( ( count+1 < argc ) && (pPrefs->filterdata.notifyTot < MAX_FILTERUNITS) ){
			ret = GetStringArg2( line, argv[count+1], pPrefs->filterdata.notify[pPrefs->filterdata.notifyTot++] );
		}
	} else
	//-------------------------------
	if ( !mystrcmpi( "graphsort", arg ) || !mystrcmpi( "graphaxis", arg ) ) {
		if ( count+1 < argc && !myisdigit( *argv[count+1]) ) {
			if ( !mystrcmpi( "main", argv[count+1] ) )	pPrefs->graph_type = TYPE_USEMAIN;
			if ( !mystrcmpi( "byte", argv[count+1] ) )	pPrefs->graph_type = TYPE_BYTES;
			if ( !mystrcmpi( "reqs", argv[count+1] ) )	pPrefs->graph_type = TYPE_REQUESTS;
			if ( !mystrcmpi( "hits", argv[count+1] ) )	pPrefs->graph_type = TYPE_REQUESTS;
			ret = 2;
		} else {
			pPrefs->graph_type = myatoi( argv[count+1] );
		}
	} else
	if ( !mystrcmpi( "style", arg )  || !mystrcmpi( "graphstyle", arg ) ) {
		if ( count+1 < argc && mystrlen( argv[count+1] )>1 ) {
			if ( !mystrcmpi( "2d", argv[count+1] ) )	pPrefs->graph_style = GRAPH_2DSTYLE;
			if ( !mystrcmpi( "3d", argv[count+1] ) )	pPrefs->graph_style = GRAPH_3DSTYLE;
			ret = 2;
		} else {
			pPrefs->graph_style = myatoi( argv[count+1] );
		}
	} else
	if ( !mystrcmpi( "webpalette", arg ) ) {
		pPrefs->paletteType = 1;
	} else
	if ( !mystrcmpi( "wider", arg )  ) {
		pPrefs->graph_wider = TRUE;
	} else
	if ( !mystrcmpi( "forceddmm", arg )  ) {
		pPrefs->forceddmm = 1;
	} else
	if ( !mystrcmpi( "ftp_passive", arg )  ) {
		pPrefs->ftp_passive = 1;
	} else
	if ( !mystrcmpi( "forcedateformat", arg )  ) {
		if ( count+1 < argc ){
			if ( myisdigit( *argv[count+1] ) )
				pPrefs->forceddmm = atoi( argv[count+1] );
			else
			switch( *argv[count+1] ){
				case 'D' : pPrefs->forceddmm = 1; break;
				case 'M' : pPrefs->forceddmm = 2; break;
				case 'Y' : pPrefs->forceddmm = 3; break;
			}
			ret = 2;
		}
	} else
	if ( !mystrcmpi( "database_active", arg )  ) {
		pPrefs->database_active = 1;
	} else
	if ( !mystrcmpi( "database_extended", arg )  ) {
		pPrefs->database_extended = 1;
	} else
	if ( !mystrcmpi( "database_file", arg )  ) {
		if ( count+1 < argc ) {
			ret = GetStringArg2( line, argv[count+1], pPrefs->database_file );
		}
	} else
	if ( !mystrcmpi( "database_no_report", arg )  ) {
		pPrefs->database_no_report = true;
	} else
	if ( !mystrcmpi( "database_excluded", arg )  ) {
		pPrefs->database_excluded = true;
	} else
	if ( !mystrcmpi( "html_fontsize", arg )  ) {
		if ( count+1 < argc ) pPrefs->html_fontsize = atoi( argv[count+1] );
	} else
	if ( !mystrcmpi( "html_font", arg )  ) {
		if ( count+1 < argc ) {
			ret = GetStringArg2( line, argv[count+1], pPrefs->html_font );
			// if the font is set to 'default' set it to what we want.
			if ( !mystrcmpi( "default", pPrefs->html_font )  )
				mystrcpy( pPrefs->html_font, DEFAULT_FONT );
		}
	} else
	if ( !mystrcmpi( "statdepth", arg )  ) {
		if ( count+1 < argc )
			pPrefs->stat_depth = atoi( argv[count+1] );

		if ( pPrefs->stat_depth == 0 ) pPrefs->stat_depth = 12;
		if ( pPrefs->stat_depth <= 12 )
			pPrefs->stat_depth -= 3;
		SetAll_Tablesize( pPrefs, pPrefs->stat_depth );
	} else
	if ( !mystrcmpi( "allimagesoff", arg )  ) {
		SetAll_GraphMode( pPrefs, 0 );
	} else
	if ( !mystrcmpi( "dollar", arg )  ) {
		if ( count+1 < argc ) pPrefs->dollarpermeg = (short)(atof( argv[count+1] )*1000);
	} else
	if ( !mystrcmpi( "sessiontime", arg )  ) {
		if ( count+1 < argc ) pPrefs->session_timewindow = atoi( argv[count+1] );
	} else
	if ( !mystrcmpi( "sessionLimit", arg )  ) {
		if ( count+1 < argc ) pPrefs->sessionLimit = atoi( argv[count+1] );
	} else
	if ( !mystrcmpi( "time_adjust", arg )  ) {
		if ( count+1 < argc ){
			char *txt = argv[count+1];
			if ( mystrchr( txt , ':' ) ){
				struct tm	date;
				time_t		ct;

				ct = StringToDaysTime( txt, &date);
				pPrefs->time_adjust = ct;
			} else
				pPrefs->time_adjust = atoi( txt );
		}
	} else
	if ( !mystrcmpi( "reportformat", arg )  ) {
		if ( count+1 < argc ){
			if ( myisdigit( *argv[count+1] ) ){
				pPrefs->report_format = atoi( argv[count+1] );
			} else {
				if ( !mystrcmpi( "html", argv[count+1] )  )		pPrefs->report_format = 0;
				if ( !mystrcmpi( "htmlframed", argv[count+1] )){pPrefs->report_format = 1; pPrefs->html_frames = 1; }
				if ( !mystrcmpi( "pdf", argv[count+1] )  )		pPrefs->report_format = 2;
				if ( !mystrcmpi( "rtf", argv[count+1] )  )		pPrefs->report_format = 3;
				if ( !mystrcmpi( "comma", argv[count+1] )  )	pPrefs->report_format = 4;
				if ( !mystrcmpi( "excel", argv[count+1] )  )	pPrefs->report_format = 5;
			}
#ifdef DEF_UNIX
			FixReportOutputExtension( pPrefs );
#endif
		}
	} else
	if ( !mystrcmpi( "html_frames", arg )  ) {
		pPrefs->html_frames = 1;
	} else
	if ( !mystrcmpi( "html_css", arg )  ) {
		pPrefs->html_css = 1;
	} else
	if ( !mystrcmpi( "imageformat", arg )  ) {
		if ( count+1 < argc ){
			if ( myisdigit( *argv[count+1] ) ){
				pPrefs->image_format = atoi( argv[count+1] );
			} else {
				if ( !mystrcmpi( "gif", argv[count+1] )  )		pPrefs->image_format = 0;
				if ( !mystrcmpi( "jpeg", argv[count+1] ) )		pPrefs->image_format = 1;
				if ( !mystrcmpi( "png", argv[count+1] )  )		pPrefs->image_format = 2;
				if ( !mystrcmpi( "bmp", argv[count+1] )  )		pPrefs->image_format = 3;
				if ( !mystrcmpi( "none", argv[count+1] ) )		SetAll_GraphMode( pPrefs, 0 );
			}
		}
		//if ( count+1 < argc ) pPrefs->image_format = atoi( argv[count+1] );
	} else
	if ( !mystrcmpi( "timestat", arg )  ) {
		pPrefs->write_timestat = 1;
	} else
	if ( !mystrcmpi( "notimestat", arg )  ) {
		pPrefs->write_timestat = 0;
	} else
	if ( !mystrcmpi( "bandwidth", arg )  ) {
		pPrefs->bandwidth = 1;
	} else
	if ( !mystrcmpi( "nobandwidth", arg )  ) {
		pPrefs->bandwidth = 0;
	} else
	if ( !mystrcmpi( "footer_label", arg )  ) {
		pPrefs->footer_label = 1;
	} else
	if ( !mystrcmpi( "nofooter_label", arg )  ) {
		pPrefs->footer_label = 0;
	} else
	if ( !mystrcmpi( "footer_label_trailing", arg )  ) {
		pPrefs->footer_label_trailing = 1;
	} else
	if ( !mystrcmpi( "nofooter_label_trailing", arg )  ) {
		pPrefs->footer_label_trailing = 0;
	} else
	if ( !mystrcmpi( "remotetitle", arg )  ) {
		pPrefs->page_remotetitle = 1;
	} else
	if ( !mystrcmpi( "noremotetitle", arg )  ) {
		pPrefs->page_remotetitle = 0;
	} else
	if ( !mystrcmpi( "headtitle", arg )  ) {
		pPrefs->head_title = 1;
	} else
	if ( !mystrcmpi( "noheadtitle", arg )  ) {
		pPrefs->head_title = 0;
	} else
	if ( !mystrcmpi( "report_title", arg ) || !mystrcmpi( "PDFReportTitle", arg ) ) {
		if ( count+1 < argc ){
			ret = GetStringArg2( line, argv[count+1], pPrefs->report_title );
		}
	} else
	if ( !mystrcmpi( "summaryfile", arg ) ) {
		if ( count+1 < argc ){
			ret = GetStringArg2( line, argv[count+1], SummaryTemplatename );
			OutDebugs( "Using alternative summary template '%s'", SummaryTemplatename );
		}
	} else
	if ( !mystrcmpi( "html_quickindex", arg )  ) {
		pPrefs->html_quickindex = 1;
	} else
	if ( !mystrcmpi( "nohtml_quickindex", arg )  ) {
		pPrefs->html_quickindex = 0;
	} else
	if ( !mystrcmpi( "headingonleft", arg )  ) {
		pPrefs->headingOnLeft = 1;
	} else
	if ( !mystrcmpi( "headingonright", arg )  ) {
		pPrefs->headingOnLeft = 0;
	} else
	if ( !mystrcmpi( "corporate_look", arg )  ) {
		pPrefs->corporate_look = 1;
	} else
	if ( !mystrcmpi( "shadow", arg )  ) {
		pPrefs->shadow = 1;
	} else
	if ( !mystrcmpi( "noshadow", arg )  ) {
		pPrefs->shadow = 0;
	} else
	if ( !mystrcmpi( "cookievar", arg ) ) {
		ret = GetStringArg2( line, argv[count+1], pPrefs->cookievar );
	}
	else if ( !mystrcmpi( "urlsessionvar", arg ) )
	{
		if ( count+1 < argc )
			ret = GetStringArg2 (line, argv[count+1], pPrefs->urlsessionvar);			// QCM #49452 RHF
	}
	else if ( !mystrcmpi( "clustering_enabled", arg ) ) {
		if ( count+1 < argc && isdigit( *argv[count+1] ) )
			pPrefs->clusteringActive = atoi( argv[count+1] );
		else
			pPrefs->clusteringActive = 1;
	}
	// more logical command set, for on/off modes.
	else if ( !mystrcmpi( "noclustering", arg ) ) {
			pPrefs->clusteringActive = 0;
	}
	else if ( !mystrcmpi( "clusters", arg ) ) {
		if ( count+1 < argc )
			pPrefs->numClustersInUse = atoi( argv[count+1] );
		else
			pPrefs->numClustersInUse = 2;
	}



	if ( !mystrcmpi( "server_bandwidth", arg ) ) {
		if ( count+1 < argc )
			pPrefs->server_bandwidth = atoi( argv[count+1] );
	} else
	if ( !mystrcmpi( "streaming", arg ) || !mystrcmpi( "flushclients", arg ) ) {
		pPrefs->streaming = 1;
	} else
		unknown = TRUE;

	// Pre Process -----------------------------
	if ( !strcmpd( "preproc_", arg )  ) {
		if ( !mystrcmpi( "preproc_tmp", arg )  ) {
			if ( count+1 < argc ){
				ret = GetStringArg2( line, argv[count+1], pPrefs->preproc_tmp );
			}
		} else
#if DEF_MAC
		// location field is just that, not host, url, password and location						QCM #50440
		// so don't try to decrypt password or we're in big trouble here
		if (!mystrcmpi ("preproc_location", arg))
		{
			if (count + 1 < argc)
				ret = GetStringArg2 (line, argv[count+1], pPrefs->preproc_location);
		}
#else
		// location field has all login information, so need to extract password first
		if ( !mystrcmpi( "preproc_location", arg )  ) {
			if ( count+1 < argc )
			{
				char* file;
				if( (file=mystrchr( line, ' ' )) == NULL ) file=argv[count+1];
				if( file )
				{
					trimLine( file );
					std::string decryptedFTPLoc( DecryptFTPLocationPassword( file+1 ) );
					mystrcpy( pPrefs->preproc_location, decryptedFTPLoc.c_str() );
				}
				ret = 2;
			}
		}
#endif		
		else if ( !mystrcmpi( "preproc_on", arg )  ) {
			pPrefs->preproc_on = 1;
		} else
		if ( !mystrcmpi( "preproc_delete", arg )  ) {
			pPrefs->preproc_delete = 1;
		} else
		if (!mystrcmpi ("preproc_luser", arg) ) {
			if ( count+1 < argc ){
				ret = GetStringArg2( line, argv[count+1], pPrefs->preproc_luser );
			}
		} else
		if (!mystrcmpi ("preproc_lpass", arg) ) {
			std::string decryptedPasswd( DecryptPassword(argv[count+1]) );
			mystrcpy (pPrefs->preproc_lpass, decryptedPasswd.c_str());
		} else
		if (!mystrcmpi ("preproc_lsite", arg) ) {
			mystrcpy (pPrefs->preproc_lsite, argv[count+1]);
		} else
		if (!mystrcmpi ("preproc_downloc", arg) ) {
			if (count + 1 < argc)
			{
				ret = GetStringArg2( line, argv[count+1], pPrefs->preproc_downloc );
			}
		}
	} else

	// Post Process -----------------------------
	if (!strcmpd("postproc_", arg)) {			// Compress Logs
		if (!mystrcmpi ("postproc_larchive", arg)) {			// Compress Logs
			pPrefs->postproc_larchive = TRUE;
		} else
		if (!mystrcmpi ("postproc_lcompressor", arg)) {
			if (count + 1 < argc)
				if (isdigit (*argv[count+1]))
					pPrefs->postproc_lcompressor = atoi (argv[count+1]);
		} else
		if (!mystrcmpi ("postproc_lsea", arg)) {
			pPrefs->postproc_lsea = TRUE;
		} else
		if (!mystrcmpi ("postproc_lbin", arg)) {
			pPrefs->postproc_lbin = TRUE;
		} else
		if (!mystrcmpi ("postproc_ldel", arg)) {
			pPrefs->postproc_ldel = TRUE;
		} else
		
		if (!mystrcmpi ("postproc_rarchive", arg)) {			// Compress Reports
			pPrefs->postproc_rarchive = TRUE;
		} else
		if (!mystrcmpi ("postproc_rcompressor", arg)) {
			if (count + 1 < argc)
				if (isdigit (*argv[count+1]))
					pPrefs->postproc_rcompressor = atoi (argv[count+1]);
		} else
		if (!mystrcmpi ("postproc_rsea", arg)) {
			pPrefs->postproc_rsea = TRUE;
		} else
		if (!mystrcmpi ("postproc_rbin", arg)) {
			pPrefs->postproc_rbin = TRUE;
		} else
		if (!mystrcmpi ("postproc_rdel", arg)) {
			pPrefs->postproc_rdel = TRUE;
		} else
		if (!mystrcmpi ("postproc_luser", arg) ) {
			mystrcpy (pPrefs->postproc_luser, argv[count+1]);
		} else
		if (!mystrcmpi ("postproc_lpass", arg) ) {
			std::string decryptedPasswd( DecryptPassword(argv[count+1]) );
			mystrcpy (pPrefs->postproc_lpass, decryptedPasswd.c_str());
		} else
		if (!mystrcmpi ("postproc_lsite", arg) ) {
			mystrcpy (pPrefs->postproc_lsite, argv[count+1]);
		} else
		if ( !mystrcmpi( "postproc_deletelog", arg )  ) {
			pPrefs->postproc_deletelog = TRUE;
		} else

		if (!mystrcmpi ("postproc_ruser", arg) ) {
			mystrcpy (pPrefs->postproc_ruser, argv[count+1]);
		} else
		if (!mystrcmpi ("postproc_rpass", arg) ) {
			std::string decryptedPasswd( DecryptPassword(argv[count+1]) );
			mystrcpy (pPrefs->postproc_rpass, decryptedPasswd.c_str());
		} else
		if (!mystrcmpi ("postproc_rsite", arg) ) {
			mystrcpy (pPrefs->postproc_rsite, argv[count+1]);
		} else
		if ( !mystrcmpi( "postproc_deletereport", arg )  ) {
			pPrefs->postproc_deletereport = TRUE;
		}
	} else
	if ( !strcmpd( "post_", arg ) )
	{
#if DEF_MAC
		// location field is just that, not host, url, password and location	QCM #50440
		// so don't try to decrypt password or we're in big trouble here
		if (!mystrcmpi ("post_logloc", arg))
		{
			if (count + 1 < argc)
				ret = GetStringArg2 (line, argv[count+1], pPrefs->postproc_uploadloglocation);
		}
#else
		if ( !mystrcmpi( "post_logloc", arg ) ) {
			if ( count+1 < argc )
			{
				std::string decryptedFTPLoc( DecryptFTPLocationPassword( argv[count+1] ) );
				mystrcpy( pPrefs->postproc_uploadloglocation, decryptedFTPLoc.c_str() );
				ret = 2;
			}
		}
#endif		

#if DEF_MAC
		// location field is just that, not host, url, password and location	QCM #50440
		// so don't try to decrypt password or we're in big trouble here
		else if (!mystrcmpi( "post_reportloc", arg) || !mystrcmpi ("postproclocation", arg))
		{
			if (count + 1 < argc)
				ret = GetStringArg2 (line, argv[count+1], pPrefs->postproc_uploadreportlocation);
		}
#else
		else if ( !mystrcmpi( "post_reportloc", arg ) || !mystrcmpi( "postproclocation", arg )  ) {
			if ( count+1 < argc )
			{
				std::string decryptedFTPLoc( DecryptFTPLocationPassword( argv[count+1] ) );
				mystrcpy( pPrefs->postproc_uploadreportlocation, decryptedFTPLoc.c_str() );
				ret = 2;
			}
		}
#endif		
		else if ( !mystrcmpi( "post_emailon", arg )  ) {
			pPrefs->postproc_emailon = TRUE;
		} else
		if ( !mystrcmpi( "post_email", arg )  ) {
			if ( count+1 < argc )
				GetStringArg2( line, argv[count+1], pPrefs->postproc_email );
		} else
		if ( !mystrcmpi( "post_emailfrom", arg )  ) {
			if ( count+1 < argc )
				GetStringArg2( line, argv[count+1], pPrefs->postproc_emailfrom );
		} else
		if ( !mystrcmpi( "post_emailsub", arg )  ) {		// QCM 44794 fixed.
			if ( count+1 < argc )
				GetStringArg2( line, argv[count+1], pPrefs->postproc_emailsub );
		} else
		if ( !mystrcmpi( "post_emailsmtp", arg )  ) {
			if ( count+1 < argc )
				GetStringArg2( line, argv[count+1], pPrefs->postproc_emailsmtp );
		} else
		if ( !mystrcmpi( "post_emailmsg", arg )  ) {
			char *pt = mystrchr( line, ' ' );
			if ( pt && count+1 < argc ){
				ReadMultiLineFormat( pPrefs->postproc_emailmsg, pt+1 );
				ret = 2;
			}
		} else
		if ( !mystrcmpi( "post_zipreport", arg ) || !mystrcmpi( "archive", arg )  ) {
			pPrefs->postproc_rarchive = TRUE;
		} else
		if ( !mystrcmpi( "post_compresslog", arg )  ) {
			pPrefs->postproc_larchive = TRUE;
		} else
		if ( !mystrcmpi( "post_uplog", arg ) || !mystrcmpi( "uploadlogfile", arg ) ) {
			pPrefs->postproc_uploadlog = TRUE;
		} else
		if ( !mystrcmpi( "post_upreport", arg ) || !mystrcmpi( "sendto", arg ) ) {
			pPrefs->postproc_uploadreport = TRUE;
		} else
		if ( !mystrcmpi( "post_uploadreportdir", arg ) ) {
			pPrefs->postproc_uploadreport = 2;
		} else
		if ( !mystrcmpi( "post_delreport", arg )  ) {
			pPrefs->postproc_deletereport = TRUE;
		}
	} else
	//-----------------------------
	if ( !mystrcmpi( "multidomains", arg ) ) {
		pPrefs->multidomains = MAX_DOMAINS;
	} else
	if ( !mystrcmpi( "multivhosts", arg ) ) {
		pPrefs->multivhosts = 1;
	} else
	if ( !mystrcmpi( "multitopdirs", arg ) ) {
		pPrefs->multivhosts = 2;
	} else
	if ( !mystrcmpi( "multitop2dirs", arg ) ) {
		pPrefs->multivhosts = 3;
	} else
	if ( !mystrcmpi( "multicustomdirs", arg ) ) {
		pPrefs->multivhosts = 4;
	} else
	if ( !mystrcmpi( "multiclients", arg ) ) {
		pPrefs->multivhosts = 5;
	} else
	if ( !mystrcmpi( "multimonths", arg ) ) {
		pPrefs->multimonths = 1;
	} else
	if ( !mystrcmpi( "multiweeks", arg ) ) {
		pPrefs->multimonths = 2;
	} else
	if ( !mystrcmpi( "sequencedir", arg ) ) {
		pPrefs->vhost_seqdirs = 1;
	} else
	if ( !mystrcmpi( "multireport_path", arg )  ) {
		if ( count+1 < argc ) mystrcpy( pPrefs->multireport_path, argv[count+1] );
	} else
	if ( !mystrcmpi( "VDsortby", arg )  ) {
		if ( count+1 < argc && !myisdigit( *argv[count+1]) ) 
		{
			if ( !mystrcmpi( "name", argv[count+1] ) )	pPrefs->VDsortby = SORT_NAMES;
			if ( !mystrcmpi( "none", argv[count+1] ) )	pPrefs->VDsortby = SORT_NAMES;
			if ( !mystrcmpi( "byte", argv[count+1] ) )	pPrefs->VDsortby = SORT_SIZES;
			if ( !mystrcmpi( "reqs", argv[count+1] ) )	pPrefs->VDsortby = SORT_REQUESTS;
			if ( !mystrcmpi( "hits", argv[count+1] ) )	pPrefs->VDsortby = SORT_REQUESTS;
			if ( !mystrcmpi( "page", argv[count+1] ) )	pPrefs->VDsortby = SORT_PAGES;
			if ( !mystrcmpi( "date", argv[count+1] ) )	pPrefs->VDsortby = SORT_DATE;
			ret = 2;
		} else {
			pPrefs->VDsortby = myatoi( argv[count+1] );
		}
	} else
	if ( !mystrcmpi( "sessionhistory", arg ) ) {
		pPrefs->stat_sessionHistory = 1;
	} else
	if ( !mystrcmpi( "nosessionhistory", arg ) || !mystrcmpi( "sessionhistoryoff", arg ) ) {
		ConfigSetStatOff( pPrefs, "meanpath" );
		ConfigSetStatOff( pPrefs, "sessiondist" );
		ConfigSetStatOff( pPrefs, "clientstream" );
		ConfigSetStatOff( pPrefs, "usersstream" );
		ConfigSetStatOff( pPrefs, "circulation" );
		ConfigSetStatOff( pPrefs, "pagesleast" );
		ConfigSetStatOff( pPrefs, "pageslasthistory" );
		ConfigSetStatOff( pPrefs, "keyvisitors" );
		ConfigSetStatOff( pPrefs, "keypageroute" );
		ConfigSetStatOff( pPrefs, "keypageroutefrom" );
	} else
	if ( !mystrcmpi( "realtimeinterval", arg ) || !mystrcmpi( "daemontime", arg )  ) {
		if ( count+1 < argc ) pPrefs->live_sleeptype = atoi( argv[count+1] );
	} else
	//-----------------------------	Custom log file Format READ
	if ( !mystrcmpi( "usecustomformat", arg )  ) {
		pPrefs->custom_format = TRUE;
	} else
	if ( !mystrcmpi( "custom_seperator", arg )  ){
		if ( count+1 < argc ){
			if ( myisdigit( *argv[count+1] ) ) pPrefs->custom_seperator = myatoi( argv[count+1] );
		}
	} else
	if ( !mystrcmpi( "custom_dateformat", arg )  ){
		if ( count+1 < argc )	pPrefs->custom_dateformat = myatoi( argv[count+1] );
	} else
	if ( !mystrcmpi( "custom_timeformat", arg )  ){
		if ( count+1 < argc )	pPrefs->custom_timeformat = myatoi( argv[count+1] );
	} else
	if ( !mystrcmpi( "custom_dataIndex", arg )  ){
		if ( count+2 < argc ){
			long i;
			i = myatoi( argv[count+1] );
			pPrefs->custom_dataIndex[i] = myatoi( argv[count+2] );
		}
	} else
	if ( !mystrcmpi( "notify_type", arg ) ) {
		if ( count+1 < argc ) pPrefs->notify_type = atoi( argv[count+1] );
	} else
	if ( !mystrcmpi( "notify_dest", arg )  ) {
		if ( count+1 < argc ) mystrcpy( pPrefs->notify_destination, argv[count+1] );
	} else
	//-----------------------------
	if ( !mystrcmpi( "remotelogin_port", arg )   ){
		if ( count+1 < argc )	pPrefs->remotelogin_port = myatoi( argv[count+1] );
	} else
	if ( !mystrcmpi( "remotelogin_passwd", arg )  ){
		if ( count+1 < argc ) {
			mystrcpy( pPrefs->remotelogin_passwd, argv[count+1] );
			CryptPasswd( pPrefs->remotelogin_passwd );
		}
	} else
	if ( !mystrcmpi( "remotelogin_pass", arg )  ){
		if ( count+1 < argc ) {
			mystrcpy( pPrefs->remotelogin_passwd, argv[count+1] );
		}
	} else
	if ( !mystrcmpi( "remoteloginon", arg ) ||
		 !mystrcmpi( "server", arg ) ||
		 !mystrcmpi( "daemon", arg )){
		pPrefs->remotelogin_enable = 1;
	} else
	//-----------------------------
	if ( !mystrcmpi( "page.ext", arg )  ){
		if ( *oarg == '+' )
			AddExtStrings( pPrefs->pageStr, argv[count+1] );
		else
			LoadExtStrings( pPrefs->pageStr, argv[count+1] );
	} else
	if ( !mystrcmpi( "download.ext", arg )  ){
		if ( *oarg == '+' )
			AddExtStrings( pPrefs->downloadStr, argv[count+1] );
		else
			LoadExtStrings( pPrefs->downloadStr, argv[count+1] );
	} else
	if ( !mystrcmpi( "audio.ext", arg )  ){
		if ( *oarg == '+' )
			AddExtStrings( pPrefs->audioStr, argv[count+1] );
		else
			LoadExtStrings( pPrefs->audioStr, argv[count+1] );
	} else
	if ( !mystrcmpi( "video.ext", arg )  ){
		if ( *oarg == '+' )
			AddExtStrings( pPrefs->videoStr, argv[count+1] );
		else
			LoadExtStrings( pPrefs->videoStr, argv[count+1] );
	} else
	//-----------------------------
	if ( !mystrcmpi( "stat", arg )  ){
		long data, i, index;
		if ( argc > 5 ){
			index = ConfigNameFindSettings( pPrefs, argv[1], &data );

			data = 0;
			if ( index >= 0 )
			{
				if ( *line == '+' )		{ STATSET( data, 1 ); }

				if ( *argv[2] == '1' ) { STATSETT( data, 1 ); }
				if ( !mystrcmpi( "tableon", argv[2] )) { STATSETT( data, 1 ); }
				if ( *argv[3] == '1' ) { STATSETG( data, 1 ); }
				if ( !mystrcmpi( "graphon", argv[3] )) { STATSETG( data, 1 ); }
				i = myatoi( argv[4] );	STATSETSORT( data, i );
				i = myatoi( argv[5] );	STATSETTAB( data, i );
				if ( argc > 6 && !mystrcmpi( "helpon", argv[6] )) { STATSETC( data, 1 ); }
				ConfigSetSettings( pPrefs, index, data );
			}
			ret = 5;
		}
	} else
	// Read the settings style we wish to use, ie if in streaming or web mode.
	if ( !mystrcmpi( "stat_style", arg )  ){
		if ( count+1 < argc )
		{
			if ( !myisdigit( *argv[count+1]) ) 
			{
				if ( !mystrcmpi( "web", argv[count+1] ) )		pPrefs->stat_style = WEB;
				if ( !mystrcmpi( "streaming", argv[count+1] ) )	pPrefs->stat_style = STREAM;
				if ( !mystrcmpi( "firewall", argv[count+1] ) )	pPrefs->stat_style = FIRE;
			} else
				pPrefs->stat_style = myatoi( argv[count+1] );
		}
	}

	//-----------------------------
	else
	if ( !mystrcmpi( "editpath", arg ) || !mystrcmpi( "vhostpath", arg ) ) {
		if ( count+2 < argc ) {
			char *host=NULL, *path=NULL;

			// accept commands from the string line, in the format "editpath blah.com MyName" or "editpath blah.com=MyName"
			if ( line )
			{
				host = mystrchr( line, ' ' );
				if ( host )
				{
					host++;
					path = mystrchr( host, ' ' );

					if ( !path )
						path = mystrchr( host, '=' );

					if ( path )
						*path++ = 0;
				}
			} else
			{
				// hand command ARG of A=B, if there is no equal sign then take then next argument of A B
				host = argv[count+1];
				if ( (path = strchr( host, '=' )) )
					*path++;
				else
					path = argv[count+2];
			}

			AddEditPath( &pPrefs->EditPaths, host, path );
			ret = 2;
		}
#if DEF_UNIX
		// Should really be for all...
		else
		if ( count+1 < argc ) 
		{
			char *host=NULL, *path=NULL;
			host = argv[count+1];
			if ( (path = strchr( host, '=' )) )
			{
				*path++;
				AddEditPath( &pPrefs->EditPaths, host, path );
			}
		}
#endif
	} else
	if ( !mystrcmpi( "theme", arg )  ) {
		if ( count+1 < argc ) {
			long default_schemes[][6] = {
				// Name,    Title,    Header,   Line Even, Line Odd, Others,  Total
				{ -1, -1, -1, -1, -1, -1 } ,
				{ 0x999999, 0xcccccc, 0xf4f4f4, 0xcccccc, 0x99cccc, 0x99cccc } ,
				{ 0x99AACC, 0x888888, 0xffffff, 0xdddddd, 0xd0c0b0, 0xa0b0c0 } ,
				{ 0xa0a0a0, 0x808080, 0xffffff, 0xdddddd, 0xd0d0d0, 0xb0b0b0 } ,
				{ 0x3080c0, 0x90d0f0, 0xdfefff, 0xb0c0d0, 0xd0c0ff, 0x50a0f0 } ,
				{ 0xff0030, 0xffc030, 0xffffef, 0xddddad, 0x80f030, 0xccffcc } ,
				{ 0xfafafa, 0xffcfff, 0xffffff, 0xffffff, 0xffcfff, 0xfafafa } ,
				{ 0xfafafa, 0x99ccff, 0xccccff, 0x99ccff, 0x99ccff, 0x3399cc } ,
				{ 0,0,0,0,0,0 } };

			if ( isdigit( *argv[count+1]) )
				pPrefs->theme = atoi( argv[count+1] );

			long i = 0, rgb = 0;

			for( i=0;i<6;i++){
				rgb = default_schemes[pPrefs->theme][i];
				if ( rgb == -1 )
					rgb = (rand() * 255)<<16 | (rand() * 255)<<8 | (rand() * 255);
				pPrefs->RGBtable[i+1] = rgb;
			}
		}
	} else
	if ( !mystrcmpi( "head", arg )  ) {
		if ( count+1 < argc ){
			FileToMem( argv[count+1], pPrefs->html_head, MAX_HTMLSIZE );
			OutDebugs( "new header file is %s", argv[count+1] );
		}
	} else
	if ( !mystrcmpi( "foot", arg ) ) {
		if ( count+1 < argc ){
			FileToMem( argv[count+1], pPrefs->html_foot, MAX_HTMLSIZE );
			OutDebugs( "new footer file is %s", argv[count+1] );
		}
	} else
	if ( !mystrcmpi( "htmlhead", arg ) ) {
		doing_htmlhead = 1;
		pPrefs->html_head[0] = 0;
	} else
	if ( !mystrcmpi( "htmlfoot", arg )  ) {
		doing_htmlfoot = 1;
		pPrefs->html_foot[0] = 0;
	} else
	if ( !mystrcmpi( "PATH", arg )  ) {
		doing_editpath = 1;
		LFcount = 0;
	} else
	if ( !mystrcmpi( "rgb", arg )   ) {
		long x = atoi( argv[count+1] ), rgb;
		sscanf( argv[count+2], "%x", &rgb );
		pPrefs->RGBtable[ x ] = rgb;
	} else
	if ( !mystrcmpi( "graphrgb", arg )   ) {
		long x = atoi( argv[count+1] ), rgb;
		sscanf( argv[count+2], "%x", &rgb );
		pPrefs->multi_rgb[ x ] = rgb;
	} else
	if ( !mystrcmpi( "graphcolor", arg )   ) {
		ReadGraphColors( argv, count );
	} else
	if ( !mystrcmpi( "graphtypecolor", arg )   ) {
		ReadGraphTypeColors( argv, count );
	} else
	if ( !mystrcmpi( "allhistoriesoff", arg ) || !mystrcmpi( "nohistories", arg ) ) {
		SetAllHistories_OnOff( pPrefs, 0 );
	} else
	if ( !mystrcmpi( "ver", arg ) || !mystrcmpi( "-version", arg ) ) {
#if (DEF_UNIX)
		ShowVersionInfo();
#endif
		exit(0);
	} else
#ifndef DEF_FREEVERSION
	if ( !mystrcmpi( "reg", arg ) ) {
		DoRegistrationAgain();
		exit(0);
	} else
#endif
	if ( !mystrcmpi( "q", arg ) || !mystrcmpi( "quiet", arg ) ) {
		gVerbose = 0;
	} else
	if ( !mystrcmpi( "q1", arg ) ) {
		gVerbose = 1;
	} else
	if ( !mystrcmpi( "q2", arg ) ) {
		gVerbose = 2;
	} else
	if ( !mystrcmpi( "debug", arg ) ) {
		gDebugPrint = 1;
	} else
	if ( !mystrcmpi( "debugprint", arg ) ) {
		gDebugPrint = 2;
	} else 
	{
		unknown2 = TRUE;
	}
	
	if ( unknown && unknown2 )
	{
#if DEF_UNIX
		if ( !plus && !minus ){
			if ( strstr( arg, ".log" ) )
				AddLogFromArg( arg );
			else
			if ( strstr( arg, "_log" ) )
				AddLogFromArg( arg );
			else
			if ( strstr( arg, ".gz" ) )
				AddLogFromArg( arg );
			else
			if ( strstr( arg, ".bz2" ) )
				AddLogFromArg( arg );
		} else
#endif
		if ( gVerbose ){
			if ( plus || minus  ){
				char tmp[MAXFILENAMESSIZE];
				sprintf ( tmp, "please try '%s -h' for a command syntax overview\n", argv[0] );
				fwrite( tmp, mystrlen(tmp), 1, stdout );
				sprintf ( tmp, "unknown option (%s)\n", oarg );
				fwrite( tmp, mystrlen(tmp), 1, stdout );
			}
		}
		//exit(1);
	}

	//printf( "DONE\n");	
 	return ret;
}


long DoReadFile(void *p, char *filename, long length)
{
	FILE *fp;
	long result;

	if ( (fp = fopen( filename, "r" ))) {
		result = fread(p, 1, length, fp);              // get info
        fclose(fp);
	}
    return result;  // let them know how long the file is (can only be shorter, of course)
}



/* write our data to the pref file */
long DoSavePrefsFile( char *filename , Prefs *p )
{
	FILE *fp;
	long result = 0;

	if ( (fp = fopen( filename, "w" )) ) {
		p->prefVER = CURRENT_PREF_VERSION;
		DoSaveAsciiPrefs( fp, p );
		fclose(fp);
	} else
		result = 1;

	return result;
}
















long GetHTMLColor( char *p )
{
	if ( p ){
		char *t;

		t = (char*)mystrchr( p,'#');
		if ( t ){
			return (long)HexStr2Ptr( t+1 );
		} else {
			if ( strstri( p, "white" ) ) return 0xffffff;
			if ( strstri( p, "black" ) ) return 0x0;
			if ( strstri( p, "red" ) ) return 0xff0000;
			if ( strstri( p, "green" ) ) return 0xff00;
			if ( strstri( p, "blue" ) ) return 0xff;
		}
	}
	return 0;
}


void GetHTMLColors( Prefs *prefs )
{
	char *p;
	char txt[256];

	if ( prefs->html_head[0] ){
		p = strstri( prefs->html_head, "<BODY" );
		if ( p ){
			strcpyuntil( txt, p, '>' );

			if ( p = strstri( txt, "bgcolor" ) ) prefs->RGBtable[7] = GetHTMLColor(p);
			if ( p = strstri( txt, "text" ) ) prefs->RGBtable[8] = GetHTMLColor(p);
			if ( p = strstri( txt, "link" ) ) prefs->RGBtable[9] = GetHTMLColor(p);
			if ( p = strstri( txt, "alink" ) ) prefs->RGBtable[10] = GetHTMLColor(p);
			if ( p = strstri( txt, "vlink" ) ) prefs->RGBtable[11] = GetHTMLColor(p);
		}
	}
}


long SetHTMLColors( Prefs *prefs )
{
	char tmp[MAX_HTMLSIZE], newbody[256], *s, *s2, *d;

	sprintf( newbody, "<BODY bgcolor=#%06x text=#%06x link=#%06x alink=#%06x vlink=#%06x>",
		prefs->RGBtable[7], prefs->RGBtable[8], prefs->RGBtable[9],
		prefs->RGBtable[10], prefs->RGBtable[11] );

	mystrcpy( tmp, prefs->html_head );

	s = strstri( tmp, "<BODY" );
	if ( s ){
		d = strstri( prefs->html_head, "<BODY" );
		s2 = mystrchr( s, '>' );
		if( s2 ){
			mystrcpy( d, newbody );
			strcat( d, s2+1 );
		}
	}
	return 1;
}



