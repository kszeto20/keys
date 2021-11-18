#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#include "exec.h"

int executeCommand(char **command) {
    if (!command[0]) return 0;
    if (!strcmp(command[0], "exit")) {
        if (command[2]) {
            printf("KEYS: exit: too many arguments\n");
            return -1;
        }
        int exitval = 0;
        if (command[1]) {
            exitval = atoi(command[1]);
        }
        exit(exitval);
    }
    if (!strcmp(command[0], "cd")) {
        if (command[2]) {
            printf("KEYS: cd: too many arguments\n");
            return -1;
        }
        if (!command[1]) command[1] = getenv("HOME");
        if (chdir(command[1])) {
            printf("KEYS: cd: %s: %m\n", command[1]);
        }
        return -1;
    }
    
    int childpid;
    if (childpid = fork()) {
        // PARENT
        int wstatus;
        wait(&wstatus);
        printf("DEBUG: wstatus: %x\n", wstatus);
        return WEXITSTATUS(wstatus);
    } else {
        // CHILD
        execvp(command[0], command);
    }
}

int execVarargs(int n, ...) {
    char **args = malloc(n * sizeof(char *));
    
    va_list varargs;
    va_start(varargs, n);
    
    int i;
    for (i = 0; i < n; i++) {
        args[i] = va_arg(varargs, char *);
    }
    
    va_end(varargs);
    
    int exitcode = executeCommand(args);
    
    free(args);
    
    return exitcode;
}
