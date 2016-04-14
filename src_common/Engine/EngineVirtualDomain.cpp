#include "FWA.h"
#include <string.h>

#include "EngineVirtualDomain.h"
#include "Hash.h"
#include "log_io.h"				// for logStyle;
#include "myansi.h"				// for mystrncpylower()
#include "EngineMeanPath.h"		// for MP_FreeMeanPath
#include "StatDefs.h"			// for NAMEIS_IP ... 
#include "EngineRestoreStat.h"	// for hourStrings;
#include "engine_drill.h"	// for hourStrings;
#include "engine_proc.h"	// for hourStrings;

/*
** The following constants specify the number of array elements by which the
** statistics lists should grow each time more memory is required:
** These defaults can change (reduce in size) as the database gets larger.
*/
long		kUserStart			= 300;
long		kDefaultFileIncr	= 100;		// default for files
long		kDateIncr			= 100;		// for date list (this will do for 3 years)
long		kHourIncr			= 25;		// for hour list (only 24 hours in a day)
long		kWDayIncr			= 8;			// for weekday list (7 days in a week)
long		kDomainIncr			= 100;		// for domain list (a lot of countries)
long		kUserIncr			= kUserStart;		// for subdomain list (a lot of computers)
long		kSubDomainIncr		= 200;		// for subdomain list (a lot of computers)
long		kSecondDomainIncr	= 40;		// for subdomain list (a lot of computers)
long		kFileIncr			= 400;		// for archive file list (a lot of files)
long		kReferIncr			= 100;		// for referrals list (a lot of referrals)
long		kDirIncr			= 100;		// dirs
long		kTypeIncr			= 30;		// file types
long		kErrorsIncr			= 45;		// errors
long		kPagesIncr			= 150;		// web pages
long		kSearchStrIncr		= 40;		// search pages
long		kDownloadIncr		= 100;		// download files
long		kCountryIncr		= 100;		// for country list (DOMAIN statements)
long		kBrowserIncr		= 45;		// for browser type
long		kOSIncr				= 45;		// for browser type
long		kRobotIncr			= 30;		// for robot type
long		kDNSListIncr		= 1000;		// for DNS lookup list
long		kUnrecognizedAgentsIncr = 32;	// for Unrecognized Agents (non browser, non-robot)

/*
** Virtual Domain specific variables
*/
long		VDnum;
VDinfoP		VD[ MAX_DOMAINS+10 ];			/* 16/32 virtual domain pointers */


// Externs
extern __int64		gFilesize;



void LimitIncrValues( long vdnum )
{
	if( vdnum == 1 && !MyPrefStruct.multivhosts ){
		if( gFilesize > 5000000 ){
			kUserStart = (long)(gFilesize / 5000);
			kUserIncr = kUserIncr * 2;
		}
	} else
	if( vdnum == 5 ){
		kDateIncr = 1;
		kErrorsIncr = 1;
		kDomainIncr = 20;
		kSubDomainIncr = 20;
		kSecondDomainIncr = 2;
		kFileIncr = 30;
		kReferIncr = 30;
		kDirIncr = 30;
		kTypeIncr = 1;
		kErrorsIncr = 10;
		kPagesIncr = 40;
		kSearchStrIncr = 1;
		kDownloadIncr = 1;
		kCountryIncr = 10;
		kBrowserIncr = 10;
		kOSIncr = 10;
		kRobotIncr = 10;
		kMeanPathIncr = 80;
	} else
	if( vdnum == 12 ){
		kDefaultFileIncr = 2;
		kDateIncr = 1;
		kErrorsIncr = 3;
		kDomainIncr = 2;
		kSubDomainIncr = 1;
		kSecondDomainIncr = 2;
		kFileIncr = 3;
		kReferIncr = 3;
		kDirIncr = 1;
		kTypeIncr = 1;
		kErrorsIncr = 1;
		kPagesIncr = 2;
		kSearchStrIncr = 1;
		kDownloadIncr = 1;
		kCountryIncr = 1;
		kBrowserIncr = 1;
		kOSIncr = 1;
		kRobotIncr = 1;
		kMeanPathIncr = 5;
	}
}

/* ---------------------------------------------------------------------------------------------- */


VDinfoP	FindVirtualDomain( char *vname )
{
	long i=0,findhash,lp;
	VDinfoP	VDptr=NULL;

	if( CheckWWWprefix(vname) )
		findhash = HashStr( vname+4, &i );
	else
		findhash = HashStr( vname, &i );

	if( findhash ){
		i = VDnum;
		if( i > MAX_DOMAINS ) i = MAX_DOMAINS;

		for( lp=0; lp<=i; lp++ ) {
			VDptr = VD[lp];
			if( VDptr ){
				if( VDptr->hash == findhash )
					return VDptr;
			}
		}
	}

	if( VDnum > MAX_DOMAINS )
		return (VDinfoP)VD-1;		// domain not found and no space free
	else
		return (VDinfoP)0;			// domain not found but there is room
}


// call as CloseVirtualDomain( 1, "this.com", 1 );
void CloseVirtualDomain( VDinfoP VDptr, long logstyle )
{
	delete VDptr;
}


//
//RS: its now very hard to turn off demographics, errors doesnt need clients, broken links can work without clients, but will have a blank column
//All marketing, pageslast, brokenlinks,key stuff, errors must be OFF for clients to be OFF
short NeedClientStats()
{
	return	STAT(MyPrefStruct.stat_client)
		||	STAT(MyPrefStruct.stat_clientHistory)
		||	STAT(MyPrefStruct.stat_clientStream)
		||	STAT(MyPrefStruct.stat_country)
		||	STAT(MyPrefStruct.stat_seconddomain)
		||	STAT(MyPrefStruct.stat_regions)
		||	STAT(MyPrefStruct.stat_orgs)
		||	STAT(MyPrefStruct.stat_user)
		||	STAT(MyPrefStruct.stat_meanpath)
		||	STAT(MyPrefStruct.stat_sessionScatter)
		||	STAT(MyPrefStruct.stat_errors)
		||	STAT(MyPrefStruct.stat_pageslast)
		||	STAT(MyPrefStruct.stat_pageslastHistory)
		||	STAT(MyPrefStruct.stat_keypageroute)
		||	STAT(MyPrefStruct.stat_keypageroutefrom)
		||	STAT(MyPrefStruct.stat_keyvisitor)
		||	STAT(MyPrefStruct.stat_brokenLinks)
		||	STAT(MyPrefStruct.stat_circulation)
		||	STAT(MyPrefStruct.stat_loyalty)
		||	STAT(MyPrefStruct.stat_timeon)
 
		// Turn on the dependancy of all the Sysmtem reports upon the ByClient 
		||	STAT(MyPrefStruct.stat_robot)
		||	STAT(MyPrefStruct.stat_robotHistory)
		||	STAT(MyPrefStruct.stat_browsers)
		||	STAT(MyPrefStruct.stat_browsersHistory)
		||	STAT(MyPrefStruct.stat_unrecognizedagents)
		||	STAT(MyPrefStruct.stat_browserVSos)
		||	STAT(MyPrefStruct.stat_opersys)
		||	STAT(MyPrefStruct.stat_opersysHistory)

#ifdef DEF_APP_FIREWALL
		// These dont really depend on the client....
		||	STAT(MyPrefStruct.stat_protHTTP) || STAT(MyPrefStruct.stat_protHTTPS) || STAT(MyPrefStruct.stat_protMail)
		||	STAT(MyPrefStruct.stat_protFTP) || STAT(MyPrefStruct.stat_protTelnet) || STAT(MyPrefStruct.stat_protDNS)
		||	STAT(MyPrefStruct.stat_protPOP3) || STAT(MyPrefStruct.stat_protReal) || STAT(MyPrefStruct.stat_protOthers)
#endif // DEF_APP_FIREWALL
		;
}



short NeedFileStats()
{
return	STAT(MyPrefStruct.stat_files)
		||	STAT(MyPrefStruct.stat_pages) || STAT(MyPrefStruct.stat_pagesHistory)
		||	STAT(MyPrefStruct.stat_pagesfirst) 	|| STAT(MyPrefStruct.stat_pagesfirstHistory)
		||	STAT(MyPrefStruct.stat_pageslast) || STAT(MyPrefStruct.stat_pageslastHistory)
		||	STAT(MyPrefStruct.stat_pagesLeastVisited)
		||	STAT(MyPrefStruct.stat_download) || STAT(MyPrefStruct.stat_downloadHistory)
		||	STAT(MyPrefStruct.stat_audio) || STAT(MyPrefStruct.stat_audioHistory)
		||	STAT(MyPrefStruct.stat_video) || STAT(MyPrefStruct.stat_videoHistory)
		||	STAT(MyPrefStruct.stat_keypageroute)
		||	STAT(MyPrefStruct.stat_keypageroutefrom)
		||	STAT(MyPrefStruct.stat_brokenLinks) 
		;
}


short NeedPagesStats()
{

return		STAT(MyPrefStruct.stat_pages)
		||  STAT(MyPrefStruct.stat_pagesHistory)
		||  STAT(MyPrefStruct.stat_pagesfirst)
		||  STAT(MyPrefStruct.stat_pagesfirstHistory)
		||  STAT(MyPrefStruct.stat_pageslast)
		||  STAT(MyPrefStruct.stat_pageslastHistory)
		||  STAT(MyPrefStruct.stat_pagesLeastVisited)
		||  STAT(MyPrefStruct.stat_keypageroute)
		||  STAT(MyPrefStruct.stat_keypageroutefrom)
		||  STAT(MyPrefStruct.stat_keyvisitor)
		||  STAT(MyPrefStruct.stat_meanpath)
		 // Add the dependancy on the ByPages.
		 // QCM 49633 - version 28;
		 // Added a dependancy upon byClient & byPages for the report generation of all the "System" reports.  Also added a dependancy on byBrowser, byRobot, byOS for byUnrecognisedAgent
		|| STAT(MyPrefStruct.stat_robot)
		|| STAT(MyPrefStruct.stat_robotHistory)
		|| STAT(MyPrefStruct.stat_browsers)
		|| STAT(MyPrefStruct.stat_browsersHistory)
		|| STAT(MyPrefStruct.stat_unrecognizedagents)
		|| STAT(MyPrefStruct.stat_browserVSos)
		|| STAT(MyPrefStruct.stat_opersys)
		|| STAT(MyPrefStruct.stat_opersysHistory)
		;

}



/*--------------------------------------------
	generate stats from webstar.log
	5 processes
	1. init proc
	2. loop and collect all log data into memory database(s)
	3. write out html report(s)
	4. sleep if need then goto 2 else cont.
	5. free/delete memory database(s)

---------------------------------------------*/

// call as InitVirtualDomain( 1, "this.com", 1 );
VDinfoP	InitVirtualDomain( int num, char *name, short logstyle )
{
	VDinfoP	VDptr;
	long		i;

	if( num > MAX_DOMAINS || !name)
		return 0;

	LimitIncrValues( num );

	/* initialize virt domains info/data */
	VD[num] = new VDinfo;	// now has a ctor!
	VDptr = VD[num];

	// so far, both of these reports require the failed requests map
	if( ReportOn(BROKENLINKS_PAGE) || ReportOn(ERRORS_PAGE) )
	{
		VDptr->m_pFailedRequestInfoMap=new VDinfo::FailedRequestInfoMap;
	}

	VDptr->domainNum			= num;
	VDptr->totalBytes			= 0;
	VDptr->totalBytesIn			= 0;
	VDptr->totalFailedRequests	= 0;
	VDptr->totalRequests		= 0;
	VDptr->badones				= 0;
	VDptr->totalCachedHits		= 0;
	VDptr->firstTime			= 0;
	VDptr->lastTime				= 0;
	VDptr->totalDays			= 0;
	VDptr->totalInDataSize		= 0;
	VDptr->logType				= logType;
	VDptr->logStyle				= logstyle;
	//VDptr->style				= MyPrefStruct.stat_style;
	VDptr->next = 0;

	// OverFlow check, (200 limit)
	i = mystrncpylower( VDptr->domainName, name, MAX_DOMAINNAMESIZE );
	if( CheckWWWprefix(VDptr->domainName) )
		VDptr->hash = HashStr( VDptr->domainName+4, &i );
	else
		VDptr->hash = HashStr( VDptr->domainName, &i );

	// general data
	VDptr->byHour 		= new StatList(VDptr,kHourIncr);
	VDptr->byWeekday	= new StatList(VDptr,kWDayIncr);
	VDptr->byDate		= new StatList(VDptr,kDateIncr);
	VDptr->byUser		= 0;		//new StatList(kSubDomainIncr);
	VDptr->byDomain		= new StatList(VDptr,kDomainIncr);

	if( NeedFileStats() )
		VDptr->byFile		= new StatList(VDptr,kFileIncr);

	if( NeedClientStats() )
		VDptr->byClient		= new StatList(VDptr,kUserStart);

	if( VDptr->byClient ){
		// use ReportOn( DOMAIN_PAGE ) instead in future.... so we arent dependant on knowing stat_ stuff/junk
		if( ReportOn(SECONDDOMAIN_PAGE) )
			VDptr->bySecondDomain = new StatList(VDptr,kSecondDomainIncr);

		if( ReportOn(REGION_PAGE) )	
			VDptr->byRegions = new StatList(VDptr,8);

		if( ReportOn(ORGNAMES_PAGE) && MyPrefStruct.filterdata.orgTot>0 )	
			VDptr->byOrgs = new StatList(VDptr,kSecondDomainIncr);

		VDptr->byClient->useOtherNames = NAMEIS_IP;
	}

	if( VDptr->byDate ){
		if( ReportOn(MONTH_PAGE) )	
			VDptr->byMonth = new StatList(VDptr,kWDayIncr);

		if( ReportOn(WEEKDAYS_PAGE) || ReportOn(WEEK_PAGE) ) {
				for(i=0;i<7;i++) VDptr->byWeekdays[i] = new StatList(VDptr,kHourIncr);
		}
	}

	if( logstyle >= LOGFORMAT_WEBSERVERS ){
		// webserver extra data
		if( ReportOn(BROWSER_PAGE) ||
			ReportOn(UNRECOGNIZEDAGENTS_PAGE) ||
			ReportOn(BROWSEROS_PAGE) )				VDptr->byBrowser			= new StatList(VDptr,kBrowserIncr);

		if( ReportOn(OPERSYS_PAGE) ||
			ReportOn(UNRECOGNIZEDAGENTS_PAGE) ||
			ReportOn(BROWSEROS_PAGE) )				VDptr->byOperSys			= new StatList(VDptr,kOSIncr);
		if( ReportOn(ROBOT_PAGE) ||
			ReportOn(UNRECOGNIZEDAGENTS_PAGE)	)	VDptr->byRobot				= new StatList(VDptr,kRobotIncr);

		if( ReportOn(UNRECOGNIZEDAGENTS_PAGE) )		VDptr->byUnrecognizedAgents	= new StatList(VDptr,kUnrecognizedAgentsIncr);

		if( ReportOn(REFERURL_PAGE) ||
			ReportOn(REFERSITE_PAGE) ||
			ReportOn(REFERSITEHIST_PAGE) ||
			ReportOn(BROKENLINKS_PAGE) ||
			ReportOn(ERRORS_PAGE) )					VDptr->byRefer		= new StatList(VDptr,kReferIncr);
		if( ReportOn(REFERSITE_PAGE) ||
			ReportOn(REFERSITEHIST_PAGE) )			VDptr->byReferSite	= new StatList(VDptr,kReferIncr);
		if( ReportOn(DIRS_PAGE))					VDptr->byDir		= new StatList(VDptr,kDirIncr);
		if( ReportOn(GROUPS_PAGE) ||
			ReportOn(GROUPSHIST_PAGE) )				VDptr->byGroups		= new StatList(VDptr,kDirIncr);
		if( ReportOn(TOPDIRS_PAGE) ||
			ReportOn(TOPDIRSHIST_PAGE) )			VDptr->byTopDir		= new StatList(VDptr,kDirIncr);

		if( ReportOn(FILETYPE_PAGE) )				VDptr->byType		= new StatList(VDptr,kTypeIncr);
		if( ReportOn(ERRORS_PAGE) ||
			ReportOn(ERRORSHIST_PAGE) )				VDptr->byErrors		= new StatList(VDptr,kErrorsIncr);
		if( ReportOn(ERRORURL_PAGE) ||
			ReportOn(ERRORURLHIST_PAGE) )			VDptr->byErrorURL	= new StatList(VDptr,kDefaultFileIncr);

		if( ReportOn(BROKENLINKS_PAGE) )
		{
			// used for either external or all pages containing broken links
			VDptr->byBrokenLinkReferal=new StatList(VDptr,kErrorsIncr);

			// if we have the capability to differentite between internal and external referals
			if( VDptr->GetSiteURL() )
			{
				// used only for internal pages containing broken links
				VDptr->byIntBrokenLinkReferal=new StatList(VDptr,kErrorsIncr);
			}
		}

		if( NeedPagesStats() )
		{
			VDptr->byPages			= new StatList(VDptr,kPagesIncr);
		}

		if( ReportOn(SEARCHSTR_PAGE) ||
			ReportOn(SEARCHSITE_PAGE) )		VDptr->bySearchStr		= new StatList(VDptr,kSearchStrIncr);
		if( ReportOn(SEARCHSITE_PAGE) )		VDptr->bySearchSite		= new StatList(VDptr,kSearchStrIncr);
		if( ReportOn(DOWNLOAD_PAGE) ||
			ReportOn(KEYPAGEROUTE_PAGE)		||
			ReportOn(KEYPAGEROUTEFROM_PAGE)	||
			ReportOn(KEYVISITORS_PAGE)			||
			ReportOn(DOWNLOADHIST_PAGE) )	VDptr->byDownload		= new StatList(VDptr,kDownloadIncr);
		if( ReportOn(UPLOAD_PAGE) ||
			ReportOn(UPLOADHIST_PAGE) )		VDptr->byUpload			= new StatList(VDptr,kDownloadIncr);
		if( ReportOn(ADVERT_PAGE) ||
			ReportOn(ADVERTHIST_PAGE) ) 	VDptr->byAdvert			= new StatList(VDptr,kDownloadIncr);
		if( ReportOn(ADVERTCAMP_PAGE) ||
			ReportOn(ADVERTCAMPHIST_PAGE) )	VDptr->byAdCamp			= new StatList(VDptr,kDownloadIncr);

		if( ReportOn(BILLING_PAGE) )
		{
			VDptr->byBilling				= new StatList(VDptr,kDownloadIncr);
			VDptr->byBilling->doTimeStat	= TIMESTAT_DAYSHISTORY;
		}

		// Streaming stuff
		if ( MyPrefStruct.stat_style == STREAM )
		{
			if( ReportOn(VIDCODECS_PAGE) )	VDptr->byVideoCodecs	= new StatList(VDptr,kTypeIncr);
			if( ReportOn(AUDCODECS_PAGE) )	VDptr->byAudioCodecs	= new StatList(VDptr,kTypeIncr);
			if( ReportOn(CPU_PAGE) )		VDptr->byCPU			= new StatList(VDptr,kTypeIncr);
			if( ReportOn(LANG_PAGE) )		VDptr->byLang			= new StatList(VDptr,kTypeIncr);
		} else {
			if( ReportOn(VIDEO_PAGE) )		VDptr->byVideo			= new StatList(VDptr,kDownloadIncr);
			if( ReportOn(AUDIO_PAGE) )		VDptr->byAudio			= new StatList(VDptr,kDownloadIncr);
		}
		if( ReportOn(MPLAYERS_PAGE) )		VDptr->byMediaPlayers	= new StatList(VDptr,kDownloadIncr);
		if( ReportOn(MEDIATYPES_PAGE) ) 	VDptr->byMediaTypes		= new StatList(VDptr,kDownloadIncr);
		if( MyPrefStruct.clusteringActive )	VDptr->byClusters		= new StatList(VDptr,kDownloadIncr);

		VDptr->byServers	= new StatList( VDptr,1 );

		// firewall/router extra data
		if( logstyle == LOGFORMAT_FIREWALLS || logstyle == LOGFORMAT_STREAMINGMEDIA )
		{
			VDptr->byProtocol	= new StatList( VDptr,10 );
		}



		if( VDptr->byClient )
		{
			// store flag to do timestat at Bit-0
			if( ReportOn(MEANPATH_PAGE) ||
				ReportOn(SESSIONS_PAGE) ||
				ReportOn(CIRC_PAGE) ||
				ReportOn(CLIENTSTREAM_PAGE) ||
				ReportOn(USERSTREAM_PAGE) ||
				ReportOn(PAGESLAST_PAGE) ||
				ReportOn(PAGESLASTHIST_PAGE) ||
				ReportOn(KEYPAGEROUTE_PAGE) ||
				ReportOn(KEYPAGEROUTEFROM_PAGE) ||
				ReportOn(KEYVISITORS_PAGE)		
				) 
			{
				MyPrefStruct.stat_sessionHistory = 1;
			} else
				MyPrefStruct.stat_sessionHistory = 0;

			if(	MyPrefStruct.stat_sessionHistory )
			{
				VDptr->byClient->doSessionStat = SESSIONSTAT_CLICKSTREAM;
			}

			// store flag to do timestat at Bit-0
			if( ReportOn(CLIENTHIST_PAGE) || 
				 ReportOn(USERHIST_PAGE) ||
				 ReportOn(LOYALTY_PAGE) ||
				 ReportOn(TIMEON_PAGE) )	
				 VDptr->byClient->doTimeStat = TIMESTAT_DAYSHISTORY;
		}

		if( ReportOn(HOURHIST_PAGE) && VDptr->byHour ) 				VDptr->byHour->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(PAGEHIST_PAGE) && VDptr->byPages ) 			VDptr->byPages->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(REFERSITEHIST_PAGE) && VDptr->byReferSite )	VDptr->byReferSite->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(DOWNLOADHIST_PAGE) && VDptr->byDownload )		VDptr->byDownload->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(UPLOADHIST_PAGE) && VDptr->byUpload )			VDptr->byUpload->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(ERRORURLHIST_PAGE) && VDptr->byErrorURL )		VDptr->byErrorURL->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(ERRORSHIST_PAGE) && VDptr->byErrors )			VDptr->byErrors->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(TOPDIRSHIST_PAGE) && VDptr->byTopDir )			VDptr->byTopDir->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(GROUPSHIST_PAGE) && VDptr->byGroups )			VDptr->byGroups->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(ADVERTHIST_PAGE) && VDptr->byAdvert )			VDptr->byAdvert->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(ADVERTCAMPHIST_PAGE) && VDptr->byAdCamp )		VDptr->byAdCamp->doTimeStat = TIMESTAT_DAYSHISTORY;

		if( ReportOn(BROWSERHIST_PAGE) && VDptr->byBrowser )		VDptr->byBrowser->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(OPERSYSHIST_PAGE) && VDptr->byOperSys )		VDptr->byOperSys->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(ROBOTHIST_PAGE) && VDptr->byRobot )			VDptr->byRobot->doTimeStat = TIMESTAT_DAYSHISTORY;

		if( ReportOn(SEARCHSTRHIST_PAGE) && VDptr->bySearchStr )	VDptr->bySearchStr->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(SEARCHSITEHIST_PAGE) && VDptr->bySearchSite )	VDptr->bySearchSite->doTimeStat = TIMESTAT_DAYSHISTORY;
		if( ReportOn(MPLAYERSHIST_PAGE) && VDptr->byMediaPlayers )	VDptr->byMediaPlayers->doTimeStat = TIMESTAT_DAYSHISTORY;

		// Web Media Streaming mode
		if ( MyPrefStruct.stat_style == STREAM )
		{
			//if( VDptr->byFile )										VDptr->byFile->doTimeStat = TIMESTAT_DAYSHISTORY|TIMESTAT_DIRECT1;
			if( VDptr->byPages )									VDptr->byPages->doTimeStat |= TIMESTAT_DIRECT2;
			if( VDptr->byPages )									VDptr->byPages->doSessionStat = SESSIONSTAT_CLIPDATA;
			if( VDptr->byProtocol )									VDptr->byProtocol->doTimeStat = TIMESTAT_DAYSHISTORY|TIMESTAT_DIRECT2;
		} else
		{
			if( ReportOn(VIDEOHIST_PAGE) )							VDptr->byVideo->doTimeStat = TIMESTAT_DAYSHISTORY;
			if( ReportOn(AUDIOHIST_PAGE) )							VDptr->byAudio->doTimeStat = TIMESTAT_DAYSHISTORY;
		}


		if( MyPrefStruct.clusteringActive && VDptr->byClusters )	VDptr->byClusters->doTimeStat = TIMESTAT_DAYSHISTORY;

		// special case storing custom data indexs, they could be on as well as normal timestat
		// so we use Bit-1 to store that
		if( VDptr->byMediaPlayers && VDptr->byOperSys )
			VDptr->byMediaPlayers->doTimeStat |= TIMESTAT_DIRECT2;

		if( VDptr->byBrowser && VDptr->byOperSys )
			VDptr->byBrowser->doTimeStat |= TIMESTAT_DIRECT2;

		if( VDptr->bySearchSite )
			VDptr->bySearchSite->doTimeStat |= TIMESTAT_DIRECT2;

		// flag all the lists which dont copy the name into memory butuse it directly as a pointer to save ram
		// 1 = uses a static string as a reference
		// 2 = uses a copy of a dynamic string from another list
		VDptr->byHour->useOtherNames = NAMEIS_STATIC;
		VDptr->byWeekday->useOtherNames = NAMEIS_STATIC;
		if( VDptr->byWeekday && VDptr->byWeekdays[0] ){
			for(i=0;i<7;i++) VDptr->byWeekdays[i]->useOtherNames = NAMEIS_STATIC;
		}

		// these use global static strings as names to conservce memory.
		if( VDptr->byRegions )		VDptr->byRegions->useOtherNames = NAMEIS_STATIC;
		if( VDptr->byDomain )		VDptr->byDomain->useOtherNames = NAMEIS_STATIC;			//this is Countries
		if( VDptr->byOperSys )		VDptr->byOperSys->useOtherNames = NAMEIS_STATIC;
		if( VDptr->byErrors )		VDptr->byErrors->useOtherNames = NAMEIS_STATIC;

		// these stats contain .name references back to byFile tosave ram on name storage
		if( VDptr->byPages ) 		VDptr->byPages->useOtherNames = NAMEIS_COPY;
		if( VDptr->byDownload )		VDptr->byDownload->useOtherNames = NAMEIS_COPY;
		if( VDptr->byUpload )		VDptr->byUpload->useOtherNames = NAMEIS_COPY;

		// contains a reference back to byRefer in order to save memory
		if( VDptr->byBrokenLinkReferal )	 VDptr->byBrokenLinkReferal->useOtherNames = NAMEIS_COPY;
		if( VDptr->byIntBrokenLinkReferal ) VDptr->byIntBrokenLinkReferal->useOtherNames = NAMEIS_COPY;

		if( ReportOn(MEANPATH_PAGE) )	MP_FreeMeanPath( VDptr );
	}

	/* initialize weekday and hour names */
	for (i=0; i<7; i++)
	{
		if( VDptr->byWeekday )
		{
			// offset the current day index according to user's first day of week setting so that
			// the specified day of week appears first within the weekly traffic reports
			size_t dayIndex=(i+MyPrefStruct.firstDayOfWeek)%7;

			VDptr->byWeekday->SearchStat( weekdays[dayIndex] );
		}
		if( VDptr->byWeekdays[i] )
		{
			for( long j=0;j<24;j++)
			{
				VDptr->byWeekdays[i]->SearchStat( hourStrings[j] );
			}
		}
	}
	for (i=0; i<24; i++)
	{
		if( VDptr->byHour )
		{
			VDptr->byHour->SearchStat( hourStrings[i] );
		}
	}
	
	// end
	VDptr->time1 = timems();
	VDptr->time2 = 0;
	return VDptr;
}

void SortRecords( VDinfoP VDptr, long logstyle )
{
	long i;

	memset( &VDptr->Done, 0 , sizeof( doneRec ) );

	VDptr->Done.Date = VDptr->byDate->GetStatListNum();
	VDptr->Done.Hour = VDptr->byHour->GetStatListNum();
	VDptr->Done.Week = VDptr->byWeekday->GetStatListNum();
	for( i=0;i<7;i++){
		if( VDptr->byWeekdays[i] )
		VDptr->Done.Weekdays[i] = VDptr->byWeekdays[i]->GetStatListNum();
	}

	//don't bother if there is no valid info
	VDptr->Done.Domain = VDptr->byDomain->GetStatListNum();
	if( VDptr->byClient )				VDptr->Done.Client = VDptr->byClient->GetStatListNum();
	if( VDptr->byRegions )				VDptr->Done.Regions = VDptr->byRegions->GetStatListNum();
	if( VDptr->byUser )					VDptr->Done.User = VDptr->byUser->GetStatListNum();
	if( VDptr->byGovt )					VDptr->Done.Govt = VDptr->byGovt->GetStatListNum();
	if( VDptr->byCirculation )			VDptr->Done.Circulation = VDptr->byCirculation->GetStatListNum();
	if( VDptr->byLoyalty )				VDptr->Done.Loyalty = VDptr->byLoyalty->GetStatListNum();
	if( VDptr->byTimeon )				VDptr->Done.Timeon = VDptr->byTimeon->GetStatListNum();
	if( VDptr->byOrgs && MyPrefStruct.filterdata.orgTot>0 )	VDptr->Done.Orgs = VDptr->byOrgs->GetStatListNum();
	if( VDptr->byOrgs && MyPrefStruct.filterdata.orgTot>0 )	VDptr->Done.Orgs = VDptr->byOrgs->GetStatListNum();
	if( VDptr->byOrgs && MyPrefStruct.filterdata.orgTot>0 )	VDptr->Done.Orgs = VDptr->byOrgs->GetStatListNum();
	if( VDptr->byOrgs && MyPrefStruct.filterdata.orgTot>0 )	VDptr->Done.Orgs = VDptr->byOrgs->GetStatListNum();
	if( VDptr->byMonth )				VDptr->Done.Month = VDptr->byMonth->GetStatListNum();
	if( VDptr->byPages )				VDptr->Done.Pages = VDptr->byPages->GetStatListNum();

	if( VDptr->byFile )					VDptr->Done.File = VDptr->byFile->GetStatListNum();
	if( VDptr->byBrowser )				VDptr->Done.Browser = VDptr->byBrowser->GetStatListNum();
	if( VDptr->byOperSys )				VDptr->Done.OperSys = VDptr->byOperSys->GetStatListNum();
	if( VDptr->byRefer )				VDptr->Done.Refer = VDptr->byRefer->GetStatListNum();
	if( VDptr->byReferSite )			VDptr->Done.ReferSite = VDptr->byReferSite->GetStatListNum();
	if( VDptr->byDir )					VDptr->Done.Dir = VDptr->byDir->GetStatListNum();
	if( VDptr->byGroups )				VDptr->Done.Groups = VDptr->byGroups->GetStatListNum();
	if( VDptr->byTopDir )				VDptr->Done.TopDir = VDptr->byTopDir->GetStatListNum();
	if( VDptr->byType )					VDptr->Done.Type = VDptr->byType->GetStatListNum();
	//don't bother if there is no valid info
	if( VDptr->bySecondDomain )			VDptr->Done.SecondDomain = VDptr->bySecondDomain->GetStatListNum();
	if( VDptr->byErrors )				VDptr->Done.Errors = VDptr->byErrors->GetStatListNum();
	if( VDptr->byErrorURL )				VDptr->Done.ErrorURL = VDptr->byErrorURL->GetStatListNum();

	if( VDptr->bySearchStr )			VDptr->Done.SearchStr = VDptr->bySearchStr->GetStatListNum();
	if( VDptr->bySearchSite )			VDptr->Done.SearchSite = VDptr->bySearchSite->GetStatListNum();
	if( VDptr->byDownload )				VDptr->Done.Download = VDptr->byDownload->GetStatListNum();
	if( VDptr->byRobot )				VDptr->Done.Robot = VDptr->byRobot->GetStatListNum();

	if( VDptr->byPages && ReportOn(MEANPATH_PAGE) )
										VDptr->Done.MeanPath = MP_GetMeanPathTotal( VDptr );
	if( VDptr->byServers )				VDptr->Done.SourceAddr = VDptr->byServers->GetStatListNum();

	if( logstyle == LOGFORMAT_FIREWALLS )
	{
		if( VDptr->byProtocol ) VDptr->Done.Protocol = VDptr->byProtocol->GetStatListNum();
		if( VDptr->byHTTP )		VDptr->Done.HTTP = VDptr->byHTTP->GetStatListNum();
		if( VDptr->byHTTPS )	VDptr->Done.HTTPS = VDptr->byHTTPS->GetStatListNum();
		if( VDptr->byMail )		VDptr->Done.Mail = VDptr->byMail->GetStatListNum();
		if( VDptr->byFTP )		VDptr->Done.FTP = VDptr->byFTP->GetStatListNum();
		if( VDptr->byTelnet )	VDptr->Done.Telnet = VDptr->byTelnet->GetStatListNum();
		if( VDptr->byDNS )		VDptr->Done.DNS = VDptr->byDNS->GetStatListNum();
		if( VDptr->byPOP3 )		VDptr->Done.POP3 = VDptr->byPOP3->GetStatListNum();
		if( VDptr->byRealAudio ) VDptr->Done.RealAudio = VDptr->byRealAudio->GetStatListNum();
		if( VDptr->byOthers )	VDptr->Done.Others = VDptr->byOthers->GetStatListNum();
	}
//OutDebugs( "sortmp total = %d", VDptr->meanpath_totals );
	// ----------------------------------------------------------------------------------				
	/* sort the stuff in either names or bytes order*/
	if( VDptr->byPages && MP_GetMeanPathTotal( VDptr ) )
		MP_SortMeanPath( VDptr );

//OutDebugs( "sortmp total = %d", VDptr->meanpath_totals );

}


//-------------------------------------------------------------------------------------------------
// Nice hacky psuedo ctor as a sanity check
//-------------------------------------------------------------------------------------------------
VDinfo::VDinfo()
{
	MemClr( this, sizeof( VDinfo ) );
}


//-------------------------------------------------------------------------------------------------
// This code used to be in CloseVirtualDomain()
//-------------------------------------------------------------------------------------------------
VDinfo::~VDinfo()
{
	delete m_pFailedRequestInfoMap;

	/* free up statlists */
	OutDebug( "Clearing hour,weekday,date,domain" );
	delete byHour;
	delete byWeekday;
	delete byDate;
	delete byDomain;

	OutDebug( "Clearing URL stats" );
	delete byFile;

	OutDebug( "Clearing Client stats" );
	delete byClient;

	// clear the user time/session history pointers because they really belong
	//   to the Client database and are deleted there not here.
	if( byUser )
	{
		byUser->ClearTimeHistoryPointers();
		delete byUser;
	}

	OutDebug( "Clearing region,dom,org,months Stats" );
	delete byRegions;
	delete bySecondDomain;
	delete byOrgs;
	delete byMonth;

	OutDebug( "Clearing WeekDays Stats" );
	for( size_t i(0); i<7; i++ )
	{
		delete byWeekdays[i];
	}

	if( ReportOn(MEANPATH_PAGE) )
	{
		MP_FreeMeanPath( this );
	}

	delete byCirculation;
	delete byLoyalty;
	delete byTimeon;

	// webserver extra data
	OutDebug( "Clearing Pages Stats" );
	delete byPages;
	OutDebug( "Clearing Directory Stats" );
	delete byDir;
	delete byTopDir;
	OutDebug( "Clearing Agent Stats" );
	delete byBrowser;
	delete byOperSys;
	delete byRobot;
	delete byUnrecognizedAgents;
	OutDebug( "Clearing Referral Stats" );
	delete byRefer;
	delete byReferSite;

	OutDebug( "Clearing Other Web Stats" );
	delete byGroups;
	delete byType;
	delete byErrors;
	delete byErrorURL;
	delete byBrokenLinkReferal;
	delete byIntBrokenLinkReferal;
	delete bySearchStr;
	delete bySearchSite;
	delete byDownload;
	delete byUpload;
	delete byAdvert;
	delete byAdCamp;

	OutDebug( "Clearing MultiMedia Stats" );
	delete byClips;
	delete byVideo;
	delete byAudio;
	delete byVideoCodecs;
	delete byAudioCodecs;
	delete byMediaTypes;
	delete byMediaPlayers;
	delete byCPU;
	delete byLang;

	delete byBilling;
	delete byClusters;

	// firewall stats
#ifdef DEF_APP_FIREWALL
	OutDebug( "Clearing Firewall Stats" );
#endif
	delete byServers;
	delete byProtocol;
	delete byHTTP;
	delete byHTTPS;
	delete byMail;
	delete byFTP;
	delete byTelnet;
	delete byRealAudio;
	delete byDNS;
	delete byPOP3;
	delete byOthers;

}


//-------------------------------------------------------------------------------------------------
// Returns a pointer to the virtual domain's site URL string, if one is specified, otherwise
// returns 0.
//-------------------------------------------------------------------------------------------------
const char* VDinfo::GetSiteURL() const
{
	if( MyPrefStruct.multivhosts>0 && domainName[0] )
	{
		return domainName;
	}
		
	return MyPrefStruct.siteurl[0] ? MyPrefStruct.siteurl : 0;
}


//-------------------------------------------------------------------------------------------------
// Returns the CQFailedRequestInfo instance for the specified statistic, which hopefully, is either
// a broken link referal or a server error.
//-------------------------------------------------------------------------------------------------
CQFailedRequestInfo& VDinfo::GetFailedRequestInfo( Statistic* failedRequestStat )
{
	// and don't you forget it...
	DEF_ASSERT( failedRequestStat );
	DEF_ASSERT( ReportOn(BROKENLINKS_PAGE) || ReportOn(ERRORS_PAGE) );
	DEF_ASSERT( m_pFailedRequestInfoMap );

	// add/find CQFailedRequestInfo entry for stat
	FailedRequestInfoMap::iterator it( m_pFailedRequestInfoMap->find(failedRequestStat->GetHash()) );
	if( it==m_pFailedRequestInfoMap->end() )
	{
		it=m_pFailedRequestInfoMap->
			insert( FailedRequestInfoMap::value_type( failedRequestStat->GetHash(), CQFailedRequestInfo() ) ).first;
	}

	return it->second;
}


//-------------------------------------------------------------------------------------------------
// Returns the number of failed requests associated with the specified statistic, which hopefully,
// is either a broken link referal or a server error.
//-------------------------------------------------------------------------------------------------
size_t VDinfo::GetNumFailedRequests( Statistic* failedRequestStat )
{
	// and don't you forget it...
	DEF_ASSERT( failedRequestStat );
	DEF_ASSERT( ReportOn(BROKENLINKS_PAGE) || ReportOn(ERRORS_PAGE) );
	DEF_ASSERT( m_pFailedRequestInfoMap );

	// find CQFailedRequestInfo entry for stat
	FailedRequestInfoMap::iterator it( m_pFailedRequestInfoMap->find(failedRequestStat->GetHash()) );

	// NOTE: no CQFailedRequestInfo entry means no failed requests
	return it==m_pFailedRequestInfoMap->end() ? 0 : it->second.getNumFailedRequests();
}



// -------------------------------- STAT VALIDATION FUNCS --------------------------------------

short AnyTrafficStatsValid( VDinfo *VDptr )
{
	return (( VDptr->Done.Hour) ||
			( VDptr->Done.Hour) ||
			( VDptr->Done.Date) ||
			( VDptr->Done.Date) ||
			( VDptr->Done.Weekdays[0]) ||
			( VDptr->Done.Weekdays[1]) ||
			( VDptr->Done.Weekdays[2]) ||
			( VDptr->Done.Weekdays[3]) ||
			( VDptr->Done.Weekdays[4]) ||
			( VDptr->Done.Weekdays[5]) ||
			( VDptr->Done.Weekdays[6]) ||
			( VDptr->Done.Weekdays[7]) ||
			( VDptr->Done.Month  ));
	//return 1;
}
short AnyDiagnosticStatsValid( VDinfo *VDptr )
{
	return VDptr->Done.Errors ||
		   VDptr->Done.Errors  ||
		   VDptr->Done.ErrorURL ||
		   VDptr->Done.ErrorURL  ||
		   (VDptr->byBrokenLinkReferal && VDptr->byBrokenLinkReferal->GetStatListNum() );
	//return 1;
}
short AnyServerStatsValid( VDinfo *VDptr )
{
	if( VDptr->byFile ){
		return (( VDptr->Done.Pages  ) ||
				( VDptr->Done.Dir ) ||
				( VDptr->Done.TopDir ) ||
				( VDptr->Done.File ) ||
				( VDptr->Done.MeanPath ) ||
				( VDptr->Done.Groups ) ||
				( VDptr->Done.Download ) ||
				( VDptr->Done.Type ) );
	}
	else
		return 0;
}
short AnyDemographicStatsValid( VDinfo *VDptr )
{
	if( VDptr->Done.Client ) {
		return (( VDptr->Done.Client  ) ||
				( VDptr->Done.User ) ||
				( VDptr->Done.SecondDomain ) ||
				( VDptr->Done.Domain ) ||
				( VDptr->Done.Regions ) ||
				( VDptr->Done.Orgs ) ||
				( VDptr->Done.Robot ) 
			);
	}
	else
		return 0;
}

short AnyReferralStatsValid( VDinfo *VDptr )
{
	return (VDptr->Done.Refer ||
			VDptr->Done.ReferSite  ||
			VDptr->Done.SearchSite ||
			VDptr->Done.SearchStr );
}


short AnyMultiMediaStatsValid( VDinfo *VDptr )
{
	if( VDptr->byMediaTypes->GetStatListNum() ||
		 VDptr->byMediaPlayers->GetStatListNum() ){
		return 1;
	}
	else 
		return 0;
}

short AnyStreamingMediaStatsValid( VDinfo *VDptr )
{
	return 0;
}

short AnySystemsStatsValid( VDinfo *VDptr )
{
	return (( VDptr->Done.Browser  ) ||
			( VDptr->byUnrecognizedAgents && VDptr->byUnrecognizedAgents->TestAnyMatches() ) ||
			( VDptr->Done.OperSys) );
}
short AnyAdvertisingStatsValid( VDinfo *VDptr )
{
	return VDptr->byAdvert->GetStatListNum() || VDptr->byAdCamp->GetStatListNum();
}
short AnyMarketingStatsValid( VDinfo *VDptr )
{
	return (( VDptr->Done.Circulation ) ||
			( VDptr->Done.Loyalty ) ||
			( VDptr->Done.Timeon ) );
}

short AnyClustersStatsValid( VDinfo *VDptr )
{
	if( VDptr->byClusters && VDptr->byClusters->GetStatListNum()>0 )
		return 1;
	else
		return 0;
}

short AnyFirewallStatsValid( VDinfo *VDptr )
{
#ifdef DEF_APP_FIREWALL
	return (( VDptr->Done.SourceAddr ) ||
		( VDptr->Done.Protocol) ||
		( VDptr->Done.HTTP) ||
		( VDptr->Done.HTTPS ) ||
		( VDptr->Done.Mail) ||
		( VDptr->Done.FTP) ||
		( VDptr->Done.Telnet) ||
		( VDptr->Done.DNS) ||
		( VDptr->Done.POP3) ||
		( VDptr->Done.RealAudio ) ||
		( VDptr->Done.Others ));
#endif // DEF_APP_FIREWALL
	return 0;
}