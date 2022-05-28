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

	int operacion = recibir_operacion(conexion_kernel);

	while(1) {
		switch(operacion) {
			case INICIO_PROCESO:
				log_info(logger, "Kernel solicita INICIO PROCESO");
				// crear tablas de paginas
				enviar_numero_de_tabla(conexion_kernel, 123);
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

	liberar_conexion(conexion_kernel);

	return EXIT_SUCCESS;
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}
