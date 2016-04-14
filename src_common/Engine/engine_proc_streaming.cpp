/*  ================================================================================================

	##########
	##
	##
	#########
			 ##
			 ##
			 ##
	##########

	Copyright © Software 2001, 2002

	File:            engine_proc_streaming.cpp
	Subsystem:       Log Decoder
	Created By:      Raul.Sobon, 01/05/02

	$Archive: /iReporter/Dev/src_common/Engine/engine_proc_streaming.cpp $
	$Revision: 14 $
	$Author: Raul.sobon $
	$Date: 2/08/02 3:14p $
  

    $Description:
	NOTE : This code is run on all platforms and MUST NOT have any OS specific includes or code
	unless wrapped in #defines, please keep OS specifics to a minimum

	General Workings:
	Because WM/QT and REAL log differently, ie real logs ONE file, and all events in the one line (probably better), and
	the other two log each event in each line, we have to commonalize it.
	We will keep one file access as one viewing, and record all events into the session history of the 'client click stream'
	system. So for REAL, we just keep the event string as one string, and have the main Process() code add all those to the CCC,
	but for WM/QT, we just add one event to the CCC, and on the next line, if its the same viewing, we add that event to the CCC.
	We also will in here in Proc_XXXX() funcs convert any unusual fields to the common standard, and ignore specific fields that
	we wont use at all.

    Stat variable usage:
	bytesIn will be split into a 2 part (packetssent/packetsrec) 32bits each, which should cover up to 1600 GIG (assuming 390bytes/packet)
	bytesIn = (packetsREC<<32) | (packetsSENT&0xffffffff)


	Event Recording:
	Only real has events for each line (ie each clip recorded), the other formats will have to detect the same client
	and add the one event (STOP/PAUSE/F<</F>>/PLAY)
	We will have an s_events[] list that will be populated with lots of events in real, and only ONE event in QT/WM.


Additions to HitDataRec
struct {
	char	*s_playercpu;
	char	*s_playeros;
	char	*s_playerosver;
	char	*s_playerGUID;
	char	*s_playerlang;
	char	*s_playerver;
	char	*s_audiocodec;
	char	*s_videocodec;
	char	*s_transport;

	long	s_starttime;
	long	s_cpu_util;

	long	s_filedur;		
	long	s_filesize;	
	long	s_buffercount;
	long	s_buffertime;	
	long	s_percOK;	
	long	s_packets_sent;	
	long	s_packets_rec;
	long	s_meanbps;
	long	s_wm_streamposition;
	long	s_wm_streamduration;

	long	*s_events;

}

*/
#include "FWA.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "myansi.h"
#include "datetime.h"

#include "config.h"				// for OutDebugs()
#include "config_struct.h"
#include "engine_proc.h"
#include "engine_proc_streaming.h"
#include "StatDefs.h"
#include "log_io.h"
#include "Utilities.h"


extern short	logDelim;
extern short	prevLogDelim;
extern short	logNumDelimInLine;





#define FORMATSIZE		64

class StreamFormatIndex
{
public:
	StreamFormatIndex()
	{
		Clear();
		//logIndex = &date;
	}
	void Clear() { MemClr( &hostname, FORMATSIZE*sizeof(void*) ); }

	long Date()
	{
		if( date >=0 && date<= 25 )
			return date;
		else
			return 0;
	}
// We are Supreme so we can access it all public
public:
	long hostname;
	long hostip;
	long date;
	long time;
	long status;
	long url;
	long agent;
	long bytes;
	long bytesIn;
	long referer;
	long user;
	long cshost;
	long method;
	long cookie;
	long query;
	long csip;
	long protocol;
	long port;
	long ms;
	long process_accounting;

	// Streaming Component Variables.
	long	s_playercpu;
	long	s_playeros;
	long	s_playerosver;
	long	s_playerGUID;
	long	s_playerlang;
	long	s_playerexe;
	long	s_playerver;
	long	s_audiocodec;
	long	s_videocodec;

	long	s_cpu_util;

	long	s_filedur;
	long	s_filesize;
	long	s_buffercount;
	long	s_buffertime;
	long	s_percOK;
	long	s_packets_sent;
	long	s_packets_rec;
	long	s_transport;
	long	s_meanbps;
	long	s_wm_rate;
	long	s_starttime;
	long	s_wm_streamposition;
	long	s_wm_streamduration;

	long filler[FORMATSIZE];
	//long *logIndex;
};

StreamFormatIndex formatIndex;



static	char	*format[FORMATSIZE*3];

typedef struct {
	char *name;
	long *value;
} Fields;

long FindFields( Fields *fields, long num )
{
	for (long x=1;x<num;x++)
	{
		long f=0;
		while( fields[f].name )
		{
			if (!mystrcmpi( fields[f].name, format[x] ) )
				*(fields[f].value) = x-1;
			f++;
		}
	}
	return num;
}




// log W3C format and return logIndex array
// also valid for SUN format tokens
void LOG_WMedia(char *buffer) 
{
	short i=1,j=0;
	
	formatIndex.Clear();

	format[i++]=buffer;

	while (*buffer)
	{
		if( *buffer <= ' ' ) {
			*buffer++ = 0;
			if( *buffer > ' ' )
    			format[i++] = buffer;
		} else
			buffer++;
	}

	Fields w3c_f[] = {
		// Server Info
		"s-ip",				&formatIndex.cshost,
		"cs-host",			&formatIndex.cshost,
		"s-dns",			&formatIndex.cshost,
		"s-computername",	&formatIndex.cshost,
		"date",				&formatIndex.date,
		"time",				&formatIndex.time,
		"c-status",			&formatIndex.status,
							
		"s-cpu-util",		&formatIndex.s_cpu_util,

		// Client Info
		"c-ip",				&formatIndex.hostip,
		"cs-ip",			&formatIndex.hostip,
		"c-dns",			&formatIndex.hostname,
		"cs-username",		&formatIndex.user,
							
		"sc-bytes",			&formatIndex.bytes,					// bytes the client recieved
		"cs(Referer)",		&formatIndex.referer,
		"cs(Cookie)",		&formatIndex.cookie,
		"cs(User-Agent)",	&formatIndex.agent,

		"c-cpu",			&formatIndex.s_playercpu,
		"c-os",				&formatIndex.s_playeros,
		"c-osversion",		&formatIndex.s_playerosver,
		"c-playerid",		&formatIndex.s_playerGUID,
		"c-playerlanguage",	&formatIndex.s_playerlang,
		"c-playerversion",	&formatIndex.s_playerver,
		"c-hostexe",		&formatIndex.s_playerexe,

		// Content/File info
		"cs-uri",			&formatIndex.url,					// filename being played
		"cs-uri-stem",		&formatIndex.url,
		"filelength",		&formatIndex.s_filedur,				// seconds in length
		"filesize",			&formatIndex.s_filesize,			// bytes
		"c-buffercount",	&formatIndex.s_buffercount,			// number of times 'buffered'
		"c-buffertime",		&formatIndex.s_buffertime,			// number of seconds
		"audiocodec",		&formatIndex.s_audiocodec,
		"videocodec",		&formatIndex.s_videocodec,
		"c-quality",		&formatIndex.s_percOK,				// percentage of packets recieved ok
		"c-buffercount",	&formatIndex.s_buffercount,
		"s-pkts-sent",		&formatIndex.s_packets_sent,			// packets sent by server
		"c-pkts-received",	&formatIndex.s_packets_rec,			// packets the client received
		"protocol",			&formatIndex.protocol,				// protocol type, MMS/RTSP/HTTP
		"transport",		&formatIndex.s_transport,			// tcp/udp  , not important at the moment
		"avgbandwidth",		&formatIndex.s_meanbps,
		"c-rate",			&formatIndex.s_wm_rate,				//
		"c-starttime",		&formatIndex.s_wm_streamposition,	// timestamp of the stream when the log entry was generated
		"x-duration",		&formatIndex.s_wm_streamduration,	// length of time the stream was played before log entry
		0,0
	};
	
	FindFields( w3c_f, i );

	//decrement counter due to #Field spec
	logDelim = i-1;
}



void LOG_QTSS( char *buffer )
{
	short i=1;
	
	format[i++]=buffer;
	while (*buffer) {
		if( *buffer == ' ' ) {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	if (*format[i-1]==0)
        logDelim=i-2;
    else
        logDelim=i-1;


	Fields qtss_f[] = {
		// Server Info
		"s-ip",				&formatIndex.cshost,
		"s-dns",			&formatIndex.cshost,
		"date",				&formatIndex.date,
		"time",				&formatIndex.time,
		"c-status",			&formatIndex.status,
							
		"s-cpu-util",		&formatIndex.s_cpu_util,

		// Client Info
		"c-ip",				&formatIndex.hostip,
		"c-dns",			&formatIndex.hostname,
		"cs-username",		&formatIndex.user,
							
		"sc-bytes",			&formatIndex.bytes,					// bytes the client recieved
		"cs(Referer)",		&formatIndex.referer,
		"cs(Cookie)",		&formatIndex.cookie,
		"cs(User-Agent)",	&formatIndex.agent,

		"c-cpu",			&formatIndex.s_playercpu,
		"c-os",				&formatIndex.s_playeros,
		"c-osversion",		&formatIndex.s_playerosver,
		"c-playerid",		&formatIndex.s_playerGUID,
		"c-playerlanguage",	&formatIndex.s_playerlang,
		"c-playerversion",	&formatIndex.s_playerver,

		// Content/File info
		"cs-uri",			&formatIndex.url,					// filename being played
		"c-uri-stem",		&formatIndex.url,
		"cs-uri-stem",		&formatIndex.url,
		"filelength",		&formatIndex.s_filedur,				// seconds in length
		"filesize",			&formatIndex.s_filesize,			// bytes
		"c-buffercount",	&formatIndex.s_buffercount,			// number of times 'buffered'
		"c-totalbuffertime",&formatIndex.s_buffertime,			// number of seconds
		"audiocodec",		&formatIndex.s_audiocodec,
		"videocodec",		&formatIndex.s_videocodec,
		"c-quality",		&formatIndex.s_percOK,				// percentage of packets recieved ok
		"c-buffercount",	&formatIndex.s_buffercount,
		"s-pkts-sent",		&formatIndex.s_packets_sent,			// packets sent by server
		"c-pkts-received",	&formatIndex.s_packets_rec,			// packets the client received
		"protocol",			&formatIndex.protocol,				// protocol type, MMS/RTSP/HTTP
		"transport",		&formatIndex.s_transport,			// tcp/udp  , not important at the moment
		"avgbandwidth",		&formatIndex.s_meanbps,
		"c-starttime",		&formatIndex.s_starttime,			// the real start time of when the stream was played
		"x-duration",		&formatIndex.s_wm_streamduration,	// length of time the stream was played before log entry
		0,0
	};
	
	FindFields( qtss_f, i );
}























// ----------------------------------------------------------------------------------------------------------------------------------------------






long AdjustLogDatebyDuration( HitDataPtr Line, long duration )
{
	// Adjust fix the time to be the real start time, not log time
	if( duration )
	{
		struct tm		tm_date;
		// String to CTIME
		StringToDaysDate( Line->date, &tm_date, 0);
		StringToDaysTime( Line->time, &tm_date);
		Date2Days( &tm_date, (time_t*)&Line->ctime );

		// ctime adjust and back to struct
		Line->ctime -= Line->s_wm_streamduration;
		DaysDateToStruct( Line->ctime, &tm_date );

		// struct to string
		StructToUSDate( &tm_date, Line->newdate );
		CTimetoString( Line->ctime, Line->newdate+16 );
		Line->date = Line->newdate;
		Line->time = Line->newdate+16;
	}
	return Line->ctime;
}








/*
#Software: Windows Media Services
#Version: 4.1
#Date: 2000-04-07 03:09:48
#Fields: c-ip date time c-dns cs-uri-stem c-starttime x-duration c-rate c-status c-playerid c-playerversion c-playerlanguage cs(User-Agent) cs(Referer) c-hostexe c-hostexever c-os c-osversion c-cpu filelength filesize avgbandwidth protocol transport audiocodec videocodec channelURL sc-bytes c-bytes s-pkts-sent c-pkts-received c-pkts-lost-client c-pkts-lost-net c-pkts-lost-cont-net c-resendreqs c-pkts-recovered-ECC c-pkts-recovered-resent c-buffercount c-totalbuffertime c-quality s-ip s-dns s-totalclients s-cpu-util
203.111.20.141 2000-03-19 23:29:52 - mms://203.111.20.141/2.asf - - 1 404 - - - - - Windows_Media_Server - - - - - - - mms TCP - - - 0 0 0 0 - 0 0 - - - - - - 203.111.20.141 ice 1 7 
203.111.20.141 2000-03-19 23:29:56 - mms://203.111.20.141/2.asf - - 1 404 - - - - - Windows_Media_Server - - - - - - - mms TCP - - - 0 0 0 0 - 0 0 - - - - - - 203.111.20.141 ice 1 6 
203.111.20.141 2000-03-19 23:30:26 - mms://203.111.20.141/2.asf - - 1 404 - - - - - - - - - - - - - mms UDP - - - 0 0 0 0 - 0 0 - - - - - - 203.111.20.141 ice 1 6 
203.111.20.141 2000-03-19 23:30:39 - mms://203.111.20.141/2.asf - - 1 404 - - - - - - - - - - - - - mms UDP - - - 0 0 0 0 - 0 0 - - - - - - 203.111.20.141 ice 1 3 
203.111.20.141 2000-03-19 23:30:43 - mms://203.111.20.141/2.asf - - 1 404 - - - - - Windows_Media_Server - - - - - - - mms TCP - - - 0 0 0 0 - 0 0 - - - - - - 203.111.20.141 ice 1 7 
203.111.20.141 2000-03-19 23:42:34 ice mms://ICE/Welcome2.asf 0 65 1 200 {287978e9-a059-4c08-bfc9-da2dddf38a22} 6.4.9.1109 en-US Mozilla/4.0_(compatible;_MSIE_5.01;_Windows_NT_5.0) file:///C:/WIN2K/system32/Windows%20Media/Server/Sample1.htm iexplore.exe 5.0.2920.0 Windows_2000 5.0.0.2195 Pentium 67 2139795 79630 mms UDP Windows_Media_Audio_V2 Microsoft_MPEG-4_Video_Codec_V3 - 705634 705634 477 477 0 0 0 0 0 0 1 5 100 203.111.20.141 ice 1 4 
203.111.20.141 2000-03-19 23:43:07 ice mms://ICE/Welcome1.asf 0 17 1 200 {287978e9-a059-4c08-bfc9-da2dddf38a22} 6.4.9.1109 en-US Mozilla/4.0_(compatible;_MSIE_5.01;_Windows_NT_5.0) file:///C:/WIN2K/system32/Windows%20Media/Server/Sample2.htm iexplore.exe 5.0.2920.0 Windows_2000 5.0.0.2195 Pentium 17 360918 32202 mms UDP Windows_Media_Audio_V2 Microsoft_MPEG-4_Video_Codec_V3 - 91951 91951 53 53 0 0 0 0 0 0 1 6 100 203.111.20.141 ice 2 8 
10.20.5.140	04/29/2002	4:42:33	bigred	mms://localhost/welcome2.asf	30	5	1	200	{bb8a2784-7aa9-40e6-8a8c-aab557817d34}	6.4.9.1109	en-US	-	-	mplayer2.exe	6.4.9.1109	Windows_2000	5.0.0.2195	Pentium	67	2139795	15117	mms	UDP	Windows_Media_Audio_V2	Microsoft_MPEG-4_Video_Codec_V3	-	61222	61222	41	41	0	0	0	0	0	0	0	0	100	127.0.0.1	bigred	1	14
*/


extern char GlobalDate[32];
extern char GlobalTime[16];

static char s_agent[128];


short PROC_WMedia(char *buffer,HitDataPtr Line )
{
	register long j,inquote=0,i=1;
	register char *p;
static long	s_events[8];
	long s_rate;
	bool foundDelim = FALSE;

	// Ignore all w3c lines with IIS4 process accounting lines.
	if( formatIndex.process_accounting )
		return LOGLINE_COMMENT;

	// Current line has a space at the front, WHOOPS, its a damn bad ass currupted line, skip it or there will be trouble.
	if( *buffer == ' ' )
		return -1;

	//Tokenise line into format fields 1 to 25 from ','
	format[i++]=buffer;
	while ( *buffer && i<FORMATSIZE )
	{
		if (*buffer=='\"' && inquote)
		{
			*buffer++ = 0;
			inquote = 0;
			continue;
		}

		if( (*buffer == ' ' || *buffer == '\t') && !inquote )
			foundDelim = TRUE;

		if( foundDelim )
		{
			*buffer=0;
			if (*(buffer+1)=='\"')
			{
				inquote=1;
				buffer++;
			}
			buffer++;
			if( *buffer )	// only use the field if its valid, if its and the end, then dont use it.
				format[i++] = buffer;
			foundDelim = FALSE;
		}
		buffer++;
	}


	j = i;
	if( i >=  63 )
		return 0;

	// This is a special case if the USERNAME has spaces in it, it screws up the format pointers
	// so we must fix them by moving them until it all fits nicely.
	logNumDelimInLine = static_cast<short>(i);

	//the less checking here the better
	if (formatIndex.Date())
		Line->date = ConvEDate( format[formatIndex.Date()], Line->newdate );
	else
		Line->date = ConvEDate( GlobalDate, Line->newdate );
	if (!Line->date || *Line->date==0 )
		return 0;
	Line->time = format[formatIndex.time];

	Line->stat = format[formatIndex.status];
	Line->clientaddr = format[formatIndex.hostname];
	if( Line->clientaddr[0] == '-' )
		Line->clientaddr = format[formatIndex.hostip];

	Line->file = format[formatIndex.url];
	if( Line->file && Line->file[0] == '-' )
		Line->file = 0;

	Line->method = format[formatIndex.method];

	if( Line->agent = format[formatIndex.agent] )
		if( *Line->agent == '-' ) Line->agent = 0;

	Line->bytes = myatoi(format[formatIndex.bytes]);

	// --------- CHECK ASSEM VERSION OF CODE HERE
	if( p = format[formatIndex.referer] )
		if( *p == '-' ) Line->refer = 0; else Line->refer = p;
	if( Line->refer = format[formatIndex.referer] )
		if( *Line->refer == '-' ) Line->refer = 0;

	if( Line->user = format[formatIndex.user] )
		if( *Line->user == '-' ) Line->user = 0;

	if( Line->vhost = format[formatIndex.cshost] )
		if( *Line->vhost == '-' ) Line->vhost = 0;

	if( Line->cookie = format[formatIndex.cookie] )
		if( *Line->cookie == '-' ) Line->cookie = 0;

	// Wmedia Specific details
	Line->s_wm_streamposition = myatoi( format[formatIndex.s_wm_streamposition] );
	Line->s_wm_streamduration = myatoi( format[formatIndex.s_wm_streamduration] );
	s_rate = myatoi( format[formatIndex.s_wm_streamduration] );
	switch( s_rate )
	{
		case 1:		s_events[0] = SESS_STREAM_PAUSE;
		case -5:	s_events[0] = SESS_STREAM_SEEK;
		case 5:		s_events[0] = SESS_STREAM_SEEK;
		default:	s_events[0] = 0;
	}
	s_events[1] = 0;
	s_events[2] = 0;
	Line->s_events = s_events;

	if( Line->protocol = format[formatIndex.protocol] )
		if( *Line->protocol == '-' ) Line->protocol = 0;

	Line->s_cpu_util = myatoi( format[formatIndex.s_cpu_util] );
	Line->s_packets_sent = myatoi( format[formatIndex.s_packets_sent] );
	Line->s_percOK = myatoi( format[formatIndex.s_percOK] );
	Line->s_playercpu =	format[formatIndex.s_playercpu];
	if( Line->s_playercpu && Line->s_playercpu[0] == '-' )
		Line->s_playercpu = 0;
	Line->s_playeros =	format[formatIndex.s_playeros];
	Line->s_playerosver = format[formatIndex.s_playerosver];
	Line->s_playerver = format[formatIndex.s_playerver];
	Line->s_playerlang = format[formatIndex.s_playerlang];
	if( Line->s_playerlang && Line->s_playerlang[0] == '-' )
		Line->s_playerlang = 0;

	if( Line->agent = format[formatIndex.agent] )
	{
		// If the agent is blank, then copy the playername and version to the agent pointer
		if( *Line->agent == '-' ) {
			if( format[formatIndex.s_playerexe] ){
				sprintf( s_agent, "%s %s", format[formatIndex.s_playerexe], Line->s_playerver );
				Line->agent = s_agent;
			} else
				Line->agent = 0;
		} else
		{
			CleanStringOfBadChars( Line->agent );
		}
	}
	
	Line->s_playerGUID = format[formatIndex.s_playerGUID];

	Line->s_filedur = myatoi( format[formatIndex.s_filedur] );
	Line->s_filesize = myatoi( format[formatIndex.s_filesize] );
	Line->s_buffercount = myatoi( format[formatIndex.s_buffercount] );
	Line->s_buffertime = myatoi( format[formatIndex.s_buffertime] );
	Line->s_packets_sent = myatoi( format[formatIndex.s_packets_sent] );
	Line->s_packets_rec = myatoi( format[formatIndex.s_packets_rec] );
	Line->s_meanbps = myatoi( format[formatIndex.s_meanbps] );

	Line->s_audiocodec = format[formatIndex.s_audiocodec];
	if( Line->s_audiocodec && Line->s_audiocodec[0] == '-' )
		Line->s_audiocodec = 0;

	Line->s_videocodec = format[formatIndex.s_videocodec];
	if( Line->s_videocodec && Line->s_videocodec[0] == '-' )
		Line->s_videocodec = 0;

	Line->s_playerGUID = format[formatIndex.s_playerGUID];

	return 1;
}



/*
#Log File Created On: 05/28/2002 00:00:00
#Software: QTSS
#Version: 4.0 [v410]
#Date: 2002-05-28 00:00:00
#Remark: all time values are in GMT.
#Fields: c-ip date time c-dns cs-uri-stem c-starttime x-duration c-rate c-status c-playerid c-playerversion c-playerlanguage cs(User-Agent) cs(Referer) c-hostexe c-hostexever c-os c-osversion c-cpu filelength filesize avgbandwidth protocol transport audiocodec videocodec channelURL sc-bytes c-bytes s-pkts-sent c-pkts-received c-pkts-lost-client c-pkts-lost-net c-pkts-lost-cont-net c-resendreqs c-pkts-recovered-ECC c-pkts-recovered-resent c-buffercount c-totalbuffertime c-quality s-ip s-dns s-totalclients s-cpu-util cs-uri-query c-username sc(Realm) 
#Remark: Streaming beginning SHUTDOWN 2002-05-28 11:25:24
#Remark: Streaming beginning STARTUP 2002-05-28 11:25:27
130.243.8.185 2000-03-16 16:55:07 - /projekt/reklamforbundet/562S-744_90k.h.mov 953254387 120 1 200 130.243.8.185 - - QTS/1.0 - - - - - - 45 504518 88744 RTP UDP X-QDM/22050/2 X-SorensonVideo/90000 - 504518 0 532 0 0 0 0 0 0 0 1 6 100 195.42.197.89 qt 3 1 - - QT%20Streaming%20Server
195.54.104.4 2000-03-16 16:55:12 - /projekt/reklamforbundet/583S-785_90k.h.mov 953254430 82 1 200 195.54.104.4 4.1 - QTS%20(qtver=4.1;os=Windows%20NT%204.0Service%20Pack%205) - - - Windows%20NT%204.0Service%20Pac - - 40 447730 88480 RTP UDP X-QDM/22050/2 X-SorensonVideo/90000 - 567451 0 596 0 0 0 0 0 0 0 1 6 100 195.42.197.89 qt 3 1 - - QT%20Streaming%20Server
193.10.174.234 2000-03-16 16:55:17 - /projekt/reklamforbundet/604_90k.h.mov 953254444 73 1 200 193.10.174.234 - - QTS/1.0.2 - - - - - - 62 704504 90200 RTP UDP X-QDM/22050/2 X-SorensonVideo/90000 - 704504 0 734 733 0 0 0 0 0 0 1 6 100 195.42.197.89 qt 3 1 - - QT%20Streaming%20Server
193.10.174.234 2000-03-16 16:55:33 - /projekt/reklamforbundet/604_90k.h.mov 953254521 12 1 200 193.10.174.234 - - QTS/1.0.2 - - - - - - 62 704504 90200 RTP UDP X-QDM/22050/2 X-SorensonVideo/90000 - 138100 0 142 87 0 0 0 0 0 0 1 6 100 195.42.197.89 qt 3 1 - - QT%20Streaming%20Server
193.10.174.234 2000-03-16 16:55:45 - /projekt/reklamforbundet/633_90k.h.mov 953254537 8 1 200 193.10.174.234 - - QTS/1.0.2 - - - - - - 40 442852 87600 RTP UDP X-QDM/22050/2 X-SorensonVideo/90000 - 78330 0 78 28 0 0 0 0 0 0 1 6 100 195.42.197.89 qt 4 1 - - QT%20Streaming%20Server
	
10.20.6.20 2002-06-05 05:29:28 - /Misc%20Videos/Medusa.mov 1023254967 0 1 415 10.20.6.20 6.0b18 - QTS%20(qtver=6.0b18;os=Windows%20NT%205.1) - - - Windows%20NT%205.1 - - 0 0 0 RTP - - - - 0 0 0 0 0 0 0 0 0 0 1 0 0 10.20.6.5 icebox.oz.redflagsoftware.com 1 0 - - Streaming%20Server 
10.20.6.20 2002-06-05 05:29:42 - /Trailers/ali_g_med.mov 1023254981 0 1 415 10.20.6.20 6.0b18 - QTS%20(qtver=6.0b18;os=Windows%20NT%205.1) - - - Windows%20NT%205.1 - - 0 0 0 RTP - - - - 0 0 0 0 0 0 0 0 0 0 1 0 0 10.20.6.5 icebox.oz.redflagsoftware.com 1 0 - - Streaming%20Server 
10.20.6.20 2002-06-05 05:29:59 - /sample_100kbit.mov 1023254998 0 1 404 10.20.6.20 6.0b18 - QTS%20(qtver=6.0b18;os=Windows%20NT%205.1) - - - Windows%20NT%205.1 - - 0 0 0 RTP - - - - 0 0 0 0 0 0 0 0 0 0 1 0 0 10.20.6.5 icebox.oz.redflagsoftware.com 1 0 - - Streaming%20Server 
10.20.6.20 2002-06-05 05:32:34 - /sample.mov 1023255010 143 1 200 10.20.6.20 6.0b18 - QTS%20(qtver=6.0b18;os=Windows%20NT%205.1) - - - Windows%20NT%205.1 - - 37 410242 87744 RTP UDP X-QT/11025/1 X-SorensonVideo/90000 - 410242 0 1050 1050 0 0 0 0 0 0 1 3 100 10.20.6.5 icebox.oz.redflagsoftware.com 1 0 - - Streaming%20Server 
10.20.6.20 2002-06-05 05:33:49 - /sample.mov 1023255154 74 1 200 10.20.6.20 6.0b18 - QTS%20(qtver=6.0b18;os=Windows%20NT%205.1) - - - Windows%20NT%205.1 - - 37 410242 87744 RTP UDP X-QT/11025/1 X-SorensonVideo/90000 - 163437 0 386 303 0 0 0 0 0 0 1 3 100 10.20.6.5 icebox.oz.redflagsoftware.com 1 0 - - Streaming%20Server 
127.0.0.1 2002-06-05 05:39:55 - /Trailers/ali_g_med.mov 1023255594 0 1 415 127.0.0.1 5.0.2 - QTS%20(qtver=5.0.2;cpu=PPC;os=Mac%2010.1.4) - - - Mac%2010.1.4 - PPC 0 0 0 RTP - - - - 0 0 0 0 0 0 0 0 0 0 1 0 0 127.0.0.1 localhost 1 0 - - Streaming%20Server 
10.20.6.20 2002-06-05 06:01:09 - /Trailers/ali_g_med.mov 1023256868 0 1 415 10.20.6.20 - - - - - - - - - 0 0 0 RTP - - - - 0 0 0 0 0 0 0 0 0 0 1 0 0 10.20.6.5 icebox.oz.redflagsoftware.com 1 0 - - Streaming%20Server 

  


Quicktime logs one line per stream, during one streaming all seeks/pauses/FF/RW are not logged, but the totals hopefully are.




*/


short PROC_QTSS(char *buffer,HitDataPtr Line )
{
	register short	nudge=1,i=1;
	register char	delim;
	
	format[i++]=buffer;
	delim = ' ';
	while (*buffer && i<FORMATSIZE ) {
		if( *buffer == delim) {
			if (*(buffer+1)=='(') { //for QTS 1.02 with bugs in Agent field
				buffer++;
				delim=')';
				nudge=2;
				continue;
			} else {
				*buffer=0;
				format[i++] = buffer+nudge;
				buffer+=nudge;
				delim=' ';
				nudge=1;
			}
		} else
			buffer++;
	}
	//check for minimum tokens for the format (10)
	logNumDelimInLine = i;
    if (i!=logDelim && i!=logDelim+1) {
        return 0;
    }

/*	for (j=0;j<i;j++) {
		if (logIndex[j]>formatIndex.agent)
			logIndex[j]+=2;
	}
*/
	Line->s_wm_streamduration = myatoi( format[formatIndex.s_wm_streamduration] );

	// Get the correct start time of when file was accessed.
	if( formatIndex.s_starttime ){
		Line->ctime = myatoi( format[formatIndex.s_starttime] );
		// Convert Ctime to Date String
		if( Line->ctime )
		{
			struct tm		tm_date;
			DaysDateToStruct( Line->ctime, &tm_date );
			StructToUSDate( &tm_date, Line->newdate );
			CTimetoString( Line->ctime, Line->newdate+16 );
			Line->date = Line->newdate;
			Line->time = Line->newdate+16;
		}
	} else {
		// Well no access time recorded, so lets work it out via LOG TIME - DURATION
		Line->date = ConvEDate( format[formatIndex.Date()], Line->newdate );
		Line->time = format[formatIndex.time];
		AdjustLogDatebyDuration( Line, Line->s_wm_streamduration );
	}
	if (!Line->date || *Line->date==0 )
		return 0;

	
	
	Line->stat = format[formatIndex.status];

	Line->clientaddr = format[formatIndex.hostname];
	if( Line->clientaddr[0] == '-' )
		Line->clientaddr = format[formatIndex.hostip];

	Line->file = format[formatIndex.url];
	Line->method = format[formatIndex.method];

	Line->bytes = myatoi(format[formatIndex.bytes]);
	if( Line->refer = format[formatIndex.referer] )
		if( *Line->refer == '-' ) Line->refer = 0;

	if( Line->vhost = format[formatIndex.cshost] )
		if( *Line->vhost == '-' ) Line->vhost = 0;

	if( Line->protocol = format[formatIndex.protocol] )
		if( *Line->protocol == '-' ) Line->protocol = 0;

	Line->s_events = NULL;


	// Quicktime Specific details.
	Line->s_cpu_util = myatoi( format[formatIndex.s_cpu_util] );
	Line->s_packets_sent = myatoi( format[formatIndex.s_packets_sent] );

	// Quality Level
	Line->s_percOK = myatoi( format[formatIndex.s_percOK] );

	Line->s_playercpu =	format[formatIndex.s_playercpu];
	if( Line->s_playercpu && Line->s_playercpu[0] == '-' )
		Line->s_playercpu = 0;
	Line->s_playeros =	format[formatIndex.s_playeros];
	Line->s_playerosver = format[formatIndex.s_playerosver];
	Line->s_playerver = format[formatIndex.s_playerver];
	Line->s_playerlang = format[formatIndex.s_playerlang];
	if( Line->s_playerlang && Line->s_playerlang[0] == '-' )
		Line->s_playerlang = 0;

	if( Line->agent = format[formatIndex.agent] )
	{
		if( *Line->agent == '-' ) {
			Line->agent = 0;
		} else
		{
			CleanStringOfBadChars( Line->agent );
		}
	}

	Line->s_playerGUID = format[formatIndex.s_playerGUID];

	Line->s_filedur = myatoi( format[formatIndex.s_filedur] );
	Line->s_filesize = myatoi( format[formatIndex.s_filesize] );
	Line->s_buffercount = myatoi( format[formatIndex.s_buffercount] );
	Line->s_buffertime = myatoi( format[formatIndex.s_buffertime] );
	Line->s_packets_sent = myatoi( format[formatIndex.s_packets_sent] );
	Line->s_packets_rec = myatoi( format[formatIndex.s_packets_rec] );
	Line->s_meanbps = myatoi( format[formatIndex.s_meanbps] );
	Line->s_audiocodec = format[formatIndex.s_audiocodec];
	if( Line->s_audiocodec && Line->s_audiocodec[0] == '-' )
		Line->s_audiocodec = 0;
	Line->s_videocodec = format[formatIndex.s_videocodec];
	if( Line->s_videocodec && Line->s_videocodec[0] == '-' )
		Line->s_videocodec = 0;
	
	return 1;
}






static long PopulateArgsFromLine( char *line, char **argv, int &argc, char seperator )
{
	char *p;

	argc = 0;
	p = line;
	// --- construct the argc/argv variables, but take quotes into account
	// Fixes CR-4088, and probably good for many more in future.
	argv[ argc++ ] = p;
	while( *p && p && argc<255 )
	{
		if( *p == seperator )
		{
			while( *p==seperator )
				*p++ = 0;

			argv[ argc ] = p;

			if( *p == 34 ) // find QUOTE "
			{
				p++;
				argv[ argc ] = p;
				if( p = strchr( p, 34 ) )
					*p = 0;
			}
			argc++;
		}
		p++;
	}
	return argc;
}

//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//----------- REAL MEDIA SERVER LOG FORMAT -----------

/*
Original Real Server Log Version 1.
64.229.75.233 - - [10/Nov/2000:18:21:31 -0500]  "GET deadend/dedit/ABAA.rm RTSP/1.0" 200 2952852 [Win98_4.10_6.0.9.380_play32_SF80_en-US_586] [00000000-0000-0000-0000-000000000000] [UNKNOWN] 14922662 89 41 2 0 219
64.229.75.233 - - [10/Nov/2000:18:21:52 -0500]  "GET ramgen/deadend/dedit/B2.rm HTTP/1.0" 200 299 [UNKNOWN] [UNKNOWN] [UNKNOWN] 102 0 0 0 0 220
64.229.75.233 - - [10/Nov/2000:18:22:17 -0500]  "GET deadend/dedit/B2.rm RTSP/1.0" 200 1523756 [Win98_4.10_6.0.9.380_play32_SF80_en-US_586] [00000000-0000-0000-0000-000000000000] [UNKNOWN] 4174162 26 24 2 0 221
142.166.204.215 - - [10/Nov/2000:19:02:07 -0500]  "GET ramgen/deadend/journals/week1-pre/harold_leaving.rm HTTP/1.0" 200 349 [UNKNOWN] [UNKNOWN] [UNKNOWN] 152 0 0 0 0 222
207.148.140.222 - - [10/Nov/2000:19:45:51 -0500]  "GET zuka/10radio/10intro.rm RTSP/1.0" 200 55800 [Win98_4.90_6.0.6.33_plus32_SP60_en-US_586] [39e77040-6810-11d2-8caa-444553540000] [Stat1:        62          0          0          0          0 64_Kbps_Music][Stat2:     64695      27662          0          0          0          0          0          0          0          0        105 64_Kbps_Music] 497122 18 14 0 0 223
24.201.253.121 - - [10/Nov/2000:23:17:32 -0500]  "GET UNKNOWN PNAT/10" 200 0 [Win98_4.10_6.0.8.122_play32_MF61_en-US_586] [00000000-0000-0000-0000-000000000000] [UNKNOWN] 0 0 0 0 0 224


24.234.77.237 - - [17/Aug/2000:02:22:25 -0700]  "GET stage/stream_100.rm RTSP/1.0" 200 39423 [MacOT_57.48.52_6.0.8.125_play32_SF60_en-US_PPC_No-FPU] [2485c81e-5451-4f6b-8d53-21b35b486b44] [UNKNOWN] 2064802 167 6 0 0 48
24.234.77.237 - - [17/Aug/2000:02:22:45 -0700]  "GET stage/stream_100.rm RTSPT/1.0" 200 210000 [MacOT_57.48.52_6.0.8.125_play32_SF60_en-US_PPC_No-FPU] [2485c81e-5451-4f6b-8d53-21b35b486b44] [Stat1:        54          0          0          0          0 20_Kbps_Music][Stat2:     20689      19268          0          0          0          0          0          0          0          1        182 20_Kbps_Music] 2064802 167 17 0 0 49
212.150.115.131 - - [17/Aug/2000:02:37:59 -0700]  "GET exorbis/smil/cooking_essentials.smi RTSP/1.0" 200 371 [Win98_4.10_6.0.9.357_play32_MF80_en-US_586] [af8f71ea-582e-4d2a-a7ab-e641e91e9ef8] [UNKNOWN] 394 20 4 0 0 50
212.150.115.131 - - [17/Aug/2000:02:39:15 -0700]  "GET exorbis/smil/cooking_essentials.smi RTSPT/1.0" 200 371 [Win98_4.10_6.0.9.357_play32_MF80_en-US_586] [af8f71ea-582e-4d2a-a7ab-e641e91e9ef8] [UNKNOWN] 394 20 74 0 0 51


10.20.5.82 - - [17/Jun/2002:14:41:11 +1000]  "GET presentation/presentation.rt RTSP/1.0" 200 233 [WinNT_5.0_6.0.9.584_plus32_SP80_en-US_686] [ab5c323d-9b92-49e6-82b6-b5ac8ae1e7e3] [Stat3:1071|0|Resume|;11316|4159|PAUSE|;11586|10996|SEEK|;11586|10996|Resume|;63311|51026|STOP|;] 232 5 63 0 0 [0 0 0 0] [17/Jun/2002:14:40:06] 10.20.5.82 406 1
10.20.5.82 - - [17/Jun/2002:14:41:11 +1000]  "GET presentation/presentation.rm RTSP/1.0" 200 259200 [WinNT_5.0_6.0.9.584_plus32_SP80_en-US_686] [ab5c323d-9b92-49e6-82b6-b5ac8ae1e7e3] [Stat1:       429          0          0          0          0 20_Kbps_Music_-_High_Response][Stat2:     20689      33058          0          0          0          0          0          0          0          0          0 20_Kbps_Music_-_High_Response][Stat3:1081|0|Resume|;11306|4159|PAUSE|;11576|10996|SEEK|;11576|10996|Resume|;63321|51026|STOP|;] 947521 104 63 0 0 [1 0 0 0] [17/Jun/2002:14:40:06] 10.20.5.82 20689 432
10.20.5.82 - - [17/Jun/2002:14:41:11 +1000]  "GET presentation/presentation.rp RTSP/1.0" 200 134050 [WinNT_5.0_6.0.9.584_plus32_SP80_en-US_686] [ab5c323d-9b92-49e6-82b6-b5ac8ae1e7e3] [Stat3:931|0|PAUSE|;992|0|Resume|;10916|4159|PAUSE|;11186|10996|SEEK|;11186|10996|Resume|;62921|51026|STOP|;] 3918 84 62 0 0 [0 0 0 0] [17/Jun/2002:14:40:06] 10.20.5.82 12000 306
10.20.5.82 - - [17/Jun/2002:14:41:11 +1000]  "GET presentation/presentation-captions.rt RTSP/1.0" 200 4147 [WinNT_5.0_6.0.9.584_plus32_SP80_en-US_686] [ab5c323d-9b92-49e6-82b6-b5ac8ae1e7e3] [Stat3:871|0|PAUSE|;972|0|Resume|;10906|4159|PAUSE|;11176|10996|SEEK|;11176|10996|Resume|;62911|51026|STOP|;] 2905 87 62 0 0 [0 0 0 0] [17/Jun/2002:14:40:06] 10.20.5.82 320 7
10.20.5.82 - - [17/Jun/2002:14:41:11 +1000]  "GET presentation/presentationvideo.rm RTSP/1.0" 200 0 [WinNT_5.0_6.0.9.584_plus32_SP80_en-US_686] [ab5c323d-9b92-49e6-82b6-b5ac8ae1e7e3] [Stat3:861|0|PAUSE|;951|0|Resume|;11166|10996|SEEK|;11166|10996|Resume|;62891|51026|STOP|;] 125184 4 62 0 0 [0 1 0 0] [17/Jun/2002:14:40:06] 10.20.5.82 80000 0
10.20.5.82 - - [17/Jun/2002:14:41:52 +1000]  "GET admin/config_logging.html HTTP/1.0" 200 15500 [Mozilla/4.0 (compatible;MSIE 6.0;Windows NT 5.0;.NET CLR 1.0.3705)] [] [] 0 0 0 0 0 [0 0 0 0] [17/Jun/2002:14:41:51] 10.20.5.82 0 0


Logging Style value Individual record format 
=============================================
0 client_IP_address - - [timestamp] "GET filename protocol/version" HTTP_status_code bytes_sent [client_info] [client_id] [StatsMask results] 
1 client_IP_address - - [timestamp] "GET filename protocol/version" HTTP_status_code bytes_sent [client_info] [client_id] [StatsMask results] file_size file_time sent_time resends failed_resends 
2 client_IP_address - - [timestamp] "GET filename protocol/version" HTTP_status_code bytes_sent [client_info] [client_id] [StatsMask results] file_size file_time sent_time resends failed_resends 
3 client_IP_address - - [timestamp] "GET filename protocol/version" HTTP_status_code bytes_sent [client_info] [client_id] [StatsMask results] file_size file_time sent_time resends failed_resends stream_components start_time server_address 
4 client_IP_address - - [timestamp] "GET filename protocol/version" HTTP_status_code bytes_sent [client_info] [client_id] [StatsMask results] file_size file_time sent_time resends failed_resends stream_components start_time server_address average_bitrate packets_sent
5 client_IP_address - - [timestamp] "GET filename protocol/version" HTTP_status_code bytes_sent [client_info] [client_id] [StatsMask results] file_size file_time sent_time resends failed_resends stream_components start_time server_address average_bitrate packets_sent presentation_id




Style and # of last params.
0	0
1	5
2	5
3	8
4	10
5	11

Detailed Log Format:
http://service.real.com/help/library/guides/g2/htmfiles/report.htm



file_size			Total amount in bytes of media data in the media file. This number is less than the size of the media file because it does not include the file header and other non-media information stored in the file. For live broadcasts, file_size is always 0.Included when Logging Style is set to 1 or higher. 
file_time			Total length, in seconds, of media stored in the media file. For live broadcasts, file_time is always 0.Included when Logging Style is set to 1 or higher. 
sent_time			Total length, in seconds, of the media sent to the client. Included when Logging Style is set to 1 or higher.
					resends	Number of packets successfully resent because of transmission errors. Included when Logging Style is set to 1 or higher.
failed_resends		Number of packets not successfully resent in time to correct transmission errors. Included when Logging Style is set to 1 or higher.
stream_components	Type of material sent, indicated in the following pattern:RealAudio RealVideo Event RealImage 1 shows that the stream includes this type, 0 indicates that it does not. Thus, a stream that included RealVideo and RealAudio but no events or RealImages would appear in the access log as:1 1 0 0. Included when Logging Style is set to 3 or higher.
start_time			Timestamp of start time.Included when Logging Style is set to 3 or higher.
server_address		IP address of RealServer supplying the clip. If a splitter is in use, the receive splitter will be indicated here, rather than the source splitter's name. Included when Logging Style is set to 3 or higher.
average_bitrate		Average bitrate of clip. Included when Logging Style is set to 4 or higher.
packets_sent		Number of packets sent. Included when Logging Style is set to 4 or higher.
presentation_id		Number used by other clips in a SMIL presentation. All elements from the same presentation use the same number. The SMIL file itself is also included in the log, and shares the number as well. The number is assigned by RealServer at the time of transmission. Included when Logging Style is 5. 





[Stat3:timestamp|elapsed_time|action|;] 

Records of activity are separated by a semicolon (;) and are in the following form:



ABORT	Abnormal client stop (not the natural end of clip play). 
CLICK	Visitor clicked on the image map. Further information includes: 
		x-coord Horizontal coordinate of click. 
		y-coord Vertical coordinate of click. 
		action Action that occurred. This is one of the following: 
			PLAYER="url" The URL of the link the viewer clicked, as used in the client 
			URL="url" The URL of the link the viewer clicked, as used in the Browser. 
			SEEK="destination" The seek destination point, in milliseconds. 
PAUSE	The visitor paused the client. 
RESUME	Resume play after a pause, seek or stop. 
SEEK	The seek destination point, in milliseconds. 
STOP	End of clip reached. 
RECSTARTRealPlayer Plus began recording the clip.  
RECEND	RealPlayer Plus stopped recording the clip. */










// ConvLDate - convert date from "Fri, 07 Mar 1997" or "28/Mar/1994" and "2/Mar/1994,
// or "28/Mar/94" or "3/28/94" to "03/28/94" also scans international dates
// need to handle "080Sep/1999" currupt format
char *ConvAnyDateToMDY( char *buff, char *out )
{
	short	monthnum, num=0, i=0;
	char	c,
			*pt,
			swapdm;
	char 	*buffPtr,
			*mdyStr[3];	// This is to be the MM/DD/YY array
	static char *months[] = { "00","01","02","03","04","05","06","07","08","09","10","11","12",0 };
	
	if (!buff || !out )
		return 0;

	swapdm = 0;

	pt = buff;
	if( pt[3] == '.' || pt[3] == ',' ) pt+=5;
	mdyStr[0] = pt;
	c = 0;
	while ( pt && i<3 ) {
		if( c <'0' || c == ':' ){		// seperator found or end found
			mdyStr[i] = pt;
		} else
			c = *pt++;
	}

	if(i<3) return 0;
	
	//if it is the extended date format, make sure to wap the year to the last field.
	//0123456789
	//YYYY-MM-DD
	if( buff[4]<='0' && buff[7]<='0' ){
		pt = mdyStr[0];
		mdyStr[0] = mdyStr[2];
		mdyStr[2] = pt;
	}

	// if month is #2, make it #1
	if( *mdyStr[1] >= 'A' ){
		pt = mdyStr[0];
		mdyStr[0] = mdyStr[1];
		mdyStr[1] = pt;
	}

	// Convert month name to number
	if( *mdyStr[0] >= 'A' ){
		monthnum = Month2Num( mdyStr[0] );
		mdyStr[0] = months[monthnum];
	}

	buffPtr = out;
	i = 0;
	pt = mdyStr[i++];
	while( i<3 ){
		c=*pt++;
		if( c < '0' || c==':' ){
			c='/';
			pt = mdyStr[i++];
		} else
		// Fix 2 digit years 9x and 0x
		if( i == 2 ){
			if( c == '9' ){
				*buffPtr++ = '1';
				*buffPtr++ = '9';
			}
			if( c != '2' ){
				*buffPtr++ = '2';
				*buffPtr++ = '0';
			}
		}
		*buffPtr++ = c;
	}
	*buffPtr++ = 0;

	return(out);
}






short PROC_Realserver(char *buffer,HitDataPtr Line)
{
	char 	ch,nextch,delim,*p;
	char	*date_ptr;
	short	nudge,x=0,i=1, num, statMask = 0;		
	
	//tokenise line delimited by various tokens ' ', [] and "
	format[i++]=buffer;
	delim = ' ';
	nudge = 0;
	while ( (ch=*buffer) && i<FORMATSIZE ) {
		if( ch == delim) {
			*buffer++ = 0;
			nextch=*(buffer);
			if (nextch=='[' || nextch==0 ) {		// space detection added by Raul 29Nov00
				format[i] = buffer;
			} else {
				buffer+=nudge;
				if( *buffer == ' ' ) buffer++;
				format[i] = buffer;
			}

			switch( *format[i] ){
				case '\"' :
					format[i]++;
					buffer++;
					delim='\"';
					nudge=1;
					break;
				case '[' :		//remove [ from string
					format[i]++;
					buffer++;
					delim=']';
					nudge=1;
					break;
				default:
					if (*buffer == ' '){	// ignore damaged lines with more than 1 space
						buffer++;
						continue;//return 0;
					}
					delim=' ';
					nudge=0;
					break;
			}
			i++;
		} else
			buffer++;
	}
	logNumDelimInLine = i;
	logDelim = i;
	//make sure it is a valid line
	if (i<8)
		return 0;
	num = i;


	// ------------------------------------------------------------------ CORE FIELDS   1 ... 7

	Line->date = Line->time = 0;
	Line->stat = format[6];
	Line->clientaddr = format[1];
	Line->bytes = myatoi(format[7]);

	// extract url from "get url type" whether type is present or not
	p = format[5];
	if( *p == '-' ){
		return -1;
	} else 
	{	// Decode the URL component.
		Line->protocol = 0;
		while (*p) {
			if (*p==' ') {
				*p++=0;
				Line->file = p;
				// url has a " GET /url HTTP/1.0" so we get the actual PROTOCOL too.   eg "GET zuka/10radio/10intro.rm RTSP/1.0"
				if( p = strrchr( p, ' ' ) ){
					*p++ = 0;
					if( *p != '-' || *p != ' ' )
						Line->protocol = p;
				}
				break;
			}
			p++;
		}
	}
	// ------------------------------------------------------------------ BASIC FIELDS   8 ... 12
	Line->agent = format[8];
	if( Line->agent )		
	{
		// [Win98_4.10_6.0.9.357_play32_MF80_en-US_586]
		// [Mozilla/4.0 (compatible;MSIE 6.0;Windows NT 5.0;.NET CLR 1.0.3705)]

		// Only if its a none browser agent, decode it into player type
		if( (p=strchr( Line->agent, '_' )) )
		{
			char *field[256]; int nfields;

			PopulateArgsFromLine( p+1, field, nfields, '_' );
			Line->s_playeros = Line->agent;
			Line->s_playerosver = field[0];
			Line->s_playerver = field[1];
			Line->s_playerlang = field[4];
			if( Line->s_playerlang && Line->s_playerlang[0] == '-' )
				Line->s_playerlang = 0;

			Line->s_playercpu = field[5];
			if( Line->s_playercpu && Line->s_playercpu[0] == '-' )
				Line->s_playercpu = 0;


			if( field[2] ){
				sprintf( s_agent, "%s %s", field[2], Line->s_playerver );
				Line->agent = s_agent;
			} else
				Line->agent = 0;
		} else {
			Line->s_playeros =
			Line->s_playerosver =
			Line->s_playerver =
			Line->s_playerlang =
			Line->s_playercpu = 0;
		}
	}
	Line->s_playerGUID = format[9];

	Line->s_audiocodec = 0;
	Line->s_videocodec = 0;
	// ------------------------------------------------------------------ STAT MASK FIELDS 10......
	x = 10;

	while( !strcmpd( "Stat", format[x] ) && x<i )
	{
		char *argv[256]; int argc;


		// Statistics Type 1 gathers basic information about how successfully audio clips were received by the client.
		// eg. [Stat1:       429          0          0          0          0 20_Kbps_Music_-_High_Response]
		if( !strcmpd( "Stat1:", format[x] ) )			// 1...5 is numeric,   #6 is bandwidth
		{
			PopulateArgsFromLine( format[x], argv, argc, ' ' );
			Line->s_packets_rec = myatoi(argv[1]);
			Line->s_audiocodec = argv[6];
			statMask |= 1;
		} else
		// Statistics Type 2 provide details about the success of clip delivery, giving information about bandwidth requests. 
		// eg. [Stat2:     20689      33058          0          0          0          0          0          0          0          0          0 20_Kbps_Music_-_High_Response]
		if( !strcmpd( "Stat2:", format[x] ) )			// 1...11 is numeric,   #6 is bandwidth
		{
			PopulateArgsFromLine( format[x], argv, argc, ' ' );
			//Line->s_clipbandwidth = myatoi(argv[1]);
			Line->s_buffercount = myatoi(argv[6]);		//Number of resend packets requested by the client.
			Line->s_buffertime = myatoi(argv[7]);		//Total number of resent packets received by the client.
			statMask |= 2;
		} else
		// Statistics Type 3 provides detailed information about viewer action while listening or viewing clips
		// timestamp|elapsed_time|action|;
		// eg. [Stat3:1071|0|Resume|;11316|4159|PAUSE|;11586|10996|SEEK|;11586|10996|Resume|;63311|51026|STOP|;]
		if( !strcmpd( "Stat3:", format[x] ) )
		{
			long *data;
			char *events[1024]; 
			int nevents, item, pos = 0;

			if( Line->s_events )
			{
				delete [] Line->s_events;
				Line->s_events = NULL;
			}

			PopulateArgsFromLine( format[x]+6, events, nevents, ';' );
			data = new long [nevents*2];

			for( item=0; item< nevents; item++ )
			{
				char	*thisevent[16]; 
				int		eventargc;
				long	s_time, s_cmd;

				PopulateArgsFromLine( events[item], thisevent, eventargc, '|' );
				s_time = Line->s_starttime + myatoi( thisevent[0] );
				if( !strcmpd( "ABORT", thisevent[2] ) )	s_cmd = SESS_STREAM_ABORT;
				if( !strcmpd( "PAUSE", thisevent[2] ) )	s_cmd = SESS_STREAM_PAUSE;
				if( !strcmpd( "RESUME", thisevent[2] ) )	s_cmd = SESS_STREAM_RESUME;
				if( !strcmpd( "SEEK", thisevent[2] ) )		s_cmd = SESS_STREAM_SEEK;
				if( !strcmpd( "STOP", thisevent[2] ) )		s_cmd = SESS_STREAM_STOP;
				if( !strcmpd( "RECSTART", thisevent[2] ) )	s_cmd = SESS_STREAM_RECSTART;
				if( !strcmpd( "RECEND", thisevent[2] ) )	s_cmd = SESS_STREAM_RECSTOP;

				data[pos++] = s_cmd;
				data[pos++] = s_time;
			}
			// This event list can be directly added to the session history for that clip access...
			Line->s_events = data;
			statMask |= 4;

		} else
		if( !strcmpd( "Stat4:", format[x] ) )
		{
			statMask |= 8;

		} else
		if( !strcmpd( "Stat5:", format[x] ) )
		{
			statMask |= 16;

		} else
		if( !strcmpd( "Stat6:", format[x] ) )
		{
			statMask |= 32;
		}
		x++;
	}


/*

file_size file_time sent_time resends failed_resends stream_components start_time server_address average_bitrate packets_sent presentation_id
947521 104 63 0 0 [1 0 0 0] [17/Jun/2002:14:40:06] 10.20.5.82 20689 432

*/

	// Check for style 1 on wards....
	if( isdigit( *format[x] ) )
	{
		Line->s_filesize = myatoi( format[x] );			//Total amount in bytes of media data in the media file. 
		Line->s_filedur = myatoi( format[x+1] );		//Total length, in seconds, of media stored in the media file. For live broadcasts, file_time is always 0.
		Line->s_wm_streamduration = myatoi( format[x+2] );		//Total length, in seconds, of the media sent to the client. 

		if( x+5 < i )
		{
			//if( format[x+5][0] == '1' )  Line->s_audiocodec = argv[6];
			if( format[x+5][2] == '1' )  Line->s_videocodec = "RealVideo";
		}

		if( x+6 < i )
		{
			// Get the 'start_time'
			Line->date = format[x+6];

			date_ptr = Line->date;
			while (*date_ptr!=':') date_ptr++;
			*date_ptr++=0;
			//extract time token
			Line->time = date_ptr;
			
			Line->date = ConvLDate( Line->date, Line->newdate );
			if (!Line->date || *Line->date==0 )
				return 0;
		} else
			Line->date = Line->time = 0;

		// get average_bitrate
		if( x+8 < i )
			Line->s_meanbps = myatoi( format[x+8] );
		else
			Line->s_meanbps = 0;

		// get packets_sent
		if( x+9 < i )
			Line->s_packets_sent = myatoi( format[x+9] );
		else
			Line->s_packets_sent = 0;
	} else {
		Line->s_filesize =
		Line->s_filedur =
		Line->s_wm_streamduration =
		Line->s_meanbps =
		Line->s_packets_sent = 0;
	}

	//  Now decode the date format if not in statmask
	if( Line->date == 0 )
	{
		//extract date from [date:time GMT] token
		date_ptr=format[4];
		while (*format[4]!=':') format[4]++;
		*format[4]++=0;
		Line->time=format[4];	//extract time token
		while (*format[4]!=' ') format[4]++;
		*format[4]=0;
		Line->date = ConvLDate( date_ptr, Line->newdate );		// Convert to MM/DD/YYYY

		// Adjust fix the time to be the real start time, not log time
		AdjustLogDatebyDuration( Line, Line->s_wm_streamduration );
	}
	
	if( (statMask & 0x1) == 0 ){
		Line->s_packets_rec = 0;
	};

	if( (statMask & 0x2) == 0 ){
		Line->s_buffercount =
		Line->s_buffertime = 0;
	};

	Line->s_buffercount = -1;
	Line->s_percOK = -1;
	return 1;
}


