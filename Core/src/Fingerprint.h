#ifndef FINGERPRINT_H
#define FINGERPRINT_H

/******************************************************************************************************
*       INCLUDES:
******************************************************************************************************/

#include "field.h"
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************************************
*		DEFINITIONS:
******************************************************************************************************/

// ALWAYS keep fingerprint_t to be field_t, since they are used together in binary operations.
typedef field_t fingerprint_t;

/******************************************************************************************************
*		FUNCTIONS TO EXTERN:
******************************************************************************************************/

/* functions to calculate fingerprints. also calculate r^len (when got previously r^prefix_len) */
fingerprint_t calc_fp(char* seq, size_t len, FieldVal* rn, FieldVal* r, fingerprint_t p);
fingerprint_t calc_fp_with_prefix(char           *seq,
                                  size_t          len,
                                  fingerprint_t   prefix_fp,
                                  size_t          prefix_len,
                                  FieldVal       *rn,
                                  FieldVal       *r,
                                  fingerprint_t   p);


/******************************************************************************************************
*		INLINE FUNCTIONS:
******************************************************************************************************/


/**
* The next functions calculate fingerprints according to known parts of the fingerprint.
*
* Definitions:
* 'prefix' is a position somewhere in the text
* 'all' is the length of the text (text = text[0..all-1])
* 'all_fp' is the all pattern fingerprint = fp(text[0..all-1])
* 'prefix_fp' is the prefix fingerprint = fp(text[0..prefix-1])
* 'suffix_fp' is the suffix fingerprint = fp(text[prefix..all-1])
* 'r_prefix' is r ^ prefix
* 'p' is the size of the field
*
* _______________________________
* |_____________|_________________|
* 0           prefix            all-1
* ---prefix_fp--
*               ----suffix_fp------
* note: suffix include the position 'prefix' and prefix not
*
* Formulas:
* prefix_fp + suffix_fp * r_prefix = all_fp
* prefix_fp = all_fp - suffix_fp * r_prefix
* suffix_fp = (all_fp - prefix_fp) * r_prefix^-1
*/
static inline fingerprint_t calc_fp_suffix(fingerprint_t  all_fp,
                                           fingerprint_t  prefix_fp,
                                           FieldVal      *r_prefix,
                                           field_t        p) {
	return ((all_fp > prefix_fp ? all_fp - prefix_fp : p - prefix_fp + all_fp) * r_prefix->inv) % p;
}

static inline fingerprint_t calc_fp_prefix(fingerprint_t  all_fp,
                                           fingerprint_t  suffix_fp,
                                           FieldVal      *r_prefix,
                                           field_t        p) {
	fingerprint_t suffix_part = (suffix_fp * r_prefix->val) % p;
	return (all_fp > suffix_part ? all_fp - suffix_part : p - suffix_part + all_fp) % p;
}

static inline fingerprint_t calc_fp_from_prefix_suffix(fingerprint_t  prefix_fp,
                                                       fingerprint_t  suffix_fp,
                                                       FieldVal      *r_prefix,
                                                       field_t        p) {
	// we do % p twice, to not get overflow:
	// All we are promised, is that p < sqrt(sizeof(field_t)), so suffix * r_prefix is in the range
	// But, suffix * r_prefix + prefix might give overflow
	return (prefix_fp + ((suffix_fp * r_prefix->val) % p)) % p;
}


#endif /* FINGERPRINT_H */
