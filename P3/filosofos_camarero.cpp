#include <iostream>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"
#include <mpi.h>

using namespace std;

#define coger     0
#define soltar    1
#define sentar    2
#define levantar  3
#define MAX       4
#define camarero  10

// Colores ansi para la consola
#define BLACK    "\033[0m"
#define RED      "\033[31m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define BLUE     "\033[34m"
#define MAGENTA  "\033[35m"
#define CYAN     "\033[36m"
#define WHITE    "\033[37m"

void Filosofo( int id, int nprocesos);
void Tenedor ( int id);
void Camarero();

// ---------------------------------------------------------------------

int main( int argc, char** argv ){
  int rank, size;

  srand(time(0));
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  if( size!=11)
  {
    if( rank == 0)
       cout<<"El numero de procesos debe ser 11" << endl << flush ;
    MPI_Finalize( );
    return 0;
  }

  if (rank == 10)
    Camarero();
  else if ( (rank%2) == 0 )
    Filosofo(rank,size-1); // Los pares son Filosofos
  else
    Tenedor(rank);  // Los impares son Tenedores

  MPI_Finalize( );
  return 0;
}

// ---------------------------------------------------------------------

void Filosofo( int id, int nprocesos ){
  int izq = (id+1) % nprocesos;
  int der = ((id+nprocesos)-1) % nprocesos;

  while(1){
    // Pensando
    cout << RED << "Filosofo " << id << " PENSANDO" << endl << flush;
    usleep( 1000U * (100U+(rand()%900U)) );

    // Solicita al camarero sentarse
    cout << RED << "Filosofo " << id << " solicita sentarse" << endl << flush;
    MPI_Ssend(NULL, 0, MPI_INT, camarero, sentar, MPI_COMM_WORLD);

    // Solicita tenedor izquierdo
    cout << RED << "Filosofo " << id << " pide tenedor izquierdo: " << izq << endl << flush;
    MPI_Ssend(NULL, 0, MPI_INT, izq, coger, MPI_COMM_WORLD);

    // Solicita tenedor derecho
    cout << RED << "Filosofo " << id << " pide tenedor derecho: " << der << endl << flush;
    MPI_Ssend(NULL, 0, MPI_INT, der, coger, MPI_COMM_WORLD);

    cout << RED << "Filosofo " << id << " COMIENDO" << endl << flush;
    sleep((rand() % 3)+1);  //comiendo

    // Suelta el tenedor izquierdo
    cout << RED << "Filosofo " << id << " suelta tenedor izquierdo: " << izq << endl << flush;
    MPI_Ssend(NULL, 0, MPI_INT, izq, soltar, MPI_COMM_WORLD);

    // Suelta el tenedor derecho
    cout << RED << "Filosofo " << id << " suelta tenedor derecho: " << der << endl << flush;
    MPI_Ssend(NULL, 0, MPI_INT, der, soltar, MPI_COMM_WORLD);

    // Solicita al camarero levantarse
    cout << RED << "Filosofo " << id << " solicita levantarse" << endl << flush;
    MPI_Ssend(NULL, 0, MPI_INT, camarero, levantar, MPI_COMM_WORLD);
 }
}

// ---------------------------------------------------------------------

void Tenedor(int id){
  int buf;
  MPI_Status status;
  int Filo;

  while( true ){
    // Espera un petición desde cualquier filosofo vecino ...
    MPI_Probe( MPI_ANY_SOURCE, coger, MPI_COMM_WORLD, &status );
    Filo = status.MPI_SOURCE;

    // Recibe la petición del filósofo ...
    MPI_Recv(&buf, 1, MPI_INT, Filo, coger, MPI_COMM_WORLD, &status);

    cout << GREEN << "Tenedor " << id << " a filosofo " << Filo << endl << flush;

    // Espera a que el filosofo suelte el tenedor...
    MPI_Probe( MPI_ANY_SOURCE, soltar, MPI_COMM_WORLD, &status );
    MPI_Recv(&buf, 1, MPI_INT, Filo, soltar, MPI_COMM_WORLD, &status);
    cout << GREEN << "Tenedor " << id << " liberado por filosofo " << Filo << endl << flush;
  }
}

// ---------------------------------------------------------------------

void Camarero(){
  int buf, Filo, sentados = 0;
  MPI_Status status;

  while(true){
    // Espera un petición desde cualquier filosofo. Si hay sitio en la mesa
    // espera a cualquier petición, si no, solo a las de levantarse.
    if(sentados < MAX)
      MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    else
      MPI_Probe(MPI_ANY_SOURCE, levantar, MPI_COMM_WORLD, &status);

    Filo = status.MPI_SOURCE;

    // Petición de sentarse
    if(status.MPI_TAG == sentar){
      MPI_Recv(NULL, 0, MPI_INT, Filo, sentar, MPI_COMM_WORLD, &status);
      sentados++;
      cout << BLUE << "Camarero: se ha sentado el filosofo " << Filo << endl << flush;
    }
    else{
      MPI_Recv(NULL, 0, MPI_INT, Filo, levantar, MPI_COMM_WORLD, &status);
      sentados--;
      cout << BLUE << "Camarero: se ha levantado el filosofo " << Filo << endl << flush;
    }

  }
}
