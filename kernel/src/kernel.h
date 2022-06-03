#ifndef KERNEL_H_
#define KERNEL_H_

#include "utils_kernel/utils_kernel.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/config.h>

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

enum {
	LISTA_DE_INSTRUCCIONES = 1, // kernel crea un proceso a partir de las instrucciones recibidas y de ser posible lo asigna a la cola ready
	RESPUESTA_EXITO, // al finalizar el proceso kernel envía la respuesta a la consola que lo haya solicitado
	ERROR = -1
} operaciones_consola;

enum {
	TABLA_PAGINAS_PRIMER_NIVEL = 1 // memoria crea las tablas de paginas para el proceso y devuelve el numero de la tabla de nivel uno
} operaciones_memoria;

enum {
	REPLANIFICACION = 100 // se envía a traves de la conexion INTERRUPT: cpu corta la ejecucion en curso y devuelve el proceso con el pcb actualizado y la estimacion restante
} operaciones_cpu;

void iterator(char* value);
bool conexion_exitosa(int);
void terminar_programa();
//t_proceso* crear_proceso(void);
//t_pcb crear_pcb();
//t_pcb crear_pcb(unsigned int);
//int solicitar_numero_de_tabla(int);
bool numero_de_tabla_valido(int);
void inicializar_colas();
void inicializar_semaforos();
int atender_consola();
void planificar_srt();
void * list_pop(t_list*);
t_proceso* menor_tiempo_restante(t_proceso*, t_proceso*);
t_proceso* crear_proceso(void);
t_pcb crear_pcb();
t_list* recibir_instrucciones(int socket_cliente);
t_proceso* recibir_proceso(int socket_cliente);
void destruir_proceso(t_proceso*);
t_proceso* lista_mas_corta(t_proceso*, t_proceso*);
bool repetido(t_proceso* p1, t_proceso* p2);

#endif /* KERNEL_H_ */
