#include "cpu.h"
#include "./utils/clientServ.h"
//interfaz provisorias
//pthread_t dispatch;

//Hilos
pthread_t interrupt;

//Semaforos
sem_t sem_interrupt;
sem_t dispatch;

//Variables Globales del Proceso
t_config* config;// free(config);
t_log* logger;// free(logger);
t_proceso* process;

//Var principal del proceso
proceso_cpu* cpu;// free(cpu);

//KERNEL-CPU
t_pcb* pcb;//free(pcb);

//Memoria-CPU
t_datos_memoria* datos_memoria;// free(datos_memoria);
uint32_t numero_pagina;
uint32_t entrada_tabla_segundo_nivel;
uint32_t marco;

int main(void) {


	iniciar_cpu();

	cpu->kernel_dispatch = esperar_cliente_dispatch();

	recibir_operacion_dispatch();

	finalizar_cpu();

	return EXIT_SUCCESS;
}

int iniciar_conexion_cpu_memoria(){

	char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");

	char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

	int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

	log_info(logger, " Conectado con Memoria, Socket cliente Memoria: %d", conexion_memoria);

	return conexion_memoria;
}

t_datos_memoria*  handshake_cpu_memoria(){
	enviar_codigo_operacion(cpu->memoria,300);
	log_info(logger, " Hicimos handshake_cpu_memoria");
	return recibir_handshake_memoria(cpu->memoria);
}


void escuchaInterrup(){

	log_info(logger, "   --- Entro: escuchaInterrup(...)");

	//Levantamos el servidor donde resiviremos las interrupciones
	iniciar_servidor_interrupt();

	//Escuchamos que el kernel se conecte al server de interrpciones
	cpu->kernel_interrupt = esperar_cliente_interrupt();
	log_info(logger, " ---- Socket kernel interrupt: %d", cpu->kernel_interrupt);


	//Tiene recibir n veces las peticiones de bloqueo por parte del kernel
	recibir_operacion_interrupt();

	log_info(logger, "     ---- Salio: escuchaInterrup(...)");
	//sem_post(&dispatch);
}

int recibir_operacion_dispatch(){

	log_info(logger, "INICIO: recibir_operaciones()");

	while (1) {
			log_info(logger, "--- A ESPERA DE UNA OPERACION\n");
			operacion cod_op = recibir_operacion(cpu->kernel_dispatch);//Recibe cada peticion que envie el kernel en el puerto interrup

			switch (cod_op) {

				case PCB:

					log_info(logger, " -----------");
					log_info(logger, " El kernel envio un PCB:");
					pcb = recibir_pcb(cpu->kernel_dispatch);

					mostrarPCB();

					int nro_intrucciones = list_size(pcb->instrucciones);

					int nro_instrucion = fetch();

					for(int i = nro_instrucion; i< nro_intrucciones ;i++){

						log_info(logger, "  Nro de instrucion: %d",i);

						char** instruccion = decode(pcb->instrucciones,i);
						char* operacion = instruccion[0];

						log_info(logger, "  instruccion/codigo op: %s", instruccion[0]);

						fetch_operands(instruccion);

						execute(instruccion);

						check_interrupt(operacion);

						if((check_interrupcion(cpu)) || (checkInstruccionInterrupcion(operacion))){
							log_warning(logger,"Entro por interrupcion al Break");
							cpu->interrupcion=false;
							sem_post(&sem_interrupt);
							liberar_pcb(pcb);

							break;
						}
					}

					break;
				case ERROR:
					log_error(logger, "el cliente se desconecto. Terminando servidor para el socket kernel Nro: %d", cpu->kernel_dispatch);
					return EXIT_FAILURE;
				default:
					log_warning(logger,"Operacion desconocida");
					break;
			}
		}

	log_info(logger, "FIN: recibir_operaciones()");
	return EXIT_SUCCESS;
}



int recibir_operacion_interrupt(){

	log_info(logger, "   INICIO: recibir_operacion_interrupt()");

	while (1) {
			log_info(logger, "  --- A ESPERA DE UNA OPERACION\n");
			operacion cod_op = recibir_operacion(cpu->kernel_interrupt);//Recibe cada peticion que envie el kernel en el puerto interrup

			switch (cod_op) {
				case INTERRUPCION:
					log_info(logger, "      --- El kernel envio la operacion: INTERRUPCION");
					cpu->interrupcion=true;//true q pidieron una interrupcion

					//Hago un bloqueo por interrupcion
					log_info(logger, "    --- Este hilo interrupt queda bloqueado hasta que Check Interrupt lo desbloquee");
					sem_wait(&sem_interrupt);

					break;
				case ERROR:
					log_error(logger, "el cliente se desconecto. Terminando servidor para el socket kernel Nro: %d", cpu->kernel_interrupt);
					return EXIT_FAILURE;
				default:
					log_warning(logger,"Operacion desconocida");
					break;
			}
		}
	return EXIT_SUCCESS;
}


void mostrarPCB(){
	log_info(logger, "-----PCB ------");
	log_info(logger, "id: %d", pcb->id);
	log_info(logger, "estimacion: %d", pcb->estimacion_rafaga);
	log_info(logger, "program counter: %d", pcb->program_counter);
	log_info(logger, "socket: %d", pcb->socket_cliente);
	log_info(logger, "numero de tabla: %d", pcb->tablas_paginas);
	log_info(logger, "tamanio de consola: %d", pcb->tamanio_proceso);
	log_info(logger, "lista Instrucciones:");
	//list_iterate(pcb->instrucciones, (void*) iterator);
	log_info(logger, "--------------");
}

void mostrar_PCB_Bloqueado(t_pcb_bloqueado* bloqueado){
//	log_info(cpu_process->logger, "-----PCB ------");
//	log_info(cpu_process->logger, "  estimacion: %d", bloqueado->pcb->estimacion_rafaga);
//	log_info(cpu_process->logger, "  program counter: %d", bloqueado->pcb->program_counter);
//	log_info(cpu_process->logger, "  socket: %d", bloqueado->pcb->socket_cliente);
//	log_info(cpu_process->logger, "  numero de tabla: %d", bloqueado->pcb->tablas_paginas);
//	log_info(cpu_process->logger, "  tamanio de consola: %d", bloqueado->pcb->tamanio_proceso);
	//log_info(cpu_process->logger, "  lista Instrucciones:");
	//list_iterate(pcb->instrucciones, (void*) iterator);
//	log_info(cpu_process->logger, "  tiempo de bloqueo: %d", bloqueado->tiempo_bloqueo);
//	log_info(cpu_process->logger, "--------------");
}

bool checkInstruccionInterrupcion(char* instruccion){
	return (strcmp(instruccion, "I/O")==0) || (strcmp(instruccion, "EXIT")==0);
}

char** decode(t_list* instrucciones,int nro_inst){
	return string_split(list_get(instrucciones,nro_inst)," ");
}
void execute(char** instruccion){

	char* instrucc = instruccion[0];

	if(strcmp("NO_OP",instrucc) == 0){

		int time = config_get_int_value(config, "RETARDO_NOOP");
		log_info(logger, "  Time: %d", time);
		no_op(time);

	}else if(strcmp("I/O",instrucc)==0){

		i_o(atoi(instruccion[1]));

	}else if(strcmp("EXIT",instrucc)==0){

		instruccion_exit();

	}else if(strcmp("COPY",instrucc)==0){

	}else if(strcmp("READ",instrucc)==0){

	}else if(strcmp("WRITE",instrucc)==0){

	}else{
		log_info(logger, "   NO TENIA INSTRUCCIONES EL PCB");
	}
}

int fetch(){
	return pcb->program_counter;
}
void fetch_operands(char** instruccion){
	log_info(logger, "  Entro a fetch_operands");
}
void no_op(int tiempo){
	usleep(1000*tiempo);
	incrementarProgramCounter();
}
void i_o(int tiempo){
	log_info(logger, "   Llego a i_o()");
	incrementarProgramCounter();
	responsePorBloqueo(tiempo);
}
void instruccion_exit(){
	incrementarProgramCounter();
	responsePorFinDeProceso();
}

void incrementarProgramCounter(){
	 pcb->program_counter = pcb->program_counter + 1;
}

void responsePorBloqueo(int tiempo){
	log_info(logger, "   Llego responsePorBloqueo()");
	t_pcb_bloqueado* bloqueado = malloc(sizeof(t_pcb_bloqueado));
	log_info(logger, "   Se le asignoMemoria:");
	bloqueado->pcb = pcb;
	bloqueado->tiempo_bloqueo = tiempo;
	enviar_pcb_bloqueado(cpu->kernel_dispatch,bloqueado);
	log_info(logger, "   Enviamos el PCB Bloqueado ");
	//mostrar_PCB_Bloqueado(cpu,bloqueado);

	free(bloqueado);
}
void responsePorFinDeProceso(){
	log_info(logger, "   responsePorFinDeProceso-Enviamos este PCB: ");
	mostrarPCB(cpu,pcb);
	enviar_pcb(pcb,cpu->kernel_dispatch,FINALIZACION_PROCESO);
}

void liberar_pcb(t_pcb* pcb){

	list_destroy(pcb->instrucciones);
	free(pcb);
}

void  check_interrupt(char* operacion){

	if((check_interrupcion(cpu)) && (!checkInstruccionInterrupcion(operacion))){
		log_info(logger, "   Ocurrio la interrupcion por puerto Interrup: 8005");

		//Volvemos a dejarlo en falso, para que inicie el ciclo
		responseInterrupcion(pcb,cpu);
	}
	//Desbloqueamos el hilo de Interrupt, asi puede seguir recibiendo peticiones de interrupcion
}

bool check_interrupcion(){
	return cpu->interrupcion;
}

void responseInterrupcion(){
	log_info(logger, "   responseInterrupcion/Enviamos este PCB: ");
	//mostrarPCB(cpu,pcb);
	enviar_pcb(pcb,cpu->kernel_dispatch,INTERRUPCION);
}

void iniciar_cpu(){

	cpu = cpu_create();
		cpu->interrupcion = false;

	iniciar_logger_cpu();

	iniciar_config_cpu();

	iniciar_config_semaforos();

	// Crear conexiones
	cpu->memoria = iniciar_conexion_cpu_memoria();
	datos_memoria = handshake_cpu_memoria();

	log_info(logger,"--- Nro de filas tabla nivel 1: %d",datos_memoria->nro_filas_tabla_nivel1);//Entrada = 1 Fila : Nro de filas tabla nivel 1 Nros de filas = Cantidad de entradas
	log_info(logger,"--- Tamano de pagina: %d",datos_memoria->tamano_pagina);

	//primer_acceso_memoria(8);

	iniciar_servidor_dispatch();


	//5) Ejecutamos el Hilo de interrupcion
	if(0 != pthread_create(&interrupt, NULL, (void*) escuchaInterrup,NULL)){
		log_info(logger,"theread de interrupcion no fue creado");
		exit(1);
	}
	log_info(logger,"Se creo el Hilo: interrupt");
}

void iniciar_logger_cpu(){
	// Iniciar logs
	logger = iniciar_logger();

	log_info(logger,"\n###### INICIO DE LOGGER ######");
}
void iniciar_config_cpu(){// Leer el archivo de Configuraciones
	config = iniciar_config();
	log_info(logger,"   ---Leyo archivo de configuraciones");
}

void iniciar_config_semaforos(){
	//Iniciar los Semaforos
	sem_init(&sem_interrupt,0,0);
	log_info(logger,"   ---Asigno a los semaforos con valores iniciales");
}
proceso_cpu* cpu_create(void){
	proceso_cpu* cpu_process = malloc(sizeof(proceso_cpu));
	return cpu_process;
}

t_proceso* process_create(void){
	t_proceso* process = malloc(sizeof(t_proceso));
	return process;
}

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



//##### FUNCIONES SERVER PARA CPU #######
//void recibir_mensaje_kernel(int socket_kernel){
//	t_pcb* pcb_process = malloc(sizeof(t_pcb));
//	pcb = pcb_process;//recibir_mensaje(socket_kernel);
//}

int esperar_cliente_dispatch(){

	log_info(logger, "Entro: esperar_cliente_cpu");

	log_info(logger, " Socket server recibido: %i",cpu->servidor_dispatch);

	log_info(logger, " Queda BLOQUEADO este hilo hasta que se conecte un cliente");

	int kernel_dispatch = esperar_cliente(cpu->servidor_dispatch);//BLOQUEANTE

	log_info(logger, " Termino el BLOQUEO");

	if (kernel_dispatch<0) {
			log_error(logger, " Error de Conexion del cliente, socket nro: %d, puerto tipo: %s",kernel_dispatch," dispatch");
			log_destroy(logger);
			exit(1);
	}
	log_info(logger, " Se conecto un cliente y se creo el socket_cliente: %d. Puerto tipo: %s",kernel_dispatch, " dispatch");

	return kernel_dispatch;
}

int esperar_cliente_interrupt(){

	log_info(logger, "Entro: esperar_cliente_cpu");

	log_info(logger, " Socket server recibido: %i",cpu->servidor_interrupt);

	log_info(logger, " Queda BLOQUEADO este hilo hasta que se conecte un cliente");

	int kernel_interrupt = esperar_cliente(cpu->servidor_interrupt);//BLOQUEANTE

	log_info(logger, " Termino el BLOQUEO");

	if (kernel_interrupt<0) {
			log_error(logger, " Error de Conexion del cliente, socket nro: %d, puerto tipo: %s",kernel_interrupt," interrupt");
			log_destroy(logger);
			exit(1);
	}
	log_info(logger, " Se conecto un cliente y se creo el socket_cliente: %d. Puerto tipo: %s",kernel_interrupt, " interrupt");

	return kernel_interrupt;
}

void iniciar_servidor_dispatch(){

	log_info(logger, "Entro: iniciar_servidor_dispatch");

	cpu->servidor_dispatch= iniciar_servidor_cpu(KEY_PUERTO_DISPATCH);
}

void iniciar_servidor_interrupt(){

	log_info(logger, "Entro: iniciar_servidor_interrupt");

	cpu->servidor_interrupt = iniciar_servidor_cpu(KEY_PUERTO_INTERRUPT);
}

//
int iniciar_servidor_cpu(char* key_puerto){

	log_info(logger, "Entro: iniciar_servidor_cpu");

	char* ip = config_get_string_value(config, KEY_IP_CPU);
	log_info(logger, " Se creara un socket_servidor para la IP: %s", ip);

	char* puerto = config_get_string_value(config, key_puerto);
	log_info(logger, " Se creara un socket_servidor en el PUERTO: %s", puerto);

	int socket_servidor = iniciar_servidor(ip,puerto);

	if( socket_servidor < 0){
		log_info(logger, " ERROR: NO SE CREO EL SOCKET SERVIDOR");
		exit(1);
	}
	log_info(logger, " Se creo el socket_servidor:  %i, listo para escuchar al cliente", socket_servidor);

	return socket_servidor;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}


t_datos_memoria* recibir_handshake_memoria(int conexion) {
	// orden en el que vienen: operacion, id, socket, tamanio, program_counter, estimacion_rafaga, numero_tabla, tamanio_instrucciones, instrucciones
	t_datos_memoria* datos_memoria = malloc(sizeof(t_datos_memoria));
	//int operacion = recibir_entero(conexion);
	datos_memoria->tamano_pagina = recibir_entero(conexion);
	datos_memoria->nro_filas_tabla_nivel1 = recibir_entero(conexion);
	return datos_memoria;
}


void accesos_memoria(){

	//

	//

	//
}

/*
 * CALCULOS
número_página = floor(dirección_lógica / tamaño_página)
entrada_tabla_1er_nivel = floor(número_página / cant_entradas_por_tabla)
entrada_tabla_2do_nivel = número_página mod (cant_entradas_por_tabla)
desplazamiento = dirección_lógica - número_página * tamaño_página
 * */


/*. Un primer acceso para conocer en qué tabla de páginas de 2do nivel está direccionado el
marco en que se encuentra la página a la que queremos acceder
PRIMER ACCESO
CPU --> MEMORIA
CPU ENVIA NUMERO DE TABLA (PCB) Y ENTRADA_TABLA_PRIMER_NIVEL (CALCULA VER ENUNCIADO)
//Entrada = 1 Fila : Nro de filas tabla nivel 1 Nros de filas = Cantidad de entradas
// Procesos Cantidad de paginas(proceso)  = (Nros de filas) al  cuadrado
 * */

uint32_t primer_acceso_memoria(int direccion_logica){

	float calculo1 = direccion_logica/datos_memoria->tamano_pagina;

	numero_pagina = floor(calculo1);//=tamano de marco

	float calculo2 = numero_pagina*12/datos_memoria->nro_filas_tabla_nivel1;

	uint32_t entrada_tabla_1er_nivel = floor(calculo2);

	uint32_t nro_entrada_tabla = 5;//pcb->tablas_paginas;

	enviar_primer_acceso_memoria(cpu->memoria,nro_entrada_tabla, entrada_tabla_1er_nivel);
	log_info(logger, " ------ Envie a MEMORIA nro_entrada_tabla: %d y entrada_tabla_1er_nivel: %i",nro_entrada_tabla,entrada_tabla_1er_nivel);

	entrada_tabla_segundo_nivel = recibir_uint32_t(cpu->memoria);
	log_info(logger, " ------ Recibi de MEMORIA entrada_tabla_segundo_nivel: %i",entrada_tabla_segundo_nivel);


	return entrada_tabla_segundo_nivel;
}



/* Un segundo acceso para conocer en qué marco está la misma
 * SEGUNDO ACCESO
CPU --> MEMORIA
NUMERO DE TABLA SEGUNDO NIVEL (EL MISMO QUE SE RESPONDIO) Y  ENTRADA_TABLA_SEGUNDO_NIVEL  (CALCULA VER ENUNCIADO)

entrada_tabla_2do_nivel = número_página mod (cant_entradas_por_tabla)
desplazamiento = dirección_lógica - número_página * tamaño_página
 *
	 * */
void segundo_acceso_memoria(){

	int entrada_tabla_2do_nivel = numero_pagina % datos_memoria->nro_filas_tabla_nivel1;

	enviar_segundo_acceso_memoria(cpu->memoria,entrada_tabla_segundo_nivel, entrada_tabla_2do_nivel);

	marco = recibir_uint32_t(cpu->memoria);
}

void tercer_acceso_memoria(){
	/*Finalmente acceder a la porción de memoria correspondiente (la dirección física)
		 * */
}








