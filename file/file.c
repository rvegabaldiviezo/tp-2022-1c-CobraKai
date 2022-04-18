/*
 * file.c
 *
 *  Created on: 17 abr. 2022
 *      Author: utnso
 */
#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>

char* leer_hasta(char caracter_de_paro, FILE* file) {
	char caracter = fgetc(file);
	char* cadena_guardada = string_new();
	while (caracter != caracter_de_paro) {
		string_append_with_format(&cadena_guardada, "%c", caracter);
		caracter = fgetc(file);
	}
	return cadena_guardada;
}
