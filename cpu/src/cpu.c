#include "cpu.h"

t_log* logger;
int conexion_memoria;

int main(void) {

	logger = log_create(PATH_LOG, "CPU", true, LOG_LEVEL_INFO);

	int server_cpu = inciar_servidor();
	conexion_memoria = esperar_cliente(server_cpu);

	if(!conexion_exitosa(conexion_memoria)) {
		log_error(logger, "No se pudo establecer la conexion con el kernel");
		return EXIT_FAILURE;
	}

	int operacion = recibir_operacion(conexion_memoria);



	return EXIT_SUCCESS;
}


bool conexion_exitosa(int cliente) {
	return cliente != -1;
}
