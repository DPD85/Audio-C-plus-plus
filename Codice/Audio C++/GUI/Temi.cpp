#include "IntestazionePrecompilata.h"

#include "Disegnatore.h"
#include "Impostazioni.h"

namespace GUI
{
    using namespace boost::locale;

    std::array<Tema, 5> Temi;

    static const constexpr float DimensioneDefaultFont = 15.0f;

    static ImGuiStyle TemaScuro();
    static ImGuiStyle TemaChiaro();
    static ImGuiStyle TemaClassico();
    static ImGuiStyle TemaArancioneScuro();
    static ImGuiStyle TemaArancioneChiaro();

    static void ModificaColore(float tonalitàAccento, float saturazioneAccento, ImVec4 &colore);
    static void ModificaColore(float tonalitàAccento, ImVec4 &colore);

    // ----- -----

    void InizializzaTemi()
    {
        size_t indice = 0;

        Temi[indice++].stile = TemaArancioneScuro();
        Temi[indice++].stile = TemaArancioneChiaro();
        Temi[indice++].stile = TemaScuro();
        Temi[indice++].stile = TemaChiaro();
        Temi[indice++].stile = TemaClassico();

        AggiornaNomiTemi();

        const float scala = ScalaDPI * Impostazioni.scalaGUI;

        for (Tema &tema : Temi)
        {
            tema.stile.ScaleAllSizes(scala);
            tema.stile.FontScaleDpi = scala;
        }
    }

    void AggiornaNomiTemi()
    {
        size_t indice = 0;

        Temi[indice++].nome = translate("Temi", "Arancione scuro");
        Temi[indice++].nome = translate("Temi", "Arancione chiaro");
        Temi[indice++].nome = translate("Temi", "Scuro");
        Temi[indice++].nome = translate("Temi", "Chiaro");
        Temi[indice++].nome = translate("Temi", "Classico ImGui");
    }

    // ----- -----

    static ImGuiStyle TemaScuro()
    {
        ImGuiStyle stile;

        ImGui::StyleColorsDark(&stile);

        stile.FontSizeBase = DimensioneDefaultFont;

        stile.FrameRounding  = 7.0f;
        stile.GrabRounding   = 7.0f;
        stile.TabRounding    = 7.0f;
        stile.WindowRounding = 7.0f;

        return stile;
    }

    static ImGuiStyle TemaChiaro()
    {
        ImGuiStyle stile;

        ImGui::StyleColorsLight(&stile);

        stile.FontSizeBase = DimensioneDefaultFont;

        stile.FrameRounding  = 7.0f;
        stile.GrabRounding   = 7.0f;
        stile.TabRounding    = 7.0f;
        stile.WindowRounding = 7.0f;

        stile.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.784f, 0.784f, 0.784f, 1.0f);

        return stile;
    }

    static ImGuiStyle TemaClassico()
    {
        ImGuiStyle stile;

        ImGui::StyleColorsClassic(&stile);

        stile.FontSizeBase = DimensioneDefaultFont;

        stile.FrameRounding  = 7.0f;
        stile.GrabRounding   = 7.0f;
        stile.TabRounding    = 7.0f;
        stile.WindowRounding = 7.0f;

        return stile;
    }

    static ImGuiStyle TemaArancioneScuro()
    {
        ImGuiStyle stile;

        ImGui::StyleColorsDark(&stile);

        stile.FontSizeBase = DimensioneDefaultFont;

        stile.FrameRounding  = 7.0f;
        stile.GrabRounding   = 7.0f;
        stile.TabRounding    = 7.0f;
        stile.WindowRounding = 7.0f;

        // -----

        ImVec4 *colori = stile.Colors;

        static const constexpr ImVec4 coloreArancione(1.0f, 0.5f, 0.0f, 1.0f);

        float tonalitàArancione, saturazioneArancione, spare;
        ImGui::ColorConvertRGBtoHSV(
            coloreArancione.x, coloreArancione.y, coloreArancione.z, tonalitàArancione, saturazioneArancione, spare);

        colori[ImGuiCol_DockingEmptyBg] = coloreArancione;

        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_FrameBg]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_FrameBgHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_FrameBgActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TitleBgActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_CheckMark]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_SliderGrab]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_SliderGrabActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_Button]);
        ModificaColore(tonalitàArancione, colori[ImGuiCol_ButtonHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_ButtonActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_Header]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_HeaderHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_HeaderActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_Separator]); // !
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_SeparatorHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_SeparatorActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_ResizeGrip]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_ResizeGripHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_ResizeGripActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TabHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_Tab]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TabSelected]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TabSelectedOverline]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TabDimmedSelected]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_PlotLines]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_PlotLinesHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_PlotHistogram]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_PlotHistogramHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TextLink]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TextSelectedBg]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_DragDropTarget]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_DragDropTargetBg]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_NavCursor]);

        return stile;
    }

    static ImGuiStyle TemaArancioneChiaro()
    {
        ImGuiStyle stile;

        ImGui::StyleColorsLight(&stile);

        stile.FontSizeBase = DimensioneDefaultFont;

        stile.FrameRounding  = 7.0f;
        stile.GrabRounding   = 7.0f;
        stile.TabRounding    = 7.0f;
        stile.WindowRounding = 7.0f;

        // -----

        ImVec4 *colori = stile.Colors;

        static const constexpr ImVec4 coloreArancione(1.0f, 0.62353f, 0.255f, 1.0f);

        float tonalitàArancione, saturazioneArancione, spare;
        ImGui::ColorConvertRGBtoHSV(
            coloreArancione.x, coloreArancione.y, coloreArancione.z, tonalitàArancione, saturazioneArancione, spare);

        colori[ImGuiCol_DockingEmptyBg]    = coloreArancione;
        colori[ImGuiCol_DockingPreview].w *= 2.0f;

        colori[ImGuiCol_PopupBg] = colori[ImGuiCol_WindowBg];

        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_FrameBgHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_FrameBgActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TitleBgActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_CheckMark]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_SliderGrab]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_SliderGrabActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_Button]);
        ModificaColore(tonalitàArancione, colori[ImGuiCol_ButtonHovered]);
        ModificaColore(tonalitàArancione, colori[ImGuiCol_ButtonActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_Header]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_HeaderHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_HeaderActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_Separator]); // !
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_SeparatorHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_SeparatorActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_ResizeGrip]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_ResizeGripHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_ResizeGripActive]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TabHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_Tab]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TabSelected]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TabSelectedOverline]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TabDimmedSelected]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_PlotLines]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_PlotLinesHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_PlotHistogram]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_PlotHistogramHovered]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TextLink]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_TextSelectedBg]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_DragDropTarget]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_DragDropTargetBg]);
        ModificaColore(tonalitàArancione, saturazioneArancione, colori[ImGuiCol_NavCursor]);

        return stile;
    }

    // ----- -----

    /// @brief Sostituisce la tonalità e la saturazione del colore specificato lasciando invariata la luminosità e la
    ///        trasparenza.
    /// @param tonalitàAccento Tonalità con cui modificare il colore specificato.
    /// @param saturazioneAccento Saturazione con cui modificare il colore specificato.
    /// @param colore Il colore da modificare.
    static void ModificaColore(float tonalitàAccento, float saturazioneAccento, ImVec4 &colore)
    {
        float spare1, saturazione, luminosità;

        // Estraggo la luminosità del colore da modificare. Ignoro le sue tonalità e saturazione.
        ImGui::ColorConvertRGBtoHSV(colore.x, colore.y, colore.z, spare1, saturazione, luminosità);

        // Monto insieme il nuovo colore.
        ImGui::ColorConvertHSVtoRGB(tonalitàAccento, saturazioneAccento, luminosità, colore.x, colore.y, colore.z);
    }

    /// @brief Sostituisce la tonalità del colore specificato lasciando invariata la saturazione, la luminosità e la
    ///        trasparenza.
    /// @param tonalitàAccento Tonalità con cui modificare il colore specificato.
    /// @param colore Il colore da modificare.
    static void ModificaColore(float tonalitàAccento, ImVec4 &colore)
    {
        float spare1, saturazione, luminosità;

        // Estraggo la luminosità del colore da modificare. Ignoro le sue tonalità e saturazione.
        ImGui::ColorConvertRGBtoHSV(colore.x, colore.y, colore.z, spare1, saturazione, luminosità);

        // Monto insieme il nuovo colore.
        ImGui::ColorConvertHSVtoRGB(tonalitàAccento, saturazione, luminosità, colore.x, colore.y, colore.z);
    }
}
