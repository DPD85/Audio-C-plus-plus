#pragma once

#include "CostantiEdAltro.h"

namespace Oscillatori
{
    class Oscillatore
    {
      public:
        /// @brief Restituisce il campione corrente e calcola il successivo
        virtual double Campione() = 0;

        virtual void Frequenza(double frequenza, unsigned int frequenzaCampionamento) = 0;

        virtual void Reset() = 0;
    };

    /// @brief Generatore di onda sinusoidale
    /// Genera un'onda sinusoidale della frequenza specificata ed ampiezza uno, generando il numero di campioni per
    /// secondo specificati. L'onda sinusoidale generata corrisponde alla funzione seno
    class OndaSinusoidale: public Oscillatore
    {
      public:
        /// @brief Inizializza la generazione dell'onda sinusoidale
        /// @param frequenza frequenza dell'onda da generare
        /// @param frequenzaCampionamento numero di campioni per secondo da generare
        OndaSinusoidale(double frequenza, unsigned int frequenzaCampionamento = Costanti::FrequenzaCampionamento)
        {
            this->Frequenza(frequenza, frequenzaCampionamento);
            m = nuovoM;
            aggiornato.test_and_set();
        }

        /// @copydoc Oscillatore::Campione
        double Campione()
        {
            if (!aggiornato.test_and_set()) m = nuovoM;

            double _campione = fase.imag();

            // Calcolo il campione successivo dell'onda
            fase *= m;
            // Normalizzo così da mantenere i numeri nell'intervallo [-1, 1] ed usufruire della massima precisione in
            // virgola mobile
            fase /= std::abs(fase);

            return _campione;
        }

        /// @brief Cambia la frequenza dell'onda sinusoidale
        /// @param frequenza nuova frequenza dell'onda da generare
        /// @param frequenzaCampionamento nuovo numero di campioni per secondo da generare
        void Frequenza(double frequenza, unsigned int frequenzaCampionamento = Costanti::FrequenzaCampionamento)
        {
            nuovoM = std::exp(dcomplex(0.0, 2 * std::numbers::pi * frequenza * (1.0 / frequenzaCampionamento)));
            aggiornato.clear();
        }

        // ATTENZIONE: non è sincronizzata con il thread audio
        void Reset()
        {
            fase._Val[0] = 1.0;
            fase._Val[1] = 0.0;
        }

      private:
        std::atomic_flag aggiornato;
        dcomplex nuovoM;
        dcomplex m;
        dcomplex fase{ 1.0, 0.0 };
    };

    /// @brief Generatore di onda quadra
    /// Genera un'onda quadra della frequenza specificata ed ampiezza uno, generando il numero di campioni per
    /// secondo specificati.
    class OndaQuadra: public Oscillatore
    {
      public:
        /// @brief Inizializza la generazione dell'onda sinusoidale
        /// @param frequenza frequenza dell'onda da generare
        /// @param frequenzaCampionamento numero di campioni per secondo da generare
        OndaQuadra(double frequenza, unsigned int frequenzaCampionamento = Costanti::FrequenzaCampionamento)
            : sin(frequenza, frequenzaCampionamento)
        {
            this->Frequenza(frequenza, frequenzaCampionamento);
        }

        /// @copydoc Oscillatore::Campione
        double Campione()
        {
            double _campione = sin.Campione();
            return _campione > 0 ? 1.0 : -1.0;
        }

        /// @brief Cambia la frequenza dell'onda sinusoidale
        /// @param frequenza nuova frequenza dell'onda da generare
        /// @param frequenzaCampionamento nuovo numero di campioni per secondo da generare
        void Frequenza(double frequenza, unsigned int frequenzaCampionamento = Costanti::FrequenzaCampionamento)
        {
            sin.Frequenza(frequenza, frequenzaCampionamento);
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
