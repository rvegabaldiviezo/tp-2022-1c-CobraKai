/*
 * consola.c
 *
 *  Created on: 17 abr. 2022
 *      Author: utnso
 */

#include "consola.h"
#include "commons/config.h"
#include "commons/string.h"
#include "commons/log.h"
#include <stdio.h>
#include <stdlib.h>
#include "./../utils/cliente/utils_cliente.h"
#include "./../file/file.h"

int main(void) {
	char* ip;
	char* puerto;
	t_config* config;
	config = config_create("./consola/consola.config");
	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");

	int conexion = crear_conexion(ip, puerto);

	paquete(conexion);


	return 0;
}

void paquete(int conexion) {
	char* instruccion;
	t_paquete* paquete = crear_paquete();
	FILE* pseudocodigo = fopen("./consola/pseudocodigo.txt", "r");

	while(!feof(pseudocodigo)) {
		instruccion = leer_hasta('\n', pseudocodigo);
		agregar_a_paquete(paquete, instruccion, string_length(instruccion));
	}
	enviar_paquete(paquete, conexion);

	eliminar_paquete(paquete);
}



