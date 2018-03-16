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

/* Enumeration of possible language value types */
enum { NUMBER, ERROR };

/* Enumeration of possible error types */
enum { DIV_ZERO, UNKNOWN_OPERATOR, LONG_OVERFLOW};

/* CIBL Value structure */
typedef struct {
    int type;
    long num;
    int error;
} value;

/* Create a new number type 'value' */
value number_value(long x) {
    value val;
    val.type = NUMBER;
    val.num = x;
    return val;
}

/* Create a new error type 'value' */
value error_value(int x) {
    value val;
    val.type = ERROR;
    val.error = x;
    return val;
}

void print_value(value val) {
    switch (val.type) {
        /* case for if the value is a number */
        case NUMBER:
            printf("%li", val.num);
        break;
        /* case for if the value is an error */
        case ERROR:
            /* check the type of error it is */
            if (val.error == DIV_ZERO) {
                printf("Error: Division By Zero!");
            } else if (val.error == UNKNOWN_OPERATOR) {
                printf("Error: Invalid Operator!"); 
            } else if (val.error == LONG_OVERFLOW) {
                printf("Error: Integer Overflow!");
            }
        break;
    }
}

void println_value(value val) {
    print_value(val);
    putchar('\n');
}

value eval_operator(value x, char* op, value y) {
    /* Immediately return the values if one of them is an error */
    if (x.type == ERROR) {
        return x;
    }
    if (y.type == ERROR) {
        return y;
    }

    if (strcmp(op, "+") == 0) {
        return number_value(x.num + y.num);
    } else if (strcmp(op, "-") == 0) { 
        return number_value(x.num - y.num);
    } else if (strcmp(op, "*") == 0) { 
        return number_value(x.num * y.num );
    } else if (strcmp(op, "/") == 0) {
        /* Return divide by zero error if the y value is 0 */
        return (y.num == 0) ? error_value(DIV_ZERO) : number_value(x.num / y.num);
    }
    return error_value(UNKNOWN_OPERATOR);
}

value eval(mpc_ast_t* t) {
    
    /* if the token happens to be a number */
    if (strstr(t->tag, "number")) {
        /* Check for potential errors in conversion */
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? number_value(x) : error_value(LONG_OVERFLOW);
    }

    char* operator = t->children[1]->contents;

    /* evaluate third child */
    value x = eval(t->children[2]);

    /* evaluate remaining children */
    int i = 3;
    while (strstr(t->children[i]->tag, "expression")) {
        x = eval_operator(x, operator, eval(t->children[i++]));
    }
    return x;
}

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
            value result = eval(r.output);
            println_value(result);
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
