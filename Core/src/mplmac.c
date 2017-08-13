/**
* Multi-Pattern Low-Memory Aho-Corasick Algorithm implementation
*
* This is basically the same file as "mpac.c" except, instead of saving the children
* in a array of size 256, we keep a list, in which every element is a pair of (char, state)
*/


/******************************************************************************
*		INCLUDES
******************************************************************************/


#include "mpac.h"


/******************************************************************************
*		DEFINITIONS
******************************************************************************/

// A node in the Aho-Corasick tree (used before ac compilation)
typedef struct tree_node {
	struct tree_node *children[256];
	pattern_id_t id;
} TreeNode;

typedef struct children_list_node {
	struct children_list_node  *next;   // The next node in the list
	char                        c;      // The character of the child
	size_t                      state;  // The child state
} ChildrenListNode;

typedef struct children_list {
	ChildrenListNode *head;
} ChildrenList;

/**
* iterate over children list
* @node:	the node that iterate (of type ChildrenListNode*)
* @ch:		the character of the current child (of type char)
* @stt:	the state of the current child (of type size_t)
* @list:	the list itself (of type ChildrenList)
*/
#define foreach_child(node,ch,stt,list) \
        for(node = (list).head, ch = node ? node->c : (char)0, stt = node ? node->state : 0 \
        	; node != NULL \
        	; node = node->next, \
        	  ch = node ? node->c : (char)0, \
        	  stt = node ? node->state : 0)

// A state in the states array (used after ac compilation)
typedef struct {
	ChildrenList children;
	size_t failure_state;
	size_t suffix_link;
	pattern_id_t id;
} State;

typedef struct {
	union {
		TreeNode *root;   // Aho-Corasick tree for before compilation
		State    *states; // states array for after compilation
	};
	size_t n_states;
	size_t current_state;
} AC;

typedef struct qnode {
	struct qnode *next;
	size_t        state;
} QNode;

typedef struct queue {
	QNode *head, *tail;
} Queue;


/******************************************************************************
*		INNER FUNCTIONS
******************************************************************************/


//=========== FOR TESTING ===================
static void _print_states(AC* ac);
//=========== FOR TESTING ===================

/**
* Create new queue
*
* @return    A new dynamically allocated queue
*/
static inline Queue* queue_create() {
	Queue *q = (Queue*)malloc(sizeof(Queue));
	memset(q, 0, sizeof(Queue));
	return q;
}

/**
* Add new state to the queue
*
* @param q      The queue
* @param state  The state to add
*/
static inline void queue_add(Queue* q, size_t state) {
	QNode *node = (QNode*)malloc(sizeof(QNode));
	node->next = NULL;
	node->state = state;
	if (q->tail) {
		q->tail->next = node;
		q->tail = node;
	} else {
		q->head = q->tail = node;
	}
}

/**
* Pop state from the queue
*
* @param q    The queue
* 
* @return    The first state was on the queue
*/
static inline size_t queue_pop(Queue* q) {
	if (!q->head) return 0;
	QNode* temp = q->head;
	size_t state = temp->state;
	q->head = temp->next;
	if (!q->head) q->tail = NULL;
	free(temp);
	return state;
}

/**
* Check whether the queue is not empty
*
* @param q    The queue
*
* @return     1 if the queue is not empty, 0 if empty
*/
static inline int queue_not_empty(Queue* q) {
	return q->head ? 1 : 0;
}

/**
* Free the queue
*
* @param q     The queue to free
*/
static inline void queue_free(Queue* q) {
	free(q);
}

/**
* Add a pair of (char, state) to the children list
*
* @param list      Pointer to the children list
* @param c         The character of that child from the parent
* @param state     The child state (index in states array)
*/
static inline void children_list_add(ChildrenList* list, char c, size_t state) {
	ChildrenListNode* node = (ChildrenListNode*)malloc(sizeof(ChildrenListNode));
	node->next = list->head;
	node->c = c;
	node->state = state;
	list->head = node;
}

/**
* Free a children list
*
* @param list     The children list to free
*/
static inline void free_children_list(ChildrenList* list) {
	ChildrenListNode *cur = list->head, *next;
	while (cur) {
		next = cur->next;
		free(cur);
		cur = next;
	}
}

/**
* Find the child state with the given char
* 
* @param list     Pointer to the children list
* @param c        The character of the child from the parent
*
* @return         The state of the child of that character, or 0 if no such child
*/
static inline size_t find_child(ChildrenList* list, char c) {
	char ch;
	size_t state;
	ChildrenListNode* node;
	foreach_child(node, ch, state, *list) {
		if (ch == c) return state;
	}
	return 0;
}

/**
* Find the child state with the given char from the states array and the index
*
* @param state       The states array
* @param index       The index of the state in the states array
* @param c           The character of the child from the parent
*
* @return            The state of the child of that character, or 0 if no such child
*/
static inline size_t find_child_from_index(State* states, size_t index, char c) {
	return find_child(&states[index].children, c);
}

/**
* Convert tree to states
*
* Receive the node that is the root of the tree, and recursively build the tree.
* Create the new node at states[from], and return where is the next free state.
*
* @param node      The root of the tree to convert
* @param states    The array of states
* @param from      The next position to add states from
*
* @return       The next available position in the states array
*/
static size_t convert_tree_to_states(TreeNode* node, State* states, size_t from) {
	size_t i, pos = from++;
	TreeNode* cur;
	states[pos].id = node->id;
	for (i = 0; i < 256; ++i) {
		cur = node->children[i];
		if (cur) {
			children_list_add(&states[pos].children, (char)i, from);
			from = convert_tree_to_states(cur, states, from);
		} else {
			// not adding anything to the list (no children)
		}
	}
	return from;
}

/**
* Add the failure link to a state from its parent and last char
*
* Also add the suffix link
*
* @param states     The states
* @param parent     The position of the parent
* @param c          The character to the node from its parent
* @param child      The position of the child
*/
static void add_failure_to_state(State* states, size_t parent, char c, size_t child) {
	size_t fs = states[parent].failure_state;
	size_t fs_child = find_child_from_index(states, fs, c);
	while (fs && !fs_child) {
		fs = states[fs].failure_state;
		fs_child = find_child_from_index(states, fs, c);
	}
	states[child].failure_state = fs_child;
	states[child].suffix_link = states[child].id == null_pattern_id ? states[states[child].failure_state].suffix_link : child;
}

/**
* Add failure links to the array of states (aldo add suffix links)
*
* @param states    The array of states
*/
static void add_failure_links(State* states) {
	Queue* q = queue_create();
	size_t i, cur_state, child_state;
	char c;
	ChildrenListNode* node;
	// add the first level to the queue, and put their failure link to 0
	states[0].failure_state = 0;
	states[0].suffix_link = 0;
	foreach_child(node, c, cur_state, states[0].children) {
		queue_add(q, cur_state);
		states[cur_state].failure_state = 0;
		states[cur_state].suffix_link = states[cur_state].id == null_pattern_id ? 0 : cur_state;
	}
	while (queue_not_empty(q)) {
		cur_state = queue_pop(q);
		foreach_child(node, c, child_state, states[cur_state].children) {
			add_failure_to_state(states, cur_state, c, child_state);
			queue_add(q, child_state);
		}
	}
	queue_free(q);
}

/**
* Free the memory of the aho-corasick tree
*
* @param root    The root of the tree
*/
static void free_tree(TreeNode *root) {
	size_t i;
	for (i = 0; i < 256; ++i) {
		if (root->children[i]) free_tree(root->children[i]);
	}
	free(root);
}


/******************************************************************************
*		API FUNCTIONS
******************************************************************************/


/**
* Create new AC struct
*
* @return     A new dynamically allocated ac object
*/
void* lmac_create() {
	AC* ac = (AC*)malloc(sizeof(AC));
	memset(ac, 0, sizeof(AC));
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
	memset(root, 0, sizeof(TreeNode));
	root->id = null_pattern_id;
	ac->root = root;
	ac->n_states = 1;
	return (void*)ac;
}

/**
* Add pattern to the ac object
*
* Add the pattern to the Aho-Corasick tree, and create all midway states.
*
* @param obj      The ac object
* @param pat      The pattern to add
* @param len      The length of the pattern
* @param id       The id of the pattern
*/
void lmac_add_pattern(void* obj, char* pat, size_t len, pattern_id_t id) {
	AC *ac = (AC*)obj;
	TreeNode *cur = ac->root, *next;
	size_t i = 0;
	while (i < len && cur->children[(unsigned char)pat[i]]) {
		cur = cur->children[(unsigned char)pat[i++]];
	}
	for (; i < len; ++i) {
		next = (TreeNode*)malloc(sizeof(TreeNode));
		memset(next, 0, sizeof(TreeNode));
		next->id = null_pattern_id;
		cur->children[(unsigned char)pat[i]] = next;
		cur = next;
		ac->n_states++;
	}
	cur->id = id;
}

/**
* Compile the Aho-Corasick object.
*
* Transfer the Aho-Corasick tree to an array of states, and add failure links.
*
* @param obj     The ac object
*/
void lmac_compile(void* obj) {
	AC *ac = (AC*)obj;
	State *states = (State*)malloc(ac->n_states * sizeof(State));
	memset(states, 0, ac->n_states * sizeof(State));

	convert_tree_to_states(ac->root, states, 0); // should return n_states
	add_failure_links(states);
	free_tree(ac->root);
	ac->states = states;
}

/**
* Aho-Corasick read next char in the stream function.
*
* It go through the failure links until it finds state which have
* a child with the given character, and reutrn its id.
*
* @param obj    The ac object
* @param c      The next character in the stream
*
* @return       The id of the longest pattern that have a match
*/
pattern_id_t lmac_read_char(void* obj, char c) {
	AC *ac = (AC*)obj;
	State* states = ac->states;
	size_t current_state = ac->current_state;
	size_t current_child = find_child_from_index(states, current_state, c);
	while (current_state && !current_child) {
		current_state = states[current_state].failure_state;
		current_child = find_child_from_index(states, current_state, c);
	}
	if (current_child) {
		ac->current_state = current_child;
	} else{
		ac->current_state = current_state;
	}
	return states[states[ac->current_state].suffix_link].id;
}

/**
* Aho-Corasick get total memory function.
*
* @param obj     The ac object
*
* @return        The total memory used for this object
*/
size_t lmac_total_mem(void* obj) {
	if (obj == NULL) return 0;
	AC* ac = (AC*)obj;
	size_t i, n = ac->n_states, total_mem;
	total_mem = sizeof(AC) + ac->n_states * sizeof(State);
	for (i = 0; i < n; ++i) {
		ChildrenListNode* node;
		size_t state;
		char c;
		foreach_child(node, c, state, ac->states[i].children) {
			total_mem += sizeof(ChildrenListNode);
		}
	}
	return total_mem;
}

/**
* Aho-Corasick reset function (reset the object back to initial state)
*
* @param obj    The ac object
*/
void lmac_reset(void* obj) {
	AC* ac = (AC*)obj;
	ac->current_state = 0;
}

/**
* Free the memory of the ac object (must be done after compilation).
*
* @param obj    The ac object to free
*/
void lmac_free(void *obj) {
	AC *ac = (AC*)obj;
	size_t i, n = ac->n_states;
	for (i = 0; i < n; ++i) {
		free_children_list(&ac->states[i].children);
	}
	if (ac->states) free(ac->states);
	free(ac);
}

/**
* The mps registering function of the Aho-Corasick Algorithm.
*/
void mps_lmac_register() {
	mps_table[MPS_LMAC].name = "Low-Memory Aho-Corasick";
	mps_table[MPS_LMAC].create = lmac_create;
	mps_table[MPS_LMAC].add_pattern = lmac_add_pattern;
	mps_table[MPS_LMAC].compile = lmac_compile;
	mps_table[MPS_LMAC].read_char = lmac_read_char;
	mps_table[MPS_LMAC].total_mem = lmac_total_mem;
	mps_table[MPS_LMAC].reset = lmac_reset;
	mps_table[MPS_LMAC].free = lmac_free;
}

//=========================================================
//================ FOR TESTING ============================
//=========================================================

static void _print_states(AC* ac) {
	State* state;
	char c;
	size_t stt;
	ChildrenListNode* node;
	printf("printing ac states, number of states = %lu\n", ac->n_states);
	for (size_t i = 0; i < ac->n_states; ++i) {
		state = &ac->states[i];
		printf("state %lu, id = %p, failure state = %lu\n", i, state->id, state->failure_state);
		foreach_child(node, c, stt, state->children) {
			printf("  ");
			print_binary_str(&c, 1);
			printf(", state = %lu\n", stt);
		}
	}
}