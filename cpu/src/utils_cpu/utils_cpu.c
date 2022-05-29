#include "utils_cpu.h"

int iniciar_servidor(void) {
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	t_config* config_memoria = config_create(PATH_CONFIG);
	char* puerto_memoria = config_get_string_value(config_memoria, "PUERTO_MEMORIA");
	char* ip_memoria = config_get_string_value(config_memoria, "IP_MEMROIA");

	getaddrinfo(ip_memoria, puerto_memoria, &hints, &servinfo);

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
