#ifndef WAITINGQUEUE_H
#define WAITINGQUEUE_H
#include "Paquete.h"
#include <queue>
#include <chrono>

//La estanteria de paquetes
class WaitingQueue
{
private:
    std::queue<Paquete> colaAlta;
    std::queue<Paquete> colaBaja;



public:
    void agregarPaquete(const Paquete& paquete);

    bool hayPaquetesAlta() const;
    bool hayPaquetesBaja() const;
    bool vacia() const;

    bool hayPaquetePromovido() const;
    Paquete obtenerSiguientePaquete();

    int cantidadAlta() const;
    int cantidadBaja() const;
};

#endif
