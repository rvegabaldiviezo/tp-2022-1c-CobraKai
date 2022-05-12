#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/config.h>
#include "utils_kernel/utils_kernel.h"

#define PATH_CONFIG "src/kernel.config"
#define IP_MEMORIA "IP_MEMORIA"
#define PUERTO_MEMORIA "PUERTO_MEMORIA"
#define IP_CPU "IP_CPU"
#define PUERTO_CPU "PUERTO_CPU"

enum {
	LISTA_DE_INSTRUCCIONES = 1,
	ERROR = -1
};

void iterator(char* value);
bool conexion_exitosa(int);
void terminar_programa(t_log*, int, int);
//t_proceso* crear_proceso(void);
//t_pcb crear_pcb(unsigned int);
//int solicitar_numero_de_tabla(int);
bool numero_de_tabla_valido(int);

#endif /* KERNEL_H_ */
