#pragma once

#include "CostantiEdAltro.h"
#include "Filtri.h"

/// @brief Cambia il volume in maniera continua utilizzando uno smussamento esponenziale
class Volume
{
  public:
    /// @brief Inizializza il controllo del volume
    /// @param _fattore Fattore di smussamento: più è grande, maggiore è lo smussamento [0, +∞]
    /// @param valoreIniziale Volume iniziale [0, 1]
    Volume(double _fattore, double valoreIniziale = 0): smussatore(_fattore, valoreIniziale)
    {
        nuovoValore.store(valoreIniziale);
    }

    double Valore() const
    {
        return smussatore.Valore();
    }

    void Valore(double _valore)
    {
        nuovoValore.store(_valore);
    }

    double Smussa() noexcept
    {
        double _nuovoValore = nuovoValore.load();
        return smussatore.Smussa(_nuovoValore);
    }

    void Reset(double _nuovoValore = 0)
    {
        smussatore.Reset(_nuovoValore);
        nuovoValore.store(_nuovoValore);
    }

  private:
    std::atomic<double> nuovoValore{ 0 };
    SmussamentoEsponenziale smussatore;
};
