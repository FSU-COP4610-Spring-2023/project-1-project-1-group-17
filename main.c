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

void tildeExpansion(tokenlist *tokens)
{
    for(int i = 0; i < tokens->size; i++)
    {
        char * copy;
        strcpy(tokens[i], copy);
        int d = 0;
        while(copy[d] != '\0')
        {
            if(copy[d] == '~')
            {
                char * home = getenv("HOME");
                strcat(home, copy);
                
            }
            d++;
        }
        //might cause memory leak
        
    }
}


int main()
{

    char *hello = malloc(strlen("Welcome to our shell!\n") + 1);
    strcpy(hello, "Welcome to our shell!\n");
    printf("%s", hello);
    free(hello);
    
    
    while (1) {
        printf("> ");

        /* input contains the whole command
         * tokens contains substrings from input split by spaces
         */

        char *input = get_input();
        printf("whole input: %s\n", input);

        tokenlist *tokens = get_tokens(input);
        for (int i = 0; i < tokens->size; i++) {
            printf("token %d: (%s)\n", i, tokens->items[i]);
        }
        
        //dealing with an ls
        int compare =strcmp (input, "ls");
        if(compare == 0){
            forker();
        }

        //Tilde Expansion
        for (int i = 0; i < tokens->size; i++)
        {
            if(strchr(tokens->items[i], '~') != NULL)
            {
                tildeExpansion(tokens);
            }
        }

        free(input);
        free_tokens(tokens);
        
        
       
    }
    
    return(0);

return 0;
}































































































