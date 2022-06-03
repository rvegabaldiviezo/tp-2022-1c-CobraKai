#ifndef UTILS_CPU_UTILS_CPU_H_
#define UTILS_CPU_UTILS_CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // Biblioteca de Socket: Crea un punto final para la comunicacion
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>
#include <commons/config.h>
#include "../cpu.config"

#define PATH_CONFIG "../../cpu.config"
typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

t_log* logger;

int iniciar_servidor(char* conf_puerto,char* conf_ip);

void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(int socket_cliente);
t_list* recibir_paquete(int socket_cliente);
int recibir_operacion(int socket_cliente);
int esperar_cliente(int socket_servidor);


int iniciar_servidor_dispatch(void);
int iniciar_servidor_interrupt(void);

#endif /* UTILS_CPU_UTILS_CPU_H_ */
