#include <stdio.h>
#include <unistd.h>

#include "parser.h"
#include "exec.h"

#include "parser.h"
#include "exec.h"

int main() {
  printf("Good News Everyone\n");

  while (1) {
  	if (isatty(fileno(stdin)))
    	printf("\e[93mKEY$\e[0m ");
    parser_read();
  }
  
  // execVarargs(2, "cd", "..");
  // execVarargs(2, "ls", "-l");
  // execVarargs(2, "echo", "Hello World!");
  // execVarargs(1, "exit");
  // execVarargs(3, "echo", "Goodbye", "World!");
}

