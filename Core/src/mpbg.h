/**
* Multi-Pattern Search algorithm of Breslauer-Galil (just duplicate the algorithm for all the patterns)
*/
#ifndef MPBG_H
#define MPBG_H

#include "bgps.h"
#include "mps.h"
#include "PatternsTree.h"


/**
* struct used to save patterns information as list (used BEFORE mpbg compilation)
*/
typedef struct s_MPBGPatternInfoList {
	struct s_MPBGPatternInfoList* next;
	BGStruct* obj;
	pattern_id_t id;
} MPBGPatternInfoList;

/**
* struct used to save patterns information in array (used AFTER mpbg compilation)
*/
typedef struct {
	BGStruct* obj;
	pattern_id_t id;
} MPBGPatternInfo;

/**
* struct for mpbg object
*/
typedef struct {
	union {
		MPBGPatternInfoList* patsList; // before compilation
		MPBGPatternInfo* pats; // after compilation
	} u;
	size_t n_pats;
} MPBGStruct;

void* mpbg_create(void);
void mpbg_add_pattern(void* obj, char* pat, size_t len, pattern_id_t id);
void mpbg_compile(void* obj);
pattern_id_t mpbg_read_char(void* obj, char c);
void mpbg_free(void* obj);

void mps_bg_register();

#endif /* MPBG_H */