//=================================================================================================
// Copyright (C) Software 2001, 2002
//
// File:            smtp.h
// Subsystem:       Networking
// Created By:      Raul, 04/10/2001
// Description:     Send a message to a FREESMS server
//
// $Archive: /iReporter/Dev/src_common/networking/SMSNotify.h $
// $Revision: 1 $
// $Author: Raul $
// $Date: 6/10/2001 5:25p $
//================================================================================================= 



#include "Compiler.h"
#include <stdio.h>
#include <io.h>		// for close()
#include <assert.h>
#include <errno.h>

#if DEF_WINDOWS
#include <windows.h>
#include <winsock.h>
#include "registry.h"
#include "resource.h"
#include "Winutil.h"
#include "WinDNR.h"
#include "Winmain.h"	// for StatusSet()
#elif DEF_UNIX
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <netdb.h>
#include "main.h"
#define	GetHostByName	gethostbyname
#define	SOCKET	int
#endif

#include "smtp.h"
#include "myansi.h"
#include "datetime.h"


// szUrl must contain the servers URL with fields for $USER, $PASS, $NUMBER, $MESSAGE
// ie, 
int SendSMSMessage( char *szPhoneNumber, char *szUrl, char *szUsername, char *szPassword, char *szMessage )
{

	return 0;
}
