/*
 * alib.h (C)opyright 1992 Darren Reed.
 */
#define	ARES_INITLIST	1
#define	ARES_CALLINIT	2
#define ARES_INITSOCK	4
#define ARES_INITDEBG	8
#define ARES_INITCACH    16

#define MAXPACKET	1024
#define MAXALIASES	35
#define MAXADDRS	35

#define	RES_CHECKPTR	0x0400

struct	hent {
	char	*h_name;	/* official name of host */
	char	*h_aliases[MAXALIASES];	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	/* list of addresses from name server */
	struct	in_addr	h_addr_list[MAXADDRS];
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

struct	resinfo {
	char	*ri_ptr;
	int	ri_size;
};

struct	reslist {
	int	re_id;
	char	re_type;
	char	re_retries;
	char	re_resend;	/* send flag. 0 == dont resend */
	char	re_sends;
	char	re_srch;
	int	re_sent;
	u_long	re_sentat;
	u_long	re_timeout;
	struct	in_addr	re_addr;
	struct	resinfo	re_rinfo;
	struct	hent re_he;
	struct	reslist	*re_next, *re_prev;
	char	re_name[65];
	char	*nick;
	char	*host;
	char	*user;
	char	*channel;
	int	server;
	void	(*func)();
};

struct	hostent	*ar_answer(char *, int, void (*func)(struct reslist *) );
void    ar_close(void);
int     ar_delete(char *, int);
int     ar_gethostbyname(char *, char *, int, char *, char *, char *, char *, int, void (*func)());
int     ar_gethostbyaddr(char *, char *, int, char *, char *, char *, char *, int, void (*func)());
int     ar_init(int);
int     ar_open(void);
long    ar_timeout(time_t, char *, int, void (*func)(struct reslist *) );
long get_nslookup(fd_set *rd);
void wait_for_lookup( void );
