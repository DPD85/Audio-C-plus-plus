#pragma once

#include "CostantiEdAltro.h"

class Clock
{
    using hrc = std::chrono::high_resolution_clock;

  public:
    /// @brief Inizializza un clock col periodo specificato
    /// @param periodo_ periodo del clock espresso in secondi
    Clock(double periodo_)
    {
        periodo = periodo_ * 1000.0;

        // Cambia la risoluzione minima per i timer periodici, questo cambia la frequenza con cui il task manager del
        // S.O programma l'esecuzione del processo e dei suoi thread, per tanto influenza la precisione della funzione
        // Sleep().
        timeBeginPeriod(static_cast<unsigned int>(sogliaSleep));
    }

    ~Clock()
    {
        // Ripristino la risoluzione dei timer e la programmazione dell'esecuzione del processo da parte dello scheduler
        // del S.O. ai valori precedenti
        timeEndPeriod(static_cast<unsigned int>(sogliaSleep));
    }

    void Avvia()
    {
        tStart = hrc::now();
    }

    /// @brief Aspetta fino al ticchettio successivo del clock tenendo in considerazione il tempo trascorso dalla
    /// chiamata precedete.
    /// @return Il tempo corrispondente alla durata del ticchettio appena passato espresso in secondi. Se il tempo è
    /// negativo significa che la durata del ticchettio è stata superiore al periodo del clock.
    double AspettaTicchettioSuccessivo()
    {
        hrc::time_point tStop = hrc::now();

        double dt = duration_cast<DurataMillisecondi>(tStop - tStart).count();

        double attesa  = periodo - dt; // Tempo per il quale è necessario aspettare
        attesa        -= sogliaSleep;  // Tolgo la precisione della funzione Sleep

        // Se devo aspettare per un tempo maggiore della precisione della sleep allora la uso riducendo il busy wait
        if (attesa >= 0) std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<unsigned long>(attesa)));

        // Aspetto per il tempo restante (busy wait)
        do
        {
            hrc::time_point tStop = hrc::now();

            attesa = duration_cast<DurataMillisecondi>(tStop - tStart).count();
        }
        while (attesa <= periodo);

        tStart = hrc::now();

        return attesa * 1000.0;
    }

  private:
    // Se il tempo restante prima del prossimo ticchettio supera questo valore allora sarà usata la funzione Sleep()
    // altrimenti verrà effettuato un busy wait.
    // Attenzione: la soglia deve essere superiore o uguale al minimo periodo supportato per i timer dal S.O. ed alla
    // precisione configurata per questo processo.
    static const constexpr double sogliaSleep = 3.0; // [ms]

    double periodo; // [ms]
    hrc::time_point tStart;
};
