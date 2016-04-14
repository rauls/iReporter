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
#include "engine_proc_streaming.h"
#include "engine_proc_firewall.h"
#include "log_io.h"
#include "LogFileFormat.h"

extern short	logDelim;
extern short	prevLogDelim;
extern short	logNumDelimInLine;



static	char	OKStr[] = "200";
static	char	CACHEStr[] = "304";
//static	long	logIndex[FORMATSIZE*3];
static	char	*format[FORMATSIZE*3];
const short HEADNAMESIZE = 16;
static char formatHeadNames[FORMATSIZE][HEADNAMESIZE];
static short tabOnEnd = 0; 

FormatIndex formatIndex;
FormatIndex prevLogFileFormatIndex;

char GlobalDate[32];
char GlobalTime[16];













/*-------------------------------------------

	Process and decode each line of the log
	into the correct format

---------------------------------------------*/
double UnixTimetoJD(char *utStr);
int issquid(const char *str);
void LOG_MacHTTP(char *buffer);
void LOG_W3C(char *buffer);
void LOG_Netscape(char *buffer);
void LOG_Purveyer(char *buffer);
void LOG_QTSS(char *buffer);
short PROC_Common(char *buffer,HitDataPtr Line);
short PROC_Netscape(char *buffer,HitDataPtr Line);
short PROC_Purveyer(char *buffer,HitDataPtr Line);
short PROC_MacHTTP(char *buffer,HitDataPtr Line);
short PROC_Website(char *buffer,HitDataPtr Line);
short PROC_IIS(char *buffer,HitDataPtr Line);
short PROC_W3C(char *buffer,HitDataPtr Line);
short PROC_Netpresenz(char *buffer,HitDataPtr Line);
short PROC_Firstclass(char *buffer,HitDataPtr Line);
short PROC_Realserver(char *buffer,HitDataPtr Line);
short PROC_Filemaker(char *buffer,HitDataPtr Line);
short PROC_Bounce(char *buffer,HitDataPtr Line);
short PROC_QTSS(char *buffer,HitDataPtr Line );
//Professional features
short PROC_Squid(char *buffer,HitDataPtr Line);
short PROC_IISProxy(char *buffer,HitDataPtr Line);
short PROC_WSProxy(char *buffer,HitDataPtr Line);
short PROC_Ciscorouter(char *buffer,HitDataPtr Line);
short PROC_Zeus(char *buffer,HitDataPtr Line );

int istimestring( char *string )
{
	// check 00:00:00
	//       01234567
	if ( string ){
		if ( string[2] == ':' && string[5] == ':' ){
			if ( !isdigit( string[0] ) ) return FALSE;
			if ( !isdigit( string[3] ) ) return FALSE;
			if ( !isdigit( string[6] ) ) return FALSE;
			return TRUE;
		}
	}
	return FALSE;
}


int isdatestring( char *string )
{
	// check 00/00/00
	//       01234567
	if ( string ){
		char *p;
		int slash=0;

		p = string;
		while( *p > ' ' ){
			if ( *p == '/' )
				slash++;
			p++;
		}
		if ( slash==2 && isdigit(*string) ){
			return TRUE;
		}
	}
	return FALSE;
}


int issquid(const char *str)
{
	short cnt=0;
	
	//first 8 must be digits
	while(*str && cnt<9) {
		if (!isdigit(*str))
			return 0;
		str++;
		cnt++;
	}
	// Must have 8 digits to be valid!
	if ( cnt < 8 )
		return 0;
	else
		return 1;
}


double UnixTimetoJD(char *utStr )
{
	double	st1;
	long		v1;
	
	v1 = myatoi( utStr );
	st1 = v1/86400.0;
	st1 += 2440587.501;			// add the JD of 1/1/1970
	return st1;
}





void LOG_Common(char *buffer) 
{
	formatIndex.Clear();
	formatIndex.SetDate( 4 );
	formatIndex.time = 4;
	formatIndex.status = 6;
	formatIndex.SetHostName( 1 );
	formatIndex.url = 5;
	formatIndex.agent = 9;
	formatIndex.bytes = 7;
	formatIndex.referer = 10;
	formatIndex.user = 2;
	formatIndex.cshost = 11;
	formatIndex.method = 5;
	formatIndex.cookie = 0;

}



// process WebStar / MacHTTP log format and return logIndex array
void LOG_MacHTTP(char *buffer) 
{
	short x,i=1;
	short logbytes=FALSE,found=FALSE;

	logType=LOGFORMAT_MACHTTP;
	
	mymemset( formatHeadNames, 0, FORMATSIZE*HEADNAMESIZE );
	format[i]=buffer;
	while (*buffer) {
		if ( *buffer == ' ' ) {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}

	//now search for required fields in format fields
	for (x=1;x<i;x++)
		if (!mystrcmpi( "DATE", format[x])) {
			formatIndex.SetDate( x );
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "TIME", format[x])) {
			formatIndex.time = x;
			break;
		}
	for (x=1;x<i;x++)																								// result_code for Quid Pro Quo
		if (!mystrcmpi( "RESULT", format[x])) {
			formatIndex.status = x;
			break;
		}
/*
!!LOG_FORMAT DATE TIME RESULT STATUS HOSTNAME AGENT REFERER HOSTFIELD URL SEARCH_ARGS BYTES
!!LOG ARCHIVER:	WebSTAR has disabled automatic log archiving
12/27/00	00:00:05	OK  	-	209.41.248.23			"www.wholesalenow.com"	:wholesalenow:main:data:WebMerchant:ProcessOrders.tpl		431	
12/27/00	00:00:06	OK  	-	209.41.248.23			"www.wholesalenow.com"	:wholesalenow:main:data:WebMerchant:ProcessOrders.tpl		431	
12/27/00	00:00:11	OK  	-	64.208.37.78	Googlebot/2.1 (+http://www.googlebot.com/bot.html)		"www.realfyre.com"	:realfyre:1-showproducts.tpl	category=*Vent-Free%20Logs%20Natural%20Gas	32394	
12/27/00	00:00:11	ERR!	404	64.12.96.235	Mozilla/4.0 (compatible; MSIE 5.0; AOL 6.0; Windows 98; DigExt)		"www.lynxgrills.net"	:lynxgrillsnet:clearpixel.gif		15238	
12/27/00	00:00:21	ERR!	404	64.208.37.60	Googlebot/2.1 (+http://www.googlebot.com/bot.html)		"www.realfyre.com"	:realfyre:robots.txt		584	
12/27/00	00:00:26	OK  	-	64.208.37.60	Googlebot/2.1 (+http://www.googlebot.com/bot.html)		"www.realfyre.com"	:realfyre:1-showproducts.tpl	category=*Vent-Free%20Logs%20Propane%20Gas	33351	
*/
	for (x=1;x<i;x++)																								// result_code for Quid Pro Quo
		if (!mystrcmpi( "CS-STATUS", format[x]) || !mystrcmpi( "SC-STATUS", format[x]) || !mystrcmpi( "RESULT_CODE", format[x]) || !mystrcmpi( "STATUS", format[x])) {
			formatIndex.status = x;
			break;
		}
	//this section tries to force finding the correct tokens first
	//to avoid the problem of blank entries with similar new style tokens
	for (x=1;x<i;x++) // find this token first whenever possible
		if (!mystrcmpi( "HOSTNAME", format[x])) {
			formatIndex.SetHostName( x );
			found=TRUE;
			break;
		}
	if (!found) { //if DNS lookups are enabled, else "-" blank
		for (x=1;x<i;x++) //uses both new and old versions of this token
			if (!mystrcmpi( "C-DNS", format[x]) || !mystrcmpi( "CS-Host", format[x])) {
				formatIndex.SetHostName( x );
				found=TRUE;
				break;
			}
	}
	for (x=1;x<i;x++){ //uses both new and old versions of this token
		if (!mystrcmpi( "C-IP", format[x]) || !mystrcmpi( "CS-IP", format[x])) {
			if (!found)	formatIndex.SetHostName( x );

			if (found)	formatIndex.csip = x;

			break;
		}
	}
	//// end of token searching
	for (x=1;x<i;x++)
		if (!mystrcmpi( "URL", format[x]) || !mystrcmpi( "CS-URI", format[x]) || !mystrcmpi( "CS-URI-STEM", format[x])) {
			formatIndex.url = x;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "AGENT", format[x]) || !mystrcmpi( "CS(USER-AGENT)", format[x])) {
			formatIndex.agent = x;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "BYTES", format[x]) || !mystrcmpi( "BYTES_SENT", format[x])) {
			formatIndex.bytes = x;
			logbytes=TRUE;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "REFERER", format[x]) || !mystrcmpi( "CS(REFERER)", format[x])) {
			formatIndex.referer = x;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "USER", format[x])) {
			formatIndex.user = x;
			break;
		}
	for (x=1;x<i;x++)    						// host required in Quid Pro Quo
		if (!mystrcmpi( "CS(HOST)", format[x]) || !mystrcmpi( "HOSTFIELD", format[x]) || !mystrcmpi( "HOST", format[x])) {
			formatIndex.cshost = x;
			break;
		}
	for (x=1;x<i;x++)    						// host required in Quid Pro Quo
		if (!mystrcmpi( "SEARCH_ARGS", format[x]) || !mystrcmpi( "CS-URI-QUERY", format[x])) {
			formatIndex.query = x;
			break;
		}
	for (x=1;x<i;x++) 
		if (!mystrcmpi( "CS(COOKIE)", format[x]) || !mystrcmpi( "CS-COOKIE", format[x]) || !mystrcmpi( "COOKIE", format[x])) {
			formatIndex.cookie = x;
			break;
		}
	
	//decrement counter due to !!LogFormat spec
	logDelim = i-2;
	if (logbytes==FALSE) MyPrefStruct.filter_zerobyte=0;
}

// log W3C format and return logIndex array
// also valid for SUN format tokens
void LOG_W3C(char *buffer) 
{
	short x,i=1,j=0;
	short logbytes=FALSE;
	
	formatIndex.Clear();

	format[i++]=buffer;

	while (*buffer)
	{
		if ( *buffer <= ' ' ) {
			*buffer++ = 0;
			if ( *buffer>32 )
    			format[i++] = buffer;
		} else
			buffer++;
	}
	
	//now search for required fields in format fields
	// Darren addition to handle WELCOME
	if (logType==LOGFORMAT_WELCOME) {
		for (x=1;x<i;x++){
			if (!mystrcmpi( "cs-uri-query", format[x]) ) {
				if ( formatIndex.url )
					formatIndex.query = x-1;
				else
					formatIndex.url = x-1;
			} else
			if (!mystrcmpi( "cs-uri-stem", format[x])  ) {
				formatIndex.url = x-1;
			}
		}
	} else { /// normal format
		for (x=1;x<i;x++){
			if (!mystrcmpi( "cs-uri-query", format[x]) ) {
				formatIndex.query = x-1;
			} else
			if (!mystrcmpi( "cs-uri-stem", format[x]) ) {
				formatIndex.url = x-1;
			} else
			if (!mystrcmpi( "cs-uri", format[x]) ) {			// ISA support
				formatIndex.url = x-1;
			}
		}
	}

	
	
	for (x=1;x<i;x++)
		if (!mystrcmpi( "date", format[x])) {
			formatIndex.SetDate( x-1 );
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "time", format[x])) {
			formatIndex.time = x-1;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "sc-status", format[x])) {
			formatIndex.status = x-1;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "c-ip", format[x]) || !mystrcmpi( "cs-ip", format[x]) || !mystrcmpi( "c-dns", format[x])) {
			formatIndex.SetHostName( x-1 );
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "cs(User-Agent)", format[x]) || !mystrcmpi( "c-agent", format[x]) ) {
			formatIndex.agent = x-1;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "sc-bytes", format[x]) || !mystrcmpi( "bytes", format[x])) {
			formatIndex.bytes = x-1;
			logbytes=TRUE;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "cs-bytes", format[x]) ) {
			formatIndex.bytesIn = x-1;
			logbytes=TRUE;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "cs(Referer)", format[x]) || !mystrcmpi( "cs(referrer)", format[x]) || !mystrcmpi( "cs-referred", format[x]) ) {
			formatIndex.referer = x-1;
			break;
		}
	for (x=1;x<i;x++)
		if (!mystrcmpi( "cs-username", format[x])) {
			formatIndex.user = x-1;
			break;
		}

	// QCM: 50211 - only in cluster mode do we only accept the IP host field which is guranteed.
	if ( MyPrefStruct.clusteringActive )
	{
		for (x=1;x<i;x++)
		{
			if ( !mystrcmpi( "s-ip", format[x])	)
			{
				formatIndex.cshost = x-1;
				//break;
			}
		}
	} else {
		for( x=1;x<i;x++ )
		{
			if( !mystrcmpi( "s-ip", format[x])
				|| !mystrcmpi( "cs(host)", format[x])
				|| !mystrcmpi( "cs-host", format[x])
				|| !mystrcmpi( "s-computername", format[x])
				)
			{
				formatIndex.cshost = x-1;
				//break;
			}
		}
	}

	for (x=1;x<i;x++)
		if (!mystrcmpi( "cs-method", format[x])) {
			formatIndex.method = x-1;
		}
	for (x=1;x<i;x++)
	{
		if (!mystrcmpi( "cs(Cookie)", format[x])) {
			formatIndex.cookie = x-1;
		} else
		if (!mystrcmpi( "r-port", format[x])) {
			formatIndex.port = x-1;
		} else
		if (!mystrcmpi( "cs-protocol", format[x])) {
			formatIndex.protocol = x-1;
		} else
		if (!mystrcmpi( "s-event", format[x])) {
			formatIndex.process_accounting = 1;
		} else
		if (!mystrcmpi( "time-taken", format[x])) {
			formatIndex.ms = x-1;
		}
	}
	
	//search for clf token last
	for (x=1;x<i;x++)
	{
		if (!mystrcmpi( "clf", format[x])) {
			formatIndex.SetDate( x-1 + 3 );
			formatIndex.time = x-1 + 3;
			formatIndex.SetHostName( x-1 );
			formatIndex.user = x-1 + 1;
			formatIndex.status = x-1 + 8;
			formatIndex.bytes = x-1 + 9;
			formatIndex.method = x-1 + 5;
			formatIndex.url = x-1 + 6;
			
			if (formatIndex.agent)
				formatIndex.agent += 9;
			
			if (formatIndex.referer)
				formatIndex.referer += 9;
				
			if (formatIndex.cshost)
				formatIndex.cshost += 9;

			if (formatIndex.cookie)
				formatIndex.cookie += 9;
	
			logbytes = TRUE;
			
			j=10;
			break;
		}
	}

	//decrement counter due to #Field spec
	logDelim=i-1+j;
	
	if ( logbytes==FALSE )
		MyPrefStruct.filter_zerobyte = 0;
}






void DoHeader_NetCache( char *buffer ) 
{
	short x,i=1;
	short logbytes=FALSE;
	char ignoreuntil = 0;
	
	format[i++]=buffer;
	while (*buffer) {
		if ( *buffer == ' ' ) {
			*buffer++ = 0;
			if ( *buffer>32 )
   				format[i++] = buffer;
		} else
			buffer++;
	}
	
	//now search for required fields in format fields
	for (x=1;x<i;x++){
		if (!mystrcmpi( "c-ip", format[x]) ) 			formatIndex.SetHostName( x-1 );
		if (!mystrcmpi( "x-localtime", format[x]) )		formatIndex.SetDate( x-1 );
		if (!mystrcmpi( "x-username", format[x]) )		formatIndex.user = x-1;
		if (!mystrcmpi( "x-request-line", format[x]) )	formatIndex.url = x-1;
		if (!mystrcmpi( "sc-status", format[x]) )		formatIndex.status = x-1;
		if (!mystrcmpi( "x-sc-contentlength", format[x]) )		formatIndex.bytes = x-1;
		if (!mystrcmpi( "cs(User-Agent)", format[x]) )	formatIndex.agent = x-1;
		if (!mystrcmpi( "cs(Referer)", format[x]) )		formatIndex.referer = x-1;
		if (!mystrcmpi( "x-transaction", format[x]) )	formatIndex.status = x-1;
	}
	
	//decrement counter due to #Field spec
	logDelim=i-1;
	
	if (logbytes==FALSE) MyPrefStruct.filter_zerobyte=0;
}




void LOG_Netscape(char *buffer) 
{
	long 	n;
	char delim;
	short x,i=1,nudge=0,logbytes=FALSE;
		
	n=mystrlen(buffer);
	if (strstr( buffer,"%Ses->client.ip% - %Req->vars.auth-user% [%SYSDATE%] \"%Req->reqpb.clf-request%\" %Req->srvhdrs.clf-status% %Req->srvhdrs.content-length%") && n<145) { // Common log format found
		logType = LOGFORMAT_COMMON;
		return;
	} else {		
		logType = LOGFORMAT_NETSCAPE;
	}
	
	//tokenise line delimited by various tokens ' ', [] and "
	format[i++]=buffer;
	delim = ' ';
	nudge = 1;
	while (*buffer) {
		if ( *buffer == delim) {
			*buffer=0;
			if (*(buffer+1)==0) {	//prevents buffer overrun
				format[i] = buffer+1;
				buffer++;
			} else {
				format[i] = buffer+nudge;
				buffer+=nudge;
			}
			if (*format[i]=='\"') {
				format[i]++;
				buffer++;
				delim='\"';
				nudge=2;
			} else if (*format[i]=='[') {
				format[i]++;
				buffer++;
				delim=']';
				nudge=2;
			} else {
				delim=' ';
				nudge=1;
			}
			i++;
		} else
			buffer++;
	}
	// set the number of delimiters
	logDelim=i;
	// now get index to fields
	
	for (x=1;x<i;x++)
		if (strstr(format[x],"%SYSDATE%")) {     // Date & Time enclosed in []
			formatIndex.SetDate( x );
			break;
		}
	for (x=1;x<i;x++)
		if (strstr(format[x],"%Ses->client.ip%")) {  // Hostname
			formatIndex.time = x;
			break;
		}
	for (x=1;x<i;x++)
		if (strstr(format[x],"%Req->srvhdrs.clf-status%")) {  // Status
			formatIndex.status = x;
			break;
		}
	for (x=1;x<i;x++)
		if (strstr(format[x],"%Req->reqpb.uri%") ||
			strstr(format[x],"%Req->reqpb.clf-request%") ||
			strstr(format[x],"%Req->reqpb.proxy-request%")) {  // URL
			formatIndex.SetHostName( x );
			break;
		}
	for (x=1;x<i;x++)
		if (strstr(format[x],"%Req->srvhdrs.content-length%")) {  // Bytes
			formatIndex.url = x;
			logbytes=TRUE;
			break;
		}
	for (x=1;x<i;x++)
		if (strstr(format[x],"%Req->headers.referer%")) {  // Referrals
			formatIndex.agent = x;
			break;
		}
	for (x=1;x<i;x++)
		if (strstr(format[x],"%Req->headers.user-agent%")) {  // Agent
			formatIndex.bytes = x;
			break;
		}
	for (x=1;x<i;x++)
		if (strstr(format[x],"%Req->vars.auth-user%")) {  // User
			formatIndex.referer = x;
			break;
		}
	for (x=1;x<i;x++)
		if (strstr(format[x],"%Req->headers.host%")) {  // VHost
			formatIndex.user = x;
			break;
		}
	for (x=1;x<i;x++)
		if (strstr(format[x],"%Req->headers.cookie.user_session_id%")) {  // VHost
			formatIndex.cookie = x;
			break;
		}

	//%Req->headers.host%
	if (logbytes==FALSE) MyPrefStruct.filter_zerobyte=0;
}



void LOG_Purveyer(char *buffer) 
{
	long 	n;
	char delim;
	short x,i=1,nudge=0,logbytes=FALSE;

	n=mystrlen(buffer);
	if (strstr( buffer,"%r %i %u [%d/%b/%Y:%H:%M:%S %O] \"%q\" %s %n") && n<70) { 
		logType = LOGFORMAT_COMMON; // Standard Common log format found
		return;
	} else if (strstr( buffer,"%r %i %u [%d/%b/%Y:%H:%M:%S %O] \"%q\" %s %n \"%:Referer:\" \"%:User-agent:\"") && n<99) {
		logType = LOGFORMAT_COMMON; // Combined log format found
		return;
	} else {		
		logType = LOGFORMAT_PURVEYER;
	}
	//tokenise line delimited by various tokens ' ', [] and "
	format[i++]=buffer;
	delim = ' ';
	nudge = 1;
	while (*buffer) {
		if ( *buffer == delim) {
			*buffer=0;
			if (*(buffer+1)==0) {	//prevents buffer overrun
				format[i] = buffer+1;
				buffer++;
			} else {
				format[i] = buffer+nudge;
				buffer+=nudge;
			}
			if (*format[i]=='\"') {
				format[i]++;
				buffer++;
				delim='\"';
				nudge=2;
			} else if (*format[i]=='[') {
				format[i]++;
				buffer++;
				delim=']';
				nudge=2;
			} else {
				delim=' ';
				nudge=1;
			}
			i++;
		} else
			buffer++;
	}
	
	for (x=3;x<i;x++)
		if (!strcmpiPart( "%d", format[x])) {  // Date
			formatIndex.SetDate( x-3 );
			break;
		}
	for (x=3;x<i;x++)
		if (!strcmpiPart( "%H:%M:%S", format[x])) {  // Time - if within a token returns -1
			formatIndex.time = x-3;
			break;
		}
	for (x=3;x<i;x++)
		if (!strcmpiPart( "%s", format[x])) {  // Result -not case sensitive
			formatIndex.status = x-3;
			break;
		}	
	for (x=3;x<i;x++)
		if (!strcmpiPart( "%R", format[x])) {  // Hostname -not case sensitive
			formatIndex.SetHostName( x-3 );
			break;
		}
	for (x=3;x<i;x++)
		if (!strcmpiPart( "%Q", format[x])) {  // URL -not case sensitive
			formatIndex.url = x-3;
			break;
		}
	for (x=3;x<i;x++)
		if (!strcmpiPart( "%n", format[x])) {  // Bytes -not case sensitive
			formatIndex.agent = x-3;
			logbytes=TRUE;
			break;
		}
	for (x=3;x<i;x++)
		if (strstr( format[x], "User-agent")) {  // Agent field
			formatIndex.bytes = x-3;
			break;
		}
	for (x=3;x<i;x++)
		if (strstr( format[x], "Referer")) {  // Referrals
			formatIndex.referer = x-3;
			break;
		}
	if (logbytes==FALSE) MyPrefStruct.filter_zerobyte=0;

}


// ------------------------------------------------------------------------------------------------------



























// ------------------------------------------------------------------------------------------------------

/* ------ SAMPLE LOGS --------

203.127.99.188 - - [02/Mar/1998:17:56:13 +1100] "GET /basketball/images/homecrt.gif HTTP/1.0" 200 1212 "http://www.ozsports.com.au/basketball/Schedule/schedule.R.1997.html" "Mozilla/3.01 (X11; I; SunOS 5.5.1 sun4u)" www.ozsports.com.au
203.9.186.164 - - [02/Mar/1998:17:56:33 +1100] "GET / HTTP/1.0" 304 - "-" "Mozilla/3.01C-PHAROS-KIT  (Win95; I)" www.franchise.net.au

emlyn.bluetongue.com - - [01/Feb/2001:10:24:45 +1100] "GET http://devcentral.iftech.com/images/ads/plugs/djtextlogo3.gif HTTP/1.0" 304 - "http://devcentral.iftech.com/Learning/tutorials/submfc.asp" "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0; DigExt)" penguin.bluetongue.com
emlyn.bluetongue.com - - [01/Feb/2001:10:24:46 +1100] "GET http://devcentral.iftech.com/Learning/tutorials/submfc.asp HTTP/1.0" 200 15156 "http://devcentral.iftech.com/Learning/tutorials/" "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0; DigExt)" penguin.bluetongue.com
emlyn.bluetongue.com - - [01/Feb/2001:10:24:47 +1100] "GET http://ww2.hitbox.com/worldImages/wss_pow.gif HTTP/1.0" 302 299 "http://devcentral.iftech.com/Learning/tutorials/submfc.asp" "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0; DigExt)" penguin.bluetongue.com
emlyn.bluetongue.com - - [01/Feb/2001:10:24:47 +1100] "GET http://www.hitbox.comworldimages/wss_pow.gif HTTP/1.0" 500 482 "http://devcentral.iftech.com/Learning/tutorials/submfc.asp" "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0; DigExt)" penguin.bluetongue.com
emlyn.bluetongue.com - - [01/Feb/2001:10:24:47 +1100] "GET http://w109.hitbox.com/Hitbox?hb=W96901132951&bn=MSIE&bv=400&ss=1152*864&sc=32&dt=11&sv=13&ja=y&rf=http%3A//devcentral.iftech.com/Learning/tutorials/&ln=&pl=&cd=1&bt=0 HTTP/1.0" 302 0 "http://devcentral.iftech.com/Learning/tutorials/submfc.asp" "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0; DigExt)" penguin.bluetongue.com
emlyn.bluetongue.com - - [01/Feb/2001:10:24:51 +1100] "GET http://www.burstnet.com/cgi-bin/ads/ad4020a.cgi/2966/RETURN-CODE HTTP/1.0" 200 272 "http://devcentral.iftech.com/Learning/tutorials/submfc.asp" "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0; DigExt)" penguin.bluetongue.com
*/





// ---------------------------------------------------------------------------
// Handles standard commonlog format and Combined log format files , HAH, standard MY ASSS
// This is the most unstandard butchered log file in existence, with toooo many exceptions and 
// custom mods by morons around the planet! W3C ROCKS
short PROC_NCSA(char *buffer,HitDataPtr Line )
{
	register char 	ch,delim, *p;
	register short	once=1,second=0,nudge=1,i=1, wn=0, cnt=0, ref=8, agt=9, date,tot, off;

	//tokenise line delimited by various tokens ' ', [] and "
	format[i++]=buffer;
	delim = ' ';
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == delim) {
			//protects client buffer over run (DW)
			if ( i == 2 ){
				if ( buffer - format[1] > 255 )
					return 0;
			}
			//make sure quoted strings are not in URL
			if (once && *(buffer-3)!='-' ) {
				if ( delim == '\"' ) {
					if ( !isdigit(buffer[2]) && buffer[2]!='-' && buffer[1]!=' ' ) {
						buffer++;
						continue; //ignores quote within URL string which comes first
					} else {
						once=0;
						second = 1;
					}
				}
			} else
			if ( second ) {
				if ( delim == '\"' ) {
					if ( (*(buffer+2)) != '\"') {
						buffer++;
						continue; //ignores quote within referral string
					} else
						second=0;
				}
			}
			*buffer=0;
			if (*(buffer+1)==0) { //prevents buffer overrun
				format[i] = buffer+1;
				break;
			} else {
				p = buffer+nudge;
				format[i] = p;
				buffer+=nudge;
			}
			ch = *format[i];
			//get the next delimiter
			if (ch=='\"' || ch==96 ) { 	//remove " from string
				//if ( *(buffer+1) == '\"' )	ch=34;  (NOIDEA)
				if (ch==96) cnt++;
				format[i]++;
				buffer++;
				delim=ch;
				nudge=2;
			} else
			if (ch=='[') { //remove [ from string
				format[i]++;
				buffer++;
				delim=']';
				nudge=2;
				date = i;
			} else
			if (ch=='<') {
				format[i]++;
				buffer++;
				delim='>';
				wn=true;
				nudge=2;
			} else {
				if (*buffer == ' ') {	// ignore damaged lines with more than 1 space
					if (*(buffer+1)== '-') { //for bad quid pro quo common log format
						format[i]++;
						buffer++;
						delim=' ';
						nudge=1;
					} else
					if (*(buffer+1)== '[') {
						format[i] +=2;
						buffer+=2;
						delim=']';
						nudge=2;
					} else
						return 0;
				} else {
					delim=' ';
					nudge=1;
				}
			}
			i++;
		} else
			buffer++;
	}
	logDelim = tot = i;
	//check for minimum tokens for the format (7)
	if (i<8 || i>28)
		return 0;
	ch = static_cast<char>(i);
	while( ch<11 ) format[ch++] = NULL;

	//----------------------------------------------------
	//now check if the VD is at the start on some
	//modified common log formats
	if ( *format[4]=='-' && *format[3]=='-' )
	{
		Line->vhost = format[1];
		// Rotate DOWN all the pointers in FORMAT[]
		for(off=1;off<tot;off++)	format[off] = format[off+1];
	} else {
		Line->vhost = format[2];
		off = 0;
	}

	
	//----------------------------------------------------
	// Domino Log Handeling for spaces in username  ie
	// 10.1.45.166 sseddt01 CN=Rod Stauffer/O=Net [12/Mar/2001:13:59:45 +0700] "GET /domdoc/radardevelopmentlib.nsf/MyHomePage?OpenForm&ForceHomePageOpen=True HTTP/1.1" 200 32003 "http://sseddt01/domdoc/radardevelopmentlib.nsf/WebNavigationPanel?OpenForm&Page=MHP" "Mozilla/4.0 (compatible; MSIE 5.5; Windows 95)"
	if ( date >=5 && *format[3] != '-' )	// if we do have a valid USER then check if its has spaces...
	{
		if ( date == 5 )
		{
			p = mystrchr( format[3], 0 );		// fill the 0x00 with ' ' spaces
			if ( p )
				*p = ' ';
			// Rotate DOWN all the pointers in FORMAT[]
			for( off=4;off<tot;off++ ) format[off] = format[off+1];
			tot--;
		} else
		if ( date == 6 )
		{
			if ( (p = mystrchr( format[3], 0 )) ) *p = ' ';
			if ( (p = mystrchr( format[4], 0 )) ) *p = ' ';
			// Rotate DOWN all the pointers in FORMAT[] by 2
			for( off=4;off<tot;off++ ) format[off] = format[off+2];
			tot-=2;
		} else
		// Handles 3 spaces in names
		if ( date == 7 )
		{
			if ( (p = mystrchr( format[3], 0 )) ) *p = ' ';
			if ( (p = mystrchr( format[4], 0 )) ) *p = ' ';
			if ( (p = mystrchr( format[5], 0 )) ) *p = ' ';
			// Rotate DOWN all the pointers in FORMAT[] by 3
			for( off=4;off<tot;off++ ) format[off] = format[off+3];
			tot-=3;
		}
	}
	/////////////////////////////////////////////////////////
	

	Line->clientaddr = format[1];
	
	//extract user info
	if ( Line->user = p = format[3] ) {
		if ((format[4]-format[3])>128 || *p == '-' )
			Line->user = 0;		
	}
	//extract date from [date:time GMT] token
	p = format[4];
	Line->date = ConvLDate( p, Line->newdate );
	if (!Line->date || *Line->date==0 ) return 0;
	if ( p ) {
		while (*p!=':') p++;
		*p++=0;
		// verify time format is vaguely ok, if its real bad, error out
		if ( p[2] == ':' )
			format[4] = p;
		else
			return 0;
	}
	//extract time token
	Line->time = format[4];
	// extract url from "get url type" whether type is present or not
	// "GET /syshelp/socks.htm HTTP/1.1"

	i = 20;
	format[i++] = p = format[5];
	while (*p && i<23)
	{
		if ( *p == ' ' )
		{
			*p++ = 0;
			format[i++] = p;
		} else
			p++;
	}
	Line->method = format[20];
	Line->file = format[21];
	Line->protocol = format[22];

	Line->stat = format[6];
	Line->bytes = myatoi(format[7]);

	//swap agent and referral if cnt>3
	if (cnt>1) { //must be crappy Oracle format
		agt=8;
		ref=9;
	} else if (wn) {
		agt=9;
		ref=10;
	}

	if ( Line->refer = p = format[ref] )
	{
		if ( *p == '-' )
			Line->refer = 0;
		else
		// Handle SAMBAR log files which are busted, damn morons
		// eg : 203.111.20.141 - - [03/Nov/2000:12:43:55 +1000] "GET /syshelp/socks.htm HTTP/1.1" 200 4124 20 "http://ice:81/syshelp/pro.htm" "Mozilla/4.0 (compatible; MSIE 5.5; Windows NT 5.0)"
		if ( isdigit(*p) )
		{
			// Rotate DOWN all the pointers in FORMAT[]
			for(off=ref;off<tot;off++)	format[off] = format[off+1];
			tot--;
			Line->refer = p = format[ref];
		}
	}

	Line->agent = format[agt];
	//need offset test so it doesn;t go
	//through if vd is already set
	// Handle BlueTonge logs, ie vhost at the end;
	// eg : butcherbird.uq.net.au - - [19/Feb/1999:10:51:01 +1100] "GET /cricket/about/cricket_exp.html HTTP/1.0" 200 61301 "http://www.ozsports.com.au/cricket/about/" "Mozilla/4.04 [en] (Win95; I ;Nav)" www.ozsports.com.au
	if ( tot>= 11 && off==0) 
	{
		Line->vhost = format[tot-1];
		//"cookie_id=194.168.30.220.27748985791302292"
		//check if the last field is really a cookie and not a Vhost.
		if ( p = mystrchr( Line->vhost, '=' ) ){
			Line->cookie = Line->vhost;
			Line->vhost = NULL;
		}
	}

	// if the vhost is empty, null it
	if ( p = Line->vhost  ) 
	{
		if ( *p=='-' || *p<32 )
			Line->vhost = 0;
	}
	
	
	// handle URLs which contain the virtual host information
	// ie  http://apple.com:80/index.html
	// NOTE: When adding to a V5 database, we always do this regardless of whether or not virtual host reporting is on.
	if( ( MyPrefStruct.multivhosts==1 || MyPrefStruct.database_active && MyPrefStruct.database_extended ) && !Line->vhost )
	{
		if ( !strcmpd( "http://", Line->file ) ) 
		{
			char *col;
			p = mystrchr( Line->file+7, '/' );
			if ( col = mystrchr( p, ':' ) )
			{
				*col = 0;
				Line->vhost = Line->file+7;
				Line->file = p;
			} else {
				strcpyuntil( Line->file, Line->file+7, '/' );
				Line->vhost = Line->file;
				Line->file = p;
			}

		}
	}
	return 1;
}

//
// You can call is for COMMON only logs , other wise NCSA can handel it, though this would be faster...
//
short PROC_Common(char *buffer,HitDataPtr Line )
{
	register char 	ch,delim, *p;
	register short	once=1,second=0,nudge=1,i=1, wn=0, cnt=0, ref=8, agt=9, date,tot, off;

	//tokenise line delimited by various tokens ' ', [] and "
	format[i++]=buffer;
	delim = ' ';
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == delim) {
			//protects client buffer over run (DW)
			if ( i == 2 ){
				if ( buffer - format[1] > 255 )
					return 0;
			}
			//make sure quoted strings are not in URL
			if (once && (*(buffer-3)!='-') ) {
				if ( delim == '\"' ) {
					if ( (!isdigit(buffer[2])) && (buffer[2]!='-') && (buffer[1]!=' ') ) {
						buffer++;
						continue; //ignores quote within URL string which comes first
					} else {
						once=0;
						second = 1;
					}
				}
			} else
			if ( second ) {
				if ( delim == '\"' ) {
					if ( (*(buffer+2)) != '\"') {
						buffer++;
						continue; //ignores quote within referral string
					} else
						second=0;
				}
			}
			*buffer=0;
			if (*(buffer+1)==0) { //prevents buffer overrun
				format[i] = buffer+1;
				break;
			} else {
				p = buffer+nudge;
				format[i] = p;
				buffer+=nudge;
			}
			ch = *format[i];
			//get the next delimiter
			if (ch=='\"' || ch==96 ) { 	//remove " from string
				//if ( *(buffer+1) == '\"' )	ch=34;  (NOIDEA)
				if (ch==96) cnt++;
				format[i]++;
				buffer++;
				delim=ch;
				nudge=2;
			} else
			if (ch=='[') { //remove [ from string
				format[i]++;
				buffer++;
				delim=']';
				nudge=2;
				date = i;
			} else
			if (ch=='<') {
				format[i]++;
				buffer++;
				delim='>';
				wn=true;
				nudge=2;
			} else {
				if (*buffer == ' ') {	// ignore damaged lines with more than 1 space
					if (*(buffer+1)== '-') { //for bad quid pro quo common log format
						format[i]++;
						buffer++;
						delim=' ';
						nudge=1;
					} else
					if (*(buffer+1)== '[') {
						format[i] +=2;
						buffer+=2;
						delim=']';
						nudge=2;
					} else
						return 0;
				} else {
					delim=' ';
					nudge=1;
				}
			}
			i++;
		} else
			buffer++;
	}
	logDelim = tot = i;
	//check for minimum tokens for the format (7)
	if (i<8 || i>28)
		return 0;
	ch = static_cast<char>(i);
	while( ch<11 ) format[ch++] = NULL;

	//now check if the VD is at the start on some
	//modified common log formats
	if (*format[3]=='-' && *format[4]=='-') {
		off=1; //offset other values
		Line->vhost = format[1];
	} else {
		off=0; //offset other values
		Line->vhost = format[2];
	}
	
	Line->clientaddr = format[1+off];
	
	//extract user info
	if ( Line->user = p = format[3+off] ) {
		if ((format[4+off]-format[3+off])>128 || *p == '-' )
			Line->user = 0;		
	}
	//extract date from [date:time GMT] token
	p = format[4+off];
	Line->date = ConvLDate( p, Line->newdate );
	if (!Line->date || *Line->date==0 ) return 0;
	if ( p ) {
		while (*p!=':') p++;
		*p++=0;
		// verify time format is vaguely ok, if its real bad, error out
		if ( p[2] == ':' )
			format[4+off] = p;
		else
			return 0;
	}
	//extract time token
	Line->time = format[4+off];
	// extract url from "get url type" whether type is present or not
	Line->method = p = format[5+off];
	i=31;
	while (*p) {
		if (*p==' ') {
			*p=0;
			format[i]=p+1;
			p++;
			if (*p==' ' || i>33) //ignore damaged lines with >1 space
				return 0;
			i++;
		} else
			p++;
	}
	Line->file = format[31];
	Line->stat = format[6+off];
	Line->bytes = myatoi(format[7+off]);

	// if the vhost is empty, null it
	if ( p = Line->vhost  ) {
		if ( *p=='-' || *p<32 )
			Line->vhost = 0;
	}
	

	// handle URLs which contain the virtual host information
	// ie  http://apple.com:80/index.html
	// NOTE: When adding to a V5 database, we always do this regardless of whether or not virtual host reporting is on.
	if( ( MyPrefStruct.multivhosts==1 || MyPrefStruct.database_active && MyPrefStruct.database_extended ) && !Line->vhost )
	{
		if ( !strcmpd( "http://", Line->file ) )
		{
			char *col;
			p = mystrchr( Line->file+7, '/' );
			if ( col = mystrchr( p, ':' ) )
			{
				*col = 0;
				Line->vhost = Line->file+7;
				Line->file = p;
			} else {
				strcpyuntil( Line->file, Line->file+7, '/' );
				Line->vhost = Line->file;
				Line->file = p;
			}

		}
	}
	return 1;
}


//--------------------------------------------------------
//
// Netscape Corporation
//
// Sample LOG:
/*
format=%Ses->client.ip% - %Req->vars.pauth-user% [%SYSDATE%] "%Req->reqpb.proxy-request%" %Req->srvhdrs.clf-status% %Req->vars.p2c-cl% "%Req->headers.user-agent%" %Req->headers.unverified-user% %Req->vars.remote-status% %Req->vars.r2p-cl% %Req->headers.content-length% %Req->vars.p2r-cl% %Req->vars.c2p-hl% %Req->vars.p2c-hl% %Req->vars.p2r-hl% %Req->vars.r2p-hl% %Req->vars.xfer-time%
10.140.6.139 - - [06/Dec/1999:00:31:50 +1100] "GET /  HTTP/1.1" 401 223 "Mozilla/4.0 (compatible; MSIE 4.01; Windows 95)" - 401 223 223 0 14 182 449 182 0
10.140.6.139 - - [06/Dec/1999:00:32:27 +1100] "GET /  HTTP/1.1" 401 223 "Mozilla/4.0 (compatible; MSIE 4.01; Windows 95)" - 401 223 223 0 14 182 298 182 0
10.140.6.139 - - [06/Dec/1999:00:33:25 +1100] "GET /  HTTP/1.1" 401 223 "Mozilla/4.0 (compatible; MSIE 4.01; Windows 95)" - 401 223 223 0 14 182 298 182 0
*/
short PROC_Netscape(char *buffer,HitDataPtr Line)
{
	register char	delim, *p;
	register short 	once=1,nudge=1,i=1;

	//tokenise line delimited by various tokens ' ', [] and "
	format[i++]=buffer;
	delim = ' ';
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == delim) {
			//make sure quoted strings are not in URL
			if (once) {
				if ( delim == '\"') {
					if (!isdigit(*(buffer+2))) {
						buffer++;
						continue; //ignores quote within URL string
					} else
						once=0;
				}
			}
			*buffer=0;
			if (*(buffer+1)==0) {	//prevents buffer overrun
				format[i] = buffer+1;
				buffer++;
			} else {
				format[i] = buffer+nudge;
				buffer+=nudge;
			}
			//get the next delimiter
			if (*format[i]=='\"') { 	//remove " from string
				format[i]++;
				buffer++;
				delim='\"';
				nudge=2;
			} else if (*format[i]=='[') { //remove [ from string
				format[i]++;
				buffer++;
				delim=']';
				nudge=2;
			} else {
				if (*buffer == ' ' && *(buffer+1)==' ')	// ignore damaged lines with more than 1 space
					return 0;
				delim=' ';
				nudge=1;
			}
			i++;
		} else
			buffer++;
	}
	logNumDelimInLine = i;
	//check for correct number of tokens for the format
	if (i!=logDelim)
		return 0;

	if ( i<11 )
		MemClr( &format[i], (19-i)*4 );

	//extract date from [date:time GMT] token
	p = format[formatIndex.Date()];
	if ( p ) {
		while (*p!=':') p++;
		*p = 0;

		p++;
		Line->time = p;
		while (*p!=' ') p++;
		*p = 0;
	}
	Line->date = ConvLDate( format[formatIndex.Date()], Line->newdate );
	if (!Line->date || *Line->date==0 )
		return 0;
	Line->clientaddr = format[formatIndex.time];
	Line->stat = format[formatIndex.status];

	if ( formatIndex.HostName() != -1 ) {
		// extract url from "get url type" whether type is present or not
		long idx( formatIndex.HostName() );
		format[18] = p = format[idx];
		i=19; //start index of spare format record
		while (*p) {
			if (*p==' ') {
				*p=0;
				p++;
				if (*p==' ') p++;
				format[i]=p;
				if (*p==' ' || i>33) //ignore damaged lines with >1 space
					return 0;
				i++;
			} else
				p++;
		}
		format[idx] = p;

		if ( Line->method = format[18] )
			Line->file = format[19];

		else Line->file = 0;
	} else
		Line->file = 0;

	Line->bytes = myatoi(format[formatIndex.url]);
	Line->refer = 0;
	if ( p = format[formatIndex.agent] )
		if ( *p != '-' ) Line->refer = p;

	Line->agent = 0;
	if ( p = format[formatIndex.bytes] )
		if ( *p != '-' ) Line->agent = p;

	Line->user = 0;
	if ( p = format[formatIndex.referer] )
		if ( *p != '-' ) Line->user = p;

	Line->vhost = 0;
	if ( p = format[formatIndex.user] )
		if ( *p != '-' ) Line->vhost = p;

	if ( Line->cookie = format[formatIndex.cookie] )
		if ( *Line->cookie == '-' ) Line->cookie = 0;

	return 1;
}


short PROC_WUFTPD( char *buffer,HitDataPtr Line)
{
	short i=1, sp = 0;
	
	//Tokenise line into format fields 1 to 32 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == ' ') {
			sp++;
			if ( sp >=6 ){
				format[i++] = buffer+1;
				*buffer=0;
			}
		}
		buffer++;
	}
	//check if it is a valid format first
	//if i<10 not enough fields
	logNumDelimInLine = i;
	logDelim = i;
	if (i<6)
		return 0;
	while( i<10 ) format[i++] = 0;

	Line->date = ConvLongDate(format[1]);
	if (!Line->date || *Line->date==0 )
		return 0;
	Line->time = format[1]+12;
	Line->stat = 0;
	Line->clientaddr = format[3];
	Line->user = format[10];
	Line->file = format[5];
	if ( format[8] && *format[8] == 'i' ){		// data in
		Line->bytesIn = myatoi(format[4]);
		Line->bytes = 0;
	} else {
		Line->bytesIn = 0;
		Line->bytes = myatoi(format[4]);
	}
	Line->agent = 0;
	Line->refer = 0;
	return 1; 
}
  
  
  
  
  
  
  
  
  
  
short PROC_Purveyer(char *buffer,HitDataPtr Line)
{
	char 	delim,*p;		
	short	once=1,nudge=1,i=1;

	//tokenise line delimited by various tokens ' ', [] and "
	format[i++]=buffer;
	delim = ' ';
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == delim) {
			//make sure quoted strings are not in URL
			if (once) {
				if ( delim == '\"') {
					if (!isdigit(*(buffer+2))) {
						buffer++;
						continue; //ignores quote within URL string
					} else
						once=0;
				}
			}
			*buffer=0;
			if (*(buffer+1)==0) {	//prevents buffer overrun
				format[i] = buffer+1;
				buffer++;
			} else {
				format[i] = buffer+nudge;
				buffer+=nudge;
			}
			//get the next delimiter
			if (*format[i]=='\"') { 	//remove " from string
				format[i]++;
				buffer++;
				delim='\"';
				nudge=2;
			} else if (*format[i]=='[') { //remove [ from string
				format[i]++;
				buffer++;
				delim=']';
				nudge=2;
			} else {
				if (*buffer == ' ')	// ignore damaged lines with more than 1 space
					return 0;
				delim=' ';
				nudge=1;
			}
			i++;
		} else
			buffer++;
	}
	logNumDelimInLine = i;
	if ( (logDelim = i) < 20 ) MemClr( &format[i], (20-i)*4 );

	// if date and time is in token [date:time GMT] break them up
	if (formatIndex.time) {
		//extract date in case it is in a [date:time GMT] token
		format[16] = p = format[formatIndex.Date()];
		while (*p!=':') p++;
		*p=0;

		format[formatIndex.Date()] = p;
		//extract time token
		format[17] = p = format[formatIndex.Date()]+1;
		while (*p!=' ') p++;
		*p=0;

		format[formatIndex.Date()] = p;

		Line->date = ConvLDate( format[16], Line->newdate );
		if (*Line->date==0) return 0;
		Line->time = format[17];
	} else {
		Line->date = ConvLDate(format[formatIndex.Date()], Line->newdate);
		if (!Line->date || *Line->date==0 )
			return 0;
		Line->time = format[formatIndex.time];
	}
	
	Line->stat = format[formatIndex.status];
	Line->clientaddr = format[formatIndex.HostName()];

	if ( formatIndex.url ) {
		// extract url from "get url type" whether type is present or not
		format[18] = p = format[formatIndex.url];
		i=19;
		while (*p) {
			if (*p==' ') {
				*p=0;
				format[i]=p+1;
				p++;
				if (*p==' ') //ignore damaged lines with >1 space
					return 0;
				i++;
			} else
				p++;
		}

		format[formatIndex.url] = p;
		if ( Line->method = format[18] )
			Line->file = format[19];
		else
			Line->file = 0;

	} else
		Line->file = 0;

	Line->bytes = myatoi(format[formatIndex.agent]);
	Line->agent = 0;
	if ( p = format[formatIndex.bytes] )
		if ( *p != '-' ) Line->agent = p;

	Line->refer = 0;
	if ( p = format[formatIndex.referer] )
		if ( *p != '-' ) Line->refer = p;

	return 1;

}





// ------------------
// WebStar LOG FORMAT
/*
!!WebSTAR	STARTUP	12/30/96:13:06
!!LOG_FORMAT AGENT BYTES BYTES_SENT CONNECTION_ID CS-HOST CS-IP CS-METHOD CS-STATUS CS-URI DATE FROM HOSTNAME METHOD PATH_ARGS REFERER RESULT SEARCH_ARGS TIME TIME_TAKEN TRANSFER_TIME URL USER
GNNworks/v1.2.0  via proxy gateway  CERN-HTTPD/3.0 libwww/2.17	72218	72218	80	www-b03.proxy.gnn.com.	-	GET	200	:staff:Raul:psx:docs:psx_cheat.txt	12/30/96		www-b03.proxy.gnn.com.	GET			OK  		13:09:04	00:01:19	4749	:staff:Raul:psx:docs:psx_cheat.txt		
Mozilla/3.0 (Win95; I)	0	0	79	cust113.max10.seattle.wa.ms.uu.net.	-	CONDITIONAL_GET	304	:staff:raul:psx:psxchip.html	12/30/96		cust113.max10.seattle.wa.ms.uu.net.	CONDITIONAL_GET			OK  		13:14:23	00:00:00	50	:staff:raul:psx:psxchip.html		
Mozilla/3.0 (Win95; I)	0	0	78	cust113.max10.seattle.wa.ms.uu.net.	-	CONDITIONAL_GET	304	:staff:raul:bg:bkg10.jpg	12/30/96		cust113.max10.seattle.wa.ms.uu.net.	CONDITIONAL_GET		http://www.medfac.unimelb.edu.au/staff/raul/psx/psxchip.html	OK  		13:14:26	00:00:01	64	:staff:raul:bg:bkg10.jpg		
Mozilla/3.0 (Win95; I)	0	0	75	cust113.max10.seattle.wa.ms.uu.net.	-	CONDITIONAL_GET	304	:staff:raul:psx:pics:18pin-80QFP.jpg	12/30/96		cust113.max10.seattle.wa.ms.uu.net.	CONDITIONAL_GET		http://www.medfac.unimelb.edu.au/staff/raul/psx/psxchip.html	OK  		13:14:26	00:00:01	107	:staff:raul:psx:pics:18pin-80QFP.jpg		
Mozilla/3.0 (Win95; I)	0	0	74	cust113.max10.seattle.wa.ms.uu.net.	-	CONDITIONAL_GET	304	:staff:raul:psx:pics:18pin-80QFP1002.jpg	12/30/96		cust113.max10.seattle.wa.ms.uu.net.	CONDITIONAL_GET		http://www.medfac.unimelb.edu.au/staff/raul/psx/psxchip.html	OK  		13:14:27	00:00:00	43	:staff:raul:psx:pics:18pin-80QFP1002.jpg		
Mozilla/3.0 (Win95; I)	0	0	72	cust113.max10.seattle.wa.ms.uu.net.	-	CONDITIONAL_GET	304	:staff:raul:psx:pics:14pin-52QFP.jpg	12/30/96		cust113.max10.seattle.wa.ms.uu.net.	CONDITIONAL_GET		http://www.medfac.unimelb.edu.au/staff/raul/psx/psxchip.html	OK  		13:14:34	00:00:04	288	:staff:raul:psx:pics:14pin-52QFP.jpg		
*/

short PROC_MacHTTP(char *buffer,HitDataPtr Line )
{
	register char 	*ptr;		
	register short	i=1;		

	tabOnEnd = 0;

	//Tokenise line into format fields 1 to 25 from \t
	format[i]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		// Ignore comment lines with !! at the beginning of the line
		if ( *buffer == '!' )  {
			if (*(buffer+1) == '!')
				return LOGLINE_COMMENT;
		}
		if ( *buffer == '\t')
		{
			*buffer=0;
			if ( *(buffer+1) != 0 ) // We don't include the last tab as it gives us a wrong indication of the number of fields
				format[++i] = buffer+1;
			else
				tabOnEnd = 1;
		}
		buffer++;
	}
	//check if it is a valid format first
	//logDelim +1 for some implementations
	//ending in a TAB, then return e.g. WS
	logNumDelimInLine = i-1;
	format[i+1] = format[i+2] = format[i+3] = 0;
//	if ( i != logDelim && i != logDelim+1 )
	if ( logNumDelimInLine != logDelim )
	{
		if ( (logNumDelimInLine+1 == logDelim) && tabOnEnd )
			logNumDelimInLine++;
	}
	if ( logNumDelimInLine != logDelim )
	{
		OutDebugs( "\nLine %d has %d tabs, where we are expecting %d tabs", Line->lineNum, logNumDelimInLine, logDelim );
		
		static char dbgBuf[2048];
		char *p = dbgBuf;
		char *s;
		long count = 0, totalcount = 0, linecount = 0, len;

		for ( short j = 0; (j <= logDelim || j <= logNumDelimInLine) && j < FORMATSIZE; j++ )
		{
			len = mystrlen( formatHeadNames[j] ) + 1 + mystrlen( format[j] );
			if ( len > 100 )
			{
				linecount = 0;
				*p++ = '\n';
			}
			if ( totalcount + len > 2040 )
				break;
			else
			{
				s = format[j];
				if ( s == 0 )
					s = "";
				count = sprintf( p, "%s=%s,\t", formatHeadNames[j], s );
				totalcount += count;
				p += count;
				linecount += count;
				if ( linecount > 100 )
				{
					linecount = 0;
					*p++ = '\n';
				}
			}
		}

		OutDebug( dbgBuf ); 
		return LOGLINE_ERROR;
	}

	//the less checking here the better
	Line->date = ConvLDate( format[formatIndex.Date()], Line->newdate );
	if (!Line->date || *Line->date==0 )
		return LOGLINE_ERROR;

	Line->time = format[formatIndex.time];
	Line->stat = format[formatIndex.status];

	Line->clientaddr = format[formatIndex.HostName()];

	if ( Line->clientaddr ) {
		if ( *(Line->clientaddr) == '-' ) {
			Line->clientaddr = format[formatIndex.csip];
		}
	}
	
	Line->file = format[formatIndex.url];
	if ( !*Line->file ) Line->file = 0;
	
	if (ptr=format[formatIndex.agent]) {
		if (*ptr=='\"') {
			ptr++;
			format[formatIndex.agent]=ptr;
			while (*ptr!='\"') ptr++;
			*ptr=0;
		}
		Line->agent = format[formatIndex.agent];
	} else
		Line->agent = 0;
	Line->bytes = myatoi(format[formatIndex.bytes]);

	if (ptr=format[formatIndex.referer]) {
		if (*ptr=='\"') {
			ptr++;
			format[formatIndex.referer]=ptr;
			while (*ptr!='\"') ptr++;
			*ptr=0;
		}
		Line->refer = format[formatIndex.referer];
	} else
		Line->refer = 0;
	
	Line->user = format[formatIndex.user];

	Line->vhost = 0;
	if (ptr=format[formatIndex.cshost]) {
		//strip quotes if present
		if (*ptr=='\"') {
			ptr++;
			format[formatIndex.cshost]=ptr;
			while (*ptr!='\"') {
				//strip out User-Agent: which is a bug in WebStar HostField
				if (*ptr==':' && *(ptr-10)=='U') {
					ptr-=10;
					break;
				}
				ptr++;
			}
			//while (*ptr!='\"' && *ptr!=':') ptr++;
			*ptr=0;
		}
		Line->vhost = format[formatIndex.cshost];
		if ( *Line->vhost == 0 )
			Line->vhost = 0;
	}
	
	if (ptr=format[formatIndex.query]) {
		if (*ptr!='-') {	
			if (*ptr=='\"') {
				ptr++;
				format[formatIndex.query]=ptr;
				while (*ptr!='\"') ptr++;
				*ptr=0;
			}
			if (*format[formatIndex.query]) {
				mystrcpy(buffer,Line->file);
				mystrcat(buffer,"?");
				mystrcat(buffer,format[formatIndex.query]);
				Line->file=buffer;
			}
		}
	}
	if ( Line->cookie = format[formatIndex.cookie] )
		if ( *Line->cookie == '-' ) Line->cookie = 0;
	
	return LOGLINE_CORRECT;
}


















short PROC_Website(char *buffer,HitDataPtr Line )
{
	char *p, *date, *time;
	short i=1;	
	
	//Tokenise line into format fields 1 to 32 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		// ignore damaged lines with !! in line string
		if ( *buffer == '!' )  {
			if (*(buffer+1) == '!')
				return -1;
		}
		if ( *buffer == '\t') {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	//check if it is a valid format first
	//if i<10 not enough fields
	logNumDelimInLine = i;
	logDelim = i;
	if (i<11)
		return 0;

	//extract date and time from "date time" token
	date = format[1];
	p = format[1];
	while (*p!=' ' && *p!=0)
		p++;
	*p=0;
	//extract time token
	time = p+1;
	
	Line->date = ConvLDate(date, Line->newdate);
	if (!Line->date || *Line->date==0 )
		return 0;
	Line->time = time;
	Line->stat = format[11];
	Line->clientaddr = format[2];
	Line->user = format[5];
	Line->vhost = format[3];
	Line->file = format[7];
	Line->bytes = myatoi(format[12]);
	Line->agent = format[10];
	Line->refer = p = format[8];

	if ( p )
		if ( *p == '-' ) Line->refer = 0;
	return 1; 
}







// ----------------------- IIS --------------------------
/*
157.228.22.42, -, 1/11/00, 12:59:50, W3SVC3, TRIANGLE, 194.128.98.202, 375, 429, 2434, 200, 0, GET, /Default.htm, -,
157.228.22.42, -, 1/11/00, 12:59:50, W3SVC3, TRIANGLE, 194.128.98.202, 281, 437, 2643, 200, 0, GET, /home.htm, -,
157.228.22.42, -, 1/11/00, 12:59:50, W3SVC3, TRIANGLE, 194.128.98.202, 297, 437, 2129, 200, 0, GET, /menu.htm, -,
157.228.22.42, -, 1/11/00, 12:59:50, W3SVC3, TRIANGLE, 194.128.98.202, 563, 445, 319, 200, 0, GET, /image/bullet.gif, -,
157.228.22.42, -, 1/11/00, 12:59:55, W3SVC3, TRIANGLE, 194.128.98.202, 4969, 443, 5107, 200, 0, GET, /image/Symp.gif, -,
157.228.22.42, -, 1/11/00, 13:00:15, W3SVC3, TRIANGLE, 194.128.98.202, 24312, 446, 1453, 200, 0, GET, /image/home-up.gif, -,
*/


short PROC_IIS(char *buffer,HitDataPtr Line )
{
	register char  c, *p;
	register short i=1;
	
	//Tokenise line into format fields 1 to 25 from ','
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == ',') {
			if (*(buffer+1)==0)	//prevents buffer overrun
				format[i++] = buffer+1;
			else
				format[i++] = buffer+2;

			*buffer=0;
		}
		buffer++;
	}
//	for (int j=i;j<FORMATSIZE*3;++j)
//		format[j] = 0;

	logNumDelimInLine = i;
	logDelim = i;
	//check for minimum tokens for the format (10)
	if (i<11 || i>25)
		return 0;
	if ( i < 18 ) MemClr( &format[i], (18-i)*sizeof(void*) );


	Line->date = ConvFloatingDate( format[3], Line->newdate, MyPrefStruct.forceddmm );
	if (!Line->date || *Line->date==0 )
		return 0;
	Line->time = format[4];
	Line->clientaddr = format[1];
	Line->user = format[2];
	Line->vhost = format[7];
	Line->ms = myatoi(format[8]);
	Line->bytesIn = myatoi(format[9]);
	Line->bytes = myatoi(format[10]);
	Line->stat = format[11];

	Line->file = p = format[14];
	if ( p ) {
		c = *p;
		if ( c == '-' || c == 0) Line->file = 0;
	}
	if ( p = format[15] ){
		if ( *p != '-' ){
			*(p-2) = '?';
			strcpy( p-1, p );
		}
	}

	Line->refer = p = format[16];
	if ( p ) {
		c = *p;
		//isdigit removes coordinates for server map
		if (isdigit(c) || c == '-' || c == 0) Line->refer = 0;
	}
	Line->agent = p = format[17];
	if ( p ) {
		if (strcmp(p, "if") == 0)
		{
			int sdf = 0;
		}

		c = *p;
		if (c=='-') {
			Line->agent = p = format[15];
			if ( p ) {
				c = *p;
				if (isdigit(c) || c=='-' || c == 0) Line->agent = 0;
			}
		} else {
			//isdigit removes coordinates for server map
			if (isdigit(c) || c == 0) Line->agent = 0;
		}
	}

	return 1;
}








// --------------------------------------------- W3C ----------------------------------------------------
char *W3C_AddField( char *pStr, const char *data )
{
	*pStr++ = '\t';
	if ( data && *data )
	{
		char *tp = pStr;
		pStr += mystrcpy( pStr, data );
		for( ; *tp; tp++ ) if ( *tp == ' ' ) *tp = '+';
	} else
		*pStr++ = '-';
	return pStr;
}

char *W3C_AddNumericField( char *pStr, const long data )
{
	*pStr++ = '\t';
	pStr += sprintf( pStr, "%d", data );
	return pStr;
}

//converts to extended date format of the form MM/DD/YYYY to YYYY-MM-DD
//"--98/12/31
char *ConvUS2EDate( const char *date, char *out )
{
	register char *buffPtr = out;
	
	if (!date ) return 0;

	*buffPtr++ = date[6];
	*buffPtr++ = date[7];
	*buffPtr++ = date[8];
	*buffPtr++ = date[9];
	*buffPtr++ = '-';
	*buffPtr++ = date[0];
	*buffPtr++ = date[1];
	*buffPtr++ = '-';
	*buffPtr++ = date[3];
	*buffPtr++ = date[4];
	*buffPtr = 0;

	return(buffPtr);
}


static 
char *ConvertParamtoURL(char *param, char *newParam)
{
	char	*s, *d, c=0, lc=0;
	long	len,outlen=0;

	d = newParam;
	s = param;

	if ( s && d ){
		while( (c=*s) && (outlen<10000) ) {
			s++;
			if( c < 33 ) {
				if ( c != lc ) {
					len = sprintf( d, "%%%02x", c );
					d+= len;
					outlen+= len;
				}
			} else {
				*d++ = c;
				outlen++;
			}
			lc = c;
		}
		*d = 0;
	}

	return newParam;
}


#include "EngineStatus.h"
static FILE *w3c_fp;


void Close_ConvertW3C( void )
{
	if ( w3c_fp )
	{
		fclose( w3c_fp );
		w3c_fp = NULL;
	}
}

// Output hitdata line into a w3c format line.
short Write_W3C( char *filename, HitDataPtr Line )
{
	{
		char lineString[10240], *p;
		if ( !Line )
		{
			if ( filename && *filename )
			{
				w3c_fp = (FILE*)AddLineToFile( filename, NULL );
				AddLineToFile( NULL, "#Software: iReporter generated log file.\n" );
				AddLineToFile( NULL, "#Version: 1.0\n" );

				char date[64];
				DaysDateToString( GetCurrentCTime(), date, 2, '/', 1,1 );
				sprintf( lineString, "#Date: %s\n", date );
				AddLineToFile( NULL, lineString );
				//CR-5189
				AddLineToFile( NULL, "#Fields: date\ttime\ts-ip\tc-ip\tcs-username\tcs-method\tcs-uri-stem\tcs-uri-query\tsc-status\tsc-bytes\tcs-bytes\ts-port\tcs(User-Agent)\tcs(Referer)\tcs(Cookie)\n" );
				//AddLineToFile( NULL, "#Fields: date\ttime\ts-computername\tc-ip\tcs-username\tcs-method\tcs-uri-stem\tcs-uri-query\tsc-status\tsc-bytes\tcs-bytes\ts-port\tcs(User-Agent)\tcs(Referer)\tcs(Cookie)\n" );
			} else
				fclose( w3c_fp );
		} else
		{
			p = lineString;
			p = ConvUS2EDate( Line->date, p );

			p = W3C_AddField( p, Line->time );
			p = W3C_AddField( p, Line->vhost );

			p = W3C_AddField( p, Line->clientaddr );
			p = W3C_AddField( p, Line->user );

			p = W3C_AddField( p, Line->method );

			if ( Line->file  )
			{
				char tmpStr[10240], *qMark;
				ConvertParamtoURL( Line->file, tmpStr );
				if ( qMark = mystrchr( tmpStr, '?' ) )
				{
					*p = 0;
					p = W3C_AddField( p, tmpStr );
					p = W3C_AddField( p, qMark );
				} else {
					p = W3C_AddField( p, tmpStr );
					p = W3C_AddField( p, 0 );
				}
			} else
			{
				p = W3C_AddField( p, 0 );
				p = W3C_AddField( p, 0 );
			}

			p = W3C_AddField( p, Line->stat );
			p = W3C_AddNumericField( p, Line->bytes );
			p = W3C_AddNumericField( p, Line->bytesIn );
			p = W3C_AddNumericField( p, Line->port );
			p = W3C_AddField( p, Line->agent );
			p = W3C_AddField( p, Line->refer );
			p = W3C_AddField( p, Line->cookie );

			*p++ = '\n';
			*p++ = 0;

			AddLineToFile( NULL, lineString );

			MemClr( Line, sizeof( HitDataRec ) );
			return 1;
		}
	}
	return 0;
}


/* SAMPLE W3C ISA LOG FORMAT
#Software: Microsoft(R) Internet Security and Acceleration Server 2000
#Version: 1.0
#Date: 2001-09-24 03:58:15
#Fields: c-ip	cs-username	c-agent	date	time	s-computername	cs-referred	r-host	r-ip	r-port	time-taken	cs-bytes	sc-bytes	cs-protocol	s-operation	cs-uri	s-object-source	sc-status
10.20.7.254	anonymous	MSMSGS	2001-09-24	03:58:15	KENNY	-	direct.ninemsn.com.au	202.58.56.36	80	656	426	600	http	GET	http://direct.ninemsn.com.au/scripts/accipiter/jserver/cat=technology/site=NINEMSN.MESSENGER/area=GROUP3/loc=top/aamsz=MESSENGERBOX	Inet	200
10.20.7.254	anonymous	MSMSGS	2001-09-24	03:58:29	KENNY	-	direct.ninemsn.com.au	202.58.56.36	80	109	426	600	http	GET	http://direct.ninemsn.com.au/scripts/accipiter/jserver/cat=technology/site=NINEMSN.MESSENGER/area=GROUP2/loc=top/aamsz=MESSENGERBOX	Inet	200
10.20.7.254	anonymous	MSMSGS	2001-09-24	03:59:00	KENNY	-	direct.ninemsn.com.au	202.58.56.36	80	125	426	600	http	GET	http://direct.ninemsn.com.au/scripts/accipiter/jserver/cat=technology/site=NINEMSN.MESSENGER/area=GROUP1/loc=top/aamsz=MESSENGERBOX	Inet	200
10.20.7.254	anonymous	MSMSGS	2001-09-24	04:02:00	KENNY	-	direct.ninemsn.com.au	202.58.56.36	80	109	426	600	http	GET	http://direct.ninemsn.com.au/scripts/accipiter/jserver/cat=technology/site=NINEMSN.MESSENGER/area=GROUP3/loc=top/aamsz=MESSENGERBOX	Inet	200
10.20.7.254	anonymous	MSMSGS	2001-09-24	04:02:15	KENNY	-	direct.ninemsn.com.au	202.58.56.36	80	140	426	600	http	GET	http://direct.ninemsn.com.au/scripts/accipiter/jserver/cat=technology/site=NINEMSN.MESSENGER/area=GROUP2/loc=top/aamsz=MESSENGERBOX	Inet	200
10.20.7.254	anonymous	MSMSGS	2001-09-24	04:02:44	KENNY	-	direct.ninemsn.com.au	202.58.56.36	80	110	426	600	http	GET	http://direct.ninemsn.com.au/scripts/accipiter/jserver/cat=technology/site=NINEMSN.MESSENGER/area=GROUP1/loc=top/aamsz=MESSENGERBOX	Inet	200
10.20.7.254	anonymous	MSMSGS	2001-09-24	04:05:45	KENNY	-	direct.ninemsn.com.au	202.58.56.36	80	93	426	600	http	GET	http://direct.ninemsn.com.au/scripts/accipiter/jserver/cat=technology/site=NINEMSN.MESSENGER/area=GROUP3/loc=top/aamsz=MESSENGERBOX	Inet	200
10.20.7.254	anonymous	MSMSGS	2001-09-24	04:05:59	KENNY	-	direct.ninemsn.com.au	202.58.56.36	80	78	426	600	http	GET	http://direct.ninemsn.com.au/scripts/accipiter/jserver/cat=technology/site=NINEMSN.MESSENGER/area=GROUP2/loc=top/aamsz=MESSENGERBOX	Inet	200


#Software: Microsoft Internet Information Server 4.0
#Version: 1.0
#Date: 1999-03-01 00:16:15
#Fields: date time c-ip cs-username s-sitename s-computername s-ip cs-method cs-uri-stem cs-uri-query sc-status sc-win32-status sc-bytes cs-bytes time-taken s-port cs-version cs(User-Agent) cs(Cookie) cs(Referer)
1999-03-01 00:16:15 128.2.18.152 - W3SVC2 RAPIDHOSTNT1 194.74.51.37 GET /Default.htm - 200 0 5533 361 3078 80 HTTP/1.0 Mozilla/4.0+(compatible;+MSIE+4.01;+Windows+NT) - http://search.excite.com/search.gw?search=mergers+and+acquisitions+samples
1999-03-01 00:16:15 128.2.18.152 - W3SVC2 RAPIDHOSTNT1 194.74.51.37 GET /1998.gif - 200 0 3384 195 828 80 HTTP/1.0 Mozilla/4.0+(compatible;+MSIE+4.01;+Windows+NT) - http://www.clarusresearch.com/
1999-03-01 00:16:15 128.2.18.152 - W3SVC2 RAPIDHOSTNT1 194.74.51.37 GET /strategybase.gif - 200 0 2212 203 282 80 HTTP/1.0 Mozilla/4.0+(compatible;+MSIE+4.01;+Windows+NT) - http://www.clarusresearch.com/
1999-03-01 00:16:16 128.2.18.152 - W3SVC2 RAPIDHOSTNT1 194.74.51.37 GET /Go.gif - 200 0 1275 193 329 80 HTTP/1.0 Mozilla/4.0+(compatible;+MSIE+4.01;+Windows+NT) - http://www.clarusresearch.com/
1999-03-01 00:17:12 128.2.18.152 - W3SVC2 RAPIDHOSTNT1 194.74.51.37 GET /Go.gif - 200 0 1275 295 55718 80 HTTP/1.0 Mozilla/4.0+(compatible;+MSIE+4.01;+Windows+NT) ASPSESSIONIDGGGGGGCE=PBFJHGNCEMKFPFBFLFBEGJMP http://www.clarusresearch.com/output0.asp?Industry=IME&Region=R&I2.x=8&I2.y=6
*/



short PROC_W3C(char *buffer,HitDataPtr Line )
{
	register long j,inquote=0,i=1, foundDelim = FALSE, offset_ref=0, offset_cookie=0;
	register char *p;

	// Ignore all w3c lines with IIS4 process accounting lines.
	if ( formatIndex.process_accounting )
		return LOGLINE_COMMENT;

	// Current line has a space at the front, WHOOPS, its a damn bad ass currupted line, skip it or there will be trouble.
	if ( *buffer == ' ' )
		return -1;

	//Tokenise line into format fields 1 to 25 from ','
	format[i++]=buffer;
	while ( *buffer && i<FORMATSIZE )
	{
		if (*buffer=='\"' && inquote)
		{
			*buffer++ = 0;
			inquote = 0;
			continue;
		}

		if ( logType == LOGFORMAT_MSISA )
		{
			if ( *buffer == '\t' && !inquote )
				foundDelim = TRUE;
		} else
		if ( (*buffer == ' ' || *buffer == '\t') && !inquote )
			foundDelim = TRUE;

		if ( foundDelim )
		{
			*buffer=0;
			if (*(buffer+1)=='\"')
			{
				inquote=1;
				buffer++;
			}
			buffer++;
			if ( *buffer )	// only use the field if its valid, if its and the end, then dont use it.
				format[i++] = buffer;
			foundDelim = FALSE;
		}
		buffer++;
	}


	j = i;
	if ( i >=  63 )
		return 0;

	// This is a special case if the USERNAME has spaces in it, it screws up the format pointers
	// so we must fix them by moving them until it all fits nicely.
	logNumDelimInLine = static_cast<short>(i);
	if (i!=logDelim) {	//check for minimum tokens for the format (10)
		// if username is separated by a space
		// then tokens are out of sync
		if( i == logDelim+1 && formatIndex.cshost ) 
		{
			long diff = i - logDelim;

			p = format[formatIndex.cshost];
			if ( !isdigit( *p ) ) {		// IP is not really an IP
				for(j=formatIndex.cshost;j<logDelim;j++)
					format[j] = format[j+diff];		//shift all fields by the amount missed by
				while( diff ){
					p = mystrchr( format[formatIndex.user], 0 );		// fill the 0x00 with ' ' spaces
					if ( p )
						*p = ' ';
					diff--;
				}
			}
		} else
		// TEST: with w3c/terra.log
		// Ah man, fix another fuckedup w3c file, where the agent fields have spaces galore
		// 2001-06-26 00:01:49 213.99.229.85 - GET /cgi-bin/search.cgi claus2=&dir=164&x=5&y=12 200 25176 HTTP/1.1 Mozilla/4.0 (compatible; MSIE 5.5; Windows 98; Win 9x 4.90) http://compras.terra.es/especiales/verano2001/
		if( i >= logDelim+1 && formatIndex.agent ) 
		{
			long offset = 0;
			p = format[formatIndex.agent];
			if ( *p && ((*p) != '-') )
			{
				char *endp=0;

				// these two assume the last 2 fields are either agent/cookie
				if ( formatIndex.referer ) endp = format[j-1];
				if ( formatIndex.cookie ) endp = format[j-2];
				// fix up the spaces.
				while( p<endp-1 && p<buffer ){		// *p != ')' &&  taken out.
					if ( *p == 0 ){
						*p = '+';
						offset++;
					}
					p++;
				}
				// fix up following wrong indexed fields.
				if( formatIndex.referer > formatIndex.agent )
					offset_ref = offset;

				if( formatIndex.cookie > formatIndex.agent )
					offset_cookie = offset;
			}
		}
		else
			return 0;
	}

	//the less checking here the better
	if (formatIndex.Date())
		Line->date = ConvEDate( format[formatIndex.Date()], Line->newdate );
	else
		Line->date = ConvEDate( GlobalDate, Line->newdate );

	if (!Line->date || *Line->date==0 )
		return 0;

	if (p=format[formatIndex.query]) {
		if (*p!='-') {
			*(p-1) = '?';
		}
	}

	Line->time = format[formatIndex.time];
	Line->stat = format[formatIndex.status];
	Line->clientaddr = format[formatIndex.HostName()];
	Line->file = format[formatIndex.url];
	if ( Line->file && Line->file[0] == '-' )
		Line->file = 0;
	Line->method = format[formatIndex.method];

	if ( Line->agent = format[formatIndex.agent] )
		if ( *Line->agent == '-' ) Line->agent = 0;

	Line->bytes = myatoi(format[formatIndex.bytes]);

	if ( formatIndex.bytesIn )
		Line->bytesIn = myatoi(format[formatIndex.bytesIn]);

	// Add in future when field is used
	//if ( formatIndex.ms )
	//	Line->ms = myatoi(format[formatIndex.ms]);
	//Line->protocol = format[formatIndex.protocol];
	//Line->port = myatoi( format[formatIndex.port] );

	if ( Line->refer = format[formatIndex.referer+offset_ref] )
		if ( *Line->refer == '-' ) Line->refer = 0;

	if ( Line->user = format[formatIndex.user] )
		if ( *Line->user == '-' ) Line->user = 0;

	if ( Line->vhost = format[formatIndex.cshost] )
		if ( *Line->vhost == '-' ) Line->vhost = 0;

	if ( Line->cookie = format[formatIndex.cookie+offset_cookie] )
		if ( *Line->cookie == '-' ) Line->cookie = 0;
	return 1;
}



short PROC_NetCache(char *buffer,HitDataPtr Line )
{
	short j,until=0,i=1;
	char *p, c;
	
	//Tokenise line into format fields 1 to 25 from ','
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if (*buffer==until && until) {
			*buffer++=0;
			until=0;
		}
		if ( (*buffer == ' ' || *buffer == '\t') && !until) {
			*buffer++=0;
			c = *buffer;
			if ( c=='\"' ) { until=c;	buffer++;	}
			if ( c=='[' ) { until=']';	buffer++;	}
			format[i++] = buffer;
		}
		buffer++;
	}
	j = i;
	while( j<28 )
		format[j++] = NULL;

	// This is a special case if the USERNAME has spaces in it, it screws up the format pointers
	// so we must fix them by moving them until it all fits nicely.
	logNumDelimInLine = i;

	p = format[formatIndex.Date()];
	Line->date = ConvLDate( p, Line->newdate );
	if (!Line->date || *Line->date==0 ) return 0;
	if ( p ) {
		while (*p!=':') p++;
		*p++=0;
		// verify time format is vaguely ok, if its real bad, error out
		if ( p[2] == ':' )
			Line->time = p;
		else
			return 0;
	}
	//Line->time = format[formatIndex.date]+13;
	Line->stat = format[formatIndex.status];
	Line->clientaddr = format[formatIndex.HostName()];
	Line->file = format[formatIndex.url];
	if ( Line->file){
		Line->method = Line->file;
		if ( p = mystrchr( Line->file, ' ' ) )
			*p++ = 0;
		Line->file = p;
		if ( p ){
			if ( p = strstr( p, " HTTP" ) )
				*p = 0;
		}
	}

	if ( Line->agent = format[formatIndex.agent] )
		if ( *Line->agent == '-' ) Line->agent = 0;

	Line->bytes = myatoi(format[formatIndex.bytes]);
	if ( Line->refer = format[formatIndex.referer] )
		if ( *Line->refer == '-' ) Line->refer = 0;

	if ( Line->user = format[formatIndex.user] )
		if ( *Line->user == '-' ) Line->user = 0;

	if ( Line->vhost = format[formatIndex.cshost] )
		if ( *Line->vhost == '-' ) Line->vhost = 0;

	if ( Line->cookie = format[formatIndex.cookie] )
		if ( *Line->cookie == '-' ) Line->cookie = 0;
	return 1;
}




short PROC_Oracle(char *buffer,HitDataPtr Line )
{
	register short i=1,inquote=0;
	register char *p;
	
	//tokenise line delimited by various tokens ' ', `
	format[i++]=buffer;
	while (*buffer) {
		if (*buffer=='`' && inquote) {			// ` = 96
			*buffer++=0;
			inquote=0;
		}
		if ( (*buffer == ' ' || *buffer == '`') && !inquote) {
			*buffer=0;
			if (*(buffer+1)=='`') {
				inquote=1;
				buffer++;
			}
			format[i++] = buffer+1;
		}
		buffer++;
	}
	logNumDelimInLine = i;
	if (i!=logDelim && i!=logDelim-1 && i!=logDelim+1) return 0;
	//check for minimum tokens for the format (10)
	if ( i < 28 ) MemClr( &format[i], (28-i)*4 );
	
	Line->date = ConvLDate(format[formatIndex.Date()], Line->newdate);
	if (!Line->date || *Line->date==0 )
		return 0;

	if (formatIndex.date == formatIndex.time) {
		//must be in the same field so extract it
		p = format[formatIndex.date];
		if ( p ) {
			while (*p!=':' && *p!=' ') p++;
			*p++=0;
			// verify time format is vaguely ok, if its real bad, error out
			if ( p[2] == ':' )
				format[formatIndex.date] = p;
			else
				return 0;
		}
	}
	Line->time = format[formatIndex.time];
		
	Line->stat = format[formatIndex.status];
	Line->clientaddr = format[formatIndex.HostName()];
	Line->file = format[formatIndex.url];
	
	p = format[formatIndex.method];
	if (*p == '\"') p++;
	Line->method = p;
	
	if ( Line->agent = format[formatIndex.agent] )
		if ( *Line->agent == '-' ) Line->agent = 0;

	Line->bytes = myatoi(format[formatIndex.bytes]);
	if ( Line->refer = format[formatIndex.referer] )
		if ( *Line->refer == '-' ) Line->refer = 0;

	if ( Line->user = format[formatIndex.user] )
		if ( *Line->user == '-' ) Line->user = 0;

	if ( Line->vhost = format[formatIndex.cshost] )
		if ( *Line->vhost == '-' ) Line->vhost = 0;

	return 1;
}



short PROC_Netpresenz(char *buffer,HitDataPtr Line)
{
	short i=1;

	return 0; 
}

short PROC_Zeus(char *buffer,HitDataPtr Line )
{
	char 	c,*p;		
	short i=1;		
	
	//Tokenise line into format fields 1 to 25 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == '|' || *buffer == 9 ) {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	if ( i < 12 ) MemClr( &format[i], (12-i)*4 );

	format[16] = format[1]; 		//date 990330
	*(format[16]+6)=0;
	format[17] = format[1]+7; 	//time 164042
	
	Line->date = ConvUDate( format[16], Line->newdate );
	if (!Line->date || *Line->date==0 )
		return 0;	
		
	Line->time = ConvUTime(format[17]);
	Line->clientaddr = format[2];

	// extract url from "get url type" whether type is present or not
	Line->method = p = format[3];
	i=31;
	if ( p ) {
		while (*p) {
			if (*p==' ') {
				*p=0;
				format[i]=p+1;
				p++;
				if (*p==' ' || i>33) //ignore damaged lines with >1 space
					return 0;
				i++;
			} else
				p++;
		}
	}
	Line->file = format[31];
	//to allow for counter.exe
	//which adds a delimiter of |
	i=0;
	if (!isdigit(*format[5]))
		i=1;
	Line->stat = format[5+i];
	Line->bytes = myatoi(format[7+i]);
	Line->refer = p = format[9+i];
	if ( p ) {
		c = *p;
		if ( c == '-' || c == 0) Line->refer = 0;
	}
	Line->agent = p = format[10+i];
	if ( p ) {
		c = *p;
		if ( c == '-' || c == 0) Line->agent = 0;
	}

	return 1;
}





/*
[04/01/2000 05:58:14] 209.5.215.53		GET	osc.oeca.org	/Icons/9201	Mozilla/4.7 [en-gb] (Win95; U)	http://osc.oeca.org/Login/mailbox/%2330300847
[04/01/2000 05:58:39] 209.5.215.53		GET	osc.oeca.org	/send/	Mozilla/4.7 [en-gb] (Win95; U)	http://osc.oeca.org/Login/mailbox/%2330300847
[04/01/2000 05:59:03] 24.42.96.228		GET	fc.tvo.org	/	Mozilla/4.0 (compatible; MSIE 5.0; Windows 98; DigExt)	
[04/01/2000 05:59:03] 24.42.96.228		GET	fc.tvo.org	/side.html	Mozilla/4.0 (compatible; MSIE 5.0; Windows 98; DigExt)	http://fc.tvo.org/
[04/01/2000 05:59:29] 24.42.96.228	jflohil	GET	fc.tvo.org	/Mailbox	Mozilla/4.0 (compatible; MSIE 5.0; Windows 98; DigExt)	http://fc.tvo.org/side.html
*/
short PROC_Firstclass(char *buffer,HitDataPtr Line)
{
	short i=1;

	//Tokenise line into format fields 1 to 25 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		// ignore damaged lines with !! in line string
		if ( *buffer == '\t') {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	logNumDelimInLine = i;
	logDelim = i;
	//check if it is a valid format first
	//if i<6 probably another delimiter
	if (i<7)
		return 0;
	if ( i < 20 ) MemClr( &format[i], (20-i)*4 );

	//strip date/time and client IP from [1/12/98 1:40:30 PM] 207.12.145.136
	format[16]=format[1]+1;
	while (*format[1]!=' ') format[1]++;
	*format[1]=0;
	//extract time token
	format[17]=format[1]+1;
	while (*format[1]!=']') format[1]++;
	*format[1]=0;
	//extract client IP
	format[18]=format[1]+2;

	Line->date = ConvFloatingDate( format[16], Line->newdate , MyPrefStruct.forceddmm );
	if (!Line->date || *Line->date==0 )
		return 0;
	
	Line->time = ConvTime(format[17]);
	Line->stat = 0;
	Line->user = format[2];
	Line->clientaddr = format[18];
	Line->file = format[5];
	Line->agent = format[6];
	Line->refer = format[7];

	return 1;
}






/*
12.73.230.165 - - [12/Jun/1999:00:00:42 -0400] "GET /fadr HTTP/1.0" 200 - "-" "Mozilla/4.5 [en] (Win98; I)"
12.73.230.165 - - [12/Jun/1999:00:00:43 -0400] "GET /fadr?a=g&id=1272 HTTP/1.0" 200 - "http://femina.cybergrrl.com/fadr" "Mozilla/4.5 [en] (Win98; I)"
209.156.56.70 - - [12/Jun/1999:00:00:50 -0400] "GET /img/inside.side.gif HTTP/1.1" 200 4741 "http://femina.cybergrrl.com/cgi-bin/search.cgi" "Mozilla/4.0 (compatible; MSIE 4.01; MSN 2.5; Windows 98)"
209.156.56.70 - - [12/Jun/1999:00:00:50 -0400] "GET /img/inside.top.gif HTTP/1.1" 200 8888 "http://femina.cybergrrl.com/cgi-bin/search.cgi" "Mozilla/4.0 (compatible; MSIE 4.01; MSN 2.5; Windows 98)"
209.156.56.70 - - [12/Jun/1999:00:00:53 -0400] "POST /cgi-bin/search.cgi HTTP/1.1" 200 30704 "http://femina.cybergrrl.com/homesurvey.html" "Mozilla/4.0 (compatible; MSIE 4.01; MSN 2.5; Windows 98)"
152.163.204.58 - - [12/Jun/1999:00:01:09 -0400] "GET /fadr?a=g&id=1272 HTTP/1.0" 200 - "http://femina.cybergrrl.com/fadr" "Mozilla/4.0 (compatible; MSIE 4.01; AOL 4.0; Windows 95)"
192.215.71.74 - - [12/Jun/1999:00:01:35 -0400] "GET /cgi-bin/ngsearch.cgi?searchstring=video%20stores HTTP/1.0" 200 21180 "-" "libwww-perl/5.41"
*/

short PROC_Filemaker(char *buffer,HitDataPtr Line)
{
	short	i=1;	
	
	//Tokenise line into format fields 1 to 32 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == ' ') {
			//if more than one space return error
			if (*(buffer+1) == ' ')
				return 0;
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}

	logNumDelimInLine = i;
	logDelim = i;
	format[i] = 0;
	//make sure it is a valid line
	if (i<5 || i>6)
		return 0;

	if ( i < 6 ) MemClr( &format[i], (6-i)*4 );
	
	Line->date = ConvFloatingDate( format[1], Line->newdate, MyPrefStruct.forceddmm );
	if (!Line->date || *Line->date==0 )
		return 0;

	//combine AM/PM values
	if (*(format[3]+1)=='M' || *(format[3]+1)=='m') {
		*(format[3]-1)=' ';
		Line->time=ConvTime(format[2]);
		//dont assume valid client due to extended
		//log format
		if (!ipstringtolong(format[4]))
			return 0;
		Line->clientaddr = format[4];
		Line->file = format[5];
	} else if (*(format[3])=='U') { //German format for time
		Line->time=format[2];
		//dont assume valid client due to extended
		//log format
		if (!ipstringtolong(format[4]))
			return 0;
		Line->clientaddr = format[4];
		Line->file = format[5];
	} else {
		Line->time=format[2];
		//dont assume valid client due to extended
		//log format
		if (!ipstringtolong(format[3]))
			return 0;
		Line->clientaddr = format[3];
		Line->file = format[4];
	}
	
	//Line->agent = 0;
	//Line->refer = 0;

	return 1;
}

#ifdef DEF_FULLVERSION
/*
03/Jan/2000	01:49:21	index.htm	62.172.199.20	200
31/Jan/2000	23:55:47	/products/g726.htm	209.67.247.153	200
*/
short PROC_Bounce(char *buffer,HitDataPtr Line)
{
	short	i=1;	
	
	//Tokenise line into format fields 1 to 32 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == '\t') {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}

	if (i!=6)
		return 0;
	
	Line->date = ConvFloatingDate( format[1], Line->newdate, MyPrefStruct.forceddmm);
	if (!Line->date || *Line->date==0 )
		return 0;
	
	Line->time = format[2];
	Line->file = format[3];
	Line->clientaddr = format[4];

	return 1;
}




/*
881721926.938   3943 203.31.198.156 TCP_MISS/200 5969 GET http://www.unimelb.edu.au/ - DIRECT/www.unimelb.edu.au text/html
881721935.912   8033 203.31.198.156 TCP_MISS/200 1260 GET http://www.unimelb.edu.au/images/black_arrow.gif - DIRECT/www.unimelb.edu.au image/gif
881721935.930   7964 203.31.198.156 TCP_MISS/200 1288 GET http://www.unimelb.edu.au/images/valid_html.4.0.gif - DIRECT/www.unimelb.edu.au image/gif
881721935.986   8107 203.31.198.156 TCP_MISS/200 41150 GET http://www.unimelb.edu.au/images/cwis-banner.gif - DIRECT/www.unimelb.edu.au image/gif
881721943.737   3473 203.31.198.156 TCP_MISS/200 5145 GET http://www.unimelb.edu.au/contact/ - DIRECT/www.unimelb.edu.au text/html
881721944.913    301 203.31.198.156 TCP_MISS/200 3111 GET http://www.unimelb.edu.au/images/logo.gif - DIRECT/www.unimelb.edu.au image/gif
881721954.333   6057 203.31.198.156 TCP_MISS/200 2182 GET http://www.unimelb.edu.au/cgi-bin/phone/phone.pl - DIRECT/www.unimelb.edu.au text/html
881721962.943   3861 203.31.198.156 TCP_MISS/200 9640 GET http://www.unimelb.edu.au/cgi-bin/phone/phone.pl - DIRECT/www.unimelb.edu.au text/html
881722002.409    384 203.31.198.156 TCP_MISS/200 2307 GET http://www.unimelb.edu.au/cgi-bin/phone/phone.pl - DIRECT/www.unimelb.edu.au text/html
881722142.133   1448 203.31.198.155 TCP_MISS/200 2182 GET http://www.unimelb.edu.au/cgi-bin/phone/phone.pl - DIRECT/www.unimelb.edu.au text/html
*/
short PROC_Squid(char *buffer,HitDataPtr Line)
{
	register char 	*ptr, *origbuffer = buffer;
	register short	i=1;	
	register long	ctime;
	
	//Tokenise line into format fields 1 to 32 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == ' ') {
			//if more than one space return error
			if (*(buffer+1) == ' ') {
				buffer++; continue;
			}
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	logNumDelimInLine = i;
	logDelim = i;
	//make sure it is a valid line
	if (i<8)
		return 0;

	ctime = myatoi( format[1] );
	
	DaysDateToString( ctime, origbuffer, 0, '/' , TRUE, TRUE);		// store "01/01/1234" date over the old UNIX date
	Line->date = origbuffer;
	if (!Line->date || *Line->date==0 )
		return 0;
	CTimetoString(ctime, buffer );	// store new timestr "00:00:00" at the end of the buffer
	Line->time=buffer;
	Line->clientaddr = format[3];
	Line->bytes = 0;
	Line->stat = CACHEStr;

	if ( strstr( format[9], "NONE" ) == NULL ){
		Line->bytes = myatoi(format[5]);
		Line->stat = OKStr;
	}
	//strip method
	Line->method = ptr = format[7];
	if (ptr) {
		if ( *ptr != '-' ) {
			while (*ptr!=':') ptr++;
			ptr+=3;
		} else
			ptr = 0;
	}

	Line->file = ptr;
	//Line->agent = 0;
	Line->refer = format[7];
	//Line->user = 0;

	return 1;
}


/*
140.168.53.202, ris, -, Y, 3/08/98, 9:02:10, 1, -, -, www.dilbert.com, -,80, 3646, 13193, 13530, http, -, -, http://www.dilbert.com/, -, Inet, 200, 1
140.168.53.202, ris, -, Y, 3/08/98, 9:02:16, 1, -, -, ad.doubleclick.net, -,80, 4446, 835, 1353, http, tcp, -,http://ad.doubleclick.net/adi/www.dilbert.com/homepage/, -, Inet, 200, 1
140.168.53.202, ris, -, Y, 3/08/98, 9:02:17, 1, -, -, www.dilbert.com, -,80, 1102, 495, 950, http, -, -,http://www.dilbert.com/comics/images/spacer.gif, -, Inet, 304, 1
140.168.53.202, ris, -, Y, 3/08/98, 9:02:20, 1, -, -, www.dilbert.com, -,80, 9974, -, 457, http, -, -, http://www.dilbert.com/comics/images/go1.gif,-, Inet, 10061, 0
140.168.53.202, ris, -, Y, 3/08/98, 9:02:28, 1, -, -, www.dilbert.com, -,80, 10725, 1616, 435, http, -, -,http://www.dilbert.com/comics/dilbert/images/dzh_header.gif, -, Inet, 10061,0
140.168.53.202, ris, -, Y, 3/08/98, 9:02:29, 1, -, -, www.dilbert.com, -,80, 1182, 514, 988, http, -, -,http://www.dilbert.com/comics/dilbert/images/dzh_3dzonimations.gif, -, Inet,304, 1
*/
short PROC_IISProxy(char *buffer,HitDataPtr Line)
{
	register char 	*ptr;		
	register short i=1;
	
	//Tokenise line into format fields 1 to 25 from ','
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == ',') {
			if (*(buffer+1)==0)	//prevents buffer overrun
				format[i++] = buffer+1;
			else
				format[i++] = buffer+2;

			*buffer=0;
		}
		buffer++;
	}
	logNumDelimInLine = i;
	logDelim = i;
	//check for minimum tokens for the format (10)
	if (i<23)
		return 0;
		
	Line->date = ConvFloatingDate( format[5], Line->newdate, MyPrefStruct.forceddmm );
	if (!Line->date || *Line->date==0 )
		return 0;
	Line->time = format[6];
	Line->clientaddr = format[1];

	Line->sourceaddr = format[10];

	Line->user = format[2];
	Line->bytes = 0;
	Line->stat = CACHEStr;
	//Line->stat = format[21];

	if ( strstr( format[21], "Cache" ) == NULL ){
		Line->bytes = myatoi(format[14]);
		Line->stat = OKStr;
	}

	//strip method
	ptr = format[19];
	if (ptr) {
		if ( *ptr != '-' ) {
			while (*ptr!=':') ptr++;
			ptr+=3;
		} else
			ptr = 0;
	}
	Line->file = ptr;
	Line->ms = myatoi( format[13] );
	Line->refer = format[19];
	Line->port = myatoi( format[12] );
	Line->protocol = format[16];


	return 1;
}
short PROC_WSProxy(char *buffer,HitDataPtr Line)
{
	char 	*ptr;		
	short i=1;		
	
	//Tokenise line into format fields 1 to 25 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == '\t') {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	logNumDelimInLine = i;
	logDelim = i;
	//check if it is a valid format first
	if (i!=10)
		return 0;
	
	Line->date = ConvFloatingDate( format[1], Line->newdate, MyPrefStruct.forceddmm );
	if (!Line->date || *Line->date==0 )
		return 0;
	Line->time = format[2];
	Line->stat = format[6];
	Line->clientaddr = format[4];
	//strip method out of string
	ptr=format[5];
	if (ptr) {
		if ( *ptr != '-' ) {
			while (*ptr!=':') ptr++;
			ptr+=3;
		} else
			ptr = 0;
	}
	Line->file = ptr;
	Line->bytes = myatoi(format[8]);
	//Line->agent = 0;
	Line->refer = format[5];
	//Line->user = 0;

	return 1;
}


/*
10/14/98 11:16:50	203.28.150.1	LYNX	0000004080	Traffic	 27223	279	0	0	1s
10/14/98 11:17:01	203.28.150.1	LYNX	0000004081	Requested:	http://www.excite.com/relocate/co=pfp_horo;http://www.excite.com/horoscopes/
10/14/98 11:17:46	203.28.150.1	LYNX	0000004081	Error:	Caught socket exception in CWWWSession::HTTPProcessRequest() Connection to Remote Host timed out - terminating
*/
short PROC_WinGate(char *buffer,HitDataPtr Line)
{
	short i=1;		
	
	//Tokenise line into format fields 1 to 25 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == '\t') {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	//check if it is a valid format first
	if (i<3)
		return 0;
	
	if ( i < 7 )
		MemClr( &format[i], (7-i)*4 );

	Line->date = ConvFloatingDate( format[1], Line->newdate, MyPrefStruct.forceddmm );
	if (!Line->date || *Line->date==0 )
		return 0;
	Line->time = format[1]+9;
	Line->clientaddr = format[2];
	Line->user = format[3];
	if( !strcmpd( "Requested:", format[5] ) )
		Line->file = format[6];
	else
		return 0;

	return 1;
}

short PROC_Ciscorouter(char *buffer,HitDataPtr Line)
{
	short i=1;		

	//Tokenise line into format fields 1 to 25 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == ' ' && *(buffer+1) != ' ') {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	//check if it is a valid format first

	if (i<4)
		return 0;

	Line->date = NULL;
	Line->time = NULL;
	Line->stat = NULL;
	//Line->file = format[0];				// source ip

	Line->sourceaddr = format[1];		// source ip
	Line->clientaddr = format[2];		// destination ip
	Line->bytes = myatoi(format[3]);
	Line->refer = NULL;

	return 1;
}

/*
Thu Aug  1 09:34:28 1996
	Acct-Session-Id = "00000074"
	User-Name = "ddeters.ppp"
	Client-Id = 207.54.12.2
	Client-Port-Id = 0
	NAS-Port-Type = Async
	Acct-Status-Type = Start
	Acct-Authentic = RADIUS
	User-Service-Type = Framed-User
	Framed-Protocol = PPP
	Framed-Address = 207.54.12.102
	Acct-Delay-Time = 0

Thu Aug  1 09:54:29 1996
	Acct-Session-Id = "00000075"
	User-Name = "theman.ppp"
	Client-Id = 207.54.12.2
	Client-Port-Id = 1
	NAS-Port-Type = Async
	Acct-Status-Type = Start
	Acct-Authentic = RADIUS
	User-Service-Type = Framed-User
	Framed-Protocol = PPP
	Framed-Address = 207.54.12.107
	Acct-Delay-Time = 0
*/
short PROC_Radius(char *buffer,HitDataPtr Line)
{
	char 	*ptr;		
	short i=1;		

	//Tokenise line into format fields 1 to 25 from \t
	format[i++]=buffer;
	while (*buffer && i<FORMATSIZE ) {
		if ( *buffer == '\n') {
			format[i++] = buffer+1;
			*buffer=0;
		}
		buffer++;
	}
	//check if it is a valid format first
	if (i<12)
		return 0;

	i--;

	if ( *format[1] != 9 ){
		Line->date = ConvLongDate( format[1] );
		Line->time = format[1] + 11;

	}
	Line->bytes =
	Line->bytesIn = 0;
	Line->protocol =
	Line->sourceaddr =
	Line->clientaddr = 0;
	Line->ms =
	Line->port = 0;
	while( i>0 ){
		if ( format[i] ){
			if ( strstr( format[i], "Timestamp" ) ){
				ptr = 2+mystrchr( format[i], '=' );
				Line->date = ConvLongDate( ptr );
				Line->time = ptr + 11;
			} else
			if ( strstr( format[i], "Client-Id" ) )
				Line->sourceaddr = 2+mystrchr( format[i], '=' );
			else
			if ( strstr( format[i], "User-Name" ) )
				Line->user = 2+mystrchr( format[i], '=' );
			else
			if ( strstr( format[i], "Framed-Address" ) )
				Line->clientaddr = 2+mystrchr( format[i], '=' );
			else
			if ( strstr( format[i], "Framed-IP-Address" ) )
				Line->clientaddr = 2+mystrchr( format[i], '=' );
			else
			if ( strstr( format[i], "Input-Octets" ) )
				Line->bytesIn = myatoi( 2+mystrchr( format[i], '=' ) );
			else
			if ( strstr( format[i], "Output-Octets" ) )
				Line->bytes = myatoi( 2+mystrchr( format[i], '=' ) );
			else
			if ( strstr( format[i], "Session-Time" ) )
				Line->ms = myatoi( 2+mystrchr( format[i], '=' ) );
			else
			if ( strstr( format[i], "Protocol" ) )
				Line->protocol = 2+mystrchr( format[i], '=' );
			else
			if ( strstr( format[i], "Port-Id" ) )
				Line->port = myatoi( 2+mystrchr( format[i], '=' ) );
		}
		i--;
	}
	Line->stat = NULL;
	Line->refer = NULL;

	return 1;
}

#endif








// x "asjkdhkajsdh" "Mozilla xc"
short PROC_Custom(char *buffer,HitDataPtr Line )
{
	short i=1, t=1, ignorespace = 0;
	long ctime;
	char *p, delim;
	
	MemClr( format, 25*sizeof(void*) );
	//Tokenise line into format fields 1 to 25 from ','
	format[t++]=buffer;

	delim = (char)MyPrefStruct.custom_seperator;

	while (*buffer && t<FORMATSIZE ) {
		if ( *buffer == delim ) {
			if ( MyPrefStruct.custom_dataIndex[0] == t-1 && *(buffer-1)!=']' ){
				buffer++;
				continue;
			}
			if ( MyPrefStruct.custom_dataIndex[5] == t-2 && !strcmpd( "\"GET",buffer+1 ) ){
				buffer++;
				continue;
			}
			*buffer++ = 0;
			if ( *buffer == '\"' ){
				delim = 34;
				buffer++;
			} else
			if ( delim == '\"' ) {
				delim = (char)MyPrefStruct.custom_seperator;
				continue;
			}

			format[t++] = buffer;
		}
		buffer++;
	}

	if (t<3)
		return 0;

	i = MyPrefStruct.custom_dataIndex[0];
	switch( MyPrefStruct.custom_dateformat ) {
		case 1:	case 2:
		case 3: Line->date = ConvLDate( format[i], Line->newdate ); break;					//convert date from "Fri, 07 Mar 1997" or "28/Mar/1994" and "2/Mar/1994,
		case 4: Line->date = ConvFloatingDate( format[i], Line->newdate, TRUE ); break;	//DD/MM/YY
		case 5: Line->date = ConvEDate( format[i], Line->newdate ); break;					//YYYY-MM-DD to MM/DD/YY
		case 6: Line->date = ConvLongDate( format[i] ); break;				//Thu Aug  1 10:32:18 1996
		case 7:	ctime = myatoi( format[i] );
				DaysDateToString(ctime, GlobalDate, 0, '/' , TRUE, TRUE);		// store "01/01/1234" date over the old UNIX date
				Line->date = GlobalDate;
				break;
		case 8: p = format[i];
				Line->date = ConvLDate( format[i], Line->newdate );
				if ( p ) {
					while (*p!=':') p++;
					p++;
					Line->time = p;
					while (*p!=' ') p++;
					*p = 0;
				}
				break;
	}

	i = MyPrefStruct.custom_dataIndex[1];
	switch( MyPrefStruct.custom_timeformat ) {
		case 1: Line->time = format[i]; break;
		case 2: Line->time = ConvTime( format[i] ); break;
		case 3: CTimetoString( ctime, GlobalTime ); Line->time=GlobalTime; break;
	}

	Line->stat = format[ MyPrefStruct.custom_dataIndex[2] ];
	Line->clientaddr = format[ MyPrefStruct.custom_dataIndex[3] ];
	Line->sourceaddr = format[ MyPrefStruct.custom_dataIndex[4] ];
	Line->file = format[ MyPrefStruct.custom_dataIndex[5] ];
	if ( Line->file){
		Line->method = Line->file;
		if ( p = mystrchr( Line->file, ' ' ) )
			*p++ = 0;
		Line->file = p;
		if ( p ){
			if ( p = strstr( p, " HTTP" ) )
				*p = 0;
		}
	}

	if ( Line->agent = format[ MyPrefStruct.custom_dataIndex[6] ] )
		if ( *Line->agent == '-' ) Line->agent = 0;

	Line->bytes = myatoi(format[ MyPrefStruct.custom_dataIndex[7] ]);
	Line->bytesIn = myatoi(format[ MyPrefStruct.custom_dataIndex[8] ]);
	Line->ms = myatoi(format[ MyPrefStruct.custom_dataIndex[9] ]);
	if ( Line->refer = format[ MyPrefStruct.custom_dataIndex[10]] )
		if ( *Line->refer == '-' ) Line->refer = 0;

	if ( Line->user = format[ MyPrefStruct.custom_dataIndex[11]] )
		if ( *Line->user == '-' ) Line->user = 0;
		
	if ( Line->vhost = format[ MyPrefStruct.custom_dataIndex[12]] )
		if ( *Line->vhost == '-' ) Line->vhost = 0;

	Line->method = format[ MyPrefStruct.custom_dataIndex[13] ];
	Line->protocol = format[ MyPrefStruct.custom_dataIndex[14] ];
	Line->port = myatoi( format[ MyPrefStruct.custom_dataIndex[15] ] );

	return 1;
}




//
//
// Full webserver comparison chart/list :  http://webcompare.internet.com/chart.html
// and some more here http://serverwatch.internet.com/webservers.html
//

short ProcessLogLine(char *buffer, HitDataPtr Line)
{
	char 	*temp, ch;
	short	sp,tb,cm,ot,i=1, err=0;

	ch = *buffer;
	if ( !ch )
		return LOGLINE_BLANK;	//skip blank lines
	temp = buffer+6;

	//if it is a known token, get new format
	if (ch == '!' || ch == '#' || *temp == '=' )
	{
		long prevlogType = logType;

		// -1 means this is not a data line and also not a bad line
		err = LOGLINE_COMMENT;
		ConvFloatingDate(0,0,0);

		if ( strstr(buffer,"#Software:") ) {
			if ( strstr(buffer,"Microsoft Internet Information Server") )
				logType=LOGFORMAT_IIS4;
			else
			if ( strstr(buffer,"Internet Security and Acceleration") )
				logType=LOGFORMAT_MSISA;
			else
			if ( strstr(buffer,"Windows Media Services") )
				logType=LOGFORMAT_WINDOWSMEDIA;
			else
			if ( strstr(buffer,"NetCache") )
				logType=LOGFORMAT_NETCACHE;
			else
			if ( strstr(buffer,"Oracle WRB") )
				logType=LOGFORMAT_ORACLE;
			else
			if ( strstr(buffer,"Welcome Plugin") )
				logType=LOGFORMAT_WELCOME;
			else
			if ( strstr(buffer,"QTSS") )
				logType=LOGFORMAT_QTSS;
		} 

		// Field descriptor detector:
		// ----------------------------

		if ( strstr(buffer,"!!WEBSTAR") || strstr(buffer,"!!WebSTAR") )
			logType=LOGFORMAT_MACHTTP;
		else {
			//check for Webstar and Extended formats
			if (strstr(buffer,"!!LOG_FORMAT") || strstr(buffer,"#Fields:")) {
				formatIndex.Clear();

				if (strstr(buffer,"COMMON_LOG_FORMAT")) {
					logType=LOGFORMAT_COMMON;	//common log format
					LOG_Common(buffer);
					// if not common log then grab custom fields	
				} else 
				if (logType==LOGFORMAT_NETCACHE) {
					DoHeader_NetCache( buffer );
				} else
				if (logType==LOGFORMAT_ORACLE) {
					LOG_W3C(buffer);
				} else
				if (logType==LOGFORMAT_WINDOWSMEDIA) {
					LOG_WMedia(buffer);
				} else
				if (logType==LOGFORMAT_WELCOME) {
					LOG_W3C(buffer);
				} else
				if (logType==LOGFORMAT_QTSS) {
					LOG_QTSS(buffer);
				} else {
					//IIS 4.0 or WebStar
					if (strstr(buffer,"date") || strstr(buffer,"time") ) { //case sensistive, so check for "date"
						if ( logType == LOGFORMAT_UNKNOWN )
							logType = LOGFORMAT_IIS4;
						LOG_W3C(buffer);
					} else {
						if ( logType == LOGFORMAT_COMMON ) {
							;
						} else {
							logType=LOGFORMAT_MACHTTP;
							LOG_MacHTTP(buffer);
						}
					}
				}
			} else if (strstr(buffer,"#Date:")) {
				mystrcpy( GlobalDate, buffer + 7 );
			} else if (strstr(buffer,"#!PARAMETER")) {
				formatIndex.Clear();
				//MemClr( logIndex, FORMATSIZE*sizeof(void*) );
				LOG_Purveyer(buffer);
			} else if (strstr(buffer,"!!Bad")) {
				err = LOGLINE_COMMENT;
			} else if (strstr(buffer,"format=") && (*temp == '=')) {
				formatIndex.Clear();
				//MemClr( logIndex, FORMATSIZE*sizeof(void*) );
				LOG_Netscape(buffer);
			} else if ( strstr( buffer ,"#Database" ) ) {	
				logType = LOGFORMAT_V4DATABASE;
				err = LOGLINE_CORRECT;
			}
			prevLogFileFormatIndex = formatIndex;

		}

		// if we are in CLUSTER mode, we cant change formats, THATS A NO NO
		// Until all the format. structure stuff is stored in the Line struct
		// this will have to do.
		if ( MyPrefStruct.clusteringActive && logType != LOGFORMAT_UNKNOWN && logType != prevlogType )
			return -2;
		// VSS:
		// ~ minor cluster checking fix, ie a protection code segment went wrong if it changed logtypes with in clusters inside w3c
		//  (that is if someone tries 2 diff formats, the protection code would kick in too early by mistake in the past : this is just fixing a fix that didnt quite work)

	//else process the line according to an established format
	} else
	if( ch > 0 ) 
	{
		if ( logType == LOGFORMAT_UNKNOWN )
		{
			short dq;
			err = -1;
			format[0] = 0;
			ConvFloatingDate( 0,0,0 );
			sp=countchar(buffer, ' ');
			tb=countchar(buffer, '\t');
			cm=countchar(buffer, ',');
			ot=countchar(buffer,'|');
			dq=countchar(buffer,'"');
			
			if ( !strcmpd( "FWDB4", buffer ) ) {				// Real Server format
				logType = LOGFORMAT_V4DATABASE;
			} else
			if ( MyPrefStruct.custom_format ) {
				logType = LOGFORMAT_CUSTOM;
			} else if ( strstr( buffer ," PNA" ) || strstr( buffer ,"]  \"" ) ) {		// Real Server format
				logType = LOGFORMAT_REALSERVER;
			} else if ( strstr( buffer ,"] \"" ) ) {
				logType = LOGFORMAT_COMMON;		// Common format found
				if ( dq > 2 )
						logType = LOGFORMAT_NCSA;
			} else if (cm > 10 && cm < 22 && strstr( buffer, "SVC" ) ) {			
				logType = LOGFORMAT_IIS;			// Microsoft IIS3,4,5 server format
			} else if (tb > 10 && isdatestring(buffer) ) {
				logType = LOGFORMAT_WEBSITE;		// Website Pro extended log
			} else if ( strstr ( buffer ,"log {" ) ) {		
				//SysBeep(1);
				logType = LOGFORMAT_OPENMARKET;	// Open Market Format
				if( Line ) err = -1;
			} else if ( ch=='[' ) {						
				logType = LOGFORMAT_FIRSTCLASS;	// First Class format
			} else if ( strstr( buffer ,"NetPresenz" ) ) {	// NetPresenz format
				logType = LOGFORMAT_NETPRESENZ;
			} else if ( strstr( buffer ,"Date/time" ) ) {	// Hotline format
				logType = LOGFORMAT_HOTLINE;
			} else if (tb==5 && istimestring(buffer+9) ) {
				logType = LOGFORMAT_MACHTTP;		// Mac HTTP must be it...
				logDelim= 6;
				formatIndex.SetDate( 0 );
				formatIndex.time	= 1;
				formatIndex.status	= 2;
				formatIndex.SetHostName( 3 );
				formatIndex.url	= 4;
				formatIndex.agent	= 0;
				formatIndex.referer= 0;
				formatIndex.user	= 0;
			} else if (ot > 5) {
				logType = LOGFORMAT_ZEUS;
			} else if ( (sp ==3 || sp==4) && isdigit( ch )) {			// FileMaker Format
				logType = LOGFORMAT_FILEMAKER;
				//make sure ignore zero bytes is off
				MyPrefStruct.filter_zerobyte=0;
		#ifdef DEF_FULLVERSION
			} else if (tb == 0 && sp>=15 && istimestring(buffer+11) ) {			// Bounce Format
				logType = LOGFORMAT_WUFTPD;
			} else if (tb == 4 && sp==0) {			// Bounce Format
				logType = LOGFORMAT_BOUNCE;
				//make sure ignore zero bytes is off
				MyPrefStruct.filter_zerobyte=0;
			} else if ( ((sp==4 || sp==5) && (istimestring(buffer+11))) || (tb==1 && sp==2)) {		// Radius Format
				logType = LOGFORMAT_RADIUS;
			} else if ( sp == 2 && tb == 0 ) {
				logType = LOGFORMAT_CISCO;
			} else if (issquid(buffer)) {
				logType = LOGFORMAT_SQUID;
			} else if (cm>=21) {
				logType = LOGFORMAT_IISPROXY;
			} else if (tb==8) {
				logType = LOGFORMAT_WSPROXY;
			} else if ( (tb>=2) && (sp>=1) && istimestring(buffer+9) ){
				logType = LOGFORMAT_WINGATE;
		#endif
		#ifdef DEF_APP_FIREWALL
			} else if ( (tb==0) && (sp>5) && istimestring(buffer+7) ) {		//Jan 08 06:53:30
				logType = LOGFORMAT_RAPTOR;
			} else if (tb == 0 && sp==8 && strstr(buffer,"service") ) {			// Firewall-1 Format
				logType = LOGFORMAT_FIREWALL1;
		#endif
			} else {
				return LOGLINE_UNRECOGNISEDFORMAT;
			}
		}

		if ( Line )
		{
			short status;

			switch (logType) 
			{
				case LOGFORMAT_NCSA:			status = (PROC_NCSA(buffer,Line));break;
				case LOGFORMAT_COMMON:			status = (PROC_NCSA(buffer,Line));break;
				case LOGFORMAT_MACHTTP:			status = (PROC_MacHTTP(buffer,Line));break;
				case LOGFORMAT_IIS:				status = (PROC_IIS(buffer,Line));break;
				case LOGFORMAT_WELCOME:
				case LOGFORMAT_MSISA:
				case LOGFORMAT_IIS4:
				case LOGFORMAT_W3C:				status = (PROC_W3C(buffer,Line));break;
				case LOGFORMAT_NETCACHE:		status = (PROC_NetCache(buffer,Line));break;
				case LOGFORMAT_PURVEYER:		status = (PROC_Purveyer(buffer,Line));break;
				case LOGFORMAT_NETSCAPE:		status = (PROC_Netscape(buffer,Line));break;
				case LOGFORMAT_FIRSTCLASS:		status = (PROC_Firstclass(buffer,Line));break;
				case LOGFORMAT_NETPRESENZ:		status = (PROC_Netpresenz(buffer,Line));break;
				case LOGFORMAT_WEBSITE:			status = (PROC_Website(buffer,Line));break;
				case LOGFORMAT_ZEUS:			status = (PROC_Zeus(buffer,Line));break;
				case LOGFORMAT_FILEMAKER:		status = (PROC_Filemaker(buffer,Line));break;
				case LOGFORMAT_ORACLE:			status = (PROC_Oracle(buffer,Line));break;
				case LOGFORMAT_CUSTOM:			status = (PROC_Custom(buffer,Line));break;

				case LOGFORMAT_WINDOWSMEDIA:	status = (PROC_WMedia(buffer,Line));break;
				case LOGFORMAT_QTSS:			status = (PROC_QTSS(buffer,Line));break;
				case LOGFORMAT_REALSERVER:		status = (PROC_Realserver(buffer,Line));break;
			#ifdef DEF_FULLVERSION
			// proxy
				case LOGFORMAT_WUFTPD:			status = (PROC_WUFTPD(buffer,Line));break;
				case LOGFORMAT_BOUNCE:			status = (PROC_Bounce(buffer,Line));break;
				case LOGFORMAT_SQUID:			status = (PROC_Squid(buffer,Line));break;
				case LOGFORMAT_IISPROXY:		status = (PROC_IISProxy(buffer,Line));break;
				case LOGFORMAT_WSPROXY:			status = (PROC_WSProxy(buffer,Line));break;
				case LOGFORMAT_WINGATE:			status = (PROC_WinGate(buffer,Line));break;
				case LOGFORMAT_CISCO:			status = (PROC_Ciscorouter(buffer,Line));break;
				case LOGFORMAT_RADIUS:			status = (PROC_Radius(buffer,Line));break;
			#endif
			// router/firewalls
			#ifdef DEF_APP_FIREWALL
				case LOGFORMAT_RAPTOR:			status = (PROC_Raptor(buffer,Line));break;
				case LOGFORMAT_FIREWALL1:		status = (PROC_Firewall(buffer,Line));break;
			#endif
			}
			prevLogDelim = logDelim;
			err = status;
		} else
			err = 0;
	}

	if ( ISLOG_WEB(logType) )			logStyle = LOGFORMAT_WEBSERVERS;
	if ( ISLOG_PROXY(logType) )			logStyle = LOGFORMAT_PROXYSERVERS;
	if ( ISLOG_STREAMING(logType) )		logStyle = LOGFORMAT_STREAMINGMEDIA;
	if ( ISLOG_FIREWALL(logType) )		logStyle = LOGFORMAT_FIREWALLS;

	return err;
}


