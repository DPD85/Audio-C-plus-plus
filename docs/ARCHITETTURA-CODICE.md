# Architettura del Codice - Analisi Tecnica Approfondita

## Struttura Generale del Progetto

```text
Codice/Audio C++/
├── Audio C++.cpp              # Entry point e gestione WASAPI
├── CostantiEdAltro.h          # Configurazioni globali e utility
├── StrumentoMusicale.h        # Interfaccia e implementazioni strumenti
├── Oscillatori.h              # Generatori forme d'onda
├── Inviluppo.h                # Sistema ADSR
├── Filtri.h                   # Effetti audio e processing
├── Volume.h                   # Controllo volume con smoothing
├── Clock.h                    # Timing preciso per real-time
├── WAVE.h/cpp                 # Export formato WAV
├── Normalizzatore.h           # Controllo dinamica
└── IntestazionePrecompilata.h # Headers Windows e librerie
```

## Flusso di Esecuzione: Dal Tasto al Suono

Questa sezione segue il percorso completo che porta dalla pressione di un tasto
all'emissione del suono, analizzando i componenti nell'ordine in cui vengono
utilizzati durante l'esecuzione.

### 1. Avvio del Programma (Audio C++.cpp:main)

```cpp
int main() {
    // Setup locale UTF-8 per console
    std::setlocale(LC_ALL, ".utf8");

    // Inizializzazione WASAPI e setup dispositivi audio
    // Creazione e avvio thread audio e tastiera
    ThreadTastiera(); // Loop principale interfaccia utente
}
```

**Punto di Ingresso**: Il main() inizializza tutto il sistema, configura WASAPI
e avvia i thread paralleli.

### 2. Rilevamento Input Utente (Audio C++.cpp:ThreadTastiera)

```cpp
static void ThreadTastiera() {
    Clock clock(1.0 / 60.0); // 60 FPS per responsività

    while (!esci) {
        // Polling diretto tasti Windows
        tasti[DO]  = (GetAsyncKeyState('A') & 0x8000) != 0;
        tasti[RE]  = (GetAsyncKeyState('S') & 0x8000) != 0;
        // ... altre note

        tastoQ = (GetAsyncKeyState('Q') & 0x8000) != 0; // Cambio strumento
```

**Input Layer**: Il thread tastiera cattura input in real-time e aggiorna lo
stato delle note attive tramite comunicazione atomic.

### 3. Configurazione Audio (CostantiEdAltro.h)

```cpp
const constexpr double FrequenzaCampionamento = 48000; // [Hz]
const constexpr unsigned int NumeroCanali = 2;        // Stereo
const constexpr bool ModalitàCondivisa = true;        // WASAPI shared mode

// Frequenze note in intonazione naturale
const constexpr double FrequenzaDo = /*...*/;
```

**Configuration Layer**: Definisce tutti i parametri fondamentali che
governano la generazione audio.

### 4. Selezione Strumento (StrumentoMusicale.h)

```cpp
static StrumentoX strumento1;                    // Serie armonica ricca
static SenzaSerieArmonica strumento2;            // Toni puri
static std::atomic<StrumentoMusicale*> strumentoMusicale = &strumento1;

// Nel thread tastiera:
if (tastoQ && !tastoQ_precedente)
    strumentoMusicale = &strumento1;
```

**Instrument Selection**: L'utente può switchare tra strumenti diversi,
ognuno con algoritmi di sintesi specifici.

### 5. Attivazione Note (StrumentoMusicale::InizioNota/FineNota)

```cpp
// Quando viene premuto un tasto:
strumentoMusicale.load()->InizioNota(Note::DO);

// Nel StrumentoX:
void InizioNota(Note nota) override {
    // Attiva oscillatori per tutte le armoniche
    for(size_t i = 0; i < LunghezzaSerieArmonica; ++i) {
        note[nota][i].Frequenza(frequenze[nota] * (i + 1)); // Armoniche
    }
    inviluppi[nota].InizioNota(); // Trigger ADSR envelope
}
```

**Note Triggering**: Attiva gli oscillatori e l'inviluppo ADSR per la nota
specifica.

### 6. Generazione Oscillatori (Oscillatori.h)

```cpp
class OndaSinusoidale {
    dcomplex fase{1.0, 0.0};    // Fasore rotante
    dcomplex m;                 // Moltiplicatore per frequenza

    double Campione() noexcept {
        fase *= m;              // Rotazione nel piano complesso
        return fase.imag();     // Componente sinusoidale
    }
```

**Signal Generation**: Ogni oscillatore genera la sua forma d'onda usando
algebra complessa per efficienza.

### 7. Controllo Inviluppo (Inviluppo.h)

```cpp
enum class Stati { Silenzio, Attacco, Decadimento, Sostentamento, Rilascio };

double Computa() noexcept {
    switch(stato) {
        case Stati::Attacco:
            valore = tempo / static_cast<double>(attacco);
            break;
        // ... altre fasi ADSR
    }
    return valore; // Moltiplicatore volume [0.0, 1.0]
}
```

**Envelope Shaping**: Controlla come il volume evolve nel tempo per ogni nota.

### 8. Mixing Strumento (StrumentoMusicale::Campione)

```cpp
virtual double Campione() {
    double valore = 0;

    for (size_t i = 0; i < Note::NumeroNote; ++i) {
        double valoreNota = 0;

        // Somma armoniche per questa nota
        for (size_t j = 0; j < LunghezzaSerieArmonica; ++j) {
            valoreNota += note[i][j].Campione() * ampiezze[j];
        }

        // Applica inviluppo ADSR
        valore += valoreNota * inviluppi[i].Computa();
    }
    return valore;
}
```

**Instrument Mixing**: Combina tutte le note attive con le loro armoniche e
inviluppi.

### 9. Elaborazione Effetti (Filtri.h)

```cpp
// Nel RiempiBufferAudio():
double campione = strumentoMusicale.load()->Campione();

// Applica effetti in catena
campione = echo.Computa(campione);           // Echo/riverbero
campione = limitatore.Computa(campione);     // Limitatore look-ahead
```

**Effects Processing**: Il segnale passa attraverso una catena di effetti
audio professionali.

### 10. Generazione Buffer Audio (Audio C++.cpp:RiempiBufferAudio)

```cpp
static void RiempiBufferAudio(unsigned int numCampioni) {
    unsigned char* bufferDati;
    clientRiproduzione->GetBuffer(numCampioni, &bufferDati);

    if constexpr (Costanti::ModalitàCondivisa) {
        float* dati = reinterpret_cast<float*>(bufferDati);

        for (unsigned int i = 0; i < numCampioni; ++i) {
            double campione = /* pipeline completa sopra */;

            // Duplica su canali stereo
            dati[i * 2 + 0] = static_cast<float>(campione); // Left
            dati[i * 2 + 1] = static_cast<float>(campione); // Right
        }
    }

    clientRiproduzione->ReleaseBuffer(numCampioni, 0);
}
```

**Buffer Generation**: Genera i campioni richiesti da WASAPI e li scrive nel
buffer di output.

### 11. Thread Audio e Timing (Audio C++.cpp:ThreadAudio + Clock.h)

```cpp
static void ThreadAudio() noexcept {
    const double durataBuffer = DaCampioniASecondi(dimensioneBuffer);
    Clock clock(durataBuffer / 2.0); // Clock a doppia frequenza

    while (!esci) {
        clock.AspettaTicchettioSuccessivo(); // Timing preciso
        RiempiBufferAudio(dimensioneBuffer / 2);
    }
}
```

**Real-time Scheduling**: Il thread audio mantiene timing preciso per evitare
dropout audio.

### 12. Output WASAPI → Altoparlanti

Il buffer completato viene inviato a Windows Audio Session API che:

1. **Mixa** con altri programmi (modalità condivisa)
2. **Converte** il formato se necessario
3. **Invia** alla scheda audio
4. **Riproduce** tramite altoparlanti/cuffie

## Tempistica dell'Esecuzione

```text
Tempo: 0ms                16.7ms          33.3ms            50ms     ...
       │                  │               │                 │
UI:    [Scan Keys]      → [Scan Keys]   → [Scan Keys]       (ogni 60 FPS, cioè circa ~16.7ms)
       │
Audio: [Gen Samples]    → [Gen Samples] → [Gen]             (~1ms ogni 21ms)
       │
WASAPI:[Play Buf]       → [Play Buf]    → [Play Buf]        (Richieste asincrone)
```

**Parallelismo**: Thread UI e Audio lavorano in parallelo con sincronizzazione
atomic per comunicazione lock-free.

## Flusso dei Dati Riassuntivo

```text
GetAsyncKeyState() → InizioNota() → Oscillatori → Inviluppi →
Mixing → Effetti → WASAPI Buffer → Scheda Audio → Suono
```

Questo flusso viene eseguito **48.000 volte al secondo** per ogni canale stereo,
richiedendo ottimizzazioni estreme per mantenere le performance real-time.

## Analisi File per File

### 1. CostantiEdAltro.h - Il Cuore della Configurazione

#### Configurazione Audio

```cpp
const constexpr double FrequenzaCampionamento = 48000; // [Hz]
const constexpr unsigned int NumeroCanali = 2;
const constexpr bool ModalitàCondivisa = true;
```

**Scelte Architetturali**:

- **48kHz**: Standard professionale vs 44.1kHz dei CD
- **Stereo**: Layout left/right interleaved
- **Modalità Condivisa**: Compatibilità vs performance esclusiva

#### Sistema di Note e Intonazione

```cpp
// Intonazione naturale basata su rapporti puri
const constexpr double RapportoDo  = 1.0;
const constexpr double RapportoRe  = 9.0 / 8.0;   // 1.125
const constexpr double RapportoMi  = 5.0 / 4.0;   // 1.25
const constexpr double RapportoFa  = 4.0 / 3.0;   // 1.333...
const constexpr double RapportoSol = 3.0 / 2.0;   // 1.5
const constexpr double RapportoLa  = 5.0 / 3.0;   // 1.666...
const constexpr double RapportoSi  = 15.0 / 8.0;  // 1.875

const constexpr double FrequenzaRiferimentoLa = 432; // [Hz]
```

**Teoria Musicale Applicata**:

- **Intonazione naturale**: Rapporti di frequenza basati su frazioni semplici
- **432 Hz**: Tuning alternativo (vs standard 440 Hz)
- **Calcolo a cascata**: Tutte le frequenze derivate dal La di riferimento

#### Utility Functions Matematiche

```cpp
inline constexpr double Clamp(double value, double min, double max)
inline constexpr unsigned char ConvertiA8Bits(double valore)
inline constexpr short ConvertiA16Bits(double valore)
```

**Pattern di Design**:

- **constexpr**: Calcolo compile-time quando possibile
- **Template specialization**: Per funzioni variadic
- **Saturazione matematica**: Prevenzione overflow nei formati audio

### 2. Oscillatori.h - Generatori di Segnale

#### Classe Base Oscillatore

```cpp
class Oscillatore {
public:
    virtual ~Oscillatore() = default;
    virtual double Campione() noexcept = 0;
    virtual void Frequenza(double frequenza) = 0;
    virtual void Reset() = 0;
};
```

**Design Pattern**: Strategy pattern per intercambiabilità forme d'onda

#### OndaSinusoidale - Implementazione Avanzata

```cpp
class OndaSinusoidale: public Oscillatore {
private:
    std::atomic<bool> daAggiornare;
    dcomplex nuovoM;  // Nuovo moltiplicatore
    dcomplex m;       // Moltiplicatore corrente
    dcomplex fase{1.0, 0.0};  // Fasore unitario

    void ImpostaFrequenza(double frequenza) {
        nuovoM = std::exp(dcomplex(0.0, 2 * std::numbers::pi *
            frequenza * (1.0 / Costanti::FrequenzaCampionamento)));
    }
};
```

**Innovazioni Tecniche**:

1. **Algebra Complessa**: Usa rotazione nel piano complesso invece di sin()
   - `fase *= m` equivale a rotazione di angolo θ
   - `fase.imag()` estrae il componente sinusoidale
   - Molto più efficiente di `sin(2πft)`

2. **Thread Safety**: `std::atomic<bool>` per aggiornamenti frequenza
   - Thread audio non si blocca mai
   - Aggiornamenti smooth senza interruzioni

3. **Normalizzazione Numerica**: `fase /= std::abs(fase)`
   - Previene deriva numerica nell'aritmetica float
   - Mantiene |fase| = 1 per precisione a lungo termine

#### OndaQuadra - Implementazione Derivata

```cpp
virtual double Campione() noexcept {
    return std::copysign(1.0, sin.Campione());
}
```

**Approccio Elegante**: Riusa oscillatore sinusoidale + funzione segno

### 3. Inviluppo.h - Sistema ADSR

#### State Machine Implementation

```cpp
enum class Stati {
    Silenzio, Attacco, Decadimento, Sostentamento, Rilascio
};

double Computa() noexcept {
    // Cambio di stato
    switch (stato) {
        case Stati::Silenzio:
            if (notaAttiva.load()) { tempo = 0; stato = Stati::Attacco; }
            break;
        case Stati::Attacco:
            if (tempo >= attacco) stato = Stati::Decadimento;
            break;
        // ...
    }

    // Calcolo valore
    switch (stato) {
        case Stati::Attacco:
            valore = tempo / static_cast<double>(attacco);
            break;
        case Stati::Decadimento:
            valore = (1.0 - (tempo - attacco) /
                static_cast<double>(decadimento)) * (1 - sostentamento) +
                sostentamento;
            break;
        // ...
    }

    ++tempo;
    return valore;
}
```

**Caratteristiche Architetturali**:

1. **State Machine**: Gestione pulita delle transizioni
2. **Lock-free**: `std::atomic<bool> notaAttiva` per comunicazione thread
3. **Temporal Tracking**: Contatore campioni invece di timestamp
4. **Mathematical Precision**: Interpolazione lineare per transizioni smooth

#### Modellazione Realistica

```cpp
// Rilascio più veloce per note acute (simula strumenti reali)
std::array<InviluppoADSR, Note::NumeroNote> inviluppi = {
    InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 0),  // do
    InviluppoADSR(0.02, 0.01, 0.8, 0.5 - 0.0333 * 1),  // do#
    // ... formula: 0.5 - 0.0333 * indiceNota
};
```

### 4. Normalizzatore.h - Sistema di Controllo Dinamica

#### Normalizzatore Basico

```cpp
class Normalizzatore {
public:
    void Normalizza(dvector &dati, size_t numeroCampioni, size_t offset = 0) {
        double massimo = std::numeric_limits<double>::min();

        // Trova il picco massimo nel blocco
        for (size_t i = 0; i < numeroCampioni; ++i)
            massimo = std::max(massimo, std::abs(dati[i + offset]));

        if (massimo <= 1) return;  // Già normalizzato

        // Scala tutto il blocco uniformemente
        scala = 1.0 / massimo;
        for (size_t i = 0; i < numeroCampioni; ++i)
            dati[i + offset] *= scala;
    }
    double scala;  // Fattore di scala applicato
};
```

#### Normalizzatore Smussato (Real-time Friendly)

```cpp
class NormalizzatoreSmussato {
private:
    Volume scala{0.1};  // Controllo volume con smussamento esponenziale

public:
    void Normalizza(dvector &dati, size_t numeroCampioni, size_t offset = 0) {
        double massimo = std::numeric_limits<double>::min();

        for (size_t i = 0; i < numeroCampioni; ++i)
            massimo = std::max(massimo, std::abs(dati[i + offset]));

        if (massimo <= 1) return;

        // Aggiorna target del normalizzatore gradualmente
        scala.Valore(1.0 / massimo);

        // Applica smussamento progressivo per ogni campione
        for (size_t i = 0; i < numeroCampioni; ++i)
            dati[i + offset] *= scala.Smussa();
    }
};
```

**Confronto Architetturale**:

- **Normalizzatore**: Scaling immediato, ideale per processing offline
- **NormalizzatoreSmussato**: Transizioni graduate, previene click in real-time
- **Integration**: Volume class provides smooth parameter changes

### 5. Filtri.h - Digital Signal Processing

#### SmussamentoEsponenziale - Filtro Passa-Basso

```cpp
double Smussa(double valore) noexcept {
    const double t = valoreSmussato;
    valoreSmussato += (valore - valoreSmussato) * fattore;
    return t;
}
```

**Formula Matematica**: Filtro IIR del primo ordine

- `y[n] = y[n-1] + α(x[n] - y[n-1])`
- `α = fattore` determina la cutoff frequency
- Converte time constant in coefficiente: `DaSmussamentoAGuadagno()`

#### RilevatoreMiluppo - Envelope Following

```cpp
double Computa(const double campione) noexcept {
    const double valore = std::fabs(campione);
    const double guadagno = (valore > inviluppo) ? attacco : rilascio;
    inviluppo += (valore - inviluppo) * guadagno;
    return inviluppo;
}
```

**Algoritmo Envelope Follower**:

- **Attack rapido**: Segue velocemente i picchi
- **Release lento**: Decade gradualmente
- **Rettificazione**: `std::fabs()` per envelope dell'ampiezza

#### Compressore/Limitatore - Controllo Dinamica

```cpp
namespace Interno {
    class Compressore {
        double Computa(double campione) noexcept {
            const double inviluppo = rilevatoremiluppo.Computa(campione);
            const double valore = ritardo.Computa(campione);

            // [Np] Neper scale
            const double inviluppoLn = std::log(inviluppo);
            const double guadagnoLn = CS * std::min(0.0,
                sogliaVolumeLn - inviluppoLn);
            const double guadagno = std::exp(guadagnoLn);

            return valore * guadagno;
        }
    };
}
```

**Architettura Look-Ahead**:

1. **Envelope Detection**: Rileva il livello del segnale
2. **Delay Line**: Ritarda il segnale per "predire" i picchi
3. **Gain Reduction**: Calcola riduzione in dB (Neper)
4. **Application**: Applica il guadagno al segnale ritardato

**Formulazione Matematica**:

- **Threshold**: Soglia in scala logaritmica
- **Compression Slope**: `CS = 1 - 1/ratio`
- **Neper Scale**: Logaritmi naturali invece di decibel per efficienza

#### Echo - Effetto Delay

```cpp
double Computa(double campione) noexcept {
    double valore = buffer.back();
    buffer.push_front(campione + velocità * valore);  // Feedback
    return (1 - volumeMassimo) * campione + volumeMassimo * valore;  // Wet/Dry mix
}
```

**Implementazione Circolare**:

- **Buffer**: `boost::circular_buffer` per efficienza memoria
- **Feedback**: `velocità` controlla il decadimento dell'eco
- **Mixing**: Blend between dry signal e wet (delayed)

### 5. StrumentoMusicale.h - Polyphonic Synthesis

#### Architettura Polifonica

```cpp
class StrumentoX: public StrumentoMusicale {
private:
    static const constexpr size_t LunghezzaSerieArmonica = 4;
    std::array<double, LunghezzaSerieArmonica> ampiezze{1.0, 0.25, 0.05, 0.01};

    using Onde = std::array<Oscillatori::OndaSinusoidale, LunghezzaSerieArmonica>;
    std::array<Onde, Note::NumeroNote> note; // 12 note × 4 armoniche = 48 oscillatori

    std::array<InviluppoADSR, Note::NumeroNote> inviluppi;
};
```

**Scalabilità Architatturale**:

- **Static Arrays**: Allocazione compile-time per real-time performance
- **Harmonic Series**: Ogni nota = somma di armoniche ponderate
- **Voice Management**: Un inviluppo per nota per controllo indipendente

#### Synthesis Algorithm

```cpp
virtual double Campione() {
    double valore = 0;

    for (size_t i = 0; i < Note::NumeroNote; ++i) {
        double valoreNota = 0;

        // Somma delle armoniche per questa nota
        for (size_t j = 0; j < LunghezzaSerieArmonica; ++j)
            valoreNota += ampiezze[j] * note[i][j].Campione();

        // Applica inviluppo
        valoreNota *= inviluppi[i].Computa();

        // Somma tutte le note attive
        valore += valoreNota;
    }

    return valore;
}
```

**Mixing Strategy**:

1. **Per-harmonic generation**: Ogni armonica generata indipendentemente
2. **Weighted sum**: Combinazione secondo ampiezze predefinite
3. **Envelope application**: Modellazione temporale per nota
4. **Final mix**: Somma di tutte le note attive

### 6. Audio C++.cpp - Engine Main Loop

#### WASAPI Initialization

```cpp
// Setup COM
CoInitialize(nullptr);

// Device enumeration
IMMLeviceEnumerator* enumerator;
CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                IID_IMMDeviceEnumerator, reinterpret_cast<void**>(&enumerator));

// Get default audio endpoint
IMMDevice* device;
enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device);

// Activate audio client
IAudioClient* clientAudio;
device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL,
                reinterpret_cast<void**>(&clientAudio));
```

#### Real-time Audio Thread

```cpp
static void ThreadAudio() noexcept {
    // Recupera dimensione buffer e calcola timing
    unsigned int dimensioneBuffer;
    clientAudio->GetBufferSize(&dimensioneBuffer);
    const double durataBuffer = DaCampioniASecondi(dimensioneBuffer);

    // Clock sincronizzato a metà del periodo buffer
    Clock clock(durataBuffer / 2.0);

    // Pre-riempie tutto il buffer prima di iniziare
    RiempiBufferAudio(dimensioneBuffer);
    clientAudio->Start();

    clock.Avvia();
    while (!esci) {
        // Timing preciso con misurazione performance
        durataPeriodoAudio.store(clock.AspettaTicchettioSuccessivo() / 1000.0);

        // Riempie prossimo chunk di buffer
        RiempiBufferAudio(dimensioneBuffer / 2);
    }
}
```

**Strategia Timing Avanzata**:

- **durataBuffer / 2.0**: Clock a doppia frequenza per buffer più piccoli
- **Pre-fill**: Riempie tutto il buffer prima dello start
- **Performance monitoring**: Misura timing reale vs teorico

#### Buffer Management

```cpp
static void RiempiBufferAudio(unsigned int numCampioni) noexcept {
    // Performance timing
    std::chrono::time_point tInizio = std::chrono::high_resolution_clock::now();

    // Get WASAPI buffer
    unsigned char* bufferDati;
    clientRiproduzione->GetBuffer(numCampioni, &bufferDati);

    StrumentoMusicale* strumento = strumentoMusicale.load();

    for (size_t i = 0; i < numCampioni; ++i) {
        // Generate mono sample from instrument
        const double campione = echo.Computa(strumento->Campione());

        if constexpr (Costanti::ModalitàCondivisa) {
            // Shared mode: Windows mixer handles limiting
            float* dati = reinterpret_cast<float*>(bufferDati);
            dati[i * Costanti::NumeroCanali + Costanti::CanaleSinistro] = static_cast<float>(campione);
            dati[i * Costanti::NumeroCanali + Costanti::CanaleDestro]   = static_cast<float>(campione);
        } else {
            // Exclusive mode: Manual limiting required
            const double valore = limitatore.Computa(campione);
            const short dato = ConvertiA16Bits(valore);

            short* dati = reinterpret_cast<short*>(bufferDati);
            dati[i * Costanti::NumeroCanali + Costanti::CanaleSinistro] = dato;
            dati[i * Costanti::NumeroCanali + Costanti::CanaleDestro]   = dato;
        }
    }

    clientRiproduzione->ReleaseBuffer(numCampioni, 0);

    // Measure and store generation time
    std::chrono::time_point tFine = std::chrono::high_resolution_clock::now();
    durataProduzioneSuono.store(
        std::chrono::duration_cast<DurataMillisecondi>(
            tFine - tInizio).count());
}
```

**Ottimizzazioni Critiche**:

- **constexpr if**: Decisione modalità a compile-time
- **Performance monitoring**: Cronometra ogni generazione buffer
- **Atomic metrics**: Thread-safe performance reporting
- **Format adaptation**: Auto-conversione float/int based modalità

**Real-time Constraints**:

- **No allocations**: Tutto pre-allocato
- **Minimal processing**: Solo operazioni essenziali
- **Error handling**: Graceful degradation su failure
- **Timing critical**: Mai bloccare o ritardare

### 7. Clock.h - Precision Timing

#### Windows-Specific Optimizations

```cpp
Clock(double periodo_) {
    periodo = periodo_ * 1000.0;
    timeBeginPeriod(static_cast<unsigned int>(sogliaSleep)); // Migliora timer resolution
}

double AspettaTicchettioSuccessivo() {
    // Hybrid sleep strategy
    double attesa = periodo - elapsed_time;
    attesa -= sogliaSleep;

    if (attesa >= 0)
        std::this_thread::sleep_for(
            std::chrono::milliseconds(
                static_cast<unsigned long>(attesa)));

    // Busy wait for final precision
    do {
        attesa = duration_cast<DurataMillisecondi>(
            hrc::now() - tStart).count();
    } while (attesa <= periodo);
}
```

**Timing Strategy**:

- **timeBeginPeriod()**: Richiede al kernel timing più preciso
- **Hybrid approach**: sleep_for() + busy wait
- **High resolution clock**: std::chrono per massima precisione

## Patterns Architetturali Utilizzati

### 1. Strategy Pattern

- **Oscillatori**: Intercambiabili tramite interfaccia comune
- **Strumenti**: Diversi algoritmi di sintesi

### 2. Observer Pattern (Implicito)

- **Thread communication**: Atomic variables per state sharing
- **Event-driven**: Note on/off triggers

### 3. Command Pattern (Implicito)

- **Audio operations**: Ogni sample = comando da eseguire

### 4. Factory Pattern (Implicito)

- **Instrument selection**: Switch between different synthesizers

### 5. RAII (Resource Acquisition Is Initialization)

- **COM objects**: Automatic cleanup tramite distruttori
- **Audio resources**: Gestione automatica lifetime

## Ottimizzazioni Specifiche

### 1. Memory Layout

- **Structure of Arrays**: Arrays di oscillatori invece di array di struct
- **Cache locality**: Dati correlati vicini in memoria
- **Pre-allocation**: Zero allocazioni in real-time path

### 2. Algorithmic Optimizations

- **Complex arithmetic**: Sostituisce trigonometry costosa
- **Table lookups**: Per funzioni complesse (potenzialmente)
- **SIMD-ready**: Strutture dati friendly per vettorizzazione futura

### 3. Thread Optimization

- **Lock-free**: Atomic operations invece di mutex
- **Thread affinity**: Possibile pinning CPU per audio thread
- **Priority elevation**: High priority per audio processing

### 4. Numerical Stability

- **Clamping**: Prevenzione overflow/underflow
- **Normalization**: Mantiene precisione a lungo termine
- **Epsilon handling**: Gestisce comparazioni floating-point

## Limitazioni e Trade-offs Architetturali

### 1. Scalabilità

- **Fixed polyphony**: Numero fisso di voci simultanee
- **Static configuration**: Parametri compile-time

### 2. Portabilità

- **Windows-specific**: WASAPI e API Windows
- **Architecture-dependent**: Atomic assumptions

### 3. Estensibilità

- **Monolithic design**: Difficile aggiungere nuovi strumenti runtime
- **Hard-coded parameters**: Configurazione non dinamica

### 4. Resource Usage

- **Memory footprint**: Arrays statici occupano sempre spazio pieno
- **CPU utilization**: Sempre calcola tutte le voci anche se silent

## Possibili Miglioramenti Architetturali

1. **Plugin Architecture**: VST-like system per strumenti modulari
2. **Dynamic Allocation**: Smart object pooling per voices
3. **SIMD Implementation**: Vettorizzazione dei loop critici
4. **Cross-platform**: Astrazione layer per diversi audio backends
5. **Configuration System**: Runtime parameter adjustment
6. **Performance Profiling**: Built-in metrics e analysis tools
