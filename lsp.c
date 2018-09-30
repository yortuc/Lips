#include "mpc.h"
#include "editline/readline.h"
#include <stdio.h>

long eval_op(long x, char* op, long y){
    if(strcmp(op, "+")==0) return x + y;
    if(strcmp(op, "-")==0) return x - y;
    if(strcmp(op, "*")==0) return x * y;
    if(strcmp(op, "/")==0) return x / y;
    return 0;
}

long eval(mpc_ast_t* ast){
    if(strstr(ast->tag, "number")){
        return atoi(ast->contents);
    }

    // op is always second child. first is '(' 
    char* op = ast->children[1]->contents;

    // next operand is third child
    char* x = eval(ast->children[2]);

    // iterate the remaining children and combine
    int i = 3;
    while(strstr(ast->children[i]->tag, "expr")){
        x = eval_op(x, op, eval(ast->children[i]));
        i++;
    }
    return x;
}

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

        // parse input
        mpc_result_t result;
        if(mpc_parse("<stdin>", input, Lispy, &result)){
            // get ast from output
            mpc_ast_t* a = result.output; 
            long result = eval(a);
            printf("%li\n", result);
            
            mpc_ast_delete(a);
        } else {
            // otherwise print error
            mpc_err_print(result.error);
            mpc_err_delete(result.error);
        }

        free(input);
    }
        
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}
