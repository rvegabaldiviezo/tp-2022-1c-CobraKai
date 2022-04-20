/*
 * consola.h
 *
 *  Created on: 17 abr. 2022
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>

t_paquete* agregar_paquete(int conexion);
char* leer_hasta(char caracter_de_paro, FILE* file);

#endif /* CONSOLA_H_ */
