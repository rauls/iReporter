
#ifndef LOG_FILE_FORMAT_H
#define LOG_FILE_FORMAT_H

#define FORMATSIZE		64

#include "myansi.h"

class FormatIndex
{
// We are Supreme so we can access it all public
public:
	long hostname;
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
	long	s_wm_streamposition;
	long	s_wm_streamduration;

	long filler[FORMATSIZE];



public:
	FormatIndex()
	{
		Clear();
		//logIndex = &date;
	}

	void Clear()
	{
		SetHostName( 0 );
		SetDate( 0 ); 
		time =
		status =
		url = 
		agent = 
		bytes = 
		bytesIn = 
		referer = 
		user = 
		cshost =
		method =
		cookie =
		query =
		protocol =
		port =
		ms =
		csip = 
		process_accounting = 0;
		for( long i=0;i<FORMATSIZE;i++) filler[i] = 0;

	}

	long HostName()
	{
		if ( hostname < 0 )
			return 0;
		else
			return hostname;
	}
	void SetHostName( long index ) { hostname = index; }
	long Date()
	{
		if ( date >=0 && date<= 25 )
			return date;
		else
			return 0;
	}
	void SetDate( long index ) { date = index; }

	//long *logIndex;
};


#endif // LOG_FILE_FORMAT_H
