#include "IntestazionePrecompilata.h"

#include "CodaEliminazione.h"

CodaEliminazione ::~CodaEliminazione()
{
    Pulisci();
}

void CodaEliminazione ::Aggiungi(const Eliminatore &eliminatore)
{
    coda.emplace(eliminatore);
}

void CodaEliminazione ::Pulisci()
{
    while (!coda.empty())
    {
        Eliminatore eliminatore = coda.top();
        eliminatore();
        coda.pop();
    }
}

void CodaEliminazione::Svuota()
{
    while (!coda.empty())
        coda.pop();
}
