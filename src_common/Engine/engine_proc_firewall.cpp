// NOTE : This code is run on all platforms and MUST NOT have any OS specific includes or code
// unless wrapped in #defines, please keep OS specifics to a minimum

#include "FWA.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "myansi.h"
#include "datetime.h"

#include "config.h"				// for OutDebugs()
#include "config_struct.h"
#include "engine_proc.h"
#include "engine_proc_firewall.h"
#include "log_io.h"
#include "LogFileFormat.h"

extern short	logDelim;
extern short	prevLogDelim;
extern short	logNumDelimInLine;

extern FormatIndex formatIndex;
extern FormatIndex prevLogFileFormatIndex;
extern char GlobalDate[32];

static	char	*format[FORMATSIZE*3];


/*

[web]
http
https
80/tcp
[email]
smtp
[ftp]
ftp
ftp-data
[telnet]
telnet
[realaudio]
realaudio

Jan 08 02:53:23 WebTrendsSample tcp-gsp[313]: 503 www.egSoftware.com 206.58.83.194: reverse address 192.168.0.194 doesn't match
Jan 08 04:03:23 WebTrendsSample smtp[299]: 228 smtp: can't connect to 195.212.24.201 port 25 (Network is unreachable.)
Jan 08 04:03:23 WebTrendsSample httpd[298]: 121 Statistics: duration=0.57 sent=320 rcvd=6259 srcif=Vpn4 src=192.168.0.164/2002 srcname=JasonG dstif=Vpn3 dst=199.230.26.69/80 dstname=www.prnewswire.com op=GET arg=/companyhed*.gif result="200 OK" proto=http rule=2 
Jan 08 04:03:24 WebTrendsSample firelogd[115]: 347 Possible Port Scan detected on Interface 206.58.83.2 (209.96.26.99->206.58.83.2): Protocol=UDP Port 137->137 [2 duplicates suppressed]
Jan 08 09:53:30 WebTrendsSample RealAudio[475]: 121 Statistics: interval=120.10 sent=431 rcvd=241834 srcif=Vpn4 src=192.168.0.242/2556 dstif=Vpn3 dst=204.164.100.21/7070 arg=mr.ra proto=RealAudio rule=2 
Jan 08 16:53:32 WebTrendsSample dnsd[535]: 120 dnsd Info: Failed to handle request from 192.168.0.49 for Address for DSMFL.SD. - no progress possible (206.58.0.35/No response, 206.58.0.34/No response)
Jan 08 19:54:11 WebTrendsSample 121 Statistics: interval=121.87 id=3rnMfB sent=128 rcvd=438 srcif=Vpn3 src=202.98.163.28/1293 dstif=Vpn4 dst=192.168.0.196/21 proto=ftp rule=5 
Jan 08 19:56:14 WebTrendsSample httpd[275]: 121 Statistics: duration=0.06 sent=308 rcvd=204 srcif=Vpn4 src=192.168.0.164/2130 srcname=JasonG dstif=Vpn3 dst=192.65.18.21/80 dstname=inet1.tek.com op=GET arg=/Gallery/camera2/ result="302 Found" proto=http rule=2 
Jan 10 20:00:12 WebTrendsSample smtp[299]: 121 Statistics: duration=5.81 user=<abell@chrono.edu> sent=1264 rcvd=367 srcif=Vpn4 src=192.168.0.104/2612 srcname=WEBSERVER dstif=Vpn3 dst=149.174.217.138/25 dstname=arl-img-8.compuserve.com op="To 1 recips" arg=<B0091645966@WebTrends.com> result="250 WAA17147 Message accepted for delivery" proto=smtp rule=11 

*/

short PROC_Raptor(char *buffer,HitDataPtr Line)
{
	char 	*ptr;		
	short	i=1, tot;
	static char globalyear[8];

	MemClr( format, 25*sizeof(void*) );
	MemClr( Line, sizeof( struct HitData ) );
	//Tokenise line into format fields 1 to 25 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == ' ') {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	tot = i;
	//check if it is a valid format first
	if (tot<8)
		return 0;

	if ( Line->lineNum > 1 )
	{
		sprintf( GlobalDate, "%02d/%s/%s", Month2Num( format[1] ), format[2], globalyear );
		Line->date = GlobalDate;				//Jan 08 02:53:23
	}
	Line->time = format[3];
	Line->stat = format[6];
	if( Line->protocol = format[5] ){
		ptr = mystrchr( Line->protocol, '[' );	// remove [ from the protocol field
		if ( ptr ) *ptr = 0;
	}

	i = 5;
	while( i<=tot ){
		if ( format[5] && format[i] ){
			if ( ptr=strstr( format[i], "src=" ) ){
				if( Line->clientaddr = 4+ptr ){
					if ( ptr=mystrchr( 4+ptr, '/' ) ) *ptr = ':';			// replace / with : for port number of address
				}
			} else
			if ( ptr=strstr( format[i], "srcname=" ) )
				Line->user = 8+ptr;
			else
			if ( ptr=strstr( format[i], "Year" ) )
			{
				// This is a line (generally the 1st line) which gives you the current year for all the other lines which 
				// just give the day & month (as well as the time)
				mystrcpy( globalyear, format[i+2] );
				globalyear[4] = 0;
				sprintf( GlobalDate, "%02d/%s/%s", Month2Num( format[1] ), format[2], globalyear );
				Line->date = GlobalDate;				//Jan 08 02:53:23
				return 0; // This is a line with the year in it
			}
			else
			if ( ptr=strstr( format[i], "dst=" ) ){
				if( Line->sourceaddr = 4+ptr ){
					if ( ptr=mystrchr( 4+ptr, '/' ) ) {
						Line->port = myatoi( ptr+1 );
						*ptr = 0;			// replace / with : for port number of address
					}
				}
			} else
			if ( ptr=strstr( format[i], "dstname=" ) ){
				if( Line->file = 8+ptr ){
					if ( ptr=mystrchr( 8+ptr, '/' ) ){
						Line->port = myatoi( ptr+1 );
						*ptr = 0;			// replace / with : for port number of address
					}
				}
			} else
			if ( ptr=strstr( format[i], "arg=" ) ){			// if name enclosed in <> only use content
				if ( ptr[4] == '<' ) {
					ptr++;
					if( Line->file ) mystrcat( Line->file, " , " );
				} else
					if( Line->file ) mystrcat( Line->file, "/" );

				if ( Line->file ) {
					mystrcat( Line->file, 4+ptr );
				} else
					Line->file = 4+ptr;
				if ( ptr=strrchr( ptr, '>' ) ) *ptr = 0;
			} else
			if ( ptr=strstr( format[i], "proto=" ) )
				Line->protocol = 6+ptr;
			else
			if ( ptr=strstr( format[i], "Protocol=" ) )
				Line->protocol = 9+ptr;
			else
			if ( ptr=strstr( format[i], "sent=" ) )
				Line->bytesIn = myatoi( 5+ptr );
			else
			if ( ptr=strstr( format[i], "rcvd=" ) )
				Line->bytes = myatoi( 5+ptr );
			else
			if ( ptr=strstr( format[i], "result=" ) )
				Line->stat = 7+ptr;
			else
			if ( ptr=strstr( format[i], "duration=" ) )
				Line->ms = myatoi( 9+ptr ) * 1000;
			else
			if ( ptr=strstr( format[i], "user=" ) ){
				if ( ptr[5] == '<' ) ptr++;
				Line->clientaddr = 4+ptr;
				if ( ptr=strrchr( ptr, '>' ) ) *ptr = 0;
			}
		}
		i++;
	}
	return 1;
}


/*
Nov 30 00:00:03 eaglent firelogd[127]: 108 starting new log file. UTC offset is +1000, Year is 1998, Eagle is 5.0.1i, OS is "NT 4.0 (Build 1381: Service Pack 3)", Platform is "Intel x86"
Nov 30 00:00:11 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for MailServer for . - no progress possible (139.130.4.4/No response)
Nov 30 00:00:19 eaglent xntpd[132]: 120 NTP Info: offset 0.023925 sec freq 13.340 ppm poll 10
Nov 30 00:27:35 eaglent smtp[357]: 121 Statistics: duration=9.19 user=<jensen@malvern.hotkey.net.au> sent=1404 rcvd=314 srcif=Vpn8 src=203.32.30.36/29807 srcname=malvern.starway.net.au dstif=Vpn5 dst=172.168.1.220/25 dstname=main.unisuper.com.au op="To 1 recips" arg=<199811291426.BAA31157@malvern.starway.net.au> result="250 Data received OK." proto=smtp rule=3 
Nov 30 00:41:30 eaglent httpd[338]: 121 Statistics: duration=0.03 sent=262 rcvd=503 srcif=Vpn6 src=212.17.88.76/1377 srcname=TK212017088076.tuwien.teleweb.at dstif=Vpn5 dst=172.168.1.230/80 dstname=www.unisuper.com.au op=GET arg=/python/ result="404 Not Found" proto=http rule=18 
Nov 30 00:46:50 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for MailServer for nothnet.com.au. - no progress possible (139.130.4.4/No response)
Nov 30 00:46:50 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for MailServer for netscape.net.au. - no progress possible (139.130.4.4/No response)
Nov 30 00:46:50 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for MailServer for netscape.net.au. - no progress possible (139.130.4.4/No response)
Nov 30 00:46:50 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for MailServer for netscape.net.au. - no progress possible (139.130.4.4/No response)
Nov 30 00:46:50 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for MailServer for mugs.cc.monash.edu.au. - no progress possible (139.130.4.4/No response)
Nov 30 00:46:50 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for MailServer for . - no progress possible (139.130.4.4/No response)
Nov 30 00:47:04 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for Address for nothnet.com.au. - no progress possible (139.130.4.4/No response)
Nov 30 00:47:06 eaglent httpd[338]: 121 Statistics: duration=0.04 sent=259 rcvd=375 srcif=Vpn6 src=195.92.199.104/21876 srcname=webcache05s.cache.pol.co.uk dstif=Vpn5 dst=172.168.1.230/80 dstname=www.unisuper.com.au op=GET arg=/startrek/strek.htm result="404 Not Found" proto=http rule=18 
Nov 30 00:47:18 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for Address for nothnet.com.au.com.au. - no progress possible (139.130.4.4/No response)
Nov 30 00:47:34 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for Address for netscape.net.au. - no progress possible (139.130.4.4/No response)
Nov 30 00:47:48 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for Address for netscape.net.au.com.au. - no progress possible (139.130.4.4/No response)
Nov 30 00:48:04 eaglent dnsd[129]: 120 dnsd Info: Failed to handle request from 172.168.1.245 for Address for netscape.net.au. - no progress possible (139.130.4.4/No response)
*/
short PROC_Firewall(char *buffer,HitDataPtr Line)
{
	char 	*ptr;		
	short	i=1, tot;
	static char globalyear[8];
	static char newFieldFound = 0;
	static char newFieldMsgDisplayed = 0;
	static char newFieldName[32];

	MemClr( format, 25*sizeof(void*) );
	MemClr( Line, sizeof( struct HitData ) );
	//Tokenise line into format fields 1 to 25 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == ' ') {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	tot = i;
	//check if it is a valid format first
	if (tot<8)
		return 0;

	i = 1;
	while( i<tot )
	{
		switch( *format[i] )
		{
		case 's':
			if ( ptr=strstr( format[i], "src=" ) ){
				if( Line->clientaddr = 4+ptr ){
					if ( ptr=mystrchr( 4+ptr, '/' ) ) *ptr = ':';			// replace / with : for port number of address
				}
			} else
			if ( ptr=strstr( format[i], "srcname=" ) )
				Line->user = 8+ptr;
			else
			if ( ptr=strstr( format[i], "service=" ) )
				Line->protocol = 8+ptr;
			else
				newFieldFound = 1;
			break;
		case 'd':
			if ( ptr=strstr( format[i], "dst=" ) ){
				if( Line->sourceaddr = 4+ptr ){
					if ( ptr=mystrchr( 4+ptr, '/' ) ) {
						Line->port = myatoi( ptr+1 );
						*ptr = 0;			// replace / with : for port number of address
					}
				}
			} else
			if ( ptr=strstr( format[i], "dstname=" ) ){
				if( Line->file = 8+ptr ){
					if ( ptr=mystrchr( 8+ptr, '/' ) ){
						Line->port = myatoi( ptr+1 );
						*ptr = 0;			// replace / with : for port number of address
					}
				}
			} else
			if ( ptr=strstr( format[i], "duration=" ) )
				Line->ms = myatoi( 9+ptr );
			else
				newFieldFound = 1;
			break;
		case 'p':
			if ( ptr=strstr( format[i], "port=" ) )
				Line->port = myatoi( 5+ptr );
			else
//			if ( ptr=strstr( format[i], "policy=" ) )
//				Line->policy = myatoi( 7+ptr );
//			else
				newFieldFound = 1;
			break;
		case 'a':
//			if ( ptr=strstr( format[i], "action=" ) )
//				Line->action = 7+ptr;
//			else
				newFieldFound = 1;
			break;
		case 't':
			//Fix this
			if ( ptr=strstr( format[i], "time=\"" ) )
			{
				// This is a line (generally the 1st line) which gives you the current year for all the other lines which 
				// just give the day & month (as well as the time)
				//mystrncpyNull( globalyear, format[i]+4, 5 );
				//globalyear[4] = 0;
				Line->date = ConvLDate( format[i]+6, Line->newdate );
//				sscanf( 
//				sprintf( GlobalDate, "%d-%d-%d", format[i] );
				//Line->date = GlobalDate;
				i++;
				char *p = format[i];
				while( *p != '\"' )
					p++;
				p = 0;
				Line->time = format[i];

				//return 0; // This is a line with the year in it
			}
			else
				newFieldFound = 1;
			break;
		}
		if ( newFieldFound )
		{
			if ( !newFieldMsgDisplayed )
			{
				// Display a message to the user...
				//ErrorMsg( format[i] );
				newFieldMsgDisplayed = 1;
			}
			newFieldFound = 0;
		}
		else
		{
			if ( ptr=strstr( format[i], "arg=" ) ){			// if name enclosed in <> only use content
				if ( ptr[4] == '<' ) {
					ptr++;
					if( Line->file ) strcat( Line->file, " , " );
				} else
					if( Line->file ) strcat( Line->file, "/" );

				if ( Line->file ) {
					strcat( Line->file, 4+ptr );
				} else
					Line->file = 4+ptr;
				if ( ptr=strrchr( ptr, '>' ) ) *ptr = 0;
			} else
			if ( ptr=strstr( format[i], "proto=" ) )
				Line->protocol = 6+ptr;
			else
			if ( ptr=strstr( format[i], "Protocol=" ) )
				Line->protocol = 9+ptr;
			else
			if ( ptr=strstr( format[i], "sent=" ) )
				Line->bytesIn = myatoi( 5+ptr );
			else
			if ( ptr=strstr( format[i], "rcvd=" ) )
				Line->bytes = myatoi( 5+ptr );
			else
			if ( ptr=strstr( format[i], "result=" ) )
				Line->stat = 7+ptr;
			else
			if ( ptr=strstr( format[i], "duration=" ) )
				Line->ms = myatoi( 9+ptr ) * 1000;
			else
			if ( ptr=strstr( format[i], "user=" ) ){
				if ( ptr[5] == '<' ) ptr++;
				Line->clientaddr = 4+ptr;
				if ( ptr=strrchr( ptr, '>' ) ) *ptr = 0;
			}
		}
		i++;
	}
	return 1;
}

