#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#define HOST_LEN 25
#define BUFFER_LEN 1024
#define NODE_NAME "api4.ipify.org"
#define SERVICE "http"
#define HTTP_REQUEST    "GET / HTTP/1.1\n"\
                        "Host: " NODE_NAME "\n"\
                        "Connection: close\n"\
                        "\n"

int main(int argc, char** argv) {
    char hostname[HOST_LEN];
    char iptext[INET_ADDRSTRLEN], myip[INET_ADDRSTRLEN];
    struct addrinfo hints = {0};
    struct addrinfo* result, *rp;
    int status;
    int sockfd;
    ssize_t bytes_sent, bytes_recv;
    char http_request[] = HTTP_REQUEST;
    char input_buffer[BUFFER_LEN] = {0};

    hints = (struct addrinfo) {
        .ai_family   = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol  = 0,
        .ai_flags    = AI_CANONNAME | AI_ADDRCONFIG
    };

    if (gethostname(hostname, HOST_LEN)) {
        perror("No se pudo obtener el nombre del servidor");
        exit(EXIT_FAILURE);
    }

    if ( (status = getaddrinfo(NODE_NAME, SERVICE, &hints, &result)) ) {
        fprintf(stderr, "Error al obtener la información del servidor: %s\n", gai_strerror(status));
    }

    /* getaddrinfo() devuelve en result una lista de struct addrinfo;
     * probamos con cada una hasta conseguir conectarse */
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if ( (sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0) continue;   /* Esta dirección no permite crear el socket */

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) break; /* Conectado con éxito */

        close(sockfd);
    }

    if (!rp) {
        fprintf(stderr, "No se pudo conectar a %s\n", NODE_NAME);
        exit(EXIT_FAILURE);
    }

    printf("Cliente %s conectado exitosamente a %s, en la IP %s por el puerto %d\n", 
            hostname, rp->ai_canonname, 
            inet_ntop(rp->ai_family, rp->ai_addr, iptext, INET_ADDRSTRLEN),
            ntohs(getservbyname(SERVICE, NULL)->s_port));

    /* Ya estamos conectados a ipify.org. Ahora tenemos que enviarle la petición
     * HTTP y procesar la respuesta */
    if ( (bytes_sent = send(sockfd, http_request, strlen(http_request) + 1, 0)) == 0 ) {
        perror("No se pudo enviar la petición de http");
        exit(EXIT_FAILURE);
    }

    if ( (bytes_recv = recv(sockfd, input_buffer, BUFFER_LEN, 0)) == 0 ) {
        perror("No se pudo recivir la respuesta de http");
        exit(EXIT_FAILURE);
    }

    /* Parsear el input_buffer hasta encontrar dos saltos de línea seguidos */
    /* Buscamos la primera aparición de dos saltos de línea, que indica que inicia el cuerpo del mensaje*/
    strncpy(myip, strstr(input_buffer, "\r\n\r\n") + 4, INET_ADDRSTRLEN);

    printf("Mi IP externa es: %s\n", myip);

    freeaddrinfo(result);    

    exit(EXIT_SUCCESS);
}
