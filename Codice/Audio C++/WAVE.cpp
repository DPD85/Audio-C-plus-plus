#include "WAVE.h"

#include "CodaEliminazione.h"
#include "Oscillatori.h"

namespace WAVE
{
    // ----- DataFormatChunk -----

    // Formato dei campioni dentro il file WAVE (interi, float, compressi, non compressi, ecc...)
    enum WAVEFormat
    {
        PCM        = 1, // I campioni sono numeri interi
        IEEE_FLOAT = 3, // I campioni sono numeri float
    };

    DataFormatChunk::DataFormatChunk() = default;

    DataFormatChunk::DataFormatChunk(
        unsigned short numberoCanali_,
        unsigned int frequenza_,
        unsigned short bitPerCampione_,
        TipoCampioni tipoCampioni)
    {
        Set(numberoCanali_, frequenza_, bitPerCampione_, tipoCampioni);
    }

    void DataFormatChunk::Set(
        unsigned short numberoCanali_,
        unsigned int frequenza_,
        unsigned short bitPerCampione_,
        TipoCampioni tipoCampioni)
    {
        if (tipoCampioni == TipoCampioni::Interi) formatTag = WAVEFormat::PCM;
        else formatTag = WAVEFormat::IEEE_FLOAT;

        numberoCanali   = numberoCanali_;
        frequenza       = frequenza_;
        bitsPerCampione = bitPerCampione_;
        bytePerBlocco   = (numberoCanali_ * bitPerCampione_) / 8u;
        bytePerSecondo  = frequenza_ * bytePerBlocco;
    }

    std::wostream &operator<<(std::wostream &stream, const DataFormatChunk &data)
    {
        // clang-format off
         return stream
             << L"  numero canali    : " << data.numberoCanali   << L'\n'
             << L"  frequenza        : " << data.frequenza       << " Hz\n"
             << L"  bits per campione: " << data.bitsPerCampione << L'\n'
             << L"  tipo campione    : "
                << (data.formatTag == WAVEFormat::PCM ? L"Integer": L"Float")
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

        DataFormatChunk df(1, static_cast<unsigned int>(Costanti::FrequenzaCampionamento), 8);

        SampledDataChunk dc;
        dc.dimensione = df.bytePerSecondo * timeLen;

        MasterRIFFChunk mc;
        mc.SetFileSize(dc.dimensione);

        const size_t numeroCampioni = DaSecondiACampioni(timeLen);

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

        Oscillatori::OndaSinusoidale sin480(480);
        Oscillatori::OndaSinusoidale sin320(320);

        for (size_t i = 1; i < numeroCampioni; ++i)
        {
            if (i == 1ull * df.frequenza) sin480.Frequenza(600);
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
        const constexpr unsigned int timeLen = 30;

        DataFormatChunk df(2, static_cast<unsigned int>(Costanti::FrequenzaCampionamento), 8);

        SampledDataChunk dc;
        dc.dimensione = df.bytePerSecondo * timeLen;

        MasterRIFFChunk mc;
        mc.SetFileSize(dc.dimensione);

        const size_t numeroCampioni = DaSecondiACampioni(timeLen);

        // Solo 1° canale (sinistro)
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

            Oscillatori::OndaSinusoidale sin480(480);
            Oscillatori::OndaSinusoidale sin320(320);

            for (size_t i = 0; i < numeroCampioni; ++i)
            {
                // 1° canale
                double v = 0;

                v += 0.8 * sin480.Campione();
                v += 0.2 * sin320.Campione();

                assert(v >= -1 && v <= 1);

                v *= 0.25;

                file << ConvertiA8Bits(v);

                // 2° canale - silenzio
                file << Costanti::Silenzio8Bit;
            }
        }

        // Solo 2° canale (destro)
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

            Oscillatori::OndaSinusoidale sin480(480);
            Oscillatori::OndaSinusoidale sin320(320);

            for (size_t i = 0; i < numeroCampioni; ++i)
            {
                // 1° canale - silenzio
                file << Costanti::Silenzio8Bit;

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

    void SalvaFile(const std::string &percorso, const dvector &campioni)
    {
        DataFormatChunk dt(1, static_cast<unsigned int>(Costanti::FrequenzaCampionamento), 8);

        SampledDataChunk dc;
        dc.dimensione = static_cast<unsigned int>(campioni.size() * sizeof(unsigned char));

        MasterRIFFChunk mc;
        mc.SetFileSize(dc.dimensione);

        CodaEliminazione eliminatori;

        std::ofstream file(percorso, std::ios_base::binary | std::ios_base::trunc);
        if (!file.is_open())
        {
            return;
        }
        eliminatori.Aggiungi(
            [&file]()
            {
                file.close();
            });

        file.write(reinterpret_cast<const char *>(&mc), sizeof(MasterRIFFChunk));
        file.write(reinterpret_cast<const char *>(&dt), sizeof(DataFormatChunk));
        file.write(reinterpret_cast<const char *>(&dc), sizeof(SampledDataChunk));

        for (const double &campione : campioni)
            file << ConvertiA8Bits(campione);
    }
}
