/*
 * arlib.c (C)opyright 1993 Darren Reed. All rights reserved.
 * This file may not be distributed without the author's permission in any
 * shape or form. The author takes no responsibility for any damage or loss
 * of property which results from the use of this software.
 */
//#include "irc.h"
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <resolv.h>
//#include "dcc.h"
//#include "misc.h"
#include "arlib.h"

#define HOSTLEN 100
extern	int	h_errno;
static	char	ar_hostbuf[HOSTLEN+1], ar_domainname[HOSTLEN+1];
static	char	ar_dot[] = ".";
static	int	ar_resfd = -1, ar_vc = 0;
static	struct	reslist	*ar_last = NULL, *ar_first = NULL;


static	int	do_query_name(struct resinfo *, char *, register struct reslist *, char *, char *, char *, char *, int , void (*func)());
static	int	do_query_number(struct resinfo *, char *, register struct reslist *, char *, char *, char *, char *, int , void (*func)());
static	int	ar_resend_query(struct reslist *);

static time_t now;

#define MAX_RETRIES 4

#ifndef HFIXEDSZ
#define HFIXEDSZ 12
#endif

#ifndef INT32SZ
#define INT32SZ 4
#endif

#define	getnow	time(0);


/*
 * malloc_strcpy:  Mallocs enough space for src to be copied in to where
 * ptr points to.
 *
 * Never call this with ptr pointinng to an uninitialised string, as the
 * call to free() might crash the client... - phone, jan, 1993.
 */
char *malloc_strcpy ( char **ptr, const char *src )
{
	if (!src)
		return free((void **)ptr);
	if (ptr && *ptr)
	{
		if (*ptr == src)
			return *ptr;
		return strcpy(*ptr, src);
	}
	*ptr = malloc(strlen(src) + 1);
	return strcpy(*ptr, src);
}

char	*m_strdup (const char *str)
{
	char *ptr;
	
	if (!str)
		str = "";
	ptr = (char *)malloc(strlen(str) + 1);
	return strcpy(ptr, str);
}




int set_non_blocking(int fd)
{
#ifdef NON_BLOCKING_CONNECTS
	int	res;

#if defined(NBLOCK_POSIX)
	int nonb = 0;
	nonb |= O_NONBLOCK;
#else
# if defined(NBLOCK_BSD)
	int nonb = 0;
	nonb |= O_NDELAY;
# else
#  if defined(NBLOCK_SYSV)
	res = 1;

	if (ioctl (fd, FIONBIO, &res) < 0)
		return -1;
#  else
#   error no idea how to set an fd to non-blocking 
#  endif
# endif
#endif
#if (defined(NBLOCK_POSIX) || defined(NBLOCK_BSD)) && !defined(NBLOCK_SYSV)
	if ((res = fcntl(fd, F_GETFL, 0)) == -1)
		return -1;
	else if (fcntl(fd, F_SETFL, res | nonb) == -1)
		return -1;
#endif
#endif
	return 0;
}

int set_blocking(int fd)
{
#ifdef NON_BLOCKING_CONNECTS
	int	res;

#if defined(NBLOCK_POSIX)
	int nonb = 0;
	nonb |= O_NONBLOCK;
#else
# if defined(NBLOCK_BSD)
	int nonb = 0;
	nonb |= O_NDELAY;
# else
#  if defined(NBLOCK_SYSV)
	res = 0;

	if (ioctl (fd, FIONBIO, &res) < 0)
		return -1;
#  else
#   error no idea how to return an fd blocking 
#  endif
# endif
#endif
#if (defined(NBLOCK_POSIX) || defined(NBLOCK_BSD)) && !defined(NBLOCK_SYSV)
	if ((res = fcntl(fd, F_GETFL, 0)) == -1)
		return -1;
	else if (fcntl(fd, F_SETFL, res &~ nonb) == -1)
		return -1;
#endif
#endif
	return 0;
}






/*
 * Statistics structure.
 */
static	struct	resstats {
	int	re_errors;
	int	re_nu_look;
	int	re_na_look;
	int	re_replies;
	int	re_requests;
	int	re_resends;
	int	re_sent;
	int	re_timeouts;
} ar_reinfo;



char empty_string[] = "";
#define	BIG_BUFFER_SIZE	1024
fd_set	readables, writables;




#include <sys/ioctl.h>
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif

#if defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX) && !defined(__EMX__)
# define IO_ARRAYLEN sysconf(_SC_OPEN_MAX)
#else
# ifdef FD_SETSIZE
#  define IO_ARRAYLEN FD_SETSIZE
# else
#  define IO_ARRAYLEN NFDBITS
# endif
#endif


#define MAX_SEGMENTS 16

typedef	struct	myio_struct
{
	char		*buffer;
	size_t		buffer_size;
	unsigned 	read_pos,
			write_pos;
	int		segments;
	int		error;
}           MyIO;

static	MyIO	**io_rec = NULL;

/*
 * Get_pending_bytes: What do you think it does?
 */
size_t get_pending_bytes (int fd)
{
	if (fd >= 0 && io_rec[fd] && io_rec[fd]->buffer)
		return strlen(io_rec[fd]->buffer);

	return 0;
}

static	void	init_io (void)
{
	static	int	first = 1;

	if (first)
	{
		int	c, max_fd = IO_ARRAYLEN;

		io_rec = (MyIO **)malloc(sizeof(MyIO *) * max_fd);
		for (c = 0; c < max_fd; c++)
			io_rec[c] = (MyIO *) 0;
		first = 0;
	}
}


static int global_max_fd = -1;

/*
 * new_select: works just like select(), execpt I trimmed out the excess
 * parameters I didn't need.  
 */
int new_select (fd_set *rd, fd_set *wd, struct timeval *timeout)
{
		int	i,
			set = 0;
		fd_set 	new;
	struct timeval	thetimeout;
	struct timeval *newtimeout = &thetimeout;

	if (timeout)
		thetimeout = *timeout;
	else
		newtimeout = NULL;

	if (!io_rec){
		printf("new select called before io_rec init\n");
		init_io();
	}

	FD_ZERO(&new);
printf( "newselect(), global_max_fd= %d\n", global_max_fd);
	
	for (i = 0; i <= global_max_fd; i++)
	{
		printf( "newselect(), checking iorec %d\n", i );
		if (io_rec[i])
		{
			if ((io_rec[i]->read_pos < io_rec[i]->write_pos) &&
				mystrchr(io_rec[i]->buffer + io_rec[i]->read_pos, '\n'))
			{
				FD_SET(i, &new);
				set++;
			}
		}
	}

	if (set)
	{
		*rd = new;
		return set;
	}
	return (select(global_max_fd + 1, rd, wd, NULL, newtimeout));
}

/*
 * Register a filedesc for readable events
 * Set up its input buffer
 */
int 	new_open (int des)
{
	if (des < 0)
		return des;		/* Invalid */

	if (!io_rec)
		init_io();

	if (!FD_ISSET(des, &readables))
		FD_SET(des, &readables);
	if (des > global_max_fd)
		global_max_fd = des;
		
	return des;
}

int 	new_open_write (int des)
{
	if (des < 0)
		return des;		/* Invalid */

	if (!io_rec)
		init_io();

	if (!FD_ISSET(des, &writables))
		FD_SET(des, &writables);
#if 0
	if (des > global_max_fd)
		global_max_fd = des;
#endif		
	return des;
}


/*
 * Unregister a filedesc for readable events 
 * and close it down and free its input buffer
 */
int	new_close (int des)
{
	if (des < 0)
		return -1;

	if (FD_ISSET(des, &readables))
		FD_CLR(des, &readables);

	if (io_rec && io_rec[des])
	{
	        free(&(io_rec[des]->buffer));
        	free((char **)&(io_rec[des]));
	}
	close(des);

	/*
	 * If we're closing the highest fd in use, then we
	 * want to adjust global_max_fd downward to the next highest fd.
	 */
	if (des == global_max_fd)
	{
		do
			des--;
		while (des >= 0 && !FD_ISSET(des, &readables));

		global_max_fd = des;
	}
	return -1;
}

int	new_close_write (int des)
{
	if (des < 0)
		return -1;

	if (FD_ISSET(des, &writables))
		FD_CLR(des, &writables);

	if (io_rec && io_rec[des])
	{
	        free(&(io_rec[des]->buffer));
        	free((char **)&(io_rec[des]));
	}
	close(des);

#if 0
	/*
	 * If we're closing the highest fd in use, then we
	 * want to adjust global_max_fd downward to the next highest fd.
	 */
	if (des == global_max_fd)
	{
		do
			des--;
		while (!FD_ISSET(des, &writables));

		global_max_fd = des;
	}
#endif
	return -1;
}

/* set's socket options */
void set_socket_options (int s)
{
	int	opt = 1;
	int	optlen = sizeof(opt);
#ifndef NO_STRUCT_LINGER
	struct linger	lin;

	lin.l_onoff = lin.l_linger = 0;
	setsockopt(s, SOL_SOCKET, SO_LINGER, (char *)&lin, optlen);
#endif

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, optlen);
	opt = 1;
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt, optlen);

#if notyet
	/* This is waiting for nonblock-aware code */
	info = fcntl(fd, F_GETFL, 0);
	info |= O_NONBLOCK;
	fcntl(fd, F_SETFL, info);
#endif
}






/*
 * ar_init
 *
 * Initializes the various ARLIB internal varilables and related DNS
 * options for res_init().
 *
 * Returns 0 or the socket opened for use with talking to name servers
 * if 0 is passed or ARES_INITSOCK is set.
 */
int	ar_init(int op)
{
	int	ret = 0;

	if (op & ARES_INITLIST)
	{
		memset(&ar_reinfo, 0, sizeof(ar_reinfo));
		ar_first = ar_last = NULL;
	}

	if (op & ARES_CALLINIT && !(_res.options & RES_INIT))
	{
		ret = res_init();
		(void)strcpy(ar_domainname, ar_dot);
		(void)strncat(ar_domainname, _res.defdname, HOSTLEN-2);
		if (!_res.nscount)
		{
			_res.nscount = 1;
			_res.nsaddr_list[0].sin_addr.s_addr = inet_addr("127.0.0.1");
		}
	}

	if (op & ARES_INITSOCK)
		ret = ar_resfd = ar_open();

	if (op & ARES_INITDEBG)
		_res.options |= RES_DEBUG;

	if (op == 0)
		ret = ar_resfd;

	return ret;
}

/*
 * ar_open
 *
 * Open a socket to talk to a name server with.
 * Check _res.options to see if we use a TCP or UDP socket.
 */
int	ar_open(void)
{
	if (ar_resfd == -1)
	{
		if (_res.options & RES_USEVC)
		{
			struct	sockaddr_in *sip;
			int	i = 0;

			sip = _res.nsaddr_list;
			ar_vc = 1;
			ar_resfd = socket(AF_INET, SOCK_STREAM, 0);

			/*
			 * Try each name server listed in sequence until we
			 * succeed or run out.
			 */
			while (connect(ar_resfd, (struct sockaddr *)sip++,
					sizeof(struct sockaddr)))
			{
				(void)close(ar_resfd);
				ar_resfd = -1;
				if (i >= _res.nscount)
					break;
				ar_resfd = socket(AF_INET, SOCK_STREAM, 0);
			}
		}
		else
		{
			int on = 0;
			ar_resfd = socket(AF_INET, SOCK_DGRAM, 0);
			(void) setsockopt(ar_resfd, SOL_SOCKET, SO_BROADCAST,(char *)&on, sizeof(on));
		}
	}
	if (ar_resfd >= 0)
	{	/* Need one of these two here - and it MUST work!! */
		if (set_non_blocking(ar_resfd) < 0)
		{
			(void)close(ar_resfd);
			ar_resfd = -1;
		}
	}
	return ar_resfd;
}

/*
 * ar_close
 *
 * Closes and flags the ARLIB socket as closed.
 */
void	ar_close(void)
{
	(void)close(ar_resfd);
	ar_resfd = -1;
	return;
}

/*
 * ar_add_request
 *
 * Add a new DNS query to the end of the query list.
 */
static	int	ar_add_request(struct reslist *new)
{
	if (!new)
		return -1;
	if (!ar_first)
		ar_first = ar_last = new;
	else 
	{
		ar_last->re_next = new;
		ar_last = new;
	}
	new->re_next = NULL;
	ar_reinfo.re_requests++;
	return 0;
}

/*
 * ar_remrequest
 *
 * Remove a request from the list. This must also free any memory that has
 * been allocated for temporary storage of DNS results.
 *
 * Returns -1 if there are anyy problems removing the requested structure
 * or 0 if the remove is successful.
 */
static	int	ar_remrequest(struct reslist *old)
{
	struct	reslist	*rptr, *r2ptr;
	register char	**s;

	if (!old)
		return -1;
	for (rptr = ar_first, r2ptr = NULL; rptr; rptr = rptr->re_next)
	{
		if (rptr == old)
			break;
		r2ptr = rptr;
	}

	if (!rptr)
		return -1;
	if (rptr == ar_first)
		ar_first = ar_first->re_next;
	else if (rptr == ar_last)
	{
		if ((ar_last = r2ptr))
			ar_last->re_next = NULL;
	}
	else
		r2ptr->re_next = rptr->re_next;

	if (!ar_first)
		ar_last = ar_first;
	if (rptr->re_he.h_name)
		free(&rptr->re_he.h_name);
	if ((s = rptr->re_he.h_aliases))
	{
		for (; *s; s++)
			free(s);
		free(&rptr->re_he.h_aliases);
	}
#if 0
	if ((s = rptr->re_he.h_addr_list))
	{
		/* CDE memleak detected here */
		for (; *s; s++)
			free(s);
		free(&rptr->re_he.h_addr_list);
	}
#endif
	if (rptr->re_rinfo.ri_ptr)
		free(&rptr->re_rinfo.ri_ptr);

	free(&rptr->nick);
	free(&rptr->user);
	free(&rptr->host);
	free(&rptr->channel);
	free(&rptr);

	return 0;
}

/*
 * ar_make_request
 *
 * Create a DNS query recorded for the request being made and place it on the
 * current list awaiting replies.  Initialization of the record with set
 * values should also be done.
 */
static	struct	reslist	*ar_make_request(register struct resinfo *resi, char *nick, char *user, char *h, char *chan, int server, void (*func)())
{
	register struct	reslist	*rptr;
	register struct resinfo *rp;

	rptr = (struct reslist *)malloc(sizeof(struct reslist));
	rp = &rptr->re_rinfo;

	rptr->re_next    = NULL; /* where NULL is non-zero ;) */
	rptr->re_sentat  = now;
	rptr->re_retries = MAX_RETRIES;/*_res.retry + 2;*/
	rptr->re_sends = 1;
	rptr->re_resend  = 1;
	rptr->re_timeout = rptr->re_sentat + _res.retrans;
	rptr->re_he.h_name = NULL;
	rptr->re_he.h_addrtype   = AF_INET;
	rptr->re_he.h_aliases[0] = NULL;
	rp->ri_ptr = resi->ri_ptr;
	rp->ri_size = resi->ri_size;
	if (nick)
		malloc_strcpy(&rptr->nick, nick);
	if (user)
		malloc_strcpy(&rptr->user, user);
	if (h)
		malloc_strcpy(&rptr->host, h);
	if (chan)
		rptr->channel = m_strdup(chan);
	rptr->server = server;
	if (func)
		rptr->func = func;
	(void)ar_add_request(rptr);

	return rptr;
}



/*
 * ar_timeout
 *
 * Remove queries from the list which have been there too long without
 * being resolved.
 */
long	ar_timeout(time_t now, char *info, int size, void (*func)(struct reslist *) )
{
	register struct	reslist	*rptr, *r2ptr;
	register long	next = 0;

	for (rptr = ar_first, r2ptr = NULL; rptr; rptr = r2ptr)
	{
		r2ptr = rptr->re_next;
		if (now >= rptr->re_timeout)
		{
			/*
			 * If the timeout for the query has been exceeded,
			 * then resend the query if we still have some
			 * 'retry credit' and reset the timeout. If we have
			 * used it all up, then remove the request.
			 */
			if (--rptr->re_retries <= 0)
			{
				ar_reinfo.re_timeouts++;
				if (info && rptr->re_rinfo.ri_ptr)
					bcopy(rptr->re_rinfo.ri_ptr, info,
						MIN(rptr->re_rinfo.ri_size, size));
				if (rptr->func)
					(*rptr->func)(rptr);
				else if (func)
					(*func)(rptr);
				(void)ar_remrequest(rptr);
				return now;
			}
			else
			{
				rptr->re_sends++;
				rptr->re_sentat = now;
				rptr->re_timeout = now + _res.retrans;
				(void)ar_resend_query(rptr);
			}
		}
		if (!next || rptr->re_timeout < next)
			next = rptr->re_timeout;
	}
	return next;
}

/*
 * ar_send_res_msg
 *
 * When sending queries to nameservers listed in the resolv.conf file,
 * don't send a query to every one, but increase the number sent linearly
 * to match the number of resends. This increase only occurs if there are
 * multiple nameserver entries in the resolv.conf file.
 * The return value is the number of messages successfully sent to 
 * nameservers or -1 if no successful sends.
 */
static	int	ar_send_res_msg(char *msg, int len, int rcount)
{
	register int	i;
	int	sent = 0;

	if (!msg)
		return -1;

	rcount = (_res.nscount > rcount) ? rcount : _res.nscount;
	if (_res.options & RES_PRIMARY)
		rcount = 1;
	if (!rcount)
		rcount = 1;
	if (ar_vc)
	    {
		ar_reinfo.re_sent++;
		sent++;
		if (write(ar_resfd, msg, len) == -1)
		    {
			int errtmp = errno;
			(void)close(ar_resfd);
			errno = errtmp;
			ar_resfd = -1;
		    }
	    }
	else
		for (i = 0; i < rcount; i++)
		{
			_res.nsaddr_list[i].sin_family = AF_INET;
			if (sendto(ar_resfd, msg, len, 0,
				   (struct sockaddr *)&(_res.nsaddr_list[i]),
				sizeof(struct sockaddr_in)) == len)
			{
				ar_reinfo.re_sent++;
				sent++;
			}
		}
	return (sent) ? sent : -1;
}

/*
 * ar_find_id
 *
 * find a dns query record by the id (id is determined by dn_mkquery)
 */
static	struct	reslist	*ar_find_id(int id)
{
	register struct	reslist	*rptr;

	for (rptr = ar_first; rptr; rptr = rptr->re_next)
		if (rptr->re_id == id)
			return rptr;
	return NULL;
}

/*
 * ar_delete
 *
 * Delete a request from the waiting list if it has a data pointer which
 * matches the one passed.
 */
int	ar_delete(char *ptr, int size)
{
	register struct	reslist	*rptr;
	register struct	reslist	*r2ptr;
	int	removed = 0;

	for (rptr = ar_first; rptr; rptr = r2ptr)
	    {
		r2ptr = rptr->re_next;
		if (rptr->re_rinfo.ri_ptr && ptr && size &&
		    memcmp(rptr->re_rinfo.ri_ptr, ptr, size) == 0)
		    {
			(void)ar_remrequest(rptr);
			removed++;
		    }
	    }
	return removed;
}

/*
 * ar_query_name
 *
 * generate a query based on class, type and name.
 */
static	int	ar_query_name(char *name, int class, int type, struct reslist *rptr)
{
	static	char buf[MAXPACKET];
	int	r,s;
	HEADER	*hptr;

	memset(buf, 0, sizeof(buf));
	r = res_mkquery(QUERY, name, class, type, NULL, 0, NULL,
			buf, sizeof(buf));
	if (r <= 0)
	    {
		h_errno = NO_RECOVERY;
		return r;
	    }
	hptr = (HEADER *)buf;
	rptr->re_id = ntohs(hptr->id);

	s = ar_send_res_msg(buf, r, rptr->re_sends);

	if (s == -1)
	    {
		h_errno = TRY_AGAIN;
		return -1;
	    }
	else
		rptr->re_sent += s;
	return 0;
}

/*
 * ar_gethostbyname
 *
 * Replacement library function call to gethostbyname().  This one, however,
 * doesn't return the record being looked up but just places the query in the
 * queue to await answers.
 */
int	ar_gethostbyname(char *name, char *info, int size, char *nick, char *user, char *h, char *chan, int server, void (*func)())
{
	char	host[HOSTLEN+1];
	struct	resinfo	resi;
	register struct resinfo *rp = &resi;

	if (size && info)
	{
		rp->ri_ptr = (char *)malloc(size+1);
		if (*info)
			bcopy(info, rp->ri_ptr, size);
		rp->ri_size = size;
	}
	else
		memset((char *)rp, 0, sizeof(resi));
	ar_reinfo.re_na_look++;
	(void)strncpy(host, name, 64);
	host[64] = '\0';

	return (do_query_name(rp, host, NULL, nick, user, h, chan, server, func));
}

static	int	do_query_name(struct resinfo *resi, char *name, register struct reslist *rptr, char *nick, char *user, char *h, char *chan, int server, void (*func)())
{
	char	hname[HOSTLEN];
	int	len;

	len = strlen((char *)strncpy(hname, name, sizeof(hname)-1));

	if (rptr && (hname[len-1] != '.'))
	{
		(void)strncat(hname, ar_dot, sizeof(hname)-len-1);
		/*
		 * NOTE: The logical relationship between DNSRCH and DEFNAMES
		 * is implies. ie no DEFNAES, no DNSRCH.
		 */
		if ((_res.options & (RES_DEFNAMES|RES_DNSRCH)) ==
		    (RES_DEFNAMES|RES_DNSRCH))
		{
			if (_res.dnsrch[(int)rptr->re_srch])
				(void)strncat(hname, _res.dnsrch[(int)rptr->re_srch],
					sizeof(hname) - ++len -1);
		}
		else if (_res.options & RES_DEFNAMES)
			(void)strncat(hname, ar_domainname, sizeof(hname) - len -1);
	}

	/*
	 * Store the name passed as the one to lookup and generate other host
	 * names to pass onto the nameserver(s) for lookups.
	 */
	if (!rptr)
	{
		rptr = ar_make_request(resi, nick, user, h, chan, server, func);
		rptr->re_type = T_A;
		(void)strncpy(rptr->re_name, name, sizeof(rptr->re_name)-1);
	}
	return (ar_query_name(hname, C_IN, T_A, rptr));
}

/*
 * ar_gethostbyaddr
 *
 * Generates a query for a given IP address.
 */
int	ar_gethostbyaddr(char *addr, char *info, int size, char *nick, char *user, char *h, char *chan, int server, void (*func)())
{
	struct	resinfo	resi;
	register struct resinfo *rp = &resi;

	if (size && info)
	{
		rp->ri_ptr = (char *)malloc(size+1);
		if (*info)
			bcopy(info, rp->ri_ptr, size);
		rp->ri_size = size;
	}
	else
		memset((char *)rp, 0, sizeof(resi));
	ar_reinfo.re_nu_look++;
	return (do_query_number(rp, addr, NULL, nick, user, h, chan, server, func));
}



/*
 * do_query_number
 *
 * Use this to do reverse IP# lookups.
 */
static	int	do_query_number(struct resinfo *resi, char *numb, register struct reslist *rptr, char *nick, char *user, char *h, char *chan, int server, void (*func)())
{
	register unsigned char	*cp;
	static	char	ipbuf[32];

	/*
	 * Generate name in the "in-addr.arpa" domain.  No addings bits to this
	 * name to get more names to query!.
	 */
	cp = (unsigned char *)numb;
	(void)sprintf(ipbuf,"%u.%u.%u.%u.in-addr.arpa.",
			(unsigned int)(cp[3]), (unsigned int)(cp[2]),
			(unsigned int)(cp[1]), (unsigned int)(cp[0]));
printf( "making request... %s\n", ipbuf );
	if (!rptr)
	    {
		rptr = ar_make_request(resi, nick, user, h, chan, server, func);
		rptr->re_type = T_PTR;
		rptr->re_he.h_length = sizeof(struct in_addr);
		memcpy((char *)&rptr->re_addr, numb, rptr->re_he.h_length);
		memcpy((char *)&rptr->re_he.h_addr_list[0].s_addr, numb, rptr->re_he.h_length);
	    }
	return (ar_query_name(ipbuf, C_IN, T_PTR, rptr));
}

/*
 * ar_resent_query
 *
 * resends a query.
 */
static	int	ar_resend_query(struct reslist *rptr)
{
	if (!rptr->re_resend)
		return -1;

	switch(rptr->re_type)
	{
	case T_PTR:
		ar_reinfo.re_resends++;
		return do_query_number(NULL, (char *)&rptr->re_addr, rptr, NULL, NULL, NULL, NULL, -1, NULL);
	case T_A:
		ar_reinfo.re_resends++;
		return do_query_name(NULL, rptr->re_name, rptr, NULL, NULL, NULL, NULL, -1, NULL);
	default:
		break;
	}

	return -1;
}

/*
 * ar_procanswer
 *
 * process an answer received from a nameserver.
 */
static	int	ar_procanswer(struct reslist *rptr, HEADER *hptr, char *buf, char *eob)
{
	char	*cp, **alias;
	int	class, type, dlen, len, ans = 0, n;
	unsigned int ttl, dr, *adr;
	struct	hent	*hp;

	cp = buf + HFIXEDSZ;
	adr = (unsigned int *)rptr->re_he.h_addr_list;

	while (*adr)
		adr++;

	alias = rptr->re_he.h_aliases;
	while (*alias)
		alias++;

	hp = &rptr->re_he;


	/*
	 * Skip over the original question.
	 */
	while (hptr->qdcount-- > 0)
		cp += dn_skipname(cp, eob) + QFIXEDSZ;
	/*
	 * proccess each answer sent to us. blech.
	 */
	while (hptr->ancount-- > 0 && cp < eob) {
		n = dn_expand(buf, eob, cp, ar_hostbuf, sizeof(ar_hostbuf)-1);
		cp += n;
		if (n <= 0)
			return ans;

		ans++;
		/*
		 * 'skip' past the general dns crap (ttl, class, etc) to get
		 * the pointer to the right spot.  Some of thse are actually
		 * useful so its not a good idea to skip past in one big jump.
		 */
		type = (int)_getshort(cp);
		cp += sizeof(short);
		class = (int)_getshort(cp);
		cp += sizeof(short);
		ttl = (unsigned int)_getlong(cp);
		cp += INT32SZ;
		dlen =  (int)_getshort(cp);
		cp += sizeof(short);
		rptr->re_type = type;

		switch(type)
		{
		case T_A :
			rptr->re_he.h_length = dlen;
			if (ans == 1)
				rptr->re_he.h_addrtype=(class == C_IN) ?
							AF_INET : AF_UNSPEC;
			memcpy(&dr, cp, dlen);
			*adr++ = dr;
			*adr = 0;
			cp += dlen;
			len = strlen(ar_hostbuf);
			if (!rptr->re_he.h_name)
				malloc_strcpy(&rptr->re_he.h_name, ar_hostbuf);
 			break;
		case T_PTR :
			if ((n = dn_expand(buf, eob, cp, ar_hostbuf,
					   sizeof(ar_hostbuf)-1 )) < 0)
			    {
				cp += n;
				continue;
			    }
			ar_hostbuf[HOSTLEN] = 0;
			cp += n;
			len = strlen(ar_hostbuf)+1;
			/*
			 * copy the returned hostname into the host name
			 * or alias field if there is a known hostname
			 * already.
			 */
			if (!rptr->re_he.h_name)
				malloc_strcpy(&rptr->re_he.h_name, ar_hostbuf);
			else
			{
				*alias = (char *)malloc(len);
				strcpy(*alias++, ar_hostbuf);
				*alias = NULL;
			}
			break;
		case T_CNAME :
			cp += dlen;
			if (alias >= &(rptr->re_he.h_aliases[MAXALIASES-1]))
				continue;
			n = strlen(ar_hostbuf)+1;
			*alias = (char *)malloc(n);
			(void)strcpy(*alias++, ar_hostbuf);
			*alias = NULL;
			break;
		default :
			break;
		}
	}

	return ans;
}

/*
 * ar_answer
 *
 * Get an answer from a DNS server and process it.  If a query is found to
 * which no answer has been given to yet, copy its 'info' structure back
 * to where "reip" points and return a pointer to the hostent structure.
 */
struct	hostent	*ar_answer(char *reip, int size, void (*func)(struct reslist *) )
{
	static	char	ar_rcvbuf[HFIXEDSZ + MAXPACKET];
	static	struct	hostent	ar_host;

	register HEADER	*hptr;
	register struct	reslist	*rptr = NULL;
	register struct hostent *hp;
	register char **s;
	unsigned long	*adr;
	int	rc, i, n, a;

	rc = recv(ar_resfd, ar_rcvbuf, sizeof(ar_rcvbuf), 0);
	if (rc <= 0)
		goto getres_err;

	ar_reinfo.re_replies++;
	hptr = (HEADER *)ar_rcvbuf;
	/*
	 * convert things to be in the right order.
	 */
	hptr->id = ntohs(hptr->id);
	hptr->ancount = ntohs(hptr->ancount);
	hptr->arcount = ntohs(hptr->arcount);
	hptr->nscount = ntohs(hptr->nscount);
	hptr->qdcount = ntohs(hptr->qdcount);
	/*
	 * response for an id which we have already received an answer for
	 * just ignore this response.
	 */
	rptr = ar_find_id(hptr->id);
	if (!rptr)
		goto getres_err;

	if ((hptr->rcode != NOERROR) || (hptr->ancount == 0))
	    {
		switch (hptr->rcode)
		{
		case NXDOMAIN:
			h_errno = HOST_NOT_FOUND;
			break;
		case SERVFAIL:
			h_errno = TRY_AGAIN;
			break;
		case NOERROR:
			h_errno = NO_DATA;
			break;
		case FORMERR:
		case NOTIMP:
		case REFUSED:
		default:
			h_errno = NO_RECOVERY;
			break;
		}
		ar_reinfo.re_errors++;
		/*
		** If a bad error was returned, we stop here and dont send
		** send any more (no retries granted).
		*/
		if (h_errno != TRY_AGAIN)
		    {
			rptr->re_resend = 0;
			rptr->re_retries = 0;
		    }
		goto getres_err;
	    }

	a = ar_procanswer(rptr, hptr, ar_rcvbuf, ar_rcvbuf+rc);

	if ((rptr->re_type == T_PTR) && (_res.options & RES_CHECKPTR))
	    {
		/*
		 * For reverse lookups on IP#'s, lookup the name that is given
		 * for the ip# and return with that as the official result.
		 * -avalon
		 */
		rptr->re_type = T_A;
		/*
		 * Clean out the list of addresses already set, even though
		 * there should only be one :)
		 */
		adr = (unsigned long *)rptr->re_he.h_addr_list;
		while (*adr)
			*adr++ = 0L;
		/*
		 * Lookup the name that we were given for the ip#
		 */
		ar_reinfo.re_na_look++;
		(void)strncpy(rptr->re_name, rptr->re_he.h_name,
			sizeof(rptr->re_name)-1);
		rptr->re_he.h_name = NULL;
		rptr->re_retries = MAX_RETRIES;/*_res.retry + 2;*/
		rptr->re_sends = 1;
		rptr->re_resend = 1;
		if (rptr->re_he.h_name)
			free(&rptr->re_he.h_name);
		ar_reinfo.re_na_look++;
		(void)ar_query_name(rptr->re_name, C_IN, T_A, rptr);
		return NULL;
	    }

	if (reip && rptr->re_rinfo.ri_ptr && size)
		memcpy(reip, rptr->re_rinfo.ri_ptr,
			MIN(rptr->re_rinfo.ri_size, size));

	/*
	 * Clean up structure from previous usage.
	 */
	hp = &ar_host;
	if (hp->h_name)
		free(&hp->h_name);
	if ((s = hp->h_aliases))
	{
		while (*s)
		{
			free(s);
			s++;
		}
		free(&hp->h_aliases);
	}
	if ((s = hp->h_addr_list))
	{
	
		while (*s)
		{
			free(s);
			s++;
		}
		free(&hp->h_addr_list);
		hp->h_addr_list = NULL;
	}
	memset ((char *)hp, 0, sizeof(*hp));
	/*
	 * Setup and copy details for the structure we return a pointer to.
	 */
	hp->h_addrtype = AF_INET;
	hp->h_length = sizeof(struct in_addr);
	malloc_strcpy(&hp->h_name, rptr->re_he.h_name);
	/*
	 * Count IP#'s.
	 */
	for (i = 0, n = 0; i < MAXADDRS; i++, n++)
		if (!rptr->re_he.h_addr_list[i].s_addr)
			break;
	s = hp->h_addr_list = (char **)malloc((n + 1) * sizeof(char *));
	if (n)
	{
		*s = (char *)malloc(sizeof(struct in_addr));
		memcpy(*s, (char *)&rptr->re_he.h_addr_list[0].s_addr,
			sizeof(struct in_addr));
		s++;
		for (i = 1; i < n; i++, s++)
		{
			*s = (char *)malloc(sizeof(struct in_addr));
			memcpy(*s, (char *)&rptr->re_he.h_addr_list[i].s_addr,
				sizeof(struct in_addr));
		}
	}
	*s = NULL;
	/*
	 * Count CNAMEs
	 */
	for (i = 0, n = 0; i < MAXADDRS; i++, n++)
		if (!rptr->re_he.h_aliases[i])
			break;
	s = hp->h_aliases = (char **)malloc((n + 1) * sizeof(char *));
	for (i = 0; i < n; i++)
	{
		*s++ = rptr->re_he.h_aliases[i];
		rptr->re_he.h_aliases[i] = NULL;
	}
	*s = NULL;
	if (rptr->func)
		(*rptr->func)(rptr);
	else if (func)
		(*func)(rptr);
	if (a > 0)
		(void)ar_remrequest(rptr);
	else
		if (!rptr->re_sent)
			(void)ar_remrequest(rptr);
	return hp;

getres_err:
	if (rptr)
	{
		if (reip && rptr->re_rinfo.ri_ptr && size)
			memcpy(reip, rptr->re_rinfo.ri_ptr,
				MIN(rptr->re_rinfo.ri_size, size));
		if ((h_errno != TRY_AGAIN) &&
		    ((_res.options & (RES_DNSRCH|RES_DEFNAMES)) ==
		     (RES_DNSRCH|RES_DEFNAMES) ))
			if (_res.dnsrch[(int)rptr->re_srch])
			{
				rptr->re_retries = MAX_RETRIES; /*_res.retry + 2;*/
				rptr->re_sends = 1;
				rptr->re_resend = 1;
				(void)ar_resend_query(rptr);
				rptr->re_srch++;
			}
		return NULL;
	}
	return NULL;
}



void print_ns_succede(struct reslist *rptr)
{
char *u, *n, *h;
	u = rptr->user ? rptr->user : empty_string;
	h = rptr->host;
	n = rptr->nick ? rptr->nick : empty_string;
//	if (do_hook(NSLOOKUP_LIST, "%s %s %s %s %s", h, rptr->re_he.h_name?rptr->re_he.h_name:"", (char *)inet_ntoa(rptr->re_he.h_addr_list[0]), n, u))
	{
		int i;
		char buffer[BIG_BUFFER_SIZE];
		struct in_addr ip;
		*buffer = 0;
		if (rptr->nick && rptr->user)
			printf("[%s!%s@%s]: %s", n, u, h, rptr->re_he.h_name ? rptr->re_he.h_name: "invalid hostname");
		else
			printf("%s is %s (%s)", h, rptr->re_he.h_name ? rptr->re_he.h_name:"invalid hostname", (char *)inet_ntoa(rptr->re_he.h_addr_list[0]));
		for (i = 0; rptr->re_he.h_addr_list[i].s_addr; i++)
		{
			bcopy(&rptr->re_he.h_addr_list[i], (char *)&ip, sizeof(ip));
			strcat( buffer, ", " );
			strcat( buffer, inet_ntoa(ip) );
			//strpcat(buffer, "[%s] ", inet_ntoa(ip));
			if (strlen(buffer) > 490)
				break;
		}
		printf("IPs: %s", buffer);
	    for (i = 0; rptr->re_he.h_aliases[i]; i++)
			printf("\talias %d = %s", i+1, rptr->re_he.h_aliases[i]);
	}

}

void print_ns_fail(struct reslist *rptr)
{
//	if (do_hook(NSLOOKUP_LIST, "%s %s %s", rptr->host, rptr->nick?rptr->nick:empty_string, rptr->user?rptr->user:empty_string))
	{
		if (rptr->nick && rptr->user)
			printf("nslookup of %s!%s@%s failed.", rptr->nick, rptr->user, rptr->host);
		else
			printf("nslookup of host %s failed.", rptr->host);
	}
}

void set_nslookupfd(fd_set *rd)
{
int s;
	if ((s = ar_init(0)) != -1)
		FD_SET(s, rd);
}

static int ar_seq = 0;
static int ar_lookup = 0;


char *do_nslookup(char *host, char *nick, char *user, char *chan, int server, void (*func)())
{
#ifndef WINNT
struct in_addr temp1;
int s;

	if (!host)
		return NULL;

	if ((s = ar_init(0)) == -1)
		ar_init(ARES_INITLIST|ARES_INITSOCK|ARES_CALLINIT);

	ar_lookup++;
	if (isdigit(*(host + strlen(host) - 1)))
	{
		ar_seq++;
		temp1.s_addr = inet_addr(host);
		ar_gethostbyaddr((char *)&temp1.s_addr, (char *)&ar_seq, sizeof(ar_seq), nick, user, host, chan, server, func);
	}
	else
	{
		ar_seq++;
		ar_gethostbyname(host, (char *)&ar_seq, sizeof(ar_seq), nick, user, host, chan, server, func);
	}
#endif
	return NULL;
}

long get_nslookup(fd_set *rd)
{
struct hostent *hp = NULL;
int ar_del = 0;
int s;
	now = time(0);
printf( "0\n" );fflush(stdout);
	if ((s = ar_init(0)) == -1)
		return -1;
printf( "1\n" );fflush(stdout);
	if (!(FD_ISSET(s, rd)))
	{
		unsigned long when = 0;
printf( "1b\n" );fflush(stdout);
		when = ar_timeout(now, (char *)&ar_del, sizeof(ar_del), print_ns_fail);
		if (ar_del)
			ar_lookup--;
		return when;
	}
printf( "2\n" );fflush(stdout);
	if ((hp = ar_answer((char *)&ar_del, sizeof(ar_del), print_ns_succede)))
	{
		char **s;
printf( "3\n" );fflush(stdout);
		ar_lookup--;
		free(&hp->h_name);
		if ((s = hp->h_aliases))
		{
			while (*s)
			{
				free(s);
				s++;
			}
			free(&hp->h_aliases);
		}
		if ((s = hp->h_addr_list))
		{
			while (*s)
			{
				free(s);
				s++;
			}
			free(&hp->h_addr_list);
		}
	}
printf( "4\n" );fflush(stdout);
	return -1;
}


void wait_for_lookup( void )
{
	struct	timeval timer;

	FD_ZERO( &readables );
	FD_ZERO( &writables );
	set_nslookupfd( &readables );

	memset( &timer, 0, sizeof(struct timeval ) );
	timer.tv_usec = 50;
	timer.tv_sec = 0;
	
	now = time(NULL);
	get_nslookup( &readables );
/*	switch (new_select(&readables, &writables, &timer ))
	{
		case 0: case -1:
			printf( "new_select got 0,-1\n" );fflush(stdout);
			break;
		default:
			printf( "new_select got default:\n" );fflush(stdout);
			now = time(NULL);
			get_nslookup( &readables );
			break;
	}
*/
	now = time(NULL);
}
