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
		log_error(logger, "No se pudo establecer la conexion con el cliente");
		log_destroy(logger);
		return EXIT_FAILURE;
	}

	t_proceso* proceso = recibir_proceso(cliente_fd);

	if(proceso->tamanio == -1) {
		log_error(logger, "Ocurrió un error al recibir el tamanio del proceso");
		log_destroy(logger);
		return EXIT_FAILURE;
	}

	char* tamanio_recibido = string_new();
	string_append(&tamanio_recibido, "Tamanio recibido: ");
	string_append(&tamanio_recibido, string_itoa(proceso->tamanio));
	log_info(logger, tamanio_recibido);

	log_info(logger, "Me llegaron los siguientes valores:\n");
	list_iterate(proceso->instrucciones, (void*) iterator);

	// Ya se copiaron todos los bytes que venian de la consola
	// de ahora en mas puedo agregar lo que quiera al proceso
	proceso->pcb = crear_pcb(proceso->tamanio);
	terminar_programa(logger, proceso);

	return EXIT_SUCCESS;
}

void iterator(char* value) {
	t_log* logger = log_create("./kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
	log_info(logger,"%s", value);
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
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

void terminar_programa(t_log* logger, t_proceso* proceso) {
	log_destroy(logger);
	destruir_proceso(proceso);
}
