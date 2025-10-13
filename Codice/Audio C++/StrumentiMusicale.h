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
    /// @brief Strumento musicale con note pure, senza serie armonica o altri effetti, e gli inviluppi delle note sono
    /// tutti uguali tra di loro.
    class Puro: public StrumentoMusicale
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
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // do
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // do#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // re
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // re#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // mi
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // fa
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // fa#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // sol
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // sol#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // la
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // la#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // si
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
            {
                const double valoreInviluppo = inviluppi[i].Computa();

                // Se la nota è muta la salto
                if (!inviluppi[i].StaSuonando()) continue;

                valore += valoreInviluppo * note[i].Campione();
            }

            return valore;
        }
    };

    /// @brief Strumento musicale con note pure, senza serie armonica o altri effetti, prodotte usando un'onda quadra e
    /// gli inviluppi delle note sono tutti uguali tra di loro.
    class PuroOndaQuadra: public StrumentoMusicale
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
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // do
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // do#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // re
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // re#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // mi
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // fa
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // fa#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // sol
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // sol#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // la
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // la#
            InviluppoADSR(0.02, 0.01, 0.8, 0.5), // si
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
            {
                const double valoreInviluppo = inviluppi[i].Computa();

                // Se la nota è muta la salto
                if (!inviluppi[i].StaSuonando()) continue;

                valore += valoreInviluppo * note[i].Campione();
            }

            return valore;
        }
    };

    /// @brief Pianoforte a 88 tasti
    ///
    /// La serie armonica di tutte le note è composta da tre armonici: la frequenza fondamentale più il 2° e 3°
    /// armonico.
    /// L'inviluppo delle note è lineare ed ha un tempo di decadimento e di rilascio progressivamente più corto con
    /// l'aumentare dell'acutezza della nota.
    class Pianoforte: public StrumentoMusicale
    {
        /* Gli 88 tasti del piano forte vanno dalla nota A0 alla C8, i corrispettivi numeri MIDI vanno da 21 a 108
         */

      private:
        // Velocity di default consigliata nelle specifiche MIDI per uno strumento che non supporta velocity dinamiche
        static const constexpr size_t MIDIVelocity = 60; // [0, 127]
        // Valore scalato tra 0 ed 1 della velocity MIDI
        static const constexpr double Velocity = MIDIVelocity / 127.0; // [0, 1]

        // Numero di segnali che compongono la serie armonica di una nota
        static const constexpr size_t LunghezzaSerieArmonica = 3;

        // Ampiezze dei segnali che formano le serie armoniche delle note [0, 1]
        std::array<double, LunghezzaSerieArmonica> ampiezze =
            CreaListaNormalizzata(0.8, Velocity * 0.3, Velocity * 0.1);

        using Onde = std::array<Oscillatori::OndaSinusoidale, LunghezzaSerieArmonica>;

        std::array<Onde, Note::NumeroNote> note;

        // Contributo al 20% della velocity, [1.0, 1.2]
        static const constexpr double ContributoVel20 = 1.0 + 0.2 * Velocity;
        // Contributo al 10% della velocity, [1.0, 1.1]
        static const constexpr double ContributoVel10 = 1.0 + 0.1 * Velocity;

        // La durata della fase di rilascio e di decadimento diminuisce con l'aumentare della frequenza della nota
        std::array<InviluppoADSR, Note::NumeroNote> inviluppi = {
            // clang-format off
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 60) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 60) * 0.005 * ContributoVel20), // do   MIDI #60
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 61) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 61) * 0.005 * ContributoVel20), // do#  MIDI #61
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 62) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 62) * 0.005 * ContributoVel20), // re   MIDI #62
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 63) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 63) * 0.005 * ContributoVel20), // re#  MIDI #63
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 64) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 64) * 0.005 * ContributoVel20), // mi   MIDI #64
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 65) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 65) * 0.005 * ContributoVel20), // fa   MIDI #65
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 66) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 66) * 0.005 * ContributoVel20), // fa#  MIDI #66
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 67) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 67) * 0.005 * ContributoVel20), // sol  MIDI #67
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 68) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 68) * 0.005 * ContributoVel20), // sol# MIDI #68
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 69) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 69) * 0.005 * ContributoVel20), // la   MIDI #69
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 70) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 70) * 0.005 * ContributoVel20), // la#  MIDI #70
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 71) * 0.002 * ContributoVel10, 0.3 * Velocity, 0.2 + (108 - 71) * 0.005 * ContributoVel20), // si   MIDI #71
            // clang-format on
        };

      public:
        Pianoforte()
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
                const double valoreInviluppo = inviluppi[i].Computa();

                // Se la nota è muta la salto
                if (!inviluppi[i].StaSuonando()) continue;

                // Calcolo la serie armonica della nota
                double valoreNota = ampiezze[0] * note[i][0].Campione();
                for (size_t j = 1; j < LunghezzaSerieArmonica; ++j)
                    valoreNota += ampiezze[j] * note[i][j].Campione();

                // Applico l'inviluppo e la velocity
                valore += valoreNota * valoreInviluppo * Velocity;
            }

            return valore;
        }
    };
}
