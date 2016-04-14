/*
 * Registration code processing....
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "macemu.h"
#include "ice.h"
#include "reg.h"
#include "unixreg.h"
#include "demorecord.h"

#define	MYREG_APP_NAME		"iReporter"
#define	kZero	0x30
#define	kNine	0x39
#define	kBigA	0x41
#define	kBigZ	0x5A
#define	kDash	0x2D
#define	kIsNumber	'#'
#define	kIsLetter	'$'
#define	kMaxSize_SerialNumStr	30
#define	DATA_ENCRYPTED		1	/* 1 = recovering encrypted data, 0 = not encrypted */
#define	DEMOTIME		(60*60*24*14)			/* 14 days free trial */


extern	SerialInfoH	GetSerialInfo( void );
extern	int			gVerbose;
extern	time_t		gBinaryDate;

static	SerialInfo		SerialData;
static	SerialInfoPtr	SerialDataP = NULL;
static	SerialInfoH 	SerialDataH = NULL;
static	int		regcode_status = FALSE;
static	int		reg_returnstat = FALSE;

long	idcode;

/*
//							     012345678901234567890123456789012
//										  TTTTTT  TT
// ***NOTE***: the values of TTTTT are together one number TTTTTTTT that has to be divisable by /21
//				to be a valid serial id #
// Other codes				AAA-123A-123456-A123		ID=12345612
//						CAT-007A-000000-T213
*/

Str31	serialNumFormat		= "~$$$-###$-######-$###";

#pragma mark --------- read registry info


/*
 * construct the ID # from the string of mixed numberrs.. this may change to what ever....
 */
long GetCustomerCode( char *serialNumStr )
{
	long val=0;
	char *serstr;
	
	serstr = serialNumStr+1;

	val += ((serstr[9]  - 0x30) * 10000000L);
	val += ((serstr[10] - 0x30) * 1000000L);
	val += ((serstr[11] - 0x30) * 100000L);
	val += ((serstr[12] - 0x30) * 10000L);
	val += ((serstr[13] - 0x30) * 1000L);
	val += ((serstr[14] - 0x30) * 100L);
	val += ((serstr[17] - 0x30) * 10L);
	val += (serstr[18]  - 0x30);
	return val;
}






const char *my2ndkey = "shutdown";

void	DecryptInfo( SerialInfoPtr regDataPtr )
{
	register short		length,bytesleft,copylength;
	unsigned char in[16], out[16], *dest;
	ICE_KEY *key;
	
	length = sizeof(SerialInfo);
	bytesleft = length;
	dest = (unsigned char *)regDataPtr;
	key = ice_key_create( 0 );
	ice_key_set( key, (unsigned char *)my2ndkey );
	copylength = 8;
	while( bytesleft >0 ){
		if ( bytesleft < 8 ) copylength = bytesleft;
		memcpy( in, dest, copylength );			/* copy mem to temp buffer */
		ice_key_decrypt( key, in, out );			/* decrypt temp buffer */
		memcpy( dest, out, copylength );		/* copy temp buffer to mem */
		cycle_key( key );
		dest += copylength;
		bytesleft -= copylength;
	}
	ice_key_destroy ( key );

}
void	EncryptInfo( SerialInfoPtr regDataPtr )
{
	register short		length,bytesleft,copylength;
	unsigned char in[8], out[8], *dest;
	ICE_KEY *key;
	
	length = sizeof(SerialInfo);
	bytesleft = length;
	dest = (unsigned char *)regDataPtr;
	key = ice_key_create( 0 );
	ice_key_set( key, (unsigned char *)my2ndkey );
	copylength = 8;
	while( bytesleft >0 ){
		if ( bytesleft < 8 ) copylength = bytesleft;
		memcpy( in, dest, copylength );			/* copy mem to temp buffer */
		ice_key_encrypt( key, in, out );			/* encrypt temp buffer */
		memcpy( dest, out, copylength );			/* copy temp buffer mem */
		cycle_key( key );
		dest += copylength;
		bytesleft -= copylength;
	}
	ice_key_destroy ( key );

}






/****************************** VerifySerialNumFmt *****************************/ 

int		VerifySerialNumFmt( char * serialNumStr )
{
	/* Compare the serial number entered to a custom format string entered in
		a 'Form' resource.  Return TRUE only if format matches.  'Form' resource
		also contains the setting for whether to save the user info into installation
		files; so I'll grab it here to return back to Fetch_SerialInfo, which will
		return it to main().
	*/
	
	int				isOk = TRUE;	/* Assume is ok, if not return FALSE */
	short					loop=0;
	Str31				formatStr;
	int				done = FALSE;
			
	memcpy( formatStr, serialNumFormat, sizeof(Str31));
	/* Copy the format string */

	if (serialNumStr[0] != formatStr[0])
		isOk = FALSE;
	else{
		UprString( serialNumStr, FALSE); 	/* Change lowercase chars to upper */
		while ((loop <= serialNumStr[0]) && (isOk))
		{
			loop++;	 /* Increment counter on each pass */
			switch (formatStr[loop]){
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

	/* if the format is correct then lets extract the ID # from the pattern of numbers in the right sequence
	 * and verify it mathamaticly to see if it is correct.
     */
	if ( isOk ){
		/* extract ID code */
		idcode = GetCustomerCode( (*SerialDataH)->serialNumber );
		/* verify ID code using MOD 21 to see if its divisable by 21 wholey */
		if ( (idcode>0) && (idcode % 21) == 0 )
			isOk = TRUE;
		else
			isOk = FALSE;
	}

	/* return TRUE only if the serial code is the right format and correct ID formulae */
	return (isOk);		
}







/*
 * remove serial # from memory so it could not be found again...
 */
void		VoidSerialNum( char *serialNumStr )
{
	short lp;
	
	for( lp=0;lp<31;lp++){
		*(serialNumStr+lp) = (rand()*10)%10;
	}
}



	

/*
 * use these for external routines to read the username/org details....
 */
char *GetRegisteredUsername( void )
{
	return (char *)(*SerialDataH)->userName+1;
}
	
char *GetRegisteredOrganization( void )
{
	return (char *)(*SerialDataH)->organization+1;
}
	

#pragma mark ------> Serial Main Call

long AskForRegistration( SerialInfoH SerialDataH )
{
	SerialInfoPtr	SerialDataP = *SerialDataH;

	printf( "\nEnter name: " );
	fscanf( stdin, "%s", &SerialDataP->userName[1] );
	SerialDataP->userName[0] = strlen( SerialDataP->userName+1 );
	
	printf( "Enter company: " );
	fscanf( stdin, "%s", &SerialDataP->organization[1] );
	SerialDataP->organization[0] = strlen( SerialDataP->organization+1 );

	printf( "Enter serialNumber: " );
	fscanf( stdin, "%s", &SerialDataP->serialNumber[1] );
	SerialDataP->serialNumber[0] = strlen( SerialDataP->serialNumber+1 );

	SerialDataP->startTime = time(0);
	return TRUE;
}


long IsDemoReg( void )
{
	return FALSE;
}


/* use this function to test for serial.... 
  * if ( DoRegistrationProcedure() ) then continue; else Exit();
  * get USERNAME from (*SerialDataH)->userName
  *		COMPANY from (*SerialDataH)->organization
  * serial number is wiped as soon it is not needed so it could not be found in ram
  */
short DoRegistrationProcedure( void )
{
	char	outtxt[128];
	short	success = FALSE;
	short	tries = 1;
	time_t	regTime;
	
	return TRUE;
	
	serialNumFormat[0] = 20;
	
	SerialDataH = GetSerialInfo();			/* read serial from file */
	if ( SerialDataH == NULL ) {
		SerialDataP = &SerialData;
		SerialDataH = &SerialDataP;
	} else
		SerialDataP = *SerialDataH;
	
	/* if what is in memory is incorrect , lets ask for a reg# */
	if ( (VerifySerialNumFmt( (*SerialDataH)->serialNumber )) == FALSE ){
		success = FALSE;
		
		if ( strncmp( GetRegisteredUsername(), "DEMO",4 ) == 0 && (*SerialDataH)->startTime ){
			if ( (time(0) - (*SerialDataH)->startTime) < DEMOTIME ) {
				regTime = GetRegDate();		/* get date of the regfile */
				/* printf( "now=%ld, file=%ld\n", time(0), gBinaryDate ); */
				if ( time(0) - regTime < DEMOTIME )	/* double check to see if the reg file is too old */
					success = TRUE;
			}
			if ( !success ){
				printf( "Sorry , your time is up, please purchase Funnel Web at http://www.redflagsoftware.com/funnel_web/\n" );
				return success;
			}
		}
		
		if ( !success ){
			if ( gVerbose > 0)
				printf( "\n\niReporter is UNREGISTERED!!!!!!!!!!!\n\n\n\n" );
	 		if ( AskForRegistration( SerialDataH ) ){		/* if attempted to register lets check it */
				success = VerifySerialNumFmt( (*SerialDataH)->serialNumber);

				if ( strncmp( GetRegisteredUsername(), "DEMO",4 ) == 0 && (*SerialDataH)->startTime ){
					if ( (time(0) - (*SerialDataH)->startTime) < DEMOTIME )
						success = TRUE;
					demorecord( 1 );
				}

				/* if it is a real reg# then save it to app res */
				if ( success ){
					SaveToFile( SerialDataH );
					tries=0;
				} else {
					/* bad reg# ,  out put error and try again */
					if ( tries > 1 )
						sprintf( outtxt, "Serial Reg code <%s> is incorrect\nPlease re-enter the code and try again.", (*SerialDataH)->serialNumber+1 );
					else
						sprintf( outtxt, "Serial Reg code <%s> is incorrect\nProgram will exit now.", (*SerialDataH)->serialNumber+1 );

					if ( gVerbose > 0)
						printf( outtxt );
				}
			}
		} else
		if ( gVerbose > 0){
			printf( "Registered to User : %s\n", GetRegisteredUsername() );
			printf( "Registered to Company : %s\n", GetRegisteredOrganization() );
		}
	} else {
		success = TRUE;
		if ( gVerbose > 0){
			printf( "Registered to User : %s\n", GetRegisteredUsername() );
			printf( "Registered to Company : %s\n", GetRegisteredOrganization() );
		}
	}

	VoidSerialNum( (*SerialDataH)->serialNumber );	/* erase serial # from memory so it cant be found... */
	return success;
}
 
 

















