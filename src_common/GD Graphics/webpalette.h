//=================================================================================================
//
// File:            webpalette.h
// Subsystem:       GD Graphics
// Description:     Declarations for webpalette array created via bin2c.  This file was created
//					in order to stop mulitple inclusions of the same satic array in our code.
//
// $Archive: /iReporter/Dev/src_common/gd graphics/webpalette.h $
// $Revision: 1 $
// $Date: 6/07/01 5:26p $
//================================================================================================= 

#ifndef	WEBPALLETE_H
#define	WEBPALLETE_H

#include <stddef.h>	// for size_t

#ifdef __cplusplus
extern "C" {
#endif

extern const size_t WEBPALETTE_LEN;

extern unsigned char webpalette[];

#ifdef __cplusplus
}
#endif

#endif // WEBPALLETE_H