/*
**	FileName	:	compiler.h
**
**	Purpose		:	This include is ONLY for compiler specific configurations.
**					It should be included in any file that needs compilter
**					switches.
**
**	Usage		:	It is important that this is included BEFORE our other includes.
**
**	History		Author	Comment
**	22/06/01	JMC		Created with only VC++ pragma 4786.
**
*/

#ifndef COMPILER_H
#define COMPILER_H


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		Defines for Microsoft Visual C++ on Windows
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef	_MSC_VER
	/*************************************************************
	**	Always good to have the Compiler & Version built-in
	*************************************************************/
	#pragma	comment( compiler )

	/*************************************************************
	**	Disable some of the warnings.
	*************************************************************/
	#pragma warning (disable:4209)	/* benign typedef redefinition				*/
	#pragma warning (disable:4786)	/* identifier was truncated to 255 characters in the debug information */

	/*************************************************************
	**	Create new pragmas Warning and ShowDefine.
	*************************************************************/
	#define	def_Prefix			"i> "		/* "warning " */
	#define	def_VarAsStr(x)		#x
	#define	def_VarVal(x)		def_VarAsStr(x)
	#define	WarningExp(desc)	message(__FILE__ "(" def_VarVal(__LINE__) ") : " def_Prefix desc)
	#define	Warning(desc)		message(__FILE__ "(" def_VarVal(__LINE__) ") : " def_Prefix #desc)
	#define	ShowDefine(def)		message(__FILE__ "(" def_VarVal(__LINE__) ") : " def_Prefix #def "\t= [" def_VarAsStr(def) "]")
	/*************************************************************
	**	#pragma Usage:
	**************************************************************
	**	#pragma Warning(Double click this line to get to the source code)
	**	
	**	displays->	c:\junk\testapp\main.cpp(36) : i> Double click this line to get to the source code
	**
	**
	**	If we have some macros which we want to dump to the Build winodw.
	**	#define	DEF1	"asd"
	**	#define	DEF2	33
	**	#define	DEF3
	**
	**	#pragma ShowDefine(DEF1)	displays->	c:\junk\testapp\main.cpp(44) : c> DEF1 = ["asd"]
	**	#pragma ShowDefine(DEF2)	displays->	c:\junk\testapp\main.cpp(45) : c> DEF2 = [33]
	**	#pragma ShowDefine(DEF3)	displays->	c:\junk\testapp\main.cpp(46) : c> DEF3 = []
	**	#pragma ShowDefine(DEF4)	displays->	c:\junk\testapp\main.cpp(47) : c> DEF4 = [DEF4]
	*************************************************************/
	
	#define	DEF_WINDOWS	1

	#ifdef _DEBUG
		#define	DEF_DEBUG 1
	#endif	// _DEBUG

	typedef unsigned __int64 __uint64;

#endif // _MSC_VER


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		Defines for Metroworks on Macintosh
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __MWERKS__
	#if	__dest_os == __mac_os		// sanity check
		#define	DEF_MAC		1

		typedef SInt64 __int64;
		typedef UInt64 __uint64;
	#else
		#error "Not sure what compiler version we've got here!"
	#endif
#endif // __MWERKS__


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		Defines for GNU on Unix Variants
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__
	#define DEF_UNIX	1
	#ifdef _SUNOS
		#include "sys/int_const.h"
		#include "sys/int_types.h"
		#include "sys/types.h"
		typedef int64_t  __int64;
		typedef uint64_t __uint64;
	#else
		#include "sys/types.h"
		typedef int64_t __int64;
		typedef u_int64_t __uint64;
	#endif
#endif	// __GNUC__


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		Defines for all compilers
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Error handling stuff 
#ifdef DEF_DEBUG
	#include <assert.h>
	#define DEF_PRECONDITION(a)		assert((a))
	#define DEF_POSTCONDITION(a)		assert((a))
	#define DEF_PRECONDITION2(a,b)	assert((b, (a)))
	#define DEF_POSTCONDITION2(a,b)	assert((b, (a)))
	#define DEF_ASSERT(a)			assert((a))
#else
	#define DEF_PRECONDITION(a)
	#define DEF_POSTCONDITION(a)
	#define DEF_PRECONDITION2(a,b)
	#define DEF_POSTCONDITION2(a,b)
	#define DEF_ASSERT(a)
#endif

// Generic types
typedef __uint64 filesize_t;

#endif
// COMPILER_H