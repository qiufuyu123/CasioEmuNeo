#include <imgui.h>
#include "CodeViewer.hpp"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL.h>

#include "Ui.hpp"
#include "hex.hpp"
#include "WatchWindow.hpp"
#include "Injector.hpp"
#include "../Peripheral/BatteryBackedRAM.hpp"

#include "../Config/Config.hpp"
ImVector<ImWchar> ranges;
	
DebugUi::DebugUi()
{

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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = true;
    io.FontGlobalScale = 1.0;
    
    EmuGloConfig.GetAtlas().AddRanges(io.Fonts->GetGlyphRangesChineseFull());
    EmuGloConfig.GetAtlas().BuildRanges(&ranges);
    io.Fonts->AddFontFromFileTTF(EmuGloConfig.GetFontPath().data(), 18.0f, nullptr, ranges.Data);
    io.Fonts->Build();
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    ui_components.push_back(new WatchWindow());
    ui_components.push_back(new Injector());
    ui_components.push_back(new CodeViewer(casioemu::Emulator::instance->GetModelFilePath("_disas.txt")));
    rom_addr = casioemu::BatteryBackedRAM::rom_addr;
    // code_viewer=new CodeViewer(emulator->GetModelFilePath("_disas.txt"),emulator);
}

void DebugUi::PaintUi(){
    
    assert(MARKED_SPANS != nullptr /* initialized in casioemu.cpp */);
    auto &marked_spans = *MARKED_SPANS;

    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGuiIO& io = ImGui::GetIO();
    
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    

    static MemoryEditor mem_edit;
    mem_edit.ReadOnly = false;
    mem_edit.DrawWindow(EmuGloConfig[UI_MEMEDIT], rom_addr, MEM_EDIT_MEM_SIZE, MEM_EDIT_BASE_ADDR, marked_spans);
    for(UiBase* a:ui_components){
        a->Show();
    }
    
    ImGui::Render();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);

    
}
 
MemoryEditor::OptionalMarkedSpans* DebugUi::MARKED_SPANS = nullptr;

void DebugUi::UpdateMarkedSpans(const MemoryEditor::OptionalMarkedSpans &spans) {
    delete MARKED_SPANS;
    auto leaked = new std::optional(spans);
    MARKED_SPANS = leaked;
}

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
