
#include "FWA.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ReportFuncs.h"
#include "translate.h"

#if DEF_WINDOWS 
	#include "httpinterface.h"
#endif
#if DEF_UNIX
	#include "httpinterface.h"
#endif

long outgoing_socket = 0;

long Fwrite( FILE *fp, const char *source )
{
	return fwrite( source, 1, strlen(source), (FILE*)fp );
}

long WriteToStorage( const char *source, void *fp )
{
	if ( (long)fp == 3 && outgoing_socket ){
#if DEF_MAC
		;
#else
		SendRemappedHtml( outgoing_socket, (char*)source, strlen(source) );
#endif
	} else {
		Fwrite( (FILE*)fp, source );
	}
	return 1;
}

int WriteLine( FILE *fp, const char *txt )
{
	if ( fp ) {
		WriteToStorage( txt, fp );
		return 1;
	}
	return 0;
}

void Fclose( FILE *fp )
{
	fclose( fp );
	fp = 0;
}
