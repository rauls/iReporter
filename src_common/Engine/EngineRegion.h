
#ifndef	ENGINEREGION_H
#define	ENGINEREGION_H

#define UNKNOWN		0
#define AFRICA		1
#define ANTARTICA	2
#define ASIA		3
#define NAMERICA	4
#define SAMERICA	5
#define OCEANIA		6
#define EUROPE		7

typedef struct
{
	char		fCode[8]; 				//	Country code
	long		fCountryId;				//	Country name id (language file string id)
	long		fRegion;				//  REGION on earth
} DomainRegionMap, *DomainRegionMapPtr,**DomainRegionMapHndl;

/////////////////////////////////////////////////////////////////////////
//	Used by "C" modules.
/////////////////////////////////////////////////////////////////////////
#ifdef	__cplusplus
	extern "C" {
#endif
	void initLookupGrid(void);
#ifdef	__cplusplus
	}
#endif

char *LookupCountryRegion( char *code, long *region );
long LookupRegion(char *code);
char *LookupDomain(char *code);

#endif
