/**
* Multi-Pattern Aho-Corasick algorithm
*/
#ifndef MPAC_H
#define MPAC_H

#include "mps.h"
#include "PatternsTree.h"


void* ac_create();
void ac_add_pattern(void* obj, char* pat, size_t len, pattern_id_t id);
void ac_compile(void* obj);
pattern_id_t ac_read_char(void* obj, char c);
size_t ac_total_mem(void* obj);
void ac_free(void *obj);

void mps_ac_register();

#endif // MPAC_H