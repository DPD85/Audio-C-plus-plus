# Analisi del Sintetizzatore Audio-C-plus-plus

## Panoramica del Progetto

Questo progetto implementa un sintetizzatore audio in tempo reale scritto in
C++ che utilizza le Windows Core Audio APIs (WASAPI) per la riproduzione
audio. Il sistema è progettato per generare suoni musicali tramite oscillatori,
applicare inviluppi dinamici e effetti audio, tutto in tempo reale.

## Architettura Generale

### Componenti Principali

1. **Core Audio Engine** - Gestisce l'interfaccia con Windows Audio APIs
2. **Strumenti Musicali** - Implementa diversi tipi di sintetizzatori
3. **Oscillatori** - Generano forme d'onda base (seno, quadra)
4. **Sistema Inviluppi** - Controlla l'evoluzione del volume nel tempo (ADSR)
5. **Filtri ed Effetti** - Elaborazione del segnale audio (echo, limitatore, compressore)
6. **Sistema di Output** - Esportazione in formato WAV

### Flusso del Segnale Audio

```text
Input Tastiera → Strumento Musicale → Oscillatori → Inviluppi ADSR →
Effetti Audio → Limitatore → Output WASAPI/WAV
```

## Componenti Dettagliati

### 1. Configurazione Audio (CostantiEdAltro.h)

Il sistema è configurato per:

- **Frequenza di campionamento**: 48kHz (standard professionale)
- **Canali**: Stereo (2 canali)
- **Modalità**: Condivisa (compatibile con altri programmi)
- **Formato campioni**: Float 32-bit per massima precisione

#### Teoria Musicale Implementata

Il progetto utilizza l'**intonazione naturale** (not temperamento equabile):

- La di riferimento: 432 Hz (alternativa ai classici 440 Hz)
- Rapporti matematici puri tra le note (es. Do-Sol = 3:2)
- Calcolo delle frequenze basato sui rapporti armonici naturali

### 2. Strumenti Musicali (StrumentoMusicale.h)

#### Classe Base: StrumentoMusicale

Interfaccia astratta che definisce:

- `InizioNota(Note nota)` - Attiva una nota
- `FineNota(Note nota)` - Disattiva una nota
- `Campione()` - Genera il prossimo campione audio

#### Implementazioni Concrete

##### a) SenzaSerieArmonica

- Un oscillatore sinusoidale per nota
- Suono "puro" senza armoniche aggiuntive
- Ideale per toni semplici e chiari

##### b) StrumentoX

- Serie armonica con 4 componenti per nota
- Ampiezze decrescenti: [1.0, 0.25, 0.05, 0.01]
- Simula il suono di strumenti acustici reali
- Ogni nota ha frequenze multiple (fondamentale + armoniche)

##### c) StrumentoY

- Utilizza onde quadre invece di sinusoidali
- Suono più "digitale" e aggressivo
- Ricco di armoniche naturali dell'onda quadra

### 3. Oscillatori (Oscillatori.h)

#### OndaSinusoidale

- Implementazione usando numeri complessi per efficienza
- Genera sin(2πft) dove f=frequenza, t=tempo
- Usa rotazione nel piano complesso per evitare calcoli trigonometrici costosi
- Thread-safe per cambio frequenza in tempo reale

#### OndaQuadra

- Basata su oscillatore sinusoidale + funzione segno
- Produce forme d'onda rettangolari perfette
- Duty cycle fisso al 50%

### 4. Inviluppi ADSR (Inviluppo.h)

Implementa il classico inviluppo **Attack-Decay-Sustain-Release**:

- **Attack (A)**: Fase di crescita rapida del volume (0.02s)
- **Decay (D)**: Decadimento a livello di sustain (0.01s)
- **Sustain (S)**: Mantenimento del volume costante (80%)
- **Release (R)**: Diminuzione graduale quando si rilascia la nota
  (variabile per frequenza)

**Dettaglio Tecnico**:

- Il tempo di release diminuisce per note acute (formula: 0.5 - 0.0333 * indiceNota)
- Simula il comportamento degli strumenti acustici reali
- Implementazione thread-safe usando std::atomic

### 5. Filtri ed Effetti (Filtri.h)

#### SmussamentoEsponenziale

- Filtro passa-basso per transizioni graduali
- Formula: `nuovoValore = vecchioValore + (target - vecchioValore) * fattore`
- Previene click e pop negli audio

#### RilevatoreInviluppo / InseguitorePicchi

- Calcola l'ampiezza istantanea del segnale
- Tempo di attacco molto veloce, rilascio più lento
- Base per compressori e limitatori

#### Limitatore (Look-ahead Limiter)

- Previene il clipping digitale mantenendo il suono sotto una soglia
- Utilizza un ritardo per "prevedere" i picchi
- Essenziale per evitare distorsioni

#### Compressore

- Riduce la dinamica del segnale
- Parametri configurabili: soglia, ratio, attacco, rilascio
- Rende il suono più uniforme e "professionale"

#### Echo/Riverbero

- Implementa eco con controllo wet/dry
- Buffer circolare per il delay
- Feedback controllabile per echi multipli

### 6. Sistema di Timing (Clock.h)

Gestisce la sincronizzazione temporale precisa:

- Utilizza `timeBeginPeriod()` per migliorare risoluzione timer Windows
- Combina `std::this_thread::sleep_for()` + busy-wait per precisione microsecond
- Cruciale per mantenere la frequenza di campionamento costante

### 7. Esportazione WAV (WAVE.h)

Implementa la scrittura di file WAV standard:

- Header RIFF compatibile
- Supporto sia per campioni integer che float
- Metadati corretti per frequenza e canali

## Concetti Audio Fondamentali

### Frequenza di Campionamento

48kHz significa che il sistema cattura/genera 48.000 campioni al secondo per
canale. Questo permette di rappresentare frequenze fino a 24kHz (teorema di
Nyquist), ben oltre l'udito umano (~20kHz).

### Campioni Audio

Ogni "campione" è un numero che rappresenta l'ampiezza dell'onda sonora in un
istante specifico. Il sistema usa float [-1, +1] per massima precisione.

### Intonazione Naturale vs Temperata

- **Temperata**: Tutti i semitoni sono equidistanti (ratio = 2^(1/12) ≈ 1.059)
- **Naturale**: Usa rapporti matematici puri (es. quinta = 3:2 = 1.5)
- Il progetto usa la naturale per suoni più "puri" matematicamente

### Threading e Real-time Audio

Il sistema usa thread separati per:

- **Audio thread**: Genera campioni a ritmo costante (priorità alta)
- **Keyboard thread**: Gestisce input tastiera e interfaccia console (60 FPS)
- **Main thread**: Setup iniziale e coordinamento
- **Atomic variables**: Comunicazione thread-safe tra i thread

### Thread Tastiera Dettagliato

- **Polling a 60 FPS**: Clock a 17ms per responsività ottimale
- **GetAsyncKeyState**: Lettura diretta stato tasti Windows
- **Console UI**: Aggiornamento real-time metriche performance
- **Layout ASDF+JKL**: Mapping ergonomico simile a piano
- **Atomic switching**: Cambio strumenti senza interruzioni audio

## Flusso di Esecuzione Completo

1. **Inizializzazione**: Setup WASAPI, inizializzazione strumenti
2. **Loop principale**:
   - Thread tastiera cattura input utente
   - Thread audio richiesto da WASAPI (callback)
   - Ogni callback genera N campioni richiesti
3. **Per ogni campione**:
   - Somma tutti gli oscillatori attivi
   - Applica inviluppo ADSR
   - Processa attraverso effetti
   - Applica limitatore finale
   - Invia a WASAPI o salva in WAV

## Performance e Ottimizzazioni

- **Lock-free design**: Uso intensivo di std::atomic per comunicazione tra thread
- **Oscillatori efficienti**: Rotazione complessa invece di sin() costosi
- **Memory pooling**: Buffer pre-allocati per evitare allocazioni real-time
- **SIMD potential**: Struttura dati friendly per future ottimizzazioni vettoriali

## Limitazioni e Possibili Miglioramenti

### Attuali Limitazioni

- Solo Windows (WASAPI)
- Numero fisso di oscillatori
- Manca modulazione (LFO, vibrato)
- Nessun controllo MIDI

### Miglioramenti Possibili

- Port a PortAudio per cross-platform
- Sistema modulare per effetti
- Supporto VST plugin
- Interfaccia grafica più avanzata
- Preset e salvataggio configurazioni
