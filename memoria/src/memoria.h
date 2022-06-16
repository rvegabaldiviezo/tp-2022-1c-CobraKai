
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

typedef struct {
	void* buffer;

} memoria_de_usuario;

typedef struct {
	int marco;
	bool bit_presencia;
	bool bit_uso;
	bool bit_modificacion;
} t_pagina;

typedef struct {
	unsigned int numero;
	t_list* paginas;
} t_tabla_paginas_segundo_nivel;

typedef struct {
	pid_t id_proceso;
	unsigned int numero_de_tabla;
} tabla_x_proceso;


// Funciones de sockets
bool conexion_exitosa(int);
pid_t recibir_id_proceso(int conexion_kernel);
void liberar_conexion();

// Funciones de tablas de paginas
int crear_tabla_paginas(pid_t);
void inicializar_tabla_segundo_nivel();
t_list* inicializar_paginas();
void destruir_tabla_segundo_nivel(t_tabla_paginas_segundo_nivel*);
bool mismo_id(pid_t id, void* indice);
bool mismo_numero_tabla(void* tabla);
void destruir_elemento(void* elemento);
//void destruir_pagina(t_pagina* pagina);

// Funciones de hilos
void atender_kernel();
void atender_cpu();

// Funciones de archivo de swap
char* get_path_archivo(int);
void crear_archivo_swap(char*);




void terminar_programa();


#endif /* MEMORIA_H_ */
