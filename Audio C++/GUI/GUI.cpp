#include "IntestazionePrecompilata.h"

#include "../Sintetizzatore/Audio.h"
#include "CostantiEdAltro.h"
#include "Disegnatore.h"
#include "Estensioni ImGui/Estensioni.h"
#include "Impostazioni.h"
#include "Internazionalizzazione.h"

// ----- -----

namespace GUI
{
    static bool finestraDemoAperta          = false;
    static bool finestraImpostazioniAperta  = false;
    static bool finestraEditorStileAperta   = false;
    static bool finestraMetricheImGuiAperta = false;
    static bool finestraDebugLogAperta      = false;
    static bool finestraDemoPlotAperta      = false;
    static int ScalaGUIPercentuale          = 100; // [%]

    static bool LinguaSelezionabile(size_t i);
    static void RegolazioneVolume();
    static void TastieraPianoforte(bool *èAperta);
}

// ----- -----

void GUI::InizializzaGUI()
{
    ScalaGUIPercentuale = static_cast<int>(Impostazioni.scalaGUI * 100); // [%]

    ImGui::GetPlatformIO().Platform_LocaleDecimalPoint = '.';
}

void GUI::GUI()
{
    // Possibilità di fare il docking di una finestra sui bordi dello schermo e non solo su un'altra finestra.
    ImGui::DockSpaceOverViewport();

    // -----

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::MenuItem(TestiGUI.menuEsci.data())) Esci = true;
        if (ImGui::MenuItem(TestiGUI.impostazioni.data())) finestraImpostazioniAperta = true;
        if (ImGui::BeginMenu(TestiGUI.menuDebug.data()))
        {
            ImGui::MenuItem(TestiGUI.demo.data(), nullptr, &finestraDemoAperta);
            ImGui::MenuItem(TestiGUI.demoPlot.data(), nullptr, &finestraDemoPlotAperta);
            ImGui::MenuItem(TestiGUI.editorStile.data(), nullptr, &finestraEditorStileAperta);
            ImGui::MenuItem(TestiGUI.metricheImGui.data(), nullptr, &finestraMetricheImGuiAperta);
            ImGui::MenuItem(TestiGUI.debugLog.data(), nullptr, &finestraDebugLogAperta);

            ImGui::EndMenu();
        }

        {
            char testoStatistiche[512];
            const int lunghezzaTesto = ImFormatString(
                testoStatistiche,
                std::size(testoStatistiche),
                "%.3f ms (%*.3f FPS)",
                DurataFotogramma,
                Costanti::AbilitàVSync ? 6 : 8,
                1000.0 / DurataFotogramma);

            // Allineamento a destra del testo
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX()
                + ImMax(
                    0.0f,
                    ImGui::GetContentRegionAvail().x
                        - ImGui::CalcTextSize(testoStatistiche, testoStatistiche + lunghezzaTesto).x));

            ImGui::TextUnformatted(testoStatistiche, testoStatistiche + lunghezzaTesto);
        }

        ImGui::EndMainMenuBar();
    }

    if (ImGui::BeginMainStatusBar())
    {
        if (ImGui::SmallButton(TestiGUI.demo.data())) finestraDemoAperta = !finestraDemoAperta;
        if (ImGui::SmallButton(TestiGUI.demoPlot.data())) finestraDemoPlotAperta = !finestraDemoPlotAperta;
        if (ImGui::SmallButton(TestiGUI.editorStile.data())) finestraEditorStileAperta = !finestraEditorStileAperta;
        if (ImGui::SmallButton(TestiGUI.metricheImGui.data()))
            finestraMetricheImGuiAperta = !finestraMetricheImGuiAperta;
        if (ImGui::SmallButton(TestiGUI.debugLog.data())) finestraDebugLogAperta = !finestraDebugLogAperta;

        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        // Icona sulla barra e finestra popup.
        RegolazioneVolume();

        ImGui::EndMainStatusBar();
    }

    // -----

    if (finestraDemoAperta) ImGui::ShowDemoWindow(&finestraDemoAperta);
    if (finestraMetricheImGuiAperta) ImGui::ShowMetricsWindow(&finestraMetricheImGuiAperta);
    if (finestraDebugLogAperta) ImGui::ShowDebugLogWindow(&finestraDebugLogAperta);

    if (finestraEditorStileAperta)
    {
        if (ImGui::Begin(TestiGUI.editorStile.data(), &finestraEditorStileAperta))
            ImGui::ShowStyleEditor(&Temi[Impostazioni.temaSelezionato].stile);
        ImGui::End();
    }

    if (finestraImpostazioniAperta)
    {
        bool impostazioniModificate = false;

        if (ImGui::Begin(TestiGUI.impostazioni.data(), &finestraImpostazioniAperta, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (ImGui::BeginCombo(TestiGUI.lingua.data(), Lingue[Impostazioni.linguaSelezionata].nome.data()))
            {
                if (LinguaSelezionabile(0)) impostazioniModificate = true;

                if (ImGui::BeginTable("", 2))
                {
                    for (size_t i = 1; i < Lingue.size(); ++i)
                    {
                        ImGui::TableNextColumn();
                        ImGui::TextAligned(0.5f, -std::numeric_limits<float>::min(), Lingue[i].bandiera);
                        ImGui::TableNextColumn();

                        if (LinguaSelezionabile(i)) impostazioniModificate = true;
                    }

                    ImGui::EndTable();
                }

                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo(TestiGUI.tema.data(), Temi[Impostazioni.temaSelezionato].nome.data()))
            {
                for (size_t i = 0; i < Temi.size(); ++i)
                    if (ImGui::Selectable(
                            Temi[i].nome.data(), i == Impostazioni.temaSelezionato, ImGuiSelectableFlags_SelectOnNav)
                        && i != Impostazioni.temaSelezionato)
                    {
                        Impostazioni.temaSelezionato = i;
                        ImGui::GetStyle()            = Temi[i].stile;
                        impostazioniModificate       = true;
                    }
                    else if (i == Impostazioni.temaSelezionato) ImGui::SetItemDefaultFocus();

                ImGui::EndCombo();
            }

            {
                constexpr int incremento = 10;
                ImGui::InputScalar(
                    TestiGUI.zoomIU.data(), ImGuiDataType_S32, &ScalaGUIPercentuale, &incremento, nullptr, "%d %%");
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    Impostazioni.scalaGUI  = ScalaGUIPercentuale / 100.0f;
                    Impostazioni.scalaGUI  = std::max(Impostazioni.scalaGUI, 0.3f);
                    Impostazioni.scalaGUI  = std::min(Impostazioni.scalaGUI, 2.0f);
                    ScalaGUIPercentuale    = static_cast<int>(Impostazioni.scalaGUI * 100.0f);
                    impostazioniModificate = true;
                    AggiornaScalaGUI();
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button(TestiGUI.default_.data()))
            {
                Impostazioni.linguaSelezionata = 0;
                ImpostaLingua(0);

                Impostazioni.temaSelezionato = 0;

                if (Impostazioni.scalaGUI != 1.0f)
                {
                    Impostazioni.scalaGUI = 1.0f;
                    ScalaGUIPercentuale   = 100;
                    AggiornaScalaGUI();
                }
                // AggiornaScalaGUI() provvede già a re-impostare il tema.
                else ImGui::GetStyle() = Temi[0].stile;

                impostazioniModificate = true;
            }

            ImGui::SameLine();

            if (ImGui::Button(TestiGUI.annulla.data())) finestraImpostazioniAperta = false;
        }
        ImGui::End();

        if (impostazioniModificate) SalvaImpostazioni();
    }

    if (finestraDemoPlotAperta) ImPlot::ShowDemoWindow(&finestraDemoPlotAperta);

    TastieraPianoforte(nullptr);
}

void GUI::AggiornaScalaGUI()
{
    InizializzaTemi();
    ImGui::GetStyle() = Temi[Impostazioni.temaSelezionato].stile;
}

// ----- -----

// Restituisce True se viene cambiata la lingua.
static bool GUI::LinguaSelezionabile(const size_t i)
{
    if (ImGui::Selectable(
            Lingue[i].nome.data(),
            i == Impostazioni.linguaSelezionata,
            ImGuiSelectableFlags_SelectOnNav | ImGuiSelectableFlags_SpanAllColumns)
        && i != Impostazioni.linguaSelezionata)
    {
        Impostazioni.linguaSelezionata = i;
        ImpostaLingua(i);
        return true;
    }
    else if (i == Impostazioni.linguaSelezionata) ImGui::SetItemDefaultFocus();

    return false;
}

static void GUI::RegolazioneVolume()
{
    const char *iconaVolume = ICON_FA_VOLUME_HIGH;
    if (Impostazioni.volumeGeneraleMuto) iconaVolume = ICON_FA_VOLUME_XMARK;
    else if (Impostazioni.volumeGenerale == 0.0) iconaVolume = ICON_FA_VOLUME_OFF;
    else if (Impostazioni.volumeGenerale > 2.0 / 3.0) iconaVolume = ICON_FA_VOLUME_HIGH;
    else if (Impostazioni.volumeGenerale > 1.0 / 3.0) iconaVolume = ICON_FA_VOLUME;
    else if (Impostazioni.volumeGenerale < 1.0 / 3.0) iconaVolume = ICON_FA_VOLUME_LOW;

    const ImVec2 dimensioneSimbolo = ImGui::CalcTextSize(ICON_FA_VOLUME_HIGH);
    const float posizioneXSimbolo =
        ImGui::GetCursorPosX() + ImMax(0.0f, ImGui::GetContentRegionAvail().x - dimensioneSimbolo.x - 6);
    ImGui::SetCursorPosX(posizioneXSimbolo);

    ImGui::TextUnformatted(iconaVolume);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        ImGui::OpenPopup("RegolazioneVolume");

        const float spaziaturaEsternaPopup = 5 * Impostazioni.scalaGUI;
        const ImVec2 posizioneFinestra     = ImGui::GetCurrentWindow()->Pos;
        const ImVec2 posizionePopup(
            posizioneXSimbolo + dimensioneSimbolo.x / 2, posizioneFinestra.y - spaziaturaEsternaPopup);
        ImGui::SetNextWindowPos(posizionePopup, ImGuiCond_None, ImVec2(0.5, 1));
    }

    if (ImGui::BeginPopup("RegolazioneVolume"))
    {
        ImGui::TextUnformatted("100");

        {
            // Nota: le dimensioni nello stile sono già scalate tramite lo zoom della GUI.
            const ImVec2 dimensioneSlider(
                ImGui::GetStyle().GrabMinSize + ImGui::GetStyle().FramePadding.x, 100 * Impostazioni.scalaGUI);

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - dimensioneSlider.x) / 2);
            ImGui::VSliderFloat(
                "##Volume", dimensioneSlider, &Impostazioni.volumeGenerale, 0.0f, 1.0f, "", ImGuiSliderFlags_NoInput);
        }

        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("0").x) / 2.0f);
        ImGui::TextUnformatted("0");

        // Spazio vuoto
        ImGui::Dummy(ImVec2(0, 3));

        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(iconaVolume).x) / 2.0f);
        ImGui::TextUnformatted(iconaVolume);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            Impostazioni.volumeGeneraleMuto = !Impostazioni.volumeGeneraleMuto;

        ImGui::EndPopup();
    }
}

static bool has_black(const int key)
{
    // int k  = key - 1;
    // int r  = k % 7;
    // bool a = k % 7 == 0;
    // bool b = k % 7 == 3;

    // return !(a || b) && key != 51;

    const int k    = key;
    const int nota = k % 12;

    return nota == 1 || nota == 3 || nota == 6 || nota == 8 || nota == 10;
}

static void GUI::TastieraPianoforte(bool *èAperta)
{
    constexpr ImU32 Black    = IM_COL32(0, 0, 0, 255);
    constexpr ImU32 White    = IM_COL32(255, 255, 255, 255);
    constexpr ImU32 Grey     = IM_COL32(200, 200, 200, 255);
    constexpr ImU32 GreyDark = IM_COL32(100, 100, 100, 255);
    constexpr float scala    = 1.0f;
    constexpr float width    = 20 * scala;
    constexpr float height   = 120 * scala;
    constexpr int numTasti   = 52; // Tasti bianchi.

    ImGui::SetNextWindowContentSize(ImVec2(numTasti * width, height));
    if (ImGui::Begin("Keyboard", èAperta, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImRect bb;
        bool pressed = false;
        bool hovered = false;
        bool held;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        const ImVec2 p        = ImGui::GetCursorScreenPos();
        int cur_key;           // Numero nota MIDI.
        int tastoPremuto = -1; // I numeri delle note identificano i tasti.
        int tastoHovered = -1;

        // ----- Interazione coi tasti (per esempio premere un tasto)

        cur_key = 22;

        for (int key = 0; key < numTasti; key++)
        {
            if (has_black(cur_key))
            {
                bb.Min = ImVec2(p.x + key * width + width * 3 / 4, p.y);
                bb.Max = ImVec2(p.x + key * width + width * 5 / 4 + 1, p.y + height * 2 / 3);

                const ImGuiID id = ImGui::GetID(cur_key);
                ImGui::ItemSize(bb);
                if (ImGui::ItemAdd(bb, id)) pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

                if (pressed || held) tastoPremuto = cur_key;
                if (hovered) tastoHovered = cur_key;

                cur_key += 2;
            }
            else cur_key++;
        }

        cur_key = 21;
        pressed = false;
        hovered = false;

        for (int key = 0; key < numTasti; key++)
        {
            bb.Min = ImVec2(p.x + key * width, p.y);
            bb.Max = ImVec2(p.x + key * width + width, p.y + height);

            const ImGuiID id = ImGui::GetID(cur_key);
            ImGui::ItemSize(bb);
            if (ImGui::ItemAdd(bb, id)) pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

            if (pressed || held) tastoPremuto = cur_key;
            if (hovered) tastoHovered = cur_key;

            cur_key++;

            if (has_black(cur_key)) cur_key++;
        }

        // ----- Disegno

        cur_key = 21;

        // Disegno i tasti bianchi.
        for (int key = 0; key < numTasti; key++)
        {
            bb.Min = ImVec2(p.x + key * width, p.y);
            bb.Max = ImVec2(p.x + key * width + width, p.y + height);

            ImU32 colUp   = White;
            ImU32 colDown = White;
            if (tastoPremuto == cur_key)
            {
                colUp   = Grey;
                colDown = GreyDark;
            }
            else if (tastoHovered == cur_key) colUp = colDown = Grey;

            draw_list->AddRectFilledMultiColor(bb.Min, bb.Max, colUp, colUp, colDown, colDown);
            draw_list->AddRect(bb.Min, bb.Max, Black);

            cur_key++;

            if (has_black(cur_key)) cur_key++;
        }

        cur_key = 22;

        // Disegno i tasti neri.
        for (int key = 0; key < numTasti; key++)
        {
            if (has_black(cur_key))
            {
                bb.Min = ImVec2(p.x + key * width + width * 3 / 4, p.y);
                bb.Max = ImVec2(p.x + key * width + width * 5 / 4 + 1, p.y + height * 2 / 3);

                ImU32 colUp   = Black;
                ImU32 colDown = Black;
                if (tastoPremuto == cur_key)
                {
                    colUp   = Grey;
                    colDown = GreyDark;
                }
                else if (tastoHovered == cur_key) colUp = colDown = Grey;

                // Tasto di un semi-tono
                draw_list->AddRectFilledMultiColor(bb.Min, bb.Max, colUp, colUp, colDown, colDown);

                if (colUp != Black || colDown != Black)
                    // Bordo nero intorno al tasto
                    draw_list->AddRect(bb.Min, bb.Max, Black);

                cur_key += 2;
            }
            else cur_key++;
        }

        // -----

        static int precTastoPremuto = -1;

        if (precTastoPremuto != tastoPremuto)
        {
            using namespace Sintetizzatore;

            auto *pianoforte = static_cast<StrumentiMusicali::Pianoforte *>(strumentoMusicale.load());

            if (precTastoPremuto != -1) pianoforte->FineNota(precTastoPremuto);
            if (tastoPremuto != -1) pianoforte->InizioNota(tastoPremuto);

            precTastoPremuto = tastoPremuto;
        }
    }

    ImGui::End();
}
