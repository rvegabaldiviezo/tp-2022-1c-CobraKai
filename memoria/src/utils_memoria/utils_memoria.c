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

void enviar_numero_de_tabla(int numero_de_tabla, int destino) {
	send(destino, &numero_de_tabla, sizeof(int), 0);
	//enviar_entero(numero_de_tabla, destino, ACCESO_TABLA_PRIMER_NIVEL);
}

void enviar_numero_de_marco(int marco, int destino) {
	//enviar_entero(marco, destino, ACCESO_TABLA_SEGUNDO_NIVEL);
}

void enviar_entero(int numero, int conexion, operacion op) {
	void* a_enviar = malloc(sizeof(int) + sizeof(operacion));
	memcpy(a_enviar, &op, sizeof(operacion));

	memcpy(a_enviar + sizeof(operacion), &numero, sizeof(int));

	send(conexion, a_enviar, sizeof(int) + sizeof(operacion), 0);
	free(a_enviar);
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

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
	                    servinfo->ai_socktype,
	                    servinfo->ai_protocol);

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
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(operacion), MSG_WAITALL) > 0) {
		return cod_op;
	} else {
		close(socket_cliente);
		return -1;
	}
}

int recibir_id_proceso(int conexion) {
	int id;
	if(recv(conexion, &id, sizeof(int), MSG_WAITALL) > 0) {
		return id;
	} else {
		close(conexion);
		return -1;
	}
}

int recibir_entero(int socket_cliente) {
	int tamanio;
	if(recv(socket_cliente, &tamanio, sizeof(int), MSG_WAITALL) > 0) {
		return tamanio;
	} else {
		close(socket_cliente);
		return -1;
	}
}

