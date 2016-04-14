#ifndef	_ENGINEDRILL_
#define	_ENGINEDRILL_


//#ifdef __cplusplus
//extern "C"{
//#endif



enum {
	TIMESORT_FILES,
	TIMESORT_BYTES,
	TIMESORT_DAYS
};


// --------------- TIME STAT

typedef struct {
        long    day;
        long    bytes;
        long    files;
} TimeRec, *TimeRecPtr;

#define TIMERECSIZE		sizeof( TimeRec )

class TimeRecStat {
public:
	TimeRecStat();
	~TimeRecStat();
	
	int					AddTimeRec( long thisday, long byte_count );
	long				GetNum(void);
	long				GetTot(void);
	long				GetBytes( long n );
	long				GetFiles( long n );
	long				SetFiles( long f, long n );
	long				GetDaysIdx( long n );
	long				GetBytesIdx( long n );
	long				GetFilesIdx( long n );
	long				Sort( long type );

	// get values from timestat2
	long				GetDaysIdx2( long n )	{ if (timeStat2) return timeStat2->GetDaysIdx(n); else return 0; };
	long				GetBytesIdx2( long n )	{ if (timeStat2) return timeStat2->GetBytesIdx(n); else return 0; };
	long				GetFilesIdx2( long n )	{ if (timeStat2) return timeStat2->GetFilesIdx(n); else return 0; };

	TimeRecPtr			byTimerec;
	long				byTimerecNum;
	long				byTimerecTot;
	TimeRecStat			*timeStat2;

};






// ------------- SESSION STAT




typedef struct {
        long		day;
        long   		pageID;
} SessionRec, *SessionRecPtr;

class SessionStat {
public:
	SessionStat();
	~SessionStat();
	
	long				RecordSessionHistory( long id, long day  );
	long				RecordCustomData( long reference, long value );
	long				RecordMeanPath( long pageid , unsigned long ulSessionIndex);
	void				SetReferral( long );
	long				GetSessionPage( int n );
	long				GetSessionTime( int n );
	long				GetCustomData1( int n );
	long				GetCustomData2( int n );
	long				GetMaxDur(void);
	long				GetMaxLen(void);
	long				GetNum(void);
	long				GetTot(void);

	void				*vd;
	long				parenthash;
	long				SessionLen;		// length of current live individual session in pages, not inc headers
	long				SessionMaxLen;	// largest length of individual session
	long				SessionMaxDur;	// longest duration

	/*long				*pageIDs; */
	long				SessionStart;
	long				SessionHash;
	long				SessionNum;		// total number of session records
	long				SessionTot;		// total slots to use
	long				SessionRef;		// first referral for session (tempary value used in live mode, not critical to save it really)
	long				SessionBytes;	// This is used in calculating the amount of bytes per session as its being added...
	SessionRecPtr		Session;
};














// ---------------- CLIP DETAILS STAT





// These are details of the clip structure which attached to each clip item.


class ClipStat {
public:
	ClipStat();
	~ClipStat();
	// although stored values are unsigned, 
	long			AddStats( long cpu, char quality, long bps, unsigned long seconds_sent, long bufcount );
	long			IncFF( void ) { m_ff_count++; };
	long			IncRW( void ) { m_rw_count++; };
	long			SetStats( unsigned short file_length, unsigned long file_size, long aud_id, long vid_id );
	
	//unsigned long	clip_stat_id;		// 0xffffee01;

	// Fixed values.
    unsigned long	m_audiocodecID;		// reference type
    unsigned long	m_videocodecID;
    unsigned long	m_file_size;		// file's size on server (bytes)
    unsigned short	m_file_length;		// file's length on server (seconds)
	// Average Values
    unsigned char	m_meancpu;			// 1...100
	unsigned char	m_meanquality;		// 1...100 (not all log types)
    unsigned long	m_meanbps;			// eg, 32,000 or 512,000 bps
    unsigned long	m_meanbufcount;		// average buffer count.
	// Counter Values
	unsigned short	m_max_concurrent;	// max concurrent connections;
	unsigned long	m_played_length;	// total seconds transfered from server to client. (as apposed to visitTot which includes pauses)
	unsigned long	m_ff_count;
	unsigned long	m_rw_count;
	unsigned long	m_uninterrupted_count;
	// Special time of event value.
	time_t			m_timeof_max;		// time of when the max concurrent connections occured.

};





//#ifdef __cplusplus
//}
//#endif

#endif
