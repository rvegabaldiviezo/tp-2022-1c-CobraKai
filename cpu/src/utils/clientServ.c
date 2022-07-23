#include "clientServ.h"
//#include "../cpu.h"

/***********************************************************************************/
/************************************ SERVER **************************************/
/***********************************************************************************/



//##### FUNCIONES SERVER  #######

// Crear el socket del servidor, para poder recibir peticiones del cliente que se conecte al servidor.
int iniciar_servidor(char* ip, char* puerto)
{
	//Guarda el File Descriptor(IDs)
	int socket_servidor;

	//Estruc q Contendra información sobre la dirección de un proveedor de servicios.
	struct addrinfo hints, *servinfo;//, *p;

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
	//printf("Entro a esperar_cliente, recibimos el socket_servidor: %d",socket_servidor);
	//printf("Esta por entrar al accept()");
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	//printf("Salio del accept(), xq se conecto un cliente, el socket_cliente: %d", socket_cliente);
	return socket_cliente;
}
// Aceptamos un nuevo cliente, accept(): Es una llamada al sistema que es BLOQUEANTE, entonces,
// el proceso servidor se quedara BLOQUEADO en accept hasta que llegue un cliente.
// Si el servidor no esta en accept, cuando el cliente intente conectarse connect(), fallara y devolvera un error

//### Esta función devuelve el número de bytes recibidos o -1 en caso de error.
int recibir_operacion(int socket_cliente)
{
	operacion cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(operacion), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}
// La función recv es como leer, pero con las banderas adicionales. Los posibles valores de las banderas son descrito en Opciones de datos de socket.


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
	//log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

int recibir_entero(int socket_cliente) {
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0) {
		return cod_op;
	} else {
		close(socket_cliente);
		return -1;
	}
}

uint32_t recibir_uint32_t(int socket_cliente) {
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) > 0) {
		return cod_op;
	} else {
		close(socket_cliente);
		return -1;
	}
}

t_pcb* recibir_pcb(int conexion) {
	// orden en el que vienen: operacion, id, socket, tamanio, program_counter, estimacion_rafaga, numero_tabla, tamanio_instrucciones, instrucciones
	t_pcb* pcb = malloc(sizeof(t_pcb));
	//int operacion = recibir_entero(conexion);
	pcb->id = recibir_entero(conexion);
	pcb->socket_cliente = recibir_entero(conexion);
	pcb->tamanio_proceso = recibir_entero(conexion);
	pcb->program_counter = recibir_entero(conexion);
	pcb->estimacion_rafaga = recibir_entero(conexion);
	pcb->tablas_paginas = recibir_entero(conexion);
	pcb->inicio_rafaga = recibir_entero(conexion);
	pcb->instrucciones = recibir_instrucciones(conexion);
	return pcb;
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

// Comunicacion Kernel: enviar PCB
void enviar_pcb(t_pcb* pcb, int conexion, operacion op) {

	t_buffer* buffer = cargar_buffer(pcb->instrucciones);
	int bytes =  8 * sizeof(int) + sizeof(time_t) + buffer->size;

	void* pcb_serializado = serializar_pcb(pcb, buffer, bytes,op);
	send(conexion, pcb_serializado, bytes, 0);

	//free(buffer);
	free(pcb_serializado);

	free(pcb);
}

void liberar_pcb(t_pcb* pcb){
	void _destroy_element(char* str){
		free(str);
	}
	list_destroy_and_destroy_elements(pcb->instrucciones, (void*) _destroy_element);

	free(pcb);
}

// Comunicacion Memoria
void enviar_codigo_operacion(int conexion,int operacion) {
	send(conexion, &operacion, sizeof(int), MSG_NOSIGNAL);
	//free(pcb_serializado);
}


//##################33333
void enviar_primer_acceso_memoria(int conexion,uint32_t nro_entrada_tabla,uint32_t entrada_tabla_1er_nivel) {

	operacion cod_op = NUMERO_DE_TABLA_PRIMER_NIVEL;
	uint32_t bytes = sizeof(operacion) + 2 *sizeof(uint32_t);
	void* a_enviar = malloc(bytes);
	uint32_t desplazamiento = 0;

	memcpy(a_enviar + desplazamiento, &cod_op, sizeof(operacion));
	desplazamiento += sizeof(operacion);

	memcpy(a_enviar + desplazamiento, &nro_entrada_tabla, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(a_enviar + desplazamiento, &entrada_tabla_1er_nivel, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	send(conexion, a_enviar, bytes, MSG_NOSIGNAL);

	free(a_enviar);
}


void enviar_segundo_acceso_memoria(int conexion,uint32_t nro_entrada_tabla,uint32_t entrada_tabla_1er_nivel) {

	operacion cod_op = NUMERO_DE_TABLA_SEGUNDO_NIVEL;
	uint32_t bytes = sizeof(operacion) + 2 *sizeof(uint32_t);
	void* a_enviar = malloc(bytes);
	uint32_t desplazamiento = 0;

	memcpy(a_enviar + desplazamiento, &cod_op, sizeof(operacion));
	desplazamiento += sizeof(operacion);

	memcpy(a_enviar + desplazamiento, &nro_entrada_tabla, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(a_enviar + desplazamiento, &entrada_tabla_1er_nivel, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	send(conexion, a_enviar, bytes, MSG_NOSIGNAL);

	free(a_enviar);
}



void enviar_tercer_acceso_memoria_lectura(int conexion,uint32_t nro_marco,uint32_t desplazamiento) {

	operacion cod_op = LECTURA_MEMORIA_USUARIO;
	uint32_t bytes = sizeof(operacion) + 2 *sizeof(uint32_t);
	void* a_enviar = malloc(bytes);
	uint32_t desplazamiento_bytes = 0;

	memcpy(a_enviar + desplazamiento_bytes, &cod_op, sizeof(operacion));
	desplazamiento_bytes += sizeof(operacion);

	memcpy(a_enviar + desplazamiento_bytes, &nro_marco, sizeof(uint32_t));
	desplazamiento_bytes += sizeof(uint32_t);

	memcpy(a_enviar + desplazamiento_bytes, &desplazamiento, sizeof(uint32_t));
	desplazamiento_bytes += sizeof(uint32_t);

	send(conexion, a_enviar, bytes, MSG_NOSIGNAL);

	free(a_enviar);
}

void enviar_tercer_acceso_memoria_escritura(int conexion,uint32_t nro_marco,uint32_t desplazamiento,uint32_t valor, int id_proceso) {

	operacion cod_op = ESCRITURA_MEMORIA_USUARIO;
	int bytes = sizeof(operacion) + 3 *sizeof(uint32_t) + sizeof(int);
	void* a_enviar = malloc(bytes);
	int desplazamiento_bytes = 0;

	memcpy(a_enviar + desplazamiento_bytes, &cod_op, sizeof(operacion));
	desplazamiento_bytes += sizeof(operacion);

	memcpy(a_enviar + desplazamiento_bytes, &nro_marco, sizeof(uint32_t));
	desplazamiento_bytes += sizeof(uint32_t);

	memcpy(a_enviar + desplazamiento_bytes, &desplazamiento, sizeof(uint32_t));
	desplazamiento_bytes += sizeof(uint32_t);

	memcpy(a_enviar + desplazamiento_bytes, &valor, sizeof(uint32_t));
	desplazamiento_bytes += sizeof(uint32_t);

	memcpy(a_enviar + desplazamiento_bytes, &id_proceso, sizeof(int));
	desplazamiento_bytes += sizeof(int);

	send(conexion, a_enviar, bytes, MSG_NOSIGNAL);

	free(a_enviar);
}


void* serializar_pcb(t_pcb* pcb, t_buffer* buffer, int bytes,operacion op) {
	void* a_enviar = malloc(bytes);
	int desplazamiento = 0;

	memcpy(a_enviar + desplazamiento, &op, sizeof(operacion));
	desplazamiento += sizeof(operacion);

	memcpy(a_enviar + desplazamiento, &(pcb->id), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->socket_cliente), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->tamanio_proceso), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->program_counter), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->estimacion_rafaga), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->tablas_paginas), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->inicio_rafaga), sizeof(time_t));
	desplazamiento += sizeof(time_t);

	memcpy(a_enviar + desplazamiento, &(buffer->size), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, buffer->stream, buffer->size);

	return a_enviar;
}


void enviar_pcb_bloqueado(int socketKernel,t_pcb_bloqueado* pcb_bloqueado){
	t_pcb* pcb = pcb_bloqueado->pcb;

	t_buffer* buffer = cargar_buffer(pcb->instrucciones);

	int bytes = 9 * sizeof(int) + sizeof(time_t) + buffer->size;

	operacion op = BLOQUEO_IO;

	void* a_enviar = malloc(bytes);
	int desplazamiento = 0;

	memcpy(a_enviar + desplazamiento, &op, sizeof(operacion));
	desplazamiento += sizeof(operacion);

	memcpy(a_enviar + desplazamiento, &(pcb_bloqueado->tiempo_bloqueo), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->id), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->socket_cliente), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->tamanio_proceso), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->program_counter), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->estimacion_rafaga), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->tablas_paginas), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, &(pcb->inicio_rafaga), sizeof(time_t));
	desplazamiento += sizeof(time_t);

	memcpy(a_enviar + desplazamiento, &(buffer->size), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(a_enviar + desplazamiento, buffer->stream, buffer->size);



	send(socketKernel, a_enviar, bytes, MSG_NOSIGNAL);

	//free(buffer);
	free(pcb_bloqueado);
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









/***********************************************************************************/
/************************************ CLIENTE **************************************/
/***********************************************************************************/

// Inicia el logger en la RUTA_LOG
t_log* iniciar_logger(void)
{
	t_log* nuevo_logger = log_create(PATH_LOG,NAME_LOG,true,LOG_LEVEL_INFO);
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
