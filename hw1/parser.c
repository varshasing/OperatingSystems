#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

// This should work for the mini-deadline, which only takes in one command and its arguments, and no special characters.
// the setup should work for the final deadline, but I won't implement the logic for the final deadline in main right now.
// assumption, no leading or trailing spaces
#define MAX_LINE 512
#define MAX_ARG_LENGTH 32

#define char_delimit " <>|"                                                     // special characters that will be used for determinig if command struct needs to be created

// writing struct commands

void tokenize(char* buffer, char* token_array[])
{
    // go through the buffer and tokenize it based on each individual parameter
    // want to return an array of c-strings

    if(buffer == NULL)                                                          // if the buffer is empty, don't parse anything
    {
        return;
    }

    char* slow = buffer;                                                        // slow pointer to the buffer. Will be at start of words (expect for special characters with no spaces)
    char* fast = buffer;                                                        // fast pointer to the buffer
    int token_index = 0;                                                        // index for the token array
                                                                                // can use pointer arithmatic to dynamically allocate memory for each token
    while(*fast != '\0')
    {
        while(*fast != '\0')
        {
            if(*fast == ' ')
            {
                fast++;
            }
            else
            {
                break;
            }
        }
        slow = fast;
        if( *fast == '<' || *fast == '>' || *fast == '|' || *fast == '&')       // if special character is found, make sure to add it to the token list and move on
        {
            token_array[token_index] = malloc(sizeof(char)*2);
            strncpy(token_array[token_index], fast, 1);
            token_array[token_index][1] = '\0';
            token_index++;
            fast++;
            slow = fast;
        }
        else{
            while(*fast != ' ' && *fast != '<' && *fast != '>' && *fast != '|' && *fast != '&' && *fast != '\0')        // while fast is still on an argument that isn't a special character
            {
                fast++;
            }
            if((long)(fast - slow) > 0)                                             // pointer arithmatic, if there is a token to be added
            {
                token_array[token_index] = malloc(sizeof(char) * ((fast - slow)+1));       // allocate memory for the token
                strncpy(token_array[token_index], slow, (long)(fast - slow));
                token_array[token_index][(long)(fast - slow)] = '\0';
                token_index++;
                slow = fast;
            }                                                                       // move the slow pointer to the next character
        }
    }
    token_array[token_index] = NULL;                                            // set the last token to NULL

}


the_command* parse_buffer(char* token_array[])                                  // create a linked structs for each command
{
    the_command* head = malloc(sizeof(the_command));                            // if isn't blank, then there is some command to be put in strtuct
    if(!head)
    {
        perror("malloc head");
        return NULL;
    }
    the_command* current = head;                                                // current pointer to the head of the linked list
    current->is_background = 0;                                                 // initialize the background flag to 0
    current->next_command = NULL;                                               // initialize the next command to NULL
    current->redirect_input = NULL;                                             // initialize the input redirection to NULL
    current->redirect_output = NULL;                                            // initialize the output redirection to NULL

    int command_end = 0;
    int command_start = 0;
    int token_index;

    while(token_array[command_end] != NULL)
    {
        int token_count = 0;
        token_index = 0;
        while(token_array[command_end] != NULL && (strcmp(token_array[command_end], "|") != 0))     // count number of arguments for structs arguments
        {
            token_count++;
            token_index++;
            command_end++;
        }
        current->command = malloc(sizeof(char)*(strlen(token_array[command_start]) + 1));          // allocate memory and store command
        if(!current->command)
        {
            perror("malloc command");
            return NULL;
        }
        strcpy(current->command, token_array[command_start]);
        current->arguments = malloc(sizeof(char*)*(token_index+1));
        int i;
        for(i = 0; i < token_count; i++)                                        // allocate memory and store all parameters for process
        {
            current->arguments[i] = malloc(sizeof(char)*(strlen(token_array[command_start + i]) + 1));
            strcpy(current->arguments[i], token_array[command_start + i]);
            if(strcmp(token_array[command_start + i], "<") == 0 && token_array[command_start + i + 1] != NULL)
            {
                current->redirect_input = malloc(sizeof(char)*(strlen(token_array[command_start + i + 1]) + 1));
                strcpy(current->redirect_input, token_array[command_start + i + 1]);
            }
            else if(strcmp(token_array[command_start + i], ">") == 0 && token_array[command_start + i + 1] != NULL)
            {
                current->redirect_output = malloc(sizeof(char)*(strlen(token_array[command_start + i + 1]) + 1));
                strcpy(current->redirect_output, token_array[command_start + i + 1]);
                i++;
            }
            else if(strcmp(token_array[command_start + i], "&") == 0)
            {
                current->is_background = 1;
            }
        }

        current->arguments[token_count] = NULL;

        command_start = command_end + 1;
        if(token_array[command_end] != NULL && strcmp(token_array[command_end], "|") == 0)
        {
            current->next_command = malloc(sizeof(the_command));
            current = current->next_command;
            current->is_background = 0;
            current->next_command = NULL;
            current->redirect_input = NULL;
            current->redirect_output = NULL;
            command_end++;
        }
    }
    return head;
}