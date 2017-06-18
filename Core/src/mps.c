#include "mps.h"

/**
* Add a new child to a node
*
* @param parent		The parent node
* @param child		The node to add the parent as child
*/
void add_child_to_node(PatternsTreeNode* parent, PatternsTreeNode* child) {
	PatternsTreeEdge* edge = (PatternsTreeEdge*) malloc(sizeof(PatternsTreeEdge));
	if (edge == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	edge->node = child;
	edge->next = parent->edge_list;
	parent->edge_list = edge;
}

/**
* Convert an FPT node to a (regular) Patterns Tree node recursively
*
* @param node				The node to convert
* @param buffer				A buffer (long enough for the longest pattern) to work on
* @param buffer_len			The length of the buffer
* @param pat_pos			The position in the buffer where the pattern of the node starts
* @param mps_object			The mps object to add the patterns to
* @param add_pattern_func	The function to add pattern
*
* @return			A dynamically allocated patterns tree node constructed from the given node
*/
PatternsTreeNode* convert_fpt_node_to_patterns_tree_node(FPTNode* node, char* buffer, size_t buffer_len, size_t pat_pos,
			void* mps_object, mps_add_pattern_func add_pattern_func) {
	PatternsTreeNode* ret = (PattersTreeNode*) malloc(sizeof(PattersTreeNode));
	if (ret == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	PatternsTreeNode* temp;
	FPTEdge* current_edge;

	memset(ret, 0, sizeof(PattersTreeNode));
	copy_pattern_internal_id(&ret->id, &node->id);
	for (current_edge = node->edge_list; current_edge; current_edge = current_edge->next) {
		size_t len = current_edge->node->len;
		char* text = current_edge->node->text;
		strncpy(buffer + pat_pos - len, text, len);
		temp = convert_fpt_node_to_patterns_tree_node(current_edge->node, buffer, buffer_len, pat_pos - len,
				mps_object, add_pattern_func);
		temp->parent = ret;
		add_child_to_node(ret, temp);
	}
	add_pattern_func(mps_object, buffer + pat_pos, buffer_len - pat_pos, ret);
	return ret;
}

/**
* Convert Full-Patterns-Tree to (regular) Patterns Tree
*
* @param full_tree	The Full Patterns Tree
*
* @return			A dynamically allocated Patterns Tree constructed by the given Full Patterns Tree
*/
PatternsTree* convert_fpt_to_patterns_tree(Conf* conf, FullPatternsTree* full_tree,
			void* mps_object, mps_add_pattern_func add_pattern_func) {
	PatternsTree* tree = (PatternsTree*) malloc(sizeof(PatternsTree));
	if (tree == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	size_t len = conf->max_pat_len;
	char* buffer = (char*)malloc(len);
	tree->root = convert_fpt_node_to_patterns_tree_node(full_tree->root, buffer, len, len, mps_object, add_pattern_func);
	free(buffer);
	return tree;
}

/**
* Initialize the multi-pattern search engine
*
* @param conf		The configuration for the program
*/
void init_mps(Conf* conf) {
	FullPatternsTree* full_tree = fpt_build(conf);
	// TODO choose mps algorithm from conf and convert the full tree to regular one
}