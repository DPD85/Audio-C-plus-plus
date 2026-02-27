#pragma once

// API pubbliche delle estensioni ad ImGui create da me.
// -------------------------------------------------------

namespace ImGui
{

    // ----- Barra di stato -----

    /// Create a status bar at the bottom of the screen and append to it.
    bool BeginMainStatusBar();
    /// @remark Only call EndMainStatusBar() if BeginMainStatusBar() returns true!
    void EndMainStatusBar();

}
