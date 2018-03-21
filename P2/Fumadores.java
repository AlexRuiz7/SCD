import monitor.* ;

// ****************************************************************************

/**
 * Clase Estanco.
 * 
 * Implementa el monitor
 *
 * @author alex
 */
class Estanco extends AbstractMonitor{
    private int ingrediente_mostrador;
    private boolean recogido;
    private Condition fumador[];
    private Condition estanquero;
    
    /**
     * Constructor
     */
    public Estanco(){
        ingrediente_mostrador   = -1;
        recogido                = false;
        estanquero              = makeCondition();
        fumador                 = new Condition[3];
        
        for(int i=0; i<3; i++)
            fumador[i] = makeCondition();
    }
    
    /**
     * Invocado por cada fumador, indicando su ingrediente o número
     * 
     * @param ingrediente
     * 
     * @see Fumador
     */
    public void obtenerIngrediente( int ingrediente ){
        enter();
        if(ingrediente != ingrediente_mostrador)
            fumador[ingrediente].await();
        recogido = true;
        System.out.println("Fumador " + ingrediente + " : \tingrediente " + ingrediente_mostrador + " recogido");
        estanquero.signal();
        leave();
    }
  
    /**
     * Invocado por el estanquero, indicando el ingrediente que pone
     * 
     * @param ingrediente 
     * 
     * @see Estanquero
     */
    public void ponerIngrediente( int ingrediente ){
        enter();
        ingrediente_mostrador = ingrediente;
        System.out.println("Estanquero:\tingrediente " + ingrediente + " generado");
        recogido = false;
        fumador[ingrediente].signal();
        leave();
    }
  
    /**
     * Invocado por el estanquero
     * 
     * @see Estanquero
     */
    public void esperarRecogidaIngrediente(){
        enter();
        if(!recogido){
            System.out.println("Estanquero:\tesperando...");
            estanquero.await();
       }
       leave();     
    }
  
}

// ****************************************************************************

/**
 * Clase fumador.
 * 
 * Una hebra fumandor debe esperar a tener los ingredientes necesarios para
 * fumar (tabaco, papel y cerillas). Hay 3 fumadores y a cada uno de ellos le
 * falta un ingrediente de los 3 mencionados para poder fumar. Este ingrediente
 * lo proporciona el Estanquero de manera aleatoria.
 * 
 * @author alex
 */
class Fumador implements Runnable{
    private int       ingrediente;
    public  Thread    thr;
    private Estanco   estanco;

    /**
     * Construuctor de fumador
     * 
     * @param miIngrediente   Ingrediente == 1:   necesita tabaco
     *                        Ingrediente == 2:   necesita papel
     *                        Ingrediente == 3:   necesita cerillas
     * @param p_estanco       Inicializa el monitor compartido
     */
    public Fumador(int miIngrediente, Estanco p_estanco){
      ingrediente = miIngrediente;
      estanco     = p_estanco;
      thr         = new Thread(this,"Fumador "+ingrediente);
    }

    /**
     * Método que lanza la hebra
     * 
     * @see Estanco#obtenerIngrediente(int) 
     */
    public void run(){

        while( true ){
            estanco.obtenerIngrediente(ingrediente);
            aux.dormir_max(1000);
        }
    }
  
}

// ****************************************************************************

/**
 * Clase Estanquero.
 * 
 * El estanquero continuamente produce ingredientes y espera a que se recojan.
 * 
 * @author alex
 */
class Estanquero implements Runnable{
    private Estanco estanco;
    public  Thread  thr   ;

    /**
     * Constructor
     * 
     * @param p_estanco       Inicializa el monitor compartido
     */
    public Estanquero(Estanco p_estanco){
        estanco = p_estanco;
        thr     = new Thread(this, "Estanquero");
    }

    /**
     * Método que lanza la hebra
     * 
     * @see Estanco#ponerIngrediente(int) 
     * @see Estanco#esperarRecogidaIngrediente() 
     */
    public void run(){
      int ingrediente;
      while( true ){
          ingrediente = (int) (Math.random() * 3.0);
          estanco.ponerIngrediente(ingrediente);
          estanco.esperarRecogidaIngrediente();
      }

    }

}

// ****************************************************************************

class MainFumadores{
    
  public static void main( String[] args ){
    // Crear fumadores, estanquero y estanco.
    Fumador[]  fumadores  = new Fumador[3];
    Estanco    estanco    = new Estanco();
    Estanquero estanquero = new Estanquero(estanco);

    // Crear hebras de fumadores
    for(int i = 0; i < 3; i++)
      fumadores[i] = new Fumador(i, estanco) ;

    // Poner en marcha las hebras
    estanquero.thr.start();
    for(int i = 0; i < 3; i++)
      fumadores[i].thr.start();
    
  }
}
