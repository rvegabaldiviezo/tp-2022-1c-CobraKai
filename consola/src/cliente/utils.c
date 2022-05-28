#include "utils.h"

void* serializar_paquete(t_proceso* proceso, int bytes) {
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(proceso->operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(proceso->tamanio), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(proceso->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, proceso->buffer->stream, proceso->buffer->size);
	desplazamiento+= proceso->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto) {
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

void crear_buffer(t_proceso* proceso) {
	proceso->buffer = malloc(sizeof(t_buffer));
	proceso->buffer->size = 0;
	proceso->buffer->stream = NULL;
}

void agregar_instruccion(t_proceso* proceso, void* valor, int tamanio) {
	proceso->buffer->stream = realloc(proceso->buffer->stream, proceso->buffer->size + tamanio + sizeof(int));
	memcpy(proceso->buffer->stream + proceso->buffer->size, &tamanio, sizeof(int));
	memcpy(proceso->buffer->stream + proceso->buffer->size + sizeof(int), valor, tamanio);
	proceso->buffer->size += tamanio + sizeof(int);
}

void enviar_a_kernel(t_proceso* proceso, int socket_cliente) {
	int bytes = proceso->buffer->size + 3*sizeof(int);
	void* a_enviar = serializar_paquete(proceso, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_proceso(t_proceso* proceso) {
	free(proceso->buffer->stream);
	free(proceso->buffer);
	free(proceso);
}

void liberar_conexion(int socket_cliente) {
	close(socket_cliente);
}

int recibir_respuesta(int conexion) {
	int respuesta;
	int bytes_recibidos = recv(conexion, &respuesta, sizeof(int), MSG_WAITALL);
	if (bytes_recibidos <= 0) {
		return -1;
	} else {
		return respuesta;
	}
}


