/*
 * Para correr desde terminal:
 * 1. Compilo kernel.c
 * 2. Me paro en tp-2022-1c-CobraKai/kernel
 * 3. Escribo ./Debug/kernel
 */

#include "kernel.h"

// Hilos
pthread_t planificador_srt;
pthread_t planificador_fifo;
pthread_t planificador_io;
pthread_t hilo_consola;
pthread_t interrumpir_ejecucion;

// Semaforos
pthread_mutex_t mutex_new_queue;
pthread_mutex_t mutex_ready_list;
pthread_mutex_t i_o;
pthread_mutex_t mutex_blocked_list;
sem_t elementos_en_cola_bloqueados;
sem_t elementos_en_cola_ready;
sem_t sem_grado_multiprogramacion;


// Colas / Listas
t_queue* new;
t_list* ready;
t_list* blocked;
t_queue* blocked_fifo;
t_queue* susp_blocked;
t_queue* susp_ready;

// Generales
t_proceso* proceso;
t_log* logger;
t_config* config;
int conexion_consola;
int socket_servidor;
int conexion_con_memoria;
int conexion_con_cpu_dispatch;
int conexion_con_cpu_interrupt;

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

	config = config_create(PATH_CONFIG);
	char* ip_memoria = config_get_string_value(config, IP_MEMORIA);
	char* puerto_memoria = config_get_string_value(config, PUERTO_MEMORIA);
	conexion_con_memoria = crear_conexion(ip_memoria, puerto_memoria);

	log_info(logger, "conectado a memoria");

	inicializar_semaforos();
	inicializar_colas();
	iniciar_planificacion(planificador);

	while(1) {
		conexion_consola = esperar_cliente(socket_servidor);
		log_info(logger, "Conexion consola: %d", conexion_consola);
		if (!conexion_exitosa(conexion_consola)) {
			log_error(logger, "No se pudo establecer la conexion con el cliente");
			log_destroy(logger);
			return EXIT_FAILURE;
		}

		pthread_create(&hilo_consola, NULL, (void*) atender_consola, NULL);

		enviar_respuesta_exitosa(conexion_consola);
	}

	terminar_programa();

	return EXIT_SUCCESS;
}

pid_t atender_consola() {
	int operacion_consola = recibir_operacion(conexion_consola);
	switch(operacion_consola) {
	case LISTA_DE_INSTRUCCIONES:
		//TODO: como hacer para distinguir entre los distintos semaforos de los procesos?
		proceso = crear_proceso();
		proceso = recibir_proceso(conexion_consola);
		// Ya se copiaron todos los bytes que venian de la consola
		// de ahora en mas puedo agregar lo que quiera al proceso
		proceso->pcb.instrucciones = parsear_instrucciones(proceso->pcb.instrucciones);
		proceso->socket = conexion_consola;
		proceso->pcb.estimacion_rafaga = config_get_int_value(config, ESTIMACION_INICIAL);
		proceso->pcb.id = pthread_self();
		sem_t fin_de_proceso;
		sem_init(&fin_de_proceso, 0, 0);
		// Calculo de la rafaga
		// prox_rafaga = alfa * tiempo_ultima_rafaga + (1 - alfa) * pcb.estimacion_rafaga
		if(proceso->pcb.tamanio_proceso == -1) {
			log_error(logger, "Ocurrió un error al recibir el tamanio del proceso");
			log_destroy(logger);
			return EXIT_FAILURE;
		}

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
		pthread_mutex_unlock(&mutex_new_queue);
		log_info(logger, "Proceso %lu asignado a la cola NEW", proceso->pcb.id);

		//TODO: chequear cómo se va a calcular el grado de multiprogramacion
		//TODO: chequear si usar un semáforo, para que continue cuando se libere un proceso
		//if(list_size(ready) < grado_multiprogramacion) {

		pthread_mutex_lock(&mutex_new_queue);
		t_proceso* proceso = queue_pop(new);
		pthread_mutex_unlock(&mutex_new_queue);

		sem_wait(&sem_grado_multiprogramacion);
		log_info(logger, "El grado de multiprogramacion despues del WAIT es: %d", sem_grado_multiprogramacion);
		pthread_mutex_lock(&mutex_ready_list);
		list_add(ready, proceso);
		if(string_equals_ignore_case(planificador, "SRT")) {
			pthread_create(&interrumpir_ejecucion, NULL, (void *) solicitar_interrupcion, NULL);
			pthread_join(interrumpir_ejecucion, NULL);
		}
		pthread_mutex_unlock(&mutex_ready_list);
		sem_post(&elementos_en_cola_ready);

			/*if(strcmp(planificador, "SRT") == 0) {

				log_info(logger, "El proceso %lu fue asignado a la cola READY", proceso->pcb.id);
			} else if(strcmp(planificador, "FIFO") == 0) {
				pthread_mutex_lock(&mutex_ready_list);
				list_add(ready, proceso);
				pthread_mutex_unlock(&mutex_ready_list);
				log_info(logger, "El proceso %lu fue asignado a la cola READY", proceso->pcb.id);
				sem_post(&elementos_en_cola_ready);
			}*/
		//}
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
	if(strcmp(planificador, "SRT") == 0) {
		pthread_join(planificador_srt, NULL);
	} else {
		pthread_join(planificador_fifo, NULL);
	}
	pthread_join(planificador_io, NULL);
	pthread_join(hilo_consola, NULL);

	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(socket_servidor);
	liberar_conexion(conexion_con_memoria);
	liberar_conexion(conexion_consola);
}

bool numero_de_tabla_valido(int numero) {
	return numero != -1;
}

void planificar_fifo(){
		log_info(logger,"se inicio la planificacion FIFO");
		iniciar_planificacion_io();

		while(1){
			sem_wait(&elementos_en_cola_ready);

			pthread_mutex_lock(&mutex_ready_list);
			t_proceso* primer_proceso = list_pop(ready);
			pthread_mutex_unlock(&mutex_ready_list);
			sleep(5);
			list_iterate(primer_proceso->pcb.instrucciones, (void *) iterator);
			//TODO: se manda el pcb a la cpu
			//log_info(logger,"Se paso un proceso de Ready a Ejecutando");

			//semaforo que sincronice con cpu. Si el proceso terminó de ejecutar, continúa
			sem_post(&sem_grado_multiprogramacion);
			log_info(logger,"Un proceso termino de ejecutar");
			log_info(logger, "Finalizo el proceso %lu", hilo_consola);

		}
}


void comunicacion_con_cpu() {
	int operacion = recibir_operacion(conexion_con_cpu_dispatch);
		switch(operacion) {
			case BLOQUEO_IO:
				log_info(logger, "La CPU envio un pcb con estado bloqueado por I/0");
				t_proceso_bloqueado* proceso_bloqueado;
				int tiempo_de_espera = recibir_tiempo_bloqueo(conexion_con_cpu_dispatch);
				//TODO
				//t_pcb* pcb = recibir_pcb(conexion_con_cpu);
				//TODO: agregar pcb y tiempo a proceso_bloqueado
				agregar_a_bloqueados(proceso_bloqueado);


				break;

			case EXIT:
				log_info(logger, "La CPU envio un pcb con estado finalizado");
			    t_proceso* proceso = recibir_proceso(conexion_con_cpu_dispatch);
			    enviar_respuesta_exitosa(proceso->socket);
			    //TODO:
			    //avisar a memoria
			    //avisar a consola
				break;
			case ERROR_CPU:
				log_error(logger, "Se desconecto el cliente");
				return;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
}

void solicitar_interrupcion() {
	enviar_interrupcion(conexion_con_cpu_interrupt);
}


void agregar_a_bloqueados(t_proceso_bloqueado* proceso){
	pthread_mutex_lock(&mutex_blocked_list);
	list_add(blocked, proceso);
	pthread_mutex_unlock(&mutex_blocked_list);
	sem_post(&elementos_en_cola_bloqueados);
	log_info(logger, "Se agrego un proceso a la cola de bloqueados por I/O");
}

void planificacion_io(){
	while(1){
		sem_wait(&elementos_en_cola_bloqueados);
		t_proceso_bloqueado* primer_proceso = queue_pop(blocked_fifo);

		usleep(primer_proceso->tiempo_de_bloqueo*1000);
		log_info(logger, "Un proceso finalizo su I/0");

		pthread_mutex_lock(&mutex_ready_list);
		list_add(ready, primer_proceso->proceso);
		pthread_mutex_unlock(&mutex_ready_list);
		sem_post(&elementos_en_cola_ready);

		log_info(logger, "Se paso un proceso de bloqueado a ready");
	}
}

void iniciar_planificacion_io(){
	pthread_create(&planificador_io, NULL, (void*) planificacion_io, NULL);
	log_info(logger, "Inicio la planificacion IO");

}


//TODO
int recibir_tiempo_bloqueo(){
	return 1;
}

void inicializar_colas() {
	new = queue_create();
	ready = list_create();
	blocked = list_create();
	susp_blocked = queue_create();
	susp_ready = queue_create();
}

void inicializar_semaforos() {
	pthread_mutex_init(&mutex_ready_list, NULL);
	pthread_mutex_init(&mutex_blocked_list, NULL);
	pthread_mutex_init(&i_o, NULL);
	sem_init(&elementos_en_cola_bloqueados, 0, 0);
	sem_init(&elementos_en_cola_ready, 0, 0);
	sem_init(&sem_grado_multiprogramacion, 0, grado_multiprogramacion);

}

void planificar_srt() {
	// TODO: implementar planificador posta
	log_info(logger, "Se inicio la planificacion SRT");
	iniciar_planificacion_io();
	while(1) {
		sem_wait(&elementos_en_cola_ready);
		sleep(5);
		pthread_mutex_lock(&mutex_ready_list);
		if(list_size(ready) > 1) {
			list_sort(ready, (void *) lista_mas_corta);
		}
		t_proceso* proceso_mas_corto = list_pop(ready);
		pthread_mutex_unlock(&mutex_ready_list);

		sem_post(&sem_grado_multiprogramacion);
		log_info(logger, "El grado de multiprogramacion despues del POST es: %d", sem_grado_multiprogramacion);
		// solo para ver si ordena bien
		log_info(logger, "Me llegaron los siguientes valores:");
		list_iterate(proceso_mas_corto->pcb.instrucciones, (void*) iterator);
	}

}

void iniciar_planificacion(char* planificacion){
	if(strcmp(planificacion, "FIFO") == 0) {
		pthread_create(&planificador_fifo, NULL, (void*) planificar_fifo, NULL);
	} else if(strcmp(planificacion, "SRT") == 0) {
		pthread_create(&planificador_srt, NULL, (void*) planificar_srt, NULL);
	}
}

void* list_pop(t_list* lista) {
	void* elemento = list_get(lista, 0);
	list_remove(lista, 0);
	return elemento;
}

bool lista_mas_corta(t_proceso* p1, t_proceso* p2) {
	return (list_size(p1->pcb.instrucciones) < list_size(p2->pcb.instrucciones));
}

t_proceso* menor_tiempo_restante(t_proceso* p1, t_proceso* p2) {
	if(p1->pcb.estimacion_rafaga > p2->pcb.estimacion_rafaga) {
		return p2;
	} else {
		return p1;
	}
}
