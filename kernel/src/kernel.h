#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include "./server/utils.h"


typedef struct {
	pid_t id;
	unsigned int tamanio_proceso;
	unsigned int program_counter;
	unsigned int tablas_paginas;
	unsigned int estimacion_rafaga;
} t_pcb;

typedef struct {
	t_pcb pcb;
	t_list instrucciones;
	unsigned int tamanio;
} t_proceso;

void iterator(char* value);
bool conexion_exitosa(int);
void terminar_programa(t_log*);
t_proceso* crear_proceso(t_list*, unsigned int);
t_pcb crear_pcb(unsigned int);

#endif /* KERNEL_H_ */
