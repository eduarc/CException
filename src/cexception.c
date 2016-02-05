/*
 * cexception.c
 *
 *  Created on: 29/05/2010
 *  Modified on: 10/01/2011
 *      Author: Eduar Castrillo Velilla
 *
 * He cambiado la implementacion del estado de 'try_env_t' de 'Valores Unicos' por
 * 'Bits de Estados' lo cual facilita consultar dicho estado por medio de los Bitmasks
 * correpondientes.
 * Ademas he corregido algunos errores de informacion relacionados con la sentencia
 * 'throw'. La correccion consiste en haber extendido la pila de datos a una pila
 * que contenga informacion de la sentencia throw.
 * Otras modificaciones varias...
 */
#include "cexception.h"
#include <stdlib.h>     // exit(int)
#include <malloc.h>     // malloc(unsigned) y free(void *)
// #include <stdint.h>  // tipos uint32_t y uint64_t

    // BitMasks para flag 'status' de 'try_env_t'
#define ST_TRY           0x1
#define ST_CATCH         0x2
#define ST_FINALLY       0x4

/* Variables de entorno para una sentencia try-catch-finally */
typedef struct __try_env {
    jmp_buf *far_jmp;
    int id_except;
    int status;
} try_env_t;

/* Informacion de una sentencia throw(id, data) */
typedef struct __throw_env {
        // Dato enviado al manejador de excepcion
    void *data;
        // informacion de la  sentencia throw
    int line;
    const char *function;
    const char *file;
    const char *what;
} throw_env_t;

/* Nodos para la pila de sentenias y de datos */
typedef struct __node_try_env {
    try_env_t *env;
    struct __node_try_env *next;
} try_stack_t;

typedef struct __node_except_data {
    throw_env_t *env;
    struct __node_except_data *next;
} except_data_stack_t;

/* Pila de sentencias */
static try_stack_t *try_stack = 0;
/* Pila de datos */
static except_data_stack_t *data_stack = 0;

// ---------------------------
// Implementacion de las PILAS
// ---------------------------

static try_env_t *pushtry(try_env_t *t) {

    try_stack_t *node = malloc(sizeof(try_stack_t));
    node->env = t;
    node->next = 0;
    if(try_stack) {
        node->next = try_stack;
    }
    try_stack = node;
    return t;
}

static  try_env_t *poptry() {

    if(!try_stack) {
        return 0;
    }
    try_env_t *top = try_stack->env;
    try_stack_t *todelete = try_stack;
    try_stack = try_stack->next;
    free(todelete);
    return top;
}

static try_env_t *toptry() {
    
    if(!try_stack) {
        return 0;
    }
    return try_stack->env;
}

static  throw_env_t *pushthrow(throw_env_t *data) {

    except_data_stack_t *node = malloc(sizeof(except_data_stack_t));
    node->env = data;
    node->next = 0;
    if(data_stack) {
        node->next = data_stack;
    }
    data_stack = node;
    return data;
}

static throw_env_t *popthrow() {

    if(!data_stack) {
        return 0;
    }
    throw_env_t *top = data_stack->env;
    except_data_stack_t *todelete = data_stack;
    data_stack = data_stack->next;
    free(todelete);
    return top;
}

static throw_env_t *topthrow() {

    if(!data_stack) {
        return 0;
    }
    return data_stack->env;
}

// -------------------------------------------------------
// Implementacion de bloques, lanzadores y manejo de datos
// -------------------------------------------------------
/*void get_retaddr();
void ret_void();
void ret32(uint32_t v);
void ret64(uint64_t v);*/

/* Lanza una excepcion de tipo ID */
static void __ithrow(int id_except, throw_env_t *t_env) {
    
	try_env_t *current = toptry();

    if(current) {    // si hay por lo menos un try en la pila
        int status = current->status;

        if(status & ST_CATCH) {
            free(popthrow());    // Retirar sentencia throw capturada
        }
        pushthrow(t_env);   // insertar nueva sentencia throw
        current->id_except = id_except;

        if(status & ST_FINALLY) { // Ejecutando bloque finally
            __end_try();    // relanzar excepcion a nuevo try, (No retorno)
        }
        longjmp(*(current->far_jmp), 0);
    }
    if(!t_env->what) {
        t_env->what = "Unhandled Exception";
    }
    fprintf(stderr, "\nException %d in function \"%s\"\n%s\n - at %s:%d\nC Result: 1\n",
            id_except,
            t_env->function,
            t_env->what,
            t_env->file, t_env->line);
    free(t_env);
    exit(1);
}

/* Lanza una excepcion de tipo ID */
void __throw(int line, const char *function, const char *file, const char *what,
                int id_except, void *data)
{
    throw_env_t *new_throw = malloc(sizeof(throw_env_t));
    new_throw->data     = data;
    new_throw->line     = line;
    new_throw->function = function;
    new_throw->file     = file;
    new_throw->what     = what;
    
    __ithrow(id_except, new_throw);
}

/* Crea un nuevo bloque try */
int __try(jmp_buf *addr) {

    try_env_t *curr = toptry();

    if(curr && curr->id_except
		&& !(curr->status & ST_FINALLY))		// Ahora si es claro :)
    { // Se ha lanzado una excepcion dentro del bloque try o algun catch de la sentencia actual
        return 1;
    }
    try_env_t *new_try = malloc(sizeof(try_env_t));
    new_try->far_jmp = addr;
    new_try->id_except = 0;
    new_try->status = ST_TRY;
    pushtry(new_try);
    return 0;
}

/* Verifica si la excepcion de tipo ID es trapada. */
int __catch(int id) {

    try_env_t *current = toptry();
    
    if(current->status & ST_CATCH) { // Se ejecutara finally si existe y end_try
        return 0;
    }

    if(current->id_except == id 
            || id == C_EXCEPTION)
    {
        current->status |= ST_CATCH;
        current->id_except = 0;
        return 1;
    }
    return 0;
}

/* ejecuta el bloque finally */
void __finally() {
    toptry()->status |= ST_FINALLY;
}

/* ejecuta la limpieza de la pila y relanza excepcion si es necesario */
void __end_try() {

    try_env_t *current = poptry();
    
    if(current->id_except) {    // Relanzar excepcion
        int id = current->id_except;
        free(current);
        __ithrow(id, popthrow());
    }
    if(current->status & ST_CATCH) {
        free(popthrow());    // Retirar sentencia throw capturada
    }
    free(current);
}

/* Retorna el dato enviado por la ultima sentencia throw capturada y que esta siendo
   procesada */
void *exception_data() {

    try_stack_t *ptr = try_stack;
    while(ptr) {
        if(ptr->env->status & ST_CATCH) {
            return topthrow()->data;
        }
        ptr = ptr->next;
    }
    return 0;
}
