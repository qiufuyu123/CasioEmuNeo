#include <imgui.h>
#include "CodeViewer.hpp"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL.h>

#include "Ui.hpp"
#include "hex.hpp"
#include "../Peripheral/BatteryBackedRAM.hpp"

#include "../Config/Config.hpp"
ImVector<ImWchar> ranges;
DebugUi::DebugUi(casioemu::Emulator *emu)
    :watch_win(emu),inject_win(emu){
    emulator = emu;
    window = SDL_CreateWindow(EmuGloConfig[UI_TITLE], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        exit(1);
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.WantCaptureKeyboard=true;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.FontGlobalScale = 1.0;
    
    EmuGloConfig.GetAtlas().AddRanges(io.Fonts->GetGlyphRangesDefault());
    EmuGloConfig.GetAtlas().BuildRanges(&ranges);
    io.Fonts->AddFontFromFileTTF(EmuGloConfig.GetFontPath().data(), 18.0f, nullptr, ranges.Data);
    io.Fonts->Build();
        // Query default monitor resolution
    

    //int win_w = display_bounds.w * 7 / 8, win_h = display_bounds.h * 7 / 8;
    //io.FontGlobalScale=dpi_scaling;
// ...
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Arial.ttf", dpi_scaling * 14.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //io.WantTextInput = true;
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    
    // Setup Platform/Renderer backends
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    bool show_demo_window = true;
    bool show_another_window = false;
    char buf[100]{0};
    // Main loop
    rom_addr = casioemu::BatteryBackedRAM::rom_addr;
    code_viewer=new CodeViewer(emulator->GetModelFilePath("_disas.txt"),emulator);
}

void DebugUi::PaintUi(){
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGuiIO& io = ImGui::GetIO();
    
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    mem_edit.ReadOnly = false;
    mem_edit.DrawWindow(EmuGloConfig[UI_MEMEDIT], rom_addr, 0x2100,0xd000);
    code_viewer->DrawWindow();
    watch_win.Show();
    inject_win.Show();
    // Rendering

    ImGui::Render();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
}
 
CodeViewer* DebugUi::code_viewer = nullptr;

void gui_loop(){
    
}
int test_gui(){
    //SDL_Delay(1000*5);
    
    //ImGui_ImplSDL2_InitForSDLRenderer(renderer);
}

// void gui_cleanup(){
//         // Cleanup
//     ImGui_ImplSDLRenderer2_Shutdown();
//     ImGui_ImplSDL2_Shutdown();
//     ImGui::DestroyContext();

//     SDL_DestroyRenderer(renderer);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
// }
