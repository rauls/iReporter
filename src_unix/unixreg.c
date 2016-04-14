/*
 * Registration code processing....
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "ice.h"
#include "reg.h"


#if DEF_FREEVERSION
	#define	UNIXREGFILE			"/etc/application45f.reg"
	#define	UNIXREGFILE2		"/usr/local/Analyzer/application45f.reg"
	#define	UNIXREGFILE3		"~/.application45f.reg"
	#define	UNIXREGFILE4		".application45f.reg"
#else
	#define	UNIXREGFILE			"/etc/application45.reg"
	#define	UNIXREGFILE2		"/usr/local/Analyzer/application45.reg"
	#define	UNIXREGFILE3		"~/.application45.reg"
	#define	UNIXREGFILE4		".application45.reg"
#endif






/*********************** GetUserInfo **********************************/
time_t	GetFileDate( char *file )
{
	int				ok;
	struct stat			mystat;

	ok = stat( file, &mystat );

	if ( !ok ){
		return mystat.st_ctime;
	} else return 0;

}



time_t	GetRegDate( void )
{
	int			ok;
	int			err = 0;
	struct stat			mystat;

	ok = stat( UNIXREGFILE, &mystat );
	if ( ok )
		ok = stat( UNIXREGFILE2, &mystat );
	if ( ok )
		ok = stat( UNIXREGFILE3, &mystat );
	if ( ok )
		ok = stat( UNIXREGFILE4, &mystat );
	
	
	if ( !ok ){
		/* printf( "%ld, %ld, %ld\n", mystat.st_atime, mystat.st_mtime,mystat.st_ctime );
		*/
		return mystat.st_ctime;
	} else
		return 0;
}


SerialInfoH	GetSerialInfoData( SerialInfoH Data, char *regStr )
{
	SerialInfoPtr	readDataP;
	int			ok;
	int			err = 0;
	unsigned long	len=SERIALINFO_SIZE;

	readDataP= *Data;

	ok = FileToMem( UNIXREGFILE, readDataP, len );
	if ( !ok )
		ok = FileToMem( UNIXREGFILE2, readDataP, len );
	if ( !ok )
		ok = FileToMem( UNIXREGFILE3, readDataP, len );
	if ( !ok )
		ok = FileToMem( UNIXREGFILE4, readDataP, len );
	
	
	if ( ok ){
		DecryptInfo( readDataP );

		return Data;
	} else
		return NULL;
}

int	SaveSerialData(SerialInfoH userInfo, char *name, char *regStr)
{
	long	ret;
	/**** ENCRYPT DATA BEFORE SAVING!!!!!! ****/

	if (userInfo)
		EncryptInfo(*userInfo);

	ret = MemToFile( UNIXREGFILE, *userInfo, SERIALINFO_SIZE );
	
	if (!ret)
		ret = MemToFile( UNIXREGFILE2, *userInfo, SERIALINFO_SIZE );
	if (!ret)
		ret = MemToFile( UNIXREGFILE3, *userInfo, SERIALINFO_SIZE );
	if (!ret)
		ret = MemToFile( UNIXREGFILE4, *userInfo, SERIALINFO_SIZE );

	if ( !ret ) 
		printf ( "Failed to save registration file %s\n", UNIXREGFILE );
	
	return ret;
}





void ShowRegMessage( void )
{

char	msg[]={
"If you haven't registered your iReporter product, please do so now. By registering, you can gain access to regular updates and technical support.\n\n\
You'll be eligible to receive product announcements, special offers, information on upcoming events.\n\n" };

	printf( msg );
}









/*
int		Unix_VerifySerialNumFmt( char * serialNumStr )
{
	
	int 				isOk = TRUE;	// Assume is ok, if not return FALSE //
	short				loop=0;
	int					done = FALSE;
			
	// Copy the format string 

	if ( serialNumStr )
	{
		UprString( serialNumStr, FALSE); 	// Change lowercase chars to upper //
		serialID= *(long*)(serialNumStr);
		while ((loop <= serialNumStr[0]) && (isOk))
		{
			loop++;	 // Increment counter on each pass
			switch (Unix_serialNumFormat[loop]){
				case kIsNumber:
					if (serialNumStr[loop] < kZero ||
							serialNumStr[loop] > kNine)
						isOk = FALSE; 
					break;
				case kIsLetter:
					if (serialNumStr[loop] < kBigA ||
							serialNumStr[loop] > kBigZ)
						isOk = FALSE; 
					break;
				case kDash:
					if (serialNumStr[loop] != kDash)
						isOk = FALSE;
					break;
			}
		}
	}

	// if the format is correct then lets extract the ID # from the pattern of numbers in the right sequence
	// and verify it mathamaticly to see if it is correct.
	if ( isOk ){
		// extract ID code 
		idcode = Unix_GetCustomerCode( (*SerialDataH)->serialNumber );
		// verify ID code using MOD 21 to see if its divisable by 21 wholey 
		if ( (idcode>0) && (idcode % REGCODE) == 0 )
			isOk = TRUE;
		else
			isOk = FALSE;
	}

	// return TRUE only if the serial code is the right format and correct ID formulae 
	return (isOk);		
}

*/
