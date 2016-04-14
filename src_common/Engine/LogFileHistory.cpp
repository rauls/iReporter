
#include "FWA.h"

#include <string.h>

#include "LogFileHistory.h"
#include "myansi.h"
#include "FileTypes.h"
#include "log_io.h"

LogFileHistoryRec LogFileHistory[MAXLOGHISTORY];
long logfilesTotal=0;

void DeleteLogXinHistory( long x )
{
	if ( x < logfilesTotal )
	{
		if( LogFileHistory[x].file )
		{
			free( LogFileHistory[x].file );
			LogFileHistory[x].file=0;
		}

		memcpy( &LogFileHistory[x],
			    &LogFileHistory[x+1],
				(sizeof(LogFileHistoryRec) * (logfilesTotal-x))
				);
	}
}

char *FindLogXinHistory( long x )
{
	if ( x < logfilesTotal ){
		return LogFileHistory[ x ].file;
	} else
		return NULL;
}

long FindLogInHistory( char *logtofind )
{
	long count;

	for ( count=0; count < logfilesTotal; count++ ){
		if ( LogFileHistory[ count ].file ){
			if ( strcmp( LogFileHistory[ count ].file, logtofind ) == 0 )
				return count;
		}
	}
	return -1;
}

void ClearLogList( void )
{
	for( long i(0); i<logfilesTotal; ++i )
	{
		if( LogFileHistory[i].file )
		{
			free( LogFileHistory[i].file );
			LogFileHistory[i].file=0;
		}
	}

	logfilesTotal = 0;
} 


// Clear the FILE TYPE/LENGTH details so they are re-read
void ResetLogList( void )
{
	for( long i(0); i<logfilesTotal; ++i )
	{
		LogFileHistory[i].type = -1;
	}
} 


// Heres the issue:
// 1. every log file processed or added to the list has to be added to the internal list
// 2. the unique case here is the GUI needs to be updated at the same time.
//    Also the GUI directly calls the functions too.
// The mac unfortunately uses the old style history which is limited
// so we have to use a diff name here.
//
// RETURNS: -1 if logfile is already in the list, or the new TOTAL
//
// NOTE: Make sure that whatever calls this, it also gets added to the GUI
//       If this returns -1, make sure to HIGHLIGHT the logfile in the GUI listview
//

long AddLogToHistoryEx( char *newlogfile, bool getEndDate )
{
	extern FormatIndex prevLogFileFormatIndex;
	extern short prevLogDelim;

	long len = strlen( newlogfile );

	// generic way to add to history (can work in all os)
	if ( logfilesTotal < MAXLOGHISTORY && len )
	{
		long pos = FindLogInHistory( newlogfile );

		if ( pos == -1 )
		{
			__int64	size = 0;
			long	date1= 0,
					date2= 0;
			long	type = -1; // default LOG TYPE is (NOT YET DETERMINED)

			// Now lets do the real hard work, and work out the type/size/dates etc.... hopefully not too time expensive.
			if ( newlogfile )
			{
#if DEF_WINDOWS
				size = WinGetLogFileType( newlogfile, &type, &date1, &date2 );
#else
				size = GetLogFileType( newlogfile, &type, &date1, &date2 );
#endif
			} else {
				type = 0;
			}

			// if the size is valid, and the type is known then it must be valid to be a log file... else ignore it.
			if ( size >= 0 && type != -1 )
			{
				LogFileHistory[logfilesTotal].size = size;
				LogFileHistory[logfilesTotal].file = (char*)malloc( len+6 );
				mystrcpy( LogFileHistory[logfilesTotal].file, newlogfile );
				LogFileHistory[logfilesTotal].type = (short)type;
				LogFileHistory[logfilesTotal].startDate = date1;
				LogFileHistory[logfilesTotal].numOfFieldDelimiters = prevLogDelim;
				LogFileHistory[logfilesTotal].logFormatIndex = prevLogFileFormatIndex;
				LogFileHistory[logfilesTotal].endDate = 0;
				LogFileHistory[logfilesTotal].endDateKnown = 0;		// crap not needed, unknown can = -1
				logfilesTotal++;
			}
		}
	}
	return logfilesTotal;
}


long CompSortLogs( void *p1, void *p2, long *result )
{
	long d1,d2;
	LogFileHistoryRec
			*l1 = (LogFileHistoryRec *)p1,
			*l2 = (LogFileHistoryRec *)p2;

	d1 = l1->startDate;
	d2 = l2->startDate;

	*result = d1-d2;
	return 0;
}

void SortLogHistoryFrom( long start, long total )
{
	LogFileHistoryRec *first;

	start = logfilesTotal-total;
	first = &LogFileHistory[start];
	FastQSort( first, total, sizeof(LogFileHistoryRec), CompSortLogs, TOP_SORT );
}
