#pragma once

#include "CostantiEdAltro.h"
#include "Inviluppo.h"
#include "Oscillatori.h"

class StrumentoMusicale
{
  public:
    virtual ~StrumentoMusicale() = default;

    virtual void InizioNota(Note nota) = 0;
    virtual void FineNota(Note nota)   = 0;

    /// @brief Restituisce il campione audio successivo dello strumento musicale. Il campione è sempre compreso
    /// nell'intervallo [-1, 1]
    virtual double Campione() = 0;
};

namespace StrumentiMusicali
{
    class SenzaSerieArmonica: public StrumentoMusicale
    {
      private:
        std::array<Oscillatori::OndaSinusoidale, Note::NumeroNote> note = {
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaDo),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaDoDiesis),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaRe),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaReDiesis),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaMi),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaFa),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaFaDiesis),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaSol),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaSolDiesis),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaLa),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaLaDiesis),
            Oscillatori::OndaSinusoidale(Costanti::FrequenzaSi),
        };

        // La durata della fase di rilascio diminuisce con l'aumentare della frequenza della nota
        std::array<InviluppoADSR, Note::NumeroNote> inviluppi = {
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 0),  // do
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 1),  // do#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 2),  // re
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 3),  // re#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 4),  // mi
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 5),  // fa
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 6),  // fa#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 7),  // sol
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 8),  // sol#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 9),  // la
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 10), // la#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 11), // si
        };

        virtual void InizioNota(Note nota)
        {
            inviluppi[nota].InizioNota();
        }

        virtual void FineNota(Note nota)
        {
            inviluppi[nota].FineNota();
        }

        virtual double Campione()
        {
            double valore = 0;

            for (size_t i = 0; i < Note::NumeroNote; ++i)
                valore += inviluppi[i].Computa() * note[i].Campione();

            return valore;
        }
    };

    class StrumentoX: public StrumentoMusicale
    {
      private:
        // Numero di segnali che compongono la serie armonica di una nota
        static const constexpr size_t LunghezzaSerieArmonica = 4;

        // Ampiezze dei segnali che formano le serie armoniche delle note [0, 1]
        std::array<double, LunghezzaSerieArmonica> ampiezze = CreaListaNormalizzata(1.0, 0.25, 0.05, 0.01);

        using Onde = std::array<Oscillatori::OndaSinusoidale, LunghezzaSerieArmonica>;

        std::array<Onde, Note::NumeroNote> note;

        // La durata della fase di rilascio diminuisce con l'aumentare della frequenza della nota
        std::array<InviluppoADSR, Note::NumeroNote> inviluppi = {
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 0),  // do
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 1),  // do#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 2),  // re
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 3),  // re#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 4),  // mi
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 5),  // fa
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 6),  // fa#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 7),  // sol
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 8),  // sol#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 9),  // la
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 10), // la#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 11), // si
        };

      public:
        StrumentoX()
        {
            // ----- Calcolo le frequenze dei segnali che formano le serie armoniche delle note

            for (size_t i = 0; i < Note::NumeroNote; ++i)
                for (size_t j = 0; j < LunghezzaSerieArmonica; ++j)
                    note[i][j].Frequenza((j + 1) * Costanti::FrequenzeNote[i]);
        }

        virtual void InizioNota(Note nota)
        {
            inviluppi[nota].InizioNota();
        }

        virtual void FineNota(Note nota)
        {
            inviluppi[nota].FineNota();
        }

        virtual double Campione()
        {
            double valore = 0;

            for (size_t i = 0; i < Note::NumeroNote; ++i)
            {
                double valoreNota = 0;

                for (size_t j = 0; j < LunghezzaSerieArmonica; ++j)
                    valoreNota += ampiezze[j] * note[i][j].Campione();

                valore += valoreNota * inviluppi[i].Computa();
            }

            return valore;
        }
    };

    class StrumentoY: public StrumentoMusicale
    {
      private:
        std::array<Oscillatori::OndaQuadra, Note::NumeroNote> note = {
            Oscillatori::OndaQuadra(Costanti::FrequenzaDo),        Oscillatori::OndaQuadra(Costanti::FrequenzaDoDiesis),
            Oscillatori::OndaQuadra(Costanti::FrequenzaRe),        Oscillatori::OndaQuadra(Costanti::FrequenzaReDiesis),
            Oscillatori::OndaQuadra(Costanti::FrequenzaMi),        Oscillatori::OndaQuadra(Costanti::FrequenzaFa),
            Oscillatori::OndaQuadra(Costanti::FrequenzaFaDiesis),  Oscillatori::OndaQuadra(Costanti::FrequenzaSol),
            Oscillatori::OndaQuadra(Costanti::FrequenzaSolDiesis), Oscillatori::OndaQuadra(Costanti::FrequenzaLa),
            Oscillatori::OndaQuadra(Costanti::FrequenzaLaDiesis),  Oscillatori::OndaQuadra(Costanti::FrequenzaSi),
        };

        // La durata della fase di rilascio diminuisce con l'aumentare della frequenza della nota
        std::array<InviluppoADSR, Note::NumeroNote> inviluppi = {
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 0),  // do
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 1),  // do#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 2),  // re
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 3),  // re#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 4),  // mi
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 5),  // fa
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 6),  // fa#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 7),  // sol
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 8),  // sol#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 9),  // la
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 10), // la#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 11), // si
        };

        virtual void InizioNota(Note nota)
        {
            inviluppi[nota].InizioNota();
        }

        virtual void FineNota(Note nota)
        {
            inviluppi[nota].FineNota();
        }

        virtual double Campione()
        {
            double valore = 0;

            for (size_t i = 0; i < Note::NumeroNote; ++i)
                valore += inviluppi[i].Computa() * note[i].Campione();

            return valore;
        }
    };
} // namespace StrumentiMusicali
