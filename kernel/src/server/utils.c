//+++ FUNCIONALIDAD DE SERVIDOR +++

#include"utils.h"

int iniciar_servidor(void)
{
	int socket_servidor; //Guarda el File Descriptor(IDs) representado por un entero.

	struct addrinfo hints, *servinfo, *p; // Estruc q Contendra información sobre la dirección de un proveedor de servicios.

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(IP, PUERTO, &hints, &servinfo); //Traduce el nombre de una ubicación de servicio a un conjunto de direcciones de socket.
	/*El cliente para poder conectarse con el servidor, deben hacerlo
	 * a traves de la IP(xej: "127.0.0.1") del servidor fisico en el
	 * que esten corriendo y el PUERTO(xej: "127.0.0.1") que esten ocupando
	 * en dicho servidor a la espera de NUEVAS CONEXIONES
	 * En este ejemplo le decimos que obtenga informacion sobre la computadora local
	 * porque es donde estamos tratando de kevantar el servidor para que los clientes
	 * en otras computadoras púedan conectar.
	 * */

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	/* bind() y listen() son las llamadas al sistema que realiza
	 * la preparacion por parte del proceso servidor */

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

void* recibir_buffer(int* size, int socket_cliente)
{
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

t_proceso* recibir_proceso(int socket_cliente) {
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->tamanio = recibir_tamanio_proceso(socket_cliente);
	proceso->instrucciones = recibir_instrucciones(socket_cliente);
	return proceso;
}

void destruir_nodo(t_link_element* nodo) {
	free(nodo);
}

void destruir_proceso(t_proceso* proceso) {
	list_destroy_and_destroy_elements(proceso->instrucciones, (void*) destruir_nodo);
	free(proceso);
}


/* Un socket es la representacion que el Sistema Operativo le da
 * a esa conexion.
 *
 * Socket: Crea un punto final para la comunicacion y devuelve un
 * descriptor  que hace referencia a ese punto final.
 *
 * int socket(int DOMINIO, int TIPO, int PROTOCOLO);
 *
 * @DOMINIO: Familias de DIRECCIONES de socket (dominios), es decir,
 * especifica un DOMINIO de comunicacion,
 * esto selecciona la familia de protocolos que se utilizara
 * para la comunicacion.
 *   Name_Comunicacion  Proposito/Objetivo
 *   	AF_UNIX				Comunicacion Local
 *  	AF_LOCAL			Sinonimo de AF_UNIX
 *  	AF_INET				Protocolos de Internet IPv4
 *   	AF_INET6			Protocolos de Internet IPv6
 *
 * @TIPO: Especifica la semantica de la comunicacion. Ej tipos:
 * 		SOCKET_STREAM: Proporsiona flujos de Bytes secuenciados,
 * 		fiables, bidireccionables y basados en conexion.
 *
 * @PROTOCOLO: Especifica un protocolo particular que se utilizara
 * con el socket.
 * 	Los sockets de tipo SOCK_STREAM son flujos de bytes de duplex
 * 	completo. No conservan los limites de los registros. Deben estar en
 * 	un estado CONECTADO ANTES DE SE PUEDAN ENVIAR O RECIBIR DATOS de el.
 * 	Se crea una conexion a otro socket con una llamada connect(). Una vez
 * 	conectados, los datos pueden transferirse mediante llamadas de lectura
 * 	y escritura o alguna variante de las llamadas de envio y recepcion.
 * 	Cuando se ha completado una sesion se puede realizar un cierre.
 * 	Los protocolos de comunicacion que implementan un SOCK_STREAM aseguran
 * 	que LOS DATOS NO SE PIERDEN NI SE DUPLIQUEN.
 *
 * Info_Oficial: https://man7.org/linux/man-pages/man2/socket.2.html
 */

/* ai_family: Este campo especifica la familia de direcciones
 * deseada para las direcciones devueltas. Los valores validos
 * para este campo incluyen AF_INET y AF_INET6. El valor AF_UNSPEC
 * indica que getaddrinfo() debe devolver direcciones de socket
 * para cualquier familia de direcciones (ya sea IPv4 o IPv6) que
 * se puedan usar con el nodo y el servicio.
 */
