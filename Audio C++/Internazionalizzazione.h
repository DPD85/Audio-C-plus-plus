#pragma once

struct Lingua
{
    std::string nome;
    const char *bandiera;
    const char *codice;
};

struct TestiGUI
{
#define TESTO_GUI(var, testo) std::string var;

#include "GUI/TestiGUI.h"

#undef TESTO_GUI
};

extern std::array<Lingua, 3> Lingue;
extern boost::locale::generator GeneratoreMultiLingua;
extern TestiGUI TestiGUI;

void InizializzaInternazionalizzazione();
void ImpostaLingua(size_t indice);
std::vector<char> CaricaCatalogoMessaggi(const std::string &nomeFile, const std::string &codifica);
