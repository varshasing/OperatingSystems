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

// run commands in standard way in function with pipes and checks to see if it is a background process and if it needs redirection in special cases
// return child PID for waitpid in main
// free memory for the_command struct(s) in main
// int isFirst isLast, bool redirection flags
int run_command(the_command* command, int input_fd, int isFirst, int isLast)
{
    /*
        command: pointer to the struct currently working with
        input_fd: file descriptor for input, opened in main function
        isFirst: flag for the first command in pipeline
        isLast: flag for the last command in pipeline
    */
    pid_t pid;
    int status;
    int pipe_fd[2];     // created for connecting commands, sink and source. Not doing anything until dup2 connects ends, use this to connect commands with the fd?
    int in_fd, out_fd, output_fd;  // realisticaly, only need one pipe for the entire pipeline, but need to connect the ends of the pipe to the commands
    // and out is what I will pass
    pipe(pipe_fd);

    // check for redirect input
    /*
    // create the pipe if needed
    if(!isLast)                             // if not the last command, need to make the pipe
    {
        if(pipe(pipe_fd) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        output_fd = pipe_fd[1];             // output goes to the write end of the pipe
    }
    else
    {

        output_fd = STDOUT_FILENO;
    }

    if(command->redirect_output == NULL)
    {
        out_fd = STDOUT_FILENO;
    }
    else
    {
        out_fd = open(command->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if(out_fd == -1)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }
    }
    */

    // fork process, child and parent
    if((pid = fork()) == 0)     // child process
    {
        // check for redirection of input and output
        if(command->redirect_output)        // sets out_fd to the opened file
        {
            out_fd = open(command->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if(out_fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            out_fd = STDOUT_FILENO;
        }
        if(out_fd != STDOUT_FILENO)         // if there is output redirection, dup2 to write port
        {
            pipe_fd[1] = out_fd;
            if(dup2(out_fd, STDOUT_FILENO) == -1)
            {
                perror("OUT_FD DUP2");
                exit(EXIT_FAILURE);
            }
            close(out_fd);
            close(pipe_fd[1]);
        }
        if(command->redirect_input)
        {
            in_fd = open(command->redirect_input, O_RDONLY);
            if(in_fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            in_fd = input_fd;
        }
        if(in_fd != STDIN_FILENO)
        {
            pipe_fd[0] = in_fd;
            if(dup2(pipe_fd[0], STDIN_FILENO) == -1)
            {
                perror("IN_FD DUP2");
                exit(EXIT_FAILURE);
            }
            close(in_fd);
            close(pipe_fd[0]);
        }

        if(command->is_background)
        {
            close(STDOUT_FILENO);
        }

        // different cases for piping logic
        // first and no input
        if(isFirst && !isLast && command->redirect_input == NULL)
        {
            dup2(pipe_fd[1], STDOUT_FILENO);
        }
        else if(!isFirst && !isLast && input_fd != 0)
        {
            dup2(input_fd, STDIN_FILENO);
            dup2(pipe_fd[1], STDOUT_FILENO);
        }
        else
        {
            if(command->redirect_input != NULL)
            {
                dup2(pipe_fd[0], STDOUT_FILENO);
            }
            if(command->redirect_output != NULL)
            {
                dup2(pipe_fd[1], STDIN_FILENO);
            }
            if(input_fd != 0)
            {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
        }
        if(execvp(command->command, command->arguments) == -1)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[1]);
        close(STDIN_FILENO);
        dup(STDIN_FILENO);
        close(pipe_fd[0]);
    }
    else
    {
        if(!command->is_background)
        {
            waitpid(pid, &status, 0);
        }
    }

    close(pipe_fd[1]);

    if(input_fd != STDIN_FILENO)
    {
        close(input_fd);
    }
    if(command->next_command == NULL)
    {
        close(pipe_fd[0]);
    }

    return pipe_fd[0];
};



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
            if(current->redirect_input != NULL && current == head)     // input redirection for first command is needed AND need to init input_fd
            {
                input_fd = open(current->redirect_input, O_RDONLY);     // open the file for reading
                if(input_fd == -1)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
            }

            run_command(current, input_fd, (current == head), (current->next_command == NULL));     // run the command

            // INPUT
            /*
                open specified input file
                close STDIN
                dup2 to redirect STDIN to input
                close fd after redirection
            */

           // OUTPUT
           /*
                open w/ flags
                close STDOUT
                dup2 direct STDOUT to output
                close fd after redirection
           */
          // PIPE
          /*
            create a pipe
            fork to exec 1st command
            redirect STDOUT to write end of pipe: dup2
            fork second for chained process
            redirect STDIN to read end of pipe: dup2
            close both ends in parent after fork
          */
         /*
            always create the pipe: only redirect in fd if redirection needed!
            if in background: close fd[1] OUT
            if first command: oly dup2 fd[1] OUT
            if last command: only dup2 fd[0] IN
            if in middle: dup2 both IN and OUT; input in IN and fd[1] OUT
         */
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


/*
I am getting confused with connecting pipes bc hpw dpes the source sink thing even work :tomatoes:
*/

/*

if(command->redirect_input == NULL)     // set to STDIN if there is no inout redirection expected
        {
            in_fd = STDIN_FILENO;
        }
        else
        {
            in_fd = open(command->redirect_input, O_RDONLY);         // open the file for reading
            if(in_fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
        }
        if(command->redirect_output == NULL)    // set to STDOUT if there is no output redirection expected
        {
            out_fd = STDOUT_FILENO;
        }
        else
        {
            out_fd = open(command->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0777);     // open the file for writing
            if(out_fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
        }
        if(in_fd != STDIN_FILENO)
        {
            if((pipe_fd[1] = open(command->redirect_input, O_RDONLY)) != 0)         // stdin reads FROM in
            {
                perror("IN_FD DUP2");
                exit(EXIT_FAILURE);
            }
            close(in_fd);
        }
        if(out_fd != STDOUT_FILENO)
        {
            if(dup2(out_fd, STDOUT_FILENO) == -1)       // stdout writes TO out
            {
                perror("OUT_FD DUP2");
                exit(EXIT_FAILURE);
            }
            close(out_fd);
        }
        //execvp(command->arguments[0], command->arguments);     // execute the command
        if(!isLast)                                     // if this is not the last command in pipe, need to util pipe logic
        {
            if(dup2(pipe_fd[1], STDOUT_FILENO) == -1)   // output goes to the write end of the pipe
            {
                perror("pipe1 DUP2");
                exit(EXIT_FAILURE);
            }
            close(pipe_fd[1]);
        }
        if(command->redirect_output != NULL && isLast)  // output redirection for last command, assumption from doc
        {
            out_fd = open(command->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if(out_fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            if(dup2(out_fd, STDOUT_FILENO) == -1)
            {
                perror("OUT_FD DUP2");
                exit(EXIT_FAILURE);
            }
            close(out_fd);
        }


    }*/