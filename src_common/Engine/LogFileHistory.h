
#ifndef LOG_FILE_HISTORY_H
#define LOG_FILE_HISTORY_H

#include "LogFileFormat.h"

#define	MAXLOGHISTORY				1000

class LogFileHistoryRec
{
public:
	char	*file;
	__int64	size;
	short	type;
	long	startDate;
	long	endDate;
	short	endDateKnown;
	short	compressionType;
	short	numOfFieldDelimiters;
	FormatIndex logFormatIndex;
};
typedef LogFileHistoryRec* LogFileHistoryPtr;

extern LogFileHistoryRec LogFileHistory[MAXLOGHISTORY];
extern long	logfilesTotal;

void DeleteLogXinHistory( long x );
char *FindLogXinHistory( long x );
long FindLogInHistory( char *logtofind );
void ClearLogList( void );
long AddLogToHistoryEx( char *newlogfile, bool getEndDate );
long CompSortLogs( void *p1, void *p2, long *result );
void SortLogHistoryFrom( long start, long total );
void ResetLogList( void );

#endif // LOG_FILE_HISTORY_H
