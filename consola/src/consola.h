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
#include "cliente/utils.h"
#include "commons/string.h"
#include "commons/config.h"
#include "commons/log.h"


#define CARACTER_SALTO_DE_LINEA '\n'

t_paquete* armar_paquete();
char* leer_hasta(char caracter_de_paro, FILE* file);
void validarCantidadParametrosConsola(int argc, t_log* logger);
void terminar_programa(int conexion,t_log* logger,t_config* config);
t_log* iniciar_logger(void);

#endif /* CONSOLA_H_ */
