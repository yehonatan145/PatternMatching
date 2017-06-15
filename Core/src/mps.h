#ifndef MPS_H
#define MPS_H

/**
* API for Multi-Pattern searching algorithms
*/

typedef (void*)(mps_new_func*)(void);
typedef (void)(mps_add_pattern_func*)(void*, char*, int);
typedef (void)(mps_compile_func*)(void*);
typedef (int)(mps_read_char_func*)(void*, char);
typedef (void)(mps_free_func*)(void*);

typedef struct {
	mps_new_func			mps_new;
	mps_add_pattern_func	mps_add_pattern;
	mps_compile_func		mps_compile;
	mps_read_char_func		mps_read_char;
	mps_free_func			mps_free;
	char*		names;
} MPSearcherAPI;

typedef struct s_MPSearcherNode {
	MPSearcherAPI mps;
	struct s_MPSearcherNode* next;
} MPSearcherNode;

typedef struct {
	MPSearcherNode* first;
} MPSearcherList;

MPSearcherList mp_searchers;

/**
* Add new Multi-Pattern search Algorithm.
* 
* @param names				The names of the Multi-Pattern searcher, the format is "name1,name2,name3,...",
*							where the names is case-insensitive
* @param mps_new			Function to create new searcher
* @param mps_add_pattern	Function to add new pattern to the searcher
* @param mps_compile		Function to compile the searcher (i.e. ready for search, no more pattern can be inserted)
* @param mps_read_char		Function to search in the stream (i.e. the function get the next stream
*								character and return whether we had a match in that posittion)
*/
void mps_register_searcher(char* names, 
			mps_new_func mps_new,
			mps_add_pattern mps_add_pattern,
			mps_compile_func mps_compile,
			mps_read_car_func mps_read_char,
			mps_free_func mps_free) {
	MPSearcherNode* mps = (MPSearcherNode*) malloc (sizeof(MPSearcherNode));
	mps->mps.mps_new = mps_new;
	mps->mps.mps_add_pattern = mps_add_pattern;
	mps->mps.mps_compile = mps_compile;
	mps->mps.mps_read_char = mps_read_char;
	mps->mps.mps_free = mps_free;
	mps->mps.names = names;
	mps->next = mp_searchers.first;
	mp_searchers.first = mps;
}

MPSearcherAPI mps_get_searcher(char* name);

#endif /* MPS_H */