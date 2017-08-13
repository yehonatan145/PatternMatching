#ifndef PARSER_H
#define PARSER_H


/******************************************************************************************************
*		INCLUDES
******************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/******************************************************************************************************
*		API FUNCTIONS
******************************************************************************************************/


struct _Conf;

/**
* Parse a pattern from a dictionary file line
*/
size_t parse_pattern_from_line(char* line, size_t n, char** ret);
void parse_arguments(int argc, char* argv[], struct _Conf* conf);

#endif