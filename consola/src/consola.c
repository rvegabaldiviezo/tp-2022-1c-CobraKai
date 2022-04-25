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
		return EXIT_FAILURE;
	}

	t_config* config = config_create(argv[1]); // RUTA: "src/consola.config"
	// en ningun momento se usa argv[2], supongo que debería usarse
	char* ip = config_get_string_value(config, IP_KERNEL);
	char* puerto = config_get_string_value(config, PUERTO_KERNEL);

	t_paquete* paquete = armar_paquete();
	int conexion = crear_conexion(ip, puerto);
	enviar_paquete(paquete, conexion);

	terminar_programa(conexion, logger, config);

	return EXIT_SUCCESS;
}

t_paquete* armar_paquete() {
	t_paquete* paquete = crear_paquete();
	char* instruccion;
	FILE* pseudocodigo = fopen("./src/pseudocodigo.txt", "rt");
	instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);
	while(!feof(pseudocodigo)) {
		agregar_a_paquete(paquete, instruccion, string_length(instruccion) + 1);
		instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);
	}
	agregar_a_paquete(paquete, instruccion, string_length(instruccion) + 1);
	free(instruccion);
	fclose(pseudocodigo);
	return paquete;
}

bool cantidad_parametros_correcta(int cantidad) {
	return cantidad == 3;
}

void terminar_programa(int conexion, t_log* logger, t_config* config) {
	config_destroy(config);
	log_destroy(logger);
	liberar_conexion(conexion);
}

