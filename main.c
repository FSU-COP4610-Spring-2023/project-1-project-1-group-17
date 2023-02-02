#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/wait.h>
#include <fcntl.h>



//____________________________________TOKENS______________________________________________
typedef struct {
    int size;
    char **items;
} tokenlist;

char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

void tildeExpansion(tokenlist *tokens);
int EnvironmentVars();
void prompt();

tokenlist *new_tokenlist(void)
{
    tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = (char **) malloc(sizeof(char *));
    tokens->items[0] = NULL; /* make NULL terminated */
    return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
    int i = tokens->size;

    tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
    tokens->items[i] = (char *) malloc(strlen(item) + 1);
    tokens->items[i + 1] = NULL;
    strcpy(tokens->items[i], item);

    tokens->size += 1;
}

char *get_input(void)
{
    char *buffer = NULL;
    int bufsize = 0;

    char line[5];
    while (fgets(line, 5, stdin) != NULL) {
        int addby = 0;
        char *newln = strchr(line, '\n');
        if (newln != NULL)
            addby = newln - line;
        else
            addby = 5 - 1;

        buffer = (char *) realloc(buffer, bufsize + addby);
        memcpy(&buffer[bufsize], line, addby);
        bufsize += addby;

        if (newln != NULL)
            break;
    }

    buffer = (char *) realloc(buffer, bufsize + 1);
    buffer[bufsize] = 0;

    return buffer;
}

tokenlist *get_tokens(char *input)
{
    char *buf = (char *) malloc(strlen(input) + 1);
    strcpy(buf, input);

    tokenlist *tokens = new_tokenlist();

    char *tok = strtok(buf, " ");
    while (tok != NULL) {
        add_token(tokens, tok);
        tok = strtok(NULL, " ");
    }

    free(buf);
    return tokens;
}

void free_tokens(tokenlist *tokens)
{
    for (int i = 0; i < tokens->size; i++)
        free(tokens->items[i]);
    free(tokens->items);
    free(tokens);
}


//__________________________________________________________________________________
void forker(){
    char *x[2];
    x[0] = "ls";
    x[1] = NULL;

    int pid = fork();
    if(pid == 0)
    {
        printf("I am a child process\n");
        execv(x[0],x); // can't use execvp, but i need to know how to print the actual file names
        //execvp(x[0], x); this works, but is not allowed?
    }
    else
    {
        printf("I am the parent process\n");
        waitpid(pid,NULL,0);
    }
    
    
}
//__________________________________________________________________________________
//                                     Tilde Expansion

/*
This function takes in the tokenlist and checks if a tilde is present.
Then it finds the specific token where the tilde is located and replaces it
with $HOME and concatenates the rest of the directory on to $HOME
*/
void tildeExpansion(tokenlist *tokens)
{
    for(int i = 0; i < tokens->size; i++)
    {
        if(strchr(tokens->items[i], '~') != NULL)
        {
            char * copy = strchr(tokens->items[i], '/');
            char copyTwo[strlen(copy)];
            strcpy(copyTwo, copy);
            
            //printf(copyTwo);

            unsigned int length = strlen(copy) + strlen(getenv("HOME"));

            tokens->items[i] = (char*) realloc(tokens->items[i], length * sizeof(char));
            strcpy(tokens->items[i], getenv("HOME"));
            strcat(tokens->items[i], copyTwo);
        }
    }
}


int main()
{

    char *hello = malloc(strlen("Welcome to our shell!\n") + 1);
    strcpy(hello, "Welcome to our shell!\n");
    printf("%s", hello);
    free(hello);
    
    
    while (1) {
        prompt();

        /* input contains the whole command
         * tokens contains substrings from input split by spaces
         */

        char *input = get_input();
        printf("whole input: %s\n", input);

        tokenlist *tokens = get_tokens(input);

        //Tilde Expansion
        for (int i = 0; i < tokens->size; i++)
        {
            if(strchr(tokens->items[i], '~') != NULL)
            {
                tildeExpansion(tokens);
            }
        }

        //$PATH search
        void pathSearch(tokens->items);

        for (int i = 0; i < tokens->size; i++) {
            printf("token %d: (%s)\n", i, tokens->items[i]);
        }
        
        //dealing with an ls
        int compare =strcmp (input, "ls");
        if(compare == 0){
            forker();
        }

        free(input);
        free_tokens(tokens);
        
        
       
    }
    
    return(0);

return 0;
}



//use this for environment variables: it identifies individual environment variables
//but need to get first occurence of $ then print the correct value
int EnvironmentVars(char * input){


    char * getenv(const char *name);
    char * user = getenv(input);
    char * machine = getenv(input);
    char * pwd = getenv(input);


    //first case = USER (Ex: abbinant)
    if (user != NULL){
        printf("USER: %s\n", user);
        return 0;
    }
    else if (user == NULL){ // error testing
        printf("ERROR: This cannot be found..");
        return 1;
    }
    //second case = MACHINE (Ex" linprog2.cs.fsu.edu)
    else if (machine != NULL){
        printf("MACHINE: %s\n", machine);
        return 0;
    }
    else if (machine == NULL){ // error testing
        printf("ERROR: This cannot be found...");
        return 1;
    }
    //third case: PWD (Ex: /home/grads/abbinant >
    else if (pwd != NULL){
        printf("PWD: %s\n", pwd);
        return 0;
    }
    else if (pwd == NULL){ // error testing
        printf("ERROR: This cannot be found...");
        return 1;
    }
    else if (input == "USER MACHINE PWD"){
        printf(user, "@", machine, " : ", pwd);
    }


}


//FINISHED------------------------
void prompt(){
    char * getenv(const char *name);
    char * user = getenv("USER");
    char * machine = getenv("MACHINE");
    char * pwd = getenv("PWD");

    printf("%s""%c""%s"" ""%c"" ""%s"">", user, 64, machine, 58, pwd);


}




























































































