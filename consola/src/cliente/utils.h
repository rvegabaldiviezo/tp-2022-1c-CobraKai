#ifndef UTILS_CLIENTE_H_
#define UTILS_CLIENTE_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	int tamanio;
	t_buffer* buffer;
} t_paquete;

typedef struct
{
	int operacion;
	int tamanio;
	t_buffer* buffer;
} t_proceso;


int crear_conexion(char*, char*);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_proceso* crear_proceso(int);
//t_paquete* crear_super_paquete(void);
void agregar_instruccion(t_proceso*, void*, int);
void enviar_a_kernel(t_proceso*, int);
void liberar_conexion(int socket_cliente);
void eliminar_proceso(t_proceso*);
void* serializar_paquete(t_proceso*, int);

#endif /* UTILS_CLIENTE_H_ */
