#ifndef UTILS_H
#define UTILS_H

#include <sdtio.h>
#include <stdlib.h>

int total_mem_used = 0;

inline void* PM_malloc(size_t size) {
	total_mem_used += size;
	return malloc(size);
}

#endif