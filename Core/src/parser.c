#include <unistd.h>
#include <ctype.h>
#include "parser.h"
#include "conf.h"
#include "util.h"
#include "mps.h"

// get index to the next index in which there is no space
#define skip_spaces(arr, index) while((arr)[(index)] == ' ') ++(index)

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
* @param line   The line in the dictionary file
* @param n      The length of the line
* @param ret    Where to put the final pattern (override old value)
*
* @return       The length of the final pattern (that was put in ret)
*/
size_t parse_pattern_from_line(char* line, size_t n, char** ret) {
	if (n == 0) {
		return 0;
	}
	char* pattern = (char*) malloc(n);
	size_t len = 0, pos = 0, first, second, err = 0;
	while (pos < n && !err) {
		if (line[pos] == '|') {
			++pos;
			while (pos < n && line[pos] != '|') {
				skip_spaces(line, pos);
				first = get_binary_val(line[pos]);
				++pos;
				skip_spaces(line, pos);
				second = get_binary_val(line[pos]);
				++pos;
				if (first == -1 || second == -1) {
					err = 1;
					break;
				}
				pattern[len++] = (char)(first * 16 + second);
			}
			if (pos >= n) err = 1;
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
	memcpy(*ret, pattern, len);
	free(pattern);
	return len;
}

/**
* Parse the main arguments for the program and updates configuration data accordingly
*/
void parse_arguments(int argc, char* argv[], Conf* conf) {
	int opt, algo;
	size_t n_dict = 0, n_stream = 0, n_output = 0, dict_ind = 0, stream_ind = 0;
	
	opterr = 0;
	while ((opt = getopt(argc, argv, "d:s:o:")) != -1) {
		switch (opt) {
			case 'd': ++n_dict; break;
			case 's': ++n_stream; break;
			case 'o': ++n_output; break;
			default: break;
		}
	}
	if (n_output > 1) {
		// more than one output file, error
		fprintf(stderr, "Error: have more than one output file\n\n");
		print_usage_and_exit();
	}
	conf->n_dictionary_files = n_dict;
	conf->n_stream_files = n_stream;
	conf->dictionary_files = (char**) malloc(n_dict);
	conf->stream_files = (char**) malloc(n_stream);
	optind = 1;
	while ((opt = getopt(argc, argv, "d:s:o:")) != -1) {
		switch (opt) {
		case 'd':
			conf->dictionary_files[dict_ind] = (char*) malloc(strlen(optarg) + 1);
			strcpy(conf->dictionary_files[dict_ind], optarg);
			++dict_ind;
			break;
		case 's':
			conf->stream_files[stream_ind] = (char*) malloc(strlen(optarg) + 1);
			strcpy(conf->stream_files[stream_ind], optarg);
			++stream_ind;
			break;
		case 'o':
			conf->output_file_name = (char*) malloc(strlen(optarg) + 1);
			strcpy(conf->output_file_name, optarg);
			break;
		case '?':
			if (optopt == 'd' || optopt == 's' || optopt == 'o') {
				fprintf(stderr, "Option -%c must have argument.\n\n", optopt);
			} else if (isprint(optopt)) {
				fprintf(stderr, "Unknown option -%c.\n\n", optopt);
			} else {
				fprintf(stderr, "Unknown option character \\x%x.\n\n", optopt & 0xff);
			}
			print_usage_and_exit();
			break;
		default:
			print_usage_and_exit();
			break;
		}
	}

}