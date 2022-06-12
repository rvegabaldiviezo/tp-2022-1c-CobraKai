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
int conexion_kernel;


int main(void) {

	proceso_cpu* cpu = iniciar_cpu();
	puts("SALIO: iniciar_cpu() ");

	iniciar_servidor_dispatch(cpu);

	int socket_kernel = esperar_cliente_dispatch(cpu);
	log_info(cpu->logger,"socket kernel: %i",socket_kernel);

	finalizar_cpu(cpu);

	return EXIT_SUCCESS;
}

void escuchaInterrup(){

	//pthread_mutex_lock(&mutex);

	puts("escuchaInterrup: Esta bloqueado el interrup ");

	sem_wait(&sem_interrupt);
	//log_info(proceso->logger,"escuchaInterrup: antes del bloqueo");
	//cpu.process.interrupcion = 1;

	//printf("salio del bloqueo, el atributo bloqueo es: %i \n", cpu.process.interrupcion);

	//log_info(pssroceso->logger,"escuchaInterrup: salio del bloqueo, se cambio la var bloqueo a 1");
	//sem_post(&dispatch);

	//pthread_mutex_lock(&mutex);
	puts("escuchaInterrup: Se bloquea el dispatch ");
	sem_post(&dispatch);

	//log_info(cpu.logger,"escuchaInterrup: Termino de ejecutar la funcion");
}

proceso_cpu* iniciar_cpu(void)
{	puts("ENTRO: iniciar_cpu() ");
	proceso_cpu* cpu_process = cpu_create();
	//t_paquete* paquete = malloc(sizeof(t_paquete));
	// Iniciar logs
	t_log* log = iniciar_logger();
	log_info(log,"\n###### INICIO DE LOGGER ######");
	log_info(log, "Entro: iniciar_cpu");

	// Leer el archivo de Configuraciones
	t_config* conf = iniciar_config();
	log_info(log," Lee archivo de configuraciones");


	/*
	//3) Asignamos los valores iniciales
	t_proceso nuevo_proceso;
	nuevo_proceso.interrupcion = 0; //en ppio es falsa la interrupcion
	cpu.process = nuevo_proceso;
	log_info(cpu.logger,"Asigno valores iniciales");

	//4) Inicio los Semaforos
	sem_init(&sem_interrupt,0,0);
	sem_init(&dispatch,0,0);
	log_info(cpu.logger,"Asigno a los semaforos con valores iniciales");

	//5) Inicio lo Hilos
	if(0 != pthread_create(&interrupt, NULL, (void*) escuchaInterrup, NULL)){//Rutina: la funcion q le pasamos
		log_info(cpu.logger,"theread de interrupcion no fue creado");
		exit(1);
	}
	log_info(cpu.logger,"Se crearon los hilos necesarios");
	*/
	//log_info(cpu_process.logger,"-- Finalizo: iniciar_cpu --");
	cpu_process->logger = log;
	cpu_process->config = conf;

	return cpu_process;
}

proceso_cpu* cpu_create(void){
	proceso_cpu* cpu_process = malloc(sizeof(proceso_cpu));
	return cpu_process;
}


void finalizar_cpu(proceso_cpu* cpu_process){

	log_info(cpu_process->logger, "Entro: finalizar_cpu");

	config_destroy(cpu_process->config);
	//log_info(cpu_process.logger, "Ejecuto config_destroy");
	liberar_conexion(cpu_process->socket_servidor_dispatch);
	//liberar_conexion(cpu.conexion_con_memoria);
	//liberar_conexion(cpu.conexion_con_kernel);
	log_info(cpu_process->logger, "\n###### FIN LOGGER ######");

	log_destroy(cpu_process->logger);
}


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

int esperar_cliente_dispatch(proceso_cpu* cpu_process){
	char* tipo_server =  "dispatch";
	log_info(cpu_process->logger, "Entro: esperar_cliente_dispatch");
	log_info(cpu_process->logger, "socket_servidor %s: %i",tipo_server, cpu_process->socket_servidor_dispatch);
	return esperar_cliente_cpu(cpu_process, cpu_process->socket_servidor_dispatch, tipo_server);
}

int esperar_cliente_interrupt(proceso_cpu* cpu_process){

	log_info(cpu_process->logger, "Entro: esperar_cliente_interrupt");
	log_info(cpu_process->logger, "socket server: %i",cpu_process->socket_servidor_dispatch);
	return esperar_cliente_cpu(cpu_process, cpu_process->socket_servidor_interrupt, "interrupt");
}

int esperar_cliente_cpu(proceso_cpu* cpu_process, int socket_server, char* tipo_puerto){

	log_info(cpu_process->logger, "Entro: esperar_cliente_cpu");

	log_info(cpu_process->logger, "socket server recibido: %i",socket_server);

	log_info(cpu_process->logger, " Queda BLOQUEADO este hilo hasta que se conecte un cliente");

	int socket_cliente = -1;//esperar_cliente(socket_server);//BLOQUEANTE

	log_info(cpu_process->logger, "salio: socket_cliente");

	if (socket_cliente<0) {
			log_error(cpu_process->logger, " Error de Conexion del cliente, socket nro: %d, puerto tipo: %s",socket_cliente,tipo_puerto);
			log_destroy(cpu_process->logger);
			exit(1);
	}
	log_info(cpu_process->logger, " Se conecto un cliente y se creo el socket_cliente: %d. Puerto tipo: %s",socket_cliente, tipo_puerto);

	return socket_cliente;
}

void iniciar_servidor_dispatch(proceso_cpu* cpu_process){
	puts("ENTRO: iniciar_servidor_dispatch ");
	log_info(cpu_process->logger, "Entro: iniciar_servidor_dispatch");

	cpu_process->socket_servidor_dispatch = iniciar_servidor_cpu(cpu_process, KEY_PUERTO_DISPATCH);
}

void iniciar_servidor_interrupt(proceso_cpu* cpu_process){

	log_info(cpu_process->logger, "Entro: iniciar_servidor_interrupt");

	cpu_process->socket_servidor_interrupt = iniciar_servidor_cpu(cpu_process, KEY_PUERTO_DISPATCH);
}

//
int iniciar_servidor_cpu(proceso_cpu* cpu_process, char* key_puerto){
	puts("ENTRO: iniciar_servidor_cpu ");
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

