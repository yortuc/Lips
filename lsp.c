#include "mpc.h"
#include "editline/readline.h"
#include <stdio.h>

// enum for lval (lisp evaluation result) types
enum { LVAL_NUM, LVAL_ERR };

// enum for errors
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

typedef struct {
    int type;
    long num;
    int err;
} lval;

// create a number type lval
lval lval_num(long x){
    lval v;
    v.num = x;
    v.type = LVAL_NUM;
    return v;
}

// create an error type lval
lval lval_err(int error_type){
    lval v;
    v.type = LVAL_ERR;
    v.err = error_type;
    return v;
}

void lval_print(lval v){
    switch(v.type){
        case LVAL_NUM:
            printf("%li", v.num);
            break;
        
        case LVAL_ERR:
            // check what type of error we have
            if(v.err == LERR_DIV_ZERO){
                printf("Error: Division by zero.");
            }
            if(v.err == LERR_BAD_OP){
                printf("Error: Invalid operator.");
            }
            if(v.err == LERR_BAD_NUM){
                printf("Errror: Invalid number.");
            }
            break;
    }
}

// print lval with a new line at the end
void lval_println(lval v){ lval_print(v); putchar('\n'); }

lval eval_op(lval x, char* op, lval y){
    // if one of the operands is error, return it
    if(x.type == LVAL_ERR) { return x; }
    if(y.type == LVAL_ERR) { return y; }

    if(strcmp(op, "+")==0) { return lval_num(x.num + y.num); }
    if(strcmp(op, "-")==0) { return lval_num(x.num - y.num); }
    if(strcmp(op, "*")==0) { return lval_num(x.num * y.num); }
    if(strcmp(op, "/")==0) {
        return y.num == 0 ? 
            lval_err(LERR_DIV_ZERO) : 
            lval_num(x.num / y.num);
    }
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* ast){
    if(strstr(ast->tag, "number")){
        //check if there is an error with conversion from string to number
        errno = 0; 
        long x = strtol(ast->contents, NULL, 10);
        return errno != ERANGE ? 
                        lval_num(x) : 
                        lval_err(LERR_BAD_NUM);
    }

    // op is always second child. first is '(' 
    char* op = ast->children[1]->contents;

    // next operand is third child
    lval x = eval(ast->children[2]);

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
            lval result = eval(a);
            lval_println(result);

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
