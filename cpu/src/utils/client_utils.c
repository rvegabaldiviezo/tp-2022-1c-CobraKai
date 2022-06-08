#include "client_utils.h"


//+++ CLIENTE +++

// Inicia el logger en la RUTA_LOG
t_log* iniciar_logger(void)
{
	t_log* nuevo_logger = log_create(PATH_LOG,NAME_LOG,false,LOG_LEVEL_INFO);
	return nuevo_logger;
}

// Lee del archivo config
t_config* iniciar_config(void)
{
	t_config* nuevo_config = config_create(PATH_CONFIG);
	return nuevo_config;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
/*
t_config* iniciar_config(void)
{
	t_config* nuevo_config;
    nuevo_config = config_create("./cliente.config");
	return nuevo_config;
}

void leer_consola(t_log* logger)
{
	char* leido;

	// Lee la linea que escribamos por consola. Retorna el string leido.
	leido = readline("> ");

	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
	while( strcmp(leido,"") != 0){
		log_info(logger, leido);
		leido = readline("> ");
	}
	// ¡No te olvides de liberar las lineas antes de regresar!
	free(leido);

}

void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete;

	// Leemos y esta vez agregamos las lineas al paquete
		// 1 - Creo un Paquete
		paquete = crear_paquete();
		// 2- Leer de Consola
		leido = readline("> ");
		while( strcmp(leido,"") != 0){
		// 3 - Agregar al paqute
			int size = string_length(leido) + 1;
			agregar_a_paquete(paquete, leido, size);
		// 4 - Leer la sig linea.
			leido = readline("> ");
		}
	// Enviar el paquete dada una conexion
	enviar_paquete(paquete, conexion);

	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	free(leido);
	eliminar_paquete(paquete);
}
*/

/*
void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	// Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) con las funciones de las commons y del TP mencionadas en el enunciado

	log_destroy(logger);

	config_destroy(config);

	liberar_conexion(conexion);
}


//#############
void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	//LLAMADA AL SISTEMA que devuelve informacion de RED sobre la IP
	// * y el PUERTO que le pasemos, en este caso del Servidor. Inyecta en
	// * variable server_info los datos necesarios para la creacion del Socket.
	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket Cliente.
	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo.
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	// nuestros procesos ya estan conectados

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = "...";
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_super_paquete(void)
{
	//me falta un malloc!
	t_paquete* paquete;

	//descomentar despues de arreglar
	//paquete->codigo_operacion = PAQUETE;
	//crear_buffer(paquete);
	return paquete;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = "PAQUETE";
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
*/
