#include <stdio.h>
#include "parser.h"
#include "exec.h"
// #include <readline/history.h> // wont compile when I include this (looked up error -> makefile flags?)


int main() {
  printf("Good News Everyone\n");
  //using_history();
  while (1) {
    char c = '\0';
    printf("KEY$ ");
    /* // supposed to be constantly listening but I'm not sure if
       // it will not get to line 18 if put into a while(1) loop
    int listEng = 0; // did listen trigger boolean
    while (1) {
      listEng = listener();
    }
    */
    //if (listEng == 0) { // if listEng did not get triggered, wait for the enter
      parser_read();
    //}
  }

  // execVarargs(2, "cd", "..");
  // execVarargs(2, "ls", "-l");
  // execVarargs(2, "echo", "Hello World!");
  // execVarargs(1, "exit");
  // execVarargs(3, "echo", "Goodbye", "World!");
}
/*
//  HIST_ENTRY * previous_history (void)
int listener() {
  if (kbhit()) {
    int first = getch();
    if (first == 0 || first == 224) {
      if (getch() == 224) {
        char *prev = "!!";
        parse_command(history_tokenize(prev));
        return 1;
      }
    }
  }
  return 0;
}
*/
