#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H

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
void handle_data(Client client);



#endif  /* SERVER_CLIENT_H */
