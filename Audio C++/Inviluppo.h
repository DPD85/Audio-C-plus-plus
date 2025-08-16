#pragma once

#include "CostantiEdAltro.h"

class InviluppoADSR
{
  public:
    /// @brief Inizializza un inviluppo di tipo ADSR
    /// @param attacco_ Durata della fase di attacco dell'inviluppo espressa in secondi
    /// @param decadimento_ Durata della fase di decadimento dell'inviluppo espressa in secondi
    /// @param sostentamento_ Ampiezza (volume) della fase di sostentamento [0, 1]
    /// @param rilascio_ Durata della fase di rilascio dell'inviluppo espressa in secondi
    InviluppoADSR(double attacco_, double decadimento_, double sostentamento_, double rilascio_)
    {
        attacco       = DaSecondiACampioni(attacco_);
        decadimento   = DaSecondiACampioni(decadimento_);
        rilascio      = DaSecondiACampioni(rilascio_);
        sostentamento = sostentamento_;
    }

    void InizioNota()
    {
        notaAttiva.test_and_set();
    }

    void FineNota()
    {
        notaAttiva.clear();
    }

    /// @brief Calcola il valore dell'inviluppo un campione alla volta
    /// @return Il valore attuale dell'inviluppo; il valore è sempre compreso nell'intervallo [0, 1]
    double Computa()
    {
        // ----- Cambio di stato

        switch (stato)
        {
            case Stati::Silenzio:
                if (notaAttiva.test())
                {
                    tempo = 0;
                    stato = Stati::Attacco;
                }
                break;
            case Stati::Attacco:
                if (tempo >= attacco) stato = Stati::Decadimento;
                break;
            case Stati::Decadimento:
                if (tempo - attacco >= decadimento) stato = Stati::Sostentamento;
                break;
            case Stati::Sostentamento:
                if (!notaAttiva.test()) stato = Stati::Rilascio;
                break;
            case Stati::Rilascio:
                if (tempo - (attacco + decadimento) >= rilascio) stato = Stati::Silenzio;
                break;
        }

        // ----- Calcolo valore dell'inviluppo

        double valore;

        switch (stato)
        {
            case Stati::Silenzio:
                return 0;
            case Stati::Attacco:
                valore = tempo / static_cast<double>(attacco);
                break;
            case Stati::Decadimento:
                valore =
                    (1.0 - (tempo - attacco) / static_cast<double>(decadimento)) * (1 - sostentamento) + sostentamento;
                break;
            case Stati::Sostentamento:
                // Intenzionale.Il tempo non avanza, in questo modo il tempo di inizio del rilascio sarà sempre uguale
                // alla durata dell'attacco sommata alla durata del decadimento.
                return sostentamento;
            case Stati::Rilascio:
                valore = (1.0 - (tempo - (attacco + decadimento)) / static_cast<double>(rilascio)) * sostentamento;
                break;
        }

        // ----- Avanzamento tempo

        ++tempo;

        // -----

        return valore;
    }

  private:
    enum class Stati
    {
        Silenzio, // fuori dall'inviluppo
        Attacco,
        Decadimento,
        Sostentamento,
        Rilascio,
    };

    std::atomic_flag notaAttiva;

    Stati stato{ Stati::Silenzio };

    size_t tempo{ 0 };    // [# campioni]
    size_t attacco;       // [# campioni]
    size_t decadimento;   // [# campioni]
    size_t rilascio;      // [# campioni]
    double sostentamento; // Ampiezza [0, 1]
};
