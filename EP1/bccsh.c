#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>

int main(){
    char* line, *command, *flagUsr, *op1, *op2;
    pid_t c;

    while (1) {
        line = readline("(<user> <diretorios>)");
        while(isspace(*line)) line++;
        
        printf("%s\n", line);
        command = strtok(line, " ");
        
        if (strcmp("mkdir", command) == 0){
            flagUsr = strtok(NULL, " ");
            op1 = strtok(NULL, " ");
        }else if( strcmp("kill", command) == 0){
            flagUsr = strtok(NULL, " ");
            op1 = strtok(NULL, " ");

            kill(c, SIGKILL);
        }else if( strcmp("ln", command) == 0){
            flagUsr = strtok(NULL, " ");
        }
        /*if (fork() != 0) {
            Codigo do pai
        } else {
             Codigo do filho
            //execve(command,parameters,0);
        }*/
    }
}