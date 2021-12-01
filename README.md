# KEY$ - A shell created by Kirsten Szeto and Yusuf Elsharawy
## Features
Support for many of bash's basic features, including redirection, piping, combining multiple statements in one line (using `;`, `&&`, and `||`, each with their own behavior), and all at once!
```
KEY$ cd existentdir && echo Success!
KEY$ cd nonexistentdir || echo Failure!
KEY$ cd .; ls -a -l >> test.txt && echo "ls complete!" && cat < test.txt; rm test.txt || exit 1
```
This works by parsing input as a tree (I believe the correct term is AST - Abstract Syntax Tree).  
Lack or excess of whitespace is not a problem; this works too!
```
KEY$ ls -al>>test.txt    &&cat<test.txt
```
You can also use left, right, backspace, and delete to your heart's content to edit your commands as you type them.  
The shell is also careful to not display a prompt or mess with your terminal's settings if started without a terminal, and will exit properly after executing all commands if input ends with EOF.
## Known Issues / Missing Features
Due to originally planning to support them, parentheses and quotation marks do not work as expected. Do not use them - they may cause segmentation faults. This was not fixed due to time constraints.  
Lines of input cannot be longer than 1024 characters. Typing more than this will cause a segmentation fault. This was not fixed due to time constraints.  
Notable issue: if the program unexpectedly stops for any reason (e.g. SEGFAULT, possibly when killed externally) there is a good chance it could mess up your terminal (typed characters won't show). This was not fixed due to time constraints, and because we don't know of a clean way to fix it. The `man` command seems to do something similar as well (ruins scrolling functionality upon SSH disconnection) so I think (and hope) we're excused in this case.  
Inputting a tab character (or any other non-1-width character) to the input messes up the input display. This was not fixed due to time constraints.  

We wanted to implement (but couldn't due to time constraints):
- parentheses, quotation marks, and backslashes being parsed properly
- all `~`'s being replaced with the user's home directory
## Functions
Due to time constraints, I will only be listing ones present in `.h` files (as all others are only helpers to these functions).
### `keys.c`
Loop for the whole program: takes a line of input, parses it, executes it.  
```c
int main();
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