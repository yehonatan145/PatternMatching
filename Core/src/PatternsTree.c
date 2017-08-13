/**
* Implementation of Patterns Tree.
*
* We use Patterns Tree in order to be able to find all matching patterns 
* in a position based on the longest pattern that had a match there.
*
* The Patterns Tree works as follows:
*     We construct a tree where every node is a pattern, and node x is child of node y
*     only if the pattern of y is suffix of the pattern of x.
*
*     In every node, we are not keeping the pattern itself, but rather an internal id
*     of the pattern which should identify the pattern. (currently the internal id is the
*     file number and line number of the pattern in the dictionary files)
*
*     In that way, if we know the node who have the longest pattern matching at some position,
*     we can simply go up in the tree until the root and get all the patterns who are suffixes
*     of the given node.
*
*     Note that if we have the longest match, than all the other matches are exactly its suffixes.
*
* We define a pointer to a patterns tree node as pattern_id_t, and that the id of the pattern that
* the Multi-Pattern searching alogirithms get.
*
* When a Multi-Pattern Searching Algorithm return the id of the longest pattern matching, we can find
* all other patterns (in the way stated above).
*
* Note that since we are not keeping the actual patterns in the tree, after the tree construction there
* is no way to find the pattern from its node (actually, we do know the file and line numbers of the pattern
* but its take time to read a specific line in a file, and doing it on all patterns can take a LOT of time).
* So, in order to give the pattern searching algorithms all the patterns with their right IDs, we give
* it to them IN CONSTRUCTION TIME (the patterns tree construction function should be given a callback function,
* to add the patterns with it).
*/


/******************************************************************************
*		INCLUDES
******************************************************************************/


#include "PatternsTree.h"
#include "conf.h"


/******************************************************************************
*		DEFINITIONS
******************************************************************************/


// ============ FOR TESTING ===================
void print_fpt(FullPatternsTree* full_tree);
void print_patterns_tree(PatternsTree* tree);
// ============ FOR TESTING ===================

/**
* We build the patterns tree in two stages:
* 1. Build a full pattern tree (where every node have a list of its childs, with the suitable prefix)
* 2. Convert the full tree to regular tree (where every node have only his parent)
*/

/**
* Copy the internal id content
*/
static inline void copy_pattern_internal_id(PatternInternalID* dest, PatternInternalID* src) {
	dest->file_number = src->file_number;
	dest->line_number = src->line_number;
};

PatternInternalID null_pattern_internal_id = {-1,-1};

#define SET_NULL_PATTERN_INTERNAL_ID(id) copy_pattern_internal_id(&(id), &null_pattern_internal_id)
#define IS_NULL_PATTERN_INTERNAL_ID(id) ((id).file_number == null_pattern_internal_id.file_number \
                                         && (id).line_number == null_pattern_internal_id.line_number)


/******************************************************************************
*		INNER FUNCTIONS
******************************************************************************/


/**
* Check whether one string is (real) suffix of another
*
* @param suf        The string that should be a suffix of the other string
* @param suf_len    The length of that suffix
* @param str        The string that should contain the other as a suffix
* @param str_len    The length of that string
*
* @return           1 if the first string is suffix of the second one, 0 otherwise
*/
static int _is_suffix_of(char* suf, size_t suf_len, char* str, size_t str_len) {
	if (str_len <= suf_len) {
		return 0;
	}
	return !memcmp(str + (str_len - suf_len), suf, suf_len);
}

/**
* Add child to the begining of the edge list
*
* @param list   Pointer to the edge list
* @param node   The node of the child
* @param pat    The prefix of the child from the parent
* @param n      The length of the pattern
*/
static void fpt_add_child_to_list(FptEdge** list, FptNode* node, char* pat, size_t n) {
	if (!list) {
		return;
	}
	FptEdge* first = (FptEdge*)malloc(sizeof(FptEdge));
	if (first == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	first->node = node;
	first->len = n;
	first->text = (char*)malloc(n);
	if (first->text == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	memcpy(first->text, pat, n);
	first->prev = NULL;
	first->next = *list;
	if (*list) (*list)->prev = first;
	*list = first;
}

/**
* Remove edge from the edge list (including freeing the edge)
*
* @param list     Pointer to the edge list
* @param child    The edge to remove from the list
*/
static void fpt_remove_child_from_list(FptEdge** list, FptEdge* child) {
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
* @param parent     The node for which we create child
* @param pat        The pattern from the parent to the child
* @param n          The length of the pattern
* @param id         The pattern id of the child
*
* @return           Pointer to the new child node created
*/
static FptNode* fpt_create_new_child(FptNode* parent, char* pat, size_t n, PatternInternalID id) {
	FptNode* node = (FptNode*)malloc(sizeof(FptNode));
	if (node == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	node->parent = parent;
	copy_pattern_internal_id(&node->pattern_id, &id);
	node->edge_list = NULL;
	fpt_add_child_to_list(&parent->edge_list, node, pat, n);
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
* @param node    The node to add the pattern to
* @param pat     The pattern to add to this node
* @param n       The length of the pattern
* @param id      The pattern id of the pattern
*/
static void fpt_add_pattern_to_node(FptNode* node, char* pat, size_t n, PatternInternalID id) {
	FptEdge *current_edge, *next_edge;
	FptNode *new_node = NULL;

	current_edge = node->edge_list;
	while (current_edge) {
		next_edge = current_edge->next;
		if (current_edge->len == n && !memcmp(current_edge->text, pat, n)) {
			// pattern is already in the tree
			return;
		}
		if (_is_suffix_of(current_edge->text, current_edge->len, pat, n)) {
			fpt_add_pattern_to_node(current_edge->node, pat, n - current_edge->len, id);
			return;
		}
		if (_is_suffix_of(pat, n, current_edge->text, current_edge->len)) {
			if (!new_node) {
				new_node = fpt_create_new_child(node, pat, n, id);
			}
			fpt_add_child_to_list(&new_node->edge_list, current_edge->node, current_edge->text, current_edge->len - n);
			fpt_remove_child_from_list(&node->edge_list, current_edge);
		}
		current_edge = next_edge;
	}
	if (!new_node) {
		// the pattern is not a suffix of a child nor conatin a child as suffix, just create new node
		fpt_create_new_child(node, pat, n, id);
	}
}


/**
* Add pattern to the full patterns tree
* 
* @param tree   The full patterns tree
* @param pat    The pattern to add to the tree
* @param n      The length of the pattern
* @param id     The id of the pattern
*/
static void fpt_add_pattern(FullPatternsTree* tree, char* pat, size_t n, PatternInternalID id) {
	fpt_add_pattern_to_node(tree->root, pat, n, id);
}


/**
* Create a new empty Full Patterns Tree
*
* @return       A new dynamically allocated empty Full Patterns Tree
*/
static FullPatternsTree* fpt_new() {
	FullPatternsTree* ret = (FullPatternsTree*) malloc(sizeof(FullPatternsTree));
	if (ret == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	ret->root = (FptNode*)malloc(sizeof(FptNode));
	if (ret->root == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	memset(ret->root, 0, sizeof(FptNode));
	SET_NULL_PATTERN_INTERNAL_ID(ret->root->pattern_id);
	return ret;
}

/**
* Fill an existing Fpt with all the patterns in a dictionary file
*
* @param tree           The Full Patterns Tree
* @param file_index     The index of the dictionary file name in dictionary_files
* @param filename       The name of the dictionary file
*
* @return               The longest patterns found
*/
static size_t fpt_fill_with_dict_file(FullPatternsTree* tree, size_t file_index, char* filename) {
	FILE* fp;
	char* line = NULL;
	char* pat = NULL;
	size_t len, pat_len, max_pat_len = 0;
	size_t line_num = 0;
	ssize_t read;
	PatternInternalID id;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "failed to open dictionary file %s: %s", filename, strerror(errno));
		FatalExit();
	}
	while ((read = getline(&line, &len, fp)) != -1) {
		++line_num;
		pat = NULL;
		if (line[read - 1] == '\n') --read;
		pat_len = parse_pattern_from_line(line, read, &pat);
		if (pat_len != 0) {
			id.file_number = file_index;
			id.line_number = line_num;
			fpt_add_pattern(tree, pat, pat_len, id);
			if (pat_len > max_pat_len) max_pat_len = pat_len;
		}
		if (pat) free(pat);
		
	}
	free(line);
	fclose(fp);
	return max_pat_len;
}

/**
* Build the full patterns tree from the dictionary files
*
* @param conf   The program configuration
*
* @return       A dynamically allocated Full-Patterns-Tree filled with the patterns from the dictionary files
*/
static FullPatternsTree* fpt_build(Conf* conf) {
	FullPatternsTree* ret = fpt_new();
	size_t i, n_dictionary_files = conf->n_dictionary_files, max_pat_len = 0, total_max_pat_len = 0;
	char** dictionary_files = conf->dictionary_files;

	for (i = 0; i < n_dictionary_files; ++i) {
		max_pat_len = fpt_fill_with_dict_file(ret, i, dictionary_files[i]);
		if (max_pat_len > total_max_pat_len) total_max_pat_len = max_pat_len;
	}
	ret->longest_pat_len = total_max_pat_len;
	conf->max_pat_len = total_max_pat_len;
	return ret;
}

/**
* Free a Full Patterns Tree node recursively
*/
static void fpt_free_node(FptNode* node) {
	FptEdge *current_edge, *next_edge;

	if (!node) {
		return;
	}
	current_edge = node->edge_list;
	while (current_edge) {
		next_edge = current_edge->next;
		if (current_edge->text) free(current_edge->text);
		if (current_edge->node) fpt_free_node(current_edge->node);
		free(current_edge);
		current_edge = next_edge;
	}
	free(node);
}

/**
* Free memory of Full Pattern Tree
*/
static void fpt_free(FullPatternsTree* full_tree) {
	fpt_free_node(full_tree->root);
	free(full_tree);
}

/**
* Add a new child to a node
*
* @param parent    The parent node
* @param child     The node to add the parent as child
*/
static void add_child_to_node(PatternsTreeNode* parent, PatternsTreeNode* child) {
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
* Convert an Fpt node to a (regular) Patterns Tree node recursively
*
* We go through the nodes in the full tree, while keeping the pattern for each node.
* For the purpose of saving the pattern for each node, we work on a buffer (long enough for the longest pattern)
* where we save the current pattern for a node in the end of the buffer, and save the position in the buffer
* from which the pattern start (so we can easily add/remove another pattern in the start).
*
* On every new pattern added, we call the callback function add_pattern_func on the given object obj
*
* @param node               The node to convert
* @param buffer             The buffer to work on
* @param buffer_len         The length of the buffer
* @param pat_pos            The position in the buffer where the pattern of the node starts
* @param obj                The object to add the patterns to
* @param add_pattern_func   The callback function to add pattern to given object
*
* @return          A dynamically allocated patterns tree node constructed from the given node
*/
static PatternsTreeNode* convert_fpt_node_to_patterns_tree_node(FptNode* node, char* buffer, size_t buffer_len, size_t pat_pos,
			void* obj, void (*add_pattern_func)(void*, char*, size_t, pattern_id_t)) {
	PatternsTreeNode* ret = (PatternsTreeNode*) malloc(sizeof(PatternsTreeNode));
	if (ret == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	PatternsTreeNode* temp;
	FptEdge* current_edge;

	memset(ret, 0, sizeof(PatternsTreeNode));
	copy_pattern_internal_id(&ret->pattern_id, &node->pattern_id);
	for (current_edge = node->edge_list; current_edge; current_edge = current_edge->next) {
		size_t len = current_edge->len;
		char* text = current_edge->text;
		memcpy(buffer + pat_pos - len, text, len);
		temp = convert_fpt_node_to_patterns_tree_node(current_edge->node, buffer, buffer_len, pat_pos - len,
				obj, add_pattern_func);
		temp->parent = ret;
		add_child_to_node(ret, temp);
	}
	if (pat_pos != buffer_len) { // if not empty pattern (root)
		add_pattern_func(obj, buffer + pat_pos, buffer_len - pat_pos, ret);
	}
	return ret;
}

/**
* Convert Full-Patterns-Tree to (regular) Patterns Tree
*
* Call callback function for each finished converted pattern, to add it to the given object
*
* @param full_tree          The Full Patterns Tree
* @param obj                The object to add the pattern to
* @param add_pattern_func   The callback function to add pattern to given object
*
* @return       A dynamically allocated Patterns Tree constructed by the given Full Patterns Tree
*/
static PatternsTree* convert_fpt_to_patterns_tree(FullPatternsTree* full_tree, void* obj,
			void (*add_pattern_func)(void*, char*, size_t, pattern_id_t)) {
	PatternsTree* tree = (PatternsTree*) malloc(sizeof(PatternsTree));
	if (tree == NULL) {
		perror("failed to allocate memory");
		FatalExit();
	}
	size_t llen = full_tree->longest_pat_len;
	char* buffer = (char*)malloc(llen);
	tree->root = convert_fpt_node_to_patterns_tree_node(full_tree->root, buffer, llen, llen, obj, add_pattern_func);
	free(buffer);
	return tree;
}

/**
* Free a node in a patterns tree recursilvely
*
* @param node       The node to free
*/
static void patterns_tree_free_node(PatternsTreeNode* node) {
	PatternsTreeEdge *current_edge, *next_edge;

	if (!node) {
		return;
	}
	current_edge = node->edge_list;
	while (current_edge) {
		next_edge = current_edge->next;
		if (current_edge->node) patterns_tree_free_node(current_edge->node);
		free(current_edge);
		current_edge = next_edge;
	}
	free(node);
}


/******************************************************************************
*		API FUNCTIONS
******************************************************************************/


/**
* Build a patterns tree from the dictionary files (configured in conf)
*
* Also call a given function for each finished converted pattern, to add the patterns to the
* given object, so we won't lose it (impossible to discover pattern from node in the patterns tree)
*
* @param conf                 The program configuration
* @param obj                  The object to add the patterns to
* @param add_patterns_func    The function that adds a pattern to the object
*
* @return     A dynamically allocated patterns tree generated from the patterns in the dictionary files
*/
PatternsTree* patterns_tree_build(Conf* conf,
                                  void* obj,
                                  void (*add_pattern_func)(void*, char*, size_t, pattern_id_t)) {
	FullPatternsTree* full_tree = fpt_build(conf);
	PatternsTree* tree = convert_fpt_to_patterns_tree(full_tree, obj, add_pattern_func);
	fpt_free(full_tree);
}

/**
* Return whether the first pattern is suffix of the second one
*
* @param first       The pattern to check if it is suffix of the ohter
* @param second      The pattern to check if it contains the other as a suffix
*
* @return            1 if the first is suffix of the second, 0 otherwise
*/
int is_pattern_suffix(pattern_id_t first, pattern_id_t second) {
	PatternsTreeNode* suf = first;
	PatternsTreeNode* cur = second;
	if (suf == NULL) return 0;
	while (cur != NULL) {
		if (cur == suf) return 1;
		cur = cur->parent;
	}
	return 0;
}

/**
* Free entire patterns tree
*
* @param tree       The tree to free
*/
void patterns_tree_free(PatternsTree* tree) {
	patterns_tree_free_node(tree->root);
	free(tree);
}

// ==============================================================
// ==================== FOT TESTING =============================
// ==============================================================
void print_fpt_node(FptNode* node, int indent) {
	FptEdge* cur;
	int i;
	for (cur = node->edge_list; cur; cur = cur->next) {
		for (i = 0; i < indent; ++i) printf(" ");
		printf(":"); print_binary_str(cur->text, cur->len); printf("\n");
		print_fpt_node(cur->node, indent + 2);
	}
}

void print_fpt(FullPatternsTree* full_tree) {
	print_fpt_node(full_tree->root, 0);
}

void print_patterns_tree_node(PatternsTreeNode* node, int indent) {
	PatternsTreeEdge* edge;
	int i;
	for (i = 0; i < indent; ++i) printf(" ");
	printf(":file = %lu:line = %lu:\n", node->pattern_id.file_number, node->pattern_id.line_number);
	for (edge = node->edge_list; edge; edge = edge->next) {
		print_patterns_tree_node(edge->node, indent + 2);
	}
}

void print_patterns_tree(PatternsTree* tree) {
	print_patterns_tree_node(tree->root, 0);
}