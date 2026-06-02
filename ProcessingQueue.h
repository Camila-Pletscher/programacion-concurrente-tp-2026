#ifndef PROCESSINGQUEUE_H_INCLUDED
#define PROCESSINGQUEUE_H_INCLUDED

#include <queue>
#include "Paquete.h"

const int CAPACIDAD_MAX = 5;

//Cinta transportadora
class ProcessingQueue{
private:
    std::queue<Paquete> cola;
public:
    void agregarPaquete(const Paquete& paquete);
    bool vacia() const;
    Paquete obtenerPaquete();

    int cantidad() const;

};

#endif // PROCESSINGQUEUE_H_INCLUDED
