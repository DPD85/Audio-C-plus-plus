#pragma once

using dcomplex           = std::complex<double>;
using dvector            = std::vector<double>;
using datomic            = std::atomic<double>;
using DurataMillisecondi = std::chrono::duration<double, std::milli>;

// ----- -----

static_assert(
    datomic::is_always_lock_free,
    "L'architettura del processore non supporta le operazioni atomiche lock-free con il tipo double.");
static_assert(
    std::atomic<void *>::is_always_lock_free,
    "L'architettura del processore non supporta le operazioni atomiche lock-free con i tipi puntatore.");

// ----- -----

/// @brief Limita un valore tra due estremi.
///
/// Quando il valore è compreso tra i due estremi esso resta invariato.<br />
/// Se il valore è minore del primo estremo allora esso diventa il primo estremo. <br />
/// Se il valore è maggiore del secondo estremo allora essere diventa il secondo estremo.
/// @param value Il valore da limitare.
/// @param min Il primo estremo.
/// @param max Il secondo estremo.
/// @return Il valore limitato tra i due estremi.
constexpr double Clamp(const double value, const double min, const double max)
{
    return std::max(std::min(value, max), min);
}

/// @brief Calcola il massimo valore presente in una lista di numeri.
/// @tparam Args I tipi dei numeri nella lista.
/// @param v1 Il primo numero della lista.
/// @param args I numeri della lista dal secondo in poi.
/// @return Il massimo valore presente nella lista di numeri.
template<typename... Args> constexpr double Max(const double v1, Args... args)
{
    return std::max(v1, Max(args...));
}

/// @brief Calcola il massimo valore tra due numeri.
/// @param v1 Il primo numero.
/// @param args Il secondo numero.
/// @return Il massimo valore tra i due numeri.
template<> constexpr double Max(const double v1, const double args)
{
    return std::max(v1, args);
}

/// @brief Creare una lista di numeri normalizzati.
///
/// La normalizzazione fa in modo che la somma dei numeri sia pari ad uno, mantenendo però invariate le proporzioni tra
/// i numeri.
/// @tparam Args I tipi dei numeri con cui creare la lista.
/// @param args I numeri con cui creare la lista.
/// @return La lista di numeri normalizzati.
template<typename... Args> constexpr std::array<double, sizeof...(Args)> CreaListaNormalizzata(Args... args)
{
    const double n = (args + ...);
    return { args * (1.0 / n)... };
}

inline void CopiaStringa(char *destinazione, const char *sorgente, const size_t dimensioneDestinazione)
{
    strncpy(destinazione, sorgente, dimensioneDestinazione);
    destinazione[dimensioneDestinazione - 1] = '\0';
}
