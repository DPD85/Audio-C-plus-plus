#pragma once

#include "CostantiEdAltro.h"

class SmussamentoEsponenziale
{
  public:
    SmussamentoEsponenziale(double _fattore, double valoreIniziale = 0)
    {
        fattore = -std::expm1(-1.0 / Costanti::FrequenzaCampionamento / _fattore);
        Reset(valoreIniziale);
    }

    double Valore() const
    {
        return valore;
    }

    void NuovoValore(double _valore)
    {
        nuovoValore.store(_valore);
    }

    double Smussa()
    {
        double _nuovoValore = nuovoValore.load();

        double t  = valore;
        valore   += (_nuovoValore - valore) * fattore;
        return t;
    }

    void Reset(double _nuovoValore = 0)
    {
        valore = _nuovoValore;
        nuovoValore.store(_nuovoValore);
    }

  private:
    double fattore;
    double valore{ 0 };
    std::atomic<double> nuovoValore{ 0 };
};
