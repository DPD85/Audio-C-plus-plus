#pragma once

enum Note
{
    DO,
    RE,
    MI,
    FA,
    SOL,
    LA,
    SI,

    NumeroNote,
};

namespace Costanti
{
    // Frequenza di campionamento dell'audio prodotto
    const constexpr double FrequenzaCampionamento = 44100; // [Hz]
    // Numero di audio prodotti
    const constexpr unsigned int NumeroCanali = 2;

    // Campione ad 8 bit corrispondente al silenzio
    const constexpr unsigned char Silenzio8Bit = 128;
    // Campione ad 16 bit corrispondente al silenzio
    const constexpr short Silenzio16Bit = 0;

    // Campione corrispondente al silenzio
    const constexpr double Silenzio = 0.0;
    // Indice canale sinistro nel caso di audio stereo
    const constexpr unsigned int CanaleSinistro = 0;
    // Indice canale destro nel caso di audio stereo
    const constexpr unsigned int CanaleDestro = 1;

    // Rapporti tra le note di una ottava secondo l'intonazione naturale usando il Do come nota di riferimento
    const constexpr double RapportoDo  = 1.0;
    const constexpr double RapportoRe  = 9.0 / 8.0;
    const constexpr double RapportoMi  = 5.0 / 4.0;
    const constexpr double RapportoFa  = 4.0 / 3.0;
    const constexpr double RapportoSol = 3.0 / 2.0;
    const constexpr double RapportoLa  = 5.0 / 3.0;
    const constexpr double RapportoSi  = 15.0 / 8.0;

    const constexpr double FrequenzaRiferimentoLa = 432; // [Hz]

#if 1
    // Frequenza del DO calcolata rispetto al LA di riferimento
    const constexpr double FrequenzaDo = FrequenzaRiferimentoLa / RapportoLa; // [Hz]
    // Frequenze delle note calcolate rispetto al DO
    const constexpr double FrequenzaRe  = FrequenzaDo * RapportoRe;  // [Hz]
    const constexpr double FrequenzaMi  = FrequenzaDo * RapportoMi;  // [Hz]
    const constexpr double FrequenzaFa  = FrequenzaDo * RapportoFa;  // [Hz]
    const constexpr double FrequenzaSol = FrequenzaDo * RapportoSol; // [Hz]
    const constexpr double FrequenzaLa  = FrequenzaDo * RapportoLa;  // [Hz]
    const constexpr double FrequenzaSi  = FrequenzaDo * RapportoSi;  // [Hz]
#else
    // Frequenza delle note calcolata secondo l'intonazione equanime (tutte i semi toni sono equidistanti).
    // Il rapporto tra due semitoni consecutivi è sempre pari alla radice dodicesima di due.

    // Formula: FrequenzaRiferimentoLa * std::pow(2.0, N / 12.0)
    // dove N è il numero di semitoni di distanza dalla nota di riferimento
    const double FrequenzaDo  = FrequenzaRiferimentoLa * std::pow(2.0, -9.0 / 12.0); // [Hz]
    const double FrequenzaRe  = FrequenzaRiferimentoLa * std::pow(2.0, -7.0 / 12.0); // [Hz]
    const double FrequenzaMi  = FrequenzaRiferimentoLa * std::pow(2.0, -5.0 / 12.0); // [Hz]
    const double FrequenzaFa  = FrequenzaRiferimentoLa * std::pow(2.0, -4.0 / 12.0); // [Hz]
    const double FrequenzaSol = FrequenzaRiferimentoLa * std::pow(2.0, -2.0 / 12.0); // [Hz]
    const double FrequenzaLa  = FrequenzaRiferimentoLa * std::pow(2.0, +0.0 / 12.0); // [Hz]
    const double FrequenzaSi  = FrequenzaRiferimentoLa * std::pow(2.0, +2.0 / 12.0); // [Hz]
#endif

    const constexpr std::array<double, Note::NumeroNote> FrequenzeNote = { FrequenzaDo, FrequenzaRe,  FrequenzaMi,
                                                                           FrequenzaFa, FrequenzaSol, FrequenzaLa,
                                                                           FrequenzaSi };
}

// ----- -----

using dcomplex           = std::complex<double>;
using dvector            = std::vector<double>;
using DurataMillisecondi = std::chrono::duration<double, std::milli>;

// ----- -----

static_assert(
    std::atomic<double>::is_always_lock_free,
    "L'architettura del processore non supporta le operazioni atomiche lock-free con il tipo double.");
static_assert(
    std::atomic<void *>::is_always_lock_free,
    "L'architettura del processore non supporta le operazioni atomiche lock-free con i tipi puntatore.");

// ----- -----

inline constexpr double Clamp(double value, double min, double max)
{
    return std::max(std::min(value, max), min);
}

inline constexpr unsigned char ConvertiA8Bits(double valore)
{
    // Nota: l'intervallo dei valori risultante è simmetrico [1, 255] con valore centrale pari a 128
    return static_cast<unsigned char>(Clamp(std::round(valore * 127 + 128), 1, 255));
}

inline constexpr short ConvertiA16Bits(double valore)
{
    // Nota: l'intervallo dei valori risultante è simmetrico [-32767, +32767] con valore centrale pari a 0
    return static_cast<short>(Clamp(std::round(valore * 32767), -32767, +32767));
}

template<typename... Args> inline constexpr double Max(double v1, Args... args)
{
    return std::max(v1, Max(args...));
}

template<> inline constexpr double Max(double v1, double v2)
{
    return std::max(v1, v2);
}

inline size_t DaSecondiACampioni(double secondi)
{
    return static_cast<size_t>(std::floor(Costanti::FrequenzaCampionamento * secondi));
}
