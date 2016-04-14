#ifndef	__RESDEFS_H
#define __RESDEFS_H

#include "FWA.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	kEmptyString		0

#ifdef DEF_MAC
	#include "resource_strings.h"
	#define	GetString( id, dest, size )	((size_t)mystrncpy( dest, ReturnString(id), size ));
#else
// other platforms use this mess
#ifdef DEF_WINDOWS
	#include "resource.h"
	#define	ids_str	long
	size_t GetString( long id, char *dest, size_t size );
	char *ReturnString( long id );
#else
	// --------- UNIX STUFF ---------
	#define	ids_str	char *
	#include "resource_strings.h"
#endif
// mess end
#endif

#ifdef __cplusplus
}
#endif

#endif // __RESDEFS_H
