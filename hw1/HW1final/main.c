#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

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
        exit(EXIT_FAILURE);
    }
    char* token_array[MAX_LINE];
    int suppress = 0;
    if(argc > 1)
    {
        if(strcmp(argv[1], "-n") == 0)                      // if the -n flag is used for grading, suppress the my_shell$
        {
            suppress = 1;
        }
    }


    while(TRUE)                                             // REPL: read, evaluate, print, loop; goes until CTRL+D ends it
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
        for(i = 0; i < MAX_LINE; i++)                       // initialize the token array to NULL, resets each time                       
        {
            token_array[i] = NULL;
        }
        //i = 0;
        tokenize(buffer, token_array);                      // tokenize the buffer into an array of c-strings
        the_command* head = parse_buffer(token_array);      // creates linked list of commands
        the_command* current = head;                        // first command in LL format

        // create components for redirection
        int input_fd = STDIN_FILENO;                        // input fd
        int output_fd = STDOUT_FILENO;                      // output fd
        int pipe_fd[2];                                     // pipe fd
        pid_t pid;                                          // processid for fork
        int child_status;                                   // signal


        while(current != NULL)                              // for each command struct
        {
            // pipe must be created BEFORE forking child process
            // multiple pipes might have input and output redirection
            // pipe closed in parent process sends eof to process reading pipe
            // pipe/file opened in parent may be closed with child is forked, won't affect fd for child

            
            if(current->next_command != NULL)               // check if piping is needed for this input
            {
                if(pipe(pipe_fd) == -1)                     // create pipe
                {
                    perror("ERROR: pipe");
                    exit(EXIT_FAILURE);
                }
            }
            
            if((pid = fork()) == -1)                        // fork and create new process
            {
                perror("ERROR: fork");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0)                               // child process
            {
                if(input_fd != 0)                           // input redirection, need to change fd in table
                {
                    if(dup2(input_fd, STDIN_FILENO) == -1)  // return value of fd duplication, fd in table
                    {
                        perror("ERROR: input redirection");
                        exit(EXIT_FAILURE);
                    }
                    close(input_fd);                        // close the input fd
                }
                if(current->next_command != NULL)           // there exists a command to pipe
                {
                    if(dup2(pipe_fd[1], STDOUT_FILENO) == -1)       // output redirection for pipe, need to update for pipe int array. Output of THIS command goes to input of next
                    {
                        perror("ERROR: piping 1");
                        exit(EXIT_FAILURE);
                    }
                    close(pipe_fd[1]);                      // close the write end of the pipe
                }


                if(current->redirect_input != NULL)         // input redirection
                {
                    int fd_input = open(current->redirect_input, O_RDONLY);     // open the file for READ ONLY
                    if(fd_input == -1)                      
                    {
                        perror("ERROR: open file redirection");
                        exit(EXIT_FAILURE);
                    }
                    if(dup2(fd_input, STDIN_FILENO) == -1)
                    {
                       perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(fd_input);
                }

                    execvp(current->command, current->arguments);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    // parent process
                    if(current->next_command != NULL)
                    {
                        close(pipe_fd[1]);
                    }
                        if(current->is_background == 0)
                        {
                            waitpid(pid, &child_status, 0);
                        }
                        child_status = pipe_fd[0];
                }

                current = current->next_command;
            }
                free_struct(head);
                fflush(stdout);
    }
    free(buffer);
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