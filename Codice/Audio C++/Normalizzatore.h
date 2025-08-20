#pragma once

#include "SmussamentoEsponenziale.h"

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

            // assert(dati[i + offset] >= -1 && dati[i + offset] <= 1);
        }
    }

    double scala;
};

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

        scala.NuovoValore(1.0 / massimo);

        for (size_t i = 0; i < numeroCampioni; ++i)
        {
            dati[i + offset] *= scala.Smussa();

            // assert(dati[i + offset] >= -1 && dati[i + offset] <= 1);
        }
    }

  private:
    SmussamentoEsponenziale scala{ 0.1 };
};
