#pragma once

#include "CostantiEdAltro.h"
#include "Filtri.h"

/// @brief Cambia il volume in maniera continua utilizzando uno smussamento esponenziale
class Volume
{
  public:
    /// @brief Inizializza il controllo del volume
    /// @param fattore_ Fattore di smussamento: più è grande, maggiore è lo smussamento [0, +∞]
    /// @param valoreIniziale Volume iniziale [0, 1]
    Volume(double fattore_, double valoreIniziale = 0): smussatore(fattore_, valoreIniziale)
    {
        nuovoValore.store(valoreIniziale);
    }

    double Valore() const
    {
        return smussatore.Valore();
    }

    void Valore(double valore_)
    {
        nuovoValore.store(valore_);
    }

    double Smussa() noexcept
    {
        double nuovoValore_ = nuovoValore.load();
        return smussatore.Computa(nuovoValore_);
    }

    void Reset(double nuovoValore_ = 0)
    {
        smussatore.Reset(nuovoValore_);
        nuovoValore.store(nuovoValore_);
    }

  private:
    std::atomic<double> nuovoValore{ 0 };
    Filtri::SmussamentoEsponenziale smussatore;
};
