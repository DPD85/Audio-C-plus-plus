#pragma once

#include "CostantiEdAltro.h"

/// @brief Inviluppo lineare di tipo ADSR.
class InviluppoADSR
{
  public:
    enum class Stati
    {
        Silenzio,      ///< Fuori dall'inviluppo
        Attacco,       ///< Fase di attacco dell'inviluppo
        Decadimento,   ///< Fase di decadimento dell'inviluppo
        Sostentamento, ///< Fase di sostentamento dell'inviluppo
        Rilascio,      ///< Fase di rilascio dell'inviluppo
    };

    /// @brief Inizializza un inviluppo lineare di tipo ADSR con durata delle fasi e livello di sostentamento pari a
    /// zero.
    InviluppoADSR(): attacco(0), decadimento(0), rilascio(0), sostentamento(0.0) {}

    /// @brief Inizializza un inviluppo lineare di tipo ADSR.
    /// @param attacco_ La durata della fase di attacco dell'inviluppo. [s]
    /// @param decadimento_ La durata della fase di decadimento dell'inviluppo. [s]
    /// @param sostentamento_ L'ampiezza della fase di sostentamento. [0, 1]
    /// @param rilascio_ La durata della fase di rilascio dell'inviluppo. [s]
    InviluppoADSR(double attacco_, double decadimento_, double sostentamento_, double rilascio_)
    {
        attacco       = DaSecondiACampioni(attacco_);
        decadimento   = DaSecondiACampioni(decadimento_);
        rilascio      = DaSecondiACampioni(rilascio_);
        sostentamento = sostentamento_;
    }

    /// @brief Indica l'istante in cui il musicista da inizio ad una nota. Per esempio la pressione di un tasto sul
    /// pianoforte.
    void InizioNota()
    {
        notaAttiva.store(true);
    }

    /// @brief Indica l'istante in cui il musicista termina una nota. Per esempio un tasto del pianoforte viene
    /// rilasciato.
    void FineNota()
    {
        notaAttiva.store(false);
    }

    /// @brief Calcola il valore successivo dell'inviluppo.
    /// @return Il valore attuale dell'inviluppo. [0, 1]
    double Computa() noexcept
    {
        // ----- Cambio di stato

        switch (stato)
        {
            case Stati::Silenzio:
                if (notaAttiva.load())
                {
                    tempo = 0;
                    stato = Stati::Attacco;
                }
                break;
            case Stati::Attacco:
                if (tempo >= attacco)
                {
                    tempo -= attacco;
                    stato  = Stati::Decadimento;
                }
                break;
            case Stati::Decadimento:
                if (tempo >= decadimento)
                {
                    tempo -= decadimento;
                    stato  = Stati::Sostentamento;
                }
                break;
            case Stati::Sostentamento:
                if (!notaAttiva.load()) stato = Stati::Rilascio;
                break;
            case Stati::Rilascio:
                if (tempo >= rilascio) stato = Stati::Silenzio;
                break;
        }

        // ----- Calcolo valore dell'inviluppo

        double valore = 0;

        switch (stato)
        {
            case Stati::Silenzio:
                return 0;
            case Stati::Attacco:
                valore = tempo / static_cast<double>(attacco);
                break;
            case Stati::Decadimento:
                valore = (1.0 - tempo / static_cast<double>(decadimento)) * (1 - sostentamento) + sostentamento;
                break;
            case Stati::Sostentamento:
                // Intenzionale. Il tempo non avanza, in questo modo il tempo di inizio del rilascio sarà indipendente
                // dalla durata del sostentamento.
                return sostentamento;
            case Stati::Rilascio:
                valore = (1.0 - tempo / static_cast<double>(rilascio)) * sostentamento;
                break;
        }

        // ----- Avanzamento tempo

        ++tempo;

        // -----

        return valore;
    }

    /// @brief Indica se la nota corrispondente a questo inviluppo sta suonando oppure se è muta.
    /// @retval True La nota sta suonando.
    /// @retval False La nota è muta.
    /// @warning Il metodo non è sincronizzato con il calcolo dell'audio, ovvero col metodo Computa().
    bool StaSuonando() const
    {
        return stato != Stati::Silenzio;
    }

    /// @brief Restituisce lo stato attuale dell'inviluppo.
    /// @warning Il metodo non è sincronizzato con il calcolo dell'audio, ovvero col metodo Computa().
    Stati Stato() const
    {
        return stato;
    }

  private:
    std::atomic<bool> notaAttiva;

    Stati stato{ Stati::Silenzio };

    size_t tempo{ 0 };    // [# campioni]
    size_t attacco;       // [# campioni]
    size_t decadimento;   // [# campioni]
    size_t rilascio;      // [# campioni]
    double sostentamento; // Ampiezza [0, 1]

    // ----- Implementazione copia e movimento dell'oggetto -----
    /// @cond CTOR_OP_COPY_MOVE
  public:
    InviluppoADSR(const InviluppoADSR &&altro)
    {
        notaAttiva.store(altro.notaAttiva.load());

        stato         = altro.stato;
        attacco       = altro.attacco;
        decadimento   = altro.decadimento;
        rilascio      = altro.rilascio;
        sostentamento = altro.sostentamento;
    }

    InviluppoADSR &operator=(const InviluppoADSR &&altro)
    {
        notaAttiva.store(altro.notaAttiva.load());

        stato         = altro.stato;
        attacco       = altro.attacco;
        decadimento   = altro.decadimento;
        rilascio      = altro.rilascio;
        sostentamento = altro.sostentamento;

        return *this;
    }

    InviluppoADSR(const InviluppoADSR &altro)
    {
        notaAttiva.store(altro.notaAttiva.load());

        stato         = altro.stato;
        attacco       = altro.attacco;
        decadimento   = altro.decadimento;
        rilascio      = altro.rilascio;
        sostentamento = altro.sostentamento;
    }

    InviluppoADSR &operator=(const InviluppoADSR &altro)
    {
        notaAttiva.store(altro.notaAttiva.load());

        stato         = altro.stato;
        attacco       = altro.attacco;
        decadimento   = altro.decadimento;
        rilascio      = altro.rilascio;
        sostentamento = altro.sostentamento;

        return *this;
    }
    /// @endcond
};
