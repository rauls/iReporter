/*  Metrowerks Standard Library  Version 2.1.2  1997 May  */

/*
 *	ansi_parms.h
 *	
 *		Copyright © 1995-1997 Metrowerks, Inc.
 *		All rights reserved.
 */
 
#ifndef __ansi_parms__
#define __ansi_parms__

#define	__MODENALIB__	/*soon to be obsolete...*/	
#define __MSL__		212		

/*	970402 bkoz 
	long long only supported on MIPS/PPC/68K CW11 tools and older
	and is on by default 
*/
#if __MC68K__ || __POWERPC__  || (__MOTO__ >= 30903)
	#define __MSL_LONGLONG_SUPPORT__				 /* mm 970110 */		
#endif

/*	970415 bkoz 
	define this if you would like FPCE functionality in math.h 
*/
/* #define __MSL_C9X__ */

#ifdef __cplusplus

#define __extern_c								extern "C" {
#define __end_extern_c						}

#define __std(ref)								/*std::*/ref

#define __namespace(space)				//namespace space {
#define __end_namespace(space)		//}

#define	__using_namespace(space)	//using namespace space;

#define __stdc_space(space)				//__c ## space ## _space

#define __import_stdc_into_std_space(space)	
/*	__namespace(std)															
		__using_namespace(__stdc_space(space))			
	__end_namespace(std)
*/
#else

#define __extern_c
#define __end_extern_c

#define __std(ref)								ref

#define __namespace(space)
#define __end_namespace(space)

#define	__using_namespace(space)

#define __stdc_space(space)

#define __import_stdc_into_std_space(space)

#endif /* __cplusplus */

#define __undef_os		0
#define __mac_os		1
#define __be_os			2
#define __win32_os		3
#define __powertv_os	4


#if	__dest_os	== __win32_os
#define	__tls	 __declspec(thread)
#define __LITTLE_ENDIAN
#else
#define	__tls 
#endif

#endif /* ndef __ansi_parms__ */

/*     Change record
*/
