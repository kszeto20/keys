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

// allows program to read stdin char by char
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

// returns program to read stdin line by line
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

// returns a string of path to history file (userHomePath/.keys_history)
char * getHistFile() {
	char * toRet = malloc (256 * sizeof(char));
	strcpy(toRet, getenv("HOME"));
  strcat(toRet, "/.keys_history");
	return toRet;
}

// returns a string of all pre-existing content in the history file
char * open_and_read () {
	char * file = getHistFile();
	int in = open(file, O_RDWR);
	if (in == -1) {
        printf("An error has occured \n%s\n", strerror(errno));
        return NULL;
    }

	struct stat fileS;
	stat(file, &fileS);
	int fileSize = fileS.st_size;

	char * all = malloc(fileSize + 1);
	read(in, all, fileSize); // reading from in, to all, fileSize bytes

	close(in);
	free(file);
	return all;

}

// returns nothing; adds new command string to the history file
void write_in (char * toWrite) {
	char * histFile = getHistFile();
	//printf("Opening\n");
	//int out = open(histFile, O_RDWR);
	//printf("Opened\n");
	char * orig = open_and_read();
	int out = open(histFile, O_RDWR);
	//printf("Start reading\n");
	//char * orig = open_and_read();
	//printf("Finished reading\n");
	write(out, toWrite, strlen(toWrite));
	write(out, "\n", 1);
	write(out, orig, strlen(orig));
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

// displays where the use is in their shell before "KEY$" command line prompt
void displayprompt() {
    char *home = getenv("HOME");
    int cwdcap = 16;
    char *cwd = NULL;
    char *ret;
    do {
        cwdcap *= 2;
        free(cwd);
        cwd = malloc(cwdcap * sizeof(char));
        ret = getcwd(cwd, cwdcap-1);
    } while (ret == NULL && errno == ERANGE);
    if (ret == NULL) {
        printf("KEYS: errno %d: %s\n", errno, strerror(errno));
    }

    // set bold, blue
    printf("\e[1m\e[34m");

    int homelen = strlen(home);
    // if `cwd` starts with `home`
    if (!strncmp(home, cwd, homelen)) {
        printf("~%s", cwd + homelen);
    } else {
        printf("%s", cwd);
    }
    //
    printf("\e[0m:\e[93mKEY$\e[0m ");
    fflush(stdout);
    free(cwd);
}

int finished = 0;

/*
takes command line input, parses input character by character by character
accounts for arrow keys, backspace, and delete
*/

char * doread() {
  int upTo = 0; // counter for how many lines up in the history log file the user is
  char * fullHistory = open_and_read(); // gets the existing history for use by up/down arrow keys
  char * fHPointer = fullHistory;
  while (strsep(&fHPointer, "\n")); // replaces '\n' with 0 so when reading a line, pointer will stop at the end of each row
  fHPointer = fullHistory; // returns fHPointer to lastest history entry

  if (finished) return NULL;
	if (isatty(fileno(stdin))) {
      displayprompt();
      enableinputmode();
  }

  char *buffer = malloc(2048 * sizeof(char));
  char *cursor = buffer;
  *cursor = 0;
  int c;
  while (1) {
    c = getchar();
    switch (c) {
    // ignored:
    case '\t':
      break;
    // escape sequences (handles arrow keys, ignores all else)
    case '\e':
      if ((c = getchar()) == '[') {
        c = getchar();
          switch (c) {
      	   	case 'A': // UP
              if (fHPointer[0] != 0) {
                if (upTo != 0) {
                  fHPointer += strlen(fHPointer) + 1; // read to the terminating 0 then add one to get to next line
                }
                // wipes over current input with spaces
                if (cursor > buffer)
                  printf("\e[%luD", cursor - buffer);
                printf("\e[K");
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
                  printf("\e[K");

                  strcpy(buffer, "");
                  cursor = buffer;
                  upTo--;
                } else if (upTo > 1) {
                  // wipes over current input with spaces
                  if (cursor > buffer)
                    printf("\e[%luD", cursor - buffer);
                  printf("\e[K");

                  fHPointer -= 2;
                  while (fHPointer[0] != 0) fHPointer--; // loops backwards to the previous 0
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
                        goto dobackspace;
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
    case EOF:
        finished = 1;
    case '\n':
        if (isatty(fileno(stdin))) printf("\n");
        goto exitloop; // ill get around to making this cleaner
    default:
        // printf("%c", c);
        insertchar(cursor, c);
        cursor++;
        if (isatty(fileno(stdin))) {
            printf("%s", cursor - 1);
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
