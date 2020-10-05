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
    char* line, *command, *flagUsr, *op1, *op2, *op3, *op4, *username, *prompt;
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
            fflush(stdout);
            if ((fork()) != 0) {
                waitpid(-1, &status, 0);
            } else {
                if (strcmp("./ep1", command) == 0){
                    op1 = strtok(NULL, " ");
                    op2 = strtok(NULL, " ");
                    op3 = strtok(NULL, " ");
                    op4 = strtok(NULL, " ");
                    if (op4){
                        char* args[] = {command, op1, op2, op3, op4, (char *)0};
                        execve(command, args, (char*const*)0);
                    }else{
                        char* args[] = {command, op1, op2, op3, (char *)0};
                        execve(command, args, (char*const*)0);
                    }
                }else if(strcmp("/usr/bin/du", command) == 0){
                    op1 = strtok(NULL, " ");
                    op2 = strtok(NULL, " ");

                    char* args[] = {command, op1, op2, (char *)0};
                    execve(command, args, (char*const*)0);
                }else if(strcmp("/usr/bin/traceroute", command) == 0){
                    op1 = strtok(NULL, " ");
                    
                    char* args[] = {command, op1, (char *)0};
                    execve(command, args, (char*const*)0);
                }  
            }
        }
        free(prompt);
    }
}