#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include "cliente/utils.h"
#include "commons/string.h"
#include "commons/config.h"
#include "commons/log.h"
#include "commons/collections/list.h"
#include "file/file.h"

#define CARACTER_SALTO_DE_LINEA '\n'
#define IP_KERNEL "IP_KERNEL"
#define PUERTO_KERNEL "PUERTO_KERNEL"


bool cantidad_parametros_correcta(int);
void terminar_programa(int, t_log*, t_config*);
t_paquete* armar_paquete();

#endif /* CONSOLA_H_ */
