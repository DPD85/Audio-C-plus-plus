#pragma once

#include "CostantiEdAltro.h"

namespace Sintetizzatore
{
    /// @brief Interfaccia comune dei vari oscillatori e generatori d'onda.
    class Oscillatore
    {
      public:
        virtual ~Oscillatore() = default;

        /// @brief Restituisce il campione corrente e calcola il successivo.
        virtual double Campione() noexcept = 0;

        /// @brief Cambia la frequenza di oscillazione.
        /// @param frequenza La nuova frequenza di oscillazione. [Hz]
        virtual void Frequenza(double frequenza) = 0;

        /// @warning Non è necessariamente sincronizzato con il calcolo dell'audio, ovvero con il metodo Campione().
        /// Consultare la documentazione degli specifici generatori per sapere se effettivamente questo metodo è
        /// sincronizzato e meno.
        virtual void Reset() = 0;
    };

    namespace Oscillatori
    {
        /// @brief Generatore di onda sinusoidale.
        ///
        /// Genera un'onda sinusoidale della frequenza specificata ed ampiezza uno. L'onda generata corrisponde alla
        /// funzione seno.
        class OndaSinusoidale: public Oscillatore
        {
          public:
            /// @brief Inizializza il generatore con frequenza zero.
            OndaSinusoidale()
                : nuovoDeltaFase(0, 0)
                , deltaFase(0, 0)
                , nuovoInvModuloDeltaFase(0.0)
                , invModuloDeltaFase(0.0)
            {}

            /// @brief Inizializza la generazione dell'onda sinusoidale.
            /// @param frequenza La frequenza dell'onda da generare.
            explicit OndaSinusoidale(const double frequenza)
            {
                ImpostaFrequenza(frequenza);
                deltaFase          = nuovoDeltaFase;
                invModuloDeltaFase = nuovoInvModuloDeltaFase;
            }

            double Campione() noexcept override
            {
                if (daAggiornare.load())
                {
                    deltaFase          = nuovoDeltaFase;
                    invModuloDeltaFase = nuovoInvModuloDeltaFase;
                    daAggiornare.store(false);
                }

                const double campione_ = fase.imag();

                // Calcolo il campione successivo dell'onda
                fase *= deltaFase;
                // Normalizzo così da mantenere i numeri nell'intervallo [-1, 1] ed usufruire della massima precisione
                // in virgola mobile oltre ad ottenere un risultato compreso nell'intervallo [0, 1]
                fase *= invModuloDeltaFase;

                return campione_;
            }

            /// @brief Cambia la frequenza dell'onda sinusoidale.
            /// @param frequenza La nuova frequenza dell'onda da generare.
            void Frequenza(const double frequenza) override
            {
                ImpostaFrequenza(frequenza);
                daAggiornare.store(true);
            }

            /// @notaudiosafe Campione().
            void Reset() override
            {
                fase.real(1.0);
                fase.imag(0.0);
            }

          private:
            std::atomic<bool> daAggiornare;
            dcomplex nuovoDeltaFase;
            // Differenza di fase tra due campioni consecutivi dell'onda.
            dcomplex deltaFase;
            dcomplex fase{ 1.0, 0.0 };
            double nuovoInvModuloDeltaFase;
            // Inverso della lunghezza di deltaFase come se deltaFase fosse un vettore sul piano cartesiano
            // (\f$sqrt(r^2 + i^2)\f$).
            double invModuloDeltaFase;

            void ImpostaFrequenza(const double frequenza)
            {
                nuovoDeltaFase = std::exp(
                    dcomplex(0.0, 2 * std::numbers::pi * frequenza * (1.0 / Costanti::FrequenzaCampionamento)));
                nuovoInvModuloDeltaFase = 1.0 / std::hypot(nuovoDeltaFase.real(), nuovoDeltaFase.imag());
            }
        };

        /// @brief Generatore di onda quadra.
        ///
        /// Genera un'onda quadra della frequenza specificata, ampiezza uno e duty-cycle del 50%.
        class OndaQuadra: public Oscillatore
        {
          public:
            /// @brief Inizializza il generatore con frequenza zero.
            OndaQuadra() = default;

            /// @brief Inizializza la generazione dell'onda quadra.
            /// @param frequenza La frequenza dell'onda da generare.
            explicit OndaQuadra(const double frequenza): sin(frequenza) {}

            double Campione() noexcept override
            {
                return std::copysign(1.0, sin.Campione());
            }

            /// @brief Cambia la frequenza dell'onda quadra
            /// @param frequenza nuova frequenza dell'onda da generare
            void Frequenza(const double frequenza) override
            {
                sin.Frequenza(frequenza);
            }

            /// @notaudiosafe Campione().
            void Reset() override
            {
                sin.Reset();
            }

          private:
            OndaSinusoidale sin;
        };
    }
}
