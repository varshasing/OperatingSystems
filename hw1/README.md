# ENG EC440: Operating Systems
## Homework 1 Mini-Deadline
### Varsha Singh, U50072781
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
###Your proogram would parse this command line into 3 different commands (technically, the last argument is a file) and output (Line numbers for illustration only).
```c
1: /bin/echo foo
2: /bin/grep -o -m 1 f
3: /user/bin/ec -c > /tmp/x
4: foo
```
#### Above, lines 1-4 are the three commands making up the command-line, line 4 (the string foo) is the result of the first command (/bin/echo foo).