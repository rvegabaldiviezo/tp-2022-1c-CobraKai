#ifndef CLI_CPU_H_
#define CLI_CPU_H_

/****************** CLIENTE ******************
 *
 * 1) CPU (CLIENTE) -->  MEMORIA (Provider/Server)
 *
 * 2) Armar las estructuras necesarias para hacer los
 * 	  paquete acorde al tipo de consultas que le va a
 * 	  hacer a la memoria.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <netdb.h>
#include "../cpu.h"

#define IP "127.0.0.1"

/*** BUFFER: Estructura para guarda cualquier tipo de dato*/
typedef struct
{
	int size; //Indica la cantidad de elementos
	void* stream; //Puntero al primer elemento.
} t_buffer;


/*** PCB: Estructura para guardar informacion de los procesos*/
typedef struct {

// 1) Identificador del proceso
	pid_t id;

// 2) Tamaño en bytes del proceso
	unsigned int tamanio_proceso; //el mismo no cambiará a lo largo de la ejecución

// 3) Lista de instrucciones a ejecutar
	unsigned int instrucciones;

// 4) Número de la próxima instrucción a ejecutar
	unsigned int program_counter;

// 5) Identificador de tabla de páginas del proceso en memoria
	unsigned int tabla_paginas; //solo si el proceso paso a estado READY

// 6) Estimación utilizada para planificar los procesos en el algoritmo SRT
	unsigned int estimacion_rafaga;
} t_pcb;



#endif /* UTILS_CLI_CPU_H_ */
