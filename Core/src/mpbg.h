/**
* Multi-Pattern Search algorithm of Breslauer-Galil (just duplicate the algorithm for all the patterns)
*/
#ifndef MPBG_H
#define MPBG_H

#include "bgps.h"
#include "mps.h"

typedef struct BGNode_s {
	int n;
	char* pattern;
	struct BGNode_s* next;
} BGNode;

typedef struct {
	int num_of_patterns;
	BGStruct* bgs; // the array of breslauer-galil stucts for all the patterns
	BGNode* first_pat; // used only for the pattern gathering, not used after the struct was compiled.
} MPBGStruct;

void* mpbg_create(void);
void mpbg_add_pattern(void* obj, char* pat, size_t len, pattern_id_t id);
void mpbg_compile(void* obj);
pattern_id_t mpbg_read_char(void* obj, char c);
void mpbg_free(void* obj);

void mps_bg_register();

#endif /* MPBG_H */