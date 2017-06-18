#ifndef CONF_H
#define CONF_H

typedef struct s_Conf {
	char** dictionary_files;
	size_t n_dictionary_files;
	char** stream_files;
	size_t n_stream_files;
	size_t max_pat_len;
} Conf;

#endif