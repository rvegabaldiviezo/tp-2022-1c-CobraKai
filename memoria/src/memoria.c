#include "memoria.h"

pthread_t hilo_cpu;
pthread_t hilo_kernel;

t_log* logger;
t_config* config;
int server_memoria;
int conexion_kernel;
int conexion_cpu;
t_list* tabla_primer_nivel;
unsigned int entradas;
char* path_swap;

int main(void) {

	logger = log_create(PATH_LOG, "MEMORIA", true, LOG_LEVEL_DEBUG);

	config = config_create(PATH_CONFIG);

	entradas = config_get_int_value(config, KEY_ENTRADAS_TABLA);
	path_swap = config_get_string_value(config, KEY_PATH_SWAP);
	string_append(&path_swap, "/");

	tabla_primer_nivel = list_create();

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

int crear_tabla_paginas() {
	t_tabla_paginas_segundo_nivel* tabla = inicializar_tabla_segundo_nivel();
	list_add(tabla_primer_nivel, tabla);
	return list_size(tabla_primer_nivel);
}

t_tabla_paginas_segundo_nivel* inicializar_tabla_segundo_nivel() {
	t_tabla_paginas_segundo_nivel* tabla = malloc(sizeof(t_tabla_paginas_segundo_nivel));
	tabla->inicializada = true;
	tabla->marco = -1;
	tabla->modificada = false;
	tabla->presencia = true; // true??
	tabla->usada = false;
	return tabla;
}

void crear_archivo_swap(char* path) {
	FILE* f = fopen(path, "w");
	fclose(f);
}

char* get_path_archivo(pid_t id) {
	char* extension = ".swap";
	char* nombre = string_new();
	char* aux = string_new();
	string_append(&aux, path_swap);
	nombre = string_itoa(id);
	string_append(&nombre, extension);
	string_append(&aux, nombre);
	return aux;
}

void terminar_programa() {
	pthread_join(hilo_cpu, NULL);
	pthread_join(hilo_kernel, NULL);
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
				//pid_t id_proceso = getpid();
				int numero_de_tabla = crear_tabla_paginas();

				crear_archivo_swap(get_path_archivo(id_proceso));

				enviar_numero_de_tabla(conexion_kernel, numero_de_tabla);

				break;
			case SUSPENCION_PROCESO:
				log_info(logger, "Kernel solicita SUSPENCION PROCESO");
				break;
			case FINALIZACION_PROCESO:
				log_info(logger, "Kernel solicita FINALIZACION PROCESO");
				pid_t id = recibir_id_proceso(conexion_kernel);
				remove(get_path_archivo(id));
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


