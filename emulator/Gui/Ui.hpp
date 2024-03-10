#pragma once
#include "../Emulator.hpp"
#include "SDL2/SDL.h"
#include "CodeViewer.hpp"

#include "hex.hpp"
#include "WatchWindow.hpp"
#include "Injector.hpp"


// #include "../Emulator.hpp"
// #include "CodeViewer.hpp"
// int test_gui();
// void gui_cleanup();
// void gui_loop();
// extern char *n_ram_buffer;
// extern casioemu::Emulator *m_emu;
// extern CodeViewer *code_viewer;

class DebugUi{

private:
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window;
    SDL_Renderer* renderer;
    casioemu::Emulator *emulator;
    MemoryEditor mem_edit;
    WatchWindow watch_win;
    Injector inject_win;
    char* rom_addr;
public:
    static CodeViewer* code_viewer;

    DebugUi(casioemu::Emulator* emu);

    void PaintUi();
};