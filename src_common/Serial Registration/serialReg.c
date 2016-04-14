#include "FWA.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "serialReg.h"
#include "ice.h"
#include "myansi.h"
#include "datetime.h"

// # remember, this source file is GENERIC for any OS and not windows specific, so wrap all
// # windows stuff in IFDEFs

#if DEF_WINDOWS
	#include "resource.h"
	#include "WinserialReg.h"
#endif				
#if DEF_UNIX
	#include "main.h"
	#include "unixreg.h"
#endif

#define	TopLeft( r )	(*(Point * ) &(r).top)

#define	kZero	0x30
#define	kNine	0x39
#define	kBigA	0x41
#define	kBigZ	0x5A
#define	kDash	0x2D
#define	kIsNumber	'#'
#define	kIsLetter	'$'

#define	kMaxSize_SerialNumStr	30

#define	SERIAL_STANDARD			0
#define	SERIAL_ENTERPRISE		1
#define	SERIAL_FIREWALL			2
#define	SERIAL_15DAYDEMO		3
#define	SERIAL_90DAYDEMO		4
#define	SERIAL_EXTENSION		100

/*
  Struct used for the user registration information.
	This is the format of the 'RVal' resource that is saved in
	installation files.
	Pascal strings.. PUKE PUKE

 typedef struct{
 	Str31		serialNumber;
 	Str255		userName;
 	Str255		organization;
 } SerialInfo, *SerialInfoPtr, **SerialInfoH;

================= EXTENSION SERIALS ===========================================
This extension brings up a dialog to prompt for the registration information.

It compares the serial number to a preset format, entered by the developer
into a 'Form' resource (template provided for editing) and aborts installation
if the format requirement is not met.

Example format string: EXT-####-####-##
# = Number, $ = Letter, - = Dash

  
Serial for 1/2001 = EXT-3131-7586-29
Serial for 2/2001 = EXT-1800-6261-98
Serial for 3/2001 = EXT-0604-2439-75
Serial for 4/2001 = EXT-3428-9125-83
Serial for 5/2001 = EXT-2201-3803-10
Serial for 6/2001 = EXT-1389-3544-69
Serial for 7/2001 = EXT-0496-5473-16
Serial for 8/2001 = EXT-2261-0441-78
Serial for 9/2001 = EXT-3381-8961-31
Serial for 10/2001 = EXT-3169-7229-43
Serial for 11/2001 = EXT-4089-0066-38
Serial for 12/2001 = EXT-0574-6816-29
================= EXTENSION SERIALS ===========================================


================= ENTERPRISE SERIAL #s
Serial for 1 , 0 clusters = FWP-1404-7010-15
Serial for 2 , 0 clusters = FWP-0869-1208-25
Serial for 3 , 0 clusters = FWP-4020-2489-74
Serial for 4 , 0 clusters = FWP-4079-1490-29
Serial for 5 , 0 clusters = FWP-0793-7242-42
Serial for 6 , 0 clusters = FWP-1329-1333-72
Serial for 7 , 0 clusters = FWP-2471-8493-55
Serial for 8 , 0 clusters = FWP-1983-7122-34
Serial for 9 , 0 clusters = FWP-2857-4068-13
Serial for 10 , 0 clusters = FWP-1904-7337-25
Serial for 11 , 0 clusters = FWP-2917-8236-58
Serial for 12 , 0 clusters = FWP-3449-6693-16
Serial for 13 , 0 clusters = FWP-0300-9649-79
Serial for 14 , 0 clusters = FWP-0225-6164-08
Serial for 15 , 0 clusters = FWP-3508-4842-71
Serial for 16 , 0 clusters = FWP-2976-4184-97
Serial for 17 , 0 clusters = FWP-1829-0484-38
Serial for 18 , 0 clusters = FWP-2288-8750-31
Serial for 19 , 0 clusters = FWP-1409-3467-20
Serial for 20 , 0 clusters = FWP-2861-5819-62
Serial for 21 , 0 clusters = FWP-1996-3484-77


Serial for 283 , 7 clusters = FWP-2428-6800-457
Serial for 284 , 7 clusters = FWP-2353-0232-387
Serial for 285 , 7 clusters = FWP-1345-1805-937
Serial for 286 , 7 clusters = FWP-0811-2296-637
Serial for 287 , 7 clusters = FWP-3963-0918-167
Serial for 288 , 7 clusters = FWP-0157-2610-657
Serial for 289 , 7 clusters = FWP-3576-8257-907
Serial for 290 , 7 clusters = FWP-3115-8688-147



//Here is a perl script to make the above
#!/usr/bin/perl

sub MakeSerials{

#printf( "$ARGV[0]\n" );
	$clusters = 0;
	$t = $ARGV[0];
	if ( $t > 1 ){
		$clusters = $ARGV[0];
	}
	$id = 1;

	for( $id = 1; $id < 1000; $id++ ){
		$var = "WIN4E:$id:$clusters";
		$temp = `echo -n $var | cksum`;
		$size = index( $temp, " " );
		if ( $size <= 0 ) { $size = 10 };
		$temp2 = substr( $temp, 0, $size );
		$ser = sprintf( "%010u", $temp2 );
		$part1 = substr( $ser, 0, 4 );
		$part2 = substr( $ser, 4, 4 );
		$part3 = substr( $ser, 8, 4 );
		if ( $clusters>0 ){
			$part3 = "$part3$clusters";
		}
		printf( "Serial for $id , $clusters clusters = FWP-$part1-$part2-$part3\n");
    }
}
MAIN:
{
	MakeSerials();
}








================ STANDARD SERIAL #s
Serial for 1 , 0 clusters = FWS-3205-2296-80
Serial for 2 , 0 clusters = FWS-3749-4697-26
Serial for 3 , 0 clusters = FWS-0051-4961-05
Serial for 4 , 0 clusters = FWS-0529-7425-30
Serial for 5 , 0 clusters = FWS-3288-2443-41
Serial for 6 , 0 clusters = FWS-2743-6692-75
Serial for 7 , 0 clusters = FWS-2145-6836-28
Serial for 8 , 0 clusters = FWS-2592-9274-37



================= DEMO SERIALS ===========================================
Each serial # is dated for the START date of the demo, which then will
last 15 days from the start. (or 90 for the free version)

Example format string: TRY-####-####-##
# = Number, $ = Letter, - = Dash

This is calculated from a cheksum of the string "try:dd/mm/yyyy" of the start date/submit date

  echo -n try:5/12/2001 | cksum

  will expire on the 20/12/2001

================= DEMO SERIALS ===========================================



*/

extern int			gNoGUI;


SerialInfo		SerialData;
SerialInfoPtr	SerialDataP = NULL;
SerialInfoH 	SerialDataH = NULL;

long	idcode, serialID, customer_id = 0;

char	serialNumFormat[32]		= "$$$-####-####-####";
//
void	DecryptInfo( SerialInfoPtr regDataPtr );
void	EncryptInfo( SerialInfoPtr regDataPtr );




/* convert a string to a magic number */
static unsigned long crctab[] = {
  0x0,
  0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
  0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6,
  0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
  0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
  0x5BD4B01B, 0x569796C2, 0x52568B75, 0x6A1936C8, 0x6ED82B7F,
  0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A,
  0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
  0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58,
  0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033,
  0xA4AD16EA, 0xA06C0B5D, 0xD4326D90, 0xD0F37027, 0xDDB056FE,
  0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4,
  0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077, 0x30476DC0,
  0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5,
  0x2AC12072, 0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
  0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA, 0x7897AB07,
  0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C,
  0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1,
  0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
  0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B,
  0xBB60ADFC, 0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698,
  0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D,
  0x94EA7B2A, 0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
  0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 0xC6BCF05F,
  0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34,
  0xDC3ABDED, 0xD8FBA05A, 0x690CE0EE, 0x6DCDFD59, 0x608EDB80,
  0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
  0x58C1663D, 0x558240E4, 0x51435D53, 0x251D3B9E, 0x21DC2629,
  0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C,
  0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E,
  0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65,
  0xEBA91BBC, 0xEF68060B, 0xD727BBB6, 0xD3E6A601, 0xDEA580D8,
  0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2,
  0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 0x9B3660C6, 0x9FF77D71,
  0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74,
  0x857130C3, 0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
  0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C, 0x7B827D21,
  0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A,
  0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E, 0x18197087,
  0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
  0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D,
  0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE,
  0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB,
  0xDBEE767C, 0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
  0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 0x89B8FD09,
  0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
  0x933EB0BB, 0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF,
  0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
  0,0,0,0,0,0
};

/* convert a string to a magic number */
unsigned long GetHash(char *string, unsigned short len)
{          
	register unsigned long crc=0;
	register unsigned short count;
	register unsigned char b0,c0;
	register char *p;

	p=string;
	for (count=0;(count<len && *p);count++) {
		c0 = *p++;
		b0 = (unsigned char)(crc >> 24);
		//answer = answer << 8;
		//answer |= b0;
		crc = (crc<<8) ^ (crctab[(b0^c0)&0xff]);
	}
	len = count;
	while (len > 0)
	{
		crc = (crc << 8) ^ crctab[((crc >> 24) ^ len) & 0xFF];
		len >>= 8;
	}
	crc = ~crc & 0xFFFFFFFF;

	return(crc);
}


const char *my2ndkey = "shutdown";



void	EncryptInfo( SerialInfoPtr regDataPtr )
{
	register short		length,bytesleft,copylength;
	unsigned char in[8], out[8], *dest;
	ICE_KEY *key;
	
	length = sizeof(SerialInfo);
	bytesleft = length;
	dest = (unsigned char *)regDataPtr;
	key = ice_key_create( 0 );
	if ( !key )
	{
		regDataPtr = 0;
		return;
	}
	ice_key_set( key, (unsigned char *)my2ndkey );
	copylength = 8;
	while( bytesleft >0 ){
		if ( bytesleft < 8 ) copylength = bytesleft;
		memcpy( in, dest, copylength );			// copy mem to temp buffer
		ice_key_encrypt( key, in, out );			// encrypt temp buffer
		memcpy( dest, out, copylength );			// copy temp buffer mem
		cycle_key( key );
		dest += copylength;
		bytesleft -= copylength;
	}
	ice_key_destroy ( key );

}


void	DecryptInfo( SerialInfoPtr regDataPtr )
{
	register short		length,bytesleft,copylength;
	unsigned char in[16], out[16], *dest;
	ICE_KEY *key;
	
	length = sizeof(SerialInfo);
	bytesleft = length;
	dest = (unsigned char *)regDataPtr;
	key = ice_key_create( 0 );
	if ( !key )
	{
		regDataPtr = 0;
		return;
	}
	ice_key_set( key, (unsigned char *)my2ndkey );
	copylength = 8;
	while( bytesleft >0 ){
		if ( bytesleft < 8 ) copylength = bytesleft;
		memcpy( in, dest, copylength );			// copy mem to temp buffer
		ice_key_decrypt( key, in, out );			// decrypt temp buffer
		memcpy( dest, out, copylength );		// copy temp buffer to mem
		cycle_key( key );
		dest += copylength;
		bytesleft -= copylength;
	}
	ice_key_destroy ( key );

}














// ============================================ VERIFY SERIAL NUMBERS ======================================


#if (DEF_FREEVERSION && DEF_FREEVERSIONDYNAMIC)
	#define	DEFAULT_FREETIME		(90*ONEDAY)
#else
	#define	DEFAULT_FREETIME		(15*ONEDAY)
#endif


static int	gEndtime = 0,
			gFreeTime = DEFAULT_FREETIME;

int CheckDemoTimeout( long idcode, long demotime )
{
	// This will make APP Expire on a given month within the next 12 month period.
	{
		long hash, temphash;
		struct tm date;
		char t[256];
		long ctime, days = 0;


		// Format for CKSUM of text being "try:3/12/2001"
		// Make the serial by running 'echo -n try:3/12/2001 | cksum" and using those digits in "TRY-XXXX-XXXX-XX"
		// or calling HashIt(string)
		ctime = GetCurrentCTime();
		ctime -= demotime;

		// search for which day this KEY was made on.
		while( days < 3000 )
		{
			DaysDateToStruct( ctime, &date );
			sprintf( t, "try:%d/%d/%04d", date.tm_mday+1, date.tm_mon, date.tm_year );

			temphash = GetHash( t, -1 );
			hash = idcode - temphash;		// if the diff between the calculated hash and the supplied hash is 0, then its GOOD
			if ( !hash ) {
				ctime += demotime;
				DaysDateToStruct( ctime, &date );
				Date2Days( &date, &gEndtime );
				return 1;
			}
			days++;
			ctime +=  ONEDAY;
		}
	}
	return 0;
}




#if DEF_UNIX
char	Unix_serialNumFormat[]		= "$$$-###$-######-$###\0";


#ifdef	_PRO
#define	REGCODE				73			//73
#define	MACREGCODE			977
#else
#define	REGCODE				79			//79
#define	MACREGCODE			401
#endif

/*
#if defined (__FreeBSD__)
#define REGCODE 2111
#else
#define	REGCODE	2099
#endif
*/


/*
 * construct the ID # from the string of mixed numberrs.. this may change to what ever....
 */
long Unix_GetCustomerCode( char *serialNumStr )
{
	long val=0;
	char *serstr;
	
	serstr = serialNumStr;

	val += ((serstr[5]  - 0x30) * 1000000000L);
	val += ((serstr[6]  - 0x30) * 100000000L);

	val += ((serstr[9]  - 0x30) * 10000000L);
	val += ((serstr[10] - 0x30) * 1000000L);
	val += ((serstr[11] - 0x30) * 100000L);
	val += ((serstr[12] - 0x30) * 10000L);
	val += ((serstr[13] - 0x30) * 1000L);
	val += ((serstr[14] - 0x30) * 100L);

	val += ((serstr[17] - 0x30) * 10L);
	val += ((serstr[18] - 0x30));
	return val;
}



int		Unix_VerifySerialNumFmt( char * serialNumStr, long regcode )
{
	/* Compare the serial number entered to a custom format string entered in
		a 'Form' resource.  Return TRUE only if format matches.  'Form' resource
		also contains the setting for whether to save the user info into installation
		files; so I'll grab it here to return back to Fetch_SerialInfo, which will
		return it to main().
	*/
	
	int 				isOk = TRUE;	/* Assume is ok, if not return FALSE */
	short				loop=0;
	int					done = FALSE;
			
	/* Copy the format string */

	if( serialNumStr )
	{
		UprString( serialNumStr, FALSE); 	/* Change lowercase chars to upper */
		serialID = ReadLong(serialNumStr);
		while( Unix_serialNumFormat[loop] && serialNumStr[loop] && (isOk) )
		{
			//printf( "checking char %d '%c' == '%c' \n", loop, serialNumStr[loop], Unix_serialNumFormat[loop] );
			loop++;	 /* Increment counter on each pass */
			switch( Unix_serialNumFormat[loop] )
			{
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
	if ( isOk )
	{
		/* extract ID code */
		idcode = Unix_GetCustomerCode( serialNumStr );

		if ( regcode == MACREGCODE )
			idcode /= 1998;

		//printf( "user id = %d\n", idcode );
		/* verify ID code using MOD 21 to see if its divisable by 21 wholey */
		if ( (idcode>0) && (idcode % regcode) == 0 )
			isOk = TRUE;
		else
			isOk = FALSE;
	}

	/* return TRUE only if the serial code is the right format and correct ID formulae */
	return (isOk);		
}

#endif

int		VerifySerialNumFmt( char *serialNumStr, int type )
{
	/* Compare the serial number entered to a custom format string entered in
		a 'Form' resource.  Return TRUE only if format matches.  'Form' resource
		also contains the setting for whether to save the user info into installation
		files; so I'll grab it here to return back to Fetch_SerialInfo, which will
		return it to main().
	*/
	
	int				isOk = 1;	// Assume is ok, if not return FALSE
	int				done = 0;
	unsigned long				value2 = 255;		// max clusters
	unsigned long				loop=0,temp2, temp3;

	if ( serialNumStr )
	{
		long value = 0, digits = 0;

		if ( *serialNumStr == '_' )
			serialNumStr++;

		idcode = 1;
		serialID = ReadLong(serialNumStr);

		UprString( serialNumStr, 0); // Change lowercase chars to upper
		loop = 0;

		while ( loop<32 && serialNumStr[loop] && (isOk))
		{
			switch (serialNumFormat[loop]){
				case kIsNumber:
					if (serialNumStr[loop] < kZero || serialNumStr[loop] > kNine)
						isOk = 0; 
					else {
						if ( digits < 10 )
							value = (value*10) + (serialNumStr[loop]-'0');
						else
							value2 = (value2*10) + (serialNumStr[loop]-'0');
						digits++;
					}
					break;
				case kIsLetter:
					if (serialNumStr[loop] < kBigA || serialNumStr[loop] > kBigZ)
						isOk = 0; 
					break;
				case kDash:
					if (serialNumStr[loop] != kDash)
						isOk = 0;
					break;
			}
			if ( isOk ) idcode = value;
 			loop++; // Increment counter on each pass
		}
	}



	// This will make APP Expire on a given month within the next 12 month period.
	if ( type == SERIAL_EXTENSION )
	{
		long hash;
		struct tm date;
		char t[256];
		long yy,mm;


		// Format for CKSUM of text being "extend:12/2001"  ie YYYY/MM
		// Make the serial by running 'echo -n exterd:12/2001 | cksum" and using those digits in "EXT-XXXX-XXXX-XX"
		// or calling HashIt(string)
		for( yy=2001; yy<2003; yy++){
			for( mm=1; mm<=12; mm++ ){
				GetCurrentDate( &date );
				sprintf( t, "extend:%d/%04d", mm, yy );
				temp2 = GetHash( t, -1 );
				hash = idcode - temp2;		// if the diff between the calculated hash and the supplied hash is 0, then its GOOD
				if ( !hash ) {
					date.tm_mon = mm-1;
					date.tm_mday = 1;
					date.tm_year = yy;

					Date2Days( &date, &gEndtime );
					return 1;
				}
			}
		}
	}


	// if the format is correct then lets extract the ID # from the pattern of numbers in the right sequence
	// and verify it mathamaticly to see if it is correct.
	if ( isOk )
	{
		// This will make APP Expire on a given month within the next 12 month period.
		if ( type == SERIAL_15DAYDEMO )
		{
			if ( CheckDemoTimeout( idcode, (15*ONEDAY) ) )
			{
				(*SerialDataH)->clusterTotal = 10;
				return 1;
			}
		} else
		// check for standard edition where its demo is always 90 and re-registrable
		if ( type == SERIAL_90DAYDEMO )
		{
			if ( CheckDemoTimeout( idcode, (90*ONEDAY) ) )
			{
				(*SerialDataH)->clusterTotal = 10;
				return 1;
			}
		} else
		if ( type <= SERIAL_FIREWALL )
		{
			// Now check for real truly valid serial #s.
			for( loop=0;loop<10000;loop++ )
			{
				char t[256];
				char *serialTypes[] = { "WIN4S", "WIN4E", "WIN4F" };
				//char *serialTypes[] = { "MAC4S", "MAC4E", "MAC4F" };
				long hash, len;

				// first check the remembered ID# to see if its valid instantly, else search
				// all ID#s to see which one is valid.
				if ( loop == 0 )
					temp3 = customer_id;
				else
					temp3 = loop;

				len = sprintf( t, "%s:%d:%d", serialTypes[type], temp3, value2 );
				temp2 = GetHash( t, -1 );
				hash = idcode - temp2;		// if the diff between the calculated hash and the supplied hash is 0, then its GOOD

				if ( !hash ) {
					gFreeTime = (ONEDAY*365) * 25;
					customer_id = temp3;
					(*SerialDataH)->customer_id = temp3;
					(*SerialDataH)->clusterTotal = value2;
					return idcode;
				}
			}
		}
	}

	// return TRUE only if the serial code is the right format and correct ID formulae
	return (0);
}


// remove serial # from memory so it could not be found again...
void		VoidSerialNum( char *serialNumStr )
{
	short lp;
	
	for( lp=0;lp<31;lp++){
		*(serialNumStr+lp) = (rand()*10)%10;
	}
}

int		VerifySerialRelease( char * serialNumStr )
{
	int rc = 0;

	if ( serialNumStr )
	{
		//FIREWALL
		//VerifySerialNumFmt( serialNumStr, SERIAL_FIREWALL );
#ifdef	DEF_FULLVERSION
		rc = VerifySerialNumFmt( serialNumStr, SERIAL_ENTERPRISE );
#else
		// Old version 4.x standard
		rc = VerifySerialNumFmt( serialNumStr, SERIAL_STANDARD );
#endif

#ifdef DEF_UNIX
		if( !rc )
			rc = Unix_VerifySerialNumFmt( serialNumStr, REGCODE );

#ifdef __MACOSX__
		if( !rc )
			rc = Unix_VerifySerialNumFmt( serialNumStr, MACREGCODE );
#endif
#endif

	}
	return rc;
}

int		VerifySerialDemo( char * serialNumStr )
{
	int rc = 0;

	if ( !strcmpd( "DEMO", serialNumStr ) )
		return 1;
	else
	if ( !strcmpd( "EXT-", serialNumStr ) )
	{
		rc = VerifySerialNumFmt( serialNumStr, SERIAL_EXTENSION );
	} else
	if ( !strcmpd( "TRY-", serialNumStr ) )
	{
#ifdef	DEF_FULLVERSION
		rc = VerifySerialNumFmt( serialNumStr, SERIAL_15DAYDEMO );
#else
		rc = VerifySerialNumFmt( serialNumStr, SERIAL_90DAYDEMO );
#endif
	}

	return rc;
}







// ======================= SERIAL IO routines

SerialInfoH	GetDemoInfo( SerialInfoH Data )
{
	SerialInfoH ret;
	ret = GetSerialInfoData( Data, APPLICATION_IRDEMOREG );
	if ( ret == NULL )
		ret = GetSerialInfoData( Data, APPLICATION_DEMOREG );
	return ret;
}



// for PRO, read PRO serial, if failed try the normal SERIAL location
SerialInfoH	GetSerialInfo( SerialInfoH Data )
{
	SerialInfoH ret;
#ifdef DEF_FULLVERSION
	// First look in the new registry settings
	ret = (SerialInfoH)GetSerialInfoData( Data, APPLICATION_IRPROREG );
	if ( ret == NULL )
		// Secondly, look in the old registry settings
		ret = (SerialInfoH)GetSerialInfoData( Data, APPLICATION_PROREG );
	if ( ret == NULL )
		ret = (SerialInfoH)GetSerialInfoData( Data, APPLICATION_IRREG );
	if ( ret == NULL )
		ret = (SerialInfoH)GetSerialInfoData( Data, APPLICATION_REG );
	return ret;
#else
	// First look in the new registry settings
	ret = (SerialInfoH)GetSerialInfoData( Data, APPLICATION_IRREG );
	if ( ret == NULL )
		// Secondly, look in the old registry settings
		ret = (SerialInfoH)GetSerialInfoData( Data, APPLICATION_REG );
	return ret;
#endif
}



// 7Aug2000, save serial in pro area for pro else normal area.
// the new reader will get it right.
int	SaveSerialInfo( SerialInfoH userInfo, char *name )
{
#ifdef DEF_FULLVERSION
	return SaveSerialData( userInfo, name, APPLICATION_IRPROREG );
#else
	return SaveSerialData( userInfo, name, APPLICATION_IRREG );
#endif
}


int	SaveDemoInfo( SerialInfoH userInfo, char *name )
{
	return SaveSerialData( userInfo, name, APPLICATION_IRDEMOREG );
}











// ======================= SERIAL IO routines

	
long GetRegisteredCustomerID( void )
{
	return customer_id;
}

	
char *GetRegisteredOrganization( void )
{
	DoRegistrationProcedure();
	if ( SerialDataH ){
		if ( *SerialDataH )
			return (char *)(*SerialDataH)->organization+1;
	}
	return NULL;
}
	


//use these for external routines to read the username/org details....
char *GetRegisteredUsername( void )
{
	DoRegistrationProcedure();
	if ( SerialDataH ){
		if ( *SerialDataH )
			return (char *)(*SerialDataH)->userName+1;
	}
	return NULL;
}

long GetClusterValue( void )
{
#ifdef DEF_DEBUG
	return 255;
#endif
	if ( IsDemoReg() )
		return 10;
	else
	if ( SerialDataH ){
		if ( *SerialDataH )
			return (*SerialDataH)->clusterTotal;
	}
	return 0;
}


// let us know if we are running under a demo serial
short IsDemoReg( void )
{
	if ( SerialDataH )
		return VerifySerialDemo( (*SerialDataH)->serialNumber+1);
	else
		return 0;
}

// let us know if we are running under a demo serial
short IsFullReg( void )
{
	return 1;

	if ( SerialDataH )
		return VerifySerialRelease( (*SerialDataH)->serialNumber+1);
	else
		return 0;
}

// return days left in a demo
double GetDaysRunning( void )
{
	double runTime = 0;

	if ( SerialDataH && (*SerialDataH)->startTime )
		runTime = time(0) - (*SerialDataH)->startTime;

	return runTime;
}

static	char fixedExpireDate[] = "07/31/2002";	// mm/dd/yyyy - Note will stop working at the start of this day.

time_t StringToCTime( const char *date )
{
	struct tm		Date;
	time_t			Days;

	StringToDaysDate( date, &Date, 0 );
	Date2Days( &Date, &Days );
	return Days;
}

// return days left in a demo
double GetDaysLeft( void )
{
	double daysLeft, runTime = 0;

	if ( SerialDataH && (*SerialDataH)->startTime )
		runTime = time(0) - (*SerialDataH)->startTime;

	if ( gEndtime )
		// expiry time is specified
		daysLeft = gEndtime - time(0);
	else

#if (DEF_FREEVERSION) && (!DEF_FREEVERSIONDYNAMIC)
		daysLeft = StringToCTime( fixedExpireDate ) - time(0);
#else
		// else its relative to start time.
		daysLeft = gFreeTime - runTime;
#endif

	daysLeft /= (double)(ONEDAY);

	return daysLeft;
}

time_t GetExpireDate( char *dateOut )
{
	time_t end;
#if (DEF_FREEVERSION) && (!DEF_FREEVERSIONDYNAMIC)
	strcpy( dateOut, fixedExpireDate );
	end = StringToCTime( fixedExpireDate );
#else
	end = (time_t) ( GetCurrentCTime() + (ONEDAY*GetDaysLeft()) );
	DateTimeToString( end, dateOut );
#endif
	return end;
}




// let us know if the demo has timed out/expired
short IsDemoTimedout( void )
{
	if ( IsFullReg() )
		return 0;
	else
	if ( GetDaysLeft()<=0 )
		return 1;
	else
		return 0;
}



//return TRUE if hit OK else FALSE if hit CANCEL
short AskForRegistration(void)
{
	if( !gNoGUI && SerialDataP )
		return Fetch_SerialInfo( SerialDataP );
	else
		return 0;
} 




/* use this function to test for serial.... 
  * if ( DoRegistrationProcedure() ) then continue; else Exit();
  * get USERNAME from (*SerialDataH)->userName
  *		COMPANY from (*SerialDataH)->organization
  * serial number is wiped as soon it is not needed so it could not be found in ram
  */
short CheckRegistration( short askagain )
{
	short	success = 0, demo = 0, demostart, timeout = 0, tries = 3;
static int runstatus = 0;	

	if ( runstatus && !askagain )
		return 0;

	runstatus = TRUE;

	SerialDataP = &SerialData;
	SerialDataH = &SerialDataP;
	SerialDataH = GetSerialInfo( SerialDataH );			// read serial from file

	if ( SerialDataH == NULL ) {
		SerialDataP = &SerialData;
		SerialDataH = &SerialDataP;
		(*SerialDataH)->startTime = time(0);
	} else
		SerialDataP = *SerialDataH;
	success = VerifySerialRelease( (*SerialDataH)->serialNumber+1 );
	if ( success == 0 ){
		void *pt;
		VoidSerialNum( (*SerialDataH)->serialNumber );
		if ( pt = GetDemoInfo( SerialDataH ) ){
			SerialDataH = pt;
			demo = VerifySerialDemo( (*SerialDataH)->serialNumber+1 );
		}
	}

	demostart = demo;

	if ( demo ){
		if ( GetDaysLeft()<=0 ){
			if( !gNoGUI && !askagain )
			{
				ShowMsg_TrialOver();
			}
			timeout = 1;
			demostart = 0;
		} 
	}


	//VerifySerialDemo
	// if what is in memory is incorrect , lets ask for a reg#
	if ( (!success && !demo) || timeout || askagain )
	{
		// ask X times
		while( tries > 0 )
		{
			short reg_status;
			reg_status = AskForRegistration();		// only comes back if hit OK/CANCEL

			if ( reg_status == TRUE )
			{		// if attempted to register lets check it
				SleepSecs(1);
				success = VerifySerialRelease( (*SerialDataH)->serialNumber+1);
				if ( !success )
					demo = VerifySerialDemo( (*SerialDataH)->serialNumber+1);
				else 
					demo = 0;

				// if it is a real reg# then save it to app res
				if ( success || (demo && !timeout)  )
				{
					(*SerialDataH)->startTime = time(0);

					GetLocalHostName( (*SerialDataH)->localHostName, 127 );
					if ( demo ){
						SaveDemoInfo( SerialDataH, kUserInfoDataName );
						SerialDataH = GetDemoInfo( SerialDataH );			// read serial from file
					} else
					if ( success ){
						SaveSerialInfo( SerialDataH, kUserInfoDataName ); tries=0;
						ShowRegMessage();
						SerialDataH = GetSerialInfo( SerialDataH );			// read serial from file
					}
					break;
				} else 
				{
					if ( !timeout )
					{
						// bad reg# ,  out put error and try again
						ShowMsg_ReEnterSerial( (*SerialDataH)->serialNumber+1 );
					}
				}
			} else {
				if ( askagain )
					runstatus = FALSE;
				return 0;
			}
			tries--;
		}
	} else {
		success = 1;
	}

	runstatus = FALSE;
	//VoidSerialNum( (*SerialDataH)->serialNumber );	// erase serial # from memory so it cant be found...
	//SerialDataH = GetSerialInfo( SerialDataH );			// read serial from file
	return success | demo;
}

void ReSaveRegistry( void )
{
#if (!DEF_FREEVERSION)
	if( !IsDemoReg() )
	{
		SaveSerialInfo( SerialDataH, kUserInfoDataName );
		SerialDataH = GetSerialInfo( SerialDataH );			// read serial from file
	}
#endif
}


short DoRegistrationAgain( void )
{
// ---------------------------------- FREE VERSION EXPIRE CHECK -----------------------------------
#if (DEF_FREEVERSION) && (!DEF_FREEVERSIONDYNAMIC)
		return 1;
#endif
// ------------------------------------------------------------------------------------------------
	return CheckRegistration( 1 );
}


short DoRegistrationProcedure( void )
{
	return 1;

#ifdef DEF_DEBUG
	return 1;
#endif
// ---------------------------------- FREE VERSION EXPIRE CHECK -----------------------------------
#if (DEF_FREEVERSION) && (!DEF_FREEVERSIONDYNAMIC)
	if ( GetDaysLeft() <= 0 )
	{
		static long lastNotice = 0;
		if ( (GetCurrentCTime()-lastNotice) > 500 || lastNotice == 0 )
		{
			lastNotice = GetCurrentCTime();
			ShowMsg_FreeVersionExpired();
			return 0;
		}

	}
	return 1;
// ------------------------------------------------------------------------------------------------
#else
	return CheckRegistration( 0 );
#endif
}

