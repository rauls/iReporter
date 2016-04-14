/*
	File:		WinshellMain.c

	Contains:	Main event loop and basic keyboard/mouse processing or interface... whatever it maybe

	Written by:	Darren Williams / Raul Sobon

	Copyright:	  1996 by Active Concepts, all rights reserved.

	PLATFORMS:	*.*

	Change History (most recent first):
		08/Apr/1997	:	Windows port started....
		15/Aug/1997 :	Unix Port of engine working... will improve args opts
						then later do GUI that uses the engine binary.
	To Do:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


// Local Header Files
#include "MacEmu.h"
#include "UnixMain.h"
#include "util.h"



extern char		logFilename[256];
extern char		prefFilename[256];
extern	int		gVerbose;
extern double	gDate1,gDate2;
extern char		*glogFilenames[16];
extern char		glogFilenamesStr[16][256];
extern long		glogFilenamesNum = 0;


void StatusWindowSetText( char * txt )
{
	int err;
	if ( gVerbose )
		printf( "Status (err=%d): %s\n", err, txt );
}
























