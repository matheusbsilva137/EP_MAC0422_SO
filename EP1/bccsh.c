#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/stat.h>

int main(){
    char* line, *command, *flagUsr, *op1, *op2, *username, *prompt;
    int status;
    char dir[150];
    pid_t c;

    while (1) {
        getcwd(dir, sizeof(dir));
        username = getenv("USERNAME");
        prompt = malloc(sizeof(username) + sizeof(dir) + 4);

        snprintf(prompt, sizeof(username) + sizeof(dir) + 4, "{%s@%s} ", username, dir);

        line = readline(prompt);
        add_history(line);
        while(isspace(*line)) line++;
        
        printf("%s\n", line);
        command = strtok(line, " ");
        
        if (strcmp("mkdir", command) == 0){
            op1 = strtok(NULL, " ");

            mkdir(op1, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        }else if( strcmp("kill", command) == 0){
            flagUsr = strtok(NULL, " ");
            op1 = strtok(NULL, " ");
            
            kill(atoi(op1), SIGKILL);
        }else if( strcmp("ln", command) == 0){
            flagUsr = strtok(NULL, " ");
            op1 =  strtok(NULL, " ");
            op2 = strtok(NULL, " ");
            
            symlink(op1, op2);
        }else{
            if ((fork()) != 0) {
                waitpid(-1, &status, 0);

                //uso do status
            } else {
                //funcionando apenas para o du
                op1 = strtok(NULL, " ");
                op2 = strtok(NULL, " ");
                char* args[] = {command, op1, op2, (char *)0};
                execve(command, args, (char*const*) 0);
            }
        }
        free(prompt);
    }
}