/**
Real-Time version of the KMP algorithm (by Galil) that is also used to find periods of patterns.
*/
#ifndef KMPRT_H
#define KMPRT_H

/******************************************************************************************************
*       INCLUDES:
******************************************************************************************************/

#include "Fingerprint.h"

/******************************************************************************************************
*		DEFINITIONS:
******************************************************************************************************/

#define KMP_LOOP_FAIL_FLAG 1 // there is still need to loop through failure function
#define KMP_HAVE_BUFFER_FLAG 2 // flag for having chars in buffer

typedef struct {
    size_t   n; // the pattern's length
    char    *pattern; // the pattern itself
    size_t  *failure_table; // the failure function table (size n + 1)
    size_t   offset; // how much characters from start of pattern until know is matched (what char are we)
    char    *buffer; // buffer for saving characters that were received during the failure-function looping
    size_t   buf_start; // the start position of the buffer (the buffer is round robin)
    size_t   buf_end; // the end position of the buffer (the buffer is round robin)
    int      flags;
} KMPRealTime;

/******************************************************************************************************
*		FUNCTIONS TO EXTERN:
******************************************************************************************************/

/* functions for using the real-time kmp algorithm */
KMPRealTime* kmp_new(char* pattern, size_t n);
int kmp_read_char(KMPRealTime* kmp, char c);
void kmp_free(KMPRealTime* kmp);
size_t kmp_get_total_mem(KMPRealTime* kmp);
void kmp_reset(KMPRealTime* kmp);

size_t kmp_get_period(char* pattern, size_t n);
size_t* kmp_create_failure_table(char* pattern, size_t n);

/******************************************************************************************************
*		INLINE FUNCTIONS:
******************************************************************************************************/

static inline size_t kmp_get_pattern_len(KMPRealTime* kmp) {
	return kmp->n;
}

/**
* Get the period of the first i characters of a pattern
*
* @param table	The failure table of the pattern
* @param i		The length of the sub-pattern which we need to get the period of
*
* @return		The period of pattern[0..(i-1)]
*/
static inline size_t kmp_get_period_from_failure_table(size_t* table, size_t i) {
	return i - table[i];
}

#endif /* KMPRT_H */
