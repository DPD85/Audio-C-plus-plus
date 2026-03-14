#pragma once

#include <functional>
#include <stack>

class CodaCancellazione
{
  public:
    using Pulitore = std::function<void()>;

    ~CodaCancellazione();

    void Aggiungi(Pulitore proc);
    void Ripulisci();
    void Svuota();

  private:
    std::stack<Pulitore> coda;
};
