#include "FWA.h"
#include "EngineStatus.h"
#include "myansi.h"
#include "datetime.h"
#include "config.h"		// OutPut()
#include "ResDefs.h"	// for ReturnString(IDS_NOTENOUGH)
#include "EngineBuff.h"
#include "translate.h"
#include "Hash.h"
#include "DateFixFileName.h"


// changes.........
// 11/Dec/2001 : ~ AddLineToFile() handles date tokens for converting logs to w3c...


const char* GetErrorString( long code )
{
	long errorStringID(SERR_000); 

	switch( code )
	{
		case 201: 	errorStringID=SERR_201; break;
		case 202:	errorStringID=SERR_202; break;
		case 204:	errorStringID=SERR_204; break;
		case 206:	errorStringID=SERR_206; break;
		case 301:	errorStringID=SERR_301; break;
		case 302:	errorStringID=SERR_302; break;
		//------------------------------------
		case 331:	errorStringID=SERR_331; break; //ftp
		case 332:	errorStringID=SERR_332; break; //ftp
		case 350:	errorStringID=SERR_350; break; //ftp
		case 400:	errorStringID=SERR_400; break;
		case 401:	errorStringID=SERR_401; break;
		case 403:	errorStringID=SERR_403; break;
		case 404:	errorStringID=SERR_404; break;
		case 405:	errorStringID=SERR_405; break;
		case 406:	errorStringID=SERR_406; break;
		case 407:	errorStringID=SERR_407; break;
		case 408:	errorStringID=SERR_408; break;
		case 421:	errorStringID=SERR_421; break; //ftp
		case 425:	errorStringID=SERR_425; break; //ftp
		case 426:	errorStringID=SERR_426; break; //ftp
		case 450:	errorStringID=SERR_450; break; //ftp
		case 451:	errorStringID=SERR_451; break; //ftp
		case 452:	errorStringID=SERR_452; break; //ftp
		case 500:	errorStringID=SERR_500; break;
		case 501:	errorStringID=SERR_501; break;
		case 502:	errorStringID=SERR_502; break;
		case 503:	errorStringID=SERR_503; break;
		case 504:	errorStringID=SERR_504; break; //ftp
		case 530:	errorStringID=SERR_530; break; //ftp
		case 532:	errorStringID=SERR_532; break; //ftp
		case 550:	errorStringID=SERR_550; break; //ftp
		case 551:	errorStringID=SERR_551; break; //ftp
		case 552:	errorStringID=SERR_552; break; //ftp
		case 553:	errorStringID=SERR_553; break; //ftp
		default: break;
	}

	return TranslateID(errorStringID);  
};


// Myansi condidate
static const long sleeptype_secs[] = {
	0, (1*60), (5*60), (10*60), (15*60), (30*60),
	(60*60), (2*60*60),(3*60*60),(6*60*60),(12*60*60),(24*60*60),
	(7*24*60*60),(14*24*60*60),(21*24*60*60),(30*24*60*60) };


long GetSleeptypeSecs( long type )
{
	if ( type < 16 )	// JMC: Needs to be the same size as the array above! sizeof(sleeptype_secs)/sizeof(long)
		return sleeptype_secs[ type ];
	else
		return type;
}


// JMC added static
static char *protocolStr[] = {
	"1=http", "1=80/tcp",
	"2=http-https",
	"3=smtp",
	"4=ftp",
	"5=telnet",
	"6=realaudio",
	"7=dns",
	"8=110/tcp", 0
};

long ConvProtocol( char *prot )
{
	long i=0, j=0; char *p;

	while( p=protocolStr[i] ){
		if ( strstri( p+2,prot ) ){
			{
				j = myatoi( p );
				return j;
			}
		}
		i++;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
int		stopall;

void StopAll( long stopcode )
{
	stopall = stopcode;
}

int IsStopped( void )
{
	return stopall;
}


//////////////////////////////////////////////////////////////////////


// if filename is NULL and line is valid, keep adding to the last FP
// if filename is VALID and line is NULL, close last file and open new file
// if filename is "" and line is NULL, close last file
// if both valid, open/write/close specified filename.
void *AddLineToFile( char *filename, char *line )
{ 
	static FILE		*fp = NULL;
	
	{
		// was a filename passed?
		if ( filename )
		{
			// close last open file if a filename is passed.
			if ( fp )
			{
				fclose( fp );
				fp = NULL;
			}

			// only open a new file if filename is valid.
			if ( *filename )
			{
				char newname[512];
				DateFixFilename( filename, newname );
				if ( line )
					fp = fopen( newname, "a+" );
				else
					fp = fopen( newname, "w" );
			}
		}

		// if we have an open file, write to it
		if ( fp )
		{
			// if we have content to write, write it
			if ( line )
			{
				fwrite( line, mystrlen( line ), 1, fp );
				fflush( fp );

				// close filename, if both filename/content are supplied
				if ( filename )
				{
					fclose( fp );
					fp = NULL;
				}
			}
		}
	} 
	return fp;
}




int DebugLog( char *str, long err )
{ 

	char 		log_str[300];
	char		logname[]="debug_log.txt", dateStr[64];
	int			ferr = 0;
	struct	tm	tr;
	time_t		Days;
	
	if ( str ){
		GetCurrentDate( &tr );
		Date2Days( &tr, &Days );
		DaysDateToString( Days, dateStr, 0, '/', 1,0);
		sprintf( log_str, "%s - err=(%ld):    %s\n", dateStr,  err, str );

		AddLineToFile( logname, log_str );
	} 
	return 0;
}






extern short	logNumDelimInLine;
// JMC: This is really bad

void ShowBadLine( long lineNum )
{
	int i = 0, count=0;
	char *s, *d, c;

	if ( OutDebug(NULL)==0 )
		return;

	s = buff;
	d = buf2;

	while( i < logNumDelimInLine && count < 10000 )
	{
		c = *s++;
		if( c == 0 ){
			i++;
			c = ' ';
		}
		*d++ = c;
		count++;
	}
	*d++ = 0;
	OutDebugs( "Ignoring Line %d: Data = %s", lineNum, buf2 );
}

/////////////////////////////////////////////////////////////////////////////


/* MemoryError - print memory error message and terminate program */
void MemoryError(char *txt, long size)
{

	if ( IsStopped() )
		return;

#ifdef DEF_MAC
	if ( txt ) {
		char	mytxt[512];

		sprintf( mytxt, "%s, needed %ld", txt, size );
		
		UserMsg (mytxt);
	}
	//else							function only called with txt parameter now, so don't need else
	//	AlertUser (18, 0);
	
	StopAll(OUTOFRAM);
#else
	if ( txt ){
		char mytxt[512];
		sprintf( mytxt, "%s, needed %ld", txt, size );
		ErrorMsg( mytxt );
	} else
		ErrorMsg( ReturnString(IDS_NOTENOUGH));

	StopAll(OUTOFRAM);
#endif
}
