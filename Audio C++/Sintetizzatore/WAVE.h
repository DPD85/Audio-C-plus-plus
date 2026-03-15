#pragma once

#include "CostantiEdAltro.h"

namespace Sintetizzatore::WAVE
{
#pragma pack(push, 1)

    constexpr unsigned int CostruisciFirma(const char a, const char b, const char c, const char d)
    {
        return static_cast<unsigned long>(a) | static_cast<unsigned long>(b) << 8 | static_cast<unsigned long>(c) << 16
             | static_cast<unsigned long>(d) << 24;
    }

    enum class TipoCampioni
    {
        Interi,
        Float
    };

    struct DataFormatChunk
    {
        unsigned int firma      = CostruisciFirma('f', 'm', 't', ' ');
        unsigned int dimensione = sizeof(DataFormatChunk) - 8;
        // Formato dei campioni (compressi, non compressi, ecc...)
        unsigned short formatTag;
        unsigned short numeroCanali;
        // Frequenza di campionamento
        unsigned int frequenza;
        unsigned int bytePerSecondo;
        // Un blocco è un insieme contente i campioni per ogni canale corrispondenti ad un istante temporale
        unsigned short bytePerBlocco;
        unsigned short bitsPerCampione;

        DataFormatChunk();

        DataFormatChunk(
            unsigned short numeroCanali_,
            unsigned int frequenza_,
            unsigned short bitPerCampione_,
            TipoCampioni tipoCampioni = TipoCampioni::Interi);

        void Set(
            unsigned short numeroCanali_,
            unsigned int frequenza_,
            unsigned short bitPerCampione_,
            TipoCampioni tipoCampioni = TipoCampioni::Interi);
    };

    std::wostream &operator<<(std::wostream &stream, const DataFormatChunk &data);

    struct SampledDataChunk
    {
        unsigned int firma = CostruisciFirma('d', 'a', 't', 'a');
        unsigned int dimensione;
    };

    struct MasterRIFFChunk
    {
        unsigned int firma = CostruisciFirma('R', 'I', 'F', 'F');
        unsigned int fileSize;
        unsigned int firmaFormato = CostruisciFirma('W', 'A', 'V', 'E');

        MasterRIFFChunk();

        explicit inline MasterRIFFChunk(unsigned int dimensioneDati);

        inline void SetFileSize(unsigned int dimensioneDati);
    };

#pragma pack(pop)

    void CreaFileSemplice();
    void CreaFileStereo();

    /// @brief Salva i campioni audio dentro un file WAVE mono a 8 bit.
    /// @param percorso Il percorso ed il nome del file WAVE.
    /// @param campioni Il vettore con i campioni audio da salvare.
    void SalvaFile(const std::string &percorso, const dvector &campioni);
}
