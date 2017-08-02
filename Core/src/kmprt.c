/**
* Real-Time version of the KMP algorithm
*
* This algorithm do exactly the same as regular KMP, but to make it real time,
* every time we need to loop through the failure function more than twice,
* we do this while other characters is being read 2 times faster, (i.e. on every arriving character from the stream,
* we loop 2 times through the failure function).
* In the meantime, in order to go over all the arriving characters, we put them in a round-robin buffer used like a queue.
* When we finish looping through the failure function, we continue to read characters from the buffer 2 at a time
* (i.e. on every arriving character from the stream, we read 2 chararcters from the buffer).
* Note that it possible that while reading characters from the buffer, we need to loop the failure function again,
* so we just continue the same way as before to handle it (while there are characters in the buffer).
*
* note: the kmp algorithm also used to find the period of a pattern, as in the next formula:
* the period length of P[1..m] = m - failure_function[m], when the failure function was built on P.
*/
#define _CRT_SECURE_NO_WARNINGS
#include "kmprt.h"

/**
* Create the failure table for the failure function.
*
* @param pattern  The pattern
* @param n        The length of the pattern
*
* @return    Dynamic allocated array of size (n + 1), which represent the failure function.
*            i.e. the i-th position is the last position we know we have match in.
*/
size_t* kmp_create_failure_table(char* pattern, size_t n) {
	size_t *failure_table = (size_t*) malloc ((n + 1) * sizeof(size_t));
	failure_table[0] = failure_table[1] = 0;
	size_t pos = 2, cnd = 0;
	while (pos < n + 1) {
		if (pattern[pos - 1] == pattern[cnd]) {
			failure_table[pos++] = cnd++ + 1;
		} else if (cnd > 0) {
			cnd = failure_table[cnd];
		} else {
			failure_table[pos++] = 0;
		}
	}
	return failure_table;
}

/**
* Create new KMPRealTime according to a specific pattern.
*
* @param pattern  The pattern for the struct
* @param n        The pattern length
*
* @return         Dynamicaly allocated KMPRealTime for searching the given pattern
*/
KMPRealTime* kmp_new(char* pattern, size_t n) {
	KMPRealTime* kmp = (KMPRealTime *) malloc (sizeof(KMPRealTime));
	memset(kmp, 0, sizeof(KMPRealTime));
	kmp->n = n;
	kmp->pattern = (char*) malloc (n * sizeof(char));
	memcpy(kmp->pattern, pattern, n);
	kmp->buffer = (char*) malloc (n * sizeof(char));
	kmp->failure_table = kmp_create_failure_table(pattern, n);
	return kmp;
}

/**
* Get the total memory used for a KMP struct.
*
* Return the size of the struct, plus all the allocated memory which is:
*   n * sizeof(char) for the pattern
*   n * sizeof(char) for the buffer
*   (n + 1) * sizeof(size_t) for the failure table
*/
size_t kmp_get_total_mem(KMPRealTime* kmp) {
	if (kmp == NULL) return 0;
	return sizeof(KMPRealTime) + kmp->n * (2 * sizeof(char) + sizeof(size_t)) + sizeof(size_t);
}

/**
* Return the period length of the pattern.
*
* @param pattern  The pattern
* @param n        The length of the pattern
*
* @return     The length of the period of the pattern
*/
size_t kmp_get_period(char* pattern, size_t n) {
	size_t* failure_table = kmp_create_failure_table(pattern, n);
	size_t ret =  n - failure_table[n];
	free(failure_table);
	return ret;
}

/**
* Move to the next step of the failure function (and update the offset accordingly).
*
* If succeed (return true), it change the kmp to the state of right AFTER the char was called
* (i.e. ready for the char that comes after {@param c})
*
* @param kmp    The kmp struct
* @param c      The char that was mismatched that started the failure function loop
*
* @return       True if the failure function finished (we got the first position at which this character matches)
*               False if not
*/
int _kmp_move_failure_function(KMPRealTime* kmp, char c) {
	kmp->offset = kmp->failure_table[kmp->offset];
	if (kmp->pattern[kmp->offset] == c) {
		kmp->offset++;
		return 1;
	} else if (kmp->offset == 0) {
		return 1;
	} else {
		return 0;
	}
}

/**
* Add character to the buffer at the end.
*
* @param kmp    The kmp struct
* @param c      The character to add to the buffer
*/
void _kmp_add_char_to_buffer(KMPRealTime* kmp, char c) {
	if (kmp->flags & KMP_HAVE_BUFFER_FLAG) {
		MOD_INC(kmp->buf_end, kmp->n);
		kmp->buffer[kmp->buf_end] = c;
	} else {
		kmp->buf_start = kmp->buf_end = 0;
		kmp->buffer[0] = c;
		kmp->flags |= KMP_HAVE_BUFFER_FLAG;
	}
}

/**
* Add character to the buffer at the start.
*
* @param kmp    The kmp struct
* @param c      The character to add tp the buffer
*/
void _kmp_push_char_to_buffer(KMPRealTime* kmp, char c) {
	if (kmp->flags & KMP_HAVE_BUFFER_FLAG) {
		MOD_DEC(kmp->buf_start, kmp->n);
		kmp->buffer[kmp->buf_start] = c;
	}
	else {
		kmp->buf_start = kmp->buf_end = 0;
		kmp->buffer[0] = c;
		kmp->flags |= KMP_HAVE_BUFFER_FLAG;
	}
}

/**
* Pop char from buffer.
*
* @param kmp    The kmp struct
*
* @return       The first char in the buffer
*/
char _kmp_pop_buffer(KMPRealTime* kmp) {
	char c = kmp->buffer[kmp->buf_start];
	if (kmp->buf_start == kmp->buf_end) {
		kmp->flags &= ~KMP_HAVE_BUFFER_FLAG;
	}
	MOD_INC(kmp->buf_start, kmp->n);
	return c;
}

/**
* Simulates the kmp algorithm to read char from stream.
* If it needs to loop through too many failure function moves: 
*  -Set KMP_LOOP_FAIL_FLAG
*  -Put the char in the start of the buffer for later use (if the buffer is empty)
*  -And return 0.
*
* @param kmp    The kmp struct
* @param c      The char
*
* @return       Whether there is a match
*/
int _kmp_read_char(KMPRealTime* kmp, char c) {
	if (kmp->pattern[kmp->offset] == c) {
		kmp->offset++;
		if (kmp->offset == kmp->n) {
			kmp->offset = kmp->failure_table[kmp->n]; // In the n-th place, there is the next offset after successful match
			return 1;
		}
	} else if (kmp->offset == 0) {
		return 0;
	} else {
		//printf("mismatch, offset is %d\n", kmp->offset);
		int i;
		for (i = 0; i < 2; ++i) {
			if (_kmp_move_failure_function(kmp, c)) {
				return 0;
			}
		}
		//printf("keep looping, offset is %d, flags is %d\n", kmp->offset, kmp->flags);
		// If got here, there is need in looping through the failure function
		kmp->flags |= KMP_LOOP_FAIL_FLAG;
		/*
		* If the buffer is empty then 'c' is the stream current character, so we need to enter it
		* to the buffer for later use.
		* If the buffer is not empty then we popped the first char in the buffer so now we need to enter
		* it back for the loop through the failure function.
		* Either way, we need to enter the char back to the start of the buffer.
		*/
		_kmp_push_char_to_buffer(kmp, c);
	}
	return 0;
}

/**
* Read char from stream and return whether we have a match.
*
* @param kmp    The KMPRealTime that used for the pattern we search
* @param c      The new character from the stream
*
* @return       1 if there is match, 0 if not
*/
int kmp_read_char(KMPRealTime* kmp, char c) {
	int i;
	if (kmp->flags & KMP_LOOP_FAIL_FLAG) { // Have failure function moves to do
		//printf("backtracing - offset: %d, flags: %d\n", kmp->offset, kmp->flags);
		_kmp_add_char_to_buffer(kmp, c);
		for (i = 0; i < 2; ++i) {
			if (_kmp_move_failure_function(kmp, kmp->buffer[kmp->buf_start])) {
				_kmp_pop_buffer(kmp);
				kmp->flags &= ~KMP_LOOP_FAIL_FLAG;
				break;
			}
		}
		return 0;
	} else if (kmp->flags & KMP_HAVE_BUFFER_FLAG) { // Have chars waiting in buffer
		//printf("have buffer of length %d - offset: %d, flags: %d\n", kmp->buf_end - kmp->buf_start + 1, kmp->offset, kmp->flags);
		_kmp_add_char_to_buffer(kmp, c);
		for (i = 0; i < 2; ++i) {
			c = _kmp_pop_buffer(kmp);
			//printf("poped from buffer: %c\n", c);
			if (_kmp_read_char(kmp,c)) {
				// If algorithm works fine, should happen only when we just finished the buffer (not before)
				return 1;
			}
		}
		//printf("after working on buffer, offset: %d, flags: %d\n", kmp->offset, kmp->flags);
		return 0;
	} else {
		 return _kmp_read_char(kmp, c);
	}
}

/**
* Free the memory for KMPRealTime
*
* @param kmp     The KMPRealTime to free
*/
void kmp_free(KMPRealTime* kmp) {
	free(kmp->failure_table);
	free(kmp->pattern);
	free(kmp->buffer);
	free(kmp);
}




// ===================     FOR TESTING     ================================

/*
int main() {
	unsigned int i;
//             0         1
//             012345678901234567
	char *p = "AAAAAAAAAAAAAAAAAB";
//             0         1         2         3         4         5
//             012345678901234567890123456789012345678901234567890
	char *t = "AAAAAAAAAAAAAAAAABAAAAAABAAAAAAAAAAAAAAAAABAAAAAAA";
	KMPRealTime* kmp = kmp_new(p, strlen(p));
	printf("failure table:\n");
	for (i = 0; i < kmp->n+1; ++i) {
		printf("    %d:%d\n",i,kmp->failure_table[i]);
	}
	for (i = 0; i < strlen(t); ++i) {
		//printf("on pos %d on char %c\n", i,t[i]);
		if (kmp_read_char(kmp, t[i])) {
			printf("found on char %d\n", i);
		}
	}
	kmp_free(kmp);
	char a;
	scanf("%d", &a);
}
*/