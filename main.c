#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>




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

        pathSearch(tokens);


        for (int i = 0; i < tokens->size; i++) {
            printf("token %d: (%s)\n", i, tokens->items[i]);
        }
        
        EnvironmentVars(tokens);
        free(input);
        free_tokens(tokens);
        
        
       
    }
    
    return(0);

return 0;
}



//FINISHED------------------------
int EnvironmentVars(tokenlist *tokens){

    for (int i = 0; i < tokens->size; i++){

        char dollar = tokens->items[i][0];

        if(dollar == '$'){

            char * doCommand = getenv(tokens->items[i] + 1);
            printf("%s", doCommand, "\n");
            printf("\n");
        }
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

//new path search() feb 3. 11:09
//External execution

int pathSearch(tokenlist * tokens)
{
    
    char * argv[tokens->size];
    //michael said to make sure argv is null terminated at the end... think this was the fix i needed
    argv[tokens->size + 1] = NULL;
    
    for(int i = 0; i < tokens->size; i++){
        argv[i] = tokens->items[i];
    }
    
    char * temp_Path= getenv("PATH");
    char * colon = strtok(temp_Path, ":");

    bool condition = false;
    while(condition == false){
        printf("\nNEW LOOP\n");
        printf("LENGTH OF COLON: \n%d", colon);
        unsigned int length = strlen(colon) + strlen(argv[0]) + 6;
       
        char buffer[length];
        printf("\nLENGTH OF BUFFER: \n%d", length);
        
        strcpy(buffer, colon);
        
        strcat(buffer, "/");
        strcat(buffer, argv[0]);
        

        printf("\nBUFFER: \n");
        printf("%s\n", buffer); //printing to confirm it works, it does
        
        //do access check
    
        if (access(buffer, F_OK) == 0)
        {
            printf("FOUND IT!\n");
            if(access(buffer, X_OK) == 0){
                printf("IT IS GOOD TO EXECUTE\n\n");
                int pid = fork();
                if (pid == 0)
                {
                    execv(buffer, argv);
                }
                else
                {
                    waitpid(pid, NULL, 0);
                }
                condition = true;
                return 0;
            }
            else{
                printf("that isn't executable\n");
            }
        }
        else{
            printf("there was a probem finding it... \n");
            printf("\nCurrent BUFFER: \n");
            printf("%s\n", buffer);
        }
        
        //realloc(colon, length + 10);
        colon = strtok(NULL, ":");
        printf(colon);
    }//end while loop
    return 1;
}

//PIPING
void pipeHandler()
{
    int pfds[2];
    pipe(pfds);

    int pid0 = 0, pid1 = 0;

    pid0 = fork();
    if (pid0 != 0)
    {
        //if pid0 returns error, fork pid1
        pid1 = fork();
    }
    if(pid0 == 0)
    {
        //child process (first command)
        close(1);
        dup(pfds[1]);   //duplicate
        close(pfds[0]);
        close(pfds[1]);

        //execv() //execute here... args in params

        //exit(1);

    }
    //second process id

    if(pid1 == 0)
    {
        close(0);
        dup(pfds[0]);
        close(pfds[0]);
        close(pfds[1]);

        //execv();
    }
    if(pid0 != 0 && pid1 != 0)
    {
        close(pfds[0]);
        close(pfds[1]);

        //waiting for child process to execute
        waitpid(pfds[0], pfds, pid0);
        waitpid(pfds[1], pfds, pid1);
    }
}
























































































