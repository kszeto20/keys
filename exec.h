#ifndef EXEC_H
#define EXEC_H

// struct wholecommand {
//     char **command;
//     int redirectStdinTo;
//     int redirectStdoutTo;
//     int redirectStderrTo;
// };

int executeCommand(char **command);

void executeTokens(char **tokens);

int execVarargs(int n, ...);

#endif