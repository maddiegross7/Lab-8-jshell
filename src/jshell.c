#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fields.h"
#include "jval.h"
#include "dllist.h"
#include "jrb.h"

typedef struct command{
    char *stdin;          /* Filename from which to redirect stdin.  NULL if empty.*/ 
    char *stdout;         /* Filename to which to redirect stdout.  NULL if empty.*/ 
    int append_stdout;    /* Boolean for appending.*/ 
    int wait;             /* Boolean for whether I should wait.*/ 
    int n_commands;       /* The number of commands that I have to execute*/ 
    int *argcs;           /* argcs[i] is argc for the i-th command*/ 
    char ***argvs;        /* argcv[i] is the argv array for the i-th command*/ 
    Dllist comlist;       /* I use this to incrementally read the commands.*/ 
  } Command;

int main(int argc, char const *argv[])
{
    IS is;

    is = new_inputstruct(NULL);

    if(is == NULL) {
        fprintf(stderr, "file cannot be opened");
        return 1;
    }
    
    while(get_line(is) >= 0){
        printf("First field: '%s' (NF: %d)\n", is->fields[0], is->NF);
        if(is->NF > 0 && strcmp(is->fields[0], "#") != 0){
            printf("passing the first if statement\n");
            if(strcmp(is->fields[0], "<") == 0){
                printf("this line contains a <\n");
            }else if(strcmp(is->fields[0], ">") == 0){
                printf("this line contains a >\n");
            }else if(strcmp(is->fields[0], ">>") == 0){
                printf("this line contains a >>\n");
            }else if(strcmp(is->fields[0], "NOWAIT") == 0){
                printf("this line contains a NOWAIT\n");
            }else if(strcmp(is->fields[0], "END") == 0){
                printf("this line contains a END\n");
            }else {
                printf("this line contains a different argument\n");
            }
            printf("-------------------\n");
        }
    }
    return 0;
}
