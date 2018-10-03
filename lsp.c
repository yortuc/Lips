#include "mpc.h"
#include "editline/readline.h"
#include <stdio.h>

// enum for lval (lisp evaluation result) types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

// enum for errors
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

typedef struct lval {
    int type;
    long num;

    // error and symbol types have some string data
    char* err;
    char* sym;

    // count and pointer to a list of lval*
    int count;
    struct lval** cell;
} lval;

// construct a new pointer to a new num lval
lval* lval_num(long x){
    lval* v = malloc(sizeof(lval));
    v->num = x;
    v->type = LVAL_NUM;
    return v;
}

// construct a new pointer to a new error lval
lval* lval_err(char* err_msg){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(err_msg) + 1);
    strcpy(v->err, err_msg);
    return v;
}

// construct a pointer to a new symbol lval
lval* lval_sym(char* s){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}
// construct a ponter to a new s-expression lval
lval* lval_sexpr(void){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

// forward declare
void lval_print(lval* v);

void lval_exp_print(lval* v, char open, char close){
    printf("lval_exp_print %i", v->count);

    putchar(open);
    for(int i=0; i< v->count; i++){
        lval_print(v->cell[i]);

        //dont print trailing space if last element
        if(i != v->count-1){
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print(lval* v){
    switch(v->type){
        case LVAL_NUM: printf("%li", v->num); break;
        case LVAL_ERR: printf("Error: %s", v->err); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_exp_print(v, '(', ')'); break;
    }
}

// print lval with a new line at the end
void lval_println(lval* v){ lval_print(v); putchar('\n'); }

void lval_del(lval* v){
    switch(v->type){
        // do nothing special for number type
        case LVAL_NUM: break;

        // for err or sym, free the string data
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;

        // if s-expression, delete all elements inside
        case LVAL_SEXPR:
            for(int i=0; i<v->count; i++){
                lval_del(v->cell[i]);
            }
            // also free the memory used to contain pointers
            free(v->cell);
            break;
    }
    // free the memory allocated by lval itself
    free(v);
}

lval* lval_read_num(mpc_ast_t* t){
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? 
        lval_num(x) :
        lval_err("invalid number");
}

lval* lval_add(lval* v, lval* x){
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

lval* lval_read(mpc_ast_t* t){
    // if symbol or number, return conversion to that type
    if(strstr(t->tag, "number")) { return lval_read_num(t); }
    if(strstr(t->tag, "sym")) { return lval_sym(t->contents); }

    // if toor (>) or or sexpr then create an empty list
    lval* x = NULL;
    if(strcmp(t->tag, ">") ==0) { return lval_sexpr(); }
    if(strstr(t->tag, "sexpr")){ return lval_sexpr(); }

    // fill this list with any valid expression contained within
    for(int i=0; i<t->children_num; i++){
        if(strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if(strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if(strcmp(t->children[i]->tag, "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }
    printf("lval_read %i", x->count);
    return x;
}

int main(int argc, char** argv) {
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                            \
        number : /-?[0-9]+/ ;                    \
        symbol : '+' | '-' | '*' | '/' ;         \
        sexpr  : '(' <expr>* ')' ;               \
        expr   : <number> | <symbol> | <sexpr> ; \
        lispy  : /^/ <expr>* /$/ ;               \
    ",
    Number, Symbol, Sexpr, Expr, Lispy);   
  
    puts("Lips v0.0.1");

    while(1) {
        char* input = readline("Lips> ");
        add_history(input);

        // parse input
        mpc_result_t result;
        if(mpc_parse("<stdin>", input, Lispy, &result)){
            // get ast from output
            lval* x = lval_read(result.output); 
            lval_println(x);
            lval_del(x);
        } else {
            // otherwise print error
            mpc_err_print(result.error);
            mpc_err_delete(result.error);
        }

        free(input);
    }
        
    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
    return 0;
}
