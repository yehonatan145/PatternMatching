
#ifndef PATTERNS_TREE_H
#define PATTERNS_TREE_H

#define _GNU_SOURCE
#include "parser.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


/**************************************************************************************
*         D A T A     S T R U C T U R E S
**************************************************************************************/

struct _Conf;

typedef struct {
	int file_number;
	int line_number;
} PatternInternalID;

//====================== Fpt = Full Patterns Tree ======================
struct fpt_node;

/**
* Struct for holding a list of childs of a node
*/
typedef struct fpt_edge {
	struct fpt_node  *node;
	struct fpt_edge  *next;
	struct fpt_edge  *prev;
	char             *text;
	size_t            len;
} FptEdge;

/**
* Struct for a node in the full patterns tree
*/
typedef struct fpt_node {
	struct fpt_node   *parent;
	PatternInternalID  pattern_id;
	FptEdge*           edge_list;
} FptNode;

/**
* Struct for the full patterns tree
*/
typedef struct full_patterns_tree {
	FptNode* root;
	size_t longest_pat_len;
} FullPatternsTree;


//==========================  Patterns Tree =============================

struct patterns_tree_node;

/**
* Struct for holding edge of a node in the Patterns Tree
*/
typedef struct patterns_tree_edge {
	struct patterns_tree_edge* next;
	struct patterns_tree_node* node;
} PatternsTreeEdge;

/**
* Struct for partial patterns tree
*/
typedef struct patterns_tree_node {
	struct patterns_tree_node  *parent;
	PatternInternalID           pattern_id;
	PatternsTreeEdge           *edge_list;
} PatternsTreeNode;

typedef struct {
	PatternsTreeNode* root;
} PatternsTree;

/**
* pattern_id_t should be primitive type (eg. int or pointer)
* so it can be copied by '=' and compared by '=='
*/
typedef PatternsTreeNode* pattern_id_t;

#define null_pattern_id NULL


/**************************************************************************************
*         F U N C T I O N S
**************************************************************************************/

FullPatternsTree* fpt_build(struct _Conf* conf);
void fpt_free(FullPatternsTree* full_tree);

PatternsTree* convert_fpt_to_patterns_tree(FullPatternsTree *full_tree,
                                           void *obj,
                                           void (*add_pattern_func)(void*, char*, size_t, pattern_id_t));

PatternsTree* patterns_tree_build(struct _Conf* conf,
                                  void* obj,
                                  void (*add_pattern_func)(void*, char*, size_t, pattern_id_t));

void patterns_tree_free(PatternsTree* tree);

// return whether the first pattern is a suffix of the second
int is_pattern_suffix(pattern_id_t first, pattern_id_t second);





#endif