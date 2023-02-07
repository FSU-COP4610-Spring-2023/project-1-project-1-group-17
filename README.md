## Project 1 Group 17


## Members 
- Thomas Vogt
- Emma Baudo
- Daniel Pijeira


## Divison of Labor: 

### Part 1 (Parsing): 
Emma Baudo, Danny Pijeira

### Part 2 (Environmental Variables): 
Emma Baudo, Thomas Vogt 

### Part 3 (Prompt): 
Emma Baudo, Thomas Vogt

### Part 4 (Tilde Expansion): 
Danny Pijeira

### Part 5 ($PATH search):
Danny Pijeira, Thomas Vogt

### Part 6 (External Command Execution):
Danny Pijeira, Thomas Vogt

### Part 7 (I/O Redirection): 
Emma Baudo, Thomas Vogt

### Part 8 (Piping):
Thomas Vogt, Danny Pijeira

### Part 9 (Background Processing):
Emma Baudo, Thomas Vogt, Danny Pijeira

### Part 10 (Built-in Functions)
Thomas Vogt, Emma Baudo, Danny Pijeira



## Bugs
First Bug: 
    - For part 6: External Command Execution, Thomas and Danny had a difficult time getting ls to print. The bug started at 2:00 pm on 2/3/23. It was occuring at 
    
Second Bug: 
    - For part 7: I/O redirection, Emma had a seg fault during runtime. The bug started at 5:00 pm on 2/2/23. The issue was with the strchr. This library function searches for the first occurence of a character. It was looking for the  characters for input and output in the string entered by the user for tokens -> items[i]. This library function did not work and caused a seg fault until it was changed to strcmp. The compares 2 strings character by character. If the strings turn out to be equal, then the function will return 0. To fix this issue, the new if statement inside the for loop that stores the filename entered by the user, uses strcmp. This will check if the input and output characters occur in th string entered. 
Third Bug: 
    - 

## Current finished portions of the project: 
- Parsing 
- Environmental Variables 
- Prompt 
- Tilde Exapansion 
- $PATH search 
- External Command Execution (ls with flags works, and other commands, but not ls by itself)


## Unfinished portions 
- 


## Extra Credit completed: 
- None 


## Special Considerations: 
- None 


## List of files in repository & Brief Description of each: 
- README.md: this file contains our group member's names, the division of labor, who worked on what parts throughout the project before submission, list of files in our github repository, how to compile our executables using the Makefile we created, known bugs, and unfinished portions of the project (if any). The purpose of this file is to contain all the work that we accomplished throughout the project, how we accomplished, and how we fixed any errors, if there was any.
- main.c: this file contains all of our group's code for project 1. It follows the specifications listed in the Project Decsription on Canvas and includes all parts required for the project. There was no extra credit problems that were attempted, so there will be no functions found relating to any within the main.c. 
- parser.c: this file in our github was provided to us for this project and has been implemented and used within the main.c. This file is based around an important of the project, that all parts and functions use which are the tokens. This file also contains the starting point for int main() which was further implemented in main.c. 
- .github: this a default folder provided by Github that allows for Github classroom feedback. It contains nothing in the folder. 
- Makefile: this file is our makefile for the project. 


## How to compile our executables using our Makefile: 
- Clone this repository to your local directory and type "make" in your terminal to compile the executables. Run by typing "shell".
