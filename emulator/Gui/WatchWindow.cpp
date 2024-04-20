#include "WatchWindow.hpp"
#include "../Chipset//Chipset.hpp"
#include "../Chipset/CPU.hpp"
#include "CodeViewer.hpp"
#include "imgui.h"
#include "../Peripheral/BatteryBackedRAM.hpp"
#include <_mingw_stat64.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>

#include "../Config/Config.hpp"

WatchWindow::WatchWindow()
{
    mem_editor.OptShowOptions = false;   
    instance = this;
}

UI_SINGLE_IMPL(WatchWindow)

void WatchWindow::PrepareRX(){
    for (int i = 0; i<16; i++) {
        sprintf((char*)reg_rx[i], "%02x",casioemu::Emulator::instance
        ->chipset.cpu.reg_r[i] & 0x0ff);
    }
    sprintf(reg_pc, "%04x",casioemu::Emulator::instance
        ->chipset.cpu.reg_pc & 0xffff);
    sprintf(reg_lr, "%04x",casioemu::Emulator::instance
        ->chipset.cpu.reg_lr & 0xffff);
    sprintf(reg_sp, "%04x",casioemu::Emulator::instance
        ->chipset.cpu.reg_sp & 0xffff);
    sprintf(reg_ea, "%04x",casioemu::Emulator::instance
        ->chipset.cpu.reg_ea & 0xffff);
    sprintf(reg_psw, "%02x",casioemu::Emulator::instance
        ->chipset.cpu.reg_psw & 0xffff);
}

void WatchWindow::ShowRX(){
    char id[10];
    ImGui::TextColored(ImVec4(0,200,0,255)
        , "RXn: ");
    for (int i = 0; i<16; i++) {
        ImGui::SameLine();
        sprintf(id, "##data%d",i);
        ImGui::SetNextItemWidth(char_width*2+2);
        ImGui::InputText(id, (char*)&reg_rx[i][0],3
        ,ImGuiInputTextFlags_CharsHexadecimal);
        
    }
    //ERn
    //不可编辑，必须通过Rn编辑
    ImGui::Text("ERn: ");
    for (int i = 0; i<16; i+=2) {
        ImGui::SameLine();
        uint16_t val = casioemu::Emulator::instance->chipset.cpu.reg_r[i+1]
            <<8|casioemu::Emulator::instance->chipset.cpu.reg_r[i];
        ImGui::Text("%04x ",val);
        
    }

    auto show_sfr = ([&](char *ptr, char *label,int i,int width = 4){
        ImGui::TextColored(ImVec4(0,200,0,255)
        , label);
        ImGui::SameLine();
        sprintf(id, "##sfr%d",i);
        ImGui::SetNextItemWidth(char_width*width+2);
        ImGui::InputText(id, (char*)ptr,width+1
        ,ImGuiInputTextFlags_CharsHexadecimal);
    });
    show_sfr(reg_pc, "PC: ", 1);
    ImGui::SameLine();
    show_sfr(reg_lr, "LR: ", 2);
    ImGui::SameLine();
    show_sfr(reg_ea, "EA: ", 3);
    ImGui::SameLine();
    show_sfr(reg_sp, "SP: ", 4);
    ImGui::SameLine();
    show_sfr(reg_psw, "PSW: ", 5,2);
}

void WatchWindow::UpdateRX(){
    for (int i = 0; i<16; i++) {
        casioemu::Emulator::instance->chipset.cpu.reg_r[i]
         = (uint8_t)strtol((char*)reg_rx[i], nullptr, 16);
    }
    casioemu::Emulator::instance->chipset.cpu.reg_pc
         = (uint16_t)strtol((char*)reg_pc, nullptr, 16);
    casioemu::Emulator::instance->chipset.cpu.reg_lr
         = (uint16_t)strtol((char*)reg_lr, nullptr, 16);
    casioemu::Emulator::instance->chipset.cpu.reg_ea
         = (uint16_t)strtol((char*)reg_ea, nullptr, 16);
    casioemu::Emulator::instance->chipset.cpu.reg_sp
         = (uint16_t)strtol((char*)reg_sp, nullptr, 16);
    casioemu::Emulator::instance->chipset.cpu.reg_psw
         = (uint16_t)strtol((char*)reg_psw, nullptr, 16);
}

void WatchWindow::Show(){
    char_width = ImGui::CalcTextSize("F").x;
    ImGui::Begin(EmuGloConfig[UI_REPORT_WINDOW]);
    ImGui::BeginChild("##stack_trace",ImVec2(0,ImGui::GetWindowHeight()/5));
    casioemu::Chipset& chipset = casioemu::Emulator::instance->chipset;
    std::string s=chipset.cpu.GetBacktrace();
    ImGui::InputTextMultiline("##as",(char*)s.c_str(),s.size(),ImVec2(ImGui::GetWindowWidth(),0),ImGuiInputTextFlags_ReadOnly);
    ImGui::EndChild();
    ImGui::BeginChild("##reg_trace",ImVec2(0,ImGui::GetWindowHeight()/3));
    if(!CodeViewer::instance->isbreaked){
        ImGui::TextColored(ImVec4(255,255,0,255), "寄存器请在断点状态下查看");
        PrepareRX();
    }else {
        ShowRX();
    }

  
    ImGui::EndChild();
    static int range=64;
    ImGui::BeginChild("##stack_view");
    ImGui::Text(EmuGloConfig[UI_REPORT_RANGE]);
    ImGui::SameLine();
    ImGui::SliderInt(EmuGloConfig[UI_REPORT_RANGE_SLIDER], &range, 64, 2048);
    uint16_t offset = chipset.cpu.reg_sp&0xffff;
    mem_editor.DrawContents(casioemu::BatteryBackedRAM::rom_addr+ offset-0xd000, range,offset);
    ImGui::EndChild();
    ImGui::End();
    
}