#include "ProcessingQueue.h"

    void ProcessingQueue::agregarPaquete(const Paquete& paquete){
        cola.push(paquete);
    }

    Paquete ProcessingQueue::obtenerPaquete(){
        Paquete paquete = cola.front();
        cola.pop();
        return paquete;
    }

    int ProcessingQueue::cantidad() const{
        return cola.size();
    }
