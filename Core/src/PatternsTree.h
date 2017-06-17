
#ifndef PATTERNS_TREE_H
#define PATTERNS_TREE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlin.h>

char** dictionary_files;
int n_dictionary_files;

typedef struct {
	int file_number;
	int line_number;
} PatternInternalID;

typedef struct s_PatternsTreeNode {
	struct s_PatternsTreeNode* parent;
	PatternInternalID pattern_id;
} PatternsTreeNode;

typedef struct {
	PatternTreeNode* root;
} PatternsTree;

typedef pattern_id_t PatternTreeNode*;

/**
* build the patterns tree from the dictionary_files
* (variable dictionary_files must be initialized for this)
*/
PatternsTree* build_patterns_tree();
/**
* free all memory used by the patterns tree
*/
void free_patterns_tree(PatternsTree* tree);

inline void copy_pattern_internal_id(PatternInternalID* dest, PatternInternalID* src) {
	dest->file_number = src->file_number;
	dest->line_number = src->line_number;
}

PatternInternalID null_pattern_internal_id = {-1,-1};

#define SET_NULL_PATTERN_INTERNAL_ID(id) copy_pattern_internal_id(&(id), &null_pattern_internal_id)
#define IS_NULL_PATTERN_INTERNAL_ID(id) id.file_number == null_pattern_internal_id.file_number \
									 && id.line_number == null_pattern_internal_id.line_number

#endif