#include <stdio.h>
#include "parser.h"
#include "exec.h"

int main() {
  printf("Good News Everyone\n");

  while (1) {
    printf("KEY$ ");
    parser_read();
  }




}
