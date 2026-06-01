#include "WaitingQueue.h"

void WaitingQueue::agregarPaquete(const Paquete& paquete)
{
    if(paquete.getPrioridad() == 1)
    {
        colaAlta.push(paquete);
    }
    else
    {
        colaBaja.push(paquete);
    }
}

bool WaitingQueue::hayPaquetesAlta() const
{
    return !colaAlta.empty();
}

bool WaitingQueue::hayPaquetesBaja() const
{
    return !colaBaja.empty();
}

Paquete WaitingQueue::obtenerSiguientePaquete()
{
    if(!colaAlta.empty())
    {
        Paquete paquete = colaAlta.front();
        colaAlta.pop();
        return paquete;
    }

    Paquete paquete = colaBaja.front();
    colaBaja.pop();
    return paquete;
}

int WaitingQueue::cantidadAlta() const
{
    return colaAlta.size();
}

int WaitingQueue::cantidadBaja() const
{
    return colaBaja.size();
}
