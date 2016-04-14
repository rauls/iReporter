#ifndef	__STATS_H
#define __STATS_H

#include "FWA.h"

#include "datetime.h"
#include "myansi.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef long (*CompFunc) (void *, void *, long *);

typedef int (*FilterFunc)(class Statistic *p );

//typedef int (*CompFunc)(const void *, const void *);





class Statistic {
public:
	Statistic(void );
	~Statistic();
	void				FreeData( void );
	long				ChangeName( char *newname );
	void				IncrStats( __int64 byte_count, __int64 in );
	unsigned long		GetVisits(void);
	void				SetVisits( long x );
	void				AddBytes( __int64 x );
	void				AddBytesIn( __int64 x );
	void				AddVisits( long x );
	void				AddCounters( long x );
	void				AddCounters4( long x );
	void				AddTimeDur( long x );
	void				AddFiles( long x );
	unsigned long		GetFiles(void);
	unsigned long		GetErrors(void);
	__int64				GetBytes(void);
	__int64				GetBytesIn(void);
	long				GetBytes2(int);
	long				GetFiles2(int);
	char *				GetName(void);
	char *				AllocateName( long len );
	void				DeleteName();
	long				StoreName( char *newName );
	long				GetDay(void);
	long				GetDur(void);
	unsigned long		GetHash(void) const { return hash; }
	long				GetVisitIn() { return counter5; }
	void				SetVisitIn( long v ) { counter5 = v; }
	// Streaming portion
	unsigned long		GetMostCommonError( void );
	unsigned long		GetMostErrorcodeHits( long errorcode );
	unsigned long		GetPacketsRec(void) {	return (unsigned long)(bytesIn>>32); };
	unsigned long		GetPacketsSent(void) {	return (unsigned long)(bytesIn&0xFFFFFFFF); };
	double				GetPacketsLost(void) {	return (GetPacketsSent()-GetPacketsRec()); };
	double				GetPacketLossPerc(void) {	if ( GetPacketsLost() ) return 100.0 * GetPacketsLost()/GetPacketsSent(); else return 0;};
	unsigned char		GetPageType( void ) { return counter2; };
	void				SetPageType( unsigned char type ) { counter2 = type; };
	unsigned char		GetProtocolType( void ) { return counter3; };
	void				SetProtocolType( unsigned char type ) { counter3 = type; };
	// -------------------------------------
	char 				*name;				// stat name											(4bytes)
	short				length;				// length of stat string 'name'							(2bytes)
	long				id;					// id index of stat with in database					(4bytes)
	unsigned long		hash;				// hash ID of stat name									(4bytes)
	unsigned long		files;				// hits for this type of stat							(4bytes)
	__int64				bytes;				// bytes sent for this stat								(8bytes)
	__int64				bytesIn;			// bytes recieved for this stat							(8bytes)
	long				visits;				// amount of unique **SESSIONS** for this statgph.type = GRAPH_MULTIVHOST3D;		(4bytes)
	long				counter;			// custom counter for whatever...						(4bytes)
	unsigned char		counter2;			// custom counter2 for browserID, or Page Type			(1bytes)
	unsigned char		counter3;			// custom counter2 for opersysID						(1bytes)
	long				counter4;			// bytesTot, or Firstpages count or pages count			(4bytes)
	long				errors;				// the amount of failures (ERRORS) for this record		(4bytes)
protected:
	long				counter5;			// This is rarely used, so far only for pageexit count, and tempararily for sessions to work out session length in time.
public:
	time_t				lastday;			// last accessed (4b)									(4bytes)
	long				visitTot;			//														(4bytes)
	class TimeRecStat	*timeStat;			// used for time spacial information for this stat (its history)		(4bytes)
	union {
	class SessionStat	*sessionStat;		// store sessions history								(4bytes)
	class ClipStat		*clipData;
	};
	class Statistic 	*next;				//														(4bytes)
	unsigned long		nextHash;			// backup next reference incase nextpointer is wrong	(4bytes)
	// -------------------------------------
};	// 80 bytes













// doTimeStat types (bit field defined, you can intermix them)
#define	TIMESTAT_DAYSHISTORY		(0x01)			// store value based on ctime logdays
#define	TIMESTAT_DIRECT2			(0x02)			// store value directly in timestat2
#define	TIMESTAT_DIRECT1			(0x04)			// Store value direct like DIRECT2, but in timestat1

// doSessionStat types (numerically defined)
enum {
	SESSIONSTAT_CLICKSTREAM = 1,
	SESSIONSTAT_CLIPDATA
};



// This defines the types of urls for byFile, to be store in byFile statistic->counter3
// So for all URLs we define its 'type' so we can list/show all pages or clips or images etc....
enum {
	URLID_UNKNOWN,
	URLID_PAGE,
	URLID_PAGECGI,
	URLID_DOWNLOAD,
	URLID_UPLOAD,
	URLID_CLIPAUDIO,
	URLID_CLIPVIDEO,
	URLID_LIVEAUDIO,
	URLID_LIVEVIDEO,
	URLID_IMAGE,
	URLID_JAVA,
	URLID_UNDEFINED=255
};

// The types of 'fields' values for each file/url.
enum {
	VALUE_TOTALNUM,
	VALUE_TOTALHITS,
	VALUE_TOTALBYTES,
	VALUE_TOTALBYTESIN,
	VALUE_TOTALVISITS,
	VALUE_TOTALVISITORS,
	VALUE_TOTALPAGES,
	VALUE_TOTALERRORS,
	VALUE_TOTALFIRSTS,
	VALUE_TOTALEXITS,
	VALUE_TOTALDURATION,
};


class StatList {
public:
	StatList( void *, long iSize);
	~StatList();
	Statistic *			GrowStatList( long );
	Statistic *			GrowSegStatList( long );
	void				IncrStatsTotals( __int64 byte_count, __int64 byte_countin, long ctime_now);
	void				IncrStats( __int64 byte_count, __int64 byte_countin, Statistic* p, long ctime_now);
	Statistic *			IncrStats( __int64 byte_count, __int64 byte_countin, char *str,    long ctime_now);
	void				IncrStatsTotalsMore( __int64 byte_count, __int64 byte_countin, long ctime_now, long flags );
	void				IncrStatsMore(__int64 byte_count, __int64 byte_countin, Statistic* p, long ctime_now, long flags );
	Statistic *			IncrStatsMore(__int64 byte_count, __int64 byte_countin, char *str,    long ctime_now, long flags );
	Statistic *			AddStatsSession( Statistic* p, __int64 byte_count, __int64 byte_countin, time_t ctime_now, time_t last, long flags, long param, long errorcode );
	Statistic *			IncrStatsDrill( __int64 byte_count, __int64 byte_countin, char *str, long ctime_now, long flags, long param );
	Statistic *			IncrStatsClickstream( __int64 byte_count, __int64 byte_countin, char *str, long ctime_now, long flags, long param, long errorcode );
	long				AddStatsToStat( Statistic *tostat, Statistic *stat1, long inctotals );
	Statistic *			AddStatsFromStat( char *name, Statistic *stat1 );
	Statistic *			IncrStatsFromStat( Statistic *stat1 );
	long				CurrentIncrVisits( void );
	long				CurrentIncrVisitors( void );
	long				IncrVisits( long index );
	long				IncrVisitors( long index );
	long				IncrPageExitCount( long index );
	long				AddToVisits( char *name );
	long				AddToVisitors( char *name );
	void				IncrCounter4( void );
	void				CurrentIncrPages( void ) { IncrCounter4(); }
	void				CurrentIncCounter( void );
	void				CurrentIncVisitor( void ) { CurrentIncCounter(); }
	long				CurrentGetCounter( void );
	void				CurrentSetCounter( long x );
	void				CurrentSetCounter4( long x );
	void				SetCounter( long n, long x );
	void				SetCounter2( long n, unsigned char x );
	void				SetCounter3( long n, unsigned char x );
	void				SetCounter4( long n, long x );
	void				IncCounter( int n );
	void				SetVisits( int n, long x );
	void				AddVisits( int n, long x );
	void				AddBytes( int n, __int64 x );
	void				AddBytesIn( int n, __int64 x );
	void				AddFiles( Statistic *p, long x );
	void				AddCounters( int n, long x );
	void				AddCounters4( Statistic *p, long x );
	void				AddCounters4To( int n, long x );
	void				AddTimeDur( int n, long x );

	void *				GetVD() const { return vd; }
	Statistic *			FindStatbyName( char *str);
	Statistic *			SearchStat( char *str);
	void				Substitute( char *(*func)(const char *));
	void				Sort( CompFunc func=0, long list=20, short sortOrder = TOP_SORT );
	void				DoSort( long type, long list, short sortOrder = TOP_SORT );
private:
	void				ReBuildPtrs( void );
	void				ReBuildHeaders( void );
public:
	void				MarkForRebuild(void);

	char *				GetName(int n);
	char *				(*GetNameTrans)( short n );
	unsigned long		GetVisits(int n);
	unsigned long		GetFiles(int n);
	unsigned long		GetErrors(int n);
	__int64				GetBytes(int n);
	__int64				GetBytesIn(int n);
	unsigned long		GetPacketsRec(int n);
	unsigned long		GetPacketsSent(int n);
	long				GetVisitIn( long n );
	long				GetCounter( long n );
	short				GetCounter2( long n );
	short				GetCounter3( long n );
	long				GetCounter4( long n );
	long				GetPagesCount( long n ) { return GetCounter4( n ); }
	long				GetDur(int n);
	long				GetDaysAt( int n, int index);
	long				GetFilesAt( int n, int index);
	long				GetBytesAt( int n, int index);
	long				GetFilesHistory(int n, int day);
	long				GetBytesHistory(int n, int day);
	long				SortTimeStat( int n, int type );
	long				GetHistoryNum( int n );

	long				GetDaysAt2( int n, int index);
	long				GetFilesAt2( int n, int index);
	long				GetBytesAt2( int n, int index);
	long				GetFilesHistory2(int n, int day);
	long				GetBytesHistory2(int n, int day);
	long				SortTimeStat2( int n, int type );
	long				GetHistoryNum2( int n );

	long 				Done( void );
	Statistic	*		GetFirstStat( void );
	Statistic	*		GetNextStat( Statistic *p );
	Statistic	*		GetStat(int index);
	Statistic	*		GetEmptyStat( void );
	long				GetDay(int n);
	unsigned long		CurrentGetHash( void );
	unsigned long		GetHash(int n);
	int 				FindHash( unsigned long thishash );
	Statistic	*		FindHashStat( unsigned long thishash );
	long				GetLength(int n);
	long 				GetStatListNum( void );
	long 				GetStatListMaxNum( void );
	__int64				GetStatListTotalBytes( void );
	__int64				GetStatListTotalBytesIn( void );
	unsigned long		GetStatListTotalErrors( void );
	unsigned long		GetStatListTotalRequests( void );
	unsigned long		GetStatListTotalCounters( void );
	unsigned long		GetStatListTotalCounters4( void );
	unsigned long		GetStatListTotalVisits( void );
	long				GetStatListTotalTime( void );
	char *				GetStatDetails( long hashid, Statistic** ppStat, long *bytes );
	void				StatSort( void *base, long members, size_t datasize, CompFunc cmp, long topn, short sortOrder );
	void				ClearTimeHistoryPointers( void );
	bool				TestAnyMatches(	 FilterFunc pFilterIndex = NULL );
	unsigned long		GetTotalbyFilter( FilterFunc pFilterIndex = NULL );
	unsigned long		GetTotalSum_byFile( int valuetype, unsigned char urlid );

	unsigned long		GetPacketsRec(void) {	return (unsigned long)(totalBytesIn>>32); };
	unsigned long		GetPacketsSent(void) {	return (unsigned long)(totalBytesIn&0xFFFFFFFF); };
	double				GetPacketsLost(void) {	return (GetPacketsSent()-GetPacketsRec()); };
	double				GetPacketLossPerc(void) {	if ( GetPacketsLost() ) return 100.0 * GetPacketsLost()/GetPacketsSent(); else return 0;};


	//-------------------------------------------------------------------------------------------
	char				stat_id[16];		// the name/id of this database, reserved for future use.
											// NOTE: consider this in database saves/loads
	long				num;				// number of entries in table
	long				maxNum;				// number of entries allocated per table
	long				incrSize;			// number of entries to expand by
	long				tableTot;			// number of tables (old style is only 1)
	long				tableNum;			// current table in use
	long				tableIdx;			// position with in table for current object
	long				totalRequests;		// total hits/ or files
	long				totalCounters;		// total custom counter
	long				totalCounters4;		// total custom counter
	long				totalVisits;		// total sessions
	long				totalErrors;		// total errors
	__int64				totalBytes;			// total bytes served
	__int64				totalBytesIn;		// total bytes recieved
	long				totalTime;			// total time of all sessions
	char				doTimeStat,			// use array of days for history over days
						doSessionStat,		// store session history for each client
						useOtherNames,		// use name directly, do not copy string.... (saves ram)
						useFixedNames;		// use fixed sized array names in side stat, no alloc, just copy (EXPERIMENTAL)
	char				doNotUseOrRemove;	// needs to be here for backward compatability with existing FWA databases
	char				newses;				// when a new record added, this is 1
	long				temp, temp2;		//
	FilterFunc			m_FilterIndex;
	long				reserved[2];		// reserved for future use for database stuff 
	Statistic			*stat;				// pointer to statistics table (first...?)
	Statistic			**tables;			// pointer to statistics tables, each one has 16384 entries
	Statistic			*current;			// current or last stat accessed/found
private:
	Statistic			**headPtr;			// pointer to array 2dim matrix of Statistic pointers
	unsigned long		*headHashRef;		// reference to above with the hash incase pointer is wrong
	long				indexSize;
public:
	void				*vd;				// virtual domain parent
};





int Filter_IsOther( class Statistic *p );
int Filter_IsPage( class Statistic *p );
int Filter_IsPageCGI( class Statistic *p );
int Filter_IsDownload( class Statistic *p );
int Filter_IsUpload( class Statistic *p );
int Filter_IsAudioClip( class Statistic *p );
int Filter_IsVideoClip( class Statistic *p );
int Filter_IsClip( class Statistic *p );
int Filter_IsLiveAudioClip( class Statistic *p );
int Filter_IsLiveVideoClip( class Statistic *p );
int Filter_IsLiveClip( class Statistic *p );
int Filter_IsAnyClip( class Statistic *p );
int Filter_IsUndefined( class Statistic *p );

#ifdef __cplusplus
}
#endif

#endif // __STATS_H