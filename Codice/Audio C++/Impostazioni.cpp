#include "IntestazionePrecompilata.h"

#include "Impostazioni.h"

#include "CodaCancellazione.h"
#include "GUI/Disegnatore.h"
#include "Internazionalizzazione.h"

using namespace std::filesystem;
using namespace GUI;

// ----- -----

struct Impostazioni Impostazioni;

// ----- -----

static const constexpr char NOME_PROGRAMMA[]         = "Germen Pulchrum";
static const constexpr char NOME_FILE_IMPOSTAZIONI[] = "Impostazioni.json";

const path DIRETTORIO_IMPOSTAZIONI          = path(sago::getConfigHome()) / NOME_PROGRAMMA;
static const path PERCORSO_FILE_IMPOSTAIONI = DIRETTORIO_IMPOSTAZIONI / NOME_FILE_IMPOSTAZIONI;
const std::string PERCORSO_IMGUI_INI_FILE   = (DIRETTORIO_IMPOSTAZIONI / "ImGui.ini").string();

// ----- -----

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    struct Impostazioni, temaSelezionato, scalaGUI, linguaSelezionata, volumeGenerale, volumeGeneraleMuto)

// ----- -----

bool CaricaImpostazioni()
{
    CodaCancellazione eliminatori;

    // ----- -----

    if (!exists(PERCORSO_FILE_IMPOSTAIONI))
    {
        Impostazioni = {};
        return true;
    }

    // ----- -----

    std::ifstream file(PERCORSO_FILE_IMPOSTAIONI);
    if (!file.is_open())
    {
        std::cout << "[Errore] Impossibile aprire il file '" << PERCORSO_FILE_IMPOSTAIONI.string()
                  << "' da cui leggere le impostazioni.\n";
        return false;
    }
    eliminatori.Aggiungi(
        [&file]()
        {
            file.close();
        });

    nlohmann::json json = nlohmann::json::parse(file);

    // ----- -----

    Impostazioni = json.get<struct Impostazioni>();

    // ----- Ripristino i valori di default se invalidi -----

    if (Impostazioni.linguaSelezionata > Lingue.size()) Impostazioni.linguaSelezionata = 0;

    Impostazioni.scalaGUI = std::max(Impostazioni.scalaGUI, 0.0f);
    Impostazioni.scalaGUI = std::min(Impostazioni.scalaGUI, 2.0f);

    if (Impostazioni.temaSelezionato > Temi.size()) Impostazioni.temaSelezionato = 0;

    Impostazioni.volumeGenerale = std::max(Impostazioni.volumeGenerale, 0.0f);
    Impostazioni.volumeGenerale = std::min(Impostazioni.volumeGenerale, 1.0f);

    // ----- -----

    return true;
}

bool SalvaImpostazioni()
{
    CodaCancellazione eliminatori;

    // ----- -----

    try
    {
        create_directories(DIRETTORIO_IMPOSTAZIONI);
    }
    catch (filesystem_error ex)
    {
        std::cout << "[Errore] Impossibile creare la cartella dove salvare le impostazioni. Eccezione: " << ex.what()
                  << '\n';
        return false;
    }

    // ----- -----

    nlohmann::json json = Impostazioni;

    // ----- -----

    std::ofstream file(PERCORSO_FILE_IMPOSTAIONI, std::ios_base::trunc);
    if (!file.is_open())
    {
        std::cout << "[Errore] Impossibile aprire il file '" << PERCORSO_FILE_IMPOSTAIONI.string()
                  << "' in cui salvare le impostazioni.\n";
        return false;
    }
    eliminatori.Aggiungi(
        [&file]()
        {
            file.close();
        });

    file << json.dump(2) << '\n';

    return true;
}
