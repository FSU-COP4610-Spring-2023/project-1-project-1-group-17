#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include <dirent.h>




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
void EnvironmentVars();
void prompt();
void InputOutputRedirection(tokenlist * tokens);
void Exit(tokenlist * tokens);
void cd_command (tokenlist * tokens);
void pipeHandler(tokenlist *tokens);
char * pathSearch(tokenlist *tokens);
void background_processing(tokenlist * tokens);


#define MAX_JOBS 100
int job_num = 0;

struct job {
    int id;
    pid_t pid;
    char cmd[1000];
    pid_t status;
};
void add_job(pid_t pid, char* cmd);
void remove_job(int id);
void jobs();

struct job activeJobs[MAX_JOBS]; //array of type job to store the active background processes

void add_job(pid_t pid, char* cmd) {
    activeJobs[job_num].id = ++job_num;
    activeJobs[job_num].pid = pid;
    activeJobs[job_num].status = 0; // waitpid(pid, NULL, WNOHANG);
    strcpy(activeJobs[job_num].cmd, cmd);
}


void remove_job(int id){
    
    for(int i = 0; i < job_num; i++) {
        if (activeJobs[i].id == id) {
            for (int j = 1; j < job_num - 1; j++) {
                activeJobs[j] = activeJobs[j + 1];
            }
            job_num--;
            break;
        }
    }
}

void jobs(){
    int i = 0;
    while (i < job_num){
        int status = waitpid(activeJobs[i].pid, &(activeJobs[i].status), WNOHANG | WUNTRACED);
        if(status == -1 || WIFEXITED(activeJobs[i].status) || WIFSIGNALED(activeJobs[i].status)) {
            remove_job(activeJobs[i].id);
        } else {
            printf("[%d] %d %s\n", activeJobs[i].id, activeJobs[i].pid, activeJobs[i].cmd);
            i++;
        }
    }
}


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

//CORRECTION: handles tilde '~' alone 
void tildeExpansion(tokenlist *tokens)
{
    for(int i = 0; i < tokens->size; i++)
    {
        if(strchr(tokens->items[i], '~') != NULL)
        {
            char * copy = strchr(tokens->items[i], '/');
            char copyTwo[strlen(copy)];
            strcpy(copyTwo, copy);

            unsigned int length = strlen(copy) + strlen(getenv("HOME"));

            tokens->items[i] = (char*) realloc(tokens->items[i], length * sizeof(char));
            
            //checking if second char of token is null, this indicates standalone tilde 
            if (tokens->items[i][1] == '\0') { //handles standalone tilde 
                strcpy(tokens->items[i], getenv("HOME")); 
            }
            else {
                strcpy(tokens->items[i], getenv("HOME"));
                strcat(tokens->items[i], copyTwo);
                
            }
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

        tokenlist *tokens = get_tokens(input);

        //Tilde Expansion
        for (int i = 0; i < tokens->size; i++)
        {
            if((strchr(tokens->items[i], '~') != NULL) && (strcmp(tokens->items[i- 1], "cd") != 0))
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

        background_processing(tokens);
        cd_command(tokens);
        pathSearch(tokens);


        for (int i = 0; i < tokens->size; i++) {
            //printf("token %d: (%s)\n", i, tokens->items[i]); (excessive printing, don't need)
            if (tokens->size > 0 && strcmp(tokens->items[i], "exit") == 0) {
                Exit(tokens);
            }

            //JOBS: added correction
            if (strcmp(tokens->items[i], "jobs") == 0){
                jobs();
                continue;
            }
        }
        
        
        EnvironmentVars(tokens);
        InputOutputRedirection(tokens);
        
        free(input);
        free_tokens(tokens);
        
        
       
    }
return 0;
}



//__________________________________________________________________________________


//__________________________________________________________________________________
//                                   ENVIRONMENTAL VARIABLES

/* This function replaces every token that starts with a dollar sign
 * character and replaces it with its corresponding value
 */

//CORRECTION: prints only one value for each env var, instead of three 
void EnvironmentVars(tokenlist *tokens){

    for (int i = 0; i < tokens->size; i++){

        char dollar = tokens->items[i][0];

        if(dollar == '$'){

            char * doCommand = getenv(tokens->items[i] + 1);
            if (doCommand != NULL){
                printf("%s\n", doCommand);
            }
            else {
                printf("%s ", tokens->items[i]); 

            }
        }
    }
    printf("\n"); 

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


//              new path search() feb 3. 11:09
//                  External execution
//------------------------------------------------------------
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
    
    //argv[tokens->size + 1] = NULL; //CHANGED
    //should fix issue on single commands w/ no arguments and satisfy point deduction
    argv[tokens->size] = NULL;
    
 
    char * temp_Path= getenv("PATH");
    char * copyPath = strdup(temp_Path);
    
    char * colon = strtok(copyPath, ":");
    
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
            if(access(buffer, X_OK) == 0)
            {
                int pid = fork();
                if (pid == 0)
                {
                    execv(buffer, argv);
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
        }
       
        colon = strtok(NULL, ":");
        
        
    }//end while loop
    
    return 1;
}

//PIPING
void pipeHandler(tokenlist * tokens)
{
    //parse commands into separate tokens then get path with pathSearch
    char * argv1[tokens->size];
    int index = 0;
    for(int i = 0; i < tokens->size; i++)
    {
        if (strcmp(tokens->items[i], "|"))
        {
            //index = i;
            break;
        }
        argv1[i] = tokens->items[i];
        index++;
    }
    argv1[index + 1] = NULL;
    for(int i = 0; i < index; i++)
    {
        printf(argv1[i]);
    }
    

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
        waitpid(pid0, NULL, 0);
    }
}




//___________________________background processing text parsing___________________________________

//send to background,
    //when execution starts: print [Job number] [the command's PID]
    //when execution ends: print [Job number] [the command's command line]

void background_processing(tokenlist *tokens)
{
    
    //getting all the commands we want to run in the background into the list 'cmdList'
    int cmdCounter = 1;
    char *cmdList[3];
    for(int i = 0; i < tokens->size; i++)
    {
        if(strcmp(tokens->items[i], "&") == 0){ //means we have to make whole line background processes
            
            cmdList[0] = tokens->items[i - 1];  //if there is 1 cmd
            //strcpy(cmdList[0], tokens->items[i-1]);
            
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
        
    }//end for loop


    pid_t pids[3];
    int status;
    for(int u = 0; u < 3; u++){
        if(cmdList[u] != NULL){
            pids[u] = fork();
            if(pids[u] == 0)
            {
                //child process
                //execv();
                add_job(pids[u], cmdList[u]); 
            }
            else{
                //parent process
                waitpid(pids[u], &status, WNOHANG);//make it run in the background
                //add_job(pids[u], cmdList[u]); //add it to list of processes running
            }
        
        }
    }//end for loop
    
    
    
    //jobs built-in stuff....
    
    for(int j = 0; j< tokens->size; j++){
        
        if(strcmp(tokens->items[j], "jobs") == 0){
            printf("%d  %d  %s",activeJobs[j].id, activeJobs[j].pid, activeJobs[j].cmd);
            
            if(activeJobs[j].status != 0){ //that process is finished
                remove_job(activeJobs[j].id);
            }
        }
        
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

void InputOutputRedirection(tokenlist * tokens) {

    char * filename;
    int fd; //output
    int status;
    char buffer[200]; //stores string
    int bufferSize = 0;

    waitpid(-1, NULL, 0); //waits for any background processing to finish

    int begintext = -1;
    int endtext = -1;
    for (int i = 0; i < tokens->size; i++){
        if(strcmp(tokens->items[i], "cmd") == 0){
            begintext = i + 1;
        }
        if(strcmp(tokens->items[i], ">") == 0 || (tokens->items[i], "<") == 0){
            endtext = i - 1;
            filename = tokens->items[i+1];
        }
    }

    int tokenLen = 0;
    //storing the text that we want to send to the file...
    for(int j = begintext; j < endtext + 1; j++){

        tokenLen = strlen(tokens->items[j]);
       
       if (bufferSize + tokenLen >= sizeof(buffer)){
            break;
        }
        memcpy(buffer + bufferSize, tokens->items[j], tokenLen);
        bufferSize += tokenLen;
        buffer[bufferSize++] = ' ';
    }
    buffer[bufferSize] = '\0';


    fd = creat(filename, 0644); // open the file for writing
    write(fd, buffer, bufferSize); // write the contents of buffer to the file


    pid_t pid = fork(); // forking
    if (pid == 0){
        close(fd); // closing the file descriptor
        char * path = getenv("PATH"); // getting path
        execv(path, buffer);
    }
    else {
        close(fd);
        waitpid(pid, status, 0); //waiting for parent process
    }
    

}


//must wait for all background processes
//CORRECTED 
void Exit(tokenlist * tokens){

/*
    int valid_cmd_count = 0;
    char * argv1[tokens->size];
    char * valid_commands[3];

    

    //wait for background process to finish
    waitpid(-1, NULL, 0);
    
    int valid;
    for (int i = 0; i < tokens->size; i++) {
        argv1[i] = tokens->items[i];

        valid = system(argv1[i]);

        if (valid == 0) {//if command is valid, store it in our little array of valid_commands[]
            valid_commands[valid_cmd_count % 3] = argv1[i];
            valid_cmd_count++;
        }
    }

    if (valid_cmd_count == 3){

        printf("The following were the last three valid commands executed: \n");
        printf("1. %s", valid_commands[(valid_cmd_count - 1) % 3]);
        printf("\n"); // want to store oldest command here
        printf("2. %s", valid_commands[(valid_cmd_count - 2) % 3]);
        printf("\n");
        printf("3. %s", valid_commands[(valid_cmd_count - 3) % 3]);
        printf("\n");
    }
    else if (valid_cmd_count < 3){
        printf("The last valid command executed was: %s\n",valid_commands[(valid_cmd_count - 1) % 3]);
        printf("\n");
    }
    else
    {
        printf("No valid commands were executed in this shell \n");
        printf("\n");
    }
*/ 

    //wait for background process to finish
    waitpid(-1, NULL, 0);

    int valid_cmd_count = 0;
    char *valid_commands[3];

    // Store valid commands in array
    for (int i = 0; i < tokens->size; i++) {
        int valid = system(tokens->items[i]);
        if (valid == 0) {//if command is valid, store it in our little array of valid_commands[]
            valid_commands[valid_cmd_count % 3] = tokens->items[i];
            valid_cmd_count++;
        }
    }

    if (valid_cmd_count > 0) {
        printf("The last valid command(s) executed were:\n");
        for (int i = 0; i < valid_cmd_count; i++) {
            printf("%d. %s\n", i+1, valid_commands[i]);
        }
    } else {
        printf("No valid commands were executed in this shell\n");
    }
}



void cd_command (tokenlist * tokens)
{
    
    char * targetDirectory;
    char * path = getenv("HOME"); //path is home by default
    char *home = getenv("HOME");
    char * pwd = getenv("PWD");


    for(int i = 0; i < tokens->size; i++){
        if (strcmp(tokens->items[i], "cd") == 0){
            if(tokens->size > 2){
                printf("Error: More than one argument is present.\n");
            }
            
            targetDirectory = tokens->items[i+1];
            
            chdir(targetDirectory); //go to specified directory
            
            if ((strcmp(tokens->items[i + 1], "~") == 0) ||
                (strcmp(tokens->items[i + 1], " ") == 0)){ //if we are given ~ or " "after cd, go home
                chdir(home);
            }
            else if(strcmp(tokens->items[i + 1], "..") == 0){ //if .. follows cd, go back a directory
                //chdir("..");
                printf("");
            }
        }//end cd check
    }//end for loop
    


}
