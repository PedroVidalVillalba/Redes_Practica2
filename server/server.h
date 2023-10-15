#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "client.h"

/**
 * Estructura que contiene toda la información relevante 
 * del servidor y el socket en el que escucha peticiones.
 */
typedef struct {
    int socket;     /* Socket asociado al servidor en el que escuchar conexiones o atender peticiones */
    int domain;     /* Dominio de comunicación. Especifica la familia de protocolos que se usan para la comunicación */
    int type;       /* Tipo de protocolo usado para el socket */
    int protocol;   /* Protocolo particular usado en el socket */
    uint16_t port;  /* Puerto en el que el servidor escucha peticiones (en orden de host) */
    int backlog;    /* Longitud máxima de la cola de conexiones pendientes (para sockets pasivos) */
    char* hostname; /* Nombre del equipo en el que está ejecutándose el servidor */
    char* ip;       /* IP externa del servidor (en formato textual) */
    struct sockaddr_in listen_address;  /* Estructura con el dominio de comunicación, IPs a las que atender
                                           y puerto al que está asociado el socket */
} Server;

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
Server create_server(int domain, int type, int protocol, uint16_t port, int backlog);


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
 * 
 * @return  Descriptor del nuevo socket creado para atender las peticiones del hijo.
 */
int listen_for_connection(Server server, Client* client);


/* NOTA: No sé como de buena idea es incluir esta función en una cabecera,
 * igual es mejor simplemente borrarla y que aparezca en cada uno de los programas
 * de la forma oportuna */
/**
 * @brief   Maneja la conexión por parte del servidor.
 * @warning Esta función no está implementada por defecto.
 *
 * Maneja la conexión entre el cliente y el servidor una vez establecida.
 * Ejecuta la funcionalidad de la que está encargado el servidor en esa conexión.
 * El funcionamiento de esta función es específico del servicio que se pretenda
 * prestar por parte del servidor; por tanto, no tiene implementación por defecto
 * y es responsabilidad de la aplicación que la llame el implementarla (piénsese 
 * como una función abstracta).
 *
 * @param server    Servidor encargado de manejar la conexión. Debe tener un socket
 *                  conectado al cliente (devuelto por listen_for_connection()).
 * @param client    Cliente que solicita el servicio proporcionado por el servidor,
 *                  y que ya se encuentra conectado a él.
 */
void handle_connection(Server server, Client client);


/**
 * @brief   Cierra el servidor.
 *
 * Cierra el socket asociado al servidor y libera toda la memoria
 * reservada para el servidor.
 *
 * @param server    Servidor a cerrar.
 */
void close_server(Server* server); 


#endif  /* SERVER_H */
