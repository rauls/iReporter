/*
##
##      ##########
##      ##
##      ##
##      #########
##				 ##
##				 ##
##				 ##
##      ##########
*/


#include "FWA.h"

// ***************************************************************
// If you are using TrueTime then just turn on this define (1)
// You want to do it in the release builds.
// ***************************************************************
#ifndef	DEF_DEBUG
#	define	USE_TRUE_TIME	0
#else
#	define	USE_TRUE_TIME	0
#endif

#if USE_TRUE_TIME
#include "C:/Program Files/Compuware/PCShared/nmtxapi.h"
#endif

#include <string>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include "datetime.h"

#ifdef DEF_MAC
	#include <string.h>
	#include <OpenTransport.h>
	#include <OpenTptInternet.h>
	//#include <profiler.h>
	//#include "main.h"			// for ShowProgressDetail()
	#include "MacStatus.h"
	#include "MacUtil.h"
	#include "progress.h"
	#include "MacDNR.h"
	//#include "macdns.h"
	//#include "server.h"
	#include "Processing.h"
#endif								


#include "myansi.h"
#include "engine_process.h"
#include "log_io.h"
#include "gd.h"
#include "config_struct.h"
#include "engine_drill.h"
#include "engine_notify.h"
#include "engine_proc.h"
#include "Report.h"
#include "ReportInterface.h"
#include "ResDefs.h"
#include "engine_dbio.h"
#include "editpath.h"
#include "asyncdnr.h"
#include "Utilities.h"
#include "OSInfo.h"
#include "Hash.h"					
#include "EngineStatus.h"			
#include "EngineRegion.h"			
#include "translate.h"				
#include "EngineRestoreStat.h"		
#include "StatList.h"				
#include "EngineCluster.h"			
#include "EngineParse.h"			
#include "EngineAddStats.h"			
#include "EngineClientDNR.h"		
#include "EngineMeanPath.h"			
#include "EngineVirtualDomain.h"	


#if DEF_WINDOWS		// WINDOWS include
	#include <windows.h>
	#include <sys/stat.h>
	#include "Windnr.h"
	#include "Winmain.h"
	#include "resource.h"
#endif				
#if DEF_UNIX		// UNIX include
	#include <sys/stat.h>
	#include "unistd.h"
	#include "main.h"
#endif

// --------- Engine Global Static Strings to speed up database string access -------------------- //

#include "Statdefs.h"




VDinfoP FireWall_MultiHostInit( VDinfoP VDptr ,HitDataRec *Line )
{
	int		src_in=0, dest_in=0;
	char	*newname;
	char	lowerNewName[256];
	VDinfoP	oldVDptr;

	if ( !CheckFilterMatch( Line->clientaddr, MyPrefStruct.filterdata.router, MyPrefStruct.filterdata.routerTot, 0 ) )
		dest_in = 1;
	if ( !CheckFilterMatch( Line->sourceaddr, MyPrefStruct.filterdata.router, MyPrefStruct.filterdata.routerTot, 0 ) )
		src_in = 1;

	if ( !src_in && dest_in ){
		newname = "Incoming_Traffic";
	} else
	if ( src_in && dest_in ){
		newname = "Internal_Traffic";
	} else

	if ( src_in && !dest_in ){
		newname = "Outgoing_Traffic";
	} else
		newname = "External_Traffic";

	mystrcpylower( lowerNewName, newname );

	oldVDptr = VDptr;
	VDptr = FindVirtualDomain( lowerNewName );
	if ( VDptr == 0 )
	{
		VDptr = InitVirtualDomain( VDnum+1, newname, logStyle ); /* initialize virt domains info/data */
		if ( VDptr )
		{
			VDnum++;
			oldVDptr->next = VDptr;
		} else {
			return NULL;
		}
	}
	return VDptr;
}


long AddFirewallProtocolStats( VDinfoP VDptr , __int64 bo, __int64 bi, long logDays, char *clients, long protocol, long errorFlag )
{
	StatList *byStat = NULL;

	switch( protocol )
	{
		case 1:
			if ( !VDptr->byHTTP ) {
				VDptr->byHTTP = byStat = new StatList(VDptr,kSubDomainIncr); byStat->useOtherNames = NAMEIS_COPY; 
			}
			break;
		case 2:
			if ( !VDptr->byHTTPS ) {
				VDptr->byHTTPS = byStat = new StatList(VDptr,kSubDomainIncr); byStat->useOtherNames = NAMEIS_COPY; 
			}
			break;
		case 3:
			if ( !VDptr->byMail ) {
				VDptr->byMail = byStat = new StatList(VDptr,kSubDomainIncr); byStat->useOtherNames = NAMEIS_COPY; 
			}
			break;
		case 4:
			if ( !VDptr->byFTP ) {
				VDptr->byFTP = byStat = new StatList(VDptr,kSubDomainIncr); byStat->useOtherNames = NAMEIS_COPY;
			}
			break;
		case 5:
			if ( !VDptr->byTelnet ) {
				VDptr->byTelnet = byStat = new StatList(VDptr,kSubDomainIncr); byStat->useOtherNames = NAMEIS_COPY; 
			}
			break;
		case 6:
			if ( !VDptr->byRealAudio ) {
				VDptr->byRealAudio = byStat = new StatList(VDptr,kSubDomainIncr); byStat->useOtherNames = NAMEIS_COPY; 
			}
			break;
		case 7:
			if ( !VDptr->byDNS ) {
				VDptr->byDNS = byStat = new StatList(VDptr,kSubDomainIncr); byStat->useOtherNames = NAMEIS_COPY; 
			}
			break;
		case 8:
			if ( !VDptr->byPOP3 ) {
				VDptr->byPOP3 = byStat = new StatList(VDptr,kSubDomainIncr); byStat->useOtherNames = NAMEIS_COPY; 
			}
			break;
		case 0:
			if ( !VDptr->byOthers ) {
				VDptr->byOthers = byStat = new StatList(VDptr,kSubDomainIncr); byStat->useOtherNames = NAMEIS_COPY; 
			}
			break;
	}

	if ( byStat )
		VDptr->byOthers->IncrStatsMore( bo, bi, clients, logDays, errorFlag );

	return 1;

}



