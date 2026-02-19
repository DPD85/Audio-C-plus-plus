#pragma once

#include "CostantiEdAltro.h"

/// @brief %Clock utile per eseguire operazioni periodiche.
class Clock
{
    using hrc = std::chrono::high_resolution_clock;

  public:
    /// @brief Inizializza il clock col periodo specificato.
    /// @param periodo_ Il periodo del clock. [s]
    Clock(double periodo_)
    {
        periodo = periodo_ * 1000.0;

#ifdef WIN32
        // Cambia la risoluzione minima per i timer periodici, questo cambia la frequenza con cui il task manager del
        // S.O programma l'esecuzione del processo e dei suoi thread, per tanto influenza la precisione della funzione
        // Sleep().
        timeBeginPeriod(static_cast<unsigned int>(sogliaSleep));
#endif
    }

    ~Clock()
    {
#ifdef WIN32
        // Ripristino la risoluzione dei timer e la programmazione dell'esecuzione del processo da parte dello scheduler
        // del S.O. ai valori precedenti
        timeEndPeriod(static_cast<unsigned int>(sogliaSleep));
#endif
    }

    /// @brief Avvia il clock, il quale comincerà a contare il tempo a partire da questo istante.
    void Avvia()
    {
        tStart = hrc::now();
    }

    /// @brief Aspetta fino al ticchettio successivo del clock tenendo in considerazione il tempo trascorso dalla
    /// chiamata precedete.
    /// @return Il tempo corrispondente alla durata del ticchettio appena passato [s]. Se il tempo è negativo significa
    /// che la durata del ticchettio è stata superiore al periodo del clock.
    double AspettaTicchettioSuccessivo()
    {
        const hrc::time_point tStop = hrc::now();

        const double dt = duration_cast<DurataMillisecondi>(tStop - tStart).count();

        double attesa  = periodo - dt; // Tempo per il quale è necessario aspettare
        attesa        -= sogliaSleep;  // Tolgo la precisione della funzione Sleep

        // Se devo aspettare per un tempo maggiore della precisione della sleep allora la uso riducendo il busy wait
        if (attesa >= 0) std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<unsigned long>(attesa)));

        // Aspetto per il tempo restante (busy wait)
        do
            attesa = duration_cast<DurataMillisecondi>(hrc::now() - tStart).count();
        while (attesa <= periodo);

        tStart = hrc::now();

        return attesa / 1000.0;
    }

    /// @brief Restituisce il periodo del clock. [s]
    double Periodo() const
    {
        return periodo * 1000.0;
    }

    /// @brief Imposta il periodo del clock.
    /// @param periodo_ Il nuovo periodo del clock. [s]
    /// @warning Questo metodo non è sincronizzato con il metodo AspettaTicchettioSuccessivo()
    void Periodo(double periodo_)
    {
        periodo = periodo_ * 1000.0;
    }

    /// @brief Aspetta per un certo tempo.
    /// @param durata Il tempo per il quale attendere. [s]
    /// @return Il tempo corrispondente alla durata dell'attesa appena effettuata [s].
    static double Aspetta(double durata)
    {
        const hrc::time_point tStart = hrc::now();

        durata = durata * 1000.0; // [ms]

        // Tolgo la precisione della funzione Sleep
        double attesa = durata - sogliaSleep;

        // Se devo aspettare per un tempo maggiore della precisione della sleep allora la uso riducendo il busy wait
        if (attesa >= 0) std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<unsigned long>(attesa)));

        // Aspetto per il tempo restante (busy wait)
        do
            attesa = duration_cast<DurataMillisecondi>(hrc::now() - tStart).count();
        while (attesa <= durata);

        return attesa / 1000.0;
    }

  private:
    // Se il tempo restante prima del prossimo ticchettio supera questo valore allora sarà usata la funzione sleep()
    // altrimenti verrà effettuato un busy wait.
#ifdef WIN32
    // Attenzione: la soglia deve essere superiore o uguale al minimo periodo supportato per i timer dal S.O. ed alla
    // precisione configurata per questo processo.
    static const constexpr double sogliaSleep = 3.0; // [ms]
#else
    // Su Linux la funzione sleep() è più precisa ed il tempo massimo del busy wait può essere ridotto.
    static const constexpr double sogliaSleep = 0.5; // [ms]
#endif

    double periodo; // [ms]
    hrc::time_point tStart;
};
