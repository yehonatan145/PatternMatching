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

#define KMP_LOOP_FAIL_FLAG 1 // flag that say: "there is still need to loop through failure function"
#define KMP_HAVE_BUFFER_FLAG 2 // flag for having chars in buffer

typedef struct {
    int n; // the pattern's length
    char *pattern; // the pattern itself
    int *failure_table; // the failure function table (size n + 1)
    int offset; // how much characters from start of pattern until know is matched (what char are we)
    char *buffer; // buffer for saving characters that were received during the failure-function looping
    int buf_start; // the start position of the buffer (the buffer is round robin)
    int buf_end; // the end position of the buffer (the buffer is round robin)
    int flag;
} KMPRealTime;

/******************************************************************************************************
*		FUNCTIONS TO EXTERN:
******************************************************************************************************/

/* functions for using the real-time kmp algorithm */
KMPRealTime* kmp_new(char* pattern, int n);
int kmp_read_char(KMPRealTime* kmp, char c);
void kmp_free(KMPRealTime* kmp);

int kmp_get_period(char* pattern, int n);
int* kmp_create_failure_table(char* pattern, int n);
#define KMP_GET_PERIOD_FROM_FAILURE_TABLE(table, k) ((k) - (table)[(k)])

/******************************************************************************************************
*		INLINE FUNCTIONS:
******************************************************************************************************/

static inline int kmp_get_pattern_len(KMPRealTime* kmp) {
	return kmp->n;
}

/*
Get the period of the first i characters of a pattern

@param table	The failure table of the pattern
@param i		The length of the sub-pattern which we need to get the period of

@return		The period of pattern[0..(i-1)]
*/
static inline int kmp_get_period_from_failure_table(int* table, int i) {
	return i - table[i];
}

#endif /* KMPRT_H */
