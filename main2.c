#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

char* nsh_read_line(){
#define NSH_RL_BUFSIZE 1024
    int bufsize = NSH_RL_BUFSIZE;
    int position = 0;
    char* buffer = malloc(sizeof(char)*bufsize);
    int c;

    if(!buffer){
        fprintf(stderr,"nsh : allocation error!\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        c = getchar();

        if(c == EOF){
            exit(EXIT_FAILURE);
        }
        else if(c == '\n'){
            buffer[position] = '\0';
            return buffer;
        }
        else{
            buffer[position] = c;
        }
        position++;

        if(position >= bufsize){
            bufsize += NSH_RL_BUFSIZE;
            buffer = realloc(buffer,bufsize);
            if(!buffer){
                fprintf(stderr,"nsh : allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char** nsh_split_line(char* line,int* num_comms){
#define NSH_COMM_BUFSIZE 64
#define NSH_COMM_DELIM "|"

    int bufsize = NSH_COMM_BUFSIZE;
    char** tokens = malloc(sizeof(char*) * bufsize);
    int position = 0;
    char* token;

    if(!tokens){
        fprintf(stderr,"nsh : allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line,NSH_COMM_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;
        
        token = strtok(NULL,NSH_COMM_DELIM);
    }
    tokens[position] = NULL;
    *num_comms = position;
    return tokens;
}

char*** nsh_split_comms(char** comms,int* num_comms){
#define NSH_ARGS_BUFSIZE 64
#define NSH_ARGS_DELIM " \t\r\n\a"

    int bufsize = NSH_ARGS_BUFSIZE;
    char*** args = malloc(sizeof(char**) * (*num_comms + 1));
    for(int i = 0; comms[i] != NULL ; ++i){
        char** comm = malloc(sizeof(char *) * bufsize);
        int position = 0;
        char* token;

        if(!comm){
            fprintf(stderr,"nsh : allocation error");
            exit(EXIT_FAILURE);
        }

        token = strtok(comms[i],NSH_ARGS_DELIM);
        while(token != NULL){
            comm[position] = token;
            position++;

            token = strtok(NULL,NSH_ARGS_DELIM);
        }
        comm[position] = NULL;
        args[i] = comm;
    }
    args[*num_comms] = NULL;

    return args;
}

void nsh_loop(){
    char* line;
    char** comms;
    int num_comms;
    char*** args;
    int status;

    do{
        printf(">");
        line = nsh_read_line();
        comms = nsh_split_line(line,&num_comms);
        args = nsh_split_comms(comms,&num_comms);
        status = lsh_execute(args);

        free(line);
        free(comms);

    }
    while(status);
}

int main(int argc,char** argv){

    nsh_loop();

    return EXIT_SUCCESS;
}