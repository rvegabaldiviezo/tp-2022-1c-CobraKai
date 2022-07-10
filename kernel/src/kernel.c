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
pthread_t hilo_cpu_dispatch;

// Semaforos
pthread_mutex_t mutex_new_queue;
pthread_mutex_t mutex_ready_list;
pthread_mutex_t i_o;
pthread_mutex_t mutex_blocked_list;
pthread_mutex_t mutex_susp_ready_queue;
sem_t elementos_en_cola_bloqueados;
sem_t elementos_en_cola_ready;
sem_t multiprogramacion;
sem_t elementos_en_cola_susp_ready;
sem_t cola_susp_ready_vacia;//todo, como se hace este semaforoo?
sem_t sem_planificacion;


// Colas / Listas
t_queue* new;
t_list* ready;
t_list* blocked;
t_queue* susp_ready;

// Generales
t_pcb* proceso;
t_log* logger;
t_config* config;
int conexion_consola;
int socket_servidor;
int conexion_con_memoria;
int tiempo_max_bloqueo;
int conexion_con_cpu_dispatch;
int conexion_con_cpu_interrupt;

float alfa;
int grado_multiprogramacion;
char* planificador;
int numero_proceso = 0;

int main(void) {

	config = config_create(PATH_CONFIG);

	grado_multiprogramacion = config_get_int_value(config, GRADO_MULTIPROGRAMACION);

	logger = log_create(PATH_LOG, "KERNEL", true, LOG_LEVEL_DEBUG);

	alfa = config_get_int_value(config, ALFA);
	planificador = config_get_string_value(config, ALGORITMO_PLANIFICACION);
	tiempo_max_bloqueo = config_get_int_value(config, TIEMPO_MAXIMO_BLOQUEADO);

	socket_servidor = iniciar_servidor();

	log_info(logger, "Kernel listo para recibir al cliente");

	config = config_create(PATH_CONFIG);
	char* ip_memoria = config_get_string_value(config, IP_MEMORIA);
	char* puerto_memoria = config_get_string_value(config, PUERTO_MEMORIA);
	conexion_con_memoria = crear_conexion(ip_memoria, puerto_memoria);

	char* ip_cpu = config_get_string_value(config, "IP_CPU");
	char* puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	conexion_con_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch);

	log_info(logger, "conectado a memoria");

	inicializar_semaforos();
	inicializar_colas();
	iniciar_planificacion(planificador);
	escuchar_cpu_dispatch();

	while(1) {
		conexion_consola = esperar_cliente(socket_servidor);
		log_info(logger, "Conexion consola: %d", conexion_consola);
		if (!conexion_exitosa(conexion_consola)) {
			log_error(logger, "No se pudo establecer la conexion con el cliente");
			log_destroy(logger);
			return EXIT_FAILURE;
		}

		pthread_create(&hilo_consola, NULL, (void*) atender_consola, NULL);

	}

	terminar_programa();

	return EXIT_SUCCESS;
}

pid_t atender_consola() {
	operacion operacion_consola = recibir_operacion(conexion_consola);
	t_pcb* proceso;
	switch(operacion_consola) {
	case LISTA_DE_INSTRUCCIONES:
		proceso = recibir_proceso(conexion_consola);
		// Ya se copiaron todos los bytes que venian de la consola
		// de ahora en mas puedo agregar lo que quiera al proceso
		proceso->instrucciones = parsear_instrucciones(proceso->instrucciones);
		proceso->socket = conexion_consola;
		proceso->estimacion_rafaga = config_get_int_value(config, ESTIMACION_INICIAL);
		proceso->id = numero_proceso;
		proceso->tablas_paginas = 0;
		proceso->program_counter = 0;
		numero_proceso++;
		// Calculo de la rafaga
		// prox_rafaga = alfa * tiempo_ultima_rafaga + (1 - alfa) * pcb.estimacion_rafaga
		if(proceso->tamanio_proceso == -1) {
			log_error(logger, "Ocurrió un error al recibir el tamanio del proceso");
			log_destroy(logger);
			return EXIT_FAILURE;
		}

		char* tamanio_recibido = string_new();
		string_append(&tamanio_recibido, "Tamanio recibido: ");
		string_append(&tamanio_recibido, string_itoa(proceso->tamanio_proceso));

		log_info(logger, "Tamaño recibido: %d", proceso->tamanio_proceso);

		int numero_de_tabla = recibir_numero_de_tabla(proceso, conexion_con_memoria);
		if(!numero_de_tabla_valido(numero_de_tabla)) {
			log_error(logger, "El número de tabla no es valido");
			return EXIT_FAILURE;
		}

		proceso->tablas_paginas = numero_de_tabla;
		log_info(logger, "Se asignó el numero de tabla: %d al proceso de id: %d\n", proceso->tablas_paginas, proceso->id);

		// TODO: AGREGAR MUTEX
		queue_push(new, proceso);
		log_info(logger, "Proceso %d asignado a la cola NEW", proceso->id);

		//TODO: aplicar semaforo para la cola de suspendido ready? Los suspendidos ready tienen mas prioridad
		sem_wait(&multiprogramacion);
		if(queue_size(susp_ready) <= 0) {
			pthread_mutex_unlock(&mutex_new_queue);
			t_pcb* procesoNuevo = queue_pop(new);
			pthread_mutex_unlock(&mutex_new_queue);

			pthread_mutex_lock(&mutex_ready_list);
			list_push(ready, procesoNuevo);
			pthread_mutex_unlock(&mutex_ready_list);
			sem_post(&elementos_en_cola_ready);
			log_info(logger, "El proceso %lu fue asignado a la cola READY", proceso->id);
			//creo que hay que chequear si la planificacion ya fue iniciada
			if((strcmp(planificador, "SRT") == 0) && (list_size(ready)>1)) {
					solicitar_interrupcion();
			}

		} else {
			//TODO: creo que no deberia pasar esto
			log_info(logger, "Se alcanzó el maximo grado de multiprogramacion, el proceso %lu permanece en la cola de NEW", proceso->id);
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
	return proceso->id;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}

t_pcb* crear_proceso() {
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->id = 0;
	pcb->tamanio_proceso = 0;
	pcb->instrucciones = list_create();
	pcb->program_counter = 0;
	pcb->estimacion_rafaga = 0;
	pcb->tablas_paginas = 0;
	pcb->socket = 0;
	return pcb;
}

void terminar_programa() {
	pthread_join(hilo_cpu_dispatch, NULL);
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
	destruir_lista(ready);
	destruir_lista(blocked);
	destruir_queue(new);
	destruir_queue(susp_ready);
}

void destruir_lista(t_list* lista) {
	list_destroy_and_destroy_elements(lista, (void *) destruir_nodo);
}

void destruir_queue(t_queue* q) {
	queue_destroy_and_destroy_elements(q, (void *) destruir_nodo);
}

bool numero_de_tabla_valido(int numero) {
	return numero != -1;
}

void planificar_fifo(){
		log_info(logger,"se inicio la planificacion FIFO");
		iniciar_planificacion_io();

		while(1){
			sem_wait(&sem_planificacion);
			log_info(logger, "Planificacion despausada");
			sem_wait(&elementos_en_cola_ready);
			pthread_mutex_lock(&mutex_ready_list);
			t_pcb* primer_proceso = list_pop(ready);
			pthread_mutex_unlock(&mutex_ready_list);
			sleep(5);
			list_iterate(primer_proceso->instrucciones, (void *) iterator);
			enviar_pcb(primer_proceso, conexion_con_cpu_dispatch);

			log_info(logger,"Se paso el proceso %lu de Ready a Ejecutando", primer_proceso->id);

		}
}

void escuchar_cpu_dispatch() {
	pthread_create(&hilo_cpu_dispatch, NULL, (void*) comunicacion_con_cpu, NULL);
}

void comunicacion_con_cpu() {
	while(1){
		operacion operacion = recibir_operacion(conexion_con_cpu_dispatch);
			switch(operacion) {
				case BLOQUEO_IO:
					log_info(logger, "Codigo BLOQUEO_IO recibido");
					t_pcb_bloqueado* proceso_bloqueado = recibir_pcb_bloqueado(conexion_con_cpu_dispatch);
					log_info(logger, "La cpu envio el proceso %lu con estado Bloqueado por IO", proceso_bloqueado->proceso->id);
					log_info(logger, "Tiempo de bloqueo: %d", proceso_bloqueado->tiempo_de_bloqueo);
					log_info(logger, "Inicio de bloqueo: %li", proceso_bloqueado->inicio_bloqueo);
					proceso_bloqueado->inicio_bloqueo = (int)time(NULL);
					proceso_bloqueado->suspendido = 0;
					agregar_a_bloqueados(proceso_bloqueado);
					sem_post(&elementos_en_cola_bloqueados);
					sem_post(&sem_planificacion);
					break;

				case INTERRUPCION:
					log_info(logger, "Un proceso fue interrumpido");
					t_pcb* pcb_interrumpido = recibir_pcb(conexion_con_cpu_dispatch);

					//actualizar estimacion

					pthread_mutex_lock(&mutex_ready_list);
					list_push(ready, pcb_interrumpido);
					pthread_mutex_unlock(&mutex_ready_list);
					sem_post(&sem_planificacion);

					break;
				case EXIT:
					log_info(logger, "La CPU envio un pcb con estado finalizado");
					t_pcb* proceso = recibir_pcb(conexion_con_cpu_dispatch);
				    enviar_finalizacion_a_memoria(proceso->id, conexion_con_memoria);
					sem_post(&multiprogramacion);
					log_info(logger, "El socket que recibi es %i", proceso->socket);
					enviar_respuesta_exitosa(proceso->socket);
					log_info(logger, "El proceso %lu finalizo correctamente", proceso->id);
					destruir_proceso(proceso);
					sem_post(&sem_planificacion);
					break;
				case ERROR:
					log_error(logger, "Se desconecto el cliente");
					return;
				default:
					log_info(logger, "Operacion desconocida");
					break;
			}
	}
}

void solicitar_interrupcion() {
	log_info(logger, "   Se solicito interrupcion");
	enviar_interrupcion(conexion_con_cpu_interrupt);
}


void agregar_a_bloqueados(t_pcb_bloqueado* proceso){
	pthread_mutex_lock(&mutex_blocked_list);
	list_add(blocked, proceso);
	pthread_mutex_unlock(&mutex_blocked_list);
	log_info(logger, "Se agrego el proceso %lu a la cola de bloqueados por I/O", proceso->proceso->id);
}

void planificacion_io(){
	while(1){

		sem_wait(&elementos_en_cola_bloqueados);
		pthread_mutex_lock(&mutex_blocked_list);
		t_pcb_bloqueado* primer_proceso = list_pop(blocked);
		pthread_mutex_unlock(&mutex_blocked_list);
		log_info(logger, "El proceso %lu inicio su I/0", primer_proceso->proceso->id);
		usleep(primer_proceso->tiempo_de_bloqueo*1000);
		log_info(logger, "El proceso %lu finalizo su I/0", primer_proceso->proceso->id);

		if(primer_proceso->suspendido == 0){

			pthread_mutex_lock(&mutex_ready_list);
			list_push(ready, primer_proceso->proceso);
			pthread_mutex_unlock(&mutex_ready_list);

			if(strcmp(planificador, "SRT") == 0) {
				solicitar_interrupcion();
			}

			sem_post(&elementos_en_cola_ready);
			log_info(logger, "Se paso el proceso %lu de bloqueado a ready", primer_proceso->proceso->id);

		} else {

			pthread_mutex_lock(&mutex_susp_ready_queue);
			queue_push(susp_ready, primer_proceso->proceso);
			pthread_mutex_unlock(&mutex_susp_ready_queue);
			sem_post(&elementos_en_cola_susp_ready);

			log_info(logger, "Se paso el proceso %lu de bloqueado a suspendido-ready", primer_proceso->proceso->id);
		}
	}
}

void iniciar_planificacion_io(){
	pthread_create(&planificador_io, NULL, (void*) planificacion_io, NULL);
	log_info(logger, "Inicio la planificacion IO");
}

void mandar_a_suspendido(){

	int tamanio_cola_bloqueados;

	while(1){
		sem_wait(&elementos_en_cola_bloqueados);

		pthread_mutex_lock(&mutex_blocked_list);
		tamanio_cola_bloqueados = list_size(blocked);
		pthread_mutex_unlock(&mutex_blocked_list);

		for(int i = 0; i < tamanio_cola_bloqueados; i++){

			t_pcb_bloqueado* proceso = list_get(blocked,i);
			if(proceso->suspendido == 0 && ((int)(time(NULL) - proceso->inicio_bloqueo) > tiempo_max_bloqueo)){
				//TODO: se manda a memoria y se espera confirmacion
				notificar_suspencion_proceso(proceso->proceso->id, conexion_con_memoria);
				proceso->suspendido = 1;
				sem_post(&multiprogramacion);
			}
		}
	}
}

void desuspendidor(){
	while(1){
		sem_wait(&elementos_en_cola_susp_ready);
		sem_wait(&multiprogramacion);
		pthread_mutex_lock(&mutex_susp_ready_queue);
		t_pcb* proceso = queue_pop(susp_ready);
		pthread_mutex_unlock(&mutex_susp_ready_queue);

		pthread_mutex_lock(&mutex_ready_list);
		list_push(ready, proceso);
		pthread_mutex_unlock(&mutex_ready_list);

		if(strcmp(planificador, "SRT") == 0) {
				solicitar_interrupcion();
		}
	}
}

void inicializar_colas() {
	new = queue_create();
	ready = list_create();
	blocked = list_create();
	susp_ready = queue_create();
}

void iniciar_planificacion(char* planificacion){
	if(strcmp(planificacion, "FIFO") == 0) {
		puts("FIFO");
		pthread_create(&planificador_fifo, NULL, (void*) planificar_fifo, NULL);
	} else if(strcmp(planificacion, "SRT") == 0) {
		puts("SRT");
		char* ip_cpu = config_get_string_value(config, "IP_CPU");
		char* puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
		conexion_con_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt);
		puts(" Se conecto al interrupt");
		pthread_create(&planificador_srt, NULL, (void*) planificar_srt, NULL);
	}
}


void inicializar_semaforos() {
	pthread_mutex_init(&mutex_ready_list, NULL);
	pthread_mutex_init(&mutex_blocked_list, NULL);
	pthread_mutex_init(&i_o, NULL);
	sem_init(&elementos_en_cola_bloqueados, 0, 0);
	sem_init(&elementos_en_cola_ready, 0, 0);
	sem_init(&multiprogramacion, 0, grado_multiprogramacion);
	sem_init(&elementos_en_cola_susp_ready, 0, 0);
	sem_init(&sem_planificacion, 0, 1);
}

void planificar_srt() {
	log_info(logger, "Se inicio la planificacion SRT");
	iniciar_planificacion_io();
	while(1) {
		sem_wait(&sem_planificacion);
		log_info(logger, "Planificacion despausada");
		sem_wait(&elementos_en_cola_ready);
		sleep(5);
		pthread_mutex_lock(&mutex_ready_list);
		if(list_size(ready) > 1) {
			list_sort(ready, (void *) menor_tiempo_restante);
		}
		t_pcb* proceso_mas_corto = list_pop(ready);
		pthread_mutex_unlock(&mutex_ready_list);


		// solo para ver si ordena bien
		log_info(logger, "Me llegaron los siguientes valores:");
		list_iterate(proceso_mas_corto->instrucciones, (void*) iterator);
		enviar_pcb(proceso_mas_corto, conexion_con_cpu_dispatch);
		//log_info(logger,"Se paso el proceso %lu de Ready a Ejecutando", proceso_mas_corto->id);

		// esto va en el EXIT, lo pongo aca para probar
		//enviar_finalizacion_a_memoria(proceso_mas_corto->id, conexion_con_memoria);
	}

}

void* list_pop(t_list* lista) {
	void* elemento = list_get(lista, 0);
	list_remove(lista, 0);
	return elemento;
}

void list_push(t_list* lista, void* elemento){
	list_add(lista, elemento);
}

t_pcb* menor_tiempo_restante(t_pcb* p1, t_pcb* p2) {
	if(p1->estimacion_rafaga > p2->estimacion_rafaga) {
		return p2;
	} else {
		return p1;
	}
}














