#include "WAVE.h"

#include "CodaEliminazione.h"
#include "CostantiEdAltro.h"
#include "Oscillatori.h"

namespace WAVE
{
    // ----- DataFormatChunk -----

    DataFormatChunk::DataFormatChunk() = default;

    inline DataFormatChunk::DataFormatChunk(
        unsigned short _numberoCanali, unsigned int _frequenza, unsigned short _bitPerCampione)
    {
        Set(_numberoCanali, _frequenza, _bitPerCampione);
    }

    inline void
    DataFormatChunk::Set(unsigned short _numberoCanali, unsigned int _frequenza, unsigned short _bitPerCampione)
    {
        numberoCanali   = _numberoCanali;
        frequenza       = _frequenza;
        bitsPerCampione = _bitPerCampione;
        bytePerBlocco   = (_numberoCanali * _bitPerCampione) / 8;
        bytePerSecondo  = _frequenza * bytePerBlocco;
    }

    DataFormatChunk::operator WAVEFORMATEX()
    {
        return WAVEFORMATEX{ .wFormatTag      = formatTag,
                             .nChannels       = numberoCanali,
                             .nSamplesPerSec  = frequenza,
                             .nAvgBytesPerSec = bytePerSecondo,
                             .nBlockAlign     = bytePerBlocco,
                             .wBitsPerSample  = bitsPerCampione,
                             .cbSize          = 0 };
    }

    DataFormatChunk ToDataFormatChunk(const WAVEFORMATEX &dati)
    {
        DataFormatChunk df;

        df.formatTag       = dati.wFormatTag;
        df.numberoCanali   = dati.nChannels;
        df.frequenza       = dati.nSamplesPerSec;
        df.bytePerSecondo  = dati.nAvgBytesPerSec;
        df.bytePerBlocco   = dati.nBlockAlign;
        df.bitsPerCampione = dati.wBitsPerSample;

        return df;
    }

    std::wostream &operator<<(std::wostream &stream, const DataFormatChunk &data)
    {
        // clang-format off
         return stream
             << L"  numero canali    : " << data.numberoCanali   << L'\n'
             << L"  frequenza        : " << data.frequenza       << " Hz\n"
             << L"  bits per campione: " << data.bitsPerCampione << L'\n'
             << L"  tipo campione    : "
                << (data.formatTag == WAVE_FORMAT_PCM ? L"Integer": L"Float")
                << L'\n';
        // clang-format on
    }

    // ----- MasterRIFFChunk -----

    MasterRIFFChunk::MasterRIFFChunk() = default;

    inline MasterRIFFChunk::MasterRIFFChunk(unsigned int dimensioneDati)
    {
        SetFileSize(dimensioneDati);
    }

    inline void MasterRIFFChunk::SetFileSize(unsigned int dimensioneDati)
    {
        fileSize = sizeof(MasterRIFFChunk) + sizeof(DataFormatChunk) + sizeof(SampledDataChunk) + dimensioneDati - 8;
    }

    // ----- -----

    void CreaFileSemplice()
    {
        CodaEliminazione eliminatori;

        unsigned int timeLen = 30;

        DataFormatChunk df(1, 44100, 8);

        SampledDataChunk dc;
        dc.dimensione = df.bytePerSecondo * timeLen;

        MasterRIFFChunk mc;
        mc.SetFileSize(dc.dimensione);

        std::ofstream file(L"test.wav", std::ios_base::binary | std::ios_base::trunc, _SH_DENYWR);
        if (!file.is_open())
        {
            return;
        }
        eliminatori.Aggiungi(
            [&file]()
            {
                file.close();
            });

        file.write(reinterpret_cast<char *>(&mc), sizeof(MasterRIFFChunk));
        file.write(reinterpret_cast<char *>(&df), sizeof(DataFormatChunk));
        file.write(reinterpret_cast<char *>(&dc), sizeof(SampledDataChunk));

        Oscillatori::OndaSinusoidale sin480(480, df.frequenza);
        Oscillatori::OndaSinusoidale sin320(320, df.frequenza);

        for (size_t i = 1; i < df.frequenza * timeLen; ++i)
        {
            if (i == df.frequenza) sin480.Frequenza(600);
            if (i == 2ull * df.frequenza) sin480.Frequenza(480);
            if (i == 3ull * df.frequenza) sin480.Frequenza(600);
            if (i == 4ull * df.frequenza) sin480.Frequenza(480);

            double v = 0;

            v += 0.8 * sin480.Campione();
            v += 0.2 * sin320.Campione();

            assert(v >= -1 && v <= 1);

            v *= 0.25;

            file << ConvertiA8Bits(v);
        }
    }

    void CreaFileStereo()
    {
        unsigned int timeLen = 30;

        DataFormatChunk df(2, 44100, 8);

        SampledDataChunk dc;
        dc.dimensione = df.bytePerSecondo * timeLen;

        MasterRIFFChunk mc;
        mc.SetFileSize(dc.dimensione);

        // Solo canale 1° canale (sinistro)
        {
            CodaEliminazione eliminatori;

            std::ofstream file(L"testStereoSinistra.wav", std::ios_base::binary | std::ios_base::trunc, _SH_DENYWR);
            if (!file.is_open())
            {
                return;
            }
            eliminatori.Aggiungi(
                [&file]()
                {
                    file.close();
                });

            file.write(reinterpret_cast<char *>(&mc), sizeof(MasterRIFFChunk));
            file.write(reinterpret_cast<char *>(&df), sizeof(DataFormatChunk));
            file.write(reinterpret_cast<char *>(&dc), sizeof(SampledDataChunk));

            Oscillatori::OndaSinusoidale sin480(480, df.frequenza);
            Oscillatori::OndaSinusoidale sin320(320, df.frequenza);

            for (size_t i = 0; i < df.frequenza * timeLen; ++i)
            {
                // 1° canale
                double v = 0;

                v += 0.8 * sin480.Campione();
                v += 0.2 * sin320.Campione();

                assert(v >= -1 && v <= 1);

                v *= 0.25;

                file << ConvertiA8Bits(v);

                // 2° canale - silenzio
                file << static_cast<unsigned char>(Costanti::Silenzio8Bit);
            }
        }

        // Solo canale 2° canale (destro)
        {
            CodaEliminazione eliminatori;

            std::ofstream file(L"testStereoDestra.wav", std::ios_base::binary | std::ios_base::trunc, _SH_DENYWR);
            if (!file.is_open())
            {
                return;
            }
            eliminatori.Aggiungi(
                [&file]()
                {
                    file.close();
                });

            file.write(reinterpret_cast<char *>(&mc), sizeof(MasterRIFFChunk));
            file.write(reinterpret_cast<char *>(&df), sizeof(DataFormatChunk));
            file.write(reinterpret_cast<char *>(&dc), sizeof(SampledDataChunk));

            Oscillatori::OndaSinusoidale sin480(480, df.frequenza);
            Oscillatori::OndaSinusoidale sin320(320, df.frequenza);

            for (size_t i = 0; i < df.frequenza * timeLen; ++i)
            {
                // 1° canale - silenzio
                file << static_cast<unsigned char>(Costanti::Silenzio8Bit);

                // 2° canale
                double v = 0;

                v += 0.8 * sin480.Campione();
                v += 0.2 * sin320.Campione();

                assert(v >= -1 && v <= 1);

                v *= 0.25;

                file << ConvertiA8Bits(v);
            }
        }
    }
} // namespace WAVE
