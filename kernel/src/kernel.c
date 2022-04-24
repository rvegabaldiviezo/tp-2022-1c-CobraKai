/*
 * Para correr desde terminal:
 * 1. Compilo kernel.c
 * 2. Me paro en tp-2022-1c-CobraKai/kernel
 * 3. Escribo ./Debug/kernel
 */

#include "kernel.h"

int main(void) {

	t_log* logger = log_create("./kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);

	int server_fd = iniciar_servidor();

	log_info(logger, "Kernel listo para recibir al cliente");

	int cliente_fd = esperar_cliente(server_fd);

	if (!conexion_exitosa(cliente_fd)) {
		log_error(logger, "No se pudo establecer la conexion con el cliente, abortando proceso");
		return EXIT_FAILURE;
	}

	// TODO: reemplazar lista por colas (ver commons/collections/queue.h), ver bien como implementar los posibles estados

	t_list* lista;

	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando Kernel");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida");
			break;
		}
	}
	terminar_programa(logger);
	return EXIT_SUCCESS;
}

void iterator(char* value) {
	t_log* logger = log_create("./kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
	log_info(logger,"%s", value);
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}

void terminar_programa(t_log* logger) {
	log_destroy(logger);
}
