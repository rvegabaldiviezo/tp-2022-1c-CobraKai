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



int main(void) {

	proceso_cpu* cpu = cpu_create();

	iniciar_cpu(cpu);

	int socket_kernel_dispatch = esperar_cliente_dispatch(cpu);

	recibir_operaciones(cpu, socket_kernel_dispatch);

	finalizar_cpu(cpu);

	return EXIT_SUCCESS;
}

void iniciar_conexion_cpu_memoria(proceso_cpu* cpu_process){

	char* ip_memoria = config_get_string_value(cpu_process->config, "IP_MEMORIA");

	char* puerto_memoria = config_get_string_value(cpu_process->config, "PUERTO_MEMORIA");

	cpu_process->conexion_con_memoria = crear_conexion(ip_memoria, puerto_memoria);

	log_info(cpu_process->logger, " Conectado con Memoria, Socket cliente Memoria: %d", cpu_process->conexion_con_memoria);
}

void escuchaInterrup(proceso_cpu* cpu_process){

	log_info(cpu_process->logger, "   --- Entro: escuchaInterrup(...)");

	//Levantamos el servidor donde resiviremos las interrupciones
	iniciar_servidor_interrupt(cpu_process);

	//Escuchamos que el kernel se conecte al server de interrpciones
	int socket_kernel_interrupt = esperar_cliente_interrupt(cpu_process); log_info(cpu_process->logger, " ---- Socket kernel interrupt: %d", socket_kernel_interrupt);

	//Tiene recibir n veces las peticiones de bloqueo por parte del kernel
	recibir_operaciones(cpu_process, socket_kernel_interrupt);

	log_info(cpu_process->logger, "     ---- Salio: escuchaInterrup(...)");
	//sem_post(&dispatch);
}

int recibir_operaciones(proceso_cpu* cpu_process, int socket_kernel){

	log_info(cpu_process->logger, "INICIO: recibir_operaciones()");

	while (1) {
			log_info(cpu_process->logger, "--- A ESPERA DE UNA OPERACION\n");
			operacion cod_op = recibir_operacion(socket_kernel);//Recibe cada peticion que envie el kernel en el puerto interrup

			switch (cod_op) {

				case PCB:

					log_info(cpu_process->logger, " -----------");
					log_info(cpu_process->logger, " El kernel envio un PCB:");
					t_pcb* pcb = recibir_pcb(socket_kernel);

					mostrarPCB(cpu_process,pcb);

					int nro_intrucciones = list_size(pcb->instrucciones);

					int nro_instrucion = fetch(pcb);

					for(int i = nro_instrucion; i< nro_intrucciones ;i++){

						log_info(cpu_process->logger, "  Nro de instrucion: %d",i);

						char** instruccion = decode(pcb->instrucciones,i);
						char* operacion = instruccion[0];

						log_info(cpu_process->logger, "  instruccion/codigo op: %s", instruccion[0]);

						fetch_operands(cpu_process,instruccion);

						execute(cpu_process,pcb, instruccion);

						check_interrupt(cpu_process,pcb,operacion);

						if((check_interrupcion(cpu_process)) || (checkInstruccionInterrupcion(operacion))){
							cpu_process->interrupcion=false;
							sem_post(&sem_interrupt);
							break;
						}

					}

					break;
				case INTERRUPCION:
					//log_info(cpu_process->logger, "      --- El kernel envio la operacion: INTERRUPCION");
					cpu_process->interrupcion=true;//true q pidieron una interrupcion

					//Hago un bloqueo por interrupcion
					//log_info(cpu_process->logger, "    --- Este hilo interrupt queda bloqueado hasta que Check Interrupt lo de desbloquee");
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


void mostrarPCB(proceso_cpu* cpu_process,t_pcb* pcb){
	//log_info(cpu_process->logger, "-----PCB ------");
	//log_info(cpu_process->logger, "id: %d", pcb->id);
	//log_info(cpu_process->logger, "estimacion: %d", pcb->estimacion_rafaga);
	//log_info(cpu_process->logger, "program counter: %d", pcb->program_counter);
	//log_info(cpu_process->logger, "socket: %d", pcb->socket_cliente);
	//log_info(cpu_process->logger, "numero de tabla: %d", pcb->tablas_paginas);
//	log_info(cpu_process->logger, "tamanio de consola: %d", pcb->tamanio_proceso);
	//log_info(cpu_process->logger, "lista Instrucciones:");
	//list_iterate(pcb->instrucciones, (void*) iterator);
//	log_info(cpu_process->logger, "--------------");
}

void mostrar_PCB_Bloqueado(proceso_cpu* cpu_process,t_pcb_bloqueado* bloqueado){
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
void execute(proceso_cpu* cpu, t_pcb* pcb,char** instruccion){

	char* instrucc = instruccion[0];

	if(strcmp("NO_OP",instrucc) == 0){

		int time = config_get_int_value(cpu->config, "RETARDO_NOOP");
		log_info(cpu->logger, "  Time: %d", time);
		no_op(time,pcb);

	}else if(strcmp("I/O",instrucc)==0){

		i_o(cpu,pcb,atoi(instruccion[1]));
		t_log* nuevo_logger = log_create("./cpu2.log",NAME_LOG,true,LOG_LEVEL_INFO);
		log_info(nuevo_logger, "    Ocurrio la interrupcion por instruccion: %s", instrucc);
		log_destroy(nuevo_logger);

	}else if(strcmp("EXIT",instrucc)==0){

		instruccion_exit(cpu,pcb);
		t_log* nuevo_logger = log_create("./cpu3.log",true,LOG_LEVEL_INFO);
		log_info(nuevo_logger, "    Ocurrio la interrupcion por instruccion: %s", instrucc);
		log_destroy(nuevo_logger);

	}else if(strcmp("COPY",instrucc)==0){

	}else if(strcmp("READ",instrucc)==0){

	}else if(strcmp("WRITE",instrucc)==0){

	}else{
		log_info(cpu->logger, "   NO TENIA INSTRUCCIONES EL PCB");
	}
}

int fetch(t_pcb* pcb){
	return pcb->program_counter;
}
void fetch_operands(proceso_cpu* cpu,char** instruccion){
	//log_info(cpu->logger, "  Entro a fetch_operands");
}
void no_op(int tiempo,t_pcb* pcb){
	usleep(1000*tiempo);
	incrementarProgramCounter(pcb);
}
void i_o(proceso_cpu* cpu, t_pcb* pcb,int tiempo){
	log_info(cpu->logger, "   Llego a i_o()");
	incrementarProgramCounter(pcb);
	responsePorBloqueo(cpu,pcb,tiempo);
}
void instruccion_exit(proceso_cpu* cpu, t_pcb* pcb){
	incrementarProgramCounter(pcb);
	responsePorFinDeProceso(pcb,cpu);
}

void incrementarProgramCounter(t_pcb* pcb){
	 pcb->program_counter = pcb->program_counter + 1;
}

void responsePorBloqueo(proceso_cpu* cpu,t_pcb* pcb,int tiempo){
	//log_info(cpu->logger, "   Llego responsePorBloqueo()");
	t_pcb_bloqueado* bloqueado = malloc(sizeof(t_pcb_bloqueado));
	bloqueado->pcb = pcb;
	bloqueado->tiempo_bloqueo = tiempo;
	enviar_pcb_bloqueado(cpu->conexion_con_kernel,bloqueado);
	//log_info(cpu->logger, "   responsePorBloqueo-Enviamos este PCB: ");
	//mostrar_PCB_Bloqueado(cpu,bloqueado);
}
void responsePorFinDeProceso(t_pcb* pcb,proceso_cpu* cpu){
	//log_info(cpu->logger, "   responsePorFinDeProceso-Enviamos este PCB: ");
	mostrarPCB(cpu,pcb);
	enviar_pcb(pcb,cpu->conexion_con_kernel,FINALIZACION_PROCESO);
}

void  check_interrupt(proceso_cpu* cpu,t_pcb* pcb,char* operacion){

	if((check_interrupcion(cpu)) && (!checkInstruccionInterrupcion(operacion))){
		//log_info(cpu->logger, "   Ocurrio la interrupcion por puerto Interrup: 8005");

		//Volvemos a dejarlo en falso, para que inicie el ciclo
		responseInterrupcion(pcb,cpu);
	}
	//Desbloqueamos el hilo de Interrupt, asi puede seguir recibiendo peticiones de interrupcion
}

bool check_interrupcion(proceso_cpu* cpu){
	return cpu->interrupcion;
}

void responseInterrupcion(t_pcb* pcb,proceso_cpu* cpu){
	//log_info(cpu->logger, "   responseInterrupcion/Enviamos este PCB: ");
	//mostrarPCB(cpu,pcb);
	enviar_pcb(pcb,cpu->conexion_con_kernel,INTERRUPCION);
}

proceso_cpu* iniciar_cpu(proceso_cpu* cpu_process)
{	// Crea un espacio de memoria para: process
	cpu_process->process = process_create();
	cpu_process->interrupcion = false; // en ppio no hay interrupcion (false: 0).

	// Iniciar logs
	cpu_process->logger = iniciar_logger(); log_info(cpu_process->logger,"\n###### INICIO DE LOGGER ######");
	log_info(cpu_process->logger, "Entro: iniciar_cpu");

	// Leer el archivo de Configuraciones
	cpu_process->config = iniciar_config();
	log_info(cpu_process->logger," Lee archivo de configuraciones");

	//Iniciar los Semaforos
	sem_init(&sem_interrupt,0,0);
	//sem_init(&dispatch,0,0);
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



//##### FUNCIONES SERVER PARA CPU #######
void recibir_mensaje_kernel(proceso_cpu* cpu_process, int socket_kernel){
	t_pcb* pcb_process = malloc(sizeof(t_pcb));
	cpu_process->process->pcb = pcb_process;//recibir_mensaje(socket_kernel);
}



int esperar_cliente_dispatch(proceso_cpu* cpu_process){

	char* tipo_server =  "dispatch";

	log_info(cpu_process->logger, "Entro: esperar_cliente_dispatch");

	cpu_process->conexion_con_kernel = esperar_cliente_cpu(cpu_process, cpu_process->socket_servidor_dispatch, tipo_server);

	return cpu_process->conexion_con_kernel;
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





