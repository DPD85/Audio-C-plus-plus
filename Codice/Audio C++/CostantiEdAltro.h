#pragma once

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

    NumeroNote,
    NumeroNoteBase = 7,
};

namespace Costanti
{
    // Frequenza di campionamento dell'audio prodotto
    const constexpr double FrequenzaCampionamento = 48'000; // [Hz]
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

    // Rapporti dei semitoni secondo l'intonazione naturale
    const constexpr double RapportoSemitonoCromatico = 25.0 / 24.0;
    const constexpr double RapportoSemitonoDiatonico = 16.0 / 15.0;

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
    // Frequenze semitoni
    const constexpr double FrequenzaDoDiesis  = RapportoSemitonoCromatico * FrequenzaDo;  // [Hz]
    const constexpr double FrequenzaReDiesis  = RapportoSemitonoCromatico * FrequenzaRe;  // [Hz]
    const constexpr double FrequenzaFaDiesis  = RapportoSemitonoCromatico * FrequenzaFa;  // [Hz]
    const constexpr double FrequenzaSolDiesis = RapportoSemitonoCromatico * FrequenzaSol; // [Hz]
    const constexpr double FrequenzaLaDiesis  = RapportoSemitonoCromatico * FrequenzaLa;  // [Hz]

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

    // Frequenze semitoni
    const double FrequenzaDoDiesis  = FrequenzaRiferimentoLa * std::pow(2.0, -8.0 / 12.0); // [Hz]
    const double FrequenzaReDiesis  = FrequenzaRiferimentoLa * std::pow(2.0, -6.0 / 12.0); // [Hz]
    const double FrequenzaFaDiesis  = FrequenzaRiferimentoLa * std::pow(2.0, -3.0 / 12.0); // [Hz]
    const double FrequenzaSolDiesis = FrequenzaRiferimentoLa * std::pow(2.0, -1.0 / 12.0); // [Hz]
    const double FrequenzaLaDiesis  = FrequenzaRiferimentoLa * std::pow(2.0, +1.0 / 12.0); // [Hz]
#endif

    const std::array<double, Note::NumeroNote> FrequenzeNote = {
        FrequenzaDo,       FrequenzaDoDiesis, FrequenzaRe,        FrequenzaReDiesis, FrequenzaMi,       FrequenzaFa,
        FrequenzaFaDiesis, FrequenzaSol,      FrequenzaSolDiesis, FrequenzaLa,       FrequenzaLaDiesis, FrequenzaSi,
    };
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
    return static_cast<short>(Clamp(std::round(valore * 32'767), -32'767, +32'767));
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

inline constexpr double DaCampioniASecondi(size_t campioni)
{
    return static_cast<double>(campioni) / Costanti::FrequenzaCampionamento;
}

inline constexpr double DaSmussamentoAGuadagno(double valore)
{
    if (valore == 0) return 1.0;
    else return -std::expm1(-1.0 / Costanti::FrequenzaCampionamento / valore);
}

// Normalizza i numeri della lista in modo che la loro somma sia pari ad uno, mantenendo però invariate le proporzioni
// tra di loro.
template<typename... Args> constexpr std::array<double, sizeof...(Args)> CreaListaNormalizzata(Args... args)
{
    const double n = (args + ...);
    return { args * (1.0 / n)... };
}
