#include <cstdint>
#include <imgui.h>
#include "CodeViewer.hpp"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL.h>
#include <iostream>

#include "Ui.hpp"
#include "hex.hpp"
#include "WatchWindow.hpp"
#include "Injector.hpp"
#include "MemBreakPoint.hpp"
#include "../Peripheral/BatteryBackedRAM.hpp"

#include "../Config/Config.hpp"
ImVector<ImWchar> ranges;
UI_SINGLE_IMPL(DebugUi)
DebugUi::DebugUi()
{
    instance = this;
    ram_length = casioemu::Emulator::instance->GetModelInfo("ram_length");
    ram_start = casioemu::Emulator::instance->GetModelInfo("ram_start");
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
    io.Fonts->AddFontFromFileTTF(EmuGloConfig.GetFontPath().data(), 16.0f, nullptr, ranges.Data);
    io.Fonts->Build();
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    ui_components.push_back(new WatchWindow());
    ui_components.push_back(new Injector());
    ui_components.push_back(new MemBreakPoint());
    ui_components.push_back(new CodeViewer(casioemu::Emulator::instance->GetModelFilePath("_disas.txt")));
    rom_addr = casioemu::BatteryBackedRAM::rom_addr;
    // code_viewer=new CodeViewer(emulator->GetModelFilePath("_disas.txt"),emulator);
}

void DebugUi::DockerHelper(){
        //p_open不需要，改成nullptr
    bool* p_open = nullptr;
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_AutoHideTabBar;    ImGuiWindowFlags window_flags =  ImGuiWindowFlags_NoTitleBar| ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |=  ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("按住SHIFT，拖拽来启用docking", p_open, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();
    if (opt_fullscreen)
        ImGui::PopStyleVar(2);    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    ImGui::End();
}

void DebugUi::PaintSDL(){
    if(!need_paint)
        return;
    // SDL_RenderClear(renderer);


    SDL_RenderPresent(renderer);
    need_paint  = false;
}

void DebugUi::PaintUi(){
    if(need_paint)
        return;
    assert(MARKED_SPANS != nullptr /* initialized in casioemu.cpp */);
    auto &marked_spans = *MARKED_SPANS;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGuiIO& io = ImGui::GetIO();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    DockerHelper();

    static MemoryEditor mem_edit;
    mem_edit.ReadOnly = false;
    mem_edit.DrawWindow(EmuGloConfig[UI_MEMEDIT], rom_addr, ram_length, ram_start, marked_spans);
    for(UiBase* a:ui_components){
        a->Show();
    }
    
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    need_paint = true;

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
