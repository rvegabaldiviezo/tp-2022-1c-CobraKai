#ifndef KERNEL_H_
#define KERNEL_H_

#include "utils_kernel/utils_kernel.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/config.h>
#include <semaphore.h>

#define PATH_CONFIG "src/kernel.config"
#define PATH_LOG "./kernel.log"
// Items de config
#define IP_MEMORIA "IP_MEMORIA"
#define PUERTO_MEMORIA "PUERTO_MEMORIA"
#define ESTIMACION_INICIAL "ESTIMACION_INICIAL"
#define ALFA "ALFA"
#define GRADO_MULTIPROGRAMACION "GRADO_MULTIPROGRAMACION"
#define IP_CPU "IP_CPU"
#define PUERTO_CPU "PUERTO_CPU"
#define ALGORITMO_PLANIFICACION "ALGORITMO_PLANIFICACION"
#define TIEMPO_MAXIMO_BLOQUEADO "TIEMPO_MAXIMO_BLOQUEADO"

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct {
	pid_t id;
	unsigned int tamanio_proceso;
	unsigned int program_counter;
	unsigned int tablas_paginas;
	unsigned int estimacion_rafaga;
	unsigned int socket;
	t_list* instrucciones;
} t_pcb;

typedef struct {
	t_pcb* proceso;
	int tiempo_de_bloqueo;
	int inicio_bloqueo;
	int suspendido;
} t_pcb_bloqueado;

typedef struct {
	char* cadena;
	unsigned int tamanio;
} t_instruccion;

enum {
	LISTA_DE_INSTRUCCIONES = 1, // kernel crea un proceso a partir de las instrucciones recibidas y de ser posible lo asigna a la cola ready
	RESPUESTA_EXITO, // al finalizar el proceso kernel envía la respuesta a la consola que lo haya solicitado
	ERROR = -1
} operaciones_consola;

enum {
	TABLA_PAGINAS_PRIMER_NIVEL = 1 // memoria crea las tablas de paginas para el proceso y devuelve el numero de la tabla de nivel uno
} operaciones_memoria;

// Operaciones con cpu
enum {
	PCB = 1,
	BLOQUEO_IO,
	EXIT,
	INTERRUPCION,
	ERROR_CPU = -1
} operaciones_cpu;


void iterator(char* value);
bool conexion_exitosa(int);
void terminar_programa();
t_pcb* crear_proceso(void);
t_pcb crear_pcb();
t_pcb* recibir_proceso(int socket_cliente);
void destruir_proceso(t_pcb*);
bool numero_de_tabla_valido(int);
void inicializar_colas();
int atender_consola();
void planificar_srt();
void planificar_fifo();
void agregar_a_bloqueados(t_pcb_bloqueado* proceso);
void iniciar_planificacion_io();
int recibir_tiempo_bloqueo();
void iniciar_planificacion(char* planificacion);
void comunicacion_con_cpu();
void* list_pop(t_list*);
t_pcb* menor_tiempo_restante(t_pcb*, t_pcb*);
t_pcb* crear_proceso(void);
t_pcb crear_pcb();
t_list* recibir_instrucciones(int socket_cliente);
t_pcb* recibir_proceso(int socket_cliente);
void destruir_proceso(t_pcb*);
t_pcb* lista_mas_corta(t_pcb*, t_pcb*);
bool repetido(t_pcb* p1, t_pcb* p2);
void inicializar_semaforos();
void list_push(t_list* lista, void* elemento);
t_pcb* menor_tiempo_restante(t_pcb* p1, t_pcb* p2);
void solicitar_interrupcion();
void enviar_pcb(t_pcb*, int);
void* serializar_pcb(t_pcb*, t_buffer*, int);
void agregar_instruccion(t_buffer* buffer, char* valor, int tamanio);
t_buffer* cargar_buffer(t_list* lista);

#endif /* KERNEL_H_ */
