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

// Semaforos
pthread_mutex_t mutex_ready_list;
pthread_mutex_t mutex_ready_queue;
pthread_mutex_t i_o;
pthread_mutex_t mutex_blocked_list;
pthread_mutex_t mutex_blocked_queue;
pthread_mutex_t mutex_susp_ready_queue;
sem_t elementos_en_cola_bloqueados;
sem_t elementos_en_cola_ready;
sem_t multiprogramacion;
sem_t elementos_en_cola_susp_ready;
sem_t cola_susp_ready_vacia;


// Colas / Listas
t_queue* new;
t_list* ready;
t_list* blocked;
t_queue* blocked_fifo;
t_queue* susp_blocked;
t_queue* susp_ready;
t_queue* ready_fifo;

// Generales
t_proceso* proceso;
t_log* logger;
t_config* config;
int conexion_consola;
int socket_servidor;
int conexion_con_memoria;
int conexion_con_cpu;
int tiempo_max_bloqueo;

float alfa;
int grado_multiprogramacion;
char* planificador;

int main(void) {

	config = config_create(PATH_CONFIG);

	pthread_mutex_init(&mutex_ready_list, NULL);
	pthread_mutex_init(&mutex_ready_queue, NULL);
	pthread_mutex_init(&mutex_blocked_list, NULL);
	pthread_mutex_init(&mutex_blocked_queue, NULL);
	pthread_mutex_init(&mutex_susp_ready_queue, NULL);
	pthread_mutex_init(&i_o, NULL);
	sem_init(&elementos_en_cola_bloqueados, 0, 0);
	sem_init(&elementos_en_cola_ready, 0, 0);
	sem_init(&elementos_en_cola_susp_ready, 0, 0);

	grado_multiprogramacion = config_get_int_value(config, GRADO_MULTIPROGRAMACION);
	sem_init(&multiprogramacion, 0, grado_multiprogramacion);

	logger = log_create(PATH_LOG, "KERNEL", true, LOG_LEVEL_DEBUG);

	alfa = config_get_int_value(config, ALFA);
	planificador = config_get_string_value(config, ALGORITMO_PLANIFICACION);
	tiempo_max_bloqueo = config_get_int_value(config, TIEMPO_MAXIMO_BLOQUEADO);

	socket_servidor = iniciar_servidor();

	log_info(logger, "Kernel listo para recibir al cliente");

	config = config_create(PATH_CONFIG);
	char* ip_memoria = config_get_string_value(config, IP_MEMORIA);
	char* planificacion = config_get_string_value(config, ALGORITMO_PLANIFICACION);
	char* puerto_memoria = config_get_string_value(config, PUERTO_MEMORIA);
	conexion_con_memoria = crear_conexion(ip_memoria, puerto_memoria);

	log_info(logger, "conectado a memoria");


	inicializar_colas();
	iniciar_planificacion(planificador);

	while(1) {
		conexion_consola = esperar_cliente(socket_servidor);
		if (!conexion_exitosa(conexion_consola)) {
			log_error(logger, "No se pudo establecer la conexion con el cliente");
			log_destroy(logger);
			return EXIT_FAILURE;
		}
		pthread_t hilo_consola;
		pthread_create(&hilo_consola, NULL, (void*) atender_consola, NULL);
		long int id_devuelto;
		pthread_join(hilo_consola, &id_devuelto);
		log_info(logger, "El proceso %lu entro en la cola de ready", id_devuelto);
	}

	terminar_programa();


	return EXIT_SUCCESS;
}
//no debería pasarse el socket por parametro?
pid_t atender_consola() {
	int operacion_consola = recibir_operacion(conexion_consola);
	//t_proceso* proceso;
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

		char* tamanio_recibido = string_new();
		string_append(&tamanio_recibido, "Tamanio recibido: ");
		string_append(&tamanio_recibido, string_itoa(proceso->pcb.tamanio_proceso));
		log_info(logger, tamanio_recibido);


		int numero_de_tabla = recibir_numero_de_tabla(conexion_con_memoria);
		if(!numero_de_tabla_valido(numero_de_tabla)) {
			log_error(logger, "El número de tabla no es valido");
			return EXIT_FAILURE;
		}

		proceso->pcb.tablas_paginas = numero_de_tabla;
		log_info(logger, "Se asignó el numero de tabla: %d al proceso de id: %lu\n", proceso->pcb.tablas_paginas, proceso->pcb.id);

		queue_push(new, proceso);
		log_info(logger, "Proceso %lu asignado a la cola NEW", proceso->pcb.id);

		//TODO: aplicar semaforo para la cola de suspendido ready? Los suspendidos ready tienen mas prioridad
		sem_wait(&multiprogramacion);
		if(queue_size(susp_ready) <= 0) {
			//todo: mutex
			t_proceso* proceso = queue_pop(new);


			if(strcmp(planificador, "SRT") == 0) {
				pthread_mutex_lock(&mutex_ready_list);
				list_add(ready, proceso);
				pthread_mutex_unlock(&mutex_ready_list);
				//TODO: replanificar
				log_info(logger, "El proceso %lu fue asignado a la cola READY", proceso->pcb.id);


			} else if(strcmp(planificador, "FIFO") == 0){
						pthread_mutex_lock(&mutex_ready_queue);
						queue_push(ready_fifo, proceso);
						pthread_mutex_unlock(&mutex_ready_queue);
						sem_post(&elementos_en_cola_ready);
						log_info(logger, "El proceso %lu fue asignado a la cola READY", proceso->pcb.id);
					}
		} else {
			//TODO: creo que no deberia pasar esto
			log_info(logger, "Se alcanzó el maximo grado de multiprogramacion, el proceso %lu permanece en la cola de NEW", proceso->pcb.id);
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
}

bool numero_de_tabla_valido(int numero) {
	return numero != -1;
}

void planificar_fifo(){
		log_info(logger,"se inicio la planificacion FIFO");
		iniciar_planificacion_io();

		while(1){
			if(strcmp(planificador, "SRT") == 0) {

						} else if(strcmp(planificador, "FIFO") == 0){
			sem_wait(&elementos_en_cola_ready);
			pthread_mutex_lock(&mutex_ready_queue);
			t_proceso* primer_proceso = queue_pop(ready_fifo);
			pthread_mutex_unlock(&mutex_ready_queue);


			//TODO: se manda el pcb a la cpu
			log_info(logger,"Se paso un proceso de Ready a Ejecutando");

			//semaforo que sincronice con cpu. Si el proceso terminó de ejecutar, continúa
			log_info(logger,"Un proceso termino de ejecutar");

		}
}


void comunicacion_con_cpu(){
	int operacion = recibir_operacion(conexion_con_cpu);
		switch(operacion) {
			case BLOQUEO_IO:
				t_proceso_bloqueado* proceso_bloqueado;
				log_info(logger, "La CPU envio un pcb con estado bloqueado por I/0");
				int tiempo_de_espera = recibir_tiempo_bloqueo(conexion_con_cpu);
				int inicio_bloqueo = (int)time(NULL);
				proceso_bloqueado->inicio_bloqueo = inicio_bloqueo;
				//TODO
				//t_pcb* pcb = recibir_pcb(conexion_con_cpu);
				//TODO: agregar pcb y tiempo a proceso_bloqueado
				agregar_a_bloqueados(proceso);
				sem_post(&elementos_en_cola_bloqueados);

				break;

			case INTERRUPT:

				break;
			case EXIT:
				log_info(logger, "La CPU envio un pcb con estado finalizado");
			    t_proceso* proceso = recibir_proceso(conexion_con_cpu);
			    //TODO: avisar a memoria
			    sem_post(&multiprogramacion);
				enviar_respuesta_exitosa(proceso->socket);
				log_error(logger, "El proceso %lu finalizo correctamente", proceso->pcb->id);
			break;
			case ERROR_CPU:
				log_error(logger, "Se desconecto el cliente");
				return;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}


void agregar_a_bloqueados(t_proceso_bloqueado* proceso){
	pthread_mutex_lock(&mutex_blocked_queue);
	queue_push(blocked_fifo, proceso);
	pthread_mutex_unlock(&mutex_blocked_queue);
	sem_post(&elementos_en_cola_bloqueados);
	log_info(logger, "Se agrego un proceso a la cola de bloqueados por I/0");

}

void planificacion_io(){
	while(1){

		sem_wait(&elementos_en_cola_bloqueados);
		pthread_mutex_lock(&mutex_blocked_queue);
		t_proceso_bloqueado* primer_proceso = queue_pop(blocked_fifo);
		pthread_mutex_unlock(&mutex_blocked_queue);

		usleep(primer_proceso->tiempo_de_bloqueo*1000);
		log_info(logger, "Un proceso finalizo su I/0");

		if(primer_proceso->suspendido == 0){

			pthread_mutex_lock(&mutex_ready_queue);
			queue_push(ready_fifo, primer_proceso->proceso);
			pthread_mutex_unlock(&mutex_ready_queue);

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
	pthread_join(planificador_io, NULL);
}

void mandar_a_suspendido(){

	int tamanio_cola_bloqueados;

	while(1){
		//TODO: aplicar semaforos
		tamanio_cola_bloqueados = queue_size(blocked_fifo);
		//TODO: volver a hacer
		for(int i = 0; i < tamanio_cola_bloqueados; i++){

			t_proceso_bloqueado* proceso = queue_pop(blocked_fifo);
			if(((int)(time(NULL) - proceso->inicio_bloqueo) > tiempo_max_bloqueo)){
				//TODO: suspender proceso, se manda a memoria y se espera confirmacion
				proceso->suspendido = 1;
				sem_post(&multiprogramacion);
			}

			queue_push(blocked_fifo, proceso);

		}
	}
}

void desuspendidor(){
	while(1){
		sem_wait(&elementos_en_cola_susp_ready);
		sem_wait(&multiprogramacion);
		pthread_mutex_lock(&mutex_susp_ready_queue);
		t_proceso* proceso = queue_pop(susp_ready);
		pthread_mutex_unlock(&mutex_susp_ready_queue);

		pthread_mutex_lock(&mutex_ready_queue);
		queue_push(ready_fifo, proceso);
		pthread_mutex_unlock(&mutex_ready_queue);


	}
}


//TODO
int recibir_tiempo_bloqueo(){
	return 1;
}

void inicializar_colas() {
	new = queue_create();
	ready = list_create();
	ready_fifo = queue_create();
	blocked = queue_create();
	susp_blocked = queue_create();
	susp_ready = queue_create();
}

void iniciar_planificacion(char* planificacion){
	if(strcmp(planificacion, "FIFO") == 0) {
		pthread_create(&planificador_fifo, NULL, (void*) planificar_fifo, NULL);
		pthread_join(planificador_fifo, NULL);
	} else if(strcmp(planificacion, "SRT") == 0) {
		pthread_create(&planificador_srt, NULL, (void*) planificar_srt, NULL);
		pthread_join(planificador_srt, NULL);
	}
}


void inicializar_semaforos() {
	sem_init(&sem_ready, 0, grado_multiprogramacion);
	sem_init(&sem_new, 0, 0);
}

void planificar_srt() {
	// TODO: implementar planificador posta
	// Agarro el mas corto de ready
	pthread_mutex_lock(&mutex_ready_queue);
	//sem_wait(&sem_ready);
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
