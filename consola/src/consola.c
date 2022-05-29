/*
 * Para correr desde terminal:
 * 1. Compilo consola.c
 * 2. Levanto el kernel (explicado en kernel.c)
 * 3. Me paro en tp-2022-1c-CobraKai/consola
 * 4. Escribo ./Debug/consola "src/pseudocodigo.txt" "tamaño"
 */

#include "consola.h" // Los includes necesarios van en consola.h, aca solo va este

// Variables Globales
int conexion_con_kernel;
t_log* logger;
t_config* config;
t_proceso* proceso;

int main(int argc, char** argv) {

	logger = log_create("./src/consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

	if (!cantidad_parametros_correcta(argc)) {
		log_error(logger, "Cantidad de parametros incorrecta, abortando...");
		log_destroy(logger);
		return EXIT_FAILURE;
	}

	config = config_create(PATH_CONFIG);
	char* ip_kernel = config_get_string_value(config, IP_KERNEL);
	char* puerto_kernel = config_get_string_value(config, PUERTO_KERNEL);

	char* path_pseudocodigo = string_new();
	string_append(&path_pseudocodigo, argv[1]); // "src/pseudocodigo.txt"
	int tamanio_proceso = atoi(argv[2]);

	conexion_con_kernel = crear_conexion(ip_kernel, puerto_kernel);

	proceso = crear_proceso();
	proceso = cargar_proceso(tamanio_proceso, path_pseudocodigo);

	enviar_a_kernel(proceso, conexion_con_kernel);

	int respuesta = recibir_respuesta(conexion_con_kernel);

	if(respuesta > 0) {
		log_info(logger, "El proceso finalizó correctamente");
	} else {
		log_error(logger, "Ocurrió un error durante la ejecucion del proceso");
		terminar_programa();
		return EXIT_FAILURE;
	}

	terminar_programa();

	return EXIT_SUCCESS;
}

t_proceso* crear_proceso() {
	t_proceso* proceso = malloc(sizeof(t_proceso));
	crear_buffer(proceso);
	return proceso;
}

t_proceso* cargar_proceso(int tamanio, char* path_pseudocodigo) {
	proceso->operacion = LISTA_DE_INSTRUCCIONES;
	proceso->tamanio = tamanio;
	char* instruccion;
	FILE* pseudocodigo = fopen(path_pseudocodigo, "rt");
	instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);
	while(!feof(pseudocodigo)) {
		agregar_instruccion(proceso, instruccion, string_length(instruccion) + 1);
		instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);
	}
	agregar_instruccion(proceso, instruccion, string_length(instruccion) + 1);
	free(instruccion);
	fclose(pseudocodigo);
	return proceso;
}

bool cantidad_parametros_correcta(int cantidad) {
	return cantidad == 3;
}

void terminar_programa() {
	config_destroy(config);
	log_destroy(logger);
	eliminar_proceso(proceso);
	liberar_conexion(conexion_con_kernel);
}
