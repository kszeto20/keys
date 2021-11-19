#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <termios.h>

#include "parser.h"
#include "exec.h"

void enableinputmode() {
	struct termios info;
	tcgetattr(0, &info);
	info.c_lflag &= ~ICANON;
	info.c_cc[VMIN] = 1;
	info.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &info);
}

void disableinputmode() {
	struct termios info;
	tcgetattr(0, &info);
	info.c_lflag |= ICANON;
	tcsetattr(0, TCSANOW, &info);
}


void readline(char **line, unsigned long *len, FILE *stream) {
	enableinputmode();
	while (1) {
		int c = getc(stream);
		printf("\nc: %c, i: %d\n", c, c);
	}
	disableinputmode();
}

void parser_read() {
  // must be set to NULL and 0 to have getline allocate a string
  char *line = NULL;
  unsigned long len = 0;
  // stops at the first space:
//   scanf("%ms", &line);
//   getline(&line, &len, stdin);
	readline(&line, &len, stdin);
  
  char *linepointer = line;
  
  int shouldStop = 0;
  while (!shouldStop && linepointer != NULL) {
  	shouldStop = parse_command(&linepointer);
  }
  free(line);
}

void print_arr(char **arr) {
	int i;
	for (i = 0; arr[i]; i++) {
		printf("%d: \"%s\"\n", i, arr[i]);
	}
}

int parse_command(char **str) {
  // tokenize
	int len = 1; // starts at 1 to ensure space for terminating null
	int cap = 1;
	char **result = malloc(cap * sizeof(char *));
	char *token;
	
	while (token = strsep(str, " \n")) {
		// ignore empty tokens
		if (*token) {
			// execute what we have so far if semicolon
			if (!strcmp(token, ";")) break;
			if (!strcmp(token, "&&")) {
				result[len-1] = NULL;
				print_arr(result);
				int returnVal = executeCommand(result);
				free(result);
				return returnVal;
			}
			if (!strcmp(token, "||")) {
				result[len-1] = NULL;
				print_arr(result);
				int returnVal = executeCommand(result);
				free(result);
				return !returnVal;
			}
			
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
	return 0;
}
