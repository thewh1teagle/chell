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
#include <limits.h>
#include <signal.h>


typedef struct ENV {
    char *username;
    char *hostname;

    void (*clean_up)(struct ENV *env);

} ENV;

void get_hostname(ENV *env) {
    env->hostname = malloc(_SC_HOST_NAME_MAX);
    gethostname(env->hostname, _SC_HOST_NAME_MAX);
}

void get_username(ENV *env) {
    env->username = getenv("USER");
    
}

void clean_up(ENV *env) {
    free(env->hostname);
}






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

int history() {
    FILE *fp;
    char line[255];
    fp = fopen(".history", "r");

    int line_n=1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        printf("%d %s", line_n, line);
        line_n++;
    }
    fclose(fp);
}

void history_handler() {
    history();
}

void save_to_history(char *command) {
    FILE *fp;
    fp = fopen(".history", "a");
    fprintf(fp, command);
    fclose(fp);
}


void exit_handler(char **argv) {
    if (strcmp("exit", argv[0]) == 0) {
        exit(0);
    }
}

int built_in_commands_handler(char **argv) {

    if (cd_handler(argv) == 0) {
        return 0;
    }
    if (strcmp(argv[0], "history") == 0) {
        history_handler();
        return 0;
    }

    exit_handler(argv);
}


void clean_up_and_exit(ENV *env, char **argv) {
    system("clear");
    printf("chell is exiting.. byee :) \n");
    exit(0);
}


int main() {
    signal(SIGINT, clean_up_and_exit);
    ENV env;
    env.clean_up = clean_up;
    get_hostname(&env);
    get_username(&env);
    do {
        printf("\033[0;32m");
        printf("%s@%s: $ ", env.username, env.hostname);
        printf("\033[0m");
        char read_line[256];
        fgets(read_line, 256, stdin);
        if (strlen(read_line) == 1) {
            continue;
        }
        save_to_history(read_line);
        read_line[strlen(read_line) - 1] = '\0'; // remove new line 
        char **argv = parse_command(read_line);
        if (built_in_commands_handler(argv)) {
            execute(argv);
        }
        free(argv);
    } while (1);
    env.clean_up(&env);
    return 0;
}



