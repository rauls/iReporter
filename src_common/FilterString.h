#ifndef	FILTER_STRING_H
#define	FILTER_STRING_H

#include "config_struct.h"	// for struct FilterData
#include <cstdlib>			// for size_t

namespace QS {

bool	LookupReportFilter(char cFilterCharacter, int iReportType);
int		FormatFilterString(struct FilterData* pFilterData, char* szDest, size_t nStrLen, int iReportType);


}


#endif