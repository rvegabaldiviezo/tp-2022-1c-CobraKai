/***************** CLIENTE *******************/

#include "utils_memoria.h"

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

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}


void enviar_confirmacion(int destino) {
	int ok = 0;
	send(destino, &ok, sizeof(int), 0);
}

void enviar_entero(int destino, int a_enviar) {
	send(destino, &a_enviar, sizeof(int), 0);
}

void enviar_numero_de_tabla(int destino, int numero_de_tabla) {
	enviar_entero(destino, numero_de_tabla);
}

void enviar_numero_de_pagina(int destino, int numero_de_pagina) {
	enviar_entero(destino, numero_de_pagina);
}

void enviar_respuesta(int destino, int respuesta) {
	enviar_entero(destino, respuesta);
}

/******************** SERVER *****************/

int iniciar_servidor(void) {

	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	t_config* config_memoria = config_create(PATH_CONFIG);

	getaddrinfo(IP, config_get_string_value(config_memoria, "PUERTO_ESCUCHA"), &hints, &servinfo);

	config_destroy(config_memoria);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
	                    servinfo->ai_socktype,
	                    servinfo->ai_protocol);

	int activado = 1;
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	//log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor) {

	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);

	return socket_cliente;
}


void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	free(buffer);
}

operacion recibir_operacion(int socket_cliente) {
	return recibir_entero(socket_cliente);
}

int recibir_id_proceso(int conexion) {
	return recibir_entero(conexion);
}

int recibir_tamanio(int socket_cliente) {
	return recibir_entero(socket_cliente);
}

int recibir_numero_tabla(int conexion_cpu) {
	return recibir_entero(conexion_cpu);
}

int recibir_numero_entrada(int conexion_cpu) {
	return recibir_entero(conexion_cpu);
}

uint32_t recibir_uint32(int socket_cliente) {
	uint32_t entero;
	if(recv(socket_cliente, &entero, sizeof(uint32_t), MSG_WAITALL) > 0) {
		return entero;
	} else {
		close(socket_cliente);
		return -1;
	}
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


