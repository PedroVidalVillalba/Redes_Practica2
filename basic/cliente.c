#include "servidor.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

#define NUM_BYTES_RECV 120

int main(int argc,char **argv){
	Client client;
	char server_ip[INET_ADDRSTRLEN]="127.0.0.1";
	
	
	client = create_client(AF_INET, SOCK_STREAM, 8000, server_ip);
	connect_to_server(client); 
	handle_data(client);
	
	
	
	/*LAS PALABRAS DEL PROFESOR CABA RESUENAN EN TU CABEZA: CIERRA LOS SOCKETS!*/
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
	inet_pton(domain, server_ip, &(client.address.sin_addr));//aqui podemos comprobar que se ha 	realizado la conversión con éxito 
	
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
		
		if ((recv_bytes = recv(client.socket, server_message, NUM_BYTES_RECV,0)) < 0) {
		perror("No se recibió el mensaje");
		exit(EXIT_FAILURE);
		}
		
		printf("Mensaje recibido: %s", server_message);

}
