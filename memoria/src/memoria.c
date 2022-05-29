#include "memoria.h"

t_log* logger;
int conexion_kernel;

int main(void) {

	logger = log_create(PATH_LOG, "MEMORIA", true, LOG_LEVEL_DEBUG);
	int server_memoria = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir clientes");

	conexion_kernel = esperar_cliente(server_memoria);

	if(!conexion_exitosa(conexion_kernel)) {
		log_error(logger, "No se pudo establecer la conexion con el kernel");
		return EXIT_FAILURE;
	}

	while(1) {
		int operacion = recibir_operacion(conexion_kernel);
		switch(operacion) {
			case INICIO_PROCESO:
				log_info(logger, "Kernel solicita INICIO PROCESO");
				pthread_t hilo_inicio_proceso;
				pthread_create(&hilo_inicio_proceso, NULL, (void*) crear_tabla_paginas, NULL);
				pthread_join(hilo_inicio_proceso, NULL);

				// crear tablas de paginas

				break;
			case SUSPENCION_PROCESO:
				log_info(logger, "Kernel solicita SUSPENCION PROCESO");
				break;
			case FINALIZACION_PROCESO:
				log_info(logger, "Kernel solicita FINALIZACION PROCESO");
				break;
			case ERROR:
				log_error(logger, "Se desconecto el cliente");
				return EXIT_FAILURE;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	return EXIT_SUCCESS;
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}

void crear_tabla_paginas() {
	enviar_numero_de_tabla(conexion_kernel, 123);
}
