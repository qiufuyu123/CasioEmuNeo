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
    instance = this;
}

UI_SINGLE_IMPL(WatchWindow)

void WatchWindow::PrepareRX(){
    for (int i = 0; i<16; i++) {
        sprintf((char*)reg_rx[i], "%02x",casioemu::Emulator::instance
        ->chipset.cpu.reg_r[i] & 0x0ff);
    }
}

void WatchWindow::ShowRX(){
    char id[10];
    ImGui::TextColored(ImVec4(0,200,0,255), "RXn: ");
    ImGui::SameLine();
    for (int i = 0; i<16; i++) {
        sprintf(id, "##data%d",i);
        ImGui::SetNextItemWidth(char_width*2);
        ImGui::InputText(id, (char*)&reg_rx[i][0],3
        ,ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::SameLine();
    }
}

void WatchWindow::UpdateRX(){
    for (int i = 0; i<16; i++) {
        casioemu::Emulator::instance->chipset.cpu.reg_r[i]
         = (uint8_t)strtol((char*)reg_rx[i], nullptr, 16);
    }
}

void WatchWindow::Show(){
    char_width = ImGui::CalcTextSize("F").x;
    ImGui::Begin(EmuGloConfig[UI_REPORT_WINDOW]);
    ImGui::BeginChild("##stack_trace",ImVec2(0,ImGui::GetWindowHeight()/5));
    casioemu::Chipset& chipset = casioemu::Emulator::instance->chipset;
    std::string s=chipset.cpu.GetBacktrace();
    ImGui::InputTextMultiline("##as",(char*)s.c_str(),s.size(),ImVec2(ImGui::GetWindowWidth(),0),ImGuiInputTextFlags_ReadOnly);
    ImGui::EndChild();
    ImGui::BeginChild("##reg_trace",ImVec2(0,ImGui::GetWindowHeight()/5));
    if(!CodeViewer::instance->isbreaked){
        ImGui::TextColored(ImVec4(255,255,0,255), "寄存器请在断点状态下查看");
        PrepareRX();
    }else {
        ShowRX();
    }
    // if(ImGui::BeginTable("table", 2)){
    //     ImGuiListClipper clipper;
    //     clipper.Begin(16+10+2);
    //     while (clipper.Step())
    //     {
    //         for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
    //         {
    //             ImGui::TableNextRow();
    //             ImGui::TableSetColumnIndex(0);
    //             if(row>=0 && row<16){
    //                 ImGui::Text("R%d:",row);
    //             }else if(row>=16 && row<24){
    //                 ImGui::Text("ER%d:",row == 16?1:(row-16)*2);
    //             }else if(row == 24){
    //                 ImGui::Text("SP:");
    //             }else if(row == 25){
    //                 ImGui::Text("LR");
    //             }else if(row == 26){
    //                 ImGui::Text("EA");
    //             }
    //             ImGui::TableSetColumnIndex(1);
    //             if(row>=0 && row<16){
    //                 ImGui::Text("0x%02x",chipset.cpu.reg_r[row]&0x0ff);
    //             }else if(row>=16 && row<24){
    //                 int x = (row -16)*2;
    //                 ImGui::Text("0x%04x",chipset.cpu.reg_r[x+1]<<8|chipset.cpu.reg_r[x]);
    //             }else if(row == 24){
    //                 ImGui::Text("0x%04x",chipset.cpu.reg_sp&0xffff);
    //             }else if(row == 25){
    //                 ImGui::Text("0x%04x",chipset.cpu.reg_lr&0xffff);
    //             }else if(row==26){
    //                 ImGui::Text("0x%04x",chipset.cpu.reg_ea&0xffff);
    //             }
    //         }
    //     }
    //     ImGui::EndTable();
    // }
  
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