#include <mpi.h>
#include <iostream>
#include <math.h>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"

#define Productor    0                // Etiqueta de los productores
#define Consumidor   1                // Etiqueta de los consumidores
#define Buffer       5                // Proceso intermedio (buffer)
#define prods        5                // Número de productores
#define prod_iter    4                // Iteraciones de cada productor
#define cons         4                // Número de consumidores
#define ELEM         prods*prod_iter  // Elementos producidos
#define TAM          5                // Tamaño del buffer

// Colores ansi para la consola
#define BLACK    "\033[0m"
#define RED      "\033[31m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define BLUE     "\033[34m"
#define MAGENTA  "\033[35m"
#define CYAN     "\033[36m"
#define WHITE    "\033[37m"

using namespace std;

// ---------------------------------------------------------------------

void productor(int x)
{
   int value ;

   for ( unsigned int i=0; i < prod_iter ; i++ )
   {
      value = i ;
      cout << GREEN << "Productor " << x << " produce valor " << value << endl << flush ;

      // espera bloqueado durante un intervalo de tiempo aleatorio
      // (entre una décima de segundo y un segundo)
      usleep( 1000U * (100U+(rand()%900U)) );

      // enviar 'value' con etiqueta Productor
      MPI_Ssend( &value, 1, MPI_INT, Buffer, Productor, MPI_COMM_WORLD );
   }
}
// ---------------------------------------------------------------------

void buffer()
{
   int        value[TAM] ,
              peticion ,
              pos  =  0,
              proceso ;
   MPI_Status status ;

   for( unsigned int i=0 ; i < ELEM*2 ; i++ )
   {
      if ( pos==0 )      // el consumidor no puede consumir, se debe producir
         proceso = 0 ;
      else if (pos==TAM) // el productor no puede producir, se debe consumir
         proceso = 1 ;
      else               // en otro caso se puede consumir o producir
      {
         // leer 'status' del siguiente mensaje (esperando si no hay)
         MPI_Probe( MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );

         // calcular el proceso en función del origen del mensaje
         if ( status.MPI_TAG == Productor )
            proceso = 0 ;
         else
            proceso = 1 ;
      }

      switch(proceso){
         case 0:  // Recibe de un productor
            MPI_Recv( &value[pos], 1, MPI_INT, MPI_ANY_SOURCE, Productor, MPI_COMM_WORLD, &status );
            cout << RED << "Buffer recibe " << value[pos] << " de Productor " << status.MPI_SOURCE << endl << flush;
            pos++;
            break;
         case 1:  // Recibe petición de un consumidor
            MPI_Recv( &peticion, 1, MPI_INT, MPI_ANY_SOURCE, Consumidor, MPI_COMM_WORLD, &status );
            MPI_Ssend( &value[pos-1], 1, MPI_INT, status.MPI_SOURCE, Buffer, MPI_COMM_WORLD);
            cout << RED << "Buffer envía " << value[pos-1] << " a Consumidor " << status.MPI_SOURCE << endl << flush;
            pos--;
            break;
      }
   }
}

// ---------------------------------------------------------------------

void consumidor(int x)
{
   int         value,
               peticion = 1 ;
   float       raiz ;
   MPI_Status  status ;

   // Cada consumidor consume 5 valores, que por 4 consumidores serán 20 elementos consumidos
   for ( unsigned int i=0 ; i < prods ; i++ ){
     // La etiqueta de envio a consumidor es 2
     MPI_Ssend( &peticion, 1, MPI_INT, Buffer, Consumidor, MPI_COMM_WORLD );
     // La etiquete de envio de buffer a consumidor es 0
     MPI_Recv ( &value, 1,    MPI_INT, Buffer, Buffer, MPI_COMM_WORLD, &status );
     cout << BLUE << "Consumidor " << x << " recibe valor " << value << " de Buffer " << endl << flush ;

     // espera bloqueado durante un intervalo de tiempo aleatorio
     // (entre una décima de segundo y un segundo)
     usleep( 1000U * (100U+(rand()%900U)) );

     raiz = sqrt(value) ;
   }
}
// ---------------------------------------------------------------------

int main(int argc, char *argv[]){
  int rank,size;

  // inicializar MPI, leer identif. de proceso y número de procesos
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  // inicializa la semilla aleatoria:
  srand ( time (NULL) );

  // comprobar el número de procesos con el que el programa
  // ha sido puesto en marcha (debe ser 3)
  if ( size != 10 ){
    cout << "El numero de procesos debe ser 10 " << endl;
    return 0;
  }

  // verificar el identificador de proceso (rank), y ejecutar la
  // operación apropiada a dicho identificador
  if (rank < Buffer)
    productor(rank);
  else if (rank == Buffer)
    buffer();
  else
    consumidor(rank);

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
