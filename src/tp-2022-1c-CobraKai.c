/*
 ============================================================================
 Name        : tp-2022-1c-CobraKai.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>

int main(void) {
	char* s = string_new();
	string_append(&s, "edfwe");
	puts(s); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;
}
