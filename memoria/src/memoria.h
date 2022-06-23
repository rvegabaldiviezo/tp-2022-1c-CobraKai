
#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <commons/log.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include "utils_memoria/utils_memoria.h"

#define PATH_CONFIG "src/memoria.config"
#define PATH_LOG "./memoria.log"
#define KEY_PATH_SWAP "PATH_SWAP"
#define KEY_ENTRADAS_TABLA "ENTRADAS_POR_TABLA"
#define KEY_TAM_MEMORIA "TAM_MEMORIA"
#define KEY_TAM_PAGINAS "TAM_PAGINA"

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
	unsigned int inicio;
	unsigned int fin;
} espacio_de_usuario;

typedef struct {
	int id;
	unsigned int tamanio;
	unsigned int numero_tabla_primer_nivel;
	espacio_de_usuario espacio_utilizable;
} t_proceso;

typedef struct {
	unsigned int marco;
	bool presencia;
	bool usada;
	bool modificada;
} t_pagina;

typedef struct {
	unsigned int numero;
	t_list* paginas;
} t_tabla_paginas_segundo_nivel;


// Funciones de tabla de paginas
void inicializar_tabla_paginas();
int crear_tabla_paginas();
t_tabla_paginas_segundo_nivel* inicializar_tabla_segundo_nivel();
t_pagina* inicializar_pagina();
t_bitarray* inicializar_bitarray();
int proximo_marco_libre();

// Funciones de hilos
void atender_kernel();
void atender_cpu();

// Funciones de archivos
void crear_archivo_swap(char* path);
char* get_path_archivo(int);

// Funciones de conexiones
bool conexion_exitosa(int);
pid_t recibir_id_proceso(int conexion_kernel);
void enviar_numero_de_tabla(int, int);
void enviar_numero_de_marco(int, int);
int recibir_entero(int conexion_kernel);
void enviar_entero(int, int, operacion);
void liberar_conexion(int);

void iterador_tablas_segundo_nivel(t_tabla_paginas_segundo_nivel* tabla);
void iterador_paginas(t_pagina* pag);

// Funciones para liberar memroia
void liberar_tablas();
void liberar_tabla_primer_nivel(int);
void liberar_tablas_segundo_nivel(t_list*);
void liberar_tabla_segundo_nivel(t_tabla_paginas_segundo_nivel* tabla);
void liberar_pagina(t_pagina* pagina);
void liberar_espacio_de_usuario(espacio_de_usuario espacio);
void terminar_programa();


#endif /* MEMORIA_H_ */
