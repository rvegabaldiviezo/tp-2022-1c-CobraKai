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

	int socket_consola = esperar_cliente(server_fd);

	if (!conexion_exitosa(socket_consola)) {
		log_error(logger, "No se pudo establecer la conexion con el cliente");
		log_destroy(logger);
		return EXIT_FAILURE;
	}

	t_config* config = config_create(PATH_CONFIG);
	char* ip_memoria = config_get_string_value(config, IP_MEMORIA);
	char* puerto_memoria = config_get_string_value(config, PUERTO_MEMORIA);
	int conexion_con_memoria = crear_conexion(ip_memoria, puerto_memoria);
	log_info(logger, "conectado a memoria");

	t_proceso* proceso;

	while(1) {
		int operacion_consola = recibir_operacion(socket_consola);
		switch(operacion_consola) {
			case LISTA_DE_INSTRUCCIONES:
				proceso = recibir_proceso(socket_consola);

				if(proceso->tamanio == -1) {
					log_error(logger, "Ocurrió un error al recibir el tamanio del proceso");
					log_destroy(logger);
					return EXIT_FAILURE;
				}

				// Ya se copiaron todos los bytes que venian de la consola
				// de ahora en mas puedo agregar lo que quiera al proceso
				proceso->pcb = crear_pcb(proceso->tamanio);

				char* tamanio_recibido = string_new();
				string_append(&tamanio_recibido, "Tamanio recibido: ");
				string_append(&tamanio_recibido, string_itoa(proceso->tamanio));
				log_info(logger, tamanio_recibido);

				log_info(logger, "Me llegaron los siguientes valores:");
				list_iterate(proceso->instrucciones, (void*) iterator);


				int numero_de_tabla = recibir_numero_de_tabla(conexion_con_memoria);
				if(!numero_de_tabla_valido(numero_de_tabla)) {
					log_error(logger, "El número de tabla no es valido");
					return EXIT_FAILURE;
				}

				// asignar proceso a estado NEW

				proceso->pcb.tablas_paginas = numero_de_tabla;
				log_info(logger, "Se asignó el numero de tabla: %d al proceso de id: %d %s", numero_de_tabla, proceso->pcb.id, "\n");

				enviar_respuesta_exitosa(socket_consola);
				liberar_conexion(socket_consola);
				break;

			case ERROR:
				log_error(logger, "La consola se desconectó inesperadamente");
				return EXIT_FAILURE;
				break;

			default:
				log_info(logger, "Operacion desconocida");
				break;
		}

	}

	terminar_programa(logger, server_fd, socket_consola);

	return EXIT_SUCCESS;
}

void iterator(char* value) {
	t_log* logger = log_create("./kernel.log", "KERNEL", true, LOG_LEVEL_DEBUG);
	log_info(logger,"%s", value);
	log_destroy(logger);
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}

t_pcb crear_pcb(unsigned int tamanio) {
	t_pcb pcb;
	pcb.id = getpid();
	pcb.estimacion_rafaga = 0;
	pcb.program_counter = 0;
	pcb.tablas_paginas = 0;
	pcb.tamanio_proceso = tamanio;
	return pcb;
} // crea un pcb muy basico, ni siquiera se si está bien, es solo para probar

void terminar_programa(t_log* logger, int socket_servidor, int socket_cliente) {
	log_destroy(logger);
	liberar_conexion(socket_servidor);
	liberar_conexion(socket_cliente);
}

bool numero_de_tabla_valido(int numero) {
	return numero != -1;
}
