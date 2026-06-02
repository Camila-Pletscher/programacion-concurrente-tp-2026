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

bool WaitingQueue::vacia() const
{
    return colaAlta.empty() &&
           colaBaja.empty();
}

bool WaitingQueue::hayPaquetesAlta() const
{
    return !colaAlta.empty();
}

bool WaitingQueue::hayPaquetesBaja() const
{
    return !colaBaja.empty();
}

bool WaitingQueue::hayPaquetePromovido() const
{
    if(colaBaja.empty()) return false;
    auto tiempoEspera = std::chrono::steady_clock::now() - colaBaja.front().getFechaCreacion();
    return std::chrono::duration_cast<std::chrono::milliseconds>(tiempoEspera).count() >= 6000;
}


Paquete WaitingQueue::obtenerSiguientePaquete()
{
    if(hayPaquetePromovido())
    {
        Paquete paquete = colaBaja.front();
        colaBaja.pop();
        return paquete;
    }
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
