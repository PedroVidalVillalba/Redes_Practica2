#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include "client.h"

#define NUM_BYTES_RECV 128


/**
 * Process the command line inputs given to main
 * @param argc  Number of arguments.
 * @param argv  Array of strings with the program arguments.
 * @return void
 */
void process_args(int argc, char** argv, char* server_ip, uint16_t* server_port);

void handle_data(Client client, char* input_file_name, char* output_file_name);

int main(int argc,char **argv){
    Client client;
    char server_ip[INET_ADDRSTRLEN];
    uint16_t server_port;

    process_args(argc, argv, server_ip, &server_port);

    client = create_client(AF_INET, SOCK_STREAM, 0, server_ip, server_port);

    connect_to_server(client); 

    handle_data(client, "archivo1.txt", "archivo2.txt");

    close_client(&client);

    exit(EXIT_SUCCESS);
}


void handle_data(Client client, char* input_file_name, char* output_file_name){
    ssize_t transmited_bytes = 0, recv_bytes = 0;
    FILE *fp_input, *fp_output;
    char send_buffer[NUM_BYTES_RECV];
    char recv_buffer[NUM_BYTES_RECV];

    /*Apertura de los archivos*/
    if ( !(fp_input = fopen(input_file_name, "r")) ) {
        perror("Error en la apertura del archivo de lectura");
        exit(EXIT_FAILURE);
    }
    if ( !(fp_output = fopen(output_file_name, "w")) ) {
        perror("Error en la apertura del archivo de escritura");
        exit(EXIT_FAILURE);
    }		

    /*Procesamiento y envio del archivo*/
    while (!feof(fp_input)) {
        /* Leemos hasta que lo que devuelve fscanf es EOF, cerramos la conexión en ese caso */
        if( (fscanf(fp_input, "%[^\r\n]%*c", send_buffer)) == EOF){ /*Escaneamos la linea hasta el final del archivo*/
            shutdown(client.socket, SHUT_RD);   /* Le decimos al servidor que pare de recibir */
            continue;
        }

        /* strcat(send_buffer, ""); // Metemos el caracter de terminacion de string '\0'*/
        printf("Se procede a enviar el mensaje: %s\n", send_buffer);

        if ( (transmited_bytes = send(client.socket, send_buffer, strlen(send_buffer) + 1, 0)) < 0) {
            perror("No se pudo enviar el mensaje");
            exit(EXIT_FAILURE);
        }
        /*Esperamos a recibir la linea*/
        if( (recv_bytes = recv(client.socket, recv_buffer, NUM_BYTES_RECV,0)) < 0){
            perror("No se pudo recibir el mensaje");
            exit(EXIT_FAILURE);                
        }
        printf("Recibida linea %s\n", recv_buffer);
        fprintf(fp_output, "%s", recv_buffer);/*@Se podria usar el mismo buffer que de envio*/
    }
    
    /* Cerramos los archivos al salir */
    if (fclose(fp_input)) {
        perror("No se pudo cerrar el archivo de lectura");
        exit(EXIT_FAILURE);
    }
    if (fclose(fp_output)) {
        perror("No se pudo cerrar el archivo de escritura");
        exit(EXIT_FAILURE);
    }

    return;
}

void print_help(void) {}

void process_args(int argc, char** argv, char* server_ip, uint16_t* server_port) {
    int i;
    char* current_arg;

    /* Si no se pasan argumentos, avisar y poner las opciones por defecto */
    if (argc == 1) {
        fprintf(stderr, "No se ha especificado la IP y el puerto al que conectarse\n\n");
        print_help();
        exit(EXIT_FAILURE);
    }

    for (i = 1; i < argc; i++) { /* Procesamos los argumentos (sin contar el nombre del ejecutable) */
        current_arg = argv[i];
        if (current_arg[0] == '-') { /* Flag de opción */
            switch(current_arg[1]) {
                case 'I':   /* IP */
                    if (++i < argc) {
                        strncpy(server_ip, argv[i], INET_ADDRSTRLEN);
                    } else {
                        fprintf(stderr, "IP no especificada tras la opción '-I'\n\n");
                        print_help();
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'p':   /* Puerto */
                    if (++i < argc) {
                        *server_port = atoi(argv[i]);
                    } else {
                        fprintf(stderr, "Puerto no especificado tras la opción '-p'\n\n");
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
