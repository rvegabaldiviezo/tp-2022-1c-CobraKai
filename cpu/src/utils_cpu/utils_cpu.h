#ifndef UTILS_CPU_UTILS_CPU_H_
#define UTILS_CPU_UTILS_CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // Biblioteca de Socket: Crea un punto final para la comunicacion
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>
#include <commons/config.h>
#include "../cpu.config"

#define PATH_CONFIG "../../cpu.config"

int iniciar_servidor(void);


#endif /* UTILS_CPU_UTILS_CPU_H_ */
