#pragma once

// ----- Configurazione -----

namespace Costanti
{
    static const constexpr bool AbilitaDebugVulkan       = true;
    static const constexpr bool AbilitaImGuiDebugPresent = false;
    // [TODO] VSync e frequenza di disegno:
    // * [Fatto] in ogni caso massimo 60 Hz poiché non sarebbe apprezzabile all'occhio umano o la differenza non sarebbe
    //   rilevante;
    // * [Fatto] limitare sempre la frequenza senza affidarsi alla Vulkan poiché alcuni driver (o schede video)
    //   potrebbero accettare il VSync ma poi non onorarlo;
    // * 60 Hz sono utili quando si sta disegnando qualcosa che potenzialmente cambia spesso (es. un grafico) altrimenti
    //   una frequenza più bassa va bene lo stesso (es. 30 Hz); così si consumano meno risorse e si allunga la durata
    //   della batteria;
    // * [Fatto] scegliere il numero di immagini nella catena di scambio, vedi
    //   https://docs.vulkan.org/samples/latest/samples/performance/swapchain_images/README.html
    static const constexpr bool AbilitàVSync            = true;
    static const constexpr size_t MaxFotogrammiInFlight = 3; // Minimo 2.
    // La GUI non verrà mai disegnata ad una frequenza superiore a questa.
    static const constexpr double FrequenzaMassimaDisegnatore = 60; // [Hz]
}

// ----- -----

/// @brief Converte un std::u8string in std::string copiando la stringa e senza modificarne la codifica.
/// @param stringa L'oggetto std::u8string da convertire.
/// @return Un oggetto std::string con lo stesso contenuto dell'oggetto std::u8string specificato.
constexpr std::string da_u8string(const std::u8string &stringa)
{
    return std::string(stringa.begin(), stringa.end());
}
