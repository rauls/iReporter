/*************************************************************************************
** Filename:		DNSCache.h
**
** Overview:		
**
** Creation:		Julien Crawford						28/06/2001
*************************************************************************************/

#ifndef		CQDNSCache_h
#define		CQDNSCache_h

class CQDNSCache {
public:
	CQDNSCache(long incr,int part);
	~CQDNSCache();

	typedef	struct {
		long	ip;
		char	*name;
		long	expire;		// date to expire at. (default NOW+2weeks???)

		long	next;
	} DNScache, *DNScachePtr;
	
	void				Add( long ip, char *name, char *exp );
	char				*Lookup( char *ipstring );
	char				*LookupIP( long ip );
	long 				SaveLookups( void );
	long 				LoadLookups( void );
	
private:
	long				num;
	long				maxNum;
	long				incrNum;

	DNScache			*DNSdata;
	//char				**list;			// Format is "<code>\0<name>\0"

	long				headref[256];	// first digits of IP#
	int					partialCompare;
};

#endif
