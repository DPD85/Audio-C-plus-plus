#include "IntestazionePrecompilata.h"

#include "Clock.h"
#include "CodaEliminazione.h"
#include "CostantiEdAltro.h"
#include "Filtri.h"
#include "Inviluppo.h"
#include "Normalizzatore.h"
#include "Oscillatori.h"
#include "StrumentiMusicali.h"
#include "Volume.h"
#include "WAVE.h"

static int AudioCallback(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData);
#ifdef WIN32
static void TastieraEIUC();
#endif
static void RegistraPerGrafico();

static StrumentiMusicali::Puro strumentoPuro;
static StrumentiMusicali::Pianoforte pianoforte;
static std::atomic<StrumentoMusicale *> strumentoMusicale = &pianoforte;

static Filtri::EchoWetDry èco(0.020, 0.9, 0.6);
// TODO: Non necessario su Windows, su Linux?
//static Filtri::Limitatore limitatore(1.0, 0.95, 0.00042);

static PaStream *flusso;
static const PaStreamInfo *infoFlusso;

static bool esci = false;
static std::atomic<double> durataProduzioneSuono; // [ms]

int main()
{
    CodaEliminazione eliminatori;

    // ----- Attiva il supporto al UTF8 sulla console

    std::setlocale(LC_ALL, ".utf8");

    // -----

    std::cout << "Formato dati audio\n";
    // clang-format off
    std::cout << "  numero canali    : " << Costanti::NumeroCanali << '\n'
              << "  frequenza        : " << Costanti::FrequenzaCampionamento << " Hz\n"
              << "  bits per campione: " << 32 << '\n'
              << "  tipo campione    : Float\n";
    // clang-format on

#if 0
    RegistraPerGrafico();
    return EXIT_SUCCESS;
#endif

    // ----- Inizializza la libreria PortAudio

    {
        PaError r = Pa_Initialize();
        if (r != paNoError)
        {
            std::cout << "Inizializzazione PortAudio fallita. Errore: " << Pa_GetErrorText(r) << '\n';
            return EXIT_FAILURE;
        }

        eliminatori.Aggiungi(
            []()
            {
                Pa_Terminate();
            });
    }

    // ----- Identifico la API sottostante usata da PortAudio e ne recupero le informazioni

    PaHostApiIndex indiceHostAPI;
    const PaHostApiInfo *infoHostAPI;

    {
        indiceHostAPI = Pa_GetDefaultHostApi();
        if (indiceHostAPI < 0)
        {
            std::cout << "Impossibile recuperare la API sottostante di default usate da PortAudio. Errore: "
                      << Pa_GetErrorText(indiceHostAPI) << '\n';
            return EXIT_FAILURE;
        }

        infoHostAPI = Pa_GetHostApiInfo(indiceHostAPI);
        if (infoHostAPI == nullptr)
        {
            std::cout
                << "Impossibile recuperare le informazioni sulla API sottostante di default usata da PortAudio.\n";
            return EXIT_FAILURE;
        }

        std::cout << "API sottostante usata da PortAudio: " << infoHostAPI->name << '\n';

#ifdef WIN32
        // Su Windows, PortAudio dovrebbe sempre usare le WASAPI per via di come è stata configurata la libreria
        if (indiceHostAPI != Pa_HostApiTypeIdToHostApiIndex(PaHostApiTypeId::paWASAPI))
        {
            std::cout << "Avviso: PortAudio non utilizza la WASAPI su Windows come atteso.";
        }
#else
        // Riguardo a Linux sono ignorante quindi lascio che decida PortAudio
#endif
    }

    // ----- Stampa il nome del dispositivo di riproduzione predefinito

    const PaDeviceInfo *infoDispositivo;

    {
        infoDispositivo = Pa_GetDeviceInfo(infoHostAPI->defaultOutputDevice);
        if (infoDispositivo == nullptr)
        {
            std::cout << "Impossibile recuperare le informazioni del dispositivo di riproduzione predefinito "
                         "(tramite WASAPI).\n";
            return EXIT_FAILURE;
        }

        std::cout << "Dispositivo di riproduzione: " << infoDispositivo->name << '\n';
    }

    // ----- Inizializzo il flusso per la riproduzione dell'audio

    {
        PaStreamParameters formatoAudio{ .device                    = infoHostAPI->defaultOutputDevice,
                                         .channelCount              = Costanti::NumeroCanali,
                                         .sampleFormat              = paFloat32,
                                         .suggestedLatency          = 0.020, // [sec.]
                                         .hostApiSpecificStreamInfo = nullptr };

        PaError r = Pa_OpenStream(
            &flusso,
            nullptr,
            &formatoAudio,
            Costanti::FrequenzaCampionamento,
            0,
            paPrimeOutputBuffersUsingStreamCallback | paClipOff | paDitherOff,
            AudioCallback,
            nullptr);
        if (r != paNoError)
        {
            std::cout << "Inizializzazione del flusso per la riproduzione dell'audio fallita. Errore: "
                      << Pa_GetErrorText(r) << '\n';
            return EXIT_FAILURE;
        }

        eliminatori.Aggiungi(
            []()
            {
                Pa_CloseStream(flusso);
            });
    }

    // ----- Recupero informazioni sul flusso appena inizializzato

    {
        infoFlusso = Pa_GetStreamInfo(flusso);
        if (infoFlusso == nullptr)
        {
            std::cout << "Impossibile recuperare le informazioni del flusso di riproduzione dell'audio.\n";
            return EXIT_FAILURE;
        }

        std::cout << "Latenza ~" << (infoFlusso->outputLatency * 1000.0) << " ms.\n";
    }

    // ----- Avvio flusso di riproduzione dell'audio

    {
        PaError r = Pa_StartStream(flusso);
        if (r != paNoError)
        {
            std::cout << "Avvio del flusso di riproduzione dell'audio fallita. Errore: " << Pa_GetErrorText(r) << '\n';
            return EXIT_FAILURE;
        }

        eliminatori.Aggiungi(
            []()
            {
                Pa_StopStream(flusso);
            });
    }

    // ----- Input tastiera e interfaccia grafica in AsciiArt

#ifdef WIN32
    TastieraEIUC();
#endif

    // ----- -----

    return EXIT_SUCCESS;
}

static int AudioCallback(
    const void * /*input*/,
    void *output,
    unsigned long frameCount, // numero di campioni da generare
    const PaStreamCallbackTimeInfo * /*timeInfo*/,
    PaStreamCallbackFlags /*statusFlags*/,
    void * /*userData*/)
{
    std::chrono::time_point tInizio = std::chrono::high_resolution_clock::now();

    // ----- -----

    StrumentoMusicale *strumento = strumentoMusicale.load();
    for (size_t i = 0; i < frameCount; ++i)
    {
        // Produco un campione audio dallo strumento musicale ed aggiungo qualche effetto.
        const double campione = /*èco.Computa*/ (strumento->Campione());

        // C'è bisogno di limitare il volume poiché ci penserà il mixer di Windows a farlo.
        // TODO: su Linux?

        float *dati = reinterpret_cast<float *>(output);

        dati[i * Costanti::NumeroCanali + Costanti::CanaleSinistro] = static_cast<float>(campione);
        dati[i * Costanti::NumeroCanali + Costanti::CanaleDestro]   = static_cast<float>(campione);
    }

    // ----- -----

    std::chrono::time_point tFine = std::chrono::high_resolution_clock::now();
    durataProduzioneSuono.store(std::chrono::duration_cast<DurataMillisecondi>(tFine - tInizio).count());

    return paContinue;
}

#ifdef WIN32
static void TastieraEIUC()
{
    std::cout << '\n';

    // ----- -----

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // ----- Calcolo la posizione lista strumenti musicali nella console -----

    COORD posizioneStrumentiMusicali;

    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);

        posizioneStrumentiMusicali   = csbi.dwCursorPosition;
        posizioneStrumentiMusicali.X = 0;
    }

    // ----- Calcolo la posizione della tastiera musicale nella console -----

    COORD posizioneTastiera;

    {
        posizioneTastiera    = posizioneStrumentiMusicali;
        posizioneTastiera.X  = 0;
        posizioneTastiera.Y += 3;
    }

    // ----- Calcolo la posizione dei tempi misurati nella console -----

    COORD posizioneTempoMisurato;

    {
        posizioneTempoMisurato    = posizioneTastiera;
        posizioneTempoMisurato.X  = 0;
        posizioneTempoMisurato.Y += 10;
    }

    // ----- Nascondo il cursore della console -----

    {
        CONSOLE_CURSOR_INFO infoCursore = { .dwSize = 25, .bVisible = false };
        SetConsoleCursorInfo(hConsole, &infoCursore);
    }

    // ----- Inizializzazione ed avvio del clock -----

    Clock clock(0.017);

    clock.Avvia();

    // ----- -----

    std::array<bool, Note::NumeroNote> tasti{};
    std::array<bool, Note::NumeroNote> tastiPrec{ true };

    bool tastoQ = false, precedenteTastoQ = false;
    bool tastoP = false, precedenteTastoP = false;

    bool stampaListaStrumenti = true;

    do
    {
        double dt = clock.AspettaTicchettioSuccessivo();

        // ----- -----

        // Usare questa funzione per leggere lo stato di 256 tasti in una sola volta
        // GetKeyboardState();

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) esci = true;

        // Note
        tasti[DO]  = (GetAsyncKeyState('A') & 0x8000) != 0;
        tasti[RE]  = (GetAsyncKeyState('S') & 0x8000) != 0;
        tasti[MI]  = (GetAsyncKeyState('D') & 0x8000) != 0;
        tasti[FA]  = (GetAsyncKeyState('F') & 0x8000) != 0;
        tasti[SOL] = (GetAsyncKeyState('J') & 0x8000) != 0;
        tasti[LA]  = (GetAsyncKeyState('K') & 0x8000) != 0;
        tasti[SI]  = (GetAsyncKeyState('L') & 0x8000) != 0;

        // Semitoni
        tasti[DO_DIESIS]  = (GetAsyncKeyState('W') & 0x8000) != 0;
        tasti[RE_DIESIS]  = (GetAsyncKeyState('E') & 0x8000) != 0;
        tasti[FA_DIESIS]  = (GetAsyncKeyState('U') & 0x8000) != 0;
        tasti[SOL_DIESIS] = (GetAsyncKeyState('I') & 0x8000) != 0;
        tasti[LA_DIESIS]  = (GetAsyncKeyState('O') & 0x8000) != 0;

        // Strumenti musicali
        tastoQ = (GetAsyncKeyState('Q') & 0x8000) != 0;
        tastoP = (GetAsyncKeyState('P') & 0x8000) != 0;

        // ----- -----

        if (tastoQ != precedenteTastoQ)
        {
            precedenteTastoQ = tastoQ;
            strumentoMusicale.store(&pianoforte);
            stampaListaStrumenti = true;
        }
        if (tastoP != precedenteTastoP)
        {
            precedenteTastoP = tastoP;
            strumentoMusicale.store(&strumentoPuro);
            stampaListaStrumenti = true;
        }

        // ----- -----

        {
            StrumentoMusicale *strumento = strumentoMusicale.load();

            for (size_t i = 0; i < tasti.size(); ++i)
                if (tasti[i]) strumento->InizioNota(static_cast<Note>(i));
                else strumento->FineNota(static_cast<Note>(i));
        }

        // ----- Stampo lista strumenti musicali -----

        if (stampaListaStrumenti)
        {
            SetConsoleCursorPosition(hConsole, posizioneStrumentiMusicali);

            StrumentoMusicale *s = strumentoMusicale.load();

            if (s == &pianoforte) std::cout << "[*] ";
            else std::cout << "[ ] ";
            std::cout << "Tasto Q = suona col primo strumento musicale\n";

            if (s == &strumentoPuro) std::cout << "[*] ";
            else std::cout << "[ ] ";
            std::cout << "Tasto P = suona col secondo strumento musicale\n";

            stampaListaStrumenti = false;
        }

        // ----- Stampo il tempo misurato -----

        {
            SetConsoleCursorPosition(hConsole, posizioneTempoMisurato);

            const double periodoAudio = Pa_GetStreamCpuLoad(flusso) * infoFlusso->outputLatency * 1000.0; // [ms]

            std::cout << std::fixed << std::setprecision(4);
            std::cout << std::setw(7) << periodoAudio << " ms. periodo audio\n";
            std::cout << std::setw(7) << durataProduzioneSuono.load() << " ms. durata produzione audio\n";
            std::cout << std::setw(7) << (dt * 1000.0) << " ms. periodo tastiera";
            std::cout << std::defaultfloat;
        }

        // ----- Stampo la tastiera musicale -----

        if (tasti != tastiPrec)
        {
            SetConsoleCursorPosition(hConsole, posizioneTastiera);

            const constexpr int lunghezzaRiga = 44;

            // clang-format off
            char tastieraSopra[] =
                "│   │███│ │███│   │   │███│ │███│ │███│   │\n"
                "│   │███│ │███│   │   │███│ │███│ │███│   │\n"
                "│   │███│ │███│   │   │███│ │███│ │███│   │\n"
                "│   │███│ │███│   │   │███│ │███│ │███│   │\n";
            char tastieraSotto[] =
                "│   └─┬─┘ └─┬─┘   │   └─┬─┘ └─┬─┘ └─┬─┘   │\n"
                "│     │     │     │     │     │     │     │\n"
                "│  A  │  S  │  D  │  F  │  J  │  K  │  L  │\n"
                "└─────┴─────┴─────┴─────┴─────┴─────┴─────┘\n"
                "\n";
            // clang-format on

            // Le variabili seguenti corrispondono alla riga della tastiera con le lettere associate ai semitoni, la
            // riga è stata spezzata in modo da poter cambiare agevolmente il colore delle lettere. Le variabili sono
            // numerate in ordine secondo il disegno da sinistra verso destra.
            char tastieraParte01[]      = "│   │█";
            char tastieraParte03[]      = "█│ │█";
            char tastieraParte05[]      = "█│   ";
            char tastieraParte06[]      = "│   │█";
            char tastieraParte08[]      = "█│ │█";
            char tastieraParte10[]      = "█│ │█";
            char tastieraParte12[]      = "█│   │\n";
            const char *tastieraParte02 = "\033[30;47mW\033[0m";
            const char *tastieraParte04 = "\033[30;47mE\033[0m";
            const char *tastieraParte07 = "\033[30;47mU\033[0m";
            const char *tastieraParte09 = "\033[30;47mI\033[0m";
            const char *tastieraParte11 = "\033[30;47mO\033[0m";

            auto disegnaTastoA = [&tastieraSopra, &tastieraSotto](const int p, char *const tastieraParte)
            {
                tastieraSopra[p + 0 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 1 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 2 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 0 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 1 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 2 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 0 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 1 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 2 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 0 + lunghezzaRiga * 3] = '▒';
                tastieraSopra[p + 1 + lunghezzaRiga * 3] = '▒';
                tastieraSopra[p + 2 + lunghezzaRiga * 3] = '▒';

                tastieraParte[1] = '▓';
                tastieraParte[2] = '▓';
                tastieraParte[3] = '▓';

                tastieraSotto[p + 0 + lunghezzaRiga * 0] = '▓';
                tastieraSotto[p + 1 + lunghezzaRiga * 0] = '▓';
                tastieraSotto[p + 2 + lunghezzaRiga * 0] = '▓';
                tastieraSotto[p + 0 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 1 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 2 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 3 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 4 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 0 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 1 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 2 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 3 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 4 + lunghezzaRiga * 2] = '█';
            };
            auto disegnaTastoB = [&tastieraSopra, &tastieraSotto](const int p, char *const tastieraParte)
            {
                tastieraSopra[p + 0 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 0 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 0 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 0 + lunghezzaRiga * 3] = '▒';

                tastieraParte[2] = '▓';

                tastieraSotto[p + 0 + lunghezzaRiga * 0] = '▓';
                tastieraSotto[p - 2 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p - 1 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 0 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 1 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 2 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p - 2 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p - 1 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 0 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 1 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 2 + lunghezzaRiga * 2] = '█';
            };
            auto disegnaTastoC = [&tastieraSopra, &tastieraSotto](const int p, char *const tastieraParte)
            {
                tastieraSopra[p + 0 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 1 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 2 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 0 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 1 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 2 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 0 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 1 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 2 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 0 + lunghezzaRiga * 3] = '▒';
                tastieraSopra[p + 1 + lunghezzaRiga * 3] = '▒';
                tastieraSopra[p + 2 + lunghezzaRiga * 3] = '▒';

                tastieraParte[2] = '▓';
                tastieraParte[3] = '▓';
                tastieraParte[4] = '▓';

                tastieraSotto[p + 0 + lunghezzaRiga * 0] = '▓';
                tastieraSotto[p + 1 + lunghezzaRiga * 0] = '▓';
                tastieraSotto[p + 2 + lunghezzaRiga * 0] = '▓';

                tastieraSotto[p - 2 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p - 1 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 0 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 1 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p + 2 + lunghezzaRiga * 1] = '█';
                tastieraSotto[p - 2 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p - 1 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 0 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 1 + lunghezzaRiga * 2] = '█';
                tastieraSotto[p + 2 + lunghezzaRiga * 2] = '█';
            };
            auto disegnaTastoS = [&tastieraSopra](const int p, const char *&tastieraParte)
            {
                tastieraSopra[p + 0 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 1 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 2 + lunghezzaRiga * 0] = '░';
                tastieraSopra[p + 0 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 1 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 2 + lunghezzaRiga * 1] = '▒';
                tastieraSopra[p + 0 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 1 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 2 + lunghezzaRiga * 2] = '▒';
                tastieraSopra[p + 0 + lunghezzaRiga * 3] = '▓';
                tastieraSopra[p + 1 + lunghezzaRiga * 3] = '▓';
                tastieraSopra[p + 2 + lunghezzaRiga * 3] = '▓';

                tastieraParte = "█";
            };

            // Note
            if (tasti[DO]) disegnaTastoA(1, tastieraParte01);
            if (tasti[RE]) disegnaTastoB(9, tastieraParte03);
            if (tasti[MI]) disegnaTastoC(15, tastieraParte05);
            if (tasti[FA]) disegnaTastoA(19, tastieraParte06);
            if (tasti[SOL]) disegnaTastoB(27, tastieraParte08);
            if (tasti[LA]) disegnaTastoB(33, tastieraParte10);
            if (tasti[SI]) disegnaTastoC(39, tastieraParte12);

            // Semitoni
            if (tasti[DO_DIESIS]) disegnaTastoS(5, tastieraParte02);
            if (tasti[RE_DIESIS]) disegnaTastoS(11, tastieraParte04);
            if (tasti[FA_DIESIS]) disegnaTastoS(23, tastieraParte07);
            if (tasti[SOL_DIESIS]) disegnaTastoS(29, tastieraParte09);
            if (tasti[LA_DIESIS]) disegnaTastoS(35, tastieraParte11);

            std::cout << tastieraSopra << tastieraParte01 << tastieraParte02 << tastieraParte03 << tastieraParte04
                      << tastieraParte05 << tastieraParte06 << tastieraParte07 << tastieraParte08 << tastieraParte09
                      << tastieraParte10 << tastieraParte11 << tastieraParte12 << tastieraSotto;
        }

        tastiPrec = tasti;
    }
    while (!esci);
}
#endif

static void RegistraPerGrafico()
{
    CodaEliminazione eliminatori;

#define ApriFile(N)                                                                   \
    std::string filePath##N = "registrazione" #N ".dat";                              \
    std::ofstream file##N(filePath##N, std::ios_base::binary | std::ios_base::trunc); \
    if (!file##N.good())                                                              \
    {                                                                                 \
        std::cout << "Impossible aprire il file '" << filePath##N << '\n';            \
        return;                                                                       \
    }                                                                                 \
    eliminatori.Aggiungi(                                                             \
        [&file##N]                                                                    \
        {                                                                             \
            file##N.close();                                                          \
        })

    ApriFile(1);
    ApriFile(2);
    ApriFile(3);
    ApriFile(4);
    ApriFile(5);
    ApriFile(6);
    ApriFile(7);
    ApriFile(8);
    ApriFile(9);
    ApriFile(10);
    ApriFile(11);
    ApriFile(12);
    ApriFile(13);
    ApriFile(14);
    ApriFile(15);
    ApriFile(16);
    ApriFile(17);

#undef ApriFile

    // ----- -----

    std::array<Oscillatori::OndaSinusoidale, Note::NumeroNote> note = {
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaDo),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaDoDiesis),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaRe),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaReDiesis),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaMi),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaFa),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaFaDiesis),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaSol),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaSolDiesis),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaLa),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaLaDiesis),
        Oscillatori::OndaSinusoidale(Costanti::FrequenzaSi),
    };

    std::array<Volume, note.size()> volumi = {
        Volume(0.01), // do
        Volume(0.01), // do#
        Volume(0.01), // re
        Volume(0.01), // re#
        Volume(0.01), // mi
        Volume(0.01), // fa
        Volume(0.01), // fa#
        Volume(0.01), // sol
        Volume(0.01), // sol#
        Volume(0.01), // la
        Volume(0.01), // la#
        Volume(0.01), // si
    };

    Normalizzatore normalizzatore;

    // ----- -----

    unsigned char numeroSerie   = 1;
    const size_t numeroCampioni = DaSecondiACampioni(0.15);
    dvector buffer;
    buffer.resize(numeroCampioni);

    // ----- -----

    {
        note[SOL].Reset();

        for (size_t i = 0; i < numeroCampioni; ++i)
        {
            buffer[i] = note[SOL].Campione();
        }

        file1.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file1.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    //----- -----

    {
        volumi[SOL].Reset();
        volumi[SOL].Valore(1.0);

        size_t i;
        for (i = 0; i < numeroCampioni / 2; ++i)
        {
            buffer[i] = volumi[SOL].Smussa();
        }

        volumi[SOL].Valore(0.5);

        for (; i < numeroCampioni; ++i)
        {
            buffer[i] = volumi[SOL].Smussa();
        }

        file2.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file2.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    //----- -----

    {
        note[SOL].Reset();
        volumi[SOL].Reset();
        volumi[SOL].Valore(1.0);

        for (size_t i = 0; i < numeroCampioni; ++i)
        {
            buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione();
        }

        file3.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file3.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- -----

    {
        note[SOL].Reset();

        for (size_t i = 0; i < numeroCampioni; ++i)
        {
            buffer[i] = ConvertiA16Bits(note[SOL].Campione());
        }

        file4.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file4.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    //----- -----

    {
        volumi[SOL].Reset();
        volumi[SOL].Valore(1.0);

        size_t i;
        for (i = 0; i < numeroCampioni; ++i)
        {
            buffer[i] = ConvertiA16Bits(volumi[SOL].Smussa());
        }

        file5.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file5.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    //----- -----

    {
        volumi[SOL].Reset();
        volumi[SOL].Valore(1.0);

        size_t i;
        for (i = 0; i < numeroCampioni / 2; ++i)
        {
            buffer[i] = ConvertiA16Bits(volumi[SOL].Smussa());
        }

        volumi[SOL].Valore(0.0);

        for (; i < numeroCampioni; ++i)
        {
            buffer[i] = ConvertiA16Bits(volumi[SOL].Smussa());
        }

        file6.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file6.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    //----- -----

    {
        note[SOL].Reset();
        volumi[SOL].Reset();
        volumi[SOL].Valore(1.0);

        for (size_t i = 0; i < numeroCampioni; ++i)
        {
            buffer[i] = ConvertiA16Bits(volumi[SOL].Smussa() * note[SOL].Campione());
        }

        file7.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file7.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- -----

    {
        note[SOL].Reset();
        note[LA].Reset();

        for (size_t i = 0; i < numeroCampioni; ++i)
        {
            buffer[i] = note[SOL].Campione() + note[LA].Campione();
        }

        file8.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file8.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- -----

    {
        note[SOL].Reset();
        volumi[SOL].Reset();
        note[LA].Reset();
        volumi[LA].Reset();

        volumi[SOL].Valore(1.0);

        size_t i;
        for (i = 0; i < numeroCampioni / 4; ++i)
        {
            buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
        }

        volumi[LA].Valore(1.0);

        for (; i < numeroCampioni; ++i)
        {
            buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
        }

        file9.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file9.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- -----

    {
        note[SOL].Reset();
        note[LA].Reset();

        for (size_t i = 0; i < numeroCampioni; ++i)
        {
            buffer[i] = note[SOL].Campione() + note[LA].Campione();
        }

        normalizzatore.Normalizza(buffer);

        file10.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file10.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- -----

    {
        note[SOL].Reset();
        volumi[SOL].Reset();
        note[LA].Reset();
        volumi[LA].Reset();

        volumi[SOL].Valore(1.0);

        size_t i;
        for (i = 0; i < numeroCampioni / 4; ++i)
        {
            buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
        }

        volumi[LA].Valore(1.0);

        for (; i < numeroCampioni; ++i)
        {
            buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
        }

        normalizzatore.Normalizza(buffer);

        file11.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file11.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- -----

    {
        note[SOL].Reset();
        volumi[SOL].Reset();
        note[LA].Reset();
        volumi[LA].Reset();

        volumi[SOL].Valore(1.0);

        size_t i;
        for (i = 0; i < numeroCampioni / 4; ++i)
        {
            buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
        }

        volumi[LA].Valore(1.0);

        for (; i < numeroCampioni; ++i)
        {
            buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
        }

        normalizzatore.Normalizza(buffer, buffer.size() / 2, 0);
        normalizzatore.Normalizza(buffer, buffer.size() / 2, buffer.size() / 2);

        file12.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file12.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- Test inviluppo completo -----

    {
        std::ofstream &file = file13;

        InviluppoADSR inviluppo(0.02, 0.01, 0.8, 0.03);
        const size_t tempoFineNota = DaSecondiACampioni(0.1);

        size_t i;
        for (i = 0; i < numeroCampioni; ++i)
        {
            if (i == 300) inviluppo.InizioNota();
            if (i == tempoFineNota) inviluppo.FineNota();

            buffer[i] = inviluppo.Computa();
        }

        file.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- Test inviluppo con prematura terminazione della nota (salta la fase di sostentamento) -----

    {
        std::ofstream &file = file14;

        InviluppoADSR inviluppo(0.02, 0.01, 0.8, 0.03);

        size_t i;
        for (i = 0; i < numeroCampioni; ++i)
        {
            if (i == 300) inviluppo.InizioNota();
            if (i == 301) inviluppo.FineNota();

            buffer[i] = inviluppo.Computa();
        }

        file.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- Test inviluppo con inizio nota durante la fase di rilascio -----

    {
        std::ofstream &file = file15;

        InviluppoADSR inviluppo(0.02, 0.01, 0.8, 0.03);
        const size_t tempoFineNota1   = DaSecondiACampioni(0.05);
        const size_t tempoInizioNota2 = DaSecondiACampioni(0.06);
        const size_t tempoFineNota2   = DaSecondiACampioni(0.1);

        size_t i;
        for (i = 0; i < numeroCampioni; ++i)
        {
            if (i == 300) inviluppo.InizioNota();
            if (i == tempoFineNota1) inviluppo.FineNota();
            if (i == tempoInizioNota2) inviluppo.InizioNota();
            if (i == tempoFineNota2) inviluppo.FineNota();

            buffer[i] = inviluppo.Computa();
        }

        file.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
        file.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- -----

    {
        std::ofstream &file = file16;

        note[SOL].Reset();
        volumi[SOL].Reset();
        note[LA].Reset();
        volumi[LA].Reset();

        buffer.resize(numeroCampioni * 2);

        {
            volumi[SOL].Valore(1.0);

            size_t i;
            for (i = 0; i < numeroCampioni / 4; ++i)
            {
                buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
            }

            volumi[LA].Valore(1.);

            for (; i < numeroCampioni; ++i)
            {
                buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
            }

            size_t n = numeroCampioni / 4;
            volumi[LA].Valore(0.0);

            for (i = numeroCampioni; i < numeroCampioni + n; ++i)
            {
                buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
            }

            volumi[LA].Valore(0.0);

            for (; i < numeroCampioni + n * 2; ++i)
            {
                buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
            }

            volumi[LA].Valore(1.);

            for (; i < numeroCampioni + n * 4; ++i)
            {
                buffer[i] = volumi[SOL].Smussa() * note[SOL].Campione() + volumi[LA].Smussa() * note[LA].Campione();
            }

            numeroSerie = 4;
            file.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));
            file.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
        }

        dvector buffer2(numeroCampioni * 2);
        const double attacco  = 0.002;
        const double rilascio = 1.0;

        {
            Filtri::RilevatoreInviluppo rilevatore(attacco, rilascio);
            for (size_t i = 0; i < numeroCampioni * 2; ++i)
            {
                double inviluppo = rilevatore.Computa(buffer[i]);
                buffer2[i]       = inviluppo;
            }

            file.write(reinterpret_cast<char *>(buffer2.data()), buffer2.size() * sizeof(dvector::value_type));
        }

        {
            Filtri::InseguitorePicchi rilevatore(rilascio);
            for (size_t i = 0; i < numeroCampioni * 2; ++i)
            {
                double inviluppo = rilevatore.Computa(buffer[i]);
                buffer2[i]       = inviluppo;
            }

            file.write(reinterpret_cast<char *>(buffer2.data()), buffer2.size() * sizeof(dvector::value_type));
        }

        {
            Filtri::Limitatore limitatore(rilascio, 0.95, 0.00042);
            for (size_t i = 0; i < numeroCampioni * 2; ++i)
            {
                double inviluppo = limitatore.Computa(buffer[i]);
                buffer2[i]       = inviluppo;

                if (std::abs(inviluppo) > 1.0)
                {
                    std::cout << "Limitatore: eccede [-1, 1], delta " << (std::abs(inviluppo) - 1.0) << ".\n";
                }
            }

            file.write(reinterpret_cast<char *>(buffer2.data()), buffer2.size() * sizeof(dvector::value_type));
        }

        buffer.resize(numeroCampioni);
    }

    // ----- Filtro passa basso/alto -----

    {
        std::ofstream &file = file17;

        note[SOL].Reset();
        note[LA].Reset();

        numeroSerie = 2;
        file.write(reinterpret_cast<const char *>(&numeroSerie), sizeof(numeroSerie));

        for (double &campione : buffer)
        {
            campione = note[SOL].Campione(); // + note[LA].Campione();
            // campione /= 2.0;
        }

        file.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));

        Filtri::FiltroPassaAlto filtro(10000.0);

        for (double &campione : buffer)
        {
            campione = filtro.Computa(campione);
        }

        file.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(dvector::value_type));
    }

    // ----- -----

    note[DO].Reset();
    note[SOL].Reset();
    volumi[SOL].Reset();
    note[LA].Reset();
    volumi[LA].Reset();
}
