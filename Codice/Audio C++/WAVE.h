#pragma once

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

        inline DataFormatChunk(
            unsigned short _numberoCanali,
            unsigned int _frequenza,
            unsigned short _bitPerCampione,
            TipoCampioni tipoCampioni = TipoCampioni::Interi);

        inline void
        Set(unsigned short _numberoCanali,
            unsigned int _frequenza,
            unsigned short _bitPerCampione,
            TipoCampioni tipoCampioni = TipoCampioni::Interi);

        operator WAVEFORMATEX();
    };

    DataFormatChunk ToDataFormatChunk(const WAVEFORMATEX &dati);

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
}
