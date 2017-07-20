#ifndef MPS_H
#define MPS_H

#include "PatternsTree.h"

// Multi-Pattern Search Algorithms
enum {
	MPS_BG = 0, // Multi-Pattern Brausler-Galil
	MPS_SIZE
};

typedef struct {
	char* name;
	void* (*create)(void);
	void (*add_pattern)(void*, char*, size_t, pattern_id_t);
	void (*compile)(void*);
	pattern_id_t (*read_char)(void*, char);
	void (*free)(void*);
} MpsElem;

typedef struct {
	void* obj;
	int algo;
} MpsInstance;

extern MpsElem mps_table[MPS_SIZE]; // definition in .c file

void mps_table_setup();
void init_mps(struct _Conf* conf);

#endif /* MPS_H */