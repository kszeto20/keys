#include <stdio.h>
#include "parser.h"
#include "exec.h"

#include "parser.h"
#include "exec.h"

int main() {
  printf("Good News Everyone\n");

  while (1) {
    printf("KEY$ ");
    parser_read();
  }
  
  // execVarargs(2, "cd", "..");
  // execVarargs(2, "ls", "-l");
  // execVarargs(2, "echo", "Hello World!");
  // execVarargs(1, "exit");
  // execVarargs(3, "echo", "Goodbye", "World!");
}
