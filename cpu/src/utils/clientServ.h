#ifndef CLIENT_SERV_H_
#define CLIENT_SERV_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <assert.h>
#include <signal.h>
#include <netdb.h>
// Librerias para Hilos y Semaforos
#include <pthread.h>
#include <semaphore.h>
// Commons que permite la catedra
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
// Include de utils
#include "../cpu.h"

//###### DEFINICIONES #####
#define KEY_IP_CPU "IP"
#define KEY_PUERTO_DISPATCH "PUERTO_ESCUCHA_DISPATCH"


//###### STRUCTURAS #####


typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


/*
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);*/
//---------------------------
t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void liberar_conexion(int socket_cliente);

/*
t_config* iniciar_config(void);
void leer_consola(t_log* logger);
void paquete(int conexion);
void terminar_programa(int conexion, t_log* logger, t_config* config);
*/
/***********************************************************************************/
/************************************ SERVER **************************************/
/***********************************************************************************/



/***********************************************************************************/
/************************************ SERVER **************************************/
/***********************************************************************************/
//### funcionales a cpu
void iniciar_servidor_dispatch(t_proceso_cpu cpu_process);
void iniciar_servidor_interrupt(t_proceso_cpu cpu_process);
int iniciar_servidor_cpu(t_proceso_cpu cpu_process, char* key_puerto);
int esperar_cliente_cpu(t_proceso_cpu cpu_process, int socket_server, char* tipo_puerto);
int esperar_cliente_dispatch(t_proceso_cpu cpu_process);
int esperar_cliente_interrupt(t_proceso_cpu cpu_process);
//### primitivas
int iniciar_servidor(char* ip, char* puerto);
int esperar_cliente(int socket_servidor);
int recibir_operacion(int socket_cliente);


#endif /* CLIENT_SERV_H_ */
