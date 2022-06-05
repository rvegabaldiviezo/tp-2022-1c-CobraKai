#include "cpu.h"
//interfaz provisorias
//pthread_t dispatch;
pthread_t interrupt;
sem_t sem_interrupt;
sem_t dispatch;

t_proceso proceso;

int main(void) {

	// 1) Inciar las configuraciones: Loggear y Archivo de configuraciones
	proceso_init();

		// 1.1) Inicio los semaforos

		// Semaforo para bloquear las interrupciones que llegan
		sem_init(&sem_interrupt,0,0);
		sem_init(&dispatch,0,0);

		// 1.2) Inicio los hilos

		//Hilo para tener abierta la conecion que escuche el bloqueo
		pthread_create(&interrupt, NULL, (void*) escuchaInterrup, NULL);


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
	puts("main: antes");
	sem_post(&sem_interrupt);
	sem_wait(&dispatch);
	puts("main: despues");
	//escuchaInterrup();
	//sem_wait(&dispatch);

	//printf("proceso interrucion (despues): %i \n",proceso->interrucion);

	//pthread_join(interrupt, NULL);


	log_info(proceso.logger,"main: Termino de ejecutar");
	puts("main: FIN PROGRAMA");

	return EXIT_SUCCESS;
}

void escuchaInterrup(){

	puts("escuchaInterrup: antes del bloqueo");
	//log_info(proceso->logger,"escuchaInterrup: antes del bloqueo");
	sem_wait(&sem_interrupt);
	//proceso->interrucion = 1;
	puts("escuchaInterrup: salio del bloqueo, se cambio en el proceso el atributo bloqueo a 1");
	//log_info(proceso->logger,"escuchaInterrup: salio del bloqueo, se cambio la var bloqueo a 1");
	//sem_post(&dispatch);
	log_info(proceso.logger,"escuchaInterrup: Termino de ejecutar la funcion");
	sem_post(&dispatch);
}

void proceso_init(){

	// 1)  Iniciar/Crear logs
	proceso.logger  = iniciar_logger();
	log_info(proceso.logger," Inicio el Logger");

	// 2) Leer el archivo de Configuraciones

	log_info(proceso.logger,"proceso_init(): 2-Leyo archivo de configuraciones");

	//3) Asignamos los valores iniciales

	log_info(proceso.logger,"proceso_init(): 3-Asigno valores iniciales a las variables");
}




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


