#include "consola.h"
#include "commons/config.h"
#include "commons/string.h"
#include "commons/log.h"
#include <stdio.h>
#include <stdlib.h>
#include "file/file.h"
#include "cliente/utils.h"

int main(int argc, char** argv) {

	t_log* logger = iniciar_logger();
	validarCantidadParametrosConsola(argc,3,logger);
	char* ip;

	t_config* config = config_create("./consola/src/consola.config");//argv[1]);
	log_info(logger,"ENTRO");
	char* key = "IP";
		int response = config_has_property(config,key);
		log_info(logger,"ENTRO");
		if(response >0){
			ip = config_get_string_value(config,key);
			log_info(logger,ip);

		}else{ log_info(logger,"Error");}
	//char* ip = config_get_string_value(config, "IP");
	log_info(logger,"SALIO");
	char* puerto = config_get_string_value(config, "PUERTO");

	int conexion = crear_conexion(ip, puerto);
	paquete(conexion);


	terminar_programa(conexion,logger,config);

	return EXIT_SUCCESS;
}

t_paquete* armar_paquete() {
	char* instruccion;
	t_paquete* paquete = crear_paquete();
	FILE* pseudocodigo = fopen("./src/pseudocodigo.txt", "rt");
	instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);
	while(!feof(pseudocodigo)) {
		agregar_a_paquete(paquete, instruccion, string_length(instruccion));
		puts(instruccion); // Solo para verificar, borrar despues
		// parsear_instrucciones() <-- Implementar
		instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);
	}
	fclose(pseudocodigo);
	free(instruccion);
	return paquete;
}
void paquete(int conexion)
{

	char* instruccion;

	t_paquete* paquete = crear_paquete();

	FILE* pseudocodigo = fopen("pseudocodigo.txt", "rt");

	instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);

	while(!feof(pseudocodigo)) {
		agregar_a_paquete(paquete, instruccion, string_length(instruccion));
		puts(instruccion);
		instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);
	}
	fclose(pseudocodigo);

	enviar_paquete(paquete, conexion);

	free(instruccion);

	eliminar_paquete(paquete);
}

void validarCantidadParametrosConsola(int argc, int nroParam,t_log* logger){
	int ERROR = -1;
	if(argc != nroParam) {
		puts("CANTIDAD DE PARAMETROS INCORRECTA");
		log_info(logger,"CANTIDAD DE PARAMETROS INCORRECTA");
		exit(ERROR);
	}
}
t_log* iniciar_logger(void)
{
	t_log* nuevo_logger;

	nuevo_logger = log_create("./src/consola.log","CONSOLA", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

//void terminar_programa(int conexion, t_log* logger, t_config* config)
void terminar_programa(int conexion,t_log* logger,t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}



