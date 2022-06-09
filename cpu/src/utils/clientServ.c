#include "clientServ.h"
#include "../cpu.h"

/***********************************************************************************/
/************************************ SERVER **************************************/
/***********************************************************************************/

//##### FUNCIONES SERVER PARA CPU #######

int esperar_cliente_dispatch(t_proceso_cpu cpu_process){//t_proceso_cpu
	return esperar_cliente_cpu(cpu_process, cpu_process.socket_servidor_dispatch, "dispatch");
}

int esperar_cliente_interrupt(t_proceso_cpu cpu_process){
	return esperar_cliente_cpu(cpu_process, cpu_process.socket_servidor_interrupt, "interrupt");
}

int esperar_cliente_cpu(t_proceso_cpu cpu_process, int socket_server, char* tipo_puerto){

	int socket_cliente = esperar_cliente(socket_server);

	if (socket_cliente<0) {
			log_error(cpu.logger, " Error de Conexion del cliente, puerto tipo: %s",tipo_puerto);
			log_destroy(cpu_process.logger);
			exit(1);
	}
	log_info(cpu_process.logger, " Conexion valida cliente, puerto tipo: %s. Nro de socket: %d",tipo_puerto, socket_cliente);

	return socket_cliente;
}

void iniciar_servidor_dispatch(t_proceso_cpu cpu_process)
{
	cpu_process.socket_servidor_dispatch = iniciar_servidor_cpu(cpu_process, KEY_PUERTO_DISPATCH);;
}

void iniciar_servidor_interrupt(t_proceso_cpu cpu_process)
{
	cpu_process.socket_servidor_interrupt = iniciar_servidor_cpu(cpu_process, KEY_PUERTO_DISPATCH);
}

//
int iniciar_servidor_cpu(t_proceso_cpu cpu_process, char* key_puerto)
{
	//Lee las configuraciones del archivo .config y obtiene las values
	char* ip = config_get_string_value(cpu_process.config, KEY_IP_CPU);
	char* puerto = config_get_string_value(cpu_process.config, key_puerto);
	log_info(cpu_process.logger, "Se creara un socket_servidor en el puerto: %s", puerto);

	int socket_servidor = iniciar_servidor(ip,puerto);

	if(0 <= socket_servidor){
		log_info(cpu_process.logger, " ERROR: NO SE CREO EL SOCKET SERVIDOR");
		exit(1);
	}
	log_info(cpu_process.logger, "Se creo el socket_servidor:  %i, listo para escuchar al cliente", socket_servidor);

	return socket_servidor;
}

//##### FUNCIONES SERVER  #######

// Crear el socket del servidor, para poder recibir peticiones del cliente que se conecte al servidor.
int iniciar_servidor(char* ip, char* puerto)
{	//Guarda el File Descriptor(IDs)
	int socket_servidor;

	//Estruc q Contendra información sobre la dirección de un proveedor de servicios.
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	//Traduce el nombre de una ubicación de servicio a un conjunto de direcciones de socket.
	getaddrinfo(ip, puerto, &hints, &servinfo);
	// El cliente para poder conectarse con el servidor, deben hacerlo
	// a traves de la IP ("127.0.0.1") y el PUERTO ("127.0.0.1") de dicho
	// servidor, que esta a la espera de NUEVAS CONEXIONES.

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	//bind() y listen() son las llamadas al sistema que realiza la preparacion por parte del proceso servidor

	// Asociamos el socket creado a un puerto
	bind(socket_servidor,servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes a ese socket, notificara cuando un nuevo cliente este intentando conectarse.
	listen(socket_servidor,SOMAXCONN);

	//Liberamos memoria tomada
	freeaddrinfo(servinfo);

	return socket_servidor;
}

//### Retona un nuevo socket que representa la CONEXION BIDIRECCIONAL entre el SERVIDOR y CLIENTE
int esperar_cliente(int socket_servidor)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	return socket_cliente;
}
// Aceptamos un nuevo cliente, accept(): Es una llamada al sistema que es BLOQUEANTE, entonces,
// el proceso servidor se quedara BLOQUEADO en accept hasta que llegue un cliente.
// Si el servidor no esta en accept, cuando el cliente intente conectarse connect(), fallara y devolvera un error

//### Esta función devuelve el número de bytes recibidos o -1 en caso de error.
int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}
// La función recv es como leer, pero con las banderas adicionales. Los posibles valores de las banderas son descrito en Opciones de datos de socket.
















/***********************************************************************************/
/************************************ CLIENTE **************************************/
/***********************************************************************************/

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
