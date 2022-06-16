#include "memoria.h"

pthread_t hilo_cpu;
pthread_t hilo_kernel;

t_log* logger;
t_config* config;
int server_memoria;
int conexion_kernel;
int conexion_cpu;
t_list* tabla_primer_nivel;
t_list* tablas_segundo_nivel;
unsigned int maximo_entradas;
char* path_swap;

unsigned int proximo_numero_tabla;

int main(void) {

	logger = log_create(PATH_LOG, "MEMORIA", true, LOG_LEVEL_DEBUG);

	config = config_create(PATH_CONFIG);

	maximo_entradas = config_get_int_value(config, KEY_ENTRADAS_TABLA);
	path_swap = config_get_string_value(config, KEY_PATH_SWAP);
	string_append(&path_swap, "/");

	tabla_primer_nivel = list_create();
	tablas_segundo_nivel = list_create();

	server_memoria = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir clientes");

	//pthread_create(&hilo_cpu, NULL, (void *) atender_cpu, NULL);
	pthread_create(&hilo_kernel, NULL, (void *) atender_kernel, NULL);

	terminar_programa();
	return EXIT_SUCCESS;
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}

int crear_tabla_paginas(pid_t id_proceso) {
	if(list_size(tabla_primer_nivel) <= maximo_entradas) {
		proximo_numero_tabla += 1;
		tabla_x_proceso* indice = malloc(sizeof(tabla_x_proceso));
		indice->id_proceso = id_proceso;
		indice->numero_de_tabla = proximo_numero_tabla;
		inicializar_tabla_segundo_nivel();
		list_add(tabla_primer_nivel, indice);
		return proximo_numero_tabla;
	} else {
		return -1;
	}
	// ver que hacer si se pasa el maximo de entradas
}

void inicializar_tabla_segundo_nivel() {
	t_tabla_paginas_segundo_nivel* tabla = malloc(sizeof(t_tabla_paginas_segundo_nivel));
	tabla->numero = proximo_numero_tabla;
	tabla->paginas = inicializar_paginas();
	list_add(tablas_segundo_nivel, tabla);
}

t_list* inicializar_paginas() {
	t_list* paginas = list_create();

	for(int i = 0; i < maximo_entradas; i++) {
		t_pagina* pagina = malloc(sizeof(t_pagina));
		pagina->bit_modificacion = false;
		pagina->bit_presencia = true;
		pagina->bit_uso = false;
		pagina->marco = i;
		list_add(paginas, pagina);
	}

	return paginas;
}

char* get_path_archivo(pid_t id) {
	char* extension = ".swap";
	char* nombre = string_itoa(id);
	char* aux = string_new();
	string_append(&aux, path_swap);
	string_append(&nombre, extension);
	string_append(&aux, nombre);
	return aux;
}

void crear_archivo_swap(char* path) {
	FILE* f = fopen(path, "w");
	fclose(f);
}

bool mismo_id(pid_t id, void* indice) {
	if(id == ((tabla_x_proceso*) indice)->id_proceso) {
		bool mismo_numero_tabla(void * tabla) {
			return ((tabla_x_proceso*) indice)->numero_de_tabla == ((t_tabla_paginas_segundo_nivel *) tabla)->numero; // ni el mismisimo Linus Torvalds
		}
		list_remove_and_destroy_by_condition(tablas_segundo_nivel, (void *) mismo_numero_tabla, (void *) destruir_tabla_segundo_nivel);
		log_info(logger, "Se borro la tabla numero: %d", ((tabla_x_proceso*) indice)->numero_de_tabla);
		return true;
	}
	return false;
}

void terminar_programa() {
	pthread_join(hilo_cpu, NULL);
	pthread_join(hilo_kernel, NULL);
	config_destroy(config);
	log_destroy(logger);
	liberar_conexion(conexion_cpu);
	liberar_conexion(conexion_kernel);
	list_destroy_and_destroy_elements(tabla_primer_nivel, (void *) destruir_elemento);
	list_destroy_and_destroy_elements(tablas_segundo_nivel, (void *) destruir_tabla_segundo_nivel);
}

void atender_cpu() {
	conexion_cpu = esperar_cliente(server_memoria);

	if(!conexion_exitosa(conexion_cpu)) {
		log_error(logger, "No se pudo establecer la conexion con la cpu");
		exit(EXIT_FAILURE);
	}

	while(1) {
		operacion operacion = recibir_operacion(conexion_cpu);
		switch(operacion) {
			case ACCESO_TABLA_PRIMER_NIVEL:
				log_info(logger, "CPU solicita acceso a tabla pagina de primer nivel");

				break;
			case ACCESO_TABLA_SEGUNDO_NIVEL:
				log_info(logger, "CPU solicita acceso a tabla pagina de segundo nivel");
				break;
			case LECTURA_MEMORIA_USUARIO:
				log_info(logger, "CPU solicita lectura a memoria de usuario");
				break;
			case ESCRITURA_MEMORIA_USUARIO:
				log_info(logger, "CPU solicita escritura a memoria de usuario");
				break;
			case ERROR:
				log_error(logger, "Se desconecto la cpu");
				exit(EXIT_FAILURE);
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}
}

void atender_kernel() {
	proximo_numero_tabla = 0;
	conexion_kernel = esperar_cliente(server_memoria);

	if(!conexion_exitosa(conexion_kernel)) {
		log_error(logger, "No se pudo establecer la conexion con el kernel");
		exit(EXIT_FAILURE);
	}

	while(1) {
		operacion operacion = recibir_operacion(conexion_kernel);
		switch(operacion) {
			case INICIO_PROCESO:
				log_info(logger, "Kernel solicita INICIO PROCESO");
				pid_t id_proceso = recibir_id_proceso(conexion_kernel);
				int numero_de_tabla = crear_tabla_paginas(id_proceso);

				crear_archivo_swap(get_path_archivo(id_proceso));


				enviar_numero_de_tabla(conexion_kernel, numero_de_tabla);

				break;
			case SUSPENCION_PROCESO:
				log_info(logger, "Kernel solicita SUSPENCION PROCESO");
				break;
			case FINALIZACION_PROCESO:
				log_info(logger, "Kernel solicita FINALIZACION PROCESO");
				pid_t id = recibir_id_proceso(conexion_kernel);
				remove(get_path_archivo(id)); // borro el archivo asociado al proceso
				bool tabla_pertenece_a_proceso(void* elemento) {
					return mismo_id(id, elemento);
				}
				list_remove_and_destroy_all_by_condition(tabla_primer_nivel, (void *) tabla_pertenece_a_proceso, (void *) destruir_elemento);
				log_info(logger, "Id a finalizar: %lu", id);

				break;
			case ERROR:
				log_error(logger, "Se desconecto el kernel");
				exit(EXIT_FAILURE);
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}
}


