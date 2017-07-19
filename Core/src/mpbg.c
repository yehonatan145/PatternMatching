#include "mpbg.h"

void mps_bg_register() {
	mps_table[MPS_BG].name = "bg";
	mps_table[MPS_BG].create = mpbg_create;
	mps_table[MPS_BG].add_pattern = mpbg_add_pattern;
	mps_table[MPS_BG].compile = mpbg_compile;
	mps_table[MPS_BG].read_char = mpbg_read_char;
	mps_table[MPS_BG].free = mpbg_free;
}

// TODO implement all the functions here

void* mpbg_create(void) {

}

void mpbg_add_pattern(void* obj, char* pat, size_t len, pattern_id_t id) {

}

void mpbg_compile(void* obj) {

}

pattern_id_t mpbg_read_char(void* obj, char c) {
	return 0;
}

void mpbg_free(void* obj) {

}