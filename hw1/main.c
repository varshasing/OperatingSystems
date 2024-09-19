#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

#define MAX_LINE 512                                        // maximum number of lines from command line. use the size for malloc
#define MAX_LENGTH_ARGS 32                                  // maximum number of letters in an argument
#define TRUE 1


// struct definition. Stores commands, arguments, flag for background, in/out redirection, and LL fashion for next command in a pipeline
// used for string parser


int main(int argc, char* argv[])
{
    char* buffer;                                           // buffer to store the input from stdin
    buffer = malloc(sizeof(char)*MAX_LINE);                 // allocate memory for the buffer
    if(!buffer)
    {
        exit(0);
    }
    char* token_array[MAX_LINE];
    int suppress = 0;
    if(argc > 1)
    {
        if(strcmp(argv[1], "-n") == 0)
        {
            suppress = 1;
        }
    }


    while(TRUE)                                             // REPL: read, evaluate, print, loop
    {
        if(!suppress)
        {
            printf("my_shell$ ");                           // print the shell prompt
            fflush(stdout);
        }
        if(fgets(buffer, MAX_LINE, stdin) == NULL)          // get the input from the command line
        {
            if(feof(stdin))
            {
                break;
            }
        }
        int len = strlen(buffer);
        if(len > 0 && buffer[len - 1] == '\n')              // remove the newline character from the buffer
        {
            buffer[len - 1] = '\0';
        }
        int i;
        for(i = 0; i < MAX_LINE; i++)
        {
            token_array[i] = NULL;
        }
        tokenize(buffer, token_array);                      // tokenize the buffer into an array of c-strings
        i = 0;
        /*
        for(; token_array[i] != NULL; i++)
        {
            printf("argument is: %s\n", token_array[i]);
        }
        */
        the_command* head = parse_buffer(token_array);
        the_command* current = head;
            while(current != NULL)
            {
                i = 0;
                for(; current->arguments[i+1] != NULL; i++) // print out each of the commands (what is input for execvp(2) args)
                {
                    if(current->arguments[i] != NULL)
                    {
                        printf("%s ", current->arguments[i]);
                        fflush(stdout);
                    }
                    if(strcmp(current->arguments[i], "-n") == 0)
                    {
                        suppress = 1;
                    }
                }
                if(current->arguments[i] != NULL)
                {
                    printf("%s", current->arguments[i]);
                    fflush(stdout);
                }
                if(current->redirect_input != NULL)
                {
                    printf(" %s", current->redirect_input);
                    fflush(stdout);
                }
                if(current->redirect_output != NULL)
                {
                    printf(" %s", current->redirect_output);
                    fflush(stdout);
                }
                if(current->is_background == 1)
                {
                    printf(" &");
                    fflush(stdout);
                }
                /*
                if(head->redirect_input != NULL)
                {
                    printf("redirectinput: %s\n", head->redirect_input);
                }
                if(head->redirect_output != NULL)
                {
                    printf("redirectoutput: %s\n", head->redirect_output);
                }
                if(head->is_background == 1)
                {
                    printf("Background process\n");
                }
                if(head->next_command != NULL)
                {
                    printf("Next command is %s\n", head->next_command->command);
                }
                */
               if(current != NULL && current->arguments[i] != NULL)
               {
                    printf("\n");
                    fflush(stdout);
               }
                current = current->next_command;
            }
            // run the first process
            // fork the process
            // if parent, wait for child
            // if child, execvp
            // if there is a next command, run the next command
            // if there is no next command, break
            pid_t pid;
            int status;

            if((pid = fork()) > 0)
            {
                // parent
                waitpid(pid, &status, 0);
            }
            else if(pid < 0)
            {
                perror("fork");
                exit(1);
            }
            else
            {
                // Child
                //execve(head->command, head->arguments, NULL);
                if(execvp(head->command, head->arguments) == -1)
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }

            }
            fflush(stdout);

    }
    return 0;

}


/*
    while(TRUE)
    {
    type_prompt();
    read_command(command, parameters);
    if(fork() != 0)
    // parent code
        waitpid(-1, &status, 0);
    else
    {
    // child code
        execve(command, parameters, 0);
    }
*/