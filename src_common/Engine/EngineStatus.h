#ifndef	ENGINESTATUS_H
#define	ENGINESTATUS_H

#include "FWA.h"

const char* GetErrorString( long errorCode );
long GetSleeptypeSecs( long type );
long ConvProtocol( char *prot );

#define	USERSTOPPED		1
#define	OUTOFRAM		2
#define	POSTCALCFAILURE	3
#define	CANTMAKEDOMAIN	4
#define	CANTOPENLOG		5
#define	ENGINEBUSY		6
#define OUTOFDISK		7		// disk is full, can't write files

// Needs to be [extern "C"] as it is used by postproc.c
extern "C"	void			StopAll( long stopcode );
extern "C"	int				IsStopped( void );
extern	int	stopall;
// JMC: This is a problem.


void *AddLineToFile( char *filename, char *line );
void ShowBadLine( long lineNum );
int DebugLog( char *str, long err );
// JMC #define	LOG(s,e) DebugLog( s, e )


#include "EngineBuff.h"

void MemoryError(char *txt, long size);

#endif
