#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

int no_pipes_launch(char*** args){
    pid_t pid;

    pid = fork();
    if(pid == 0){
        if(execvp(args[0][0],args[0]) == -1){
            perror("nsh : error");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_FAILURE);
    }else if(pid < 0){
        perror("nsh");
    }else{
        waitpid(pid,NULL,0);
    }

    return 1;

}

int nsh_launch(char*** args,int* num){
    int num_pipes = *num - 1;
    if(num_pipes == 0){
        return no_pipes_launch(args);
    }
    int pipes[num_pipes][2];
    pid_t pids[*num];

    for(int i = 0 ; i < num_pipes ; ++i){
        if(pipe(pipes[i]) == -1){
            perror("nsh : pipe");
            return 1;
        }
    }

    for(int i = 0 ; i < *num ; ++i){
        pids[i] = fork();

        if(pids[i] == 0){
            if(i > 0){
                dup2(pipes[i-1][0],STDIN_FILENO);
            }

            if(i < *num - 1){
                dup2(pipes[i][1],STDOUT_FILENO);
            }

            for(int j = 0 ; j < num_pipes ; ++j){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(args[i][0],args[i]);

            perror("nsh");
            exit(EXIT_FAILURE);
        }

        if(pids[i] < 0){
            perror("nsh : fork");
            return 1;
        }
    }

    for(int i = 0 ; i < num_pipes ; ++i){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for(int i = 0 ; i < *num ; ++i){
        waitpid(pids[i],NULL,0);
    }

    return 1;
}

int nsh_execute(char*** args,int* num){
    int i;

    if(*num == 0) return 1; //empty command

    return nsh_launch(args,num);
}

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
            exit(EXIT_SUCCESS);
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
        printf("nsh shell >");
        line = nsh_read_line();
        comms = nsh_split_line(line,&num_comms);
        args = nsh_split_comms(comms,&num_comms);
        status = nsh_execute(args,&num_comms);

        for(int i = 0; args[i] != NULL ; ++i){
            free(args[i]);
        }
        free(args);
        free(line);
        free(comms);

    }
    while(status);
}

int main(int argc,char** argv){

    nsh_loop();

    return EXIT_SUCCESS;
}