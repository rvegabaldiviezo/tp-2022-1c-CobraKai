#ifndef UTILS_CLIENT_SERV_H_
#define UTILS_CLIENT_SERV_H_

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

//###### STRUCTURAS #####


//typedef enum
//{
//	MENSAJE,
//	PAQUETE
//}op_code;


/*typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;*/


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
t_config* iniciar_config(char*);
void liberar_conexion(int socket_cliente);



void enviar_codigo_operacion(int conexion,int operacion);
void enviar_primer_acceso_memoria(int conexion,uint32_t nro_entrada_tabla, uint32_t entrada_tabla_1er_nivel);
void enviar_segundo_acceso_memoria(int conexion,uint32_t nro_entrada_tabla,uint32_t entrada_tabla_1er_nivel);
uint32_t recibir_uint32_t(int socket_cliente);
void enviar_tercer_acceso_memoria_escritura(int conexion,uint32_t nro_marco,uint32_t desplazamiento,uint32_t valor, int id);
void enviar_tercer_acceso_memoria_lectura(int conexion,uint32_t nro_marco,uint32_t desplazamiento);


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

//### primitivas
int iniciar_servidor(char* ip, char* puerto);
int esperar_cliente(int socket_servidor);
int recibir_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
t_list* recibir_paquete(int socket_cliente);
int crear_conexion(char *ip, char* puerto);




#endif /* UTILS_CLIENT_SERV_H_ */
