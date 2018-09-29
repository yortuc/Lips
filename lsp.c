#include "mpc.h"
#include "editline/readline.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT, 
        "                                                     \
            number   : /-?[0-9]+/ ;                             \
            operator : '+' | '-' | '*' | '/' ;                  \
            expr     : <number> | '(' <operator> <expr>+ ')' ;  \
            lispy    : /^/ <operator> <expr>+ /$/ ;             \
        ",
        Number, Operator, Expr, Lispy);    puts("Lips v0.0.1");

    while(1) {
        char* input = readline("Lips> ");
        add_history(input);

        printf("Lips> %s\n", input);
        free(input);
    }
        
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}