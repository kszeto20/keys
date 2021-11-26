#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>

void enableinputmode();

void disableinputmode();

void parser_read();

char * open_and_read();

int parse_command(char **str);

#endif
