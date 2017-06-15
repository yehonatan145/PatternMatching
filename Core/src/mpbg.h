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

void* mpbg_new(void);
void mpbg_add_pattern(void* p, char* pat, int len);
void mpbg_compile(void* p);
int mpbg_read_char(void* p, char c);
void mpbg_free(void* p);

mps_register_searcher("breslauer-galil,breslauer galil,bg", mpbg_new, mpbg_add_pattern, mpbg_compile, mpbg_read_char, mpbg_free);

#endif /* MPBG_H */