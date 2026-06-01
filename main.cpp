#include <iostream>
#include "WaitingQueue.h"

using namespace std;

int main()
{
    WaitingQueue waiting;

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
