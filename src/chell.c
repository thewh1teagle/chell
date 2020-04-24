#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/wait.h> 
#include <readline/readline.h> 
#include <readline/history.h> 
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>


char **split(char string[], char *sep) {
    char *token = strtok(string, sep);
    char **argv = calloc(1, sizeof(char*));
    
    int i = 0;
    while (token != NULL) {
        argv = realloc(argv, sizeof(argv) + sizeof(char*));
        argv[i] = calloc(strlen(token), sizeof(char));
        strcpy(argv[i++], token);
        token = strtok(NULL, sep);
    }
    return argv;
}


char **parse_command(char command[]) {
    char **argv = split(command, " ");
    return argv;
}


void execute(char **argv) {
    pid_t  pid;
    int    status;

    if ((pid = fork()) < 0) {     // fork a child process          
        printf("*** ERROR: forking child process failed \n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {          // for the child process:         
        if (execvp(*argv, argv) < 0) {     // execute the command  
            printf("chell: %s: command not found \n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else {                                  // for the parent:      
        while (wait(&status) != pid);       // wait for completion  
    }
}



int cd(char *pth) {
    char path[256];
    strcpy(path,pth);

    char cwd[256];
    if(pth[0] != '/') {// true for the dir in cwd
        getcwd(cwd,sizeof(cwd));
        strcat(cwd,"/");
        strcat(cwd,path);
        chdir(cwd);
    } else { //true for dir w.r.t. /
        chdir(pth);
    }
}


int cd_handler(char **argv) {
    if (strcmp("cd", argv[0]) == 0) {
        if (argv[1]) {
            char *tempTok = argv[1];
                argv[1] = tempTok;
                char *locationOfNewLine = strchr(argv[1], '\n');
                if(locationOfNewLine) {
                    *locationOfNewLine = '\0';
                }
                cd(argv[1]);
        }
        return 0;
    }
}


void exit_handler(char **argv) {
    if (strcmp("exit", argv[0]) == 0) {
        exit(0);
    }
}

int built_in_commands_handler(char **argv) {
    cd_handler(argv);
    exit_handler(argv);
}


int main() {
    do {
        printf("$ ");
        char read_line[256];
        fgets(read_line, 256, stdin);
        read_line[strlen(read_line) - 1] = '\0'; // remove new line 
        char **argv = parse_command(read_line);
        if (built_in_commands_handler(argv)) {
            execute(argv);
        }
    } while (1);
    return 0;
}



