/*
 todo: handshake cpu
 	 enviar cantidad de entradas por tabla de paginas
 	 tamaño de pagina
 */


#include "memoria.h"

int main(void) {

	t_log* logger = log_create("./memoria.log", "MEMORIA", true, LOG_LEVEL_DEBUG);
	int server_memoria = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir clientes");

	int conexion_kernel = esperar_cliente(server_memoria);

	if(!conexion_exitosa(conexion_kernel)) {
		log_error(logger, "No se pudo establecer la conexion con el kernel");
		return EXIT_FAILURE;
	}

	// crear tablas de paginas
	int numero_de_pagina = 5; // para probar
	enviar_numero_de_tabla(conexion_kernel, numero_de_pagina);

	liberar_conexion(conexion_kernel);

	return EXIT_SUCCESS;
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}
