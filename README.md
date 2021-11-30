# KEY$ - A shell created by Kirsten Szeto and Yusuf Elsharawy
## Features
Support for many of bash's basic features, including redirection, piping, combining multiple statements in one line (using `;`, `&&`, and `||`, each with their own behavior), and all at once!
```
KEY$ cd existentdir && echo Success!
KEY$ cd nonexistentdir || echo Failure!
KEY$ cd .; ls -a -l >> test.txt && echo "ls complete!" && cat < test.txt; rm test.txt || exit 1
```
This works by parsing input as a tree (I believe the correct term is AST - Abstract Syntax Tree).  
You can also use left, right, backspace, and delete to your heart's content to edit your commands as you type them.  
The shell is also careful to not display a prompt or mess with your terminal's settings if started without a terminal, and will exit properly after executing all commands if input ends with EOF.
## Known Issues / Missing Features
I implemented `&&` and `||` in such a way that `a && b || c` gets parsed as `a && (b || c)` instead of `(a && b) || c` by accident, causing some useful bash idioms to be impossible. This was not fixed due to time constraints.  
Due to originally planning to support them, parentheses and quotation marks do not work as expected. Do not use them - they may cause segmentation faults. This was not fixed due to time constraints.  
Lines of input cannot be longer than 1024 characters. Typing more than this will cause a segmentation fault. This was not fixed due to time constraints.  
A lot of work was put into implementing history (pressing up & down to choose previous commands). However, it was not completed due to time constraints.
We wanted a more useful prompt than just `KEY$` (showing username, current working directory, etc). This was not done due to time constraints.
Inputting a tab character (or any other non-1-width character) to the input messes up the input display. This was not fixed due to time constraints.
## Functions
Due to time constraints, I will only be listing ones present in `.h` files (as all others are only helpers to these functions).
### `keys.c`
Loop for the whole program: takes a line of input, parses it, executes it.  
```c
main();
```
### `exec.c`
Starts a command & returns the pid of the child process started. This function always starts a child process no matter the command (`cd` and `exit` create "dummy" child processes).
```c
int startCommand(char **command);
```
Waits for a child to finish, given its pid, and returns its exit status.
```c
int waitfor(int pid);
```
Effectively does the same as `waitfor(startCommand(command))` (runs a command and returns its exit status), but does not start dummy processes.
```c
int executeCommand(char **command);
```
### `input.c`
Reads a line of input, accounting for arrow keys, backspace, and delete. Returns a dynamically-allocated string containing the input. Currently a set 1024 bytes are allocated.
```c
char * doread();
```
### `ast.c`
(I know these are not functions, but they are worth mentioning.)  
Represents the kind of an AST node.
```c
enum astnodekind {
    stmt_conj,
    redirection,
    command
};
```
Struct containing the data of an AST node. Only a pointer to this type is really used.
```c
struct astnode {
    enum astnodekind kind;
    union astnodedata {
        struct {
            char *connector;
            struct astnode *statements[2];
            // ^ other stmt_conj's, redirections, or commands
        } stmt_conj_data;
        struct {
            int redirfd; // stream to be redirected
            int flags; // flags to open file with
            char *file_loc; // path to file
            struct astnode *command_node; // should have kind = command, but should also work with others?
        } redirection_data;
        char **command_data;
    } data;
};
```
Parses a KEYS command into an AST node, which is returned.
```c
struct astnode * parsetree(char *str);
```
Evaluates the AST and returns the exit code.
```c
int evalnode(struct astnode *node);
```
Prints the AST in a user-friendly way (unused, but for debugging).
```c
void print_astnode(struct astnode *node);
```
Does the same as `print_astnode(node); printf("\n");`
```c
void println_astnode(struct astnode *node);
```
Frees an entire tree, its children, and all dynamically-allocated parts from memory.
```c
void free_tree(struct astnode *node);
```