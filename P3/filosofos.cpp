
// Se puede prodcuir interbloqueo ya que todos los filosofos cogen en primer
// lugar su tenedor izquierdo. Dado que los procesos pares son lo filosofos y
// tenemos 10 procesos supongamos que empieza la ejecución y el filosofo 0 coge
// su tenedor izquierdo(1), luego el filosofo 2 hace lo mismo, y así hasta el
// filosofo 8 sin que ninguno de ellos coja su tenedor derecho.
// El tenedor izquierdo de cada filosofo es el tenedor derecho del filosofo anterior.
// Dada esta secuencia ninguno tiene ambos tenedores ni los puede tener.
// Para evitarlo se puede hacer que el primer filosofo coja su tenedor derecho
// en lugar del izquierdo, ambos a la vez, o ninguno.

#include <iostream>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"
#include <mpi.h>

using namespace std;

#define coger   0
#define soltar  1

// Colores ansi para la consola
#define RED      "\033[31m"
#define GREEN    "\033[32m"

void Filosofo( int id, int nprocesos);
void Tenedor ( int id, int nprocesos);

// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int rank, size;

   srand(time(0));
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   MPI_Comm_size( MPI_COMM_WORLD, &size );

   if( size!=10)
   {
      if( rank == 0)
         cout<<"El numero de procesos debe ser 10" << endl << flush ;
      MPI_Finalize( );
      return 0;
   }

   if ((rank%2) == 0)
      Filosofo(rank,size); // Los pares son Filosofos
   else
      Tenedor(rank,size);  // Los impares son Tenedores

   MPI_Finalize( );
   return 0;
}
// ---------------------------------------------------------------------
// Etiqueta = 0: Coger tenedor
// Etiqueta = 1: Soltar tenedor

void Filosofo( int id, int nprocesos )
{
   int izq = (id+1) % nprocesos;
   int der = ((id+nprocesos)-1) % nprocesos;

   while(1){
      // El primer filosofo coge los tenedores al revés para evitar interbloqueo.
      if(id == 0){
        // Solicita tenedor derecho
        cout <<RED<<"Filosofo "<<id<< " pide tenedor derecho: " << der << endl << flush;
        MPI_Ssend(NULL, 0, MPI_INT, der, coger, MPI_COMM_WORLD);

        // Solicita tenedor izquierdo
        cout <<RED<< "Filosofo "<<id<< " pide tenedor izquierdo: " << izq << endl << flush;
        MPI_Ssend(NULL, 0, MPI_INT, izq, coger, MPI_COMM_WORLD);
      }
      else{
        // Solicita tenedor izquierdo
        cout <<RED<< "Filosofo "<<id<< " pide tenedor izquierdo: " << izq << endl << flush;
        MPI_Ssend(NULL, 0, MPI_INT, izq, coger, MPI_COMM_WORLD);

        // Solicita tenedor derecho
        cout <<RED<<"Filosofo "<<id<< " pide tenedor derecho: " << der << endl << flush;
        MPI_Ssend(NULL, 0, MPI_INT, der, coger, MPI_COMM_WORLD);
      }

      cout<<RED<<"Filosofo "<<id<< " COMIENDO"<<endl<<flush;
      sleep((rand() % 3)+1);  //comiendo

      // Suelta el tenedor izquierdo
      cout <<RED<<"Filosofo "<<id<< " suelta tenedor izquierdo: " << izq << endl << flush;
      MPI_Ssend(NULL, 0, MPI_INT, izq, soltar, MPI_COMM_WORLD);

      // Suelta el tenedor derecho
      cout <<RED<<"Filosofo "<<id<< " suelta tenedor derecho: " << der << endl << flush;
      MPI_Ssend(NULL, 0, MPI_INT, der, soltar, MPI_COMM_WORLD);

      // Piensa (espera bloqueada aleatorio del proceso)
      cout <<RED<< "Filosofo " << id << " PENSANDO" << endl << flush;

      // espera bloqueado durante un intervalo de tiempo aleatorio
      // (entre una décima de segundo y un segundo)
      usleep( 1000U * (100U+(rand()%900U)) );
 }
}
// ---------------------------------------------------------------------

void Tenedor(int id, int nprocesos)
{
  int buf;
  MPI_Status status;
  int Filo;

  while( true )
  {
    // Espera un petición desde cualquier filosofo vecino ...
    MPI_Probe( MPI_ANY_SOURCE, coger, MPI_COMM_WORLD, &status );
    Filo = status.MPI_SOURCE;
    // Recibe la petición del filósofo ...

    // La petición puede ser coger o liberar el tenedor
    //if(status.MPI_TAG == coger)
    MPI_Recv(&buf, 1, MPI_INT, Filo, coger, MPI_COMM_WORLD, &status);

    cout <<GREEN<< "Tenedor " << id << " a filosofo " << Filo << endl << flush;

    // Espera a que el filosofo suelte el tenedor...
    MPI_Probe( MPI_ANY_SOURCE, soltar, MPI_COMM_WORLD, &status );
    MPI_Recv(&buf, 1, MPI_INT, Filo, soltar, MPI_COMM_WORLD, &status);
    cout <<GREEN<< "Tenedor " << id << " liberado por filosofo " << Filo << endl << flush;
  }
}
// ---------------------------------------------------------------------
