# Guida alla Compilazione e all'Uso del Sintetizzatore

## Requisiti di Sistema

### Sistema Operativo

- **Windows 10/11**: Richiesto per WASAPI (Windows Audio Session API)
- **Architettura**: x64 preferibile per performance ottimali

### Compilatore e Tools

- **Visual Studio 2019/2022**: Con C++ toolkit completo
- **Windows SDK**: Per le API audio native
- **C++20 Standard**: Per `std::numbers::pi` e altre features moderne

### Dipendenze Librerie

- **Boost Libraries**: Per `circular_buffer`
- **Windows Core Audio APIs**: Incluse nel Windows SDK

## Struttura Build System

### File di Progetto

```text
Codice/
├── Audio C++.sln                 # Visual Studio Solution
├── Audio C++/
│   ├── Audio C++.vcxproj        # Project file principale
│   ├── Audio C++.vcxproj.filters # Organizzazione files in IDE
│   ├── conanfile.py             # Gestione dipendenze con Conan
│   └── conandata.yml            # Configurazione Conan
```

### Configurazione Conan

Il progetto usa **Conan** per gestione dipendenze. File `conanfile.py` specifica:

- Versione Boost richiesta
- Configurazioni di build
- Opzioni platform-specific

## Processo di Compilazione

### Setup Iniziale

```cmd
# 1. Installa dipendenze via Conan
conan install . --build missing

# 2. Genera project files se necessario
conan create . --build missing

# 3. Apri la solution in Visual Studio
start "Audio C++.sln"
```

### Build Configurations

#### Debug Configuration

- **Ottimizzazioni**: Disabilitate per debugging
- **Symbols**: Full debug information
- **Runtime**: Multi-threaded Debug DLL (/MDd)
- **Warnings**: Tutte abilitate (/W4)

#### Release Configuration

- **Ottimizzazioni**: Massime (/O2)
- **Symbols**: Ottimizzate per release
- **Runtime**: Multi-threaded DLL (/MD)
- **Intrinsics**: Enable per performance (/Oi)

### Possibili Problemi di Compilazione

#### 1. Encoding Issues

Alcuni file hanno caratteri Unicode che potrebbero causare problemi:

```cpp
// Se hai errori come "carattere non riconosciuto"
// Assicurati che i file siano salvati in UTF-8 with BOM
```

#### 2. Missing Windows Headers

```cpp
// Se mancano definizioni WASAPI:
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <audioclient.h>
```

#### 3. Boost Linking

```cpp
// Se hai errori di linking con boost:
#pragma comment(lib, "boost_system.lib")  // Se necessario
```

## Utilizzo del Sintetizzatore

### Controlli Base

#### Tastiera Computer → Note Musicali

Mapping tastiera **effettivo** implementato nel codice:

**Note naturali** (tasti inferiori):

```text
A = Do    S = Re    D = Mi    F = Fa    J = Sol    K = La    L = Si
```

**Diesis/Bemolli** (tasti superiori):

```text
W = Do#   E = Re#             U = Fa#   I = Sol#   O = La#
```

Layout visuale della "tastiera musicale":

```text
[W] [E]     [U] [I] [O]     ← Diesis/Bemolli (tasti neri del piano)
 │   │       │   │   │
[A] [S] [D] [F] [J] [K] [L] ← Note naturali (tasti bianchi)
Do  Re  Mi  Fa  Sol La  Si
```

#### Comandi Durante l'Esecuzione

- **Q**: Seleziona StrumentoX (armoniche multiple)
- **P**: Seleziona SenzaSerieArmonica (toni puri)
- **ESC**: Uscita dal programma
- **Note**: Usa le combinazioni di tasti sopra indicate

#### Interfaccia Console

Il programma mostra un'interfaccia testuale real-time aggiornata a 60 FPS:

```text
Formato dati audio
... informazioni formato WASAPI ...

Dispositivo di riproduzione: Speakers (Realtek HD Audio)

[*] Tasto Q = suona col primo strumento musicale
[ ] Tasto P = suona col secondo strumento musicale

  2.1234 ms. periodo audio
  0.8765 ms. durata produzione audio
 17.0001 ms. periodo tastiera
```

**Metriche Real-time**:

- **Periodo audio**: Intervallo tra callback WASAPI (ideale: ~2ms per 48kHz)
- **Durata produzione**: Tempo CPU per generare buffer (deve essere < periodo)
- **Periodo tastiera**: Timing thread UI (target: 17ms = 60 FPS)

**Indicatori Status**:

- `[*]`: Strumento attualmente selezionato
- `[ ]`: Strumento disponibile ma non attivo

### Modi Operativi

#### 1. Modalità Sintetizzatore Real-time

```cpp
// Il programma parte in modalità normale
// Premi tasti per suonare note
// Il suono esce immediatamente dagli altoparlanti
```

#### 2. Modalità Registrazione (se abilitata)

```cpp
#if 0  // Nel codice main
    RegistraPerGrafico();  // Se decommentato
    return EXIT_SUCCESS;
#endif
```

### Strumenti Disponibili

#### StrumentoX (Default)

- **Caratteristiche**: Serie armonica ricca, suono naturale
- **Timbro**: Simile a strumenti acustici
- **Complessità**: 4 armoniche per nota

#### SenzaSerieArmonica

- **Caratteristiche**: Toni puri sinusoidali
- **Timbro**: Pulito, elettronico
- **Uso**: Diapason elettronico, test

#### StrumentoY (Onde Quadre)

- **Caratteristiche**: Forme d'onda rettangolari
- **Timbro**: Aggressivo, sintetizzatore vintage
- **Applicazioni**: Suoni elettronici, chiptune

## Configurazione Avanzata

### Modifica Parametri Audio

#### Frequenza di Campionamento

```cpp
// In CostantiEdAltro.h
const constexpr double FrequenzaCampionamento = 48000; // Cambiabile
// Valori comuni: 22050, 44100, 48000, 96000
```

#### Numero Canali

```cpp
const constexpr unsigned int NumeroCanali = 2;  // Stereo
// 1 = Mono, 2 = Stereo, 6 = 5.1 surround (teorico)
```

#### Modalità Audio

```cpp
const constexpr bool ModalitàCondivisa = true;
// true  = Condivisa (consigliato)
// false = Esclusiva (bassa latenza, conflitti)
```

### Tuning e Temperamento

#### Cambio La di Riferimento

```cpp
const constexpr double FrequenzaRiferimentoLa = 432; // [Hz]
// 432 Hz = A4 alternativo
// 440 Hz = A4 standard moderno
// 415 Hz = Barocco storico
```

#### Passaggio a Temperamento Equabile

```cpp
// Decommentare sezione #else in CostantiEdAltro.h
#else
    const double FrequenzaDo = FrequenzaRiferimentoLa * std::pow(2.0, -9.0/12.0);
    // ... resto delle frequenze temperate
#endif
```

### Modifica Effetti Audio

#### Parametri Echo

```cpp
static EchoWetDry echo(0.020, 0.9, 0.6);
//                     ^^^^  ^^^  ^^^
//                   Delay  Decay WetLevel
// Delay:    0.001-1.0 secondi
// Decay:    0.1-0.99 feedback amount
// WetLevel: 0.0-1.0 mix wet/dry
```

#### Parametri Limitatore

```cpp
static Limitatore limitatore(1.0, 1.0, 0.00042);
//                          ^^^  ^^^  ^^^^^^^
//                       Release Thresh Lookahead
// Release:   0.1-10.0 secondi
// Thresh:    0.1-1.0 threshold
// Lookahead: 0.0001-0.01 secondi
```

## Output e Export

### Export WAV

Il codice include funzionalità per salvare in formato WAV:

```cpp
// Funzioni disponibili in WAVE.cpp
void CreaFileSemplice();  // File mono
void CreaFileStereo();    // File stereo
```

### Formato Output WAV

- **Sample Rate**: 48kHz (configurabile)
- **Bit Depth**: 32-bit float (modalità condivisa) o 16-bit int (esclusiva)
- **Channels**: Stereo
- **Format**: Standard RIFF/WAVE

## Performance e Ottimizzazione

### Monitoring Performance

#### Metriche Built-in

```cpp
static std::atomic<double> durataPeriodoAudio;         // [ms]
static std::atomic<double> durataProduzioneSuono;     // [ms]
```

Queste variabili mostrano:

- **Periodo Audio**: Tempo tra callback WASAPI
- **Produzione Suono**: Tempo per generare i campioni

#### Soglie di Allarme

- **Periodo > 5ms**: Possibili glitch audio
- **Produzione > Periodo**: Definitivi dropout audio

### Ottimizzazione Sistema

#### Priorità Thread

```cpp
// Nel thread audio
SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
```

#### Configurazione Windows

1. **Modalità Alte Prestazioni**: Power plan
2. **Disabilita risparmio energetico**: USB e audio devices
3. **Chiudi programmi non necessari**: Libera CPU

#### Buffer Size Tuning

```cpp
// Nel setup WASAPI - modificabile
REFERENCE_TIME requestedDuration = 10000000;  // 1 secondo in 100ns units
// Valori più bassi = latenza minore ma maggior rischio dropout
```

## Troubleshooting

### Problemi Audio

#### Nessun Suono

1. **Verifica device audio**: Controlla device di output Windows
2. **Modalità esclusiva**: Altri programmi potrebbero bloccare
3. **Formato non supportato**: Cambia sample rate o bit depth

#### Crackling/Popping

1. **Buffer underrun**: Aumenta buffer size
2. **CPU overload**: Chiudi altri programmi
3. **Driver audio**: Aggiorna driver scheda audio

#### Latenza Alta

1. **Buffer size**: Riduci con attenzione
2. **Modalità esclusiva**: Considera switch se necessario
3. **Audio driver**: ASIO drivers se disponibili

### Problemi di Compilazione

#### Boost Non Trovato

```cmd
# Reinstalla con percorsi specifici
conan install . --build boost --build missing
```

#### Errori Unicode

```cmd
# Converti files in UTF-8
# O modifica project per CP1252
```

#### Linking Errors

```cmd
# Verifica architettura x86 vs x64 consistency
# Controlla runtime library matching (/MD vs /MT)
```

## Estensioni e Modifiche

### Aggiungere Nuove Forme d'Onda

```cpp
// In Oscillatori.h
class OndaTringleare: public Oscillatore {
    // Implementa logica specifica
    virtual double Campione() noexcept {
        // ... algoritmo onda triangolare
    }
};
```

### Nuovi Strumenti Musicali

```cpp
// In StrumentoMusicale.h
class StrumentoCustom: public StrumentoMusicale {
private:
    // Definisci oscillatori e parametri specifici

public:
    virtual double Campione() override {
        // Implementa algoritmo di sintesi
    }
};
```

### Effetti Aggiuntivi

```cpp
// In Filtri.h
class Phaser {
public:
    double Computa(double campione) noexcept {
        // Implementa algoritmo phaser
    }
};
```

## Best Practices per Sviluppo

### 1. Real-time Safety

- **Mai allocare memoria** nel thread audio
- **No mutex/locks** nel path critico
- **Pre-calcola** tutti i parametri possibili

### 2. Testing Audio

- **Oscilloscopio software**: Per analizzare forme d'onda
- **Spectrum analyzer**: Per contenuto armonico
- **Latency meter**: Per misurare round-trip delay

### 3. Code Organization

- **Separazione concerns**: Audio engine vs UI vs I/O
- **Unit testing**: Per componenti DSP
- **Version control**: Backup frequenti durante modiche

### 4. Performance Profiling

- **Audio thread priority**: Monitora thread priority
- **CPU usage**: Verifica carico per core
- **Memory access**: Profila cache misses

## Conclusioni

Questo sintetizzatore rappresenta un ottimo esempio di:

- **Real-time audio programming** in C++
- **Digital Signal Processing** applicato
- **Windows audio programming** con WASAPI
- **High-performance C++** per applicazioni critiche

Per approfondire, studia i libri referenziati in `Documenti/Readme.md` e
sperimenta con i parametri per capire gli effetti sui suoni prodotti.
