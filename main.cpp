#include <iostream>
#include "WaitingQueue.h"
#include "ProcessingQueue.h"
#include "Semaforo.h"
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;

// Indica si todos los productores ya terminaron de generar paquetes
bool produccionFinalizada = false;

// Cuenta cuántos productores terminaron
int productoresFinalizados = 0;

// Protege las variables de finalización
mutex mtx_productores;

// Configuración de la simulación
int cantidadProductores;
int cantidadConsumidores;
int paquetesTotales;
int consumidos = 0;
int escenario;

// Tiempos de espera
const int T_WAITING = 90; // 9ms -> 90ms
const int T_DESPACHO = 420; // 42ms -> 420ms
const int T_CINTA = 550; // 55ms -> 550ms
const int T_CONSUMO = 270; // 27ms -> 270ms
// Semáforos de sincronización
Semaforo hay_paquetes_waiting; // paquetes disponibles en WaitingQueue
Semaforo hay_espacio_cinta;    // espacios libres en ProcessingQueue
Semaforo hay_paquetes_cinta;   // paquetes disponibles en ProcessingQueue

// Mutex de exclusión mutua
mutex mtx_waiting;     // protege WaitingQueue
mutex mtx_processing;  // protege ProcessingQueue
mutex mtx_contador;    // protege contador_global
mutex mtx_consumidos;  // protege contador_consumidos;
mutex mtx_cout;        // protege impresiones por consola;
// Generador de IDs únicos para los paquetes
int contador_global = 0;

// Metricas de tiempo de espera por prioridad
mutex mtx_metricas;
long long tiempoEsperaAltaTotal = 0;
long long tiempoEsperaBajaTotal = 0;
int cantidadAltaProcesada = 0;
int cantidadBajaProcesada = 0;

void productor (WaitingQueue& waiting, int cantPaquetes);
void despachador(WaitingQueue& waiting, ProcessingQueue& processing);
void consumidor(ProcessingQueue& processing);

int main()
{

    vector<thread> productores;
    vector<thread> consumidores;
    int configuracion;
    cout << " ========== CONFIGURACION ==========" << endl;
    cout << " - Configuracion A: 1 Productor y 2 Consumidores" << endl;
    cout << " - Configuracion B: 3 Productores y 1 consumidor" << endl;
    cout << " - Configuracion C: 3 Productores y 3 Consumidores" << endl;
    cout << "Seleccione configuracion (1 al 3): ";
    while(!(cin>>configuracion)){
        cout << "Valor no valido, ingrese nuevamente: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }
    switch(configuracion) {
        case 1: cantidadProductores = 1; cantidadConsumidores = 2; break;
        case 2: cantidadProductores = 3; cantidadConsumidores = 1; break;
        case 3: cantidadProductores = 3; cantidadConsumidores = 3; break;
    }
    cout << " \n========== ESCENARIO ==========" << endl;
    cout << "1 - Carga masiva (1550 paquetes)" << endl;
    cout << "2 - Vacuidad (0 paquetes)" << endl;
    cout << "3 - Saturacion (8 paquetes alta prioridad)" << endl;
    cout << "4 - Anti-starvation" << endl;
    cout << "Seleccione escenario (1 al 4): ";
    while(!(cin >> escenario)){
        cout << "Valor no valido, ingrese nuevamente: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }

    switch(escenario){
        case 1: paquetesTotales = 1550; break;
        case 2: paquetesTotales = 0; break;
        case 3: paquetesTotales = 8; break;
        case 4: paquetesTotales = 50; break;
    }

    // Cantidad de paquetes que producirá cada productor
    int paquetesPorProductor = paquetesTotales / cantidadProductores;
    int resto = paquetesTotales % cantidadProductores;
// Inicialización de semáforos
    init(hay_paquetes_waiting,0); // inicialmente no hay paquetes esperando
    init(hay_espacio_cinta,5);    // la cinta tiene 5 espacios libres
    init(hay_paquetes_cinta,0);   // inicialmente no hay paquetes en la cinta

// Estructuras compartidas del sistema
    WaitingQueue waiting;
    ProcessingQueue processing;

    //Creador de productores segun config
    for(int i=0; i<cantidadProductores; i++)
    {
        int extra = (i == 0) ? resto : 0;
        productores.emplace_back(
            productor,
            ref(waiting),
            paquetesPorProductor + extra
        );
    }

    //Creador de despachador
    thread desp(despachador,
                ref(waiting),
                ref(processing));

    //Creador de consumidor
    for(int i=0; i<cantidadConsumidores; i++)
    {
        consumidores.emplace_back(
            consumidor,
            ref(processing)
        );
    }

    for(auto& p : productores)
    {
        p.join();
    }

    desp.join();

    for(auto& c : consumidores)
    {
        c.join();
    }

    // Metricas finales
    cout << "\n===== METRICAS FINALES =====" << endl;
    cout << "Paquetes producidos totales: " << contador_global << endl;

    if(cantidadAltaProcesada > 0)
    {
        cout << "Tiempo promedio de espera [ALTA prioridad]: "
             << (tiempoEsperaAltaTotal / cantidadAltaProcesada)
             << " ms (" << cantidadAltaProcesada << " paquetes)" << endl;
    }
    else
    {
        cout << "Tiempo promedio de espera [ALTA prioridad]: sin paquetes" << endl;
    }

    if(cantidadBajaProcesada > 0)
    {
        cout << "Tiempo promedio de espera [BAJA prioridad]: "
             << (tiempoEsperaBajaTotal / cantidadBajaProcesada)
             << " ms (" << cantidadBajaProcesada << " paquetes)" << endl;
    }
    else
    {
        cout << "Tiempo promedio de espera [BAJA prioridad]: sin paquetes" << endl;
    }

    cout << "============================" << endl;

    return 0;
}

//Fabrica paquetes y lo agrega a la estanteria
void productor (WaitingQueue& waiting, int cantPaquetes)
{
    for(int i = 0; i < cantPaquetes ; i++)
    {

        mtx_contador.lock(); //protege la variable contador_global
        int id = contador_global; //asigna un ID
        contador_global++; // incrementa en 1 el contador_global
        mtx_contador.unlock(); //libera la variable contador_global

        int prioridad;
        if(escenario == 3){                 //Este fragmento maneja los escenarios
            prioridad = 1;                  //En el escenario 3 son todos de alta prioridad
        } else if(escenario == 4){          //En el escenario 4 el primero es de baja prioridad y
            prioridad = (id == 0) ? 0 : 1;  //y el resto de alta prioridad;
        }else{
            prioridad = rand() % 2;
        }
        Paquete p(id,prioridad); //le asigna a paquete el id y un numero randome en 1 y 0

        std::this_thread::sleep_for(std::chrono::milliseconds(T_WAITING)); //espera 90 milisegundos
        mtx_waiting.lock(); //bloquea el waiting (estanteria)
        waiting.agregarPaquete(p); //agrega el paquete a la estanteria
        mtx_waiting.unlock(); // libera el waiting

        signal(hay_paquetes_waiting); // avisa que hay un nuevo paquete disponible en WaitingQueue
        mtx_cout.lock();
        cout << "[PRODUCTOR] Generado paquete "
             << p.getId()
             << " prioridad "
             << p.getPrioridad()
             << endl;
        mtx_cout.unlock();
    }

    mtx_productores.lock();

    productoresFinalizados++;

    if(productoresFinalizados ==
            cantidadProductores)
    {
        produccionFinalizada = true;

        signal(hay_paquetes_waiting); //despierta al despachador por si estaba dormido
        mtx_cout.lock();
        cout << "\n[INFO] Produccion finalizada\n"
             << endl;
        mtx_cout.unlock();
    }

    mtx_productores.unlock();
}

//mueve paquetes de waiting a processing (estanteria -> cinta transp)
void despachador(WaitingQueue& waiting, ProcessingQueue& processing)
{
    while(true)
    {
        wait(hay_paquetes_waiting); //espera si no hay paquetes en el waiting consume 1 permiso del semáforo

        mtx_waiting.lock();
        bool waitingVacia = waiting.vacia();
        mtx_waiting.unlock();

        if(produccionFinalizada && // Si ya no habrá ms paquetes y WaitingQueue quedó vacía, el despachador puede finalizar.
                waitingVacia)
        {
            if(escenario == 2){
                for(int i = 0; i < cantidadConsumidores; i++){
                    signal(hay_paquetes_cinta);

                }
            }
            mtx_cout.lock();
            cout << "\n[INFO] Despachador finalizado\n"
                 << endl;
            mtx_cout.unlock();

            break;
        }
        wait(hay_espacio_cinta); //espera si no hay espacio en la cinta

        mtx_waiting.lock(); //accede a la seccion critica del waiting
        Paquete p = waiting.obtenerSiguientePaquete(); //asigna el siguiente paquete
        mtx_waiting.unlock(); //sale de la seccion critica apra que otro accede

        // Acumula el tiempo que el paquete estuvo en WaitingQueue para las metricas finales
        long long espera = p.getTiempoEsperaMs();
        mtx_metricas.lock();
        if(p.getPrioridad() == 1)
        {
            tiempoEsperaAltaTotal += espera;
            cantidadAltaProcesada++;
        }
        else
        {
            tiempoEsperaBajaTotal += espera;
            cantidadBajaProcesada++;
        }
        mtx_metricas.unlock();

        p.setIngresoCinta(std::chrono::steady_clock::now()); // registra el instante en que el paquete entra a la cinta(guarda marca detiempo)

        mtx_processing.lock(); //accede a la seccion critica del processing
        processing.agregarPaquete(p); //seccion critica  agrega el paquete a la cinta
        mtx_processing.unlock();// sale de la seccion critica apra que otro accede

        signal(hay_paquetes_cinta); //avisa que hay un nuevo paquete en la cinta

        std::this_thread::sleep_for(std::chrono::milliseconds(T_DESPACHO)); //espera 420ms
        mtx_cout.lock();
        cout << "[DESPACHADOR] Enviado paquete "
             << p.getId()
             << " a ProcessingQueue"
             << endl;
        mtx_cout.unlock();
    }
}

//Toma paquetes de la cinta
void consumidor(ProcessingQueue& processing)
{
    while(true)
    {
        wait(hay_paquetes_cinta);

        mtx_processing.lock();
        // Verificamos condición de salida ANTES de intentar obtener paquete
        if(processing.vacia() &&
           produccionFinalizada &&
           consumidos == paquetesTotales) {
            mtx_processing.unlock();
            mtx_cout.lock();
            cout << "[INFO] Consumidor finalizado" << endl;
            mtx_cout.unlock();
            break;
        }

        Paquete p = processing.obtenerPaquete();
        mtx_processing.unlock();

        auto tiempoEnCinta = std::chrono::steady_clock::now() - p.getIngresoCinta();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tiempoEnCinta).count();

        if(ms < T_CINTA) {
            std::this_thread::sleep_for(std::chrono::milliseconds(T_CINTA - ms));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(T_CONSUMO)); // simula procesamiento

        signal(hay_espacio_cinta); // libera espacio en la cinta

        mtx_consumidos.lock();
        consumidos++;
        bool esUltimo = (produccionFinalizada && consumidos == paquetesTotales);
        mtx_consumidos.unlock();
        if(esUltimo){
            for(int i = 0; i < cantidadConsumidores - 1; i++) {
                signal(hay_paquetes_cinta);
            }
                mtx_cout.lock();
                cout << "[INFO] Consumidor finalizado" << endl;
                mtx_cout.unlock();
                break;
        }
        mtx_cout.lock();
        cout << "Procesado paquete " << p.getId() << endl;
        mtx_cout.unlock();
    }
}
