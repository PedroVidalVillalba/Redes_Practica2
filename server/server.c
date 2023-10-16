#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include "server.h"
#include "getip.h"

#define BUFFER_LEN 64


/**
 * @brief   Crea un servidor.
 *
 * Crea un servidor nuevo con un nuevo socket, le asigna un puerto y 
 * lo marca como pasivo para poder escuchar conexiones.
 *
 * @param domain    Dominio de comunicación. 
 * @param type      Tipo de protocolo usado para el socket.
 * @param protocol  Protocolo particular a usar en el socket. Normalmente solo existe
 *                  un protocolo para la combinación dominio-tipo dada, en cuyo caso se
 *                  puede especificar con un 0.
 * @param port      Número de puerto en el que escuchar (en orden de host).
 * @param backlog   Longitud máxima de la cola de conexiones pendientes.
 *
 * @return  Servidor que guarda toda la información relevante sobre sí mismo con la que
 *          fue creado, y con un socket pasivo abierto en el cual está listo para escuchar 
 *          y aceptar conexiones entrantes desde cualquier IP y del dominio y por puerto 
 *          especificados.
 */
Server create_server(int domain, int type, int protocol, uint16_t port, int backlog) {
    Server server;
    char buffer[BUFFER_LEN] = {0};

    memset(&server, 0, sizeof(Server));     /* Inicializar los campos a 0 */

    server = (Server) {
        .domain = domain,
        .type = type,
        .protocol = protocol,
        .port = port,
        .backlog = backlog,
        .listen_address.sin_family = domain,
        .listen_address.sin_port = htons(port),
        .listen_address.sin_addr.s_addr = htonl(INADDR_ANY) /* Aceptar conexiones desde cualquier IP */
    };

    /* Guardar el nombre del equipo en el que se ejecuta el servidor.
     * No produce error crítico, por lo que no hay que salir */
    if (gethostname(buffer, BUFFER_LEN)) {
        perror("No se pudo obtener el nombre de host del servidor");
    } else {
        server.hostname = (char *) calloc(strlen(buffer) + 1, sizeof(char));
        strcpy(server.hostname, buffer);
    }

    /* Guardar la IP externa del servidor.
     * Tampoco supone un error crítico. */
    if (!getip(buffer, BUFFER_LEN)) {
        perror("No se pudo obtener la IP externa del servidor");
    } else {
        server.ip = (char *) calloc(strlen(buffer) + 1, sizeof(char));
        strcpy(server.ip, buffer);
    }

    /* Crear el socket del servidor */
    if ( (server.socket = socket(domain, type, protocol)) < 0) {
        perror("No se pudo crear el socket");
        exit(EXIT_FAILURE);
    }

    /* Asignar IPs a las que escuchar y número de puerto por el que atender peticiones (bind) */
    if (bind(server.socket, (struct sockaddr *) &server.listen_address, sizeof(struct sockaddr_in)) < 0) {
        perror("No se pudo asignar dirección IP");
        exit(EXIT_FAILURE);
    }

    /* Marcar el socket como pasivo, para que pueda escuchar conexiones de clientes */
    if (listen(server.socket, backlog) < 0) {   
        perror("No se pudo marcar el socket como pasivo");
        exit(EXIT_FAILURE);
    }

    printf("Servidor creado con éxito y listo para escuchar solicitudes de conexión.\n"
            "Hostname: %s; IP: %s; Puerto: %d\n", server.hostname, server.ip, server.port);

    return server;
}


/**
 * @brief   Escucha conexiones de clientes.
 *
 * Pone al servidor a escuchar intentos de conexión.
 * El proceso se bloquea hasta que aparece una solicitud de conexión.
 * Cuando se recibe una, se acepta, se informa de ella y se crea una nueva
 * estructura en la que guardar la información del cliente conectado, y un nuevo socket
 * conectado al cliente para atender sus peticiones.
 * El uso esperado es crear un proceso hijo para manejar esa conexión, y en ese proceso
 * hijo asignar al socket del servidor el nuevo socket asociado al cliente que devuelve
 * esta función. 
 * 
 * @param server    Servidor que poner a escuchar conexiones. Debe tener un socket
 *                  asociado marcado como pasivo.
 * @param client    Dirección en la que guardar la información del cliente conectado.
 *                  Debe iniciarse con el campo socket a -1.
 * 
 * @return  Descriptor del nuevo socket creado para atender las peticiones del hijo.
 */
int listen_for_connection(Server server, Client* client) {
    int sockfd;
    char ipname[INET_ADDRSTRLEN];
    socklen_t address_length = sizeof(struct sockaddr_in);

    close_client(client);   /* Resetear el cliente en caso de que ya tuviese información */

    /* Aceptar la conexión del cliente en el socket pasivo del servidor */
    if ( (sockfd = accept(server.socket, (struct sockaddr *) &(client->address), &address_length)) < 0) {
        perror("No se pudo aceptar la conexión");
        exit(EXIT_FAILURE);
    }

    /* Rellenar los campos del cliente con la información relevante para el servidor.
     * Todos los campos que se desconozcan se dejarán con el valor por defecto que deja 
     * la función close_client */
    client->domain = client->address.sin_family;
    client->type = server.type; /* Asumimos que usan el mismo tipo de protocolo de comunicación */
    client->protocol = server.protocol;
    /* Obtener el nombre de la dirección IP del cliente y guardarlo */
    inet_ntop(client->domain, &(client->address.sin_addr), ipname, INET_ADDRSTRLEN);
    client->ip = (char *) calloc(strlen(ipname) + 1, sizeof(char));
    strcpy(client->ip, ipname);
    client->server_ip = (char *) calloc(strlen(server.ip) + 1, sizeof(char));
    strcpy(client->server_ip, server.ip);
    client->server_port = server.port;

    /* Informar de la conexión */
    printf("Cliente conectado desde la IP %s:%u\n", client->ip, ntohs(client->address.sin_port));

    return sockfd;
}


/**
 * @brief   Cierra el servidor.
 *
 * Cierra el socket asociado al servidor y libera toda la memoria
 * reservada para el servidor.
 *
 * @param server    Servidor a cerrar.
 */
void close_server(Server* server) {
    /* Cerrar el socket del servidor */
    if (server->socket != -1) {
        if (close(server->socket)) {
            perror("No se pudo cerrar el socket del servidor");
            exit(EXIT_FAILURE);
        }
    }

    if (server->hostname) free(server->hostname);
    if (server->ip) free(server->ip);

    /* Limpiar la estructura poniendo todos los campos a 0 */
    memset(server, 0, sizeof(Server));
    server->socket = -1;    /* Poner un socket no válido para que se sepa que no se puede usar ni volver a cerrar */
    
    return;
}
