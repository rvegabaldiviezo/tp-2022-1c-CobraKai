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

	//t_queue* cola_new = queue_create();

	// TODO: reemplazar lista por colas (ver commons/collections/queue.h), ver bien como implementar los posibles estados

	t_list* instrucciones;

	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			instrucciones = recibir_paquete(cliente_fd);
			t_proceso* proceso = crear_proceso(instrucciones, sizeof(instrucciones)); /* sizeof(instrucciones) da siempre 4?? */
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(instrucciones, (void*) iterator);
			log_info(logger, "tamanio de proceso: %s", string_itoa(proceso->pcb.tamanio_proceso));
			free(proceso);
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

t_proceso* crear_proceso(t_list* instrucciones, unsigned int tamanio) {
	t_proceso* proceso = malloc(sizeof(*proceso));
	t_pcb pcb = crear_pcb(tamanio);
	proceso->pcb = pcb;
	proceso->instrucciones = *instrucciones;
	return proceso;
}

t_pcb crear_pcb(unsigned int tamanio) {
	t_pcb pcb;
	pcb.id = 0;
	pcb.estimacion_rafaga = 0;
	pcb.program_counter = 0;
	pcb.tablas_paginas = 0;
	pcb.tamanio_proceso = tamanio;
	return pcb;
} // crea un pcb muy basico, ni siquiera se si está bien, es solo para probar

void terminar_programa(t_log* logger) {
	log_destroy(logger);
}
