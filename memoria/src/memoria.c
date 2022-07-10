#include "memoria.h"

pthread_t hilo_cpu;
pthread_t hilo_kernel;

t_log* logger;
t_config* config;
//t_proceso* proceso;
int server_memoria;
int conexion_kernel;
int conexion_cpu;
t_dictionary* tablas_primer_nivel;
t_list* tablas_segundo_nivel;
t_bitarray* marcos_libres;
unsigned int entradas_por_tabla;
char* path_swap;
unsigned int tamanio_memoria;
unsigned int tamanio_pagina;
unsigned int cantidad_paginas;

int main(void) {

	logger = log_create(PATH_LOG, "MEMORIA", true, LOG_LEVEL_DEBUG);

	config = config_create(PATH_CONFIG);

	entradas_por_tabla = config_get_int_value(config, KEY_ENTRADAS_TABLA);
	path_swap = config_get_string_value(config, KEY_PATH_SWAP);
	string_append(&path_swap, "/");

	tamanio_memoria = config_get_int_value(config, KEY_TAM_MEMORIA);
	tamanio_pagina = config_get_int_value(config, KEY_TAM_PAGINAS);
	cantidad_paginas = tamanio_memoria / tamanio_pagina;
	marcos_libres = inicializar_bitarray();
	//log_info(logger, "Cantidad de marcos: %d", bitarray_get_max_bit(marcos_libres));

	tablas_primer_nivel = dictionary_create();

	server_memoria = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir clientes");

	pthread_create(&hilo_cpu, NULL, (void *) atender_cpu, NULL);

	terminar_programa();
	return EXIT_SUCCESS;
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}

int crear_tabla_paginas() {
	tablas_segundo_nivel = list_create();

	for(int i = 0; i < entradas_por_tabla; i++) {
		t_tabla_paginas_segundo_nivel* tabla = inicializar_tabla_segundo_nivel();
		tabla->numero = list_size(tablas_segundo_nivel) + 1;
		list_add(tablas_segundo_nivel, tabla);

	}

	char* proximo_numero = string_itoa(dictionary_size(tablas_primer_nivel) + 1);
	dictionary_put(tablas_primer_nivel, proximo_numero, tablas_segundo_nivel);
	return dictionary_size(tablas_primer_nivel);
}

t_tabla_paginas_segundo_nivel* inicializar_tabla_segundo_nivel() {
	t_tabla_paginas_segundo_nivel* tabla = malloc(sizeof(t_tabla_paginas_segundo_nivel));
	tabla->paginas = list_create();
	for(int i = 0; i < entradas_por_tabla; i++) {
		t_pagina* pagina = inicializar_pagina();
		list_add(tabla->paginas, pagina);
	}

	return tabla;
}

t_pagina* inicializar_pagina() {
	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->marco = proximo_marco_libre();
	pagina->modificada = false;
	pagina->presencia = true;
	pagina->usada = true;
	return pagina;
}

t_bitarray* inicializar_bitarray() {
	void* puntero_a_bits = malloc(cantidad_paginas);
	t_bitarray* bitarray = bitarray_create_with_mode(puntero_a_bits, cantidad_paginas, MSB_FIRST);
	free(puntero_a_bits);
	return bitarray;
}

int proximo_marco_libre() {
	for(int i = 0; i < bitarray_get_max_bit(marcos_libres); i++) {
		if(!bitarray_test_bit(marcos_libres, i)) {
			bitarray_set_bit(marcos_libres, i);
			return i;
		}
	}
	// TODO: preguntar que pasa si no hay marcos libres, por ahora retorno -1
	return -1;
}

espacio_de_usuario reservar_espacio_de_usuario(unsigned int tamanio) {
	espacio_de_usuario espacio;
	espacio.buffer = malloc(sizeof(tamanio));
	espacio.inicio = 0;
	espacio.fin = tamanio;
	return espacio;
}

void crear_archivo_swap(char* path) {
	FILE* f = fopen(path, "w");
	fclose(f);
}

char* get_path_archivo(pid_t id) {
	char* extension = ".swap";
	char* nombre = string_itoa(id);
	char* aux = string_new();
	string_append(&aux, path_swap);
	string_append(&nombre, extension);
	string_append(&aux, nombre);
	return aux;
}

void liberar_tabla_primer_nivel(int numero) {
	t_list* tablas_a_remover = (t_list*) dictionary_remove(tablas_primer_nivel, string_itoa(numero));
	liberar_tablas_segundo_nivel(tablas_a_remover);
}

void liberar_tablas_segundo_nivel(t_list* tablas) {
	list_destroy_and_destroy_elements(tablas, (void *) liberar_tabla_segundo_nivel);
}

void liberar_tabla_segundo_nivel(t_tabla_paginas_segundo_nivel* tabla) {
	list_destroy_and_destroy_elements(tabla->paginas, (void *) liberar_pagina);
	free(tabla);
}

void liberar_pagina(t_pagina* pagina) {
	bitarray_clean_bit(marcos_libres, pagina->marco);
	free(pagina);
}

void liberar_espacio_de_usuario(espacio_de_usuario espacio) {
	free(espacio.buffer);
}

void terminar_programa() {
	pthread_join(hilo_kernel, NULL);
	pthread_join(hilo_cpu, NULL);

	//liberar_conexion(conexion_cpu);
	liberar_conexion(conexion_kernel);
	liberar_tablas();
	bitarray_destroy(marcos_libres);
}

void liberar_tablas() {
	dictionary_destroy_and_destroy_elements(tablas_primer_nivel, (void *) liberar_tablas_segundo_nivel);
}

void iterador_tablas_segundo_nivel(t_tabla_paginas_segundo_nivel* tabla) {
	log_info(logger, "Tablas segundo nivel:");
	log_info(logger, "%d", tabla->numero);
	log_info(logger, "Marcos: ");
	list_iterate(tabla->paginas, (void*) iterador_paginas);
}

void iterador_paginas(t_pagina* pag) {
	log_info(logger, "%d", pag->marco);
}

void atender_cpu() {
	log_info(logger, "Entro Atender CPU");
	conexion_cpu = esperar_cliente(server_memoria);
	log_info(logger, "Se conecto CPU: %d",conexion_cpu);

	if(!conexion_exitosa(conexion_cpu)) {
		log_error(logger, "No se pudo establecer la conexion con la cpu");
		exit(EXIT_FAILURE);
	}

	while(1) {
		operacion operacion = recibir_operacion(conexion_cpu);
		switch(operacion) {
			case HANDSHAKE_CPU:
				log_info(logger, "CPU solicita acceso a info nro_filas_tabla_nivel1 y tamano_pagina");
				enviar_numero_de_tabla(conexion_cpu,tamanio_pagina);
				enviar_numero_de_tabla(conexion_cpu,entradas_por_tabla);
				pthread_create(&hilo_kernel, NULL, (void *) atender_kernel, NULL);
				break;
			case ACCESO_TABLA_PRIMER_NIVEL:
				log_info(logger, "CPU solicita acceso a tabla pagina de primer nivel");
				//unsigned int numero_tabla = recibir_numero_tabla(conexion_kernel);
				//char* tabla_segundo_nivel = dictionary_get(tablas_primer_nivel, string_itoa(numero_tabla));
				int  nro_entrada_tabla = recibir_entero(conexion_cpu);
				log_info(logger, "CPU envio: %d",nro_entrada_tabla);

				int entrada_tabla_1er_nivel = recibir_entero(conexion_cpu);
				log_info(logger, "CPU envio: %d",entrada_tabla_1er_nivel);

				int entrada_tabla_segundo_nivel = 2;

				enviar_numero_de_tabla(conexion_cpu,entrada_tabla_segundo_nivel);
				log_info(logger, "Le envie a CPU entrada_tabla_segundo_nivel: %d", entrada_tabla_segundo_nivel);

				break;
			case ACCESO_TABLA_SEGUNDO_NIVEL:
				log_info(logger, "CPU solicita acceso a tabla pagina de segundo nivel");
				break;
			case LECTURA_MEMORIA_USUARIO:
				log_info(logger, "CPU solicita lectura a memoria de usuario");
				break;
			case ESCRITURA_MEMORIA_USUARIO:
				log_info(logger, "CPU solicita escritura a memoria de usuario");
				break;
			case ERROR:
				log_error(logger, "Se desconecto la cpu");
				exit(EXIT_FAILURE);
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}
}

void atender_kernel() {
	log_info(logger, "Entro Atender KERNEL");
	conexion_kernel = esperar_cliente(server_memoria);

	if(!conexion_exitosa(conexion_kernel)) {
		log_error(logger, "No se pudo establecer la conexion con el kernel");
		exit(EXIT_FAILURE);
	}

	while(1) {
		operacion operacion = recibir_operacion(conexion_kernel);
		switch(operacion) {
			case INICIO_PROCESO:
				// TODO: el kernel debería solicitar esto solo si el proceso no tiene ya asignado un numero de tabla
				log_info(logger, "Kernel solicita INICIO PROCESO");
				t_proceso* proceso = malloc(sizeof(t_proceso));

				proceso->id= recibir_id_proceso(conexion_kernel);
				proceso->tamanio = recibir_tamanio(conexion_kernel);
				proceso->numero_tabla_primer_nivel = crear_tabla_paginas();
				proceso->espacio_utilizable = reservar_espacio_de_usuario(proceso->tamanio);
				log_info(logger, "Se reservo un espacio de %d bytes para el proceso %d", proceso->espacio_utilizable.fin, proceso->id);

				log_info(logger, "Se creo la tabla de primer nivel: %d", proceso->numero_tabla_primer_nivel);

				// Itero la tabla de nivel 1 y las de nivel 2 para ver que se asignen bien
				//char* numero = string_itoa(proceso->numero_tabla_primer_nivel);
				//list_iterate((t_list*) dictionary_get(tablas_primer_nivel, numero), (void*) iterador_tablas_segundo_nivel);

				crear_archivo_swap(get_path_archivo(proceso->id));

				enviar_numero_de_tabla(conexion_kernel, proceso->numero_tabla_primer_nivel);

				break;
			case SUSPENCION_PROCESO:
				log_info(logger, "Kernel solicita SUSPENCION PROCESO");
				break;
			case FINALIZACION_PROCESO:
				log_info(logger, "Kernel solicita FINALIZACION PROCESO");
				// No hace falta recibir nada ya que se libera el proceso que actualmente está en memoria
				liberar_tabla_primer_nivel(proceso->numero_tabla_primer_nivel);
				liberar_espacio_de_usuario(proceso->espacio_utilizable);
				remove(get_path_archivo(proceso->id));
				log_info(logger, "Id a finalizar: %d", proceso->id);

				break;
			case ERROR:
				log_error(logger, "Se desconecto el kernel");
				exit(EXIT_FAILURE);
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}
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


