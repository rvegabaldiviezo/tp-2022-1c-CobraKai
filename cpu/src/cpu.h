#ifndef CPU_H_
#define CPU_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <assert.h>
#include <signal.h>
#include <netdb.h>
// Librerias para Hilos y Semaforos
#include <pthread.h>
#include <semaphore.h>
// Commons que permite la catedra
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
// Include de utils
#include "./utils/clientServ.h"

//###### Rutas de Archivos ######
#define PATH_CONFIG "./cpu.config"
#define PATH_LOG "./cpu.log"
#define NAME_LOG "CPU"

//###### DEFINICIONES #####
#define KEY_IP_CPU "IP_CPU"
#define KEY_PUERTO_DISPATCH "PUERTO_ESCUCHA_DISPATCH"
#define KEY_PUERTO_INTERRUPT "PUERTO_ESCUCHA_INTERRUPT"


//###### ESTRUCTURAS ######
typedef struct instruction {// cadena nro nro ->  "candena", "nro",
    char* id;
    t_list* params;
} t_instruction;

typedef struct {
	int id;
	unsigned int tamanio_proceso;
	unsigned int program_counter;
	unsigned int tablas_paginas;
	unsigned int estimacion_rafaga;
	int socket_cliente;
	t_list* instrucciones;
} t_pcb;

typedef struct {
	int tiempo_bloqueo;
	t_pcb* pcb;
} t_pcb_bloqueado;



typedef struct {
	t_pcb* pcb; //
	int socket_cliente;
	bool interrupcion; // false(0) o true(1)
} t_proceso;

typedef struct {
	t_config* config;
	t_log* logger;
	t_proceso* process;
	int socket_servidor;
	int conexion_con_memoria;
	int conexion_con_kernel;
	int socket_servidor_dispatch;//DISPATCH
	int socket_servidor_interrupt;//INTERRUPT
} proceso_cpu;

enum {
	NO_OP = 1,
	I_O,
	EXIT,
	COPY,
	READ,
	WRITE,
};

typedef enum {
	// Operaciones con cpu
	PCB = 200, //Recibimos del kernel una lista de instrucciones
	BLOQUEO_IO, // notifico al kernel que un proceso entró a I/O
	FINALIZACION_PROCESO, // notifico al kernel la finalizacion de un proceso
	INTERRUPCION, //Recibimos del kernel la orden de interrupcion

	// Operaciones con memoria
	HANDSHAKE = 300, // solicito a memoria cantidad de entradas por tabla de pagina y tamanio de las paginas
	NUMERO_DE_TABLA_PRIMER_NIVEL, // solicito a memoria el numero de tabla de primer nivel de un proceso
	NUMERO_DE_TABLA_SEGUNDO_NIVEL, // solicito a memoria el numero de tabla de segundo snivel de un proceso
	LECTURA_MEMORIA_USUARIO, // solicito lectura de un espacio de memoria
	ESCRITURA_MEMORIA_USUARIO, // solicito escritura en un espacio de memoria
	CONFIRMACION_ESCRITURA, // recibo de memoria la confirmacion de la escritura exitosa

	ERROR = -1 //Ocurrio un error en la comunicacion
} operacion;

enum {
	COD_RESP_BLOQUEO = 150, // pcb + tiempo
	COD_RESP_EXIT, // finalizo sus intrucciones
	COD_RESP_INTERRUPCION, // interrupion por puerto
};

typedef struct
{
	int size;
	void* stream;
} t_buffer;

//###### INTERFAZ FUNCIONES ######

void iniciar_conexion_cpu_memoria(proceso_cpu* cpu_process);
void escuchaInterrup(proceso_cpu* cpu_process);
proceso_cpu* iniciar_cpu(proceso_cpu*);
proceso_cpu* cpu_create(void);
t_proceso* process_create(void);
void finalizar_cpu(proceso_cpu* cpu_process);
void atender_kernel_dispatch(proceso_cpu* cpu_process,int conexion_kernel);
int recibir_operaciones(proceso_cpu* cpu_process, int socket_kernel);
//void recibir_mensaje_kernel(proceso_cpu* cpu_process, int socket_kernel);

t_pcb* recibir_pcb(int);
t_list* recibir_instrucciones(int);
int recibir_entero(int);
void iterator(char* value);
void agregar_instruccion(t_buffer* buffer, char* valor, int tamanio);
t_buffer* cargar_buffer(t_list* lista);
void* serializar_pcb(t_pcb* pcb, t_buffer* buffer, int bytes,operacion op);
void enviar_pcb(t_pcb* pcb, int conexion, operacion op);
void enviar_pcb_bloqueado(int socketKernel,t_pcb_bloqueado* bloqueado);
t_list* recibir_instrucciones(int socket_cliente);
t_pcb* recibir_pcb(int conexion);
/*
int fetch(t_proceso proceso);
void fetch_operands(t_proceso proces, t_instruction instruccion);
t_instruction decode(t_proceso proceso,int nro_intruccion);
void execute(t_proceso proceso, t_instruction instruccion);

void no_op(int tiempo,int repeticiones);
void i_o(t_proceso proceso, int tiempo);
void exitt(t_proceso proceso);
void  check_interrupt(t_proceso proceso);

void responseInterrupcion(t_proceso proceso);
void responsePorBloqueo(t_proceso proceso,int tiempo);
void responsePorFinDeProceso(t_proceso proceso);
void incrementarpcb(t_proceso proceso);
int size_instrucciones(t_proceso proceso);
*/
//### funcionales a cpu
void iniciar_servidor_dispatch(proceso_cpu* cpu_process);
void iniciar_servidor_interrupt(proceso_cpu* cpu_process);
int iniciar_servidor_cpu(proceso_cpu* cpu_process, char* key_puerto);
int esperar_cliente_cpu(proceso_cpu* cpu_process, int socket_server, char* tipo_puerto);
int esperar_cliente_dispatch(proceso_cpu* cpu_process);
int esperar_cliente_interrupt(proceso_cpu* cpu_process);
char** getParametrosInstruccion(t_list* instrucciones,int nro_inst);



char** decode(t_list* instrucciones,int nro_inst);
void execute(proceso_cpu* cpu, t_pcb* pcb,char** instruccion);
int fetch(t_pcb* pcb);
void fetch_operands(char** instruccion);
void no_op(int tiempo,t_pcb* pcb);
void i_o(proceso_cpu* cpu, t_pcb* pcb,int tiempo);
void instruccion_exit(proceso_cpu* cpu, t_pcb* pcb);
void incrementarProgramCounter(t_pcb* pcb);
void responsePorBloqueo(proceso_cpu* cpu,t_pcb* pcb,int tiempo);
void responsePorFinDeProceso(t_pcb* pcb,proceso_cpu* cpu);
bool checkInstruccionInterrupcion(char* instruccion);
void check_interrupt(proceso_cpu* cpu,t_pcb* pcb,char* operacion);
bool check_interrupcion(proceso_cpu* cpu);
void responseInterrupcion(t_pcb* pcb,proceso_cpu* cpu);
void verPCB(proceso_cpu* cpu,t_pcb* pcb);

#endif /* CPU_H_ */
