#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

/* basic read-evaluate-print-loop (REPL) that echoes input */ 
/* buffer for storing user input */

int main(int argc, char** argv) {
    /* Print Version and Exit Information */
    puts("Lispy Version 0.0.0.0.1");
    puts("Press C-c to Exit\n");

    /* Infinite Loop */
    while (1) { 
        
        char* input = readline("lispy> ");  

        /* Output our argument */
        add_history(input);
         
        /* Echo input back at user */
        printf("Echo: %s\n", input);
        
        /* Free retrieved input */
        free(input);
        
    }

    return 0;
}

