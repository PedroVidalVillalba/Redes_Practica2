#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include "cliente.h"

typedef struct {
    int domain;
    int type;
    int protocol;
    uint16_t port;
    int backlog;
    int socket;
    struct sockaddr_in address;
} Server;

/**
 * Create the server socket, binds it and sets it as passive
 * @param port  Port number to listen in host order.
 */
Server create_server(int domain, int type, int protocol, uint16_t port, int backlog);

/**
 * Waits for a client connection and accepts it
 */
Client listen_for_connection(Server server);

/**
 * Sends the default message to the client
 */
void handle_connection(Server server, Client client);

#endif  //SERVIDOR_H
