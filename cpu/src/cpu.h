#ifndef CPU_H_
#define CPU_H_

// Include de utils
#include "./utils/clientServ.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <math.h>
#include <stdio.h>
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
#include <commons/collections/queue.h>


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
	double estimacion_rafaga;
	int socket_cliente;
	double inicio_rafaga;
	double estimacion_rafaga_restante;
	double tiempo_ejecucion;
	t_list* instrucciones;
} t_pcb;

typedef struct {
	int tiempo_bloqueo;
	t_pcb* pcb;
} t_pcb_bloqueado;


typedef struct {
	t_pcb* pcb; //
	int socket_cliente;
} t_proceso;


typedef struct {
	int socket;
	char* descriptcion;//Ej: si es Dispatch o Interrupt
} conexion;


typedef struct {
	int memoria;
	int kernel_dispatch;
	int kernel_interrupt;
	int servidor_dispatch;//DISPATCH
	int servidor_interrupt;//INTERRUPT
	bool interrupcion; // true(1): si se conectan al puerto interrup y envian su respectivo codigo de operacion, si no, false(0): valor por default
} proceso_cpu;


typedef struct{
	uint32_t nro_filas_tabla_nivel1;
	uint32_t tamano_pagina;
}t_datos_memoria;


typedef struct{
	uint32_t nro_pagina;
	uint32_t nro_marco;
	time_t timestamps;
}t_tlb;

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

void iniciar_conexion_cpu_memoria(void);
void escuchaInterrup(void);
proceso_cpu* cpu_create(void);
t_proceso* process_create(void);
void finalizar_cpu(void);
void atender_kernel_dispatch(void);
int recibir_operacion_interrupt(void);
int recibir_operacion_dispatch(void);
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
void iniciar_servidor_dispatch();
void iniciar_servidor_interrupt();
int iniciar_servidor_cpu(char* key_puerto);
int esperar_cliente_cpu(char* tipo_puerto);
void esperar_cliente_dispatch();
void esperar_cliente_interrupt();
char** getParametrosInstruccion(t_list* instrucciones,int nro_inst);



char** decode(t_list* instrucciones,int nro_inst);
void execute();
int fetch();
void fetch_operands();
void no_op(int tiempo);
void i_o(int tiempo);
void instruccion_exit();
void incrementarProgramCounter();
void responsePorBloqueo(int tiempo);
void responsePorFinDeProceso();
bool checkInstruccionInterrupcion();
void check_interrupt();
bool check_interrupcion();
void responseInterrupcion();
void mostrarPCB();
void mostrar_PCB_Bloqueado(t_pcb_bloqueado* bloqueado);

void iniciar_cpu();
void iniciar_logger_cpu();
void iniciar_config_cpu();
void iniciar_config_semaforos();

void handshake_cpu_memoria(void);
t_datos_memoria* recibir_handshake_memoria(int conexion);
void liberar_pcb(t_pcb* pcb);
uint32_t nro_pagina(int direccion_logica);
bool existeEnTLB(uint32_t numero_pagina);
uint32_t  obtener_marco();



//##################
void iniciar_memoria(void);
void iniciar_comunicacion_cpu_memoria(void);
void iniciar_tlb(void);
void iniciar_interrupt(void);
void iniciar_dispatch(void);
void reiniciar_tlb(void);


uint32_t leer_valor_en_memoria();
bool esta_completa_cola_TLB();
t_tlb* obtener_TLB(uint32_t numero_pagina_buscada);
void primer_acceso_memoria(int direccion_logica);
uint32_t segundo_acceso_memoria(int numero_pagina);
void guardar_en_TLB(uint32_t numero_pagina,uint32_t nro_marco);
void asignar_marco_tlb_memoria(uint32_t direccion_logica_origen);
uint32_t tercer_acceso_memoria_lectura(int direccion_logica, int numero_pagina, int marco);
void tercer_acceso_memoria_escritura(int direccion_logica, int numero_pagina, int marco, uint32_t valor_escribir);
uint32_t ejecutar_instruccion_read(int direccion_logica);
void ejecutar_instruccion_write(int direccion_logica,uint32_t valor_a_escribir);

#endif /* CPU_H_ */
