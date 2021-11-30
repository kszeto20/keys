#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "input.h"

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
        info.c_lflag |= ECHO;
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

int finished = 0;

char *doread() {
    int upTo = 0;
    char * fullHistory = open_and_read();
    char * fHPointer = fullHistory;
    //printf("%s\n", fHPointer);
    while (strsep(&fHPointer, "\n"));
    fHPointer = fullHistory;

    if (finished) return NULL;
  	if (isatty(fileno(stdin)))
        enableinputmode();

    char *buffer = malloc(1024 * sizeof(char));
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
                    // upHandler(fullHistory, upTo);
                    if (fHPointer[0] != 0) {
                      if (upTo != 0) {
                        fHPointer += strlen(fHPointer) + 1;
                      }
                      // wipes over current input with spaces
                      if (cursor > buffer)
                        printf("\e[%luD", cursor - buffer);
                      int i;
                      for (i = 0; buffer[i]; i++) printf(" ");
                      if (i > 0) printf("\e[%dD", i);

                      strcpy(buffer, fHPointer);
                      cursor = buffer + strlen(buffer);
                      printf("%s", buffer);
                      upTo++;
                    }
                    break;
      	          case 'B': // DOWN
                    // printf("\nupTo: %d\n", upTo);
      	            if (upTo == 1) {
                      // wipes over current input with spaces
                      if (cursor > buffer)
                        printf("\e[%luD", cursor - buffer);
                      int i;
                      for (i = 0; buffer[i]; i++) printf(" ");
                      if (i > 0) printf("\e[%dD", i);

                      strcpy(buffer, "");
                      cursor = buffer;
                      upTo--;
                    } else if (upTo > 1) {
                      // wipes over current input with spaces
                      if (cursor > buffer)
                        printf("\e[%luD", cursor - buffer);
                      int i;
                      for (i = 0; buffer[i]; i++) printf(" ");
                      if (i > 0) printf("\e[%dD", i);

                      fHPointer -= 2;
                      while (fHPointer[0] != 0) fHPointer--;
                      fHPointer++;

                      strcpy(buffer, fHPointer);
                      cursor = buffer + strlen(buffer);
                      printf("%s", buffer);
                      upTo--;
                    }
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
        case -1:
            finished = 1;
        case '\n':
            goto exitloop; // ill get around to making this cleaner
        default:
            // printf("%c", c);
            insertchar(cursor, c);
            if (isatty(fileno(stdout))) {
                printf("%s", cursor);
                cursor++;
                unsigned long n = strlen(cursor);
                if (n) printf("\e[%luD", n);
            }
            break;
        }
        // printf("\n\"%s\"\n", buffer);
        // printf("\nc: '%c', i: %i\n", c, c);
        fflush(stdout);
    }
    exitloop:
    // printf("\nREAD LINE: \"%s\"\n", buffer);

  	if (isatty(fileno(stdin)))
        disableinputmode();
    return buffer;
}

// for testing
// int main() {
//     int i;
//     for (i = 0; i < 10; i++) {
//         printf("fakeshell$ ");
//         fflush(stdout);
//         doread();
//     }
//     printf("\n");
// }
