/*
 * =============================================================================
 * FILE: src/main.cpp
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Eng. Zain (Lead Architect)
 * =============================================================================
 * Entry point — initialises and runs the GuiManager (Dark Neon Dashboard).
 * All GUI logic lives exclusively in gui/ — this file only bootstraps it.
 * The original integration-test main() is preserved in git history.
 * =============================================================================
 */

// ---- GUI bridge (the ONLY gui-layer import allowed in src/) ----------------
#include "../gui/GuiManager.h"

#include <cstdio>

// ---------------------------------------------------------------------------
// main — launches the Dark Neon Dashboard
// ---------------------------------------------------------------------------
int main()
{
    GuiManager gui;

    if (!gui.init()) {
        fprintf(stderr, "[main] GuiManager failed to initialise. Aborting.\n");
        return 1;
    }

    gui.run();       // blocks until the user closes the window
    gui.shutdown();
    return 0;
}
