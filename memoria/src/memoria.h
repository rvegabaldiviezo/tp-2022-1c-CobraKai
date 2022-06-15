
#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <commons/log.h>
#include <pthread.h>
#include <commons/string.h>
#include "utils_memoria/utils_memoria.h"

#define PATH_CONFIG "src/memoria.config"
#define PATH_LOG "./memoria.log"
#define KEY_PATH_SWAP "PATH_SWAP"
#define KEY_ENTRADAS_TABLA "ENTRADAS_POR_TABLA"

typedef enum {
	// Operaciones kernel
	INICIO_PROCESO = 100, // kernel solicita inicio de proceso, crear tabla y archivo de swap
	SUSPENCION_PROCESO, // kernel solicita suspencion de proceso, enviar marcos a swap de
	FINALIZACION_PROCESO, // kernel solicita finalizacion de proceso, liberar tablas y borrar archivo de swap

	// Operaciones cpu
	HANDSHAKE_CPU = 300, // cpu solicita cantidad de entradas por tabla y tamanio de paginas
	ACCESO_TABLA_PRIMER_NIVEL, // cpu solicita numero de tabla de primer nivel
	ACCESO_TABLA_SEGUNDO_NIVEL, // cpu solicita numero de tabla de segundo nivel
	LECTURA_MEMORIA_USUARIO, // cpu solicita lectura de espacio de memoria
	ESCRITURA_MEMORIA_USUARIO, // cpu solicita escritura de espacio de memoria
	ESCRITURA_EXITOSA, // envio a cpu mensaje "OK" para confirmacion de escritura

	ERROR = -1
} operacion;

/*enum {
	ACCESO_TABLA_PRIMER_NIVEL = 1,
	ACCESO_TABLA_SEGUNDO_NIVEL,
	LECTURA_MEMORIA_USUARIO,
	ESCRITURA_MEMORIA_USUARIO,
	ERROR_CPU = -1
} operaciones_cpu;*/

typedef struct {
	void* buffer;

} memoria_de_usuario;

typedef struct {
	int marco;
	bool presencia;
	bool inicializada;
	bool usada;
	bool modificada;
} t_tabla_paginas_segundo_nivel;


bool conexion_exitosa(int);
void inicializar_tabla_paginas();
int crear_tabla_paginas();
void atender_kernel();
void atender_cpu();
char* get_path_archivo(int);
pid_t recibir_id_proceso(int conexion_kernel);
t_tabla_paginas_segundo_nivel* inicializar_tabla_segundo_nivel();

void terminar_programa();


#endif /* MEMORIA_H_ */
