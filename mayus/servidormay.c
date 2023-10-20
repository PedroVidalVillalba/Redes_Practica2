#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#include "server.h"

#define MESSAGE_SIZE 10000
#define DEFAULT_PORT 8000
#define DEFAULT_BACKLOG 16
#define MAX_BYTES_RECV 128

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
    Client client;
    uint16_t port;
    int backlog;

    process_args(argc, argv, &port, &backlog);

    server = create_server(AF_INET, SOCK_STREAM, 0, port, backlog);

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("No se pudo cambiar la respuesta a la señal SIGTERM");
        exit(EXIT_FAILURE);
    }

    loop = 1;
    while (loop) {
        listen_for_connection(server, &client);

        handle_connection(server, client);

        close_client(&client);
    }

    close_server(&server);

    exit(EXIT_SUCCESS);
}


void signal_handler(int signum) {
    loop = 0;   /* Hace que deje de iterar en el bucle */
}

void handle_connection(Server server, Client client) {
    char* output;
    char input[MAX_BYTES_RECV];
    ssize_t recv_bytes, sent_bytes;
    int i;

    while (1) {
        if ( (recv_bytes = recv(client.socket, input, MAX_BYTES_RECV, 0)) < 0) {
            perror("Error al recibir la línea de texto");
            exit(EXIT_FAILURE);
        } else if (!recv_bytes) {   /* Se recibió una orden de cerrar la conexión */
            return;
        }

        output = (char *) calloc(recv_bytes + 1, sizeof(char)); 

        for (i = 0; input[i]; i++) {
            output[i] = toupper((unsigned char) input[i]);
        }

        if ( (sent_bytes = send(client.socket, output, recv_bytes + 1, 0)) < 0) {
            perror("Error al enviar la línea de texto");
            exit(EXIT_FAILURE);
        }

        if (output) free(output);
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


