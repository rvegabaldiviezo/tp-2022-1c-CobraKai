/*
 * Para correr desde terminal:
 * 1. Compilo kernel.c
 * 2. Me paro en tp-2022-1c-CobraKai/kernel
 * 3. Escribo ./Debug/kernel
 */

#include "kernel.h"

t_log* logger;
t_config* config;
t_proceso* proceso;
int conexion_consola;
int socket_servidor;
int conexion_con_memoria;

int main(void) {

	logger = log_create(PATH_LOG, "KERNEL", true, LOG_LEVEL_DEBUG);

	socket_servidor = iniciar_servidor();

	log_info(logger, "Kernel listo para recibir al cliente");

	conexion_consola = esperar_cliente(socket_servidor);

	if (!conexion_exitosa(conexion_consola)) {
		log_error(logger, "No se pudo establecer la conexion con el cliente");
		log_destroy(logger);
		return EXIT_FAILURE;
	}

	config = config_create(PATH_CONFIG);
	char* ip_memoria = config_get_string_value(config, IP_MEMORIA);
	char* puerto_memoria = config_get_string_value(config, PUERTO_MEMORIA);
	conexion_con_memoria = crear_conexion(ip_memoria, puerto_memoria);
	log_info(logger, "conectado a memoria");

	while(1) {
		int operacion_consola = recibir_operacion(conexion_consola);
		switch(operacion_consola) {
			case LISTA_DE_INSTRUCCIONES:

				proceso = crear_proceso();
				//proceso->pcb = crear_pcb();
				proceso = recibir_proceso(conexion_consola);

				if(proceso->tamanio == -1) {
					log_error(logger, "Ocurrió un error al recibir el tamanio del proceso");
					log_destroy(logger);
					return EXIT_FAILURE;
				}

				// Ya se copiaron todos los bytes que venian de la consola
				// de ahora en mas puedo agregar lo que quiera al proceso


				char* tamanio_recibido = string_new();
				string_append(&tamanio_recibido, "Tamanio recibido: ");
				string_append(&tamanio_recibido, string_itoa(proceso->pcb.tamanio_proceso));
				log_info(logger, tamanio_recibido);

				log_info(logger, "Me llegaron los siguientes valores:");
				list_iterate(proceso->pcb.instrucciones, (void*) iterator);


				int numero_de_tabla = recibir_numero_de_tabla(conexion_con_memoria);
				if(!numero_de_tabla_valido(numero_de_tabla)) {
					log_error(logger, "El número de tabla no es valido");
					return EXIT_FAILURE;
				}
				liberar_conexion(conexion_con_memoria);
				// asignar proceso a estado NEW

				proceso->pcb.tablas_paginas = numero_de_tabla;
				log_info(logger, "Se asignó el numero de tabla: %d al proceso de id: %d %s", proceso->pcb.tablas_paginas, proceso->pcb.id, "\n");

				enviar_respuesta_exitosa(conexion_consola);
				//liberar_conexion(conexion_consola);
				//liberar_conexion(conexion_con_memoria);
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

	terminar_programa();

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

t_proceso* crear_proceso() {
	t_proceso* proceso;
	proceso->instrucciones = list_create();
	proceso->tamanio = 0;
	proceso->pcb = crear_pcb();
	return proceso;
}

t_pcb crear_pcb() {
	t_pcb pcb;
	pcb.id = getpid();
	pcb.tamanio_proceso = 0;
	pcb.instrucciones = list_create();
	pcb.program_counter = 0;
	pcb.estimacion_rafaga = 0;
	pcb.tablas_paginas = 0;
	return pcb;
} // crea un pcb muy basico, ni siquiera se si está bien, es solo para probar

void terminar_programa() {
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(socket_servidor);

	liberar_conexion(conexion_consola);
}

bool numero_de_tabla_valido(int numero) {
	return numero != -1;
}
