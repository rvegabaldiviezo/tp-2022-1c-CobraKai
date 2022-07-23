#include "memoria.h"

pthread_t hilo_cpu;
pthread_t hilo_kernel;
pthread_t hilo_swap;
sem_t swap;

t_log* logger;
t_config* config;
int server_memoria;
int conexion_kernel;
int conexion_cpu;
t_list* tablas_primer_nivel;
t_list* tablas_segundo_nivel;
t_bitarray* marcos_memoria;
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
int contador_paginas;
int numero_de_tabla_primer_nivel;
t_list* punteros_procesos;
int id_a_suspender;


int main(void) {

	logger = log_create(PATH_LOG, "MEMORIA", true, LOG_LEVEL_DEBUG);

	leer_config();

	cantidad_paginas = tamanio_memoria / tamanio_pagina;
	marcos_memoria = inicializar_bitarray();
	log_info(logger, "Cantidad de marcos: %d", bitarray_get_max_bit(marcos_memoria));

	tablas_primer_nivel = list_create();

	espacio_de_usuario = malloc(tamanio_memoria);

	punteros_procesos = list_create();

	server_memoria = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir clientes");

	crear_carpeta_swap();
	sem_init(&swap, 0, 0);
	id_a_suspender = -1;

	pthread_create(&hilo_cpu, NULL, (void *) atender_cpu, NULL);
	//pthread_create(&hilo_swap, NULL, (void *) solicitudes_swap, NULL);

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
	contador_paginas = 0;
	for(int i = 0; i < entradas_por_tabla; i++) {
		t_tabla_paginas_segundo_nivel* tabla = inicializar_tabla_segundo_nivel();
		tabla->numero =	i;
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
		t_pagina* pagina = inicializar_pagina(i, tabla->numero);
		list_add(tabla->paginas, pagina);
	}

	return tabla;
}

t_pagina* inicializar_pagina(int numero, int tabla) {
	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina->marco = -1;
	pagina->modificada = false;
	pagina->presencia = false;
	pagina->usada = true;
	pagina->tiempo_carga = time(NULL);
	pagina->tabla_segundo_nivel = tabla;
	pagina->numero = contador_paginas;
	contador_paginas++;
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
	for(off_t i = 0; i < bitarray_get_max_bit(marcos_memoria); i++) {
		if(!bitarray_test_bit(marcos_memoria, i)) {
			bitarray_set_bit(marcos_memoria, i);
			return i;
		}
	}
	return -1;
}

void crear_carpeta_swap() {
	DIR* dir = opendir(path_swap);
	if(dir) {
		closedir(dir);
	} else {
		mkdir(path_swap, 0777);
	}
}

int crear_archivo_swap(int id, int tamanio) {
	int fd = open(get_path_archivo(id), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd != -1) {
		ftruncate(fd, tamanio);
		close(fd);
		return fd;
	} else {
		log_info(logger, "Ocurrio un error al cread el archivo");
		return -1;
	}

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

void swapear_paginas_modificadas(int id) {
	t_list* paginas_modificadas = get_paginas_modificadas(id);

	for(int i = 0; i < list_size(paginas_modificadas); i++) {
		t_pagina* pagina = list_get(paginas_modificadas, i);
		log_info(logger, "Marco pagina modificada: %d", pagina->marco);
		escribir_en_archivo(pagina, id);
		pagina->modificada = false;
		pagina->presencia = false;
		//liberar_pagina(pagina);
	}

	t_list* paginas_en_memoria = encontrar_paginas_en_memoria();
	for(int i = 0; i < list_size(paginas_en_memoria); i++) {
		t_pagina* pagina = list_get(paginas_en_memoria, i);
		pagina->presencia = false;
		//liberar_pagina(pagina);
	}
}

void liberar_elementos(void* elemento) {
	free(elemento);
}

//char* list_split(t_list* lista, char* separador) {
	//char* lista_string = string_new();

	//for(int i = 0; i < list_size(lista) - 1; i++) {
		//uint32_t* elemento = list_get(lista, i);
		//char* string_elemento = string_itoa(*elemento);
		//string_append(&string_elemento, separador);
		//string_append(&lista_string, string_elemento);
		//free(elemento);
	//}
	//uint32_t* ultimo_elemento = list_get(lista, list_size(lista));
	//char* string_ult_elemento = string_itoa(*ultimo_elemento);
	//string_append(&lista_string, string_ult_elemento);
	//free(ultimo_elemento);
	//return lista_string;
//}

t_list* get_paginas_modificadas(int id) {
	t_list* tablas_segundo_nivel = list_get(tablas_primer_nivel, id);
	t_list* paginas_modificadas = list_create();
	for(int i = 0; i < list_size(tablas_segundo_nivel); i++) {
		t_tabla_paginas_segundo_nivel* tabla_segundo_nivel = list_get(tablas_segundo_nivel, i);
		for(int j = 0; j < list_size(tabla_segundo_nivel->paginas); j++){
			t_pagina* pagina = list_get(tabla_segundo_nivel->paginas, j);
			if(pagina->modificada && pagina->presencia) {
				list_add(paginas_modificadas, pagina);
			}
		}
	}
	return paginas_modificadas;
}

//t_list* get_contenido_pagina(t_pagina* pagina) {
	//t_list* contenido_pagina = list_create();
	//int cantidad_max_elementos = floor(tamanio_pagina / sizeof(uint32_t));

	//for(int i = 0; i < cantidad_max_elementos; i++) {
		//uint32_t contenido_en_desplazamiento = leer_contenido_marco(pagina->marco, i * sizeof(uint32_t));
		//if(contenido_en_desplazamiento != -1) {
			//list_add(contenido_pagina, &contenido_en_desplazamiento);
		//}
	//}
	//return contenido_pagina;
//}

t_tabla_paginas_segundo_nivel* buscar_tabla_segundo_nivel(int tabla_primer_nivel, int entrada) {
	t_list* tablas_segundo_nivel = list_get(tablas_primer_nivel, tabla_primer_nivel);
	return list_get(tablas_segundo_nivel, entrada);
}

t_pagina* buscar_pagina(int numero_tabla_segundo_nivel, int entrada) {
	int tabla_primer_nivel = floor(numero_tabla_segundo_nivel / entradas_por_tabla);
	t_list* tablas_segundo_nivel = list_get(tablas_primer_nivel, numero_de_tabla_primer_nivel);
	bool mismo_numero_tabla(t_tabla_paginas_segundo_nivel* tabla) {
		return tabla->numero == numero_tabla_segundo_nivel;
	}
	t_tabla_paginas_segundo_nivel* tabla_segundo_nivel = list_find(tablas_segundo_nivel, (void *) mismo_numero_tabla);



	t_pagina* pagina_encontrada = list_get(tabla_segundo_nivel->paginas, entrada);

	if(!pagina_encontrada->presencia){
		page_fault(pagina_encontrada);
	} else {
		modificar_bit_uso(pagina_encontrada);
	}

	return pagina_encontrada;
}

void modificar_bit_uso(t_pagina* pagina){
	if(!pagina->usada){
		pagina->usada = true;
	}
}

void page_fault(t_pagina* pagina_en_disco){

	if(cantidad_marcos_ocupados_proceso() < marcos_por_proceso){
		asignar_marco(pagina_en_disco);
	} else {
		reemplazar_pagina(pagina_en_disco);
	}
}

void reemplazar_pagina(t_pagina* pagina){
	if(strcmp(algoritmo_reemplazo, "CLOCK") == 0){
		algoritmo_clock(pagina);
	} else {
		algoritmo_clock_modificado(pagina);
	}
	log_warning(logger, "Se reemplazó la pagina %d", pagina->numero);
}

void algoritmo_clock(t_pagina* pagina){

	t_list* paginas = encontrar_paginas_en_memoria();
	list_sort(paginas, (void*) mayor_tiempo_espera);

	bool reemplazada = false;
	t_pagina* pagina_en_memoria;
	int indice_puntero = encontrar_indice_puntero(paginas);

	if(indice_puntero == -1) { // si nunca se habia movido el puntero se inicializa en el primer marco
		indice_puntero = 0;
	}

	while(!reemplazada){

		pagina_en_memoria = list_get(paginas, indice_puntero);

		if(!pagina_en_memoria->usada){

			if(pagina_en_memoria->modificada){
				escribir_en_archivo(pagina_en_memoria, numero_de_tabla_primer_nivel);
			}

			void* contenido_archivo = leer_contenido_pagina_archivo(pagina);


			escribir_marco_completo(pagina_en_memoria->marco, contenido_archivo);


			pagina->marco = pagina_en_memoria->marco;
			pagina->usada = true;
			pagina->presencia = true;
			pagina->modificada = false;
			pagina_en_memoria->presencia = false;
			reemplazada = true;

		} else {
			pagina_en_memoria->usada = false;
		}

		if(indice_puntero == marcos_por_proceso - 1){
			indice_puntero = 0;
		} else {
			indice_puntero++;
		}
	}

	pagina_en_memoria = list_get(paginas, indice_puntero);
	actualizar_puntero_proceso(pagina_en_memoria->marco);

}

void actualizar_puntero_proceso(int marco){
	t_puntero* puntero_proceso = encontrar_puntero_proceso();
	puntero_proceso->puntero_marco = marco;
}

void escribir_marco_completo(int marco, void* contenido) {
	memcpy(espacio_de_usuario + (marco * tamanio_pagina), contenido, tamanio_pagina);
}

void algoritmo_clock_modificado(t_pagina* pagina){
	t_list* paginas = encontrar_paginas_en_memoria();
	log_info(logger, "El proceso tiene actualmente %d paginas en memoria", list_size(paginas));
	list_sort(paginas, (void*) mayor_tiempo_espera);

	bool reemplazada = false;
	int indice_puntero = encontrar_indice_puntero(paginas);

	if(indice_puntero == -1) { // si nunca se habia movido el puntero se inicializa en el primer marco
		indice_puntero = 0;
	}

	while(!reemplazada){
		clock_m_paso_1(paginas, pagina, &indice_puntero, &reemplazada);

		if(!reemplazada){
			clock_m_paso_2(paginas, pagina, &indice_puntero, &reemplazada);
		}
	}

	t_pagina* siguiente_pagina_en_memoria = list_get(paginas, indice_puntero);
	actualizar_puntero_proceso(siguiente_pagina_en_memoria->marco);

}

void clock_m_paso_1(t_list* paginas,t_pagina* pagina, int *indice_puntero, bool *reemplazada){

	int i = 0;
	t_pagina* pagina_en_memoria = malloc(sizeof(t_pagina));

	while(i < marcos_por_proceso && !(*reemplazada)){

			pagina_en_memoria = list_get(paginas, *indice_puntero);

			if(!pagina_en_memoria->usada && !pagina_en_memoria->modificada){

				void* contenido_archivo = leer_contenido_pagina_archivo(pagina);

				escribir_marco_completo(pagina_en_memoria->marco, contenido_archivo);

				pagina->marco = pagina_en_memoria->marco;
				pagina->usada = true;
				pagina->presencia = true;
				pagina_en_memoria->presencia = false;
				*reemplazada = true;

			}

			if(*indice_puntero == marcos_por_proceso - 1){
				*indice_puntero = 0;
			} else {
				(*indice_puntero)++;
			}

			i++;
		}
}

void clock_m_paso_2(t_list* paginas,t_pagina* pagina, int *indice_puntero, bool *reemplazada){

	int i = 0;
	t_pagina* pagina_en_memoria;

	while(i < marcos_por_proceso && !(*reemplazada)){

		pagina_en_memoria = list_get(paginas, *indice_puntero);

		if(!pagina_en_memoria->usada && pagina_en_memoria->modificada){

			//uint32_t contenido_pagina_en_memoria = leer_contenido_marco(pagina_en_memoria->marco, 0);
			escribir_en_archivo(pagina_en_memoria, numero_de_tabla_primer_nivel);

			void* contenido_archivo = leer_contenido_pagina_archivo(pagina);

			escribir_marco_completo(pagina_en_memoria->marco, contenido_archivo);
			//log_info(logger, "Se reemplazó la página %d", pagina->numero);

			free(contenido_archivo);

			pagina->marco = pagina_en_memoria->marco;
			pagina->usada = true;
			pagina->presencia = true;
			pagina->modificada = false;
			pagina_en_memoria->presencia = false;
			*reemplazada = true;

		} else {
			pagina_en_memoria->usada = false;
		}

		if(*indice_puntero == marcos_por_proceso - 1){
			*indice_puntero = 0;
		} else {
			(*indice_puntero)++;
		}

		i++;
	}
}

void* leer_contenido_pagina_archivo(t_pagina* pagina){
	int fd = open(get_path_archivo(numero_de_tabla_primer_nivel), O_RDONLY, S_IRUSR | S_IWUSR);
	lseek(fd, tamanio_pagina * pagina->numero, SEEK_SET);
	void* buffer = malloc(tamanio_pagina);
	read(fd, buffer, tamanio_pagina);
	close(fd);
	//for(int i = 0; i < tamanio_pagina / 4; i++) {
		//uint32_t* cont = malloc(sizeof(uint32_t));
		//memcpy(cont, buffer + i * sizeof(uint32_t), sizeof(uint32_t));
		//log_info(logger, "contenido buffer: %d", *cont);
		//free(cont);
	//}
	return buffer;
}

//char* leer_hasta(char caracter_de_paro, FILE* file) {
	//char caracter = fgetc(file);
	//char* cadena_guardada = string_new();
	//while (caracter != caracter_de_paro && caracter != EOF) {
		//string_append_with_format(&cadena_guardada, "%c", caracter);
		//caracter = fgetc(file);
	//}
	//return cadena_guardada;
//}

void escribir_en_archivo(t_pagina* pagina, int id) {
	usleep(retardo_swap * 1000);
	int fd = open(get_path_archivo(id), O_WRONLY, S_IRUSR | S_IWUSR);
	log_info(logger, "File descriptor a escribir/leer: %d", fd);
	log_info(logger, "Tabla segundo nivel: %d", pagina->tabla_segundo_nivel);
	log_info(logger, "Numero de pagina: %d", pagina->numero);
	lseek(fd, tamanio_pagina * pagina->numero, SEEK_SET);
	void* buffer = malloc(tamanio_pagina);
	memcpy(buffer, espacio_de_usuario + (pagina->marco * tamanio_pagina), tamanio_pagina);

	//for(int i = 0; i < tamanio_pagina / 4; i++) {
		//uint32_t* cont = malloc(sizeof(uint32_t));
		//memcpy(cont, buffer + i * sizeof(uint32_t), sizeof(uint32_t));
		//log_info(logger, "contenido buffer: %d", *cont);
		//free(cont);
	//}
	int bytes = write(fd, buffer, tamanio_pagina);
	close(fd);
	log_info(logger, "bytes escritos: %d", bytes);
	free(buffer);
}

int encontrar_indice_puntero(t_list* paginas){
	t_puntero* puntero = encontrar_puntero_proceso();
	return encontrar_indice_puntero_segun_marco(paginas, puntero->puntero_marco);
}

t_puntero* encontrar_puntero_proceso(){
	int i = 0;
	bool encontrada = false;
	t_puntero* puntero_encontrado;

	while( i < list_size(punteros_procesos) && !encontrada){
			t_puntero* puntero_proceso = list_get(punteros_procesos, i);

			if(puntero_proceso->numero_tabla_primer_nivel == numero_de_tabla_primer_nivel){
				puntero_encontrado = puntero_proceso;
				encontrada = true;
			}

			i++;
	}

	return puntero_encontrado;
}

int encontrar_indice_puntero_segun_marco(t_list* paginas,int marco){
	for(int i = 0; i < list_size(paginas); i++){
		t_pagina* pagina = list_get(paginas, i);
		if(pagina->marco == marco){
			return i;
		}
	}

	return -1;
}


t_pagina* mayor_tiempo_espera(t_pagina* p1, t_pagina* p2) {
	if(difftime(p1->tiempo_carga, p2->tiempo_carga) > 0) {
		return p2;
	} else {
		return p1;
	}
}

t_list* encontrar_paginas_en_memoria(){
	t_list* paginas_en_memoria = list_create();

	t_list* tablas_segundo_nivel = list_get(tablas_primer_nivel, numero_de_tabla_primer_nivel);

	for(int i = 0; i < list_size(tablas_segundo_nivel); i++) {

		t_tabla_paginas_segundo_nivel* tabla = list_get(tablas_segundo_nivel, i);
		for(int j = 0; j < list_size(tabla->paginas); j++) {
			t_pagina* pagina = list_get(tabla->paginas, j);

			if(pagina->presencia) {
				list_add(paginas_en_memoria, pagina);
			}
		}
	}

	return paginas_en_memoria;
}

void asignar_marco(t_pagina* pagina){
	pagina->marco = proximo_marco_libre();
	pagina->presencia = true;
	pagina->usada = true;
	void* contenido = leer_contenido_pagina_archivo(pagina);

	escribir_marco_completo(pagina->marco, contenido);

	if(cantidad_marcos_ocupados_proceso() == marcos_por_proceso){
		t_list* paginas = encontrar_paginas_en_memoria();
		list_sort(paginas, (void*)mayor_tiempo_espera);
		t_pagina* primer_pagina = list_get(paginas, 0);

		t_puntero* puntero_proceso = encontrar_puntero_proceso();
		puntero_proceso->puntero_marco = primer_pagina->marco;
	}
	log_warning(logger, "No hizo falta reemplazar, se asignó el marco %d a la página %d", pagina->marco, pagina->numero);
}


int cantidad_marcos_ocupados_proceso(){
	int marcos_ocupados = 0;

	t_list* tablas_segundo_nivel = list_get(tablas_primer_nivel, numero_de_tabla_primer_nivel);

	for(int i = 0; i < list_size(tablas_segundo_nivel); i++) {
		t_tabla_paginas_segundo_nivel* tabla = list_get(tablas_segundo_nivel, i);
		for(int j = 0; j < list_size(tabla->paginas); j++) {
			t_pagina* pagina = list_get(tabla->paginas, j);
			if(pagina->presencia) {
				marcos_ocupados++;
			}
		}
	}
	return marcos_ocupados;
}


void liberar_marcos_proceso(int id) {
	t_list* tablas_segundo_nivel = list_get(tablas_primer_nivel, id);

	for(int i = 0; i < list_size(tablas_segundo_nivel); i++) {
		t_tabla_paginas_segundo_nivel* tabla = list_get(tablas_segundo_nivel, i);
		for(int j = 0; j < list_size(tabla->paginas); j++) {
			t_pagina* pagina = list_get(tabla->paginas, j);
			liberar_marco(pagina->marco);
		}
	}
}

void liberar_marco(int marco) {
	bitarray_clean_bit(marcos_memoria, marco);
}

uint32_t leer_contenido_marco(int numero_de_marco, int desplazamiento) {
	if((numero_de_marco * tamanio_pagina) + desplazamiento > tamanio_memoria - 1) {
		return -1;
	} else {
		uint32_t* contenido = malloc(sizeof(uint32_t));
		memcpy(contenido, espacio_de_usuario + (numero_de_marco * tamanio_pagina) + desplazamiento, sizeof(uint32_t));
		if(contenido != NULL) {
			return *contenido;
		} else {
			return -1;
		}
	}

}

int escribir_en_marco(int numero_de_marco, int desplazamiento, uint32_t valor) {
	if((numero_de_marco * tamanio_pagina) + desplazamiento > tamanio_memoria - 1) {
		log_warning(logger, "Error intentando escribir en el marco, supera limite memoria");
		return -1;
	} else {
		memcpy(espacio_de_usuario + (numero_de_marco * tamanio_pagina) + desplazamiento, &valor, sizeof(uint32_t));
		cambiar_bit_modificado(numero_de_marco);
		return 1;
	}
}

void cambiar_bit_modificado(int numero_de_marco){
	t_pagina* pagina = encontrar_pagina_por_marco(numero_de_marco);
	pagina->modificada = true;

}

t_pagina* encontrar_pagina_por_marco(int numero_de_marco){
	//suponiendo que tengo guardado el pid
	t_list* paginas = encontrar_paginas_en_memoria();
	t_pagina* pagina_encontrada = NULL;

	int i = 0;
	bool encontrada = false;
	t_pagina* pagina_aux;

	while(i < list_size(paginas) && !encontrada){
		pagina_aux = list_get(paginas, i);

		if(pagina_aux->marco == numero_de_marco){
			pagina_encontrada = pagina_aux;
			encontrada = true;
		}

		i++;
	}
	return pagina_encontrada;
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
	//bitarray_clean_bit(marcos_memoria, pagina->marco);
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
	bitarray_destroy(marcos_memoria);
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
				enviar_entero(conexion_cpu, tamanio_pagina);
				enviar_entero(conexion_cpu, entradas_por_tabla);
				pthread_create(&hilo_kernel, NULL, (void *) atender_kernel, NULL);
				break;
			case ACCESO_TABLA_PRIMER_NIVEL:
				log_info(logger, "CPU solicita acceso a tabla pagina de primer nivel");
				numero_de_tabla_primer_nivel = recibir_numero_tabla(conexion_cpu);
				int entrada_tabla_primer_nivel = recibir_numero_entrada(conexion_cpu);
				log_info(logger, "entrada tabla: %d ; numero tabla: %d", entrada_tabla_primer_nivel, numero_de_tabla_primer_nivel);
				t_tabla_paginas_segundo_nivel* tabla = buscar_tabla_segundo_nivel(numero_de_tabla_primer_nivel, entrada_tabla_primer_nivel);
				enviar_numero_de_tabla(conexion_cpu, tabla->numero);

				break;
			case ACCESO_TABLA_SEGUNDO_NIVEL:
				if(numero_de_tabla_primer_nivel == id_a_suspender) {
					log_info(logger, "El proceso esta ejecutando swap, se espera a que termine");
					sem_wait(&swap);
				}
				log_info(logger, "CPU solicita acceso a tabla pagina de segundo nivel");

				int numero_de_tabla_segundo_nivel = recibir_numero_tabla(conexion_cpu);
				int entrada_tabla_segundo_nivel = recibir_numero_entrada(conexion_cpu);
				t_pagina* pagina = buscar_pagina(numero_de_tabla_segundo_nivel, entrada_tabla_segundo_nivel);
				enviar_numero_de_pagina(conexion_cpu, pagina->marco);

				break;
			case LECTURA_MEMORIA_USUARIO:
				if(numero_de_tabla_primer_nivel == id_a_suspender) {
					log_info(logger, "El proceso esta ejecutando swap, se espera a que termine");
					sem_wait(&swap);
				}
				log_info(logger, "CPU solicita lectura a memoria de usuario");
				//verificar_swap();

				int marco_lectura = recibir_entero(conexion_cpu);
				int desplazamiento_lectura = recibir_entero(conexion_cpu);
				uint32_t contenido = leer_contenido_marco(marco_lectura, desplazamiento_lectura);
				log_info(logger, "Se leyó el contenido: %d del marco: %d con desplazamiento: %d", contenido, marco_lectura, desplazamiento_lectura);
				usleep(retardo_memoria*1000);
				enviar_uint32(conexion_cpu, contenido);
				break;
			case ESCRITURA_MEMORIA_USUARIO:
				if(numero_de_tabla_primer_nivel == id_a_suspender) {
					log_info(logger, "El proceso esta ejecutando swap, se espera a que termine");
					sem_wait(&swap);
				}
				log_info(logger, "CPU solicita escritura a memoria de usuario");
				//verificar_swap();

				int marco_escritura = recibir_entero(conexion_cpu);
				int desplazamiento_escritura = recibir_entero(conexion_cpu);
				uint32_t valor = recibir_uint32(conexion_cpu);
				numero_de_tabla_primer_nivel = recibir_numero_tabla(conexion_cpu);
				log_info(logger, "Cpu solicita escribir el valor %d en el marco %d con desplazamiento: %d", valor, marco_escritura, desplazamiento_escritura);
				int respuesta = escribir_en_marco(marco_escritura, desplazamiento_escritura, valor);

				log_info(logger, "Se escribio el contenido: %d en el marco: %d", valor, marco_escritura);
				usleep(retardo_memoria*1000);
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
				log_info(logger, "Kernel solicita INICIO PROCESO");
				t_puntero* puntero_proceso = malloc(sizeof(t_puntero));

				int id_proceso = recibir_id_proceso(conexion_kernel);
				int tamanio = recibir_tamanio(conexion_kernel);
				int numero_tabla_primer_nivel = crear_tabla_paginas();

				puntero_proceso->numero_tabla_primer_nivel = numero_tabla_primer_nivel;
				puntero_proceso->puntero_marco = -1;

				log_info(logger, "Se creo la tabla de primer nivel: %d", numero_tabla_primer_nivel);

				crear_archivo_swap(id_proceso, tamanio);

				list_add(punteros_procesos, puntero_proceso);

				enviar_numero_de_tabla(conexion_kernel, numero_tabla_primer_nivel);

				break;
			case SUSPENCION_PROCESO:
				log_info(logger, "Kernel solicita SUSPENCION PROCESO");
				id_a_suspender = recibir_entero(conexion_kernel);
				log_info(logger, "id: %d", id_a_suspender);

				swapear_paginas_modificadas(id_a_suspender);

				if(numero_de_tabla_primer_nivel == id_a_suspender) {
					sem_post(&swap);
				}

				liberar_marcos_proceso(id_a_suspender);
				id_a_suspender = -1;

				enviar_confirmacion(conexion_kernel);

				break;
			case FINALIZACION_PROCESO:
				log_info(logger, "Kernel solicita FINALIZACION PROCESO");
				int id_a_finalizar = recibir_entero(conexion_kernel);
				liberar_marcos_proceso(id_a_finalizar);
				int fd_a_finalizar = open(get_path_archivo(id_a_finalizar), O_RDWR, S_IRUSR | S_IWUSR);
				close(fd_a_finalizar);
				remove(get_path_archivo(id_a_finalizar));
				log_info(logger, "Id a finalizar: %d", id_a_finalizar);

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
