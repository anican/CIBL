#include<stdio.h>

/* basic read-evaluate-print-loop (REPL) that echoes input */ 
/* buffer for storing user input */
static char input[2048];

int main(int argc, char** argv) {
    /* Print Version and Exit Information */
    puts("Lispy Version 0.0.0.0.1");
    puts("Press C-c to Exit\n");

    /* Infinite Loop */
    while (1) { 
        
        /* Output our argument */
        fputs("lispy> ", stdout);
        
        /* Read a line of user input of max size 2048 */
        fgets(input, 2048, stdin);

        /* Echo input back at user */
        printf("Echo: %s", input");

    }

    return 0;
}

