#include "mps.h"
#include "conf.h"
#include "mpbg.h"

/**
* Setup the table for the mps (should run before parsing arguments, so we can know what algorithms we have)
*/
void mps_table_setup() {
	mps_bg_register();
}

/**
* Initialize the Multi-Pattern Search (mps) in the configuration
*
* @param conf		The configuration where we need to initialize the mps
*/
void init_mps(Conf* conf) {
	MpsElem* mps = &mps_table[conf->mps_algo];
	conf->mps_obj = mps->create();
	conf->patterns_tree = patterns_tree_build(conf, conf->mps_obj, mps->add_pattern);
	mps->compile(conf->mps_obj);
}

/**
* Get the enum value of the algorithm by its name
*
* @param name		The name of the algorithm
*
* @return			The enum value that match the name, or -1 if none match
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