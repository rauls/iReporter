/* net_io.h */

/* Interface file for networking calls across several platforms */

#ifndef _NETIO_H
#define _NETIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FWA.h"

#ifdef DEF_WINDOWS
#include "winnet_io.h"
#endif

#ifdef DEF_UNIX
#include "unixnet_io.h"
#endif

#ifdef DEF_MAC
#include "OTStuff.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
