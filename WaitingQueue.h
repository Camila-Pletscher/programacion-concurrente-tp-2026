#ifndef WAITINGQUEUE_H
#define WAITINGQUEUE_H

#include <queue>
#include "Paquete.h"

class WaitingQueue
{
private:
    std::queue<Paquete> colaAlta;
    std::queue<Paquete> colaBaja;

public:
    void agregarPaquete(const Paquete& paquete);

    bool hayPaquetesAlta() const;
    bool hayPaquetesBaja() const;

    Paquete obtenerSiguientePaquete();

    int cantidadAlta() const;
    int cantidadBaja() const;
};

#endif
