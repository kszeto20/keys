#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "exec.h"

void parser_read() {
  // must be set to NULL and 0 to have getline allocate a string
  char *line = NULL;
  unsigned long len = 0;
  // stops at the first space:
//   scanf("%ms", &line);
  getline(&line, &len, stdin);
  parse_command(line);
  free(line);
}

void parse_command(char *str) {
  // tokenize
	int len = 1; // starts at 1 to ensure space for terminating null
	int cap = 1;
	char **result = malloc(cap * sizeof(char *));
	char *token;
	while (token = strsep(&str, " \n")) {
		// ignore empty tokens
		if (*token) {
			result[len-1] = token;
			// resize if reached end of array
			if (len == cap) {
				cap *= 2;
				result = realloc(result, cap * sizeof(char *));
			}
			len++;
		}
	}
	result[len-1] = NULL;
  //execute
  executeCommand(result);
  free(result);
}
