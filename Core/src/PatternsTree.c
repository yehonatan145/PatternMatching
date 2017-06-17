#include "PatternsTree"

/**
* We build the patterns tree in two stages:
* 1. Build a full pattern tree (where every node have a list of its childs, with the suitable suffix)
* 2. Convert the full tree to regular tree (where every node have only his parent)
*/

// FPT = full patterns tree
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

/**
* Check whether one string is (real) suffix of another
*
* @param suf		The string that should be a suffix of the other string
* @param suf_len	The length of that suffix
* @param str		The string that should contain the other as a suffix
* @param str_len	The length of that string
*
* @return		True if the first string is suffix of the second one, false otherwise
*/
int _is_suffix_of(char* suf, size_t suf_len, char* str, size_t str_len) {
	if (str_len <= suf_len) {
		reutrn 0;
	}
	return !strncmp(str + (str_len - suf_len), suf, suf_len);
}

/**
* Add child to the begining of the edge list
*
* @param list	Pointer to the edge list
* @param node	The node of the child
* @param pat	The prefix of the child from the parent
* @param n		The length of the pattern
*/
void _add_child_to_list(FPTEdge** list, FPTNode* node, char* pat, size_t n) {
	if (!list) {
		return;
	}
	FPTEdge* first = (FPTEdge*)malloc(sizeof(FPTEdge));
	first->node = node;
	first->len = n;
	first->text = (char*)malloc(n);
	strncpy(fisrt->text, pat, n);
	first->prev = NULL;
	first->next = *list;
	if (*list) (*list)->prev = first;
	*list = first;
}

/**
* Remove edge from the edge list (including freeing the edge)
*
* @param list	Pointer to the edge list
* @param child	The edge to remove from the list
*/
void _remove_child_from_list(FPTEdge** list, FPTEdge* child) {
	if (child->next) {
		child->next->prev = child->prev;
	}
	if (child->prev) {
		child->prev->next = child->next;
	} else {
		*list = child->next;
	}
	if (child->text) free(child->text);
	free(child);
}

/**
* Create new child for a given node
*
* @param parent		The node for which we create child
* @param pat		The pattern from the parent to the child
* @param n			The length of the pattern
* @param id			The pattern id of the child
*
* @return			Pointer to the new child node created
*/
FPTNode* _create_new_child(FPTNode* parent, char* pat, size_t n, PatternInternalID id) {
	FPTNode* node = (FPTNode*)malloc(sizeof(FPTNode));
	node->parent = parent;
	copy_pattern_internal_id(&node->id, &id);
	node->edge_list = NULL;
	_add_child_to_list(&parent->edge_list, node, pat, n);
	return node;
}

/**
* Add pattern to a node in the full patterns tree
*
* To add the pattern to the node, the next should be performed:
* 1. check whether there is a child of the node who is a suffix of the current pattern,
*    if there is such child, add the pattern except that suffix to him recursively.
* 2. otherwise, if the pattern is suffix of some childs, then the pattern should seperate the edges
*    between the node to those childs (add the pattern as a mid-node between the current node to the suitable  childs)
* 3. otherwise, just make the pattern a new child
*
* @param node	The node to add the pattern to
* @param pat	The pattern to add to this node
* @param n		The length of the pattern
* @param id		The pattern id of the pattern
*/
void _add_pattern_to_node(FPTNode* node, char* pat, size_t n, PatternInternalID id) {
	FPTEdge *current_edge, *next_edge;
	FPTNode *new_node = NULL;
	current_edge = node->edge_list;
	while (current_edge) {
		next_edge = current_edge->next;
		if (current_edge->len == n && !strncpy(current_edge->text, pat, n)) {
			// pattern is already in the tree
			return;
		}
		if (_is_suffix_of(current_edge->text, current_edge->len, pat, n)) {
			_add_pattern_to_node(current_edge->node, pat, n - current_edge->len, id);
			return;
		}
		if (_is_suffix_of(pat, n, current_edge->text, current_edge->len)) {
			if (!new_node) {
				new_node = _create_new_child(node, pat, n, id);
			}
			_add_child_to_list(&new_node->edge_list, current_edge->node, current_edge->text, current_edge->len - n);
			_remove_child_from_list(&node->edge_list, current_edge);
		}
		current_edge = next_edge;
	}
	if (!new_node) {
		// the pattern is not a suffix of a child nor conatin a child as suffix, just create new node
		_create_new_child(node, pat, n, id);
	}
}


/**
* Add pattern to the full patterns tree
* 
* @param tree	The full patterns tree
* @param pat	The pattern to add to the tree
* @param n		The length of the pattern
* @param id		The id of the pattern
*/
void _add_pattern_to_full_tree(FullPatternsTree* tree, char* pat, size_t n, PatternInternalID id) {
	_add_pattern_to_node(tree->root, pat, n, id);
}


/**
* Create a new empty Full Patterns Tree
*
* @return		A new dynamically allocated empty Full Patterns Tree
*/
FullPatternsTree* _new_fpt() {
	FullPatternsTree* ret = (FullPatternsTree*) malloc(sizeof(FullPatternsTree));
	ret->root = (FPTNode*)malloc(sizeof(FPTNode));
	memset(&ret->root, 0, sizeof(FPTNode));
	SET_NULL_PATTERN_INTERNAL_ID(ret->root->id);
	return ret;
}

/**
* Fill an existing FPT with all the patterns in a dictionary file
*
* @param tree			The Full Patterns Tree
* @param file_index		The index of the dictionary file name in dictionary_files
*/
void _fill_fpt_with_dict_file(FullPatternsTree* tree, int file_index) {
	FILE* fp;
	char* line = NULL;
	char* pat = NULL;
	size_t len, pat_len;
	int line_num = 0;
	ssize_t read;
	PatternInternalID id;

	fp = fopen(dictionary_files[file_index], "r");
	if (fp == NULL) {
		return;
	}
	while ((read = getline(&line, &len, fp)) != -1) {
		if (line[read - 1] == '\n') --read;
		pat_len = parse_pattern_from_line(line, read, &pat);
		if (pat_len != 0) {
			id.file_number = file_index;
			id.line_number = line_num;
			_add_pattern_to_full_tree(tree, pat, pat_len, id);
		}
		++line_num;
	}
}

/**
* Build the full patterns tree from the dictionary files
*
* @return		A dynamically allocated Full-Patterns-Tree filled with the patterns from the dictionary files
*/
FullPatternsTree* build_full_patterns_tree() {
	FullPatternsTree* ret = _new_fpt();
	int i;
	for (i = 0; i < n_dictionary_files; ++i) {
		_fill_fpt_with_dict_file(ret, i);
	}
	return ret;
}

/**
* Build the patterns tree from the dictionary files
*
* @return	A patterns tree generated from the patterns in the dictionary files
*/
PatternsTree* build_patterns_tree() {
	// TODO implement
}