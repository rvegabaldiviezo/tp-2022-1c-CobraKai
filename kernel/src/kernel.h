#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include "./server/utils.h"


void iterator(char* value);
bool conexion_exitosa(int);
void terminar_programa(t_log*, t_proceso*);
t_proceso* crear_proceso();
t_pcb crear_pcb(unsigned int);

#endif /* KERNEL_H_ */
