#include "IntestazionePrecompilata.h"

#include "Internazionalizzazione.h"

#include "GUI/Disegnatore.h"
#include "GUI/Icone bandiere.h"
#include "Impostazioni.h"
#include "Lingue/en_GB.h"

using namespace boost::locale;
using namespace GUI;

generator GeneratoreMultiLingua;
std::array<Lingua, 3> Lingue;
struct TestiGUI TestiGUI;

static gnu_gettext::messages_info infoMsg;

static void AggiornaNomiLingue();
static void AggiornaTestiGUI();

// ----- -----

void InizializzaInternazionalizzazione()
{
    infoMsg.paths.emplace_back("");
    infoMsg.domains.emplace_back("Lingua");
    infoMsg.callback = CaricaCatalogoMessaggi;

    // ----- -----

    size_t index = 0;

    Lingue[index].bandiera = nullptr;
    Lingue[index].codice   = "";

    ++index;
    Lingue[index].bandiera = Fonts::ICON_IT;
    Lingue[index].codice   = "it_IT.UTF-8";

    ++index;
    Lingue[index].bandiera = Fonts::ICON_GB;
    Lingue[index].codice   = "en_GB.UTF-8";

    // ----- -----

    ImpostaLingua(Impostazioni.linguaSelezionata);
}

void ImpostaLingua(const size_t indice)
{
    const char *codice = Lingue[indice].codice;

    std::locale locale = GeneratoreMultiLingua(codice);

    // Se va impostata la lingua di sistema allora ignoro la nazione per l'italiano ed l'inglese.
    if (codice[0] == '\0')
    {
        const info &localeProp = std::use_facet<info>(locale);

        if (localeProp.language() == "it")
        {
            locale = GeneratoreMultiLingua("it_IT.UTF-8");
        }
        else if (localeProp.language() == "en")
        {
            locale = GeneratoreMultiLingua("en_GB.UTF-8");
        }
    }

    const info &proprietà = std::use_facet<info>(locale);

    infoMsg.language = proprietà.language();
    infoMsg.country  = proprietà.country();
    infoMsg.encoding = proprietà.encoding();
    infoMsg.variant  = proprietà.variant();

    // Configurazione caricamento del catalogo dei messaggi per i caratteri di tipo char.
    locale = std::locale(locale, gnu_gettext::create_messages_facet<char>(infoMsg));
    // Configurazione caricamento del catalogo dei messaggi per i caratteri UTF-8.
    locale = std::locale(locale, gnu_gettext::create_messages_facet<char8_t>(infoMsg));

    // Imposta per il C
    std::setlocale(LC_ALL, codice);

    // Imposta per il C++
    std::locale::global(locale);
    std::cin.imbue(locale);
    std::cout.imbue(locale);
    std::cerr.imbue(locale);
    std::wcin.imbue(locale);
    std::wcout.imbue(locale);
    std::wcerr.imbue(locale);

    // Imposta per ImGui
    if (ImGui::GetCurrentContext() != nullptr)
        ImGui::GetPlatformIO().Platform_LocaleDecimalPoint = *std::localeconv()->decimal_point;

    // -----

    AggiornaNomiLingue();
    AggiornaTestiGUI();
    AggiornaNomiTemi();
}

// Carica il file con il catalogo dei messaggi tradotti.
std::vector<char> CaricaCatalogoMessaggi(const std::string &nomeFile, const std::string & /*codifica*/)
{
    constexpr char prefisso[] = "/en_GB/";

    if (nomeFile.starts_with(prefisso))
        return std::vector<char>(
            CatalogoLingue::en_GB::Data, CatalogoLingue::en_GB::Data + std::size(CatalogoLingue::en_GB::Data));

    return std::vector<char>();
}

// ----- -----

static void AggiornaNomiLingue()
{
    size_t index = 0;

    Lingue[index++].nome = translate("Lingua di sistema");
    Lingue[index++].nome = translate("Italiano");
    Lingue[index++].nome = translate("Inglese (Inghilterra)");
}

static void AggiornaTestiGUI()
{
#define TESTO_GUI(var, testo) TestiGUI.var = translate(testo).str() + "###" testo
#include "GUI/TestiGUI.h"

#undef TESTO_GUI
}
