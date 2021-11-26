#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <termios.h>

#include "parser.h"
#include "exec.h"
#include "input.h"

struct cInput {char *input; struct cInput * before; struct cInput * after;};

mode_t mainMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
/*
void enableinputmode() {
	if (isatty(fileno(stdin))) {
		struct termios info;
		tcgetattr(0, &info);
		info.c_lflag &= ~ICANON;
		info.c_cc[VMIN] = 1;
		info.c_cc[VTIME] = 0;
		tcsetattr(0, TCSANOW, &info);
	}
}

void disableinputmode() {
	if (isatty(fileno(stdin))) {
		struct termios info;
		tcgetattr(0, &info);
		info.c_lflag |= ICANON;
		tcsetattr(0, TCSANOW, &info);
	}
}
*/

char * getHistFile() {
	char * toRet = malloc (256 * sizeof(char));
	strcpy(toRet, getenv("HOME"));
	strcat(toRet, "/command_history");
	return toRet;
}

char * open_and_read () {
	char * file = getHistFile();
	//printf("THE FILE NAME IS : %s\n", file);
	int in = open(file, O_RDONLY);
	//int in = creat(file, mainMode);
	if (errno != 0) {
    printf("An error has occured \n%s", strerror(errno));
    return NULL;
  }
	struct stat fileS;
	stat(file, &fileS);
	int fileSize = fileS.st_size;

	char * all = malloc(fileSize + 1);
	read(in, all, fileSize);
	//printf("THIS IT THE STRING%s\n", all);
	close(in);
	free(file);
	return all;
}

void write_in (char * toWrite) {
	char * histFile = getHistFile();
	//printf("Opening\n");
	//int out = open(histFile, O_RDWR);
	//printf("Opened\n");
	int out = creat(histFile, mainMode);
	//printf("Start reading\n");
	char * orig = open_and_read();
	//printf("Finished reading\n");
	write(out, toWrite, strlen(toWrite));
	write(out, "\n", 1);
	write(out, orig, strlen(orig));
	free(histFile);
	free(orig);
	close(out);
}


void parser_read() {
  // must be set to NULL and 0 to have getline allocate a string
  char *line = NULL;
  unsigned long len = 0;
  // stops at the first space:
//   scanf("%ms", &line);
	//doread();
  getline(&line, &len, stdin);
	//printf("Got here\n");
	write_in(line);
//	readline(&line, &len);

  char *linepointer = line;

  int shouldStop = 0;
  while (!shouldStop && linepointer != NULL) {
  	shouldStop = parse_command(&linepointer);
  }
	//char * hello = "what's good homies";
  free(line);
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
				//print_arr(result);
				int returnVal = executeCommand(result);
				free(result);
				return returnVal;
			}
			if (!strcmp(token, "||")) {
				result[len-1] = NULL;
				//print_arr(result);
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

/*
void print_arr(char **arr) {
	int i;
	for (i = 0; arr[i]; i++) {
		printf("%d: \"%s\"\n", i, arr[i]);
	}
}
*/

/*
struct cInput * takeIn(char * into, struct cInput * b, struct cInput * a) {
	struct cInput *i = malloc(sizeof(struct cInput));
	i->input = into;
	i->before = b;
	i->after = a;
	return i;
}
*/
/*

void readline(char **line, unsigned long *len) {
	enableinputmode();
	int counter = 0;
	char * c;
	char charIn;
	charIn = (char) (getc(stdin));
	c = &charIn;
	struct cInput * front = takeIn(c, NULL, NULL);

	struct cInput * current = front;
	while (*c != '\n') {
		charIn = (char) (getc(stdin));
		c = &charIn;
		struct cInput * madeNew = takeIn(c, current, NULL);
		current->after = madeNew;
		current = madeNew;
		counter++;
	}
	print_list(front);
	//free_list(front);

	printf("%d\n", counter);

	// while (1) {
	// 	int c = getc(stdin);
	// 	printf("\nc: %c, i: %d\n", c, c);
	// }
	disableinputmode();
}
*/
/*
struct cInput * free_list(struct cInput * toFree) {
  while(toFree != NULL) {
    struct cInput *nToFree = toFree->after;
    free(toFree);
    toFree = nToFree;
  }
  return toFree;
}
*/
/*

void print_list(struct cInput *front) {
  while (front) {
    printMember(front);
    front = front->after;
  }
}


int printMember(struct cInput * p) {
  printf("%s", p->input);
  return 0;
}
*/
