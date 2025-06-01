
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 100

// modularizo para simpllificar
int split_commands(char *line, char *commands[]) {
    int count = 0;
    char *token = strtok(line, "|");
    while (token != NULL && count < MAX_COMMANDS) {
        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && (*end == ' ')) {
            *end = '\0';
            end--;
        }
        commands[count++] = token;
        token = strtok(NULL, "|");
    }
    return count;
}

int parse_args(char *command, char *args[]) {
    int arg_count = 0;
    char *token = strtok(command, " ");
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;
    return arg_count;
}

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count;

    while (1) 
    {   
        // agrego para usar el tester
        if (isatty(STDIN_FILENO)){
            printf("MyShell> ");
        }
        
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }
        
        command[strcspn(command, "\n")] = '\0';

        command_count = split_commands(command, commands);

        if (command_count == 0) {
            continue;
        }

        int pipefd[command_count - 1][2];

        for (int i = 0; i < command_count - 1; i++) {
            if (pipe(pipefd[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < command_count; i++) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) { 
                if (i > 0) {
                    dup2(pipefd[i - 1][0], STDIN_FILENO);
                }

                if (i < command_count - 1) {
                    dup2(pipefd[i][1], STDOUT_FILENO);
                }

                for (int j = 0; j < command_count - 1; j++) {
                    close(pipefd[j][0]);
                    close(pipefd[j][1]);
                }

                char *args[MAX_ARGS];
                parse_args(commands[i], args);

                if (execvp(args[0], args) == -1) {
                    perror("execvp error");
                    exit(EXIT_FAILURE);
                }
            }
        }

        for (int i = 0; i < command_count - 1; i++) {
            close(pipefd[i][0]);
            close(pipefd[i][1]);
        }

        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }
    }

    return 0;
}
