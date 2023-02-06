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

//________________________________DECLARATIONS______________________________
char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

void tildeExpansion(tokenlist *tokens);
int EnvironmentVars();
void prompt();
void pipeHandler(tokenlist *tokens);
char * pathSearch(tokenlist *tokens);

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
    
    //if arrow call function, then pass in token list
    
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
        
        //PIPING
        for (int i = 0; i < tokens->size; i++)
        {
            if (strchr(tokens->items[i], '|') != NULL)
            {
                pipeHandler(tokens);
            }
        }

        //pathSearch(tokens);


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



//__________________________________________________________________________________


//__________________________________________________________________________________
//                                   ENVIRONMENTAL VARIABLES

/* This function replaces every token that starts with a dollar sign
 * character and replaces it with its corresponding value
 */
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


//__________________________________________________________________________________
//                                      PROMPT

/* This function indicates the working directory, the user name, and the machine name
 * The format will print: USER@MACHINE : PWD >
 */
void prompt(){
    char * getenv(const char *name);
    char * user = getenv("USER");
    char * machine = getenv("MACHINE");
    char * pwd = getenv("PWD");

    printf("%s""%c""%s"" ""%c"" ""%s"">", user, 64, machine, 58, pwd);


}


//new path search() feb 3. 11:09
//External execution

char * pathSearch(tokenlist * tokens){
//----------------ECHO-----------------------------
    int commandPos = -1;
    
    int before = 0;
    int after = 0;
    
    for(int i = 0; i < tokens->size; i++){
        
        if(strcmp(tokens->items[0], "echo") == 0){
            
            //dealing with env vars
            char dollar = tokens->items[i][0];
            if(dollar == '$'){
                //the command will be stored here
                char * doCommand = getenv(tokens->items[i] + 1);
                commandPos = i; //lets me know which token has a command
                
                after = tokens->size;
                for (int u = 1 ; u < commandPos; u++){
                    printf("%s ", tokens->items[u]);
                }
                //getenv(tokens->items[commandPos]);
                printf("%s ", doCommand);  //call the $ command between the regular text
                for(int a = commandPos + 1; a < after; a++){
                    printf("%s ", tokens->items[a]);
                }
                printf("\n");
            }
        }
    }
    
    //----------ENV_VARS-------------------------------
    for (int i = 0; i < tokens->size; i++){

        char dollar = tokens->items[i][0];

        if(dollar == '$'){

            char * doCommand = getenv(tokens->items[i] + 1);
            printf("%s", doCommand, "\n");
            printf("\n");
        }
        
    }
    //----------------------BUILT_IN_COMMANDS--------------------------
    char * argv[tokens->size];
    
    for(int i = 0; i < tokens->size; i++){
        argv[i] = tokens->items[i];
    }
    
    argv[tokens->size + 1] = NULL;
    
 
    char * temp_Path= getenv("PATH");
    char * copyPath = strdup(temp_Path);
    
    char * colon = strtok(copyPath, ":");
    
    char * bufHit;
    while(colon != NULL)
    {
        
        unsigned int length = strlen(colon) + strlen(argv[0]) + 2;
        char buffer[length];
        strcpy(buffer, colon);
        
        strcat(buffer, "/");
        strcat(buffer, argv[0]);
        
        
        //do access check
        if (access(buffer, F_OK) == 0)
        {
            printf("\nBUFFER: %s\n", buffer);
            if(access(buffer, X_OK) == 0)
            {
                printf("IT IS GOOD TO EXECUTE\n\n");
                int pid = fork();
                if (pid == 0)
                {
                    printf("We got to execv\n");
                    execv(buffer, argv);
                    printf("we got after exec\n");
                }
                else
                {
                    waitpid(pid, NULL, 0);
                }
                return buffer;
            }
            else
            {
                printf("that isn't executable\n");
            }
            //bufHit = malloc(strlen(buffer) + 1);
            //strcpy(bufHit, buffer);
        }
        else
        {
            printf("there was a problem finding it... \n");
        }
       
        
        colon = strtok(NULL, ":");
        
        
    }//end while loop
    
    //temp
    return 1;
}

//PIPING
void pipeHandler(tokenlist * tokens)
{
    printf("\nWE ARE IN THE PIPE FUNCTION!!\n");
    //parse commands into separate tokens then get buffer with path
    char * argv1[tokens->size];
    int index = 0;
    for(int i = 0; i < tokens->size; i++)
    {
        if (tokens->items[i] == "|")
        {
            //index = i;
            break;
        }
        argv1[i] = tokens->items[i];
        index++;
    }
    argv1[index + 1] = NULL;
    printf("\nPRINTING ARGV1\n");
    for(int i = 0; i < index; i++)
    {
        printf(argv1[i]);
    }
    
    tokenlist * cmd1Tokens = get_tokens(argv1);

    char * argv2[tokens->size];
    index++;
    int begin = 0;
    for(int i = index; i < tokens->size; i++)
    {
        argv2[0] = tokens->items[i];
        begin++;
    }
    argv2[begin + 1] = NULL;

    char * buffer;
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

        buffer = pathSearch(argv1);
        printf(buffer);
        execv(buffer, argv1);


    }
    //second process id

    if(pid1 == 0)
    {
        close(0);
        dup(pfds[0]);
        close(pfds[0]);
        close(pfds[1]);

        buffer = pathSearch(argv2);
        printf(buffer);
        execv(buffer, argv2);
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
//cd (change directory)
int cd2(tokenlist * tokens){
    
    char * target; //stores directory we want to change to
    for(int i = 0; i < tokens->size; i++){
        if(strcmp(tokens->items[i], "cd") == 0){
            target = tokens->items[i + 1];
        }
    }

    //2 step process, chdir and $PWD
    char * path= getenv("PATH");
    char *  cwd = getcwd(NULL, 0);
    
    if(chdir(target) == 0){ //successful
        getcwd(path, strlen(path));
        setenv("PWD", target, 1); //1 means overwrite
    }
    else{
        perror("WE are getting stuck here...");
    }
    
    //setenv("PWD", cwd, 1);
    free(cwd); //they say this is important...


    //temporary
    return 1;
}





//___________________________background processing text parsing___________________________________

//send to background,
    //when execution starts: print [Job number] [the command's PID]
    //when execution ends: print [Job number] [the command's command line]

    
    //getting all the commands we want to run in the background into the list 'cmdList'
    int cmdCounter = 1;
    char * cmdList[3];
    cmdList[0] = "";
    cmdList[1] = "";
    cmdList[2] = "";
    
    for(int i = 0; i < tokens->size; i++){
        if(strcmp(tokens->items[i], "&") == 0){ //means we have to make whole line background processes
            
            cmdList[0] = tokens->items[i - 1];  //if there is 1 cmd
            
            if(cmdCounter == 1){
                cmdList[0] = tokens->items[i -1];
                cmdList[1] = NULL;
                cmdList[2] = NULL;
            }
            if(cmdCounter == 2){
                cmdList[0] = tokens->items[0];
                cmdList[1] = tokens->items[i - 1];
                cmdList[2] = NULL;
            }
            if(cmdCounter == 3){
                if(strcmp(tokens->items[i], "|") == 0){
                    cmdList[1] = tokens->items[i - 1];
                }
                cmdList[0] = tokens->items[0];
                cmdList[2] = tokens->items[i -1];
            }
            
            //getenv(tokens->items[i - 1]);
            //pid_t waitpid(pid_t pid, int *stat_loc, int options);
            //WNOHANG - waitpid returns immediately with finish status
        }
        if(strcmp(tokens->items[i], "|") == 0){ //need to make every command on the line exec in the background
            cmdCounter++;
            cmdList[1] = tokens->items[i - 1];
            if(cmdCounter > 2){
                cmdList[2] = tokens->items[i + 3];
            }
            
        }
        
        
        
    }

    for(int u = 0; u < 3; u++){
        printf("List of background commands: %d, %s\n", u, cmdList[u]);
    }

//need to do waitpid/execute all the things inside of cmdList[]

//___________________________background processing text parsing___________________________________















































































