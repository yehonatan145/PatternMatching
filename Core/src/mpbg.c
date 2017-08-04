#include "mpbg.h"

/**
* struct used to save patterns information as list (used BEFORE mpbg compilation)
*/
typedef struct mpbg_pattern_info_list {
	struct mpbg_pattern_info_list *next;
	BGStruct                      *obj;
	pattern_id_t                   id;
} MPBGPatternInfoList;

/**
* struct used to save patterns information in array (used AFTER mpbg compilation)
*/
typedef struct {
	BGStruct     *obj;
	pattern_id_t  id;
} MPBGPatternInfo;

/**
* struct for mpbg object
*/
typedef struct {
	union {
		MPBGPatternInfoList  *patsList; // before compilation
		MPBGPatternInfo      *pats;     // after compilation
	} u;
	size_t n_pats;
} MPBGStruct;


/**
* Create the mpbg object
*/
void* mpbg_create(void) {
	MPBGStruct* ret = (MPBGStruct*) malloc(sizeof(MPBGStruct));
	memset(ret, 0, sizeof(MPBGStruct));
	return (void*)ret;
}

/**
* Add pattern to the mpbg struct
*
* This function already compile the pattern (since there is no dependency between patterns in this implementation),
* and add it to the pattern information list
*
* @param obj      The mpbg object to work on
* @param pat      The pattern to add
* @param len      The length of the pattern
* @param id       The id of the pattern
*/
void mpbg_add_pattern(void* obj, char* pat, size_t len, pattern_id_t id) {
	MPBGStruct* mpbg = (MPBGStruct*)obj;
	MPBGPatternInfoList* patInf = (MPBGPatternInfoList*) malloc(sizeof(MPBGPatternInfoList));
	patInf->obj = bg_new(pat, len, 2147483647);
	patInf->id = id;
	patInf->next = mpbg->u.patsList;
	mpbg->u.patsList = patInf;
	mpbg->n_pats++;
}

/**
* Compile the mpbg struct
*
* Convert the pattern info list to array, and free the list
*
* @param obj      The mpbg object
*/
void mpbg_compile(void* obj) {
	MPBGStruct* mpbg = (MPBGStruct*)obj;
	size_t i;
	MPBGPatternInfo* arr = (MPBGPatternInfo*) malloc(mpbg->n_pats * sizeof(MPBGPatternInfo));
	MPBGPatternInfoList *cur, *next;

	// transfering the list to array
	cur = mpbg->u.patsList;
	i = 0;
	while (cur) {
		arr[i].obj = cur->obj;
		arr[i].id = cur->id;
		++i;
		cur = cur->next;
	}
	// free the list
	cur = mpbg->u.patsList;
	while (cur) {
		next = cur->next;
		free(cur);
		cur = next;
	}
	mpbg->u.pats = arr;
}

/**
* The mpbg reading character fucntion
*
* This function just use bg on every pattern independently and return the id of the longest that match
*
* @param obj     The mpbg object
* @param c       The char arrived from the stream
*
* @return        The id of the longest pattern matched
*/
pattern_id_t mpbg_read_char(void* obj, char c) {
	MPBGStruct* mpbg = (MPBGStruct*)obj;
	MPBGPatternInfo* iter;
	size_t i, length, longest = 0;
	pattern_id_t longest_id = null_pattern_id;
	for (i = mpbg->n_pats, iter = mpbg->u.pats; i; --i, ++iter) {
		if (bg_read_char(iter->obj, c) &&
		    (length = bg_get_length(iter->obj)) > longest) {
			longest = length;
			longest_id = iter->id;
		}
	}
	return longest_id;
}

/**
* The mpbg total memory function
*
* @param obj    The mpbg object
*
* @return       The total memory allocated for this object
*/
size_t mpbg_total_mem(void* obj) {
	if (obj == NULL) return 0;
	MPBGStruct* mpbg = (MPBGStruct*)obj;
	size_t total_mem = sizeof(MPBGStruct), i, n_pats = mpbg->n_pats;
	total_mem += n_pats * sizeof(MPBGPatternInfo);
	MPBGPatternInfo* cur = mpbg->u.pats;
	for (i = n_pats; i; --i, ++cur) {
		total_mem += bg_get_total_mem(cur->obj);
	}
	return total_mem;
}

/**
* The mpbg reset function (returning to initial state)
*
* @param obj    The mpbg object
*/
void mpbg_reset(void* obj) {
	MPBGStruct* mpbg = (MPBGStruct*)obj;
	size_t i;
	MPBGPatternInfo* cur;
	for (i = mpbg->n_pats, cur = mpbg->u.pats; i; --i, ++cur) {
		bg_reset(cur->obj);
	}
}

/**
* Free the mpbg object (should be called AFTER compilation using mpbg_compile)
*
* @param obj    The mpbg object to free
*/
void mpbg_free(void* obj) {
	MPBGStruct* mpbg = (MPBGStruct*)obj;
	size_t i, n_pats = mpbg->n_pats;

	for (i = 0; i < n_pats; ++i) {
		bg_free(mpbg->u.pats[i].obj);
	}
	free(mpbg->u.pats);
	free(mpbg);
}

/**
* The mps registering function of Multi-Pattern Breslauer-Galil algorithm
*/
void mps_bg_register() {
	mps_table[MPS_BG].name = "Multi-Pattern Breslauer-Galil";
	mps_table[MPS_BG].create = mpbg_create;
	mps_table[MPS_BG].add_pattern = mpbg_add_pattern;
	mps_table[MPS_BG].compile = mpbg_compile;
	mps_table[MPS_BG].read_char = mpbg_read_char;
	mps_table[MPS_BG].total_mem = mpbg_total_mem;
	mps_table[MPS_BG].reset = mpbg_reset;
	mps_table[MPS_BG].free = mpbg_free;
}