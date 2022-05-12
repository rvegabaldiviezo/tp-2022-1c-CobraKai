/*
 * Para correr desde terminal:
 * 1. Compilo consola.c
 * 2. Levanto el kernel (explicado en kernel.c)
 * 3. Me paro en tp-2022-1c-CobraKai/consola
 * 4. Escribo ./Debug/consola "src/consola.config" "tamaño"
 */

#include "consola.h" // Los includes necesarios van en consola.h, aca solo va este

int main(int argc, char** argv) {
	t_log* logger = log_create("./src/consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

	if (!cantidad_parametros_correcta(argc)) {
		log_error(logger, "Cantidad de parametros incorrecta, abortando...");
		log_destroy(logger);
		return EXIT_FAILURE;
	}

	t_config* config = config_create(argv[1]); // RUTA: "src/consola.config"
	char* ip_kernel = config_get_string_value(config, IP_KERNEL);
	char* puerto_kernel = config_get_string_value(config, PUERTO_KERNEL);

	int conexion_con_kernel = crear_conexion(ip_kernel, puerto_kernel);
	t_proceso* proceso = crear_proceso(atoi(argv[2]));

	proceso = cargar_proceso(proceso);

	enviar_a_kernel(proceso, conexion_con_kernel);

	terminar_programa(conexion_con_kernel, logger, config, proceso);

	return EXIT_SUCCESS;
}

t_proceso* cargar_proceso(t_proceso* proceso) {
	proceso->operacion = LISTA_DE_INSTRUCCIONES;
	char* instruccion;
	FILE* pseudocodigo = fopen("./src/pseudocodigo.txt", "rt");
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

void terminar_programa(int conexion, t_log* logger, t_config* config, t_proceso* proceso) {
	config_destroy(config);
	log_destroy(logger);
	//liberar_conexion(conexion);
	eliminar_proceso(proceso);
}
