#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>  
#include <unistd.h> 
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

Command* initializeCommand(){
    Command *newCommand = malloc(sizeof(Command));

    newCommand->stdin = NULL;
    newCommand->stdout = NULL;
    newCommand->append_stdout = 0;
    newCommand->wait = 1;
    newCommand->n_commands = 0;
    newCommand->argcs = NULL;
    newCommand->argvs = NULL;
    newCommand->comlist = new_dllist();

    return newCommand;
}

void freeCommand(Command *commandToFree){
    if(commandToFree->stdin != NULL){
        free(commandToFree->stdin);
    }
    if(commandToFree->stdout != NULL){
        free(commandToFree->stdout);
    }
    if(commandToFree->comlist != NULL){
        Dllist temp;

        dll_traverse(temp, commandToFree->comlist) {
            char **argv = (char **) jval_v(temp->val);
            for(int i = 0; argv[i] != NULL; i++){
                free(argv[i]);
            }
            free(argv);
        }
        free_dllist(commandToFree->comlist);
    }
    if(commandToFree->argcs != NULL){
        free(commandToFree->argcs);
    }
    if(commandToFree->argvs != NULL){
        free(commandToFree->argvs);
    }
    
}

void print_comlist(Command *cmd) {
    Dllist node;
    int cmd_num = 0;

    printf("=== Printing comlist ===\n");

    dll_traverse(node, cmd->comlist) {
        char **argv = (char **) jval_v(node->val);
        printf("comlist Command %d: ", cmd_num);

        for (int i = 0; argv[i] != NULL; i++) {
            printf("'%s' ", argv[i]);
        }

        printf("\n");
        cmd_num++;
    }

    printf("Total commands in comlist: %d\n", cmd->n_commands);

    // Print argvs and argcs if available
    if (cmd->argvs != NULL && cmd->argcs != NULL) {
        printf("=== Printing argvs and argcs ===\n");
        for (int i = 0; i < cmd->n_commands; i++) {
            printf("argvs[%d] (argc: %d): ", i, cmd->argcs[i]);
            for (int j = 0; j < cmd->argcs[i]; j++) {
                printf("'%s' ", cmd->argvs[i][j]);
            }
            printf("\n");
        }
    } else {
        printf("argvs and argcs not set yet.\n");
    }
}


int getArgc(char **argv){
    int count = 0;
    while(argv[count] != NULL){
        count++;
    }
    return count;
}

void getArgsFromComList(Command *thisCommand){
    thisCommand->argcs = malloc(sizeof(int) * thisCommand->n_commands);
    thisCommand->argvs = malloc(sizeof(char **) * thisCommand->n_commands);

    Dllist temp;

    int i = 0;
    dll_traverse(temp, thisCommand->comlist){
        char **argv = (char **) jval_v(temp->val);
        thisCommand->argcs[i] = getArgc(argv);
        thisCommand->argvs[i] = argv;
        i++;
    }
}

void executeCommand(Command *commandToExecute){
    pid_t pid;
    int fileDescriptorOpen;
    int fileDescriptorEnd = 0;
    int ends[2];
    JRB pids = make_jrb();
    JRB tmp;

    for(int j = 0; j < commandToExecute->n_commands; j++){
        fflush(stdin);
        fflush(stdout);
        fflush(stderr);

        if(j < commandToExecute->n_commands - 1){
            if(pipe(ends) < 0){
                fprintf(stderr, "Error creating pipe\n");
                exit(1);
            }
        }

        pid = fork();


        if(pid == 0){
            if(j == 0 && commandToExecute->stdin != NULL){
                fileDescriptorOpen = open(commandToExecute->stdin, O_RDONLY);
                if(fileDescriptorOpen == -1){
                    fprintf(stderr, "Error opening file %s\n", commandToExecute->stdin);
                    exit(1);
                }
                dup2(fileDescriptorOpen, STDIN_FILENO);
                close(fileDescriptorOpen);
            }
            if(commandToExecute->stdout != NULL){
                if(commandToExecute->append_stdout == 1){
                    fileDescriptorOpen = open(commandToExecute->stdout, O_WRONLY | O_CREAT | O_APPEND, 0644);
                }else{
                    fileDescriptorOpen = open(commandToExecute->stdout, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                if(fileDescriptorOpen == -1){
                    fprintf(stderr, "Error opening file %s\n", commandToExecute->stdout);
                    exit(1);
                }  
                dup2(fileDescriptorOpen, STDOUT_FILENO);
                close(fileDescriptorOpen);
            }
            if(j > 0){
                dup2(fileDescriptorEnd, STDIN_FILENO);
                //close(fileDescriptorEnd);
            }
            if(j < commandToExecute->n_commands - 1){
                close(ends[0]);
                dup2(ends[1], STDOUT_FILENO);
                close(ends[1]);
            }
            execvp(commandToExecute->argvs[j][0], commandToExecute->argvs[j]);
            fprintf( stderr, "%s: No such file or directory\n", commandToExecute->argvs[j][0]);
            exit(1); 
        }else{
            if(commandToExecute->wait == 1){
                jrb_insert_int(pids, pid, new_jval_i(0));
            }
            if(commandToExecute->n_commands > 1){
                if(fileDescriptorEnd != 0){
                    close(fileDescriptorEnd);
                }
            }
            if(j < commandToExecute->n_commands - 1){
                close(ends[1]);
                fileDescriptorEnd = ends[0];
            }
        }
    }

    while(!jrb_empty(pids)){
        pid = wait(NULL);
        tmp = jrb_find_int(pids, pid);
        if(tmp != NULL){
            jrb_delete_node(tmp);
        }
    }
}


int main(int argc, char const *argv[])
{
    IS is;

    is = new_inputstruct(NULL);

    if(is == NULL) {
        fprintf(stderr, "file cannot be opened");
        return 1;
    }
    Command *thisCommand = initializeCommand();
    while(get_line(is) >= 0){
        //printf("First field: '%s' (NF: %d)\n", is->fields[0], is->NF);
        if(is->NF > 0 && strcmp(is->fields[0], "#") != 0){
            // printf("passing the first if statement\n");
            if(strcmp(is->fields[0], "<") == 0){
                // printf("this line contains a <\n");
                thisCommand->stdin = strdup(is->fields[1]);
            }else if(strcmp(is->fields[0], ">") == 0){
                // printf("this line contains a >\n");
                thisCommand->stdout = strdup(is->fields[1]);
            }else if(strcmp(is->fields[0], ">>") == 0){
                // printf("this line contains a >>\n");
                thisCommand->append_stdout = 1;
                thisCommand->stdout = strdup(is->fields[1]);
            }else if(strcmp(is->fields[0], "NOWAIT") == 0){
                // printf("this line contains a NOWAIT\n");
                thisCommand->wait = 0;
            }else if(strcmp(is->fields[0], "END") == 0){
                // printf("this line contains a END\n");
                // printf("process argument\n");
                getArgsFromComList(thisCommand);
                // print_comlist(thisCommand);
                executeCommand(thisCommand);
                freeCommand(thisCommand);
                thisCommand = initializeCommand();
            }else {
                // printf("this line contains a different argument\n");
                // printf("line 1?: %s\n", is->text1);
                thisCommand->n_commands++;
                char **args = malloc(sizeof(char*) * (is->NF + 1));
                for(int i = 0; i < is->NF; i++){
                    args[i] = strdup(is->fields[i]);
                }
                args[is->NF] = NULL;
                dll_append(thisCommand->comlist, new_jval_v(args));
            }
            // printf("-------------------\n");
        }
    }
    return 0;
}
