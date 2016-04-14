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

	File:            engine_drill.cpp
	Subsystem:       Log Decoder
	Created By:      Raul.Sobon, 01/01/97

	$Archive: /iReporter/Dev/src_common/Engine/engine_drill.cpp $
	$Revision: 25 $
	$Author: Raul.sobon $
	$Date: 1/08/02 2:36p $

    This file adds extra functionality to each stat

	TIMESTAT (daily stat)
	--------
	This adds an array of historical time data per stat, which can be either indexed, or search by .day and store bytes/files for each
	days access.

	TIMESTAT2 (browser/search-engine/error-stat)
	---------
	Hangs off timestat->timestat,
	Errors:
	The detail would be for all error types, just 2 values, ERROR#,COUNT,BYTES


	SESSION STAT
	------------
	This stores an array of access history per client in LONG pairs which details which pages/clips were accessed at what time, and
	when a session was started/finished and the total bytes transfered for that session also.

	CLIP STAT
	-----------
	Using same pointer as sessionStat, this would store extra details about a clip, such as bandwidth/quality/codecs/etc... lots of goodies
	This would hang of the byPages

	Stream Fields:
		mean cpu util,
		clip duration on disc (seconds),
		mean bps,
		audio codec,
		video codec,





	
	








  */




#include "Compiler.h"
#include <stddef.h>	// for size_t
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "myansi.h"
#include "zlib.h"
#include "gd.h"

#include "config_struct.h"

#include "Stats.h"				// for TOP_SORT
#include "EngineStatus.h"		// for MemoryError'
#include "EngineMeanPath.h"		// for 'MP_IncMeanPath'
#include "VirtDomInfo.h"		// for 'VDinfoP'
#include "HitData.h"
#include "engine_drill.h"

/* ----------------- Spacial Time recording......*/
unsigned long JDtoCompressedTime( double jd, double firstday );
double CompressedTimetoJD( unsigned long cjd, double firstday );

#define	kTimeRecInit	0		// initial timestat size (2 days)
#define	kTimeRecInc		4		// if full then increment by X (32 days)
#define	kSessionRecInc	10

extern "C" struct	App_config MyPrefStruct;





// ---------------------------------------------------------------------------------

TimeRecStat::TimeRecStat()
{
	byTimerecNum = 0;
	byTimerecTot = kTimeRecInit;
	byTimerec = 0;
	timeStat2 = NULL;
}

TimeRecStat::~TimeRecStat()
{
	delete [] byTimerec;
	if( timeStat2 )
		delete timeStat2;
	byTimerec = NULL;
}
	
long TimeRecStat::GetNum()
{
	if( this ) 
		return byTimerecNum;
	else
		return 0;
}
	
long TimeRecStat::GetTot()
{
	if( this ) 
		return byTimerecTot;
	else
		return 0;
}


// Sort field 1
long CompTimeFiles(void *p1, void *p2, long *result)
{
	long n1,n2;
	n1 = ((TimeRecPtr)p1)->files;
	n2 = ((TimeRecPtr)p2)->files;
	*result=n2-n1;
	return 0;
}

// Sort field 2
long CompTimeBytes(void *p1, void *p2, long *result)
{
	long n1,n2;
	n1 = ((TimeRecPtr)p1)->bytes;
	n2 = ((TimeRecPtr)p2)->bytes;
	*result=n2-n1;
	return 0;
}

// Sort field 0
long CompTimeDay(void *p1, void *p2, long *result)
{
	long n1,n2;
	n1 = ((TimeRecPtr)p1)->day;
	n2 = ((TimeRecPtr)p2)->day;
	*result=n2-n1;
	return 0;
}



long TimeRecStat::Sort( long type )
{
	switch ( type ){
		case TIMESORT_FILES:
		default:
			FastQSort( byTimerec, byTimerecNum, TIMERECSIZE, CompTimeFiles, TOP_SORT );
			break;
		case TIMESORT_BYTES:
			FastQSort( byTimerec, byTimerecNum, TIMERECSIZE, CompTimeBytes, TOP_SORT );
			break;
		case TIMESORT_DAYS:
			FastQSort( byTimerec, byTimerecNum, TIMERECSIZE, CompTimeDay, TOP_SORT );
			break;
	};
	return 0;
}

	
// get Bytes/Files info from record either using a DayOffset or REAL DAY!!!
long TimeRecStat::GetBytes( long n )
{
	if( this ){
		for( long f=0; f < byTimerecNum; f++){
			if( byTimerec[f].day == n ){
				return byTimerec[f].bytes;
			}
		}
	}
	return -1;
}

long TimeRecStat::GetFiles( long n )
{
	if( this ){
		for( long f=0; f < byTimerecNum; f++){
			if( byTimerec[f].day == n ){
				return byTimerec[f].files;
			}
		}
	}
	return -1;
}




// get Bytes/Files info from record either using a DayOffset or REAL DAY!!!
long TimeRecStat::GetDaysIdx( long n )
{
	if( this ){
		if( n< byTimerecTot )
			return byTimerec[n].day;
	}
	return -1;
}
long TimeRecStat::GetBytesIdx( long n )
{
	if( this ){
		if( n< byTimerecTot )
			return byTimerec[n].bytes;
	}
	return -1;
}
long TimeRecStat::GetFilesIdx( long n )
{
	if( this ){
		if( n< byTimerecTot )
			return byTimerec[n].files;
	}
	return -1;
}



long TimeRecStat::SetFiles( long f, long n )
{
	byTimerec[f].files = n;
	return n;
}


// add timerecord stamp data to current unique stat entry 
// either add suppling a day offset or supply a actual DAY
int TimeRecStat::AddTimeRec( long thisday, long byte_count )
{
	// find thisday within the byTimerrec array
	size_t timeRecIndex(0);

	for( timeRecIndex=0; timeRecIndex<static_cast<size_t>(byTimerecNum); timeRecIndex++)
	{
		if( byTimerec[timeRecIndex].day == thisday )
		{
			TimeRecPtr tr = &byTimerec[timeRecIndex];
			tr->day = thisday;
			tr->bytes += byte_count;
			tr->files ++;
			return timeRecIndex;
		}
	}
	
	// if we get to here then thisday wasn't found so add it on to the end
	DEF_ASSERT( timeRecIndex==static_cast<size_t>(byTimerecNum) );

	// do we need to extend the byTimerec array?
	if( timeRecIndex >= static_cast<size_t>(byTimerecTot) )
	{
		//increase data block size and add one time stamp
		long tries=1000;
		long newsize=byTimerecTot+kTimeRecInc;

jump1:
		TimeRecPtr tempTimerec=new TimeRec[newsize];
		if(tempTimerec)
		{
			if(byTimerecTot)
			{
				memcpy( tempTimerec, byTimerec, TIMERECSIZE*byTimerecTot );
				delete [] byTimerec;
			}
			byTimerec = tempTimerec;
			byTimerecTot+=kTimeRecInc;
		}
		else
		{
			if( tries>0 )
			{
				tries--;
				goto jump1;
			}
			MemoryError( "TimeRec Failed to Add", TIMERECSIZE*newsize );
		}
	}

	TimeRecPtr tr = &byTimerec[timeRecIndex];
	tr->day = thisday;
	tr->bytes = byte_count;
	tr->files = 1;
	byTimerecNum++;

	DEF_POSTCONDITION(byTimerecNum<=byTimerecTot);

	return timeRecIndex;
}

// convert a double julian day into a (15:17) bit struct (day:sec)
unsigned long JDtoCompressedTime( double jd, double firstday )
{
	double	jdday,jdsec;
	unsigned long day,sec, cjd;
	
	jdsec = modf( jd, &jdday );
	day = ((long)jdday) - ((long)firstday);
	sec = (unsigned long)(jdsec*86400);
	cjd = (day << 17) | sec;
	return cjd;
}

double CompressedTimetoJD( unsigned long cjd, double firstday )
{
	double	jd;
	unsigned long day,sec;

	day = cjd >> 17;
	sec = cjd & 0x1ffff;
	jd = firstday + day + (sec/86400.0);
	return jd;
}


















// ------------------------------------------------------------------------------------------------------------------


























/*
        Data to be stored as
                long page hash id to page where -1 ID = session break, -2 = END
                { 1,5,3,-1,  3,2,1,45,3,2,-1,  3,1,2,-2 }
                default size per client?... 16 shorts.
*/

SessionStat::SessionStat()
{
	SessionNum = 0;
	SessionTot = 0;
	Session = 0 ;//new SessionRec[SessionTot];

	SessionMaxLen = 0;
	SessionMaxDur = 0;
	SessionLen = 0;
	SessionStart = 0;
	SessionHash = 0;
}

SessionStat::~SessionStat()
{
	delete [] Session;
	Session = NULL;
}
	
long SessionStat::GetNum()
{
	return SessionNum;
}

long SessionStat::GetMaxDur()
{
	return SessionMaxDur;
}
long SessionStat::GetMaxLen()
{
	return SessionMaxLen;
}


	
long SessionStat::GetTot()
{
	return SessionTot;
}

long SessionStat::GetSessionPage( int n )
{
	if( n < SessionTot )
		return Session[n].pageID;
	else
		return -1;
}

long SessionStat::GetSessionTime( int n )
{
	if( n < SessionTot )
		return Session[n].day;
	else
		return -1;
}

long SessionStat::GetCustomData1( int n )
{
	return GetSessionTime( n );
}

long SessionStat::GetCustomData2( int n )
{
	return GetSessionPage( n );
}

void SessionStat::SetReferral( long id )
{
	if( this ){
		SessionRef = id;
	}
}


extern unsigned long crctab[256];


// go back to beginnning of session.
long SessionStat::RecordMeanPath( long pagehash , unsigned long ulSessionIndex)
{
	if( pagehash == 1 || pagehash == 4 ){
		SessionHash = 0;
//		SessionStart = SessionNum+1;
		SessionStart = ulSessionIndex+1;
	} else
	if( (unsigned long)pagehash > 10 ){
		// add new pathhash from pagehash
		// record session sequence
		if( SessionLen > 1 && SessionLen < 10 ){
			SessionHash = ((SessionHash<<16)^pagehash) ^ crctab[SessionLen&0xff] ;
			MP_IncMeanPath( (VDinfoP)vd, SessionHash, parenthash, SessionStart, static_cast<short>(SessionLen) );
		}
	}
	return 0;
}

// just record 1 entry in the session list.
//
// pagehash		1 = start
//				2 = byte length store
//				3 = end
//				4 = referral
//
long SessionStat::RecordSessionHistory( long pagehash, long day )
{
	if( SessionNum >= SessionTot ){
		SessionRecPtr tempSession;
		long inc;
		
		if( SessionNum > 100 )
			inc = (SessionTot>>3);
		else
			inc = kSessionRecInc;

		tempSession = new SessionRec[SessionTot+inc];

		if(tempSession) {
			if(SessionTot) {
				memcpy( tempSession, Session, (sizeof(SessionRec)*SessionTot) );
				delete [] Session;
			}
			Session = tempSession;
			SessionTot += inc;
		} else {
			MemoryError( "SessionRec Failed to Add", SessionTot+kSessionRecInc );
		}
	}
	if( Session ){
		Session[ SessionNum ].pageID = pagehash;
		Session[ SessionNum ].day = day;

		if( STAT(MyPrefStruct.stat_meanpath) )
		{
			// ***********************************************************************
			// Removed the building of the MeanPath data from here and have moved it
			// to the Post Processing code.
			// ***********************************************************************
			// RecordMeanPath( pagehash , SessionNum);
		}
		
		SessionNum++;
	}
	
	return SessionNum;
}



long SessionStat::RecordCustomData( long reference, long value )
{
	if( SessionNum >= SessionTot ){
		SessionRecPtr tempSession;
		long inc;
		
		if( SessionNum > 100 )
			inc = (SessionTot>>3);
		else
			inc = kSessionRecInc;

		tempSession = new SessionRec[SessionTot+inc];

		if(tempSession) {
			if(SessionTot) {
				memcpy( tempSession, Session, (sizeof(SessionRec)*SessionTot) );
				delete [] Session;
			}
			Session = tempSession;
			SessionTot += inc;
		} else {
			MemoryError( "SessionRec Failed to Add", SessionTot+kSessionRecInc );
		}
	}
	if( Session ){
		Session[ SessionNum ].day = reference;
		Session[ SessionNum ].pageID = value;

		SessionNum++;
	}
	
	return SessionNum;
}












// ------------------------------------



ClipStat::ClipStat()
{
	m_audiocodecID = 0;
	m_videocodecID = 0;
	m_file_size = 0;
	m_file_length = 0;

	m_meancpu = 0;
	m_meanquality = 0;
	m_meanbps = 0;
	m_meanbufcount = 0;

	m_max_concurrent = 0;
	m_played_length = 0;
	m_ff_count = m_rw_count = 0;
	m_uninterrupted_count = 0;
	m_timeof_max = 0;
}


ClipStat::~ClipStat()
{
}

long	ClipStat::AddStats( long cpu, char quality, long bps, unsigned long seconds_sent, long bufcount  )
{
	if( m_meancpu )
		m_meancpu = (m_meancpu+cpu)/2;
	else
		m_meancpu = cpu;

	if( m_meanquality && quality>0 )
		m_meanquality = (m_meanquality+quality)/2;
	else
	if( quality>0 )
		m_meanquality = quality;

	if( m_meanbps )
		m_meanbps = (m_meanbps+bps)/2;
	else
		m_meanbps = bps;

	if( m_meanbufcount && bufcount>0 )
		m_meanbufcount = (m_meanbufcount+bufcount)/2;
	else
	if( bufcount>0 )
		m_meanbufcount = bufcount;

	m_played_length += seconds_sent;

	if( seconds_sent == m_file_length )
		m_uninterrupted_count++;

	return 1;
};



long	ClipStat::SetStats( unsigned short length, unsigned long size, long aud_id, long vid_id )
{
	m_file_length = length;		// (seconds)
	m_file_size = size;			// (bytes)
	m_audiocodecID = aud_id;
	m_videocodecID = vid_id;
	return 1;
};






