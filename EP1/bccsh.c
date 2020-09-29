#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(){
    char* line, *command, *flag, *op1, *op2;

    while (1) {
        line = readline("(<user> <diretorios>)");
        while(isspace(*line)) line++;

        
        
        printf("%s\n", line);
        command = strtok(line, " ");
        

        if (strcmp("mkdir", command) == 0){
            flag = strtok(NULL, " ");
            op1 = strtok(NULL, " ");
            
        }else if( strcmp("kill", command) == 0){
            flag = strtok(NULL, " ");
            op1 = strtok(NULL, " ");
            
            
            kill(atoi(op1),SIGKILL);
        }else if( strcmp("ln", command) == 0){
            flag = strtok(NULL, " ");
        }
        /*if (fork() != 0) {
            Codigo do pai
        } else {
             Codigo do filho
            //execve(command,parameters,0);
        }*/
    }
}