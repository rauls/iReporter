
#ifndef	__CONFIG_H
#define __CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FWA.h"
#include "config_struct.h"
#include "datetime.h"
#include "myansi.h"

extern struct	App_config MyPrefStruct;
extern struct	App_config TempPrefStruct;

#if DEF_MAC
typedef struct App_config	Prefs;
#endif

extern int		gVerbose, gDebugPrint;			// needed externs for all files

extern const char *HTML_EXT;
extern const char *PDF_EXT;
extern const char *RTF_EXT;
extern const char *EXCEL_EXT;
extern const char *COMMA_EXT;

#define REPORT_TYPE_NORMAL	0
#define REPORT_TYPE_XML		1
#define REPORT_DONTDOIT		99

enum {
	CONVERT_NOTHING,
	CONVERT_TOW3C
};


typedef struct {
	char	**fsFile;
	int		logNum;
	char	reportType;
	char	*queryString;

	long	dns_timeout;
	long	dns_retries;
	char	*dns_server;

	long	outputSocket;					//for XML query and also for realtime data viewing in html
	
	struct App_config *prefs;			//optional at the moment.

	int		setup_Flag;
	int		convert_Flag;
	int		post_Flag;
	int		view_Flag;

	void	*updateCallBack;
	void	(*ShowProgressDetail)( long level, long reqs, char *msg, double timesofar  );
} ProcessData, *ProcessDataPtr;

typedef struct {
	long	style;
	char	*title;
	char	*name;
	unsigned int data;
	long	id;
} ConfigNames, *ConfigNamesPtr;


enum {
	ALL,	
	WEB,
	PROXY,
	STREAM,
	FIRE,
	HIDDEN
};


#define	IS_ALL(x)		(x==ALL)
#define	IS_WEB(x)		(x==ALL || x==WEB)
#define	IS_PROXY(x)		(x==ALL || x==WEB || x==PROXY)
#define	IS_STREAM(x)	(x==ALL || x==STREAM)
#define	IS_FIRE(x)		(x==ALL || x==FIRE)


int IsDebugMode( void );
int AddFileToLogQ( char *filename, long index );
int AddLogFile( char *filename, long index );
long SortLogQ( void );


enum {
	VHOST_PAGE,
	TRAFFIC_GROUP,
	HOUR_PAGE,			
	HOURHIST_PAGE,	
	DATE_PAGE,			
	RECENTDATE_PAGE,	
	WEEK_PAGE,
	WEEKDAYS_PAGE,
	SUNDAY_PAGE,		
	MONDAY_PAGE,		
	TUEDAY_PAGE,		
	WEDDAY_PAGE,		
	THUDAY_PAGE,		
	FRIDAY_PAGE,		
	SATDAY_PAGE,		
	MONTH_PAGE,			

	//----------------------------
	DIAGNOSTIC_GROUP,
	ERRORS_PAGE,		
	ERRORSHIST_PAGE,
	ERRORURL_PAGE,		
	ERRORURLHIST_PAGE,
	BROKENLINKS_PAGE,
	EXTBROKENLINKS_PAGE,
	INTBROKENLINKS_PAGE,
	CLIPSERRORS_PAGE,
	CLIPSFAILED_PAGE,
	CLIPSFAILEDHIST_PAGE,
	CLIPSBROKENLINKS_PAGE,
	CLIPSPACKETLOSS_PAGE,
	CLIPSLOSSRATE_PAGE,
	CLIPSSECSBUF_PAGE,
	CLIPSHIGH_PAGE,
	CLIPSLOW_PAGE,

	//----------------------------
	STREAMINGCONTENT_GROUP,
	CLIPSVID_PAGE,
	CLIPSVIDHIST_PAGE,
	CLIPSVIDVIEW_PAGE,
	CLIPSVIDRATES_PAGE,
	CLIPSAUD_PAGE,
	CLIPSAUDHIST_PAGE,
	CLIPSAUDVIEW_PAGE,
	CLIPSAUDRATES_PAGE,

	LIVEVID_PAGE,
	LIVEVIDHIST_PAGE,
	LIVEVIDVIEW_PAGE,
	LIVEVIDRATES_PAGE,
	LIVEAUD_PAGE,	
	LIVEAUDHIST_PAGE,
	LIVEAUDVIEW_PAGE,
	LIVEAUDRATES_PAGE,

	CLIPS_PAGE,
	CLIPSHIST_PAGE,			
	CLIPSLEAST_PAGE,			
	CLIPSPERCENT_PAGE,
	CLIPSMAXCON_PAGE,
	CLIPS_FF_PAGE,
	CLIPS_RW_PAGE,
	CLIPSCOMPLETED_PAGE,
	CLIPSPROTOCOLS_PAGE,
	VIDCODECS_PAGE,
	AUDCODECS_PAGE,



	//----------------------------
	SERVER_GROUP,
	PAGES_PAGE,			
	PAGEHIST_PAGE,		
	PAGESLEAST_PAGE,			
	PAGESFIRST_PAGE,	
	PAGESFIRSTHIST_PAGE,	
	PAGESLAST_PAGE,		
	PAGESLASTHIST_PAGE,		
	REDIRECTS_PAGE,
	DIRS_PAGE,			
	TOPDIRS_PAGE,		
	TOPDIRSHIST_PAGE,	
	FILE_PAGE,			
	MEANPATH_PAGE,		
	KEYPAGEROUTE_PAGE,
	KEYPAGEROUTEFROM_PAGE,
	GROUPS_PAGE,		
	GROUPSHIST_PAGE,		
	UPLOAD_PAGE,		
	UPLOADHIST_PAGE,	
	DOWNLOAD_PAGE,		
	DOWNLOADHIST_PAGE,	
	FILETYPE_PAGE,		
	MEDIATYPES_PAGE,
	DEBUGPAGES_PAGE,

	//----------------------------
	BILLING_GROUP,
	BILLING_PAGE,

	DEMOGRAPHIC_GROUP,
	CLIENT_PAGE,		
	CLIENTSTREAM_PAGE,	
	CLIENTHIST_PAGE,	
	USER_PAGE,
	USERHIST_PAGE,		
	USERSTREAM_PAGE,
	SECONDDOMAIN_PAGE,	
	DOMAIN_PAGE,		
	REGION_PAGE,		
	ORGNAMES_PAGE,
	SESSIONS_PAGE,		
	KEYVISITORS_PAGE,
	
	//----------------------------
	REFERRALS_GROUP,
	REFERURL_PAGE,		
	REFERSITE_PAGE,		
	REFERSITEHIST_PAGE,	
	SEARCHSITE_PAGE,	
	SEARCHSITEHIST_PAGE,
	SEARCHSTR_PAGE,
	SEARCHSTRHIST_PAGE,

	//----------------------------
	MULTIMEDIA_GROUP,
	AUDIO_PAGE,
	AUDIOHIST_PAGE,
	VIDEO_PAGE,
	VIDEOHIST_PAGE,

	//----------------------------
	SYSTEMS_GROUP,
	MPLAYERS_PAGE,
	MPLAYERSHIST_PAGE,
	MPLAYERSOS_PAGE,		
	CPU_PAGE,
	LANG_PAGE,
	BROWSER_PAGE,
	BROWSERHIST_PAGE,
	BROWSEROS_PAGE,		
	OPERSYS_PAGE,		
	OPERSYSHIST_PAGE,
	ROBOT_PAGE,			
	ROBOTHIST_PAGE,
	UNRECOGNIZEDAGENTS_PAGE,

	//----------------------------
	ADVERTISING_GROUP,
	ADVERT_PAGE,	
	ADVERTHIST_PAGE,
	ADVERTCAMP_PAGE,	
	ADVERTCAMPHIST_PAGE,

	//----------------------------
	MARKETING_GROUP,
	CIRC_PAGE,			
	LOYALTY_PAGE,		
	TIMEON_PAGE,		

	//----------------------------
	CLUSTER_GROUP,
	CLUSTER_PAGE,
	CLUSTERHIST_PAGE,

	//----------------------------
	FIREWALL_GROUP,
	SRCADDR_PAGE,		
	PROTSUMMARY_PAGE,
	PROTHTTP_PAGE,		
	PROTHTTPS_PAGE,		
	PROTMAIL_PAGE,		
	PROTFTP_PAGE,		
	PROTTELNET_PAGE,	
	PROTDNS_PAGE,		
	PROTPOP3_PAGE,
	PROTREAL_PAGE,
	PROTOTHERS_PAGE,

	TOTALREPORTS_PAGE

};

long OutDebug( const char *txt );
long OutDebugs( const char *txt, ... );
long LogMessage ( char *filename,char *str, long err );


void CommaToMultiline( char *string, char *dest, long max );
void MultilineToComma( char *string, char *dest, long max );

int myfgets( char *str, long max, FILE *fp );

char **GetHelpText( void );


int ConfigInStyle( Prefs *, long item );
long ConfigFindIndex( long id, long style_mask );
long ConfigNameFindIndex( char *name, long style_mask );
char *ConfigFindSettings( Prefs *, long item, long *data );
char *ConfigSetSettings( Prefs *,long item, long data );
long ConfigNameFindSettings( Prefs *,char *title, long *data );

long ConfigGetSettingsbyStyle( Prefs *pPrefs, long id, long style );
long *ConfigFindSettingsPtrbyStyle( Prefs *pPrefs, long id, long style  );
long *ConfigFindSettingsPtr( Prefs *pPrefs, long id );

const char *ConfigLookupFilename(unsigned long item);
const char *ConfigLookupTitle(unsigned long item);
const char *ConfigLookupName(unsigned long item);

const char *ConfigNameFindFilename( Prefs *pPrefs, char *name );
const char *ConfigFindTitle( Prefs *pPrefs, long id );

void CopyPrefs( Prefs *dest, Prefs *source );

void SetStatStyle( long mode );
void SetAll_Data( Prefs *pPrefs, long def );
void SetGroup_Data( Prefs *pPrefs, long item, long def );
void DefaultExtensions( Prefs *pPrefs );
long IsPrefsFile(char *prefsname);
void ClearConfig( Prefs *pPrefs );
void ConfigSettings_ClearReportStatus( Prefs *pPrefs );




int IsGroupReportNameOn( Prefs *pPrefs, char *name );
int IsGroupReportNameCompleted( Prefs *pPrefs, char *name );
int GroupReportOn( Prefs *pPrefs, long id );
int GroupReportDone( Prefs *pPrefs, long id );

int ReportCommentOn( long id );
int ReportDone( long id );
int ReportOn( long id );
int ReportsOn( long *ids );
int IsReportNameOn( Prefs *pPrefs, char *name );
int IsReportNameCompleted( Prefs *pPrefs, char *name );

int AnyTrafficStatsOn( Prefs * config );
int AnyDiagnosticStatsOn( Prefs * config );
int AnyServerStatsOn( Prefs * config );
int AnyDemographicStatsOn( Prefs * config );
int AnyReferralStatsOn( Prefs * config );
int AnyMultiMediaStatsOn( Prefs * config );
int AnySystemsStatsOn( Prefs * config );
int AnyAdvertisingStatsOn( Prefs * config );
int AnyMarketingStatsOn( Prefs * config );
int AnyClustersStatsOn( Prefs * config );
int AnyFirewallStatsOn( Prefs * config );
int AnyStreamingMediaStatsOn( Prefs * config );






void FillDefaults( Prefs *pPrefs );
void WizardDefaults( Prefs *pPrefs, long type );

long ProcessPrefsLine( int argc, char **argv, char *line, long count, Prefs *pPrefs );
long DoReadAsciiPrefs( char *filename, Prefs *pPrefs );
void DoSaveAsciiPrefs( FILE *fp, Prefs *pPrefs );
long DoReadPrefsFile(char *prefsname, Prefs *pPrefs );

#ifdef __cplusplus
void CreateDefaults( Prefs *pPrefs, long deforgs, int stat_style = WEB );
#else
void CreateDefaults( Prefs *pPrefs, long deforgs, int stat_style );
#endif

long DoReadFile(void *p, char *filename, long length);
long DoSavePrefsFile( char *filename , Prefs *p);

typedef struct {
	long	id;
	long	style;
	long	col1,col2,col3;
	long	bcol;
} GraphDataStyles, *GraphDataStylesP;
GraphDataStylesP FindGraphTypeData( long id );

typedef struct _TopLevelDomains
{
	const char *tldName;
	long descId;
} TopLevelDomains;
const char *TopLevelDomainName( long i );
const char *TopLevelDomainDesc( long i );
const char *TopLevelDomainString( long i );
void TopLevelDomainStrings( Prefs *pPrefs );


long GetHTMLColor( char *p );
void GetHTMLColors( Prefs *prefs );
long SetHTMLColors( Prefs *prefs );

// ************

#ifdef __cplusplus
}
#endif

#endif  // __CONFIG_H
 


