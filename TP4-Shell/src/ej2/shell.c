// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <string.h>

// #define MAX_COMMANDS 200
// #define MAX_ARGS 100

// int main() {
//     char command[256];
//     char *commands[MAX_COMMANDS];
//     int command_count;

//     while (1) 
//     {
//         if (isatty(STDIN_FILENO)){
//             printf("Shell> ");
//         }
        
//         if (!fgets(command, sizeof(command), stdin)) {
//             break;
//         }
        
//         command[strcspn(command, "\n")] = '\0';

//         // separor comandos por pipe
//         command_count = 0;
//         char *token = strtok(command, "|");
//         while (token != NULL && command_count < MAX_COMMANDS) 
//         {
//             // Quitar espacios al inicio y fin
//             while (*token == ' ') token++;
//             char *end = token + strlen(token) - 1;
//             while (end > token && (*end == ' ')) {
//                 *end = '\0';
//                 end--;
//             }

//             commands[command_count++] = token;
//             token = strtok(NULL, "|");
//         }

//         if (command_count == 0) {
//             continue; // Nada que hacer
//         }

//         // Pipes: para n comandos, necesitamos n-1 pipes
//         int pipefd[command_count - 1][2];

//         for (int i = 0; i < command_count - 1; i++) {
//             if (pipe(pipefd[i]) == -1) {
//                 perror("pipe");
//                 exit(EXIT_FAILURE);
//             }
//         }

//         for (int i = 0; i < command_count; i++) {
//             pid_t pid = fork();
//             if (pid == -1) {
//                 perror("fork");
//                 exit(EXIT_FAILURE);
//             }

//             if (pid == 0) { 
//                 // Si no es el primer comando, lee del pipe anterior
//                 if (i > 0) {
//                     dup2(pipefd[i - 1][0], STDIN_FILENO);
//                 }

//                 // Si no es el último comando, escribe en el pipe actual
//                 if (i < command_count - 1) {
//                     dup2(pipefd[i][1], STDOUT_FILENO);
//                 }

//                 // Cerrar todos los pipes que no se usan
//                 for (int j = 0; j < command_count - 1; j++) {
//                     close(pipefd[j][0]);
//                     close(pipefd[j][1]);
//                 }

//                 // Parsear los argumentos del comando i
//                 char *args[MAX_ARGS];
//                 int arg_count = 0;
//                 char *arg = strtok(commands[i], " ");
//                 while (arg != NULL && arg_count < MAX_ARGS - 1) {
//                     args[arg_count++] = arg;
//                     arg = strtok(NULL, " ");
//                 }
//                 args[arg_count] = NULL;

//                 // Ejecutar el comando
//                 if (execvp(args[0], args) == -1) {
//                     perror("execvp");
//                     exit(EXIT_FAILURE);
//                 }
//             }
//             // Proceso padre sigue para crear el siguiente hijo
//         }

//         // Proceso padre: cerrar todos los pipes
//         for (int i = 0; i < command_count - 1; i++) {
//             close(pipefd[i][0]);
//             close(pipefd[i][1]);
//         }

//         // Esperar a todos los hijos
//         for (int i = 0; i < command_count; i++) {
//             wait(NULL);
//         }
//     }

//     return 0;
// }
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
        if (isatty(STDIN_FILENO)){
            printf("Shell> ");
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
                    perror("execvp");
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
