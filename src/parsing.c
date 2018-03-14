#include "mpc.h"
/* Compile following functions if operating on Windows */
#ifdef _WIN32
#include <string.h>
static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Else compile for OSX/Linux using editline headers */
#else
#include <editline/readline.h>
#endif

int main(int argc, char** argv) {
    /* Create language parsers */
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expression = mpc_new("expression");
    mpc_parser_t* Lispy = mpc_new("lispy");

    /* Define parsers with following grammar */
    char* grammar = " "
                    " number : /-?[0-9]+/ ; "
                    " operator : '+' | '-' | '*' | '/' ; "
                    " expression : <number> | '(' <operator> <expression>+ ')' ; "
                    " lispy : /^/ <operator> <expression>+ /$/ ; ";
    mpca_lang(MPCA_LANG_DEFAULT, grammar, Number, Operator, Expression, Lispy);


    /* Print Version and Exit Information */
    puts("Lispy Version 0.0.0.0.1");
    puts("Press C-c to Exit\n");

    /* Infinite Loop */
    while (1) {

        char* input = readline("lispy> ");

        /* Output our argument */
        add_history(input);

        /* Parses user input to console */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            /* If parsed input is well-formed */
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            /* Error throwback if input is malformed */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        /* Free retrieved input */
        free(input);

    }
    mpc_cleanup(4, Number, Operator, Expression, Lispy);
    return 0;
}
