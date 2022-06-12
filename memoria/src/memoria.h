
#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <commons/log.h>
#include <pthread.h>
#include "utils_memoria/utils_memoria.h"

#define PATH_CONFIG "src/memoria.config"
#define PATH_LOG "./memoria.log"
#define KEY_ENTRADAS_TABLA "ENTRADAS_POR_TABLA"

enum {
	INICIO_PROCESO = 1,
	SUSPENCION_PROCESO,
	FINALIZACION_PROCESO,
	ERROR_KERNEL = -1
} operaciones_kernel;

enum {
	ACCESO_TABLA_PRIMER_NIVEL = 1,
	ACCESO_TABLA_SEGUNDO_NIVEL,
	LECTURA_MEMORIA_USUARIO,
	ESCRITURA_MEMORIA_USUARIO,
	ERROR_CPU = -1
} operaciones_cpu;

typedef struct {
	void* buffer;

} memoria_de_usuario;

typedef struct {
	int marco;
	bool presencia;
	bool inicializada;
	// U??
	// M??
} t_tabla_paginas;


bool conexion_exitosa(int);
void inicializar_tabla_paginas();
void crear_tabla_paginas();
void atender_kernel();
void atender_cpu();
void terminar_programa();


#endif /* MEMORIA_H_ */
