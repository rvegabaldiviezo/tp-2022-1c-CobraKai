
#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <commons/log.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <math.h>
#include "utils_memoria/utils_memoria.h"
#include <commons/txt.h>
#include <math.h>

#define PATH_CONFIG "src/memoria.config"
#define PATH_LOG "./memoria.log"
#define KEY_PATH_SWAP "PATH_SWAP"
#define KEY_ENTRADAS_TABLA "ENTRADAS_POR_TABLA"
#define KEY_TAM_MEMORIA "TAM_MEMORIA"
#define KEY_TAM_PAGINAS "TAM_PAGINA"
#define KEY_RETARDO_SWAP "RETARDO_SWAP"
#define KEY_RETARDO_MEMORIA "RETARDO_MEMORIA"
#define KEY_ALGORITMO_REEMPLAZO "ALGORITMO_REEMPLAZO"
#define KEY_MARCOS_POR_PROCESO "MARCOS_POR_PROCESO"

typedef enum {
	// Operaciones kernel
	INICIO_PROCESO = 100, // kernel solicita inicio de proceso, crear tabla y archivo de swap
	SUSPENCION_PROCESO, // kernel solicita suspencion de proceso, enviar marcos a swap de
	FINALIZACION_PROCESO, // kernel solicita finalizacion de proceso, liberar tablas y borrar archivo de swap
	DESUSPENCION_PROCESO,

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
	unsigned int marco;
	bool presencia;
	bool usada;
	bool modificada;
	time_t tiempo_carga;
} t_pagina;

typedef struct {
	int id;
	unsigned int tamanio;
	unsigned int numero_tabla_primer_nivel;
	t_pagina* puntero;
	//espacio_de_usuario espacio_utilizable;
} t_proceso;

typedef struct {
	int numero_tabla_primer_nivel;
	int puntero_marco;
} t_puntero;

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
void escribir_en_archivo(char*, t_pagina*);

// Funciones de conexiones
bool conexion_exitosa(int);
int recibir_entero(int);
int recibir_id_proceso(int conexion_kernel);
int recibir_tamanio(int conexion_kernel);
int recibir_numero_tabla_primer_nivel(int);
int recibir_numero_tabla(int);
int recibir_numero_entrada(int);
uint32_t recibir_uint32(int);
void enviar_numero_de_pagina(int, int);
void enviar_confirmacion(int);
void enviar_respuesta(int, int);
void enviar_uint32(int, uint32_t);
void enviar_entero(int destino, int a_enviar);
void enviar_numero_de_tabla(int destino, int numero_de_tabla);
void liberar_conexion(int);

void iterador_tablas_segundo_nivel(t_tabla_paginas_segundo_nivel* tabla);
void iterador_paginas(t_pagina* pag);

//Funciones de swap
void swapear_paginas_modificadas(int id);
t_list* get_paginas_modificadas(int id);
t_list* get_contenido_pagina(t_pagina*);
char* leer_hasta(char, FILE*);
void reemplazar_pagina(t_pagina* pagina);

// Funciones para liberar memroia
void liberar_tablas();
void liberar_tabla_primer_nivel(int);
void liberar_tablas_segundo_nivel(t_list*);
void liberar_tabla_segundo_nivel(t_tabla_paginas_segundo_nivel* tabla);
void liberar_pagina(t_pagina* pagina);
void liberar_elementos(void* elemento);
void liberar_marcos_proceso(int id);
void liberar_marco(int marco);
void terminar_programa();

//Otras
bool igual_numero(t_tabla_paginas_segundo_nivel*, int);
void leer_config();
uint32_t leer_contenido_marco(int, int);
int escribir_en_marco(int, int, uint32_t);
t_pagina* mayor_tiempo_espera(t_pagina* p1, t_pagina* p2);

void algoritmo_clock(t_pagina* pagina);
void modificar_bit_uso(t_pagina* pagina);
void page_fault(t_pagina* pagina);
t_list* encontrar_paginas_en_memoria();
int cantidad_marcos_ocupados_proceso();
void algoritmo_clock_modificado(t_pagina* pagina);
void clock_m_paso_1(t_list* paginas,t_pagina* pagina, int *indice_puntero, bool *reemplazada);
void clock_m_paso_2(t_list* paginas,t_pagina* pagina, int *indice_puntero, bool *reemplazada);
t_list* leer_contenido_pagina_archivo(char* path_archivo, t_pagina* pagina);
uint32_t leer_pagina_disco(t_pagina*);
void asignar_marco(t_pagina* pagina);
int encontrar_indice_puntero();
void escribir_marco_completo(int marco, t_list* contenido);
t_puntero* encontrar_puntero_proceso();
t_pagina* encontrar_pagina_por_marco(int numero_de_marco);

#endif /* MEMORIA_H_ */
