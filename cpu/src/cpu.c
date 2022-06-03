#include <stdio.h>
#include <stdlib.h>
#include "cpu-3.h"


int interrupcion = 0;//false


int main(void) {
	t_proceso proceso;
	proceso.interrucion=0;

	//Hilo interrup: iniciar_conexion()y  escuchar(proceso_interrup)

	//Hilo Proceso de las instrucciones: iniciar_conexion()y  escuchar(proceso_intrucciones)

	// instanciar_proceso_instrucciones(&proceso,sockect_cliente)

	int nro_intrucciones = size_instrucciones(proceso);

	for(int i = 0; i< nro_intrucciones ;i++){

		int nro_instrucion = fetch(proceso);

		t_instruction instruccion = decode(proceso,nro_instrucion);

		fetch_operands(instruccion);

		execute(proceso, instruccion);

		check_interrupt(proceso);
	}

	return EXIT_SUCCESS;
}

int fetch(t_proceso proceso){
	//Proxima instruccion a ejecutar
	return proceso.pcb.program_counter;
}
t_instruction decode(t_proceso proceso,int nro_intruccion){
	t_instruction instruction;
	//Busca en las lista la instruccion a ejecutar:

	return  instruction;
}

void fetch_operands(t_proceso proces, t_instruction instruccion){

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



/*
void config(){

	t_config* config = config_create(PATH_CONFIG);
	char* puerto = config_get_string_value(config, conf_puerto);
	char* ip = config_get_string_value(config, conf_ip);
}*/



