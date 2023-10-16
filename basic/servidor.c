#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#include "server.h"

#define MESSAGE_SIZE 128
#define DEFAULT_PORT 8000
#define DEFAULT_BACKLOG 16

static int loop;    /* Para saber si seguimos iterando o hay que parar */

/**
 * Process the command line inputs given to main
 * @param argc  Number of arguments.
 * @param argv  Array of strings with the program arguments.
 * @return void
 */
void process_args(int argc, char** argv, uint16_t* port, int* backlog);

void handle_connection(Server server, Client client);

void signal_handler(int);   /* Función para gestionar las señales */

int main(int argc, char** argv) {
    Server server;
    Client client = {.socket = -1};
    int listen_socket;
    uint16_t port;
    int backlog;
    pid_t child;

    process_args(argc, argv, &port, &backlog);

    server = create_server(AF_INET, SOCK_STREAM, 0, port, backlog);

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("No se pudo cambiar la respuesta a la señal SIGTERM");
        exit(EXIT_FAILURE);
    }

    loop = 1;
    while (loop) {
        listen_socket = server.socket;
        server.socket = listen_for_connection(server, &client);

        child = fork();

        handle_connection(server, client);

        close(server.socket);
        server.socket = listen_socket;

        if (child == 0) loop = 0;   /* Que el hijo salga del bucle y termine */
    }

    close_server(&server);
    close_client(&client);

    exit(EXIT_SUCCESS);
}


void signal_handler(int signum) {
    loop = 0;   /* Hace que deje de iterar en el bucle */
}

void handle_connection(Server server, Client client) {
    char message[MESSAGE_SIZE] = {0};
    ssize_t transmited_bytes;

    snprintf(message, MESSAGE_SIZE, "Tu conexión al servidor %s en %s:%u ha sido aceptada\n", server.hostname, server.ip, server.port);

    if ( (transmited_bytes = send(server.socket, message, strlen(message) + 1, 0)) < 0) {
        perror("No se pudo enviar el mensaje");
        exit(EXIT_FAILURE);
    }
}


void print_help(void) {}


void process_args(int argc, char** argv, uint16_t* port, int* backlog) {
    int i;
    char* current_arg;

    /* Si no se pasan argumentos, avisar y poner las opciones por defecto */
    if (argc == 1) {        
        printf("Ejecutando el servidor con las opciones por defecto: PORT=%u, BACKLOG=%d\n\n", DEFAULT_PORT, DEFAULT_BACKLOG);
        *port = DEFAULT_PORT;
        *backlog = DEFAULT_BACKLOG;
        return;
    }

    for (i = 1; i < argc; i++) { /* Procesamos los argumentos (sin contar el nombre del ejecutable) */
        current_arg = argv[i];
        if (current_arg[0] == '-') { /* Flag de opción */
            switch(current_arg[1]) {
                case 'p':   /* Puerto */
                    if (++i < argc) {
                        *port = atoi(argv[i]);
                    } else {
                        fprintf(stderr, "Puerto no especificado tras la opción '-p'\n\n");
                        print_help();
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'b':   /* Backlog */
                    if (++i < argc) {
                        *backlog = atoi(argv[i]);
                    } else {
                        fprintf(stderr, "Tamaño del backlog no especificado tras la opción '-b'\n\n");
                        print_help();
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'h':
                    print_help();
                    exit(EXIT_SUCCESS);
                default:
                    fprintf(stderr, "Opción '%s' desconocida\n\n", current_arg);
                    exit(EXIT_FAILURE);
            }
        }
    }
}


