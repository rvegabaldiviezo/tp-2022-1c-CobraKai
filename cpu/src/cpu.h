#ifndef CPU_H_
#define CPU_H_

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
#include "./utils/clientServ.h"

//###### Rutas de Archivos ######
#define PATH_CONFIG "./cpu.config"
#define PATH_LOG "./cpu.log"
#define NAME_LOG "CPU"

//###### DEFINICIONES #####
#define KEY_IP_CPU "IP_CPU"
#define KEY_PUERTO_DISPATCH "PUERTO_ESCUCHA_DISPATCH"
#define KEY_PUERTO_INTERRUPT "PUERTO_ESCUCHA_INTERRUPT"


//###### ESTRUCTURAS ######
typedef struct instruction {
    char* id;
    u_int32_t* params;
} t_instruction;

typedef struct {
	int id;
	unsigned int tamanio_proceso;
	t_list* instrucciones;
	unsigned int program_counter;
	unsigned int tablas_paginas;
	unsigned int estimacion_rafaga;
} t_pcb;

typedef struct {
	t_pcb pcb;
	int socket;
	int interrupcion;//0: false
	t_config* config;
	t_log* logger;
} t_proceso;

typedef struct {
	t_config* config;
	t_log* logger;
	t_proceso* process;
	int socket_servidor;
	int conexion_con_memoria;
	int conexion_con_kernel;
	int socket_servidor_dispatch;//DISPATCH
	int socket_servidor_interrupt;//INTERRUPT
} proceso_cpu;

enum {
	NO_OP = 1,
	I_O,
	EXIT,
	COPY,
	READ,
	WRITE,
};

// Operaciones recibidas por kernel
enum {
	PCB = 1,//Recibimos del kernel una lista de instrucciones
	INTERRUPCION,//Recibimos del kernel la orden de interrupcion
	ERROR = -1 //Ocurrio un error en la comunicacion
};

enum {
	COD_RESP_BLOQUEO = 150, // pcb + tiempo
	COD_RESP_EXIT, // finalizo sus intrucciones
	COD_RESP_INTERRUPCION, // interrupion por puerto
};

//###### INTERFAZ FUNCIONES ######

void iniciar_conexion_cpu_memoria(proceso_cpu* cpu_process);
void escuchaInterrup(proceso_cpu* cpu_process);
proceso_cpu* iniciar_cpu(proceso_cpu*);
proceso_cpu* cpu_create(void);
t_proceso* process_create(void);
void finalizar_cpu(proceso_cpu* cpu_process);
void atender_kernel_dispatch(proceso_cpu* cpu_process,int conexion_kernel);
int recibir_operaciones(proceso_cpu* cpu_process, int socket_kernel);
/*
int fetch(t_proceso proceso);
void fetch_operands(t_proceso proces, t_instruction instruccion);
t_instruction decode(t_proceso proceso,int nro_intruccion);
void execute(t_proceso proceso, t_instruction instruccion);

void no_op(int tiempo,int repeticiones);
void i_o(t_proceso proceso, int tiempo);
void exitt(t_proceso proceso);
void  check_interrupt(t_proceso proceso);

void responseInterrupcion(t_proceso proceso);
void responsePorBloqueo(t_proceso proceso,int tiempo);
void responsePorFinDeProceso(t_proceso proceso);
void incrementarpcb(t_proceso proceso);
int size_instrucciones(t_proceso proceso);
*/
//### funcionales a cpu
void iniciar_servidor_dispatch(proceso_cpu* cpu_process);
void iniciar_servidor_interrupt(proceso_cpu* cpu_process);
int iniciar_servidor_cpu(proceso_cpu* cpu_process, char* key_puerto);
int esperar_cliente_cpu(proceso_cpu* cpu_process, int socket_server, char* tipo_puerto);
int esperar_cliente_dispatch(proceso_cpu* cpu_process);
int esperar_cliente_interrupt(proceso_cpu* cpu_process);

#endif /* CPU_H_ */
