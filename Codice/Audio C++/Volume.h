#pragma once

#include "CostantiEdAltro.h"
#include "Filtri.h"

/// @brief Cambia il volume in maniera continua utilizzando uno smussamento esponenziale.
class Volume
{
  public:
    /// @brief Inizializza il controllo del volume.
    /// @param fattore_ Fattore di smussamento: più è grande, maggiore è lo smussamento. [0, +∞]
    /// @param volumeIniziale Il volume iniziale. [0, 1]
    Volume(double fattore_, double volumeIniziale = 0): smussatore(fattore_, volumeIniziale)
    {
        Reset(volumeIniziale);
    }

    /// @brief Restituisce il valore corrente del volume. [0, 1]
    double Valore() const
    {
        return smussatore.Valore();
    }

    /// @brief Cambia il volume corrente.
    /// @param valore_ Il nuovo volume. [0, 1]
    void Valore(double valore_)
    {
        nuovoValore.store(valore_);
    }

    /// @brief Smussa il cambiamento di volume e calcola il volume attuale.
    /// @return Il valore attuale del volume. [0, 1]
    double Smussa() noexcept
    {
        const double nuovoValore_ = nuovoValore.load();
        return smussatore.Computa(nuovoValore_);
    }

    /// @brief Resetta il volume al valore iniziale.
    /// @param nuovoValore_ Il volume iniziale. [0, 1]
    /// @remark Il volume viene impostato in modo netto ed immediato per tanto cambierà istantaneamente.
    /// @remark Il metodo è sincronizzato col calcolo dell'audio, ovvero col metodo Smussa().
    void Reset(double nuovoValore_ = 0)
    {
        smussatore.Reset(nuovoValore_);
        nuovoValore.store(nuovoValore_);
    }

  private:
    std::atomic<double> nuovoValore;
    Filtri::SmussamentoEsponenziale smussatore;
};
