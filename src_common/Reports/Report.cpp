/*
##
##      ##
##      ##
##      ##  ##
##      #########
##          ##
##          ##
##          ##
##
##		V4 Analyzer -	Write module, this outputs all html/rtf/, and spits out calls to make
##						Doc/Excel/PDF to other modules, but basically handles all the reports.
##
##
##
####################################################*/



// ************************************************************************
// Includes
// ************************************************************************    
#include "FWA.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <iostream>
#include "datetime.h"
#include "version.h"
#include "ReportHTML.h"
#include "FilterString.h"		// for QS::FormatFilterString().
#include "report_keypages.h"	// for DoRouteToKeyPages && FilterNonKeyPages
#include "ReportFuncs.h"

#include "Hash.h"
#include "EngineBuff.h"
#include "EngineMeanPath.h"
#include "EngineVirtualDomain.h"
#include "EngineParse.h"
#include "ResDefs.h"

#ifdef DEF_MAC
	#ifndef __QUICKDRAW__
		#include <Quickdraw.h>
	#endif
	#include <MacErrors.h>
	#include <OpenTransport.h>
	#include <OpenTptInternet.h>
	#include <memory.h>
	#include "main.h"
	#include "config.h"
	#include "MacStatus.h"
	#include "MacUtil.h"
	#include "progress.h"
	#include "post_comp.h"
	#include "Processing.h"
	#include "stdfile.h"
	#define	RemoveFile(x)	remove(x)
#endif

#include "myansi.h"
#include "zlib.h"
#include "gd.h"
#include "config_struct.h"
#include "Stats.h"
#include "VirtDomInfo.h"
#include "StandardPlot.h"
#include "engine_proc.h"
#include "editpath.h"
#include "translate.h"
#include "Registration.h"


#if DEF_WINDOWS 
	#include <sys/stat.h>
	#include "Winmain.h"
	#include "Winutil.h"
	#include "resource.h"
	#include "postproc.h"
	#include "httpinterface.h"
#endif				

#if DEF_UNIX		// UNIX include
	#include <sys/stat.h>
	#include <errno.h>
	#include "unistd.h"
	#include "main.h"
	#include "postproc.h"
	#include "httpinterface.h"
#endif

#include "country_data.c"
#include "Report.h"
#include "ReportClass.h"
#include "OSInfo.h"
#include "StatDefs.h"	// for SESS_BYTES, SESS_TIME, etc.


#include <string>
#define NO_INLINE
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>

//#include "Utilities.h"	// for match(...)
//#include "SettingsAppWide.h"

#include "HelpCard.h"	// for WriteHelpCard(...);


// ************************************************************************
// Extern functions
// ************************************************************************
extern int IsDebugMode( void );
//extern "C" short	IsDemoReg( void );		// should be defined in serialReg.h


// ************************************************************************
// MACROs
// ************************************************************************
#undef ShowProgress
#define ShowProgress( percent, forceShow, msg )		ShowProgressDetail( percent*10,  forceShow, msg, 0 )


// ************************************************************************
// Defines
// ************************************************************************
#define	FILTER_STRING_LENGTH			2046


// ************************************************************************
// Globals & Externs.
// ************************************************************************
extern std::string	pdf_graph;
extern int			pdf_currHeight;

extern struct App_config MyPrefStruct;
extern long			allTotalRequests;
extern long			allTotalFailedRequests;
extern long			allTotalCachedHits;
extern __int64		allTotalBytes;
extern long			allfirstTime, allendTime;
extern long			badones;
extern long			totalDomains;
extern int	 		endall,stopall;
extern short		logStyle;
extern long			VDnum;
extern VDinfoP		VD[MAX_DOMAINSPLUS];			/* 16/32 virtual domain pointers */
extern "C" long		serialID;

VDinfoP				VDptrs[MAX_DOMAINS+1];
FILE				*out;

char *imagesuffixStrings[] = { ".gif", ".jpg", ".png", ".bmp", ".svg",0,0,0,0 };


char *HeaderFormat[] = {
// type,total, sessionExtras, columns IDs, each number defines the column type, (number/data/date/etc)
// make sure at least the first 4 chars are unique and that they also have min 4chars, the rest chars are ignored but/
// are kept so that we humans can understand them, computers dont really care do they.
	"none,  name",
	"norm,  reqbytes,name",
	"days,  reqbytes,sessions,Visitors,pages,errors,cost, date",
	"rday,  reqbytes,sessions,Visitors,pages,errors,cost, date",
	"mont,  reqbytes,sessions,Visitors,pages,errors,cost, month",
	"hour,  reqbytes,sessions,Visitors,pages,errors,cost, hour",					// the hour reports dont have visitors, but they could be added
	"hr3d,  reqbytes,sessions,Visitors,pages,errors,cost, hour",
	"wday,  reqbytes,sessions,Visitors,pages,errors,cost, hour",
	"week,  reqbytes,sessions,visitors,pages,errors,cost, weekday",
	"dirs,  reqbytes,pages,errors,cost, urls",
	"type,  reqbytes,errors,cost, name",
	"rbot,  reqbytes,errors,cost, name",
	"regs,  reqbytes,sessions,visitors,pages,total,errors,cost, country",
	"clie,  reqbytes,sessions,Durat,pages,errors,cost, domain",
	"clis,  reqbytes,sessions,Durat,pages,errors,cost, clienth",
	"clih,  reqbytes,sessions,Durat,pages,errors,cost, domain",
	"usre,  reqbytes,sessions,Durat,pages,errors,cost, name",
	"usrs,  reqbytes,sessions,Durat,pages,errors,cost, user",
	"usrh,  reqbytes,sessions,Durat,pages,errors,cost, domain",
	"file,  reqbytes,errors,cost, urls",
	
	"mpla,  reqbytes,Visitors,errors,cost, name",
	"brow,  reqbytes,visitors,pages,errors,cost, name",
	"bros,  reqbytes,req1,req2,req3,visitors,pages,errors,cost, name",
	"oper,  reqbytes,visitors,pages,errors,cost, name",
	"refe,  reqbytes,sessions,pages,errors,cost, link",
	"orgs,  reqbytes,sessions,visitors,pages,total,errors,cost, domain",
	"ertr,  reqbytes,cost,sver",
	"errs,  reqbytes,cost, name",

	"refh,  reqbytes,sessions,pages,errors,cost, link",
	"srch,  reqbytes,errors,cost, name",
	"seng,  reqbytes,errors,cost, engine",

	"page,  reqbytes,Sessions,Durat,Visitors,errors,cost, urlpage",
	"pagl,  reqbytes,Sessions,Durat,Visitors,errors,cost, urlpage",
	"tope,  Fses,errors,cost, urlpage",
	"topx,  Lses,errors,cost, urlpage",
	"pagh,  reqbytes,Sessions,Durat,Visitors,errors,cost, urlpage",

	"down,  reqbytes,errors,cost, urls",
	"mean,  requests,name",
	"grou,  reqbytes,pages,errors,cost, name",
	"loya,  reqbytes,pages,visitors,cost, name",
	"adds,  requests,cnt4,addpercent,errors,name",
	"addh,  requests,cnt4,addpercent,errors,name",
	"adcs,  requests,cnt4,addcost,errors,name",
	"adch,  requests,cnt4,addcost,errors,name",
#ifdef DEF_APP_FIREWALL
	"sadd,  reqbytes,sessions,visitors,pages,errors,cost, name",
	"prot,  reqbytes,errors,cost, protocol",
#endif // DEF_APP_FIREWALL

	"kvis,  %pages,requests,%sessions,Durat,errors, naml",		// name link
	"kpag,  reqbytes,errors, name",
	"kpaf,  reqbytes,errors, name",

	"clus,  reqbytes,cacheHits,pages,errors,cost, domain",		// Clusters

	"pcbl,  broklink,reqfailed,requests,visitors,blrf",			// counter4, errors, files, counter
	"flrq,  reqfailed,visitors,name",							// errors, counter
	"urag,  reqbytes,visitors,pages,errors,cost, name",

	"cpak,  capl,catr,requests,Visitors,errors cost, urlpage",	// Clip Average Packet loss
	"cpvr,  capl,catr,requests,Visitors,errors cost, urlpage",	// Clip Packet Loss vs Rate
	"cbuf,  cbuf,capl,cmce,errors,cost, urlpage",				// X Quality Clips
	"cqual, catq,requests,cmce,errors,cost, urlpage",			// X Quality Clips

	"clip,  requests,catt,capl,visitors,cost, urlpage",			// Most Popular Clips
	"cview, catv,clen,capv,capl-packetloss,visitors,cost, urlpage",			// X Viewing Times.
	"lview, catv,capv,capl-packetloss,visitors,cost, urlpage",				// X Live Viewing Times.
	"crate, catv,clen,catr,catt,capl,visitors,cost, urlpage",				// X Viewing Rates
	"lrate, catv,catr,catt,capl,visitors,cost, urlpage",					// X Live Viewing Rates

	"cperc, catt,cett,caps,cmce,cost, urlpage",					// Percentaged Streamed Per Clip
	"cmcc,  cmax,time,cost, urlpage",							// Max Concurrent Connections
	"caff,  caff,ctff,requests,cost, urlpage",					// Average Forwards Per Clip
	"carw,  carw,ctrw,requests,cost, urlpage",					// Average Forwards Per Clip
	"cunp,  cunp,requests,capl,visitors,cost, urlpage",			// Uninterrupted Plays per clip
	"cprot, requests,capl,cmce,errors,cost, name",				// Protocols Used to Transfer Clips
	"codec, requests,errors,cost, name",						// Codecs
	"cpu_,  pages,visitors,cost, name",							// CPU
	"lang,  pages,visitors,cost, name",							// Languages
	"tslot, visitors,visits,pages,btyes,errors,cost, name",		// Time Slot Analysis list
	"cmcer, err1,err2,err3,err4,err5,errors,cost, name",		// Most Common Error codes per Clip

	"bill,  bCID,bMBtransfered,bMBallowance,bExcessCharge,bTotalCharge,bCustomer",	// They get rotated!
   NULL

};



// ----------------------------------------------------------------------------
// TODO: add byStats pointer into the map to help shrink other code down
//		 
//
//

ReportTypes mainreport_datap[] = {
//	Index to data			Type	Report Title ID				Column Text ID
	VHOST_PAGE,				'virt', REPORT_VIRTUALHOSTS,		LABEL_VIRTUALHOST,		
	HOUR_PAGE,				'hour', REPORT_HOURLY,				TIME_TIME,				
	HOURHIST_PAGE,			'hr3d', REPORT_HOURLYHIST,			TIME_TIME,				
	DATE_PAGE,				'days', REPORT_DAILY,				TIME_DATE,				
	RECENTDATE_PAGE,		'rday', REPORT_RECENTDAYS,			TIME_DATE,				
	WEEK_PAGE,				'week', REPORT_WEEKLY,				TIME_DAY,				
	SUNDAY_PAGE,			'hour', REPORT_SUNDAY,  			TIME_TIME,				
	MONDAY_PAGE,			'hour', REPORT_MONDAY,	  			TIME_TIME,				
	TUEDAY_PAGE,			'hour', REPORT_TUESDAY,				TIME_TIME,				
	WEDDAY_PAGE,			'hour', REPORT_WEDNESDAY,			TIME_TIME,				
	THUDAY_PAGE,			'hour', REPORT_THURSDAY,  			TIME_TIME,				
	FRIDAY_PAGE,			'hour', REPORT_FRIDAY,  			TIME_TIME,				
	SATDAY_PAGE,			'hour', REPORT_SATURDAY,	  		TIME_TIME,				
	MONTH_PAGE,				'mont', REPORT_MONTHLY,				TIME_MONTH,				
																						
	ERRORS_PAGE,			'ertr', REPORT_SERVERERRORS,		LABEL_SERVERERROR,		
	ERRORSHIST_PAGE,		'errs', REPORT_SERVERERRORSHIST,	LABEL_SERVERERROR,		
	ERRORURL_PAGE,			'errs', REPORT_FAILURLS, 			LABEL_FAILEDURL,		
	ERRORURLHIST_PAGE,		'errs', REPORT_FAILURLSHIST,		LABEL_FAILEDURL,		
	BROKENLINKS_PAGE,		'pcbl', REPORT_BROKENLINKS,			LABEL_PAGE,
	EXTBROKENLINKS_PAGE,	'pcbl', REPORT_EXTBROKENLINKS,		LABEL_PAGE,
	INTBROKENLINKS_PAGE,	'pcbl', REPORT_INTBROKENLINKS,		LABEL_PAGE,

	// ---- STREAMING GOODIES
	CLIPSFAILED_PAGE,		'ertr', REPORT_CLIPSFAILED,			LABEL_CLIP,
	CLIPSFAILEDHIST_PAGE,	'errs',	REPORT_CLIPSFAILEDHIST,		LABEL_CLIP,
	CLIPSBROKENLINKS_PAGE,	'pcbl', REPORT_PAGES_W_INVALIDCLIPS,LABEL_CLIP,
	CLIPSERRORS_PAGE,		'cmce', REPORT_CLIPSERRORCODES,		LABEL_CLIP,
	CLIPSPACKETLOSS_PAGE,	'cpak',	REPORT_CLIPSPACKETLOSS,		LABEL_CLIP,
	CLIPSLOSSRATE_PAGE	,	'cpvr',	REPORT_CLIPSLOSSRATE,		LABEL_CLIP,
	CLIPSSECSBUF_PAGE	,	'cbuf',	REPORT_CLIPSSECSBUFF,		LABEL_CLIP,
	CLIPSHIGH_PAGE		,	'cqua',	REPORT_CLIPSHIGH,			LABEL_CLIP,
	CLIPSLOW_PAGE		,	'cqua',	REPORT_CLIPSLOW,			LABEL_CLIP,
																						
	CLIPSVID_PAGE,			'clip', REPORT_CLIPSVID,			LABEL_VIDEOCLIP,
	CLIPSVIDHIST_PAGE,		'clip', REPORT_CLIPSVIDHIST,		LABEL_VIDEOCLIP,
	CLIPSVIDVIEW_PAGE,		'cvie', REPORT_CLIPSVIDVIEW,		LABEL_VIDEOCLIP,
	CLIPSVIDRATES_PAGE,		'crat', REPORT_CLIPSVIDRATES,		LABEL_VIDEOCLIP,
	CLIPSAUD_PAGE,			'clip', REPORT_CLIPSAUD,			LABEL_AUDIOCLIP,
	CLIPSAUDHIST_PAGE,		'clip', REPORT_CLIPSAUDHIST,		LABEL_AUDIOCLIP,
	CLIPSAUDVIEW_PAGE,		'cvie', REPORT_CLIPSAUDVIEW,		LABEL_AUDIOCLIP,
	CLIPSAUDRATES_PAGE,		'crat', REPORT_CLIPSAUDRATES,		LABEL_AUDIOCLIP,
																			
	LIVEVID_PAGE,			'clip', REPORT_LIVEVID,				LABEL_VIDEOCLIP,
	LIVEVIDHIST_PAGE,		'clip', REPORT_LIVEVIDHIST,			LABEL_VIDEOCLIP,
	LIVEVIDVIEW_PAGE,		'lvie', REPORT_LIVEVIDVIEW,			LABEL_VIDEOCLIP,
	LIVEVIDRATES_PAGE,		'lrat', REPORT_LIVEVIDRATES,		LABEL_VIDEOCLIP,
	LIVEAUD_PAGE,			'clip', REPORT_LIVEAUD,				LABEL_AUDIOCLIP,
	LIVEAUDHIST_PAGE,		'clip', REPORT_LIVEAUDHIST,			LABEL_AUDIOCLIP,
	LIVEAUDVIEW_PAGE,		'lvie', REPORT_LIVEAUDVIEW,			LABEL_AUDIOCLIP,
	LIVEAUDRATES_PAGE,		'lrat', REPORT_LIVEAUDRATES,		LABEL_AUDIOCLIP,
	
	CLIPS_PAGE,				'clip', REPORT_CLIPS,				LABEL_CLIP,
	CLIPSHIST_PAGE,			'clip', REPORT_CLIPSHIST,			LABEL_CLIP,
	CLIPSLEAST_PAGE,		'clip', REPORT_CLIPSLEAST,			LABEL_CLIP,
	CLIPSPERCENT_PAGE,		'cper', REPORT_CLIPSPERCENT,		LABEL_CLIP,
	CLIPSMAXCON_PAGE,		'cmcc', REPORT_CLIPSMAXCON,			LABEL_CLIP,
	CLIPS_FF_PAGE,			'caff', REPORT_CLIPS_FF,			LABEL_CLIP,
	CLIPS_RW_PAGE,			'carw', REPORT_CLIPS_RW,			LABEL_CLIP,
	CLIPSCOMPLETED_PAGE,	'cunp', REPORT_CLIPSCOMPLETED,		LABEL_CLIP,
	CLIPSPROTOCOLS_PAGE,	'cpro', REPORT_CLIPSPROTOCOLS,		LABEL_PROTOCOLS,
	VIDCODECS_PAGE,			'code', REPORT_VIDCODECS,			LABEL_VIDEOCODEC,
	AUDCODECS_PAGE,			'code', REPORT_AUDCODECS,			LABEL_AUDIOCODEC,
	CPU_PAGE,				'cpu_', REPORT_CPU,					LABEL_CPUTYPE,
	LANG_PAGE,				'lang', REPORT_LANG,				LABEL_LANGUAGE,

	// --- Normal web pages
	PAGES_PAGE,				'page', REPORT_PAGES,				LABEL_PAGES,
	PAGEHIST_PAGE,			'pagh', REPORT_PAGESHIST,			LABEL_PAGES,
	PAGESLEAST_PAGE,		'pagl', REPORT_PAGESLEAST,			LABEL_PAGES,
	PAGESFIRST_PAGE,		'tope', REPORT_ENTRY,				LABEL_PAGES,
	PAGESFIRSTHIST_PAGE,	'tope', REPORT_ENTRYHIST,			LABEL_PAGES,
	PAGESLAST_PAGE,			'topx', REPORT_EXIT,				LABEL_PAGES,
	PAGESLASTHIST_PAGE,		'topx', REPORT_EXITHIST,			LABEL_PAGES,
																						
	REDIRECTS_PAGE,			'page', REPORT_PAGES, 				LABEL_PAGES,
	DIRS_PAGE,				'dirs', REPORT_DIRS, 				LABEL_DIRECTORY,
	TOPDIRS_PAGE,			'dirs', REPORT_TOPLEVDIRS,			LABEL_DIRECTORY,
	TOPDIRSHIST_PAGE,		'dirs', REPORT_TOPLEVDIRSHIST,		LABEL_DIRECTORY,
	FILE_PAGE,				'file', REPORT_URLS,				LABEL_FILES,	
	MEANPATH_PAGE,			'mean', REPORT_MEANPATHS,			LABEL_MEANPATH,			
	KEYPAGEROUTE_PAGE,		'kpag', REPORT_KEYPAGEROUTE,		LABEL_KEYPAGEROUTE,		
	KEYPAGEROUTEFROM_PAGE,	'kpaf',	REPORT_KEYPAGEROUTEFROM,	LABEL_KEYPAGEROUTEFROM,	
	GROUPS_PAGE,			'grou', REPORT_CONTENT, 			LABEL_CONTENTGROUP,		
	GROUPSHIST_PAGE,		'grou', REPORT_CONTENTHIST,			LABEL_CONTENTGROUP,		
	DOWNLOAD_PAGE,			'down', REPORT_DNLOADS,				LABEL_FILE,
	DOWNLOADHIST_PAGE,		'down', REPORT_DNLOADSHIST,			LABEL_FILE,
	FILETYPE_PAGE,			'type', REPORT_FILETYPES, 			LABEL_FILETYPE,			

	CLIENT_PAGE,			'clie', REPORT_CLIENTS,				LABEL_CLIENT,			
	CLIENTSTREAM_PAGE,		'clis', REPORT_CLIENTSCLICK,		LABEL_CLIENT,			
	CLIENTHIST_PAGE,		'clih', REPORT_CLIENTSHIST,			LABEL_CLIENT,			
	USER_PAGE,				'usre', REPORT_AUTHUSERS, 			LABEL_USER,				
	USERSTREAM_PAGE,		'usrs', REPORT_AUTHUSERSCLICK,		LABEL_USER,				
	USERHIST_PAGE,			'usrh', REPORT_AUTHUSERSHIST,		LABEL_USER,				
	SECONDDOMAIN_PAGE,		'orgs', REPORT_DOMAINS,				LABEL_DOMAIN,			
	DOMAIN_PAGE,			'regs', REPORT_COUNTRIES,			LABEL_COUNTRY,			
	REGION_PAGE,			'regs', REPORT_WORLDREGIONS,		LABEL_REGION,			
	ORGNAMES_PAGE,			'orgs', REPORT_ORGS,				LABEL_ORGANIZATION,		
	SESSIONS_PAGE,			'none', REPORT_SESSIONDEPTH,		TIME_DURATIONMINS,		
	KEYVISITORS_PAGE,		'kvis', REPORT_KEYVISITORS,			LABEL_KEYVISITORS,		
																						
	REFERURL_PAGE,			'refe', REPORT_REFERRALS,			LABEL_URL,				
	REFERSITE_PAGE,			'refe', REPORT_REFERRALSITES,		LABEL_REFERRAL,			
	REFERSITEHIST_PAGE,		'refh', REPORT_REFERRALSITESHIST,	LABEL_REFERRAL,			
	SEARCHSITE_PAGE,		'seng', REPORT_SEARCHENGINES,		LABEL_SEARCHENGINE,		
	SEARCHSITEHIST_PAGE,	'seng', REPORT_SEARCHENGINESHIST,	LABEL_SEARCHENGINE,		
	SEARCHSTR_PAGE,			'srch', REPORT_SEARCHTERMS, 		LABEL_SEARCHTERM,		
	SEARCHSTRHIST_PAGE,		'srch', REPORT_SEARCHTERMSHIST,		LABEL_SEARCHTERM,		
																						
	// Web Media																		
	AUDIO_PAGE,				'down', REPORT_AUDIOCONTENT, 		LABEL_AUDIOCONTENT,
	AUDIOHIST_PAGE,			'down', REPORT_AUDIOCONTENTHIST,	LABEL_AUDIOCONTENT,
	VIDEO_PAGE,				'down', REPORT_VIDEOCONTENT, 		LABEL_VIDEOCONTENT,
	VIDEOHIST_PAGE,			'down', REPORT_VIDEOCONTENTHIST,	LABEL_VIDEOCONTENT,
	MEDIATYPES_PAGE,		'type', REPORT_MEDIATYPES, 			LABEL_MEDIATYPE,		
	CLIPSPROTOCOLS_PAGE,	'prot', REPORT_PROTSUMMARY, 		LABEL_PROTOCOLS,		
																						
	MPLAYERS_PAGE,			'mpla', REPORT_MEDIAPLAYERS, 		LABEL_MEDIAPLAYER,
	MPLAYERSHIST_PAGE,		'mpla', REPORT_MEDIAPLAYERSHIST,	LABEL_MEDIAPLAYER,
	MPLAYERSOS_PAGE,		'bros', REPORT_MPLAYERVSOS,			LABEL_MEDIAPLAYER,
	CPU_PAGE,				'type', REPORT_BROWSERSOS,			LABEL_CPUTYPE,
	LANG_PAGE,				'type', REPORT_BROWSERSOS,			LABEL_LANGUAGE,
	BROWSER_PAGE,			'brow', REPORT_BROWSERS,			LABEL_BROWSER,	
	BROWSERHIST_PAGE,		'brow', REPORT_BROWSERSHIST,		LABEL_BROWSER,			
	BROWSEROS_PAGE,			'bros', REPORT_BROWSERSOS,			LABEL_BROWSER,			
	OPERSYS_PAGE,			'oper', REPORT_OS,					LABEL_OPERATINGSYSTEM,	
	OPERSYSHIST_PAGE,		'oper', REPORT_OSHIST,				LABEL_OPERATINGSYSTEM,	
	ROBOT_PAGE,				'rbot', REPORT_ROBOTS,				LABEL_ROBOT,			
	ROBOTHIST_PAGE,			'rbot', REPORT_ROBOTSHIST,			LABEL_ROBOT,			
	UNRECOGNIZEDAGENTS_PAGE,'urag', REPORT_UNRECOGNIZEDAGENTS,	LABEL_UNRECOGNIZEDAGENTS,

	ADVERT_PAGE,			'adds', REPORT_IMPRESSIONS, 		LABEL_ADNAME,			
	ADVERTHIST_PAGE,		'addh', REPORT_IMPRESSIONSHIST,		LABEL_ADNAME,			
	ADVERTCAMP_PAGE,		'adcs', REPORT_CAMPAIGNS,			LABEL_ADNAME,			
	ADVERTCAMPHIST_PAGE,	'adch', REPORT_CAMPAIGNSHIST,		LABEL_ADNAME,			
			
	BILLING_PAGE,			'bill', REPORT_BILLING,				LABEL_BILLING,

	CIRC_PAGE,				'loya', REPORT_CIRCULATION,			LABEL_FREQUENCY, 		
	LOYALTY_PAGE,			'loya', REPORT_LOYALTY,				LABEL_SESSIONS,			
	TIMEON_PAGE,			'loya', REPORT_TIMEONLINE,			LABEL_TIMEONLINE,		
																						
#ifdef DEF_APP_FIREWALL																	
	SRCADDR_PAGE,			'sadd', REPORT_SRCADDR, 			LABEL_SRCADDR,			
	PROTSUMMARY_PAGE,		'prot', REPORT_PROTSUMMARY, 		LABEL_PROTOCOLS,		
	PROTHTTP_PAGE,			'prot', REPORT_PROTHTTP, 			LABEL_HTTPCONN,			
	PROTHTTPS_PAGE,			'prot', REPORT_PROTHTTPS, 			LABEL_HTTPSCONN,		
	PROTMAIL_PAGE,			'prot', REPORT_PROTMAIL, 			LABEL_MAILCONN,			
	PROTFTP_PAGE,			'prot', REPORT_PROTFTP, 			LABEL_FTPCONN,			
	PROTTELNET_PAGE,		'prot', REPORT_PROTTELNET, 			LABEL_TELNETCONN,		
	PROTDNS_PAGE,			'prot', REPORT_PROTDNS, 			LABEL_DNSCONN,			
	PROTPOP3_PAGE,			'prot', REPORT_PROTPOP3, 			LABEL_POP3CONN,			
	PROTREAL_PAGE,			'prot', REPORT_PROTREAL, 			LABEL_REALCONN,			
	PROTOTHERS_PAGE,		'prot', REPORT_PROTOTHERS, 			LABEL_OTHERSCONN,		
#endif // DEF_APP_FIREWALL																
																						
	CLUSTER_PAGE,			'clus', REPORT_CLUSTER,				LABEL_CLUSTER,			
	CLUSTERHIST_PAGE,		'clus', REPORT_CLUSTERHIST,			LABEL_CLUSTER,			

	0,-1,0,0,0
};



extern char *	protocolStrings[];

ReportTypesP FindReportTypeData( long l_id )
{
	ReportTypesP data;
static
	long i = 0;

	data = mainreport_datap;

	// check the last searched item just incase we are searching again for the same one....
	if ( data[i].type == l_id )
		return &data[i];
	else
		i = 0;

	while ( data[i].titleStringID ){
		if ( data[i].type == l_id )
		{
			return &data[i];
		}
		i++;
	}
	i = 0;
	return NULL;
}

long GetTotalReportTypes( void )
{
	ReportTypesP pReportRec;
	long	i = 0,
			lTotal = 0,
			*pData;

	pReportRec = mainreport_datap;

	while ( pReportRec[i].titleStringID ){
		pData = ConfigFindSettingsPtr( &MyPrefStruct, pReportRec[i].type );
		if ( (pData && REPORT_CHECK_ON(*pData)) )			// new method
		//if ( ANYSTAT( (*pData[i].flags) ) )				// old method
			lTotal++;
		i++;
	}
	return i;
}






// new code RS

char *FindReportTitleStr( long l_id )
{
	ReportTypesP pReportRec;
	long i = 0;

	pReportRec = FindReportTypeData( l_id );

	if ( pReportRec ){
		const char *pTitleStr;
		if ( pReportRec->titleStringID ){
			pTitleStr = TranslateID(pReportRec->titleStringID);
			if ( !pTitleStr )
				pTitleStr = ConfigFindTitle( &MyPrefStruct, l_id );
		} else
			pTitleStr = ConfigFindTitle( &MyPrefStruct, l_id );
		return (char*)pTitleStr;
	}else
		return NULL;
}




int CleanupFilename( const char *source, char *out )
{
	if ( source && out )
	{
		char *d, c;
	const char defaultc = '_', *s;

		s = source;
		d = out;
		while ( c=*s++ )
		{
			if ( c==' ' )
				continue;
			if( c=='\\' || c=='/' )
				c = '-';
			if( c<32 || c>127 || 
				c==34 || c=='<' || c=='>' || c== '|' || c=='?' || c=='*' )
				c = defaultc;
			*d++ = c;
		}
		*d++ = 0;
		return 1;
	}
	else
		return 0;

}



char *FindReportFilenameStr( long id )
{
	static char l_filename[256];
	const char *title;

	// Find the reports name from the config list
	title = ConfigFindTitle( &MyPrefStruct, id );

	// If its not found, get the reports 'header title' (it should rarely if ever go here)
	if ( !title )
	{
		ReportTypesP data;

		data = FindReportTypeData( id );
		if ( data ){
			title = DefaultEnglishStr( data->titleStringID );
		}
	}
	if ( title )
	{
		CleanupFilename( title, l_filename );
		return l_filename;
	} else
		return NULL;
}


long *FindReportFlags( long id )
{
	return ConfigFindSettingsPtrbyStyle( &MyPrefStruct, id, ALL );
}

// end new code RS

/*
 date - normal date.
 month - 
 name - plain text with tooltip
 colname - plain text with color
 url - url with color

  */
void FixURL( char *s, char *d )
{
	if ( s && d ){
		while( *s) {
			if ( *s == '%' && s[1] != '%' )	*d++ = '%';
			*d++ = *s++;
		}
	}
}

char *ConvertParamtoURL(char *param, char *newParam)
{
	char	*s, *d, c=0, lc=0;
	long	len,outlen=0;

	d = newParam;
	s = param;

	if ( s && d ){
		while( (c=*s) && (outlen<10000) ) {
			s++;
			if( c < 33 ) {
				if ( c != lc ) {
					len = sprintf( d, "%%%02x", c );
					d+= len;
					outlen+= len;
				}
			} else {
				*d++ = c;
				outlen++;
			}
			lc = c;
		}
		*d = 0;
	}

	return newParam;
}

/*
typedef struct SearchEngineData {
	char name[32];
	char image[128];
	char search[128];
	char query[16];
	char special;
} SearchEngineRec, *SearchEnginePtr;
*/




char *GetCIACountryURL_old( char *cname, char *outname )
{
	char *clist = (char*)country_data, *p;
	p = strstr( clist, cname );
	if ( p ){
		while( *p != '=' ){
			p--;
		}
		strcpybrk( outname, p+2, '\"', 1 );
		return outname;
	} else
		return 0;
}

char *GetCIACountryURL( char *cname, char *outname )
{
	char *clist = (char*)country_data, *p;
	p = strstr( clist, cname );
	if ( p ){
		char link[64];
		while( *p != '=' ){
			p--;
		}
		strcpybrk( link, p+2, '\"', 1 );
		sprintf( outname, "http://www.odci.gov/cia/publications/factbook/%s", link );
		return outname;
	} else
		return 0;
}

long strcmpExtensions( char *string, char exts[256][8] )
{
	char *compare;
	long i = 0;

	while( exts[i][0] ){
		compare = exts[i];
		if ( strcmpiPart( compare, string ) == 0 )
			return 0;
		i++;
	}
	return 1;
}

extern short logType;

void GetCorrectSiteURL( VDinfoP VDptr, char *host )
{
	int remap = FALSE;

	if ( MyPrefStruct.multivhosts == 1 ){
		if ( VDptr->domainName[0] != '-' )
			remap = TRUE;
		if ( logType == LOGFORMAT_MACHTTP ){
			if ( MyPrefStruct.siteurl[0] != ':' && MyPrefStruct.siteurl[0] != '/' )
				remap = FALSE;
		}
	}

	if ( remap ) {
		if ( strstr( VDptr->domainName, "http" ) )
			mystrcpy( host , VDptr->domainName );
		else
			sprintf( host, "http://%s", VDptr->domainName );
	} else
		mystrcpy( host , MyPrefStruct.siteurl );
}


#if DEF_WINDOWS
#include "asyncdnr.h"
#define DISPLAY_SUMMARYFLOAT( value, text ) FormatDoubleNum( value, number ); p+=sprintf( p, "%-24s  %s    \n", text, number );
#define DISPLAY_SUMMARYNUM( value, text ) FormatLongNum( value, number );     p+=sprintf( p, "%-24s  %s    \n", text, number );

/* DisplayRealtimeSummary - print summary of total day statistics */
extern "C" void GetRealtimeSummary( VDinfoP VDptr, char *out )
{
	char number[32], *p;

	if ( !VDptr ) 
		VDptr = VD[0];

	if ( VDptr ){
		p = out;

		DISPLAY_SUMMARYNUM( VDptr->totalRequests, TranslateID(SUMM_TOTALREQS) );
		DISPLAY_SUMMARYNUM( VDptr->totalCachedHits, TranslateID(SUMM_TOTALCACHEDREQS) );
		DISPLAY_SUMMARYNUM( VDptr->totalFailedRequests, TranslateID(SUMM_TOTALFAILREQS) );
		DISPLAY_SUMMARYNUM( VDptr->badones, TranslateID(SUMM_INVALLOGENTRIES) );

		if ( VDptr->byClient ){
			DISPLAY_SUMMARYNUM( VDptr->byClient->GetStatListTotalVisits(), TranslateID(SUMM_TOTALSESSIONS) );
			DISPLAY_SUMMARYNUM( VDptr->byClient->GetStatListTotalCounters4(), TranslateID(SUMM_TOTALPAGES) );		// includes wrong urls/non existent pages attepted
		}

		if ( VDptr->byDownload ){
			if ( VDptr->byDownload->GetStatListTotalRequests() ){
				DISPLAY_SUMMARYNUM( VDptr->byDownload->GetStatListTotalRequests(), TranslateID(SUMM_TOTALDLOADFILES) );
				//DISPLAY_SUMMARYFLOAT( (double)VDptr->byDownload->GetStatListTotalBytesIn()/(ONEMEGFLOAT), TranslateID(SUMM_TOTALDOWNLOADMB) );
			}
		}

		if ( VDptr->byClient ){
			DISPLAY_SUMMARYNUM( VDptr->byClient->GetStatListNum(), TranslateID(SUMM_TOTALUNIQUEVISITORS) );
			DISPLAY_SUMMARYNUM( CountRepeatVisits( VDptr->byClient ), TranslateID(SUMM_TOTALRPTVISITORS) );
		}
		DISPLAY_SUMMARYNUM( GetUnique(), "Unique DNRs" );
		DISPLAY_SUMMARYNUM( GetReturnedDNR(), "Returned DNRs" );
		DISPLAY_SUMMARYNUM( GetWaitingDNR(), "Waiting DNRs" );
	}
}
#endif

long CountValidLast( StatList *byStat )
{
	Statistic *p;
	long i = 0, count = 0;

	while( (p=byStat->GetStat(i)) && i<byStat->num ){
		if ( p->GetVisitIn() != 0 )
			count++;
		i++;
	}
	return count;
}

long CountValidFirst( StatList *byStat )
{
	Statistic *p;
	long i = 0, count = 0;

	while( (p=byStat->GetStat(i)) && i<byStat->num ){
		if ( p->counter4 != 0 )
			count++;
		i++;
	}
	return count;
}


long CompSortVDomain(void *p1, void *p2, long *result)
{
	char *domain1,*domain2;
	VDinfoP VDptr1, VDptr2;

	VDptr1 = *(VDinfoP *)p1;
	VDptr2 = *(VDinfoP *)p2;

	if ( VDptr1 ) domain1 = VDptr1->domainName;
	if ( VDptr2 ) domain2 = VDptr2->domainName;

	*result = strcmp(domain1,domain2);
	return 0;
}

long CompSortReqVDomain(void *p1, void *p2, long *result)
{
	VDinfoP VDptr1, VDptr2;
	VDptr1 = *(VDinfoP *)p1;
	VDptr2 = *(VDinfoP *)p2;

	if ( VDptr1 && VDptr2 )
		*result = VDptr2->totalRequests - VDptr1->totalRequests;

	return 0;
}

long CompSortBytesVDomain(void *p1, void *p2, long *result)
{
	VDinfoP VDptr1, VDptr2;

	VDptr1 = *(VDinfoP *)p1;
	VDptr2 = *(VDinfoP *)p2;

	if ( VDptr1 && VDptr2 )
		*result = (long)(VDptr2->totalBytes - VDptr1->totalBytes);

	return 0;
}

//?????????????????????????????????????????????????????????????????
long CompSortVisitsVDomain(void *p1, void *p2, long *result)
{
	VDinfoP VDptr1, VDptr2;

	VDptr1 = *(VDinfoP *)p1;
	VDptr2 = *(VDinfoP *)p2;

	if ( VDptr1 && VDptr2 )
		*result = (long)(VDptr2->totalBytes - VDptr1->totalBytes);

	return 0;
}

long CompSortVisitorsVDomain(void *p1, void *p2, long *result)
{
	VDinfoP VDptr1, VDptr2;

	VDptr1 = *(VDinfoP *)p1;
	VDptr2 = *(VDinfoP *)p2;

	if ( VDptr1 && VDptr2 )
		*result = (long)(VDptr2->byClient->num - VDptr1->byClient->num);

	return 0;
}

long CompSortPagesVDomain(void *p1, void *p2, long *result)
{
	VDinfoP VDptr1, VDptr2;

	VDptr1 = *(VDinfoP *)p1;
	VDptr2 = *(VDinfoP *)p2;

	if ( VDptr1 && VDptr2 ){
		long p1=0,p2=0;

		if ( VDptr1->byClient )	p1 = VDptr1->byClient->GetStatListTotalCounters4();
		if ( VDptr2->byClient )	p2 = VDptr2->byClient->GetStatListTotalCounters4();

		*result = p2-p1;
	}

	return 0;
}

long CompSortErrorsVDomain(void *p1, void *p2, long *result)
{
	VDinfoP VDptr1, VDptr2;

	VDptr1 = *(VDinfoP *)p1;
	VDptr2 = *(VDinfoP *)p2;

	if ( VDptr1 && VDptr2 )
		*result = (long)(VDptr2->totalFailedRequests - VDptr1->totalFailedRequests);

	return 0;
}

long CompSortDateVDomain(void *p1, void *p2, long *result)
{
	VDinfoP VDptr1, VDptr2;

	VDptr1 = *(VDinfoP *)p1;
	VDptr2 = *(VDinfoP *)p2;

	if ( VDptr1 && VDptr2 )
		*result = (long)(VDptr1->firstTime - VDptr2->firstTime);

	return 0;
}

#define BUF16 16
static char *TimeonStrings[] = { "1 ", "2-4 ", "5-9 ", "10-29 ", "30-44 ", "45-59 ", "60+ ", 0 };
static char TimeonStringsTrans[8][BUF16];

char *GetTimeonStrings( short lp )
{
	return TimeonStrings[lp];
}

char *GetTimeonStringsTrans( short lp ) 
{
	strcpy( TimeonStringsTrans[lp], TimeonStrings[lp] );
	long len = strlen( TimeonStringsTrans[lp] );
	if ( lp == 0 ) // For 1 min
		mystrncpy( &TimeonStringsTrans[lp][len], TranslateID( TIME_MIN ), BUF16-len );
	else // For >1 min
		mystrncpy( &TimeonStringsTrans[lp][len], TranslateID( TIME_MINS ), BUF16-len );
	TimeonStringsTrans[lp][BUF16-1] = 0;
	return TimeonStringsTrans[lp];
}

static char *LoyaltyStrings[] = { "1 ", "2-4 ", "5-9 ", "10-24 ", "25-49 ", "50-99 ", "100+ ", 0 };
static char LoyaltyStringsTrans[8][BUF16];

char *GetLoyaltyStrings( short lp )
{
	return LoyaltyStrings[lp];
}

// function is duplicated because other one is in the class (it shouldnt be really)
static char *sFindHeadFormat( long type )
{
	long i = 0;
	char *p;

	while( p=HeaderFormat[i] ){
		if ( ReadLong( (unsigned char *)p ) == type )
			return p;
		i++;
	}
	return HeaderFormat[0];
}



// This will check the column types for X report and verify
// that the selected sort type exists in those columns
long IsThisSortValid( const long sorttype, const long reporttype )
{
	ReportTypesP	report_data;

	if ( sorttype == SORT_NAMES )
		return TRUE;

	if( report_data = FindReportTypeData( reporttype ) )
	{
		long columnID;
		char *formatStr, *type;
		columnID = report_data->typeID;

		formatStr = sFindHeadFormat( columnID );

		type = mystrchr( formatStr, ',' );

		if ( type )
		{
			long id, comp1, comp2, comp3 = 0;

			switch ( sorttype )
			{
				case SORT_BYTES:	comp1 =	'reqb'; break;
				case SORT_REQUESTS:	comp1 = 'reqb'; comp2 = 'requ'; break;
				case SORT_PAGES:	comp1 = 'page'; comp2 = 'cnt4'; comp3 = '%pag'; break;
				case SORT_ERRORS:	comp1 = 'erro'; break;
				case SORT_VISITS:	comp1 = 'Sess'; comp2 = 'sess'; break;
				case SORT_COUNTER:	comp1 = 'Visi'; comp2 = 'visi'; break;
			}



			type++;
			while( *type == ' ' )	type++;			// skip spaces

			while( type )
			{
				id = ReadLong( (unsigned char *)type );

				if ( id == comp1 || id == comp2 || id == comp3 )
					return TRUE;

				type = mystrchr( type, ',' );
				if ( type )
				{
					type++;
					while( *type == ' ' ) type++;			// skip spaces
				}
			}
		}
	}
	return FALSE;
}
