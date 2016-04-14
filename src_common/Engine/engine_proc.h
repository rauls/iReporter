#ifndef __ENGINE_PROC
#define __ENGINE_PROC

// NOTE : This code is run on all platforms and MUST NOT have any OS specific includes or code
// unless wrapped in #defines, please keep OS specifics to a minimum

#ifdef __cplusplus
extern "C" {
#endif

extern const short HEADNAMESIZE;

#define LOGLINE_CORRECT				1
#define LOGLINE_ERROR				0
#define LOGLINE_BLANK				-1
#define LOGLINE_COMMENT				-2
#define LOGLINE_UNRECOGNISEDFORMAT	-3



enum {
	LOGFORMAT_UNKNOWN,
	LOGFORMAT_WEBSERVERS,
	LOGFORMAT_COMMON,
	LOGFORMAT_NCSA,
	LOGFORMAT_MACHTTP,	
	LOGFORMAT_PURVEYER,	
	LOGFORMAT_NETSCAPE,
	LOGFORMAT_IIS,	
	LOGFORMAT_NETPRESENZ,
	LOGFORMAT_WEBSITE,
	LOGFORMAT_FILEMAKER,
	LOGFORMAT_FIRSTCLASS,
	LOGFORMAT_IIS4,
	LOGFORMAT_OPENMARKET,
	LOGFORMAT_SENDMAIL,	
	LOGFORMAT_HOMEDOOR,
	LOGFORMAT_ZEUS,	
	LOGFORMAT_BOUNCE,	
	LOGFORMAT_ORACLE,	
	LOGFORMAT_WELCOME,	
	LOGFORMAT_HOTLINE,	
	LOGFORMAT_NETCACHE,
	LOGFORMAT_W3C,		

	LOGFORMAT_FTPSERVERS = 40,
	LOGFORMAT_UNIXFTPD,	
	LOGFORMAT_WUFTPD,
	LOGFORMAT_FTPSERVERS_END,

	LOGFORMAT_WEBSERVERS_END,

	//proxies
	LOGFORMAT_PROXYSERVERS = 50,
	LOGFORMAT_SQUID,
	LOGFORMAT_MSISA,
	LOGFORMAT_IISPROXY,
	LOGFORMAT_WSPROXY,
	LOGFORMAT_WINGATE,
	LOGFORMAT_PROXYSERVERS_END,

	//streaming media logs
	LOGFORMAT_STREAMINGMEDIA = 60,
	LOGFORMAT_REALSERVER,
	LOGFORMAT_QTSS,
	LOGFORMAT_WINDOWSMEDIA,
	LOGFORMAT_STREAMINGMEDIA_END,

	//routers
	LOGFORMAT_FIREWALLS = 70,
	LOGFORMAT_RADIUS,
	LOGFORMAT_CISCO,
	LOGFORMAT_FIREWALL1,
	LOGFORMAT_RAPTOR,
	LOGFORMAT_FIREWALLS_END,

	LOGFORMAT_V4DATABASE,				// V4 compact (.fdb files) database
	LOGFORMAT_V5DATABASE,				// v5 extended (.fxdb files) database

	LOGFORMAT_CUSTOM = 300
};


// Optional log sub format param added to GetLogFileType()
enum LogSubFormat
{
	LOGSUBFORMAT_UNKNOWN,
	LOGSUBFORMAT_V5DATABASE_WEB,
	LOGSUBFORMAT_V5DATABASE_STREAMING
};


#define ISLOG_WEB(x)		(x>LOGFORMAT_WEBSERVERS && x<LOGFORMAT_WEBSERVERS_END)
#define ISLOG_STREAMING(x)	(x>LOGFORMAT_STREAMINGMEDIA && x<LOGFORMAT_STREAMINGMEDIA_END)
#define ISLOG_PROXY(x)		(x>LOGFORMAT_PROXYSERVERS && x<LOGFORMAT_PROXYSERVERS_END)
#define ISLOG_FIREWALL(x)	(x>LOGFORMAT_FIREWALLS && x<LOGFORMAT_FIREWALLS_END)
#define ISLOG_DB(x)			(x>LOGFORMAT_DATABASES && x<LOGFORMAT_DATABASES_END)


#include "HitData.h"		// for HitDataPtr.

short ProcessLogLine(char *buffer, HitDataPtr Line );
short PROC_Common(char *buffer,HitDataPtr Line);
short PROC_Netscape(char *buffer,HitDataPtr Line);
short PROC_Purveyer(char *buffer,HitDataPtr Line);
short PROC_MacHTTP(char *buffer,HitDataPtr Line);
short PROC_Website(char *buffer,HitDataPtr Line);
short PROC_IIS(char *buffer,HitDataPtr Line);
short PROC_IIS4(char *buffer,HitDataPtr Line);
short PROC_Netpresenz(char *buffer,HitDataPtr Line);
short PROC_Firstclass(char *buffer,HitDataPtr Line);
short PROC_Realserver(char *buffer,HitDataPtr Line);
short PROC_Filemaker(char *buffer,HitDataPtr Line);


void LOG_MacHTTP(char *buffer);
void LOG_IIS4(char *buffer);
void LOG_Netscape(char *buffer);
void LOG_Purveyer(char *buffer);

short Write_W3C( char *filename, HitDataPtr Line );
void Close_ConvertW3C( void );

#ifdef __cplusplus
}
#endif


#endif
