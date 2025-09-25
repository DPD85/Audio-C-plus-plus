# Concetti Audio di Base - Spiegazione per Principianti

## Che Cos'è il Suono

### Il Suono come Fenomeno Fisico

Il suono è una **vibrazione** che si propaga nell'aria. Immagina di buttare
un sasso in un lago: le onde che si creano sulla superficie dell'acqua sono
simili a come il suono si propaga nell'aria.

### Parametri Fondamentali del Suono

1. **Frequenza (Hz)**: Quante volte al secondo l'onda vibra
   - Basse frequenze = suoni gravi (100-200 Hz)
   - Alte frequenze = suoni acuti (2000-8000 Hz)
   - L'orecchio umano sente circa da 20 Hz a 20.000 Hz

2. **Ampiezza**: Quanto forte è la vibrazione
   - Maggiore ampiezza = suono più forte
   - Si misura spesso in decibel (dB)

3. **Fase**: In che punto del ciclo si trova l'onda
   - Importante quando si mescolano più suoni insieme

## Come il Computer "Vede" il Suono

### Conversione Analogico-Digitale

Il suono naturale è **analogico** (continuo), ma il computer lavora con
numeri **digitali** (discreti). Per rappresentare il suono nel computer:

1. **Campionamento**: Si "fotografa" l'onda sonora migliaia di volte al secondo
2. **Quantizzazione**: Ogni "foto" diventa un numero
3. **Frequenza di campionamento**: Quante "foto" al secondo
   - 44.1 kHz = 44.100 foto/secondo (qualità CD)
   - 48 kHz = 48.000 foto/secondo (qualità professionale, usata in questo progetto)

### Esempio Pratico

Se hai un'onda che vibra a 440 Hz (la nota La), con campionamento a 48kHz
ottieni:

- 48.000 ÷ 440 = circa 109 campioni per ogni ciclo completo dell'onda
- Questo è più che sufficiente per rappresentare fedelmente la nota

## Sintesi Audio: Creare Suoni dal Nulla

### Oscillatori: I Generatori di Base

Un **oscillatore** è come un "generatore di onde". Nel progetto ne abbiamo due tipi:

#### 1. Onda Sinusoidale (OndaSinusoidale)

- Forma matematicamente perfetta: sin(2πft)
- Suono "puro", senza armoniche
- Come un diapason perfetto
- Usata per toni base semplici

#### 2. Onda Quadra (OndaQuadra)

- Forma rettangolare: +1, +1, -1, -1, +1...
- Ricca di armoniche (multipli della frequenza base)
- Suono più "digitale" e aggressivo
- Tipica dei sintetizzatori vintage

### Inviluppi ADSR: Come Cambia il Volume nel Tempo

Nessun strumento reale ha volume costante. L'**inviluppo ADSR** simula come
cambia il volume:

1. **Attack**: Quanto veloce cresce il volume
   - Pianoforte = attacco veloce (colpo martello)
   - Archi = attacco lento (archetto che inizia)

2. **Decay**: Quanto veloce scende al livello di sostentamento
   - Simula il comportamento naturale dopo l'attacco iniziale

3. **Sustain**: Volume mantenuto mentre tieni premuto il tasto
   - Non è un tempo ma un livello (es. 80% del massimo)

4. **Release**: Quanto veloce svanisce quando lasci il tasto
   - Note acute = rilascio veloce (suoni corti)
   - Note gravi = rilascio lento (risuonano di più)

### Armoniche: Perché gli Strumenti Suonano Diverso

Una nota (es. Do a 261.6 Hz) in natura non è mai "pura". Contiene anche:

- **Frequenza fondamentale**: 261.6 Hz (quella che sentiamo come "altezza")
- **Armoniche**: 523.2 Hz, 784.8 Hz, 1046.4 Hz... (multipli)

Nel **StrumentoX** del progetto:

- 1° armonica (fondamentale): ampiezza 1.0 (100%)
- 2° armonica (ottava): ampiezza 0.25 (25%)
- 3° armonica: ampiezza 0.05 (5%)
- 4° armonica: ampiezza 0.01 (1%)

Questo mix crea un suono più "ricco" e naturale.

## Effetti Audio: Modificare il Suono

### Echo/Riverbero

**Cos'è**: Quando parli in una cattedrale, senti echi delle tue parole
**Come funziona nel codice**:

1. Salva il suono in un "buffer" (memoria temporanea)
2. Dopo un po' (es. 20 millisecondi), riproduce il suono salvato
3. Lo mixa con il suono originale
4. Il parametro "velocità" controlla quanto velocemente l'eco svanisce

### Limitatore

**Problema**: Se il volume supera il 100%, si crea distorsione orribile
**Soluzione**: Il limitatore taglia tutto quello che supera una soglia
**Magia**: Usa "look-ahead" = vede in anticipo i picchi e riduce il volume
prima che arrivino

### Compressore

**Cos'è**: Riduce la differenza tra suoni forti e deboli
**Esempio**: Cantante che sussurra e poi grida → compressore rende tutto più
uniforme
**Parametri**:

- Soglia: "sopra questo volume, intervieni"
- Ratio: "quanto riduci" (2:1 = se entra 2dB sopra soglia, esce solo 1dB)

## Teoria Musicale nel Codice

### Frequenze delle Note

Nel progetto si usa **intonazione naturale** invece del temperamento equabile:

**Temperamento Equabile** (pianoforti moderni):

- Ogni semitono ha rapporto fisso: 2^(1/12) ≈ 1.059
- Do-Do# = Do#-Re = Re-Re# = stesso rapporto
- Matematicamente uniforme ma non "puro"

**Intonazione Naturale** (questo progetto):

- Rapporti basati su frazioni semplici:
  - Do-Sol = 3:2 = 1.5 (quinta perfetta)
  - Do-Mi = 5:4 = 1.25 (terza maggiore)
- Più "puro" matematicamente ma complicato da accordare

**La di riferimento = 432 Hz** (invece dei classici 440 Hz)

- Alcuni credono suoni più "naturale"
- Matematicamente dà numeri più "tondi" con l'intonazione naturale

## Programmazione Audio Real-Time

### Threading: Perché Servono Thread Multipli

**Thread Audio**: Deve generare campioni a ritmo perfettamente costante

- 48.000 campioni/secondo = un campione ogni 0.0000208 secondi
- Se è in ritardo anche di poco, si sente "scoppiettio"
- Priorità alta del sistema operativo

**Thread UI**: Gestisce tastiera, grafica, interfaccia

- Può essere più lento senza problemi
- Non deve mai bloccare il thread audio

### Sincronizzazione Thread-Safe

**Problema**: Due thread che modificano la stessa variabile simultaneamente
**Soluzione**: `std::atomic<double>`

- Operazioni "atomiche" = indivisibili
- Thread audio legge frequenze mentre UI thread le scrive
- Nessun conflitto o corruzione dati

### Buffer Circolari

Per gli effetti di delay/echo serve "memoria":

```cpp
boost::circular_buffer<double> buffer(1000); // 1000 campioni di storia
buffer.push_front(nuovo_campione);          // nuovo entra davanti
double vecchio = buffer.back();             // vecchio esce dietro
```

## Formati Audio e Compatibilità

### WAV: Il Contenitore

File WAV = Header + Dati:

**Header** dice:

- Frequenza campionamento (48.000 Hz)
- Numero canali (2 per stereo)
- Bit per campione (32-bit float)
- Dimensione totale file

**Dati**:

- Sequenza di numeri che rappresentano i campioni
- Stereo = alternati: SinistroCamp1, DestroCamp1, SinistroCamp2,
  DestroCamp2...

### WASAPI: Comunicazione con Windows

**Windows Audio Session API** = il modo "moderno" per far suonare qualcosa su Windows

**Modalità Condivisa** (usata nel progetto):

- Altri programmi possono suonare contemporaneamente
- Windows mixa tutto insieme
- Campioni float [-1.0, +1.0]

**Modalità Esclusiva** (alternativa):

- Solo questo programma usa l'audio
- Controllo diretto della scheda audio
- Latenza minore ma conflitti con altri programmi

## Ottimizzazioni e Performance

### Perché Serve Efficienza Estrema

Con 48kHz stereo = 96.000 numeri da calcolare ogni secondo
Anche un calcolo che sembra "veloce" può diventare un problema.

### Trucchi Usati nel Codice

**Oscillatori con Numeri Complessi**:
Invece di `sin(2πft)` costoso:

```cpp
fase *= std::exp(i * 2π * frequenza / sampleRate); // rotazione
campione = fase.imag();                             // parte immaginaria = seno
```

**Pre-calcolo di Costanti**:

- Frequenze calcolate all'avvio, non ogni campione
- Fattori di smussamento calcolati una volta

**Lock-Free Programming**:

- Nessun `mutex` o `lock` nel thread audio
- Solo operazioni atomiche ultra-veloci

## Debugging Audio: Come Capire Cosa Va Storto

### Problemi Comuni

1. **Crackling/Popping**: Thread audio non abbastanza veloce
2. **Suono distorto**: Volume troppo alto (>1.0)
3. **Suono metallico**: Aliasing (frequenze troppo alte)
4. **Latenza**: Buffer troppo grandi
5. **Drop-out**: Sistema troppo carico

### Strumenti di Analisi

Nel codice sono presenti cronometri per misurare:

- Tempo generazione campioni
- Tempo periodo audio WASAPI
- Detect underruns e performance issues

## Conclusioni per Principianti

Questo progetto è un **sintetizzatore completo** che:

1. **Genera** suoni da formule matematiche (oscillatori)
2. **Modella** come cambiano nel tempo (ADSR)
3. **Processa** con effetti realistici (echo, limiter)
4. **Esporta** in formato standard (WAV)
5. **Suona** in tempo reale (WASAPI)

La **matematica dietro** non è complessa di per sé, ma deve essere eseguita
**decine di migliaia di volte al secondo** senza mai ritardare.

È un ottimo esempio di come **teoria musicale**, **matematica**,
**programmazione sistem** e **ottimizzazione** si combinino per creare
qualcosa che fa musica!
