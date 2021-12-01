#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "ast.h"
#include "input.h"

mode_t mainMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

int main() {

  // creates history file
  char * histfile = getHistFile();
  int fd = open(histfile, O_CREAT, 0666);
  close(fd);

  while (1) {
    char *input = doread(); // returns line of commands
    if (!input) break;
    if (*input) write_in(input); // adding to history
    struct astnode *tree = parsetree(input);
    // println_astnode(tree); // for debugging
    evalnode(tree);
    free_tree(tree);
    free(input);
  }

}
