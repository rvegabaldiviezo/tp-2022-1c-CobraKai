/*
 * Para correr el módulo hay que compilarlo con el martillito (o con CTRL + B), luego
 * en una terminal ir a la CARPETA RAíZ del proyecto y desde ahi escribir:
 * ./Debug/tp-2022-1c-CobraKai "consola/consola.config"
 * Aclaraciones:
 * 1. Aunque se valide que se pasen 2 parametros, solo se pasa la ruta al archivo
 * 	  ya que el primer parametro se pasa implicitamente.
 * 2. Si el archivo consola.config estuviera en otra ubicacion, al correrlo
 * 	  se le pasa la ruta desde la raíz del proyecto
 * Para detener la ejecucion del modulo, desde la consola apretar CTRL + C
 */

#include "consola.h"
#include "commons/config.h"
#include "commons/string.h"
#include "commons/log.h"
#include <stdio.h>
#include <stdlib.h>
#include "./../utils/cliente/utils_cliente.h"
#include "./../file/file.h"

int main(int argc, char** argv) {
	if(argc != 2) {
		puts("Cantidad de parametros incorrecta");
		return EXIT_FAILURE;
	}

	char* ip;
	char* puerto;
	t_config* config = config_create(argv[1]);
	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");

	int conexion = crear_conexion(ip, puerto);

	t_paquete* paquete = armar_paquete(conexion);


	return EXIT_SUCCESS;
}

void armar_paquete(int conexion) {
	char* instruccion;
	t_paquete* paquete = crear_paquete();
	FILE* pseudocodigo = fopen("./consola/pseudocodigo.txt", "r");

	while(!feof(pseudocodigo)) {
		// parsear_instrucciones()
		instruccion = leer_hasta('\n', pseudocodigo);
		puts(instruccion);
		agregar_a_paquete(paquete, instruccion, string_length(instruccion));
	}
	enviar_paquete(paquete, conexion);

	eliminar_paquete(paquete);
}



