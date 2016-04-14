// NTService.cpp
// 
// This is the main program file containing the entry point.

#include "NTServApp.h"
#include "myservice.h"


static     CMyService MyService;

int IsServiceInstalled( long flag )
{
	static long last_result, req=0;
	if ( ((req%250) == 0) || flag ){
		last_result = MyService.IsInstalled();
		req = 0;
	}
	req++;
	return last_result;
}

int IsServiceRunning( void )
{
	long	status;
	
	status = MyService.CheckStatus();

	if ( status == SERVICE_RUNNING  )
		return TRUE;
	else
		return FALSE;
}

int InitNTService(int argc, char* argv[], int allowprintf, char *errmsg )
{
    // Create the service object
    
    // Parse for standard arguments (install, uninstall, version etc.)
    if (!MyService.ParseStandardArgs(argc, argv, allowprintf, errmsg )) {

        // Didn't find any standard args so start the service
        // Uncomment the DebugBreak line below to enter the debugger
        // when the service is started.
        //DebugBreak();
		MyService.DebugMsg( "StartDispatcher..." );
        MyService.StartDispatcher();
    } // else	MyService.DebugMsg( "ParseStandardArgs failed" );
    // When we get here, the service has been stopped
    return MyService.m_Status.dwWin32ExitCode;
}

void GetEXEPath( char *newPath )
{
	MyService.GetModulePath( newPath );
}
