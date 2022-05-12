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

enum {
	LISTA_DE_INSTRUCCIONES = 1
};


bool cantidad_parametros_correcta(int);
void terminar_programa(int, t_log*, t_config*, t_proceso*);
t_paquete* parsear_instrucciones(t_paquete* paquete);
//t_proceso* crear_proceso();
t_proceso* cargar_proceso(t_proceso*);
void destruir_proceso(t_proceso*);

#endif /* CONSOLA_H_ */
