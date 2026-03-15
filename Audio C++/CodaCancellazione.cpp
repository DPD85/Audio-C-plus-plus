#include "CodaCancellazione.h"

CodaCancellazione::~CodaCancellazione()
{
    Ripulisci();
}

void CodaCancellazione::Aggiungi(Pulitore proc)
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
