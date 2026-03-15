#pragma once

#include "../Utilità.h"

namespace Sintetizzatore
{
    /// @brief %Note di una ottava
    enum Note
    {
        DO,
        DO_DIESIS,
        RE,
        RE_DIESIS,
        MI,
        FA,
        FA_DIESIS,
        SOL,
        SOL_DIESIS,
        LA,
        LA_DIESIS,
        SI,

        NumeroNote, ///< Numero di note all'interno di una ottava
    };

    namespace Costanti
    {
        /// @brief Frequenza di campionamento dell'audio prodotto. [Hz]
        constexpr double FrequenzaCampionamento = 48'000;
        /// @brief Numero di canali audio prodotti.
        constexpr unsigned int NumeroCanali = 2;

        /// @brief Campione ad 8 bit corrispondente al silenzio.
        constexpr unsigned char Silenzio8Bit = 128;
        /// @brief Campione ad 16 bit corrispondente al silenzio.
        constexpr short Silenzio16Bit = 0;

        /// @brief Campione corrispondente al silenzio.
        constexpr double Silenzio = 0.0;
        /// @brief Indice canale sinistro nel caso di audio stereo.
        constexpr unsigned int CanaleSinistro = 0;
        /// @brief Indice canale destro nel caso di audio stereo.
        constexpr unsigned int CanaleDestro = 1;

        /// @brief Velocità di default delle note consigliata nelle specifiche %MIDI per uno strumento che non supporta
        /// velocità dinamiche. [0, 127]
        /// @note Una velocità pari a zero corrisponde ad una nota muta.
        constexpr size_t VelocitàDefaultMIDI = 60;
        /// @brief Velocità di default delle note scalata tra 0 ed 1. [0, 1]
        /// @note Una velocità pari a zero corrisponde ad una nota muta.
        constexpr double VelocitàDefault = VelocitàDefaultMIDI / 127.0;
        /// @brief Impostazione tempo di default nelle specifiche %MIDI: 120 battiti per minuto. [ms]
        constexpr double TempoDefaultMIDI = 120.0 / 60.0 * 1'000.0; // [ms/qn] qn = quarto di nota

        // Rapporti tra le note di una ottava secondo l'intonazione naturale usando il Do come nota di riferimento.

        /// @brief Rapporto del Do in una ottava secondo l'intonazione naturale.
        constexpr double RapportoDo = 1.0;
        /// @brief Rapporto del Re in una ottava secondo l'intonazione naturale.
        constexpr double RapportoRe = 9.0 / 8.0;
        /// @brief Rapporto del Mi in una ottava secondo l'intonazione naturale.
        constexpr double RapportoMi = 5.0 / 4.0;
        /// @brief Rapporto del Fa in una ottava secondo l'intonazione naturale.
        constexpr double RapportoFa = 4.0 / 3.0;
        /// @brief Rapporto del Sol in una ottava secondo l'intonazione naturale.
        constexpr double RapportoSol = 3.0 / 2.0;
        /// @brief Rapporto del La in una ottava secondo l'intonazione naturale.
        constexpr double RapportoLa = 5.0 / 3.0;
        /// @brief Rapporto del Si in una ottava secondo l'intonazione naturale.
        constexpr double RapportoSi = 15.0 / 8.0;

        /// @brief Rapporto del semitono cromatico secondo l'intonazione naturale.
        constexpr double RapportoSemitonoCromatico = 25.0 / 24.0;
        /// @brief Rapporto del semitono diatonico secondo l'intonazione naturale.
        constexpr double RapportoSemitonoDiatonico = 16.0 / 15.0;

        /// @brief Frequenza del La all'interno dell'ottava base, utilizzata come frequenza di riferimento. [Hz]
        ///
        /// Corrisponde al La4, il La della 4° ottava da sinistra sulla tastiera di un pianoforte standard a 88 tasti.
        constexpr double FrequenzaRiferimentoLa = 432;

        /// @brief Frequenza del Do calcolata rispetto al La di riferimento. [Hz]
        constexpr double FrequenzaDo = FrequenzaRiferimentoLa / RapportoLa;
        /// @brief Frequenza del Re calcolata rispetto al Do. [Hz]
        constexpr double FrequenzaRe = FrequenzaDo * RapportoRe;
        /// @brief Frequenza del Mi calcolata rispetto al Do. [Hz]
        constexpr double FrequenzaMi = FrequenzaDo * RapportoMi;
        /// @brief Frequenza del Fa calcolata rispetto al Do. [Hz]
        constexpr double FrequenzaFa = FrequenzaDo * RapportoFa;
        /// @brief Frequenza del Sol calcolata rispetto al Do. [Hz]
        constexpr double FrequenzaSol = FrequenzaDo * RapportoSol;
        /// @brief Frequenza del La calcolata rispetto al Do. [Hz]
        constexpr double FrequenzaLa = FrequenzaDo * RapportoLa;
        /// @brief Frequenza del Si calcolata rispetto al Do. [Hz]
        constexpr double FrequenzaSi = FrequenzaDo * RapportoSi;
        /// @brief Frequenza del Do# calcolata rispetto al Do. [Hz]
        constexpr double FrequenzaDoDiesis = RapportoSemitonoCromatico * FrequenzaDo;
        /// @brief Frequenza del Re# calcolata rispetto al Re. [Hz]
        constexpr double FrequenzaReDiesis = RapportoSemitonoCromatico * FrequenzaRe;
        /// @brief Frequenza del Fa# calcolata rispetto al Fa. [Hz]
        constexpr double FrequenzaFaDiesis = RapportoSemitonoCromatico * FrequenzaFa;
        /// @brief Frequenza del Sol# calcolata rispetto al Sol. [Hz]
        constexpr double FrequenzaSolDiesis = RapportoSemitonoCromatico * FrequenzaSol;
        /// @brief Frequenza del La# calcolata rispetto al La. [Hz]
        constexpr double FrequenzaLaDiesis = RapportoSemitonoCromatico * FrequenzaLa;

        /// @brief Lista delle frequenze delle note all'interno dell'ottava base.
        constexpr std::array FrequenzeNote = {
            FrequenzaDo,       FrequenzaDoDiesis, FrequenzaRe,        FrequenzaReDiesis, FrequenzaMi,       FrequenzaFa,
            FrequenzaFaDiesis, FrequenzaSol,      FrequenzaSolDiesis, FrequenzaLa,       FrequenzaLaDiesis, FrequenzaSi,
        };

        // Qualche controllo per esser certi che le note abbiano una frequenza sensata.
        static_assert(
            FrequenzaDo < FrequenzaDoDiesis && FrequenzaDoDiesis < FrequenzaRe,
            "La frequenza del Do# deve essere compresa tra la frequenza del Do ed del Re");
        static_assert(
            FrequenzaRe < FrequenzaReDiesis && FrequenzaReDiesis < FrequenzaMi,
            "La frequenza del Re# deve essere compresa tra la frequenza del Re ed del Mi");
        static_assert(
            FrequenzaFa < FrequenzaFaDiesis && FrequenzaFaDiesis < FrequenzaSol,
            "La frequenza del Fa# deve essere compresa tra la frequenza del Fa ed del Sol");
        static_assert(
            FrequenzaSol < FrequenzaSolDiesis && FrequenzaSolDiesis < FrequenzaLa,
            "La frequenza del Sol# deve essere compresa tra la frequenza del Sol ed del La");
        static_assert(
            FrequenzaLa < FrequenzaLaDiesis && FrequenzaLaDiesis < FrequenzaSi,
            "La frequenza del La# deve essere compresa tra la frequenza del La ed del Si");
    }

    // ----- -----

    /// @brief Calcola la frequenza di una nota ad una specifica ottava.
    /// @param nota La nota di cui calcolare la frequenza. [0, 11]
    /// @param ottava Il numero dell'ottava rispetto a quella di base. [-5, 5]
    /// @return La frequenza della nota.
    constexpr double CalcolaFrequenzaNota(const Note nota, const int ottava)
    {
        // Calcolo corrispondente a 2^ottava
        const double coefficiente = ottava >= 0 ? 1 << ottava : 1.0 / (1 << -ottava);
        return Costanti::FrequenzeNote[nota] * coefficiente;
    }

    /// @brief Calcola la frequenza di una nota %MIDI.
    /// @param numeroNota Il numero %MIDI della nota da calcolare. [0, 127]
    /// @return La frequenza della nota.
    constexpr double CalcolaFrequenzaNota(const size_t numeroNota)
    {
        // Numero dell'ottava di cui fa parte la nota
        const size_t ottava = numeroNota / 12;
        // Numero del semitono corrispondete alla nota all'interno dell'ottava
        const size_t nota = numeroNota % 12;

        // Nota: l'ottava base corrisponde alla 5° nella numerazione %MIDI
        return CalcolaFrequenzaNota(static_cast<Note>(nota), static_cast<int>(ottava) - 5);
    }

    /// @brief Converte un campione audio in un intero a 8 bit senza segno con valore centrale pari a 128.
    /// @param valore Il campione audio.
    /// @return Il corrispondente valore intero a 8 bit senza segno. [1, 255]
    constexpr unsigned char ConvertiA8Bits(const double valore)
    {
        // Nota: l'intervallo dei valori risultante è simmetrico [1, 255] con valore centrale pari a 128
        return static_cast<unsigned char>(Clamp(std::round(valore * 127 + 128), 1, 255));
    }

    /// @brief Converte un campione audio in un intero a 16 bit con segno con valore centrale pari a zero.
    /// @param valore Il campione audio.
    /// @return Il corrispondente valore intero a 16 bit con segno. [-32767, +32767]
    constexpr short ConvertiA16Bits(const double valore)
    {
        // Nota: l'intervallo dei valori risultante è simmetrico [-32767, +32767] con valore centrale pari a zero.
        return static_cast<short>(Clamp(std::round(valore * 32'767), -32'767, +32'767));
    }

    /// @brief Calcola il numero di campioni audio corrispondente ad un certo tempo arrotondando per difetto.
    /// @param secondi La durata di tempo. [s]
    /// @return Il numero di campioni corrispondete alla durata.
    inline size_t DaSecondiACampioni(const double secondi)
    {
        return static_cast<size_t>(std::floor(Costanti::FrequenzaCampionamento * secondi));
    }

    /// @brief Calcola la durata di tempo corrispondente ad un certo numero di campioni audio.
    /// @param campioni Il numero di campioni audio.
    /// @return La durata di tempo corrispondente ai campioni audio. [s]
    constexpr double DaCampioniASecondi(const size_t campioni)
    {
        return static_cast<double>(campioni) / Costanti::FrequenzaCampionamento;
    }

    /// @brief Calcola il fattore di guadagno da utilizzare nel calcolo dello smussamento esponenziale corrispondente ad
    /// uno specifico fattore di smussamento.
    /// @param valore Il fattore di smussamento. Più è grande, maggiore è lo smussamento [0, +∞]
    /// @return Il guadagno corrispondente da utilizzare nel calcolo dello smussamento esponenziale.
    constexpr double DaSmussamentoAGuadagno(const double valore)
    {
        if (valore == 0) return 1.0;

        return -std::expm1(-1.0 / Costanti::FrequenzaCampionamento / valore);
    }
}
