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
pthread_mutex_t mutex_susp_ready_queue;
sem_t elementos_en_cola_bloqueados;
sem_t elementos_en_cola_ready;
sem_t multiprogramacion;
sem_t elementos_en_cola_susp_ready;
sem_t cola_susp_ready_vacia;
//semaforo que controla la planificacion srt. Podría ser un mutex creo
sem_t sem_planificacion_srt;


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

	}

	terminar_programa();

	return EXIT_SUCCESS;
}
//no debería pasarse el socket por parametro?
pid_t atender_consola() {
	int operacion_consola = recibir_operacion(conexion_consola);
	t_pcb* proceso;
	switch(operacion_consola) {
	case LISTA_DE_INSTRUCCIONES:
		proceso = crear_proceso();
		proceso = recibir_proceso(conexion_consola);
		// Ya se copiaron todos los bytes que venian de la consola
		// de ahora en mas puedo agregar lo que quiera al proceso
		proceso->instrucciones = parsear_instrucciones(proceso->instrucciones);
		proceso->socket = conexion_consola;
		proceso->estimacion_rafaga = config_get_int_value(config, ESTIMACION_INICIAL);
		proceso->id = pthread_self();
		// Calculo de la rafaga
		// prox_rafaga = alfa * tiempo_ultima_rafaga + (1 - alfa) * pcb.estimacion_rafaga
		if(proceso->tamanio_proceso == -1) {
			log_error(logger, "Ocurrió un error al recibir el tamanio del proceso");
			log_destroy(logger);
			return EXIT_FAILURE;
		}

		// Ya se copiaron todos los bytes que venian de la consola
		// de ahora en mas puedo agregar lo que quiera al proceso

		char* tamanio_recibido = string_new();
		string_append(&tamanio_recibido, "Tamanio recibido: ");
		string_append(&tamanio_recibido, string_itoa(proceso->tamanio_proceso));

		log_info(logger, "Tamaño recibido: %d", proceso->tamanio_proceso);

		int numero_de_tabla = recibir_numero_de_tabla(conexion_con_memoria);
		if(!numero_de_tabla_valido(numero_de_tabla)) {
			log_error(logger, "El número de tabla no es valido");
			return EXIT_FAILURE;
		}

		proceso->tablas_paginas = numero_de_tabla;
		log_info(logger, "Se asignó el numero de tabla: %d al proceso de id: %lu\n", proceso->tablas_paginas, proceso->id);

		queue_push(new, proceso);
		log_info(logger, "Proceso %lu asignado a la cola NEW", proceso->id);

		//TODO: aplicar semaforo para la cola de suspendido ready? Los suspendidos ready tienen mas prioridad
		sem_wait(&multiprogramacion);
		if(queue_size(susp_ready) <= 0) {
			//todo: mutex
			pthread_mutex_unlock(&mutex_new_queue);
			log_info(logger, "Proceso %lu asignado a la cola NEW", proceso->id);
			t_pcb* proceso = queue_pop(new);
			pthread_mutex_unlock(&mutex_new_queue);

			if(strcmp(planificador, "SRT") == 0) {
				pthread_mutex_lock(&mutex_ready_list);
				list_add(ready, proceso);
				pthread_mutex_unlock(&mutex_ready_list);
				sem_post(&elementos_en_cola_ready);
				log_info(logger, "El proceso %lu fue asignado a la cola READY", proceso->id);
				solicitar_interrupcion();
			} else if(strcmp(planificador, "FIFO") == 0){
						pthread_mutex_lock(&mutex_ready_list);
						list_push(ready, proceso);
						pthread_mutex_unlock(&mutex_ready_list);
						sem_post(&elementos_en_cola_ready);
						log_info(logger, "El proceso %lu fue asignado a la cola READY", proceso->id);
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
			t_pcb* primer_proceso = list_pop(ready);
			pthread_mutex_unlock(&mutex_ready_list);
			sleep(5);
			list_iterate(primer_proceso->instrucciones, (void *) iterator);
			//TODO: se manda el pcb a la cpu
			enviar_pcb(primer_proceso, conexion_con_cpu_dispatch);
			//log_info(logger,"Se paso un proceso de Ready a Ejecutando");

			//TODO: semaforo que sincronice con cpu. Si el proceso terminó de ejecutar, continúa
			log_info(logger,"Un proceso termino de ejecutar");

		}
}

void comunicacion_con_cpu() {
	int operacion = recibir_operacion(conexion_con_cpu_dispatch);
		switch(operacion) {
			case BLOQUEO_IO:
				log_info(logger, "La CPU envio un pcb con estado bloqueado por I/0");
				t_pcb_bloqueado* proceso_bloqueado;
				int tiempo_de_espera = recibir_tiempo_bloqueo(conexion_con_cpu_dispatch);
				int inicio_bloqueo = (int)time(NULL);
				proceso_bloqueado->inicio_bloqueo = inicio_bloqueo;
				//TODO
				//t_pcb* pcb = recibir_pcb(conexion_con_cpu);
				//TODO: agregar pcb y tiempo a proceso_bloqueado
				agregar_a_bloqueados(proceso);
				sem_post(&elementos_en_cola_bloqueados);

				break;

			case INTERRUPCION:
				log_info(logger, "Un proceso fue interrumpido");
				//t_pcb* pcb = recibir_pcb(conexion_con_cpu);
				//agregar al proceso el pcb
				//actualizar estimacion
				//agregar el proceso a la lista de ready
				sem_post(&sem_planificacion_srt);

				break;
			case EXIT:
				log_info(logger, "La CPU envio un pcb con estado finalizado");
			    t_pcb* proceso = recibir_proceso(conexion_con_cpu_dispatch);
			    //TODO: avisar a memoria
			    sem_post(&multiprogramacion);
				enviar_respuesta_exitosa(proceso->socket);
				log_info(logger, "El proceso %lu finalizo correctamente", proceso->id);
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


void agregar_a_bloqueados(t_pcb_bloqueado* proceso){
	pthread_mutex_lock(&mutex_blocked_list);
	list_add(blocked, proceso);
	pthread_mutex_unlock(&mutex_blocked_list);
	sem_post(&elementos_en_cola_bloqueados);
	log_info(logger, "Se agrego un proceso a la cola de bloqueados por I/O");
}

void planificacion_io(){
	while(1){

		sem_wait(&elementos_en_cola_bloqueados);
		pthread_mutex_lock(&mutex_blocked_list);
		t_pcb_bloqueado* primer_proceso = list_pop(blocked);
		pthread_mutex_unlock(&mutex_blocked_list);

		usleep(primer_proceso->tiempo_de_bloqueo*1000);
		log_info(logger, "Un proceso finalizo su I/0");

		if(primer_proceso->suspendido == 0){

			pthread_mutex_lock(&mutex_ready_list);
			list_push(ready, primer_proceso->proceso);
			pthread_mutex_unlock(&mutex_ready_list);

			if(strcmp(planificador, "SRT") == 0) {
				solicitar_interrupcion();
			}

			sem_post(&elementos_en_cola_ready);
			log_info(logger, "Se paso un proceso de bloqueado a ready");

		} else {

			pthread_mutex_lock(&mutex_susp_ready_queue);
			queue_push(susp_ready, primer_proceso->proceso);
			pthread_mutex_unlock(&mutex_susp_ready_queue);
			sem_post(&elementos_en_cola_susp_ready);

			log_info(logger, "Se paso un proceso de bloqueado a suspendido-ready");
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

//TODO
int recibir_tiempo_bloqueo(){
	return 1;
}

void inicializar_colas() {
	new = queue_create();
	ready = list_create();
	blocked = list_create();
	susp_ready = queue_create();
}

void iniciar_planificacion(char* planificacion){
	if(strcmp(planificacion, "FIFO") == 0) {
		pthread_create(&planificador_fifo, NULL, (void*) planificar_fifo, NULL);
	} else if(strcmp(planificacion, "SRT") == 0) {
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
	sem_init(&sem_planificacion_srt, 0, 1);
}

void planificar_srt() {
	log_info(logger, "Se inicio la planificacion SRT");
	iniciar_planificacion_io();
	while(1) {
		sem_wait(&sem_planificacion_srt);
		sem_wait(&elementos_en_cola_ready);
		sleep(5);
		pthread_mutex_lock(&mutex_ready_list);
		if(list_size(ready) > 1) {
			list_sort(ready, (void *) menor_tiempo_restante);
		}
		t_pcb* proceso_mas_corto = list_pop(ready);
		pthread_mutex_unlock(&mutex_ready_list);

		/*sem_post(&sem_grado_multiprogramacion);
		log_info(logger, "El grado de multiprogramacion despues del POST es: %d", sem_grado_multiprogramacion);
		*/
		// solo para ver si ordena bien
		log_info(logger, "Me llegaron los siguientes valores:");
		list_iterate(proceso_mas_corto->instrucciones, (void*) iterator);
		enviar_pcb(proceso_mas_corto, conexion_con_cpu_dispatch);
		//TODO: mandar pcb a cpu
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



