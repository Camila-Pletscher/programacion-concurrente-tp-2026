#ifndef PAQUETE_H
#define PAQUETE_H

#include <chrono>

class Paquete
{
private:
    int id;
    int prioridad;

    std::chrono::steady_clock::time_point fechaCreacion;

public:
    Paquete(int id, int prioridad);

    int getId() const;
    int getPrioridad() const;

    std::chrono::steady_clock::time_point getFechaCreacion() const;
};

#endif
