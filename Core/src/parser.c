#include "parser.h"

// get index to the next index in which there is no space
#define skip_spaces(arr, index) while(arr[index] == ' ') ++index

/**
* Get the value of binary char (return -1 if not binary)
*/
int get_binary_val(char bin) {
	if (bin >= '0' && bin <= '9') {
		return bin - '0';
	} else if (bin >= 'a' && bin <= 'f') {
		return bin - 'a' + 10;
	} else if (bin >= 'A' && bin <= 'F') {
		return bin - 'A' + 10;
	} else {
		return -1;
	}
}

/**
* Parse a pattern from a dictionary file line
* 
* @param line	The line in the dictionary file
* @param n		The length of the line
* @param ret	Where to put the final pattern
*
* @return		The length of the final pattern (that was put in ret)
*/
size_t parse_pattern_from_line(char* line, size_t n, char** ret) {
	char* pattern = (char*) malloc(n);
	size_t len = 0, pos = 0, first, second, err = 0;
	while (pos < n && !err) {
		if (line[pos] == '|') {
			++pos;
			while (line[pos] != '|') {
				skip_spaces(line, pos);
				first = get_bin_val(line[pos]);
				++pos;
				skip_spaces(line, pos);
				second = get_bin_val(line[pos]);
				++pos;
				if (first == -1 || second == -1) {
					err = 1;
					break;
				}
				pattern[len++] = (char)(first * 16 + second);
			}
			++pos;
		} else {
			pattern[len++] = line[pos++];
		}
	}
	if (err) {
		free(pattern);
		return 0;
	}
	*ret = (char*)malloc(len);
	strncpy(*ret, pattern, len);
	free(pattern);
	return len;
}

/**
* Parse the main arguments for the program and updates global data accordingly
*/
void parse_arguments(int argv, char* argc[]) {
	// TODO: implement
}