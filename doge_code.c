#include "mpc.h"

int main(int argc, char const *argv[])
{
    // build a parser Adjective, to recognize things
    mpc_parser_t* Adjective = mpc_or(4, 
        mpc_sym("wow"), mpc_sym("many"),    
        mpc_sym("so"), mpc_sym("such")
    );

    // build a parser , Noun, to recognize things
    mpc_parser_t* Noun = mpc_or(5,
        mpc_sym("lisp"), mpc_sym("language"),    
        mpc_sym("book"), mpc_sym("build"), 
        mpc_sym("c")
    );

    mpc_parser_t* Phrase = mpc_and(2, mpcf_strfold, 
        Adjective, Noun, free);

    mpc_parser_t* Doge = mpc_many(mpcf_strfold, Phrase);

    // do some parsing here

    mpc_delete(Doge);

    return 0;
}

