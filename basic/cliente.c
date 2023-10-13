#include "servidor.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NUM_BYTES_RECV 120

/**
 * Process the command line inputs given to main
 * @param argc  Number of arguments.
 * @param argv  Array of strings with the program arguments.
 * @return void
 */
void process_args(int argc, char** argv, char* server_ip, uint16_t* port);

int main(int argc,char **argv){
	Client client;
	char server_ip[INET_ADDRSTRLEN];
    uint16_t port;
	

    process_args(argc, argv, server_ip, &port);
	client = create_client(AF_INET, SOCK_STREAM, port, server_ip);

	connect_to_server(client); 
	/*sleep(3); //Apartado 1(c)*/
	handle_data(client);
	
	close(client.socket);
}

Client create_client(int domain, int service, uint16_t port, char* server_ip){
	Client client;
	
	//Creamos el socket y comprobamos su correcta creación
	if ((client.socket = socket(domain, service, 0)) < 0) {
		perror("No se pudo crear el socket ");
		exit( EXIT_FAILURE );
	}	
	
	client.address.sin_family = domain,//AF_INET;
	client.address.sin_port = htons(port);
    if (!inet_pton(domain, server_ip, &(client.address.sin_addr))) {    /* La string no se pudo traducir a una IP válida */
        fprintf(stderr, "La IP especificada no es válida\n\n");
        exit(EXIT_FAILURE);
    }; 
	
	return client;
}

void connect_to_server(Client client){
	socklen_t address_length = (socklen_t)sizeof(struct sockaddr_in);
	
	if ((connect(client.socket, (struct sockaddr *) &client.address, address_length)) < 0) {
		perror("No se pudo conectar");
		exit(EXIT_FAILURE);
	}
}

void handle_data(Client client){
		ssize_t recv_bytes=0;
		char server_message[NUM_BYTES_RECV];
		while((recv_bytes = recv(client.socket, server_message, NUM_BYTES_RECV,0)) > 0){
			printf("Mensaje recibido: %s. Han sido recibidos %ld bytes.\n", server_message, recv_bytes);
		}
		/*if ((recv_bytes = recv(client.socket, server_message, NUM_BYTES_RECV,0)) < 0) {
		perror("No se recibió el mensaje");
		exit(EXIT_FAILURE);
		} */
		
		/*printf("Mensaje recibido: %s. Han sido recibidos %ld bytes.\n", server_message, recv_bytes);*/

}

void print_help(void) {}

void process_args(int argc, char** argv, char* server_ip, uint16_t* port) {
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
                        *port = atoi(argv[i]);
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
