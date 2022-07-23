#include "cpu.h"
//#include "./utils/clientServ.h"

// HILOS
pthread_t interrupt;

//SEMAFOS
sem_t sem_interrupt;
sem_t dispatch;

//LOGGER Y CONFIG
t_log* logger;// free(logger);
t_config* config; // free(config);

// CPU
proceso_cpu* cpu;// free(cpu);
//t_proceso* process;

// KERNEL
t_pcb* pcb;//free(pcb);
char** instruccion_con_parametros;
char* instruccion;

// MEMORIA
uint32_t entradas_tlb;
char* reemplazo_tlb;
t_datos_memoria* datos_memoria;// free(datos_memoria);
uint32_t numero_pagina;
uint32_t nro_tabla_segundo_nivel;

// COPY
int valor_lectura_origen;
int valor_lectura_destino;
//TLB
t_queue* cola_tlb;//free(cola_tlb)

char* path_config;


int main(int argc, char** argv) {
	path_config = string_new();
	string_append(&path_config, argv[1]);

	iniciar_cpu();

	iniciar_memoria();

	iniciar_interrupt();

	iniciar_dispatch();

	finalizar_cpu();

	return EXIT_SUCCESS;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ INTERRUP ++++++++++++++++++++++++++++++++++++++++++++++++++++++
void iniciar_interrupt(){
	if(0 != pthread_create(&interrupt, NULL, (void*) escuchaInterrup,NULL)){
		log_info(logger," INTERRUPT, theread de interrupcion no fue creado");
		exit(1);
	}
	log_info(logger," INTERRUPT, SE CREO EL HILO");
}
// --- AUX ---
void escuchaInterrup(){

	iniciar_servidor_interrupt();

	esperar_cliente_interrupt();

	recibir_operacion_interrupt();
}
int recibir_operacion_interrupt(){

	while (1) {
			log_info(logger, "### INTERRUPT, EN ESPERA DE PETICIONES DEL KERNEL\n");
			operacion cod_op = recibir_operacion(cpu->kernel_interrupt);

			switch (cod_op) {
				case INTERRUPCION:
					log_info(logger, " INTERRUPT, KERNEL PIDIO LA INTERRUPCION DEL CICLO DE EJECUCION DE INSTRUCCIONES DE PROCESO");
					cpu->interrupcion=true;//true xq pidieron una interrupcion

					log_info(logger, " INTERRUPT, EL HILO QUEDA BLOQUEADO");
					//Hago un bloqueo del hilo de interrupt, hasta que Check Interrupt lo desbloquee.
					sem_wait(&sem_interrupt);

					break;
				case ERROR:
					log_error(logger, " INTERRUPT, EL KERNEL SE DESCONECTO");
					return EXIT_FAILURE;
				default:
					log_warning(logger,"INTERRUPT, Operacion desconocida");
					break;
			}
		}
	return EXIT_SUCCESS;
}
//##################################################################


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ INICIAR DISPATCH ++++++++++++++++++++++++++++++++++++++++++++++
void iniciar_dispatch(){

	iniciar_servidor_dispatch();

	esperar_cliente_dispatch();

	recibir_operacion_dispatch();
}
// --- AUX ---
int recibir_operacion_dispatch(){

	while (1) {
		log_info(logger, " ### DISPATCH, EN ESPERA DE PETICIONES DEL KERNEL\n");
		operacion cod_op = recibir_operacion(cpu->kernel_dispatch);

		switch (cod_op) {
			case PCB:
				log_info(logger, " DISPATCH, RECIBIO UN PCB");
				pcb = recibir_pcb(cpu->kernel_dispatch);
				pcb->inicio_rafaga = time(NULL);

				mostrarPCB();

				int nro_intrucciones = list_size(pcb->instrucciones);

				int nro_instrucion = fetch();

				for(int pos = nro_instrucion; pos< nro_intrucciones ;pos++){

					log_info(logger, " DISPATCH, NRO INSTRUCCION: %d",pos);

					instruccion_con_parametros = decode(pcb->instrucciones,pos);
					instruccion = instruccion_con_parametros[0];

					log_info(logger, " DISPATCH, INSTRUCCION: %s", instruccion);

					fetch_operands();

					execute();

					check_interrupt();

					if((check_interrupcion()) || (checkInstruccionInterrupcion())){
						log_warning(logger," DISPATCH, RETORNAMOS EL PCB AL KERNEL");
						cpu->interrupcion=false;
						sem_post(&sem_interrupt);
						reiniciar_tlb();
						break;
					}
				}
				break;
			case ERROR:
				log_error(logger, " DISPATCH, EL KERNEL SE DESCONECTO");
				return EXIT_FAILURE;
			default:
				log_warning(logger," OPERACION DESCONOCIDA");
				break;
		}
	}
	return EXIT_SUCCESS;
}

char** decode(t_list* instrucciones,int nro_inst){
	return string_split(list_get(instrucciones,nro_inst)," ");
}
void reiniciar_tlb(){
	queue_destroy_and_destroy_elements(cola_tlb,(void*)free);
	log_info(logger, " DISPATCH, REINICIAMOS LA COLA DE LA TLB");
	iniciar_tlb();
}

//##################################################################


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ CICLO DE INSTRUCCIONES ++++++++++++++++++++++++++++++++++++++++++++
// 1) fetch
int fetch(){
	return pcb->program_counter;
}
// 2) fetch_operands
void fetch_operands(){

	if(strcmp("COPY",instruccion)==0){

		int direccion_logica_origen = atoi(instruccion_con_parametros[2]);//Accedemos al segundo parametro

		valor_lectura_origen =  ejecutar_instruccion_read(direccion_logica_origen);//lectura direccion fisica del valor de Origen a COPIAR
	}
}


// 3) execute
void execute(){

	incrementarProgramCounter();

	if(strcmp("NO_OP",instruccion) == 0){
		int tiempo_retardo = config_get_int_value(config, "RETARDO_NOOP");
		usleep(1000*tiempo_retardo);

	}else if(strcmp("READ",instruccion)==0){
		int direccion_logica = atoi(instruccion_con_parametros[1]);
		uint32_t valor_leido_memoria = ejecutar_instruccion_read(direccion_logica);
		log_info(logger, " READ, Leyo de memoria: %d", valor_leido_memoria);

	}else if(strcmp("WRITE",instruccion)==0){
		int direccion_logica = atoi(instruccion_con_parametros[1]);
		uint32_t valor_a_escribir = atoi(instruccion_con_parametros[2]);

		ejecutar_instruccion_write(direccion_logica,valor_a_escribir);

	}else if(strcmp("COPY",instruccion)==0){
		int direccion_logica_destino = atoi(instruccion_con_parametros[1]);

		ejecutar_instruccion_write(direccion_logica_destino,valor_lectura_origen);

	}else if(strcmp("I/O",instruccion)==0){
		int tiempo_bloqueado = atoi(instruccion_con_parametros[1]);
		t_pcb_bloqueado* pcb_bloqueado = malloc(sizeof(t_pcb_bloqueado));
			pcb_bloqueado->pcb = pcb;
			pcb_bloqueado->tiempo_bloqueo = tiempo_bloqueado;
		enviar_pcb_bloqueado(cpu->kernel_dispatch,pcb_bloqueado);

	}else if(strcmp("EXIT",instruccion)==0){
		enviar_pcb(pcb,cpu->kernel_dispatch,FINALIZACION_PROCESO);
	}else{

	}
}

uint32_t ejecutar_instruccion_read(int direccion_logica){
	//Buscamos la instruccion logica
	int nro_pag = nro_pagina(direccion_logica);

	//Buscamos si existe en la TLB
	t_tlb* entrada_tlb = obtener_TLB(nro_pag);

	if(entrada_tlb != NULL){//Existe en la TLB
		entrada_tlb->timestamps = time(NULL);//ACTUALIZAMOS SU TIMESTUP.
		log_info(logger, " READ, EN TLB: se encontró la pag: %d y el marco: %d en la TLB", entrada_tlb->nro_pagina, entrada_tlb->nro_marco);
		return tercer_acceso_memoria_lectura(direccion_logica, entrada_tlb->nro_pagina, entrada_tlb->nro_marco);
	} else {//NO EXISTE, BUscamos en memoria
		primer_acceso_memoria(nro_pag);
		uint32_t marco_a_leer = segundo_acceso_memoria(nro_pag);
		guardar_en_TLB(nro_pag, marco_a_leer);
		return tercer_acceso_memoria_lectura(direccion_logica, nro_pag, marco_a_leer);
	}
}

void ejecutar_instruccion_write(int direccion_logica,uint32_t valor_a_escribir){

	int nro_pag = nro_pagina(direccion_logica);

	t_tlb* entrada_tlb = obtener_TLB(nro_pag);


	if(entrada_tlb != NULL) {
		entrada_tlb->timestamps = time(NULL);//ACTUALIZAMOS SU TIMESTUP.
		log_info(logger, " WRITE, EN TLB: se encontró la pag: %d y el marco: %d en la TLB", entrada_tlb->nro_pagina, entrada_tlb->nro_marco);
		log_info(logger, " WRITE: se va a grabar en memoria: %d", valor_a_escribir);
		tercer_acceso_memoria_escritura(direccion_logica, entrada_tlb->nro_pagina, entrada_tlb->nro_marco, valor_a_escribir);
	} else {
		primer_acceso_memoria(nro_pag);
		uint32_t marco_a_escribir = segundo_acceso_memoria(nro_pag);
		log_info(logger, " WRITE: marco a escribir: %d; valor a escribir: %d", marco_a_escribir, valor_a_escribir);
		guardar_en_TLB(nro_pag, marco_a_escribir);
		tercer_acceso_memoria_escritura(direccion_logica, nro_pag, marco_a_escribir, valor_a_escribir);
	}
}


// 4) check_interrupt
void  check_interrupt(){

	if((check_interrupcion()) && (!checkInstruccionInterrupcion())){
		log_info(logger, " DISPATCH, INTERRUPCION SOLICITADA POR KERNEL");

		//Volvemos a dejarlo en falso, para que inicie el ciclo
		enviar_pcb(pcb,cpu->kernel_dispatch,INTERRUPCION);
	}
	//Desbloqueamos el hilo de Interrupt, asi puede seguir recibiendo peticiones de interrupcion
}
// --- AUX ---
bool checkInstruccionInterrupcion(){
	return (strcmp(instruccion, "I/O")==0) || (strcmp(instruccion, "EXIT")==0);
}
bool check_interrupcion(){
	return cpu->interrupcion;
}
void incrementarProgramCounter(){
	 pcb->program_counter = pcb->program_counter + 1;
}
//##############################################


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ MEMORIA TLB ++++++++++++++++++++++++++++++++++++++++++++
uint32_t nro_pagina(int direccion_logica_instruccion){

	float calculo_del_nro_pagina = direccion_logica_instruccion/datos_memoria->tamano_pagina;

	uint32_t nro_pagina_resultante = floor(calculo_del_nro_pagina);//=tamano de marco

	return nro_pagina_resultante;
}
t_tlb* obtener_TLB(uint32_t numero_pagina_buscada){

	t_tlb* tlb_buscada = NULL;

	if(!queue_is_empty(cola_tlb)){//Si no esta vacia la cola, buscamos.

		bool _tlb_buscada(t_tlb* elemento_tlb) {
			return elemento_tlb->nro_pagina == numero_pagina_buscada;
		}
		tlb_buscada = list_find(cola_tlb->elements,(void*) _tlb_buscada);
	}
	return tlb_buscada;
}

void primer_acceso_memoria(int numero_pagina){

	float calculo_entrada_tabla_1er_nivel = numero_pagina / datos_memoria->nro_filas_tabla_nivel1;// param2::dato

	uint32_t entrada_tabla_1er_nivel = floor(calculo_entrada_tabla_1er_nivel);

	uint32_t nro_entrada_tabla = pcb->tablas_paginas;// param1::dato

	enviar_primer_acceso_memoria(cpu->memoria,nro_entrada_tabla, entrada_tabla_1er_nivel);
	log_info(logger, " DISPATCH, envie a MEMORIA nro_entrada_tabla: %d y entrada_tabla_1er_nivel: %i",nro_entrada_tabla,entrada_tabla_1er_nivel);

	nro_tabla_segundo_nivel = recibir_uint32_t(cpu->memoria);
	log_info(logger, " DISPATCH, recibi de MEMORIA entrada_tabla_segundo_nivel: %i",nro_tabla_segundo_nivel);
}

uint32_t segundo_acceso_memoria(int numero_pagina) {

	int entrada_tabla_2do_nivel = numero_pagina % datos_memoria->nro_filas_tabla_nivel1;

	enviar_segundo_acceso_memoria(cpu->memoria,nro_tabla_segundo_nivel, entrada_tabla_2do_nivel);

	uint32_t marco_recibido = recibir_uint32_t(cpu->memoria);

	return marco_recibido;
}

//uint32_t leer_valor_en_memoria(){
//	return tercer_acceso_memoria_lectura();
//}

uint32_t tercer_acceso_memoria_lectura(int direccion_logica, int numero_pagina, int marco){

	int desplazamiento = direccion_logica - numero_pagina * datos_memoria->tamano_pagina;

	enviar_tercer_acceso_memoria_lectura(cpu->memoria,marco,desplazamiento);

	int valor_lectura = recibir_uint32_t(cpu->memoria);

	return valor_lectura;
}

void guardar_en_TLB(uint32_t numero_pagina,uint32_t nro_marco){

    if(esta_completa_cola_TLB()){//APLICAR ALGORITMOS DE REEMPLAZO DE TLB

        if(strcmp(reemplazo_tlb,"LRU") == 0){//ALGORITMO FIFO
            //Ordenamos de menor tiempo a mayor (Asi queda primero. El de menor uso tiene menor tiempo, los de mayor uso tienen un tiempo mayor, mas actual)
            bool _tlb_menor(t_tlb* tlb1, t_tlb* tlb2) {
                return tlb1->timestamps <= tlb2->timestamps;
            }
             list_sort(cola_tlb->elements, (void*)_tlb_menor);
        }

         t_tlb* primer_tlb = queue_peek(cola_tlb);
         primer_tlb->nro_pagina = numero_pagina;
         primer_tlb->nro_marco = nro_marco;
         primer_tlb->timestamps = time(NULL);//lo usamos principalmente para LRU

    }else{//La cola esta incompleta, agregamos un elemnto

        //Instanciamos el elemento de la TLB a guardar
        t_tlb* element_tlb = malloc(sizeof(t_tlb));
        element_tlb->nro_pagina = numero_pagina;
        element_tlb->nro_marco = nro_marco;
        element_tlb->timestamps = time(NULL);//lo usamos principalmente para LRU

        //Agregamos un nuevo elemento a la TLB, al final de cola
        queue_push(cola_tlb,element_tlb);
    }
}

bool esta_completa_cola_TLB(){
	return queue_size(cola_tlb)>entradas_tlb - 1;
}

void tercer_acceso_memoria_escritura(int direccion_logica, int numero_pagina, int marco, uint32_t valor_escribir){

	int desplazamiento = direccion_logica - numero_pagina * datos_memoria->tamano_pagina;

	enviar_tercer_acceso_memoria_escritura(cpu->memoria,marco,desplazamiento,valor_escribir, pcb->id);

	int respuesta = recibir_entero(cpu->memoria);

	if(respuesta == -1) {
		log_error(logger, "No se pudo escribir");
	}
}
//##############################################################

void mostrarPCB(){
	log_info(logger, "-----PCB ------");
	log_info(logger, "id: %d", pcb->id);
	log_info(logger, "estimacion: %d", pcb->estimacion_rafaga);
	log_info(logger, "program counter: %d", pcb->program_counter);
	log_info(logger, "socket: %d", pcb->socket_cliente);
	log_info(logger, "numero de tabla: %d", pcb->tablas_paginas);
	log_info(logger, "tamanio de consola: %d", pcb->tamanio_proceso);
	log_info(logger, "lista Instrucciones:");
	list_iterate(pcb->instrucciones, (void*) iterator);
	log_info(logger, "--------------");
}
void iterator(char* value) {
	log_info(logger,"%s", value);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ Iniciar CPU +++++++++++++++++++++++++++++++++++++
void iniciar_cpu(){
	cpu = cpu_create();
	cpu->interrupcion = false;

	iniciar_logger_cpu();

	iniciar_config_cpu();

	iniciar_config_semaforos();
}
// --- AUX ---
proceso_cpu* cpu_create(void){
	proceso_cpu* cpu_process = malloc(sizeof(proceso_cpu));
	return cpu_process;
}
void iniciar_logger_cpu(){
	logger = iniciar_logger();
	log_info(logger,"\n###### INICIO DE LOGGER ######");
}
void iniciar_config_cpu(){
	config = iniciar_config(path_config);

	reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
	log_info(logger, "  REEMPLAZO_TLB: %s", reemplazo_tlb);

	entradas_tlb = config_get_int_value(config, "ENTRADAS_TLB");
	log_info(logger, "  ENTRADAS_TLB: %d", entradas_tlb);
}
void iniciar_config_semaforos(){
	//Iniciar los Semaforos
	sem_init(&sem_interrupt,0,0);
}
//#########################################################




//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ Iniciar Memoria +++++++++++++++++++++++++++++++++
void iniciar_memoria(){

	iniciar_comunicacion_cpu_memoria();

	iniciar_tlb();
}
//AUX
void iniciar_comunicacion_cpu_memoria(){
	iniciar_conexion_cpu_memoria();
	handshake_cpu_memoria();
	log_info(logger," --- MEMORIA, Nro de filas tabla nivel 1: %d",datos_memoria->nro_filas_tabla_nivel1);//Entrada = 1 Fila : Nro de filas tabla nivel 1 Nros de filas = Cantidad de entradas
	log_info(logger," --- MEMORIA, Tamanio de pagina: %d",datos_memoria->tamano_pagina);
}
void iniciar_conexion_cpu_memoria(){

	char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");

	char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

	int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

	log_info(logger, " MEMORIA, CPU conectada con memoria, socket: %d", conexion_memoria);

	cpu->memoria =  conexion_memoria;
}
void  handshake_cpu_memoria(){
	enviar_codigo_operacion(cpu->memoria,300);
	log_info(logger, " MEMORIA, handshake con memoria");
	datos_memoria = recibir_handshake_memoria(cpu->memoria);
}
t_datos_memoria* recibir_handshake_memoria(int conexion) {
	t_datos_memoria* datos_memoria = malloc(sizeof(t_datos_memoria));
	datos_memoria->tamano_pagina = recibir_entero(conexion);
	datos_memoria->nro_filas_tabla_nivel1 = recibir_entero(conexion);
	return datos_memoria;
}
void iniciar_tlb(){
	cola_tlb = queue_create();//free(cola_tlb)
	log_info(logger, " MEMORIA, inicio la TLB");
}
//########################################################################



//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ FINALIZAR CPU +++++++++++++++++++++++++++++++++++++
void finalizar_cpu(){

	log_info(logger, "Entro: finalizar_cpu");
	//Hilos
	pthread_join(interrupt, NULL);
		log_info(logger,"Finalizamos el Hilo interrupt");
	//Config
	config_destroy(config);
	liberar_conexion(cpu->kernel_dispatch);
	liberar_conexion(cpu->kernel_interrupt);
	liberar_conexion(cpu->servidor_dispatch);
	liberar_conexion(cpu->servidor_interrupt);
	liberar_conexion(cpu->memoria);
	log_info(logger, "\n###### FIN LOGGER ######");
	log_destroy(logger);
}
// --- AUX ---
t_proceso* process_create(void){
	t_proceso* process = malloc(sizeof(t_proceso));
	return process;
}
//################################################################

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ INICIAR SERVIDORES PARA RECIBIR CLIENTES +++++++++++++++++++++
void iniciar_servidor_dispatch(){

	log_info(logger, " DISPATCH, LEVANTANDO SERVIDOR");

	cpu->servidor_dispatch= iniciar_servidor_cpu(KEY_PUERTO_DISPATCH);
}
void iniciar_servidor_interrupt(){

	log_info(logger, " INTERRUPT, LEVANTANDO SERVIDOR");

	cpu->servidor_interrupt = iniciar_servidor_cpu(KEY_PUERTO_INTERRUPT);
}
int iniciar_servidor_cpu(char* key_puerto){

	char* ip = config_get_string_value(config, KEY_IP_CPU);
	char* puerto = config_get_string_value(config, key_puerto);

	int socket_servidor = iniciar_servidor(ip,puerto);

	if( socket_servidor < 0){
		log_info(logger, "   ERROR: NO SE CREO EL SOCKET SERVIDOR");
		exit(1);
	}
	log_info(logger, "   SERVIDOR LEVANTADO, nro_socket:  %i. Puede recibir clientes", socket_servidor);

	return socket_servidor;
}
//##########################################################

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ ESCUCHAR CLIENTE ++++++++++++++++++++++++++++++
void esperar_cliente_dispatch(){

	log_info(logger, " DISPATCH, BLOQUEADO A ESPERA DE QUE SE CONECTEN");

	int kernel_dispatch = esperar_cliente(cpu->servidor_dispatch);//BLOQUEANTE

	if (kernel_dispatch<0) {
			log_error(logger, " DISPATCH, Error de Conexion del cliente, socket nro: %d",kernel_dispatch);
			log_destroy(logger);
			exit(1);
	}
	log_info(logger, " DISPATCH, CLIENTE CONECTADO, socket_cliente: %d",kernel_dispatch);

	cpu->kernel_dispatch = kernel_dispatch;
}
void esperar_cliente_interrupt(){

	log_info(logger, " INTERRUPT, BLOQUEADO A ESPERA DE QUE SE CONECTEN");

	int kernel_interrupt = esperar_cliente(cpu->servidor_interrupt);//BLOQUEANTE

	if (kernel_interrupt<0) {
			log_error(logger, " INTERRUPT, Error de Conexion del cliente, socket nro: %d",kernel_interrupt);
			log_destroy(logger);
			exit(1);
	}
	log_info(logger, " INTERRUPT, CLIENTE CONECTADO, socket: %d",kernel_interrupt);

	cpu->kernel_interrupt = kernel_interrupt;
}
//############################################################
