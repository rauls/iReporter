/* Registration.h */
/*
	This file should be included for any code that requires serial number routines, to hide
	
	different implementations on different platforms
*/

#include "FWA.h"

#ifdef DEF_MAC
	#include "SerialRegMac.h"
#else
	#include "serialReg.h"		// assumed that this works for WinUnix
#endif