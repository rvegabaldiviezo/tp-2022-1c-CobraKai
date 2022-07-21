#ifndef KERNEL_H_
#define KERNEL_H_

#include "utils_kernel/utils_kernel.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
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
	int tablas_paginas;
	unsigned int estimacion_rafaga;
	unsigned int socket;
	t_list* instrucciones;
	int inicio_rafaga;
} t_pcb;

typedef struct {
	t_pcb* proceso;
	int tiempo_de_bloqueo;
	time_t inicio_bloqueo;
	int suspendido;
} t_pcb_bloqueado;

typedef struct {
	char* cadena;
	unsigned int tamanio;
} t_instruccion;

typedef enum {
	// Operaciones con consola
	LISTA_DE_INSTRUCCIONES = 1, // kernel crea un proceso a partir de las instrucciones recibidas y de ser posible lo asigna a la cola ready
	RESPUESTA_EXITO, // al finalizar el proceso kernel envía la respuesta a la consola que lo haya solicitado

	// Operaciones con memoria
	INICIO_PROCESO = 100, // memoria crea las tablas de paginas para el proceso y devuelve el numero de la tabla de nivel uno
	SUSPENCION_PROCESO, // memoria libera espacio del proceso y manda a swap espacio de memoria de usuario
	FINALIZACION_PROCESO, // memoria libera las estructuras asociadas al proceso y elimina su archivo de swap, no eliminar tablas

	TABLA_PAGINAS_PRIMER_NIVEL, // se recibe el numero de tabla asociado al proceso
	CONFIRMACION_PROCESO_SUSPENDIDO, // luego de enviar una suspencion a memoria, se espera la confirmacion de la misma

	// Operaciones con cpu
	PCB = 200, // envio el pcb para su ejecucion y recibo un pcb de cpu
	BLOQUEO_IO, // se recibe de la cpu la notificacion de proceso bloqueado por I/O
	EXIT, // se recibe de la cpu la notificacion de finalizacion de un proceso
	INTERRUPCION, // solo para SRT, se envia una interrupcion a cpu para que se corte la ejecucion y devuelva el pcb actualizado

	ERROR = -1
} operacion;

typedef struct {
	operacion cod_op;
	pid_t elemento;
} paquete;

void iterator(char* value);
bool conexion_exitosa(int);
void terminar_programa();
t_pcb* crear_proceso(void);
t_pcb crear_pcb();
t_pcb* recibir_proceso(int socket_cliente);
void destruir_proceso(t_pcb*);
void destruir_lista(t_list*);
void destruir_queue(t_queue*);
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
t_list* recibir_instrucciones(int socket_cliente);
bool repetido(t_pcb* p1, t_pcb* p2);
void inicializar_semaforos();
void list_push(t_list* lista, void* elemento);
void solicitar_interrupcion();
void enviar_pcb(t_pcb*, int);
void* serializar_pcb(t_pcb*, t_buffer*, int);
void agregar_instruccion(t_buffer* buffer, char* valor, int tamanio);
t_buffer* cargar_buffer(t_list* lista);
paquete cargar_id(operacion, pid_t);
void notificar_suspencion_proceso(pid_t, int);
void enviar_finalizacion_a_memoria(pid_t id, int conexion_con_memoria);
int recibir_entero(int socket_cliente);
t_pcb* recibir_pcb(int conexion);
t_pcb_bloqueado* recibir_pcb_bloqueado(int conexion);
void solicitar_numero_de_tabla(t_pcb*, int);
int recibir_numero_de_tabla(t_pcb*, int);
void escuchar_cpu_dispatch();
void pasar_a_ready(t_pcb*);
void esperar_y_suspender(t_pcb_bloqueado*);
void desuspendidor();
bool esta_en_lista_bloqueados(t_pcb_bloqueado*);
void iniciar_hilo_desuspendidor();
void iniciar_planificador_largo_plazo();
void pasar_de_new_a_ready();


#endif /* KERNEL_H_ */
