#include "FWA.h"

#include "EngineVirtualDomain.h"
#include "Stats.h"
#include "Hash.h"

#include "datetime.h"		// for weekdays & others.
#include "EngineStatus.h"	// for errorStrings.

char *hourStrings[] = {
	"00","01","02","03","04","05","06","07","08","09","10","11",
	"12","13","14","15","16","17","18","19","20","21","22","23", "23", "23", "23", 0
};

char *regionStrings[] = {
	"Unknown", "Africa", "Antartica", "Asia", "North America", "South America", "Oceania", "Europe", "", 0
};

char *opersysStrings[] = {
	"Unknown", 
	"Windows 3.1", "Windows 95", "Windows 98", "Windows NT", "Windows 2000", 	// 1 .. 5
	"Mac OS", "Mac OS X", "WebTV",												// 6 .. 8
	"SunOS", "IRIX", "AIX", "HP-UX", "OSF", "FreeBSD", "Linux",					// 9 .. 15
	"Amiga", "OpenVMS", "OS/2", "Unix Console", "BeOS", "Windows XP", "Windows Me", // 16 ..22
	"Windows Unknown",
	0			
};

// if a stat has a missing name pointer, find out what it is via its hashs of the names
static void RestoreStatisticNames( Statistic* stat, char** strings )
{
	unsigned long hash;
	long dummyLen;

	while( *strings ){
		hash = HashStr( (char*)*strings, &dummyLen );
		if( hash == stat->hash ){
			stat->name = (char*)*strings;
			break;
		}
		strings++;
	}
}


void RestoreHourNames( Statistic *stat )
{
	RestoreStatisticNames( stat, hourStrings );
}


void RestoreWeekdaysNames( Statistic *stat )
{
	RestoreStatisticNames( stat, weekdays );
}


void RestoreRegionNames( Statistic *stat )
{
	RestoreStatisticNames( stat, regionStrings );
}


void RestoreCountryNames( Statistic *stat )
{
	RestoreStatisticNames( stat, regionStrings );
}


void RestoreOpersysNames( Statistic *stat )
{
	RestoreStatisticNames( stat, opersysStrings );
}


void RestoreErrorsNames( Statistic *stat )
{
	// stat->counter just happens to be the error code - see engine_process.cpp
	stat->name=(char*)GetErrorString( stat->counter );	

	DEF_POSTCONDITION( stat->name );		// should have matched ok
}


void RestoreStatisticName( Statistic* pToStat, StatList* pFromStatList )
{
	DEF_PRECONDITION(pToStat);
	DEF_PRECONDITION(pFromStatList);

	// search for source statistic
	Statistic* pFromStat=pFromStatList->FindHashStat(pToStat->GetHash());

	// if found then restore destination statistic's name
	if( pFromStat )
	{
		pToStat->name=pFromStat->GetName();
	}	
}


/*
// QCM:40345 --------------------------- Fix bad visitor count after DBLOAD for Brower/Opersys

Solution Design: 
1. before sorting browsers/os , remember original positions with in array.
    That is, for each position, store its HashID. So at hashids[7].hash, we can find the correct one later.

2. after sorting the browsers/os, go through all clients , and fix up the ids for each B/OS based upon a lookup table.
    So if client[4].counter2 is 5, we can go FindStat( hashids[5].hash ), get its real position, and put it back into the client[4].counter2

3. now all counter2/3s should match the proper browsers/os.

// --------------------------------------------------------------------------------------------

*/

void Restore_AllIDs( StatList* stat )
{
	for( size_t count=0; count<stat->GetStatListNum(); count++ )
	{
		Statistic *pItem = stat->GetStat(count);
		pItem->id = count;
	}
}


size_t FindStatPos_byID( StatList* stat, long id )
{
	for( size_t count=0; count<stat->GetStatListNum(); count++ )
	{
		Statistic *pItem = stat->GetStat(count);
		if ( pItem->id == id )
			return count;
	}
	return 0;
}


void FixClientBrowserOS_IDS( VDinfoP vd )
{
	DEF_PRECONDITION(vd);

	// for all clients ....
	for( size_t count=0; count<vd->byClient->GetStatListNum(); count++ )
	{
		Statistic *pClient = vd->byClient->GetStat( count );

		// fix clients referrence to browsers IDs to be correct and not the wrong ones
		pClient->counter  = FindStatPos_byID( vd->byUnrecognizedAgents, pClient->counter );
		pClient->counter2 = FindStatPos_byID( vd->byBrowser, pClient->counter2 );
		pClient->counter3 = FindStatPos_byID( vd->byOperSys, pClient->counter3 );
	}

	// Fix up the respective item->id field to match its position in the array
	Restore_AllIDs( vd->byUnrecognizedAgents );
	Restore_AllIDs( vd->byBrowser );
	Restore_AllIDs( vd->byOperSys );
}












