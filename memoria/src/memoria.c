#include "memoria.h"

pthread_t hilo_cpu;
pthread_t hilo_kernel;

t_log* logger;
t_config* config;
int server_memoria;
int conexion_kernel;
int conexion_cpu;
t_list* tablas_primer_nivel;
t_list* tablas_segundo_nivel;
t_bitarray* marcos_libres;
void* espacio_de_usuario;
unsigned int entradas_por_tabla;
char* path_swap;
unsigned int tamanio_memoria;
unsigned int tamanio_pagina;
unsigned int cantidad_paginas;
unsigned int retardo_swap;
unsigned int retardo_memoria;
char* algoritmo_reemplazo;
int marcos_por_proceso;
int contador_tablas_segundo_nivel;
t_proceso* proceso;

int main(void) {

	logger = log_create(PATH_LOG, "MEMORIA", true, LOG_LEVEL_DEBUG);

	leer_config();

	cantidad_paginas = tamanio_memoria / tamanio_pagina;
	marcos_libres = inicializar_bitarray();

	tablas_primer_nivel = list_create();
	contador_tablas_segundo_nivel = 0;

	espacio_de_usuario = malloc(sizeof(tamanio_memoria));

	server_memoria = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir clientes");

	pthread_create(&hilo_cpu, NULL, (void *) atender_cpu, NULL);

	terminar_programa();
	return EXIT_SUCCESS;
}

void leer_config() {
	config = config_create(PATH_CONFIG);
	entradas_por_tabla = config_get_int_value(config, KEY_ENTRADAS_TABLA);
	char* aux_path = config_get_string_value(config, KEY_PATH_SWAP);
	string_append(&aux_path, "/");
	path_swap = string_duplicate(aux_path);
	retardo_swap = config_get_int_value(config, KEY_RETARDO_SWAP);
	retardo_memoria = config_get_int_value(config, KEY_RETARDO_MEMORIA);
	char* aux_algoritmo = config_get_string_value(config, KEY_ALGORITMO_REEMPLAZO);
	algoritmo_reemplazo = string_duplicate(aux_algoritmo);
	marcos_por_proceso = config_get_int_value(config, KEY_MARCOS_POR_PROCESO);
	tamanio_memoria = config_get_int_value(config, KEY_TAM_MEMORIA);
	tamanio_pagina = config_get_int_value(config, KEY_TAM_PAGINAS);
	config_destroy(config);
}

bool conexion_exitosa(int cliente) {
	return cliente != -1;
}

int crear_tabla_paginas() {
	tablas_segundo_nivel = list_create();

	for(int i = 0; i < entradas_por_tabla; i++) {
		t_tabla_paginas_segundo_nivel* tabla = inicializar_tabla_segundo_nivel();
		tabla->numero =	contador_tablas_segundo_nivel;
		contador_tablas_segundo_nivel++;
		list_add(tablas_segundo_nivel, tabla);
	}

	if(list_is_empty(tablas_primer_nivel)) {
		tablas_primer_nivel = list_create();
	}

	list_add(tablas_primer_nivel, (void *) tablas_segundo_nivel);
	return list_size(tablas_primer_nivel) - 1;
}

t_tabla_paginas_segundo_nivel* inicializar_tabla_segundo_nivel() {
	t_tabla_paginas_segundo_nivel* tabla = malloc(sizeof(t_tabla_paginas_segundo_nivel));
	tabla->paginas = list_create();
	for(int i = 0; i < entradas_por_tabla; i++) {
		t_pagina* pagina = inicializar_pagina();
		list_add(tabla->paginas, pagina);
	}

	proceso->puntero = list_get(tabla->paginas, 0);

	return tabla;
}

t_pagina* inicializar_pagina() {
	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->marco = -1;
	pagina->modificada = false;
	pagina->presencia = false;
	pagina->usada = true;
	pagina->tiempo_carga = time(NULL);
	return pagina;
}

t_bitarray* inicializar_bitarray() {
	int cant_bits = (int) ceil(cantidad_paginas / 8);
	void* puntero_a_bits = malloc(cant_bits);
	t_bitarray* bitarray = bitarray_create_with_mode(puntero_a_bits, cant_bits, MSB_FIRST);
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
	return -1;
}

//espacio_de_usuario reservar_espacio_de_usuario(unsigned int tamanio) {
//	espacio_de_usuario espacio;
//	espacio.buffer = malloc(sizeof(tamanio));
//	espacio.inicio = 0;
//	espacio.fin = tamanio;
//	return espacio;
//}

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

void swapear_paginas_modificadas(t_proceso* proceso) {
	t_list* paginas_modificadas = get_paginas_modificadas(proceso);
	FILE* f = txt_open_for_append(get_path_archivo(proceso->id));
	usleep(retardo_swap * 1000);
	for(int i = 0; i < list_size(paginas_modificadas); i++) {
		t_pagina* pagina = list_get(paginas_modificadas, i);
		int contenido = get_contenido_pagina(pagina);
		txt_write_in_file(f, string_itoa(contenido));
	}
	txt_close_file(f);
}

t_list* get_paginas_modificadas(t_proceso* proceso) {
	t_list* tablas_segundo_nivel = list_get(tablas_primer_nivel, proceso->numero_tabla_primer_nivel);
	t_list* paginas_modificadas = list_create();
	for(int i = 0; i < list_size(tablas_segundo_nivel); i++) {
		t_tabla_paginas_segundo_nivel* tabla_segundo_nivel = list_get(tablas_segundo_nivel, i);
		for(int j = 0; j < list_size(tabla_segundo_nivel->paginas); j++){
			t_pagina* pagina = list_get(tabla_segundo_nivel->paginas, j);
			if(pagina->modificada) {
				list_add(paginas_modificadas, pagina);
			}
		}
	}
	list_iterate(paginas_modificadas, (void *) iterador_paginas);
	return paginas_modificadas;
}

int get_contenido_pagina(t_pagina* pagina) {
	// TODO: implementar
	return 1;
}

bool igual_numero(t_tabla_paginas_segundo_nivel* tabla, int numero_a_comparar) {
	return tabla->numero == numero_a_comparar;
}

t_tabla_paginas_segundo_nivel* buscar_tabla_segundo_nivel(int tabla_primer_nivel, int entrada) {
	t_list* tablas_segundo_nivel = list_get(tablas_primer_nivel, tabla_primer_nivel);
	return list_get(tablas_segundo_nivel, entrada);
}

t_pagina* buscar_pagina(int numero_tabla_segundo_nivel, int entrada) {
	int tabla_primer_nivel = floor(numero_tabla_segundo_nivel / entradas_por_tabla);
	t_list* tablas_segundo_nivel = list_get(tablas_primer_nivel, tabla_primer_nivel);
	bool mismo_numero_tabla(t_tabla_paginas_segundo_nivel* tabla) {
		return tabla->numero == numero_tabla_segundo_nivel;
	}
	t_tabla_paginas_segundo_nivel* tabla_segundo_nivel = list_find(tablas_segundo_nivel, (void *) mismo_numero_tabla);

	t_pagina* pagina_encontrada = list_get(tabla_segundo_nivel->paginas, entrada);

	if(!pagina_encontrada->presencia){
		page_fault(pagina_encontrada);
	}//TODO: modificar bit de uso si está en presencia

	return pagina_encontrada;
}


void page_fault(t_pagina* pagina){

	if(cantidad_marcos_proceso() < marcos_por_proceso){
		asignar_marco(pagina);
	} else {
		reemplazar_pagina(pagina);
	}
}

void reemplazar_pagina(t_pagina* pagina){
	if(strcmp(algoritmo_reemplazo, "CLOCK") == 0){

		t_list* paginas = encontrar_paginas_en_memoria();
		list_sort(paginas, (void*)mayor_tiempo_espera);
		int indice_puntero = encontrar_indice_puntero(paginas);

		int i = 0;
		bool reemplazada = false;

		while(i < marcos_por_proceso + 1 && !reemplazada){

			t_pagina* pagina_en_memoria = list_get(paginas, indice_puntero);

			if(!pagina_en_memoria->usada){

				if(pagina_en_memoria->modificada){
					uint32_t contenido = leer_contenido_marco(pagina->marco, 0);
					escribir_en_archivo(contenido);
				}

				pagina->marco = pagina_en_memoria->marco;
				pagina_en_memoria->presencia = false;
				reemplazada = true;

			} else {
				pagina_en_memoria->usada = false;
			}

			if(indice_puntero == marcos_por_proceso){
				indice_puntero = 0;
			} else {
				indice_puntero++;
			}

			i++;
		}

		proceso->puntero = list_get(paginas, indice_puntero);

	} else {

	}
}

void escribir_en_archivo(uint32_t contenido){
	//TODO
}

int encontrar_indice_puntero(t_list* paginas){
	for(int i = 0; i < list_size(paginas); i++){
		t_pagina* pagina = list_get(paginas, i);
		if(pagina->marco == proceso->puntero->marco){
			return i;
		}
	}

	return -1;
}


t_pagina* mayor_tiempo_espera(t_pagina* p1, t_pagina* p2) {
	if(p1->tiempo_carga > p2->tiempo_carga) {
		return p2;
	} else {
		return p1;
	}
}

t_list* encontrar_paginas_en_memoria(){
	//TODO
	return NULL;
}

void asignar_marco(t_pagina* pagina){
	pagina->marco = proximo_marco_libre();
	pagina->presencia = true;
	uint32_t contenido = leer_pagina_disco(pagina);
	escribir_en_marco(contenido);
}

void reemplazar_pagina(t_pagina* pagina){

}

int cantidad_marcos_proceso(){
	//TODO
	return 1;
}

uint32_t leer_pagina_disco(t_pagina* pagina){

}

uint32_t leer_contenido_marco(int numero_de_marco, int desplazamiento) {
	if((numero_de_marco * tamanio_pagina) + desplazamiento > tamanio_memoria - 1) {
		return -1;
	} else {
		uint32_t* contenido = malloc(sizeof(uint32_t));
		memcpy(contenido, espacio_de_usuario + (numero_de_marco * tamanio_pagina) + desplazamiento, sizeof(uint32_t));
		return *contenido;
	}

}

int escribir_en_marco(int numero_de_marco, int desplazamiento, uint32_t valor) {
	if((numero_de_marco * tamanio_pagina) + desplazamiento > tamanio_memoria - 1) {
		return -1;
	} else {
		memcpy(espacio_de_usuario + (numero_de_marco * tamanio_pagina) + desplazamiento, &valor, sizeof(uint32_t));
		return 1;
	}

}

void liberar_tabla_primer_nivel(int numero) {
	//t_list* tablas_a_remover = (t_list*) dictionary_remove(tablas_primer_nivel, string_itoa(numero));
	//liberar_tablas_segundo_nivel(tablas_a_remover);
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

//void liberar_espacio_de_usuario(espacio_de_usuario espacio) {
//	free(espacio.buffer);
//}

void terminar_programa() {
	pthread_join(hilo_kernel, NULL);
	pthread_join(hilo_cpu, NULL);
	liberar_conexion(conexion_cpu);
	liberar_conexion(conexion_kernel);
	liberar_tablas();
	bitarray_destroy(marcos_libres);
	log_destroy(logger);
}

void liberar_tablas() {
	//dictionary_destroy_and_destroy_elements(tablas_primer_nivel, (void *) liberar_tablas_segundo_nivel);
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
				enviar_numero_de_tabla(conexion_cpu, tamanio_pagina);
				enviar_numero_de_tabla(conexion_cpu, entradas_por_tabla);
				pthread_create(&hilo_kernel, NULL, (void *) atender_kernel, NULL);
				break;
			case ACCESO_TABLA_PRIMER_NIVEL:
				log_info(logger, "CPU solicita acceso a tabla pagina de primer nivel");
				int numero_de_tabla_primer_nivel = recibir_numero_tabla(conexion_cpu);
				int entrada_tabla_primer_nivel = recibir_numero_entrada(conexion_cpu);
				t_tabla_paginas_segundo_nivel* tabla = buscar_tabla_segundo_nivel(numero_de_tabla_primer_nivel, entrada_tabla_primer_nivel);
				enviar_numero_de_tabla(conexion_cpu, tabla->numero);

				break;
			case ACCESO_TABLA_SEGUNDO_NIVEL:
				log_info(logger, "CPU solicita acceso a tabla pagina de segundo nivel");
				int numero_de_tabla_segundo_nivel = recibir_numero_tabla(conexion_cpu);
				int entrada_tabla_segundo_nivel = recibir_numero_entrada(conexion_cpu);
				t_pagina* pagina = buscar_pagina(numero_de_tabla_segundo_nivel, entrada_tabla_segundo_nivel);
				enviar_numero_de_pagina(conexion_cpu, pagina->marco);
				break;
			case LECTURA_MEMORIA_USUARIO:
				log_info(logger, "CPU solicita lectura a memoria de usuario");
				int marco_lectura = recibir_entero(conexion_cpu);
				int desplazamiento_lectura = recibir_entero(conexion_cpu);
				uint32_t contenido = leer_contenido_marco(marco_lectura, desplazamiento_lectura);
				break;
			case ESCRITURA_MEMORIA_USUARIO:
				log_info(logger, "CPU solicita escritura a memoria de usuario");
				int marco_escritura = recibir_entero(conexion_cpu);
				int desplazamiento_escritura = recibir_entero(conexion_cpu);
				uint32_t valor = recibir_uint32(conexion_cpu);
				int respuesta = escribir_en_marco(marco_escritura, desplazamiento_escritura, valor);
				enviar_respuesta(conexion_cpu, respuesta);
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
				proceso = malloc(sizeof(t_proceso));

				proceso->id= recibir_id_proceso(conexion_kernel);
				proceso->tamanio = recibir_tamanio(conexion_kernel);
				proceso->numero_tabla_primer_nivel = crear_tabla_paginas();

				log_info(logger, "Se creo la tabla de primer nivel: %d", proceso->numero_tabla_primer_nivel);

				crear_archivo_swap(get_path_archivo(proceso->id));

				enviar_numero_de_tabla(conexion_kernel, proceso->numero_tabla_primer_nivel);

				break;
			case SUSPENCION_PROCESO:
				log_info(logger, "Kernel solicita SUSPENCION PROCESO");
				swapear_paginas_modificadas(proceso);

				// todo: liberar marcos swapeados
				enviar_confirmacion(conexion_kernel);

				break;
			case FINALIZACION_PROCESO:
				log_info(logger, "Kernel solicita FINALIZACION PROCESO");

				// todo: liberar espacio de usuario de proceso
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


