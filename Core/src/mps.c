#include "mps.h"
#include "conf.h"
#include "mpbg.h"

/**
* Setup the table for the mps.
*
* Should run before parsing arguments, so we can know what algorithms we have at parsing time.
*/
void mps_table_setup() {
	mps_bg_register();
}

/**
* Initialize the mps instances in the configuration.
*
* Currently create one instance per algorithm
*
* @param conf     The configuration
*/
void init_mps_instances(Conf* conf) {
	size_t i;
	conf->mps_instances = (MpsInstance*) malloc(MPS_SIZE * sizeof(MpsInstance));
	for (i = 0; i < MPS_SIZE; ++i) {
		conf->mps_instances[i].algo = i;
		conf->mps_instances[i].obj = mps_table[i].create();
	}
}

/**
* Add the pattern received to all mps instances (callback function for adding new pattern)
*
* @param pconf      The configuration (in void pointer, since this is callback function)
* @param pat        The pattern to add
* @param len        The length of the pattern
* @param id         The id of the pattern
*/
void add_pattern_to_all_instances(void* pconf, char* pat, size_t len, pattern_id_t id) {
	Conf* conf = (Conf*)pconf;
	size_t i, n = conf->n_mps_instances;
	int algo;
	void* obj;
	for (i = 0; i < n; ++i) {
		algo = conf->mps_instances[i].algo;
		obj = conf->mps_instances[i].obj;
		mps_table[algo].add_pattern(obj, pat, len, id);
	}
}

/**
* Compile all the mps instances in the configuration file.
*
* @param conf     The configuration
*/
void compile_all_instances(Conf* conf) {
	size_t i, n = conf->n_mpst_instances;
	int algo;
	void* obj;
	for (i = 0; i < n; ++i) {
		algo = conf->mps_instances[i].algo;
		obj = conf->mps_instances[i].obj;
		mpst_table[algo].compile(obj);
	}
}

/**
* Initialize the Multi-Pattern Search (mps) in the configuration
*
* @param conf     The configuration
*/
void init_mps(Conf* conf) {
	init_mps_instances(conf);
	conf->patterns_tree = patterns_tree_build(conf, (void*)conf, add_pattern_to_all_instances);
	compile_all_instances(conf);
}

/**
* Get the enum value of the algorithm by its name
*
* @param name     The name of the algorithm
*
* @return         The enum value that match the name, or -1 if none match
*/
int get_mps_algo(char* name) {
	int i;
	for (i = 0; i < MPS_SIZE; ++i) {
		if (!strcmp(name, mps_table[i].name)) {
			return i;
		}
	}
	return -1;
}