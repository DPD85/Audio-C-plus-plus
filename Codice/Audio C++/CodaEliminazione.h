#pragma once

#include <functional>
#include <stack>

class CodaEliminazione
{
  private:
    using Eliminatore = std::function<void()>;
    std::stack<Eliminatore> coda;

  public:
    ~CodaEliminazione();

    void Aggiungi(const Eliminatore &eliminatore);

    void Pulisci();
};
