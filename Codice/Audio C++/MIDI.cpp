#include "MIDI.h"

#include "CodaEliminazione.h"
#include "CostantiEdAltro.h"

namespace MIDI
{
    enum FormatiFile
    {
        Formato0,
        Formato1,
        Formato2
    };

    static inline unsigned short Endianess(unsigned short valore);
    static inline unsigned int Endianess(unsigned int valore);
    static unsigned char LeggiByte(std::ifstream &file);
    static unsigned short LeggiInt16(std::ifstream &file);
    static unsigned int LeggiInt24(std::ifstream &file);
    static unsigned int LeggiInt32(std::ifstream &file);
    static std::string LeggiStringa(std::ifstream &file, size_t lunghezza);
    static unsigned int LeggiInteroALunghezzaVariabile(std::ifstream &file);

    bool LeggiFile(std::wstring percorso)
    {
        CodaEliminazione eliminatori;

        // ----- -----

        std::ifstream file(percorso, std::ios_base::binary);
        if (!file.is_open()) return false;
        eliminatori.Aggiungi(
            [&file]()
            {
                file.close();
            });

        // ----- -----

        size_t dimensioneFile;

        {
            file.seekg(0, std::ios_base::end);
            dimensioneFile = file.tellg();
            file.seekg(0, std::ios_base::beg);
        }

        // ----- -----

        unsigned int lunghezzaIntestazione;

        {
            char firma[4];
            file.read(firma, 4);

            if (firma[0] != 'M' || firma[1] != 'T' || firma[2] != 'h' || firma[3] != 'd')
            {
                return false;
            }

            lunghezzaIntestazione = LeggiInt32(file);
            if (lunghezzaIntestazione >= dimensioneFile)
            {
                return false;
            }
        }

        // ----- -----

        unsigned short formatoFile;

        {
            formatoFile = LeggiInt16(file);

            if (formatoFile != FormatiFile::Formato0 && formatoFile != FormatiFile::Formato1
                && formatoFile != FormatiFile::Formato2)
            {
                return false;
            }

            std::wcout << L"File in formato: " << formatoFile << L'\n';
        }

        // ----- -----

        unsigned short numeroTracce;

        {
            numeroTracce = LeggiInt16(file);

            if (formatoFile == FormatiFile::Formato0 && numeroTracce != 1)
            {
                return false;
            }

            std::wcout << L"Numero tracce: " << numeroTracce << L'\n';
        }

        // ----- -----

        unsigned short divisione;

        {
            divisione = LeggiInt16(file);

            if (divisione & 0x8000)
            {
                // formato SMPTE non supportato
                return false;
            }
            else {
                // Numero ticchettii corrispondenti ad un quarto di nota
            }

            std::wcout << L"Divisione: " << divisione << L'\n';
        }

        // ----- Salto eventuali byte inattesi nell'intestazione del file -----

        // Nel caso l'intestazione sia più lunga di quanto atteso mi sposto in avanti della lunghezza specificata così
        // da saltare ed ignorare i dati inattesi.
        file.seekg(4 + 4 + lunghezzaIntestazione, std::ios_base::beg);

        // ----- 1° traccia -----

        unsigned int lunghezzaTraccia; // [byte]

        {
            char firma[4];
            file.read(firma, 4);

            if (firma[0] != 'M' || firma[1] != 'T' || firma[2] != 'r' || firma[3] != 'k')
            {
                return false;
            }

            lunghezzaTraccia = LeggiInt32(file);
            if (lunghezzaTraccia >= dimensioneFile)
            {
                return false;
            }
        }

        const size_t posFineTraccia = static_cast<size_t>(file.tellg()) + lunghezzaTraccia;

        // ----- Lista eventi nella traccia -----

        double tempo      = Costanti::TempoDefaultMIDI; // [ms/qn] qn = quarto di nota
        char statoECanale = 0;
        bool fineTraccia  = false;

        unsigned char byte;
        size_t posizioneNelFile;

        do
        {
            const unsigned int deltaTempo = LeggiInteroALunghezzaVariabile(file);
            std::wcout << L"Delta tempo: " << deltaTempo << L' ' << (deltaTempo * (tempo / divisione)) << L" ms.\n";

            byte = LeggiByte(file);

            // System exclusive message format 1 (0xF1) or 2 (0xF7)
            if (byte == 0xF1 || byte == 0xF7)
            {
                // Ignoro il messaggio
                const size_t lunghezza = LeggiInteroALunghezzaVariabile(file);
                file.seekg(lunghezza, std::ios_base::cur);
            }
            // Meta-evento
            else if (byte == 0xFF)
            {
                const unsigned char tipo = LeggiByte(file); // dovrebbe essere [0, 127]
                const size_t lunghezza   = LeggiInteroALunghezzaVariabile(file);
                const size_t posFineDati = static_cast<size_t>(file.tellg()) + lunghezza;

                switch (tipo)
                {
                    case 0x01: // Evento testo
                    {
                        const std::string testo = LeggiStringa(file, lunghezza);
                        std::wcout << L"Testo: " << testo.data() << L'\n';
                        break;
                    }
                    case 0x02: // Copyright
                    {
                        const std::string copyright = LeggiStringa(file, lunghezza);
                        std::wcout << L"Copyright: " << copyright.data() << L'\n';
                        break;
                    }
                    case 0x03: // Nome sequenza o traccia
                    {
                        const std::string nome = LeggiStringa(file, lunghezza);
                        std::wcout << L"Nome sequenza o traccia: " << nome.data() << L'\n';
                        break;
                    }
                    case 0x04: // Nome strumento
                    {
                        const std::string nome = LeggiStringa(file, lunghezza);
                        std::wcout << L"Nome strumento: " << nome.data() << L'\n';
                        break;
                    }
                    case 0x2F: // Fine straccia
                        fineTraccia = true;
                        break;
                    case 0x51: // Impostazione tempo
                    {
                        assert(lunghezza >= 3);

                        const unsigned int t = LeggiInt24(file); // [µs]

                        tempo = t / 1000.0; // [ms]
                        std::wcout << L"Impostazione tempo: " << tempo << L" ms.\n";
                        break;
                    }
                    case 0x58: // Firma temporale
                    {
                        assert(lunghezza >= 4);

                        const unsigned char nn = LeggiByte(file);
                        const unsigned char dd = LeggiByte(file);
                        const unsigned char cc = LeggiByte(file);
                        const unsigned char bb = LeggiByte(file);
                        std::wcout << L"Firma temporale: " << nn << L'\\' << std::pow(2, dd) << L' ' << cc << L' ' << bb
                                   << L'\n';
                        break;
                    }
                    case 0x59: // Firma della chiave
                    {
                        assert(lunghezza >= 2);

                        const unsigned char sf = LeggiByte(file);
                        const unsigned char mi = LeggiByte(file);
                        switch (sf)
                        {
                            case -7:
                                std::wcout << L"7 flats";
                                break;
                            case -1:
                                std::wcout << L"1 flats";
                                break;
                            case 0:
                                std::wcout << L"key of C";
                                break;
                            case 1:
                                std::wcout << L"1 sharp";
                                break;
                            case 7:
                                std::wcout << L"7 sharp";
                                break;
                        }
                        switch (mi)
                        {
                            case 0:
                                std::wcout << L" major\n";
                                break;
                            case 1:
                                std::wcout << L" minor\n";
                                break;
                        }
                        break;
                    }
                    default: // Ignoro tutti gli altri eventi
                        break;
                }

                // Ignoro i dati degli eventi non gestiti o gli eventuali dati aggiuntivi non attesi per gli eventi
                // gestiti
                file.seekg(posFineDati, std::ios_base::beg);
            }
            // Evento MIDI, nelle specifiche del file MIDI sono previsti solo i messaggi relativi ai canali
            else {
                // I byte nell'intervallo [0x00, 0x7F] sono byte di dati, bit più significativo è sempre settato a zero
                // I byte nell'intervallo [0x80, 0xFF] sono byte di stato, bit più significativo è sempre settato a uno
                // I messaggi di un canale sono composti da un byte di stato seguito da one o due byte di dati
                //  Nel byte di stato:
                //      - i 4 bit meno significativi indicano il numero del canale;
                //      - i 4 bit più significativi indicano il tipo di messaggio.

                // Il primo byte di dato del messaggio corrente
                char dato1;

                // Se si tratta di un byte di stato
                if (byte & 0x80)
                {
                    statoECanale = byte;
                    dato1        = LeggiByte(file);
                }
                // Si tratta di un byte di dati
                else dato1 = byte;

                const int canale = statoECanale & 0x0F;
                const int stato  = statoECanale & 0xF0;

                switch (stato)
                {
                    case 0x80: // nota off
                    {
                        const char nota     = dato1;           // [0, 127]
                        const char velocità = LeggiByte(file); // [0, 127]
                        std::wcout << L"Nota off (" << canale << L"): " << (int)nota << ", " << (int)velocità << L'\n';
                        break;
                    }
                    case 0x90: // nota on
                    {
                        const char nota     = dato1;           // [0, 127]
                        const char velocità = LeggiByte(file); // [0, 127]
                        std::wcout << L"Nota on (" << canale << L"): " << (int)nota << ", " << (int)velocità << L'\n';
                        break;
                    }
                    case 0xA0: // polyphonic key pressure (After touch): modifica la nota mentre suona
                    {
                        const char nota = dato1, pressione = LeggiByte(file);
                        std::wcout << L"Poly key pressure (" << canale << L"): " << (int)nota << ", " << (int)pressione
                                   << L'\n';
                        break;
                    }
                    case 0xB0: // control change: un altro controllo che non corrisponde alle note è stato
                               // azionato oppure c'è un cambio di modalità del canale
                    {
                        const char controllo = dato1, valore = LeggiByte(file);
                        std::wcout << L"Control change (" << canale << L"): " << (int)controllo << ", " << (int)valore;
                        if (controllo >= 120 && controllo <= 127) std::wcout << L" (Modalità canale)";
                        if (controllo == 120) std::wcout << L" (All sounds off)";
                        if (controllo == 123) std::wcout << L" (All notes off)";
                        std::wcout << L'\n';
                        break;
                    }
                    case 0xC0: // program change
                    {
                        const char programma = dato1;
                        std::wcout << L"Program change (" << canale << L"): " << (int)programma << L'\n';
                        break;
                    }
                    case 0xD0: // channel pressure (After touch)
                    {
                        const char pressione = dato1;
                        std::wcout << L"Channel pressure (" << canale << L"): " << (int)pressione << L'\n';
                        break;
                    }
                    case 0xE0: // pitch bend: modifica del pitch, massimo 14 bit (2 byte di dati)
                    {
                        const char lsb = dato1, msb = LeggiByte(file);
                        std::wcout << L"Pitch bend (" << canale << L"): " << ((int)msb << 8 | (int)lsb) << L'('
                                   << (int)lsb << L", " << (int)msb << L")\n";
                        break;
                    }
                    default:
                        assert(false && "Stato non riconosciuto");
                        break;
                }
            }

            posizioneNelFile = file.tellg();
        }
        while (!fineTraccia || posizioneNelFile < posFineTraccia);

        if (posizioneNelFile > posFineTraccia)
        {
            std::wcout << L"Errore: letti più byte della dimensione della traccia. Letti " << posizioneNelFile
                       << " byte invece di " << posFineTraccia << L" byte.\n";
        }

        if (!fineTraccia && posizioneNelFile >= posFineTraccia)
        {
            std::wcout << L"Errore: manca l'evento di fine traccia.\n";
        }

        if (fineTraccia && posizioneNelFile < posFineTraccia)
        {
            // La traccia è finita prima della lunghezza specificata nell'intestazione.
            // Salto ed ignoro tutti i byte rimanenti.
            file.seekg(posFineTraccia, std::ios_base::beg);
        }

        // ----- -----

        file.close();
        eliminatori.Pulisci();

        return true;
    }

    static inline unsigned int Endianess(unsigned int valore)
    {
        return (valore & 0x00'00'00'FF) << 24 | (valore & 0x00'00'FF'00) << 8 | (valore & 0xFF'00'00) >> 8
             | (valore & 0xFF'00'00'00) >> 24;
    }

    static inline unsigned short Endianess(unsigned short valore)
    {
        return (valore & 0x00FF) << 8 | (valore & 0xFF00) >> 8;
    }

    static unsigned char LeggiByte(std::ifstream &file)
    {
        unsigned char valore;
        file.read(reinterpret_cast<char *>(&valore), 1);
        return valore;
    }

    static unsigned short LeggiInt16(std::ifstream &file)
    {
        unsigned short valore;
        file.read(reinterpret_cast<char *>(&valore), 2);
        valore = Endianess(valore);
        return valore;
    }

    static unsigned int LeggiInt24(std::ifstream &file)
    {
        unsigned int valore = 0;
        file.read(reinterpret_cast<char *>(&valore), 3);
        valore <<= 8;
        valore   = Endianess(valore);
        return valore;
    }

    static unsigned int LeggiInt32(std::ifstream &file)
    {
        unsigned int valore;
        file.read(reinterpret_cast<char *>(&valore), 4);
        valore = Endianess(valore);
        return valore;
    }

    static std::string LeggiStringa(std::ifstream &file, size_t lunghezza)
    {
        std::string valore(lunghezza + 1, '\0');
        file.read(valore.data(), lunghezza);
        return valore;
    }

    static unsigned int LeggiInteroALunghezzaVariabile(std::ifstream &file)
    {
        unsigned int valore = 0;
        char byte;
        do
        {
            valore <<= 7;
            file.read(&byte, 1);
            valore |= byte & 0x7F;
        }
        while (byte & 0x80);

        return valore;
    }
}
