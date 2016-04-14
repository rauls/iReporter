
#ifndef GLOBAL_PATHS_H
#define GLOBAL_PATHS_H

#define	MAXFILENAMESSIZE		(320)
#define	MAX_LOGFILES			(5000)
#define	MAXFILEDATASIZE			(MAX_LOGFILES*MAXFILENAMESSIZE)

extern char		gPath[MAXFILENAMESSIZE];					// global Application Path
extern char		gDataPath[MAXFILENAMESSIZE];				// global User data Path
extern char		gPersonalPath[MAXFILENAMESSIZE];			// global User data Path
extern char		gPrefsFilename[MAXFILENAMESSIZE];

extern char		glogFilename[MAXFILENAMESSIZE];				// extern char		glogFilename[256];
extern char		*glogFilenamesData;
extern char     *glogFilenames[MAX_LOGFILES];
extern long     glogFilenamesNum;

#if DEF_MAC
extern char		glogFilenamesStr[MAX_LOGFILES][MAXFILENAMESSIZE];			// mac version
#endif

#endif // GLOBAL_PATHS_H
