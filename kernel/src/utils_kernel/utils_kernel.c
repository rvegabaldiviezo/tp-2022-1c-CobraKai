//+++ FUNCIONALIDAD DE SERVIDOR +++

#include "utils_kernel.h"

int iniciar_servidor(void) {
	int socket_servidor; //Guarda el File Descriptor(IDs) representado por un entero.

	struct addrinfo hints, *servinfo; // Estruc q Contendra información sobre la dirección de un proveedor de servicios.

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(IP, PUERTO, &hints, &servinfo); //Traduce el nombre de una ubicación de servicio a un conjunto de direcciones de socket.

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	/* bind() y listen() son las llamadas al sistema que realiza
	 * la preparacion por parte del proceso servidor */

	int activado = 1;
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	// 1) Asociamos el socket creado a un puerto
	bind(socket_servidor,servinfo->ai_addr, servinfo->ai_addrlen);

	// 2) Escuchamos las conexiones entrantes a ese socket, cuya unica responsabilidad
	// es notificar cuando un nuevo cliente este intentando conectarse.
	listen(socket_servidor,SOMAXCONN); // El servidor esta listo para recibir a los clientes (ESTA ESCUCHANDO).
	freeaddrinfo(servinfo);
	return socket_servidor;
}

/* Retona un nuevo socket (FILE DESCRIPTOR) que representa la
 * CONEXION BIDIRECCIONAL entre el SERVIDOR y CLIENTE*/
int esperar_cliente(int socket_servidor) {
	/* Aceptamos un nuevo cliente
	 * accept(): Es una llamada al sistema que es BLOQUEANTE, entonces,
	 * el proceso servidor se quedara BLOQUEADO en accept hasta que llegue
	 * un cliente.
	 * Si el servidor no esta en accept, cuando el cliente intente
	 * conectarse (mediante connect()) fallara y devolvera un error*/
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	return socket_cliente;
}

void* recibir_buffer(int* size, int socket_cliente) {
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

int recibir_tamanio_proceso(int socket_cliente) {
	int tamanio;
	if(recv(socket_cliente, &tamanio, sizeof(int), MSG_WAITALL) > 0) {
		return tamanio;
	} else {
		close(socket_cliente);
		return -1;
	}
}

int recibir_operacion(int socket_cliente) {
	int operacion;
	if(recv(socket_cliente, &operacion, sizeof(int), MSG_WAITALL) > 0) {
		return operacion;
	} else {
		close(socket_cliente);
		return -1;
	}
}

t_list* recibir_instrucciones(int socket_cliente) {
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* instrucciones = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size) {
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(instrucciones, valor);
	}
	free(buffer);
	return instrucciones;
}

t_pcb* recibir_proceso(int socket_cliente) {
	t_pcb* proceso = malloc(sizeof(t_pcb));
	proceso->tamanio_proceso = recibir_tamanio_proceso(socket_cliente);
	proceso->instrucciones = recibir_instrucciones(socket_cliente);
	return proceso;
}

void destruir_nodo(t_link_element* nodo) {
	free(nodo);
}

void destruir_proceso(t_pcb* proceso) {
	list_destroy_and_destroy_elements(proceso->instrucciones, (void*) destruir_nodo);
	free(proceso);
}

t_list* parsear_instrucciones(t_list* instrucciones) {
	t_list* aux = list_create();
	for(int i = 0; i < list_size(instrucciones); i++) {
		char* instruccion = list_get(instrucciones, i);
		if(strstr(instruccion, "NO_OP") != NULL) {
			int parametro = atoi(string_substring(instruccion, 6, string_length(instruccion)));
			if(parametro != 0) {
				for(int j = 0; j < parametro; j++) {
					list_add(aux, "NO_OP");
				}
			} else {
				list_add(aux, "NO_OP");
			}

		} else {
			list_add(aux, instruccion);
		}

	}
	return aux;
}

void enviar_pcb(t_pcb* pcb, int conexion) {
	t_buffer* buffer = cargar_buffer(pcb->instrucciones);
	int bytes = sizeof(pid_t) + 7 * sizeof(int) + buffer->size;

	void* pcb_serializado = serializar_pcb(pcb, buffer, bytes);
	send(conexion, pcb_serializado, bytes, 0);
	free(pcb_serializado);
}

void* serializar_pcb(t_pcb* pcb, t_buffer* buffer, int bytes) {
	void* a_enviar = malloc(bytes);
	int desplazamiento = 0;

	int operacion = PCB;
	memcpy(a_enviar + desplazamiento, &operacion, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->id), sizeof(pid_t));
	desplazamiento += sizeof(pid_t);

	memcpy(a_enviar + desplazamiento, &(pcb->socket), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->tamanio_proceso), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->program_counter), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->estimacion_rafaga), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->tablas_paginas), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(buffer->size), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, buffer->stream, buffer->size);

	return a_enviar;
}

t_buffer* cargar_buffer(t_list* lista) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = 0;
	buffer->stream = NULL;
	for(int i = 0; i < list_size(lista); i++) {
		char* instruccion = list_get(lista, i);
		agregar_instruccion(buffer, instruccion, string_length(instruccion) + 1);
	}
	return buffer;
}

void agregar_instruccion(t_buffer* buffer, char* valor, int tamanio) {
	buffer->stream = realloc(buffer->stream, buffer->size + tamanio + sizeof(int));
	memcpy(buffer->stream + buffer->size, &tamanio, sizeof(int));
	memcpy(buffer->stream + buffer->size + sizeof(int), valor, tamanio);
	buffer->size += tamanio + sizeof(int);
}

int recibir_entero(int socket_cliente) {
	int entero;
	if(recv(socket_cliente, &entero, sizeof(int), MSG_WAITALL) > 0) {
		return entero;
	} else {
		close(socket_cliente);
		return -1;
	}
}

int recibir_tiempo_bloqueo(int conexion){
	return recibir_entero(conexion);
}

t_pcb* recibir_pcb(int conexion) {
	// orden en el que vienen: operacion, id, socket, tamanio, program_counter, estimacion_rafaga, numero_tabla, tamanio_instrucciones, instrucciones
	t_pcb* pcb = malloc(sizeof(t_pcb));
	//int operacion = recibir_entero(conexion);
	pcb->id = recibir_entero(conexion);
	pcb->socket = recibir_entero(conexion);
	pcb->tamanio_proceso = recibir_entero(conexion);
	pcb->program_counter = recibir_entero(conexion);
	pcb->estimacion_rafaga = recibir_entero(conexion);
	pcb->tablas_paginas = recibir_entero(conexion);
	pcb->instrucciones = recibir_instrucciones(conexion);
	pcb->inicio_rafaga = recibir_entero(conexion);
	return pcb;
}

t_pcb_bloqueado* recibir_pcb_bloqueado(int conexion) {
	t_pcb_bloqueado* bloqueado = malloc(sizeof(t_pcb_bloqueado));
	bloqueado->proceso = recibir_pcb(conexion);
	bloqueado->tiempo_de_bloqueo = recibir_tiempo_bloqueo(conexion);
	bloqueado->inicio_bloqueo = time(NULL);
	return bloqueado;
}



/***********************************************************************************/
/************************************ CLIENTE **************************************/
/***********************************************************************************/

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
	                    		server_info->ai_socktype,
								server_info->ai_protocol);


	// Ahora que tenemos el socket, vamos a conectarlo
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);


	freeaddrinfo(server_info);

	return socket_cliente;
}


void liberar_conexion(int socket_cliente) {
	close(socket_cliente);
}

void solicitar_numero_de_tabla(t_pcb* pcb, int conexion) {
	operacion op = INICIO_PROCESO;
	//paquete p = cargar_id(operacion, pcb->id);

	unsigned int bytes = sizeof(operacion) + sizeof(int) + sizeof(int);
	void* a_enviar = malloc(bytes);
	int desp = 0;
	memcpy(a_enviar + desp, &(op), sizeof(operacion));
	desp += sizeof(operacion);

	memcpy(a_enviar + desp, &(pcb->id), sizeof(int));
	desp += sizeof(int);

	memcpy(a_enviar + desp, &(pcb->tamanio_proceso), sizeof(int));
	//desp += sizeof(int);

	send(conexion, a_enviar, bytes, 0);
}

void enviar_respuesta_exitosa(int conexion) {
	operacion operacion = RESPUESTA_EXITO;
	send(conexion, &operacion, sizeof(operacion), 0);
}

int recibir_numero_de_tabla(t_pcb* pcb, int conexion_memoria) {
	solicitar_numero_de_tabla(pcb, conexion_memoria);
	int numero_de_tabla;
	int bytes_recibidos = recv(conexion_memoria, &numero_de_tabla, sizeof(int), MSG_WAITALL);
	if (bytes_recibidos <= 0) {
		return -1;
	} else {
		return numero_de_tabla;
	}
}

void enviar_interrupcion(int conexion_con_cpu_interrupt) {
	operacion operacion = INTERRUPCION;
	send(conexion_con_cpu_interrupt, &operacion, sizeof(operacion), 0);
}

void notificar_suspencion_proceso(pid_t id, int conexion) {
	operacion op = SUSPENCION_PROCESO;
	paquete paquete = cargar_id(op, id);

	void* a_enviar = malloc(sizeof(paquete));
	int desp = 0;
	memcpy(a_enviar + desp, &(paquete.cod_op), sizeof(operacion));
	desp += sizeof(operacion);

	memcpy(a_enviar + desp, &(paquete.elemento), sizeof(paquete.elemento));
	desp += sizeof(paquete.elemento);


	send(conexion, &paquete, sizeof(paquete), 0);
}

paquete cargar_id(operacion op, pid_t id) {
	paquete p;
	p.cod_op = op;
	p.elemento = id;
	return p;
}

void enviar_finalizacion_a_memoria(pid_t id, int conexion_con_memoria) {
	operacion op = FINALIZACION_PROCESO;
//	paquete paquete = cargar_id(op, id);
//
//	void* a_enviar = malloc(sizeof(paquete));
//	int desp = 0;
//	memcpy(a_enviar + desp, &(paquete.cod_op), sizeof(operacion));
//	desp += sizeof(operacion);
//
//	memcpy(a_enviar + desp, &(paquete.elemento), sizeof(paquete.elemento));
//	desp += sizeof(paquete.elemento);

	send(conexion_con_memoria, &op, sizeof(operacion), 0);
}

