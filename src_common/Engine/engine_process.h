/*
**	File:		engine.h
**
**	Author:		Raul Sobon
**
**	Comments:	Class definitions for App program
*/

#ifndef	_ENGINE_H_
#define	_ENGINE_H_

#include "FWA.h"
#include <stdio.h>
#include <time.h>

#include "datetime.h"
#include "myansi.h"
#include "engine_drill.h"
#include "config.h"
#include "Stats.h"
#include "HitData.h"
#include "VirtDomInfo.h"

extern __int64		gFilesize;

#ifdef DEF_MAC
	#if PRAGMA_STRUCT_ALIGN
		#pragma options align=mac68k
	#endif
#endif

#include "StatDefs.h"

/////////////////////////////////////////////////////////////////////////////////////
//	CQDNSCache global methods.
/////////////////////////////////////////////////////////////////////////////////////
void ClearLookupCacheFile();

long GoProcessLogs( ProcessDataPtr logdata );

VDinfoP LoadV4Database( long file );

#ifdef DEF_MAC
	#if PRAGMA_STRUCT_ALIGN
		#pragma options align=reset
	#endif
#endif

#endif
