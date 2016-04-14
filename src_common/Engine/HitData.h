
#ifndef	__HITDATA_H
#define __HITDATA_H

#include <time.h>
#include <ctype.h>		// for size_t

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HitData
{
	// the following data member is used when adding to and reading from a V5 database
	const char* logFileName;	// filename of current log being processed

	// the following members are set by the GetLineDetails() functions
	char	*date;				// format varies
	char	*time;				// format varies
	char	*stat;				// status code
	char	*clientaddr;		// client ip address or domain name
	char	*file;				// requested URL (may or may not be fully qualified)
	char	*agent;				// agent field as specified within request
	long	bytes;				// bytes out - server to client
	long	bytesIn;			// bytes in - client to server
	char	*refer;				// referral URL - fully qualified and could by dynamic
	char	*cookie;			// dump text
	char	*user;				// dump text
	char	*vhost;				// server ip address or domain name
	char	*method;			// GET, POST, GET_CONDITIONAL

	// the following members are used as buffers to avoid overwritting the originals
	char	newdate[32];

	// the following members are set by the GetLineDetails() functions but are not used for web server logs
	char	*sourceaddr;		// src IP
	char	*protocol;
	long	port;
	long	ms;					// time to transfer that data...


	// Streaming variables.....
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
	
	// the following members are set within the Kahoona loop (engine_process.cpp)
	size_t  lineNum;			// log line number of current hit being processed
	long	ctime;				// time_t representation of date/time - adjusted accoring to settings
	char	newusername[128];	// username as obtained from URL CGI param
} HitDataRec, *HitDataPtr;

#ifdef __cplusplus
}
#endif

#endif // __HITDATA_H



