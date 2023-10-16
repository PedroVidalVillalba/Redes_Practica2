#ifndef CLIENTE_H
#define CLIENTE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include "servidor.h"

typedef struct {
    int socket;
    struct sockaddr_in address;
} Client;

Client create_client(int domain, int service, uint16_t port, char* server_ip);
void connect_to_server(Client client);
void handle_data(Client client, char* input_file_name, char* output_file_name);



#endif /*CLIENTE_H*/
