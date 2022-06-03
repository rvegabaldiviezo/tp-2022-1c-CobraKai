/*
 * Para correr desde terminal:
 * 1. Compilo kernel.c
 * 2. Me paro en tp-2022-1c-CobraKai/kernel
 * 3. Escribo ./Debug/kernel
 */

#include "kernel.h"

// Hilos
pthread_t planificador_srt;

// Semaforos Mutex
pthread_mutex_t mutex_ready_queue;
pthread_mutex_t mutex_new_queue;

// Semaforos
sem_t sem_ready;
sem_t sem_new;

// Colas / Listas
t_queue* new;
t_list* ready;
t_queue* blocked;
t_queue* susp_blocked;
t_queue* susp_ready;

// Generales
//t_proceso* proceso;
t_log* logger;
t_config* config;
int conexion_consola;
int socket_servidor;
int conexion_con_memoria;
float alfa;
int grado_multiprogramacion;
char* planificador;

int main(void) {

	logger = log_create(PATH_LOG, "KERNEL", true, LOG_LEVEL_DEBUG);

	config = config_create(PATH_CONFIG);

	alfa = config_get_int_value(config, ALFA);
	grado_multiprogramacion = config_get_int_value(config, GRADO_MULTIPROGRAMACION);
	planificador = config_get_string_value(config, ALGORITMO_PLANIFICACION);

	socket_servidor = iniciar_servidor();

	log_info(logger, "Kernel listo para recibir al cliente");

	char* ip_memoria = config_get_string_value(config, IP_MEMORIA);
	char* puerto_memoria = config_get_string_value(config, PUERTO_MEMORIA);
	conexion_con_memoria = crear_conexion(ip_memoria, puerto_memoria);

	inicializar_colas();
	inicializar_semaforos();

	while(1) {
		conexion_consola = esperar_cliente(socket_servidor);
		if (!conexion_exitosa(conexion_consola)) {
			log_error(logger, "No se pudo establecer la conexion con el cliente");
			log_destroy(logger);
			return EXIT_FAILURE;
		}
		pthread_t hilo_consola;
		pthread_create(&hilo_consola, NULL, (void*) atender_consola, NULL);
		//pthread_join(hilo_consola, NULL);
		log_info(logger, "Finalizo el proceso %lu", hilo_consola);
		enviar_respuesta_exitosa(conexion_consola);
	}

	terminar_programa();

	return EXIT_SUCCESS;
}

pid_t atender_consola() {
	int operacion_consola = recibir_operacion(conexion_consola);
	t_proceso* proceso;
	pthread_mutex_init(&mutex_ready_queue, NULL);
	pthread_mutex_init(&mutex_new_queue, NULL);
	switch(operacion_consola) {
	case LISTA_DE_INSTRUCCIONES:

		proceso = crear_proceso();
		proceso = recibir_proceso(conexion_consola);
		proceso->socket = conexion_consola;
		proceso->pcb.estimacion_rafaga = config_get_int_value(config, ESTIMACION_INICIAL);
		proceso->pcb.id = pthread_self();
		// Calculo de la rafaga
		// prox_rafaga = alfa * tiempo_ultima_rafaga + (1 - alfa) * pcb.estimacion_rafaga
		if(proceso->pcb.tamanio_proceso == -1) {
			log_error(logger, "Ocurrió un error al recibir el tamanio del proceso");
			log_destroy(logger);
			return EXIT_FAILURE;
		}

		// Ya se copiaron todos los bytes que venian de la consola
		// de ahora en mas puedo agregar lo que quiera al proceso

		log_info(logger, "Tamaño recibido: %d", proceso->pcb.tamanio_proceso);


		int numero_de_tabla = recibir_numero_de_tabla(conexion_con_memoria);
		if(!numero_de_tabla_valido(numero_de_tabla)) {
			log_error(logger, "El número de tabla no es valido");
			return EXIT_FAILURE;
		}

		proceso->pcb.tablas_paginas = numero_de_tabla;
		log_info(logger, "Se asignó el numero de tabla: %d al proceso de id: %lu\n", proceso->pcb.tablas_paginas, proceso->pcb.id);

		pthread_mutex_lock(&mutex_new_queue);
		queue_push(new, proceso);
		sem_post(&sem_new);
		pthread_mutex_unlock(&mutex_new_queue);

		log_info(logger, "Proceso %lu asignado a la cola NEW\n", proceso->pcb.id);

		if(strcmp(planificador, "SRT") == 0) {
			pthread_create(&planificador_srt, NULL, (void*) planificar_srt, NULL);
			//pthread_join(planificador_srt, NULL);
		}

		break;

	case ERROR:
		log_error(logger, "La consola se desconectó inesperadamente");
		return EXIT_FAILURE;
		break;

	default:
		log_info(logger, "Operacion desconocida");
		break;
	}
	return proceso->pcb.id;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}

t_proceso* crear_proceso() {
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->socket = 0;
	proceso->pcb = crear_pcb();
	return proceso;
}

t_pcb crear_pcb() {
	t_pcb pcb;
	pcb.id = 0;
	pcb.tamanio_proceso = 0;
	pcb.instrucciones = list_create();
	pcb.program_counter = 0;
	pcb.estimacion_rafaga = 0;
	pcb.tablas_paginas = 0;
	return pcb;
}

void terminar_programa() {
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(socket_servidor);
	liberar_conexion(conexion_con_memoria);
	liberar_conexion(conexion_consola);
	sem_destroy(&sem_new);
	sem_destroy(&sem_ready);
}

bool numero_de_tabla_valido(int numero) {
	return numero != -1;
}

void inicializar_colas() {
	new = queue_create();
	ready = list_create();
	blocked = queue_create();
	susp_blocked = queue_create();
	susp_ready = queue_create();
}

void inicializar_semaforos() {
	sem_init(&sem_ready, 0, grado_multiprogramacion);
	sem_init(&sem_new, 0, 0);
}

void planificar_srt() {
	// TODO: implementar planificador posta
	// Agarro el mas corto de ready
	pthread_mutex_lock(&mutex_new_queue);
	sem_wait(&sem_new);
	t_proceso* proceso = queue_pop(new);
	pthread_mutex_unlock(&mutex_new_queue);

	pthread_mutex_lock(&mutex_ready_queue);
	sem_wait(&sem_ready);
	list_add(ready, proceso);
	log_info(logger, "Proceso %lu asignado a la cola READY", proceso->pcb.id);
	pthread_mutex_unlock(&mutex_ready_queue);

	// Pido a cpu pcb y tiempo restante de proceso que esté ejecutando y comparo con proceso_mas_corto_disponible
	// t_proceso* proceso_ejecutando = solicitar_a_cpu(REPLANIFICACION); -> implementar

	sleep(5); // solo para probar
	pthread_mutex_lock(&mutex_ready_queue);
	list_sort(ready, (void*) lista_mas_corta);
	t_proceso* proceso_mas_corto_disponible = list_pop(ready);
	sem_post(&sem_ready);
	pthread_mutex_unlock(&mutex_ready_queue);

	// comparo con estimacion del que llegó, mando al mas corto

	log_info(logger, "Me llegaron los siguientes valores:");
	list_iterate(proceso_mas_corto_disponible->pcb.instrucciones, (void*) iterator);

}

void * list_pop(t_list* list) {
	void * elemento = list_get(list, 0);
	list_remove(list, 0);
	return elemento;

}

t_proceso* menor_tiempo_restante(t_proceso* p1, t_proceso* p2) {
	if(p1->pcb.estimacion_rafaga > p2->pcb.estimacion_rafaga) {
		return p2;
	} else {
		return p1;
	}
}

// funcion para probar las prioridade de la cola ready
t_proceso* lista_mas_corta(t_proceso* p1, t_proceso* p2) {
	if(list_size(p1->pcb.instrucciones) > list_size(p2->pcb.instrucciones)) {
		return p2;
	} else {
		return p1;
	}
}
