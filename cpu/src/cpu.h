#ifndef CPU_H_
#define CPU_H_

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

#include "utils_cpu/utils_cpu.h"

#define PATH_CONFIG "src/cpu.config"
#define PATH_LOG "./cpu.log"

enum {
	ACCESO_TABLA_PAGINAS_PRIMER_NIVEL = 1,
	ACCESO_TABLA_PAGINAS_SEGUNDO_NIVEL,
	PEDIDO_LECTURA,
	PEDIDO_ESCRITURA
} operaciones_memoria;

bool conexion_exitosa(int cliente);
