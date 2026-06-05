#ifndef PAQUETE_H
#define PAQUETE_H

#include <chrono>


class Paquete
{
private:
    int id;
    int prioridad;

    std::chrono::steady_clock::time_point fechaCreacion;
    std::chrono::steady_clock::time_point ingresoCinta;
public:
    Paquete(int id, int prioridad);

    int getId() const;
    int getPrioridad() const;

    std::chrono::steady_clock::time_point getFechaCreacion() const;
    std::chrono::steady_clock::time_point getIngresoCinta() const;
    void setIngresoCinta(std::chrono::steady_clock::time_point nuevoIngresoCinta);

    long long getTiempoEsperaMs() const;
};

#endif
