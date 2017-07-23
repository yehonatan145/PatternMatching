#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char* program_name;

static inline void FatalExit(void) {
	exit(EXIT_FAILURE);
};

void usage();

static inline void print_usage_and_exit() {
	usage();
	exit(EXIT_FAILURE);
};

static inline void print_binary_str(char* str, size_t len) {
	size_t i;
	for (i = 0; i < len; ++i) {
		if (isprint(str[i])) {
			printf("%c", str[i]);
		} else {
			printf("\\x%x", str[i] & 0xff);
		}
	}
}

#endif