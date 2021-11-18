#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "exec.h"

void executeCommand(char **command) {
    if (!strcmp(command[0], "exit")) exit(0);
    
    int childpid;
    if (childpid = fork()) {
        // PARENT
        wait();
    } else {
        // CHILD
        execvp(command[0], command);
    }
}