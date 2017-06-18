#ifndef UTILS_H
#define UTILS_H

#include <sdtio.h>
#include <stdlib.h>

int total_mem_used = 0;
char* program_name;

inline void* PM_malloc(size_t size) {
	total_mem_used += size;
	return malloc(size);
}

inline void FatalExit(void) {
	exit(EXIT_FAILURE);
}

void usage();

void print_usage_and_exit() {
	usage();
	exit(EXIT_FAILURE);
}

#endif