/*
 * cexception.h
 *
 * Eduar Castrillo Velilla
 * Ingenieria de Sistemas
 * Universidad Nacional de Colombia
 * eduarcastrillo@gmail.com
 *
 * Created on: 29/05/2010
 *
 * Implementacion de un sistema de excepciones para C.
 * Posee varias caracteristicas de ejecucion del sistema de excepciones
 * de JAVA tales como
 *  - Lanzamiento de excepciones
 *  - relanzamiento de excepciones no capturadas
 *  - sentencias anidadas, etc, etc...
 *  - bloque finally
 *  - alcance de bloque para la variables definidas detro de bloques try, catch o
 *    finally.
 *
 * Cuando una excepcion no es capturada, se imprime en "stderr" el mensaje de excepcion
 * y se termina la ejecucion del programa
 * 
 * El flujo de ejecucion de las sentencias es igual al de JAVA, Bueno! hasta
 * donde pude testear xD ...si encuentran alguna falla Avisar!!! 
 * La excepcion est'a cuando se involucran sentencias "return" ejecutadas dentro de los bloques
 * los cuales afectan el correcto funcionamiento del sistema.
 * No se puede controlar una sentencia return. La unica solucion ser'ia crear un sistema
 * de retorno que se ajuste al funcionamiento del sistema (En proceso).
 * Igual se pueden evitar los retornos dentro de los bloques creando estrucuturas de control
 * bien definidas, as'i se ahorra problemas y a'un as'i retorna :)
 * 
 * Un ejemplo:
 * 
 * ...algun codigo...
 *
 *  try {
 *      ...
 *      return; // imperidia que se ejecute el codigo finally (si existe) 
 *              // e impediria una correcta limpieza de la pila del sistema (ejecutada por la macro end_try)
 *  } catch(id) {
 *      // caso similar si el return se encuentra aqu'i
 *  } finally {
 *      // y aqu'i :(
 *  }
 *  end_try
 *
 * Para escribir sentencias try-catch-finally, se sigue la siguiente estructura
 * general.
 *
 * Ejemplo:
 * 
 * 	try
 *          // codigo del try
 * 	catch(id_exception)
 *          // codigo del catch
 *      catch(id_exception_2)
 *                  ... mas bloques catch aqui ...
 *      catch(id_exception_n)
 *          // codigo del catch
 *      finally
 *          // Codigo finally
 * 	end_try
 *
 * - Los bloques catch son opcionales, puede colocar varios o ninguno.
 *   Utilice un bloque "catch(C_EXCEPTION)" para capturar cualquier excepcion.
 *   Dicho bloque es el equivalente a usar "catch(...)" en C++ o "catch(Excepction e)" en JAVA.
 * - El bloque finally es opcional, pero debe ser unico.
 * - Toda sentencia debe ser terminada con la macro end_try
 *
 * El codigo escrito entre bloques, o entre un bloque delimitado por la macro end_try
 * pertenece al bloque al cual el codigo sucede.
 *
 * ejemplo:
 *
 *      try
 *          // codigo perteneciente al bloque try
 *      catch(id_except_1)
 *          // codigo perteneciente al bloque catch(id_except_1)
 *      catch(id_except_n)
 *          // codigo perteneciente al bloque catch(id_except_n)
 *      finally
 *          // codigo perteneciente al bloque finally
 *      end_try
 *
 * Para mayor claridad se pueden colocar corchetes...
 *
 *      try {
 *          // codigo try
 *      }
 *      catch(id) {
 *          // codigo del bloque catch(id_1)
 *      }
 *      catch(id_n) {
 *          // codigo del catch(id_n)
 *      }
 *      finally {
 *          // codigo del finally
 *      }
 *      end_try
 *
 * OJO!!!
 * Aunque coloque corchetes, ellos no indican realmente la delimitacion del bloque
 *
 * ejemplo:
 *      // suponga X una variable de tipo int
 *
 *      try {
 *          // Codigo del bloque try
 *      }
 *      X = 5;
 *      catch(id_except) {
 *          // ... algun codigo
 *      }
 *      end_try
 *
 * En este caso la sentencia "X = 5;" que est'a delimitada por los bloques try y catch(id_except)
 * pertenece al bloque try, ello por la propiedad anterior.
 *
 * El unico valor que no pueden se usado como identificador de excepcion es el 0 (Cero).
 * Utiliza dicho valor puede generar un comportamiento inesperado.
 * Este valor est'a reservado para uso del sistema.
 *
 * Para lanzar excepciones, se deben usar las macros throw o exthrow.
 * 
 *          throw(id_exception, ptr_data);
 *          exthrow(id_exception, ptr_data, what);
 *
 * Donde...
 * - id_exception es el valor entero que identifica la excepcion (NO puede ser cero)
 * - ptr_data es un puntero de tipo (void *) mediante el cual se envian datos al manejador de excepcion.
 * - what (en el caso de exthrow) es un mensaje que indica cual fue el motivo de la excepcion el cual es
 *   impreso en "stderr" junto a la linea de codigo que gener'o la excepcion cuando
 *   no es capturada.
 *
 * el valor enviado en ptr_data es recuperado mediante la funcion exception_data();
 * Ejemplo:
 * 
 *      try {
 *          char *cad = "Excepcion 1 generada\n";
 *          throw(1, cad);      // algun codigo que lanza excepcion tipo 1. Enviar cad como dato de excepcion
 *      } catch(1) {
 *          char *msg = exception_data();
 *          if(msg) {  // verificar si la excepcion ha enviado datos
 *              printf(msg);        // imprime en pantalla "Excepcion 1 generada", que fue el dato enviado
 *          }
 *      }
 *      end_try
 */
#ifndef _CEXCEPTION_H_
#define _CEXCEPTION_H_

#include <setjmp.h>

/* ID para Excepcion generica.
 * Cada vez que se lance una excepcion, y se encuentre un bloque de tipo "catch(C_EXCEPTION)"
 * ser'a ejecutado */
#define C_EXCEPTION -1

/* Funciones del sistema. NO usar, a menos que sepa lo que est'a
 * haciendo */
void __throw(int _line, const char *_function, const char *_file, const char *_what, int _id_exception, void *_data);
int  __try(jmp_buf *_ref_addr);
int  __catch(int _id);
void __finally();
void __end_try();

/* Retorna el dato enviado por la ultima sentencia throw capturada y que esta siendo
   procesada */
void *exception_data();

/* Sentencia sthrow. */
#define throw(id_except, data)          __throw(__LINE__, __FUNCTION__, __FILE__, 0, id_except, data)
#define exthrow(id_except, data, what)  __throw(__LINE__, __FUNCTION__, __FILE__, what, id_except, data)

/* Bloque try */
#define try { \
            jmp_buf __ref_addr; \
            setjmp(__ref_addr); \
            if(!__try(&__ref_addr)) { \

/* Bloque catch */
#define catch(id)  } else if(__catch(id)) {

/* Bloque finally */
#define finally } __finally(); {

/* La macro end_try, marca el final de la sentencia try-catch-finally */
#define end_try } __end_try(); }

#endif /* _CEXCEPTION_H_ */
