//--------------------------------------------------------------------------------------
/*
**	FileName	:	FWA.h
**
**	Purpose		:	This include is for controlling version builds.
**
**	Usage		:	It is important that this is included BEFORE our other includes.
**
**	History		Author	Comment
**	27/08/01	RHF		Includes "Compiler.h" file for compiler options 
**
*/
//--------------------------------------------------------------------------------------

#ifndef DEF_HEADER
#define DEF_HEADER

#include "Compiler.h"									// never remove !!!

//--------------------------------------------------------------------------------------
// Multi-Platform macros go here
//--------------------------------------------------------------------------------------

//#define DEF_FREEVERSION	1						// for free version, independent of Std/Ent flag


//--------------------------------------------------------------------------------------
// Specific Platform macros go here
//--------------------------------------------------------------------------------------

#ifdef DEF_MAC
	#if __ide_target("iReporter 2.0")
		#define	DEF_FULLVERSION		1
	#elif __ide_target("iReporter 2.0 free")
		#define DEF_FREEVERSION	1
	#endif

	#ifdef DEF_FULLVERSION
		#define	_PRO				1					// for backwards-compatibility
		#define	DEF_PRO				1					// for backwards-compatibility
	#endif

	#define DEF_APP_45									// comment out for 4.0.x builds

	#define	kNewStyleAlerts			1					// for code-based alert dialogs				
	#define kUsePDFManual			1
	#define kUseCarbonEvents		1
#endif

#ifdef DEF_WINDOWS

#endif

#ifdef DEF_UNIX

#endif


#endif	// DEF_HEADER