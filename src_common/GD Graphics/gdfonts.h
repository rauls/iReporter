#ifndef GDFONTS_H
#define GDFONTS_H 1
#ifdef __cplusplus
extern "C" {
#endif


/* gdfonts.h: brings in the smaller of the two provided fonts.
	Also link with gdfonts.c. */

#include "gd.h"

/* 6x12 font derived from a public domain font in the X
        distribution. Only contains the 96 standard ascii characters,
        sorry. Feel free to improve on this. */

extern gdFontPtr gdFontSmall;
#ifdef __cplusplus
}
#endif

#endif
