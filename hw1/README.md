# ENG EC440: Operating Systems, Varsha Singh, U50072781
## Homework 1 Mini-Deadline Specifications
#### For the first (mini-deadline),  you shell must demonstrate that it can accept user-input, parse that input, and invoke simple commands with arguments. That is, you will need to implement the parsing logic, and at least steps 1, 2, and 4 from the "some hints" section. In order to test your shell-program, it must REPL and for each command-line output the following:
```c
<first command>
<second command>
...
<nth command>
whatever execvp() of the first command produces
```
#### For illustration purposes consider the following example:
```c
/bin/echo foo | /bin/grep -o m 1 f | /user/bin/wc -c > /tmp/x
```
###Your program would parse this command line into 3 different commands (technically, the last argument is a file) and output (Line numbers for illustration only).
```c
1: /bin/echo foo
2: /bin/grep -o -m 1 f
3: /user/bin/ec -c > /tmp/x
4: foo
```
#### Above, lines 1-4 are the three commands making up the command-line, line 4 (the string foo) is the result of the first command (/bin/echo foo).

## Approach
To meet the mini- deadline, the following needed to be implemented:
1. A command-line parser to figure out what the user is trying to do.
2. If a valid command is entered, the shell should ```c fork(2)``` to create a new (child) process, and the child process should ```c exec``` the command.
3. Split apart the command(s) to separate the command line into different commands via piping,
4. Prepare for the user to run the executable file with -n as an argument to suppress the ```c my_shell$ ``` command-line prompt.
5. Utilize ```c CTRL+D``` to terminate the shell program.

With these steps, I outlined a simple procedure:
###__Implement a String Parser__
The shell parser must take in the user inputs and *tokenize* the command-line input.
In *Lecture 0*, we discussed an approach to a string parser by using the ```c strpbrk(3)``` function, which is called via ```c char* strpbrk(const char *s, const char *accept)``` The ```c strpbrk(3)``` functin locates the first occurence in the string s of any of the bytes in the string accept, returning a pointer to the bute in s that matches one of the bytes in accept, or NULL if no such byte is found.

I found this method to be a good starting point, but it introduced very erroneous heuristic approaches that would fail to correctly parse with the caveat in special characters. The caveat is that spaces aren't neccessary between special characters, so my first implementation struggled with this function.

This leads to my implemented approach: using dynamic memory allocation to fill in the ```c char* token_array[]``` with an implementation of a fast and slow pointer.
#### __Fast and SLow Pointers__
Fast and Slow pointers is a coding pattern that uses two pointeers to traverse a data structure at different speeds, and is also known as the Hare and Tortoise algorithm. They are usually used with arrays and linked lists, and is a methodical approach I learned about in *ENG EC330: Algorithms and Data Structures for Engineers* as well as when solving *Leetcode* questions for interview preparation.
