// NTService.cpp
//
// Implementation of CNTService

#include <windows.h>
#include <stdio.h>
#include "NTService.h"

// static variables
CNTService* CNTService::m_pThis = NULL;

CNTService::CNTService(const char* szServiceName, long ver, long minver )
{
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize=sizeof(vi);  // init this.
	GetVersionEx(&vi);      //lint !e534
	m_bWinNT = (vi.dwPlatformId == VER_PLATFORM_WIN32_NT);
    
	// copy the address of the current object so we can access it from
    // the static member callback functions. 
    // WARNING: This limits the application to only one CNTService object. 
    m_pThis = this;
    
    // Set the default service name and version
    strncpy(m_szServiceName, szServiceName, sizeof(m_szServiceName)-1);
    m_iMajorVersion = ver;
    m_iMinorVersion = minver;
    m_hEventSource = NULL;

    // set up the initial service status 
    m_hServiceStatus = NULL;
    m_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_Status.dwCurrentState = SERVICE_STOPPED;
    m_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_Status.dwWin32ExitCode = 0;
    m_Status.dwServiceSpecificExitCode = 0;
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    m_bIsRunning = FALSE;
}

CNTService::~CNTService()
{
    //DebugMsg("CNTService::~CNTService()");
    if (m_hEventSource) {
        ::DeregisterEventSource(m_hEventSource);
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// Default command line argument parsing

// Returns TRUE if it found an arg it recognised, FALSE if not
// Note: processing some arguments causes output to stdout to be generated.
BOOL CNTService::ParseStandardArgs(int argc, char* argv[], int verbose, char *errmsg )
{
	char szErr[256];

    // See if we have any command line args we recognise
    if (argc <= 1) return FALSE;


	if (_stricmp(argv[1], "-ntstart") == 0) {
		StartupService();
	} else 
	if (_stricmp(argv[1], "-ntend") == 0) {
		EndService();
	} else 
	if (_stricmp(argv[1], "-ntv") == 0) {
        // Spit out version info
		if ( verbose )
			sprintf( errmsg, "%s Version %d.%d\n",
            m_szServiceName, m_iMajorVersion, m_iMinorVersion);
		if ( verbose )
			sprintf( errmsg, "The service is %s installed\n",
               IsInstalled() ? "currently" : "not");
		if ( verbose ) DebugMsg( errmsg );
        return TRUE; // say we processed the argument
    } else

	if (_stricmp(argv[1], "-bootinstall") == 0 ) {
        // Request to install.
        if (IsInstalled()) {
			if ( verbose )
				sprintf( errmsg, "%s is already installed\n", m_szServiceName);
        } else {
            // Try and install the copy that's running
            if (Install( TRUE )) {
				if ( verbose )
	                sprintf( errmsg, "%s installed\n", m_szServiceName);
            } else {
				if ( verbose )
	                sprintf( errmsg, "%s failed to install. Error %s\n", m_szServiceName, GetLastErrorText(szErr,256) );
            }
        }
		if ( verbose ) DebugMsg( errmsg );
        return TRUE; // say we processed the argument
    } else

	if (_stricmp(argv[1], "-i") == 0 || _stricmp(argv[1], "-nti") == 0 ) {
        // Request to install.
        if (IsInstalled()) {
			if ( verbose )
				sprintf( errmsg, "%s is already installed\n", m_szServiceName);
        } else {
            // Try and install the copy that's running
            if (Install( FALSE )) {
				if ( verbose )
	                sprintf( errmsg, "%s installed\n", m_szServiceName);
            } else {
				if ( verbose )
	                sprintf( errmsg, "%s failed to install. Error %s\n", m_szServiceName, GetLastErrorText(szErr,256) );
            }
        }
		if ( verbose ) DebugMsg( errmsg );
        return TRUE; // say we processed the argument
    } else

	if (_stricmp(argv[1], "-u") == 0 || _stricmp(argv[1], "-ntu") == 0 ) {

        // Request to uninstall.
        if (!IsInstalled()) {
			if ( verbose )
	            sprintf( errmsg, "%s is not installed\n", m_szServiceName);
        } else {
            // Try and remove the copy that's installed
            if (Uninstall()) {
                // Get the executable file path
                char szFilePath[_MAX_PATH];
                ::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
				if ( verbose )
	                sprintf( errmsg, "%s removed.\n", m_szServiceName, szFilePath);
            } else {
				if ( verbose )
	                sprintf( errmsg, "Could not remove %s. Error %s\n", m_szServiceName, GetLastErrorText(szErr,256) );
            }
        }
		if ( verbose ) DebugMsg( errmsg );
        return TRUE; // say we processed the argument
    
    }
    // Don't recognise the args
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
// Install/uninstall routines

// Test if the service is currently installed
BOOL CNTService::IsInstalled()
{
    BOOL bResult = FALSE;

    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (hSCM) {

        // Try to open the service
        SC_HANDLE hService = ::OpenService(hSCM,
                                           m_szServiceName,
                                           SERVICE_QUERY_CONFIG);
        if (hService) {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }

        ::CloseServiceHandle(hSCM);
    }
    
    return bResult;
}


long CNTService::CheckStatus()
{
    long bResult = FALSE;

    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (hSCM) {

        // Try to open the service
        SC_HANDLE hService = ::OpenService(hSCM,
                                           m_szServiceName,
                                           SERVICE_ALL_ACCESS);
        if (hService) {
			SERVICE_STATUS ServiceStatus;
			QueryServiceStatus( hService, &ServiceStatus );
			bResult = ServiceStatus.dwCurrentState;
            ::CloseServiceHandle(hService);
        }

        ::CloseServiceHandle(hSCM);
    }
    
    return bResult;
}







void CNTService::GetModulePath( char *p )
{
    ::GetModuleFileName(NULL, p, 256 );
}

extern "C" int	gDebugPrint;

BOOL CNTService::Install( int autoboot )
{
	long st;

    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (!hSCM) return FALSE;

    // Get the executable file path
    char szFilePath[_MAX_PATH];
    ::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
	strcat( szFilePath, " -ntservice" );

	if ( gDebugPrint )
		strcat( szFilePath, " -debug" );

	if ( autoboot )
		st = SERVICE_AUTO_START;
	else
		st = SERVICE_DEMAND_START;

    // Create the service
    SC_HANDLE hService = ::CreateService(hSCM,
                                         m_szServiceName,
                                         m_szServiceName,
                                         SERVICE_ALL_ACCESS,
                                         SERVICE_WIN32_OWN_PROCESS,
                                         st,        // start condition
                                         SERVICE_ERROR_NORMAL,
                                         szFilePath,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);
    if (!hService) {
        ::CloseServiceHandle(hSCM);
        return FALSE;
    }

    // make registry entries to support logging messages
    // Add the source name as a subkey under the Application
    // key in the EventLog service portion of the registry.
    char szKey[256];
    HKEY hKey = NULL;
    strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
    strcat(szKey, m_szServiceName);
    if (::RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) != ERROR_SUCCESS) {
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hSCM);
        return FALSE;
    }

    // Add the Event ID message-file name to the 'EventMessageFile' subkey.
    ::RegSetValueEx(hKey,
                    "EventMessageFile",
                    0,
                    REG_EXPAND_SZ, 
                    (CONST BYTE*)szFilePath,
                    strlen(szFilePath) + 1);     

    // Set the supported types flags.
    DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    ::RegSetValueEx(hKey,
                    "TypesSupported",
                    0,
                    REG_DWORD,
                    (CONST BYTE*)&dwData,
                     sizeof(DWORD));
    ::RegCloseKey(hKey);

    LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_INSTALLED, m_szServiceName);

    // tidy up
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return TRUE;
}

BOOL CNTService::Uninstall()
{
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (!hSCM) return FALSE;

    BOOL bResult = FALSE;
    SC_HANDLE hService = ::OpenService(hSCM,
                                       m_szServiceName,
                                       DELETE);
    if (hService) {
        if (::DeleteService(hService)) {
            LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_REMOVED, m_szServiceName);
            bResult = TRUE;
        } else {
            LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_NOTREMOVED, m_szServiceName);
        }
        ::CloseServiceHandle(hService);
    }
    
    ::CloseServiceHandle(hSCM);
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////
// Logging functions

// This function makes an entry into the application event log
void CNTService::LogEvent(WORD wType, DWORD dwID,
                          const char* pszS1,
                          const char* pszS2,
                          const char* pszS3)
{
    const char* ps[3];
    ps[0] = pszS1;
    ps[1] = pszS2;
    ps[2] = pszS3;

    int iStr = 0;
    for (int i = 0; i < 3; i++) {
        if (ps[i] != NULL) iStr++;
    }
        
    // Check the event source has been registered and if
    // not then register it now
    if (!m_hEventSource) {
        m_hEventSource = ::RegisterEventSource(NULL,  // local machine
                                               m_szServiceName); // source name
    }

    if (m_hEventSource) {
        ::ReportEvent(m_hEventSource,
                      wType,
                      0,
                      dwID,
                      NULL, // sid
                      iStr,
                      0,
                      ps,
                      NULL);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Service startup and registration

BOOL CNTService::StartDispatcher()
{
    SERVICE_TABLE_ENTRY st[] = {
        {m_szServiceName, ServiceMain},
        {NULL, NULL}
    };
	if ( gDebugPrint )
	    DebugMsg("Service is in debug mode.");

    //DebugMsg("Calling StartServiceCtrlDispatcher()");
    BOOL b = ::StartServiceCtrlDispatcher(st);
    //DebugMsg("Returned from StartServiceCtrlDispatcher()");
    return b;
}

// static member function (callback)
void CNTService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Get a pointer to the C++ object
    CNTService* pService = m_pThis;
    
    //pService->DebugMsg("Entering CNTService::ServiceMain()");
    // Register the control request handler
    pService->m_Status.dwCurrentState = SERVICE_START_PENDING;
    pService->m_hServiceStatus = RegisterServiceCtrlHandler(pService->m_szServiceName,
                                                           Handler);
    if (pService->m_hServiceStatus == NULL) {
        pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_CTRLHANDLERNOTINSTALLED);
        return;
    }

    // Start the initialisation
    if (pService->Initialize()) {

        // Do the real work. 
        // When the Run function returns, the service has stopped.
        pService->m_bIsRunning = TRUE;
        pService->m_Status.dwWin32ExitCode = 0;
        pService->m_Status.dwCheckPoint = 0;
        pService->m_Status.dwWaitHint = 0;
        pService->Run();
    }

    // Tell the service manager we are stopped
    pService->SetStatus(SERVICE_STOPPED);

    //pService->DebugMsg("Leaving CNTService::ServiceMain()");
}

///////////////////////////////////////////////////////////////////////////////////////////
// status functions

void CNTService::SetStatus(DWORD dwState)
{
    //DebugMsg("CNTService::SetStatus(%lu, %lu)", m_hServiceStatus, dwState);
    m_Status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_Status);
}

///////////////////////////////////////////////////////////////////////////////////////////
// Service initialization

BOOL CNTService::Initialize()
{
    DebugMsg("Entering ::Initialize()");

    // Start the initialization
    SetStatus(SERVICE_START_PENDING);
    
    // Perform the actual initialization
    BOOL bResult = OnInit(); 
    
    // Set final state
    m_Status.dwWin32ExitCode = GetLastError();
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    if (!bResult) {
        LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_FAILEDINIT);
        SetStatus(SERVICE_STOPPED);
        return FALSE;    
    }
    
    LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STARTED);
    SetStatus(SERVICE_RUNNING);

    DebugMsg("Leaving ::Initialize()");
    return TRUE;
}



LPTSTR CNTService :: GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize ) {
    LPTSTR lpszTemp = 0;

    DWORD dwRet =	::FormatMessage(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
						0,
						GetLastError(),
						LANG_NEUTRAL,
						(LPTSTR)&lpszTemp,
						0,
						0
					);

    if( !dwRet || (dwSize < dwRet+14) )
        lpszBuf[0] = TEXT('\0');
    else {
        lpszTemp[strlen(lpszTemp)-2] = TEXT('\0');  //remove cr/nl characters
        strcpy(lpszBuf, lpszTemp);
    }

    if( lpszTemp )
        LocalFree(HLOCAL(lpszTemp));

    return lpszBuf;
}

BOOL CNTService :: EndService() {
	BOOL bRet = FALSE;
	char szErr[256];

	SC_HANDLE schSCManager = ::OpenSCManager(
								0,						// machine (NULL == local)
								0,						// database (NULL == default)
								SC_MANAGER_ALL_ACCESS	// access required
							);
	if( schSCManager ) {
		SC_HANDLE schService =	::OpenService(
									schSCManager,
									m_szServiceName,
									SERVICE_ALL_ACCESS
								);

		if( schService ) {
			// try to stop the service
			if( ::ControlService(schService, SERVICE_CONTROL_STOP, &m_Status) ) {
				DebugMsg( "Stopping %s." , m_szServiceName);
				::Sleep(1000);

				while( ::QueryServiceStatus(schService, &m_Status) ) {
					if( m_Status.dwCurrentState == SERVICE_STOP_PENDING ) {
						DebugMsg(".");
						::Sleep( 1000 );
					} else
						break;
				}

				if( m_Status.dwCurrentState == SERVICE_STOPPED )
					bRet = TRUE, DebugMsg( "%s stopped.", m_szServiceName );
                else
                    DebugMsg( "%s failed to stop - %s", m_szServiceName, GetLastErrorText(szErr,256));
			}

			::CloseServiceHandle(schService);
		} else {
			DebugMsg( "OpenService failed - %s", GetLastErrorText(szErr,256));
		}

        ::CloseServiceHandle(schSCManager);
    } else {
		DebugMsg( "OpenSCManager failed - %s", GetLastErrorText(szErr,256));
	}

	return bRet;
}


BOOL CNTService :: StartupService() {
	BOOL bRet = FALSE;
	char szErr[256];

	SC_HANDLE schSCManager = ::OpenSCManager(
								0,						// machine (NULL == local)
								0,						// database (NULL == default)
								SC_MANAGER_ALL_ACCESS	// access required
							);
	if( schSCManager ) {
		SC_HANDLE schService =	::OpenService(
									schSCManager,
									m_szServiceName,
									SERVICE_ALL_ACCESS
								);

		if( schService ) {
			// try to start the service
			DebugMsg( "Starting up %s." , m_szServiceName);
			if( ::StartService(schService, 0, 0) ) {
				Sleep(1000);

				while( ::QueryServiceStatus(schService, &m_Status) ) {
					if( m_Status.dwCurrentState == SERVICE_START_PENDING ) {
						DebugMsg( "." );
						Sleep( 1000 );
					} else
						break;
				}

				if( m_Status.dwCurrentState == SERVICE_RUNNING )
					bRet = TRUE, DebugMsg( "%s started." , m_szServiceName);
                else
                    DebugMsg( "\n%s failed to start - %s\n" , m_szServiceName, GetLastErrorText(szErr,256));
			} else {
				// StartService failed
				DebugMsg( "\n%s failed to start: %s " , m_szServiceName, GetLastErrorText(szErr,256));
			}

			::CloseServiceHandle(schService);
		} else {
			DebugMsg( "OpenService failed - %s " , GetLastErrorText(szErr,256));
		}

        ::CloseServiceHandle(schSCManager);
    } else {
		DebugMsg( "OpenSCManager failed - %s " , GetLastErrorText(szErr,256));
	}

	return bRet;
}



///////////////////////////////////////////////////////////////////////////////////////////////
// main function to do the real work of the service

// This function performs the main work of the service. 
// When this function returns the service has stopped.
void CNTService::Run()
{
 //   DebugMsg("Entering CNTService::Run()");

    while (m_bIsRunning) {
        DebugMsg("Sleeping...");
        Sleep(5000);
    }

    // nothing more to do
  //  DebugMsg("Leaving CNTService::Run()");
}

//////////////////////////////////////////////////////////////////////////////////////
// Control request handlers

// static member function (callback) to handle commands from the
// service control manager
void CNTService::Handler(DWORD dwOpcode)
{
    // Get a pointer to the object
    CNTService* pService = m_pThis;
    
    pService->DebugMsg("CNTService::Handler(%lu)", dwOpcode);
    switch (dwOpcode) {
    case SERVICE_CONTROL_STOP: // 1
        pService->SetStatus(SERVICE_STOP_PENDING);
        pService->OnStop();
        pService->m_bIsRunning = FALSE;
        pService->LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STOPPED);
        break;

    case SERVICE_CONTROL_PAUSE: // 2
        pService->OnPause();
        break;

    case SERVICE_CONTROL_CONTINUE: // 3
        pService->OnContinue();
        break;

    case SERVICE_CONTROL_INTERROGATE: // 4
        pService->OnInterrogate();
        break;

    case SERVICE_CONTROL_SHUTDOWN: // 5
        pService->OnShutdown();
        break;

    default:
        if (dwOpcode >= SERVICE_CONTROL_USER) {
            if (!pService->OnUserControl(dwOpcode)) {
                pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
            }
        } else {
            pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
        }
        break;
    }

    // Report current status
    pService->DebugMsg("Updating status (%lu, %lu)",
                       pService->m_hServiceStatus,
                       pService->m_Status.dwCurrentState);
    ::SetServiceStatus(pService->m_hServiceStatus, &pService->m_Status);
}
        
// Called when the service is first initialized
BOOL CNTService::OnInit()
{
    DebugMsg("CNTService::OnInit()");
	return TRUE;
}

// Called when the service control manager wants to stop the service
void CNTService::OnStop()
{
    DebugMsg("CNTService::OnStop()");
}

// called when the service is interrogated
void CNTService::OnInterrogate()
{
    DebugMsg("CNTService::OnInterrogate()");
}

// called when the service is paused
void CNTService::OnPause()
{
    DebugMsg("CNTService::OnPause()");
}

// called when the service is continued
void CNTService::OnContinue()
{
    DebugMsg("CNTService::OnContinue()");
}

// called when the service is shut down
void CNTService::OnShutdown()
{
    DebugMsg("CNTService::OnShutdown()");
}

// called when the service gets a user control message
BOOL CNTService::OnUserControl(DWORD dwOpcode)
{
    DebugMsg("CNTService::OnUserControl(%8.8lXH)", dwOpcode);
    return FALSE; // say not handled
}


////////////////////////////////////////////////////////////////////////////////////////////
// Debugging support
extern void NTSLogSchedule( char *txt );

void CNTService::DebugMsg(const char* pszFormat, ...)
{
    char buf[1024];
    sprintf(buf, "[%s](%lu): ", m_szServiceName, GetCurrentThreadId());
	va_list arglist;
	va_start(arglist, pszFormat);
    vsprintf(&buf[strlen(buf)], pszFormat, arglist);
	va_end(arglist);
    strcat(buf, "\n");
    OutputDebugString(buf);
	NTSLogSchedule( buf );
}

#include <io.h>
#include <fcntl.h>

//!! TCW MOD - function to create console for faceless apps if not already there
void CNTService::SetupConsole() {
	if( !m_fConsoleReady ) {
		AllocConsole();	// you only get 1 console.

		// lovely hack to get the standard io (printf, getc, etc) to the new console. Pretty much does what the
		// C lib does for us, but when we want it, and inside of a Window'd app.
		// The ugly look of this is due to the error checking (bad return values. Remove the if xxx checks if you like it that way.
		DWORD astds[3]={STD_OUTPUT_HANDLE,STD_ERROR_HANDLE,STD_INPUT_HANDLE};
		FILE *atrgs[3]={stdout,stderr,stdin};
		for( register int i=0; i<3; i++ ) {
			long hand=(long)GetStdHandle(astds[i]);
			if( hand!=(long)INVALID_HANDLE_VALUE ) {
				int osf=_open_osfhandle(hand,_O_TEXT);
				if( osf!=-1 ) {
					FILE *fp=_fdopen(osf,(astds[i]==STD_INPUT_HANDLE) ? "r" : "w");
					if( fp!=NULL ) {
						*(atrgs[i])=*fp;
						setvbuf(fp,NULL,_IONBF,0);
					}
				}
			}
		}
		m_fConsoleReady=TRUE;
	}
}
