#ifndef	__XLATECOUN
#define	__XLATECOUN
#endif

// String IDs for Language lookup

// Version 4.0fc5, 04/04/01
// Checked against English.lang in "Languages" directory in source safe
// every other file in Languages should have same entries
// please use version numbers so we know when a file is up to date

// If you change something here you have to change English.h and every other language file !!!

/* Change history */

//	08/03/01	RHF		4.0fc4, checked against English.lang which had several errors
//	04/04/01	RHF		4.0fc5, checked against English.lang, version info synchronised again, index error
//	05/09/01	RHF		4.5d1,  added HELP_ tags

enum
{
	SUMM_BEGIN				= 1,		// ss00
	SUMM_SERVERLOADSTATS,				// ss01
	SUMM_DETAILEDREPORTS,
	SUMM_REQUESTS,
	SUMM_DURATION,
	SUMM_DATERANGEFROM,
	SUMM_DATERANGETO,
	SUMM_TOTALREQS,
	SUMM_TOTALCACHEDREQS,
	SUMM_TOTALFAILREQS,
	SUMM_INVALLOGENTRIES,				// ss10
	SUMM_AVGDAILYREQS,
	SUMM_AVGREQSPERHR,
	SUMM_SESSIONSINFO,
	SUMM_TOTALSESSIONS,
	SUMM_TOTALUNIQUEVISITORS,
	SUMM_TOTALRPTVISITORS,
	SUMM_TOTALONETIMEVISITORS,
	SUMM_AVGDAILYSESSIONS,
	SUMM_AVGSESSIONLENGTH,
	SUMM_AVGPAGESPERSESSION,			// ss20
	SUMM_AVGREQSPERSESSION,
	SUMM_PAGESINFO,
	SUMM_TOTALPAGES,
	SUMM_AVGPAGESPERDAY,
	SUMM_TOTALDLOADFILES,
	SUMM_TOTALDOWNLOADMB,
	SUMM_BANDWIDTHOUT,
	SUMM_TOTALMB,
	SUMM_AVGDAILYMB,
	SUMM_AVGBITSPERSEC,					// ss30
	SUMM_PERCENTOF,
	SUMM_KBPS,
	SUMM_BANDWIDTHIN,
	SUMM_TOTALKB,
	SUMM_AVGDAILYKB,
	SUMM_ACCESSSTATS,				// ss36
	SUMM_LOGPROCESSTIME,
	SUMM_LOGLINESPERMIN,
	SUMM_TIMEPERIOD,
	SUMM_END,

	RCAT_BEGIN,							// rc00	
	RCAT_STATISTICS,					// rc01
	RCAT_TRAFFIC,
	RCAT_DIAGNOSTIC,
	RCAT_SERVER,
	RCAT_DEMOGRAPH,
	RCAT_REFERRALS,
	RCAT_STREAMING,
	RCAT_SYSTEMS,
	RCAT_ADVERT,
	RCAT_MARKETING,						// rc10
	RCAT_FIREWALL,						// rc11
	RCAT_CLUSTERS,
	RCAT_CLIPACCESSINFO,
	RCAT_LIVECLIPACCESSINFO,
	RCAT_END,
	
	REPORT_BEGIN,						// rl00
	REPORT_SUMMARY,						// rl01
	REPORT_HOURLY,
	REPORT_HOURLYHIST,
	REPORT_DAILY,
	REPORT_RECENTDAYS,
	REPORT_WEEKLY,
	REPORT_MONTHLY,
	REPORT_SERVERERRORS,
	REPORT_SERVERERRORSHIST,
	REPORT_FAILURLS,					// rl10
	REPORT_FAILURLSHIST,
	REPORT_PAGES,					
	REPORT_PAGESHIST,
	REPORT_ENTRY,
	REPORT_ENTRYHIST,
	REPORT_EXIT,
	REPORT_EXITHIST,
	REPORT_DIRS,
	REPORT_TOPLEVDIRS,
	REPORT_TOPLEVDIRSHIST,				// rl20
	REPORT_URLS,
	REPORT_MEANPATHS,
	REPORT_CONTENT,
	REPORT_CONTENTHIST,
	REPORT_DNLOADS,
	REPORT_DNLOADSHIST,
	REPORT_FILETYPES,
	REPORT_CLIENTS,
	REPORT_CLIENTSCLICK,
	REPORT_CLIENTSHIST,					// rl30
	REPORT_AUTHUSERS,
	REPORT_AUTHUSERSHIST,
	REPORT_DOMAINS,
	REPORT_COUNTRIES,
	REPORT_WORLDREGIONS,
	REPORT_ORGS,
	REPORT_SESSIONDEPTH,
	REPORT_ROBOTS,
	REPORT_REFERRALS,
	REPORT_REFERRALSITES,				// rl40
	REPORT_REFERRALSITESHIST,
	REPORT_SEARCHENGINES,
	REPORT_SEARCHTERMS,
	REPORT_BROWSERS,
	REPORT_BROWSERSOS,
	REPORT_OS,
	REPORT_LOYALTY,
	REPORT_TIMEONLINE,
	REPORT_IMPRESSIONS,
	REPORT_IMPRESSIONSHIST,				// rl50
	REPORT_CAMPAIGNS,
	REPORT_CAMPAIGNSHIST,
	REPORT_CIRCULATION,
	REPORT_MEDIAPLAYERS,
	REPORT_AUDIOCONTENT,
	REPORT_VIDEOCONTENT,
	REPORT_MEDIATYPES,
	REPORT_SOURCEADDRESSES,
	REPORT_PROTSUMMARY,
	REPORT_SUNDAY,						// rl60
	REPORT_MONDAY,
	REPORT_TUESDAY,
	REPORT_WEDNESDAY,
	REPORT_THURSDAY,
	REPORT_FRIDAY,
	REPORT_SATURDAY,
	REPORT_VIRTUALHOSTS,
	REPORT_VIRTUALHOSTSHIST,
	REPORT_PROTHTTP,
	REPORT_PROTHTTPS,					// rl70
	REPORT_PROTMAIL,
	REPORT_PROTFTP,
	REPORT_PROTTELNET,
	REPORT_PROTDNS,
	REPORT_PROTPOP3,
	REPORT_PROTREAL,
	REPORT_SRCADDR,
	REPORT_APPREPORT,
	REPORT_PROTOTHERS,			
	REPORT_PAGESLEAST,					// rl80		
	REPORT_KEYVISITORS,
	REPORT_KEYPAGEROUTE,
	REPORT_BROKENLINKS,
	REPORT_SEARCHENGINESHIST,
	REPORT_SEARCHTERMSHIST,
	REPORT_BROWSERSHIST,
	REPORT_OSHIST,
	REPORT_ROBOTSHIST,
	REPORT_MEDIAPLAYERSHIST,
	REPORT_AUDIOCONTENTHIST,			// rl90
	REPORT_VIDEOCONTENTHIST,
	REPORT_CLUSTER,
	REPORT_CLUSTERHIST,
	REPORT_UNRECOGNIZEDAGENTS,
	REPORT_VIRTUALHOSTSTITLE,
	REPORT_KEYPAGEROUTEFROM,
	REPORT_EXTBROKENLINKS,
	REPORT_INTBROKENLINKS,
	REPORT_AUTHUSERSCLICK,
	REPORT_KEYVISITORS_TABLE,			// rl100
	REPORT_ROUTESTO_TABLE,
	REPORT_ROUTESFROM_TABLE,
	// more reports to go in here
	REPORT_CLIPSERRORCODES,
	REPORT_CLIPSFAILED,
	REPORT_CLIPSFAILEDHIST,
	REPORT_PAGES_W_INVALIDCLIPS,
	REPORT_CLIPSPACKETLOSS,
	REPORT_CLIPSLOSSRATE,
	REPORT_CLIPSSECSBUFF,
	REPORT_CLIPSHIGH,
	REPORT_CLIPSLOW,

	REPORT_CLIPSVID,
	REPORT_CLIPSVIDHIST,
	REPORT_CLIPSVIDVIEW,
	REPORT_CLIPSVIDRATES,
	REPORT_CLIPSAUD,
	REPORT_CLIPSAUDHIST,
	REPORT_CLIPSAUDVIEW,
	REPORT_CLIPSAUDRATES,

	REPORT_LIVEVID,
	REPORT_LIVEVIDHIST,
	REPORT_LIVEVIDVIEW,
	REPORT_LIVEVIDRATES,
	REPORT_LIVEAUD,
	REPORT_LIVEAUDHIST,
	REPORT_LIVEAUDVIEW,
	REPORT_LIVEAUDRATES,

	REPORT_CLIPS,
	REPORT_CLIPSHIST,
	REPORT_CLIPSLEAST,
	REPORT_CLIPSPERCENT,
	REPORT_CLIPSMAXCON,
	REPORT_CLIPS_FF,
	REPORT_CLIPS_RW,
	REPORT_CLIPSCOMPLETED,
	REPORT_CLIPSPROTOCOLS,

	REPORT_VIDCODECS,
	REPORT_AUDCODECS,
	REPORT_MPLAYERVSOS,
	REPORT_CPU,
	REPORT_LANG,

	REPORT_BILLING,
	REPORT_UPLOAD,
	REPORT_END,							// rl103
	
	// General Labels
	LABEL_BEGIN,						// gl00
	LABEL_REQUESTS,						// gl01
	LABEL_SESSIONS,
	LABEL_FILE,
	LABEL_FILES,
	LABEL_TRAFFIC,
	LABEL_AVERAGE,
	LABEL_ERRORS,
	LABEL_TOTALUNITS,
	LABEL_CURRENCYUNIT,
	LABEL_RECENT,						// gl10
	LABEL_RECENTTOTALS,
	LABEL_DIRECTORY,
	LABEL_OTHER,
	LABEL_FILENOTFOUND,
	LABEL_HIGH,
	LABEL_LOW,
	LABEL_CLIENT,
	LABEL_CLIENTS,
	LABEL_CIRCULATION,
	LABEL_FREQUENCY,					// gl20
	LABEL_VISIT,
	LABEL_VISITS,
	LABEL_VISITORS,
	LABEL_BYTES,
	LABEL_PAGE,
	LABEL_PAGES,
	LABEL_OTHERS,
	LABEL_TOTALS,
	LABEL_SUBTOTALS,
	LABEL_REFERRAL,						// gl30
	LABEL_COUNTRY,
	LABEL_REGION,
	LABEL_PATH,
	LABEL_DOMAIN,
	LABEL_ORGANIZATION,
	LABEL_DOWNLOADFILE,
	LABEL_TOTALCOST,
	LABEL_PERCENTOF,
	LABEL_TOPLEVDIRECTORY,
	LABEL_FILETYPE,						// gl40
	LABEL_SERVERERROR,
	LABEL_FAILEDURL,
	LABEL_VIRTUALHOST,
	LABEL_URL,
	LABEL_BROWSER,
	LABEL_OPERATINGSYSTEM,
	LABEL_MEANPATH,
	LABEL_ROBOT,
	LABEL_SEARCHTERM,
	LABEL_SEARCHENGINE,					// gl50
	LABEL_MEDIAPLAYER,
	LABEL_AUDIOCONTENT,
	LABEL_VIDEOCONTENT,
	LABEL_MEDIATYPE,
	LABEL_TOTALSESSIONS,
	LABEL_CLICKTHROUGHS,
	LABEL_COSTPERMONTH,
	LABEL_FIRSTSESSIONS,
	LABEL_LASTSESSIONS,
	LABEL_CLICKS,						// gl60
	LABEL_COSTPERCLICK,
	LABEL_COST,
	LABEL_SEARCHES,
	LABEL_BYTESSENT,
	LABEL_TIMEONLINE,
	LABEL_CONTENTGROUP,
	LABEL_USER,
	LABEL_ADNAME,
	LABEL_HTTPCONN,
	LABEL_HTTPSCONN,					// gl70
	LABEL_MAILCONN,
	LABEL_FTPCONN,
	LABEL_TELNETCONN,
	LABEL_DNSCONN,
	LABEL_POP3CONN,
	LABEL_REALCONN,
	LABEL_SRCADDR,
	LABEL_MILLIONS,
	LABEL_KB,
	LABEL_MB,							// gl80
	LABEL_GB,
	LABEL_BPS,
	LABEL_KBPS,
	LABEL_MBPS,
	LABEL_GBPS,
	LABEL_PERCENT,
	LABEL_PROTOCOLS,
	LABEL_BYTESDELIVERED,
	LABEL_OTHERSCONN,
	LABEL_PAGESLEAST,					// gl90
	LABEL_KEYVISITORS,
	LABEL_KEYPAGEROUTE,
	LABEL_BROKENLINKS,
	LABEL_ROUTE,
	LABEL_TIMESACCESSED,
	LABEL_PERCENT_ROUTES,
	LABEL_ROUTESTAKENTO,
	LABEL_ALLOTHERROUTES,
	LABEL_CLUSTER,
	LABEL_UNRECOGNIZEDAGENTS,			// gl100
	LABEL_BROKENLINKREQUESTS,
	LABEL_BROKENLINK,
	LABEL_CACHEDHITS,
	LABEL_KEYPAGEROUTEFROM,
	LABEL_ROUTESTAKENFROM,
	LABEL_REQUESTSREFERRED,
	LABEL_TOPREFERRALSOF,
	LABEL_OF,

	LABEL_VIDEOCLIP,					
	LABEL_AUDIOCLIP,					// gl110
	LABEL_CLIP,
	LABEL_CLIPS,
	LABEL_AVGTIMEVIEW,
	LABEL_CLIPLENGTH,
	LABEL_PERCPLAYED,
	LABEL_PERCSTREAMED,
	LABEL_NUMOFPLAYS,
	LABEL_AVGTIMELISTENING,
	LABEL_AVGBPSRATE,
	LABEL_AVGPACKETLOSS,				// gl120
	LABEL_AVGSECSBUFFERED,
	LABEL_TOTALPACKETS_TX,
	LABEL_TOTALPACKETS_RX,
	LABEL_TOTALMBSTREAMED,
	LABEL_TOTALMBEXPECTED,
	LABEL_MOSTCOMMONERROR,
	LABEL_MAXCONNECTIONS,
	LABEL_TIMESTAMP,
	LABEL_AUDIOCODEC,
	LABEL_VIDEOCODEC,					// gl130
	LABEL_CPUTYPE,
	LABEL_LANGUAGE,
	LABEL_NUMOF_FF,
	LABEL_NUMOF_RW,
	LABEL_AVG_FF,
	LABEL_AVG_RW,
	LABEL_TRANSFERQUALITY,
	LABEL_ATTEMPTEDPLAYS,
	LABEL_PLAYS,						// gl140
	LABEL_UNINTERRUPTEDPLAYS,
	LABEL_TOTALPLAYS,
	LABEL_NA,
	LABEL_BILLING,

	LABEL_BILLING_CUSTOMER_NAME,
	LABEL_BILLING_CUSTOMER_ID,
	LABEL_BILLING_BYTES_TRANSFERRED,
	LABEL_BILLING_BYTE_ALLOWANCE,
	LABEL_BILLING_EXCESS_CHARGE,
	LABEL_BILLING_TOTAL_CHARGE,

	LABEL_END,							// gl145

	// time-related
	TIME_BEGIN,							// ti00
	TIME_SUNDAY,						// ti01
	TIME_MONDAY,
	TIME_TUESDAY,
	TIME_WEDNESDAY,
	TIME_THURSDAY,
	TIME_FRIDAY,
	TIME_SATURDAY,
	TIME_SUN,
	TIME_MON,
	TIME_TUE,							// ti10
	TIME_WED,
	TIME_THU,
	TIME_FRI,
	TIME_SAT,
	TIME_JANUARY,
	TIME_FEBRUARY,
	TIME_MARCH,
	TIME_APRIL,
	TIME_MAY,
	TIME_JUNE,							// ti20
	TIME_JULY,
	TIME_AUGUST,
	TIME_SEPTEMBER,
	TIME_OCTOBER,
	TIME_NOVEMBER,
	TIME_DECEMBER,
	TIME_JAN,
	TIME_FEB,
	TIME_MAR,
	TIME_APR,							// ti30
	TIME_MAYSHORT,
	TIME_JUN,
	TIME_JUL,
	TIME_AUG,
	TIME_SEP,
	TIME_OCT,
	TIME_NOV,
	TIME_DEC,
	TIME_TIME,
	TIME_DURATION,						// ti40
	TIME_SEC,
	TIME_MIN,
	TIME_HOUR,
	TIME_DAYLC,
	TIME_DAY,
	TIME_DAYS,
	TIME_DATE,
	TIME_MONTH,
	TIME_MONTHS,
	TIME_HOURSOFTHEDAY,					// ti50
	TIME_DAYSOFTHEWEEK,
	TIME_MEANTIME,
	TIME_TOTALTIME,
	TIME_DURATIONMINS,
	TIME_MANYTIMESADAY,
	TIME_TWICEADAY,
	TIME_ONCEADAY,
	TIME_TWICEAWEEK,
	TIME_ONCEAWEEK,
	TIME_TWICEAMONTH,					// ti60
	TIME_ONCEAMONTH,
	TIME_RARELY,
	TIME_ONCEONLY,						// ti63
	TIME_MINS,
	TIME_WEEKSTARTING,
	TIME_END,
	
	// sessions
	SESS_BEGIN,							// se00
	SESS_SESSLISTINGFOR,				// se01
	SESS_AVGSESSLENGTH,
	SESS_SESSLIST,
	SESS_TOTALDAYS,
	SESS_TOTALBYTESUSED,
	SESS_TOTALBYTESSENT,
	SESS_AVGBYTESPERDAY,
	SESS_TOTALONLINETIME,
	SESS_AVGONLINETIME,
	SESS_ESTSESSLENGTH,					// se10
	SESS_SESSION,
	SESS_BYTESUSED,
	SESS_AVGPAGESPERSESSION,			// se13
	SESS_ATTEPTDOWNLOADS,
	SESS_TOTALATTEPTDOWNLOADS,
	SESS_BROWSERUSED,
	SESS_OPERSYSUSED,					// se17
	SESS_END,

	COMM_BEGIN,							// cm00
	COMM_CLICKSESSIONHIST,				// cm01 - NOTE: Click on the clients to show individual Session History
	COMM_CLICKUSERHIST,					// cm02 - NOTE: Click on the clients to show individual User History
	COMM_NOPAGESVIEWED,					// cm03 - No pages viewed during this session
	COMM_UNRESOLVEDIP,
	COMM_INVALID,
	COMM_LOCALNET,
	COMM_SERVERERRORS,
	COMM_UNKNOWNHOST,
	COMM_OK,
	COMM_ERR,							// cm10
	COMM_NOREFERRER,
	COMM_BOOKMARKED,
	COMM_OTHERS,
	COMM_UNKNOWN,
	COMM_CONTNEXTPAGE,
	COMM_CONTPREVPAGE,					// cm16
	COMM_END,

	// server errors
	SERR_BEGIN,
	SERR_000,							// er01
	SERR_201,
	SERR_202,
	SERR_204,
	SERR_206,
	SERR_301,
	SERR_302,
	SERR_304,
	SERR_400,
	SERR_401,							// er10
	SERR_403,							
	SERR_404,
	SERR_405,
	SERR_406,
	SERR_407,
	SERR_408,
	SERR_500,
	SERR_501,
	SERR_502,
	SERR_503,							// er20
	SERR_331,
	SERR_332,
	SERR_350,
	SERR_421,
	SERR_425,
	SERR_426,
	SERR_450,
	SERR_451,
	SERR_452,
	SERR_504,							// er30
	SERR_530,
	SERR_532,
	SERR_550,
	SERR_551,
	SERR_552,
	SERR_553,							// er36
	SERR_END,							// er37
	
	// system info
	SYSI_BEGIN,							// sy00
	SYSI_CHARSET,
	SYSI_GRAPHFONTNAME,					// sy02
	SYSI_END,							

	// Countries
	COUN_BEGIN,
	COUN_ANDORRA,						// {co01}
	COUN_UNITED_ARAB_EMIRATES,
	COUN_AFGHANISTAN,
	COUN_ANTIGUA,
	COUN_ANGUILLA,
	COUN_ALBANIA,
	COUN_ARMENIA,
	COUN_NETHERLANDS,
	COUN_ANGOLA,
	COUN_ANTARCTICA,					// {co10}
	COUN_ARGENTINA,
	COUN_ADV_PROJ_RES_AGENCY,
	COUN_AMERICAN_SAMOA,
	COUN_AUSTRIA,
	COUN_AUSTRALIA,
	COUN_ARUBA,
	COUN_AZERBAIJAN,
	COUN_BOSNIA_HERZ,
	COUN_BARBADOS,
	COUN_BANGLADESH,					// {co20}
	COUN_BELGIUM,
	COUN_BURKINA_FASO,
	COUN_BULGARIA,
	COUN_BAHRAIN,
	COUN_BURUNDI,
	COUN_BENIN,
	COUN_BERMUDA,
	COUN_BRUNEI,
	COUN_BOLIVIA,
	COUN_BRAZIL,						// {co30}
	COUN_BAHAMAS,
	COUN_BHUTAN,
	COUN_BOUVET,
	COUN_BOTSWANA,
	COUN_BELARUS,
	COUN_BYELORUSSIAN,
	COUN_BELIZE,
	COUN_CANADA,
	COUN_COCOS_ISLANDS,
	COUN_CONGO,							// {co40}
	COUN_CENTRAL_AFRICA,
	COUN_SWITSERLAND,
	COUN_COTE_D_IVOIRE,
	COUN_COOK_ISLANDS,
	COUN_CHILE,
	COUN_CAMEROON,
	COUN_CHINA,
	COUN_COLOMBIA,
	COUN_COSTA_RICA,
	COUN_CZECHOSLAVAKIA,				// {co50}
	COUN_CUBA,
	COUN_CAPE_VERDA,
	COUN_CHRISMAS_ISLAND,
	COUN_CYPRUS,
	COUN_CZECH_REPUBLIC,
	COUN_GERMANY,
	COUN_DJIBOUTI,
	COUN_DENMARK,
	COUN_DOMINICA,
	COUN_DOMINICAN_REP,					// {co60}
	COUN_ALGERIA,
	COUN_ECUADOR,
	COUN_US_EDUCATIONAL,
	COUN_ESTONIA,
	COUN_EGYPT,
	COUN_WESTERN_SAHARA,
	COUN_ERITREA,
	COUN_SPAIN,
	COUN_ETHIOPIA,
	COUN_FINLAND,						// {co70}
	COUN_FIJI,
	COUN_FALKLAND_ISLANDS,
	COUN_MICRONESIA,
	COUN_FAROS_ISLANDS,
	COUN_FRANCE,
	COUN_FRANCE_METRO,
	COUN_GABON,
	COUN_UNITED_KINGDOM,
	COUN_GRENADA,
	COUN_GEORGIA,						// {co80}
	COUN_FRENCH_GUIANA,
	COUN_GHANA,
	COUN_GIBRALTAR,
	COUN_GREENLAND,
	COUN_GAMBIA,
	COUN_GUINEA,
	COUN_US_GOVT,
	COUN_GUADELOUPE,
	COUN_EQUATORIAL_GUINEA,
	COUN_GREEK,							// {co90}
	COUN_SOUTH_GEORGIA,
	COUN_GUATEMALA,
	COUN_GUAM,
	COUN_GUINEA_BISSAU,
	COUN_GUYANA,
	COUN_HONG_KONG,
	COUN_HEARD_MAC_ISLANDS,
	COUN_HONDURUS,
	COUN_CROATIA,
	COUN_HAITI,							// {co100}
	COUN_HUNGARY,
	COUN_INDONESIA,
	COUN_IRELAND,
	COUN_ISRAEL,
	COUN_INDIA,
	COUN_INTERNATIONAL,
	COUN_BRITISH_INDIAN_TERR,
	COUN_IRAQ,
	COUN_IRAN,
	COUN_ICELAND,						// {co110}
	COUN_ITALY,
	COUN_JAMAICA,
	COUN_JORDAN,
	COUN_JAPAN,
	COUN_KENYA,
	COUN_KYRGYZSTAN,
	COUN_CAMBODIA,
	COUN_KIRIBATI,
	COUN_COMOROS,
	COUN_SAINT_KITTS_NEVIS,				// {co120}
	COUN_KOREA,
	COUN_KUWAIT,
	COUN_CAYMAN_ISLANDS,
	COUN_KAZAKHSTAN,
	COUN_LAOS,
	COUN_LEBANON,
	COUN_SAINT_LUCIA,
	COUN_LIECHTENSTEIN,
	COUN_SRI_LANKA,
	COUN_LIBERIA,						// {co130}
	COUN_LESOTHO,
	COUN_LITHUANIA,
	COUN_LUXEMBOURG,
	COUN_LATVIA,
	COUN_LIBYA,
	COUN_MOROCCO,
	COUN_MONACO,
	COUN_MOLDOVA,
	COUN_MADAGASCAR,
	COUN_MARSHALL_ISLAND,				// {co140}
	COUN_MACEDONIA,
	COUN_MALI,
	COUN_MYANMAR,
	COUN_MONGOLIA,
	COUN_MACAU,
	COUN_NORTHERN_MARIANA_IS,
	COUN_MARTINIQUE,
	COUN_MAURITANIA,
	COUN_MONTSERRAT,
	COUN_MALTA,							// {co150}
	COUN_MAURITIUS,
	COUN_MALDIVES,
	COUN_MALAWI,
	COUN_MEXICO,
	COUN_MALYSIA,
	COUN_MOZAMBIQUE,
	COUN_NAMIBIA,
	COUN_NATO,
	COUN_NEW_CALEDONIA,
	COUN_NIGER,							// {co160}
	COUN_NORFORK_ISLAND,
	COUN_NIGERIA,
	COUN_NICARAGUA,
	COUN_NETEHRLANDS,
	COUN_NORWAY,
	COUN_NEPAL,
	COUN_NAURU,
	COUN_NEUTRAL_ZONE,
	COUN_NIUE,
	COUN_NEW_ZEALAND,					// {co170}
	COUN_OMAN,
	COUN_US_ORGANISATION,
	COUN_PANAMA,
	COUN_PERU,
	COUN_FRENCH_POLYNESIA,
	COUN_PAPUA_NEW_GUINEA,
	COUN_PHILIPPINES,
	COUN_PAKISTAN,
	COUN_POLAND,
	COUN_ST_PIERRE_MIQUELON,			// {co180}
	COUN_PITCAIRN,
	COUN_PUERTO_RICO,
	COUN_PORTUGAL,
	COUN_PALAU,
	COUN_PARAGUAY,
	COUN_QATAR,
	COUN_REUNION,
	COUN_ROMANIA,
	COUN_RUSSIA,
	COUN_RWANDA,						// {co190}
	COUN_SAUDI_ARABIA,
	COUN_SOLOMON_ISLANDS,
	COUN_SEYCHELLES,
	COUN_SUDAN,
	COUN_SWEDEN,
	COUN_SINGAPORE,
	COUN_ST_HELENA,
	COUN_SOLENIA,
	COUN_SVALBARD,
	COUN_SLOVAKIA,						// {co200}
	COUN_SIERRA_LEONE,
	COUN_SAN_MARINO,
	COUN_SENEGAL,
	COUN_SOMALIA,
	COUN_SURINAME,
	COUN_SAO_TOME_PRINCIPE,
	COUN_SOVIET_UNION,
	COUN_EL_SALVADOR,
	COUN_SYRIA,
	COUN_SWAZILAND,						// {co210}
	COUN_TURKS_CAICOS_ISLANDS,
	COUN_CHAD,
	COUN_FRENCH_SOUTH_TERR,
	COUN_TOGO,
	COUN_THAILAND,
	COUN_TAJIKISTAN,
	COUN_TOKELAU,
	COUN_TURKMENISTAN,
	COUN_TUNISIA,
	COUN_TONGA,							// {co220}
	COUN_EAST_TIMOR,
	COUN_TURKEY,
	COUN_TRINIDAD_AND_TOBAGO,
	COUN_TUVALU,
	COUN_TAIWAN,
	COUN_TANZANIA,
	COUN_UKRAINE,
	COUN_UGANDA,
	COUN_US_MINOR_ISLANDS,
	COUN_USA,							// {co230}
	COUN_URUGUAY,
	COUN_UZBEKISTAN,
	COUN_VATICAN_CITY,
	COUN_SAINT_VINCENT_GRENADINES,
	COUN_VENEZUELA,
	COUN_VIRGIN_ISLANDS_BR,
	COUN_VIRGIN_ISLANDS_US,
	COUN_VIETNAM,
	COUN_VANUATU,
	COUN_WALLIS_FUTUNA_ISLANDS,			// {co240}
	COUN_SAMOA,
	COUN_YEMEN,
	COUN_MAYOTTE,
	COUN_SOUTH_AFRICA,
	COUN_ZAMBIA,
	COUN_ZAIRE,
	COUN_ZIMBABWE,
	COUN_US_COMMERCIAL,
	COUN_US_MILITARY,
	COUN_NETWORK,						// {co250}
	COUN_YUGOSLAVIA,					// {co251}

	COUN_BUSINESSES,	
	COUN_INDIVIDUALS,	
	COUN_MUSEUMS,		
	COUN_PROFESSIONSALS,
	COUN_AVIATION,		
	COUN_COOPERATIVES,	
	COUN_GNERALINFO,					// 258
	
	COUN_END,

	// Organisations
	ORGS_BEGIN,							// or00
	ORGS_US_ORGANISATIONS,				// or01
	ORGS_ORGANISATIONS,
	ORGS_US_EDUCATION,
	ORGS_EDUCATION,
	ORGS_US_MILITARY,
	ORGS_MILITARY,
	ORGS_US_GOVERNMENT,
	ORGS_GOVERNMENT,
	ORGS_US_COMMERCIAL,
	ORGS_COMMERCIAL,					// or10
	ORGS_US_NETWORK,
	ORGS_NETWORK,
	ORGS_PROXY_SERVER,
	ORGS_SEARCH_ROBOTS,					// or14
	ORGS_END,							// or15

	END_OF_STRINGS
};
