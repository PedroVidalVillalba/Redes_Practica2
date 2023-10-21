#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include "client.h"

#define MAX_BYTES_RECV 15

/**
 * Process the command line inputs given to main
 * @param argc  Number of arguments.
 * @param argv  Array of strings with the program arguments.
 * @return void
 */
void process_args(int argc, char** argv, char* server_ip, uint16_t* server_port);

void handle_data(Client client);

int main(int argc, char** argv){
	Client client;
	char server_ip[INET_ADDRSTRLEN];
    uint16_t server_port;
	
    process_args(argc, argv, server_ip, &server_port);

	client = create_client(AF_INET, SOCK_STREAM, 0, server_ip, server_port);

	connect_to_server(client); 
	
	handle_data(client);
	
	close_client(&client);

    exit(EXIT_SUCCESS);
}


void handle_data(Client client){
		ssize_t recv_bytes=0;
		char server_message[MAX_BYTES_RECV];
		
		/* Código asociado al apartado 1(d) */
		while((recv_bytes = recv(client.socket, server_message, MAX_BYTES_RECV,0)) > 0){
			printf("Mensaje recibido: %s. Han sido recibidos %ld bytes.\n", server_message, recv_bytes);
		}
		
		/* Código asociado al apartado 1(d) */
		
		/* Código asociado al apartado 1(c) 
		if ((recv_bytes = recv(client.socket, server_message, MAX_BYTES_RECV,0)) < 0) {
		perror("No se recibió el mensaje");
		exit(EXIT_FAILURE);
		} 
		
		printf("Mensaje recibido: %s. Han sido recibidos %ld bytes.\n", server_message, recv_bytes);
		
		 Fin del código asociado al apartado 1(c) */

}

void print_help(void) {}

void process_args(int argc, char** argv, char* server_ip, uint16_t* server_port) {
    int i;
    char* current_arg;

    /* Si no se pasan argumentos, avisar y poner las opciones por defecto */
    if (argc == 1) {
        fprintf(stderr, "No se ha especificado la IP y el puerto al que conectarse\n\n");
        print_help();
        exit(EXIT_FAILURE);
    }

    for (i = 1; i < argc; i++) { /* Procesamos los argumentos (sin contar el nombre del ejecutable) */
        current_arg = argv[i];
        if (current_arg[0] == '-') { /* Flag de opción */
            switch(current_arg[1]) {
                case 'I':   /* IP */
                    if (++i < argc) {
                        strncpy(server_ip, argv[i], INET_ADDRSTRLEN);
                    } else {
                        fprintf(stderr, "IP no especificada tras la opción '-I'\n\n");
                        print_help();
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'p':   /* Puerto */
                    if (++i < argc) {
                        *server_port = atoi(argv[i]);
                    } else {
                        fprintf(stderr, "Puerto no especificado tras la opción '-p'\n\n");
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
