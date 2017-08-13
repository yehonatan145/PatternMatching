/**
* Multi-Pattern Search algorithm of Breslauer-Galil (just duplicate the algorithm for all the patterns)
*/
#ifndef MPBG_H
#define MPBG_H


/******************************************************************************************************
*		INCLUDES
******************************************************************************************************/


#include "bgps.h"
#include "mps.h"
#include "PatternsTree.h"


/******************************************************************************************************
*		API FUNCTIONS
******************************************************************************************************/


void* mpbg_create(void);
void mpbg_add_pattern(void* obj, char* pat, size_t len, pattern_id_t id);
void mpbg_compile(void* obj);
pattern_id_t mpbg_read_char(void* obj, char c);
size_t mpbg_total_mem(void* obj);
void mpbg_reset(void* obj);
void mpbg_free(void* obj);

void mps_bg_register();

#endif /* MPBG_H */