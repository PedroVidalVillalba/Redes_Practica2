#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>



typedef struct 
{
    
  
    struct sockaddr_in address;
    int socket;

}Client;

Client create_client(int domain, int service, uint16_t port, char* server_ip);
void connect_to_server(Client client);
void handle_data(Client client);



#endif /*SERVIDOR_H*/
