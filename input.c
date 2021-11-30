#include <stdlib.h>
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

int finished = 0;

char *doread() {
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
                case 'B': // DOWN
                    // printf("\nkirsten implement history stuff\n");
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