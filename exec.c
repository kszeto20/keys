#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#include "exec.h"

void executeCommand(char **command) {
    if (!command[0]) return;
    if (!strcmp(command[0], "exit")) exit(0);
    if (!strcmp(command[0], "cd")) {
        if (!command[1]) command[1] = getenv("HOME");
        chdir(command[1]);
    }
    
    int childpid;
    if (childpid = fork()) {
        // PARENT
        int wstatus;
        wait(&wstatus);
    } else {
        // CHILD
        execvp(command[0], command);
    }
}

void execVarargs(int n, ...) {
    char **args = malloc(n * sizeof(char *));
    
    va_list varargs;
    va_start(varargs, n);
    
    int i;
    for (i = 0; i < n; i++) {
        args[i] = va_arg(varargs, char *);
    }
    
    va_end(varargs);
    
    executeCommand(args);
    
    free(args);
}
