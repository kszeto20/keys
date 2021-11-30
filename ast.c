#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "exec.h"
#include "ast.h"

#ifdef UNUSED
char **make_arr(int n, ...) {
    char **args = malloc(n * sizeof(char *));
    
    va_list varargs;
    va_start(varargs, n);
    
    int i;
    for (i = 0; i < n; i++) {
        args[i] = va_arg(varargs, char *);
    }
    
    va_end(varargs);
    
    return args;
}

char **make_arr_NULL(char *first, ...) {
    char **args;
    if (!first) {
        args = malloc(sizeof(char *));
        args[0] = NULL;
        return args;
    }
    
    va_list varargs;
    va_start(varargs, first);
    int n;
    for (n = 2; va_arg(varargs, char *); n++);
    va_start(varargs, first);
    args = malloc(n * sizeof(char *));
    args[0] = first;
    int i;
    for (i = 1; i < n; i++) {
        args[i] = va_arg(varargs, char *);
    }
    
    va_end(varargs);
    
    return args;
}

void print_arr(char **arr) {
	int i;
	for (i = 0; arr[i]; i++) {
		printf("%d: \"%s\"\n", i, arr[i]);
	}
}

char *dyncatarr(char **arr) {
    char *result;
    int len = 0;
    int i;
    for (i = 0; arr[i]; i++) len += strlen(arr[i]) + 1;
    if (!len) len = 1;
    
    result = malloc(len * sizeof(char) + 1);
    result[0] = 0;
    for (i = 0; arr[i]; i++) {
        strcat(result, arr[i]);
        strcat(result, " ");
    }
    result[len-1] = 0;
    return result;
}

// will remove potential outermost parens in `str` only if surrounded by whitespace
// TODO: FIX THIS FUNCTION: TRYING TO STRIP "(A) (B)" DOES NOT WORK IN ANY WAY
void trystripparens(char *str) {
    char *lParen = strchr(str, '(');
    char *rParen = strchr(str, ')');
    
    // only if both are found (assumes rParen is after lParen, since `str` is assumed to be balanced)
    if (lParen && rParen) {
        int shouldRemove = 1;
        char *cur;
        // checks whitespace before lParen
        for (cur = str; cur < lParen; cur++)
            shouldRemove = shouldRemove && strchr(" \t", *str);
        // checks whitespace after rParen
        for (cur = rParen + 1; *cur; cur++)
            shouldRemove = shouldRemove && strchr(" \t", *str);
        
        if (shouldRemove) {
            *lParen = ' ';
            *rParen = ' ';
        }
    }
}

#endif

void print_astnode(struct astnode *node) {
    if (!node) {
        printf("NULL");
        return;
    }
    
    switch (node->kind) {
    case stmt_conj:
        printf("STMT_CONJ(");
        print_astnode(node->data.stmt_conj_data.statements[0]);
        printf(", \"%s\", ", node->data.stmt_conj_data.connector);
        print_astnode(node->data.stmt_conj_data.statements[1]);
        printf(")");
        break;
    case redirection:
        printf("REDIRECTION(%d, 0x%x, \"%s\", ",
            node->data.redirection_data.redirfd, node->data.redirection_data.flags, node->data.redirection_data.file_loc);
        print_astnode(node->data.redirection_data.command_node);
        printf(")");
        break;
    case command:;
        char **tokens = node->data.command_data;
        printf("COMMAND(");
        if (tokens[0]) {
            printf("\"%s\"", tokens[0]);
            int i;
            for (i = 1; tokens[i]; i++) {
                printf(", \"%s\"", tokens[i]);
            }
        }
        printf(")");
        break;
    default:
        printf("ERRORTYPE(%d)", node->kind);
        break;
    }
}

// frees a whole tree, including its children and any parts in heap memory
void free_tree(struct astnode *node) {
    if (!node) return;
    
    switch (node->kind) {
    case stmt_conj:
        free_tree(node->data.stmt_conj_data.statements[0]);
        free_tree(node->data.stmt_conj_data.statements[1]);
        break;
    case redirection:
        free_tree(node->data.redirection_data.command_node);
        free(node->data.redirection_data.file_loc);
        break;
    case command:
        free(node->data.command_data);
        break;
    }
    free(node);
}

void println_astnode(struct astnode *node) {
    print_astnode(node);
    printf("\n");
}

int ret_zero(int x) { return 0; }
int ret_same(int x) { return x; }
int ret_not(int x) { return !x; }

typedef int (*unary_int_func)(int);

int evalnode(struct astnode *node) {
    if (!node) {
        printf("CANNOT EXECUTE NULL NODE\n");
        return -1;
    }
    
    int result = -1;
    switch (node->kind) {
    case stmt_conj:;
        char *connector = node->data.stmt_conj_data.connector;
        unary_int_func f;
        if (!strcmp(connector, ";")) f = ret_zero;
        else if (!strcmp(connector, "&&")) f = ret_same;
        else if (!strcmp(connector, "||")) f = ret_not;
        else if (!strcmp(connector, "|")) {
            // special case for piping
            if (node->data.stmt_conj_data.statements[0]->kind != command
                || node->data.stmt_conj_data.statements[1]->kind != command) {
                // for the sake of my sanity
                printf("KEYS: piping is only supported from one raw command to another, sorry!\n");
                return -1;
            }
            // start both one after the other with piping set up, wait for both to finish
            int fds[2]; // {read, write}
            pipe(fds);
            
            // redirect stdout for first process
            int origstdout = dup(1);
            dup2(fds[1], 1);
            close(fds[1]);
            int pid0 = startCommand(node->data.stmt_conj_data.statements[0]->data.command_data);
            dup2(origstdout, 1);
            close(origstdout);
            
            // redirect stdin for second process
            int origstdin = dup(0);
            dup2(fds[0], 0);
            close(fds[0]);
            int pid1 = startCommand(node->data.stmt_conj_data.statements[1]->data.command_data);
            dup2(origstdin, 0);
            close(origstdin);
            
            int result = 0;
            
            result |= waitfor(pid0);
            
            result |= waitfor(pid1);
            return result; 
        }
        else {
            printf("INVALID CONNECTOR \"%s\"\n", connector);
            return -1;
        }
        struct astnode **statements = node->data.stmt_conj_data.statements;
        result = evalnode(statements[0]);
        if (!f(result)) result = evalnode(statements[1]);
        break;
    case command:
        // char *command = dyncatarr(node->data.command_data);
        // result = system(command);
        // printf("exitcode: %d\n", result);
        // free(command);
        result = executeCommand(node->data.command_data);
        break;
    case redirection:;
        // store copy
        int fd = node->data.redirection_data.redirfd;
        int originalcopy = dup(fd);
        // replace fd
        int opened = open(node->data.redirection_data.file_loc, node->data.redirection_data.flags, 0666); // mode will be ignored if not creating anyway
        dup2(opened, fd);
        close(opened); // closes extra to same file
        // at time of running: fd is replaced with opened file, no other new fd's besides originalcopy
        result = evalnode(node->data.redirection_data.command_node);
        // switch back
        dup2(originalcopy, fd);
        close(originalcopy); // no longer needed
        break;
    default:
        printf("INVALID NODE KIND: %d\n", node->kind);
        return -1;
        break;
    }
    return result;
}

// returns 0 if unbalanced, anything else otherwise
int balanced(char *str) {
    int level = 0;
    int i;
    for (i = 0; str[i]; i++) {
        switch (str[i]) {
        case '(':
            level++;
            break;
        case ')':
            level--;
            if (level < 0) return -1;
            break;
        case '"':
            // moves i to next quote character not after a backslash:
            while (str[++i] != '"') {
                while (str[i] == '\\') i++;
            }
            break;
        }
    }
    // level == 0 -> balanced
    return !level;
}

// returns pointer to first top-level `needle`
// returns NULL on failure
char * findtoplevel(char *haystack, char *needle) {
    int needlelen = strlen(needle);
    int level = 0;
    int i;
    for (i = 0; haystack[i]; i++) {
        switch (haystack[i]) {
        case '(':
            level++;
            break;
        case ')':
            level--;
            break;
        case '"':
            // moves i to next quote character not after a backslash:
            while (haystack[++i] != '"') {
                // intentional behavior: the input `"\\"` (quote, BS, BS, quote) should stop on the second quote
                while (haystack[i] == '\\') i++;
            }
            break;
        case '\\':
            i++;
            break;
        default:
            // must be at top level (level == 0) to match
            if (!level && !strncmp(haystack + i, needle, needlelen)) {
                return haystack + i;
            }
        }
    }
    return NULL;
}

char *stmt_conj_connectors[] = {";","&&","||","|",NULL};

char *redirection_connectors[] = {">>",">","<",NULL};

// returns the needle matched, NULL if none found
// if one is found, `*loc` is set to its location
// priority is in order of location in haystack, then by order in `needles`
char * findfirsttoplevel(char *str, char **needles, char **loc) {
    // int numneedles;
    // for (numneedles = 0; needles[numneedles]; numneedles++);
    int level = 0;
    int i;
    for (i = 0; str[i]; i++) {
        switch (str[i]) {
        case '(':
            level++;
            break;
        case ')':
            level--;
            break;
        case '"':
            // moves i to next quote character not after a backslash:
            while (str[++i] != '"') {
                // intentional behavior: the input `"\\"` (quote, BS, BS, quote) should stop on the second quote
                while (str[i] == '\\') i++;
            }
            break;
        case '\\':
            i++;
            break;
        default:
            // must be at top level (level == 0) to match
            if (!level) {
                // printf("%s\n", str + i);
                int j;
                for (j = 0; needles[j]; j++) {
                    char *needle = needles[j];
                    if (!strncmp(str+i, needle, strlen(needle))) {
                        *loc = str + i;
                        return needle;
                    }
                }
            }
        }
    }
    return NULL;
}

// TODO: DOES NOT WORK
// returns the needle matched, NULL if none found
// if one is found, `*loc` is set to its location
// priority is in reverse order of location in haystack by the end of the needle, then by order in `needles`
char * findlasttoplevel(char *str, char **needles, char **loc) {
    // int numneedles;
    // for (numneedles = 0; needles[numneedles]; numneedles++);
    int level = 0;
    int i;
    for (i = strlen(str); i >= 0; i--) {
        switch (str[i]) {
        case '(':
            level++;
            break;
        case ')':
            level--;
            break;
        case '"':
            // moves i to next quote character not after a backslash:
            while (str[++i] != '"') {
                // intentional behavior: the input `"\\"` (quote, BS, BS, quote) should stop on the second quote
                while (str[i] == '\\') i++;
            }
            break;
        case '\\':
            i++;
            break;
        default:
            // must be at top level (level == 0) to match
            if (!level) {
                // printf("%s\n", str + i);
                int j;
                for (j = 0; needles[j]; j++) {
                    char *needle = needles[j];
                    int l = strlen(needle);
                    if (i >= l && !strncmp(str+i-l, needle, l)) {
                        *loc = str + i - l;
                        return needle;
                    }
                }
            }
        }
    }
    return NULL;
}

// because Linux doesn't include this in string.h apparently??
void strrev(char *str) {
    int j = strlen(str)-1;
    int i;
    for (i = 0; i < j; i++, j--) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

// returns a whitespace-stripped dynamically-allocated copy of `str`
char *strip(char *str) {
    // `strspn` gives the length at the beginning of `str`
    // `strrev` reverses a string
    int len = strlen(str);
    int front = strspn(str, " \t\n");
    strrev(str);
    int back = strspn(str, " \t\n");
    strrev(str);
    int newlen = len - front - back;
    
    char *result = malloc((newlen + 1) * sizeof(char));
    strncpy(result, str + front, newlen);
    return result;
}

char **tokenize(char *str) {
	int len = 1; // starts at 1 to ensure space for terminating null
	int cap = 1;
	char **result = malloc(cap * sizeof(char *));
	char *token;
	
	while (token = strsep(&str, " \n")) {
		// ignore empty tokens
		if (*token) {
			
			result[len-1] = token;
			// resize if reached end of array
			if (len == cap) {
				cap *= 2;
				result = realloc(result, cap * sizeof(char *));
			}
			len++;
		}
	}
	result[len-1] = NULL;
	return result;
}

// parses str into a `astnode`
// warning: `str` will be desecrated
struct astnode * parsetree(char *str) {
    // printf("PARSING: \"%s\"\n", str);
    if (!balanced(str)) {
        printf("KEYS: parsing error: \"%s\" is not balanced\n", str);
        return NULL;
    }
    // trystripparens(str); // broken
    struct astnode *result = malloc(sizeof(struct astnode));
    // try parsing as stmt_conj
    char *conat;
    char *matched;
    // semicolon takes priority
    if (!((conat = findtoplevel(str, ";")) && (matched = ";")))
        matched = findlasttoplevel(str, stmt_conj_connectors, &conat);
    if (matched) {
        result->kind = stmt_conj;
        // replace connector with 0's
        int len = strlen(matched);
        int i;
        for (i = 0; i < len; i++) conat[i] = 0;
        
        result->data.stmt_conj_data.connector = matched;
        result->data.stmt_conj_data.statements[0] = parsetree(str);
        result->data.stmt_conj_data.statements[1] = parsetree(conat+len);
        return result;
    }
    matched = findlasttoplevel(str, redirection_connectors, &conat);
    if (matched) {
        result->kind = redirection;
        // replace connector with 0's
        int len = strlen(matched);
        int i;
        for (i = 0; i < len; i++) conat[i] = 0;
        
        result->data.redirection_data.command_node = parsetree(str);
        result->data.redirection_data.file_loc = strip(conat + len);
        switch (matched[0]) {
        case '<':
            result->data.redirection_data.redirfd = 0;
            result->data.redirection_data.flags = O_RDONLY;
            break;
        case '>':
            result->data.redirection_data.redirfd = 1;
            result->data.redirection_data.flags = O_WRONLY | O_CREAT;
            if (len == 2) { // ">>"
                result->data.redirection_data.flags |= O_APPEND;
            }
            break;
        }
        return result;
    }
    // parse as command
    result->kind = command;
    result->data.command_data = tokenize(str);
    return result;
}

// for testing
// int main() {
//     // print_arr(make_arr_NULL("cd", "..", NULL));
//     char input[128] = "cd .; ls -a -l >> test.txt && echo ls complete! && cat < test.txt; rm test.txt; echo piping too!; ls | cat";
//     // char input[128] = "ls | cat";
    
//     printf("a\n");
//     struct astnode *my_node;
//     my_node = parsetree(input);
//     printf("b\n");
//     /*
//     my_node.kind = stmt_conj;
//     my_node.data.stmt_conj_data.connector = ";";
//     my_node.data.stmt_conj_data.statements[0] = &(struct astnode){
//         .kind = command,
//         .data.command_data = make_arr_NULL("cd", "..", NULL),
//     };
//     my_node.data.stmt_conj_data.statements[1] = &(struct astnode){
//         .kind = command,
//         .data.command_data = make_arr_NULL("ls", "-a", "-l", NULL),
//     };
//     */
//     println_astnode(my_node);
//     printf("final result: %d\n", evalnode(my_node));
//     free_tree(my_node);
// }