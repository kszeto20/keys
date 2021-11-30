#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <termios.h>

#include "parser.h"
#include "exec.h"

struct cInput {char *input; struct cInput * before; struct cInput * after;};

//mode_t mainMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

void enableinputmode() {
    if (isatty(fileno(stdin))) {
        struct termios info;
        tcgetattr(0, &info);
        info.c_lflag &= ~ICANON;
        info.c_lflag |= IEXTEN;
        info.c_lflag &= ~ECHO;
        info.c_cc[VMIN] = 1;
        info.c_cc[VTIME] = 0;
        // info.c_lflag |= PENDIN; // for typeahead?
        tcsetattr(0, TCSANOW, &info);
    }
}

void disableinputmode() {
    if (isatty(fileno(stdin))) {
        struct termios info;
        tcgetattr(0, &info);
        info.c_lflag |= ICANON;
        info.c_lflag &= ~IEXTEN;
        // info.c_lflag &= ~PENDIN;
        tcsetattr(0, TCSANOW, &info);
    }
}

// inserts `c` at `str`, pushing all characters down, up to and including terminating 0
void insertchar(char *str, char c) {
    char oldC;
    do {
        oldC = c;
        char replaced = *str;
        *str = c;
        str++;
        c = replaced;
    } while (oldC);
}

// deletes the character at `str`, pushing all characters to the front, up to and including terminating 0
void delchar(char *str) {
    while (*str) {
        *str = str[1];
        str++;
    }
}

static char * upHandler(char * source, int yesOrNom, int uptoRow) {
  if (yesOrNo) {
    int i;
    char * delim = "\n";
    char * oldCommand = NULL;
    for (i = 0; i < upToRow; i++) {
      oldCommand = strsep(source, delim);
    }
  }
  return oldCommand;
}

void doread() {
  int upTo = 0;
  char * fullHistory = open_and_read();


  enableinputmode();

  char buffer[256]; // should probably be dynamically allocated but meh for now
  char *cursor = buffer;
  *cursor = 0;
  int c;
  while (1) {
    c = getchar();
    switch (c) {
	    case '\e':
	    	if ((c = getchar()) == '[') {
	      	c = getchar();
	        switch (c) {
	        	case 'A': // UP

	          case 'B': // DOWN
	            printf("\nkirsten implement history stuff\n");
	            break;
	          case 'C': // RIGHT
	            if (*cursor) { // don't move right if at terminating 0
	                  cursor++;
	                  printf("\e[C");
	              }
	              break;
	          case 'D': // LEFT
	            if (cursor != buffer) { // don't move left if at beginning of line
	                cursor--;
	              printf("\e[D");
	            }
							break;
          	case '3':
            	if ((c = getchar()) == '~') { // DELETE
            		if (*cursor) {
                	cursor++;
                	printf("\e[C");
                	goto dobackspace; // i got lazy
								}
            	}
          	}
        	}
					break;
      		case 127: // BACKSPACE
        		dobackspace:
        		if (cursor != buffer) { // don't erase if at beginning of line
		          cursor--;
		          delchar(cursor);
		          printf("\e[D");
		          printf("%s ", cursor);
		          unsigned long n = strlen(cursor) + 1;
		          printf("\e[%luD", n);
        	}
        	break;
					case '\n':
	        	goto exitloop; // ill get around to making this cleaner
	      	default:
	        		// printf("%c", c);
		        insertchar(cursor, c);
		        printf("%s", cursor);
		        cursor++;
		        unsigned long n = strlen(cursor);
		        if (n) printf("\e[%luD", n);
		        	break;
		      }
        // printf("\n\"%s\"\n", buffer);
        // printf("\nc: '%c', i: %i\n", c, c);
    		//fflush(stdout);
	}
  exitloop:
  printf("\nREAD LINE: \"%s\"\n", buffer);

  disableinputmode();
}

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

char * getHistFile() {                                   // gets path of home + command file
	//printf("Getting History File Path\n");
	char * toRet = malloc (256 * sizeof(char));           // malloc char[] of path size + extra space + assign to pointer
	strcpy(toRet, getenv("HOME"));                        // copy home path into toRet
	strcat(toRet, "/command_history.txt");                // concat command_history.txt to end of path
	//printf("Finished Getting History File Path\n");
	return toRet;
}

char * open_and_read () {                               // open the file, store all pre-existing data into char[], return char pointer
	char * file = getHistFile();                          // get file path
	//printf("THE FILE NAME IS : %s\n", file);
	int in = open(file, O_RDWR);                          // open the file
	//int in = creat(file, mainMode);
	if (errno != 0) {
    printf("An error has occured \n%s", strerror(errno));
    return NULL;
  }
	struct stat fileS;                                    // get stat struct
	stat(file, &fileS);
	int fileSize = fileS.st_size;                         // get file size

	char * all = malloc(fileSize + 1);                    // malloc file sized char[]
	read(in, all, fileSize);                              // transfer all info from file to all
	//printf("THIS IT THE STRING%s\n", all);
	close(in);
	free(file);
	return all;
}

void write_in (char * toWrite) {            // overwrite file with new command, \n, then all old commands
	char * histFile = getHistFile();          // get path
	//printf("Opening\n");
	//int out = open(histFile, O_RDWR);
	//printf("Opened\n");
	char * orig = open_and_read();            // store pre-existing commands
	int out = open(histFile, O_RDWR);         // open file anew
	//printf("Start reading\n");
	//char * orig = open_and_read();
	//printf("Finished reading\n");
	write(out, toWrite, strlen(toWrite));     // store the "new command first"
	write(out, "\n", 1);                      // add a "\n"
	write(out, orig, strlen(orig));           // concat the previous commands to the end
	free(histFile);
	free(orig);
	close(out);
}

void print_arr(char **arr) {
	int i;
	for (i = 0; arr[i]; i++) {
		printf("%d: \"%s\"\n", i, arr[i]);
	}
}


char **tokenize(char **str) {
	int len = 1; // starts at 1 to ensure space for terminating null
	int cap = 1;
	char **result = malloc(cap * sizeof(char *));
	char *token;

	while (token = strsep(str, " \n")) {

		// ignore empty tokens
		if (*token) {
			// execute what we have so far if semicolon
			if (!strcmp(token, ";")) break;
			if (!strcmp(token, "<")) break;
			// THIS IS ALL BROKEN BUT WE DON'T CARE RN
			if (!strcmp(token, "&&")) {
				result[len-1] = NULL;
				//print_arr(result);
				int returnVal = executeCommand(result);
				free(result);
				// return returnVal;
			}
			if (!strcmp(token, "||")) {
				result[len-1] = NULL;
				//print_arr(result);
				int returnVal = executeCommand(result);
				free(result);
				// return !returnVal;
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
	// executeCommand(result);
	// free(result);
	// return 0;
	return result;
}

void parser_read() {
  // must be set to NULL and 0 to have getline allocate a string
  char *line = NULL;
  unsigned long len = 0;
  // stops at the first space:
//   scanf("%ms", &line);
  //printf("Got here #1\n");
	doread();
  //printf("Got here #2\n");
  getline(&line, &len, stdin);
  printf("Got here #3\n");
//	readline(&line, &len);
	write_in(line);
  printf("Got here #4\n");
  char *linepointer = line;

  int shouldStop = 0;
//   while (!shouldStop && linepointer != NULL) {
//   	shouldStop = parse_command(&linepointer);
//   }
  char ** tokens = tokenize(&line);
  executeCommand(tokens);

  free(tokens);
  free(line);
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
