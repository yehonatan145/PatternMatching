
#ifndef PATTERNS_TREE_H
#define PATTERNS_TREE_H

#define _GNU_SOURCE
#include "conf.h"
#include "parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlin.h>
#include <errno.h>

/**************************************************************************************
*         D E F I N E S
**************************************************************************************/

#define SET_NULL_PATTERN_INTERNAL_ID(id) copy_pattern_internal_id(&(id), &null_pattern_internal_id)
#define IS_NULL_PATTERN_INTERNAL_ID(id) id.file_number == null_pattern_internal_id.file_number \
									 && id.line_number == null_pattern_internal_id.line_number


/**************************************************************************************
*         D A T A     S T R U C T U R E S
**************************************************************************************/

typedef struct {
	int file_number;
	int line_number;
} PatternInternalID;

//====================== FPT = Full Patterns Tree ======================
struct s_FPTNode;

/**
* Struct for holding a list of childs of a node
*/
typedef struct s_FPTEdge {
	struct s_FPTNode* node;
	struct s_FPTEdge* next;
	struct s_FPTEdge* prev;
	char* text;
	size_t len;
} FPTEdge;

/**
* Struct for a node in the full patterns tree
*/
typedef struct s_FPTNode {
	struct s_FPTNode* parent;
	PatternInternalID id;
	FPTEdge* edge_list;
} FPTNode;

/**
* Struct for the full patterns tree
*/
typedef struct s_FullPatternsTree {
	FPTNode* root;
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
	PatternTreeNode* root;
} PatternsTree;

typedef pattern_id_t PatternTreeNode*;


/**************************************************************************************
*         F U N C T I O N S
**************************************************************************************/

/**
* build the patterns tree from the dictionary files in conf
*/
PatternsTree* build_patterns_tree(Conf* conf);

FullPatternsTree* fpt_build(Conf* conf);
void fpt_free(FullPatternsTree* full_tree);

/**
* free all memory used by the patterns tree
*/
void free_patterns_tree(PatternsTree* tree);

/**
* Copy the internal id content
*/
inline void copy_pattern_internal_id(PatternInternalID* dest, PatternInternalID* src) {
	dest->file_number = src->file_number;
	dest->line_number = src->line_number;
}

PatternInternalID null_pattern_internal_id = {-1,-1};



#endif