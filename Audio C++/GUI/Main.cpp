#include "IntestazionePrecompilata.h"

#include "Disegnatore.h"
#include "Impostazioni.h"
#include "Internazionalizzazione.h"

namespace GUI
{
    using namespace boost::locale;

    int MainGUI()
    {
        // ----- -----

        CaricaImpostazioni();

        // ----- Inizializzazione internazionalizzazione -----

        {
            InizializzaInternazionalizzazione();

            // ----- Stampa informazioni sull'internazionalizzazione

            std::cout << "----- Informazioni internazionalizzazione -----\n\n";

            const localization_backend_manager gestoreBackEnd = localization_backend_manager::global();
            std::cout << "Back-end disponibili:\n";
            for (std::string &name : gestoreBackEnd.get_all_backends())
                std::cout << "  " << name << '\n';
            std::cout << '\n';

            const auto &proprietà = std::use_facet<info>(std::locale());
            std::cout << "Internazionalizzazione utilizzata:\n";
            std::cout << "  Lingua  : " << proprietà.language() << '\n'
                      << "  Stato   : " << proprietà.country() << '\n'
                      << "  Codifica: " << proprietà.encoding() << '\n'
                      << "  Variante: " << proprietà.variant() << '\n';

            std::cout << '\n';

            // Se viene utilizzata la lingua di sistema
            if (Impostazioni.linguaSelezionata == 0)
            {
                // Carica la informazioni in base alle impostazioni dell'ambiente e del S.O.
                const std::locale localeDefault    = GeneratoreMultiLingua("");
                const auto &proprietàLocaleDefault = std::use_facet<info>(localeDefault);

                if (proprietàLocaleDefault.language() == "it")
                {
                    std::cout
                        << "Nota: utilizzo sempre it_IT per la lingua italiana indipendentemente dalla nazione.\n";
                    std::cout << '\n';
                }
                else if (proprietàLocaleDefault.language() == "en")
                {
                    std::cout << "Nota: utilizzo sempre en_GB per la lingua inglese indipendentemente dalla nazione.\n";
                    std::cout << '\n';
                }
            }
        }

        // ----- -----

        const int valoreUscita = Disegnatore();

        // ----- -----

        SalvaImpostazioni();

        // ----- -----

        return valoreUscita;
    }
}
