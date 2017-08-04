/**
* Multi-Pattern Search
*/

#include "mps.h"
#include "PatternsTree.h"
#include "conf.h"
// include the algorithms
#include "mpbg.h"
#include "mpac.h"


MpsElem mps_table[MPS_SIZE];


/**
* Setup the algorithms table for the mps.
*
* Should run before parsing arguments, so we can know what algorithms we have at parsing time.
*/
void mps_table_setup() {
	mps_ac_register();
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
	conf->n_mps_instances = MPS_SIZE;
	for (i = 0; i < MPS_SIZE; ++i) {
		conf->mps_instances[i].algo = i;
		conf->mps_instances[i].obj = mps_table[i].create();
	}
	conf->reliable_mps_instance.algo = MPS_AC;
	conf->reliable_mps_instance.obj = mps_table[MPS_AC].create();
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
	algo = conf->reliable_mps_instance.algo;
	obj = conf->reliable_mps_instance.obj;
	mps_table[algo].add_pattern(obj, pat, len, id);
}

/**
* Compile all the mps instances in the configuration file.
*
* @param conf     The configuration
*/
void compile_all_instances(Conf* conf) {
	size_t i, n = conf->n_mps_instances;
	int algo;
	void* obj;
	for (i = 0; i < n; ++i) {
		algo = conf->mps_instances[i].algo;
		obj = conf->mps_instances[i].obj;
		mps_table[algo].compile(obj);
	}
	algo = conf->reliable_mps_instance.algo;
	obj = conf->reliable_mps_instance.obj;
	mps_table[algo].compile(obj);
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
