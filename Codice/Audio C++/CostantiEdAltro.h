#pragma once

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
    const constexpr double FrequenzaCampionamento = 48'000;
    /// @brief Numero di canali audio prodotti.
    const constexpr unsigned int NumeroCanali = 2;

    /// @brief Campione ad 8 bit corrispondente al silenzio.
    const constexpr unsigned char Silenzio8Bit = 128;
    /// @brief Campione ad 16 bit corrispondente al silenzio.
    const constexpr short Silenzio16Bit = 0;

    /// @brief Campione corrispondente al silenzio.
    const constexpr double Silenzio = 0.0;
    /// @brief Indice canale sinistro nel caso di audio stereo.
    const constexpr unsigned int CanaleSinistro = 0;
    /// @brief Indice canale destro nel caso di audio stereo.
    const constexpr unsigned int CanaleDestro = 1;

    /// @brief Velocità di default delle note consigliata nelle specifiche %MIDI per uno strumento che non supporta
    /// velocità dinamiche. [0, 127]
    /// @note Una velocità pari a zero corrisponde ad una nota muta.
    const constexpr size_t VelocitàDefaultMIDI = 60;
    /// @brief Velocità di default delle note scalata tra 0 ed 1. [0, 1]
    /// @note Una velocità pari a zero corrisponde ad una nota muta.
    const constexpr double VelocitàDefault = VelocitàDefaultMIDI / 127.0;
    /// @brief Impostazione tempo di default nelle specifiche %MIDI: 120 battiti per minuto. [ms]
    const constexpr double TempoDefaultMIDI = 120.0 / 60.0 * 1'000.0; // [ms/qn] qn = quarto di nota

    // Rapporti tra le note di una ottava secondo l'intonazione naturale usando il Do come nota di riferimento.

    /// @brief Rapporto del Do in una ottava secondo l'intonazione naturale.
    const constexpr double RapportoDo = 1.0;
    /// @brief Rapporto del Re in una ottava secondo l'intonazione naturale.
    const constexpr double RapportoRe = 9.0 / 8.0;
    /// @brief Rapporto del Mi in una ottava secondo l'intonazione naturale.
    const constexpr double RapportoMi = 5.0 / 4.0;
    /// @brief Rapporto del Fa in una ottava secondo l'intonazione naturale.
    const constexpr double RapportoFa = 4.0 / 3.0;
    /// @brief Rapporto del Sol in una ottava secondo l'intonazione naturale.
    const constexpr double RapportoSol = 3.0 / 2.0;
    /// @brief Rapporto del La in una ottava secondo l'intonazione naturale.
    const constexpr double RapportoLa = 5.0 / 3.0;
    /// @brief Rapporto del Si in una ottava secondo l'intonazione naturale.
    const constexpr double RapportoSi = 15.0 / 8.0;

    /// @brief Rapporto del semitono cromatico secondo l'intonazione naturale.
    const constexpr double RapportoSemitonoCromatico = 25.0 / 24.0;
    /// @brief Rapporto del semitono diatonico secondo l'intonazione naturale.
    const constexpr double RapportoSemitonoDiatonico = 16.0 / 15.0;

    /// @brief Frequenza del La all'interno dell'ottava base, utilizzata come frequenza di riferimento. [Hz]
    ///
    /// Corrisponde al La4, il La della 4° ottava da sinistra sulla tastiera di un pianoforte standard a 88 tasti.
    const constexpr double FrequenzaRiferimentoLa = 432;

    /// @brief Frequenza del Do calcolata rispetto al La di riferimento. [Hz]
    const constexpr double FrequenzaDo = FrequenzaRiferimentoLa / RapportoLa;
    /// @brief Frequenza del Re calcolata rispetto al Do. [Hz]
    const constexpr double FrequenzaRe = FrequenzaDo * RapportoRe;
    /// @brief Frequenza del Mi calcolata rispetto al Do. [Hz]
    const constexpr double FrequenzaMi = FrequenzaDo * RapportoMi;
    /// @brief Frequenza del Fa calcolata rispetto al Do. [Hz]
    const constexpr double FrequenzaFa = FrequenzaDo * RapportoFa;
    /// @brief Frequenza del Sol calcolata rispetto al Do. [Hz]
    const constexpr double FrequenzaSol = FrequenzaDo * RapportoSol;
    /// @brief Frequenza del La calcolata rispetto al Do. [Hz]
    const constexpr double FrequenzaLa = FrequenzaDo * RapportoLa;
    /// @brief Frequenza del Si calcolata rispetto al Do. [Hz]
    const constexpr double FrequenzaSi = FrequenzaDo * RapportoSi;
    /// @brief Frequenza del Do# calcolata rispetto al Do. [Hz]
    const constexpr double FrequenzaDoDiesis = RapportoSemitonoCromatico * FrequenzaDo;
    /// @brief Frequenza del Re# calcolata rispetto al Re. [Hz]
    const constexpr double FrequenzaReDiesis = RapportoSemitonoCromatico * FrequenzaRe;
    /// @brief Frequenza del Fa# calcolata rispetto al Fa. [Hz]
    const constexpr double FrequenzaFaDiesis = RapportoSemitonoCromatico * FrequenzaFa;
    /// @brief Frequenza del Sol# calcolata rispetto al Sol. [Hz]
    const constexpr double FrequenzaSolDiesis = RapportoSemitonoCromatico * FrequenzaSol;
    /// @brief Frequenza del La# calcolata rispetto al La. [Hz]
    const constexpr double FrequenzaLaDiesis = RapportoSemitonoCromatico * FrequenzaLa;

    /// @brief Lista delle frequenze delle note all'interno dell'ottava base.
    const std::array<double, Note::NumeroNote> FrequenzeNote = {
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

/// @brief Calcola la frequenza di una nota ad una specifica ottava.
/// @param nota La nota di cui calcolare la frequenza. [0, 11]
/// @param ottava Il numero dell'ottava rispetto a quella di base. [-5, 5]
/// @return La frequenza della nota.
inline constexpr double CalcolaFrequenzaNota(Note nota, int ottava)
{
    // Calcolo corrispondente a 2^ottava
    const double coefficente = ottava >= 0 ? 1 << ottava : 1.0 / (1 << -ottava);
    return Costanti::FrequenzeNote[nota] * coefficente;
}

/// @brief Calcola la frequenza di una nota %MIDI.
/// @param numeroNota Il numero %MIDI della nota da calcolare. [0, 127]
/// @return La frequenza della nota.
inline constexpr double CalcolaFrequenzaNota(size_t numeroNota)
{
    // Numero dell'ottava di cui fa parte la nota
    const size_t ottava = numeroNota / 12;
    // Numero del semitono corrispondete alla nota all'interno dell'ottava
    const size_t nota = numeroNota % 12;

    // Nota: l'ottava base corrisponde alla 5° ottava nella numerazione %MIDI
    return CalcolaFrequenzaNota(static_cast<Note>(nota), static_cast<int>(ottava) - 5);
}

/// @brief Limita un valore tra due estremi.
///
/// Quando il valore è compreso tra i due estremi esso resta invariato.<br />
/// Se il valore è minore del primo estremo allora esso diventa il primo estremo. <br />
/// Se il valore è maggiore del secondo estremo allora essere diventa il secondo estremo.
/// @param value Il valore da limitare.
/// @param min Il primo estremo.
/// @param max Il secondo estremo.
/// @return Il valore limitato tra i due estremi.
inline constexpr double Clamp(double value, double min, double max)
{
    return std::max(std::min(value, max), min);
}

/// @brief Converte un campione audio in un intero a 8 bit senza segno con valore centrale pari a 128.
/// @param valore Il campione audio.
/// @return Il corrispondente valore intero a 8 bit senza segno. [1, 255]
inline constexpr unsigned char ConvertiA8Bits(double valore)
{
    // Nota: l'intervallo dei valori risultante è simmetrico [1, 255] con valore centrale pari a 128
    return static_cast<unsigned char>(Clamp(std::round(valore * 127 + 128), 1, 255));
}

/// @brief Converte un campione audio in un intero a 16 bit con segno con valore centrale pari a zero.
/// @param valore Il campione audio.
/// @return Il corrispondente valore intero a 16 bit con segno. [-32767, +32767]
inline constexpr short ConvertiA16Bits(double valore)
{
    // Nota: l'intervallo dei valori risultante è simmetrico [-32767, +32767] con valore centrale pari a zero.
    return static_cast<short>(Clamp(std::round(valore * 32'767), -32'767, +32'767));
}

/// @brief Calcola il massimo valore presente in una lista di numeri.
/// @tparam ...Args I tipi dei numeri nella lista.
/// @param v1 Il primo numero della lista.
/// @param ...args I numeri della lista dal secondo in poi.
/// @return Il massimo valore presente nella lista di numeri.
template<typename... Args> inline constexpr double Max(double v1, Args... args)
{
    return std::max(v1, Max(args...));
}

/// @brief Calcola il massimo valore tra due numeri.
/// @param v1 Il primo numero.
/// @param v2 Il secondo numero.
/// @return Il massimo valore tra i due numeri.
template<> inline constexpr double Max(double v1, double v2)
{
    return std::max(v1, v2);
}

/// @brief Calcola il numero di campioni audio corrispondente ad un certo tempo arrotondando per difetto.
/// @param secondi La durata di tempo. [s]
/// @return Il numero di campioni corrispondete alla durata.
inline size_t DaSecondiACampioni(double secondi)
{
    return static_cast<size_t>(std::floor(Costanti::FrequenzaCampionamento * secondi));
}

/// @brief Calcola la durata di tempo corrispondente ad un certo numero di campioni audio.
/// @param campioni Il numero di campioni audio.
/// @return La durata di tempo corrispondente ai campioni audio. [s]
inline constexpr double DaCampioniASecondi(size_t campioni)
{
    return static_cast<double>(campioni) / Costanti::FrequenzaCampionamento;
}

/// @brief Calcola il fattore di guadagno da utilizzare nel calcolo dello smussamento esponenziale corrispondente ad uno
/// specifico fattore di smussamento.
/// @param valore Il fattore di smussamento. Più è grande, maggiore è lo smussamento [0, +∞]
/// @return Il guadagno corrispondente da utilizzare nel calcolo dello smussamento esponenziale.
inline constexpr double DaSmussamentoAGuadagno(double valore)
{
    if (valore == 0) return 1.0;
    else return -std::expm1(-1.0 / Costanti::FrequenzaCampionamento / valore);
}

/// @brief Creare una lista di numeri normalizzati.
///
/// La normalizzazione fa in modo che la somma dei numeri sia pari ad uno, mantenendo però invariate le proporzioni tra
/// i numeri.
/// @tparam ...Args I tipi dei numeri con cui creare la lista.
/// @param ...args I numeri con cui creare la lista.
/// @return La lista di numeri normalizzati.
template<typename... Args> constexpr std::array<double, sizeof...(Args)> CreaListaNormalizzata(Args... args)
{
    const double n = (args + ...);
    return { args * (1.0 / n)... };
}
