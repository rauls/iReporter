// ****************************************************************************
// Copyright (C) 2000-2004
//
// File 	:	HelpTextMap.cpp
// Created 	:	Monday, 8 October 2001
// Author 	:	Julien Crawford [JMC]
//
// Abstract :	This file implements the mapping of the Control Ids to HelpText
//				defined in the HelpText.xml file.
//				We have hardcoded a list of ControlIds to Location-descriptions
//				and the xml file contains the Location-descriptions to Helptext.
//				We join these tables and populate a map to use as our lookup
//				table.
//
// ****************************************************************************

#include "FWA.h"
#include "HelpTextMap.h"
#include "ResDefs.h"	// for GetString() etc

#include <map>
#include <string>
#include "qsxml.h"		// To read in the XML file.
#include "GlobalPaths.h"

// ****************************************************************************
// This is the lookup table of ControlId's to Location-descriptions.
// ****************************************************************************
typedef struct GadgetMap_
{
	const char*		szGadgetDesc;
	unsigned int	uiControlId;
}	GadgetMap;

const GadgetMap g_GadgetMap[] = 
{
#ifdef	DEF_WINDOWS
    {"Schedule/Button/Cancel"				,					IDC_SCHED_CANCEL					},
    {"Schedule/Button/Save"					,					IDC_SCHED_OK						},
    {"Schedule/Button/Transcript"			,					IDC_SCHED_VIEWLOG					},
    {"Schedule/Button/Properties"			,					IDC_SCHED_EDIT						},
    {"Schedule/Button/Runnow"				,					IDC_SCHED_RUNNOW					},
    {"Schedule/Button/Remove"				,					IDC_SCHED_DEL						},
    {"Schedule/Button/New"					,					IDC_SCHED_ADD						},
    {"Schedule/Checkbox/Disable"			,					IDC_SCHED_DISABLE					},
    {"Schedule/Listctrl/List"				,					IDC_SCHED_LISTRECT					},
    {"ScheduleEdit/Edit/Name"				,					IDC_SCHED_NAME						},
    {"ScheduleEdit/Edit/Startdate"			,					IDC_SCHED_DATE						},
    {"ScheduleEdit/Edit/Starttime"			,					IDC_SCHED_TIME						},
    {"ScheduleEdit/Spin/Startdate"			,					IDC_SCHED_DATEADJUST				},
    {"ScheduleEdit/Spin/Starttime"			,					IDC_SCHED_TIMEADJUST				},
    {"ScheduleEdit/Edit/Every"				,					IDC_SCHED_NUM						},
    {"ScheduleEdit/Combo/Period"			,					IDC_SCHED_NUMTYPE					},
    {"ScheduleEdit/Edit/Logs"				,					IDC_SCHED_LOGFILE					},
//    {"ScheduleEdit/Button/AddDB"			,					IDC_SCHED_ADDDB						},
//    {"ScheduleEdit/Button/SelectDB"			,					IDC_SCHED_SELECTDB					},
    {"ScheduleEdit/Button/Add"				,					IDC_SCHED_ADDLOG					},
    {"ScheduleEdit/Button/Select"			,					IDC_SCHED_SELECTLOG					},
    {"ScheduleEdit/Edit/Settings"			,					IDC_SCHED_PREFSFILE					},
    {"ScheduleEdit/Button/Browse"			,					IDC_SCHED_SELECTPREFS				},
    {"ScheduleEdit/Button/Editsettings"		,					IDC_SCHED_EDITPREFS					},
    {"ScheduleEdit/Edit/UploadURL"			,					IDC_SCHED_UPLOAD					},
    {"ScheduleEdit/Edit/Reportfile"			,					IDC_SCHED_REPORTFILE				},
    {"ScheduleEdit/Button/View"				,					IDC_SCHED_VIEW						},
    {"ScheduleEdit/Checkbox/Enabled"		,					IDC_SCHED_ACTIVE					},
    {"ScheduleEdit/Button/Ok"				,					IDC_SCHED_EDITOK					},

	{"Settings/Button/Cancel",									IDC_PREFCANCEL						},
	{"Settings/Button/Ok",										IDC_PREFOK							},
	{"Settings/Button/Defaults",								IDC_PREFDEFAULT						},
	{"Settings/Button/Revert",									IDC_PREFREVERT						},
	{"Preprocess/Download/Checkbox/Enabledownload",				IDC_PRE_ON							},
	{"Preprocess/Download/Edit/Hostname",						IDC_PRE_HOST						},
	{"Preprocess/Download/Edit/Username",						IDC_PRE_USER						},
	{"Preprocess/Download/Edit/Password",						IDC_PRE_PASSWD						},
	{"Preprocess/Download/Edit/Path",							IDC_PRE_PATH						},
	{"Preprocess/Options/Checkbox/Deletefile",					IDC_PREPROC_DEL						},
	{"Preprocess/Options/Edit/Download",						IDC_PREPROC_DEST					},
	{"Preprocess/Options/Button/Browse",						IDC_PREPROC_BROWSE					},
	{"Report/Output/Edit/Location",								IDC_OUTDIR							},
	{"Report/Output/Edit/PathTokens",							IDC_TOKENS							},
	{"Report/Output/Button/Saveas",								IDC_PREFGEN_OUT						},
	{"Report/Output/Button/Insert",								IDC_PREFGEN_INSERT					},
	{"Report/Output/Database/Radio/None",						IDC_DB_NONE							},
//	{"Report/Output/Database/Radio/Select",						IDC_DB_SELECT						},
	{"Report/Output/Database/Edit/Name",						IDC_DB_NAME							},
	{"Report/Output/Database/Button/Delete",					IDC_DB_DELETE						},
	{"Report/Output/Database/Button/Browse",					IDC_DB_BROWSE						},
	{"Report/Output/Database/Checkbox/NewLogsOnly",				IDC_DB_EXCLUDE						},
	{"Report/Output/Combo/Language",							IDC_PREF_LANG						},
	{"Report/Output/Combo/Format",								IDC_PREFGEN_TYPE					},
	{"Report/Output/Combo/Graphics",							IDC_PREFGEN_GTYPE					},
	{"Report/Page/Display/Checkbox/Bandwidth",					IDC_PREF_BANDWIDTH					},
	{"Report/Page/Display/Checkbox/Logname",					IDC_PREF_HEADTITLE					},
	{"Report/Page/Display/Checkbox/Timestamp",					IDC_PREF_TIMESTAT					},
	{"Report/Page/Display/Checkbox/Keyonleft",					IDC_PREF_LEFTSIDE					},
	{"Report/Page/Display/Checkbox/QuickLinks",					IDC_PREF_QINDEX						},
	{"Report/Page/Display/Checkbox/Reportshadows",				IDC_PREF_SHADOW						},
	{"Report/Page/Display/Checkbox/Footerlabel",				IDC_PREF_FOOTERLABEL				},
	{"Report/Page/Display/Checkbox/Trailinglabel",				IDC_PREF_FOOTERLABELTRAIL			},
	{"Report/Page/Display/Edit/Reporttitle",					IDC_REPORTTITLE						},
	{"Report/Page/Colors/Listbox/Theme",						IDC_SCHEMES							},
	{"Report/Page/Colors/Button/Title",							IDC_PREFHTML_RGB1					},
	{"Report/Page/Colors/Button/Headers",						IDC_PREFHTML_RGB2					},
	{"Report/Page/Colors/Button/Items",							IDC_PREFHTML_RGB3					},
	{"Report/Page/Colors/Button/Others",						IDC_PREFHTML_RGB4					},
	{"Report/Page/Colors/Button/Average",						IDC_PREFHTML_RGB5					},
	{"Report/Page/Colors/Button/Totals",						IDC_PREFHTML_RGB6					},
	{"Report/Pdf/Style/Page/Combo/Font",						IDC_PREF_PDF_FONT					},
	{"Report/Pdf/Style/Page/Combo/Numbers",						IDC_PREF_PDF_PAGENUMBERINGPOS		},
	{"Report/Pdf/Style/Page/Combo/Size",						IDC_PREF_PDF_PAGESIZE				},
	{"Report/Pdf/Style/Page/Edit/Width",						IDC_PREF_PDF_PAGEWIDTH				},
	{"Report/Pdf/Style/Page/Edit/Height",						IDC_PREF_PDF_PAGEHEIGHT				},
	{"Report/Pdf/Style/Page/Edit/Left",							IDC_PREF_PDF_LEFTMARGIN				},
	{"Report/Pdf/Style/Page/Edit/Right",						IDC_PREF_PDF_RIGHTMARGIN			},
	{"Report/Pdf/Style/Page/Edit/Top",							IDC_PREF_PDF_TOPMARGIN				},
	{"Report/Pdf/Style/Page/Edit/Bottom",						IDC_PREF_PDF_BOTTOMMARGIN			},
	{"Report/Pdf/Style/Page/Checkbox/Image",					IDC_PREF_PDF_SHOWBANNER				},
	{"Report/Pdf/Style/Page/Edit/Filename",						IDC_PREF_PDF_BANNERFILE				},
	{"Report/Pdf/Style/Page/Button/Browse",						IDC_PREF_PDF_BROWSEBANNERFILE		},
	{"Report/Pdf/Style/Table/Checkbox/Wrapping",				IDC_PREF_PDF_TEXTWRAP				},
	{"Report/Pdf/Style/Table/Button/Advanced",					IDC_PREF_PDF_ADVANCED				},
	{"Report/Pdf/Style/Table/Combo/Truncation",					IDC_PREF_PDF_TRUNCATETEXT			},
	{"Report/Pdf/Style/Table/Combo/Section",					IDC_PREF_PDF_TABLETEXTNAME			},
	{"Report/Pdf/Style/Table/Combo/Size",						IDC_PREF_PDF_FONTSIZE				},
	{"Report/Pdf/Style/Table/Combo/Style",						IDC_PREF_PDF_FONTSTYLE				},
	{"Report/Pdf/Style/Table/Combo/Color",						IDC_PREF_PDF_FONTCOLOR				},
	{"Report/Pdf/Style/Spacing/Combo/Spacing",					IDC_PREF_PDF_SPACING				},
	{"Report/Pdf/Style/Spacing/Edit/Pixels",					IDC_PREF_PDF_SPACINGSIZE			},
	{"Report/Pdf/Style/Graphtext/Combo/Name",					IDC_PREF_PDF_GRAPHFONT				},
	{"Report/Pdf/Style/Graphtext/Combo/Size",					IDC_PREF_PDF_GRAPHFONTSIZE			},
	{"Report/Style/Button/BgColor",								IDC_PREFHTML_RGB7					},
	{"Report/Style/Button/Text",								IDC_PREFHTML_RGB8					},
	{"Report/Style/Button/Link",								IDC_PREFHTML_RGB9					},
	{"Report/Style/Button/ALink",								IDC_PREFHTML_RGB10					},
	{"Report/Style/Button/VLink",								IDC_PREFHTML_RGB11					},
	{"Report/Style/Edit/Header",								IDC_PREFHTML_HEAD					},
	{"Report/Style/Edit/Footer",								IDC_PREFHTML_FOOT					},
	{"Analysis/Options/Date/Combo/Option",						IDC_PREFGEN_DATESTYLE				},
	{"Analysis/Options/Date/Edit/Start",						IDC_DATESTART						},
	{"Analysis/Options/Date/Edit/End",							IDC_DATEEND							},
	{"Analysis/Options/Date/Combo/Reports",						IDC_PREFGEN_SPLITDATE				},
	{"Analysis/Options/Date/Combo/Format",						IDC_DATEOVERIDE						},
	{"Analysis/Options/Date/Edit/Adjust",						IDC_DATEOFFSET						},
	{"Analysis/Options/Ignore/Checkbox/Zerobytes",				IDC_PREFGEN_FILTERZERO				},
	{"Analysis/Options/Ignore/Checkbox/Selfreferal",			IDC_PREFGEN_IGNORESELF				},
	{"Analysis/Options/Ignore/Checkbox/Urlcase",				IDC_PREFGEN_IGNORECASE				},
	{"Analysis/Options/Ignore/Checkbox/Clientusernames",		IDC_PREFGEN_IGNOREUSERS				},
	{"Analysis/Options/Ignore/Checkbox/Bookmarks",				IDC_PREFGEN_IGNOREBOOKMARK			},
	{"Analysis/Options/Url/Defaulthost",						IDC_PREFGEN_URL						},
	{"Analysis/Options/Url/Defaultpage",						IDC_PREFGEN_INDEXFILE				},
	{"Analysis/Network/Dns/Radio/Lookupsoff",					IDC_PREFGEN_DNSNO					},
	{"Analysis/Network/Dns/Radio/Lookupson",					IDC_PREFGEN_DNSYES					},
	{"Analysis/Network/Dns/Edit/Simultaneous",					IDC_PREFGEN_DNSNUM					},
	{"Analysis/Network/Dns/Radio/Cache",						IDC_PREFGEN_DNSCACHE				},
	{"Analysis/Network/Dns/Button/Clearcache",					IDC_CLEARCACHE						},
	{"Analysis/Network/General/Passiveftp",						IDC_PREFFTP_PASSIVE					},
	{"Analysis/Network/General/Retrievetitles",					IDC_PREF_REMOTETITLE				},
	{"Analysis/Extensions/Treeview/Extensions",					IDC_EXTEDIT							},
	{"Analysis/Extensions/Button/Add",							IDC_GLOB_PAGEADD					},
	{"Analysis/Extensions/Button/Delete",						IDC_GLOB_PAGEREM					},
	{"Analysis/Extensions/Button/Edit",							IDC_GLOB_PAGEEDIT					},
	{"Analysis/Extensions/Button/Defaults",						IDC_GLOB_PAGEDEFAULT				},
	{"Analysis/Dynamic/Url/Checkbox/Retainall",					IDC_PREFGEN_USECGI					},
	{"Analysis/Dynamic/Url/Checkbox/Retainvariables",			IDC_PREFGEN_RETAINVAR				},
	{"Analysis/Dynamic/Url/Edit/Variables",						IDC_PREFGEN_RETAINVARLIST			},
	{"Analysis/Dynamic/Analysis/Checkbox/Flushing",				IDC_PREFGEN_FLUSH					},
	{"Filters/Inputdata/Combo/Include",							IDC_FILTERIN1						},
	{"Filters/Inputdata/Combo/Type",							IDC_FILTERIN2						},
	{"Filters/Inputdata/Combo/Contains",						IDC_FILTERIN3						},
	{"Filters/Inputdata/Edit/Data",								IDC_FILTERIN_DATA					},
	{"Filters/Inputdata/Listctrl/Filters",						IDC_FILTERIN_LIST					},
	{"Filters/Inputdata/Button/Add",							IDC_FILTERIN_ADD					},
	{"Filters/Inputdata/Button/Delete",							IDC_FILTERIN_REM					},
	{"Filters/Inputdata/Button/Change",							IDC_FILTERIN_CHANGE					},
	{"Statistics/Statistics/Treeview/All",						IDC_STATEDIT						},
	{"Statistics/Statistics/Button/Expandall",					IDC_BUTTON_EXPANDALL				},
	{"Statistics/Statistics/Button/Defaults",					IDC_STATDEFAULT						},
	{"Statistics/Statistics/Button/Applyall",					IDC_APPLYALL						},
	{"Statistics/Statistics/Checkbox/Table",					IDC_STAT_TABLE						},
	{"Statistics/Statistics/Combo/Rows",						IDC_TABSIZE							},
	{"Statistics/Statistics/Checkbox/Graph",					IDC_STAT_GRAPH						},
	{"Statistics/Statistics/Combo/Sorting",						IDC_SORTING							},
	{"Statistics/Statistics/Checkbox/HelpCard",					IDC_USEDESC							},
	{"Statistics/Statistics/Button/EditCard",					IDC_STATEDITDESC					},
	{"Statistics/Global/Graph/Checkbox/3d",						IDC_PREFGRAPH_3DBARS				},
	{"Statistics/Global/Graph/Checkbox/Wide",					IDC_PREFGRAPH_WIDER					},
	{"Statistics/Global/Graph/Checkbox/Webpallet",				IDC_PREFGRAPH_WEBPAL				},
	{"Statistics/Global/Graph/Checkbox/Paperprintable",			IDC_PREFGRAPH_CORP					},
	{"Statistics/Global/Sessions/Edit/Cost",					IDC_PREFVD_COST						},
	{"Statistics/Global/Sessions/Edit/Timeout",					IDC_PREFSTAT_SESSIONTIME			},
	{"Statistics/Global/Sessions/Edit/Cookie",					IDC_COOKIEVAR						},
	{"Statistics/Global/Sessions/Edit/Urlparameter",			IDC_URLVAR							},
	{"Statistics/Organizational/Edit/Pattern",					IDC_PREFORG_PATTERN					},
	{"Statistics/Organizational/Edit/Name",						IDC_PREFORG_NAME					},
	{"Statistics/Organizational/Listctrl/List",					IDC_PREFORG_LIST					},
	{"Statistics/Organizational/Button/Add",					IDC_PREFORG_ADD						},
	{"Statistics/Organizational/Button/Delete",					IDC_PREFORG_DEL						},
	{"Statistics/Organizational/Button/Change",					IDC_PREFORG_CHANGE					},
	{"Statistics/Organizational/Button/Defaults",				IDC_PREFORG_DEFAULT					},
	{"Statistics/Contentgroups/Edit/Pattern",					IDC_PREFGRP_PATTERN					},
	{"Statistics/Contentgroups/Edit/Name",						IDC_PREFGRP_NAME					},
	{"Statistics/Contentgroups/Listctrl/List",					IDC_PREFGRP_LIST					},
	{"Statistics/Contentgroups/Button/Add",						IDC_PREFGRP_ADD						},
	{"Statistics/Contentgroups/Button/Delete",					IDC_PREFGRP_REM						},
	{"Statistics/Contentgroups/Button/Change",					IDC_PREFGRP_CHANGE					},
	{"Statistics/Referalgroups/Edit/Pattern",					IDC_PREFREF_PATTERN					},
	{"Statistics/Referalgroups/Edit/Name",						IDC_PREFREF_NAME					},
	{"Statistics/Referalgroups/Listctrl/List",					IDC_PREFREF_LIST					},
	{"Statistics/Referalgroups/Button/Add",						IDC_PREFREF_ADD						},
	{"Statistics/Referalgroups/Button/Delete",					IDC_PREFREF_REM						},
	{"Statistics/Referalgroups/Button/Change",					IDC_PREFREF_CHANGE					},
	{"Routes/Keyvisitors/Edit/Name",							IDC_PREFKEYVISITOR_NAME				},
	{"Routes/Keyvisitors/Listctrl/List",						IDC_PREFKEYVISITOR_LIST				},
	{"Routes/Keyvisitors/Button/Add",							IDC_PREFKEYVISITOR_ADD				},
	{"Routes/Keyvisitors/Button/Delete",						IDC_PREFKEYVISITOR_DEL				},
	{"Routes/Keyvisitors/Button/Change",						IDC_PREFKEYVISITOR_CHANGE			},
	{"Routes/Keyvisitors/Button/Defaults",						IDC_PREFKEYVISITOR_DEFAULT			},
	{"Routes/Keypagesto/Edit/Page",								IDC_ROUTE2KEYPAGE_KEYPAGE			},
	{"Routes/Keypagesto/Edit/Traceback",						IDC_ROUTE2KEYPAGE_TRACEBACK			},
	{"Routes/Keypagesto/Edit/Number",							IDC_ROUTE2KEYPAGE_NUMBERTOREPORT	},
	{"Routes/Keypagesto/Listctrl/List",							IDC_LIST_KEYPAGES					},
	{"Routes/Keypagesto/Button/Add",							IDC_ROUTE2KEYPAGE_ADD				},
	{"Routes/Keypagesto/Button/Delete",							IDC_ROUTE2KEYPAGE_REM				},
	{"Routes/Keypagesto/Button/Change",							IDC_ROUTE2KEYPAGE_CHANGE			},
	{"Routes/Keypagesto/Button/Defaults",						IDC_ROUTE2KEYPAGE_CLEAR				},
	{"Routes/Keypagesfrom/Edit/Page",							IDC_ROUTEFROMKEYPAGE_KEYPAGE		},
	{"Routes/Keypagesfrom/Edit/Traceback",						IDC_ROUTEFROMKEYPAGE_TRACEBACK		},
	{"Routes/Keypagesfrom/Edit/Number",							IDC_ROUTEFROMKEYPAGE_NUMBERTOREPORT	},
	{"Routes/Keypagesfrom/Listctrl/List",						IDC_ROUTEFROMKEYPAGE_LIST			},
	{"Routes/Keypagesfrom/Button/Add",							IDC_ROUTEFROMKEYPAGE_ADD			},
	{"Routes/Keypagesfrom/Button/Delete",						IDC_ROUTEFROMKEYPAGE_REM			},
	{"Routes/Keypagesfrom/Button/Change",						IDC_ROUTEFROMKEYPAGE_CHANGE			},
	{"Routes/Keypagesfrom/Button/Defaults",						IDC_ROUTEFROMKEYPAGE_CLEAR			},
	{"Advertising/Impressions/Edit/Description",				IDC_PREFADD_NAME					},
	{"Advertising/Impressions/Edit/Filename",					IDC_PREFADD_PATTERN1				},
	{"Advertising/Impressions/Edit/Clickstring",				IDC_PREFADD_PATTERN2				},
	{"Advertising/Impressions/Listctrl/List",					IDC_PREFADD_LIST					},
	{"Advertising/Impressions/Button/Add",						IDC_PREFADD_ADD						},
	{"Advertising/Impressions/Button/Delete",					IDC_PREFADD_REM						},
	{"Advertising/Impressions/Button/Change",					IDC_PREFADD_CHANGE					},
	{"Advertising/Campaigns/Edit/Description",					IDC_ADDCAMP_NAME					},
	{"Advertising/Campaigns/Edit/Referal",						IDC_ADDCAMP_PATTERN					},
	{"Advertising/Campaigns/Edit/Costmonth",					IDC_ADDCAMP_COST					},
	{"Advertising/Campaigns/Listctrl/List",						IDC_ADDCAMP_LIST					},
	{"Advertising/Campaigns/Button/Add",						IDC_ADDCAMP_ADD						},
	{"Advertising/Campaigns/Button/Delete",						IDC_ADDCAMP_REM						},
	{"Advertising/Campaigns/Button/Change",						IDC_ADDCAMP_CHANGE					},
	{"Virtual/Domains/Combo/Identify",							IDC_VDREPORTBY						},
	{"Virtual/Domains/Edit/Directory",							IDC_VDMULTIDIR						},
	{"Virtual/Domains/Combo/Sorting",							IDC_VDSORTBY						},
	{"Virtual/Domains/Combo/Numberdomains",						IDC_PREFVD_SEQDIR					},
	{"Virtual/Aggregation/Edit/Host",							IDC_PREFVD_PATTERN					},
	{"Virtual/Aggregation/Edit/Name",							IDC_PREFVD_NAME						},
	{"Virtual/Aggregation/Listctrl/List",						IDC_PREFVD_LIST						},
	{"Virtual/Aggregation/Button/Add",							IDC_PREFVD_ADD						},
	{"Virtual/Aggregation/Button/Delete",						IDC_PREFVD_REM						},
	{"Virtual/Aggregation/Button/Change",						IDC_PREFVD_CHANGE					},
	{"Virtual/Mapping/Edit/Host",								IDC_EDITPATH_HOST					},
	{"Virtual/Mapping/Edit/Path",								IDC_EDITPATH_PATH					},
	{"Virtual/Mapping/Button/Browse",							IDC_EDITPATH_BROWSE					},
	{"Virtual/Mapping/Listctrl/List",							IDC_PREFS_EDITPATH					},
	{"Virtual/Mapping/Button/Add",								IDC_EDITPATH_ADD					},
	{"Virtual/Mapping/Button/Delete",							IDC_EDITPATH_DEL					},
	{"Virtual/Mapping/Button/Change",							IDC_EDITPATH_CHANGE					},
	{"Virtual/Clustering/Checkbox/Enabled",						IDC_PREFVD_CLUSTER					},
	{"Virtual/Clustering/Edit/Enabledfor",						IDC_PREFVD_CLUSTERINFO				},
	{"Virtual/Clustering/Edit/Clusters",						IDC_PREFVD_CLUSTERNUM				},
	{"Virtual/Clustering/Edit/Name",							IDC_CLUSTER_NAME					},
	{"Virtual/Clustering/Edit/Ip",								IDC_CLUSTER_IP						},
	{"Virtual/Clustering/Edit/Path",							IDC_CLUSTER_PATTERN					},
	{"Virtual/Clustering/Listctrl/List",						IDC_CLUSTER_LIST					},
	{"Virtual/Clustering/Button/Add",							IDC_CLUSTER_ADD						},
	{"Virtual/Clustering/Button/Delete",						IDC_CLUSTER_DEL						},
	{"Virtual/Clustering/Button/Change",						IDC_CLUSTER_CHANGE					},
	{"Postprocess/Compress/Checkbox/Compressgzip",				IDC_PP_COMPRESSLOG					},
	{"Postprocess/Compress/Checkbox/Deleteoriginallog",			IDC_PP_DELETELOG					},
	{"Postprocess/Compress/Checkbox/Archivezip",				IDC_PP_ZIPREPORT					},
	{"Postprocess/Compress/Checkbox/Deleteoriginalreport",		IDC_PP_DELETEREPORT					},
	{"Postprocess/Upload/Log/Checkbox/Enable",					IDC_PP_UPLOADLOG					},
	{"Postprocess/Upload/Log/Edit/Hostname",					IDC_PP_HOST							},
	{"Postprocess/Upload/Log/Edit/Username",					IDC_PP_USER							},
	{"Postprocess/Upload/Log/Edit/Password",					IDC_PP_PASSWD						},
	{"Postprocess/Upload/Log/Edit/Path",						IDC_PP_PATH							},
	{"Postprocess/Upload/Reports/Checkbox/Enable",				IDC_PP_UPLOADREPORT					},
	{"Postprocess/Upload/Reports/Edit/Hostname",				IDC_PP_HOST2						},
	{"Postprocess/Upload/Reports/Edit/Username",				IDC_PP_USER2						},
	{"Postprocess/Upload/Reports/Edit/Password",				IDC_PP_PASSWD2						},
	{"Postprocess/Upload/Reports/Edit/Path",					IDC_PP_PATH2						},
	{"Postprocess/Upload/Reports/Checkbox/UploadWhole",			IDC_PP_UPLOADFOLDER					},
	{"Postprocess/Email/Checkbox/Send",							IDC_PP_EMAILON						},
	{"Postprocess/Email/Button/Test",							IDC_EMAIL_TEST						},
	{"Postprocess/Email/Edit/To",								IDC_PP_EMAIL						},
	{"Postprocess/Email/Edit/From",								IDC_PP_EMAILFROM					},
	{"Postprocess/Email/Edit/Subject",							IDC_PP_EMAILSUB						},
	{"Postprocess/Email/Edit/Host",								IDC_PP_EMAILSMTP					},
	{"Postprocess/Email/Edit/Message",							IDC_PP_MSG							},
	{"Customformat/Logfile/Checkbox/Usecustom",					IDC_CUSTOM_USE						},
	{"Customformat/Logfile/Edit/Separator",						IDC_CUSTOM_SEP						},
	{"Customformat/Logfile/Combo/Datafield",					IDC_CUSTOM_DATA						},
	{"Customformat/Logfile/Slider/Index",						IDC_CUSTOM_POS						},
	{"Customformat/Logfile/Combo/Dateformat",					IDC_CUSTOM_DATE						},
	{"Customformat/Logfile/Combo/Timeformat",					IDC_CUSTOM_TIME						},

	{"Remotecontrol/Checkbox/Enable",							IDC_REMOTEON						},
	{"Remotecontrol/Edit/Password",								IDC_REMOTEPASSWD					},
	{"Remotecontrol/Edit/Tcpport",								IDC_REMOTEPORT						},
	{"Routerfirewall/Internalnetwork/Edit/Ip",					IDC_PREFROUTER_NAME					},
	{"Routerfirewall/Internalnetwork/List",						IDC_PREFROUTER_LIST					},
	{"Routerfirewall/Internalnetwork/Button/Add",				IDC_PREFROUTER_ADD					},
	{"Routerfirewall/Internalnetwork/Button/Change",			IDC_PREFROUTER_CHANGE				},
	{"Routerfirewall/Internalnetwork/Button/Remove",			IDC_PREFROUTER_REM					},
	{"Routerfirewall/Internalnetwork/Button/Clear",				IDC_PREFROUTER_CLR					},

	{"LanguageBuilder/Combo/Language",							IDC_LANGTOKEN_LANG					}, // List of languages currently available to edit.
	{"LanguageBuilder/Edit/Text",								IDC_LANGTOKEN_TEXT					}, // Text of the current item. (Press return to update text). 
	{"LanguageBuilder/Button/Update",							IDC_LANGTOKEN_UPDATE				}, // Updates the currently selected item in the list.
	{"LanguageBuilder/Listctrl/List",							IDC_LANGTOKEN_LIST					}, // Language strings, contains the default & currently selected language.
	{"LanguageBuilder/Edit/Findtext",							IDC_LANGTOKEN_FINDTEXT				}, // Search text to find  (Press F3 or return to activate a search).
	{"LanguageBuilder/Button/Next",								IDC_LANGTOKEN_FIND					}, // Press to activate a search for the text.
	{"LanguageBuilder/Button/Previous",							IDC_LANGTOKEN_FINDPREV				}, // Press to activate a reverse search for the text.
	{"LanguageBuilder/Button/Saveas",							IDC_LANGTOKEN_SAVEAS				}, // Save current language string to a file.
	{"LanguageBuilder/Button/Exit",								IDC_LANGTOKEN_EXIT					}, // Exits the Language Builder dialog.

	{"Main/Button/Settings",									IDC_PREFS,							}, 
	{"Main/Button/Schedule",									IDC_SCHED,							}, 
	{"Main/Button/Process",										IDC_PROCESS,						}, 
	{"Main/Button/View",										IDC_VIEW,							}, 
	{"Main/Button/Help",										IDC_APPHELP,						}, 
	{"Main/Button/About",										IDC_APPLOGO,						}, 
	{"Main/Button/Showhide",									IDC_SHOWLIST,						}, 
	{"Main/Listctrl/List",										IDC_LOGLIST,						}, 

#else if (DEF_MAC)
// ****************************************************************************
// Hopefully it will be a similar method on the Mac and we can just put the
// control ids here. 
// ****************************************************************************
    {"Schedule/Button/Cancel"				,					MAKEID(0,0,0)							},
    {"Schedule/Button/Save"					,					MAKEID(0,0,0)							},
    {"Schedule/Button/Transcript"			,					MAKEID(0,0,0)							},
    {"Schedule/Button/Properties"			,					MAKEID(0,0,0)							},
    {"Schedule/Button/Runnow"				,					MAKEID(0,0,0)							},
    {"Schedule/Button/Remove"				,					MAKEID(0,0,0)							},
    {"Schedule/Button/New"					,					MAKEID(0,0,0)							},
    {"Schedule/Checkbox/Disable"			,					MAKEID(0,0,0)							},
    {"Schedule/Listctrl/List"				,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Edit/Name"				,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Edit/Startdate"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Edit/Starttime"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Spin/Startdate"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Spin/Starttime"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Edit/Every"				,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Combo/Period"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Edit/Logs"				,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Button/AddDB"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Button/SelectDB"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Button/Add"				,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Button/Select"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Edit/Settings"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Button/Browse"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Button/Editsettings"		,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Edit/UploadURL"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Edit/Reportfile"			,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Button/View"				,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Checkbox/Enabled"		,					MAKEID(0,0,0)							},
    {"ScheduleEdit/Button/Ok"				,					MAKEID(0,0,0)							},

	{"Preprocess/Download/Checkbox/Enabledownload",				MAKEID(0,0,0)	},
	{"Preprocess/Download/Edit/Hostname",						MAKEID(0,0,0)	},
	{"Preprocess/Download/Edit/Username",						MAKEID(0,0,0)	},
	{"Preprocess/Download/Edit/Password",						MAKEID(0,0,0)	},
	{"Preprocess/Download/Edit/Path",							MAKEID(0,0,0)	},
	{"Preprocess/Options/Checkbox/Deletefile",					MAKEID(0,0,0)	},
	{"Preprocess/Options/Edit/Download",						MAKEID(0,0,0)	},
	{"Preprocess/Options/Button/Browse",						MAKEID(0,0,0)	},
	{"Report/Output/Edit/Location",								MAKEID(0,0,0)	},
	{"Report/Output/Edit/PathTokens",							MAKEID(0,0,0)	},
	{"Report/Output/Button/Saveas",								MAKEID(0,0,0)	},
	{"Report/Output/Button/Insert",								MAKEID(0,0,0)	},
	{"Report/Output/Database/Radio/None",						MAKEID(0,0,0)	},
	{"Report/Output/Database/Radio/Select",						MAKEID(0,0,0)	},
	{"Report/Output/Database/Edit/Name",						MAKEID(0,0,0)	},
	{"Report/Output/Database/Button/Delete",					MAKEID(0,0,0)	},
	{"Report/Output/Database/Button/Browse",					MAKEID(0,0,0)	},
	{"Report/Output/Database/Checkbox/NewLogsOnly",				MAKEID(0,0,0)	},
	{"Report/Output/Combo/Language",							MAKEID(0,0,0)	},
	{"Report/Output/Combo/Format",								MAKEID(0,0,0)	},
	{"Report/Output/Combo/Graphics",							MAKEID(0,0,0)	},
	{"Report/Page/Display/Checkbox/Bandwidth",					MAKEID(0,0,0)	},
	{"Report/Page/Display/Checkbox/Logname",					MAKEID(0,0,0)	},
	{"Report/Page/Display/Checkbox/Timestamp",					MAKEID(0,0,0)	},
	{"Report/Page/Display/Checkbox/Keyonleft",					MAKEID(0,0,0)	},
	{"Report/Page/Display/Checkbox/QuickLinks",					MAKEID(0,0,0)	},
	{"Report/Page/Display/Checkbox/Reportshadows",				MAKEID(0,0,0)	},
	{"Report/Page/Display/Checkbox/Footerlabel",				MAKEID(0,0,0)	},
	{"Report/Page/Display/Checkbox/Trailinglabel",				MAKEID(0,0,0)	},
	{"Report/Page/Display/Edit/Reporttitle",					MAKEID(0,0,0)	},
	{"Report/Page/Colors/Listbox/Theme",						MAKEID(0,0,0)	},
	{"Report/Page/Colors/Button/Title",							MAKEID(0,0,0)	},
	{"Report/Page/Colors/Button/Headers",						MAKEID(0,0,0)	},
	{"Report/Page/Colors/Button/Items",							MAKEID(0,0,0)	},
	{"Report/Page/Colors/Button/Others",						MAKEID(0,0,0)	},
	{"Report/Page/Colors/Button/Average",						MAKEID(0,0,0)	},
	{"Report/Page/Colors/Button/Totals",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Combo/Font",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Combo/Numbers",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Combo/Size",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Edit/Width",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Edit/Height",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Edit/Left",							MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Edit/Right",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Edit/Top",							MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Edit/Bottom",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Checkbox/Image",					MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Edit/Filename",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Page/Button/Browse",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Table/Checkbox/Wrapping",				MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Table/Button/Advanced",					MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Table/Combo/Truncation",					MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Table/Combo/Section",					MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Table/Combo/Size",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Table/Combo/Style",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Table/Combo/Color",						MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Spacing/Combo/Spacing",					MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Spacing/Edit/Pixels",					MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Graphtext/Combo/Name",					MAKEID(0,0,0)	},
	{"Report/Pdf/Style/Graphtext/Combo/Size",					MAKEID(0,0,0)	},
	{"Report/Style/Button/BgColor",								MAKEID(0,0,0)	},
	{"Report/Style/Button/Text",								MAKEID(0,0,0)	},
	{"Report/Style/Button/Link",								MAKEID(0,0,0)	},
	{"Report/Style/Button/ALink",								MAKEID(0,0,0)	},
	{"Report/Style/Button/VLink",								MAKEID(0,0,0)	},
	{"Report/Style/Edit/Header",								MAKEID(0,0,0)	},
	{"Report/Style/Edit/Footer",								MAKEID(0,0,0)	},
	{"Analysis/Options/Date/Combo/Option",						MAKEID(0,0,0)	},
	{"Analysis/Options/Date/Edit/Start",						MAKEID(0,0,0)	},
	{"Analysis/Options/Date/Edit/End",							MAKEID(0,0,0)	},
	{"Analysis/Options/Date/Combo/Reports",						MAKEID(0,0,0)	},
	{"Analysis/Options/Date/Combo/Format",						MAKEID(0,0,0)	},
	{"Analysis/Options/Date/Edit/Adjust",						MAKEID(0,0,0)	},
	{"Analysis/Options/Ignore/Checkbox/Zerobytes",				MAKEID(0,0,0)	},
	{"Analysis/Options/Ignore/Checkbox/Selfreferal",			MAKEID(0,0,0)	},
	{"Analysis/Options/Ignore/Checkbox/Urlcase",				MAKEID(0,0,0)	},
	{"Analysis/Options/Ignore/Checkbox/Clientusernames",		MAKEID(0,0,0)	},
	{"Analysis/Options/Ignore/Checkbox/Bookmarks",				MAKEID(0,0,0)	},
	{"Analysis/Options/Url/Defaulthost",						MAKEID(0,0,0)	},
	{"Analysis/Options/Url/Defaultpage",						MAKEID(0,0,0)	},
	{"Analysis/Network/Dns/Radio/Lookupsoff",					MAKEID(0,0,0)	},
	{"Analysis/Network/Dns/Radio/Lookupson",					MAKEID(0,0,0)	},
	{"Analysis/Network/Dns/Edit/Simultaneous",					MAKEID(0,0,0)	},
	{"Analysis/Network/Dns/Radio/Cache",						MAKEID(0,0,0)	},
	{"Analysis/Network/Dns/Button/Clearcache",					MAKEID(0,0,0)	},
	{"Analysis/Network/General/Passiveftp",						MAKEID(0,0,0)	},
	{"Analysis/Network/General/Retrievetitles",					MAKEID(0,0,0)	},
	{"Analysis/Extensions/Treeview/Extensions",					MAKEID(0,0,0)	},
	{"Analysis/Extensions/Button/Add",							MAKEID(0,0,0)	},
	{"Analysis/Extensions/Button/Delete",						MAKEID(0,0,0)	},
	{"Analysis/Extensions/Button/Edit",							MAKEID(0,0,0)	},
	{"Analysis/Extensions/Button/Defaults",						MAKEID(0,0,0)	},
	{"Analysis/Dynamic/Url/Checkbox/Retainall",					MAKEID(0,0,0)	},
	{"Analysis/Dynamic/Url/Checkbox/Retainvariables",			MAKEID(0,0,0)	},
	{"Analysis/Dynamic/Url/Edit/Variables",						MAKEID(0,0,0)	},
	{"Analysis/Dynamic/Analysis/Checkbox/Flushing",				MAKEID(0,0,0)	},
	{"Filters/Inputdata/Combo/Include",							MAKEID(0,0,0)	},
	{"Filters/Inputdata/Combo/Type",							MAKEID(0,0,0)	},
	{"Filters/Inputdata/Combo/Contains",						MAKEID(0,0,0)	},
	{"Filters/Inputdata/Edit/Data",								MAKEID(0,0,0)	},
	{"Filters/Inputdata/Listctrl/Filters",						MAKEID(0,0,0)	},
	{"Filters/Inputdata/Button/Add",							MAKEID(0,0,0)	},
	{"Filters/Inputdata/Button/Delete",							MAKEID(0,0,0)	},
	{"Filters/Inputdata/Button/Change",							MAKEID(0,0,0)	},
	{"Statistics/Statistics/Treeview/All",						MAKEID(0,0,0)	},
	{"Statistics/Statistics/Button/Expandall",					MAKEID(0,0,0)	},
	{"Statistics/Statistics/Button/Defaults",					MAKEID(0,0,0)	},
	{"Statistics/Statistics/Button/Applyall",					MAKEID(0,0,0)	},
	{"Statistics/Statistics/Checkbox/Table",					MAKEID(0,0,0)	},
	{"Statistics/Statistics/Combo/Rows",						MAKEID(0,0,0)	},
	{"Statistics/Statistics/Checkbox/Graph",					MAKEID(0,0,0)	},
	{"Statistics/Statistics/Combo/Sorting",						MAKEID(0,0,0)	},
	{"Statistics/Statistics/Checkbox/HelpCard",					MAKEID(0,0,0)	},
	{"Statistics/Statistics/Button/EditCard",					MAKEID(0,0,0)	},
	{"Statistics/Global/Graph/Checkbox/3d",						MAKEID(0,0,0)	},
	{"Statistics/Global/Graph/Checkbox/Wide",					MAKEID(0,0,0)	},
	{"Statistics/Global/Graph/Checkbox/Webpallet",				MAKEID(0,0,0)	},
	{"Statistics/Global/Graph/Checkbox/Paperprintable",			MAKEID(0,0,0)	},
	{"Statistics/Global/Sessions/Edit/Cost",					MAKEID(0,0,0)	},
	{"Statistics/Global/Sessions/Edit/Timeout",					MAKEID(0,0,0)	},
	{"Statistics/Global/Sessions/Edit/Cookie",					MAKEID(0,0,0)	},
	{"Statistics/Global/Sessions/Edit/Urlparameter",			MAKEID(0,0,0)	},
	{"Statistics/Organizational/Edit/Pattern",					MAKEID(0,0,0)	},
	{"Statistics/Organizational/Edit/Name",						MAKEID(0,0,0)	},
	{"Statistics/Organizational/Listctrl/List",					MAKEID(0,0,0)	},
	{"Statistics/Organizational/Button/Add",					MAKEID(0,0,0)	},
	{"Statistics/Organizational/Button/Delete",					MAKEID(0,0,0)	},
	{"Statistics/Organizational/Button/Change",					MAKEID(0,0,0)	},
	{"Statistics/Organizational/Button/Defaults",				MAKEID(0,0,0)	},
	{"Statistics/Contentgroups/Edit/Pattern",					MAKEID(0,0,0)	},
	{"Statistics/Contentgroups/Edit/Name",						MAKEID(0,0,0)	},
	{"Statistics/Contentgroups/Listctrl/List",					MAKEID(0,0,0)	},
	{"Statistics/Contentgroups/Button/Add",						MAKEID(0,0,0)	},
	{"Statistics/Contentgroups/Button/Delete",					MAKEID(0,0,0)	},
	{"Statistics/Contentgroups/Button/Change",					MAKEID(0,0,0)	},
	{"Statistics/Referalgroups/Edit/Pattern",					MAKEID(0,0,0)	},
	{"Statistics/Referalgroups/Edit/Name",						MAKEID(0,0,0)	},
	{"Statistics/Referalgroups/Listctrl/List",					MAKEID(0,0,0)	},
	{"Statistics/Referalgroups/Button/Add",						MAKEID(0,0,0)	},
	{"Statistics/Referalgroups/Button/Delete",					MAKEID(0,0,0)	},
	{"Statistics/Referalgroups/Button/Change",					MAKEID(0,0,0)	},
	{"Routes/Keyvisitors/Edit/Name",							MAKEID(0,0,0)	},
	{"Routes/Keyvisitors/Listctrl/List",						MAKEID(0,0,0)	},
	{"Routes/Keyvisitors/Button/Add",							MAKEID(0,0,0)	},
	{"Routes/Keyvisitors/Button/Delete",						MAKEID(0,0,0)	},
	{"Routes/Keyvisitors/Button/Change",						MAKEID(0,0,0)	},
	{"Routes/Keyvisitors/Button/Defaults",						MAKEID(0,0,0)	},
	{"Routes/Keypagesto/Edit/Page",								MAKEID(0,0,0)	},
	{"Routes/Keypagesto/Edit/Traceback",						MAKEID(0,0,0)	},
	{"Routes/Keypagesto/Edit/Number",							MAKEID(0,0,0)	},
	{"Routes/Keypagesto/Listctrl/List",							MAKEID(0,0,0)	},
	{"Routes/Keypagesto/Button/Add",							MAKEID(0,0,0)	},
	{"Routes/Keypagesto/Button/Delete",							MAKEID(0,0,0)	},
	{"Routes/Keypagesto/Button/Change",							MAKEID(0,0,0)	},
	{"Routes/Keypagesto/Button/Defaults",						MAKEID(0,0,0)	},
	{"Routes/Keypagesfrom/Edit/Page",							MAKEID(0,0,0)	},
	{"Routes/Keypagesfrom/Edit/Traceback",						MAKEID(0,0,0)	},
	{"Routes/Keypagesfrom/Edit/Number",							MAKEID(0,0,0)	},
	{"Routes/Keypagesfrom/Listctrl/List",						MAKEID(0,0,0)	},
	{"Routes/Keypagesfrom/Button/Add",							MAKEID(0,0,0)	},
	{"Routes/Keypagesfrom/Button/Delete",						MAKEID(0,0,0)	},
	{"Routes/Keypagesfrom/Button/Change",						MAKEID(0,0,0)	},
	{"Routes/Keypagesfrom/Button/Defaults",						MAKEID(0,0,0)	},
	{"Advertising/Impressions/Edit/Description",				MAKEID(0,0,0)	},
	{"Advertising/Impressions/Edit/Filename",					MAKEID(0,0,0)	},
	{"Advertising/Impressions/Edit/Clickstring",				MAKEID(0,0,0)	},
	{"Advertising/Impressions/Listctrl/List",					MAKEID(0,0,0)	},
	{"Advertising/Impressions/Button/Add",						MAKEID(0,0,0)	},
	{"Advertising/Impressions/Button/Delete",					MAKEID(0,0,0)	},
	{"Advertising/Impressions/Button/Change",					MAKEID(0,0,0)	},
	{"Advertising/Campaigns/Edit/Description",					MAKEID(0,0,0)	},
	{"Advertising/Campaigns/Edit/Referal",						MAKEID(0,0,0)	},
	{"Advertising/Campaigns/Edit/Costmonth",					MAKEID(0,0,0)	},
	{"Advertising/Campaigns/Listctrl/List",						MAKEID(0,0,0)	},
	{"Advertising/Campaigns/Button/Add",						MAKEID(0,0,0)	},
	{"Advertising/Campaigns/Button/Delete",						MAKEID(0,0,0)	},
	{"Advertising/Campaigns/Button/Change",						MAKEID(0,0,0)	},
	{"Virtual/Domains/Combo/Identify",							MAKEID(0,0,0)	},
	{"Virtual/Domains/Edit/Directory",							MAKEID(0,0,0)	},
	{"Virtual/Domains/Combo/Sorting",							MAKEID(0,0,0)	},
	{"Virtual/Domains/Combo/Numberdomains",						MAKEID(0,0,0)	},
	{"Virtual/Aggregation/Edit/Host",							MAKEID(0,0,0)	},
	{"Virtual/Aggregation/Edit/Name",							MAKEID(0,0,0)	},
	{"Virtual/Aggregation/Listctrl/List",						MAKEID(0,0,0)	},
	{"Virtual/Aggregation/Button/Add",							MAKEID(0,0,0)	},
	{"Virtual/Aggregation/Button/Delete",						MAKEID(0,0,0)	},
	{"Virtual/Aggregation/Button/Change",						MAKEID(0,0,0)	},
	{"Virtual/Mapping/Edit/Host",								MAKEID(0,0,0)	},
	{"Virtual/Mapping/Edit/Path",								MAKEID(0,0,0)	},
	{"Virtual/Mapping/Button/Browse",							MAKEID(0,0,0)	},
	{"Virtual/Mapping/Listctrl/List",							MAKEID(0,0,0)	},
	{"Virtual/Mapping/Button/Add",								MAKEID(0,0,0)	},
	{"Virtual/Mapping/Button/Delete",							MAKEID(0,0,0)	},
	{"Virtual/Mapping/Button/Change",							MAKEID(0,0,0)	},
	{"Virtual/Clustering/Checkbox/Enabled",						MAKEID(0,0,0)	},
	{"Virtual/Clustering/Edit/Enabledfor",						MAKEID(0,0,0)	},
	{"Virtual/Clustering/Edit/Clusters",						MAKEID(0,0,0)	},
	{"Virtual/Clustering/Edit/Name",							MAKEID(0,0,0)	},
	{"Virtual/Clustering/Edit/Ip",								MAKEID(0,0,0)	},
	{"Virtual/Clustering/Edit/Path",							MAKEID(0,0,0)	},
	{"Virtual/Clustering/Listctrl/List",						MAKEID(0,0,0)	},
	{"Virtual/Clustering/Button/Add",							MAKEID(0,0,0)	},
	{"Virtual/Clustering/Button/Delete",						MAKEID(0,0,0)	},
	{"Virtual/Clustering/Button/Change",						MAKEID(0,0,0)	},
	{"Postprocess/Compress/Checkbox/Compressgzip",				MAKEID(0,0,0)	},
	{"Postprocess/Compress/Checkbox/Deleteoriginallog",			MAKEID(0,0,0)	},
	{"Postprocess/Compress/Checkbox/Archivezip",				MAKEID(0,0,0)	},
	{"Postprocess/Compress/Checkbox/Deleteoriginalreport",		MAKEID(0,0,0)	},
	{"Postprocess/Upload/Log/Checkbox/Enable",					MAKEID(0,0,0)	},
	{"Postprocess/Upload/Log/Edit/Hostname",					MAKEID(0,0,0)	},
	{"Postprocess/Upload/Log/Edit/Username",					MAKEID(0,0,0)	},
	{"Postprocess/Upload/Log/Edit/Password",					MAKEID(0,0,0)	},
	{"Postprocess/Upload/Log/Edit/Path",						MAKEID(0,0,0)	},
	{"Postprocess/Upload/Reports/Checkbox/Enable",				MAKEID(0,0,0)	},
	{"Postprocess/Upload/Reports/Edit/Hostname",				MAKEID(0,0,0)	},
	{"Postprocess/Upload/Reports/Edit/Username",				MAKEID(0,0,0)	},
	{"Postprocess/Upload/Reports/Edit/Password",				MAKEID(0,0,0)	},
	{"Postprocess/Upload/Reports/Edit/Path",					MAKEID(0,0,0)	},
	{"Postprocess/Email/Checkbox/Send",							MAKEID(0,0,0)	},
	{"Postprocess/Email/Button/Test",							MAKEID(0,0,0)	},
	{"Postprocess/Email/Edit/To",								MAKEID(0,0,0)	},
	{"Postprocess/Email/Edit/From",								MAKEID(0,0,0)	},
	{"Postprocess/Email/Edit/Subject",							MAKEID(0,0,0)	},
	{"Postprocess/Email/Edit/Host",								MAKEID(0,0,0)	},
	{"Postprocess/Email/Edit/Message",							MAKEID(0,0,0)	},
	{"Customformat/Logfile/Checkbox/Usecustom",					MAKEID(0,0,0)	},
	{"Customformat/Logfile/Edit/Separator",						MAKEID(0,0,0)	},
	{"Customformat/Logfile/Combo/Datafield",					MAKEID(0,0,0)	},
	{"Customformat/Logfile/Slider/Index",						MAKEID(0,0,0)	},
	{"Customformat/Logfile/Combo/Dateformat",					MAKEID(0,0,0)	},
	{"Customformat/Logfile/Combo/Timeformat",					MAKEID(0,0,0)	},
	{"Remotecontrol/Checkbox/Enable",							MAKEID(0,0,0)	},
	{"Remotecontrol/Edit/Password",								MAKEID(0,0,0)	},
	{"Remotecontrol/Edit/Tcpport",								MAKEID(0,0,0)	},
	{"Routerfirewall/Internalnetwork/Edit/Ip",					MAKEID(0,0,0)	},
	{"Routerfirewall/Internalnetwork/List",						MAKEID(0,0,0)	},
	{"Routerfirewall/Internalnetwork/Button/Add",				MAKEID(0,0,0)	},
	{"Routerfirewall/Internalnetwork/Button/Change",			MAKEID(0,0,0)	},
	{"Routerfirewall/Internalnetwork/Button/Remove",			MAKEID(0,0,0)	},
	{"Routerfirewall/Internalnetwork/Button/Clear",				MAKEID(0,0,0)	},

	{"LanguageBuilder/Combo/Language",							MAKEID(0,0,0)	}, // List of languages currently available to edit.
	{"LanguageBuilder/Edit/Text",								MAKEID(0,0,0)	}, // Text of the current item. (Press return to update text). 
	{"LanguageBuilder/Button/Update",							MAKEID(0,0,0)	}, // Updates the currently selected item in the list.
	{"LanguageBuilder/Listctrl/List",							MAKEID(0,0,0)	}, // Language strings, contains the default & currently selected language.
	{"LanguageBuilder/Edit/Findtext",							MAKEID(0,0,0)	}, // Search text to find  (Press F3 or return to activate a search).
	{"LanguageBuilder/Button/Next",								MAKEID(0,0,0)	}, // Press to activate a search for the text.
	{"LanguageBuilder/Button/Previous",							MAKEID(0,0,0)	}, // Press to activate a reverse search for the text.
	{"LanguageBuilder/Button/Saveas",							MAKEID(0,0,0)	}, // Save current language string to a file.
	{"LanguageBuilder/Button/Exit",								MAKEID(0,0,0)	}, // Exits the Language Builder dialog.

	{"Main/Button/Settings",									MAKEID(0,0,0)	}, 
	{"Main/Button/Schedule",									MAKEID(0,0,0)	}, 
	{"Main/Button/Process",										MAKEID(0,0,0)	}, 
	{"Main/Button/View",										MAKEID(0,0,0)	}, 
	{"Main/Button/Help",										MAKEID(0,0,0)	}, 
	{"Main/Button/About",										MAKEID(0,0,0)	}, 
	{"Main/Button/Showhide",									MAKEID(0,0,0)	}, 
	{"Main/Listctrl/List",										MAKEID(0,0,0)	}, 
#endif	// if DEF_WINDOWS	
	{ 0, 0 }
};


// ****************************************************************************
// This map is to contain the translation from ControlId to HelpText.
// ****************************************************************************
typedef std::map<unsigned int, std::string>	MapHelpText;
static MapHelpText	g_mapHelpText;

// ****************************************************************************
// This function populates the above map with the HelpText from the XML file.
// ****************************************************************************
bool
HelpText_Load(void)
{
	// ****************************************************************************
	// Do not populate it more than once!
	// ****************************************************************************
#ifndef	DEF_DEBUG
	if (g_mapHelpText.size()>1)
		return false;
#endif
	g_mapHelpText.clear();

	CQXmlNode	xml;
	std::string strXml;

	// ****************************************************************************
	// Contruct the full filename of the HelpText xml file.
	// ****************************************************************************
	std::string	strFileName(gPath);
	strFileName += "\\HelpText.xml";

	// ****************************************************************************
	// Load the file content into a string.
	// (When the streaming operators are added to the QCXmlNode class we can change this)
	// ****************************************************************************
	FILE* fXmlFile = fopen(strFileName.c_str(), "r");
	if (!fXmlFile)
	{
#ifdef	DEF_DEBUG
		std::string str;
		str = "(Debug only message) You are missing the file : ";
		str += strFileName;
		g_mapHelpText[0] = str.c_str();
#else
		g_mapHelpText[0] = "";
#endif
		return false;
	}

	char sz[2];
	sz[1] = sz[0] = 0;
	while ((sz[0]=fgetc(fXmlFile)) != EOF)
		strXml += sz;

	(void)fclose(fXmlFile);

	// ****************************************************************************
	// Load up the xml into the empty CQXmlNode.
	// ****************************************************************************
	xml.ReadNode(strXml.c_str());
	CQXmlNode* pTag;

	// ****************************************************************************
	// As we 'know' the format of the XML lets just iterate through and
	// extract the translations and push them into the global map.
	// ****************************************************************************
	CQXmlNode::const_iterator it;
	for (it=xml.GetStartNode(); it!=xml.GetEndNode(); ++it)
	{
		pTag = (*it);
		CQXmlNode* pDescTag = pTag->LocateNode("Desc");
		CQXmlNode* pHelpTag = pTag->LocateNode("HelpText");

		if (!pDescTag)
			continue;
		if (!pHelpTag)
			continue;

		const char* szDesc = pDescTag->GetData().c_str();
		const char* szHelp = pHelpTag->GetData().c_str();

		int i;
		for (i=0; g_GadgetMap[i].szGadgetDesc && szDesc; ++i)
		{
			if (strcmp(g_GadgetMap[i].szGadgetDesc, szDesc) == 0)
				break;
		}

		if (g_GadgetMap[i].szGadgetDesc)
			g_mapHelpText[g_GadgetMap[i].uiControlId] = szHelp;
	}

	return true;
}

// ****************************************************************************
// This method is used by the window message callback to get the HelpText
// from the Control id - the above method HelpText_Load() has populate the
// map for us.
// ****************************************************************************
const char*	HelpText_Lookup(unsigned int uiControlId)
{
	MapHelpText::const_iterator it;

	it = g_mapHelpText.find(uiControlId);
	if (it == g_mapHelpText.end())
		return "";
	return it->second.c_str();
}

// ****************************************************************************
// In Debug mode it is useful to have the location-description strings displayed
// So this function is called to get the text to popuate the readonly edit.
// ****************************************************************************
const char*	HelpText_LookupDesc(unsigned int uiControlId)
{
	int i;
	for (i=0; g_GadgetMap[i].szGadgetDesc; ++i)
	{
		if (uiControlId == g_GadgetMap[i].uiControlId)
			break;
	}

	return g_GadgetMap[i].szGadgetDesc;
}


// ****************************************************************************
// Copyright (C) 2000-2004
// ****************************************************************************

