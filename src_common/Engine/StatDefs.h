
#ifndef	STATDEFS_H
#define	STATDEFS_H

#include "Compiler.h"

#define GRF_BORDER				20
#define	GRF_AXISINDENT			4

#define	MAKERGB(r,g,b)			((r<<16) | (g<<8) | (b))
#define RGB_RED(x) 				((x & 0xff0000)>>16)
#define	RGB_GREEN(x) 			((x & 0x00ff00)>>8)
#define RGB_BLUE(x)				( x & 0x0000ff)
#define	SWAP(a,b)				{char *temp; temp=a; a=b; b=temp; } 


#define	STATLIST_INDEXLAYERS		16
#define	STATLIST_SMALLINDEXSIZE		512
#define	STATLIST_BIGINDEXSIZE		(STATLIST_SMALLINDEXSIZE*16)
#define	STATLIST_HUGEINDEXSIZE		(STATLIST_BIGINDEXSIZE*16)
#define	STATLIST_MEGAINDEXSIZE		(STATLIST_HUGEINDEXSIZE*16)
#define	STATLIST_INDEXSIZE			STATLIST_SMALLINDEXSIZE

#define	NAMEIS_NORMAL		0
#define	NAMEIS_STATIC		1
#define	NAMEIS_COPY			2
#define	NAMEIS_IP			3
#define	NAMEIS_FILESTAT		4


// this is for IncrStatsDrill flags
#define	DRILLF_COUNTER			(1<<0)		//1,      1 COUNTER MUST BE FIRST!!!!
#define	DRILLF_INCVISITORS		(DRILLF_COUNTER)
#define	DRILLF_COUNTER4			(1<<1)		//1,      1 COUNTER MUST BE FIRST!!!!
#define	DRILLF_INCPAGES			(DRILLF_COUNTER4)
#define	DRILLF_SETFIRST			(1<<2)		//2,     10
#define	DRILLF_TIME				(1<<3)		//4,    100
#define	DRILLF_SESSIONSTAT		(1<<4)		//8,   1000
#define	DRILLF_INCVISITS		(1<<5)		//8,   1000
#define	DRILLF_SESSIONADDREF	(1<<6)		//16, 10000		// add a referral
#define	DRILLF_INCERR			(1<<7)		//32,100000
#define	DRILLF_STOREVALUE		(1<<8)		//64


#include "config.h"				// for extern of "MyPrefStruct"
#define	SILENT_TIME	MyPrefStruct.session_timewindow*60



/*
TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --
	IncrStats - increment the stats for given string
	and add time spacial data to record aswell (slower) but not
	too slow , fast enough :)
TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --TIMESTAT   --

  add the whole function below

The date field there is the unix time format time_t, (ie seconds from 1970) pageid is the hash of the page, but note:
also if the 'pageid ' field is between 1...10, then the pageid is another value, ie if its pageid =2, then day=bytes transfered for the whole session etc... it can be expanded to carry extra data between hits or at the end of any session.
*/
#define SESS_START_PAGE		1	// pageid = 1, days = time of the first page, indicates the start of a session
#define SESS_BYTES			2	// pageid = 2, days = bytes used, indicates the number of bytes recieved in a session, days = total bytes for all pages/urls in this session 
#define SESS_TIME			3	// pageid = 3, days = last date, indicates the time of the last page/url in the session
#define SESS_START_REFERAL	4	// pageid = 4, referal page that the start page came from, days = referalID
#define SESS_ERROR_CODE		5	// record the page error code if it has one other than 2xx,30x


#define SESS_STREAM_ABORT		10
#define SESS_STREAM_PAUSE		11
#define SESS_STREAM_RESUME		12
#define SESS_STREAM_SEEK		13
#define SESS_STREAM_STOP		14
#define SESS_STREAM_RECSTART	15
#define SESS_STREAM_RECSTOP		16

#define SESS_ABOVE_IS_PAGE	20		// pageid > 10   then the pageid is a valid pageid


#ifdef DEF_MAC
	#include "progress.h"
	#define	MACOS_PROGRESS_IDLE		CheckIdle();
#else
	#define	MACOS_PROGRESS_IDLE ;
#endif

// Do not change order as they are directly numerically accessed
enum {
	OS_UNKNOWN=0,
	OS_WIN31, OS_WIN95, OS_WIN98, OS_WINNT, OS_WIN2000,
	OS_MACOS, OS_MACOSX, OS_WEBTV,
	OS_SUNOS, OS_IRIX, OS_AIX, OS_HPUX, OS_OSF, OS_FBSD, OS_LINUX,
	OS_AMIGA, OS_VMS, OS_2, OS_UNIX, OS_BEOS, OS_WINXP, OS_WINME,
	OS_WINUNKNOWN
};



#endif