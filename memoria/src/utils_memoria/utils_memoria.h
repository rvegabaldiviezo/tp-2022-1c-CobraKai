#ifndef UTILS_MEMORIA_UTILS_MEMORIA_H_
#define UTILS_MEMORIA_UTILS_MEMORIA_H_

/****************** CLIENTE *******************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <netdb.h>
#include <assert.h>
#include <commons/config.h>
#include "../memoria.h"

#define IP "127.0.0.1"

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void enviar_numero_de_tabla(int, int);

/******************* SERVER ******************/

void* recibir_buffer(int*, int);
int iniciar_servidor(void);
int esperar_cliente(int);
int recibir_operacion(int);

#endif /* UTILS_MEMORIA_UTILS_MEMORIA_H_ */
