#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#include "exec.h"

// returns pid of child
int startdummychild(int exitcode) {
    int childpid;
    if (childpid = fork()) {
        // PARENT
        return childpid;
    }
    // CHILD
    exit(exitcode);
}


// returns pid of child
int startCommand(char **command) {
    if (!command[0]) return 0;
    if (!strcmp(command[0], "exit")) {
        int exitval = 0;
        if (command[1]) {
            if (command[2]) {
                printf("KEYS: exit: too many arguments\n");
                return startdummychild(-1);
            }
            exitval = atoi(command[1]);
        }
        exit(exitval);
    }
    if (!strcmp(command[0], "cd")) {
        if (command[1] && command[2]) {
            printf("KEYS: cd: too many arguments\n");
            return startdummychild(-1);
        }
        if (!command[1]) command[1] = getenv("HOME");
        if (chdir(command[1])) {
            printf("KEYS: cd: %s: %m\n", command[1]);
            return startdummychild(-1);
        }
        return startdummychild(0);
    }

    int childpid;
    if (childpid = fork()) {
        // PARENT
        return childpid;
        // int wstatus;
        // wait(&wstatus);
        // printf("\e[91mDEBUG:\e[0m wstatus: %x\n", wstatus);
        // return WEXITSTATUS(wstatus);
    } else {
        // CHILD
        execvp(command[0], command);
        printf("%s: command not found\n", command[0]);
        exit(-1);
    }
}

int waitfor(int pid) {
    int wstatus;
    waitpid(pid, &wstatus, 0);
    return WEXITSTATUS(wstatus);
}

/*
excutes all the commands in the list of tokens
returns WEXITSTATUS upon parent processing or exits the program upon child execution
*/
int executeCommand(char **command) {

    if (!command[0]) return 0;
    if (!strcmp(command[0], "exit")) {
        int exitval = 0;
        if (command[1]) {
            if (command[2]) {
                printf("KEYS: exit: too many arguments\n");
                return -1;
            }
            exitval = atoi(command[1]);
        }
        exit(exitval);
    }
    if (!strcmp(command[0], "cd")) {
        if (command[1] && command[2]) {
            printf("KEYS: cd: too many arguments\n");
            return -1;
        }
        if (!command[1]) command[1] = getenv("HOME");
        if (chdir(command[1])) {
            printf("KEYS: cd: %s: %m\n", command[1]);
            return -1;
        }
        return 0;
    }

    int childpid;
    if (childpid = fork()) {
        // PARENT
        int wstatus;
        wait(&wstatus);
        // printf("\e[91mDEBUG:\e[0m wstatus: %x\n", wstatus);
        return WEXITSTATUS(wstatus);
    } else {
        // CHILD
        execvp(command[0], command);
        printf("%s: command not found\n", command[0]);
        exit(-1);
    }

}

/*
tokenizes varargs and calls executeCommand to execute commands
returns executeComand return value upon completion
*/
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
