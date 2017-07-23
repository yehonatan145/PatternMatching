
#ifndef PATTERNS_TREE_H
#define PATTERNS_TREE_H

#define _GNU_SOURCE
#include "parser.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/**************************************************************************************
*         D E F I N E S
**************************************************************************************/

#define SET_NULL_PATTERN_INTERNAL_ID(id) copy_pattern_internal_id(&(id), &null_pattern_internal_id)
#define IS_NULL_PATTERN_INTERNAL_ID(id) ((id).file_number == null_pattern_internal_id.file_number \
									 && (id).line_number == null_pattern_internal_id.line_number)


/**************************************************************************************
*         D A T A     S T R U C T U R E S
**************************************************************************************/

struct _Conf;

typedef struct {
	int file_number;
	int line_number;
} PatternInternalID;

//====================== Fpt = Full Patterns Tree ======================
struct s_FptNode;

/**
* Struct for holding a list of childs of a node
*/
typedef struct s_FptEdge {
	struct s_FptNode* node;
	struct s_FptEdge* next;
	struct s_FptEdge* prev;
	char* text;
	size_t len;
} FptEdge;

/**
* Struct for a node in the full patterns tree
*/
typedef struct s_FptNode {
	struct s_FptNode* parent;
	PatternInternalID pattern_id;
	FptEdge* edge_list;
} FptNode;

/**
* Struct for the full patterns tree
*/
typedef struct s_FullPatternsTree {
	FptNode* root;
	size_t longest_pat_len;
} FullPatternsTree;


//==========================  Patterns Tree =============================

struct s_PatternsTreeNode;

/**
* Struct for holding edge of a node in the Patterns Tree
*/
typedef struct s_PatternsTreeEdge {
	struct s_PatternsTreeEdge* next;
	struct s_PatternsTreeNode* node;
} PatternsTreeEdge;

/**
* Struct for partial patterns tree
*/
typedef struct s_PatternsTreeNode {
	struct s_PatternsTreeNode* parent;
	PatternInternalID pattern_id;
	PatternsTreeEdge* edge_list;
} PatternsTreeNode;

typedef struct {
	PatternsTreeNode* root;
} PatternsTree;

/**
* pattern_id_t should be primitive type (eg. int or pointer) so it can be copied by '='
*/
typedef PatternsTreeNode* pattern_id_t;

static pattern_id_t null_pattern_id = NULL;


/**************************************************************************************
*         F U N C T I O N S
**************************************************************************************/

FullPatternsTree* fpt_build(struct _Conf* conf);
void fpt_free(FullPatternsTree* full_tree);

PatternsTree* convert_fpt_to_patterns_tree(FullPatternsTree* full_tree, void* obj,
				void (*add_pattern_func)(void*, char*, size_t, pattern_id_t));

PatternsTree* patterns_tree_build(struct _Conf* conf, void* obj, void (*add_pattern_func)(void*, char*, size_t, pattern_id_t));
void patterns_tree_free(PatternsTree* tree);

/**
* Copy the internal id content
*/
static inline void copy_pattern_internal_id(PatternInternalID* dest, PatternInternalID* src) {
	dest->file_number = src->file_number;
	dest->line_number = src->line_number;
};

extern PatternInternalID null_pattern_internal_id; // defeinition in .c file



#endif