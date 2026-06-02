#ifndef PROCESSINGQUEUE_H_INCLUDED
#define PROCESSINGQUEUE_H_INCLUDED

#include <queue>
#include "Paquete.h"
const int CAPACIDAD_MAX = 5;

class ProcessingQueue{
private:
    std::queue<Paquete> cola;
public:
    void agregarPaquete(const Paquete& paquete);

    Paquete obtenerPaquete();

    int cantidad() const;

};

#endif // PROCESSINGQUEUE_H_INCLUDED
