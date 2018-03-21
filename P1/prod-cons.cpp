// *****************************************************************************
//
// Prácticas de SCD. Práctica 1.
// Plantilla de código para el ejercicio del productor-consumidor con
// buffer intermedio.
//
// *****************************************************************************
#include <iostream>
#include <cassert>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>     // necesario para {\ttbf usleep()}
#include <stdlib.h>     // necesario para {\ttbf random()}, {\ttbf srandom()}
#include <time.h>       // necesario para {\ttbf time()}

using namespace std ;

// ---------------------------------------------------------------------------//
// Colores de consola configurables:
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define RESET   "\033[00m"

// ---------------------------------------------------------------------------//
// Constantes configurables:
const unsigned int
  num_items  = 40 ,         // numero total de items que se producen o consumen
  tam_vector = 10 ;         // tamaño del vector, debe ser menor que el número de items

  int buffer[tam_vector];   // vector de datos
  int usados = 0,           // Nº de espacios ocupados en el buffer   (LIFO)
      pos_libre = 0,        // Posición a la siguiente posición libre del buffer.
                            // Usado por el proceso prodcutor (FIFO).
      pos_leer  = 0,           // Posición a la siguiente posición a leer. Usado por
                            // el proceso consumidor (FIFO).
      size   = 0;           // Ocupación del buffer (realmente nº de datos no pos_leer. FIFO)

  sem_t mutex, consumir, producir;

// ---------------------------------------------------------------------------//
// Introduce un retraso aleatorio de duración comprendida entre
// 'smin' y 'smax' (dados en segundos)
void retraso_aleatorio( const float smin, const float smax ){
  static bool primera = true ;
  if ( primera ){      //   si es la primera vez:
    srand(time(NULL)); //   inicializar la semilla del generador
    primera = false ;  //   no repetir la inicialización
  }
  // calcular un número de segundos aleatorio, entre {\ttbf smin} y {\ttbf smax}
  const float tsec = smin+(smax-smin)*((float)random()/(float)RAND_MAX);
  // dormir la hebra (los segundos se pasan a microsegundos, multiplicándos por 1 millón)
  usleep( (useconds_t) (tsec*1000000.0)  );
}

// ---------------------------------------------------------------------------//
// Función que simula la producción de un dato
unsigned producir_dato(){
  static int contador = 0 ;
  contador++ ;
  retraso_aleatorio( 0.1, 0.5 );
  cout << "Productor: dato producido: " << contador << endl << flush ;
  return contador ;
}

// ---------------------------------------------------------------------------//
// Función que simula la consumición de un dato
void consumir_dato( int dato ){
   retraso_aleatorio( 0.1, 1.5 );
   cout << "Consumidor: dato consumido: " << dato << endl << flush ;
}

// ---------------------------------------------------------------------------//
//      FUNCIONES IMPLEMENTADAS EN PILA (LIFO)
//
// ---------------------------------------------------------------------------//

// Función que ejecuta la hebra del productor
void * funcion_productor_lifo( void * ){
  int dato;

  for(int i=0; i<num_items; i++){
    // Inserción de dato en el buffer.
    if(usados == tam_vector){   // Si el buffer está lleno, bloquear productor
      sem_wait(&producir);
    }
    else{                       // En otro caso, producir e insertar dato
      dato = producir_dato();
      sem_wait(&mutex);
      buffer[usados] = dato;
      usados++;

      cout << YELLOW << "Productor : dato insertado: " << dato << endl << RESET << flush ;
      cout << RED << "Ocupación del vector: " << usados << endl << RESET << flush;
      cout << RED << "Iteración: " << i << endl << RESET << flush;
      sem_post(&mutex);
    }
    sem_post(&consumir);        // Despertar a consumidor. El buffer no está vacío
  }
  return NULL;
}

// ---------------------------------------------------------------------------//

// Función que ejecuta la hebra del consumidor
void * funcion_consumidor_lifo( void * ){
  int dato ;

  for(int i=0; i<num_items; i++){
    if(usados == 0){            // Si el buffer está vacío, bloquear consumidor
      sem_wait(&consumir);
    }
    else{                       // En otro caso, consumir dato
      sem_wait(&mutex);
      usados--;
      dato = buffer[usados];

      cout << BLUE << "Consumidor: dato extraído : " << dato << RESET << endl << flush ;
      cout << RED << "Ocupación del vector: " << usados << endl << RESET << flush;
      sem_post(&mutex);

      consumir_dato( dato ) ;
    }
    sem_post(&producir);        // Despertar a productor. El buffer no está lleno
  }
  return NULL ;
}

// ---------------------------------------------------------------------------//
//      FUNCIONES IMPLEMENTADAS EN COLA (FIFO)
//
// ---------------------------------------------------------------------------//

// Función que ejecuta la hebra del productor
void * funcion_productor_fifo( void * ){
  int dato;

  for(int i=0; i<num_items; i++){
    dato = producir_dato();
    sem_wait(&mutex);
    buffer[pos_libre] = dato;
    pos_libre = (pos_libre+1)%tam_vector;
    size++;

    cout << YELLOW << "Productor: dato insertado: " << dato << endl << RESET << flush ;
    cout << RED << "Ocupación del vector: " << size << endl << RESET << flush;
    cout << RED << "Iteración: " << i << endl << RESET << flush;
    sem_post(&mutex);
    sem_post(&consumir);

    // Bloquear productor si ha alcanzado a consumidor.
    // IMPORTANTE: pos_leer --> Posición del último dato consumido.
    if(pos_libre == pos_leer)
      sem_wait(&producir);
  }
  return NULL;
}

//----------------------------------------------------------------------

// Función que ejecuta la hebra del consumidor
void * funcion_consumidor_fifo( void * ){
  int dato ;

  for(int i=0; i<num_items; i++){
    // Bloquear consumidor si usados está alcanza al productor o si el buffer está vacío.
    if( (pos_leer == pos_libre) || size == 0)
      sem_wait(&consumir);

    sem_wait(&mutex);
    dato = buffer[pos_leer];
    pos_leer = (pos_leer+1)%tam_vector;
    size--;

    cout << BLUE << "Consumidor: dato extraído : " << dato << RESET << endl << flush ;
    cout << RED << "Ocupación del vector: " << size << endl << RESET << flush;
    consumir_dato(dato);

    sem_post(&mutex);
    sem_post(&producir);


  }
  return NULL ;
}

//----------------------------------------------------------------------

int main(int argc, char *argv[]){

  if(argc != 2){
    cout << "\nSeleccione tipo de ejecución <LIFO/FIFO>";
    exit(-1);
  }

  pthread_t prod, cons;

  sem_init(&mutex, 0, 1);
  sem_init(&producir, 0, 0);
  sem_init(&consumir, 0, 0);

  if( ( (string) argv[1] == "lifo") || ( (string) argv[1] == "LIFO") ){
    pthread_create(&prod, NULL, funcion_productor_lifo, NULL);
    pthread_create(&cons, NULL, funcion_consumidor_lifo, NULL);
  }
  else{
    pthread_create(&prod, NULL, funcion_productor_fifo, NULL);
    pthread_create(&cons, NULL, funcion_consumidor_fifo, NULL);
  }

  pthread_join(prod, NULL);
  pthread_join(cons, NULL);

  cout << "\nHebras finalizadas. Fin de programa.";

  sem_destroy(&producir);
  sem_destroy(&consumir);
  sem_destroy(&mutex);

  return 0 ;
}
