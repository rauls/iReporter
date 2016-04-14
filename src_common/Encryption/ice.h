/*
 * Header file for the ICE encryption library.
 *
 * Written by Matthew Kwan - July 1996
 */

#ifndef _ICE_H
#define _ICE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ice_key_struct	ICE_KEY;

/*#if __STDC__
#define P_(x) x
#else
#define P_(x) ()
#endif*/

ICE_KEY		*ice_key_create(int n);
void 	cycle_key( ICE_KEY	*ik );
void		ice_key_destroy (ICE_KEY *ik);
void		ice_key_set (ICE_KEY *ik, const unsigned char *k);
unsigned char	*ice_key_encrypt (const ICE_KEY *ik, const unsigned char *ptxt, unsigned char *ctxt);
unsigned char	*ice_key_decrypt (const ICE_KEY *ik, const unsigned char *ctxt, unsigned char *ptxt);

//#undef P_

#ifdef __cplusplus
} // extern "C"
#endif


#endif
