#ifndef GDFONTT_H
#define GDFONTT_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* gdfontt.h: brings in the tinyest of the provided fonts.
	Also link with gdfontt.c. */

#include "gd.h"

/* 5x8  font derived from a public domain font in the X
        distribution. Contains the 127 standard ascii characters. */

extern gdFontPtr gdFontTiny;
#ifdef __cplusplus
}
#endif
#endif
