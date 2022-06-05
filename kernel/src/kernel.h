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
#include <pthread.h>
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

typedef struct {
	pid_t id;
	unsigned int tamanio_proceso;
	t_list* instrucciones;
	unsigned int program_counter;
	unsigned int tablas_paginas;
	unsigned int estimacion_rafaga;
} t_pcb;

typedef struct {
	t_pcb pcb;
	int socket;
} t_proceso;

typedef struct
{
	t_proceso* proceso;
	int tiempo_de_bloqueo;
} t_proceso_bloqueado;

// Operaciones con consola
enum {
	LISTA_DE_INSTRUCCIONES = 1,
	EXIT,
	RESPUESTA_EXITO,
	ERROR = -1
};

// Operaciones con memoria
enum {
	TABLA_PAGINAS_PRIMER_NIVEL = 1
};

// Operaciones con cpu
enum {
	BLOQUEO_IO = 1,
	ERROR_CPU = -1
} operaciones_cpu;


void iterator(char* value);
bool conexion_exitosa(int);
void terminar_programa();

t_proceso* crear_proceso(void);
t_pcb crear_pcb();
t_proceso* recibir_proceso(int socket_cliente);
void destruir_proceso(t_proceso*);
bool numero_de_tabla_valido(int);
void inicializar_colas();
int atender_consola();
void planificar_srt();
void planificar_fifo();
void agregar_a_bloqueados(t_proceso_bloqueado* proceso);
void iniciar_planificacion_io();
int recibir_tiempo_bloqueo();
void iniciar_planificacion(char* planificacion);
void comunicacion_con_cpu();
void inicializar_semaforos();
void * list_pop(t_list* lista);

#endif /* KERNEL_H_ */
