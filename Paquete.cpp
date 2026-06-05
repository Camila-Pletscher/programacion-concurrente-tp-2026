#include "Paquete.h"

Paquete::Paquete(int id, int prioridad)
{
    this->id = id;
    this->prioridad = prioridad;
    this->fechaCreacion = std::chrono::steady_clock::now();
}

int Paquete::getId() const
{
    return id;
}

int Paquete::getPrioridad() const
{
    return prioridad;
}

std::chrono::steady_clock::time_point Paquete::getFechaCreacion() const
{
    return fechaCreacion;
}
std::chrono::steady_clock::time_point Paquete::getIngresoCinta() const{
    return ingresoCinta;
}
void Paquete::setIngresoCinta(std::chrono::steady_clock::time_point nuevoIngresoCinta){
    ingresoCinta = nuevoIngresoCinta;
}
long long Paquete::getTiempoEsperaMs() const{
    auto tiempoEspera = std::chrono::steady_clock::now() - fechaCreacion;
    return std::chrono::duration_cast<std::chrono::milliseconds>(tiempoEspera).count();
}
