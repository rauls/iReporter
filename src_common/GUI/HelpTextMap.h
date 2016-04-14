
#ifndef	HELPTEXTMAP_h
#define	HELPTEXTMAP_h

// *******************************************************************
// The MAC build makes unique access to controls using the following
// macro.  The arguements are as follows:
//	X	- The OutlookBar ID	(or enum)
//	Y	- The Tab ID (or enum)
//	Z	- The Control ID (or enum)
// *******************************************************************
#ifdef	DEF_MAC
#	define			MAKEID(X,Y,Z)	( ((X)<<16) | ((Y)<<8) | (Z) )
#endif


bool		HelpText_Load(void);
const char*	HelpText_Lookup(unsigned int uiControlId);
const char*	HelpText_LookupDesc(unsigned int uiControlId);



#endif