//=================================================================================================
// Copyright © Analysis Software 2001, 2002
//
// File:            OutputMessages.cpp
// Subsystem:       Networking
// Created By:      Zoltan, 13/07/01
// Description:     Moved some common message output functions from the Windows and UNIX versions
//					of remotecontrol.c to here.
//
// $Archive: /iReporter/Dev/src_common/networking/OutputMessages.cpp $
// $Revision: 2 $
// $Author: Zoltan $
// $Date: 1/08/01 1:36p $
//================================================================================================= 

//================================================================================================= 
// System Includes
//================================================================================================= 

#include "FWA.h"

#include <malloc.h>
#include <cstdio>
#include <cassert>
#include <cstring>


//================================================================================================= 
// Project Includes
//================================================================================================= 

#if DEF_WINDOWS
	#include "WinMain.h"	
#elif DEF_UNIX
	#include "main.h"
#elif DEF_MAC
	#include "MacStatus.h"
#endif					// for StatusSet() - ugh!
#include "Config.h"		// for OutputDegug()


//================================================================================================= 
// Initialisations & Declarations
//================================================================================================= 

extern void ShowRecievedLine( char* pre, char* txt )
{
	assert(pre);
	assert(txt);

	size_t txtLen( strlen(txt) );

	if( txtLen )
	{
		static const char formatStr[]="%s REC: '%s' %x";

		// will allocate bit more than need but that's ok
		char* buf=reinterpret_cast<char*>( alloca( txtLen + sizeof(formatStr) + strlen(pre) ) );

		sprintf( buf, formatStr, pre, txt, txt[txtLen-1] );
		StatusSet( buf );
	}
}


extern "C" void DebugRecievedLine( char* pre, char* txt )
{
	assert(pre);
	assert(txt);

	static const char formatStr[]="%s REC: '%s'";

	// will allocate bit more than need but that's ok
	char* buf=reinterpret_cast<char*>( alloca( strlen(txt) + sizeof(formatStr) + strlen(pre) ) );

	sprintf( buf, formatStr, pre, txt );
	OutDebug( buf );
}