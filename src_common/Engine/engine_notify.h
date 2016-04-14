#ifndef	_ENGINENOTIFY_
#define	_ENGINENOTIFY_

#include "config.h"				// for long OutDebugs( const char *txt, ... );
#include "VirtDomInfo.h"		// for VDinfoP
#include "EngineAddStats.h"		// for AddClientDetailsToOthers()

extern char *LogicStr[];
extern char *StatListStr[];
extern char *StatFieldStr[];

#ifdef __cplusplus
extern "C" {
#endif

#include "OSInfo.h"

#define	NOTIFY_OUTFILENAME		"notify_msg.txt"

void FlushOldData( VDinfoP VDptr, long date, short keeptopx );
long PerformNotification( VDinfoP VDptr, long date, short keeptopx );
long CheckForNotifyRules( VDinfoP VDptr, long date, char notifyfules[MAX_FILTERUNITS][MAX_FILTERSIZE] );

#ifdef __cplusplus
}
#endif

#endif
