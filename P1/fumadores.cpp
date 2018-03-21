// *****************************************************************************
//
// Prácticas de SCD. Práctica 1.
// Plantilla de código para el ejercicio de los fumadores
//
// *****************************************************************************

#include <iostream>
#include <cassert>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>      // incluye "time(....)"
#include <unistd.h>    // incluye "usleep(...)"
#include <stdlib.h>    // incluye "rand(...)" y "srand"

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
  num_hebras = 3,                                       // Nº de fumadores
  it = 20 ;                                             // Iteraciones por fumador
int ingrediente;                                        // Ingrediente producido

sem_t mutex, puede_fumar[num_hebras], puede_producir;   // Semáforos

// Opción de semáforo compartido
// sem_t puede_fumar;
// Incialización de semáforo compartido a num_hebras
// sem_init(&fumadores, num_hebras, 0);

// ---------------------------------------------------------------------
// Introduce un retraso aleatorio de duración comprendida entre
// 'smin' y 'smax' (dados en segundos)

void retraso_aleatorio( const float smin, const float smax )
{
  static bool primera = true ;
  if ( primera )        // si es la primera vez:
  {  srand(time(NULL)); //   inicializar la semilla del generador
     primera = false ;  //   no repetir la inicialización
  }
  // calcular un número de segundos aleatorio, entre {\ttbf smin} y {\ttbf smax}
  const float tsec = smin+(smax-smin)*((float)random()/(float)RAND_MAX);
  // dormir la hebra (los segundos se pasan a microsegundos, multiplicándos por 1 millón)
  usleep( (useconds_t) (tsec*1000000.0)  );
}

// ----------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatorio de la hebra.
// recibe como parámetro el numero de fumador
// el tiempo que tarda en fumar está entre dos y ocho décimas de segundo.

void fumar( int num_fumador )
{
   sem_wait(&mutex);
   cout << "Fumador número " << num_fumador << ": comienza a fumar." << endl << flush ;
   sem_post(&mutex);

   retraso_aleatorio( 0.2, 0.8 );

   sem_wait(&mutex);
   cout << "Fumador número " << num_fumador << ": termina de fumar." << endl << flush ;
   sem_post(&mutex);
}
// ----------------------------------------------------------------------------
//
//
//

void * funcion_estanquero(void * ){

  while(true){
    sem_wait(&mutex);
    ingrediente = rand()%3;
    retraso_aleatorio (0.2, 0.5);     // Producir ingrediente
    cout << RED << "Estanquero: ingrediente " << ingrediente << " producido" << RESET << endl << flush;
    sem_post(&mutex);
    sem_post(&puede_fumar[ingrediente]);
    sem_wait(&puede_producir);        // El estanquero espera a la retirada del ingrediente
  }

}

// ----------------------------------------------------------------------------
//
//
//

void * funcion_fumador(void * id_fumador){
  unsigned long id = (unsigned long) id_fumador;

  while(true){
    if(id == ingrediente){
      sem_wait(&mutex);
      cout << YELLOW << "Fumador[" << id << "]: ingrediente " << ingrediente << " recogido." << RESET << endl << flush;
      sem_post(&mutex);
      fumar(id);
      sem_post(&puede_producir);    // Ingrediente recogido. Despertar al estanquero.
    }
    else
      sem_wait(&puede_fumar[id]);
  }

}

// ----------------------------------------------------------------------------

int main()
{
  srand( time(NULL) );

  pthread_t h_estanquero, h_fumadores[num_hebras];

  sem_init(&mutex, 0, 1);
  sem_init(&puede_producir, 0, 1);
  for(int i=0; i<num_hebras; i++)
    sem_init(&puede_fumar[i], 0, 0);

  // Fumador 0: necesita papel
  // Fumador 1: necesita tabaco
  // Fumador 2: necesita cerillas

  pthread_create(&h_estanquero, NULL, funcion_estanquero, NULL);
  for(unsigned long i=0; i<num_hebras; i++)
    pthread_create(&h_fumadores[i], NULL, funcion_fumador, (void *) i);


  for(int i=0; i<num_hebras; i++)
    pthread_join(h_fumadores[i], NULL);
  pthread_join(h_estanquero, NULL);

  for(int i=0; i<num_hebras; i++)
    sem_destroy(&puede_fumar[i]);
  sem_destroy(&puede_producir);

  return 0 ;
}
