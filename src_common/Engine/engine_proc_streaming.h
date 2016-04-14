#ifndef __ENGINE_PROC_STREAMING
#define __ENGINE_PROC_STREAMING

#include "HitData.h"


void LOG_WMedia(char *buffer);
void LOG_QTSS( char *buffer );

short PROC_WMedia(char *buffer,HitDataPtr Line );
short PROC_QTSS(char *buffer,HitDataPtr Line );
short PROC_Realserver(char *buffer,HitDataPtr Line);




#endif
