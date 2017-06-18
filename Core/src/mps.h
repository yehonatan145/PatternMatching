#ifndef MPS_H
#define MPS_H

// Multi-Pattern Search Algorithms
enum {
	MPS_BG = 0, // Multi-Pattern Brausler-Galil
	MPS_SIZE
};

typedef struct {
	char* name;
	void* (*create)(void);
	void (*add_pattern)(void*, char*, size_t, pattern_id_t);
	void (*compile)(void*);
	pattern_id_t (*read_char)(void*, char);
	void (*free)(void*);
} MpsElem;

MpsElem mps_table[MPS_SIZE];
int default_mps_algo = MPS_BG;

void mps_table_setup();
void init_mps(struct _Conf* conf);
// get the enum value of the algorithm by its name
int get_mps_algo(char* name);

#endif /* MPS_H */