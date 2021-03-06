# KEY$ - A shell created by Kirsten Szeto and Yusuf Elsharawy
## Features
Support for many of bash's basic features, including redirection, piping, combining multiple statements in one line (using `;`, `&&`, and `||`, each with their own behavior), and all at once!
```
KEY$ cd existentdir && echo Success!
KEY$ cd nonexistentdir || echo Failure!
KEY$ cd mightexist && echo Success! || echo Failure!; echo Done either way!
KEY$ cat<original.txt>copy.txt
KEY$ cd ..; ls -a -l >> test.txt && echo ls complete! && cat < test.txt; rm test.txt || exit 1
```
This works by parsing input as a tree (I believe the correct term is AST - Abstract Syntax Tree). This allows for the most flexibility when it comes to commands!  
Lack or excess of whitespace is not a problem; this works too!
```
KEY$ ls -al>>test.txt    &&cat<test.txt
```
Note that the order of operations is equivalent (or at least very similar to) bash. In order of operator precedence, we support:
- `|` - piping, from one command to another
- `&&`, `||` - "and" and "or", to execute the next command only if the previous succeeds/fails respectively
- `;` - executes the next command after the other, regardless of its exit status
- `<`, `>`, `>>` - redirection to stdin / from stdout / from stdout with appending, respectively  

Operators of the same precedence have left-to-right associativity (i.e. `a && b || c` -> `(a && b) || c` and `a || b && c` -> `(a || b) && c`). Although this applies for redirection as well, it has no noticeable effect. (Unless you redirect the same stream twice, but why would you do that?)

You can also use left, right, backspace, and delete to your heart's content to edit your commands as you type them.  
In the case that the shell starts with input that is not a terminal (e.g. redirected input from file), the shell is also careful to:  
- not display a prompt  
- not mess with your terminal's settings  
- exit properly after executing all commands if input ends with `EOF`  

(Note: Issues may still occur if you have arrow key sequences, backspace, or delete in your redirected input. But who would do that?)

We also implemented **history!** You can press the up and down arrow keys to go forwards and backwards through previous commands. Unlike bash, your original command is not stored, but similar to bash, your history is stored in a file, `~/.keys_history`, so that it persists between sessions.  

As little bonus features:  
- Similar to bash, `exit [exitcode]` can be used to exit the shell with a particular exitcode.
- Although using `~` in a command isn't implemented, you can still use `cd` command (with no arguments) to go to your home directory.
- The prompt shows your current directory, and appropriately shortens it when it's a subdirectory of your home directory.
## Known Limtations & Issues / Missing Features
Although we originally planned to support them, parentheses, quotation marks, and backslashes do not work as expected. They should not cause any segmentation faults, but they will be treated as regular characters. This was not implemented due to time constraints.  
Lines of input cannot be longer than 2048 characters. Typing more than this will cause a segmentation fault. This was not fixed due to time constraints. (It would be a bit frustrating to make reallocation play nicely with selecting from history, and I personally don't think you should be typing 2 KB of text on one line anyway.)  
Notable issue: if the program unexpectedly stops for any reason (e.g. SEGFAULT, possibly when killed externally) there is a good chance it could mess up your terminal (typed characters won't show). This was not fixed due to time constraints, and because we don't know of a clean way to fix it. The `man` command seems to do something similar as well (ruins scrolling functionality upon SSH disconnection) so I think (and hope) we're excused in this case.  
Inputting any non-1-width character to the input messes up the input display. `TAB` is the only exclusion to this, as we explicitly excluded all tab characters from the input. This was not fixed due to time constraints.  
Piping has one limitation: it can only be done from one raw command to another. This means you cannot pipe and redirect at the same time, or pipe more than 2 commands together. We had ideas for how to fix this issue, which we did not get around to implementing due to time constraints. Thankfully, because we made `|` have a higher precedence than `&&` and `||`, which in turn have a higher precedence than `;`, so it should still work as expected with those conjunctions (e.g. `cd .. && ls | cat && echo success! || echo failed!; echo done!`);

We also wanted to implement (but couldn't due to time constraints):
- parentheses, quotation marks, and backslashes being parsed properly
- all `~`'s being replaced with the user's home directory
- using `<<` to redirect from a string of text to the stdin of a command (would work by writing the string to a temporary file, then redirecting as usual from that file)
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
Takes in a char pointer to the new command to add to history. Opens the file, adds the string to the file, a "\n" character, and then the preexisting history to the end of the file
```c
void write_in (char * toWrite);
```
Returns pointer to string of homepath/historyfile
```c
char * getHistFile();
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
