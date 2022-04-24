#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include "cliente/utils.h"
#include "commons/string.h"
#include "commons/config.h"
#include "commons/log.h"
#include "file/file.h"

#define CARACTER_SALTO_DE_LINEA '\n'
#define IP_KERNEL "IP_KERNEL"
#define PUERTO_KERNEL "PUERTO_KERNEL"

typedef struct {
	char* instruccion;
	int parametros[2];
} t_instruccion; // Para no enviar instrucciones como string, igual consultar

t_paquete* armar_paquete();
bool cantidad_parametros_correcta(int argc);
void terminar_programa(int conexion, t_log* logger, t_config* config);

#endif /* CONSOLA_H_ */
