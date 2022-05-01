#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h> // Biblioteca de Socket: Crea un punto final para la comunicacion
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>

#define IP "127.0.0.1"
#define PUERTO "4444"

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct {
	pid_t id;
	unsigned int tamanio_proceso;
	unsigned int program_counter;
	unsigned int tablas_paginas;
	unsigned int estimacion_rafaga;
} t_pcb;

typedef struct {
	int tamanio;
	t_list* instrucciones;
	t_pcb pcb;
} t_proceso;

void* recibir_buffer(int*, int);

int iniciar_servidor(void);
int esperar_cliente(int);
t_proceso* crear_proceso(void);
t_list* recibir_instrucciones(int socket_cliente);
t_proceso* recibir_proceso(int socket_cliente);
void recibir_mensaje(int);
int recibir_tamanio_proceso(int);
void destruir_proceso(t_proceso*);
void destruir_nodo(t_link_element *);

#endif /* UTILS_H_ */
