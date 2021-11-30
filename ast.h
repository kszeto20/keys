#ifndef AST_H
#define AST_H

enum astnodekind {
    stmt_conj,
    redirection,
    command
};

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

struct astnode * parsetree(char *str);

int evalnode(struct astnode *node);

void print_astnode(struct astnode *node);

void println_astnode(struct astnode *node);

void free_tree(struct astnode *node);

#endif