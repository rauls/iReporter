#include "FWA.h"

#include <cstdio>
#include <cstring>

#include "Stats.h"

/* All of the commented out includes and externs below appear to be redundant, and certainly
   aren't needed for a compile - should be deleted RHF 31/01/02 */
   
//#include "myansi.h"
//#include "datetime.h"
//#include "config.h"				// for extern of "MyPrefStruct"
//#include "config_struct.h"		// for extern of MAX_DOMAINS
//#include "VirtDomInfo.h"		// for VDinfoP.

#include "EngineStatus.h"
#include "engine_drill.h"
//#pragma Warning("Need to find a location for MemoryError()")
//extern "C" void MemoryError(char *txt, long size);

//extern	long			VDnum;
//extern	VDinfoP		VD[ MAX_DOMAINS+10 ];			/* 16/32 virtual domain pointers */

//#include "StatDefs.h"



/*-----------------------------------------------------------------------------------------
** Statistic class methods
*/
/* Statistic - constructor */
Statistic::Statistic(void)
{
	/* empty method */
}

/* ~Statistic - destructor */
Statistic::~Statistic()
{
	FreeData();
}

/* ~Statistic - destructor */
void Statistic::FreeData( void )
{
	DeleteName();
	if ( timeStat )		{ delete timeStat; timeStat = 0; }
	if ( sessionStat )	{ delete sessionStat; sessionStat = 0; }
}


/* IncrStats - increment statistics */
void Statistic::IncrStats( __int64 bytesout, __int64 bytesin )
{
	files++;
	bytes += bytesout;
	bytesIn += bytesin;
}

/* GetVisits - return statistics */

unsigned long Statistic::GetVisits(void)
{
	return visits;
}

long Statistic::GetDur(void)
{
	return visitTot;
}

void Statistic::SetVisits( long x )
{
	visits = x;
}

void Statistic::AddBytes( __int64 x )
{
	bytes += x;
}

void Statistic::AddBytesIn( __int64 x )
{
	bytesIn += x;
}

void Statistic::AddVisits( long x )
{
	visits += x;
}

void Statistic::AddTimeDur( long x )
{
	visitTot += x;
}

void Statistic::AddFiles( long x )
{
	files += x;
}

void Statistic::AddCounters( long x )
{
	counter += x;
}

void Statistic::AddCounters4( long x )
{
	counter4 += x;
}

/* GetFiles - return statistics */
unsigned long Statistic::GetFiles(void)
{
	return files;
}

/* GetBytes - return statistics */
__int64 Statistic::GetBytes(void)
{
	return bytes;
}

/* GetBytesIn - return statistics */

__int64 Statistic::GetBytesIn(void)
{
	return bytesIn;
}



long Statistic::GetFiles2( int day )
{
	if( timeStat ){
		int i;
		i = timeStat->timeStat2->GetFiles(day);	// return count
		if ( i < 0 ) i = 0;
		return i;
	} else
		return 0;
}
long Statistic::GetBytes2( int day )
{
	if( timeStat ){
		int i;
		i = timeStat->timeStat2->GetBytes(day);	// return count
		if ( i < 0 ) i = 0;
		return i;
	} else
		return 0;
}



/* GetErrors - return statistics errors */

unsigned long Statistic::GetErrors(void)
{
	return errors;
}

/* GetName - increment statistics

If length is valid, name is a true string
If length is ZERO, then name is a pointer clone to another string, do not delete
If length is -1, then the pointer is a long value representing the IP

 */
char * Statistic::GetName( void )
{
	if ( length < 0 ){
		static char ipstr[32];
		return IPnumToIPStr( (long)name, ipstr );
	} else
		return name;
}


char *Statistic::AllocateName( long len )
{
	name = new char[len+1];
	if (!name)
	{
		MemoryError( "AllocateName() cant find %d bytes;", len+1 );
		return NULL;
	}

	return name;
}

void Statistic::DeleteName()
{
	// Do not call funcs to get these coz we want to know them directly
	if( length>0 && name )
	{
		delete[] name;
	}

	name = NULL;
}


long Statistic::StoreName( char *newname )
{
	short	len = mystrlen(newname);

	if ( AllocateName( len ) == NULL )
		return 0;

	mystrcpy( name, newname );
	length = len;
	return len;
}

// change a valid name to a new name, if no name exists, dont make a new one
long Statistic::ChangeName( char *newname )
{
	if ( newname ){
		DeleteName();
		return StoreName( newname );
	} else
		return 0;
}


/* GetDay - Get the last accessed date from this record */
long Statistic::GetDay( void )
{
	return (lastday);
}












// ------------------------------------------------------------------------
//				MOST COMMON ERROR CODES
// ------------------------------------------------------------------------

unsigned long Statistic::GetMostCommonError( void )
{
	if ( timeStat )
	{
		if ( timeStat->timeStat2 )
		{
			timeStat->timeStat2->Sort( TIMESORT_FILES );
			return this->timeStat->GetDaysIdx2(0);
		}
	}
	return 0;
}


unsigned long Statistic::GetMostErrorcodeHits( long errorcode )
{
	if ( timeStat )
	{
		if ( timeStat->timeStat2 )
		{
			timeStat->timeStat2->Sort( TIMESORT_FILES );
			return this->timeStat->GetFilesIdx2(errorcode);
		}
	}
	return 0;
}














