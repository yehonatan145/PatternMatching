#ifndef PATTERNS_TREE_H
#define PATTERNS_TREE_H


/******************************************************************************
*		INCLUDES
******************************************************************************/


#define _GNU_SOURCE
#include "parser.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


/******************************************************************************
*		DEFINITIONS
******************************************************************************/


struct _Conf;

typedef struct {
	size_t file_number;
	size_t line_number;
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
	/**
	* The "text" member is the prefix of the child pattern until the parent pattern
	* e.g. if the parent pattern is "de", and the child pattern is "abcde",
	* then the "text" member in the corresponding edge is "abc".
	* 
	* In that way, by moving from the root to some node through some edges,
	* then by adding each "text" member in each edge, as a prefix of the total text,
	* we get the pattern of the node.
	* e.g. if the patterns are "abcdef", "cdef", "ef",
	* then we have: root ---parent of---> "ef" ---parent of---> "cdef" ---parent of---> "abcdef",
	* with edges:   root -----"ef"------> "ef" -----"cd"------> "cdef" -----"ab"------> "abcdef"
	* so from the root to "abcdef" we have the edges "ef","cd","ab" (which construct "abcdef").
	*/
	char             *text;
	size_t            len; // the length of text member
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


/******************************************************************************
*		API FUNCTIONS
******************************************************************************/


PatternsTree* patterns_tree_build(struct _Conf* conf,
                                  void* obj,
                                  void (*add_pattern_func)(void*, char*, size_t, pattern_id_t));

void patterns_tree_free(PatternsTree* tree);

// return whether the first pattern is a suffix of the second
int is_pattern_suffix(pattern_id_t first, pattern_id_t second);


/******************************************************************************
*		INLINE FUNCTIONS
******************************************************************************/


static inline void print_pattern_id(pattern_id_t id) {
	if (id == NULL) {
		printf("<no pattern>");
		return;
	}
	PatternsTreeNode* ptn = (PatternsTreeNode*)id;
	PatternInternalID* iid = &ptn->pattern_id;
	printf("file number: %lu, line number: %lu", iid->file_number, iid->line_number);
}


#endif