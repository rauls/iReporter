/*
**	File:		config_struct.h
**
**	Author:		Raul Sobon
**
**	Comments:	Config Structure definition
*/

#ifndef	__CONFIG_clips_
#define	__CONFIG_clips_

#include "FWA.h"

#include "Myansi.h"		// for bool which is defined only for c source files
#include "datetime.h"
#include "editpath.h"
#include "PDFSettingsC.h"

#ifdef DEF_APP_FIREWALL
	#define	MAX_DOMAINS		10000		// 1million will work if you have 8gig ram
#else
	#ifdef DEF_FULLVERSION
		#define	MAX_DOMAINS		1000
	#else
		#define	MAX_DOMAINS		10
	#endif
#endif
#define	MAX_DOMAINSPLUS		MAX_DOMAINS+10


/* sorting type for tables */
#define	SORT_NONE				-1
#define	SORT_NAMES				0
#define	SORT_SIZES				1
#define	SORT_BYTES				1
#define	SORT_REQUESTS			2
#define	SORT_PAGES				3
#define	SORT_ERRORS				4
#define	SORT_VISITS				5
#define	SORT_SESSIONS			5
#define	SORT_COUNTER			6
#define	SORT_COUNTER2			7
#define	SORT_COUNTER3			8
#define	SORT_COUNTER4			9
#define	SORT_BYTESIN			10
#define	SORT_DATE				11
#define	SORT_NUMBER				12
#define	SORT_VISITIN			13
#define	SORT_LASTDAY			14
#define	SORT_BYTESFULL			15
#define	SORT_COUNTER4THENERRORS	16

/* sort format that is graphed */
#define TYPE_USEMAIN		SORT_NONE
#define TYPE_BYTES			SORT_BYTES
#define	TYPE_BYTESFULL		SORT_BYTESFULL
#define TYPE_REQUESTS		SORT_REQUESTS
#define TYPE_PAGES			SORT_PAGES
#define TYPE_SESSIONS		SORT_VISITS
#define TYPE_COUNTER		SORT_COUNTER
#define TYPE_COUNTER2		SORT_COUNTER2
#define TYPE_COUNTER3		SORT_COUNTER3
#define TYPE_COUNTER4		SORT_COUNTER4
#define TYPE_ERRORS			SORT_ERRORS
#define TYPE_BYTESIN		SORT_BYTESIN


#define	GRAPH_WIDTH				600	//580
#define	GRAPH_HEIGHT			430	//`420

/* graph visual types */
#define GRAPH_VBAR			0
#define GRAPH_HBAR			1
#define GRAPH_LINE			2
#define GRAPH_PIE			3
#define GRAPH_MULTILINE		4
#define GRAPH_MULTI3D		5
#define GRAPH_MULTIVBAR4	6
#define	GRAPH_MULTIVHOST3D	7
#define	GRAPH_SCATTER		8
#define	GRAPH_WORLD			9

/* graph visual styles */
#define	GRAPH_2DSTYLE		0
#define	GRAPH_3DSTYLE		1

#define	GRAPH_CUSTOMCOLOR	0
#define	GRAPH_WEBCOLOR		1

#define	REPORT_HTML			0
#define	REPORT_RTF			1
#define	REPORT_COMMA		2
#define	REPORT_PDF			3

#define	FORMAT_HTML			0
#define	FORMAT_HTMLFRAMED	1
#define	FORMAT_PDF			2
#define	FORMAT_RTF			3
#define	FORMAT_COMMA		4
#define	FORMAT_EXCEL		5

#define GRAPH_GIF			0
#define GRAPH_JPEG			1
#define GRAPH_PNG			2
#define GRAPH_BMP			3
#define GRAPH_SVG			4		// SVG vector format
#define GRAPH_JPEG2000		5		// New jpeg 2000 format.

#define	MAX_HTMLSIZE		6000

// ~ QCM: 47001  , max filters set to 500
#define MAX_FILTERUNITS		500	
#define MAX_FILTERSIZE		160

enum
{
	kUnknown = 0,
	kGzip,
	kStuffIt,
	kZip
};


// ----------------- Graph base colors defined
enum {
	GRAPHCOLORS_VBAR,
	GRAPHCOLORS_HBAR,
	GRAPHCOLORS_LINE,
	GRAPHCOLORS_PIE,
	GRAPHCOLORS_MULTI3D,
	GRAPHCOLORS_MULTIVBAR,
	GRAPHCOLORS_SCATTER,
	GRAPHCOLORS_MAX
};

typedef struct {
	long label;
	long grid;
	long base;
	long border_hilight;
	long border_lolight;
	long border;

} GraphColors, *GraphColorsPtr;
// --------------------------------------------



// D,pattern  (D=which database list, 0...9(
typedef struct FilterData {
	char	filterIn[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	filterInTot;

	char	router[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	routerTot;
	char	org[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	orgTot;
	char	group[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	groupTot;
	char	advert[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	advertTot;
	char	advert2[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	advert2Tot;
	char	vhostmap[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	vhostmapTot;
	char	referralMap[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	referralTot;
	char	notify[32][MAX_FILTERSIZE];
	long	notifyTot;
	char	keyvisitors[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	keyvisitorsTot;
} Filters, *FiltersP;


#define		DOMAINNAME_LENGTH		255
#define		KEYPAGE_LENGTH			255
#define		NUMBER_OF_KEYPAGES		32
#define		MAX_KEYPAGE_ROWS		500
#define		MAX_KEYPAGE_DEPTH		50

#define		DNR_LOOKUPSON			(0x01)
#define		DNR_CACHEONLY			(0x02)
#define		DNR_BACKGROUNDMODE		(0x04)
#define		DNR_DYNAMICMODE			(0x08)

#define		DATE_WIDTH				10
#define		BILLINGTITLE_WIDTH		127
#define		CURRENCY_WIDTH			7

typedef	struct	BillingSetupStruct
{
	unsigned char	bEnabled;
	unsigned char	ucBillingPeriod;
	unsigned char	ucOutputFormat;
	char			szStartDate[DATE_WIDTH+1];
	char			szEndDate[DATE_WIDTH+1];
	char			szTitle[BILLINGTITLE_WIDTH+1];
	char			szCurrencySymbol[CURRENCY_WIDTH+1];
	unsigned char	bHeaderCustomerName;
	unsigned char	bHeaderCustomerID;
	unsigned char	bHeaderMBTransfered;
	unsigned char	bHeaderMBAllowance;
	unsigned char	bHeaderMaxConnections;
	unsigned char	bHeaderExcessCharges;
	unsigned char	bHeaderTotalCharges;
}	BillingSetupStruct;


#define		BILLINGCUSTOMER_NAME_WIDTH			63
#define		BILLINGCUSTOMER_ID_WIDTH			32
#define		BILLINGCUSTOMER_URL_WIDTH			255
#define		BILLINGCUSTOMER_CHARGE_WIDTH		15
#define		BILLINGCUSTOMER_PERIOD_WIDTH		5

typedef	struct	BillingCustomerStruct
{
	struct	BillingCustomerStruct*	pNext;
	char			szCustomer[BILLINGCUSTOMER_NAME_WIDTH+1];
	char			szCustomerID[BILLINGCUSTOMER_ID_WIDTH+1];
	char			szURL[BILLINGCUSTOMER_URL_WIDTH+1];
	char			szFixedCharge[BILLINGCUSTOMER_CHARGE_WIDTH+1];
	char			szFixedPeriod[BILLINGCUSTOMER_PERIOD_WIDTH+1];
	unsigned char	ucFixedGroup;
	unsigned char	bExcessCharge;
	char			szExcessCharge[BILLINGCUSTOMER_CHARGE_WIDTH+1];
	char			szExcessPeriod[BILLINGCUSTOMER_PERIOD_WIDTH+1];
	unsigned char	ucExcessGroup;
}	BillingCustomerStruct;

typedef	struct	BillingChargesStruct
{
	size_t		nCustomers;
	BillingCustomerStruct*	pFirstCustomer;
}	BillingChargesStruct;


struct App_config {
	char	prefID[4];				/* string for version ... e.g. "4.5" */
	short	prefVER;
	/*--- GENERAL */
	char	report_title[128];
	char	outfile[256];
	unsigned char firstDayOfWeek;	/* index of first day of week for weekly trafic reports: 0=Sunday, 1=Monday, 2=Tuesday etc */
	time_t	startTimeT;				/* start date of log analysis period + 00:00:00 */
	time_t	endTimeT;				/*   end date of log analysis period + 23:59:59 */
	long	stat_style;				// we can define if we want WEBSTATS/PROXYSTATS/STREAMING/FIREWALL reports.
	char	alldates;

	char	dnslookup;				/* 0=none,1=full, 2=quickndirty */
	short	dnr_ttl;				/* default 45 */
	short	dnr_expire;				/* default 90 days */
	long	dnsAmount;				/* amount of concurrent DNS lookups, 50 is fine, 3000 is pushing it */
	short	ftp_passive;
	short	multitask;
	// ------ old vars
	short	sortby,					/* global stat sort */
			graph_type,				/* global graph sort */
			stat_depth;				/* global table size */
			
	short	background;

	char	language[64];			/* id , 0=none, 1=german, 2=more, 3= etc... */
	char	ignore_selfreferral;
	char	report_format;			/* 0=FORMAT_HTML, 1=FORMAT_HTMLFRAMED, 2=,FORMAT_PDF, 3=FORMAT_RTF, 4=FORMAT_COMMA, 5=FORMAT_EXCEL */
	char	image_format;			/* 0=GRAPH_GIF, 1=GRAPH_JPEG, 2=GRAPH_PNG, 3=GRAPH_BMP, 4=quicktime???, 5=vrml??? */
	char	image_depth;			/* not used */

	short	live_sleeptype;			/* timeSec to sleep between udpates (secs) */
	char	vhost_seqdirs;
	char	VDsortby;				/* 0=none,1=bytes,2=req */
	short	multidomains;			/* maximum multidomain reports allowed from X input logs */
									/* GUI="Combined report" or "Multi-domain report" */
	char	multivhosts;			/* multiple virtual hosts per log file [1 or 0]*/
	char	multimonths;			/* 1=months, 2=weeks */
	char	multireport_path[256];
	char	session_timewindow;		/* session time window in minutes, 1..256 allowed , default 5 */
	char	clusteringActive;		/* for 4.5, use this as a flag, need to convert 4.0 settings files */
	short	numClustersInUse;		/* for 4.5, use this to hold the actual count */
	long	dollarpermeg;

	char	ignorecase;				/* ignore case for all urls */
	char	ignore_usernames;
	char	ignore_bookmarkreferral;
	char	filter_robots;
	char	filter_zerobyte;		/* filter zero byte transfers! YES/NO */
	char	headingOnLeft;			/* place headin on left side of data columns */

	short	sessionLimit;			/* # of sessions to show in the client Click Stream */
	long	time_adjust;			

	char	html_frames;			/* use frames in html index page */
	char	html_onefile;
	char	html_quickindex;
	char	html_css;

	char	graph_style;			/* 1=3d bars, 2=2d bars */
	char	corporate_look;
	char	graph_wider;			  /* wider format graphs */
	char	paletteType;			/* 0=custom, 1=web */
	char	shadow;
	char	footer_label;			/* 0=false (no footer label), !0=true (Want footer Label). */
	char	footer_label_trailing;	/* 0=above the 'Average/subtotal/total' rows, 1=last row. */

	char	page_remotetitle;		/* 1 = get remote html title, 0=nope */
	char	write_timestat;			/* time processing information */
	char	forceddmm;				/* force ddmm date on digit dates if cant be auto detected */
	char	head_title;
	char	useCGI;
	char	bandwidth;				/* write bandwith stats or not */
	short	database_active;	    /* save data into an FWA database file */
	short	database_extended;		/* if true, then database is an extended (.fxdb) one otherwise it's a compact (.fdb) one */
	char	database_file[260];		/* FWA database file name - Do not reduce size!! */
	short	database_no_report;		/* if true, a report will NOT be produced after log file data has been added to the database */
	short	database_excluded;	    /* if true, the report will only include the processed log files */
	char	graph_ascii;
	short	graph_width;
	short	graph_height;
	char	stat_sessionHistory;		/* record pages for each client */
	char	streaming;
	short	server_bandwidth;
/* STATISTICS
* 0x76 54 32 10
* byte 3 = Sort Type
* byte 2 = Table Size
* byte 1 = res
* byte 0 = flags , %76543210,
	0=Stat on, 
	1=Table 
	2=Graph
	3=Comment
	4=Thumbnail Image



*
*/

// Bit definitions

#define		BIT_STAT			0
#define		BIT_TABLE			1
#define		BIT_GRAPH			2
#define		BIT_COMMENT			3
#define		BIT_THUMB			4
#define		BIT_RES1			5
#define		BIT_RES2			6
#define		BIT_REPORTGEN		7

// Bit masks
#define	MASK_STAT					(1<<BIT_STAT)
#define	MASK_TABLE					(1<<BIT_TABLE)
#define	MASK_GRAPH					(1<<BIT_GRAPH)
#define	MASK_COMMENT				(1<<BIT_COMMENT)
#define	MASK_THUMB					(1<<BIT_THUMB)
#define	MASK_REPORTGEN				(1<<BIT_REPORTGEN)

// Set flag definitions
#define	STATSETSORT(x,v)			(x = (x&0x03FFFFFF)|(v<<26))
#define	STATSETTAB(x,v)				(x = (x&0xFC00FFFF)|(v<<16))
#define	STATSET(x,v)				(x = (x&0xFFFFFFFE)|(v))
#define	STATSETT(x,v)				(x = (x&0xFFFFFFFD)|(v<<1))
#define	STATSETG(x,v)				(x = (x&0xFFFFFFFB)|(v<<2))
#define	STATSETC(x,v)				(x = (x&0xFFFFFFF7)|(v<<3))
#define	STATSETTHUMB(x,v)			(x = (x&0xFFFFFFEF)|(v<<4))
#define	STATSETDONE(x,v)			(x = (x&0xFFFFFF7F)|(v<<BIT_REPORTGEN))		// this signifies the report has been made...

// Read flag definitions
#define	TABSORT(x)					(x>>26)
#define	TABSIZE(x)					((x&0x03ff0000)>>16)
#define	STAT(x)						(x&1)				// To be removed and replaced by line below
#define	REPORT_CHECK_ON(x)			(x&1)				// Check if the GUI has the report's checkbox checked or not...
#define	TABLE(x)					((x&0x02)==0x02)	// To be removed and replaced by line below
#define	TABLE_ON(x)					((x&0x03))			// ie ((x&1) && (x&2)) // Would we like to produce a table, so report has to be turned on
#define	GRAPH(x)					((x&0x04)==0x04)	// To be removed and replaced by line below
#define	GRAPH_ON(x)					((x&0x05))			// ie ((x&1) && (x&4)) // Would like to produce a graph, so report has to be turned on
#define	COMMENT(x)					((x&0x08)==0x08)	// To be removed and replaced by two lines below
#define	THUMB_ON(x)					((x&0x10)==0x10)	// thumb image.
#define	TABLE_COMMENT_ON(x)			(x&11)				// ie ((x&1) && (x&2) && (x&8)) // Would like to produce table comment, so table has to be on
#define	GRAPH_COMMENT_ON(x)			(x&21)				// ie ((x&1) && (x&4) && (x&16)) // Would like to produce graph comment, so graph has to be on
#define	ANYSTAT(x)					((x&0x06) && (x&1))	// To be removed and replaced by line below
#define	REPORT_TOBE_GENERATED(x)	((x&0x06) && (x&1))	// Only generate reports if report is turned on, and a graph or table
#define	REPORT_WAS_GENERATED(x)		(x&MASK_REPORTGEN)


	long
			stat_traffic,
			stat_diagnostics,
			stat_streaming,
			stat_livestreaming,
			stat_server,
			stat_multimedia,
			stat_demographics,
			stat_referrals,
			stat_systems,
			stat_advertizing,
			stat_billing,
			stat_marketing,
			stat_firewall;

	long	stat_vhost,
			stat_summary,
			stat_hourly,
			stat_hourlyHistory,
			stat_daily,
			stat_recentdaily,
			stat_weekly,
			stat_weekdays,
			stat_monthly,

			stat_errors,
			stat_errorsHistory,
			stat_errorurl,
			stat_errorurlHistory,

			stat_pages,
			stat_pagesHistory,
			stat_pagesLeastVisited,
			stat_pagesfirst,
			stat_pagesfirstHistory,
			stat_pageslast,
			stat_pageslastHistory,
			stat_redirects,				// redirected pages.
			stat_dir,
			stat_topdir,
			stat_topdirHistory,
			stat_files,
			stat_meanpath,
			stat_groups,
			stat_groupsHistory,
			stat_download,
			stat_downloadHistory,
			stat_upload,
			stat_uploadHistory,
			stat_type,

			stat_client,
			stat_clientStream,			/* show pages usage per client */
			stat_clientHistory,
			stat_user,
			stat_userStream,
			stat_userHistory,
			stat_seconddomain,
			stat_country,
			stat_regions,
			stat_orgs,
			stat_sessionScatter,
			stat_robot,
			stat_robotHistory,

			stat_refer,
			stat_refersite,
			stat_refersiteHistory,
			stat_searchstr,
			stat_searchstrHistory,
			stat_searchsite,
			stat_searchsiteHistory,

			// Web Media
			stat_audio,
			stat_audioHistory,
			stat_video,
			stat_videoHistory,
			stat_mediatypes,

			stat_mplayers,
			stat_mplayersHistory,
			stat_mplayersVSos,
			stat_browsers,			/* optional stats */
			stat_browsersHistory,
			stat_browserVSos,
			stat_opersys,
			stat_opersysHistory,

			stat_advert,
			stat_advertHistory,
			stat_advertcamp,
			stat_advertcampHistory,

			stat_billingCustomers,

			stat_circulation,
			stat_loyalty,
			stat_timeon,

			// Streaming stuff.
			stat_clipserrors,
			stat_clipsfailed,
			stat_clipsfailedHistory,
			stat_clipspacketloss,
			stat_clipslossvsrate,
			stat_clipssecsbuffered,
			stat_clipshighiquality,
			stat_clipslowquality,

			stat_clipsvideo,
			stat_clipsvideoHistory,
			stat_clipsvideorates,
			stat_clipsvideoviews,
			stat_clipsaudio,
			stat_clipsaudioHistory,
			stat_clipsaudiorates,
			stat_clipsaudioviews,

			stat_livevideo,
			stat_livevideoHistory,
			stat_livevideorates,
			stat_livevideoviews,
			stat_liveaudio,
			stat_liveaudioHistory,
			stat_liveaudiorates,
			stat_liveaudioviews,

			stat_clips,
			stat_clipsHistory,
			stat_clipsleast,
			stat_clipspercent,
			stat_clipsmaxconcurrent,
			stat_clipsforwards,
			stat_clipsrewinds,
			stat_clipscomplete,
			stat_clipsvideocodecs,
			stat_clipsaudiocodecs,

			stat_cpu,			// streaming
			stat_lang,			// streaming

			// Firewall stuff
			stat_sourceaddress,
			stat_protSummary,
			stat_protHTTP,
			stat_protHTTPS,
			stat_protMail,
			stat_protFTP,	
			stat_protTelnet,
			stat_protDNS,	
			stat_protPOP3,
			stat_protReal,
			stat_protOthers,

			stat_keyvisitor,
			stat_keypageroute,

			stat_brokenLinks,

			// Cluster settings
			stat_cluster,
			stat_clusterServers,
			stat_clusterServersHistory,

			stat_unrecognizedagents,
			stat_keypageroutefrom,

			stat_end;

	char 	siteurl[DOMAINNAME_LENGTH+1];
	char	defaultindex[64];
	char	cookievar[128];
	char	urlsessionvar[128];
	char	searchfor[128];
	char	replacewith[128];

	char	retain_variablelist[256];
	short	retain_variable;
	short	theme;

	char	html_head[MAX_HTMLSIZE];
	char	reserved_space1[MAX_HTMLSIZE];
	
	char	html_foot[MAX_HTMLSIZE];

	char	html_font[128];
	short	html_fontsize;

	long	RGBtable[16];		// table colours
	long	multi_rgb[32];		// 10 colors for general highlighting of top 10 stuff


	GraphColors	graphcolors[GRAPHCOLORS_MAX];

	short	preproc_on;
	char	preproc_luser[32]; //dw
	char	preproc_lpass[32]; //dw
	char	preproc_lsite[256]; //dw
	char	preproc_location[256];
	char	preproc_downloc[256]; //dw
	short	preproc_delete;
	char	preproc_tmp[256];		// this is the download location

	short	postproc_larchive;		// compress log using...
	short	postproc_lcompressor;   // log compressor 1 = Gzip, 2 = StuffIt, 3 = Zip
	short	postproc_lsea;
	short	postproc_lbin;
	short	postproc_ldel;

	short	postproc_rarchive;		// compress report using...
	short	postproc_rcompressor;   // report compressor 1 = Gzip, 2 = StuffIt, 3 = Zip
	short	postproc_rsea;
	short	postproc_rbin;
	short	postproc_rdel;

	short	postproc_uploadreport;
	short	postproc_deletereport;
	char	postproc_ruser[32]; //dw
	char	postproc_rpass[32]; //dw
	char	postproc_rsite[256]; //dw
	char	postproc_uploadreportlocation[256];

	short	postproc_uploadlog;
	short	postproc_deletelog;
	char	postproc_luser[32]; //dw
	char	postproc_lpass[32]; //dw
	char	postproc_lsite[256]; //dw
	char	postproc_uploadloglocation[256];
	
	// ---------- dynamic sized data to follow... only the above is identical to file structure...
	Filters	filterdata;
	char	clusterNames[MAX_FILTERUNITS][MAX_FILTERSIZE];
	long	clusterNamesTot;

	short	custom_format;				// TRUE or FALSE
	short	custom_seperator;
	short	custom_dataIndex[25];
	short	custom_dateformat;
	short	custom_timeformat;
	short	ignore_ipvhosts;

	char	remotelogin_passwd[32];
	short	remotelogin_enable;
	short	remotelogin_port;

	short	notify_type;				//0=none, 1=screen, 2=email, 3=speech
	char	notify_destination[256];	//count be a mailto: or file????
	char	dnr_cache_file[256];

	short	postproc_emailon;
	char	postproc_email[256];
	char	postproc_emailfrom[128];
	char	postproc_emailmsg[4090];
	char	postproc_emailsub[128];
	char	postproc_emailsmtp[128];


	char	pageStr[256][8];
	char	downloadStr[256][8];
	char	audioStr[256][8];
	char	videoStr[256][8];

	PDFAllSettingsC pdfAllSetC; // All the PDF Settings

	EditPathRecPtr	EditPaths;

	// **************************************************************************
	// Data from/to the "Routes to Key Pages" dialog.
	// **************************************************************************
	char	szToKeyPages[NUMBER_OF_KEYPAGES][KEYPAGE_LENGTH+1];
	size_t	nToKeyPages;
	size_t	nToKeyPageRouteDepth;
	size_t	nToKeyPageMaxRows;

	// **************************************************************************
	// Data from/to the "Routes from Key Pages" dialog.
	// **************************************************************************
	char	szFromKeyPages[NUMBER_OF_KEYPAGES][KEYPAGE_LENGTH+1];
	size_t	nFromKeyPages;
	size_t	nFromKeyPageRouteDepth;
	size_t	nFromKeyPageMaxRows;
#ifdef __cplusplus
	short GetImageFormat()
	{
		if ( report_format == FORMAT_RTF ) // If report format is RTF make image format PNG
			return GRAPH_PNG;
		else if ( report_format == FORMAT_HTML && image_format > GRAPH_PNG )
			return GRAPH_GIF;
		else if ( report_format == FORMAT_PDF )
			return GRAPH_JPEG; // If report format is PDF make image format JPEG

		// Return the existing format
		return image_format;
	};
	bool ShowVirtualHostGraph()
	{
		return multimonths || multivhosts || multidomains;
	}
#endif // __cplusplus

	// *****************************************************************************
	// Billing Setup Data.
	// *****************************************************************************
	BillingSetupStruct		billingSetup;
	BillingChargesStruct	billingCharges;
};

typedef struct App_config FWSettingsRec;
typedef struct App_config Prefs;

#define		CONFIG_SIZE		( sizeof(struct App_config) )
#endif 

