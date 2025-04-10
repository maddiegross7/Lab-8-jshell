//Lab 8: JShell
//Madelyn Gross
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

//command struct from TA Maria
//I got rid of the argcs array because I never used it, idk what I was supposed to do with it
//but I didnt do it
typedef struct command{
    char *stdin;          /* Filename from which to redirect stdin.  NULL if empty.*/ 
    char *stdout;         /* Filename to which to redirect stdout.  NULL if empty.*/ 
    int append_stdout;    /* Boolean for appending.*/ 
    int wait;             /* Boolean for whether I should wait.*/ 
    int n_commands;       /* The number of commands that I have to execute*/ 
    char ***argvs;        /* argcv[i] is the argv array for the i-th command*/ 
    Dllist comlist;       /* I use this to incrementally read the commands.*/ 
} Command;

//initializing this command, returning it
Command* initializeCommand(){
    Command *newCommand = malloc(sizeof(Command));

    newCommand->stdin = NULL;
    newCommand->stdout = NULL;
    newCommand->append_stdout = 0;
    newCommand->wait = 1;
    newCommand->n_commands = 0;
    newCommand->argvs = NULL;
    newCommand->comlist = new_dllist();

    return newCommand;
}

//just freeing the command
void freeCommand(Command *commandToFree){
    if(commandToFree->stdin != NULL){
        free(commandToFree->stdin);
    }
    if(commandToFree->stdout != NULL){
        free(commandToFree->stdout);
    }
    //I am going through the comlist and freeing the arrays, each argument will be freed
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
    //because this is simply a pointer to the arrays in comlist I have already freed everything inside
    //each argument of this array so I am just freeing this list
    if(commandToFree->argvs != NULL){
        free(commandToFree->argvs);
    }
    //freeing the command itself
    free(commandToFree);
}

//this function is just assigning the argvs from comlist to the array of argcs
void getArgsFromComList(Command *thisCommand){
    thisCommand->argvs = malloc(sizeof(char **) * thisCommand->n_commands);

    Dllist temp;
    int i = 0;
    dll_traverse(temp, thisCommand->comlist){
        char **argv = (char **) jval_v(temp->val);
        thisCommand->argvs[i] = argv;
        i++;
    }
}

//this is my main execution function
void executeCommand(Command *commandToExecute){
    //initializing the variables I will need
    pid_t pid;
    int fileDescriptorOpen;
    int fileDescriptorEnd = 0;
    int ends[2];
    JRB pids = make_jrb();
    JRB tmp;

    //for each command I have in my commandToExecute struct
    for(int j = 0; j < commandToExecute->n_commands; j++){
        //flushing stdin, stdout, and stderr because they said to 
        fflush(stdin);
        fflush(stdout);
        fflush(stderr);

        //creating pipe if I have more than 1 command
        if(j < commandToExecute->n_commands - 1){
            if(pipe(ends) < 0){
                fprintf(stderr, "Error creating pipe\n");
                exit(1);
            }
        }

        //forking 
        pid = fork();

        //if I am in a child
        if(pid == 0){
            //if I am in my first command and stdin has something to read from
            if(j == 0 && commandToExecute->stdin != NULL){
                fileDescriptorOpen = open(commandToExecute->stdin, O_RDONLY);
                if(fileDescriptorOpen < 0){
                    fprintf(stderr, "Error opening file %s\n", commandToExecute->stdin);
                    exit(1);
                }
                dup2(fileDescriptorOpen, STDIN_FILENO);
                close(fileDescriptorOpen);
            }
            //if stdout has anything 
            if(commandToExecute->stdout != NULL){
                //opening to append or truncate depending on the input
                if(commandToExecute->append_stdout == 1){
                    fileDescriptorOpen = open(commandToExecute->stdout, O_WRONLY | O_CREAT | O_APPEND, 0644);
                }else{
                    fileDescriptorOpen = open(commandToExecute->stdout, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                if(fileDescriptorOpen < 0){
                    fprintf(stderr, "Error opening file %s\n", commandToExecute->stdout);
                    exit(1);
                }  
                dup2(fileDescriptorOpen, STDOUT_FILENO);
                close(fileDescriptorOpen);
            }
            //if this is not the first command
            if(j > 0){
                dup2(fileDescriptorEnd, STDIN_FILENO);
                close(fileDescriptorEnd);
            }
            //if this is not the last command
            if(j < commandToExecute->n_commands - 1){
                close(ends[0]);
                dup2(ends[1], STDOUT_FILENO);
                close(ends[1]);
            }
            //execute the command for this iteration and error to not fork bomb
            //because fork bombing is scary and awful and horrible and plank says you shouldnt do it
            execvp(commandToExecute->argvs[j][0], commandToExecute->argvs[j]);
            fprintf( stderr, "%s: No such file or directory\n", commandToExecute->argvs[j][0]);
            exit(1); 
        //if I am in the partent
        }else{
            //if we want to wait add them to a tree
            if(commandToExecute->wait == 1){
                jrb_insert_int(pids, pid, new_jval_i(0));
            }
            //if this is not the first command I have to close the file descriptor maybe?
            if(commandToExecute->n_commands > 1){
                if(fileDescriptorEnd != 0){
                    close(fileDescriptorEnd);
                }
            }
            //if this is not the last command close the last end and reset the file descriptor
            if(j < commandToExecute->n_commands - 1){
                close(ends[1]);
                fileDescriptorEnd = ends[0];
            }
        }
    }

    //this is where we wait for the children to finish making sure to delete them to the tree when
    //the do so that we can finish
    while(!jrb_empty(pids)){
        pid = wait(NULL);
        tmp = jrb_find_int(pids, pid);
        if(tmp != NULL){
            jrb_delete_node(tmp);
        }
    }

    jrb_free_tree(pids);
}

//main
int main(int argc, char const *argv[])
{
    //new input struct to read the files
    IS is;
    is = new_inputstruct(NULL);

    if(is == NULL) {
        fprintf(stderr, "file cannot be opened");
        return 1;
    }

    Command *thisCommand = initializeCommand();

    //just reading all of the input and stuff
    while(get_line(is) >= 0){
        if(is->NF > 0 && strcmp(is->fields[0], "#") != 0){
            if(strcmp(is->fields[0], "<") == 0){
                thisCommand->stdin = strdup(is->fields[1]);
            }else if(strcmp(is->fields[0], ">") == 0){
                thisCommand->stdout = strdup(is->fields[1]);
            }else if(strcmp(is->fields[0], ">>") == 0){
                thisCommand->append_stdout = 1;
                thisCommand->stdout = strdup(is->fields[1]);
            }else if(strcmp(is->fields[0], "NOWAIT") == 0){
                thisCommand->wait = 0;
            }else if(strcmp(is->fields[0], "END") == 0){
                //this is where everything gets called because we have come across an end line
                getArgsFromComList(thisCommand);
                executeCommand(thisCommand);
                freeCommand(thisCommand);
                thisCommand = initializeCommand();
            }else {
                thisCommand->n_commands++;
                char **args = malloc(sizeof(char*) * (is->NF + 1));
                for(int i = 0; i < is->NF; i++){
                    args[i] = strdup(is->fields[i]);
                }
                args[is->NF] = NULL;
                dll_append(thisCommand->comlist, new_jval_v(args));
            }
        }
    }
    //freeing the last command that I initialize after I free the last real one
    freeCommand(thisCommand);
    jettison_inputstruct(is);
    return 0;
}
