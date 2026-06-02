#include <iostream>
#include "WaitingQueue.h"
#include "ProcessingQueue.h"
#include "Semaforo.h"
#include <mutex>
#include <thread>
#include <chrono>
using namespace std;

Semaforo hay_paquetes_waiting;
Semaforo hay_espacio_cinta;
Semaforo hay_paquetes_cinta;
mutex mtx_waiting;
mutex mtx_processing;
mutex mtx_contador;

int contador_global = 0;

void productor (WaitingQueue& waiting, int cantPaquetes);
void despachador(WaitingQueue& waiting, ProcessingQueue& processing);
void consumidor(ProcessingQueue& processing);
int main()
{
    init(hay_paquetes_waiting,0);
    init(hay_espacio_cinta,5);
    init(hay_paquetes_cinta,0);

    WaitingQueue waiting;
    ProcessingQueue processing;

    waiting.agregarPaquete(Paquete(1, 0));
    waiting.agregarPaquete(Paquete(2, 1));
    waiting.agregarPaquete(Paquete(3, 0));
    waiting.agregarPaquete(Paquete(4, 1));

    cout << "Altas: " << waiting.cantidadAlta() << endl;
    cout << "Bajas: " << waiting.cantidadBaja() << endl;

    cout << endl;

    Paquete p = waiting.obtenerSiguientePaquete();

    cout << "Se obtuvo paquete ID: "
         << p.getId()
         << " Prioridad: "
         << p.getPrioridad()
         << endl;

    return 0;
}
void productor (WaitingQueue& waiting, int cantPaquetes){
    for(int i = 0; i < cantPaquetes ; i++){

        mtx_contador.lock();
        int id = contador_global;
        contador_global++;
        mtx_contador.unlock();

        Paquete p(id,rand()%2);

        std::this_thread::sleep_for(std::chrono::milliseconds(90));
        mtx_waiting.lock();
        waiting.agregarPaquete(p);
        mtx_waiting.unlock();

        signal(hay_paquetes_waiting);
    }
}
void despachador(WaitingQueue& waiting, ProcessingQueue& processing){
    while(true){
        wait(hay_paquetes_waiting);
        wait(hay_espacio_cinta);

        mtx_waiting.lock();
        Paquete p = waiting.obtenerSiguientePaquete();
        mtx_waiting.unlock();

        p.setIngresoCinta(std::chrono::steady_clock::now());

        mtx_processing.lock();
        processing.agregarPaquete(p);
        mtx_processing.unlock();

        signal(hay_paquetes_cinta);

        std::this_thread::sleep_for(std::chrono::milliseconds(420));
    }
}
void consumidor(ProcessingQueue& processing){

}
