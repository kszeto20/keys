#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "input.h"
#include "ast.h"

mode_t mainMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

int main() {


  while (1) {
  	if (isatty(fileno(stdin))) {
    	printf("\e[93mKEY$\e[0m ");
      fflush(stdout);
    }
    char *input = doread();
    if (!input) break;
    printf("\n");
    struct astnode *tree = parsetree(input);
    println_astnode(tree); // for debugging
    evalnode(tree);
    free_tree(tree);
    free(input);
  }
  // testing stuff
  // execVarargs(2, "cd", "..");
  // execVarargs(2, "ls", "-l");
  // execVarargs(2, "echo", "Hello World!");
  // execVarargs(1, "exit");
  // execVarargs(3, "echo", "Goodbye", "World!");
}
