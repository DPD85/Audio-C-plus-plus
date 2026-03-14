#include "IntestazionePrecompilata.h"

#include "CodaCancellazione.h"

CodaCancellazione::~CodaCancellazione()
{
    Ripulisci();
}

void CodaCancellazione::Aggiungi(CodaCancellazione::Pulitore proc)
{
    coda.emplace(proc);
}

void CodaCancellazione::Ripulisci()
{
    while (!coda.empty())
    {
        Pulitore pulitore = coda.top();
        pulitore();
        coda.pop();
    }
}

void CodaCancellazione::Svuota()
{
    while (!coda.empty())
        coda.pop();
}
