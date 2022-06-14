#include "cpu.h"
#include "./utils/clientServ.h"
//interfaz provisorias
//pthread_t dispatch;

//Hilos
pthread_t interrupt;
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


//Semaforos
sem_t sem_interrupt;
sem_t dispatch;

//Variables Globales del Proceso
//proceso_cpu* cpu;
t_proceso proceso;
//int conexion_kernel;
int interrup_kernel;



int main(void) {

	proceso_cpu* cpu = cpu_create();

	iniciar_cpu(cpu);

	int socket_kernel_dispatch = esperar_cliente_dispatch(cpu);

	recibir_operaciones(cpu, socket_kernel_dispatch);


	//sem_wait(&dispatch);
	if( cpu->process->interrupcion > 0){
		log_info(cpu->logger, "   El kernel pidio INTERRUPCION");
		//hago algo, habilito que de nuevo vuelva a escuchar por interrupcion
		sem_post(&sem_interrupt);
		sem_wait(&dispatch);
		//	printf("proceso interrucion (en mutex): %i \n", cpu.process.interrupcion);
	}else{
		log_info(cpu->logger, "   El pcb tiene un id de proceso: %d", cpu->process->pcb->id);
	}

		//puts("main: despues del desbloqueo");
		//escuchaInterrup();
		//sem_wait(&dispatch);


	finalizar_cpu(cpu);

	return EXIT_SUCCESS;
}

void iniciar_conexion_cpu_memoria(proceso_cpu* cpu_process){
	log_info(cpu_process->logger, "Entro: iniciar_conexion_cpu_memoria");
	//Desarrollar que la cpu se conecte con la memoria
}

void escuchaInterrup(proceso_cpu* cpu_process){

	log_info(cpu_process->logger, "Entro: escuchaInterrup(...)");

	//Levantamos el servidor donde resiviremos las interrupciones
	iniciar_servidor_interrupt(cpu_process);

	//Escuchamos que el kernel se conecte al server de interrpciones
	int socket_kernel_interrupt = esperar_cliente_interrupt(cpu_process); log_info(cpu_process->logger, " Socket kernel interrupt: %d", socket_kernel_interrupt);

	//Tiene recibir n veces las peticiones de bloqueo por parte del kernel
	recibir_operaciones(cpu_process, socket_kernel_interrupt);

	log_info(cpu_process->logger, "Salio: escuchaInterrup(...)");
	sem_post(&dispatch);
}

int recibir_operaciones(proceso_cpu* cpu_process, int socket_kernel){

	log_info(cpu_process->logger, "INICIO: recibir_operaciones()");

	while (1) {
			int cod_op = recibir_operacion(socket_kernel);//Recibe cada peticion que envie el kernel en el puerto interrup

			switch (cod_op) {

				case PCB:
					log_info(cpu_process->logger, " El kernel envio un PCB");
					t_pcb* pcb = recibir_pcb(socket_kernel);

					log_info(cpu_process->logger, "id: %d\n", pcb->id);
					log_info(cpu_process->logger, "estimacion: %d\n", pcb->estimacion_rafaga);
					log_info(cpu_process->logger, "program counter: %d\n", pcb->program_counter);
					log_info(cpu_process->logger, "socket: %d\n", pcb->socket_cliente);
					log_info(cpu_process->logger, "numero de tabla: %d\n", pcb->tablas_paginas);
					log_info(cpu_process->logger, "tamanio de consola: %d\n", pcb->tamanio_proceso);
					log_info(cpu_process->logger, "lista:\n");
					list_iterate(pcb->instrucciones, (void*) iterator);
					break;
				case INTERRUPCION:
					log_info(cpu_process->logger, " El kernel envio la operacion: INTERRUPCION");
					cpu_process->process->interrupcion=1;//true q pidieron una interrupcion

					//Desbloque en hilo principal de

					//Hago un bloqueo por interrupcion
					log_info(cpu_process->logger, "  Este hilo interrupt queda bloqueado hasta que Check Interrupt lo de desbloquee");
					sem_wait(&sem_interrupt);

					break;
				case ERROR:
					log_error(cpu_process->logger, "el cliente se desconecto. Terminando servidor para el socket kernel Nro: %d", socket_kernel);
					return EXIT_FAILURE;
				default:
					log_warning(cpu_process->logger,"Operacion desconocida");
					break;
			}
		}

	log_info(cpu_process->logger, "FIN: recibir_operaciones()");
	return EXIT_SUCCESS;
}




proceso_cpu* iniciar_cpu(proceso_cpu* cpu_process)
{	// Crea un espacio de memoria para: process
	cpu_process->process = process_create();
	cpu_process->process->interrupcion = 0; // en ppio no hay interrupcion (false: 0).

	// Iniciar logs
	cpu_process->logger = iniciar_logger(); log_info(cpu_process->logger,"\n###### INICIO DE LOGGER ######");
	log_info(cpu_process->logger, "Entro: iniciar_cpu");

	// Leer el archivo de Configuraciones
	cpu_process->config = iniciar_config();
	log_info(cpu_process->logger," Lee archivo de configuraciones");

	//Iniciar los Semaforos
	sem_init(&sem_interrupt,0,0);
	sem_init(&dispatch,0,0);
	log_info(cpu_process->logger,"Asigno a los semaforos con valores iniciales");

	// Crear conexiones
	iniciar_conexion_cpu_memoria(cpu_process);

	iniciar_servidor_dispatch(cpu_process);


	//5) Ejecutamos el Hilo de interrupcion
	if(0 != pthread_create(&interrupt, NULL, (void*) escuchaInterrup, cpu_process)){
		log_info(cpu_process->logger,"theread de interrupcion no fue creado");
		exit(1);
	}
	log_info(cpu_process->logger,"Se creo el Hilo: interrupt");

	return cpu_process;
}

proceso_cpu* cpu_create(void){
	proceso_cpu* cpu_process = malloc(sizeof(proceso_cpu));
	return cpu_process;
}

t_proceso* process_create(void){
	t_proceso* process = malloc(sizeof(t_proceso));
	return process;
}

void finalizar_cpu(proceso_cpu* cpu_process){

	log_info(cpu_process->logger, "Entro: finalizar_cpu");

	//Hilos
	pthread_join(interrupt, NULL); log_info(cpu_process->logger,"Finalizamos el Hilo interrupt");

	//Config
	config_destroy(cpu_process->config);
	//log_info(cpu_process.logger, "Ejecuto config_destroy");
	liberar_conexion(cpu_process->socket_servidor_dispatch);
	//liberar_conexion(cpu.conexion_con_memoria);
	//liberar_conexion(cpu.conexion_con_kernel);
	log_info(cpu_process->logger, "\n###### FIN LOGGER ######");

	log_destroy(cpu_process->logger);
}

/*
void atender_kernel_dispatch(proceso_cpu* cpu_process,int conexion_kernel){

	log_info(cpu_process->logger, "Entro: atender_kernel_dispatch");

	while(1){

		int operacion_kernel = recibir_operacion(conexion_kernel);

		switch(operacion_kernel) {

			case PCB:
				log_info(cpu_process->logger, " Me llego un pcb desde el kernel");
				break;

			case ERROR:
				log_error(cpu_process->logger, " El kernel se desconectó inesperadamente");
				break;

			default:
				log_info(cpu_process->logger, " Operacion desconocida");
				break;
		}
	}
}
*/

/*
 * 1) levantar la conecciones cliente servidor
 * 2)
 *
 *
 * */


/*
 *
int fetch(t_proceso proceso){
	//Proxima instruccion a ejecutar
	return proceso.pcb.program_counter;
}
void fetch_operands(t_proceso proces, t_instruction instruccion){
}
t_instruction decode(t_proceso proceso,int nro_intruccion){
	t_instruction instruction;
	//Busca en las lista la instruccion a ejecutar:
	return  instruction;
}
void execute(t_proceso proceso, t_instruction instruccion){

	switch(instruccion.id) {
		case NO_OP://NO_OP
			int time = config_get_string_value(proceso.config, "RETARDO_NOOP");//
			no_op(time,instruccion.params[1]);
			break;
		case I_O://I/O
			i_o(proceso,instruccion.params[1]);
			//return EXIT_FAILURE;
			break;
		case EXIT://EXIT
			exit(proceso);
			break;
		case COPY://COPY
			break;
		case READ://READ
			break;
		case WRITE://WRITE
			break;
		default:
			//log_info(logger, "Operacion desconocida");
			break;
	}


}

void no_op(int tiempo,int repeticiones){
	usleep(1000*tiempo*repeticiones);
}
void i_o(t_proceso proceso, int tiempo){
	incrementarpcb(proceso);
	responsePorBloqueo(proceso,tiempo);
}
void exit(t_proceso proceso){
	incrementarpcb(proceso);
	responsePorFinDeProceso(proceso);
}
void  check_interrupt(t_proceso proceso){

	incrementarpcb(proceso);

	if(proceso.interrucion){
		//responderle al kernel
		responseInterrupcion(proceso);
	}
}

void responseInterrupcion(t_proceso proceso){
	int socket = proceso.socket;
}
void responsePorBloqueo(t_proceso proceso,int tiempo){
	//
}
void responsePorFinDeProceso(t_proceso proceso){
	//
}
void incrementarpcb(t_proceso proceso){
	 proceso.pcb.id = proceso.pcb.id + 1;
}
int size_instrucciones(t_proceso proceso){
	return proceso.pcb.instrucciones->elements_count;
}

*/

/*
void config(){

	t_config* config = config_create(PATH_CONFIG);
	char* puerto = config_get_string_value(config, conf_puerto);
	char* ip = config_get_string_value(config, conf_ip);
}*/



/*
 * void log_s(char* titulo_log, char* valor_log){
	char * mensaje_log = string_new();
	string_append(&mensaje_log, titulo_log);
	string_append(&mensaje_log, valor_log);
	log_info(proceso.logger,"%s", mensaje_log);
}
void log_i(char* titulo_log, int valor_log){
	char * mensaje_log = string_new();
	string_append(&mensaje_log, titulo_log);
	string_append(&mensaje_log, string_itoa(valor_log));
	log_info(proceso.logger,"%s",mensaje_log);
}

void loggear(char* mensajelog){
	log_info(proceso->logger,"%s",mensajelog);
}



 * */







//##### FUNCIONES SERVER PARA CPU #######
void recibir_mensaje_kernel(proceso_cpu* cpu_process, int socket_kernel){
	t_pcb* pcb_process = malloc(sizeof(t_pcb));
	cpu_process->process->pcb = pcb_process;//recibir_mensaje(socket_kernel);
}



int esperar_cliente_dispatch(proceso_cpu* cpu_process){

	char* tipo_server =  "dispatch";

	log_info(cpu_process->logger, "Entro: esperar_cliente_dispatch");

	return esperar_cliente_cpu(cpu_process, cpu_process->socket_servidor_dispatch, tipo_server);
}

int esperar_cliente_interrupt(proceso_cpu* cpu_process){

	char* tipo_server =  "interrupt";

	log_info(cpu_process->logger, "Entro: esperar_cliente_interrupt");

	return esperar_cliente_cpu(cpu_process, cpu_process->socket_servidor_interrupt, tipo_server);
}

int esperar_cliente_cpu(proceso_cpu* cpu_process, int socket_server, char* tipo_puerto){

	log_info(cpu_process->logger, "Entro: esperar_cliente_cpu");

	log_info(cpu_process->logger, " Socket server recibido: %i",socket_server);

	log_info(cpu_process->logger, " Queda BLOQUEADO este hilo hasta que se conecte un cliente");

	int socket_cliente = esperar_cliente(socket_server);//BLOQUEANTE

	log_info(cpu_process->logger, " Termino el BLOQUEO");

	if (socket_cliente<0) {
			log_error(cpu_process->logger, " Error de Conexion del cliente, socket nro: %d, puerto tipo: %s",socket_cliente,tipo_puerto);
			log_destroy(cpu_process->logger);
			exit(1);
	}
	log_info(cpu_process->logger, " Se conecto un cliente y se creo el socket_cliente: %d. Puerto tipo: %s",socket_cliente, tipo_puerto);

	return socket_cliente;
}

void iniciar_servidor_dispatch(proceso_cpu* cpu_process){

	log_info(cpu_process->logger, "Entro: iniciar_servidor_dispatch");

	cpu_process->socket_servidor_dispatch = iniciar_servidor_cpu(cpu_process, KEY_PUERTO_DISPATCH);
}

void iniciar_servidor_interrupt(proceso_cpu* cpu_process){

	log_info(cpu_process->logger, "Entro: iniciar_servidor_interrupt");

	cpu_process->socket_servidor_interrupt = iniciar_servidor_cpu(cpu_process, KEY_PUERTO_INTERRUPT);
}

//
int iniciar_servidor_cpu(proceso_cpu* cpu_process, char* key_puerto){

	log_info(cpu_process->logger, "Entro: iniciar_servidor_cpu");

	char* ip = config_get_string_value(cpu_process->config, KEY_IP_CPU);
	log_info(cpu_process->logger, " Se creara un socket_servidor para la IP: %s", ip);

	char* puerto = config_get_string_value(cpu_process->config, key_puerto);
	log_info(cpu_process->logger, " Se creara un socket_servidor en el PUERTO: %s", puerto);

	int socket_servidor = iniciar_servidor(ip,puerto);

	if( socket_servidor < 0){
		log_info(cpu_process->logger, " ERROR: NO SE CREO EL SOCKET SERVIDOR");
		exit(1);
	}
	log_info(cpu_process->logger, " Se creo el socket_servidor:  %i, listo para escuchar al cliente", socket_servidor);

	return socket_servidor;
}

void iterator(char* value) {
	t_log* logger = log_create(PATH_LOG, "CPU", true, LOG_LEVEL_INFO);
	log_info(logger,"%s", value);
}




//char* tipo(){

//}


//int main(void) {

	// 1) Inciar las configuraciones: Loggear y Archivo de configuraciones
//	iniciar_cpu(cpu);

	//iniciar_servidor_dispatch(cpu);

	//conexion_kernel = esperar_cliente_dispatch(cpu);

	//atender_kernel(cpu, conexion_kernel);
	/*
	t_proceso proceso = iniciar_cpu();
	   iniciar logear, 	proceso.interrucion=0;

	//Hilo interrup: iniciar_conexion()y  escuchar(proceso_interrup)

	//Hilo Proceso de las instrucciones: iniciar_conexion()y  escuchar(proceso_intrucciones)

	// instanciar_proceso_instrucciones(&proceso,sockect_cliente)

	int nro_intrucciones = size_instrucciones(proceso);
	int nro_instrucion = fetch(proceso);

	for(int i = nro_instrucion; i< nro_intrucciones ;i++){

		nro_instrucion = fetch(proceso);

		t_instruction instruccion = decode(proceso,nro_instrucion);

		fetch_operands(instruccion);

		execute(proceso, instruccion);

		check_interrupt(proceso);
	}
	*/
	//printf("proceso interrucion (antes): %i \n",proceso->interrucion);
	//puts("main: antes del desbloque de interrup");
	//printf("proceso interrucion (antes): %i \n", cpu.process.interrupcion);


	//sem_post(&sem_interrupt);

	//Bloqueo en hilo principal
	//sem_wait(&dispatch);
	//if(0!=cpu.process.interrupcion){
	//	printf("proceso interrucion (en mutex): %i \n", cpu.process.interrupcion);
	//}
	//puts("main: despues del desbloqueo");
	//escuchaInterrup();
	//sem_wait(&dispatch);

	//printf("proceso interrucion (despues): %i \n",proceso->interrucion);

	//pthread_join(interrupt, NULL);


	//log_info(cpu->logger,"main: Termino de ejecutar");

	//finalizar_cpu(cpu);

	//puts("main: FIN PROGRAMA");

	//return EXIT_SUCCESS;
//}






//pthread_mutex_lock(&mutex);

//sem_wait(&sem_interrupt);
//log_info(proceso->logger,"escuchaInterrup: antes del bloqueo");
//cpu.process.interrupcion = 1;

//printf("salio del bloqueo, el atributo bloqueo es: %i \n", cpu.process.interrupcion);

//log_info(pssroceso->logger,"escuchaInterrup: salio del bloqueo, se cambio la var bloqueo a 1");
//sem_post(&dispatch);

//pthread_mutex_lock(&mutex);
//puts("escuchaInterrup: Se bloquea el dispatch ");
//sem_post(&dispatch);

//log_info(cpu.logger,"escuchaInterrup: Termino de ejecutar la funcion");


