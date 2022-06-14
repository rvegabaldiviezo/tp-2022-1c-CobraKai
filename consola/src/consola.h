#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "cliente/utils.h"
#include "commons/string.h"
#include "commons/config.h"
#include "commons/log.h"
#include "commons/collections/list.h"
#include "file/file.h"

#define CARACTER_SALTO_DE_LINEA '\n'
#define IP_KERNEL "IP_KERNEL"
#define PUERTO_KERNEL "PUERTO_KERNEL"
#define PATH_CONFIG "src/consola.config"

typedef enum {
	LISTA_DE_INSTRUCCIONES = 1
} operacion;


bool cantidad_parametros_correcta(int);
t_proceso* crear_proceso();
t_proceso* cargar_proceso(int, char*);
void destruir_proceso(t_proceso*);
void terminar_programa();

#endif /* CONSOLA_H_ */
