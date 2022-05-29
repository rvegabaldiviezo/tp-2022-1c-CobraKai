
#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <commons/log.h>
#include <pthread.h>
#include "utils_memoria/utils_memoria.h"

#define PATH_CONFIG "src/memoria.config"
#define PATH_LOG "./memoria.log"

enum {
	INICIO_PROCESO = 1,
	SUSPENCION_PROCESO,
	FINALIZACION_PROCESO,
	ERROR = -1
} operaciones_kernel;

typedef struct {
	void* buffer;

} memoria_de_usuario;


typedef struct {

} t_tabla_paginas_segundo_nivel;

typedef struct {
	int numero;
	t_tabla_paginas_segundo_nivel* tabla_segundo_nivel;
} t_tabla_paginas_primer_nivel;

bool conexion_exitosa(int);
void crear_tabla_paginas();
#endif /* MEMORIA_H_ */
