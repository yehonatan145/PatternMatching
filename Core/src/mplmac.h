/**
* Multi-Pattern Low-Memory Aho-Corasick algorithm
*/
#ifndef MPLMAC_H
#define MPLMAC_H


/******************************************************************************
*		INCLUDES
******************************************************************************/


#include "mps.h"
#include "PatternsTree.h"


/******************************************************************************
*		API FUNCTIONS
******************************************************************************/


void* lmac_create();
void lmac_add_pattern(void* obj, char* pat, size_t len, pattern_id_t id);
void lmac_compile(void* obj);
pattern_id_t lmac_read_char(void* obj, char c);
size_t lmac_total_mem(void* obj);
void lmac_reset(void* obj);
void lmac_free(void *obj);

void mps_lmac_register();

#endif // MPLMAC_H