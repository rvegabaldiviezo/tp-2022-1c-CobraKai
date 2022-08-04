
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
pthread_t hilo_suspender;
pthread_t hilo_desuspender;
pthread_t planificador_largo_plazo;

// Semaforos
pthread_mutex_t mutex_new_queue;
pthread_mutex_t mutex_ready_list;
pthread_mutex_t i_o;
pthread_mutex_t mutex_blocked_list;
pthread_mutex_t mutex_susp_ready_queue;
sem_t elementos_en_cola_new;
sem_t elementos_en_cola_bloqueados;
sem_t elementos_en_cola_ready;
sem_t multiprogramacion;
sem_t elementos_en_cola_susp_ready;
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
bool ejecutando;
int contador_bloqueo;


double alfa;
int grado_multiprogramacion;
char* planificador;
int numero_proceso = 0;

int main(int argc, char** argv) {

	config = config_create(argv[1]);

	grado_multiprogramacion = config_get_int_value(config, GRADO_MULTIPROGRAMACION);

	logger = log_create(PATH_LOG, "KERNEL", true, LOG_LEVEL_DEBUG);

	alfa = config_get_double_value(config, ALFA);
	
	log_info(logger, "alfa: %f", alfa);
	planificador = config_get_string_value(config, ALGORITMO_PLANIFICACION);
	tiempo_max_bloqueo = config_get_int_value(config, TIEMPO_MAXIMO_BLOQUEADO);

	socket_servidor = iniciar_servidor();
	contador_bloqueo = 0;

	log_info(logger, "Kernel listo para recibir al cliente");

	//config = config_create(PATH_CONFIG);
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
	ejecutando = false;

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
		proceso->estimacion_rafaga = config_get_double_value(config, ESTIMACION_INICIAL);;
		proceso->estimacion_rafaga_restante = config_get_double_value(config, ESTIMACION_INICIAL);
		log_warning(logger, "LA ESTIMACION DEL CONFIG ES: %f", proceso->estimacion_rafaga);
		proceso->tiempo_ejecucion = 0;
		proceso->id = numero_proceso;
		proceso->tablas_paginas = -1;
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

		pthread_mutex_unlock(&mutex_new_queue);
		queue_push(new, proceso);
		pthread_mutex_unlock(&mutex_new_queue);

		log_info(logger, "Proceso %d asignado a la cola NEW", proceso->id);

		sem_post(&elementos_en_cola_new);

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
	pthread_join(hilo_desuspender, NULL);
	pthread_join(planificador_largo_plazo, NULL);
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
			sem_wait(&elementos_en_cola_ready);
			pthread_mutex_lock(&mutex_ready_list);
			t_pcb* primer_proceso = list_pop(ready);
			pthread_mutex_unlock(&mutex_ready_list);

			enviar_pcb(primer_proceso, conexion_con_cpu_dispatch);

			log_info(logger,"Se paso el proceso %d de Ready a Ejecutando", primer_proceso->id);

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
					ejecutando = false;
					log_info(logger, "Codigo BLOQUEO_IO recibido");
					t_pcb_bloqueado* proceso_bloqueado = recibir_pcb_bloqueado(conexion_con_cpu_dispatch);
					double tiempo_real_b = difftime((double) time(NULL), proceso_bloqueado->proceso->inicio_rafaga) * 1000;
					proceso_bloqueado->proceso->tiempo_ejecucion += tiempo_real_b;
					proceso_bloqueado->proceso->estimacion_rafaga = alfa *  proceso_bloqueado->proceso->tiempo_ejecucion + (1 - alfa) * proceso_bloqueado->proceso->estimacion_rafaga;
					proceso_bloqueado->proceso->estimacion_rafaga_restante = proceso_bloqueado->proceso->estimacion_rafaga;
					proceso_bloqueado->proceso->tiempo_ejecucion = 0;
					//log_warning(logger, "program counter: %d", proceso_bloqueado->proceso->program_counter);
					log_info(logger, "La cpu envio el proceso %d con estado Bloqueado por IO", proceso_bloqueado->proceso->id);
					log_info(logger, "Tiempo de bloqueo: %d", proceso_bloqueado->tiempo_de_bloqueo);
					proceso_bloqueado->inicio_bloqueo = time(NULL);
					proceso_bloqueado->suspendido = 0;
					proceso_bloqueado->id_block = contador_bloqueo;
					t_pcb_bloqueado_con_id* proceso_bloqueado_con_id = malloc(sizeof(t_pcb_bloqueado_con_id));
					proceso_bloqueado_con_id->id = contador_bloqueo;
					proceso_bloqueado_con_id->pcb_bloqueado = proceso_bloqueado;

					agregar_a_bloqueados(proceso_bloqueado_con_id);


					pthread_create(&hilo_suspender, NULL, (void*) esperar_y_suspender, proceso_bloqueado_con_id);
					pthread_detach(hilo_suspender);
					contador_bloqueo++;
					sem_post(&elementos_en_cola_bloqueados);
					sem_post(&sem_planificacion);

					break;

				case INTERRUPCION:
					ejecutando = false;
					log_info(logger, "Un proceso fue interrumpido");
					t_pcb* pcb_interrumpido = recibir_pcb(conexion_con_cpu_dispatch);

					// prox_rafaga = alfa * tiempo_ultima_rafaga + (1 - alfa) * pcb.estimacion_rafaga
					double tiempo_real = difftime(time(NULL), pcb_interrumpido->inicio_rafaga) * 1000;
					pcb_interrumpido->tiempo_ejecucion += tiempo_real;
					pcb_interrumpido->estimacion_rafaga_restante = pcb_interrumpido->estimacion_rafaga_restante - tiempo_real; //= alfa *  tiempo_real + (1 - alfa) * pcb_interrumpido->estimacion_rafaga;
					pasar_a_ready(pcb_interrumpido);
					sem_post(&sem_planificacion);

					break;
				case EXIT:
					ejecutando = false;
					log_info(logger, "La CPU envio un pcb con estado finalizado");
					t_pcb* proceso = recibir_pcb(conexion_con_cpu_dispatch);
				    enviar_finalizacion_a_memoria(proceso->id, conexion_con_memoria);
					sem_post(&multiprogramacion);
					log_info(logger, "El socket que recibi es %i", proceso->socket);
					enviar_respuesta_exitosa(proceso->socket);
					log_info(logger, "El proceso %d finalizo correctamente", proceso->id);
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


void agregar_a_bloqueados(t_pcb_bloqueado_con_id* proceso){
	pthread_mutex_lock(&mutex_blocked_list);
	list_add(blocked, proceso);
	pthread_mutex_unlock(&mutex_blocked_list);
	log_info(logger, "Se agrego el proceso %d a la cola de bloqueados por I/O", proceso->pcb_bloqueado->proceso->id);
}

void planificacion_io(){
	iniciar_hilo_desuspendidor();
	iniciar_planificador_largo_plazo();

	while(1){

		sem_wait(&elementos_en_cola_bloqueados);
		pthread_mutex_lock(&mutex_blocked_list);
		t_pcb_bloqueado_con_id* pcb_bllll = list_get(blocked, 0);;
		t_pcb_bloqueado* primer_proceso = pcb_bllll->pcb_bloqueado;
		pthread_mutex_unlock(&mutex_blocked_list);
		log_warning(logger, "El proceso %d inicio su I/O", primer_proceso->proceso->id);
		usleep(primer_proceso->tiempo_de_bloqueo*1000);
		list_pop(blocked);
		log_warning(logger, "El proceso %d finalizo su I/O", primer_proceso->proceso->id);

		if(primer_proceso->suspendido == 0){

			pasar_a_ready(primer_proceso->proceso);


			if(strcmp(planificador, "SRT") == 0 && ejecutando) {
				solicitar_interrupcion();
			}

			log_info(logger, "Se paso el proceso %d de bloqueado a ready", primer_proceso->proceso->id);

		} else {

			pthread_mutex_lock(&mutex_susp_ready_queue);
			queue_push(susp_ready, primer_proceso->proceso);
			pthread_mutex_unlock(&mutex_susp_ready_queue);
			sem_post(&elementos_en_cola_susp_ready);

			log_info(logger, "Se paso el proceso %d de bloqueado a suspendido-ready", primer_proceso->proceso->id);
		}
	}
}

void iniciar_planificador_largo_plazo(){
	pthread_create(&planificador_largo_plazo, NULL, (void*) pasar_de_new_a_ready, NULL);
}

void iniciar_planificacion_io(){
	pthread_create(&planificador_io, NULL, (void*) planificacion_io, NULL);
	log_info(logger, "Inicio la planificacion IO");
}

void iniciar_hilo_desuspendidor(){
	pthread_create(&hilo_desuspender, NULL, (void*) desuspendidor, NULL);
}

void esperar_y_suspender(t_pcb_bloqueado_con_id* proceso){
	 usleep(tiempo_max_bloqueo*1000);
	 if(esta_en_lista_bloqueados(proceso)){
		 notificar_suspencion_proceso(proceso->pcb_bloqueado->proceso->id, conexion_con_memoria);
		 int respuesta = recibir_entero(conexion_con_memoria);
		 if(respuesta == 0){
			 proceso->pcb_bloqueado->suspendido = 1;
			 log_warning(logger, "EL PROCESO %d SE SUSPENDIO POR MAX TIEMPO BLOQUEO", proceso->pcb_bloqueado->proceso->id);
			 sem_post(&multiprogramacion);
		 } else {
			 log_error(logger, "No se pudo suspender el proceso");
		 }
	 }

}

bool esta_en_lista_bloqueados(t_pcb_bloqueado_con_id* pcb){
	bool esta_en_lista = false;
	int i = 0;
	//pthread_mutex_lock(&mutex_blocked_list);
	int tamanio_cola_bloqueados = list_size(blocked);
	//pthread_mutex_unlock(&mutex_blocked_list);
	t_pcb_bloqueado_con_id* pcb_aux;

	while(!esta_en_lista && i < tamanio_cola_bloqueados){

		pthread_mutex_lock(&mutex_blocked_list);
		pcb_aux = list_get(blocked, i);
		pthread_mutex_unlock(&mutex_blocked_list);

		if(pcb_aux->pcb_bloqueado->proceso->id == pcb->pcb_bloqueado->proceso->id && pcb_aux->id == pcb->id){
			esta_en_lista = true;
			break;
		}

		i++;
	}

	return esta_en_lista;
}

void desuspendidor(){
	while(1){
		sem_wait(&elementos_en_cola_susp_ready);
		sem_wait(&multiprogramacion);

		pthread_mutex_lock(&mutex_susp_ready_queue);
		t_pcb* proceso = queue_pop(susp_ready);
		pthread_mutex_unlock(&mutex_susp_ready_queue);

		pasar_a_ready(proceso);

		if(strcmp(planificador, "SRT") == 0 && ejecutando) {
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
	pthread_mutex_init(&mutex_new_queue, NULL);
	sem_init(&elementos_en_cola_bloqueados, 0, 0);
	sem_init(&elementos_en_cola_ready, 0, 0);
	sem_init(&multiprogramacion, 0, grado_multiprogramacion);
	sem_init(&elementos_en_cola_susp_ready, 0, 0);
	sem_init(&sem_planificacion, 0, 1);
	sem_init(&elementos_en_cola_new, 0, 0);
}

void planificar_srt() {
	log_info(logger, "Se inicio la planificacion SRT");
	iniciar_planificacion_io();
	while(1) {
		sem_wait(&sem_planificacion);
		sem_wait(&elementos_en_cola_ready);
		pthread_mutex_lock(&mutex_ready_list);
		if(list_size(ready) > 1) {
			list_sort(ready, (void *) menor_tiempo_restante);
			log_info(logger, "lista ordenada: ");
			void iterador(t_pcb* pcb) {

				log_info(logger, "id: %d estimacion: %f",pcb->id, pcb->estimacion_rafaga_restante);
			}
			list_iterate(ready, (void*) iterador);
		}
		t_pcb* proceso_mas_corto = list_pop(ready);
		pthread_mutex_unlock(&mutex_ready_list);
		proceso_mas_corto->inicio_rafaga = (double) time(NULL);
		enviar_pcb(proceso_mas_corto, conexion_con_cpu_dispatch);
		ejecutando = true;
		log_info(logger,"Se paso el proceso %i de Ready a Ejecutando", proceso_mas_corto->id);

		// esto va en el EXIT, lo pongo aca para probar
		//enviar_finalizacion_a_memoria(proceso_mas_corto->id, conexion_con_memoria);
	}

}

void pasar_a_ready(t_pcb* proceso) {
	pthread_mutex_lock(&mutex_ready_list);
	list_push(ready, proceso);
	pthread_mutex_unlock(&mutex_ready_list);
	if(proceso->tablas_paginas == -1) {
		int numero_de_tabla = recibir_numero_de_tabla(proceso, conexion_con_memoria);
		if(!numero_de_tabla_valido(numero_de_tabla)) {
			log_error(logger, "El número de tabla no es valido");
			//return EXIT_FAILURE;
		}

		proceso->tablas_paginas = numero_de_tabla;
		log_info(logger, "Se asignó el numero de tabla: %d al proceso de id: %d\n", numero_de_tabla, proceso->id);
	}

	if((strcmp(planificador, "SRT") == 0) && ejecutando) {
			solicitar_interrupcion();
	}

	sem_post(&elementos_en_cola_ready);
	log_info(logger, "El proceso %d fue asignado a la cola READY", proceso->id);
}

void* list_pop(t_list* lista) {
	void* elemento = list_get(lista, 0);
	list_remove(lista, 0);
	return elemento;
}

void list_push(t_list* lista, void* elemento){
	list_add(lista, elemento);
}

bool menor_tiempo_restante(t_pcb* p1, t_pcb* p2) {
	return p1->estimacion_rafaga_restante < p2->estimacion_rafaga_restante;
}

void pasar_de_new_a_ready(){
	int tamanio_cola_susp_ready;
	t_pcb* proceso;

	while(1){

		sem_wait(&elementos_en_cola_new);
		sem_wait(&multiprogramacion);
		pthread_mutex_lock(&mutex_susp_ready_queue);
		tamanio_cola_susp_ready = queue_size(susp_ready);
		pthread_mutex_unlock(&mutex_susp_ready_queue);

		if(tamanio_cola_susp_ready == 0){
			pthread_mutex_lock(&mutex_new_queue);
			proceso = queue_pop(new);
			pthread_mutex_unlock(&mutex_new_queue);

			pasar_a_ready(proceso);

		} else {
			sem_post(&multiprogramacion);
			sem_post(&elementos_en_cola_new);
		}
	}
}

