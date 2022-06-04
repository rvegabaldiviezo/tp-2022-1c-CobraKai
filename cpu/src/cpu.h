#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
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


#define PATH_CONFIG "src/cpu.config"

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
	int interrucion;//0: false
	t_config* config;
} t_proceso;

enum {
	NO_OP = 1,
	I_O,
	EXIT,
	COPY,
	READ,
	WRITE
};

enum {
	COD_RESP_BLOQUEO = 150, // pcb + tiempo
	COD_RESP_EXIT, // finalizo sus intrucciones
	COD_RESP_INTERRUPCION, // interrupion por puerto
};

//###### INTERFAZ FUNCIONES ######

void escuchaInterrup();


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

#endif /* CPU_H_ */
