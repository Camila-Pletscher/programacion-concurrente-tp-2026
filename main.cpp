#include <iostream>
#include "WaitingQueue.h"
#include "ProcessingQueue.h"
#include "Semaforo.h"
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;

//Metricas
long long esperaTotalAlta = 0;
long long esperaTotalBaja = 0;
int cantidadAlta = 0;
int cantidadBaja = 0;

//Finalizacion
bool produccionFinalizada = false;
int productoresFinalizados = 0;

//Configuracion
int cantidadProductores;
int cantidadConsumidores;
int paquetesTotales;
int consumidos = 0;
int escenario;

// Tiempos de espera
const int T_WAITING = 90; // 9ms -> 90ms
const int T_DESPACHO = 420; // 42ms -> 420ms
const int T_CINTA = 550; // 55ms -> 550ms
const int T_CONSUMO = 270; // 27ms -> 270ms

// Semáforos de sincronización
Semaforo hay_paquetes_waiting;
Semaforo hay_espacio_cinta;
Semaforo hay_paquetes_cinta;

// Mutex de exclusión mutua
mutex mtx_waiting;
mutex mtx_processing;
mutex mtx_contador;
mutex mtx_consumidos;
mutex mtx_cout;
mutex mtx_productores;
mutex mtx_metricas;

int contador_global = 0;

//Funciones
void productor (WaitingQueue& waiting, int cantPaquetes);
void despachador(WaitingQueue& waiting, ProcessingQueue& processing);
void consumidor(ProcessingQueue& processing);

int main()
{

    vector<thread> productores;
    vector<thread> consumidores;

    int configuracion;

    cout << " ========== CONFIGURACION ==========" << endl;
    cout << " - Configuracion A: 1 Productor y 2 Consumidores" << endl;
    cout << " - Configuracion B: 3 Productores y 1 consumidor" << endl;
    cout << " - Configuracion C: 3 Productores y 3 Consumidores" << endl;
    cout << "Seleccione configuracion (1 al 3): ";

    while(!(cin>>configuracion))
    {
        cout << "Valor no valido, ingrese nuevamente: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }
    switch(configuracion)
    {
    case 1:
        cantidadProductores = 1;
        cantidadConsumidores = 2;
        break;
    case 2:
        cantidadProductores = 3;
        cantidadConsumidores = 1;
        break;
    case 3:
        cantidadProductores = 3;
        cantidadConsumidores = 3;
        break;
    }

    cout << " \n========== ESCENARIO ==========" << endl;
    cout << "1 - Carga masiva (1550 paquetes)" << endl;
    cout << "2 - Vacuidad (0 paquetes)" << endl;
    cout << "3 - Saturacion (8 paquetes alta prioridad)" << endl;
    cout << "4 - Anti-starvation" << endl;
    cout << "Seleccione escenario (1 al 4): ";

    while(!(cin >> escenario))
    {
        cout << "Valor no valido, ingrese nuevamente: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }

    switch(escenario)
    {
    case 1:
        paquetesTotales = 1550;
        break;
    case 2:
        paquetesTotales = 0;
        break;
    case 3:
        paquetesTotales = 8;
        break;
    case 4:
        paquetesTotales = 50;
        break;
    }

    // Cantidad de paquetes que producirá cada productor
    int paquetesPorProductor = paquetesTotales / cantidadProductores;
    int resto = paquetesTotales % cantidadProductores;

    //Inicialización de semáforos
    init(hay_paquetes_waiting,0); // inicialmente no hay paquetes esperando
    init(hay_espacio_cinta,5);    // la cinta tiene 5 espacios libres
    init(hay_paquetes_cinta,0);   // inicialmente no hay paquetes en la cinta

    //Estructuras compartidas del sistema
    WaitingQueue waiting;
    ProcessingQueue processing;

    //Creador de productor/es segun config
    for(int i=0; i<cantidadProductores; i++)
    {
        int extra = (i == 0) ? resto : 0;
        productores.emplace_back(
            productor,
            ref(waiting),
            paquetesPorProductor + extra
        );
    }

    //Creador de despachador
    thread desp(despachador,
                ref(waiting),
                ref(processing));

    //Creador de consumidor/es segun config
    for(int i=0; i<cantidadConsumidores; i++)
    {
        consumidores.emplace_back(
            consumidor,
            ref(processing)
        );
    }

    for(auto& p : productores)
    {
        p.join();
    }

    desp.join();

    for(auto& c : consumidores)
    {
        c.join();
    }

    cout << "\n========== METRICAS ==========\n";

    cout << "Paquetes producidos: "
         << contador_global
         << endl;

    if(cantidadAlta > 0)
    {
        cout << "Espera promedio prioridad ALTA: "
             << (esperaTotalAlta / cantidadAlta)
             << " ms"
             << endl;
    }
    else
    {
        cout << "No se producieron paquetes de prioridad ALTA"<< endl;
    }

    if(cantidadBaja > 0)
    {
        cout << "Espera promedio prioridad BAJA: "
             << (esperaTotalBaja / cantidadBaja)
             << " ms"
             << endl;
    }
    else
    {
        cout << "No se producieron paquetes de prioridad BAJA"<< endl;
    }



    return 0;
}

//Fabrica paquetes y lo agrega a la estanteria/Waiting
void productor (WaitingQueue& waiting, int cantPaquetes)
{
    for(int i = 0; i < cantPaquetes ; i++)
    {

        mtx_contador.lock();
        int id = contador_global;
        contador_global++;
        mtx_contador.unlock();

        int prioridad;
        if(escenario == 3)
        {
            prioridad = 1;
        }
        else if(escenario == 4)
        {
            prioridad = (id == 25) ? 0 : 1;
        }
        else
        {
            prioridad = rand() % 2;
        }
        Paquete p(id,prioridad);

        std::this_thread::sleep_for(std::chrono::milliseconds(T_WAITING));
        mtx_waiting.lock();
        waiting.agregarPaquete(p);
        mtx_waiting.unlock();

        signal(hay_paquetes_waiting);
    }

    mtx_productores.lock();

    productoresFinalizados++;

    if(productoresFinalizados ==
            cantidadProductores)
    {
        produccionFinalizada = true;

        signal(hay_paquetes_waiting);
        mtx_cout.lock();
        cout << "\n[INFO] Produccion finalizada\n"
             << endl;
        mtx_cout.unlock();
    }

    mtx_productores.unlock();
}

//mueve paquetes de waiting a processing (estanteria -> cinta transp)
void despachador(WaitingQueue& waiting, ProcessingQueue& processing)
{
    while(true)
    {
        wait(hay_paquetes_waiting);

        mtx_waiting.lock();
        bool waitingVacia = waiting.vacia();
        mtx_waiting.unlock();

        if(produccionFinalizada &&
                waitingVacia)
        {
            if(escenario == 2)
            {
                for(int i = 0; i < cantidadConsumidores; i++)
                {
                    signal(hay_paquetes_cinta);

                }
            }
            mtx_cout.lock();
            cout << "\n[INFO] Despachador finalizado\n"
                 << endl;
            mtx_cout.unlock();

            break;
        }
        wait(hay_espacio_cinta);

        mtx_waiting.lock();
        Paquete p = waiting.obtenerSiguientePaquete();
        mtx_waiting.unlock();

        auto tiempoEspera = std::chrono::steady_clock::now() - p.getFechaCreacion();

        long long msEspera = std::chrono::duration_cast<std::chrono::milliseconds>(tiempoEspera).count();

        mtx_metricas.lock();

        if(p.getPrioridad() == 1)
        {
            esperaTotalAlta += msEspera;
            cantidadAlta++;
        }
        else
        {
            esperaTotalBaja += msEspera;
            cantidadBaja++;
        }

        mtx_metricas.unlock();

        p.setIngresoCinta(std::chrono::steady_clock::now());

        mtx_processing.lock();
        processing.agregarPaquete(p);
        mtx_processing.unlock();

        signal(hay_paquetes_cinta);

        std::this_thread::sleep_for(std::chrono::milliseconds(T_DESPACHO));
    }
}

//Toma paquetes de la cinta
void consumidor(ProcessingQueue& processing)
{
    while(true)
    {
        wait(hay_paquetes_cinta);

        mtx_processing.lock();

        if(processing.vacia() &&
                produccionFinalizada &&
                consumidos == paquetesTotales)
        {
            mtx_processing.unlock();
            mtx_cout.lock();
            cout << "[INFO] Consumidor finalizado" << endl;
            mtx_cout.unlock();
            break;
        }

        Paquete p = processing.obtenerPaquete();
        mtx_processing.unlock();

        auto tiempoEnCinta = std::chrono::steady_clock::now() - p.getIngresoCinta();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tiempoEnCinta).count();

        if(ms < T_CINTA)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(T_CINTA - ms));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(T_CONSUMO));

        signal(hay_espacio_cinta);

        mtx_consumidos.lock();
        consumidos++;
        bool esUltimo = (produccionFinalizada && consumidos == paquetesTotales);
        mtx_consumidos.unlock();
        if(esUltimo)
        {
            for(int i = 0; i < cantidadConsumidores - 1; i++)
            {
                signal(hay_paquetes_cinta);
            }
            mtx_cout.lock();
            cout << "[INFO] Consumidor finalizado" << endl;
            mtx_cout.unlock();
            break;
        }
    }
}
