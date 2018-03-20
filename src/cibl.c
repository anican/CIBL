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
enum { NUMBER, ERROR, SYMBOL, SEXPRESSION };

/* Enumeration of possible error types */
enum { DIV_ZERO, UNKNOWN_OPERATOR, LONG_OVERFLOW };

/* CIBL Value structure */
typedef struct {
    int type;
    long num;
    /* Error and Symbol types are message strings */
    char* error;
    char* symbol;
    /* Pointer to list of stored values and number of values */
    int count;
    struct value** cell;
} value;

/* Create a pointer to new number type 'value' */
value* number_value(long x) {
    value* val = malloc(sizeof(value));
    val->type = NUMBER;
    val->num = x;
    return val;
}

/* Create a pointer to a new error type 'value' */
value* error_value(char* m) {
    value* val = malloc(sizeof(value));
    val->type = ERROR;
    val->error = malloc(strlen(m) + 1);
    strcpy(val->error, m);
    return val;
}

/* Create a pointer to a new symbol value */
value* symbol_value(char* s) {
    value* val = malloc(sizeof(value));
    val->type = SYMBOL;
    val->symbol = malloc(strlen(s) + 1);
    strcpy(val->symbol, s);
    return val;
}

/* Create a pointer to a new S-Expression value */
value* sexpression_value(void) {
    value* val = malloc(sizeof(value));
    val->type = SEXPRESSION;
    val->count = 0;
    val->cell = NULL;
    return val;
}

void delete_value(value* val) {
    switch (val->type) {
        case NUMBER: break;
        /* Free the strings of either error or symbol*/
        case ERROR: free(val->error); break;
        case SYMBOL: free(val->symbol); break;
        /* Delete all the elements of the S-Expression */
        case SEXPRESSION:
            for (int i = 0; i < val->count; i++) {
                delete_value(val->cell[i]);
            }
            /* Free memory allocated to cell */
            free(val->cell);
        break;    
    }
    /* free memory for value itself */
    free(val);
}

value* read_number_value(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? number_value(x) : error_value("Invalid number");
}

value* read_value(mpc_ast_t* t) {
    /* If the value is a number or a symbol, return converted t */
    if (strstr(t->tag, "number")) {
        return read_number_value(t);
    }
    if (strstr(t->tag, "symbol")) { 
        return symbol_value(t->contents);
    }

    value* x = NULL;
    if (strcmp(t->tag, ">") == 0) {
        x = sexpression_value(); 
    }
    if (strstr(t->tag, "sexpression")) {
        x = sexpression_value(); 
    }

    /* Fill list with all remaining valid expressions */
    for (int i = 0; i < t->children_num; i++) {
        /* Ignore reading the value if the current character is a expression
         * parenthesis or a regular expression
         */
        if (strcmp(t->children[i]->contents, "(") == 0 ||
            strcmp(t->children[i]->contents, ")") == 0 ||
            strcmp(t->children[i]->tag,  "regex")) {
            continue;
        }
        x = add_value(x, read_value(t->children[i]);
    }
    return x;
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
    mpc_parser_t* Sexpression = mpc_new("sexpresion");

    /* Define parsers with following grammar */
    char* grammar = " "
                    " number : /-?[0-9]+/ ; "
                    " operator : '+' | '-' | '*' | '/' ; "
                    " expression : <number> | '(' <operator> <expression>+ ')' ; "
                    " lispy : /^/ <operator> <expression>+ /$/ ; "
                    " sexpression : '(' <expression>* ')' ;";
    mpca_lang(MPCA_LANG_DEFAULT, grammar, Number, Operator, Expression, Lispy,                      Sexpression);

    
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

    mpc_cleanup(5, Number, Operator, Expression, Lispy, Sexpression);
    return 0;
}
