#pragma once

struct Impostazioni
{
    size_t temaSelezionato   = { 0 };     // [0, numero temi disponibili)
    float scalaGUI           = { 1.0f };  // [0.0, 2.0]
    size_t linguaSelezionata = { 0 };     // [0, numero lingue disponibili)
    float volumeGenerale     = { 0.70f }; // [0.0, 1.0]
    bool volumeGeneraleMuto  = false;
};

extern struct Impostazioni Impostazioni;
extern const std::filesystem::path DIRETTORIO_IMPOSTAZIONI;
extern const std::string PERCORSO_IMGUI_INI_FILE;

bool CaricaImpostazioni();
bool SalvaImpostazioni();
