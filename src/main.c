/*
 * main.c
 *
 *  Created on: 29/05/2010
 *      Author: Eduar Castrillo Velilla
 *
 * Un ejemplo basico de como usar cexception.h
 */
#include "cexception.h"
#include <stdio.h>

#define DIV_BY_ZERO 0x31

/*
 * Retorna el cociente entre n y d
 * Lanza excepcion DIV_BY_ZERO si d es 0
 * y envia como dato un mensaje indicando el error.
 */
float quot(float n, float d) {

    char *msg = "/ By Zero\n";
    if(!d)
        throw(DIV_BY_ZERO, msg); // lanza si d = 0
    return n / d;
}

/* main, inicio del programa */
int main() {

    float n, d;

    printf("Input Numerator: ");
    scanf("%f", &n);
    printf("Input Denominator: ");
    scanf("%f", &d);
    
    try {
        printf("%f / %f = %f\n", n, d, quot(n, d));
    }
    catch(DIV_BY_ZERO) {
        char *cad = exception_data();
        printf(cad);
    } finally {
        printf("Finally code\n");
    }
    end_try

    return 0;
}
