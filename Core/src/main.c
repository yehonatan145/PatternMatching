#include <stdio.h>
#include "util.h"
#include "conf.h"

int main(int argc, char **argv) {
	program_name = argv[0];
	Conf* conf = (Conf*) malloc(sizeof(Conf));
	if (conf == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	memset(conf, 0, sizeof(Conf));
	mps_table_setup();
	parse_arguments(argc, argv);
	init_mps(conf);
}