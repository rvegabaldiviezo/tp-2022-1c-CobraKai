
#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <commons/log.h>
#include "utils_memoria/utils_memoria.h"

#define PATH_CONFIG "src/memoria.config"

enum {
	CREAR_TABLA_PAGINAS = 1,
	ERROR = -1
};

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

#endif /* MEMORIA_H_ */
