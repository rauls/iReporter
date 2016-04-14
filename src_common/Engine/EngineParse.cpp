#include <ctype.h>
#include <string.h>

#include "Compiler.h"

#include "Hash.h"
#include "config.h"

#include "EngineParse.h"

#include "log_io.h"

#include "EngineRestoreStat.h"	// for externed char string arrays
#include "EngineBuff.h"
#include "Statdefs.h"
#include "Utilities.h"			// "C" style 

#include "EngineAddStats.h"		// for extern char ErrVhostStr[];
#include "engine_drill.h"
#include "SettingsAppWide.h"
#include "FileTypes.h"
#include "DateFixFileName.h"

// externs
extern long			totalDomains;

// FORWARDS
char *WorkOutSessionReferral( VDinfoP VDptr, HitDataRec *Line );



long strcmpManyFast( char *string, long *hash, char **comps )
{
	char *compare;
	long hashtofind;
	long *lp = hash, i;
	short srclen = -1;

	if ( *hash == 0 ) {			// hash up the strings for fast lookups
		char **cs = comps;
		i = 0;
		while( *cs && i<256){
			*lp++ = HashIt( *cs++, 0xfff );
			i++;
		}
	}

	lp = hash;
	if ( MyPrefStruct.useCGI ){
		compare = mystrchr( string, '?' );
		if ( compare )
			srclen = compare - string;
	}

	hashtofind = HashIt( string, srclen );

	while( *hash ){
		if ( *hash == hashtofind )
			return 0;
		hash++;
	}
	return 1;

	//slow way to do it here....
	while( (compare=*comps) ){
		if ( strcmpiPart( compare, string ) == 0 )
			return 0;
		comps++;
	}
	return 1;
}


char *GetDomainPath( VDinfoP VDptr )
{
	if (VDptr->domainPath && VDptr->domainPath[0])
		return VDptr->domainPath;
	else
		return VDptr->domainName;
}


char *GetNewOutpath( VDinfoP VDptr, char *path, char *dirName )
{
	char	newpath[1024];
	char	*ret = NULL;

	PathFromFullPath( path, buf2 );
	sprintf( newpath, "%s%s%c", buf2, GetDomainPath(VDptr), PATHSEP );
	if ( MyPrefStruct.EditPaths )
		ret = EditPathReplacePathFromHost( MyPrefStruct.EditPaths, GetDomainPath(VDptr), newpath );
	DateFixFilename( newpath, dirName );
	return ret;
}

size_t	GetReportPath(VDinfoP VDptr, char* szDestPath, size_t nLength)
{
	size_t nPos(0);

	if ( totalDomains>0 && VDptr ) 
	{
		nPos += PathFromFullPath( MyPrefStruct.outfile, szDestPath);	// not safe - oh well.
		nPos += mystrncpy(szDestPath+nPos, GetDomainPath(VDptr), (nLength-nPos));
		nPos += mystrncpy(szDestPath+nPos, PATHSEPSTR,			 (nLength-nPos));
		if (nPos >= nLength)	// if there is no space left in the string.
			return nPos;
		ReplacePathFromHost( GetDomainPath(VDptr), szDestPath );
	}
	else
	{
		nPos += PathFromFullPath( MyPrefStruct.outfile, szDestPath);
	}

	return nPos;
}

/* ReverseAddress - reverse an IP address string 	*/
/* - kills source string						*/
/* - returns pointer to last field in addr		 	*/
char *ReverseAddressARPA(char *srcDomain, char *dst)
{
	char	*pt, *pt2, *src;
	char	*last_field = 0;
	char	tempSrc[512];
	int		count=0;

	if ( srcDomain==dst ) {
		mystrncpy( tempSrc, srcDomain, 512 );
		src = tempSrc;
	} else
		src = srcDomain;
		
	dst[0] = 0;
	while( ((pt=strrchr(src,'.')) || (pt2=strrchr(src,'/'))) && count<=4 ) {
		if ( !pt ) pt = pt2;
		if ( !last_field ) last_field = pt + 1;
		if ( count<4 ){
			strcat( dst,pt+1 );
			if ( count<3 ) strcat( dst,"." );
			count++;
		}
		*pt = 0;
	}
	if ( count < 4 ){
		if ( !last_field ) last_field = src;
		strcat( dst,src );
	}
	return( last_field );
}

/* ReverseAddress - reverse an IP address string 	*/
/* - doesnt destroy source string 
   - much faster than previous version.
   - returns pointer to last field in addr		 	*/
char *ReverseAddress(char *srcDomain, char *dst)
{
	char	*src;
	char	*last_field = 0;
	char	*ptrs[42];
	char	tempSrc[1024+4];

	long	l = 0, i, c, total_len=0, copylen;
	long	len[42];

	if ( srcDomain==dst ) 
	{
		mystrncpy( tempSrc, srcDomain, 512 );
		src = tempSrc;
	} else
		src = srcDomain;

	ptrs[0] = src;
	i = 0;
	// gather pointers to items at dots and store lengths
	while( (c=*src++) && i<31 )
	{
		if ( c == '.' )
		{
			len[i] = l;
			i++;
			ptrs[i] = src;
			l = -1;
		}
		if ( c == '=' )
		{
			i = 0;
			break;
		}
		l++;
		total_len++;
	}

	// Only if there are 3 DOTS
	if ( i && 3 )
	{
		len[i] = l;
		last_field = ptrs[i];
		copylen = 0;
		// copy each item in reverse to the output string.
		while( i>=0 )
		{
			src = ptrs[i];
			l = len[i];
			if ( l<255 )
				while( l-- ){
					*dst++ = *src++;
					copylen++;
				}
			if ( i ) *dst++ = '.';
			i--;
		}
	// if no dots, then just copy the whole damn lot
	} else
		while( *dst++ = *srcDomain++ );
	*dst++ = 0;

	return last_field;
}

/* GetIPAddress - get numerical address from string */
long GetIPAddress(char *hostname)
{
	int		i;
	long	addr = 0;
	
	for (i=3;;) {
		addr |= (long)myatoi(hostname) << 8*i;
		if (--i < 0) break;
		hostname = mystrchr(hostname,'.') + 1;
		if (!hostname) return(0L);
	}
	return(addr);
}

void swapdata( void *s1, void *s2, long datasize )
{
	if ( s2 ){
		memcpy( buf2, s1, datasize );
		memcpy( s1, s2, datasize );
		memcpy( s2, buf2, datasize );
	}
}


void trimrefer(char *txt)
{
	char  *ptr = txt;

	if ( ptr ) {
		while ( *ptr ) {
			if ( *ptr == '$' || *ptr=='?' || *ptr=='#') {
				*ptr='\0';
				return;
			}
			ptr++;
		}
	}
}


// returns success flag.
static long GetSearchQueryStrings( const char *sStr, char*out )
{
	char* dStr=out;
	
	char lc, cc, c=0;
	while( *sStr )
	{
		cc = c = *sStr;
		if ( c == '+' )	{ c = ' '; }
		if ( c == '%' )	{ c = HexToChar((char*)sStr+1); sStr+=2; }
		if ( c == '&' )
			break;
		if ( c < ' ' )
		{
			c = ' ';
			lc = '+';
		}

		if ( !(lc=='+' && cc=='+') )
			*dStr++ = c;
		lc = cc;
		sStr++;
	}
	if ( c == ' ' )
		*(dStr-1) = 0;
	*dStr++ = 0;

	if ( *out == ' ' )
		out++;

	if ( dStr != out && *out )
		return TRUE;
	else
		return FALSE;
}


char* DecodeSearch( char* outStr, const char* referralURLStr, const char* referralParamStr ) 
{
	const CQSearchEngineList::SearchEngineSettings& searchEngines=
		CQSettings::TheSettings().SearchEngines().GetList();

	// for each app settings defined search engine...
	for( CQSearchEngineList::SearchEngineSettings::const_iterator it( searchEngines.begin() );
		 it!=searchEngines.end();
		 ++it
		 )
	{
		// ...if current search engine is contained within specified referral URL string...
		if( strstr( referralURLStr, it->LogFileStamp().c_str() ) )
		{
			const CQSearchEngineList::Strings& params=it->LogFileStampParams();
			
			// for each of the current search engine's params
			const size_t numParams(params.size());
			for( size_t i(0); i<numParams; ++i )
			{
				// ..if current param is contained within specified referral param string
				const char* paramStr=strstr( referralParamStr, params[i].c_str() );
				if( paramStr )
				{
					// if we can find the actual params..
					if( GetSearchQueryStrings( paramStr+params[i].length(), outStr ) )
					{
						return outStr;
					}
				}
			}
		}
	}

	return 0;
}





// ------------------------------- AGENT STRING DECODERS ---------------------------------

const char* InterpretRobotString( const char* robotStr, long* robotId )
{
	const CQRobotSettingList::NameToLogSettings& robots=
		CQSettings::TheSettings().Robots().GetList();

	// for each app settings defined robot...
	size_t numRobots( robots.size() );
	for( size_t i(0); i<numRobots; ++i )
	{
		// ...if current robot matches specified robot string...
		if( strstr( robotStr, robots[i].LogFileStamp().c_str() ) )
		{
			*robotId=i+1;	// set id to current index + 1 (which is what old code used to do)
			return robots[i].UserAssignedName().c_str();	
		}
	}

	*robotId=0;		// indicate no robot found
	return 0;
}


// return the name of the OS string
char *InterpretOperSysString( char *opersys_str, long *outOStype, long *outOSid  )
{
	char *osPtr, *s = opersys_str;
	long OStype = 4, OSid, i;		// 0=bad!!!

	if ( !outOStype || !outOSid ) return 0;

	i = 0;
	s = mystrchr( opersys_str, '(' );
	// Sometimes some linux agents dont have brackets and basic streaming agents dont either
	if ( !s )
		s = opersys_str;

	OSid = OS_UNKNOWN;

	if ( s )
	{
		int fcase = FALSE;
		opersys_str = s;

		if ( (osPtr = mystrstr(  opersys_str, "Win" )) ){
			opersys_str = osPtr;
			OStype = 1;

			if ( (osPtr = mystrstr(  opersys_str, "16" )) ){
				OSid = OS_WIN31;
			} else
			if ( (osPtr = mystrstr(  opersys_str, "95" )) ){
				OSid = OS_WIN95;
			} else
			if ( (match(  opersys_str, "*9x?4.90*", fcase )) ){
				OSid = OS_WINME;
			} else
			if ( (osPtr = mystrstr(  opersys_str, "98" )) ){
				OSid = OS_WIN98;
			} else
			if ( (match( opersys_str, "Windows?NT?5.0*", fcase )) ){
				OSid = OS_WIN2000;
			} else
			if ( (match( opersys_str, "Windows?NT?5.1*", fcase )) ){
				OSid = OS_WINXP;
			} else
			if ( (match( opersys_str, "Windows?NT?5*", fcase )) ){
				OSid = OS_WIN2000;
			} else
			if ( (match( opersys_str, "Windows?2002*", fcase )) ){
				OSid = OS_WINXP;
			} else
			if ( (match( opersys_str, "Windows?2000*", fcase )) ){
				OSid = OS_WIN2000;
			} else
			if ( (match( opersys_str, "Windows?NT*", fcase )) ){
				OSid = OS_WINNT;
			} else
			if ( (osPtr = mystrstr(  opersys_str, "Windows%20NT" )) ){
				OSid = OS_WINNT;
			} else
			if ( (match(  opersys_str, "WinNT?5.0", fcase  )) ){
				OSid = OS_WIN2000;
			} else
			if ( (match(  opersys_str, "WinNT?5.1", fcase  )) ){
				OSid = OS_WINXP;
			} else
			if ( (osPtr = mystrstr(  opersys_str, "WinNT" )) ){
				OSid = OS_WINNT;
			} else
			if ( (osPtr = mystrstr(  opersys_str, "Win" )) ){
				OSid = OS_WIN31;
			}
		} else
		if ( (osPtr = mystrstr(  opersys_str, "NT 5" )) ){
			OSid = OS_WINNT;
			OStype = 1;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "PowerPC" )) ){
			OStype = 2;
			if ( (osPtr = mystrstr(  opersys_str, "OmniWeb" )) ){
				OSid = OS_MACOSX;
			} else
			if ( (osPtr = mystrstr(  opersys_str, "MSIE 5.1" )) ){
				OSid = OS_MACOSX;
			} else
			{
				OSid = OS_MACOS;
			}
		} else
		if ( (osPtr = mystrstr(  opersys_str, "PPC" )) ){
			OSid = OS_MACOS;
			OStype = 2;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "68k" )) ){
			OSid = OS_MACOS;
			OStype = 2;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "Mac" )) ){
			OSid = OS_MACOS;
			OStype = 2;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "WebTV" )) ){
			OSid = 8;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "SunOS" )) ){
			OSid = 9;
			OStype = 3;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "IRIX" )) ){
			OSid = 10;
			OStype = 3;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "AIX" )) ){
			OSid = 11;
			OStype = 3;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "HP-UX" )) ){
			OSid = 12;
			OStype = 3;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "OSF" )) ){
			OSid = 13;
			OStype = 3;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "FreeBSD" )) ){
			OSid = 14;
			OStype = 3;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "Linux" )) ){
			OSid = 15;
			OStype = 3;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "Amiga" )) ){
			OSid = 16;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "OpenVMS" )) ){
			OSid = 17;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "OS/2" )) ){
			OSid = 18;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "BeOS" )) ){
			OSid = 20;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "gozilla" )) ){
			OSid = OS_WINUNKNOWN; OStype = 1;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "downloadaccelerator.com" )) ){
			OSid = OS_WINUNKNOWN; OStype = 1;
		} else
		if ( (osPtr = mystrstr(  opersys_str, "Lynx" )) ){
			OSid = 19;
			OStype = 3;
		}
	}
	
	*outOStype = OStype;
	*outOSid = OSid;

	return opersysStrings[OSid];
}



int InterpretMediaPlayerString( char *out, char *agentstr )
{
	//char clientstr[512];
	char *wcPtr;
	char *clientstr = out;
	long i;

	// For NSPlayer, mplayer2.exe wmplayer.exe
	if ( (wcPtr = strstri(agentstr, "player" ))) {
		i = mystrcpy( clientstr, "WindowsMedia ");
		getVersion( clientstr+i, wcPtr, 10 );
	} else
	if ( (wcPtr = strstri(agentstr, "play32" ))) {
		i = mystrcpy( clientstr, "Realplayer ");
		getVersion( clientstr+i, wcPtr, 10 );
	} else
	if ( (wcPtr = strstri(agentstr, "RealMedia" ))) {
		i = mystrcpy( clientstr, "RealMedia ");	
		getVersion( clientstr+i, wcPtr, 10 );
	} else
	if ( (wcPtr = strstri(agentstr, "Winamp" ))) {
		i = mystrcpy( clientstr, "Winamp " );	
		getVersion( clientstr+i, wcPtr, 10 );
	} else
	if ( (wcPtr = strstri(agentstr, "qtver=" ))) {
		i = mystrcpy( clientstr, "QuickTime " );
		wcPtr+=6;
		strcpyuntil( clientstr+i, wcPtr, ';' );
	} 
	else
	{
		return 0;
	}

	return 1;
}
	
	
int InterpretBrowserString( char* outStr, const char* browserStr )
{
	const CQBrowserSettingList::NameToLogSettings& browsers=
		CQSettings::TheSettings().Browsers().GetList();

    // for each app settings defined browser...
	for( CQBrowserSettingList::NameToLogSettings::const_iterator it( browsers.begin() );
		 it!=browsers.end();
		 ++it
		 )
	{
		// ...if current browser matches specified browser string...
		char* ptr=(char*)strstr( browserStr, it->LogFileStamp().c_str() );
		if( ptr )
		{
			// ...copy across the user defined name
			const std::string& userAssignedName=it->UserAssignedName();
			strcpy( outStr, userAssignedName.c_str() );

			// if a version number can be found in the specified browser string...
			char version[32];
			if( getVersion( version, ptr+it->LogFileStamp().size(), sizeof(version)-1 ) )
			{
				// ...copy across the version number as well
				outStr+=userAssignedName.size();
				strcpy( outStr++, " " );
				strcpy( outStr, version );
			}

			return 1;	// found
		}
	}

	return 0;	// not found
}


// ------------------------------------------ FILTER DECODERS ----------------------------------------------



// check to see if data is to be included

// if no includes match, then throw out

// if any exlucdes match, then throw out

int CheckFilterMatch( char *mydata, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, char fcase )
{
	long incnum=0, excnum=0, foundinc = 0, foundexc = 0,i;


	if ( !mydata )
		return 1;

	for ( i=0; i<tot; i++) {
		if ( filters[i][0] != '!' ){
			if (match(mydata, filters[i],fcase))
				foundinc++;
			incnum++;
		} else {
			if ( !match(mydata, filters[i],fcase))
				foundexc++;
			excnum++;
		}
	}
	if ( incnum && !foundinc )	return 1;
	if ( excnum && foundexc )	return 1;
	return 0;

}



int CheckFilterMatch2( char *mydata, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, char fcase )
{

	long foundinc = TRUE,foundexc = FALSE,i;
	for ( i=0; i<tot; i++) {
		if ( filters[i][0] != '!' ){
			if ( match( mydata, filters[i],fcase)) {
				foundinc = TRUE;	break;	// if found stop and find any exclusions
			} else
				foundinc = FALSE;
		}
	}
	for ( i=0; i<tot; i++) {
		if ( filters[i][0] == '!' ){
		if ( !match( mydata, filters[i],fcase))

			foundexc |= TRUE;
		}
	}
	if ( !foundinc || foundexc ) return TRUE;

	return FALSE;

}


///////////////////////////////////////////////////////////////////////////////////////////////



char *WorkOutSessionReferral( VDinfoP VDptr, HitDataRec *Line )
{
	register Statistic	*p;
	long	cdate;

	if ( VDptr->byClient ){
		p = VDptr->byClient->FindStatbyName( Line->clientaddr );
		
		if ( p ){			// if its a sane index # its ok
			long difftime, lasttime;

			cdate = Line->ctime;
			lasttime = p->lastday;
			difftime = (cdate - lasttime);
			if ( difftime < 0 ) difftime = -difftime;

			if ( p->visits == 0 )
				return Line->refer;
			else
			if ( difftime > SILENT_TIME )
				return Line->refer;
			else {
				long index;
				char *data;
				if ( VDptr->byRefer && p->sessionStat ){
					// get HASH of referral for this CLIENT
					index = VDptr->byRefer->FindHash( p->sessionStat->SessionRef );
					if ( index >= 0 ){						// get NAME of this referral
						data = VDptr->byRefer->GetName(index) ;
						return data;
					}
				}
			}
		}
	}
	return Line->refer;
}


int PerformFilterCheck( VDinfoP VDptr, HitDataRec *Line )
{
	long skipit = 0, keepit = 0, i;
	if ( MyPrefStruct.filterdata.filterInTot>0) {
		i = CheckMultiFilterMatch( VDptr, Line, MyPrefStruct.filterdata.filterIn, MyPrefStruct.filterdata.filterInTot, 1 );
		if ( i ) {
			return 1;
		}
	}
	return 0;
}

// returns
// 0 ... found none to match
// 1 ... keep the match
// 2 ... ignore the match
long CheckMultiFilterMatch( VDinfoP VDptr, HitDataRec *Line, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, long fcase )
{
	long incnum=0, excnum=0, foundinc = 0, foundexc = 0,i, checked = 0,
		 skipit = 0, keepit = 0;
	char *p, *data, lastc=0,c;

	if ( !Line ) return 0;

	for ( i=0; i<tot; i++) {
		p = filters[i];
		c = *p;
		data = NULL;
		switch( c ){
			case '0' : data = Line->file; break;
			case '1' : data = Line->clientaddr; break;
			case '2' : data = Line->agent; break;
			case '3' : data = Line->refer; break;
			case '4' : data = Line->stat; break;
			case '5' : data = Line->vhost; break;
			case '6' : data = Line->cookie; break;
			case '7' : data = Line->user; break;
			case '8' : data = Line->method; break;
			case '9' : data = WorkOutSessionReferral( VDptr, Line ); break;

			case '-' :	// This filter has been explicitly set as non-active.
			default	 : continue;	// back to the for loop!
		}

		if (!data)
			data = "";

		{
			checked++;

			if ( !lastc ) lastc = c;
			// if we change data types then we use new set as a subset
			// else continue to match new patterns as a combined match
			if ( lastc != c){
				if ( incnum && !foundinc )	skipit = 1; else
				if ( excnum && foundexc )	skipit = 1;
				else keepit = 1;
				if ( skipit && !keepit)
					return 1;
				// reset matching flags, to start from a known state
				skipit = keepit = 0;
				incnum = foundinc = 0;
				excnum = foundexc = 0;
			}

			p++; 
			// ---- INCLUDES
			if ( *p != '!' ){
				incnum++;
				if( match( data, p, static_cast<short>(fcase) ) )
					foundinc++;
				else
				if ( p[0] == '-' && *data == 0 )
					foundinc++;
			} else {
			// ---- EXCLUDES
				excnum++;
				if ( !match( data, p, static_cast<short>(fcase) ) )
					foundexc++;
				else
				if ( p[1] == '-' && *data == 0 )
					foundexc++;
			}
			lastc = c;
		}
	}
	if ( checked ){
		if ( incnum && !foundinc )	skipit = 1;	else
		if ( excnum && foundexc )	skipit = 1;
		else keepit = 1;
	}
	if ( skipit && !keepit)
		return 1;
	else
		return 0;
}


// **** a much better way to define the delimeter.  (RS)
//#ifdef DEF_MAC		DON'T FIDDLE WITH MAC CODE !!!
//#define	MAPPING_DELIM	'\t'	THIS TOOK 1/2 DAY TO TRACK DOWN
//#else								IT'S NOT REQUIRED!
#define	MAPPING_DELIM	'='
//#endif

char *MultiMappingMatch( char *mydata, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long *start, long tot, char fcase )
{
	if ( mydata ){

		long incnum=0, excnum=0, foundinc = 0, foundexc = 0,i;
		char pattern[MAX_FILTERSIZE];

		for ( i = *start; i<tot; i++)
		{
			*start = i+1;
			if ( filters[i][0] != '!' )
			{
				char	*thename, *s, *d;

				d = pattern;
				s = filters[i];
				thename = mystrrchr( s, MAPPING_DELIM );
				while( s < thename )
					*d++ = *s++;
				*d++ = 0;

				// newdynamic content group, where we extract a variable and use its value.
				if ( thename[1] == '?' )
				{
					char	*var = mystrstr( mydata, pattern );

					if ( var )
					{
						char	*start = NULL, c;

						while (c = *var)
						{
							if (c == '=')
								start = var+1;
							if (c == ';' || c == '&' || !c)
							{
								*var = 0;
								continue;
							}
							var++;
						}
						return start;
					}
				} else
				// basic match, if filter exists in mydata then return filter name
				if ( match( mydata, pattern, fcase ) ){
					return thename+1;
				}
			}
		}
	}
	return 0;
}

// filter contains "NAME,Pattern for View CGI* , Pattern for Click CGI*"
char *AdvertMappingMatch( char *url, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long *ret, long tot, char fcase )
{
	if ( url ){
		long i;
		char *p;
		char pattern[200];

		for ( i = 0; i<tot; i++) {
			p = filters[i];
			if ( *p != '!' ) {
				p = mystrchr( p, ',' );
				strcpyuntil( pattern , p+1, ',' );
				if ( match( url, pattern ,fcase ) ){
					*ret = 0;
					return filters[i];
				}
				p = strrchr( p, ',' );
				if ( match( url, p+1,fcase ) ){
					*ret = DRILLF_COUNTER4;
					return filters[i];
				}
			}
		}
	}
	return 0;
}




// filter contains "NAME,Pattern for View CGI* , Pattern for Click CGI*"
char *AdvertCampMappingMatch( char *url, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long *ret, long tot, char fcase )
{
	if ( url ){
		long i;
		char *p;
		char pattern[200];

		for ( i = 0; i<tot; i++) {
			p = filters[i];
			if ( *p != '!' ) {
				p = mystrchr( p, ',' );
				strcpyuntil( pattern , p+1, ',' );
				if ( match( url, pattern,fcase ) ){
					long cost = 0;
					*ret = 0;
					p = mystrchr( p+1, ',' );
					if (p){
						cost = myatoi( p+1 );
					}
					*ret = cost;
					return filters[i];
				}
			}
		}
	}
	return 0;
}




// TO DO: This has to be faster by making the delimeters 0 before hand when loading, and populate a char * array[3] with relative pointers. (URGENT)
/* This can be done with a typedef for filters.
typedef struct {
	char string[MAX_FILTERSIZE];
	char *item1;
	char *item2;
	char *item3;
} Filter_Items;

Then it can be defined in the main Filters struct as...

Filter_Items	filterIn[MAX_FILTERUNITS];
long			filterInTot;

They can be accessed by;
filterIn[5].item1

instead of the old way of;
filterin[5][1]

*/

// 18/DEC/2001 ~ QCM:45828 - fixing referral groups to handel = sign in data component. 
char *CheckMappingMatch( char *mydata, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, char fcase )
{

	if ( mydata )
	{
		long i;
		char *p;

		for ( i=0; i<tot; i++) 
		{
			if ( filters[i][0] != '!' ) 
			{
				//len = mystrcpy( string, filters[i] );
				//len = strcpyx( string, filters[i], MAPPING_DELIM );
				//p = (char*)revstrrchr( string, string+len-1, MAPPING_DELIM );
				p = strrchr( filters[i], MAPPING_DELIM );
				if ( p )
				{
					*p = 0;		// CUt the string in half
					if ( match( mydata, filters[i], fcase ) )
					{
						*p = MAPPING_DELIM;		// restore our string thats used again.
						return p+1;
					} else
						*p = MAPPING_DELIM;
				}
			}
		}
	}
	return 0;
}





char *CheckHostMappingMatch( char *host, char *url, char filters[MAX_FILTERUNITS][MAX_FILTERSIZE], long tot, char fcase )
{
	if ( host ){
		long i;
		char find1[MAX_FILTERSIZE];
		char find2[MAX_FILTERSIZE];
		char *s, *d, *newhost;


		for ( i=0; i<tot; i++) {
			if ( filters[i][0] != '!' ) {
				char *filterurl = NULL;

				d = find1;
				s = filters[i];
				while( *s != MAPPING_DELIM ){
					if ( *s == '/' && !filterurl ){
						*d++ = 0;
						d = find2;
						filterurl = s;
					}
					*d++ = *s++;
				}
				*d++ = 0;
				newhost = s+1;
				//strcpybrk( string, filters[i] , MAPPING_DELIM, 1 );

				// normal DOMAINname found and replace it with newhost
				if ( filterurl == NULL ){
					if ( match(host,find1,fcase) )
						return newhost;
				} else {
					// special domain/url mapping to newdomain
					if ( match(host,find1,fcase) ){
						if (! strcmpd(find2,url) ){
							i = strlen( find2 );
							if ( find2[i-1] == '/' ) i--;	// make sure the first char / is preserved
							mystrcpy( url, url+i );		// strip the begining of the URL of the filterURL
							return newhost;
						}
					}

				}
			}
		}
	}
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////////


int CopyDomainNameOld( char *d, char *src )
{
	char	c, *s = src;
	int		cnt=0, done=0, checkCount=0,l=0;
	char	check[]= { "http://\0" };

	if ( s && d ){
		char	waitfor=0;

		while( (c=*s) && done==0){
			if ( waitfor == 0 ){
				if ( c == check[cnt] ){
					checkCount++;
					cnt++;
				} else waitfor = 1;
			}
			if ( waitfor == 1 ){
				if ( c==':' || c=='/' || c=='\\' )
					waitfor = 1;
				else waitfor = 2;
			}
			if ( waitfor == 2 ){
				if ( c==':' || c=='/' || c=='\\' ){
					done = 1;
				} else {
					*d++ = tolower(c);
					l++;
				}
			}
			s++;
		}
		if ( *(d-1) == '.' ) *(d-1)=0;
		*d++ = 0;
	}
	return l;
}



int CopyDomainName_Safely( char *d, char *src )
{
    char    c, *s = src;
    int        cnt=0, done=0, l=0;
    char    check[]= { "http" };
    
    if ( s && d ){
        char waitfor=0;

		if ( !strcmpd( check, s ) )
			s+=4;
		else waitfor = 1;

        while( (c=*s) && done==0) {
			if ( waitfor == 1 ) {
                if ( c==':' || c=='/' || c=='\\' )
                    waitfor = 1;
                else waitfor = 2;
            }
            if ( waitfor == 2 ){
                if ( c==':' || c=='/' || c=='\\' ){
                    done = 1;
                } else {
                    *d++ = tolower(c); //start of string copy
                    l++;
                }
            }
            s++;
        }
        if ( *(d-1) == '.' ) *(d-1)=0;
        *d++ = 0; //terminate
    }
    return l;
}



int CopyDomainName( char *d, char *src )
{
    char    c, *s = src;
    int        cnt=0, done=0, l=0;
    char    check[]= { "http" };
    long     c1;
    
    if ( s && d ){

        c1 = *(long*)src;
        char waitfor=0;

        while( (c=*s) && done==0) {
            if ( waitfor == 0 ){
                if( *(long*)check==c1 ) {
                    s+=4;
                } else waitfor = 1;
            }
			if ( waitfor == 1 ) {
                if ( c==':' || c=='/' || c=='\\' )
                    waitfor = 1;
                else waitfor = 2;
            }
            if ( waitfor == 2 ){
                if ( c==':' || c=='/' || c=='\\' ){
                    done = 1;
                } else {
                    *d++ = tolower(c); //start of string copy
                    l++;
                }
            }
            s++;
        }
        if ( *(d-1) == '.' ) *(d-1)=0;
        *d++ = 0; //terminate
    }
    return l;
}

// ---------------------------------- Init / Close options for lists
void CorrectVirtualName( char *name, char *vname, char nameisfile )
{
	long i;
	if ( nameisfile ){
		mystrcpylower( vname, FileFromPath( name, NULL ) );
	} else
	if ( *name ){
#if _SUNOS
		CopyDomainName_Safely( vname, name );
#else
		CopyDomainName( vname, name );
#endif
	} else
		i = mystrcpylower( vname, ErrVhostStr );
}

int IsNameBad( char *name )
{
	while( *name ){
		if ( *name < 32 || *name > 127 ){
			return 1;
		}
		name++;
	}
	return 0;
}


