#pragma once

#include "CostantiEdAltro.h"
#include "Inviluppo.h"
#include "Oscillatori.h"

class StrumentoMusicale
{
  public:
    virtual void InizioNota(Note nota) = 0;
    virtual void FineNota(Note nota)   = 0;

    /// @brief Restituisce il campione audio successivo dello strumento musicale. Il campione è sempre compreso
    /// nell'intervallo [-1, 1]
    virtual double Campione() = 0;
};

class StrumentoMusicaleSenzaSerieArmonica: public StrumentoMusicale
{
  private:
    std::array<Oscillatori::OndaSinusoidale, Note::NumeroNote> note = {
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaDo),  Oscillatori::OndaSinusoidale(Costanti::FrequenzaRe),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaMi),  Oscillatori::OndaSinusoidale(Costanti::FrequenzaFa),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaSol), Oscillatori::OndaSinusoidale(Costanti::FrequenzaLa),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaSi),
    };

    // La durata della fase di rilascio diminuisce con l'aumentare della frequenza della nota
    std::array<InviluppoADSR, Note::NumeroNote> inviluppi = {
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 0), // do
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 1), // re
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 2), // mi
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 3), // fa
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 4), // sol
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 5), // la
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 6), // si
    };

  public:
    void InizioNota(Note nota)
    {
        inviluppi[nota].InizioNota();
    }

    void FineNota(Note nota)
    {
        inviluppi[nota].FineNota();
    }

    double Campione()
    {
        double valore = 0;

        for (size_t i = 0; i < Note::NumeroNote; ++i)
            valore += inviluppi[i].Computa() * note[i].Campione();

        return valore;
    }
};

class StrumentoMusicaleX: public StrumentoMusicale
{
  private:
    // Numero di segnali che compongono la serie armonica di una nota
    static const constexpr size_t LunghezzaSerieArmonica = 4;

    // Ampiezze dei segnali che formano le serie armoniche delle note [0, 1]
    std::array<double, LunghezzaSerieArmonica> ampiezze{ 1.0, 0.25, 0.05, 0.01 };

    using Onde = std::array<Oscillatori::OndaSinusoidale, LunghezzaSerieArmonica>;

    std::array<Onde, Note::NumeroNote> note;

    // La durata della fase di rilascio diminuisce con l'aumentare della frequenza della nota
    std::array<InviluppoADSR, Note::NumeroNote> inviluppi = {
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 0), // do
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 1), // re
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 2), // mi
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 3), // fa
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 4), // sol
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 5), // la
        InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 6), // si
    };

  public:
    StrumentoMusicaleX()
    {
        // ----- Normalizzo le ampiezze dei segnali delle serie armoniche

        double n = 0;
        for (double &ampiezza : ampiezze)
            n += ampiezza;

        for (double &ampiezza : ampiezze)
            ampiezza *= 1.0 / n;

        // ----- Calcolo le frequenze dei segnali che formano le serie armoniche delle note

        for (size_t i = 0; i < Note::NumeroNote; ++i)
            for (size_t j = 0; j < LunghezzaSerieArmonica; ++j)
                note[i][j].Frequenza((j + 1) * Costanti::FrequenzeNote[i]);
    }

    void InizioNota(Note nota)
    {
        inviluppi[nota].InizioNota();
    }

    void FineNota(Note nota)
    {
        inviluppi[nota].FineNota();
    }

    double Campione()
    {
        double valore = 0;

        for (size_t i = 0; i < Note::NumeroNote; ++i)
        {
            double valoreNota = 0;

            for (size_t j = 0; j < LunghezzaSerieArmonica; ++j)
                valoreNota += ampiezze[j] * note[i][j].Campione();

            valoreNota *= inviluppi[i].Computa();

            valore += valoreNota;
        }

        return valore;
    }
};
