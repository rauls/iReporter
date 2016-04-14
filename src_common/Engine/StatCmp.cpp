#include "Compiler.h"

#include <cstdio>
#include <cstring>
#include <cctype>	// for isdigit.
#include <ctype.h>	// for isdigit?

#include "StatCmp.h"
#include "Stats.h"	// For Statistic & StatList.
#include "myansi.h"
#include "StatDefs.h"	// For MACOS_PROGRESS_IDLE


/*-----------------------------------------------------------------------------------------
** Comparison routines
*/

/* CompStat - compare two statistics string names (used in qsort()) */
long CompStat(void *p1, void *p2, long *result)
{
	char	*str1 = ((Statistic *)p1)->GetName();
	char	*str2 = ((Statistic *)p2)->GetName();
	MACOS_PROGRESS_IDLE

	*result=strcmpd(str1,str2);  //was strcmpi
	return 0;
}

/* CompStatNum - compare two statistics string names, evaluating numerical addresses */
long CompStatNum(void *p1, void *p2, long *result)
{
	int		n1, n2;
	char	*str1 = ((Statistic *)p1)->GetName();
	char	*str2 = ((Statistic *)p2)->GetName();

	MACOS_PROGRESS_IDLE

	if (isdigit(*str1) && isdigit(*str2)) {

		for (;;) {
			n1 = myatoi(str1);
			n2 = myatoi(str2);
			if (n1 == n2) {
				str1 = mystrchr(str1,'.');
				if (!str1) { *result=0; return 0; }
				str2 = mystrchr(str2,'.');
				if (!str2) { *result=0; return 0; }
				++str1;
				++str2;
			} else if (n1 < n2) {
				*result=-1;
				return(0);
			} else {
				*result=1;
				return(0);
			}
		}

	} else {
		*result=strcmpd(str1,str2);
		return 0;   //was strcmpi
	}
}

/* CompStatBytes - compare two statistics string bytes size (used in qsort()) */
long CompStatBytes(void *p1, void *p2, long *result)
{
	__int64 n1,n2;
	MACOS_PROGRESS_IDLE
	n1 = ((Statistic *)p1)->bytes;
	n2 = ((Statistic *)p2)->bytes;
	if (n2<n1)
		*result = -1;
	else
		*result = 1;
	return (0);
}

/* CompStatBytes - compare two statistics string bytes size (used in qsort()) */
long CompStatBytesIn(void *p1, void *p2, long *result)
{
	__int64 n1,n2;
	MACOS_PROGRESS_IDLE
	n1 = ((Statistic *)p1)->bytesIn;
	n2 = ((Statistic *)p2)->bytesIn;
	if (n2<n1)
		*result = -1;
	else
		*result = 1;
	return (0);
}


/* CompStatFiles - compare two statistics string bytes size (used in qsort()) */
long CompStatFiles(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->files;
	n2 = ((Statistic *)p2)->files;
	*result=n2-n1;
	return 0;
}

long CompStatCounter(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->counter;
	n2 = ((Statistic *)p2)->counter;
	*result=n2-n1;
	return 0;
}

long CompStatCounter2(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->counter2;
	n2 = ((Statistic *)p2)->counter2;
	*result=n2-n1;
	return 0;
}
long CompStatCounter3(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->counter3;
	n2 = ((Statistic *)p2)->counter3;
	*result=n2-n1;
	return 0;
}


long CompStatCounter4(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->counter4;
	n2 = ((Statistic *)p2)->counter4;
	*result=n2-n1;
	return 0;
}

long CompStatVisitIn(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->GetVisitIn();
	n2 = ((Statistic *)p2)->GetVisitIn();
	*result=n2-n1;
	return 0;
}

long CompStatErrors(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->errors;
	n2 = ((Statistic *)p2)->errors;
	*result=n2-n1;
	return 0;
}

long CompStatvisitTot(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->visitTot;
	n2 = ((Statistic *)p2)->visitTot;
	*result=n2-n1;
	return 0;
}


long CompStatVisits(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->visits;
	n2 = ((Statistic *)p2)->visits;
	*result=n2-n1;
	return 0;
}

/* CompStat - compare two statistics string names (used in qsort()) */
long CompStatDate(void *p1, void *p2, long *result)
{
	struct tm		dateTm;
	time_t			date1, date2;
	char			*str1 = ((Statistic *)p1)->GetName();
	char			*str2 = ((Statistic *)p2)->GetName();

	MACOS_PROGRESS_IDLE

	StringToDaysDate(str1, &dateTm, 0);
	Date2Days(&dateTm, &date1);

	StringToDaysDate(str2, &dateTm, 0);
	Date2Days(&dateTm, &date2);

	*result=(long)( date1 - date2 );
	return 0;
}


long CompStatLastDay(void *p1, void *p2, long *result)
{
	long n1,n2;
	MACOS_PROGRESS_IDLE

	n1 = ((Statistic *)p1)->lastday;
	n2 = ((Statistic *)p2)->lastday;
	*result=n1-n2;
	return 0;
}


// Compares by counter4 values and then by error values.
long CompStatCounter4ThenErrors(void *p1, void *p2, long *result)
{
	MACOS_PROGRESS_IDLE

	// first compare counter4 values
	*result=((Statistic *)p2)->counter4-((Statistic *)p1)->counter4;

	// only compare errors if counter4 values are the same
	if( !*result )
	{
		*result=((Statistic *)p2)->errors-((Statistic *)p1)->errors;
	}
	return 0;
}



