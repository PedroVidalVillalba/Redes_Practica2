#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <wctype.h>
#include <locale.h>

#include "server.h"

#define MESSAGE_SIZE 10000
#define DEFAULT_PORT 8000
#define DEFAULT_BACKLOG 16
#define MAX_BYTES_RECV 128

#define fail(message) { perror((message)); exit(EXIT_FAILURE); }

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
 * @brief   Procesa los argumentos del main
 *
 * Procesa los argumentos proporcionados al programa por línea de comandos,
 * e inicializa las variables del programa necesarias acorde a estos.
 *
 * @param args  Estructura con los argumentos del programa y punteros a las
 *              variables que necesitan inicialización.
 */
static void process_args(struct arguments args);

/**
 * @brief Imprime la ayuda del programa
 *
 * @param exe_name  Nombre del ejecutable (argv[0])
 */
static void print_help(char* exe_name);

/**
 * @brief   Transforma una string a mayúsculas
 *
 * Transforma la string source a mayúsculas, utilizando para ello wstrings para poder transformar
 * caracteres especiales que ocupen más de un byte. Por tanto, permite pasar a mayúsculas strings en
 * el idioma definido en locale.
 *
 * @param source    String fuente a transformar en mayúsculas.
 *
 * @return  String dinámicamente alojada (por tanto, debe liberarse con un free) que contiene los mismos
 *          caractereres que source pero en mayúsculas.
 */
static char* toupper_string(const char* source);

void handle_connection(Server server, Client client);

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

    if (!setlocale(LC_ALL, "")) fail("No se pudo establecer la locale del programa");   /* Establecer la locale según las variables de entorno */

    process_args(args);

    printf("Ejecutando servidor de mayúsculas con parámetros: PORT=%u, BACKLOG=%d.\n\n", port, backlog);
    server = create_server(AF_INET, SOCK_STREAM, 0, port, backlog);

    while (!terminate) {
        if (!socket_io_pending) pause();    /* Pausamos la ejecución hasta que se reciba una señal de I/O o de terminación */
        listen_for_connection(server, &client);
        if (client.socket == -1) continue;  /* Falsa alarma, no había conexiones pendientes o se recibió una señal de terminación */

        handle_connection(server, client);

        printf("\nCerrando la conexión del cliente %s:%u.\n\n", client.ip, ntohs(client.address.sin_port));
        close_client(&client);  /* Ya hemos gestionado al cliente, podemos olvidarnos de él */
    }

    printf("\nCerrando el servidor y saliendo...\n");
    close_server(&server);

    exit(EXIT_SUCCESS);
}


void handle_connection(Server server, Client client) {
    char* output;
    char input[MAX_BYTES_RECV];
    ssize_t recv_bytes, sent_bytes;

    printf("\nManejando la conexión del cliente %s:%u...\n", client.ip, ntohs(client.address.sin_port));

    while (1) {
        if ( (recv_bytes = recv(client.socket, input, MAX_BYTES_RECV, 0)) < 0) fail("Error al recibir la línea de texto");
        if (!recv_bytes) return;    /* Se recibió una orden de cerrar la conexión */

        output = toupper_string(input); 

        if ( (sent_bytes = send(client.socket, output, strlen(output) + 1, 0)) < 0) {
            perror("Error al enviar la línea de texto");
            exit(EXIT_FAILURE);
        }

        if (output) free(output);
    }
}


/**
 * @brief   Transforma una string a mayúsculas
 *
 * Transforma la string source a mayúsculas, utilizando para ello wstrings para poder transformar
 * caracteres especiales que ocupen más de un byte. Por tanto, permite pasar a mayúsculas strings en
 * el idioma definido en locale.
 *
 * @param source    String fuente a transformar en mayúsculas.
 *
 * @return  String dinámicamente alojada (por tanto, debe liberarse con un free) que contiene los mismos
 *          caractereres que source pero en mayúsculas.
 */
static char* toupper_string(const char* source) {
    wchar_t* wide_source;
    wchar_t* wide_destiny;
    ssize_t wide_size, size;
    char* destiny;
    int i;

    wide_size = mbstowcs(NULL, source, 0); /* Calcular el número de wchar_t que ocupa el string source */
    /* Alojar espacio para wide_source y wide_destiny */
    wide_source = (wchar_t *) calloc(wide_size + 1, sizeof(wchar_t));
    wide_destiny = (wchar_t *) calloc(wide_size + 1, sizeof(wchar_t));

    /* Transformar la fuente en un wstring */
    mbstowcs(wide_source, source, wide_size + 1);
    /* Transformar wide_source a mayúsculas y guardarlo en wide_destiny */
    for (i = 0; wide_source[i]; i++) {
        wide_destiny[i] = towupper(wide_source[i]);
    }

    /* Transoformar de vuelta a un string normal */
    size = wcstombs(NULL, wide_destiny, 0); /* Calcular el número de char que ocupa el wstring wide_destiny */
    destiny = (char *) calloc(size + 1, sizeof(char));
    wcstombs(destiny, wide_destiny, size + 1);

    if (wide_source) free(wide_source);
    if (wide_destiny) free(wide_destiny);

    return destiny;
}


static void print_help(char* exe_name){
    /** Cabecera y modo de ejecución **/
    printf("Uso: %s [[-p] <port>] [-b <backlog>] [-h]\n\n", exe_name);

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


static void process_args(struct arguments args) {
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
                case 'h':   /* Ayuda */
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
