#pragma once

namespace GUI
{
    // ----- Disegnatore -----

    extern bool Esci;
    // Durata dell'ultimo fotogramma disegnato
    extern double DurataFotogramma; // [ms]
    extern float ScalaDPI;

    /// @brief Procedura principale di esecuzione del disegnatore (responsabile delle operazioni di disegno). Essa è
    /// strutturata come la funzione principale di un thread, per tanto essa termina la propria esecuzione solamente
    /// quando il disegnatore viene distrutto o si verifica un errore che impedisce la prosecuzione dell'esecuzione.
    /// @retval EXIT_SUCCESS se tutto ok.
    /// @retval EXIT_FAILURE in caso di errore.
    int Disegnatore();

    // ----- GUI -----

    void InizializzaGUI();
    void GUI();
    void AggiornaScalaGUI();

    // ----- Fonts -----

    extern ImFont *FontNormale;
    extern ImFont *FontGrassetto;
    extern ImFont *FontItalico;
    extern ImFont *FontItalicoGrassetto;

    // ----- Temi -----

    struct Tema
    {
        std::string nome;
        ImGuiStyle stile;
    };

    extern std::array<Tema, 5> Temi;

    void InizializzaTemi();
    void AggiornaNomiTemi();

    // ----- -----
}
