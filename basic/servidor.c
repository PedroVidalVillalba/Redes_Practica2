#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "server.h"

#define MESSAGE_SIZE 128
#define DEFAULT_PORT 8000
#define DEFAULT_BACKLOG 16

static int loop;    /* Para saber si seguimos iterando o hay que parar */

/**
 * Estructura de datos para pasar a la función process_args.
 * Debe contener siempre los campos int argc, char** argv, provenientes de main,
 * y luego una cantidad variable de punteros a las variables que se quieran inicializar
 * a partir de la entrada del programa.
 */
struct arguments {
    int argc;
    char** argv;
    uint16_t* port;
    int* backlog;
};

/**
 * @brief Procesa los argumentos del main
 *
 * Procesa los argumentos proporcionados al programa por línea de comandos,
 * e inicializa las variables del programa necesarias acorde a estos.
 *
 * @param args  Estructura con los argumentos del programa y punteros a las variables
 *              que necesitan inicialización.
 */
void process_args(struct arguments args);

void handle_connection(Server server, Client client);

void signal_handler(int);   /* Función para gestionar las señales */

int main(int argc, char** argv) {
    Server server;
    Client client;
    uint16_t port;
    int backlog;
    
    struct arguments args = {
        .argc = argc,
        .argv = argv,
        .port = &port,
        .backlog = &backlog
    };

    process_args(args);

    server = create_server(AF_INET, SOCK_STREAM, 0, port, backlog);

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("No se pudo cambiar la respuesta a la señal SIGTERM");
        exit(EXIT_FAILURE);
    }

    loop = 1;
    while (loop) {
        listen_for_connection(server, &client);

        handle_connection(server, client);

        close_client(&client);  /* Ya hemos gestionado al cliente, podemos olvidarnos de él */     
    }

    close_server(&server);

    exit(EXIT_SUCCESS);
}


void signal_handler(int signum) {
    loop = 0;   /* Hace que deje de iterar en el bucle */
}

void handle_connection(Server server, Client client) {
    char message[MESSAGE_SIZE] = {0};
    ssize_t transmited_bytes;

   

    /* Código asociado al apartado 1(c)*/
    
    /* Enviamos un primer mensaje*/
    strcpy(message, "hola");
    if ((transmited_bytes = send(client.socket, message, strlen(message) + 1, 0)) < 0) {
        perror("No se pudo enviar el mensaje");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    /*Envamos un segundo mensaje*/
    strcpy(message, "Mundo");
    if ((transmited_bytes = send(client.socket, message, strlen(message) + 1, 0)) < 0) {
      perror("No se pudo enviar el mensaje");
      exit(EXIT_FAILURE);
    }
    
    /* Fin del código asociado al apartado 1(c)*/
    
    /* Enviar el mensaje al cliente */
    /* snprintf(message, MESSAGE_SIZE, "Tu conexión al servidor %s en %s:%u ha sido aceptada\n", server.hostname, server.ip, server.port); */
 /*   if ( (transmited_bytes = send(client.socket, message, strlen(message) + 1, 0)) < 0) {
        perror("No se pudo enviar el mensaje");
        exit(EXIT_FAILURE);
    }*/
}


void print_help(void) {}


void process_args(struct arguments args) {
    int i;
    char* current_arg;

    /* Si no se pasan argumentos, avisar y poner las opciones por defecto */
    if (args.argc == 1) {        
        printf("Ejecutando el servidor con las opciones por defecto: PORT=%u, BACKLOG=%d\n\n", DEFAULT_PORT, DEFAULT_BACKLOG);
        *args.port = DEFAULT_PORT;
        *args.backlog = DEFAULT_BACKLOG;
        return;
    }

    for (i = 1; i < args.argc; i++) { /* Procesamos los argumentos (sin contar el nombre del ejecutable) */
        current_arg = args.argv[i];
        if (current_arg[0] == '-') { /* Flag de opción */
            switch(current_arg[1]) {
                case 'p':   /* Puerto */
                    if (++i < args.argc) {
                        *args.port = atoi(args.argv[i]);
                    } else {
                        fprintf(stderr, "Puerto no especificado tras la opción '-p'\n\n");
                        print_help();
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'b':   /* Backlog */
                    if (++i < args.argc) {
                        *args.backlog = atoi(args.argv[i]);
                    } else {
                        fprintf(stderr, "Tamaño del backlog no especificado tras la opción '-b'\n\n");
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


