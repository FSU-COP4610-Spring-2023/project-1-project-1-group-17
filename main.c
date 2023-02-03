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

//________________________________DECLARATIONS______________________________
char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

void tildeExpansion(tokenlist *tokens);
int EnvironmentVars();
void prompt();

//__________________________________________________________


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


//__________________________________________________________________________________
//
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

void pathSearch(tokenlist * tokens){
    
    char * argv[tokens->size];
    //michael said to make sure argv is null terminated at the end... think this was the fix i needed
    argv[tokens->size + 1] = NULL;
    
    for(int i = 0; i < tokens->size; i++){
        argv[i] = tokens->items[i];
    }
    //int checker = 0;
    
 
    char * temp_Path= getenv("PATH");
    char * colon = strtok(temp_Path, ":");

    while(colon != NULL){
    
        unsigned int length = strlen(colon) + strlen(argv[0]) + 2;
        char buffer[length];
        strcpy(buffer, colon);
        
        strcat(buffer, "/");
        strcat(buffer, argv[0]);
        

        printf("\nBUFFER: \n");
        printf("%s\n", buffer); //printing to confirm it works, it does
        
        //do access check
    
        if (access(buffer, F_OK) == 0)
        {
            //printf("FOUND IT!\n");
            /*if(checker == 1)
            {
                return(0);
            }*/
            //checker ++;
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
                return 0;
            }
            else{
                printf("that isn't executable\n");
            }
        }
        else{
            printf("there was a probem finding it... \n");
        }
        
        colon = strtok(NULL, ":");
    }//end while loop
}

//background processing, this function might have to be connected to another one...
void background_processing(tokenlist * tokens){
    
    //background processes are called with a '&' behind the command.
    for(int i = 0; i < tokens->size; i++){

        char * amper;
        char * back_cmd;
        if(strcmp(tokens->items[i], "&") == 0){
            amper = tokens->items[i]; //store the amper
            
            back_cmd = tokens->items[i -1]; //store the background command
        }
    }
    
    // *** back_cmd is storing the command that we wish to run in the background ***
    
    
    
    pid_t waitpid(pid_t pid, int *stat_loc, int options);
    
    //WNOHANG - waitpid returns immediately with finish status
    
    pid_t pid; //temp
    pid_t status = waitpid(pid, NULL, WNOHANG);
    
    if(status != 0){ //process is finished
        printf("process is finished");
    }
    
}

//__________________________________________________________________________________
//                                      I/O REDIRECTION

/* I/O redirection from/to a file. Shell receives input from keyboard and writes output to
 * screen. Input redirection (<) will replace keyboard, Output (>) will replace
 * screen with specified file
 */

/*  fd  file
 *  0   keyboard (input)
 *  1   file (after dup)
 *  2   screen (output)
 *  3   file (then this will be closed, so only have 0,1,2)
 */

//it is creating the file, opening and closing
int InputOutputRedirection(int argc, char * argv[], tokenlist * tokens) {

    char * filename;
    pid_t pid = fork();
    int fd = open(filename, O_WRONLY | O_CREAT); //output
    int status;

    //------------------------------------------------------STORES & PRINTS FILENAME
    for (int i = 0; i < tokens->size; i++){
        if(strcmp(tokens->items[i], ">") == 0){
            filename = tokens->items[i+1];
        }
    }

    //------------------------------------------------------

    char * path = getenv("PATH");

    //child process
    if (pid == 0){
        //this will write only to file, and create file if it exists
        //if file does exist, O_CREAT will not be executed
        int fd = open(filename, O_WRONLY | O_CREAT); //output
        close(stdout);
        dup(fd);
        close(fd);
        execv(path, filename);

    }

    else {
        close(fd); //closing
        waitpid(pid, status, 0); //waiting for pid (parent process)

    }

    printf("\nFILENAME: %s\n", filename);

}






















































































