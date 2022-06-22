#ifndef UTILS_H_
#define UTILS_H_

#include "../kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <netdb.h>


#define IP "127.0.0.1"
#define PUERTO "4444"


void* recibir_buffer(int*, int);
int recibir_operacion(int);
int iniciar_servidor(void);
int esperar_cliente(int);
t_list* recibir_instrucciones(int socket_cliente);
void recibir_mensaje(int);
int recibir_tamanio_proceso(int);
void destruir_nodo(t_link_element *);
void enviar_respuesta_exitosa(int conexion);
t_list* parsear_instrucciones(t_list* instrucciones);
void enviar_interrupcion(int);
void notificar_a_memoria(int, int*);
int recibir_tiempo_bloqueado(int);

void liberar_conexion(int socket_cliente);

#endif /* UTILS_H_ */
