#include "IntestazionePrecompilata.h"

#include "Estensioni.h"

bool ImGui::BeginMainStatusBar()
{
    ImGuiContext &g          = *GImGui;
    ImGuiViewportP *viewport = (ImGuiViewportP *)GetMainViewport();

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

    float height = GetFrameHeight();

    bool is_open = BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, window_flags);
    if (!is_open)
    {
        End();
        return false;
    }

    // Temporarily disable _NoSavedSettings, in the off-chance that tables or child windows submitted within the
    // menu-bar may want to use settings. (#8356)
    g.CurrentWindow->Flags &= ~ImGuiWindowFlags_NoSavedSettings;

    BeginMenuBar();

    return is_open;
}

void ImGui::EndMainStatusBar()
{
    ImGuiContext &g = *GImGui;

    if (!g.CurrentWindow->DC.MenuBarAppending)
    {
        // Not technically testing that it is the main menu bar
        IM_ASSERT_USER_ERROR(0, "Calling EndMainStatusBar() not from a menu-bar!");
        return;
    }

    EndMenuBar();

    g.CurrentWindow->Flags |= ImGuiWindowFlags_NoSavedSettings; // Restore _NoSavedSettings (#8356)

    // When the user has left the menu layer (typically: closed menus through activation of an item), we restore focus
    // to the previous window
    // FIXME: With this strategy we won't be able to restore a NULL focus.
    if (g.CurrentWindow == g.NavWindow && g.NavLayer == ImGuiNavLayer_Main && !g.NavAnyRequest && g.ActiveId == 0)
        FocusTopMostWindowUnderOne(
            g.NavWindow,
            NULL,
            NULL,
            ImGuiFocusRequestFlags_UnlessBelowModal | ImGuiFocusRequestFlags_RestoreFocusedChild);

    End();
}
