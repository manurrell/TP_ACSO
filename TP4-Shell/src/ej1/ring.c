#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Uso: anillo <n> <c> <s>\n");
        exit(1);
    }

    int n = atoi(argv[1]);
    int mensaje_inicial = atoi(argv[2]);
    int start = atoi(argv[3]);

    printf("Se crearán %d procesos, se enviará el caracter %d desde proceso %d\n",
           n, mensaje_inicial, start);
    fflush(stdout);

    int pipes[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    // Pipe extra para que el ultimo hijo envíe el resultado al padre
    int pipe_padre[2];
    if (pipe(pipe_padre) == -1) {
        perror("pipe_padre");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            int buffer;
            int prev = (i - 1 + n) % n;

            // Cerrar todos los pipes que no usa
            for (int j = 0; j < n; j++) {
                if (j != prev) close(pipes[j][0]);
                if (j != i) close(pipes[j][1]);
            }
            close(pipe_padre[0]); 

            // Leer mensaje de proceso anterior
            ssize_t r = read(pipes[prev][0], &buffer, sizeof(int));
            if (r != sizeof(int)) {
                perror("read hijo");
                exit(1);
            }

            printf("Hijo %d recibió: %d\n", i, buffer);
            fflush(stdout);

            buffer++; // Incrementar el msj

            // Si es el proceso anterior al start, enviar a padre
            if (i == (start - 1 + n) % n) {
                ssize_t w = write(pipe_padre[1], &buffer, sizeof(int));
                if (w != sizeof(int)) {
                    perror("write pipe_padre");
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
            close(pipe_padre[1]);

            exit(0);
        }
    }

    // PADRE cierra los extremos de los pipes que no usa
    for (int i = 0; i < n; i++) {
        close(pipes[i][0]);
        if (i != ((start - 1 + n) % n))
            close(pipes[i][1]);
    }
    close(pipe_padre[1]);

    int prev = (start - 1 + n) % n;

    // Enviar mensaje inicial
    ssize_t w = write(pipes[prev][1], &mensaje_inicial, sizeof(int));
    if (w != sizeof(int)) {
        perror("write padre");
        exit(1);
    }
    close(pipes[prev][1]);

    int resultado;
    ssize_t r = read(pipe_padre[0], &resultado, sizeof(int));
    if (r != sizeof(int)) {
        perror("read padre");
        exit(1);
    }
    close(pipe_padre[0]);

    printf("Resultado final recibido por el padre: %d\n", resultado);
    fflush(stdout);

    // Esperar a hijos
    for (int i = 0; i < n; i++){
        wait(NULL);
    } 

    return 0;
}

