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

    /// @brief Pianoforte a 88 tasti.
    ///
    /// La serie armonica di tutte le note è composta da otto armoniche.
    /// L'inviluppo delle note è lineare ed ha un tempo di decadimento e di rilascio progressivamente più corto con
    /// l'aumentare dell'acutezza della nota.
    class Pianoforte: public StrumentoMusicale
    {
        /* Gli 88 tasti del piano forte vanno dalla nota A0 alla C8, i corrispettivi numeri MIDI vanno da 21 a 108
         */

      private:
        // Numero di segnali che compongono la serie armonica di una nota
        static const constexpr size_t NumeroArmoniche = 8;

        using Armoniche = std::array<Oscillatori::OndaSinusoidale, NumeroArmoniche>;
        using Ampiezze  = std::array<double, NumeroArmoniche>;

        // Ampiezze base delle armoniche che compongono una nota, [0, 1]
        static const constexpr Ampiezze AmpiezzeArmoniche =
            CreaListaNormalizzata(0.3982, 0.2863, 0.1141, 0.0963, 0.0437, 0.0305, 0.0194, 0.0116);

        // Massimo incremento delle ampiezze delle armoniche di una nota durante la fase di attacco dell'inviluppo.
        static const constexpr double IncrementoAmpiezze = 3.0;

        struct Nota
        {
            Armoniche armoniche;
            Ampiezze ampiezze;
        };

        std::array<Nota, Note::NumeroNote> note;

        // Contributo della velocità tra 0% ed il 20%, [1.0, 1.2]
        static const constexpr double ContributoVel20 = 1.0 + 0.2 * Costanti::VelocitàDefault;
        // Contributo della velocità tra 0% ed il 10%, [1.0, 1.1]
        static const constexpr double ContributoVel10 = 1.0 + 0.1 * Costanti::VelocitàDefault;
        // Livello di sostentamento delle note
        static const constexpr double LivelloSostentamento = 0.2 * Costanti::VelocitàDefault;

        // La durata della fase di rilascio e di decadimento diminuisce con l'aumentare della frequenza della nota
        std::array<InviluppoADSR, Note::NumeroNote> inviluppi = {
            // clang-format off
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 60) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 60) * 0.005 * ContributoVel20), // do   MIDI #60
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 61) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 61) * 0.005 * ContributoVel20), // do#  MIDI #61
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 62) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 62) * 0.005 * ContributoVel20), // re   MIDI #62
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 63) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 63) * 0.005 * ContributoVel20), // re#  MIDI #63
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 64) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 64) * 0.005 * ContributoVel20), // mi   MIDI #64
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 65) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 65) * 0.005 * ContributoVel20), // fa   MIDI #65
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 66) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 66) * 0.005 * ContributoVel20), // fa#  MIDI #66
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 67) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 67) * 0.005 * ContributoVel20), // sol  MIDI #67
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 68) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 68) * 0.005 * ContributoVel20), // sol# MIDI #68
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 69) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 69) * 0.005 * ContributoVel20), // la   MIDI #69
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 70) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 70) * 0.005 * ContributoVel20), // la#  MIDI #70
            InviluppoADSR(0.005 * ContributoVel20, 0.1 + (108 - 71) * 0.002 * ContributoVel10, LivelloSostentamento, 0.2 + (108 - 71) * 0.005 * ContributoVel20), // si   MIDI #71
            // clang-format on
        };

        Riverbero riverbero;

      public:
        Pianoforte(): riverbero(0.85, 0.3, 5000.0)
        {
            // Calcolo le frequenze dei segnali che formano le serie armoniche delle note

            for (size_t i = 0; i < note.size(); ++i)
                for (size_t j = 0; j < NumeroArmoniche; ++j)
                    note[i].armoniche[j].Frequenza((j + 1) * Costanti::FrequenzeNote[i]);
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
                const double inviluppo = inviluppi[i].Computa();

                // Se la nota è muta la salto
                if (!inviluppi[i].StaSuonando()) continue;

                Nota &nota = note[i];

                const InviluppoADSR::Stati stato = inviluppi[i].Stato();
                if (stato == InviluppoADSR::Stati::Attacco)
                {
                    // Converto l'intervallo di variazione dell'inviluppo nella fase di attacco ([0, 1]) nell'intervallo
                    // [1, 1 + IncrementoAmpiezze].
                    const double scala = 1.0 + IncrementoAmpiezze * inviluppo;
                    assert(scala >= 1 && scala <= 1 + IncrementoAmpiezze);

                    AggiornaArmonica(nota.ampiezze, scala);
                }
                else if (stato == InviluppoADSR::Stati::Decadimento)
                {
                    // Converto l'intervallo di variazione dell'inviluppo nella fase di decadimento (che corrisponde a
                    // [1, LivelloSostentamento]) nell'intervallo [1, 0].
                    const double i = (inviluppo - LivelloSostentamento) / (1 - LivelloSostentamento);
                    assert(i >= 0 && i <= 1);
                    // Converto l'intervallo [1, 0] in [1 + IncrementoAmpiezze, 1]
                    const double scala = 1.0 + IncrementoAmpiezze * i;
                    assert(scala >= 1 && scala <= 1 + IncrementoAmpiezze);

                    AggiornaArmonica(nota.ampiezze, scala);
                }

                // Calcolo la serie armonica della nota
                double valoreNota = 0.0;
                for (size_t j = 0; j < NumeroArmoniche; ++j)
                    valoreNota += nota.ampiezze[j] * nota.armoniche[j].Campione();

                // Applico l'inviluppo e la velocità
                valore += valoreNota * inviluppo * Costanti::VelocitàDefault;
            }

            valore = riverbero.Computa(valore);

            return valore;
        }

      private:
        /// @brief Aggiorna le ampiezze delle armoniche di una nota.
        ///
        /// Amplifica o riduce il volume delle armoniche con frequenza più alta.
        /// @param ampiezze Lista delle ampiezze attuali delle armoniche.
        /// @param scala Fattore di scala applicato alle ampiezze delle armoniche.
        void AggiornaArmonica(Ampiezze &ampiezze, const double scala)
        {
            ampiezze[0] = AmpiezzeArmoniche[0];
            ampiezze[1] = AmpiezzeArmoniche[1] * Costanti::VelocitàDefault;
            ampiezze[2] = AmpiezzeArmoniche[2] * Costanti::VelocitàDefault;
            ampiezze[3] = AmpiezzeArmoniche[3] * Costanti::VelocitàDefault;
            ampiezze[4] = AmpiezzeArmoniche[4] * Costanti::VelocitàDefault * scala;
            ampiezze[5] = AmpiezzeArmoniche[5] * Costanti::VelocitàDefault * scala;
            ampiezze[6] = AmpiezzeArmoniche[6] * Costanti::VelocitàDefault * scala;
            ampiezze[7] = AmpiezzeArmoniche[7] * Costanti::VelocitàDefault * scala;

            // Normalizzo le ampiezze in modo che la loro somma sia pari ad uno, mantenendo invariate le proporzioni tra
            // di loro.
            volatile double n = std::accumulate(ampiezze.cbegin(), ampiezze.cend(), 0.0);
            for (double &ampiezza : ampiezze)
                ampiezza /= n;
        }
    };
}
