#include "IntestazionePrecompilata.h"

#include "Disegnatore.h"

#include "Clock.h"
#include "CodaCancellazione.h"
#include "CostantiEdAltro.h"
#include "Fonts/AwesomeSolid.h"
#include "Fonts/BabelStoneFlags.h"
#include "Fonts/IBMPlexSans.h"
#include "Fonts/IBMPlexSansItalic.h"
#include "Impostazioni.h"
#include "Internazionalizzazione.h"

// ----- -----

namespace GUI
{
    using namespace std::chrono;
    using namespace boost::locale;

    static SDL_Window *finestra;
    static vkb::Instance istanza;
    static VkSurfaceKHR superficie;
    static vkb::PhysicalDevice dispositivoFisico;
    static vkb::Device dispositivo;
    static uint32_t indiceFamigliaCodaGraphica;
    static VkQueue codaGrafica;
    static VkQueue codaPresentazione;
    static vkb::Swapchain catenaScambio;
    static std::vector<VkImageView> visteImmagini;
    static VkRenderPass renderPass;
    static std::vector<VkFramebuffer> buffersFotogrammi;
    static VkCommandPool commandPool;
    static std::array<VkCommandBuffer, Costanti::MaxFotogrammiInFlight> buffersComandi;
    static std::array<VkSemaphore, Costanti::MaxFotogrammiInFlight> semaforiImmagineDisponibile;
    static std::vector<VkSemaphore> semaforiDisegnoUltimato;
    static std::array<VkFence, Costanti::MaxFotogrammiInFlight> recinti;

    bool Esci               = false;
    double DurataFotogramma = 0.0; // [ms]
    float ScalaDPI          = 1.0f;

    ImFont *FontNormale          = nullptr;
    ImFont *FontGrassetto        = nullptr;
    ImFont *FontItalico          = nullptr;
    ImFont *FontItalicoGrassetto = nullptr;

    static void CheckVkResultFn(VkResult err);
    static bool CreaCatenaScambio();
    static bool RecuperaVisteImmagini();
    static bool CreaRenderPass();
    static bool CreaBuffersFotogrammi();
    static bool RicreaCatenaScambio();
    static void InizializzaScalaDPI();
    static void AggiornaScalaDPI();
    static bool AggiungiEmojiAlFont();
}

int GUI::Disegnatore()
{
    CodaCancellazione eliminatori;

    // ----- Stampa informazioni Vulkan -----

    {
        const vkb::Result<vkb::SystemInfo> risultato = vkb::SystemInfo::get_system_info();
        if (!risultato)
        {
            std::cout << "Impossibile recuperare le informazioni di sistema della Vulkan.\n";
            return EXIT_FAILURE;
        }

        const vkb::SystemInfo &info = risultato.value();

        std::cout << "----- Informazioni Vulkan -----\n\n";

        std::cout << "Versione API disponibile: " << VK_API_VERSION_MAJOR(info.instance_api_version) << '.'
                  << VK_API_VERSION_MINOR(info.instance_api_version) << '.'
                  << VK_API_VERSION_PATCH(info.instance_api_version) << '.'
                  << VK_API_VERSION_VARIANT(info.instance_api_version) << '\n';

        std::cout << "Estensioni disponibili:";
        if (info.available_extensions.empty()) std::cout << " nessuna\n";
        else std::cout << '\n';
        for (const VkExtensionProperties &p : info.available_extensions)
            std::cout << "  " << p.extensionName << '\n';

        size_t masLun = std::numeric_limits<size_t>::min();
        for (const VkLayerProperties &p : info.available_layers)
            masLun = std::max(masLun, strlen(p.layerName));

        std::cout << "Layers disponibili:";
        if (info.available_layers.empty()) std::cout << " nessuno\n";
        else std::cout << '\n';
        for (const VkLayerProperties &p : info.available_layers)
            std::cout << "  " << std::left << std::setw(masLun) << p.layerName << ": " << p.description << '\n';

        std::cout << "\n----- -----\n\n";
    }

    // ----- Inizializzazione SDL -----

    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::cout << "Inizializzazione SDL3 fallita: " << SDL_GetError() << '\n';
            return EXIT_FAILURE;
        }

        eliminatori.Aggiungi(
            []()
            {
                SDL_Quit();
            });
    }

    // ----- Finestra -----

    {
        finestra = SDL_CreateWindow(
            "Germen Pulchrum",
            1'280,
            800,
            SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
        if (finestra == nullptr)
        {
            std::cout << "Creazione finestra fallita: " << SDL_GetError() << '\n';
            return EXIT_FAILURE;
        }

        eliminatori.Aggiungi(
            []()
            {
                SDL_DestroyWindow(finestra);
            });
    }

    // ----- Estensioni Vulkan per SDL3 -----

    uint32_t numeroEstensioni;
    const char *const *estensioni;

    {
        estensioni = SDL_Vulkan_GetInstanceExtensions(&numeroEstensioni);
        if (estensioni == nullptr)
        {
            std::cout << "Recupero estensioni Vulkan richieste dalla libreria SDL3 fallita: " << SDL_GetError() << '\n';
            return EXIT_FAILURE;
        }

        std::cout << "Estensioni per SDL3:\n";
        for (size_t i = 0; i < numeroEstensioni; ++i)
            std::cout << "  " << estensioni[i] << '\n';

        std::cout << "\n----- -----\n\n";
    }

    // ----- Istanza Vulkan -----

    {
        vkb::InstanceBuilder costruttore;

        if constexpr (Costanti::AbilitaDebugVulkan)
        {
            // Recupero nuovamente le informazioni di sistema poiché l'oggetto vkb::SystemInfo ha un costruttore
            // privato e non se ne può fare una copia.
            const vkb::Result<vkb::SystemInfo> risultato = vkb::SystemInfo::get_system_info();

            costruttore.use_default_debug_messenger();

            if (risultato.value().validation_layers_available) costruttore.enable_validation_layers();
            else std::cout << "[Avviso] Strato di validazione della Vulkan non disponibile.\n";
        }

        const vkb::Result<vkb::Instance> risultato =
            costruttore.require_api_version(VK_API_VERSION_1_1).enable_extensions(numeroEstensioni, estensioni).build();
        if (!risultato)
        {
            std::cout << "Creazione istanza Vulkan fallita: " << risultato.error().message() << ".\n";
            return EXIT_FAILURE;
        }

        istanza = risultato.value();

        eliminatori.Aggiungi(
            []()
            {
                vkb::destroy_instance(istanza);
            });
    }

    // ----- Superficie -----

    {
        if (!SDL_Vulkan_CreateSurface(finestra, istanza, nullptr, &superficie))
        {
            std::cout << "Creazione superficie fallita: " << SDL_GetError() << '\n';
            return EXIT_FAILURE;
        }

        eliminatori.Aggiungi(
            []()
            {
                SDL_Vulkan_DestroySurface(istanza, superficie, nullptr);
            });
    }

    // ----- Selezione scheda video -----

    {
        vkb::PhysicalDeviceSelector selettore(istanza);

        // Attenzione: vk-bootstrap non permette di specificare quali tipi di code sono richiesti durante la ricerca
        // di una scheda video idonea; conseguentemente l'inizializzazione della scheda video potrebbe fallire
        // poiché la scheda selezionata non supporta i tipi di code necessari.
        const vkb::Result<vkb::PhysicalDevice> risultato =
            selettore.set_surface(superficie)
                .require_present(true)
                .prefer_gpu_device_type(vkb::PreferredDeviceType::integrated)
                .disable_portability_subset()
                .select();
        if (!risultato)
        {
            if (risultato.error() == vkb::PhysicalDeviceError::no_suitable_device)
            {
                const auto &detailed_reasons = risultato.detailed_failure_reasons();
                if (!detailed_reasons.empty())
                {
                    std::cout << "Selezione della scheda video fallita, ragioni:\n";
                    for (const std::string &reason : detailed_reasons)
                        std::cout << "  " << reason << '\n';
                }
            }
            else std::cout << "Selezione della scheda video fallita: " << risultato.error().message() << '\n';

            return EXIT_FAILURE;
        }

        dispositivoFisico = risultato.value();

        std::cout << "Scheda video selezionata: " << risultato.value().name << '\n';

        std::cout << "\n----- -----\n\n";
    }

    // ----- Inizializzazione scheda video -----

    {
        float priorità = 1.0f;

        vkb::CustomQueueDescription descrizioneCode[] = {
            vkb::CustomQueueDescription(0, 1, &priorità), // grafica
            vkb::CustomQueueDescription(0, 1, &priorità)  // presentazione
        };

        size_t numeroCode;

        // Seleziono la prima famiglia di code di tipo grafico che trovo in modo da creare una sola coda di tale
        // tipo.
        {
            std::vector<VkQueueFamilyProperties> famiglieCode = dispositivoFisico.get_queue_families();

            bool graficaTrovata       = false;
            bool presentazioneTrovata = false;

            for (size_t indiceFamiglia = 0;
                 indiceFamiglia < famiglieCode.size() && !graficaTrovata && !presentazioneTrovata;
                 ++indiceFamiglia)
            {
                if (!graficaTrovata && famiglieCode[indiceFamiglia].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    indiceFamigliaCodaGraphica = static_cast<uint32_t>(indiceFamiglia);
                    descrizioneCode[0].index   = static_cast<uint32_t>(indiceFamiglia);
                    graficaTrovata             = true;
                }

                if (!presentazioneTrovata)
                {
                    VkBool32 supportata;
                    const VkResult risultato = vkGetPhysicalDeviceSurfaceSupportKHR(
                        dispositivoFisico, static_cast<uint32_t>(indiceFamiglia), superficie, &supportata);
                    if (risultato == VK_SUCCESS && supportata)
                    {
                        descrizioneCode[1].index = static_cast<uint32_t>(indiceFamiglia);
                        presentazioneTrovata     = true;
                    }
                }
            }

            if (!graficaTrovata)
            {
                std::cout << "Nessuna coda grafica supportata dal dispositivo di rendering.\n";
                return EXIT_FAILURE;
            }

            if (!presentazioneTrovata)
            {
                std::cout << "Nessuna coda per la presentazione supportata dal dispositivo di rendering.\n";
                return EXIT_FAILURE;
            }

            // Evito di duplicare le informazioni se la stessa famiglia di code supporta sia la grafica che la
            // presentazione.
            numeroCode = descrizioneCode[0].index == descrizioneCode[1].index ? 1 : 2;
        }

        vkb::DeviceBuilder costruttore(dispositivoFisico);
        const vkb::Result<vkb::Device> risultato = costruttore.custom_queue_setup(numeroCode, descrizioneCode).build();
        if (!risultato)
        {
            std::cout << "Creazione dispositivo di rendering fallita: " << risultato.error().message() << '\n';
            return EXIT_FAILURE;
        }

        dispositivo = risultato.value();

        eliminatori.Aggiungi(
            []()
            {
                vkb::destroy_device(dispositivo);
            });
    }

    // ----- Code -----

    {
        vkb::Result<VkQueue> risultato = dispositivo.get_queue(vkb::QueueType::graphics);
        if (!risultato)
        {
            std::cout << "Nessuna coda grafica supportata dal dispositivo di rendering: " << risultato.error().message()
                      << '\n';
            return EXIT_FAILURE;
        }

        codaGrafica = risultato.value();

        risultato = dispositivo.get_queue(vkb::QueueType::present);
        if (!risultato)
        {
            std::cout << "Nessuna coda per la presentazione supportata dal dispositivo di rendering: "
                      << risultato.error().message() << '\n';
            return EXIT_FAILURE;
        }

        codaPresentazione = risultato.value();
    }

    // ----- Catena di scambio -----

    {
        if (!CreaCatenaScambio()) return EXIT_FAILURE;

        eliminatori.Aggiungi(
            []()
            {
                vkb::destroy_swapchain(catenaScambio);
            });
    }

    // ----- Vista immagini della catena di scambio -----

    {
        if (!RecuperaVisteImmagini()) return EXIT_FAILURE;

        eliminatori.Aggiungi(
            []()
            {
                catenaScambio.destroy_image_views(visteImmagini);
            });
    }

    // ----- Render pass -----

    {
        if (!CreaRenderPass()) return EXIT_FAILURE;

        eliminatori.Aggiungi(
            []()
            {
                vkDestroyRenderPass(dispositivo, renderPass, nullptr);
            });
    }

    // ----- Scala finestra e GUI -----

    InizializzaScalaDPI();

    // ----- ImGui -----

    {
        IMGUI_CHECKVERSION();

        ImGui::CreateContext();
        eliminatori.Aggiungi(
            []()
            {
                ImGui::DestroyContext();
            });

        ImGuiIO &io     = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        io.ConfigDebugIsDebuggerPresent = Costanti::AbilitaImGuiDebugPresent;
        io.ConfigWindowsResizeFromEdges = true;

        io.IniFilename = PERCORSO_IMGUI_INI_FILE.data();

        ImGui::GetPlatformIO().Platform_LocaleDecimalPoint = *std::localeconv()->decimal_point;
    }

    // ----- ImGui temi -----

    {
        InizializzaTemi();
        ImGui::GetStyle() = Temi[Impostazioni.temaSelezionato].stile;
    }

    // ----- ImGui fonts -----

    {
        ImGuiIO &io = ImGui::GetIO();

        // ----- Normale

        {
            ImFontConfig configFont;

            snprintf(configFont.Name, std::size(configFont.Name), Fonts::IBMPlexSans::Name);
            configFont.Flags = ImFontFlags_NoLoadError;

            FontNormale = io.Fonts->AddFontFromMemoryCompressedTTF(
                Fonts::IBMPlexSans::CompressedData, std::size(Fonts::IBMPlexSans::CompressedData), 0.0f, &configFont);
            if (FontNormale == nullptr)
            {
                std::cout << "Impossible caricare il font per lo stile normale.\n";
                return EXIT_FAILURE;
            }

            // ----- Aggiunta emoji colorate

            if (!AggiungiEmojiAlFont())
            {
                std::cout << "Impossible caricare il font con le emoji colorate.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Grassetto

        {
            ImFontConfig configFont;

            configFont.Name[0] = '\0';
            snprintf(configFont.Name, std::size(configFont.Name), "%s %s", Fonts::IBMPlexSans::Name, "grassetto");
            configFont.Flags           = ImFontFlags_NoLoadError;
            configFont.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_Bold;

            FontGrassetto = io.Fonts->AddFontFromMemoryCompressedTTF(
                Fonts::IBMPlexSans::CompressedData, std::size(Fonts::IBMPlexSans::CompressedData), 0.0f, &configFont);
            if (FontGrassetto == nullptr)
            {
                std::cout << "Impossible caricare il font per lo stile grassetto.\n";
                return EXIT_FAILURE;
            }

            // ----- Aggiunta emoji colorate

            if (!AggiungiEmojiAlFont())
            {
                std::cout << "Impossible caricare il font con le emoji colorate.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Italico

        {
            ImFontConfig configFont;

            snprintf(configFont.Name, std::size(configFont.Name), Fonts::IBMPlexSansItalic::Name);
            configFont.Flags = ImFontFlags_NoLoadError;

            FontItalico = io.Fonts->AddFontFromMemoryCompressedTTF(
                Fonts::IBMPlexSansItalic::CompressedData,
                std::size(Fonts::IBMPlexSansItalic::CompressedData),
                0.0f,
                &configFont);
            if (FontItalico == nullptr)
            {
                std::cout << "Impossible caricare il font per lo stile italico.\n";
                return EXIT_FAILURE;
            }

            // ----- Aggiunta emoji colorate

            if (!AggiungiEmojiAlFont())
            {
                std::cout << "Impossible caricare il font con le emoji colorate.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Italico grassetto

        {
            ImFontConfig configFont;

            snprintf(configFont.Name, std::size(configFont.Name), "%s %s", Fonts::IBMPlexSansItalic::Name, "grassetto");
            configFont.Flags           = ImFontFlags_NoLoadError;
            configFont.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_Bold;

            FontItalicoGrassetto = io.Fonts->AddFontFromMemoryCompressedTTF(
                Fonts::IBMPlexSansItalic::CompressedData,
                std::size(Fonts::IBMPlexSansItalic::CompressedData),
                0.0f,
                &configFont);
            if (FontItalicoGrassetto == nullptr)
            {
                std::cout << "Impossible caricare il font per lo stile italico.\n";
                return EXIT_FAILURE;
            }

            // ----- Aggiunta emoji colorate

            if (!AggiungiEmojiAlFont())
            {
                std::cout << "Impossible caricare il font con le emoji colorate.\n";
                return EXIT_FAILURE;
            }
        }
    }

    // ----- ImGui back-end SDL3 -----

    {
        ImGui_ImplSDL3_InitForVulkan(finestra);

        eliminatori.Aggiungi(
            []()
            {
                ImGui_ImplSDL3_Shutdown();
            });
    }

    // ----- ImGui back-end Vulkan -----

    {
        ImGui_ImplVulkan_InitInfo info    = {};
        info.Instance                     = istanza;
        info.PhysicalDevice               = dispositivoFisico;
        info.Device                       = dispositivo;
        info.QueueFamily                  = indiceFamigliaCodaGraphica;
        info.Queue                        = codaGrafica;
        info.DescriptorPoolSize           = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
        info.MinImageCount                = catenaScambio.requested_min_image_count;
        info.ImageCount                   = catenaScambio.image_count;
        info.PipelineInfoMain.RenderPass  = renderPass;
        info.PipelineInfoMain.Subpass     = 0;
        info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        info.CheckVkResultFn              = CheckVkResultFn;

        if (!ImGui_ImplVulkan_Init(&info))
        {
            std::cout << "Inizializzazione del back-end Vulkan di ImGui fallita.\n";
            return EXIT_FAILURE;
        }

        eliminatori.Aggiungi(
            []()
            {
                ImGui_ImplVulkan_Shutdown();
            });
    }

    // ----- ImPlot -----

    {
        ImPlot::CreateContext();
        eliminatori.Aggiungi(
            []()
            {
                ImPlot::DestroyContext();
            });
    }

    // ----- Buffers fotogrammi -----

    {
        if (!CreaBuffersFotogrammi()) return EXIT_FAILURE;

        eliminatori.Aggiungi(
            []()
            {
                for (const VkFramebuffer &frameBuffer : buffersFotogrammi)
                    vkDestroyFramebuffer(dispositivo, frameBuffer, nullptr);
            });
    }

    // ----- Command pool -----

    {
        VkCommandPoolCreateInfo info = {};

        info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = indiceFamigliaCodaGraphica;

        const VkResult risultato = vkCreateCommandPool(dispositivo, &info, nullptr, &commandPool);
        if (risultato != VK_SUCCESS)
        {
            std::cout << "Creazione command pool fallita.\n";
            return EXIT_FAILURE;
        }

        eliminatori.Aggiungi(
            []()
            {
                vkDestroyCommandPool(dispositivo, commandPool, nullptr);
            });
    }

    // ----- Buffers comandi -----

    {
        VkCommandBufferAllocateInfo info = {};

        info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool        = commandPool;
        info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = static_cast<uint32_t>(buffersComandi.size());

        const VkResult risultato = vkAllocateCommandBuffers(dispositivo, &info, buffersComandi.data());
        if (risultato != VK_SUCCESS)
        {
            std::cout << "Creazione buffers comandi fallita.\n";
            return EXIT_FAILURE;
        }
    }

    // ----- Semafori -----

    {
        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        semaforiImmagineDisponibile.fill(VK_NULL_HANDLE);

        for (size_t i = 0; i < semaforiImmagineDisponibile.size(); ++i)
        {
            const VkResult risultato = vkCreateSemaphore(dispositivo, &info, nullptr, &semaforiImmagineDisponibile[i]);
            if (risultato != VK_SUCCESS)
            {
                std::cout << "Creazione semaforo " << i << " di immagine disponibile fallita.\n";
                return EXIT_FAILURE;
            }
        }

        eliminatori.Aggiungi(
            []()
            {
                for (const VkSemaphore &semforo : semaforiImmagineDisponibile)
                    vkDestroySemaphore(dispositivo, semforo, nullptr);
            });

        // -----

        semaforiDisegnoUltimato.resize(catenaScambio.image_count);
        std::fill(semaforiDisegnoUltimato.begin(), semaforiDisegnoUltimato.end(), VK_NULL_HANDLE);

        for (size_t i = 0; i < semaforiDisegnoUltimato.size(); ++i)
        {
            const VkResult risultato = vkCreateSemaphore(dispositivo, &info, nullptr, &semaforiDisegnoUltimato[i]);
            if (risultato != VK_SUCCESS)
            {
                std::cout << "Creazione semaforo " << i << " di disegno ultimato fallita.\n";
                return EXIT_FAILURE;
            }
        }

        eliminatori.Aggiungi(
            []()
            {
                for (const VkSemaphore &semforo : semaforiDisegnoUltimato)
                    vkDestroySemaphore(dispositivo, semforo, nullptr);
            });
    }

    // ----- Recinto -----

    {
        VkFenceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        recinti.fill(VK_NULL_HANDLE);

        for (size_t i = 0; i < recinti.size(); ++i)
        {
            const VkResult risultato = vkCreateFence(dispositivo, &info, nullptr, &recinti[i]);
            if (risultato != VK_SUCCESS)
            {
                std::cout << "Creazione recinto " << i << " fallita.\n";
                return EXIT_FAILURE;
            }
        }

        eliminatori.Aggiungi(
            []()
            {
                for (const VkFence &recinto : recinti)
                    vkDestroyFence(dispositivo, recinto, nullptr);
            });
    }

    // ----- Stampa informazioni per debug -----

    {
        std::cout << std::hex << std::uppercase;

        std::cout << "Semafori immagine disponibile[" << semaforiImmagineDisponibile.size() << "]:\n";
        for (const VkSemaphore &semaforo : semaforiImmagineDisponibile)
            std::cout << "  0x" << semaforo << '\n';

        std::cout << "Semafori disegno ultimato[" << semaforiDisegnoUltimato.size() << "]:\n";
        for (const VkSemaphore &semaforo : semaforiDisegnoUltimato)
            std::cout << "  0x" << semaforo << '\n';

        std::cout << "Recinti[" << recinti.size() << "]:\n";
        for (const VkFence &recento : recinti)
            std::cout << "  0x" << recento << '\n';

        std::cout << std::dec;
    }

    // ----- Inizializzazione GUI -----

    InizializzaGUI();

    // ----- Conclusione inizializzazione -----

    {
        SDL_ShowWindow(finestra);

        std::cout << "\n----- -----\n\n";
    }

    // ----- -----

    Clock clock(1.0 / Costanti::FrequenzaMassimaDisegnatore);

    size_t indiceFotogramma    = 0;
    bool ricreareCatenaScambio = false;
    bool pausa                 = false;

    high_resolution_clock::time_point tempoInizio = high_resolution_clock::now();

    while (!Esci)
    {
        // ----- Processazione eventi S.O. ed eventualmente sospensione operazioni di disegno -----

        {
            SDL_Event evento;

            // Attenzione: la funzione SDL_PollEvent() va chiamata solamente quando non si è in pausa, mentre
            // SDL_WaitEvent() va chiamata solamente quando si è in pausa; in modo tale da smette di disegnare
            // quando si è in pausa.
            while ((!pausa && SDL_PollEvent(&evento)) || (pausa && SDL_WaitEvent(&evento)))
            {
                ImGui_ImplSDL3_ProcessEvent(&evento);

                switch (evento.type)
                {
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    case SDL_EVENT_QUIT:
                        Esci = true;
                        break;

                    case SDL_EVENT_WINDOW_MINIMIZED:
                    case SDL_EVENT_WINDOW_HIDDEN:
                        pausa = true;
                        break;
                    case SDL_EVENT_WINDOW_RESTORED:
                    case SDL_EVENT_WINDOW_SHOWN:
                        pausa = false;
                        break;

                    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
                        AggiornaScalaDPI();
                        break;

                    case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
                        // evento.window.data1; // idDisplay;
                        // SDL_DisplayID idDisplay   = SDL_GetDisplayForWindow(finestra);
                        // SDL_DisplayMode *modalità = SDL_GetCurrentDisplayMode(idDisplay);
                        // modalità->refresh_rate; // 0.0f quando non specificato.

                    case SDL_EVENT_WINDOW_RESIZED:
                    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                        ricreareCatenaScambio = true;
                        break;
                }

                // Se devo uscire smetto subito di processare gli eventi.
                if (Esci) break;
            }

            // Se devo uscire non faccio il rendering del fotogramma, esco subito.
            if (Esci) break;
        }

        // ----- ImGui - inizio nuovo fotogramma -----

        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
        }

        // ----- GUI -----

        GUI();

        // Se devo uscire non faccio il rendering del fotogramma, esco subito.
        if (Esci) break;

        // ----- ImGui - fine fotogramma e preparazione dati per il disegno -----

        ImGui::Render();

        // ----- Variabili locali -----

        uint32_t indiceImmagine;

        // ----- Attesa fine utilizzo risorse del fotogramma corrente da parte dei comandi di disegno precedenti -----

        {
            const VkResult risultato = vkWaitForFences(dispositivo, 1, &recinti[indiceFotogramma], VK_TRUE, UINT64_MAX);
            if (risultato != VK_SUCCESS)
            {
                std::cout << "Attesa del recinto fallita.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Acquisizione successiva immagine su cui disegnare -----

        {
            const VkResult risultato = vkAcquireNextImageKHR(
                dispositivo,
                catenaScambio,
                UINT64_MAX,
                semaforiImmagineDisponibile[indiceFotogramma],
                VK_NULL_HANDLE,
                &indiceImmagine);

            if (risultato == VK_ERROR_OUT_OF_DATE_KHR || risultato == VK_SUBOPTIMAL_KHR) ricreareCatenaScambio = true;
            else if (risultato != VK_SUCCESS)
            {
                std::cout << "Acquisizione della prossima immagine della catena di scambio fallita.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Reset sincronizzazione risorse fotogramma corrente tra programma e Vulkan -----

        {
            const VkResult risultato = vkResetFences(dispositivo, 1, &recinti[indiceFotogramma]);
            if (risultato != VK_SUCCESS)
            {
                std::cout << "Reset del recinto fallito.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Inizio registrazione comandi di disegno -----

        {
            VkCommandBufferBeginInfo info = {};

            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            const VkResult risultato = vkBeginCommandBuffer(buffersComandi[indiceFotogramma], &info);
            if (risultato != VK_SUCCESS)
            {
                std::cout << "Inizio registrazione comandi di disegno fallita.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Inizio render pass -----

        {
            VkRenderPassBeginInfo info = {};

            info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass        = renderPass;
            info.framebuffer       = buffersFotogrammi[indiceImmagine];
            info.renderArea.offset = { 0, 0 };
            info.renderArea.extent = catenaScambio.extent;

            vkCmdBeginRenderPass(buffersComandi[indiceFotogramma], &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        // ----- Registrazione comandi di disegno di ImGui -----

        {
            ImDrawData *datiDisegno = ImGui::GetDrawData();
            ImGui_ImplVulkan_RenderDrawData(datiDisegno, buffersComandi[indiceFotogramma]);
        }

        // ----- Termine render pass -----

        {
            vkCmdEndRenderPass(buffersComandi[indiceFotogramma]);
        }

        // ----- Termine registrazione comandi di disegno -----

        {
            const VkResult risultato = vkEndCommandBuffer(buffersComandi[indiceFotogramma]);
            if (risultato != VK_SUCCESS)
            {
                std::cout << "Conclusione registrazione comandi di disegno fallita.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Variabili locali -----

        const VkSemaphore semaforiDaAspettare[] = { semaforiImmagineDisponibile[indiceFotogramma] };
        const VkSemaphore semaforiDaSegnalare[] = { semaforiDisegnoUltimato[indiceImmagine] };

        // ----- Invio comandi di disegno alla scheda video -----

        {
            const constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

            VkSubmitInfo submitInfo = {};

            submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount   = 1;
            submitInfo.pWaitSemaphores      = semaforiDaAspettare;
            submitInfo.pWaitDstStageMask    = waitStages;
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &buffersComandi[indiceFotogramma];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = semaforiDaSegnalare;

            const VkResult risultato = vkQueueSubmit(codaGrafica, 1, &submitInfo, recinti[indiceFotogramma]);
            if (risultato != VK_SUCCESS)
            {
                std::cout << "Invio del buffer dei comandi per l'esecuzione fallito.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Presentazione immagine disegnata -----

        {
            const VkSwapchainKHR cateneScambio[] = { catenaScambio };

            VkPresentInfoKHR info = {};

            info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores    = semaforiDaSegnalare;
            info.swapchainCount     = 1;
            info.pSwapchains        = cateneScambio;
            info.pImageIndices      = &indiceImmagine;

            const VkResult risultato = vkQueuePresentKHR(codaPresentazione, &info);
            if (risultato == VK_ERROR_OUT_OF_DATE_KHR || risultato == VK_SUBOPTIMAL_KHR) ricreareCatenaScambio = true;
            else if (risultato != VK_SUCCESS)
            {
                std::cout << "Presentazione immagine disegnata fallita.\n";
                return EXIT_FAILURE;
            }
        }

        // ----- Ricreazione catena di scambio (e risorse associate) secondo necessità -----

        if (ricreareCatenaScambio)
        {
            if (!RicreaCatenaScambio()) return EXIT_FAILURE;
            ricreareCatenaScambio = false;
        }

        // ----- Avanzamento al fotogramma successivo -----

        {
            indiceFotogramma = (indiceFotogramma + 1) % Costanti::MaxFotogrammiInFlight;
        }

        // ----- Limite velocità e misurazione del tempo impiegato a disegnare il fotogramma -----

        // Alcuni driver, schede video o implementazioni della Vulkan potrebbe accettare il VSync in fase di
        // configurazione ma poi non lo onorano (per esempio llvmpipe), per tanto utilizzo un clock per assicurarmi
        // di non superare la frequenza massima di disegno.
        if (Costanti::AbilitàVSync) DurataFotogramma = clock.AspettaTicchettioSuccessivo() * 1000.0;
        else
        {
            high_resolution_clock::time_point tempoFine = high_resolution_clock::now();
            DurataFotogramma = duration_cast<DurataMillisecondi>(tempoFine - tempoInizio).count();
            tempoInizio      = tempoFine;
        }
    }

    // ----- Attesa che la scheda video termini tutte le operazioni -----

    {
        const VkResult risultato = vkDeviceWaitIdle(dispositivo);
        if (risultato != VK_SUCCESS)
            std::cout << "[Avviso] Impossibile attendere che la scheda video finisca tutte le operazioni prima di "
                         "rilasciare le risorse.\n";
    }

    // ----- Pulitura ed uscita -----

    return EXIT_SUCCESS;
}

static void GUI::CheckVkResultFn(VkResult err)
{
#ifdef WIN32
    if (err != VK_SUCCESS) DebugBreak();
#else
    if (err != VK_SUCCESS) raise(SIGTRAP);
#endif
}

static bool GUI::CreaCatenaScambio()
{
    int larghezza, altezza;
    if (!SDL_GetWindowSizeInPixels(finestra, &larghezza, &altezza))
    {
        std::cout << "SDL_GetWindowSizeInPixels() fallita: " << SDL_GetError() << '\n';
        return false;
    }

    const constexpr VkPresentModeKHR modalitàPresentazione =
        Costanti::AbilitàVSync ? VK_PRESENT_MODE_FIFO_KHR      // Tradizionale sincronizzazione con lo schermo (VSync).
                               : VK_PRESENT_MODE_IMMEDIATE_KHR // Nessuna sincronizzazione.
        ;

    vkb::SwapchainBuilder costruttore(dispositivo);
    const vkb::Result<vkb::Swapchain> risultato =
        costruttore.set_desired_min_image_count(3)
            .set_desired_extent(larghezza, altezza)
            .set_desired_format({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR })
            .add_fallback_format({ VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR })
            .add_fallback_format({ VK_FORMAT_B8G8R8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR })
            .add_fallback_format({ VK_FORMAT_R8G8B8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR })
            .set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            .set_desired_present_mode(modalitàPresentazione)
            .set_old_swapchain(catenaScambio)
            .build();
    if (!risultato)
    {
        std::cout << "Ricreazione della catena di scambio fallita: " << risultato.error().message() << '\n';
        catenaScambio.swapchain = VK_NULL_HANDLE;
        return false;
    }

    vkb::destroy_swapchain(catenaScambio);

    catenaScambio = risultato.value();

    if (catenaScambio.present_mode != modalitàPresentazione)
    {
        std::cout << "Modalità di presentazione desiderata non supportata, utilizzo "
                  << string_VkPresentModeKHR(catenaScambio.present_mode) << ".\n";
    }

    if (catenaScambio.image_count != 3)
    {
        std::cout << "Buffer triplo non supportato, utilizzo ";

        if (catenaScambio.image_count == 2) std::cout << " un buffer doppio";
        else if (catenaScambio.image_count == 1) std::cout << " un buffer singolo";
        else std::cout << catenaScambio.image_count << " buffer";

        std::cout << ".\n";
    }

    return true;
}

static bool GUI::RecuperaVisteImmagini()
{
    const vkb::Result<std::vector<VkImageView>> risultato = catenaScambio.get_image_views();
    if (!risultato)
    {
        std::cout << "Creazione vista delle immagini della scatena di scambio fallita: " << risultato.error().message()
                  << '\n';
        return false;
    }

    visteImmagini = risultato.value();

    return true;
}

static bool GUI::CreaRenderPass()
{
    VkAttachmentDescription colorAttachment = {};

    colorAttachment.format         = catenaScambio.image_format;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};

    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};

    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkSubpassDependency dependency = {};

    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};

    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments    = &colorAttachment;
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    const VkResult risultato = vkCreateRenderPass(dispositivo, &renderPassInfo, nullptr, &renderPass);
    if (risultato != VK_SUCCESS)
    {
        std::cout << "Creazione del render pass fallita.\n";
        return false;
    }

    return true;
}

static bool GUI::CreaBuffersFotogrammi()
{
    buffersFotogrammi.resize(visteImmagini.size());
    std::fill(buffersFotogrammi.begin(), buffersFotogrammi.end(), VK_NULL_HANDLE);

    for (size_t i = 0; i < visteImmagini.size(); i++)
    {
        const VkImageView allegati[] = { visteImmagini[i] };

        VkFramebufferCreateInfo info{};
        info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass      = renderPass;
        info.attachmentCount = 1;
        info.pAttachments    = allegati;
        info.width           = catenaScambio.extent.width;
        info.height          = catenaScambio.extent.height;
        info.layers          = 1;

        const VkResult risultato = vkCreateFramebuffer(dispositivo, &info, nullptr, &buffersFotogrammi[i]);
        if (risultato != VK_SUCCESS)
        {
            std::cout << "Creazione del buffer del fotogramma " << i << " fallita.\n";
            return false;
        }
    }

    return true;
}

static bool GUI::RicreaCatenaScambio()
{
    {
        const VkResult risultato = vkDeviceWaitIdle(dispositivo);
        if (risultato != VK_SUCCESS)
            std::cout << "[Avviso] Impossibile attendere che la scheda video finisca tutte le operazioni prima di "
                         "ricostruire la catena di scambio.\n";
    }

    // -----

    for (VkFramebuffer frameBuffer : buffersFotogrammi)
        vkDestroyFramebuffer(dispositivo, frameBuffer, nullptr);

    vkDestroyRenderPass(dispositivo, renderPass, nullptr);

    for (VkImageView vistaImmagine : visteImmagini)
        vkDestroyImageView(dispositivo, vistaImmagine, nullptr);

    // -----

    // Nota: il render pass è da ricostruire solamente se il formato della superficie nella nuova catena di scambio
    // è diverso rispetto al precedente.

    if (!CreaCatenaScambio() || !RecuperaVisteImmagini() || !CreaRenderPass() || !CreaBuffersFotogrammi()) return false;

    // -----

    return true;
}

static void GUI::InizializzaScalaDPI()
{
    // ----- Recupera il fattore di scala della finestra appropriato per i DPI dello schermo.

    ScalaDPI = SDL_GetWindowDisplayScale(finestra);
    if (ScalaDPI == 0.0f)
    {
        std::cout << "[Errore] SDL_GetWindowDisplayScale(): " << SDL_GetError() << '\n';
        // In caso di errore usa un valore di default al fine di avere un valore valido.
        ScalaDPI = 1.0f;
    }

    // ----- Finestra

    SDL_SetWindowSize(finestra, static_cast<int>(1'280 * ScalaDPI), static_cast<int>(800 * ScalaDPI));
    SDL_SetWindowPosition(finestra, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

static void GUI::AggiornaScalaDPI()
{
    // ----- Recupera il fattore di scala della finestra appropriato per i DPI dello schermo.

    ScalaDPI = SDL_GetWindowDisplayScale(finestra);
    if (ScalaDPI == 0.0f)
    {
        std::cout << "[Errore] SDL_GetWindowDisplayScale(): " << SDL_GetError() << '\n';
        // In caso di errore usa un valore di default al fine di avere un valore valido.
        ScalaDPI = 1.0f;
    }

    // ----- Finestra

    int larghezza, altezza;
    if (!SDL_GetWindowSize(finestra, &larghezza, &altezza))
    {
        std::cout << "[Errore] SDL_GetWindowSize(): " << SDL_GetError() << '\n';
        // In caso di errore utilizzo la dimensione di default della finestra in modo da avere dei dati validi.
        larghezza = 1'280;
        altezza   = 800;
    }

    SDL_SetWindowSize(finestra, static_cast<int>(larghezza * ScalaDPI), static_cast<int>(altezza * ScalaDPI));

    // ----- GUI

    AggiornaScalaGUI();
}

static bool GUI::AggiungiEmojiAlFont()
{
    // ----- BabelStone Flags -----

    {
        ImFontConfig configFont;

        snprintf(configFont.Name, std::size(configFont.Name), Fonts::BabelStoneFlags::Name);
        configFont.Flags           = ImFontFlags_NoLoadError;
        configFont.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_LoadColor;
        configFont.MergeMode       = true;

        ImFont *font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
            Fonts::BabelStoneFlags::CompressedData,
            std::size(Fonts::BabelStoneFlags::CompressedData),
            0.0f,
            &configFont);
        if (font == nullptr)
        {
            std::cout << "Impossible caricare il font con le bandiere delle regioni.\n";
            return false;
        }
    }

    // ----- Font Awesome solid -----

    {
        ImFontConfig configFont;

        snprintf(configFont.Name, std::size(configFont.Name), Fonts::AwesomeSolid::Name);
        configFont.Flags     = ImFontFlags_NoLoadError;
        configFont.MergeMode = true;

        ImFont *font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
            Fonts::AwesomeSolid::CompressedData, std::size(Fonts::AwesomeSolid::CompressedData), 0.0f, &configFont);
        if (font == nullptr)
        {
            std::cout << "Impossible caricare il font con le icone.\n";
            return false;
        }
    }

    return true;
}
