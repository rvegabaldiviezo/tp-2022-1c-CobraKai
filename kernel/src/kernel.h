#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include "./server/utils.h"


typedef struct {
	pid_t id;
	unsigned int tamanio_proceso;
	unsigned int program_counter;
	unsigned int tablas_paginas;
	unsigned int estimacion_rafaga;
} t_pcb;

void iterator(char* value);
bool conexion_exitosa(int);
void terminar_programa(t_log*);

#endif /* KERNEL_H_ */
