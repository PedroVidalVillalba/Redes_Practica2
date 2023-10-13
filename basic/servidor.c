#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>


#include "servidor.h"


#define MESSAGE_SIZE 100
#define DEFAULT_PORT 8000
#define DEFAULT_BACKLOG 16

/**
 * Process the command line inputs given to main
 * @param argc  Number of arguments.
 * @param argv  Array of strings with the program arguments.
 * @return void
 */
void process_args(int argc, char** argv, uint16_t* port, int* backlog);


int main(int argc, char** argv) {
    Server server;
    Client client;
    uint16_t port;
    int backlog;
    pid_t child;

    process_args(argc, argv, &port, &backlog);

    server = create_server(AF_INET, SOCK_STREAM, 0, port, backlog);
    
    while (1) {
        client = listen_for_connection(server);
        
        child=fork();
	
        handle_connection(server, client);
        
        if(child == 0){//Esto solo se ejecuta en el hijo
            exit(EXIT_SUCCESS);
        }
    }

    close(server.socket);

    exit(EXIT_SUCCESS);
}

Server create_server(int domain, int type, int protocol, uint16_t port, int backlog) {
    Server server = {
        .domain = domain,
        .type = type,
        .protocol = protocol,
        .port = port,
        .backlog = backlog,
        .address.sin_family = domain,
        .address.sin_port = htons(port),
        .address.sin_addr.s_addr = htonl(INADDR_ANY)
    };

    /* Create the server socket */
    if ((server.socket = socket(domain, type, protocol)) < 0) {
        perror("No se pudo crear el socket");
        exit(EXIT_FAILURE);
    }

    /* Assign IP to listen to and port number (bind)*/
    if (bind(server.socket, (struct sockaddr *) &server.address, sizeof(server.address)) < 0) {
        perror("No se pudo asignar dirección IP");
        exit(EXIT_FAILURE);
    }

    /* Mark socket as passive, so that it listens to possible clients */
    if (listen(server.socket, backlog) < 0) {   
        perror("No se pudo marcar el socket como pasivo");
        exit(EXIT_FAILURE);
    }

    return server;
}

Client listen_for_connection(Server server) {
    Client client;
    char ipname[INET_ADDRSTRLEN];

    socklen_t address_length = sizeof(struct sockaddr_in);

    if ((client.socket = accept(server.socket, (struct sockaddr *) &client.address, &address_length)) < 0) {
        perror("No se pudo aceptar la conexión");
        exit(EXIT_FAILURE);
    }

    printf("Cliente conectado desde %s:%u\n", inet_ntop(server.domain, &client.address.sin_addr, ipname, INET_ADDRSTRLEN), ntohs(client.address.sin_port));
    
    return client;
}

void handle_connection(Server server, Client client) {
    char message[MESSAGE_SIZE] = {0};
    char ipname[INET_ADDRSTRLEN];
    ssize_t transmited_bytes;

    snprintf(message, MESSAGE_SIZE, "Tu conexión al servidor %s:%u ha sido aceptada\n", inet_ntop(server.domain, &client.address.sin_addr, ipname, INET_ADDRSTRLEN), server.port);

    if ((transmited_bytes = send(client.socket, message, strlen(message), 0)) < 0) {
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


