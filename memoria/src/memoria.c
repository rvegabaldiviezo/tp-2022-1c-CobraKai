#include "memoria.h"

pthread_t hilo_cpu;
pthread_t hilo_kernel;

t_log* logger;
t_config* config;
int server_memoria;
int conexion_kernel;
int conexion_cpu;
t_tabla_paginas* tabla_primer_nivel;
unsigned int entradas;


int main(void) {

	logger = log_create(PATH_LOG, "MEMORIA", true, LOG_LEVEL_DEBUG);

	config = config_create(PATH_CONFIG);

	entradas = config_get_int_value(config, KEY_ENTRADAS_TABLA);

	inicializar_tabla_paginas();

	server_memoria = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir clientes");

	pthread_create(&hilo_cpu, NULL, (void *) atender_cpu, NULL);
	pthread_create(&hilo_cpu, NULL, (void *) atender_kernel, NULL);

	terminar_programa();
	return EXIT_SUCCESS;
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}

void crear_tabla_paginas() {
	t_tabla_paginas tabla;
	for(int i = 0; i < entradas; i++) {
		if(tabla_primer_nivel[i].inicializada == false) {
			// TODO: ver bien como se inicializa la tabla y crear una tabla por proceso
			//tabla.marco = i;
			//tabla.presencia = true;
			tabla.inicializada = true;
			tabla_primer_nivel[i] = tabla;
			log_info(logger, "Se creo la tabla numero: %d", i);
			break;
		}
	}

	if(tabla.inicializada == true) {
		enviar_numero_de_tabla(conexion_kernel, tabla.marco);
	} else { //TODO: Preguntar si enviar error o esperar a que se libere un espacio
		log_info(logger, "Se alcanzó el maximo de entradas por tabla");
		enviar_numero_de_tabla(conexion_kernel, -1);
	}

}

void inicializar_tabla_paginas() {
	tabla_primer_nivel = calloc(entradas, sizeof(t_tabla_paginas));
}

void terminar_programa() {
	pthread_join(hilo_cpu, NULL);
	pthread_join(hilo_kernel, NULL);
}

void atender_cpu() {
	conexion_cpu = esperar_cliente(server_memoria);

	if(!conexion_exitosa(conexion_cpu)) {
		log_error(logger, "No se pudo establecer la conexion con la cpu");
		exit(EXIT_FAILURE);
	}

	while(1) {
		int operacion = recibir_operacion(conexion_kernel);
		switch(operacion) {
			case ACCESO_TABLA_PRIMER_NIVEL:
				log_info(logger, "CPU solicita acceso a tabla pagina de primer nivel");

				break;
			case ACCESO_TABLA_SEGUNDO_NIVEL:
				log_info(logger, "CPU solicita acceso a tabla pagina de segundo nivel");
				break;
			case LECTURA_MEMORIA_USUARIO:
				log_info(logger, "CPU solicita lectura a memoria de usuario");
				break;
			case ESCRITURA_MEMORIA_USUARIO:
				log_info(logger, "CPU solicita escritura a memoria de usuario");
				break;
			case ERROR_CPU:
				log_error(logger, "Se desconecto la cpu");
				exit(EXIT_FAILURE);
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}
}

void atender_kernel() {
	conexion_kernel = esperar_cliente(server_memoria);

	if(!conexion_exitosa(conexion_kernel)) {
		log_error(logger, "No se pudo establecer la conexion con el kernel");
		exit(EXIT_FAILURE);
	}

	while(1) {
		int operacion = recibir_operacion(conexion_kernel);
		switch(operacion) {
			case INICIO_PROCESO:
				log_info(logger, "Kernel solicita INICIO PROCESO");
				pthread_t hilo_inicio_proceso;
				pthread_create(&hilo_inicio_proceso, NULL, (void*) crear_tabla_paginas, NULL);
				pthread_join(hilo_inicio_proceso, NULL);

				break;
			case SUSPENCION_PROCESO:
				log_info(logger, "Kernel solicita SUSPENCION PROCESO");
				break;
			case FINALIZACION_PROCESO:
				log_info(logger, "Kernel solicita FINALIZACION PROCESO");
				break;
			case ERROR_KERNEL:
				log_error(logger, "Se desconecto el kernel");
				exit(EXIT_FAILURE);
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}
}


