#pragma once

#include "CostantiEdAltro.h"

namespace Oscillatori
{
    class Oscillatore
    {
      public:
        /// @brief Restituisce il campione corrente e calcola il successivo
        virtual double Campione() = 0;

        virtual void Frequenza(double frequenza) = 0;

        virtual void Reset() = 0;
    };

    /// @brief Generatore di onda sinusoidale
    /// Genera un'onda sinusoidale della frequenza specificata ed ampiezza uno, generando il numero di campioni per
    /// secondo specificati. L'onda sinusoidale generata corrisponde alla funzione seno
    class OndaSinusoidale: public Oscillatore
    {
      public:
        /// @brief Inizializza il generatore con frequenza zero
        OndaSinusoidale(): nuovoM(0, 0), m(0, 0) {}

        /// @brief Inizializza la generazione dell'onda sinusoidale
        /// @param frequenza frequenza dell'onda da generare
        OndaSinusoidale(double frequenza)
        {
            Frequenza_Interno(frequenza);
            m = nuovoM;
        }

        /// @copydoc Oscillatore::Campione
        double Campione()
        {
            if (daAggiornare.load())
            {
                m = nuovoM;
                daAggiornare.store(false);
            }

            double _campione = fase.imag();

            // Calcolo il campione successivo dell'onda
            fase *= m;
            // Normalizzo così da mantenere i numeri nell'intervallo [-1, 1] ed usufruire della massima precisione in
            // virgola mobile oltre ad ottenere un risultato compreso nell'intervallo [0, 1]
            fase /= std::abs(fase);

            return _campione;
        }

        /// @brief Cambia la frequenza dell'onda sinusoidale
        /// @param frequenza nuova frequenza dell'onda da generare
        void Frequenza(double frequenza)
        {
            Frequenza_Interno(frequenza);
            daAggiornare.store(true);
        }

        // ATTENZIONE: non è sincronizzata con il thread audio
        void Reset()
        {
            fase._Val[0] = 1.0;
            fase._Val[1] = 0.0;
        }

      private:
        std::atomic<bool> daAggiornare;
        dcomplex nuovoM;
        dcomplex m;
        dcomplex fase{ 1.0, 0.0 };

        void Frequenza_Interno(double frequenza)
        {
            nuovoM =
                std::exp(dcomplex(0.0, 2 * std::numbers::pi * frequenza * (1.0 / Costanti::FrequenzaCampionamento)));
        }
    };

    /// @brief Generatore di onda quadra
    /// Genera un'onda quadra della frequenza specificata ed ampiezza uno, generando il numero di campioni per
    /// secondo specificati.
    class OndaQuadra: public Oscillatore
    {
      public:
        /// @brief Inizializza il generatore con frequenza zero
        OndaQuadra() = default;

        /// @brief Inizializza la generazione dell'onda sinusoidale
        /// @param frequenza frequenza dell'onda da generare
        OndaQuadra(double frequenza): sin(frequenza) {}

        /// @copydoc Oscillatore::Campione
        double Campione()
        {
            double _campione = sin.Campione();
            return _campione > 0 ? 1.0 : -1.0;
        }

        /// @brief Cambia la frequenza dell'onda sinusoidale
        /// @param frequenza nuova frequenza dell'onda da generare
        void Frequenza(double frequenza)
        {
            sin.Frequenza(frequenza);
        }

        // ATTENZIONE: non è sincronizzata con il thread audio
        void Reset()
        {
            sin.Reset();
        }

      private:
        OndaSinusoidale sin;
    };
} // namespace Oscillatori
