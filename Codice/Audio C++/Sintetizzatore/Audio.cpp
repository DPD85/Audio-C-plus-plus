#include "../IntestazionePrecompilata.h"

#include "Audio.h"

#include "../CodaCancellazione.h"
#include "CostantiEdAltro.h"
#include "Filtri.h"
#include "Inviluppo.h"
#include "Oscillatori.h"
#include "Volume.h"

namespace Sintetizzatore
{
    static int ProceduraProduzioneAudio(
        const void *input,
        void *output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData);
    static void LogMessaggiALSA(const char *file, int line, const char *function, int errorcode, const char *fmt, ...);
    static void RegistraPerGrafico();

    static StrumentiMusicali::Puro strumentoPuro;
    static StrumentiMusicali::Pianoforte pianoforte;

    std::atomic<StrumentoMusicale *> strumentoMusicale = &pianoforte;

    static Filtri::EchoWetDry èco(0.020, 0.9, 0.6);

    static PaStream *flusso;
    static const PaStreamInfo *infoFlusso;

    static std::atomic<double> durataProduzioneSuono; // [ms]
}

bool Sintetizzatore::InizializzaAudio()
{
    CodaCancellazione eliminatori;

    // -----

    std::cout << "----- Informazioni sull'audio -----\n\n";

    std::cout << "Formato dati audio\n";
    // clang-format off
    std::cout << "  numero canali    : " << Costanti::NumeroCanali << '\n'
                << "  frequenza        : " << Costanti::FrequenzaCampionamento << " Hz\n"
                << "  bits per campione: " << 32 << '\n'
                << "  tipo campione    : Float\n";
    // clang-format on
    std::cout << '\n';

#if 0
    RegistraPerGrafico();
    return EXIT_SUCCESS;
#endif

    // ----- Intercettazione dei messaggi di errore di ALSA -----

#ifndef WIN32
    snd_lib_error_set_handler(LogMessaggiALSA);
    eliminatori.Aggiungi(
        []()
        {
            snd_lib_error_set_handler(nullptr);
        });
#endif

    // ----- Inizializza la libreria PortAudio

#ifndef WIN32

    // int stderrBackup = dup(STDERR_FILENO);
    // int devNullFD    = open("/dev/null", O_WRONLY);
    // dup2(devNullFD, STDERR_FILENO);
    // close(devNullFD);

#endif

    {
        PaError r = Pa_Initialize();
        if (r != paNoError)
        {
            std::cout << "Inizializzazione PortAudio fallita. Errore: " << Pa_GetErrorText(r) << '\n';
            return false;
        }

        eliminatori.Aggiungi(
            []()
            {
                Pa_Terminate();
            });
    }

#ifndef WIN32
    // dup2(stderrBackup, STDERR_FILENO);
    // close(stderrBackup);
#endif

    // ----- Identifico la API sottostante usata da PortAudio e ne recupero le informazioni

    const PaHostApiInfo *infoHostAPI;

    {
        const PaHostApiIndex indiceHostAPI = Pa_GetDefaultHostApi();
        if (indiceHostAPI < 0)
        {
            std::cout << "Impossibile recuperare la API sottostante di default usate da PortAudio. Errore: "
                      << Pa_GetErrorText(indiceHostAPI) << '\n';
            return false;
        }

        infoHostAPI = Pa_GetHostApiInfo(indiceHostAPI);
        if (infoHostAPI == nullptr)
        {
            std::cout
                << "Impossibile recuperare le informazioni sulla API sottostante di default usata da PortAudio.\n";
            return false;
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

    {
        const PaDeviceInfo *infoDispositivo = Pa_GetDeviceInfo(infoHostAPI->defaultOutputDevice);
        if (infoDispositivo == nullptr)
        {
            std::cout << "Impossibile recuperare le informazioni del dispositivo di riproduzione predefinito.\n";
            return false;
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
            ProceduraProduzioneAudio,
            nullptr);
        if (r != paNoError)
        {
            std::cout << "Inizializzazione del flusso per la riproduzione dell'audio fallita. Errore: "
                      << Pa_GetErrorText(r) << '\n';
            return false;
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
            return false;
        }

        std::cout << "Latenza ~" << (infoFlusso->outputLatency * 1000.0) << " ms.\n";

        std::cout << '\n';
    }

    // ----- Avvio flusso di riproduzione dell'audio

    {
        PaError r = Pa_StartStream(flusso);
        if (r != paNoError)
        {
            std::cout << "Avvio del flusso di riproduzione dell'audio fallita. Errore: " << Pa_GetErrorText(r) << '\n';
            return false;
        }

        eliminatori.Aggiungi(
            []()
            {
                Pa_StopStream(flusso);
            });
    }

    // ----- -----

    eliminatori.Svuota();

    // ----- -----

    return true;
}

void Sintetizzatore::TerminaAudio()
{
    Pa_StopStream(flusso);
    Pa_CloseStream(flusso);
    Pa_Terminate();
}

// ----- -----

static int Sintetizzatore::ProceduraProduzioneAudio(
    const void * /*input*/,
    void *output,
    const unsigned long numeroCampioni, // numero di campioni da generare
    const PaStreamCallbackTimeInfo * /*timeInfo*/,
    PaStreamCallbackFlags /*statusFlags*/,
    void * /*userData*/)
{
    std::chrono::time_point tInizio = std::chrono::high_resolution_clock::now();

    // ----- -----

    StrumentoMusicale *strumento = strumentoMusicale.load();
    for (size_t i = 0; i < numeroCampioni; ++i)
    {
        // Produco un campione audio dallo strumento musicale ed aggiungo qualche effetto.
        const double campione = /*èco.Computa*/ (strumento->Campione());

        // Non c'è bisogno di limitare il volume poiché ci penserà il mixer di Windows a farlo.
        // TODO: su Linux?

        auto dati = static_cast<float *>(output);

        dati[i * Costanti::NumeroCanali + Costanti::CanaleSinistro] = static_cast<float>(campione);
        dati[i * Costanti::NumeroCanali + Costanti::CanaleDestro]   = static_cast<float>(campione);
    }

    // ----- -----

    std::chrono::time_point tFine = std::chrono::high_resolution_clock::now();
    durataProduzioneSuono.store(std::chrono::duration_cast<DurataMillisecondi>(tFine - tInizio).count());

    return paContinue;
}

// Nota: questa funzione viene usata solo su Linux.
static void Sintetizzatore::LogMessaggiALSA(
    const char *file, const int line, const char *function, const int errorcode, const char *fmt, ...)
{
    // Stampa solo i messaggi di errore.
    if (errorcode == 0) return;

    // Stampo solo il nome del file senza il percorso.
    const char *nomeFile = strrchr(file, '/');
    if (nomeFile == nullptr) nomeFile = file;
    else nomeFile++;

    std::cout << "ALSA [" << nomeFile << ":" << line << " " << function << "()] ";
    if (errorcode != 0) std::cout << "(err = " << errorcode << "): ";

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    std::cout << '\n';
}

static void Sintetizzatore::RegistraPerGrafico()
{
    CodaCancellazione eliminatori;

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
