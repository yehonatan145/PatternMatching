/**
* Multi-Pattern Aho-Corasick Algorithm implementation
*
* We work in 2 stages:
*
* First, we add the patterns to the Aho-Corasick tree, which is implemented such
* that every node keep pointers to its children.
* 
* Then, when compiling, we transfer the tree to an array of states, where every state
* keep the position of its children in the states array (better for memory locality).
* 
* The failure likns are anyway generated after all patterns are inside the structure,
* So we creating the failure links only on the states array (not on the tree)
*/
#include "mpac.h"

/******************************************************************************
*      DEFINITIONS
******************************************************************************/

// A node in the Aho-Corasick tree (used before ac compilation)
typedef struct tree_node {
	struct tree_node *children[256];
	pattern_id_t id;
} TreeNode;

// A state in the states array (used after ac compilation)
typedef struct {
	size_t children[256];
	size_t failure_state;
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
*     INNER FUNCTIONS
******************************************************************************/

/**
* Create new queue
*
* @return    A new dynamically allocated queue
*/
Queue* queue_create() {
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
void queue_add(Queue* q, size_t state) {
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
size_t queue_pop(Queue* q) {
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
size_t convert_tree_to_states(TreeNode* node, State* states, size_t from) {
	size_t i, pos = from++;
	TreeNode* cur;
	states[pos].id = node->id;
	for (i = 0; i < 256; ++i) {
		cur = node->children[i];
		if (cur) {
			states[pos].children[i] = from;
			from = convert_tree_to_states(cur, states, from);
		} else {
			states[pos].children[i] = 0;
		}
	}
	return from;
}

/**
* Add the failure link to a state from its parent and last char
*
* @param states     The states
* @param parent     The position of the state's parent
* @param c          The character to the node from its parent
*/
void add_failure_to_state(State* states, size_t parent, unsigned char c) {
	size_t fs = states[parent].failure_state;
	size_t state = states[parent].children[c];
	while (!states[fs].children[c] && fs) {
		fs = states[fs].failure_state;
	}
	if (states[fs].children[c]) {
		states[state].failure_state = states[fs].children[c];
	} else { // fs is 0, and there is no child with letter c
		states[state].failure_state = 0;
	}
}

/**
* Add failure links to the array of states
*
* @param states    The array of states
*/
void add_failure_links(State* states) {
	Queue* q = queue_create();
	size_t i, curState;
	queue_add(q, 0);
	// add the first level to the queue, and put their failure link to 0
	states[0].failure_state = 0;
	for(i = 0; i < 256; ++i) {
		curState = states[0].children[i];
		if (curState) {
			queue_add(q, curState);
			states[curState].failure_state = 0;
		}
	}
	while (queue_not_empty(q)) {
		curState = queue_pop(q);
		for (i = 0; i < 256; ++i) {
			if (states[curState].children[i]) {
				add_failure_to_state(states, curState, (unsigned char)i);
				queue_add(q, states[curState].children[i]);
			}
		}
	}
}

/**
* Free the memory of the aho-corasick tree
*
* @param root    The root of the tree
*/
void free_tree(TreeNode *root) {
	size_t i;
	for (i = 0; i < 256; ++i) {
		if (root->children[i]) free_tree(root->children[i]);
	}
	free(root);
}

/******************************************************************************
*     API FUNCTIONS
******************************************************************************/

/**
* Create new AC struct
*
* @return     A new dynamically allocated ac object
*/
void* ac_create() {
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
void ac_add_pattern(void* obj, char* pat, size_t len, pattern_id_t id) {
	AC *ac = (AC*)obj;
	TreeNode *cur = ac->root, *next;
	size_t i = 0;
	while (i < len && cur->children[pat[i]]) {
		cur = cur->children[pat[i++]];
	}
	for (; i < len; ++i) {
		next = (TreeNode*)malloc(sizeof(TreeNode));
		memset(next, 0, sizeof(TreeNode));
		next->id = null_pattern_id;
		cur->children[pat[i]] = next;
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
void ac_compile(void* obj) {
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
pattern_id_t ac_read_char(void* obj, char c) {
	AC *ac = (AC*)obj;
	size_t current_state = ac->current_state;
	State* states = ac->states;
	while (!states[current_state].children[c] && current_state) {
		current_state = states[current_state].failure_state;
	}
	if (states[current_state].children[c]) {
		ac->current_state = states[current_state].children[c];
		return states[ac->current_state].id;
	} else {
		ac->current_state = current_state;
		return states[current_state].id;
	}
}

/**
* Aho-Corasick get total memory function.
*
* @param obj     The ac object
*
* @return        The total memory used for this object
*/
size_t ac_total_mem(void* obj) {
	if (obj == NULL) return 0;
	AC* ac = (AC*)obj;
	return sizeof(AC) + ac->n_states * sizeof(State);
}

/**
* Aho-Corasick reset function (reset the object back to initial state)
*
* @param obj    The ac object
*/
void ac_reset(void* obj) {
	AC* ac = (AC*)obj;
	ac->current_state = 0;
}

/**
* Free the memory of the ac object (must be done after compilation).
*
* @param obj    The ac object to free
*/
void ac_free(void *obj) {
	AC *ac = (AC*)obj;
	if (ac->states) free(ac->states);
	free(ac);
}

/**
* The mps registering function of the Aho-Corasick Algorithm.
*/
void mps_ac_register() {
	mps_table[MPS_AC].name = "Aho-Corasick";
	mps_table[MPS_AC].create = ac_create;
	mps_table[MPS_AC].add_pattern = ac_add_pattern;
	mps_table[MPS_AC].compile = ac_compile;
	mps_table[MPS_AC].read_char = ac_read_char;
	mps_table[MPS_AC].total_mem = ac_total_mem;
	mps_table[MPS_AC].reset = ac_reset;
	mps_table[MPS_AC].free = ac_free;
}