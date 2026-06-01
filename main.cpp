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

void despachador(WaitingQueue& waiting, ProcessingQueue& processing);

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
void despachador(WaitingQueue& waiting, ProcessingQueue& processing){
    // Nexo entre las dos queue
    //TODO
}
