#ifndef EXEC_H
#define EXEC_H

int startCommand(char **command);

int waitfor(int pid);

int executeCommand(char **command);

#ifdef UNUSED
void executeTokens(char **tokens);

int execVarargs(int n, ...);
#endif

#endif