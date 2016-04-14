
#ifndef	STATCMP_H
#define	STATCMP_H


long CompStat(			void *p1, void *p2, long *result);
long CompStatNum(		void *p1, void *p2, long *result);
long CompStatBytes(		void *p1, void *p2, long *result);
long CompStatBytesIn(	void *p1, void *p2, long *result);
long CompStatFiles(		void *p1, void *p2, long *result);
long CompStatCounter(	void *p1, void *p2, long *result);
long CompStatCounter2(	void *p1, void *p2, long *result);
long CompStatCounter3(	void *p1, void *p2, long *result);
long CompStatCounter4(	void *p1, void *p2, long *result);
long CompStatVisitIn(	void *p1, void *p2, long *result);
long CompStatErrors(	void *p1, void *p2, long *result);
long CompStatvisitTot(	void *p1, void *p2, long *result);
long CompStatDate(		void *p1, void *p2, long *result);
long CompStatLastDay(	void *p1, void *p2, long *result);
long CompStatVisits(	void *p1, void *p2, long *result);
long CompStatCounter4ThenErrors(void *p1, void *p2, long *result);

#endif