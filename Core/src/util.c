#include "util.h"

void usage() {
	fprintf(stderr, "Usage: %s [OPTION]...\n", program_name);
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  -d FILE               use FILE as one of the dictionary files (can be used many times).\n");
	fprintf(stderr, "  -s FILE               use FILE as one of the stream files (can be used many times).\n");
	fprintf(stderr, "  -o FILE               set FILE to be the output file.\n");
}