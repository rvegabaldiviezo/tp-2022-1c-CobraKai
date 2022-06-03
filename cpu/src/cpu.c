#include "cpu.h"

t_log* logger;
t_list* lista;
int conexion_dispatch;
int conexion_interrupt;

int main(void) {

	// 1) loggear
	logger = log_create(PATH_LOG, "CPU", true, LOG_LEVEL_INFO);

	// 2) Conectarnos con la memoria  (no hacerlo)
	/*
		int server_cpu = inciar_servidor();
		conexion_memoria = esperar_cliente(server_cpu);

		if(!conexion_exitosa(conexion_memoria)) {
			log_error(logger, "No se pudo establecer la conexion con el kernel");
			return EXIT_FAILURE;
		}

		int operacion = recibir_operacion(conexion_memoria);  */

	// 3) Levantar server puerto DISPATCH
	int server_dispatch = iniciar_servidor_dispatch();
	conexion_dispatch= esperar_cliente(server_dispatch);


	// 4) Levantar server puerto INTERRUPT
	//int server_interrupt = iniciar_servidor_interrupt();
	//conexion_interrupt = esperar_cliente(server_interrupt);
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
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida");
			break;
		}
	}
	return EXIT_SUCCESS;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
//bool conexion_exitosa(int cliente) {
//	return cliente != -1;
//}
