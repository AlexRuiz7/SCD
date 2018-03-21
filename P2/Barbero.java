import monitor.* ;

// ****************************************************************************

/**
 * Clase Barbería.
 * 
 * Implementa el monitor
 *
 * @author alex
 */
class Barberia extends AbstractMonitor{
    private Condition barbero;
    private Condition clientes;
    private Condition cliente_actual;
    
    /**
     * Constructor
     * 
     * Inicializa todas las variables de estado privadas del monitor.
     */
    public Barberia(){
        barbero        = makeCondition();
        clientes       = makeCondition();
        cliente_actual = makeCondition();
    }
    
    /**
     * Invocado por los clientes para cortarse el pelo.
     * El cliente despierta al barbero o espera a que termine con el cliente 
     * anterior.
     * 
     * @see Cliente
     */
    public void cortarPelo(){
        enter();
        if(!cliente_actual.isEmpty()){
            System.out.println("Cliente: el barbero está ocupado. Esperaré.");
            clientes.await();
        }  
        else{
            System.out.println("Cliente: despierto al barbero.");
            barbero.signal();
            System.out.println("Cliente: me están cortando el pelo.");
            cliente_actual.await();
            System.out.println("Cliente: me han cortado el pelo. Salgo de la barbería.");
        }   
        leave();
    }
  
    /**
     * Invocado por el barbero para esperar (si procede) a un nuevo cliente.
     * y sentarlo para el corte.
     * Si no hay ningún cliente, duerme.
     * 
     * @see Barbero
     */
    public void siguienteCliente(){
        enter();
        if(clientes.isEmpty() && cliente_actual.isEmpty()){
            System.out.println("Barbero: no hay clientes. A dormir!");
            barbero.await();
        }  
        else
            clientes.signal();
        leave();
    }
  
    /**
     * Invocado por el barbero para indicar que ha terminado de cortar el pelo.
     * Avisa al cliente actual de que puede marcharse.
     * 
     * @see Barbero
     */
    public void finCliente(){
        enter();
        System.out.println("Barbero: he terminado. Aviso al cliente.");
        cliente_actual.signal();
        leave();     
    }
  
}

// ****************************************************************************

/**
 * Clase Cliente.
 * 
 * Un cliente entra a la barbería a cortarse le pelo y despierta al babero si
 * está dormido, o espera si está cortándole el pelo a otro cliente.
 * 
 * @author alex
 */
class Cliente implements Runnable{
    public  Thread      thr;
    private Barberia    barberia;

    /**
     * Constructor de cliente
     * 
     * @param p_barberia    Inicializa el monitor compartido
     * @param numero        Indica el numero de cliente que ha llegado
     */
    public Cliente(Barberia p_barberia, int numero){
      barberia    = p_barberia;
      thr         = new Thread(this,"Cliente " + numero);
    }

    /**
     * Método que lanza la hebra
     * 
     * @see Barberia#cortarPelo() 
     */
    public void run(){
        while( true ){
            barberia.cortarPelo();
            aux.dormir_max(2000);
        }
    }
  
}

// ****************************************************************************

/**
 * Clase Barbero.
 * 
 * El barbero atiende a los clientes (si los hay) o duerme si no tiene ninguno.
 * 
 * @author alex
 */
class Barbero implements Runnable{
    private Barberia barberia;
    public  Thread   thr;

    /**
     * Constructor
     * 
     * @param p_barberia    Inicializa el monitor compartido
     */
    public Barbero(Barberia p_barberia){
        barberia = p_barberia;
        thr      = new Thread(this, "Barbero");
    }

    /**
     * Método que lanza la hebra
     * 
     * @see Barberia#siguienteCliente() 
     * @see Barberia#finCliente() 
     */
    public void run(){
      while( true ){
          barberia.siguienteCliente();
          aux.dormir_max(2500);
          barberia.finCliente();
      }

    }

}

// ****************************************************************************

class MainBarberia{
    
  public static void main( String[] args ){
    
    if( args.length != 1){
      System.err.println("Necesita 1 argumento: (1) núm.clientes.");
      return ;
    }
    
    // Número de clientes distintos que atenderá el babero,
    int n = Integer.parseInt(args[0]);
      
    // Crear clientes, barbero y barbería.
    Cliente[]   clientes    = new Cliente[n];
    Barberia    barberia    = new Barberia();
    Barbero     barbero     = new Barbero(barberia);

    // Crear hebras de clientes
    for(int i = 0; i < n; i++)
      clientes[i] = new Cliente(barberia, i) ;

    // Poner en marcha las hebras
    barbero.thr.start();
    for(int i = 0; i < n; i++)
      clientes[i].thr.start();
    
  }
}
