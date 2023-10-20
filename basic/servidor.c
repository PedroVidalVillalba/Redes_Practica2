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
    pid_t child;
    struct arguments args = {
        .argc = argc,
        .argv = argv,
        .port = &port,
        .backlog = &backlog
    };

    process_args(args);

    printf("Ejecutando servidor con parámetros: PORT=%u, BACKLOG=%d\n\n", port, backlog);
    server = create_server(AF_INET, SOCK_STREAM, 0, port, backlog);

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("No se pudo cambiar la respuesta a la señal SIGTERM");
        exit(EXIT_FAILURE);
    }

    loop = 1;
    while (loop) {
        listen_for_connection(server, &client);

        child = fork();

        handle_connection(server, client);

        printf("Cerrando la conexión del cliente %s:%u\n", client.ip, ntohs(client.address.sin_port));
        close_client(&client);  /* Ya hemos gestionado al cliente, podemos olvidarnos de él */

        if (child == 0) loop = 0;   /* Que el hijo salga del bucle y termine */
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

    printf("Manejando la conexión del cliente %s:%u...\n", client.ip, ntohs(client.address.sin_port));

    snprintf(message, MESSAGE_SIZE, "Tu conexión al servidor %s en %s:%u ha sido aceptada\n", server.hostname, server.ip, server.port);

    /* Enviar el mensaje al cliente */
    if ( (transmited_bytes = send(client.socket, message, strlen(message) + 1, 0)) < 0) {
        perror("No se pudo enviar el mensaje");
        exit(EXIT_FAILURE);
    }
}


static void print_help(char* exe_name){
    /** Cabecera y modo de ejecución **/
    printf("Uso: %s [[-p] <port>] [-b <backlog>]\n\n", exe_name);

    /** Lista de opciones de uso **/
    printf(" Opción\t\tOpción larga\t\tSignificado\n");
    printf(" -p <port>\t--port <port>\t\tPuerto en el que escuchará el servidor.\n");
    printf(" -b <backlog>\t--backlog <backlog>\tTamaño máximo de la cola de conexiones pendientes.\n");
    printf(" -h\t\t--help\t\t\tMostrar este texto de ayuda y salir.\n");

    /** Consideraciones adicionales **/
    printf("\nPuede especificarse el parámetro <port> para el puerto en el que escucha el servidor sin escribir la opción '-p', siempre y cuando este sea el primer parámetro que se pasa a la función.\n");
    printf("\nSi no se especifica alguno de los argumentos, el servidor se ejecutará con su valor por defecto, a saber: DEFAULT_PORT=%u; DEFAULT_BACKLOG=%d\n", DEFAULT_PORT, DEFAULT_BACKLOG);
    printf("\nSi se especifica varias veces un argumento, el comportamiento está indefinido.\n");
}


void process_args(struct arguments args) {
    int i;
    char* current_arg;

    /* Inicializar los valores de puerto y backlog a sus valores por defecto */
    *args.port = DEFAULT_PORT;
    *args.backlog = DEFAULT_BACKLOG;

    for (i = 1; i < args.argc; i++) { /* Procesamos los argumentos (sin contar el nombre del ejecutable) */
        current_arg = args.argv[i];
        if (current_arg[0] == '-') { /* Flag de opción */
            /* Manejar las opciones largas */
            if (current_arg[1] == '-') { /* Opción larga */
                if (!strcmp(current_arg, "--port")) current_arg = "-p";
                else if (!strcmp(current_arg, "--backlog")) current_arg = "-b";
                else if (!strcmp(current_arg, "--help")) current_arg = "-h";
            } 
            switch(current_arg[1]) {
                case 'p':   /* Puerto */
                    if (++i < args.argc) {
                        *args.port = atoi(args.argv[i]);
                        if (*args.port < 0) {
                            fprintf(stderr, "El valor de puerto especificado (%s) no es válido.\n\n", args.argv[i]);
                            print_help(args.argv[0]);
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        fprintf(stderr, "Puerto no especificado tras la opción '-p'.\n\n");
                        print_help(args.argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'b':   /* Backlog */
                    if (++i < args.argc) {
                        *args.backlog = atoi(args.argv[i]);
                        if (*args.backlog < 0) {
                            fprintf(stderr, "El valor de backlog especificado (%s) no es válido.\n\n", args.argv[i]);
                            print_help(args.argv[0]);
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        fprintf(stderr, "Tamaño del backlog no especificado tras la opción '-b'.\n\n");
                        print_help(args.argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'h':
                    print_help(args.argv[0]);
                    exit(EXIT_SUCCESS);
                default:
                    fprintf(stderr, "Opción '%s' desconocida\n\n", current_arg);
                    print_help(args.argv[0]);
                    exit(EXIT_FAILURE);
            }
        } else if (i == 1) {    /* Se especificó el puerto como primer argumento */
            *args.port = atoi(args.argv[i]);
            if (*args.port < 0) {
                fprintf(stderr, "El valor de puerto especificado como primer argumento (%s) no es válido.\n\n", args.argv[i]);
                print_help(args.argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
}


