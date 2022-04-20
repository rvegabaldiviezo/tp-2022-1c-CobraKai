/*
 * Para correr el módulo hay que compilarlo con el martillito (o con CTRL + B), luego
 * en una terminal ir a la CARPETA RAíZ del proyecto y desde ahi escribir:
 * ./Debug/tp-2022-1c-CobraKai "consola/consola.config" "TAMAÑO_DEL_PROCESO"
 * Aclaraciones:
 * 1. Aunque se valide que se pasen 3 parametros, solo se pasan 2 ya que primer parametro se pasa implicitamente.
 * 2. Si el archivo consola.config estuviera en otra ubicacion,
 * 	  al correrlo se le pasa la ruta desde la raíz del proyecto.
 * 3. TAMAÑO_DEL_PROCESO aunque sea un número, se pasa como string, por eso va entre comillas.
 * 4. Si el modulo no termina solo es porque algo falló, se conta con CTRL + C
 */

#include "consola.h"
#include "commons/config.h"
#include "commons/string.h"
#include "commons/log.h"
#include <stdlib.h>
#include "./../utils/cliente/utils_cliente.h"
#include "./../file/file.h"

int main(int argc, char** argv) {
	if(argc != 3) {
		puts("Cantidad de parametros incorrecta");
		return EXIT_FAILURE;
	}

	char* ip;
	char* puerto;
	t_config* config = config_create(argv[1]);
	ip = config_get_string_value(config, "IP_KERNEL");
	puerto = config_get_string_value(config, "PUERTO_KERNEL");

	int conexion = crear_conexion(ip, puerto);
	t_paquete* paquete = armar_paquete();
	enviar_paquete(paquete, conexion);

	// poner en funcion?
	eliminar_paquete(paquete);
	liberar_conexion(conexion);

	return EXIT_SUCCESS;
}

t_paquete* armar_paquete() {
	char* instruccion;
	t_paquete* paquete = crear_paquete();
	FILE* pseudocodigo = fopen("./consola/pseudocodigo.txt", "rt");
	instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);
	while(!feof(pseudocodigo)) {
		agregar_a_paquete(paquete, instruccion, string_length(instruccion));
		puts(instruccion); // Solo para verificar, borrar despues
		// parsear_instrucciones() <-- Implementar
		instruccion = leer_hasta(CARACTER_SALTO_DE_LINEA, pseudocodigo);
	}
	fclose(pseudocodigo);
	return paquete;
}

