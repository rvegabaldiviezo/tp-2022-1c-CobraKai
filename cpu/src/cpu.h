#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <netdb.h>

#define PATH_CONFIG "src/cpu.config"

typedef struct{
	int id_proceso;
	int size_proceso;
	char* instrucciones_ejecutar;
	int program_counter;
} pcb;


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



#endif /* CPU_H_ */
