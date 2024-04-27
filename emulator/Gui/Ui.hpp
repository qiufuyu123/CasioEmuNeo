#pragma once
#include "../Emulator.hpp"
#include "SDL2/SDL.h"
#include "CodeViewer.hpp"
#include "SDL_video.h"
#include "hex.hpp"

#include "UiBase.hpp"
#include <mutex>
#include <vector>

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
    std::mutex render_lock;
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI );
    SDL_Window* window;
    SDL_Renderer* renderer;
    char* rom_addr;
    std::vector<UiBase*> ui_components;
    bool need_paint = false;

    void DockerHelper();

public:
    UI_SINGLE_HEAD(DebugUi)
    int ram_start = 0,ram_length = 0;
    static MemoryEditor::OptionalMarkedSpans *MARKED_SPANS;

    static void UpdateMarkedSpans(const MemoryEditor::OptionalMarkedSpans &spans);

    DebugUi();

    void PaintUi();

    void PaintSDL();
};