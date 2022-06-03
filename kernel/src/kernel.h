#ifndef KERNEL_H_
#define KERNEL_H_

#include <pthread.h>
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

// Operaciones con consola
enum {
	LISTA_DE_INSTRUCCIONES = 1,
	RESPUESTA_EXITO,
	ERROR = -1
};

// Operaciones con memoria
enum {
	TABLA_PAGINAS_PRIMER_NIVEL = 1
};

void iterator(char* value);
bool conexion_exitosa(int);
void terminar_programa();
//t_proceso* crear_proceso(void);
//t_pcb crear_pcb();
//t_pcb crear_pcb(unsigned int);
//int solicitar_numero_de_tabla(int);
bool numero_de_tabla_valido(int);
void inicializar_colas();
int atender_consola();
void planificar_srt();

#endif /* KERNEL_H_ */
