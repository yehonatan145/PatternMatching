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
typedef struct s_FPTChildListNode {
	struct s_FPTNode* node;
	struct s_FPTChildListNode* next;
	struct s_FPTChildListNode* prev;
	char* text;
	size_t len;
} FPTChildListNode;

/**
* Struct for a node in the full patterns tree
*/
typedef struct s_FPTNode {
	struct s_FPTNode* parent;
	PatternInternalID id;
	FPTChildListNode* children_list;
} FPTNode;

/**
* Struct for the full patterns tree
*/
typedef struct s_PatternsFullTree {
	PatternsFullTreeNode* root;
} PatternsFullTree;

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
* Add child to the begining of the child list
*
* @param list	Pointer to the child list
* @param node	The node of the child
* @param pat	The prefix of the child from the parent
* @param n		The length of the pattern
*/
void _add_child_to_list(FPTChildListNode** list, FPTNode* node, char* pat, size_t n) {
	if (!list) {
		return;
	}
	FPTChildListNode* first = (FPTChildListNode*)malloc(sizeof(FPTChildListNode));
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
* Remove the child from the child list (including freeing the child)
*
* @param list	Pointer to the child list
* @param child	The child to remove from the list
*/
void _remove_child_from_list(FPTChildListNode** list, FPTChildListNode* child) {
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
* Create new child for a given node and add the node to its children list
*
* @param parent		The node for which we create child
* @param id			The pattern id of the child
* @param pat		The pattern from the parent to the child
* @param n			The length of the pattern
*
* @return			Pointer to the new child node created
*/
FPTNode* _create_new_child(FPTNode* parent, PatternInternalID id, char* pat, size_t n) {
	FPTNode* node = (FPTNode*)malloc(sizeof(FPTNode));
	node->parent = parent;
	copy_pattern_internal_id(node->id, id);
	node->children_list = NULL;
	_add_child_to_list(&parent->children_list, node, pat, n);
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
	FPTChildListNode *current_child, *next_child;
	for (current_child = node->children_list; current_child; current_child = current_child->next) {
		if (_is_suffix_of(current_child->text, current_child->len, pat, n)) {
			// continue recursively to add the pattern (except the suffix) to the child
			_add_pattern_to_node(current_child->node, pat, n - current_child->len, id);
			return;
		}
	}
	int created_child = 0;
	FPTNode *child_node;
	current_child = node->children_list;
	while (current_child) {
		next_child = current_child->next;
		if (_is_suffix_of(pat, n, current_child->text, current_child->len)) {
			if (!created_child) {
				child_node = _create_new_child(node, id, pat, n);
				created_child = 1;
			}
			_add_child_to_list(&child_node->children_list, current_child->node, current_child->pat, current_child->len - n);
			_remove_child_from_list(&node->children_list, current_child);
		}
		current_child = next_child;
	} else {
		_create_new_child(node, id, pat, n);
	}
}


/**
* Add pattern to the full patterns tree
* 
* @param tree	The full patterns tree
* @param pat	The pattern to add to the tree
* @param n		The length of the pattern
*/
void _add_pattern_to_full_tree(PatternsFullTree* tree, char* pat, size_t n) {
	// TODO: implement
}

/**
* function to build the full patterns tree from the dictionary files
*/
PatternsFullTree* build_full_patterns_tree() {
	PatternsFullTree* ret = (PatternsFullTree*)malloc(sizeof(PatternsFullTree));
	ret.root = (PatternsFullTreeNode)malloc(sizeof(PatternsFullTreeNode));
	memset(ret.root, 0, sizeof(PatternsFullTreeNode));

}

/**
* Build the patterns tree from the dictionary files
*
* @return	A patterns tree generated from the patterns in the dictionary files
*/
PatternsTree* build_patterns_tree() {

}