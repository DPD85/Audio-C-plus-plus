#pragma once

#include "CostantiEdAltro.h"

class Oscillatore
{
  public:
    virtual ~Oscillatore() = default;

    /// @brief Restituisce il campione corrente e calcola il successivo
    virtual double Campione() noexcept = 0;

    virtual void Frequenza(double frequenza) = 0;

    virtual void Reset() = 0;
};

namespace Oscillatori
{
    /// @brief Generatore di onda sinusoidale
    /// Genera un'onda sinusoidale della frequenza specificata ed ampiezza uno. L'onda sinusoidale generata corrisponde
    /// alla funzione seno.
    class OndaSinusoidale: public Oscillatore
    {
      public:
        /// @brief Inizializza il generatore con frequenza zero
        OndaSinusoidale(): nuovoM(0, 0), m(0, 0) {}

        /// @brief Inizializza la generazione dell'onda sinusoidale
        /// @param frequenza frequenza dell'onda da generare
        OndaSinusoidale(double frequenza)
        {
            ImpostaFrequenza(frequenza);
            m = nuovoM;
        }

        /// @copydoc Oscillatore::Campione
        virtual double Campione() noexcept
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
        virtual void Frequenza(double frequenza)
        {
            ImpostaFrequenza(frequenza);
            daAggiornare.store(true);
        }

        // ATTENZIONE: non è sincronizzata con il thread audio
        virtual void Reset()
        {
            fase._Val[0] = 1.0;
            fase._Val[1] = 0.0;
        }

      private:
        std::atomic<bool> daAggiornare;
        dcomplex nuovoM;
        dcomplex m;
        dcomplex fase{ 1.0, 0.0 };

        void ImpostaFrequenza(double frequenza)
        {
            nuovoM =
                std::exp(dcomplex(0.0, 2 * std::numbers::pi * frequenza * (1.0 / Costanti::FrequenzaCampionamento)));
        }
    };

    /// @brief Generatore di onda quadra
    /// Genera un'onda quadra della frequenza specificata, ampiezza uno ed duty-cycle del 50%.
    class OndaQuadra: public Oscillatore
    {
      public:
        /// @brief Inizializza il generatore con frequenza zero
        OndaQuadra() = default;

        /// @brief Inizializza la generazione dell'onda quadra
        /// @param frequenza frequenza dell'onda da generare
        OndaQuadra(double frequenza): sin(frequenza) {}

        /// @copydoc Oscillatore::Campione
        virtual double Campione() noexcept
        {
            return std::copysign(1.0, sin.Campione());
        }

        /// @brief Cambia la frequenza dell'onda quadra
        /// @param frequenza nuova frequenza dell'onda da generare
        virtual void Frequenza(double frequenza)
        {
            sin.Frequenza(frequenza);
        }

        // ATTENZIONE: non è sincronizzata con il thread audio
        virtual void Reset()
        {
            sin.Reset();
        }

      private:
        OndaSinusoidale sin;
    };
} // namespace Oscillatori
