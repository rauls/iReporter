
#include "FWA.h"

#include <stdio.h>
#include "GlobalPaths.h"

char	gPath[MAXFILENAMESSIZE];								// global Application Path
char	gDataPath[MAXFILENAMESSIZE];							// global User data Path
char	gPersonalPath[MAXFILENAMESSIZE];						// global User data Path
char	gPrefsFilename[MAXFILENAMESSIZE];

char	glogFilename[MAXFILENAMESSIZE];
char	*glogFilenamesData = NULL;		// Never ever change this, as it will crash
char	*glogFilenames[MAX_LOGFILES];
long	glogFilenamesNum;

#if DEF_MAC
char	glogFilenamesStr[MAX_LOGFILES][MAXFILENAMESSIZE];			// mac version
#endif
