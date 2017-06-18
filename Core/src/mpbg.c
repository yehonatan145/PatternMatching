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