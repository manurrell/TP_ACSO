#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <num_processes> <initial_value> <start_index>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int mensaje_inicial = atoi(argv[2]);
    int start = atoi(argv[3]);

    printf("%d %d %d\n", n, mensaje_inicial, start);

    int pipes[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    // Pipe extra
    int return_pipe[2];
    if (pipe(return_pipe) == -1) {
        perror("pipe_r");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(1);
        }

        if (pid == 0) {
            int buffer;
            int prev = (i - 1 + n) % n;

            // cerrar pipes
            for (int j = 0; j < n; j++) {
                if (j != prev) close(pipes[j][0]);
                if (j != i) close(pipes[j][1]);
            }
            close(return_pipe[0]); 


            ssize_t res = read(pipes[prev][0], &buffer, sizeof(int));
            if (res != sizeof(int)) {
                perror("read hijo");
                exit(1);
            }
            // debug print
            printf("Hijo: %d msj recibido: %d\n", i, buffer);

            buffer++; // Incrementar el msj

            // Si es el proceso anterior al start (ultimo), enviar a padre
            if (i == (start - 1 + n) % n) {
                ssize_t w = write(return_pipe[1], &buffer, sizeof(int));
                if (w != sizeof(int)) {
                    perror("write return_pipe");
                    exit(1);
                }
            } else {
                // Sino enviar al siguiente proceso
                ssize_t w = write(pipes[i][1], &buffer, sizeof(int));
                if (w != sizeof(int)) {
                    perror("write hijo");
                    exit(1);
                }
            }

            // Cerrar pipes usados
            close(pipes[prev][0]);
            close(pipes[i][1]);
            close(return_pipe[1]);

            exit(0);
        }
    }

    // cierro los pipes que no usa
    for (int i = 0; i < n; i++) {
        close(pipes[i][0]);
        if (i != ((start - 1 + n) % n))
            close(pipes[i][1]);
    }
    close(return_pipe[1]);

    int prev = (start - 1 + n) % n;

    // Enviar mensaje inicial
    ssize_t w = write(pipes[prev][1], &mensaje_inicial, sizeof(int));
    if (w != sizeof(int)) {
        perror("write padre");
        exit(1);
    }
    close(pipes[prev][1]);

    int resultado;
    ssize_t r = read(return_pipe[0], &resultado, sizeof(int));
    if (r != sizeof(int)) {
        perror("read padre");
        exit(1);
    }
    close(return_pipe[0]);

    printf("Resultado final recibido por el padre: %d\n", resultado);

    // Esperar a hijos
    for (int i = 0; i < n; i++){
        wait(NULL);
    } 

    return 0;
}

