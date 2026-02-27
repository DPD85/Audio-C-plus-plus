#pragma once

#include "StrumentiMusicali.h"

namespace Sintetizzatore
{
    extern std::atomic<StrumentoMusicale *> strumentoMusicale;

    bool InizializzaAudio();
    void TerminaAudio();
}
