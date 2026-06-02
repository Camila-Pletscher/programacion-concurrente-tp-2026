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
int cantidadProductores = 1;
int cantidadConsumidores = 2;
int paquetesTotales = 20;

// Semáforos de sincronización
Semaforo hay_paquetes_waiting; // paquetes disponibles en WaitingQueue
Semaforo hay_espacio_cinta;    // espacios libres en ProcessingQueue
Semaforo hay_paquetes_cinta;   // paquetes disponibles en ProcessingQueue

// Mutex de exclusión mutua
mutex mtx_waiting;     // protege WaitingQueue
mutex mtx_processing;  // protege ProcessingQueue
mutex mtx_contador;    // protege contador_global

// Generador de IDs únicos para los paquetes
int contador_global = 0;

void productor (WaitingQueue& waiting, int cantPaquetes);
void despachador(WaitingQueue& waiting, ProcessingQueue& processing);
void consumidor(ProcessingQueue& processing);

int main()
{

    vector<thread> productores;
    vector<thread> consumidores;

    // Cantidad de paquetes que producirá cada productor
    int paquetesPorProductor = paquetesTotales / cantidadProductores;

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
        productores.emplace_back(
            productor,
            ref(waiting),
            paquetesPorProductor
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

        Paquete p(id,rand()%2); //le asigna a paquete el id y un numero randome en 1 y 0

        std::this_thread::sleep_for(std::chrono::milliseconds(90)); //espera 90 milisegundos
        mtx_waiting.lock(); //bloquea el waiting (estanteria)
        waiting.agregarPaquete(p); //agrega el paquete a la estanteria
        mtx_waiting.unlock(); // libera el waiting

        signal(hay_paquetes_waiting); // avisa que hay un nuevo paquete disponible en WaitingQueue

        cout << "[PRODUCTOR] Generado paquete "
             << p.getId()
             << " prioridad "
             << p.getPrioridad()
             << endl;
    }

    mtx_productores.lock();

    productoresFinalizados++;

    if(productoresFinalizados ==
            cantidadProductores)
    {
        produccionFinalizada = true;

        signal(hay_paquetes_waiting); //despierta al despachador por si estaba dormido

        cout << "\n[INFO] Produccion finalizada\n"
             << endl;
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
        if(produccionFinalizada && // Si ya no habrá más paquetes y WaitingQueue quedó vacía, el despachador puede finalizar.
                waitingVacia)
        {
            for(int i = 0; i < cantidadConsumidores; i++)
            {
                signal(hay_paquetes_cinta); // Despierta a todos los consumidores para que puedan verificar la condición de finalización.
            }
            cout << "\n[INFO] Despachador finalizado\n"
                 << endl;


            break;
        }
        wait(hay_espacio_cinta); //espera si no hay espacio en la cinta

        mtx_waiting.lock(); //accede a la seccion critica del waiting
        Paquete p = waiting.obtenerSiguientePaquete(); //asigna el siguiente paquete
        mtx_waiting.unlock(); //sale de la seccion critica apra que otro accede

        p.setIngresoCinta(std::chrono::steady_clock::now()); // registra el instante en que el paquete entra a la cinta(guarda marca detiempo)

        mtx_processing.lock(); //accede a la seccion critica del processing
        processing.agregarPaquete(p); //seccion critica  agrega el paquete a la cinta
        mtx_processing.unlock();// sale de la seccion critica apra que otro accede

        signal(hay_paquetes_cinta); //avisa que hay un nuevo paquete en la cinta

        std::this_thread::sleep_for(std::chrono::milliseconds(420)); //espera 420ms

        cout << "[DESPACHADOR] Enviado paquete "
             << p.getId()
             << " a ProcessingQueue"
             << endl;
    }
}

//Toma paquetes de la cinta
void consumidor(ProcessingQueue& processing)
{
    while(true)
    {
        wait(hay_paquetes_cinta);

        mtx_processing.lock();

        if(produccionFinalizada && // Si la producción terminó y ya no quedan paquetes en ProcessingQueue, este consumidor puede finalizar.
                processing.vacia())
        {
            mtx_processing.unlock();

            cout << "[INFO] Consumidor finalizado"
                 << endl;

            break;
        }

        Paquete p = processing.obtenerPaquete();

        mtx_processing.unlock();

        auto tiempoEnCinta =std::chrono::steady_clock::now() - p.getIngresoCinta();//calcula cuánto tiempo lleva el paquete dentro de la ProcessingQueue

        auto ms = std::chrono::duration_cast <std::chrono::milliseconds>(tiempoEnCinta).count();//convierte el tiempo transcurrido a milisegundos

        if(ms < 550)// garantiza que el paquete permanezca al menos 550 ms en la cinta
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(550 - ms));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(270)); // simula el tiempo de procesamiento del paquete

        signal(hay_espacio_cinta);// libera un espacio de la cinta

        cout // muestra qué paquete terminó de procesarse
                << "Procesado paquete "
                << p.getId()
                << endl;
    }
}
