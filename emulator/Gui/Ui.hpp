#pragma once
#include "../Emulator.hpp"
#include "SDL2/SDL.h"
#include "CodeViewer.hpp"
#include "hex.hpp"

#include "UiBase.hpp"
#include <vector>

#define MEM_EDIT_BASE_ADDR 0xD000
#define MEM_EDIT_MEM_SIZE 0x2100

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
    char* rom_addr;
    std::vector<UiBase*> ui_components;
public:
    static MemoryEditor::OptionalMarkedSpans *MARKED_SPANS;

    static void UpdateMarkedSpans(const MemoryEditor::OptionalMarkedSpans &spans);

    DebugUi();

    void PaintUi();
};