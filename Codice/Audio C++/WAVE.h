#pragma once

#include "CostantiEdAltro.h"

namespace WAVE
{
#pragma pack(push, 1)

    enum class TipoCampioni
    {
        Interi,
        Float
    };

    struct DataFormatChunk
    {
        unsigned int firma      = MAKEFOURCC('f', 'm', 't', ' ');
        unsigned int dimensione = sizeof(DataFormatChunk) - 8;
        // Formato dei campioni (compressi, non compressi, ecc...)
        unsigned short formatTag;
        unsigned short numberoCanali;
        // Frequenza di campionamento
        unsigned int frequenza;
        unsigned int bytePerSecondo;
        // Un blocco è un insieme contente i campioni per ogni canale corrispondenti ad un istante temporale
        unsigned short bytePerBlocco;
        unsigned short bitsPerCampione;

        DataFormatChunk();

        DataFormatChunk(
            unsigned short numberoCanali_,
            unsigned int frequenza_,
            unsigned short bitPerCampione_,
            TipoCampioni tipoCampioni = TipoCampioni::Interi);

        void Set(
            unsigned short numberoCanali_,
            unsigned int frequenza_,
            unsigned short bitPerCampione_,
            TipoCampioni tipoCampioni = TipoCampioni::Interi);
    };

    std::wostream &operator<<(std::wostream &stream, const DataFormatChunk &data);

    struct SampledDataChunk
    {
        unsigned int firma = MAKEFOURCC('d', 'a', 't', 'a');
        unsigned int dimensione;
    };

    struct MasterRIFFChunk
    {
        unsigned int firma = MAKEFOURCC('R', 'I', 'F', 'F');
        unsigned int fileSize;
        unsigned int firmaFormato = MAKEFOURCC('W', 'A', 'V', 'E');

        MasterRIFFChunk();

        inline MasterRIFFChunk(unsigned int dimensioneDati);

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
