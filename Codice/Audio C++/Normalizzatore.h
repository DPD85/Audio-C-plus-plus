#pragma once

#include "Volume.h"

/// @remark L'implementazione è parziale e non terminata, non è pronto per essere usato.
class Normalizzatore
{
  public:
    void Normalizza(dvector &dati)
    {
        Normalizza(dati, dati.size());
    }

    void Normalizza(dvector &dati, size_t numeroCampioni, size_t offset = 0)
    {
        double massimo = std::numeric_limits<double>::min();

        for (size_t i = 0; i < numeroCampioni; ++i)
            massimo = std::max(massimo, std::abs(dati[i + offset]));

        if (massimo <= 1) return;

        scala = 1.0 / massimo;

        for (size_t i = 0; i < numeroCampioni; ++i)
        {
            dati[i + offset] *= scala;
        }
    }

    double scala;
};

/// @remark L'implementazione è parziale e non terminata, non è pronto per essere usato.
class NormalizzatoreSmussato
{
  public:
    void Normalizza(dvector &dati)
    {
        Normalizza(dati, dati.size());
    }

    void Normalizza(dvector &dati, size_t numeroCampioni, size_t offset = 0)
    {
        double massimo = std::numeric_limits<double>::min();

        for (size_t i = 0; i < numeroCampioni; ++i)
            massimo = std::max(massimo, std::abs(dati[i + offset]));

        if (massimo <= 1) return;

        scala.Valore(1.0 / massimo);

        for (size_t i = 0; i < numeroCampioni; ++i)
        {
            dati[i + offset] *= scala.Smussa();
        }
    }

  private:
    Volume scala{ 0.1 };
};
