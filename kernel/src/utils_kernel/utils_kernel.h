#ifndef UTILS_H_
#define UTILS_H_

#include "../kernel.h"
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


#define IP "127.0.0.1"
#define PUERTO "4444"

typedef struct
{
	int size;
	void* stream;
} t_buffer;



void* recibir_buffer(int*, int);
int recibir_operacion(int);
int iniciar_servidor(void);
int esperar_cliente(int);
t_list* parsear_instrucciones(t_list* instrucciones);
void recibir_mensaje(int);
int recibir_tamanio_proceso(int);
void destruir_nodo(t_link_element *);
void enviar_respuesta_exitosa(int conexion);




typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;


typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct
{
	t_proceso* proceso;
	int tiempo_de_bloqueo;
	int inicio_bloqueo;
	int suspendido = 0;
} t_proceso_bloqueado;



int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void solicitar_numero_de_tabla(int);
int recibir_numero_de_tabla(int);
void notificar_a_memoria(int, int*);

#endif /* UTILS_H_ */
